//============================================================================
// Name        : p5-worker.cpp
// Author      : Roman Vasilyev
// Version     :
// Copyright   : Created for Technosphere Project @Mail.ru
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
//using namespace std;
#include <cstdlib>

#include "WorkerProcess.h"

int main(int argc, char** argv) {
	int worker_id = atoi(argv[1]);
	WorkerProcess worker_proc(worker_id);
	worker_proc.start();
	return 0;
}
