/***************************************************************************
    File                 : nsl_diff.c
    Project              : LabPlot
    Description          : NSL numerical differentiation functions
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
#include <stdlib.h>
#include <float.h>
#include "nsl_diff.h"

double nsl_diff_first_central(double xm, double fm, double xp, double fp) {
	return (fp - fm)/(xp - xm);
}

int nsl_diff_deriv_first(double *x, double *y, size_t n) {
	if (n < 2)
		return -1;

	double dy=0, oldy=0;
	size_t i;
	for (i=0; i < n; i++) {
		if (i == 0)
			dy = (y[1]-y[0])/(x[1]-x[0]);
		else if (i == n-1)
			y[i] = (y[i]-y[i-1])/(x[i]-x[i-1]);
		else
			dy = (y[i+1]-y[i-1])/(x[i+1]-x[i-1]);

		if (i != 0)
			y[i-1] = oldy;
		oldy = dy;
	}

	return 0;
}
int nsl_diff_deriv_first_unequal(double *x, double *y, size_t n) {
	/*TODO: use general version */
	return nsl_diff_deriv_first(x, y, n);
}

int nsl_diff_deriv_second_unequal(double *x, double *y, size_t n) {
	if (n < 3)
		return -1;

	/* TODO: check formula for border */
	double dx1, dx2, dy=0., oldy=0., oldoldy=0.;
	size_t i;
	for (i=0; i<n; i++) {
		/* see http://websrv.cs.umt.edu/isis/index.php/Finite_differencing:_Introduction */
		if (i == 0) {
			dx1 = x[1]-x[0];
			dx2 = x[2]-x[1];
			dy = 2.*(dx1*y[2]-(dx1+dx2)*y[1]+dx2*y[0])/(dx1*dx2*(dx1+dx2));
		}
		else if (i == n-1) {
			dx1 = x[i-1]-x[i-2];
			dx2 = x[i]-x[i-1];
			y[i] = 2.*(dx1*y[i]-(dx1+dx2)*y[i-1]+dx2*y[i-2])/(dx1*dx2*(dx1+dx2));
			y[i-2] = oldoldy;
		}
		else {
			dx1 = x[i]-x[i-1];
			dx2 = x[i+1]-x[i];
			dy = (dx1*y[i+1]-(dx1+dx2)*y[i]+dx2*y[i-1])/(dx1*dx2*(dx1+dx2));
		}

		/* set value (attention if i == n-2) */
		if (i != 0 && i != n-2)
			y[i-1] = oldy;
		if (i == n-2)
			oldoldy = oldy;

		oldy=dy;
	}
	
	return 0;
}
