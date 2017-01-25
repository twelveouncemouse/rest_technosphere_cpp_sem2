/*
 * UserCommand.cpp
 *
 *  Created on: Jan 23, 2017
 *      Author: lord
 */

#include "UserCommand.h"

UserCommand::UserCommand() : operation { CMD_NONE }, ttl {0}, params_data { nullptr } {
	params_data = new char[RECORD_SIZE];
	memset(params_data, 0, RECORD_SIZE * sizeof(char));
}

UserCommand* UserCommand::read_from_buffer(struct evbuffer* buff) {
	size_t n_read_out;
	UserCommand* cmd = new UserCommand();
	char* p_cmdstr = nullptr;

	int max_tries = 1000;
	int n_try = 0;
	while ((p_cmdstr = evbuffer_readln(buff, &n_read_out, EVBUFFER_EOL_CRLF)) == nullptr) {
		if (++n_try >= max_tries) {
			return cmd;
		}
	}
	std::cout << "Command incoming: " << p_cmdstr << std::endl;
	cmd->parse(p_cmdstr);

	if (cmd->tokens.size() < 2) {
		return cmd;
	}
	std::string operation_string = cmd->tokens.front();
	std::string key_str;
	if (operation_string == "get") {
		cmd->operation = CMD_GET;
		key_str = cmd->tokens[1];
	}
	else if (operation_string == "set") {
		if (cmd->tokens.size() < 4) {
			return cmd;
		}
		cmd->operation = CMD_SET;
		key_str = cmd->tokens[2];
		std::string ttl_str = cmd->tokens[1];
		cmd->ttl = atoi(ttl_str.c_str());
		// Minimum TTL = 10s
		if (cmd->ttl < 10) {
			cmd->ttl = 10;
		}
		std::string value_str = cmd->tokens[3];
		if (value_str.size() > RECORD_SIZE - KEY_SIZE - 6) {
			value_str.resize(RECORD_SIZE - KEY_SIZE - 6);
		}
		const char* value_ptr = value_str.c_str();
		strncpy(cmd->params_data + KEY_SIZE + 1, value_ptr, value_str.size() + 1);
	}
	else if (operation_string == "del") {
		cmd->operation = CMD_DEL;
		key_str = cmd->tokens[1];
	}
	if (key_str.size() > KEY_SIZE - 1) {
		key_str.resize(KEY_SIZE - 1);
	}
	const char* key_ptr = key_str.c_str();
	strncpy(cmd->params_data, key_ptr, key_str.size() + 1);
	cmd->tokens.clear();

	if (p_cmdstr != nullptr) {
		delete[] p_cmdstr;
	}
	return cmd;
}

void UserCommand::parse(char* p_cmdstr) {
	std::string cmd_line = std::string(p_cmdstr);
	std::string std_buffer = "";
	std::string quoted_arg_buffer = "";
	bool quotes_on = false;

	for (char chr : cmd_line) {
		if (quotes_on) {
			if (chr == '\'' || chr == '\"') {
				collect_token_from_buffer(quoted_arg_buffer);
				quotes_on = false;
			}
			else {
				quoted_arg_buffer += chr;
			}
		}
		else {
			if (isspace(chr)) {
				collect_token_from_buffer(std_buffer);
			}
			else if (chr == '\'' || chr == '\"') {
				collect_token_from_buffer(std_buffer);
				quotes_on = true;
			}
			else {
				std_buffer += chr;
			}
		}
	}
	collect_token_from_buffer(std_buffer);
}

void UserCommand::collect_token_from_buffer(std::string& s) {
    if (s.size() > 0) {
        tokens.push_back(s);
        s.clear();
    }
}

cmd_operation_t UserCommand::get_operation() const {
	return operation;
}

const char* UserCommand::get_key_param() const {
	return params_data;
}

char* UserCommand::get_value_param() const {
	return &params_data[KEY_SIZE + 1];
}

const int UserCommand::get_ttl() const {
	return ttl;
}

UserCommand::~UserCommand() {
	if (params_data != nullptr) {
		delete[] params_data;
		params_data = nullptr;
	}
}
