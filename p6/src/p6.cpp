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

//int main() {
//	int shared_mem_fd;
//	void *shared_mem_addr;
//	sem_t *semaphores[HASHTABLE_SEGMENTS_COUNT];
//	boost::shared_ptr<PosixHashTable> hash_table;
//
//	size_t table_size = HASHTABLE_SEGMENTS_COUNT * HASHTABLE_SEGMENT_SIZE * RECORD_SIZE;
//	if ((shared_mem_fd = shm_open("emrakul_ht_memory", O_CREAT | O_RDWR, 0666)) == -1) {
//		throw std::runtime_error(strerror(errno));
//	}
//
//	if (ftruncate(shared_mem_fd, table_size) == -1) {
//		throw std::runtime_error(strerror(errno));
//	}
//
//	if ((shared_mem_addr = mmap(0, table_size, PROT_WRITE, MAP_SHARED, shared_mem_fd, 0)) == MAP_FAILED) {
//		throw std::runtime_error(strerror(errno));
//	}
//	memset(shared_mem_addr, 0, table_size * sizeof(char));
//
//	for (int i = 0; i < HASHTABLE_SEGMENTS_COUNT; i++) {
//		char temp[13];
//		sprintf(temp, "/ht_lock_%03d", i);
//		semaphores[i] = sem_open(temp, O_CREAT, 0666, 1);
//	}
//
//	//char* pFooMem = new char[table_size];
//	//memset(pFooMem, 0, table_size * sizeof(char));
//	PosixHashTable* p_table = new PosixHashTable(
//			(char*)shared_mem_addr,
//			semaphores
//	);
//	hash_table = boost::shared_ptr<PosixHashTable>(p_table);
//
//	std::cout << "Still alive" << std::endl;
//	char buf[1024];
//	int val = -1;
//	sem_getvalue(semaphores[6], &val);
//	std::cout << "Value = " << val << std::endl;
//	int ret = hash_table.get()->set("mouse", "nora", 60);
//	std::cout << "Still alive" << std::endl;
//	int ret2 = hash_table.get()->get("mouse", buf);
//	std::cout << "Still alive" << std::endl;
//	std::cout << "GC HashTable test: " << ret << " " << ret2 << " " << buf << std::endl;
//
//
//	//size_t table_size = HASHTABLE_SEGMENTS_COUNT * HASHTABLE_SEGMENT_SIZE * RECORD_SIZE;
//    munmap(shared_mem_addr, table_size);
//    close(shared_mem_fd);
//
//    for (int i = 0; i < HASHTABLE_SEGMENTS_COUNT; i++) {
//        sem_close(semaphores[i]);
//    }
//
//    shm_unlink("emrakul_ht_memory");
//
//    for (int i = 0; i < HASHTABLE_SEGMENTS_COUNT; i++) {
//        char temp[13];
//        sprintf(temp, "/ht_lock_%03d", i);
//        sem_unlink(temp);
//    }
//	return 0;
//}

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

