/*
    File                 : nsl_filter.h
    Project              : LabPlot
    Description          : NSL Fourier filter functions
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2016 Stefan Gerlach <stefan.gerlach@uni.kn>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef NSL_FILTER_H
#define NSL_FILTER_H

#include <stdlib.h>

#define NSL_FILTER_TYPE_COUNT 4
typedef enum {nsl_filter_type_low_pass, nsl_filter_type_high_pass, nsl_filter_type_band_pass, 
	nsl_filter_type_band_reject} nsl_filter_type;	/*TODO: Threshold */
extern const char* nsl_filter_type_name[];
#define NSL_FILTER_FORM_COUNT 6
typedef enum {nsl_filter_form_ideal, nsl_filter_form_butterworth, nsl_filter_form_chebyshev_i, 
	nsl_filter_form_chebyshev_ii, nsl_filter_form_legendre, nsl_filter_form_bessel} nsl_filter_form;
extern const char* nsl_filter_form_name[];
/* unit for cutoff 
Frequency=0..f_max, Fraction=0..1, Index=0..N-1
*/
#define NSL_FILTER_CUTOFF_UNIT_COUNT 3
typedef enum {nsl_filter_cutoff_unit_frequency, nsl_filter_cutoff_unit_fraction,
	nsl_filter_cutoff_unit_index} nsl_filter_cutoff_unit;
extern const char* nsl_filter_cutoff_unit_name[];

/* Gain G(x) for Bessel filter */
double nsl_filter_gain_bessel(int n, double x);

int nsl_filter_apply(double data[], size_t n, nsl_filter_type type, nsl_filter_form form, int order, double cutindex, double bandwidth);
int nsl_filter_fourier(double data[], size_t n, nsl_filter_type type, nsl_filter_form form, int order, int cutindex, int bandwidth);

#endif /* NSL_FILTER_H */
