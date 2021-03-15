/***************************************************************************
    File                 : nsl_math.c
    Project              : LabPlot
    Description          : NSL math functions
    --------------------------------------------------------------------
    Copyright            : (C) 2018-2020 by Stefan Gerlach (stefan.gerlach@uni.kn)

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
	if (value == 0. || fabs(value) > 1.e16 || fabs(value) < 1.e-16 || isnan(value) || isinf(value))
		return value;

	double scale = gsl_pow_int(10., n);
	double scaled_value = value*scale;
	if (fabs(scaled_value) > 1.e16)
		return value;
	if (fabs(scaled_value) < .5)
		return 0.;

	double eps = 1.e-15;
	/*printf("nsl_math_places(): DBL_EPSILON = %.19g, scale = %.19g, scaled_value = %.19g, ceil(scaled_value) = %.19g ceil(scaled_value)/scale = %.19g\n",
			DBL_EPSILON, scale, scaled_value - eps, ceil(scaled_value - eps), ceil(scaled_value - eps)/scale);
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

	double scale = gsl_pow_uint(10., p);
	double scaled_value = value*scale;

	return round(scaled_value)/scale * gsl_pow_int(10., e);
}
