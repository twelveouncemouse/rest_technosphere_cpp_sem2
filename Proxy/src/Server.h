/*
 * Server.h
 *
 *  Created on: Jan 21, 2017
 *      Author: lord
 */

#ifndef SERVER_H_
#define SERVER_H_

#include <boost/shared_ptr.hpp>
#include <string>
#include "Selector.h"
#include "ProxyListener.h"

class Server {
public:
	Server(const std::string config);
	void start();
private:
	boost::shared_ptr<Selector> selector;
	std::vector<boost::shared_ptr<ProxyListener>> listeners;
};

#endif /* SERVER_H_ */
