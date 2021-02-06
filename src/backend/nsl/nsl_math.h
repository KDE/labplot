/***************************************************************************
    File                 : nsl_math.h
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

#ifndef NSL_MATH_H
#define NSL_MATH_H

#include <stdbool.h>

#define M_PI_180 (M_PI/180.)
#define M_180_PI (180./M_PI)

/*
 * more intelligent comparison of doubles,
 * taken from Knuth's "The art of computer programming"
 */
bool nsl_math_approximately_equal(double a, double b);
bool nsl_math_essentially_equal(double a, double b);
bool nsl_math_definitely_greater_than(double a, double b);
bool nsl_math_definitely_less_than(double a, double b);
bool nsl_math_approximately_equal_eps(double a, double b, double epsilon);
bool nsl_math_essentially_equal_eps(double a, double b, double epsilon);
bool nsl_math_definitely_greater_than_eps(double a, double b, double epsilon);
bool nsl_math_definitely_less_than_eps(double a, double b, double epsilon);

/* returns decimal places of signed value
* 0.1 -> 1, 0.06 -> 2, 23 -> -1, 100 -> -2
*/
int nsl_math_decimal_places(double value);

/* return decimal places of signed value rounded to one digit
* 0.1 -> 1, 0.006 -> 2, 0.8 -> 0, 12 -> -1, 520 -> -3
*/
int nsl_math_rounded_decimals(double value);

/* nsl_math_rounded_decimals() but max 'max'
 */
int nsl_math_rounded_decimals_max(double value, int max);

/* round double value to n decimal places
 * 1234.556 & n = 3 -> 1234.556, 0.001234 & n = 4 -> 0.0012
 */
double nsl_math_round_places(double value, unsigned int n);
double nsl_math_floor_places(double value, unsigned int n);
double nsl_math_ceil_places(double value, unsigned int n);
double nsl_math_trunc_places(double value, unsigned int n);
double nsl_math_places(double value, unsigned int n, int method);

/* round double value to precision p
 * 1234.5 & p = 2 -> 1230 (1.23e3), 0.012345 & p = 2 -> 0.0123 (1.23e-2)
 */
double nsl_math_round_precision(double value, unsigned int p);

#endif /* NSL_MATH_H */
