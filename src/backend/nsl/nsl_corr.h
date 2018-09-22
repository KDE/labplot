/***************************************************************************
    File                 : nsl_corr.h
    Project              : LabPlot
    Description          : NSL discrete correlation functions
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

#ifndef NSL_CORR_H
#define NSL_CORR_H

#include <stdlib.h>

#define NSL_CORR_TYPE_COUNT 2
/* linear (zero-padded), circular */
typedef enum {nsl_corr_type_linear, nsl_corr_type_circular} nsl_corr_type_type;
extern const char* nsl_corr_type_name[];

#define NSL_CORR_NORM_COUNT 4
/* how to normalize */
typedef enum {nsl_corr_norm_none, nsl_corr_norm_biased, nsl_corr_norm_unbiased, nsl_corr_norm_coeff} nsl_corr_norm_type;
extern const char* nsl_corr_norm_name[];


int nsl_corr_correlation(double s[], size_t n, double r[], size_t m, nsl_corr_type_type, nsl_corr_norm_type normalize, double out[]);

/* linear/circular correlation using FFT method
 * s and r are untouched
 */
int nsl_corr_fft_type(double s[], size_t n, double r[], size_t m, nsl_corr_type_type, nsl_corr_norm_type normalize, double out[]);
/* actual FFT method calculation using zero-padded arrays
 * uses FFTW if available and GSL otherwise
 * s and r are overwritten
 */
#ifdef HAVE_FFTW3
int nsl_corr_fft_FFTW(double s[], double r[], size_t n, double out[]);
#endif
int nsl_corr_fft_GSL(double s[], double r[], size_t n, double out[]);

#endif /* NSL_CORR_H */
