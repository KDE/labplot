/***************************************************************************
    File                 : nsl_sf_poly.h
    Project              : LabPlot
    Description          : NSL special polynomial functions
    --------------------------------------------------------------------
    Copyright            : (C) 2016 by Stefan Gerlach (stefan.gerlach@uni.kn)

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/

#ifndef NSL_SF_POLY_H
#define NSL_SF_POLY_H

#include <stdlib.h>
#include <gsl/gsl_complex.h>
#include <gsl/gsl_complex_math.h>

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
double nsl_sf_poly_interp_lagrange_0_int(double *x, double y);
double nsl_sf_poly_interp_lagrange_1(double v, double *x, double *y);
double nsl_sf_poly_interp_lagrange_1_deriv(double *x, double *y);
double nsl_sf_poly_interp_lagrange_1_int(double *x, double *y);
double nsl_sf_poly_interp_lagrange_1_absint(double *x, double *y);
double nsl_sf_poly_interp_lagrange_2(double v, double *x, double *y);
double nsl_sf_poly_interp_lagrange_2_deriv(double v, double *x, double *y);
double nsl_sf_poly_interp_lagrange_2_deriv2(double *x, double *y);
double nsl_sf_poly_interp_lagrange_2_int(double *x, double *y);
double nsl_sf_poly_interp_lagrange_3(double v, double *x, double *y);
double nsl_sf_poly_interp_lagrange_3_deriv(double v, double *x, double *y);
double nsl_sf_poly_interp_lagrange_3_deriv2(double v, double *x, double *y);
double nsl_sf_poly_interp_lagrange_3_deriv3(double *x, double *y);
double nsl_sf_poly_interp_lagrange_3_int(double *x, double *y);
double nsl_sf_poly_interp_lagrange_4(double v, double *x, double *y);
double nsl_sf_poly_interp_lagrange_4_deriv(double v, double *x, double *y);
double nsl_sf_poly_interp_lagrange_4_deriv2(double v, double *x, double *y);
double nsl_sf_poly_interp_lagrange_4_deriv3(double v, double *x, double *y);
double nsl_sf_poly_interp_lagrange_4_deriv4(double *x, double *y);
double nsl_sf_poly_interp_lagrange_6_deriv4(double v, double *x, double *y);
double nsl_sf_poly_interp_lagrange_6_deriv5(double v, double *x, double *y);
double nsl_sf_poly_interp_lagrange_6_deriv6(double *x, double *y);

#endif /* NSL_SF_POLY_H */
