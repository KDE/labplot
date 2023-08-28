/*
	File                 : nsl_kde.c
	Project              : LabPlot
	Description          : NSL functions for the kernel density estimation
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2021-2023 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "nsl_kde.h"

#include <gsl/gsl_math.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_sort.h>
#include <gsl/gsl_statistics.h>

double nsl_kde(const double* data, double x, nsl_kernel_type kernel, double h, size_t n) {
	double density = 0;
	switch (kernel) {
	case nsl_kernel_uniform:
		for (size_t i = 0; i < n; i++)
			density += nsl_sf_kernel_uniform((data[i] - x) / h) / (n * h);
		break;
	case nsl_kernel_triangular:
		for (size_t i = 0; i < n; i++)
			density += nsl_sf_kernel_triangular((data[i] - x) / h) / (n * h);
		break;
	case nsl_kernel_binomial:
		// TODO
		break;
	case nsl_kernel_parabolic:
		for (size_t i = 0; i < n; i++)
			density += nsl_sf_kernel_parabolic((data[i] - x) / h) / (n * h);
		break;
	case nsl_kernel_quartic:
		for (size_t i = 0; i < n; i++)
			density += nsl_sf_kernel_quartic((data[i] - x) / h) / (n * h);
		break;
	case nsl_kernel_triweight:
		for (size_t i = 0; i < n; i++)
			density += nsl_sf_kernel_triweight((data[i] - x) / h) / (n * h);
		break;
	case nsl_kernel_tricube:
		for (size_t i = 0; i < n; i++)
			density += nsl_sf_kernel_tricube((data[i] - x) / h) / (n * h);
		break;
	case nsl_kernel_cosine:
		for (size_t i = 0; i < n; i++)
			density += nsl_sf_kernel_cosine((data[i] - x) / h) / (n * h);
		break;
	case nsl_kernel_gauss:
		for (size_t i = 0; i < n; i++)
			density += nsl_sf_kernel_gaussian((data[i] - x) / h) / (n * h);
	}

	return density;
}

double nsl_kde_normal_dist_bandwidth(double* data, int n) {
	gsl_sort(data, 1, n);
	const double sigma = gsl_stats_sd(data, 1, n);
	const double iqr = gsl_stats_quantile_from_sorted_data(data, 1, n, 0.75) - gsl_stats_quantile_from_sorted_data(data, 1, n, 0.25);

	return 0.9 * GSL_MIN(sigma, iqr / 1.34) * pow(n, -0.2);
}

double nsl_kde_bandwidth(double* data, int n, nsl_kde_bandwidth_type type) {
	switch (type) {
	case nsl_kde_bandwidth_gaussian:
		return nsl_kde_normal_dist_bandwidth(data, n);
	case nsl_kde_bandwidth_custom:
		return 1e-6;
	}
}
