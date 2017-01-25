/*
 * SynchronizedProcess.h
 *
 *  Created on: Jan 24, 2017
 *      Author: lord
 */

#ifndef SYNCHRONIZEDPROCESS_H_
#define SYNCHRONIZEDPROCESS_H_

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <set>
#include <errno.h>
#include <sys/types.h>
#include <system_error>
#include <boost/shared_ptr.hpp>

#include <stdexcept>

#include "PosixHashTable.h"

class SynchronizedProcess {
public:
	static void ipc_delete();
	SynchronizedProcess();
	~SynchronizedProcess();

protected:
	int ipc_create();
	void ipc_close();

	int shared_mem_fd;
	void *shared_mem_addr;
	sem_t *semaphores[HASHTABLE_SEGMENTS_COUNT];
	boost::shared_ptr<PosixHashTable> hash_table;
};

#endif /* SYNCHRONIZEDPROCESS_H_ */
