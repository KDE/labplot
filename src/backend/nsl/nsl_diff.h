/*
    File                 : nsl_diff.h
    Project              : LabPlot
    Description          : NSL numerical differentiation functions
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2016 Stefan Gerlach (stefan.gerlach@uni.kn)
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef NSL_DIFF_H
#define NSL_DIFF_H

#include <stdlib.h>

#define NSL_DIFF_DERIV_ORDER_COUNT 6
typedef enum {nsl_diff_deriv_order_first, nsl_diff_deriv_order_second, nsl_diff_deriv_order_third,
	nsl_diff_deriv_order_fourth, nsl_diff_deriv_order_fifth, nsl_diff_deriv_order_sixth} nsl_diff_deriv_order_type;
extern const char* nsl_diff_deriv_order_name[];

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
