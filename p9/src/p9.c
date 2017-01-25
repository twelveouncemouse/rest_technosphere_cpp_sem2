//============================================================================
// Name        : p9.cpp
// Author      : Roman Vasilyev
// Version     :
// Copyright   : Created for Technosphere Project @Mail.ru
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <stdio.h>
#include <omp.h>
#include <time.h>
#include <stdlib.h>

#include "qsort.h"

int main() {
	srand(time(NULL));

	int* p_data = (int*)malloc(BENCH_MAX * sizeof(int));
	for (int sz = BENCH_MIN; sz <= BENCH_MAX; sz *= 10) {
		printf("%d elements test\n", sz);
		for (int i = 0; i < sz; i++) {
			p_data[i] = rand();
		}

		double start_time = omp_get_wtime();
		std_qsort(p_data, sz);
		double end_time = omp_get_wtime();
		printf("Serial QSort: %lf s\n", end_time - start_time);

		for (int i = 0; i < sz; i++) {
			p_data[i] = rand();
		}
		start_time = omp_get_wtime();
		omp_qsort(p_data, sz);
		end_time = omp_get_wtime();
		printf("Parallel QSort: %lf s\n\n", end_time - start_time);
	}
	free(p_data);
	return 0;
}
