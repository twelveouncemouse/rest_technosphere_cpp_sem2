/*
 * MasterProcess.cpp
 *
 *  Created on: Jan 23, 2017
 *      Author: lord
 */

#include "MasterProcess.h"

MasterProcess::MasterProcess(int port) :
port {port},
base { nullptr },
hash_table {nullptr}
{ }

int MasterProcess::start() {
	init();

	struct evconnlistener *listener;
	struct event *signal_event_int, *signal_event_term;

	struct sockaddr_in sin;
	base = event_base_new();
	if (!base) {
		std::cerr << "Could not initialize libevent!" << std::endl;
		return 1;
	}

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);

	listener = evconnlistener_new_bind(base, listener_cb, (void *)this,
		LEV_OPT_REUSEABLE|LEV_OPT_CLOSE_ON_FREE, -1,
		(struct sockaddr*)&sin,
		sizeof(sin));

	if (!listener) {
		std::cerr << "Could not initialize listener!" << std::endl;
		return 1;
	}

	signal_event_int = evsignal_new(base, SIGINT, signal_cb, this);
	signal_event_term = evsignal_new(base, SIGTERM, signal_cb, this);

	if (!signal_event_int || event_add(signal_event_int, NULL) < 0) {
		std::cerr << "Could not create/add a signal event!" << std::endl;
		return 1;
	}
	if (!signal_event_term || event_add(signal_event_term, NULL) < 0) {
		std::cerr << "Could not create/add a signal event!" << std::endl;
		return 1;
	}

	std::cout << "Listening started" << std::endl;
	event_base_dispatch(base);

	evconnlistener_free(listener);
	event_free(signal_event_int);
	event_free(signal_event_term);
	event_base_free(base);

	std::cout << "Listener at port " << port << " stopped" << std::endl;
	return 0;
}

void MasterProcess::remove_connection(ClientConnection* conn) {
	for (std::vector<boost::shared_ptr<ClientConnection>>::iterator current = conn_list.begin();
			current != conn_list.end();
			current++) {
		boost::shared_ptr<ClientConnection> foo = *current;

		if (*foo == *conn) {
			conn_list.erase(current);
			std::cout << "Connection removed from list" << std::endl;
			break;
		}
	}
}

void MasterProcess::listener_cb(struct evconnlistener *listener, evutil_socket_t fd,
	    struct sockaddr *sa, int socklen, void *user_data) {
	MasterProcess* pMasterProcess = static_cast<MasterProcess*>(user_data);
	struct event_base *base = pMasterProcess->base;

	ClientConnection* conn = new ClientConnection(base, pMasterProcess);
	pMasterProcess->conn_list.push_back(boost::shared_ptr<ClientConnection>(conn));
	int err = (*conn).establish(fd);
	if (!err) {
		std::cout << pMasterProcess->conn_list.size() << " connections registered" << std::endl;
	}
	else {
		std::cerr << "Error establishing connection" << std::endl;
	}
}

void MasterProcess::signal_cb(evutil_socket_t sig, short events, void *user_data)
{
	MasterProcess* pMasterProcess = static_cast<MasterProcess*>(user_data);
	struct event_base *base = pMasterProcess->base;
	for (auto conn : pMasterProcess->conn_list) {
		ClientConnection* conn_ptr = conn.get();
		ClientConnection::close(conn_ptr);
	}
	//struct event_base *base = static_cast<event_base*>(user_data);
	struct timeval delay = { 2, 0 };

	std::cout << "Caught an interrupt signal; exiting cleanly in two seconds." << std::endl;

	// TODO: TERM and INT signals handling
	event_base_loopexit(base, &delay);
}

int MasterProcess::execute_query(UserCommand* query) {
	// TODO: Execute command
//	std::cout << "Prihod slovlen" << std::endl;
//	std::cout << query.get_operation() << " key: " << query.get_key_param();
//	if (query.get_operation() == CMD_SET) {
//		std::cout << " value: " << query.get_value_param();
//	}
//	std::cout << std::endl;

	std::string response;
	int err_code = -1;
	switch (query->get_operation()) {
	case CMD_GET:
		err_code = hash_table.get()->get(query->get_key_param(), query->get_value_param());
		if (!err_code) {
			response.append("200 ");
			response.append(query->get_value_param());
		}
		else {
			response = "301 Key not found";
		}
		break;
	case CMD_SET:
		err_code = hash_table.get()->set(query->get_key_param(), query->get_value_param());
		if (!err_code) {
			response = "200 SET OK";
		}
		else {
			response = "301 Key not found";
		}
		break;
	case CMD_DEL:
		err_code = hash_table.get()->del(query->get_key_param());
		if (!err_code) {
			response = "200 DEL OK";
		}
		else {
			response = "301 Key not found";
		}
		break;
	case CMD_NONE:
		response = "303 Invalid command or internal error";
		break;
	}

	std::cout << "Response to send: " << response << std::endl;
	// TODO: Determine target connection
	ClientConnection* conn = conn_list[0].get();
	//const char* response = "F-OFF!";
	conn->send_response(response.c_str());
	return 0;
}

void MasterProcess::init() {
	char* p_shared_mem = new char[RECORD_SIZE * 256];
	table_mem = boost::shared_ptr<char>(p_shared_mem);
	HashTableBase* p_table = new HashTableBase(table_mem.get(), 8, 32);
	hash_table = boost::shared_ptr<HashTableBase>(p_table);
}
