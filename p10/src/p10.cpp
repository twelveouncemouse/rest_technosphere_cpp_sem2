/*
 ============================================================================
 Name        : p10.cpp
 Author      : Roman Vasilyev
 Version     :
 Copyright   : Created for Technosphere Project @Mail.ru
 Description : Compute exp in MPI C++
 ============================================================================
 */
#include "mpi.h"
#include <iostream>
using namespace std;

double fact(double n) {
	if (n == 0) {
		return 1;
	}
	else {
		return n * fact(n - 1);
	}
}

int main(int argc, char *argv[]) {
	int N = 32;
	MPI_Init(&argc, &argv);

	int rank(0);
	int num_procs(0);
	MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);

	int std_portion_size = N / num_procs;
	int portion_size = std_portion_size;
	int rest = N % num_procs;
	if (rank == num_procs - 1 && rest > 0) {
		portion_size += rest;
	}
	double local_result(0.0);
	for (int i = rank * std_portion_size;
			i < rank * std_portion_size + portion_size;
			i++) {
		local_result += 1.0 / fact(i);
	}

	double global_result(0.0);
	MPI_Reduce(&local_result, &global_result, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
	if (rank == 0) {
		std::cout << "Exponent of 1.0 = " << global_result << std::endl;
	}

	MPI_Finalize();
	return 0;
}
