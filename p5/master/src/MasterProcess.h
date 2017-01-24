/*
 * MasterProcess.h
 *
 *  Created on: Jan 23, 2017
 *      Author: lord
 */

#ifndef MASTERPROCESS_H_
#define MASTERPROCESS_H_

#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/shm.h>

#include <wait.h>
#include <signal.h>
#include <cstdlib>

#include "CommandMessage.h"
#include "ClientConnection.h"
#include "HashTableBase.h"

class ClientConnection;

#define MASTER_MSGTYPE 1

class MasterProcess {
public:
	int start();
	MasterProcess(int port);
	void remove_connection(ClientConnection* conn);
	int execute_query(UserCommand query, int client_connection_id);
private:

	static void listener_cb(struct evconnlistener *, evutil_socket_t,
	    struct sockaddr *, int socklen, void *);
	static void signal_cb(evutil_socket_t sig, short events, void *user_data);
	void init_ipc();
	void shutdown_workers();
	void release_ipc();
	void relay_command_to_worker(CommandMessage* cmd_msg);
	void process_response_from_worker(CommandMessage& cmd_msg);
	void send_response_to_client(std::string response, int connection_id);

	const int port;
	struct event_base* base;
	int message_queue_id;
	int shared_mem_id;
	int semaphores_id;
	std::vector<boost::shared_ptr<ClientConnection>> conn_list;
};

#endif /* MASTERPROCESS_H_ */
