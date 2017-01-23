/*
 * CommandItem.h
 *
 *  Created on: Jan 23, 2017
 *      Author: Roman Vasilyev
 */

#ifndef COMMANDITEM_H_
#define COMMANDITEM_H_

#include "BaseItem.h"

class CommandItem : public BaseItem
{
	boost::shared_ptr<char*> args;
    int status, n_args;
    int stdin_fd, stdout_fd;
    int other_side_of_pipe_fd;
    bool stdin_redirected, stdout_redirected;

public:
    CommandItem(const std::vector<std::string>& cmd);
    void run();
    void redirect_stdin(int fd, int other_side_fd);
    void redirect_stdout(int fd, int other_side_fd);
    int get_exit_code() const;

    const std::string type() const;
};

#endif /* COMMANDITEM_H_ */
