/*
 * LisonShell.h
 *
 *  Created on: Jan 23, 2017
 *      Author: Roman Vasilyev
 */

#ifndef LISONSHELL_H_
#define LISONSHELL_H_

#include <iostream>
#include <iostream>
#include <vector>

#include "CmdInterperter.h"
#include "BaseItem.h"

class LisonShell {
public:
	static pid_t master_pid, shell_pid;
	static std::set<pid_t> child_processes;
    static void init();
    static void execute_line(std::string cmd_line);

private:
	LisonShell();
	~LisonShell();
	LisonShell(LisonShell const&) = delete;
	LisonShell& operator= (LisonShell const&) = delete;

	static LisonShell* shell_object;
	static bool is_initialized;
	static CmdInterperter cmd_interpreter;
	static void interrupt_handler(int signum);
	static void child_end_handler(int signum);
	static void run_sequence(CmdSequence& operands);
	static void run_sequence_in_background(CmdSequence& operands);
};

#endif /* LISONSHELL_H_ */
