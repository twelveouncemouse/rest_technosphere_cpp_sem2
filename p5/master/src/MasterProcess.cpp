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
message_queue_id {0},
shared_mem_id {0},
semaphores_id {0}
{ }

int MasterProcess::start() {
	init_ipc();

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

	shutdown_workers();
	release_ipc();
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
	int err = conn->establish(fd);
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

	struct timeval delay = { 2, 0 };
	std::cout << "Signal received, exiting..." << std::endl;
	event_base_loopexit(base, &delay);
}

int MasterProcess::execute_query(UserCommand* query, int client_connection_id) {
	if (query->get_operation() == CMD_NONE) {
		// Invalid command case
		std::string response = "303 Invalid command or internal error";
		send_response_to_client(response, client_connection_id);
		return 0;
	}

	CommandMessage comm_msg;
	comm_msg.operation = query->get_operation();
	memcpy(comm_msg.data, query->get_key_param(), RECORD_SIZE * sizeof(char));
	comm_msg.target_connection_id = client_connection_id;
	comm_msg.mtype = 1 + rand() % WORKER_PROCESSES_COUNT;

	relay_command_to_worker(&comm_msg);

	if (msgrcv(message_queue_id,
			(struct msgbuf*) &comm_msg,
			sizeof(CommandMessage),
			MASTER_MSGTYPE,
			0) != -1) {
		process_response_from_worker(&comm_msg);
		return 0;
	}
	return -1;
}

void MasterProcess::init_ipc() {
    key_t key = ftok("./emrakul", 'd');

	message_queue_id = msgget(key, 0666 | IPC_CREAT);
	size_t table_data_size = HASHTABLE_SEGMENT_SIZE * HASHTABLE_SEGMENTS_COUNT * RECORD_SIZE;
	shared_mem_id = shmget(key, table_data_size, 0666 | IPC_CREAT);
    semaphores_id = semget(key, HASHTABLE_SEGMENTS_COUNT + 1, 0666 | IPC_CREAT);

    ushort *init_sem = (ushort *) calloc(HASHTABLE_SEGMENTS_COUNT + 1, sizeof(ushort));
    for (int i = 0; i < HASHTABLE_SEGMENTS_COUNT; i++) {
        init_sem[i] = WORKER_PROCESSES_COUNT;
    }
    init_sem[HASHTABLE_SEGMENTS_COUNT] = 1;

    semctl(semaphores_id, 0, SETALL, (union semun*)init_sem);
    free(init_sem);

    void *p_shared_mem = shmat(shared_mem_id, NULL, 0);
    memset(p_shared_mem, 0, sizeof(char) * table_data_size);
    shmdt(p_shared_mem);

    char worker_id[4];
    for (int i = 0; i < WORKER_PROCESSES_COUNT; i++) {
        if (fork() == 0) {
            sprintf(worker_id, "%d", i + 2);
            execlp("./p5-worker", "./p5-worker", worker_id, 0);
            std::cout << "Error spawning worker process" << std::endl;
            exit(1);
        }
    }
}

void MasterProcess::shutdown_workers() {
    CommandMessage term_msg;
    term_msg.operation = CMD_TERM;
    for (int i = 0; i < WORKER_PROCESSES_COUNT; i++) {
    	term_msg.mtype = i + 2;
        relay_command_to_worker(&term_msg);
    }

    while (wait(NULL) > 0);
}

void MasterProcess::release_ipc() {
    semctl(semaphores_id, 0, IPC_RMID, 0);
    shmctl(shared_mem_id, IPC_RMID, 0);
    msgctl(message_queue_id, IPC_RMID, 0);
}

void MasterProcess::relay_command_to_worker(CommandMessage* cmd_msg) {
	msgsnd(message_queue_id, (struct msgbuf*) cmd_msg, sizeof(CommandMessage), 0);
}

void MasterProcess::process_response_from_worker(CommandMessage* response_msg) {
	std::string response;
	int err_code = response_msg->status;
	switch (response_msg->operation) {
	case CMD_GET:
		if (!err_code) {
			response.append("200 ");
			response.append(response_msg->get_value());
		}
		else {
			response = "301 Key not found";
		}
		break;
	case CMD_SET:
		if (!err_code) {
			response = "200 SET OK";
		}
		else {
			response = "301 Key not found";
		}
		break;
	case CMD_DEL:
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
	case CMD_TERM:
		break;
	}

	send_response_to_client(response, response_msg->target_connection_id);
}

void MasterProcess::send_response_to_client(std::string& response,
		int connection_id) {
	std::cout << "Response to send: " << response << std::endl;
	for (size_t conn_idx = 0; conn_idx < conn_list.size(); conn_idx++) {
		ClientConnection* current = conn_list[conn_idx].get();
		if (current->get_id() == connection_id) {
			current->send_response(response.c_str());
		}
	}
}
