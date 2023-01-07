/*
	File                 : nsl_baseline.h
	Project              : LabPlot
	Description          : NSL baseline detection and subtraction methods
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2016-2023 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef NSL_BASELINE_H
#define NSL_BASELINE_H

#include <stdlib.h>

/* remove mimimum base line from data */
void nsl_baseline_remove_minimum(double* data, size_t n);
/* remove maximum base line from data */
void nsl_baseline_remove_maximum(double* data, size_t n);
/* remove mean base line from data */
void nsl_baseline_remove_mean(double* data, size_t n);
/* remove median base line from data */
void nsl_baseline_remove_median(double* data, size_t n);

/* remove base line through end points (first and last point). xdata and ydata must be of same size n */
int nsl_baseline_remove_endpoints(double* xdata, double* ydata, size_t n);

/* remove linear regression. xdata and ydata must be of same size n */
int nsl_baseline_remove_linreg(double* xdata, double* ydata, size_t n);

/* TODO: ALS */

#endif
