/*
	File                 : nsl_sort.h
	Project              : LabPlot
	Description          : NSL functions for the kernel density estimation
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2021-2023 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef NSL_KDE_H
#define NSL_KDE_H

#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
#define __BEGIN_DECLS extern "C" {
#define __END_DECLS }
#else
#define __BEGIN_DECLS /* empty */
#define __END_DECLS /* empty */
#endif
__BEGIN_DECLS

#include "nsl_sf_kernel.h"
#include <stdlib.h>

#define NSL_KDE_BANDWITH_TYPE_COUNT 2
typedef enum { nsl_kde_bandwidth_silverman, nsl_kde_bandwidth_scott, nsl_kde_bandwidth_custom } nsl_kde_bandwidth_type;

/* calculates the density at point x for the sample data with the bandwidth h */
double nsl_kde(const double data[], double x, nsl_kernel_type kernel, double h, size_t n);

/*!
 * calculates the value of the bandwidth parameter for different methods based on the available statistics (count, sigma, iqr).
 * supported bandwidth types:
 *  * "Silverman's rule of thumb" - Silverman, B. W. (1986). Density Estimation. London: Chapman and Hall.
 *  * "Scott's rule of thumb" - Scott, D. W. (1992) Multivariate Density Estimation: Theory, Practice, and Visualization. New York: Wiley.
 */
double nsl_kde_bandwidth(int n, double sigma, double iqr, nsl_kde_bandwidth_type type);

/* similar to nsl_kde_bandwidth, but calculates the required values for count, sigma and iqr from the data */
double nsl_kde_bandwidth_from_data(double* data, int n, nsl_kde_bandwidth_type type);

/* calculates the bandwidth according to Silverman's rule of thumb" */
double nsl_kde_silverman_bandwidth(double* data, int n);

/* calculates the bandwidth according to Scott's rule of thumb" */
double nsl_kde_scott_bandwidth(double* data, int n);

__END_DECLS

#endif /* NSL_KDE_H */
