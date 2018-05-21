/***************************************************************************
    File                 : nsl_math.c
    Project              : LabPlot
    Description          : NSL math functions
    --------------------------------------------------------------------
    Copyright            : (C) 2018 by Stefan Gerlach (stefan.gerlach@uni.kn)

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
#include <gsl/gsl_math.h>

double nsl_math_round_places(double value, unsigned int n) {
	// no need to round
	if (value == 0. || fabs(value) > 1.e16 || fabs(value) < 1.e-16 || isnan(value) || isinf(value))
		return value;

	double scale = gsl_pow_uint(10., n);
	double scaled_value = value*scale;
	if (fabs(scaled_value) > 1.e16)
		return value;
	if (fabs(scaled_value) < .5)
		return 0.;

	return round(scaled_value)/scale;
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
