/*
	File                 : nsl_baseline.c
	Project              : LabPlot
	Description          : NSL baseline detection and subtraction functions
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2016-2023 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "nsl_baseline.h"
#include "nsl_stats.h"

#include "gsl/gsl_sort.h"
#include "gsl/gsl_statistics_double.h"

#include <string.h> // memcpy

void nsl_baseline_remove_minimum(double* data, const size_t n) {
	const double min = nsl_stats_minimum(data, n, NULL);

	for (size_t i = 0; i < n; i++)
		data[i] -= min;
}

void nsl_baseline_remove_maximum(double* data, const size_t n) {
	const double max = nsl_stats_maximum(data, n, NULL);

	for (size_t i = 0; i < n; i++)
		data[i] -= max;
}

void nsl_baseline_remove_mean(double* data, const size_t n) {
	const double mean = gsl_stats_mean(data, 1, n);

	for (size_t i = 0; i < n; i++)
		data[i] -= mean;
}

void nsl_baseline_remove_median(double* data, const size_t n) {
	// copy data
	double* tmp_data = (double*)malloc(n * sizeof(double));
	if (!tmp_data)
		return;
	memcpy(tmp_data, data, n * sizeof(double));

	const double median = gsl_stats_median(tmp_data, 1, n); // rearranges tmp_data
	// printf("MEDIAN = %g\n", median);

	for (size_t i = 0; i < n; i++)
		data[i] -= median;

	free(tmp_data);
}
