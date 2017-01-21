/*
 * Connection.cpp
 *
 *  Created on: Jan 21, 2017
 *      Author: Roman Vasilyev
 */

#include "Connection.h"

int Connection::id_base;

Connection::Connection(event_base* base, ProxyListener* owner):
		base { base },
		owner { owner },
		connection_counter { 0 },
		bev_in { nullptr },
		bev_out { nullptr },
		id { id_base++ }
{
}

int Connection::establish(int incoming_fd, std::pair<std::string, int> remote_server_address) {
	bev_in = bufferevent_socket_new(base, incoming_fd, BEV_OPT_CLOSE_ON_FREE);
	if (!bev_in) {
		std::cerr << "Error constructing bufferevent!" << std::endl;
		event_base_loopbreak(base);
		return 1;
	}

	std::cout << "Input bufferevent created" << std::endl;

	// Create socket to target server
	int farseek_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (farseek_socket == -1) {
		std::cout << strerror(errno) << std::endl;
		return 1;
	}
	std::cout << "Remote socket created" << std::endl;

	struct sockaddr_in SockAddr;
	SockAddr.sin_family = AF_INET;
	SockAddr.sin_port = htons(remote_server_address.second);
	inet_pton(AF_INET, remote_server_address.first.c_str(), &(SockAddr.sin_addr));
	evutil_make_socket_nonblocking(farseek_socket);
	bev_out = bufferevent_socket_new(base, farseek_socket, BEV_OPT_CLOSE_ON_FREE);
	if (!bev_out) {
		std::cerr << "Error constructing bufferevent!" << std::endl;
		event_base_loopbreak(base);
		return 1;
	}

	bufferevent_setcb(bev_in, conn_readcb, NULL, conn_eventcb, this);
	bufferevent_setcb(bev_out, conn_readcb, NULL, conn_eventcb, this);

	bufferevent_enable(bev_in, EV_READ | EV_WRITE);
	bufferevent_enable(bev_out, EV_READ | EV_WRITE);

	std::cout << "Buffer events callbacks added" << std::endl;

	int conn_result = connect(farseek_socket, static_cast<struct sockaddr*>(&SockAddr), sizeof(SockAddr)) < 0;
	if(conn_result == -1) {
		std::cout << strerror(errno) << std::endl;
		return 1;
	}

	connection_counter = 2;
	std::cout << "Connected to target server" << std::endl;
	return 0;
}

void Connection::self_remove() {
	owner->remove_connection(this);
}

bool Connection::operator==(const Connection &other) const {
    return this->id == other.id;
}

void Connection::conn_readcb(struct bufferevent *bev, void *user_data) {
	Connection* connObject = static_cast<Connection*>(user_data);
	struct bufferevent *counterpart =
			(connObject->bev_in == bev) ? connObject->bev_out : connObject->bev_in;

	struct evbuffer *output = bufferevent_get_output(counterpart);
	struct evbuffer *input = bufferevent_get_input(bev);
	evbuffer_add_buffer(output, input);
}

void Connection::conn_eventcb(struct bufferevent *bev, short events, void *user_data) {
	Connection* connObject = static_cast<Connection*>(user_data);
	struct bufferevent *counterpart =
			(connObject->bev_in == bev) ? connObject->bev_out : connObject->bev_in;

	if (events & BEV_EVENT_EOF) {
		std::cout << "Caught EOF" << std::endl;
		connObject->connection_counter--;
		conn_readcb(bev, connObject);
		bufferevent_disable(bev, EV_READ);
		if (connObject->connection_counter == 0) {
			close_connection(bev, counterpart, connObject);
		}
	} else if (events & BEV_EVENT_ERROR) {
		std::cerr << "Got an error on the connection:" << strerror(errno) << std::endl;
		close_connection(bev, counterpart, connObject);
	}
}

void Connection::close_connection(
		struct bufferevent *bev,
		struct bufferevent *counterpart,
		Connection* connObject) {
	bufferevent_free(bev);
	bufferevent_free(counterpart);
	std::cout << "Connections closed" << std::endl;
	connObject->self_remove();
}
