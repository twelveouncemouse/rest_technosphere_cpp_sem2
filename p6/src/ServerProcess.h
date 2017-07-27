/*
 * MasterProcess.h
 *
 *  Created on: Jan 23, 2017
 *      Author: lord
 */

#ifndef SERVERPROCESS_H_
#define SERVERPROCESS_H_

#include <event2/buffer.h>
#include <event2/event.h>
#include <event2/listener.h>

#include <wait.h>
#include <signal.h>
#include <cstdlib>
#include <set>

#include "SynchronizedProcess.h"
#include "PosixHashTable.h"
#include "SocketTricks.h"


class ServerProcess {
public:
	int start();
	ServerProcess(int port);

	static std::set<pid_t> child_procs;
	static int* fd_channel_hub;

private:

	static void listener_cb(struct evconnlistener *, evutil_socket_t,
	    struct sockaddr *, int socklen, void *);
	static void signal_cb(evutil_socket_t sig, short events, void *user_data);

	void send_fd_to_worker(int fd);


	const int port;
	struct event_base* base;
};

#endif /* SERVERPROCESS_H_ */
