/*
    File                 : nsl_conv.h
    Project              : LabPlot
    Description          : NSL discrete convolution functions
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2018 Stefan Gerlach (stefan.gerlach@uni.kn)
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef NSL_CONV_H
#define NSL_CONV_H

#include <stdlib.h>

/* when to switch from direct to FFT method */
/* set to zero to use FFT method for any length */
#define NSL_CONV_METHOD_BORDER 100

#define NSL_CONV_DIRECTION_COUNT 2
/* forward: convolution, backward: deconvolution */
typedef enum {nsl_conv_direction_forward, nsl_conv_direction_backward} nsl_conv_direction_type;
extern const char* nsl_conv_direction_name[];

#define NSL_CONV_TYPE_COUNT 2
/* linear (zero-padded), circular */
typedef enum {nsl_conv_type_linear, nsl_conv_type_circular} nsl_conv_type_type;
extern const char* nsl_conv_type_name[];

#define NSL_CONV_METHOD_COUNT 3
/* auto: use direct method for small data size (NSL_CONV_METHOD_BORDER) and FFT method otherwise */
typedef enum {nsl_conv_method_auto, nsl_conv_method_direct, nsl_conv_method_fft} nsl_conv_method_type;
extern const char* nsl_conv_method_name[];

#define NSL_CONV_NORM_COUNT 3
/* how to normalize response */
typedef enum {nsl_conv_norm_none, nsl_conv_norm_sum, nsl_conv_norm_euclidean} nsl_conv_norm_type;
extern const char* nsl_conv_norm_name[];

#define NSL_CONV_WRAP_COUNT 3
/* how to wrap response */
typedef enum {nsl_conv_wrap_none, nsl_conv_wrap_max, nsl_conv_wrap_center} nsl_conv_wrap_type;
extern const char* nsl_conv_wrap_name[];

/* TODO: mode: full, same, valid (see NumPy, SciPy) */

#define NSL_CONV_KERNEL_COUNT 10
/* standard kernel (response)
 * option: number of points
 */
typedef enum {nsl_conv_kernel_avg, nsl_conv_kernel_smooth_triangle, nsl_conv_kernel_smooth_gaussian, nsl_conv_kernel_first_derivative, nsl_conv_kernel_smooth_first_derivative,
	nsl_conv_kernel_second_derivative, nsl_conv_kernel_third_derivative, nsl_conv_kernel_fourth_derivative, nsl_conv_kernel_gaussian, nsl_conv_kernel_lorentzian} nsl_conv_kernel_type;
extern const char* nsl_conv_kernel_name[];

/* standard kernel */
int nsl_conv_standard_kernel(double k[], size_t n, nsl_conv_kernel_type);

/* calculate convolution/deconvolution
 * of signal s of size n with response r of size m
 */
int nsl_conv_convolution_direction(double s[], size_t n, double r[], size_t m, nsl_conv_direction_type, nsl_conv_type_type, nsl_conv_method_type, nsl_conv_norm_type normalize, nsl_conv_wrap_type wrap, double out[]);

int nsl_conv_convolution(double s[], size_t n, double r[], size_t m, nsl_conv_type_type, nsl_conv_method_type, nsl_conv_norm_type normalize, nsl_conv_wrap_type wrap, double out[]);
/* deconvolution only supported by FFT method */
int nsl_conv_deconvolution(double s[], size_t n, double r[], size_t m, nsl_conv_type_type, nsl_conv_norm_type normalize, nsl_conv_wrap_type wrap, double out[]);

/* linear/circular convolution using direct method
 * s and r are untouched
 */
int nsl_conv_linear_direct(double s[], size_t n, double r[], size_t m, nsl_conv_norm_type normalize, nsl_conv_wrap_type wrap, double out[]);
int nsl_conv_circular_direct(double s[], size_t n, double r[], size_t m, nsl_conv_norm_type normalize, nsl_conv_wrap_type wrap, double out[]);
/* linear/circular convolution/deconvolution using FFT method
 * s and r are untouched
 */
int nsl_conv_fft_type(double s[], size_t n, double r[], size_t m, nsl_conv_direction_type, nsl_conv_type_type, nsl_conv_norm_type normalize, nsl_conv_wrap_type wrap, double out[]);
/* actual FFT method calculation using zero-padded arrays
 * uses FFTW if available and GSL otherwise
 * wi is the wrap index
 * s and r are overwritten
 */
#ifdef HAVE_FFTW3
int nsl_conv_fft_FFTW(double s[], double r[], size_t n, nsl_conv_direction_type, size_t wi, double out[]);
#endif
int nsl_conv_fft_GSL(double s[], double r[], size_t n, nsl_conv_direction_type, double out[]);

#endif /* NSL_CONV_H */
