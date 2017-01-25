/*
 * qsort.cpp
 *
 *  Created on: Jan 25, 2017
 *      Author: Roman Vasilyev
 */
#include "qsort.h"

void std_qsort_inner(int* p_data, int left_border, int right_border) {
	if (right_border > left_border) {
		int pivot_elem = p_data[right_border];
		int less = left_border - 1;
		for (int more = left_border; more <= right_border; more++) {
			if (p_data[more] <= pivot_elem) {
				less++;
				int tmp_elem = p_data[less];
				p_data[less] = p_data[more];
				p_data[more] = tmp_elem;
			}
		}
		std_qsort_inner(p_data, left_border, less - 1);
		std_qsort_inner(p_data, less + 1, right_border);
	}
}

void std_qsort(int* p_data, int len) {
	std_qsort_inner(p_data, 0, len - 1);
}

void omp_qsort_inner(int* p_data, int left_border, int right_border) {
	if (right_border > left_border) {
		int pivot = p_data[right_border];
		int less = left_border - 1;
		for (int more = left_border; more <= right_border; more++) {
			if (p_data[more] <= pivot) {
				less++;
				int tmp = p_data[less];
				p_data[less] = p_data[more];
				p_data[more] = tmp;
			}
		}

		if (right_border - left_border > 1000) {
			#pragma omp task
			omp_qsort_inner(p_data, left_border, less - 1);
			#pragma omp task
			omp_qsort_inner(p_data, less + 1, right_border);
			#pragma omp taskwait
		}
		else {
			std_qsort_inner(p_data, left_border, less - 1);
			std_qsort_inner(p_data, less + 1, right_border);
		}
	}
}

void omp_qsort(int* p_data, int len) {
	#pragma omp parallel num_threads(2)
	{
		#pragma omp single
		{
			omp_qsort_inner(p_data, 0, len - 1);
		}
	}
}

