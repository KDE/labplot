/*
    File                 : nsl_interp.h
    Project              : LabPlot
    Description          : NSL interpolation functions
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2016 Stefan Gerlach (stefan.gerlach@uni.kn)
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef NSL_INTERP_H
#define NSL_INTERP_H

#define NSL_INTERP_TYPE_COUNT 11
typedef enum {nsl_interp_type_linear, nsl_interp_type_polynomial, nsl_interp_type_cspline, nsl_interp_type_cspline_periodic, 
	nsl_interp_type_akima, nsl_interp_type_akima_periodic, nsl_interp_type_steffen, nsl_interp_type_cosine,
	nsl_interp_type_exponential, nsl_interp_type_pch, nsl_interp_type_rational} nsl_interp_type;
extern const char* nsl_interp_type_name[];

#define NSL_INTERP_PCH_VARIANT_COUNT 4
typedef enum {nsl_interp_pch_variant_finite_difference, nsl_interp_pch_variant_catmull_rom, nsl_interp_pch_variant_cardinal,
	nsl_interp_pch_variant_kochanek_bartels} nsl_interp_pch_variant;
extern const char* nsl_interp_pch_variant_name[];

#define NSL_INTERP_EVALUATE_COUNT 4
typedef enum {nsl_interp_evaluate_function, nsl_interp_evaluate_derivative, nsl_interp_evaluate_second_derivative, 
	nsl_interp_evaluate_integral} nsl_interp_evaluate;
extern const char* nsl_interp_evaluate_name[];

/* calculates rational interpolation of n points of xy-data at xn using Burlisch-Stoer method. result in v (error dv) */
int nsl_interp_ratint(double *x, double *y, int n, double xn, double *v, double *dv);

#endif /* NSL_INTERP_H */
