/*
 * PosixHashTable.cpp
 *
 *  Created on: Jan 24, 2017
 *      Author: lord
 */

#include "PosixHashTable.h"

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

int PosixHashTable::hash(const char* key) const {
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
	return h1 % (HASHTABLE_SEGMENT_SIZE * HASHTABLE_SEGMENTS_COUNT);
}

//-----------------------------------------------------------------------------

PosixHashTable::PosixHashTable(char* p_memory, sem_t** semaphores) :
	p_data{p_memory}, semaphores{semaphores} {
}

void PosixHashTable::unlock(int i) {
	sem_post(semaphores[i]);
}

void PosixHashTable::lock(int i) {
    sem_wait(semaphores[i]);
}

int PosixHashTable::find_filled(const char* key) {
	int h = hash(key);

	int block_idx = h / HASHTABLE_SEGMENT_SIZE;
	lock(block_idx);
	for (int cur_idx = h; cur_idx < HASHTABLE_SEGMENT_SIZE * HASHTABLE_SEGMENTS_COUNT; cur_idx++) {
		int new_block_idx = cur_idx / HASHTABLE_SEGMENT_SIZE;
		if (new_block_idx != block_idx) {
			unlock(block_idx);
			lock(new_block_idx);
			block_idx = new_block_idx;
		}
		bool found = p_data[RECORD_SIZE * cur_idx + KEY_SIZE] == CL_FILLED &&
			strcmp(key, &p_data[RECORD_SIZE * cur_idx]) == 0;
		if (found) {
			unlock(block_idx);
			return cur_idx;
		}
	}
	for (int cur_idx = 0; cur_idx < h; cur_idx++) {
		int new_block_idx = cur_idx / HASHTABLE_SEGMENT_SIZE;
		if (new_block_idx != block_idx) {
			unlock(block_idx);
			lock(new_block_idx);
			block_idx = new_block_idx;
		}
		bool found = p_data[RECORD_SIZE * cur_idx + KEY_SIZE] == CL_FILLED &&
			strcmp(key, &p_data[RECORD_SIZE * cur_idx]) == 0;
		if (found) {
			unlock(block_idx);
			return cur_idx;
		}
	}
	unlock(block_idx);
	return -1;
}

int PosixHashTable::find_free(const char* key) {
	int h = hash(key);
	int block_idx = h / HASHTABLE_SEGMENT_SIZE;
	lock(block_idx);
	for (int cur_idx = h;
			cur_idx < HASHTABLE_SEGMENT_SIZE * HASHTABLE_SEGMENTS_COUNT;
			cur_idx++) {
		int new_block_idx = cur_idx / HASHTABLE_SEGMENT_SIZE;
		if (new_block_idx != block_idx) {
			unlock(block_idx);
			lock(new_block_idx);
			block_idx = new_block_idx;
		}
		bool found = (p_data[RECORD_SIZE * cur_idx + KEY_SIZE] == CL_FREE);
		if (found) {
			unlock(block_idx);
			return cur_idx;
		}
	}
	for (int cur_idx = 0; cur_idx < h; cur_idx++) {
		int new_block_idx = cur_idx / HASHTABLE_SEGMENT_SIZE;
		if (new_block_idx != block_idx) {
			unlock(block_idx);
			lock(new_block_idx);
			block_idx = new_block_idx;
		}
		bool found = (p_data[RECORD_SIZE * cur_idx + KEY_SIZE] == CL_FREE);
		if (found) {
			unlock(block_idx);
			return cur_idx;
		}
	}
	unlock(block_idx);
	return -1;
}

int PosixHashTable::get(const char* key, char* value) {
	int cell_idx = find_filled(key);
	int block_idx = cell_idx / HASHTABLE_SEGMENT_SIZE;
	if (cell_idx >= 0) {
		lock(block_idx);
		strcpy(value, &p_data[RECORD_SIZE * cell_idx + KEY_SIZE + 1 + sizeof(int)]);
		unlock(block_idx);
		return 0;
	}
	else {
		return -1;
	}
}

int PosixHashTable::set(const char* key, const char* value, const int ttl) {
	int cell_idx = find_filled(key);
	if (cell_idx == -1) {
		cell_idx = find_free(key);
	}

	int block_idx = cell_idx / HASHTABLE_SEGMENT_SIZE;
	if (cell_idx >= 0) {
		lock(block_idx);
		strcpy(&p_data[RECORD_SIZE * cell_idx], key);
		int death_time = ttl + time(NULL);
		memcpy(&p_data[RECORD_SIZE * cell_idx + KEY_SIZE + 1], &death_time, sizeof(int));
		strcpy(&p_data[RECORD_SIZE * cell_idx + KEY_SIZE + 1 + sizeof(int)], value);
		p_data[RECORD_SIZE * cell_idx + KEY_SIZE] = CL_FILLED;
		unlock(block_idx);
		return 0;
	}
	else {
		return -1;
	}
}

int PosixHashTable::del(const char* key) {
	int cell_idx = find_filled(key);
	int block_idx = cell_idx / HASHTABLE_SEGMENT_SIZE;
	if (cell_idx >= 0) {
		lock(block_idx);
		p_data[RECORD_SIZE * cell_idx + KEY_SIZE] = CL_RIP;
		unlock(block_idx);
		return 0;
	}
	else {
		return -1;
	}
}

int PosixHashTable::get_ttl(const int idx) {
	int block_idx = idx / HASHTABLE_SEGMENT_SIZE;
	lock(block_idx);
	int ttl;
	memcpy(&ttl, &p_data[RECORD_SIZE * idx + KEY_SIZE + 1], sizeof(int));
	unlock(block_idx);
	return ttl;
}

int PosixHashTable::collect_garbage() {
	int collected = 0;
	int block_idx = 0;
	lock(block_idx);
	for (int cur_idx = 0;
			cur_idx < HASHTABLE_SEGMENT_SIZE * HASHTABLE_SEGMENTS_COUNT;
			cur_idx++) {
		int new_block_idx = cur_idx / HASHTABLE_SEGMENT_SIZE;
		if (new_block_idx != block_idx) {
			unlock(block_idx);
			lock(new_block_idx);
			block_idx = new_block_idx;
		}
		int current_time = time(NULL);
		int expiration_time;
		memcpy(&expiration_time, &p_data[RECORD_SIZE * cur_idx + KEY_SIZE + 1], sizeof(int));
		if (expiration_time && current_time > expiration_time) {
			// delete record
			if (p_data[RECORD_SIZE * cur_idx + KEY_SIZE] != CL_RIP) {
				p_data[RECORD_SIZE * cur_idx + KEY_SIZE] = CL_RIP;
				collected++;
			}
		}
	}
	unlock(block_idx);
	return collected;
}
