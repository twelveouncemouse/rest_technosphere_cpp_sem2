#include "HashTableBase.h"

//-----------------------------------------------------------------------------
// Murmurhash3 x86-32

static uint32_t rotl32(uint32_t x, int8_t r)
{
	return (x << r) | (x >> (32 - r));
}

#define	ROTL32(x,y)	rotl32(x,y)
#define getblock(p, i) (p[i])

static uint32_t fmix32(uint32_t h)
{
	h ^= h >> 16;
	h *= 0x85ebca6b;
	h ^= h >> 13;
	h *= 0xc2b2ae35;
	h ^= h >> 16;

	return h;
}

int HashTableBase::hash(const char* key) const {
	size_t len = strlen(key);
	uint32_t seed = 0U;
	const uint8_t * data = (const uint8_t*)key;
	const int nblocks = len / 4;
	int i;

	uint32_t h1 = seed;

	uint32_t c1 = 0xcc9e2d51;
	uint32_t c2 = 0x1b873593;

	//----------
	// body

	const uint32_t * blocks = (const uint32_t *)(data + nblocks * 4);

	for (i = -nblocks; i; i++)
	{
		uint32_t k1 = getblock(blocks, i);

		k1 *= c1;
		k1 = ROTL32(k1, 15);
		k1 *= c2;

		h1 ^= k1;
		h1 = ROTL32(h1, 13);
		h1 = h1 * 5 + 0xe6546b64;
	}

	//----------
	// tail

	const uint8_t * tail = (const uint8_t*)(data + nblocks * 4);

	uint32_t k1 = 0;

	switch (len & 3)
	{
	case 3: k1 ^= tail[2] << 16;
	case 2: k1 ^= tail[1] << 8;
	case 1: k1 ^= tail[0];
		k1 *= c1; k1 = ROTL32(k1, 15); k1 *= c2; h1 ^= k1;
	};

	//----------
	// finalization

	h1 ^= len;

	h1 = fmix32(h1);
	return h1 % table_size;
}

//-----------------------------------------------------------------------------

HashTableBase::HashTableBase(void* p_memory, int n_blocks, int block_size) :
	p_data{ static_cast<char*>(p_memory) },
	n_blocks{ n_blocks },
	block_size{ block_size },
	table_size{ n_blocks * block_size }
{
	memset(p_data, 0, RECORD_SIZE * table_size);
}

HashTableBase::~HashTableBase() { }

void HashTableBase::lock(int block_idx, bool exclusive) const {
//	std::cout << "Locked block " << block_idx <<
//		(exclusive ? " exclusive" : " non-exclusive") << std::endl;
}

void HashTableBase::unlock(int block_idx, bool exclusive) const {
//	std::cout << "Unlocked block " << block_idx <<
//		(exclusive ? " exclusive" : " non-exclusive") << std::endl;
}

int HashTableBase::find_filled(const char* key) const {
	int h = hash(key);

	int block_idx = h / block_size;
	lock(block_idx, false);
	for (int cur_idx = h; cur_idx < table_size; cur_idx++) {
		int new_block_idx = cur_idx / block_size;
		if (new_block_idx != block_idx) {
			unlock(block_idx, false);
			lock(new_block_idx, false);
			block_idx = new_block_idx;
		}
		bool found = p_data[RECORD_SIZE * cur_idx + KEY_SIZE] == CL_FILLED &&
			strcmp(key, &p_data[RECORD_SIZE * cur_idx]) == 0;
		if (found) {
			unlock(block_idx, false);
			return cur_idx;
		}
	}
	for (int cur_idx = 0; cur_idx < h; cur_idx++) {
		int new_block_idx = cur_idx / block_size;
		if (new_block_idx != block_idx) {
			unlock(block_idx, false);
			lock(new_block_idx, false);
			block_idx = new_block_idx;
		}
		bool found = p_data[RECORD_SIZE * cur_idx + KEY_SIZE] == CL_FILLED &&
			strcmp(key, &p_data[RECORD_SIZE * cur_idx]) == 0;
		if (found) {
			unlock(block_idx, false);
			return cur_idx;
		}
	}
	unlock(block_idx, false);
	return -1;
}

int HashTableBase::find_free(const char* key) const {
	int h = hash(key);
	int block_idx = h / block_size;
	lock(block_idx, false);
	for (int cur_idx = h; cur_idx < table_size; cur_idx++) {
		int new_block_idx = cur_idx / block_size;
		if (new_block_idx != block_idx) {
			unlock(block_idx, false);
			lock(new_block_idx, false);
			block_idx = new_block_idx;
		}
		bool found = (p_data[RECORD_SIZE * cur_idx + KEY_SIZE] == CL_FREE);
		if (found) {
			unlock(block_idx, false);
			return cur_idx;
		}
	}
	for (int cur_idx = 0; cur_idx < h; cur_idx++) {
		int new_block_idx = cur_idx / block_size;
		if (new_block_idx != block_idx) {
			unlock(block_idx, false);
			lock(new_block_idx, false);
			block_idx = new_block_idx;
		}
		bool found = (p_data[RECORD_SIZE * cur_idx + KEY_SIZE] == CL_FREE);
		if (found) {
			unlock(block_idx, false);
			return cur_idx;
		}
	}
	unlock(block_idx, false);
	return -1;
}

int HashTableBase::get(const char* key, char* value) const {
	int cell_idx = find_filled(key);
	int block_idx = cell_idx / block_size;
	if (cell_idx >= 0) {
		lock(block_idx, false);
		strcpy(value, &p_data[RECORD_SIZE * cell_idx + KEY_SIZE + 1]);
		unlock(block_idx, false);
		return 0;
	}
	else {
		return -1;
	}
}

int HashTableBase::set(const char* key, const char* value, int ttl) {
	int cell_idx = find_filled(key);
	if (cell_idx == -1) {
		cell_idx = find_free(key);
	}
	
	int block_idx = cell_idx / block_size;
	if (cell_idx >= 0) {
		lock(block_idx, true);
		strcpy(&p_data[RECORD_SIZE * cell_idx], key);
		strcpy(&p_data[RECORD_SIZE * cell_idx + KEY_SIZE + 1], value);
		p_data[RECORD_SIZE * cell_idx + KEY_SIZE] = 1;
		unlock(block_idx, true);
		if (ttl > 0) {
			schedule_deletion(key, ttl);
		}
		return 0;
	}
	else {
		return -1;
	}
}

int HashTableBase::del(const char* key) {
	int cell_idx = find_filled(key);
	int block_idx = cell_idx / block_size;
	if (cell_idx >= 0) {
		lock(block_idx, true);
		p_data[RECORD_SIZE * cell_idx + KEY_SIZE] = CL_RIP;
		unlock(block_idx, true);
		return 0;
	}
	else {
		return -1;
	}
}

void HashTableBase::schedule_deletion(const char* key, int ttl) const {

}
