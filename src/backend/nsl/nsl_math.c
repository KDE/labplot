/*
    File                 : nsl_math.c
    Project              : LabPlot
    Description          : NSL math functions
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2018-2020 Stefan Gerlach (stefan.gerlach@uni.kn)

*/

/***************************************************************************
 *                                                                         *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/

#include "nsl_math.h"
#include <stdio.h>
#include <gsl/gsl_math.h>

bool nsl_math_approximately_equal(double a, double b) {
	return nsl_math_approximately_equal_eps(a, b, 1.e-7);
}

bool nsl_math_approximately_equal_eps(double a, double b, double epsilon) {
        return fabs(a - b) <= ( (fabs(a) < fabs(b) ? fabs(b) : fabs(a)) * epsilon);
}

bool nsl_math_essentially_equal(double a, double b) {
	return nsl_math_essentially_equal_eps(a, b, 1.e-7);
}

bool nsl_math_essentially_equal_eps(double a, double b, double epsilon) {
        return fabs(a - b) <= ( (fabs(a) > fabs(b) ? fabs(b) : fabs(a)) * epsilon);
}

bool nsl_math_definitely_greater_than(double a, double b) {
	return nsl_math_definitely_greater_than_eps(a, b, 1.e-7);
}

bool nsl_math_definitely_greater_than_eps(double a, double b, double epsilon) {
        return (a - b) > ( (fabs(a) < fabs(b) ? fabs(b) : fabs(a)) * epsilon);
}

bool nsl_math_definitely_less_than(double a, double b) {
	return nsl_math_definitely_less_than_eps(a, b, 1.e-7);
}

bool nsl_math_definitely_less_than_eps(double a, double b, double epsilon) {
        return (b - a) > ( (fabs(a) < fabs(b) ? fabs(b) : fabs(a)) * epsilon);
}



int nsl_math_decimal_places(double value) {
	return -(int)floor(log10(fabs(value)));
}

int nsl_math_rounded_decimals(double value) {
	int places = nsl_math_decimal_places(value);

// printf("places = %d, rv = %g\n", places, round(fabs(value) * pow(10, places)));
	if (round(fabs(value) * gsl_pow_int(10., places)) >= 5.)
		places--;

	return places;
}

int nsl_math_rounded_decimals_max(double value, int max) {
	return GSL_MIN_INT(max, nsl_math_rounded_decimals(value));
}

double nsl_math_round_places(double value, int n) {
	return nsl_math_places(value, n, 0);
}
double nsl_math_floor_places(double value, int n) {
	return nsl_math_places(value, n, 1);
}
double nsl_math_ceil_places(double value, int n) {
	return nsl_math_places(value, n, 2);
}
double nsl_math_trunc_places(double value, int n) {
	return nsl_math_places(value, n, 3);
}

double nsl_math_places(double value, int n, int method) {
	// no need to round
	if (value == 0. || fabs(value) > 1.e16 || fabs(value) < 1.e-16 || isnan(value) || isinf(value)) {
/*		printf("nsl_math_places(): not changed : %.19g\n", value); */
		return value;
	}

	double scale = gsl_pow_int(10., n);
	double scaled_value = value*scale;
	if (fabs(scaled_value) > 1.e16)
		return value;
	if (fabs(scaled_value) < .5)
		return 0.;

	double eps = 1.e-15;
/*
	printf("nsl_math_places(): value = %g, n = %d, DBL_EPSILON = %.19g, scale = %.19g, scaled_value = %.19g, round(scaled_value) = %.19g round(scaled_value)/scale = %.19g\n",
			value, n, DBL_EPSILON, scale, scaled_value, round(scaled_value), round(scaled_value)/scale);
*/
	switch (method) {
	case 0:
		return round(scaled_value)/scale;
		break;
	case 1:
		return floor(scaled_value + eps)/scale;
		break;
	case 2:
		return ceil(scaled_value - eps)/scale;
		break;
	case 3:
		return trunc(scaled_value)/scale;
		break;
	default:
		printf("ERROR: unknown rounding method %d\n", method);
		return value;
	}

}

double nsl_math_round_precision(double value, unsigned int p) {
/*	printf("nsl_math_round_precision(%g, %d)\n", value, p); */

	// no need to round
	if (value == 0. || p > 16 || isnan(value) || isinf(value))
		return value;

	int e = 0;
	while (fabs(value) > 10.) {
		value /= 10.;
		e++;
	}
	while (fabs(value) < 1.) {
		value *= 10.;
		e--;
	}
	double order_of_magnitude = gsl_pow_int(10., e);

	if (p <= 0)
		return order_of_magnitude;

	double scale = gsl_pow_uint(10., p);
	double scaled_value = value*scale;
/*
	printf("nsl_math_round_precision(): scale = %g, scaled_value = %g, e = %d, return: %g\n", scale, scaled_value, e, round(scaled_value)/scale * gsl_pow_int(10., e));
*/

	return round(scaled_value)/scale * order_of_magnitude;
}
