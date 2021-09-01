/*
    File                 : nsl_sort.c
    Project              : LabPlot
    Description          : NSL sorting functions
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2016 Stefan Gerlach (stefan.gerlach@uni.kn)
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#include "nsl_sort.h"
#include <stdlib.h>

int nsl_sort_compare_size_t(const void* a, const void* b) {
	size_t _a = * ( (size_t*) a );
	size_t _b = * ( (size_t*) b );

	/* an easy expression for comparing */
	return (_a > _b) - (_a < _b);
}

void nsl_sort_size_t(size_t array[], const size_t n) {
	qsort(array, n, sizeof(size_t), nsl_sort_compare_size_t);
}

