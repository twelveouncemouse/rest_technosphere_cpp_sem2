/*
 * Selector.h
 *
 *  Created on: Jan 21, 2017
 *      Author: lord
 */

#ifndef SELECTOR_H_
#define SELECTOR_H_

#include <string>
#include <map>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>

typedef std::vector<std::pair<std::string, int>> route;

class Selector {
public:
	Selector(const std::string config_filename);
	std::pair<std::string, int> select(int port);
	std::vector<int> get_incoming_ports();
private:
	std::string config_filename;
	std::map<int, route> routing_table;
	void read_routing_table();
};

#endif /* SELECTOR_H_ */
