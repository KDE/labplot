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

#include <stdio.h>
#include <math.h>
#include "nsl_sf_poly.h"

double nsl_sf_poly_chebyshev_T(int n, double x) {
	if(x<0) {
		printf("range error in nsl_sf_poly_chebyshev_T: x = %g\n",x);
		return 0;
	}

	if(x < 1.0)
		return cos(n*acos(x));
	else
		return cosh(n*acosh(x));
}
