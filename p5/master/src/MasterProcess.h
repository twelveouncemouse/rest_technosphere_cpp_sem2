/*
 * MasterProcess.h
 *
 *  Created on: Jan 23, 2017
 *      Author: lord
 */

#ifndef MASTERPROCESS_H_
#define MASTERPROCESS_H_

#include <signal.h>
#include "UserCommand.h"
#include "ClientConnection.h"
#include "HashTableBase.h"

class ClientConnection;

class MasterProcess {
public:
	int start();
	MasterProcess(int port);
	void remove_connection(ClientConnection* conn);
	int execute_query(UserCommand* query);
private:
	const int port;
	struct event_base* base;
	boost::shared_ptr<HashTableBase> hash_table;
	boost::shared_ptr<char> table_mem;
	std::vector<boost::shared_ptr<ClientConnection>> conn_list;
	static void listener_cb(struct evconnlistener *, evutil_socket_t,
	    struct sockaddr *, int socklen, void *);
	static void signal_cb(evutil_socket_t sig, short events, void *user_data);
	void init();
};

#endif /* MASTERPROCESS_H_ */
