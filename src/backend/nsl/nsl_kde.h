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
typedef enum { nsl_kde_bandwidth_gaussian, nsl_kde_bandwidth_custom} nsl_kde_bandwidth_type;

/* calculates the density at point x for the sample data with the bandwith h */
double nsl_kde(const double data[], double x, nsl_kernel_type kernel, double h, size_t n);

/* calculates the "normal distribution approximation" bandwidth */
double nsl_kde_normal_dist_bandwidth(double* data, int n);

double nsl_kde_bandwidth(double* data, int n, nsl_kde_bandwidth_type type);

__END_DECLS

#endif /* NSL_KDE_H */
