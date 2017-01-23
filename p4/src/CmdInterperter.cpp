/*
 * CmdInterperter.cpp
 *
 *  Created on: Jan 23, 2017
 *      Author: Roman Vasilyev
 */

#include "CmdInterperter.h"

CmdInterperter::CmdInterperter() {

}

std::vector<std::string>& CmdInterperter::tokenize_line(std::string& cmd_line)
{
    tokens.clear();
    std::string cmd_buffer = "";
    std::string operator_buffer = "";
    std::string quoted_arg_buffer = "";

    for (char chr : cmd_line) {
        if (quoted_arg_buffer.size() > 0) {
        	quoted_arg_buffer += chr;
            if (chr == quoted_arg_buffer[0]) {
                collect_token_from_buffer(quoted_arg_buffer);
            }
        }
        else {
            if (isspace(chr)) {
                collect_token_from_buffer(cmd_buffer);
                collect_token_from_buffer(operator_buffer);
            }
            else if (chr == '<' || chr == '>' || chr == ';') {
                collect_token_from_buffer(cmd_buffer);
                collect_token_from_buffer(operator_buffer);
                tokens.push_back(std::string("") + chr);
            }
            else if (chr == '&' || chr == '|') {
                collect_token_from_buffer(cmd_buffer);
                operator_buffer += chr;
            }
            else if (chr == '\'' || chr == '\"') {
                collect_token_from_buffer(cmd_buffer);
                collect_token_from_buffer(operator_buffer);
                quoted_arg_buffer += chr;
            }
            else {
                collect_token_from_buffer(operator_buffer);
                cmd_buffer += chr;
            }
        }
    }

    collect_token_from_buffer(cmd_buffer);
    collect_token_from_buffer(operator_buffer);

    return tokens;
}

void CmdInterperter::collect_token_from_buffer(std::string& s) {
    if (s.size() > 0) {
        tokens.push_back(s);
        s.clear();
    }
}

CmdSequence CmdInterperter::build_command_sequence(std::vector<std::string>& tokens) {
	CmdSequence result_sequence;
	std::vector<std::string> current_command;

	for (auto token = tokens.begin(); token < tokens.end(); token++) {
		if (!is_operator(*token)) {
			current_command.push_back(*token);
		}
		else {
			CommandItem* pCmd = new CommandItem(current_command);
			result_sequence.push_back(boost::shared_ptr<BaseItem>(pCmd));
			current_command.clear();

			BaseItem *item = NULL;

			if (*token == "|") {
				item = new OperItem(OP_PIPELINE);
			}
			else if (*token == "||") {
				item = new OperItem(OP_OR);
			}
			else if (*token == "&") {
				item = new OperItem(OP_RUN_IN_BACKGROUND);
			}
			else if (*token == "&&") {
				item = new OperItem(OP_AND);
			}
			else if (*token == ";") {
				item = new OperItem(OP_DELIMITER);
			}

			result_sequence.push_back(boost::shared_ptr<BaseItem>(item));
		}
	}

	if (current_command.size() > 0) {
		CommandItem* pCmd = new CommandItem(current_command);
		result_sequence.push_back(boost::shared_ptr<BaseItem>(pCmd));
	}

	for (auto elem = result_sequence.begin(); elem < result_sequence.end(); elem++) {
		boost::shared_ptr<BaseItem> operand = *elem;
		BaseItem* pOperand = operand.get();
		if ((*elem)->type() == "op" &&
				dynamic_cast<OperItem *>(pOperand)->name() == OP_PIPELINE) {
			int fd[2];
			pipe(fd);

			boost::shared_ptr<BaseItem> prev_operand = *(elem-1);
			boost::shared_ptr<BaseItem> next_operand = *(elem+1);
			dynamic_cast<CommandItem *>(next_operand.get())->redirect_stdin(fd[0], fd[1]);
			dynamic_cast<CommandItem *>(prev_operand.get())->redirect_stdout(fd[1], fd[0]);
		}
	}
	return result_sequence;
}
