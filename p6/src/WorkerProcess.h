/*
 * WorkerProcess.h
 *
 *  Created on: Jan 24, 2017
 *      Author: lord
 */

#ifndef WORKERPROCESS_H_
#define WORKERPROCESS_H_

#define MASTER_PROCESS_MSGTYPE 1

#include <signal.h>

#include <boost/shared_ptr.hpp>

#include <event2/buffer.h>
#include <event2/event.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>

#include <sys/socket.h>

#include "UserCommand.h"
#include "ClientConnection.h"
#include "SynchronizedProcess.h"
#include "SocketTricks.h"

class ClientConnection;

class WorkerProcess : SynchronizedProcess {
public:
	WorkerProcess(int fd_endpoint);
	void start();
	int execute_query(UserCommand* query, int client_connection_id);
	void remove_connection(ClientConnection* conn);

private:
	void send_response_to_client(std::string& response, int connection_id);
	static void fd_channel_callback(evutil_socket_t fd, short events, void *arg);
	static void signal_cb(evutil_socket_t sig, short events, void *user_data);

	int fd_endpoint;
	struct event_base* base;
	std::vector<boost::shared_ptr<ClientConnection>> conn_list;
};

#endif /* WORKERPROCESS_H_ */
