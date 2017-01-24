/*
 * ClientConnection.h
 *
 *  Created on: Jan 23, 2017
 *      Author: lord
 */

#ifndef CLIENTCONNECTION_H_
#define CLIENTCONNECTION_H_

#include <vector>
#include <boost/shared_ptr.hpp>
#include <iostream>
#include <cerrno>
#include <cstring>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <event2/event.h>
#include <event2/listener.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>

#include "MasterProcess.h"
#include "UserCommand.h"

class MasterProcess;

class ClientConnection {
public:
	ClientConnection(event_base* base, MasterProcess* owner);
	int establish(int incoming_fd);
	static void close(ClientConnection* connObject);
	void self_remove();
	void send_response(const char* response);
	bool operator==(const ClientConnection &other) const;
private:
	static int id_base;
	static void conn_readcb(struct bufferevent *bev, void *user_data);
	static void conn_eventcb(struct bufferevent *bev, short events, void *user_data);

	struct event_base* base;
	MasterProcess* owner;
	struct bufferevent* bev_in;
	const int id;
};

#endif /* CLIENTCONNECTION_H_ */
