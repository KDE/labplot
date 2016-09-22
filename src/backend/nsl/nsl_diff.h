/***************************************************************************
    File                 : nsl_diff.h
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

#ifndef NSL_DIFF_H
#define NSL_DIFF_H

/************ finite differences *********/

/* first derivative, central
	remark: do we need this?
*/
double nsl_diff_first_central(double xm, double fm, double xp, double fp);

/************ first derivatives *********/

/* calculates derivative of n points of xy-data.
	for equal/unequal spaced data.
	result in y 
*/
int nsl_diff_first_deriv_equal(const double *x, double *y, const size_t n);
int nsl_diff_first_deriv(const double *x, double *y, const size_t n, int order);
int nsl_diff_first_deriv_second_order(const double *x, double *y, const size_t n);
int nsl_diff_first_deriv_fourth_order(const double *x, double *y, const size_t n);
/* using average between left and right diff (like in other programs) */
int nsl_diff_first_deriv_avg(const double *x, double *y, const size_t n);

/************ second derivatives *********/

/* calculates second derivative of n points of xy-data
	for unequal spaced data.
	result in y
*/
int nsl_diff_second_deriv(const double *x, double *y, const size_t n, int order);
int nsl_diff_second_deriv_first_order(const double *x, double *y, const size_t n);
int nsl_diff_second_deriv_second_order(const double *x, double *y, const size_t n);
int nsl_diff_second_deriv_third_order(const double *x, double *y, const size_t n);

/************ third derivatives *********/

/* calculates third derivative of n points of xy-data
	for unequal spaced data.
	result in y
*/
int nsl_diff_third_deriv(const double *x, double *y, const size_t n, int order);
int nsl_diff_third_deriv_second_order(const double *x, double *y, const size_t n);

/************ fourth derivatives *********/

/* calculates fourth derivative of n points of xy-data
	for unequal spaced data.
	result in y
*/
int nsl_diff_fourth_deriv(const double *x, double *y, const size_t n, int order);
int nsl_diff_fourth_deriv_first_order(const double *x, double *y, const size_t n);
int nsl_diff_fourth_deriv_third_order(const double *x, double *y, const size_t n);

/************ fifth derivatives *********/

/* calculates fifth derivative of n points of xy-data
	for unequal spaced data.
	result in y
*/
int nsl_diff_fifth_deriv(const double *x, double *y, const size_t n, int order);
int nsl_diff_fifth_deriv_second_order(const double *x, double *y, const size_t n);

/************ sixth derivatives *********/

/* calculates sixth derivative of n points of xy-data
	for unequal spaced data.
	result in y
*/
int nsl_diff_sixth_deriv(const double *x, double *y, const size_t n, int order);
int nsl_diff_sixth_deriv_first_order(const double *x, double *y, const size_t n);

#endif /* NSL_DIFF_H */
