/*
 * WorkerProcess.cpp
 *
 *  Created on: Jan 24, 2017
 *      Author: lord
 */

#include "WorkerProcess.h"

WorkerProcess::WorkerProcess(int fd_endpoint) :
	fd_endpoint {fd_endpoint},
	base {nullptr}
{ }

void WorkerProcess::start() {
	int pid = getpid();
	ipc_create();
	std::cout << "Emrakul DB v2.0 worker # " << pid << " started" << std::endl;
	struct event *signal_event_int, *signal_event_term;

	base = event_base_new();
	if (!base) {
		std::cerr << "Could not initialize libevent!" << std::endl;
		ipc_close();
		exit(1);
	}

	struct event* master_econn = event_new(base,
			fd_endpoint,
			EV_READ | EV_PERSIST,
			fd_channel_callback,
			this
	);
	event_add(master_econn, NULL);

	signal_event_int = evsignal_new(base, SIGINT, signal_cb, this);
	signal_event_term = evsignal_new(base, SIGTERM, signal_cb, this);

	if (!signal_event_int || event_add(signal_event_int, NULL) < 0) {
		std::cerr << "Could not create/add a signal event!" << std::endl;
		ipc_close();
		exit(1);
	}
	if (!signal_event_term || event_add(signal_event_term, NULL) < 0) {
		std::cerr << "Could not create/add a signal event!" << std::endl;
		ipc_close();
		exit(1);
	}

	event_base_dispatch(base);

	event_free(master_econn);
	event_free(signal_event_int);
	event_free(signal_event_term);
	event_base_free(base);

	std::cout << "Listener at port " << fd_endpoint << " stopped" << std::endl;
	ipc_close();
	exit(0);
}

void WorkerProcess::fd_channel_callback(evutil_socket_t fd, short events, void *user_data) {
	int client_connection_fd(0);
	char c(0);
	sock_fd_read(fd, &c, 1, &client_connection_fd);

	WorkerProcess* pWorkerProcess = static_cast<WorkerProcess*>(user_data);
	struct event_base *base = pWorkerProcess->base;

	ClientConnection* conn = new ClientConnection(base, pWorkerProcess);
	pWorkerProcess->conn_list.push_back(boost::shared_ptr<ClientConnection>(conn));
	int err = conn->establish(client_connection_fd);
	if (!err) {
		std::cout << pWorkerProcess->conn_list.size() << " connections registered" <<
				" on worker # " << getpid() << std::endl;
	}
	else {
		std::cerr << "Error establishing connection" << std::endl;
	}
}


void WorkerProcess::signal_cb(evutil_socket_t sig, short events, void *user_data) {
	WorkerProcess* pWorkerProcess = static_cast<WorkerProcess*>(user_data);
	struct event_base *base = pWorkerProcess->base;

	std::cout << "Closing all connections of worker process " << getpid() << "..." << std::endl;
	for (auto conn : pWorkerProcess->conn_list) {
		ClientConnection* conn_ptr = conn.get();
		ClientConnection::close(conn_ptr);
	}

	struct timeval delay = { 2, 0 };
	std::cout << "Exiting worker process... " << getpid() << "..." << std::endl;
	event_base_loopexit(base, &delay);
}


int WorkerProcess::execute_query(UserCommand* query, int client_connection_id) {
	std::string response;
	PosixHashTable* p_htable = hash_table.get();
	int err_code = -1;

	// TODO: support TTL in query
	//int ttl = 60;

	switch (query->get_operation()) {
	case CMD_GET:
		err_code = p_htable->get(query->get_key_param(), query->get_value_param());
		if (!err_code) {
			response.append("200 ");
			response.append(query->get_value_param());
		}
		else {
			response = "201 Key not found";
		}
		break;
	case CMD_SET:
		err_code = p_htable->set(
				query->get_key_param(),
				query->get_value_param(),
				query->get_ttl());
		if (!err_code) {
			response = "200 SET OK";
		}
		else {
			response = "302 Table degraded, no memory left";
		}
		break;
	case CMD_DEL:
		err_code = p_htable->del(query->get_key_param());
		if (!err_code) {
			response = "200 DEL OK";
		}
		else {
			response = "201 Key not found";
		}
		break;
	case CMD_NONE:
		response = "303 Invalid command or internal error";
		break;
	case CMD_TERM:
		break;
	}

	send_response_to_client(response, client_connection_id);
	return 0;
}

void WorkerProcess::send_response_to_client(std::string& response,
		int connection_id) {
	std::cout << "Response to send: " << response << std::endl;
	for (size_t conn_idx = 0; conn_idx < conn_list.size(); conn_idx++) {
		ClientConnection* current = conn_list[conn_idx].get();
		if (current->get_id() == connection_id) {
			current->send_response(response.c_str());
		}
	}
}

void WorkerProcess::remove_connection(ClientConnection* conn) {
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
