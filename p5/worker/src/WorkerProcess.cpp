/*
 * WorkerProcess.cpp
 *
 *  Created on: Jan 24, 2017
 *      Author: lord
 */

#include "WorkerProcess.h"

WorkerProcess::WorkerProcess(int my_id) :
	worker_id {my_id},
	message_queue_id { 0 },
	shared_mem_id { 0 },
	semaphores_id { 0 },
	shared_mem_addr { nullptr } { }

void WorkerProcess::start() {
	init();

	std::cout << "Emrakul DB v1.0 worker # " << worker_id << " started" << std::endl;
	CommandMessage cmd_msg;
	while (msgrcv(message_queue_id,
			(struct msgbuf*) &cmd_msg,
			sizeof(CommandMessage),
			worker_id,
			0) > 0)	{
		int error_code = dispatch_message(&cmd_msg);
		if (error_code) {
			break;
		}
	}

	std::cout << "Worker process " << worker_id << " terminated" << std::endl;
}

int WorkerProcess::dispatch_message(CommandMessage* cmd_msg) {
	cmd_msg->mtype = MASTER_PROCESS_MSGTYPE;
	int error_code = -1;
	SysVHashTable* p_htable = hash_table.get();
	switch (cmd_msg->operation)
	{
		case CMD_SET:
			error_code = p_htable->set(cmd_msg->get_key(), cmd_msg->get_value());
			break;

		case CMD_GET:
			error_code = p_htable->get(cmd_msg->get_key(), cmd_msg->get_value());
			break;

		case CMD_DEL:
			error_code = p_htable->del(cmd_msg->get_key());
			break;

		case CMD_TERM:
			shmdt(shared_mem_addr);
			return 1;

		case CMD_NONE:
			break;
	}

	cmd_msg->status = error_code;
	msgsnd(message_queue_id, (struct msgbuf*) cmd_msg, sizeof(CommandMessage), 0);
	return 0;
}

void WorkerProcess::init() {
	key_t key = ftok("./emrakul", 'd');
	message_queue_id = msgget(key, 0666);

	size_t table_data_size =
		HASHTABLE_SEGMENT_SIZE * HASHTABLE_SEGMENTS_COUNT * RECORD_SIZE;
	shared_mem_id = shmget(key, table_data_size, 0666);
	shared_mem_addr = shmat(shared_mem_id, NULL, 0);

	semaphores_id = semget(key, HASHTABLE_SEGMENTS_COUNT + 1, 0666);

	SysVHashTable* p_table = new SysVHashTable(
			shared_mem_addr,
			semaphores_id
	);
	hash_table = boost::shared_ptr<SysVHashTable>(p_table);
}
