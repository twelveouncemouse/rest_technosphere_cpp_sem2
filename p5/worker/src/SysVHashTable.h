/*
 * SysVHashTable.h
 *
 *  Created on: Jan 24, 2017
 *      Author: lord
 */

#ifndef SYSVHASHTABLE_H_
#define SYSVHASHTABLE_H_

#include <sys/ipc.h>
#include <sys/sem.h>

#include "HashTableBase.h"

class SysVHashTable: public HashTableBase {
public:
	SysVHashTable(void* p_memory, int semaphores_id);
	virtual ~SysVHashTable();

protected:
	virtual void lock(int block_idx, bool exclusive) const;
	virtual void unlock(int block_idx, bool exclusive) const;
	virtual void schedule_deletion(const char* key, int ttl) const;

private:
	void sem_V(uint16_t i, short c=1) const;
	void sem_P(uint16_t i, short c=1) const;

	int semaphores_id;
};

#endif /* SYSVHASHTABLE_H_ */
