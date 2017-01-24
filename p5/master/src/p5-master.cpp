//============================================================================
// Name        : p5-master.cpp
// Author      : Roman Vasilyev
// Version     :
// Copyright   : Created for Technosphere Project @Mail.ru
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
using namespace std;

#include "MasterProcess.h"

#define DEF_PORT 10500

int main() {
	cout << "rvncerr rocks!" << endl;
	pid_t pid = getpid();
	cout << "Master pid = " << pid << std::endl;
	MasterProcess masterProc(DEF_PORT);
	masterProc.start();
	return 0;
}
