/*
 * SynchronizedProcess.cpp
 *
 *  Created on: Jan 24, 2017
 *      Author: lord
 */

#include "SynchronizedProcess.h"

SynchronizedProcess::SynchronizedProcess() :
	shared_mem_fd {0},
	shared_mem_addr {nullptr},
	semaphores {nullptr}
{ }

SynchronizedProcess::~SynchronizedProcess() {

}

void SynchronizedProcess::ipc_delete() {
    shm_unlink("emrakul_ht_memory");

    for (int i = 0; i < HASHTABLE_SEGMENTS_COUNT; i++) {
        char temp[16];
        sprintf(temp, "/ht_lock_%03d", i);
        sem_unlink(temp);
    }
}

int SynchronizedProcess::ipc_create() {
	size_t table_size = HASHTABLE_SEGMENTS_COUNT * HASHTABLE_SEGMENT_SIZE * RECORD_SIZE;
    if ((shared_mem_fd = shm_open("emrakul_ht_memory", O_CREAT | O_RDWR, 0666)) == -1) {
    	throw std::runtime_error(strerror(errno));
    }

    if (ftruncate(shared_mem_fd, table_size) == -1) {
    	throw std::runtime_error(strerror(errno));
    }

    if ((shared_mem_addr = mmap(0, table_size, PROT_WRITE, MAP_SHARED, shared_mem_fd, 0)) == MAP_FAILED) {
    	throw std::runtime_error(strerror(errno));
    }
    memset(shared_mem_addr, 0, table_size * sizeof(char));

    for (int i = 0; i < HASHTABLE_SEGMENTS_COUNT; i++) {
        char temp[16];
        sprintf(temp, "/ht_lock_%03d", i);
        semaphores[i] = sem_open(temp, O_CREAT, 0666, 1);
    }

	PosixHashTable* p_table = new PosixHashTable(
			static_cast<char*>(shared_mem_addr),
			semaphores
	);
	hash_table = boost::shared_ptr<PosixHashTable>(p_table);
	return 0;
}

void SynchronizedProcess::ipc_close() {
	size_t table_size = HASHTABLE_SEGMENTS_COUNT * HASHTABLE_SEGMENT_SIZE * RECORD_SIZE;
    munmap(shared_mem_addr, table_size);
    close(shared_mem_fd);

    for (int i = 0; i < HASHTABLE_SEGMENTS_COUNT; i++) {
        sem_close(semaphores[i]);
    }
}
