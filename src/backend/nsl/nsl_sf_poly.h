/*
	File                 : nsl_sf_poly.h
	Project              : LabPlot
	Description          : NSL special polynomial functions
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2016 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef NSL_SF_POLY_H
#define NSL_SF_POLY_H

#include <gsl/gsl_complex.h>
#include <gsl/gsl_complex_math.h>
#include <stdlib.h>

/* Chebychev T_n(x) */
double nsl_sf_poly_chebyshev_T(int n, double x);
/* Chebychev U_n(x) */
double nsl_sf_poly_chebyshev_U(int n, double x);

/* Optimal "L"egendre Polynomials */
double nsl_sf_poly_optimal_legendre_L(int n, double x);

/* Bessel polynomials y_n(x) */
gsl_complex nsl_sf_poly_bessel_y(int n, gsl_complex x);
/* reversed Bessel polynomials \theta_n(x) */
gsl_complex nsl_sf_poly_reversed_bessel_theta(int n, gsl_complex x);

/* interpolating polynomial (Lagrange)
	0 - zeroth order (1-point) integral (rectangle rule)
	1 - first order (2-point), derivative, integral and absolute area (trapezoid rule)
	2 - second order (3-point), derivatives and integral (Simpson's 1/3 rule)
	3 - third order (4-point), derivatives and integral (Simpson's 3/8 rule)
	4 - fourth order (5-point) and derivatives
	6 - sixth order (7-point) and derivatives
	TODO: barycentric form (https://en.wikipedia.org/wiki/Lagrange_polynomial)
*/
double nsl_sf_poly_interp_lagrange_0_int(const double* x, double y);
double nsl_sf_poly_interp_lagrange_1(double v, const double* x, const double* y);
double nsl_sf_poly_interp_lagrange_1_deriv(const double* x, const double* y);
double nsl_sf_poly_interp_lagrange_1_int(const double* x, const double* y);
double nsl_sf_poly_interp_lagrange_1_absint(const double* x, const double* y);
double nsl_sf_poly_interp_lagrange_2(double v, const double* x, const double* y);
double nsl_sf_poly_interp_lagrange_2_deriv(double v, const double* x, const double* y);
double nsl_sf_poly_interp_lagrange_2_deriv2(const double* x, const double* y);
double nsl_sf_poly_interp_lagrange_2_int(const double* x, const double* y);
double nsl_sf_poly_interp_lagrange_3(double v, const double* x, const double* y);
double nsl_sf_poly_interp_lagrange_3_deriv(double v, const double* x, const double* y);
double nsl_sf_poly_interp_lagrange_3_deriv2(double v, const double* x, const double* y);
double nsl_sf_poly_interp_lagrange_3_deriv3(const double* x, const double* y);
double nsl_sf_poly_interp_lagrange_3_int(const double* x, const double* y);
double nsl_sf_poly_interp_lagrange_4(double v, const double* x, const double* y);
double nsl_sf_poly_interp_lagrange_4_deriv(double v, const double* x, const double* y);
double nsl_sf_poly_interp_lagrange_4_deriv2(double v, const double* x, const double* y);
double nsl_sf_poly_interp_lagrange_4_deriv3(double v, const double* x, const double* y);
double nsl_sf_poly_interp_lagrange_4_deriv4(const double* x, const double* y);
double nsl_sf_poly_interp_lagrange_6_deriv4(double v, const double* x, const double* y);
double nsl_sf_poly_interp_lagrange_6_deriv5(double v, const double* x, const double* y);
double nsl_sf_poly_interp_lagrange_6_deriv6(const double* x, const double* y);

#endif /* NSL_SF_POLY_H */
