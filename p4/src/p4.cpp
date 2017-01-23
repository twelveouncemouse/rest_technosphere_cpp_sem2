//============================================================================
// Name        : p4.cpp
// Author      : Roman Vasilyev
// Version     :
// Copyright   : Created for Technosphere Project @Mail.ru
// Description : LisonShell - homework #4, oldschool Linux IPC
//============================================================================

#include "LisonShell.h"

int main() {
	LisonShell::init();

    std::string cmd_line;
    while (std::getline(std::cin, cmd_line)) {
    	if (cmd_line == "quit") {
			break;
		}
    	LisonShell::execute_line(cmd_line);
    }
    return 0;
}
