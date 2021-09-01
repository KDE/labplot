/*
    File                 : nsl_int.c
    Project              : LabPlot
    Description          : NSL numerical integration functions
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2016 Stefan Gerlach <stefan.gerlach@uni.kn>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

/* TODO:
	* absolute area for Simpson/Simpson-3/8 rules (needs more numerics)
*/

#include "nsl_int.h"
#include "nsl_common.h"
#include "nsl_sf_poly.h"


const char* nsl_int_method_name[] = {i18n("rectangle (1-point)"), i18n("trapezoid (2-point)"), i18n("Simpson's (3-point)"), i18n("Simpson's 3/8 (4-point)")};

int nsl_int_rectangle(const double *x, double *y, const size_t n, int abs) {
	if (n == 0)
		return -1;

	size_t i, j;
	double sum = 0, xdata[2];
	for (i = 0; i < n-1; i++) {
		for (j=0; j < 2; j++)
			xdata[j] = x[i+j];
		double s = nsl_sf_poly_interp_lagrange_0_int(xdata, y[i]);
		if (abs)
			s = fabs(s);
		y[i] = sum;
		sum += s;
	}
	y[n-1] = sum;

	return 0;
}

int nsl_int_trapezoid(const double *x, double *y, const size_t n, int abs) {
	if (n < 2)
		return -1;

	size_t i, j;
	double sum = 0, xdata[2], ydata[2];
	for (i = 0; i < n-1; i++) {
		for (j = 0; j < 2; j++)
			xdata[j] = x[i+j], ydata[j] = y[i+j];
		y[i] = sum;
		if (abs)
			sum += nsl_sf_poly_interp_lagrange_1_absint(xdata, ydata);
		else
			sum += nsl_sf_poly_interp_lagrange_1_int(xdata, ydata);
	}
	y[n-1] = sum;

	return 0;
}

size_t nsl_int_simpson(double *x, double *y, const size_t n, int abs) {
	if (n < 3)
		return 0;
	if (abs != 0) {
		printf("absolute area Simpson rule not implemented yet.\n");
		return 0;
	}

	size_t i, j, np=1;
	double sum=0, xdata[3], ydata[3];
	for (i = 0; i < n-2; i+=2) {
		for (j=0; j < 3; j++)
			xdata[j] = x[i+j], ydata[j] = y[i+j];

		sum += nsl_sf_poly_interp_lagrange_2_int(xdata, ydata);
		y[np] = sum;
		x[np++] = (x[i]+x[i+1]+x[i+2])/3.;
		/*printf("i/sum: %zu-%zu %g\n", i, i+2, sum);*/
	}

	/* handle possible last point: use trapezoid rule */
	if (i == n-2) {
		for (j=0; j < 2; j++)
			xdata[j] = x[i+j], ydata[j] = y[i+j];
		sum += nsl_sf_poly_interp_lagrange_1_int(xdata, ydata);
		y[np] = sum;
		x[np++] = x[i];
	}

	/* first point */
	y[0]=0;

	return np;
}

size_t nsl_int_simpson_3_8(double *x, double *y, const size_t n, int abs) {
	if (n < 4) {
		printf("minimum number of points is 4 (given %d).\n", (int)n);
		return 0;
	}
	if (abs != 0) {
		printf("absolute area Simpson 3/8 rule not implemented yet.\n");
		return 0;
	}

	size_t i, j, np=1;
	double sum=0, xdata[4], ydata[4];
	for (i = 0; i < n-3; i+=3) {
		for (j=0; j < 4; j++)
			xdata[j] = x[i+j], ydata[j] = y[i+j];

		sum += nsl_sf_poly_interp_lagrange_3_int(xdata, ydata);
		y[np] = sum;
		x[np++] = (x[i]+x[i+1]+x[i+2]+x[i+3])/4.;
		/*printf("i/sum: %zu-%zu %g\n", i, i+3, sum);*/
	}

	/* handle possible last point(s): use trapezoid (one point) or simpson rule (two points) */
	if (i == n-2) {
		for (j=0; j < 2; j++)
			xdata[j] = x[i+j], ydata[j] = y[i+j];
		sum += nsl_sf_poly_interp_lagrange_1_int(xdata, ydata);
		y[np] = sum;
		x[np++] = x[i];
	} else if ( i == n-3) {
		for (j=0; j < 3; j++)
			xdata[j] = x[i+j], ydata[j] = y[i+j];
		sum += nsl_sf_poly_interp_lagrange_2_int(xdata, ydata);
		y[np] = sum;
		x[np++] = (x[i]+x[i+1]+x[i+2])/3.;
	}

	/* first point */
	y[0]=0;

	return np;
}
