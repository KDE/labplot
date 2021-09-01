/*
    File                 : nsl_sf_kernel.h
    Project              : LabPlot
    Description          : NSL special kernel functions
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2016 Stefan Gerlach <stefan.gerlach@uni.kn>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef NSL_SF_KERNEL_H
#define NSL_SF_KERNEL_H

/* see https://en.wikipedia.org/wiki/Kernel_%28statistics%29 */

/* kernel on [-1:1] */
/* uniform */
double nsl_sf_kernel_uniform(double u);
/* triangular */
double nsl_sf_kernel_triangular(double u);
/* parabolic (Epanechnikov) */
double nsl_sf_kernel_parabolic(double u);
/* quartic (biweight) */
double nsl_sf_kernel_quartic(double u);
/* triweight */
double nsl_sf_kernel_triweight(double u);
/* tricube */
double nsl_sf_kernel_tricube(double u);
/* cosine */
double nsl_sf_kernel_cosine(double u);
/* semi circle */
double nsl_sf_kernel_semicircle(double u);

/* kernel on (-inf,inf) */
/* Gaussian */
double nsl_sf_kernel_gaussian(double u);
/* Cauchy */
double nsl_sf_kernel_cauchy(double u);
/* Logistic */
double nsl_sf_kernel_logistic(double u);
/* Picard */
double nsl_sf_kernel_picard(double u);
/* Sigmoid */
double nsl_sf_kernel_sigmoid(double u);
/* Silverman */
double nsl_sf_kernel_silverman(double u);

#endif /* NSL_SF_KERNEL_H */
