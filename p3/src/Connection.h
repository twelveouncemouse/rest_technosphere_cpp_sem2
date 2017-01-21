/*
 * Connection.h
 *
 *  Created on: Jan 21, 2017
 *      Author: lord
 */

#ifndef CONNECTION_H_
#define CONNECTION_H_

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

#include "ProxyListener.h"

class ProxyListener;

class Connection {
public:
	Connection(event_base* base, ProxyListener* owner);
	int establish(int incoming_fd, std::pair<std::string, int> remote_server_address);
	void self_remove();
	bool operator==(const Connection &other) const;
private:
	static int id_base;
	static void conn_readcb(struct bufferevent *bev, void *user_data);
	static void conn_eventcb(struct bufferevent *bev, short events, void *user_data);
	static void close_connection(struct bufferevent *bev,
			struct bufferevent *counterpart,
			Connection* connObject);

	struct event_base* base;
	ProxyListener* owner;
	int connection_counter;
	struct bufferevent* bev_in;
	struct bufferevent* bev_out;
	const int id;
};

#endif /* CONNECTION_H_ */
