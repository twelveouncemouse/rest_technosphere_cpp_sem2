/*
 * LisonShell.cpp
 *
 *  Created on: Jan 23, 2017
 *      Author: Roman Vasilyev
 */

#include "LisonShell.h"

bool LisonShell::is_initialized;
int LisonShell::master_pid;
int LisonShell::shell_pid;
CmdInterperter LisonShell::cmd_interpreter;
std::set<pid_t> LisonShell::child_processes;

void LisonShell::init() {
	if (is_initialized == false) {
		signal(SIGINT, &interrupt_handler);
		signal(SIGCHLD, &child_end_handler);
		master_pid = shell_pid = getpid();
		is_initialized = true;
	}
}

void LisonShell::interrupt_handler(int signum) {
    signal(signum, &interrupt_handler);

    if (signum == SIGINT) {
		for (auto p : child_processes) {
			kill(p, SIGINT);
		}

        if (getpid() != shell_pid) {
            exit(1);
        }
    }
}

void LisonShell::child_end_handler(int signum) {
	if (signum == SIGCHLD) {
		union wait wstat;
		pid_t pid;

		while ((pid = wait3(&wstat, WNOHANG, NULL)) > 0) {
			std::cerr << "Child process " << pid << " ended with code: " <<
						wstat.w_retcode << std::endl;
		}
	}
}

void LisonShell::run_sequence(CmdSequence& operands) {
    int exit_code = -1;

    for (auto itr = operands.begin(); itr < operands.end(); itr++) {
    	BaseItem* elem = (*itr).get();
        if ((*itr)->type() == "cmd") {
            CommandItem* cmd = dynamic_cast<CommandItem *>(elem);
            cmd->run();
            exit_code = cmd->get_exit_code();
        }
        else {
            op_type next_op = dynamic_cast<OperItem *>(elem)->name();

            if ((next_op == OP_OR && exit_code == 0) ||
            	(next_op == OP_AND && exit_code != 0)) {
            	break;
            }
        }
    }
}

void LisonShell::run_sequence_in_background(CmdSequence& operands) {
    if ((master_pid = fork()) == 0) {
        master_pid = getpid();
        run_sequence(operands);
        exit(0);
    }
    else {
        std::cerr << "Child process " << master_pid << " started" << std::endl;
    }
}

void LisonShell::execute_line(std::string cmd_line) {
    std::vector<std::string> commands_parsed =
    		cmd_interpreter.tokenize_line(cmd_line);
    if (commands_parsed.size() == 0) {
        return;
    }
    CmdSequence operands =
    		cmd_interpreter.build_command_sequence(commands_parsed);
    BaseItem *back = operands.back().get();

    if (back->type() == "op" &&
    		dynamic_cast<OperItem*>(back)->name() == OP_RUN_IN_BACKGROUND) {
    	run_sequence_in_background(operands);
    }
    else {
        run_sequence(operands);
    }
}

LisonShell::LisonShell() { }

LisonShell::~LisonShell() { }
