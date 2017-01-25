/*
 * CleaningCompany.cpp
 *
 *  Created on: Jan 24, 2017
 *      Author: lord
 */

#include "CleaningCompany.h"

sem_t CleaningCompany::timed_sem;
bool CleaningCompany::exit_trigger;

CleaningCompany::CleaningCompany() {
	delay.tv_sec = 30 + time(NULL);
	delay.tv_nsec = 0L;
}

void CleaningCompany::start() {
	std::cout << "Emrakul DB v2.0 garbage collector started, pid = " << getpid() << std::endl;
	sem_init(&timed_sem, 0, 0);
	signal(SIGINT, sig_handler);
	signal(SIGTERM, sig_handler);
	ipc_create();

	while (true) {
		sem_timedwait(&timed_sem, &delay);
		if (exit_trigger == true) {
			break;
		}
		std::cout << "Garbage collection started..." << std::endl;
		int n_collected = hash_table.get()->collect_garbage();
		std::cout << "Garbage collection finished, " << n_collected << " records thrown" << std::endl;
		delay.tv_sec = 30 + time(NULL);
	}
	ipc_close();
	std::cout << "Garbage collector exited with code 0" << std::endl;
	exit(0);
}

void CleaningCompany::sig_handler(int signum) {
	signal(signum, sig_handler);
	if (signum == SIGINT || signum == SIGTERM) {
		exit_trigger = true;
		sem_post(&timed_sem);
	}
}
