/*
 * Server.cpp
 *
 *  Created on: Jan 21, 2017
 *      Author: lord
 */

#include "Server.h"

Server::Server(const std::string config) {
	Selector* pSelector = new Selector(config);
	this->selector = boost::shared_ptr<Selector>(pSelector);
}

void Server::start() {
	std::vector<int> ports = (*selector).get_incoming_ports();
	for (int port: ports) {
		ProxyListener* listener = new ProxyListener(port, selector);
		listeners.push_back(boost::shared_ptr<ProxyListener>(listener));
		listener->start();
	}
}
