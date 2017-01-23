/*
 * CmdInterperter.h
 *
 *  Created on: Jan 23, 2017
 *      Author: Roman Vasilyev
 */

#ifndef CMDINTERPERTER_H_
#define CMDINTERPERTER_H_

#include <vector>
#include <string>

#include "CommandItem.h"
#include "OperItem.h"

typedef std::vector<boost::shared_ptr<BaseItem>> CmdSequence;

class CmdInterperter {
public:
	CmdInterperter();
	std::vector<std::string>& tokenize_line(std::string& cmd_line);
	CmdSequence build_command_sequence(std::vector<std::string>& tokens);

private:
	std::vector<std::string> tokens;
	void collect_token_from_buffer(std::string& s);
	static inline bool is_operator(std::string& s) {
		return s == "||" || s == "&&" || s == "|" || s == "&";
	}
};

#endif /* CMDINTERPERTER_H_ */
