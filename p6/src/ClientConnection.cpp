/*
 * ClientConnection.cpp
 *
 *  Created on: Jan 23, 2017
 *      Author: lord
 */

#include "ClientConnection.h"

int ClientConnection::id_base;

ClientConnection::ClientConnection(event_base* base, WorkerProcess* owner):
		base { base },
		owner { owner },
		bev_in { nullptr },
		id { id_base++ }
{
}

int ClientConnection::establish(int incoming_fd) {
	bev_in = bufferevent_socket_new(base, incoming_fd, BEV_OPT_CLOSE_ON_FREE);
	if (!bev_in) {
		std::cerr << "Error constructing bufferevent!" << std::endl;
		event_base_loopbreak(base);
		return 1;
	}

	bufferevent_setcb(bev_in, conn_readcb, NULL, conn_eventcb, this);
	bufferevent_enable(bev_in, EV_READ | EV_WRITE);

	struct evbuffer* output_buf = bufferevent_get_output(bev_in);
	evbuffer_add_printf(output_buf, "Emrakul DB v2.0 welcomes you!\n");
	evbuffer_add_printf(output_buf, "Usage:\n");
	evbuffer_add_printf(output_buf, "get key value\n");
	evbuffer_add_printf(output_buf, "set ttl key value\n");
	evbuffer_add_printf(output_buf, "del key\n");

	std::cout << "Connection with client established" << std::endl;
	return 0;
}

void ClientConnection::self_remove() {
	owner->remove_connection(this);
}

bool ClientConnection::operator==(const ClientConnection &other) const {
    return this->id == other.id;
}

void ClientConnection::conn_readcb(struct bufferevent *bev, void *user_data) {
	ClientConnection* connObject = static_cast<ClientConnection*>(user_data);
	struct evbuffer *input = bufferevent_get_input(bev);
	UserCommand* command = UserCommand::read_from_buffer(input);

	std::cout << "Command parsed" << std::endl;

	connObject->owner->execute_query(command, connObject->id);
	delete command;
}

void ClientConnection::conn_eventcb(struct bufferevent *bev, short events, void *user_data) {
	ClientConnection* connObject = static_cast<ClientConnection*>(user_data);

	if (events & BEV_EVENT_EOF) {
		std::cout << "Caught EOF" << std::endl;
		close(connObject);
	} else if (events & BEV_EVENT_ERROR) {
		std::cerr << "Got an error on the connection:" <<
				strerror(errno) << std::endl;
		close(connObject);
	}
}

void ClientConnection::close(ClientConnection* connObject) {
	bufferevent_free(connObject->bev_in);
	std::cout << "Connection " << connObject->id << " closed" << std::endl;
	connObject->self_remove();
}

void ClientConnection::send_response(const char* response) {
	struct evbuffer* buffer = bufferevent_get_output(bev_in);
	evbuffer_add_printf(buffer, "%s\n", response);
}

const int ClientConnection::get_id() const {
	return id;
}
