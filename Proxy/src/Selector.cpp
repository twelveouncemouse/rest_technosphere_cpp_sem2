/*
 * Selector.cpp
 *
 *  Created on: Jan 21, 2017
 *      Author: lord
 */

#include "Selector.h"

Selector::Selector(const std::string config_filename):
	config_filename { config_filename } {
	read_routing_table();
	srand(time(NULL));
}

std::pair<std::string, int> Selector::select(int port) {
	route target_rt = routing_table[port];
	int sz = target_rt.size();
	int idx = rand() % sz;
	return target_rt[idx];
	//return std::pair<std::string, int>("127.0.0.1", 9000);
}

void Selector::read_routing_table() {
	std::ifstream config(config_filename);
	if (!config) {
		printf("No such config file\n");
		route rt;
		rt.push_back(std::pair<std::string, int>("127.0.0.1", 9000));
		routing_table[9995] = rt;
		return;
	}

    std::string route_record, str_port, str_ip;
    unsigned short port = 0;

    while (!config.eof())
    {
        std::getline(config, route_record);
        route_record.erase(std::remove_if(route_record.begin(), route_record.end(), isspace), route_record.end());
        if (route_record.empty()) {
        	continue;
        }

        std::istringstream streamline(route_record);

        std::getline(streamline, str_port, ',');
        port = static_cast<unsigned short>(atoi(str_port.c_str()));
        route rt = std::vector<std::pair<std::string, int>>();

        while (!streamline.eof())
        {
            std::getline(streamline, str_ip, ':');
            std::getline(streamline, str_port, ',');
            rt.push_back(std::pair<std::string, int>(str_ip, atoi(str_port.c_str())));
        }
        routing_table[port] = rt;
    }
//	route rt;
//	rt.push_back(std::pair<std::string, int>("127.0.0.1", 9000));
//	routing_table[9995] = rt;
}

std::vector<int> Selector::get_incoming_ports() {
	std::vector<int> ports;
	for (auto elem: routing_table) {
		ports.push_back(elem.first);
	}
	return ports;
}
