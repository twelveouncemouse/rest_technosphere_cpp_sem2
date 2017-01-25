/*
 * CleaningCompany.h
 *
 *  Created on: Jan 24, 2017
 *      Author: lord
 */

#ifndef CLEANINGCOMPANY_H_
#define CLEANINGCOMPANY_H_

#include <ctime>
#include <signal.h>
#include "SynchronizedProcess.h"

class CleaningCompany : SynchronizedProcess {
public:
	CleaningCompany();
	void start();

private:
	struct timespec delay;
	static sem_t timed_sem;
	static void sig_handler(int);
	static bool exit_trigger;
};

#endif /* CLEANINGCOMPANY_H_ */
