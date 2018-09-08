/***************************************************************************
    File                 : nsl_conv.h
    Project              : LabPlot
    Description          : NSL discrete convolution functions
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

#ifndef NSL_CONV_H
#define NSL_CONV_H

#include <stdlib.h>

#define NSL_CONV_METHOD_BORDER 100

#define NSL_CONV_DIRECTION_COUNT 2
/* forward: convolution, backward: deconvolution */
typedef enum {nsl_conv_direction_forward, nsl_conv_direction_backward} nsl_conv_direction_type;
extern const char* nsl_conv_direction_name[];

#define NSL_CONV_METHOD_COUNT 3
/* auto: use direct method for small data size (NSL_CONV_METHOD_BORDER) and FFT method otherwise */
typedef enum {nsl_conv_method_auto, nsl_conv_method_direct, nsl_conv_method_fft} nsl_conv_method_type;
extern const char* nsl_conv_method_name[];

#define NSL_CONV_TYPE_COUNT 2
/* linear (zero-padded), circular */
typedef enum {nsl_conv_type_linear, nsl_conv_type_circular} nsl_conv_type_type;
extern const char* nsl_conv_type_name[];

#define NSL_CONV_WRAP_COUNT 3
/* how to wrap response */
typedef enum {nsl_conv_wrap_none, nsl_conv_wrap_max, nsl_conv_wrap_center} nsl_conv_wrap_type;
extern const char* nsl_conv_wrap_name[];

/* calculate convolution/deconvolution
 * of signal s of size n with response r of size m
 */
int nsl_conv_convolution_direction(double s[], size_t n, double r[], size_t m, nsl_conv_direction_type, nsl_conv_method_type, nsl_conv_type_type, int normalize, nsl_conv_wrap_type wrap, double out[]);

int nsl_conv_convolution(double s[], size_t n, double r[], size_t m, nsl_conv_method_type, nsl_conv_type_type, int normalize, nsl_conv_wrap_type wrap, double out[]);
/* deconvolution only supported by FFT method */
int nsl_conv_deconvolution(double s[], size_t n, double r[], size_t m, nsl_conv_type_type, int normalize, nsl_conv_wrap_type wrap, double out[]);

/* linear/circular convolution using direct method
 * s and r are untouched
 */
int nsl_conv_linear_direct(double s[], size_t n, double r[], size_t m, int normalize, nsl_conv_wrap_type wrap, double out[]);
int nsl_conv_circular_direct(double s[], size_t n, double r[], size_t m, int normalize, nsl_conv_wrap_type wrap, double out[]);
/* linear/circular convolution/deconvolution using FFT method
 * s and r are untouched
 */
int nsl_conv_fft_type(double s[], size_t n, double r[], size_t m, nsl_conv_direction_type, nsl_conv_type_type, int normalize, nsl_conv_wrap_type wrap, double out[]);
/* actual FFT method calculation using zero-padded arrays
 * s and r are overwritten
 */
#ifdef HAVE_FFTW3
int nsl_conv_fft_FFTW(double s[], double r[], size_t n, nsl_conv_direction_type, double out[]);
#endif
int nsl_conv_fft_GSL(double s[], double r[], size_t n, nsl_conv_direction_type, double out[]);

#endif /* NSL_CONV_H */
