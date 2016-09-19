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
		else if (i == n-1) {	/* backward */
			y[i] = (3.*y[i] - 4.*y[i-1] + y[i-2])/(x[i]-x[i-2]);
			y[i-1] = oldy;
		}
		else
			dy = (y[i+1]-y[i-1])/(x[i+1]-x[i-1]);

		if (i > 1)
			y[i-2] = oldoldy;
		if (i > 0 && i < n-1)
			oldoldy = oldy;

		oldy = dy;
	}

	return 0;
}

int nsl_diff_first_deriv(const double *x, double *y, const size_t n, int order) {
	switch (order) {
	case 2:
		return nsl_diff_first_deriv_second_order(x, y, n);
	case 4:
		return nsl_diff_first_deriv_fourth_order(x, y, n);
	/*TODO: higher order */
	default:
		printf("nsl_diff_first_deriv() unsupported order %d\n", order);
		return -1;
	}

	return 0;
}

int nsl_diff_first_deriv_second_order(const double *x, double *y, const size_t n) {
	if (n < 3)
		return -1;

	double h1, h2, dy=0, oldy=0, oldoldy=0;
	size_t i;
	for (i=0; i < n; i++) {
		if (i == 0) {	
			h1=x[1]-x[0];
			h2=x[2]-x[1];
			/* 3-point forward */
			dy = (y[1]-y[0])/h1 - (y[2]-y[1])/h2 + (y[2]-y[0])/(h1+h2);
		} else if (i == n-1) {
			h1=x[i-1]-x[i-2];
			h2=x[i]-x[i-1];
			/* 3-point backward */
			y[i] = (y[i]-y[i-1])/h2 - (y[i-1]-y[i-2])/h1 + (y[i]-y[i-2])/(h1+h2);
			y[i-1] = oldy;
		} else {
			h1=x[i]-x[i-1];
			h2=x[i+1]-x[i];
			/* 3-point center */
			dy = ( (y[i+1]-y[i])*h1*h1 + (y[i]-y[i-1])*h2*h2)/(h1*h2*(h1+h2));
		}

		if (i > 1)
			y[i-2] = oldoldy;
		if (i > 0 && i < n-1)
			oldoldy = oldy;

		oldy = dy;
	}

	return 0;
}

int nsl_diff_first_deriv_fourth_order(const double *x, double *y, const size_t n) {
	if (n < 5)
		return -1;

	/* TODO */
	double h1, h2, dy, oldy, oldy2, oldy3, oldy4;
	size_t i;
	for (i=0; i < n; i++) {
		if (i == 0) {	
			h1=x[1]-x[0];
			h2=x[2]-x[1];
			/* TODO: 5-point forward */
			dy = (y[1]-y[0])/h1 - (y[2]-y[1])/h2 + (y[2]-y[0])/(h1+h2);
		} else if (i == n-1) {
			h1=x[i-1]-x[i-2];
			h2=x[i]-x[i-1];
			/* TODO: 5-point backward */
			y[i] = (y[i]-y[i-1])/h2 - (y[i-1]-y[i-2])/h1 + (y[i]-y[i-2])/(h1+h2);
			y[i-3] = oldy3;
			y[i-2] = oldy2;
			y[i-1] = oldy;
		} else {
			h1=x[i]-x[i-1];
			h2=x[i+1]-x[i];
			/* TODO: 5-point center */
			dy = ( (y[i+1]-y[i])*h1*h1 + (y[i]-y[i-1])*h2*h2)/(h1*h2*(h1+h2));
		}

		if (i > 3)
			y[i-4] = oldy4;
		if (i > 2)
			oldy4 = oldy3;
		if (i > 1)
			oldy3 = oldy2;
		if (i > 0)
			oldy2 = oldy;

		oldy = dy;
	}

	return 0;
}

int nsl_diff_first_deriv_avg(const double *x, double *y, const size_t n) {
	if (n < 1)
		return -1;

	size_t i;
	double dy=0, oldy=0;
	for (i=0; i<n; i++) {
		if(i == 0) {
			dy = (y[1]-y[0])/(x[1]-x[0]);
		} else if (i == n-1) {
			y[i] = (y[i]-y[i-1])/(x[i]-x[i-1]);
		} else {
			dy = ( (y[i+1]-y[i])/(x[i+1]-x[i]) + (y[i]-y[i-1])/(x[i]-x[i-1]) )/2.;
		}

		if (i > 0)
			y[i-1] = oldy;

		oldy = dy;
	}

	return 0;
}

int nsl_diff_second_deriv(const double *x, double *y, const size_t n, int order) {
	switch (order) {
	case 1:
		return nsl_diff_second_deriv_first_order(x, y, n);
	case 2:
		return nsl_diff_second_deriv_second_order(x, y, n);
	/*TODO: higher order */
	default:
		printf("nsl_diff_second_deriv() unsupported order %d\n", order);
		return -1;
	}

	return 0;
}

int nsl_diff_second_deriv_first_order(const double *x, double *y, const size_t n) {
	if (n < 3)
		return -1;

	double h1, h2, h12, dy=0., oldy=0., oldy2=0.;
	size_t i;
	for (i=0; i<n; i++) {
		if (i == 0) {
			h1 = x[1]-x[0];
			h2 = x[2]-x[1];
			h12 = h1 + h2;
			/* 3-point forward: first order */
			dy = 2.*(h1*y[2] - h12*y[1] + h2*y[0])/(h1*h2*h12);
		}
		else if (i == n-1) {
			h1 = x[i-1]-x[i-2];
			h2 = x[i]-x[i-1];
			h12 = h1 + h2;
			/* 3-point backward: first order */
			y[i] = 2.*(h1*y[i] - h12*y[i-1] + h2*y[i-2])/(h1*h2*h12);
			y[i-1] = oldy;
		}
		else {
			h1 = x[i]-x[i-1];
			h2 = x[i+1]-x[i];
			h12 = h1 + h2;
			/* 3-point center: second order */
			dy = 2.*(h1*y[i+1] - h12*y[i] + h2*y[i-1])/(h1*h2*h12);
		}

		if (i > 1)
			y[i-2] = oldy2;
		if (i > 0 && i < n-1)
			oldy2 = oldy;

		oldy = dy;
	}

	return 0;
}

int nsl_diff_second_deriv_second_order(const double *x, double *y, const size_t n) {
	if (n < 4)
		return -1;

	double h1, h2, h3, h12, h23, h123, dy=0., oldy=0., oldy2=0., oldy3=0.;
	size_t i;
	for (i=0; i<n; i++) {
		if (i == 0) {
			h1 = x[1]-x[0];
			h2 = x[2]-x[1];
			h3 = x[3]-x[2];
			h12 = h1 + h2;
			h23 = h2 + h3;
			h123 = h1 + h23;
			/* 4-point forward */
			dy = 2.*( y[0]*(h1+h12+h123)/(h1*h12*h123) - y[1]*(h12+h123)/(h1*h2*h23) + y[2]*(h1+h123)/(h12*h2*h3) - y[3]*(h1+h12)/(h123*h23*h3) );
		}
		else if (i == n-1) {
			h1 = x[i-2]-x[i-3];
			h2 = x[i-1]-x[i-2];
			h3 = x[i]-x[i-1];
			h12 = h1 + h2;
			h23 = h2 + h3;
			h123 = h1 + h23;
			/* 4-point backward */
			y[i] = 2.*( y[i]*(h123+h23+h3)/(h123*h23*h3) - y[i-1]*(h123+h23)/(h12*h2*h3) + y[i-2]*(h123+h3)/(h1*h2*h23) - y[i-3]*(h23+h3)/(h1*h12*h123) );
			y[i-2] = oldy2;
			y[i-1] = oldy;
		}
		else {
			h1 = x[i]-x[i-1];
			h2 = x[i+1]-x[i];
			h12 = h1 + h2;
			/* 3-point center */
			dy = 2.*(h1*y[i+1] - h12*y[i] + h2*y[i-1])/(h1*h2*h12);
		}

		if (i > 2)
			y[i-3] = oldy3;
		if (i > 1)
			oldy3 = oldy2;
		if (i > 0)
			oldy2 = oldy;

		oldy = dy;
	}

	return 0;
}
