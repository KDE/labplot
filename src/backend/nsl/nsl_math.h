/*
	File                 : nsl_math.h
	Project              : LabPlot
	Description          : NSL math functions
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2018-2024 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef NSL_MATH_H
#define NSL_MATH_H

#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
#define __BEGIN_DECLS extern "C" {
#define __END_DECLS }
#else
#define __BEGIN_DECLS /* empty */
#define __END_DECLS /* empty */
#endif
__BEGIN_DECLS

#include <stdbool.h>

#ifdef SDK
#include "labplot_export.h"
#endif

#define M_PI_180 (M_PI / 180.)
#define M_180_PI (180. / M_PI)

typedef enum round_method { Round, Floor, Ceil, Trunc } round_method;

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

/*
 * split x into fraction and (optional) exponent where x = fraction * 10^e
 */
double nsl_math_frexp10(double x, int* e);

/* returns decimal places of signed value
 * 0.1 -> 1, 0.06 -> 2, 23 -> -1, 100 -> -2
 */
int nsl_math_decimal_places(double value);

/* return decimal places of signed value rounded to one digit
 * 0.1 -> 1, 0.006 -> 2, 0.8 -> 0, 12 -> -1, 520 -> -3
 */
#ifdef SDK
int LABPLOT_EXPORT nsl_math_rounded_decimals(double value); // required by Range template class in SDK
#else
int nsl_math_rounded_decimals(double value);
#endif

/* nsl_math_rounded_decimals() but max 'max'
 */
int nsl_math_rounded_decimals_max(double value, int max);

/* round double value to n decimal places
 * 1234.556 & n = 3 -> 1234.556, 0.001234 & n = 4 -> 0.0012
 */
double nsl_math_round_places(double value, int n);
double nsl_math_floor_places(double value, int n);
double nsl_math_ceil_places(double value, int n);
double nsl_math_trunc_places(double value, int n);
double nsl_math_places(double value, int n, int method);

/* round double value to precision p in scientific notation
 * 1234.5 & p = 2 -> 1230 (1.23e3), 0.012345 & p = 2 -> 0.0123 (1.23e-2)
 * p <= 0 : order of magnitude (power of 10)
 */
#ifdef SDK
double LABPLOT_EXPORT nsl_math_round_precision(double value, int p); // required by Range template class in SDK
#else
double nsl_math_round_precision(double value, int p);
#endif
/* same as above but for any base x
 * p <= 0 : power of x
 */
#ifdef SDK
double LABPLOT_EXPORT nsl_math_round_basex(double value, int p, double base); // required by Range template class in SDK
#else
double nsl_math_round_basex(double value, int p, double base);
#endif

/* round double value 'value' to multiple of 'multiple'
 * (2.5, 2) -> 2,2,4,2 (4.5, 3) -> 6,3,6,3
 */
double nsl_math_round_multiple(double value, double multiple);
double nsl_math_floor_multiple(double value, double multiple);
double nsl_math_ceil_multiple(double value, double multiple);
double nsl_math_trunc_multiple(double value, double multiple);
double nsl_math_multiple(double value, double multiple, round_method method);

__END_DECLS

#endif /* NSL_MATH_H */
