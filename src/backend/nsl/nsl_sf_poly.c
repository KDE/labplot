/***************************************************************************
    File                 : nsl_sf_poly.c
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

#include "nsl_sf_poly.h"
#include <stdio.h>	// debugging
#include <math.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_sf_pow_int.h>

/* see https://en.wikipedia.org/wiki/Chebyshev_polynomials */
double nsl_sf_poly_chebyshev_T(int n, double x) {
	if (fabs(x) <= 1)
		return cos(n * acos(x));
	else if (x > 1)
		return cosh(n * gsl_acosh(x));
	else 
		return pow(-1., n)*cosh(n * gsl_acosh(-x));
}

double nsl_sf_poly_chebyshev_U(int n, double x) {
	double sq = sqrt(x*x - 1);
	return (pow(x + sq, n + 1) - pow(x - sq, n + 1))/2./sq;
}

/* from http://www.crbond.com/papers/lopt.pdf */
double nsl_sf_poly_optimal_legendre_L(int n, double x) {
	if (n < 1 || n > 10)
		return 0.0;

	switch (n) {
	case 1:
		return x;
	case 2:
		return x*x;
	case 3:
		return (1. + (-3. + 3.*x)*x)*x;
	case 4:
		return (3. + (-8. + 6*x)*x)*x*x;
	case 5:
		return (1. + (-8. + (28. + (-40. + 20*x)*x)*x)*x)*x;
	case 6:
		return (6. + (-40. + (105. + (-120. + 50.*x)*x)*x)*x)*x*x;
	case 7:
		return (1. + (-15. + (105. + (-355. + (615. + (-525. + 175.*x)*x)*x)*x)*x)*x)*x;
	case 8:
		return (10. + (-120. + (615. + (-1624. + (2310. + (-1680. + 490.*x)*x)*x)*x)*x)*x)*x*x;
	case 9:
		return (1. + (-24. + (276. + (-1624. + (5376. + (-10416. + (11704. + (-7056. + 1764*x)*x)*x)*x)*x)*x)*x)*x)*x;
	case 10:
		return (15. + (-280. + (2310. + (-10416. + (27860. + (-45360. + (44100. + (23520. + 5292.*x)*x)*x)*x)*x)*x)*x)*x)*x*x;
	}

	return 0.0;
}

/*
 * https://en.wikipedia.org/wiki/Bessel_polynomials
 * using recursion
*/
COMPLEX nsl_sf_poly_bessel_y(int n, COMPLEX x) {
#ifdef _MSC_VER
	if (n == 0) {
		COMPLEX z = {1.0, 0.0};
		return z;
	} else if (n == 1) {
		COMPLEX z = {(const double)(creal(x) + 1.0), (const double)cimag(x)};
		return z;
	}
	double factor = 2 * n - 1;
	COMPLEX z1 = nsl_sf_poly_bessel_y(n - 1, x);
	COMPLEX z2 = nsl_sf_poly_bessel_y(n - 2, x);
	COMPLEX z = {(const double)(factor * (creal(x)*creal(z1)-cimag(x)*cimag(z1)) + creal(z2)), (const double)(factor * (creal(x)*cimag(z1) - cimag(x)*creal(z1)) + cimag(z2))};
	return z;
#else
	if (n == 0)
		return 1.0;
	else if (n == 1)
		return 1.0 + x;

	return (2*n - 1) * x * nsl_sf_poly_bessel_y(n - 1, x) + nsl_sf_poly_bessel_y(n - 2, x);
#endif
}

/*
 * https://en.wikipedia.org/wiki/Bessel_polynomials
 * using recursion
*/
COMPLEX nsl_sf_poly_reversed_bessel_theta(int n, COMPLEX x) {
#ifdef _MSC_VER
	if (n == 0) {
		COMPLEX z = {1.0, 0.0};
		return z;
	} else if (n == 1) {
		COMPLEX z = {(const double)(creal(x) + 1.0), (const double)cimag(x)};
		return z;
	}

	COMPLEX z1 = nsl_sf_poly_reversed_bessel_theta(n - 1, x);
	COMPLEX z2 = nsl_sf_poly_reversed_bessel_theta(n - 2, x);
	double rex2 = creal(x)*creal(x) - cimag(x)*cimag(x);
	double imx2 = 2.*creal(x)*cimag(x);
	double factor = 2. * n - 1.;

	COMPLEX z = {(const double)(factor * creal(z1) + rex2*creal(z2) - imx2*cimag(z2)), (const double)(factor * cimag(z1) + rex2*cimag(z2) + imx2*creal(z2))};
	return z;
#else
	if (n == 0)
		return 1.0;
	else if (n == 1)
		return 1.0 + x;

	return (2.*n - 1.) * nsl_sf_poly_reversed_bessel_theta(n - 1, x) + x * x * nsl_sf_poly_reversed_bessel_theta(n - 2, x);
#endif
}

/***************** interpolating polynomials *************/

double nsl_sf_poly_interp_lagrange_0_int(double *x, double y) {
	/* rectangle rule */
	return (x[1]-x[0])*y;
}

double nsl_sf_poly_interp_lagrange_1(double v, double *x, double *y) {
	return (y[0]*(x[1]-v) + y[1]*(v-x[0]))/(x[1]-x[0]);
}
double nsl_sf_poly_interp_lagrange_1_deriv(double *x, double *y) {
	return (y[0]-y[1])/(x[1]-x[0]);
}
double nsl_sf_poly_interp_lagrange_1_int(double *x, double *y) {
	/* trapezoid rule (2-point) */
	return (x[1]-x[0])*(y[0]+y[1])/2.;
}
double nsl_sf_poly_interp_lagrange_1_absint(double *x, double *y) {
	double dx = x[1]-x[0];
	if (y[0]*y[1] < 0)	/* sign change */
		return dx*( (fabs(y[0])-fabs(y[1]))/(fabs(y[1]/y[0])+1) + fabs(y[1]) )/2.;
	if (y[0] < 0 && y[1] < 0)
		return dx*(fabs(y[0])+fabs(y[1]))/2.;
	else
		return dx*(y[0]+y[1])/2.;
}

double nsl_sf_poly_interp_lagrange_2(double v, double *x, double *y) {
	double h1 = x[1]-x[0], h2 = x[2]-x[1];	
	double h12 = h1+h2;

	return y[0]*(v-x[1])*(v-x[2])/(h1*h12) + y[1]*(x[0]-v)*(v-x[2])/(h1*h2) + y[2]*(x[0]-v)*(x[1]-v)/(h12*h2);
}
double nsl_sf_poly_interp_lagrange_2_deriv(double v, double *x, double *y) {
	double h1 = x[1]-x[0], h2 = x[2]-x[1];	
	double h12 = h1+h2;

	return y[0]*(2.*v-x[1]-x[2])/(h1*h12) + y[1]*(x[0]-2.*v+x[2])/(h1*h2) + y[2]*(2.*v-x[0]-x[1])/(h12*h2);
}
double nsl_sf_poly_interp_lagrange_2_deriv2(double *x, double *y) {
	double h1 = x[1]-x[0], h2 = x[2]-x[1];	
	double h12 = h1+h2;

	return 2.*( y[0]/(h1*h12) - y[1]/(h1*h2) + y[2]/(h12*h2) );
}
double nsl_sf_poly_interp_lagrange_2_int(double *x, double *y) {
	/* Simpson's 1/3 rule for non-uniform grid (3-point) */
	double h1 = x[1]-x[0], h2 = x[2]-x[1];
	double h12 = h1+h2, h1_2 = h1/h2;

	return h12/6.*( (2.-1./h1_2)*y[0] + (2.+h1_2+1./h1_2)*y[1] + (2.-h1_2)*y[2] );
}

double nsl_sf_poly_interp_lagrange_3(double v, double *x, double *y) {
	double h1 = x[1]-x[0], h2 = x[2]-x[1], h3 = x[3]-x[2];	
	double h12 = h1+h2, h23 = h2+h3, h13=h12+h3;

	return y[0]*(v-x[1])*(v-x[2])*(v-x[3])/(h1*h12*h13) + y[1]*(v-x[0])*(v-x[2])*(v-x[3])/(h1*h2*h23) 
		- y[2]*(v-x[0])*(v-x[1])*(v-x[3])/(h12*h2*h3) + y[3]*(v-x[0])*(v-x[1])*(v-x[2])/(h13*h23*h3);
}
double nsl_sf_poly_interp_lagrange_3_deriv(double v, double *x, double *y) {
	double h1 = x[1]-x[0], h2 = x[2]-x[1], h3 = x[3]-x[2], S = x[0]+x[1]+x[2]+x[3];
	double h12 = h1+h2, h23 = h2+h3, h13=h12+h3;

	return -y[0]*(3*v*v-2.*v*(S-x[0])+x[1]*x[2]+(S-x[0]-x[3])*x[3])/(h1*h12*h13)
		+ y[1]*(3*v*v-2.*v*(S-x[1])+(x[0]+x[2])*x[3])/(h1*h2*h23)
		+ y[2]*(3*v*v+x[0]*x[1]-2.*v*(S-x[2])+(x[0]+x[1])*x[3])/(h12*h2*h3)
		+ y[3]*(3*v*v+x[0]*x[1]+(x[0]+x[1])*x[2]-2.*v*(S-x[3]))/(h13*h23*h3);
}
double nsl_sf_poly_interp_lagrange_3_deriv2(double v, double *x, double *y) {
	double h1 = x[1]-x[0], h2 = x[2]-x[1], h3 = x[3]-x[2], S = x[0]+x[1]+x[2]+x[3];
	double h12 = h1+h2, h23 = h2+h3, h13=h12+h3;

	return 2.*( y[0]*(S-3.*v-x[0])/(h1*h12*h13) + y[1]*(3.*v-S+x[1])/(h1*h2*h23) 
			+ y[2]*(S-3.*v-x[2])/(h12*h2*h3) + y[3]*(3.*v-S+x[3])/(h13*h23*h3) );
}
double nsl_sf_poly_interp_lagrange_3_deriv3(double *x, double *y) {
	double h1 = x[1]-x[0], h2 = x[2]-x[1], h3 = x[3]-x[2];
	double h12 = h1+h2, h23 = h2+h3, h13=h12+h3;

	return 6.*( -y[0]/(h1*h12*h13) + y[1]/(h1*h2*h23) - y[2]/(h12*h2*h3) +y[3]/(h13*h23*h3) );
}
double nsl_sf_poly_interp_lagrange_3_int(double *x, double *y) {
	/* Simpson's 3/8 rule for non-uniform grid (4-point) */
	double h1 = x[1]-x[0], h2 = x[2]-x[1], h3 = x[3]-x[2];
	double h12 = h1+h2, h23 = h2+h3, h13 = h12+h3;

	return h13/12.*( (3.*x[0]*x[0]-4.*x[0]*(x[1]+x[2])+6.*x[1]*x[2]+2.*x[0]*x[3]-2.*(x[1]+x[2])*x[3]+x[3]*x[3])/(h1*h12)*y[0] 
			+ h13*h13*(h12-h3)/(h1*h2*h23)*y[1] + h13*h13*(h23-h1)/(h12*h2*h3)*y[2] 
			+ (x[0]*x[0]-2.*x[0]*(x[1]+x[2])+6.*x[1]*x[2]+2.*x[0]*x[3]-4.*(x[1]+x[2])*x[3]+3.*x[3]*x[3])/(h23*h3)*y[3] );
}

/* 1/2 sum_{i != j}^n (x_i x_j) for n=4 */
double sum_of_2product_combinations_4_2(double a, double b, double c, double d) {
	return a*(b+c+d) + b*(c+d) + c*d;
}
/* 1/6 sum_{i != j, j != k, i != k}^n (x_i x_j x_k) for n=4 */
double sum_of_3product_combinations_4_6(double a, double b, double c, double d) {
	return a*(b*(c+d) + c*d) + b*c*d;
}

double nsl_sf_poly_interp_lagrange_4(double v, double *x, double *y) {
	double h1 = x[1]-x[0], h2 = x[2]-x[1], h3 = x[3]-x[2], h4 = x[4]-x[3];
	double h12 = h1+h2, h23 = h2+h3, h34 = h3+h4, h13=h12+h3, h24 = h23+h4, h14 = h12+h34;

	return y[0]*(v-x[1])*(v-x[2])*(v-x[3])*(v-x[4])/(h1*h12*h13*h14)
		- y[1]*(v-x[0])*(v-x[2])*(v-x[3])*(v-x[4])/(h1*h2*h23*h24)
		+ y[2]*(v-x[0])*(v-x[1])*(v-x[3])*(v-x[4])/(h12*h2*h3*h34)
		- y[3]*(v-x[0])*(v-x[1])*(v-x[2])*(v-x[4])/(h13*h23*h3*h4)
		+ y[4]*(v-x[0])*(v-x[1])*(v-x[2])*(v-x[3])/(h14*h24*h34*h4);
}
double nsl_sf_poly_interp_lagrange_4_deriv(double v, double *x, double *y) {
	double h1 = x[1]-x[0], h2 = x[2]-x[1], h3 = x[3]-x[2], h4 = x[4]-x[3], S = x[0]+x[1]+x[2]+x[3]+x[4];
	double h12 = h1+h2, h23 = h2+h3, h34 = h3+h4, h13=h12+h3, h24 = h23+h4, h14 = h12+h34;

	return    y[0]*(4.*v*v*v - 3.*v*v*(S-x[0]) - sum_of_3product_combinations_4_6(x[1],x[2],x[3],x[4])
			+ 2.*v*sum_of_2product_combinations_4_2(x[1],x[2],x[3],x[4]) )/(h1*h12*h13*h14)
		- y[1]*(4.*v*v*v - 3.*v*v*(S-x[1]) - sum_of_3product_combinations_4_6(x[0],x[2],x[3],x[4])
			+ 2.*v*sum_of_2product_combinations_4_2(x[0],x[2],x[3],x[4]) )/(h1*h2*h23*h24)
		+ y[2]*(4.*v*v*v - 3.*v*v*(S-x[2]) - sum_of_3product_combinations_4_6(x[0],x[1],x[3],x[4])
			+ 2.*v*sum_of_2product_combinations_4_2(x[0],x[1],x[3],x[4]) )/(h12*h2*h3*h34)
		- y[3]*(4.*v*v*v - 3.*v*v*(S-x[3]) - sum_of_3product_combinations_4_6(x[0],x[1],x[2],x[4])
			+ 2.*v*sum_of_2product_combinations_4_2(x[0],x[1],x[2],x[4]) )/(h13*h23*h3*h4)
		+ y[4]*(4.*v*v*v - 3.*v*v*(S-x[4]) - sum_of_3product_combinations_4_6(x[0],x[1],x[2],x[3])
			+ 2.*v*sum_of_2product_combinations_4_2(x[0],x[1],x[2],x[3]) )/(h14*h24*h34*h4);
}
double nsl_sf_poly_interp_lagrange_4_deriv2(double v, double *x, double *y) {
	double h1 = x[1]-x[0], h2 = x[2]-x[1], h3 = x[3]-x[2], h4 = x[4]-x[3], S = x[0]+x[1]+x[2]+x[3]+x[4];
	double h12 = h1+h2, h23 = h2+h3, h34 = h3+h4, h13=h12+h3, h24 = h23+h4, h14 = h12+h34;

	return 2.*(
		  y[0]*(6.*v*v-3.*v*(S-x[0]) + sum_of_2product_combinations_4_2(x[1],x[2],x[3],x[4]))/(h1*h12*h13*h14)
		- y[1]*(6.*v*v-3.*v*(S-x[1]) + sum_of_2product_combinations_4_2(x[0],x[2],x[3],x[4]))/(h1*h2*h23*h24)
		+ y[2]*(6.*v*v-3.*v*(S-x[2]) + sum_of_2product_combinations_4_2(x[0],x[1],x[3],x[4]))/(h12*h2*h3*h34)
		- y[3]*(6.*v*v-3.*v*(S-x[3]) + sum_of_2product_combinations_4_2(x[0],x[1],x[2],x[4]))/(h13*h23*h3*h4)
		+ y[4]*(6.*v*v-3.*v*(S-x[4]) + sum_of_2product_combinations_4_2(x[0],x[1],x[2],x[3]))/(h14*h24*h34*h4) );
}
double nsl_sf_poly_interp_lagrange_4_deriv3(double v, double *x, double *y) {
	double h1 = x[1]-x[0], h2 = x[2]-x[1], h3 = x[3]-x[2], h4 = x[4]-x[3], S = x[0]+x[1]+x[2]+x[3]+x[4];
	double h12 = h1+h2, h23 = h2+h3, h34 = h3+h4, h13=h12+h3, h24 = h23+h4, h14 = h12+h34;

	return 6.*( y[0]*(4.*v-S+x[0])/(h1*h12*h13*h14) + y[1]*(S-4.*v-x[1])/(h1*h2*h23*h24) + y[2]*(4.*v-S+x[2])/(h12*h2*h3*h34)
			+ y[3]*(S-4.*v-x[3])/(h13*h23*h3*h4) + y[4]*(4.*v-S+x[4])/(h14*h24*h34*h4) );
}
double nsl_sf_poly_interp_lagrange_4_deriv4(double *x, double *y) {
	double h1 = x[1]-x[0], h2 = x[2]-x[1], h3 = x[3]-x[2], h4 = x[4]-x[3];
	double h12 = h1+h2, h23 = h2+h3, h34 = h3+h4, h13=h12+h3, h24 = h23+h4, h14 = h12+h34;

	return 24.*( y[0]/(h1*h12*h13*h14) - y[1]/(h1*h2*h23*h24) + y[2]/(h12*h2*h3*h34)- y[3]/(h13*h23*h3*h4) + y[4]/(h14*h24*h34*h4) );
}

/* 1/2 sum_{i != j}^n (x_i x_j) for n=6 */
double sum_of_product_combinations_6_2(double a, double b, double c, double d, double e, double f) {
	return a*(b+c+d+e+f) + b*(c+d+e+f) + c*(d+e+f) + d*(e+f) + e*f;
}

double nsl_sf_poly_interp_lagrange_6_deriv4(double v, double *x, double *y) {
	double h1 = x[1]-x[0], h2 = x[2]-x[1], h3 = x[3]-x[2], h4 = x[4]-x[3], h5 = x[5]-x[4], h6 = x[6]-x[5];
	double h12 = h1+h2, h23 = h2+h3, h34 = h3+h4, h45 = h4+h5, h56 = h5+h6;
	double h13 = h12+h3, h24 = h23+h4, h35 = h34+h5, h46 = h45+h6, h14 = h13+h4, h25 = h24+h5, h36 = h35+h6;
	double h15 = h14+h5, h26 = h25 + h6, h16 = h15+h6, S = x[0]+x[1]+x[2]+x[3]+x[4]+x[5]+x[6];

	return 24.*(
		  y[0]*(15.*v*v-5.*v*(S-x[0]) + sum_of_product_combinations_6_2(x[1],x[2],x[3],x[4],x[5],x[6]))/(h1*h12*h13*h14*h15*h16)
		- y[1]*(15.*v*v-5.*v*(S-x[1]) + sum_of_product_combinations_6_2(x[0],x[2],x[3],x[4],x[5],x[6]))/(h1*h2*h23*h24*h25*h26)
		+ y[2]*(15.*v*v-5.*v*(S-x[2]) + sum_of_product_combinations_6_2(x[0],x[1],x[3],x[4],x[5],x[6]))/(h12*h2*h3*h34*h35*h36)
		- y[3]*(15.*v*v-5.*v*(S-x[3]) + sum_of_product_combinations_6_2(x[0],x[1],x[2],x[4],x[5],x[6]))/(h13*h23*h3*h4*h45*h46)
		+ y[4]*(15.*v*v-5.*v*(S-x[4]) + sum_of_product_combinations_6_2(x[0],x[1],x[2],x[3],x[5],x[6]))/(h14*h24*h34*h4*h5*h56)
		- y[5]*(15.*v*v-5.*v*(S-x[5]) + sum_of_product_combinations_6_2(x[0],x[1],x[2],x[3],x[4],x[6]))/(h15*h25*h35*h45*h5*h6)
		+ y[6]*(15.*v*v-5.*v*(S-x[6]) + sum_of_product_combinations_6_2(x[0],x[1],x[2],x[3],x[4],x[5]))/(h16*h26*h36*h46*h56*h6) );
}
double nsl_sf_poly_interp_lagrange_6_deriv5(double v, double *x, double *y) {
	double h1 = x[1]-x[0], h2 = x[2]-x[1], h3 = x[3]-x[2], h4 = x[4]-x[3], h5 = x[5]-x[4], h6 = x[6]-x[5];
	double h12 = h1+h2, h23 = h2+h3, h34 = h3+h4, h45 = h4+h5, h56 = h5+h6;
	double h13 = h12+h3, h24 = h23+h4, h35 = h34+h5, h46 = h45+h6, h14 = h13+h4, h25 = h24+h5, h36 = h35+h6;
	double h15 = h14+h5, h26 = h25 + h6, h16 = h15+h6, S = x[0]+x[1]+x[2]+x[3]+x[4]+x[5]+x[6];

	return 120.*( y[0]*(6.*v-S+x[0])/(h1*h12*h13*h14*h15*h16) - y[1]*(6.*v-S+x[1])/(h1*h2*h23*h24*h25*h26)
			+ y[2]*(6.*v-S+x[2])/(h12*h2*h3*h34*h35*h36) - y[3]*(6.*v-S+x[3])/(h13*h23*h3*h4*h45*h46)
			+ y[4]*(6.*v-S+x[4])/(h14*h24*h34*h4*h5*h56) - y[5]*(6.*v-S+x[5])/(h15*h25*h35*h45*h5*h6)
			+ y[6]*(6.*v-S+x[6])/(h16*h26*h36*h46*h56*h6) );
}
double nsl_sf_poly_interp_lagrange_6_deriv6(double *x, double *y) {
	double h1 = x[1]-x[0], h2 = x[2]-x[1], h3 = x[3]-x[2], h4 = x[4]-x[3], h5 = x[5]-x[4], h6 = x[6]-x[5];
	double h12 = h1+h2, h23 = h2+h3, h34 = h3+h4, h45 = h4+h5, h56 = h5+h6;
	double h13 = h12+h3, h24 = h23+h4, h35 = h34+h5, h46 = h45+h6, h14 = h13+h4, h25 = h24+h5, h36 = h35+h6;
	double h15 = h14+h5, h26 = h25 + h6, h16 = h15+h6;

	return 720.*( y[0]/(h1*h12*h13*h14*h15*h16) - y[1]/(h1*h2*h23*h24*h25*h26)
			+ y[2]/(h12*h2*h3*h34*h35*h36) - y[3]/(h13*h23*h3*h4*h45*h46)
			+ y[4]/(h14*h24*h34*h4*h5*h56) - y[5]/(h15*h25*h35*h45*h5*h6)
			+ y[6]/(h16*h26*h36*h46*h56*h6) );
}
