/*
 * WorkerProcess.h
 *
 *  Created on: Jan 24, 2017
 *      Author: lord
 */

#ifndef WORKERPROCESS_H_
#define WORKERPROCESS_H_

#define MASTER_PROCESS_MSGTYPE 1

#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <boost/shared_ptr.hpp>

#include "CommandMessage.h"
#include "SysVHashTable.h"

class WorkerProcess {
public:
	WorkerProcess(int my_id);
	void start();

private:
	void init();
	int dispatch_message(CommandMessage* cmd_msg);

	int worker_id;
	int message_queue_id;
	int shared_mem_id;
	int semaphores_id;
	void *shared_mem_addr;
	boost::shared_ptr<SysVHashTable> hash_table;
};

#endif /* WORKERPROCESS_H_ */
