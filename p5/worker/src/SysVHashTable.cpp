/*
 * SysVHashTable.cpp
 *
 *  Created on: Jan 24, 2017
 *      Author: lord
 */

#include "SysVHashTable.h"

SysVHashTable::SysVHashTable(void* p_memory, int semaphores_id) :
	HashTableBase(p_memory), semaphores_id(semaphores_id) {
}

SysVHashTable::~SysVHashTable() { }

void SysVHashTable::sem_V(uint16_t i, short c) const {
	struct sembuf sem_operations;
	sem_operations.sem_num = i;
	sem_operations.sem_flg = 0;
	sem_operations.sem_op =  c;
    semop(semaphores_id, &sem_operations, 1);
}

void SysVHashTable::sem_P(uint16_t i, short c) const {
    sem_V(i, -c);
}

void SysVHashTable::lock(int block_idx, bool exclusive) const {
	short int count = exclusive ? WORKER_PROCESSES_COUNT : 1;
	sem_P(uint16_t(block_idx), count);
}

void SysVHashTable::unlock(int block_idx, bool exclusive) const {
	short int count = exclusive ? WORKER_PROCESSES_COUNT : 1;
	sem_V(uint16_t(block_idx), count);
}

void SysVHashTable::schedule_deletion(const char* key, int ttl) const {
}
