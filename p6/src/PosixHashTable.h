/*
 * PosixHashTable.h
 *
 *  Created on: Jan 24, 2017
 *      Author: lord
 */

#ifndef POSIXHASHTABLE_H_
#define POSIXHASHTABLE_H_

#include <iostream>
#include <cstring>
#include <ctime>
#include <semaphore.h>

#define WORKER_PROCESSES_COUNT 4

#define KEY_SIZE 63
#define VALUE_SIZE 956
#define RECORD_SIZE 1024
#define HASHTABLE_SEGMENTS_COUNT 8
#define HASHTABLE_SEGMENT_SIZE 128

#define CL_FREE 0
#define CL_FILLED 1
#define CL_RIP 2

class PosixHashTable {
public:
	int get(const char* key, char* value);
	int get_ttl(const int idx);
	int set(const char* key, const char* value, const int ttl);
	int del(const char* key);
	int collect_garbage();
	PosixHashTable(char* p_memory, sem_t** semaphores);

private:
	int hash(const char* key) const;
	int find_filled(const char* key);
	int find_free(const char* key);
	void lock(int block_idx);
	void unlock(int block_idx);

	char* const p_data;
	sem_t **semaphores;
};

#endif /* POSIXHASHTABLE_H_ */
