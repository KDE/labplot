/*
	File                 : nsl_baseline.h
	Project              : LabPlot
	Description          : NSL baseline detection and subtraction methods
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef NSL_BASELINE_H
#define NSL_BASELINE_H

#include <cstdlib>

/* remove mimimum base line from data */
void nsl_baseline_remove_minimum(double* data, size_t n);
/* remove maximum base line from data */
void nsl_baseline_remove_maximum(double* data, size_t n);
/* remove mean base line from data */
void nsl_baseline_remove_mean(double* data, size_t n);
/* remove median base line from data */
void nsl_baseline_remove_median(double* data, size_t n);

/* remove base line through end points (first and last point). xdata and ydata must be of same size n */
int nsl_baseline_remove_endpoints(const double* xdata, double* ydata, size_t n);

/* remove linear regression. xdata and ydata must be of same size n */
int nsl_baseline_remove_linreg(double* xdata, double* ydata, size_t n);

/* ALS algorithm, see https://irfpy.irf.se/projects/ica/_modules/irfpy/ica/baseline.html */
/*  baseline correction by asymmetrically reweighted penalized least square (arPLS) */
/*  returns reached tolerance */
double nsl_baseline_remove_arpls(double* data, size_t n, double p, double lambda, int niter);
double nsl_baseline_remove_arpls_Eigen3(double* data, size_t n, double p, double lambda, int niter);
double nsl_baseline_remove_arpls_GSL(double* data, size_t n, double p, double lambda, int niter);
/* TODO: ALS - asymmetric least square, airPLS - adaptive iteratively reweighted Penalized Least Squares */

#endif
