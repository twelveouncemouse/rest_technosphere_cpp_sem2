//============================================================================
// Name        : p6.cpp
// Author      : Roman Vasilyev
// Version     :
// Copyright   : Created for Technosphere Project @Mail.ru
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <set>
#include <signal.h>

#include "ServerProcess.h"
#include "WorkerProcess.h"
#include "CleaningCompany.h"
#include "SocketTricks.h"

#define DEF_PORT 10501
#define NUM_WORKERS 4

std::set<pid_t> ServerProcess::child_procs;
int* ServerProcess::fd_channel_hub;

int main() {
    int fd_channel_hub[WORKER_PROCESSES_COUNT];
    int worker_pid = getpid();
	try {
		for (int worker_id = 0; worker_id < WORKER_PROCESSES_COUNT; worker_id++) {
			int fd_channel[2];

			socketpair(AF_LOCAL, SOCK_STREAM, 0, fd_channel);
			set_nonblock(fd_channel[0]);
			set_nonblock(fd_channel[1]);

			if ((worker_pid = fork()) == 0) {
				close(fd_channel[0]);

				WorkerProcess worker_proc = WorkerProcess(fd_channel[1]);
				worker_proc.start();
				break;
			}
			else {
				close(fd_channel[1]);
				fd_channel_hub[worker_id] = fd_channel[0];
				ServerProcess::child_procs.insert(worker_pid);
			}
		}

		if ((worker_pid = fork()) > 0) {
			ServerProcess::child_procs.insert(worker_pid);

			ServerProcess server_proc = ServerProcess(DEF_PORT);
			ServerProcess::fd_channel_hub = fd_channel_hub;
			server_proc.start();
		}
		else {
			CleaningCompany cleaning_proc = CleaningCompany();
			cleaning_proc.start();
		}
	}
    catch (std::runtime_error& exc) {
        std::cout << "Error occured: " << exc.what() << std::endl;
        std::cout << "Sending SIGTERM to master process..." << std::endl;
		for (pid_t pid : ServerProcess::child_procs) {
			kill(pid, SIGTERM);
		}
		while (wait(NULL) > 0);

		SynchronizedProcess::ipc_delete();
		exit(EXIT_FAILURE);
    }
    return 0;
}

