#pragma once

#include <cstring>
#include <iostream>

#define KEY_SIZE 63
#define VALUE_SIZE 960
#define RECORD_SIZE 1024

#define CL_FREE 0
#define CL_FILLED 1
#define CL_RIP 2

class HashTableBase
{
public:
	int get(const char* key, char* value) const;
	int set(const char* key, const char* value, int ttl = 0);
	int del(const char* key);
	HashTableBase(void* p_memory, int n_blocks, int block_size);
	virtual ~HashTableBase();
private:
	char* const p_data;
	const int n_blocks;
	const int block_size;
	const int table_size;
	//int free_counter;

	int hash(const char* key) const;
	int find_filled(const char* key) const;
	int find_free(const char* key) const;
	virtual void lock(int block_idx, bool exclusive) const;
	virtual void unlock(int block_idx, bool exclusive) const;
	virtual void schedule_deletion(const char* key, int ttl) const;
};

