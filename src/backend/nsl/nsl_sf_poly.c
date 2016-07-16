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

#include <math.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_sf_pow_int.h>
#include "nsl_sf_poly.h"

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
double complex nsl_sf_poly_bessel_y(int n, double complex x) {
	if (n == 0)
		return 1.0;
	else if (n == 1)
		return 1.0 + x;

	return (2*n - 1)*x*nsl_sf_poly_bessel_y(n - 1, x) + nsl_sf_poly_bessel_y(n - 2, x);
}

/*
 * https://en.wikipedia.org/wiki/Bessel_polynomials
 * using recursion
*/
double complex nsl_sf_poly_reversed_bessel_theta(int n, double complex x) {
	if (n == 0)
		return 1.0;
	else if (n == 1)
		return 1.0 + x;

	return (2*n - 1)*nsl_sf_poly_reversed_bessel_theta(n - 1, x) + x*x*nsl_sf_poly_reversed_bessel_theta(n - 2, x);
}
