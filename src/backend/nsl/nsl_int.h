/*
    File                 : nsl_int.h
    Project              : LabPlot
    Description          : NSL numerical integration functions
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2016 Stefan Gerlach (stefan.gerlach@uni.kn)
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef NSL_INT_H
#define NSL_INT_H

#include <stdlib.h>

#define NSL_INT_NETHOD_COUNT 4
typedef enum {nsl_int_method_rectangle, nsl_int_method_trapezoid, nsl_int_method_simpson, nsl_int_method_simpson_3_8} nsl_int_method_type;
extern const char* nsl_int_method_name[];

/*
	numerical integration for non-uniform samples
	rectangle rule (1-point)
	trapezoid rule (2-point)
	Simpson-1/3 rule (3-point)	returns number of points, abs not supported yet
	Simpson-3/8 rule (4-point)	returns number of points, abs not supported yet
	abs - 0:return mathem. area, 1: return absolute area
*/
int nsl_int_rectangle(const double *x, double *y, const size_t n, int abs);
int nsl_int_trapezoid(const double *x, double *y, const size_t n, int abs);
size_t nsl_int_simpson(double *x, double *y, const size_t n, int abs);
size_t nsl_int_simpson_3_8(double *x, double *y, const size_t n, int abs);

#endif /* NSL_INT_H */
