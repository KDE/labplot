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
int nsl_diff_deriv_first(const double *x, double *y, const size_t n);
int nsl_diff_deriv_first_unequal(const double *x, double *y, const size_t n);

/************ second derivatives *********/

/* calculates second derivative of n points of xy-data
	for unequal spaced data.
	result in y
*/
int nsl_diff_deriv_second_unequal(const double *x, double *y, const size_t n);

#endif /* NSL_DIFF_H */
