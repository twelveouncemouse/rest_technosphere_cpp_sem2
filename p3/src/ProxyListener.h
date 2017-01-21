/*
 * ProxyListener.h
 *
 *  Created on: Jan 21, 2017
 *      Author: lord
 */

#ifndef PROXYLISTENER_H_
#define PROXYLISTENER_H_

#include "Selector.h"
#include "Connection.h"
#include <signal.h>

class Connection;

class ProxyListener {
public:
	ProxyListener(const int port, const boost::shared_ptr<Selector> sel);
	int start();
	void remove_connection(Connection* conn);
private:
	const int port;
	const boost::shared_ptr<Selector> selector;
	struct event_base* base;
	std::vector<boost::shared_ptr<Connection>> conn_list;
	static void listener_cb(struct evconnlistener *, evutil_socket_t,
	    struct sockaddr *, int socklen, void *);
	static void signal_cb(evutil_socket_t sig, short events, void *user_data);
};

#endif /* PROXYLISTENER_H_ */
