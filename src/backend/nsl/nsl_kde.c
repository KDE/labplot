/*
    File                 : nsl_kde.c
    Project              : LabPlot
    Description          : NSL functions for the kernel density estimation
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2021 Alexander Semke <alexander.semke@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#include <gsl/gsl_statistics.h>
#include <gsl/gsl_sort.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_math.h>

double nsl_kde_gauss_kernel(double x) {
	return exp(-(gsl_pow_2(x)/2)) / (M_SQRT2*sqrt(M_PI));
}

double nsl_kde(double* data, double x, double h, size_t n) {
	double density = 0;
	for (size_t i = 0; i < n; i++)
		density += gsl_ran_gaussian_pdf((data[i] - x)/h, 1.) / (n * h);

	return density;
}

double nsl_kde_normal_dist_bandwith(double* data, int n) {
	gsl_sort(data, 1, n);
	const double sigma = gsl_stats_sd(data, 1, n);
	const double iqr = gsl_stats_quantile_from_sorted_data(data, 1, n, 0.75) -
				gsl_stats_quantile_from_sorted_data(data, 1, n, 0.25);

	return 0.9 * GSL_MIN(sigma, iqr/1.34) * pow(n, -0.2);
}


