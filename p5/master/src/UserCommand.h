/*
 * UserCommand.h
 *
 *  Created on: Jan 23, 2017
 *      Author: lord
 */

#ifndef USERCOMMAND_H_
#define USERCOMMAND_H_

#include <cstring>

#include <string>
#include <iostream>

#include <boost/shared_ptr.hpp>
#include <event2/buffer.h>

#include "HashTableBase.h"

typedef enum {
	CMD_GET,
	CMD_SET,
	CMD_DEL,
	CMD_NONE
} cmd_operation_t;

class UserCommand {
public:
	static UserCommand* read_from_buffer(struct evbuffer* buff);
	cmd_operation_t get_operation() const;
	const char* get_key_param() const;
	char* get_value_param() const;

private:
	cmd_operation_t operation;
	boost::shared_ptr<char> params_data;
	void parse(char* p_cmdstr);
	void collect_token_from_buffer(std::string& s);
	std::vector<std::string> tokens;
	UserCommand();
};

#endif /* USERCOMMAND_H_ */
