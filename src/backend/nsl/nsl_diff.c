/*
    File                 : nsl_diff.c
    Project              : LabPlot
    Description          : NSL numerical differentiation functions
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2016 Stefan Gerlach <stefan.gerlach@uni.kn>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

/* TODO:
	* add more orders
*/

#include "nsl_diff.h"
#include "nsl_common.h"
#include "nsl_sf_poly.h"

const char* nsl_diff_deriv_order_name[] = {i18n("first"), i18n("second"), i18n("third"), i18n("fourth"), i18n("fifth"), i18n("sixth")};

double nsl_diff_first_central(double xm, double fm, double xp, double fp) {
	return (fp - fm)/(xp - xm);
}

int nsl_diff_deriv_first_equal(const double *x, double *y, const size_t n) {
	if (n < 3)
		return -1;

	double dy=0, oldy=0, oldy2=0;
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
			y[i-2] = oldy2;
		if (i > 0 && i < n-1)
			oldy2 = oldy;

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
}

int nsl_diff_first_deriv_second_order(const double *x, double *y, const size_t n) {
	if (n < 3)
		return -1;

	double dy=0, oldy=0, oldy2=0, xdata[3], ydata[3];
	size_t i, j;
	for (i=0; i < n; i++) {
		if (i == 0) {
			/* 3-point forward */
			for (j=0; j < 3; j++)
				xdata[j]=x[j], ydata[j]=y[j];
			dy = nsl_sf_poly_interp_lagrange_2_deriv(x[0], xdata, ydata);
		} else if (i < n-1) {
			/* 3-point center */
			for (j=0; j < 3; j++)
				xdata[j]=x[i-1+j], ydata[j]=y[i-1+j];
			dy = nsl_sf_poly_interp_lagrange_2_deriv(x[i], xdata, ydata);
		} else if (i == n-1) {
			/* 3-point backward */
			y[i] = nsl_sf_poly_interp_lagrange_2_deriv(x[i], xdata, ydata);
			y[i-1] = oldy;
		}

		if (i > 1)
			y[i-2] = oldy2;
		if (i > 0 && i < n-1)
			oldy2 = oldy;

		oldy = dy;
	}

	return 0;
}

int nsl_diff_first_deriv_fourth_order(const double *x, double *y, const size_t n) {
	if (n < 5)
		return -1;

	double dy[5]={0}, xdata[5], ydata[5];
	size_t i, j;
	for (i=0; i < n; i++) {
		if (i == 0)
			for (j=0; j < 5; j++)
				xdata[j]=x[j], ydata[j]=y[j];
		else if (i > 1 && i < n-2)
			for (j=0; j < 5; j++)
				xdata[j]=x[i-2+j], ydata[j]=y[i-2+j];

		/* 5-point rule */
		dy[0] = nsl_sf_poly_interp_lagrange_4_deriv(x[i], xdata, ydata);

		if (i == n-1)
			for (j=0; j < 4; j++)
				y[i-j] = dy[j];

		if (i > 3)
			y[i-4] = dy[4];
		for (j=4; j > 0; j--)
			if (i >= j-1)
				dy[j] = dy[j-1];
	}

	return 0;
}

int nsl_diff_first_deriv_avg(const double *x, double *y, const size_t n) {
	if (n < 1)
		return -1;

	size_t i;
	double dy=0, oldy=0;
	for (i=0; i<n; i++) {
		if(i == 0)
			dy = (y[1]-y[0])/(x[1]-x[0]);
		else if (i == n-1)
			y[i] = (y[i]-y[i-1])/(x[i]-x[i-1]);
		else
			dy = ( (y[i+1]-y[i])/(x[i+1]-x[i]) + (y[i]-y[i-1])/(x[i]-x[i-1]) )/2.;

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
	case 3:
		return nsl_diff_second_deriv_third_order(x, y, n);
	/*TODO: higher order */
	default:
		printf("nsl_diff_second_deriv() unsupported order %d\n", order);
		return -1;
	}
}

int nsl_diff_second_deriv_first_order(const double *x, double *y, const size_t n) {
	if (n < 3)
		return -1;

	double dy[3]={0}, xdata[3], ydata[3];
	size_t i, j;
	for (i=0; i<n; i++) {
		if (i == 0)
			for (j=0; j < 3; j++)
				xdata[j]=x[j], ydata[j]=y[j];
		else if (i > 1 && i < n-1)
			for (j=0; j < 3; j++)
				xdata[j]=x[i-1+j], ydata[j]=y[i-1+j];

		/* 3-point rule */
		dy[0] = nsl_sf_poly_interp_lagrange_2_deriv2(xdata, ydata);

		if (i == n-1) {
			y[i] = dy[0];
			y[i-1] = dy[1];
		}

		if (i > 1)
			y[i-2] = dy[2];
		if (i > 0)
			dy[2] = dy[1];

		dy[1] = dy[0];
	}

	return 0;
}

int nsl_diff_second_deriv_second_order(const double *x, double *y, const size_t n) {
	if (n < 4)
		return -1;

	double dy[4]={0}, xdata[4], ydata[4];
	size_t i, j;
	for (i=0; i<n; i++) {
		if (i == 0) {
			/* 4-point forward */
			for (j=0; j < 4; j++)
				xdata[j]=x[j], ydata[j]=y[j];
			dy[0] = nsl_sf_poly_interp_lagrange_3_deriv2(x[i], xdata, ydata);
		}
		else if (i == n-1) {
			/* 4-point backward */
			for (j=0; j < 4; j++)
				xdata[j]=x[i-3+j], ydata[j]=y[i-3+j];
			y[i] = nsl_sf_poly_interp_lagrange_3_deriv2(x[i], xdata, ydata);
			y[i-1] = dy[1];
			y[i-2] = dy[2];
		}
		else {
			/* 3-point center */
			for (j=0; j < 3; j++)
				xdata[j]=x[i-1+j], ydata[j]=y[i-1+j];
			dy[0] = nsl_sf_poly_interp_lagrange_2_deriv2(xdata, ydata);
		}

		if (i > 2)
			y[i-3] = dy[3];
		for (j=3; j > 0; j--)
			if (i >= j-1)
				dy[j] = dy[j-1];
	}

	return 0;
}

int nsl_diff_second_deriv_third_order(const double *x, double *y, const size_t n) {
	if (n < 5)
		return -1;

	double dy[5]={0}, xdata[5], ydata[5];
	size_t i, j;
	for (i=0; i < n; i++) {
		if (i == 0)
			for (j=0; j < 5; j++)
				xdata[j]=x[j], ydata[j]=y[j];
		else if (i > 2 && i < n-3)
			for (j=0; j < 5; j++)
				xdata[j]=x[i-2+j], ydata[j]=y[i-2+j];

		/* 5-point rule */
		dy[0] = nsl_sf_poly_interp_lagrange_4_deriv2(x[i], xdata, ydata);

		if (i == n-1)
			for (j=0; j < 4; j++)
				y[i-j] = dy[j];

		if (i > 3)
			y[i-4] = dy[4];
		for (j=4; j > 0; j--)
			if (i >= j-1)
				dy[j] = dy[j-1];
	}

	return 0;
}

int nsl_diff_third_deriv(const double *x, double *y, const size_t n, int order) {
	switch (order) {
	case 2:
		return nsl_diff_third_deriv_second_order(x, y, n);
	/*TODO: higher order */
	default:
		printf("nsl_diff_third_deriv() unsupported order %d\n", order);
		return -1;
	}
}

int nsl_diff_third_deriv_second_order(const double *x, double *y, const size_t n) {
	if (n < 5)
		return -1;

	double dy[5]={0}, xdata[5], ydata[5];
	size_t i, j;
	for (i=0; i < n; i++) {
		if (i == 0)
			for (j=0; j < 5; j++)
				xdata[j]=x[j], ydata[j]=y[j];
		else if (i > 2 && i < n-3)
			for (j=0; j < 5; j++)
				xdata[j]=x[i-2+j], ydata[j]=y[i-2+j];

		/* 5-point rule */
		dy[0] = nsl_sf_poly_interp_lagrange_4_deriv3(x[i], xdata, ydata);

		if (i == n-1)
			for (j=0; j < 4; j++)
				y[i-j] = dy[j];

		if (i > 3)
			y[i-4] = dy[4];
		for (j=4; j > 0; j--)
			if (i >= j-1)
				dy[j] = dy[j-1];
	}

	return 0;
}

int nsl_diff_fourth_deriv(const double *x, double *y, const size_t n, int order) {
	switch (order) {
	case 1:
		return nsl_diff_fourth_deriv_first_order(x, y, n);
	case 3:
		return nsl_diff_fourth_deriv_third_order(x, y, n);
	/*TODO: higher order */
	default:
		printf("nsl_diff_fourth_deriv() unsupported order %d\n", order);
		return -1;
	}
}

int nsl_diff_fourth_deriv_first_order(const double *x, double *y, const size_t n) {
	if (n < 5)
		return -1;

	double dy[5]={0}, xdata[5], ydata[5];
	size_t i, j;
	for (i=0; i < n; i++) {
		if (i == 0)
			for (j=0; j < 5; j++)
				xdata[j]=x[j], ydata[j]=y[j];
		else if (i > 2 && i < n-3)
			for (j=0; j < 5; j++)
				xdata[j]=x[i-2+j], ydata[j]=y[i-2+j];

		/* 5-point rule */
		dy[0] = nsl_sf_poly_interp_lagrange_4_deriv4(xdata, ydata);

		if (i == n-1)
			for (j=0; j < 4; j++)
				y[i-j] = dy[j];

		if (i > 3)
			y[i-4] = dy[4];
		for (j=4; j > 0; j--)
			if (i >= j-1)
				dy[j] = dy[j-1];
	}

	return 0;
}

int nsl_diff_fourth_deriv_third_order(const double *x, double *y, const size_t n) {
	if (n < 7)
		return -1;

	double dy[7]={0}, xdata[7], ydata[7];
	size_t i, j;
	for (i=0; i < n; i++) {
		if (i == 0)
			for (j=0; j < 7; j++)
				xdata[j]=x[j], ydata[j]=y[j];
		else if (i > 3 && i < n-4)
			for (j=0; j < 7; j++)
				xdata[j]=x[i-3+j], ydata[j]=y[i-3+j];

		/* 7-point rule */
		dy[0] = nsl_sf_poly_interp_lagrange_6_deriv4(x[i], xdata, ydata);

		if (i == n-1)
			for (j=0; j < 6; j++)
				y[i-j] = dy[j];

		if (i > 5)
			y[i-6] = dy[6];
		for (j=6; j > 0; j--)
			if (i >= j-1)
				dy[j] = dy[j-1];
	}

	return 0;
}

int nsl_diff_fifth_deriv(const double *x, double *y, const size_t n, int order) {
	switch (order) {
	case 2:
		return nsl_diff_fifth_deriv_second_order(x, y, n);
	/*TODO: higher order */
	default:
		printf("nsl_diff_fifth_deriv() unsupported order %d\n", order);
		return -1;
	}
}

int nsl_diff_fifth_deriv_second_order(const double *x, double *y, const size_t n) {
	if (n < 7)
		return -1;

	double dy[7]={0}, xdata[7], ydata[7];
	size_t i, j;
	for (i=0; i < n; i++) {
		if (i == 0)
			for (j=0; j < 7; j++)
				xdata[j]=x[j], ydata[j]=y[j];
		else if (i > 3 && i < n-4)
			for (j=0; j < 7; j++)
				xdata[j]=x[i-3+j], ydata[j]=y[i-3+j];

		/* 7-point rule */
		dy[0] = nsl_sf_poly_interp_lagrange_6_deriv5(x[i], xdata, ydata);

		if (i == n-1)
			for (j=0; j < 6; j++)
				y[i-j] = dy[j];

		if (i > 5)
			y[i-6] = dy[6];
		for (j=6; j > 0; j--)
			if (i >= j-1)
				dy[j] = dy[j-1];
	}

	return 0;
}

int nsl_diff_sixth_deriv(const double *x, double *y, const size_t n, int order) {
	switch (order) {
	case 1:
		return nsl_diff_sixth_deriv_first_order(x, y, n);
	/*TODO: higher order */
	default:
		printf("nsl_diff_sixth_deriv() unsupported order %d\n", order);
		return -1;
	}
}

int nsl_diff_sixth_deriv_first_order(const double *x, double *y, const size_t n) {
	if (n < 7)
		return -1;

	double dy[7]={0}, xdata[7], ydata[7];
	size_t i, j;
	for (i=0; i < n; i++) {
		if (i == 0)
			for (j=0; j < 7; j++)
				xdata[j]=x[j], ydata[j]=y[j];
		else if (i > 3 && i < n-4)
			for (j=0; j < 7; j++)
				xdata[j]=x[i-3+j], ydata[j]=y[i-3+j];

		/* 7-point rule */
		dy[0] = nsl_sf_poly_interp_lagrange_6_deriv6(xdata, ydata);

		if (i == n-1)
			for (j=0; j < 6; j++)
				y[i-j] = dy[j];

		if (i > 5)
			y[i-6] = dy[6];
		for (j=6; j > 0; j--)
			if (i >= j-1)
				dy[j] = dy[j-1];
	}

	return 0;
}

