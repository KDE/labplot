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
	if(fabs(x) <= 1)
		return cos(n*acos(x));
	else if (x > 1)
		return cosh(n*gsl_acosh(x));
	else 
		return pow(-1.,n)*cosh(n*gsl_acosh(-x));
}

double nsl_sf_poly_chebyshev_U(int n, double x) {
	double sq=sqrt(x*x-1);
	return (pow(x+sq,n+1)-pow(x-sq,n+1))/2./sq;
}

/* from http://www.crbond.com/papers/lopt.pdf */
double nsl_sf_poly_optimal_legendre(int n, double x) {
	if (n<1 || n>10)
		return 0.0;

	switch (n) {
	case 1:
		return x;
	case 2:
		return gsl_pow_2(x);
	case 3:
		return x - 3.*gsl_pow_2(x) + 3.*gsl_pow_3(x);
	case 4:
		return 3.*gsl_pow_2(x) - 8.*gsl_pow_3(x) + 6.*gsl_pow_4(x);
	case 5:
		return x - 8.*gsl_pow_2(x) + 28.*gsl_pow_3(x) - 40.*gsl_pow_4(x) + 20.*gsl_pow_5(x);
	case 6:
		return 6.*gsl_pow_2(x) - 40.*gsl_pow_3(x) + 105.*gsl_pow_4(x) - 120.*gsl_pow_5(x) + 50.*gsl_pow_6(x);
	case 7:
		return x - 15.*gsl_pow_2(x) + 105.*gsl_pow_3(x) - 355.*gsl_pow_4(x) + 615.*gsl_pow_5(x) - 525.*gsl_pow_6(x) + 175.*gsl_pow_7(x);
	case 8:
		return 10.*gsl_pow_2(x) - 120.*gsl_pow_3(x) + 615.*gsl_pow_4(x) - 1624.*gsl_pow_5(x) + 2310.*gsl_pow_6(x) - 1680.*gsl_pow_7(x) + 490.*gsl_pow_8(x);
	case 9:
		return x - 24.*gsl_pow_2(x) + 276.*gsl_pow_3(x) - 1624.*gsl_pow_4(x) + 5376.*gsl_pow_5(x) - 10416.*gsl_pow_6(x) + 11704.*gsl_pow_7(x) 
			- 7056.*gsl_pow_8(x) + 1764.*gsl_pow_9(x);
	case 10:
		return 15.*gsl_pow_2(x) - 280.*gsl_pow_3(x) + 2310.*gsl_pow_4(x) - 10416.*gsl_pow_5(x) + 27860.*gsl_pow_6(x) - 45360.*gsl_pow_7(x) 
			+ 44100.*gsl_pow_8(x) - 23520.*gsl_pow_9(x) + 5292.*gsl_pow_5(gsl_pow_2(x));
	}

	return 0.0;
}
