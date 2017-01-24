/*
 * CommandMessage.h
 *
 *  Created on: Jan 24, 2017
 *      Author: lord
 */

#ifndef COMMANDMESSAGE_H_
#define COMMANDMESSAGE_H_

#include "UserCommand.h"

struct CommandMessage {
    long mtype;
    cmd_operation_t operation;
    int target_connection_id;
    int status;
    char data[RECORD_SIZE];

    char* get_key() { return data; }
    char* get_value() { return &data[KEY_SIZE + 1]; }
};


#endif /* COMMANDMESSAGE_H_ */
