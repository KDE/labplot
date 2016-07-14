/***************************************************************************
    File                 : nsl_filter.h
    Project              : LabPlot
    Description          : NSL Fourier filter functions
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

#ifndef NSL_FILTER_H
#define NSL_FILTER_H

#include <stdlib.h>

#define NSL_FILTER_TYPE_COUNT 4
typedef enum {nsl_filter_type_low_pass, nsl_filter_type_high_pass, nsl_filter_type_band_pass, 
	nsl_filter_type_band_reject} nsl_filter_type;	/*TODO: Threshold */
extern const char* nsl_filter_type_name[];
#define NSL_FILTER_FORM_COUNT 5
typedef enum {nsl_filter_form_ideal, nsl_filter_form_butterworth, nsl_filter_form_chebyshev_i, 
	nsl_filter_form_chebyshev_ii, nsl_filter_form_legendre} nsl_filter_form;	/*TODO: Gaussian, Bessel, ... */
extern const char* nsl_filter_form_name[];
/* unit for cutoff 
Frequency=0..f_max, Fraction=0..1, Index=0..N-1
*/
#define NSL_FILTER_CUTOFF_UNIT_COUNT 3
typedef enum {nsl_filter_cutoff_unit_frequency, nsl_filter_cutoff_unit_fraction,
	nsl_filter_cutoff_unit_index} nsl_filter_cutoff_unit;
extern const char* nsl_filter_cutoff_unit_name[];

int nsl_filter_apply(double data[], size_t n, nsl_filter_type type, nsl_filter_form form, int order, double cutindex, double bandwidth);
int nsl_filter_fourier(double data[], size_t n, nsl_filter_type type, nsl_filter_form form, int order, int cutindex, int bandwidth);

#endif /* NSL_FILTER_H */
