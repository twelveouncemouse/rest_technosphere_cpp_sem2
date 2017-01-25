/*
 * MasterProcess.cpp
 *
 *  Created on: Jan 23, 2017
 *      Author: lord
 */

#include "ServerProcess.h"

ServerProcess::ServerProcess(int port) :
	port { port },
	base { nullptr }
{ }

int ServerProcess::start() {
	std::cout << "Emrakul DB v2.0 master process # " << getpid() << " started" << std::endl;
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

	listener = evconnlistener_new_bind(base, listener_cb, NULL,
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

	std::cout << "Listening at port " << port << " started" << std::endl;
	event_base_dispatch(base);

	evconnlistener_free(listener);
	event_free(signal_event_int);
	event_free(signal_event_term);
	event_base_free(base);
	std::cout << "Master Listener at port " << port << " stopped" << std::endl;

	std::cout << "Deleting IPC shared resources..." << std::endl;
	SynchronizedProcess::ipc_delete();
	std::cout << "IPC shared resources free" << std::endl;
	return 0;
}

void ServerProcess::listener_cb(struct evconnlistener *listener, evutil_socket_t fd,
	    struct sockaddr *sa, int socklen, void *user_data) {

    if (fd > 0) {
        set_nonblock(fd);
        int worker_id = rand() % WORKER_PROCESSES_COUNT;
        ssize_t ret = sock_fd_write(fd_channel_hub[worker_id], const_cast<char *>("1"), 1, fd);
    }
}

void ServerProcess::signal_cb(evutil_socket_t sig, short events, void *user_data) {
	ServerProcess* pMasterProcess = static_cast<ServerProcess*>(user_data);
	struct event_base *base = pMasterProcess->base;

	std::cout << "Signal received, waiting for child processes to exit..." << std::endl;
	for (pid_t pid : ServerProcess::child_procs) {
		kill(pid, SIGTERM);
	}
	while (wait(NULL) > 0);
	std::cout << "Child processes stopped" << std::endl;

	struct timeval delay = { 2, 0 };
	std::cout << "Exiting master process..." << std::endl;
	event_base_loopexit(base, &delay);
}
