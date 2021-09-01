/*
    File                 : nsl_corr.h
    Project              : LabPlot
    Description          : NSL discrete correlation functions
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2018 Stefan Gerlach (stefan.gerlach@uni.kn)
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef NSL_CORR_H
#define NSL_CORR_H

#include <stdlib.h>

#define NSL_CORR_TYPE_COUNT 2
/* linear (zero-padded), circular */
typedef enum {nsl_corr_type_linear, nsl_corr_type_circular} nsl_corr_type_type;
extern const char* nsl_corr_type_name[];

#define NSL_CORR_NORM_COUNT 4
/* how to normalize (see octave)
 * biased - 1/N
 * unbiased - 1/(N-|m|)
 * coeff - 1/sqrt(R_xx(0) R_yy(0)) = 1/sqrt(sum x_i^2 sum y_i^2) [used in Origin]
 * */
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
