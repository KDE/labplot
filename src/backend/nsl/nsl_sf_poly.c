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
