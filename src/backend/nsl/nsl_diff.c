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

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <float.h>
#include "nsl_diff.h"

double nsl_diff_first_central(double xm, double fm, double xp, double fp) {
	return (fp - fm)/(xp - xm);
}

int nsl_diff_deriv_first_equal(const double *x, double *y, const size_t n) {
	if (n < 3)
		return -1;

	double dy=0, oldy=0, oldoldy=0;
	size_t i;
	for (i=0; i < n; i++) {
		if (i == 0)	/* forward */
			dy = (-y[2] + 4.*y[1] - 3.*y[0])/(x[2]-x[0]);
		else if (i == n-1)	/* backward */
			y[i] = (3.*y[n-1] - 4.*y[n-2] + y[n-3])/(x[n-1]-x[n-3]);
		else
			dy = (y[i+1]-y[i-1])/(x[i+1]-x[i-1]);

		if (i > 1)
			y[i-2] = oldoldy;
		if (i > 0 && i < n-1)
			oldoldy = oldy;

		if (i == n-1)
			y[n-2] = oldy;
		oldy = dy;
	}

	return 0;
}
int nsl_diff_deriv_first(const double *x, double *y, const size_t n) {
	if (n < 3)
		return -1;

	double dy=0, oldy=0, oldoldy=0;
	size_t i;
	for (i=0; i < n; i++) {
		if (i == 0) {	/* forward */
			double h1=x[1]-x[0], h2=x[2]-x[1];
			dy = (-h1*h1*y[2] + (h1+h2)*(h1+h2)*y[1] - (h2*h2+2.*h1*h2)*y[0])/(h1*h2*(h1+h2));
		} else if (i == n-1) {	/* backward */
			double h1=x[n-2]-x[n-3], h2=x[n-1]-x[n-2];
			y[i] = ( (h1*h1+2.*h1*h2)*y[n-1] - (h1+h2)*(h1+h2)*y[n-2] + h2*h2*y[n-3])/(h1*h2*(h1+h2));
		} else {
			double h1=x[i]-x[i-1], h2=x[i+1]-x[i];
			dy = (y[i+1]*h1*h1 + (h2*h2-h1*h1)*y[i] - h2*h2*y[i-1])/(h1*h2*(h1+h2));
		}

		if (i > 1)
			y[i-2] = oldoldy;
		if (i > 0 && i < n-1)
			oldoldy = oldy;

		if (i == n-1)
			y[n-2] = oldy;
		oldy = dy;
	}

	return 0;
}

int nsl_diff_deriv_first_avg(const double *x, double *y, const size_t n) {
	if (n < 1)
		return -1;

	size_t i;
	double dy=0, oldy=0;
	for (i=0; i<n; i++) {
		if(i == 0) {
			dy = (y[1]-y[0])/(x[1]-x[0]);
		} else if (i == n-1) {
			y[i] = (y[n-1]-y[n-2])/(x[n-1]-x[n-2]);
		} else {
			dy = ( (y[i+1]-y[i])/(x[i+1]-x[i]) + (y[i]-y[i-1])/(x[i]-x[i-1]) )/2.;
		}

		if (i > 0)
			y[i-1] = oldy;

		oldy = dy;
	}

	return 0;
}

int nsl_diff_deriv_second(const double *x, double *y, const size_t n) {
	if (n < 3)
		return -1;

	/* TODO: same order for all points */
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
