/*
 * qsort.h
 *
 *  Created on: Jan 25, 2017
 *      Author: Roman Vasilyev
 */

#ifndef QSORT_H_
#define QSORT_H_

#define BENCH_MIN 100
#define BENCH_MAX 100000000

void std_qsort_inner(int* p_data, int left_border, int right_border);
void std_qsort(int* p_data, int len);
void omp_qsort_inner(int* p_data, int left_border, int right_border);
void omp_qsort(int* p_data, int len);

#endif /* QSORT_H_ */
