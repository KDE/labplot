/***************************************************************************
    File                 : nsl_dft.h
    Project              : LabPlot
    Description          : NSL discrete Fourier transform functions
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

#ifndef NSL_DFT_H
#define NSL_DFT_H

#include <stdlib.h>
#include "nsl_sf_window.h"

/* DFT result type:
	real = x
	imag = y
	magnitude = sqrt(x^2+y^2)
	amplitude = magnitude/n
	power = (x^2+y^2)/n aka periodigram, SSA
	phase = atan(y/x)
	dB = 20.*log_10(amplitude)
	normdB = dB - max(dB)
	squaremagnitude = magnitude^2
	squareamplitude = amplitude^2 aka MSA
	raw = halfcomplex GSL output (also for FFTW)
	TODO: PSD (aka TISA)
 */
#define NSL_DFT_RESULT_TYPE_COUNT 11
typedef enum {nsl_dft_result_magnitude, nsl_dft_result_amplitude, nsl_dft_result_real, nsl_dft_result_imag, nsl_dft_result_power, 
	nsl_dft_result_phase, nsl_dft_result_dB, nsl_dft_result_normdB, nsl_dft_result_squaremagnitude, nsl_dft_result_squareamplitude, 
	nsl_dft_result_raw} nsl_dft_result_type;
extern const char* nsl_dft_result_type_name[];
/* x axis scaling */
#define NSL_DFT_XSCALE_COUNT 3
typedef enum {nsl_dft_xscale_frequency, nsl_dft_xscale_index, nsl_dft_xscale_period} nsl_dft_xscale; 
extern const char* nsl_dft_xscale_name[];

/* transform data of size n. result in data 
	calculates the two-sided DFT
*/
int nsl_dft_transform(double data[], size_t stride, size_t n, int two_sided, nsl_dft_result_type type);
/* windowed version */
int nsl_dft_transform_window(double data[], size_t stride, size_t n, int two_sided, nsl_dft_result_type type, nsl_sf_window_type window);

#endif /* NSL_DFT_H */
