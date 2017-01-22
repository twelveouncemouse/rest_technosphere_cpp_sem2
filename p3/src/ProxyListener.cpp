/*
 * ProxyListener.cpp
 *
 *  Created on: Jan 21, 2017
 *      Author: Roman Vasilyev
 */

#include "ProxyListener.h"

ProxyListener::ProxyListener(const int port, const boost::shared_ptr<Selector> sel):
port { port }, selector { sel }, base { nullptr } {
}

void ProxyListener::remove_connection(Connection* conn) {
	for (std::vector<boost::shared_ptr<Connection>>::iterator current = conn_list.begin();
			current != conn_list.end();
			current++) {
		boost::shared_ptr<Connection> foo = *current;

		if (*foo == *conn) {
			conn_list.erase(current);
			std::cout << "Connection removed from list" << std::endl;
			break;
		}
	}
}

int ProxyListener::start() {
	struct evconnlistener *listener;
	struct event *signal_event;

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

	signal_event = evsignal_new(base, SIGINT, signal_cb, (void *)base);

	if (!signal_event || event_add(signal_event, NULL)<0) {
		std::cerr << "Could not create/add a signal event!" << std::endl;
		return 1;
	}

	std::cout << "Listening started" << std::endl;
	event_base_dispatch(base);

	evconnlistener_free(listener);
	event_free(signal_event);
	event_base_free(base);

	std::cout << "Listener at port " << port << " stopped" << std::endl;
	return 0;
}

void ProxyListener::listener_cb(struct evconnlistener *listener, evutil_socket_t fd,
	    struct sockaddr *sa, int socklen, void *user_data) {
	ProxyListener* pProxyListener = static_cast<ProxyListener*>(user_data);
	struct event_base *base = pProxyListener->base;

	Connection* conn = new Connection(base, pProxyListener);
	Selector selector = *(pProxyListener->selector);
	int listening_port = pProxyListener->port;
	std::pair<std::string, int> remote_server_address = selector.select(listening_port);

	pProxyListener->conn_list.push_back(boost::shared_ptr<Connection>(conn));
	int err = (*conn).establish(fd, remote_server_address);
	if (!err) {
		std::cout << pProxyListener->conn_list.size() << " connections registered" << std::endl;
	}
	else {
		std::cerr << "Error establishing connection" << std::endl;
	}
}

void ProxyListener::signal_cb(evutil_socket_t sig, short events, void *user_data)
{
	struct event_base *base = static_cast<event_base*>(user_data);
	struct timeval delay = { 2, 0 };

	std::cout << "Caught an interrupt signal; exiting cleanly in two seconds." << std::endl;
	event_base_loopexit(base, &delay);
}
