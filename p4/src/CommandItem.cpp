/*
 * CommandItem.cpp
 *
 *  Created on: Jan 23, 2017
 *      Author: Roman Vasilyev
 */

#include "CommandItem.h"
#include "LisonShell.h"

CommandItem::CommandItem(const std::vector<std::string>& cmd_tokens) :
		status { -1 },
		n_args { 0 },
		other_side_of_pipe_fd { 0 },
		stdin_redirected { false },
		stdout_redirected { false }
{
	char** pArgs = new char*[cmd_tokens.size() + 1];
	args = boost::shared_ptr<char*>(pArgs);
    int arg_idx = 0;

    for (size_t token_idx = 0; token_idx < cmd_tokens.size(); token_idx++) {
        if (cmd_tokens[token_idx] == "<") {
            stdin_fd = open(cmd_tokens[++token_idx].c_str(), O_RDONLY);
            fcntl(stdin_fd, F_SETFD, FD_CLOEXEC);
            stdin_redirected = true;
        }
        else if (cmd_tokens[token_idx] == ">") {
            stdout_fd = open(cmd_tokens[++token_idx].c_str(), O_WRONLY | O_CREAT | O_APPEND | O_TRUNC, 0666);
            fcntl(stdout_fd, F_SETFD, FD_CLOEXEC);
            stdout_redirected = true;
        }
        else {
        	pArgs[arg_idx++] = const_cast<char*>(cmd_tokens[token_idx].c_str());
        }
    }

    pArgs[arg_idx] = nullptr;
    n_args = arg_idx + 1;
}

void CommandItem::run() {
    pid_t pid;
    if ((pid = fork()) == 0) {
        if (stdin_redirected) {
        	close(STDIN_FILENO);
            dup2(stdin_fd, STDIN_FILENO);
            close(stdin_fd);
            if (other_side_of_pipe_fd != 0) {
            	close(other_side_of_pipe_fd);
            	other_side_of_pipe_fd = 0;
            }
        }

        if (stdout_redirected) {
        	close(STDOUT_FILENO);
            dup2(stdout_fd, STDOUT_FILENO);
            close(stdout_fd);
            if (other_side_of_pipe_fd != 0) {
				close(other_side_of_pipe_fd);
				other_side_of_pipe_fd = 0;
			}
        }

        char** pArgs = args.get();
        execvp(pArgs[0], pArgs);
    }
    else {
        LisonShell::child_processes.insert(pid);

        if (stdin_redirected) {
            close(stdin_fd);
        }

        if (stdout_redirected) {
            close(stdout_fd);
        }

        pid_t pid = wait(&status);
        int code = get_exit_code();
        std::cerr << "Process " << pid << " exited: " << code << std::endl;

        LisonShell::child_processes.erase(pid);
    }
}

void CommandItem::redirect_stdin(int fd, int other_side_fd) {
    stdin_fd = fd;
    stdin_redirected = true;
    other_side_of_pipe_fd = other_side_fd;
}

void CommandItem::redirect_stdout(int fd, int other_side_fd) {
    stdout_fd = fd;
    stdout_redirected = true;
    other_side_of_pipe_fd = other_side_fd;
}

const std::string CommandItem::type() const {
	return "cmd";
}

int CommandItem::get_exit_code() const {
	return WIFEXITED(status) ? WEXITSTATUS(status) : WIFEXITED(status);
}

