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
// forward: convolution, backward: deconvolution
typedef enum {nsl_conv_direction_forward, nsl_conv_direction_backward} nsl_conv_direction_type;
extern const char* nsl_conv_direction_name[];

#define NSL_CONV_METHOD_COUNT 3
// auto: use direct method for small data size (NSL_CONV_METHOD_BORDER) and FFT method otherwise
typedef enum {nsl_conv_method_auto, nsl_conv_method_direct, nsl_conv_method_fft} nsl_conv_method_type;
extern const char* nsl_conv_method_name[];

#define NSL_CONV_TYPE_COUNT 2
// linear (zero-padded), circular
typedef enum {nsl_conv_type_linear, nsl_conv_type_circular} nsl_conv_type_type;
extern const char* nsl_conv_type_name[];

/* calculate convolution/deconvolution
 * of signal sig of size n with response res of size m
 */
int nsl_conv_convolution_direction(double sig[], size_t n, double res[], size_t m, nsl_conv_type_type, nsl_conv_method_type, nsl_conv_direction_type, int normalize, int wrap);

int nsl_conv_convolution(double sig[], size_t n, double res[], size_t m, nsl_conv_type_type, nsl_conv_method_type, int normalize, int wrap);
int nsl_conv_deconvolution(double sig[], size_t n, double res[], size_t m, nsl_conv_type_type, nsl_conv_method_type, int normalize, int wrap);

/* linear/circular convolution using direct method */
int nsl_conv_linear_direct(double sig[], size_t n, double res[], size_t m, int normalize, int wrap);
int nsl_conv_circular_direct(double sig[], size_t n, double res[], size_t m, int normalize, int wrap);
/* linear/circular deconvolution using direct method */
int nsl_conv_linear_direct_backward(double sig[], size_t n, double res[], size_t m, int normalize, int wrap);
int nsl_conv_circular_direct_backward(double sig[], size_t n, double res[], size_t m, int normalize, int wrap);
/* linear/circular convolution/deconvolution using FFT method */
/* TODO: normalize, wrap option */
int nsl_conv_linear_fft(double sig[], size_t n, double res[], size_t m, nsl_conv_direction_type direction);
int nsl_conv_circular_fft(double sig[], size_t n, double res[], size_t m, nsl_conv_direction_type direction);

#endif /* NSL_CONV_H */
