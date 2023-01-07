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

#include "gsl/gsl_fit.h"
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

/* do a linear interpolation using first and last point and substract that */
int nsl_baseline_remove_endpoints(double* xdata, double* ydata, const size_t n) {
	// not possible
	if (xdata[0] == xdata[n - 1])
		return -1;

	for (size_t i = 0; i < n; i++) {
		// y = y1 + (x-x1)*(y2-y1)/(x2-x1)
		const double y = ydata[0] + (xdata[i] - xdata[0]) * (ydata[n - 1] - ydata[0]) / (xdata[n - 1] - xdata[0]);
		ydata[i] -= y;
	}

	return 0;
}

/* do a linear regression and substract that */
int nsl_baseline_remove_linreg(double* xdata, double* ydata, const size_t n) {
	double c0, c1, cov00, cov01, cov11, chisq;
	gsl_fit_linear(xdata, 1, ydata, 1, n, &c0, &c1, &cov00, &cov01, &cov11, &chisq);

	for (size_t i = 0; i < n; i++) {
		double y, y_err;
		gsl_fit_linear_est(xdata[i], c0, c1, cov00, cov01, cov11, &y, &y_err);
		ydata[i] -= y;
	}

	return 0;
}
