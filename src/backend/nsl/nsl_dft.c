/***************************************************************************
    File                 : nsl_dft.c
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

#include <math.h>
#include <gsl/gsl_fft_real.h>
#include <gsl/gsl_fft_halfcomplex.h>
#include "nsl_dft.h"

#include <stdio.h>

const char* nsl_dft_result_type_name[] = { "Magnitude", "Amplitude", "real", "imag", "Power", "Phase", 
		"dB", "Magnitude squared", "Amplitude squared", "raw" };

int nsl_dft_transform(double data[], size_t stride, size_t n, nsl_dft_result_type type) {
	/* 1. transform */
	gsl_fft_real_wavetable *real = gsl_fft_real_wavetable_alloc(n);
	gsl_fft_real_workspace *work = gsl_fft_real_workspace_alloc(n);

	gsl_fft_real_transform (data, stride, n, real, work);
	gsl_fft_real_wavetable_free (real);

	/* 2. unpack data */	
	double res[2*n];
	gsl_complex_packed_array result = res;
	gsl_fft_halfcomplex_unpack(data, result, stride, n);

	/* 3. write result */
	unsigned int i;
	switch(type) {
	case nsl_dft_result_magnitude:
		for (i = 0; i < n; i++)
			data[i] = sqrt(gsl_pow_2(result[2*i])+gsl_pow_2(result[2*i+1]));
		break;
	case nsl_dft_result_amplitude:
		for (i = 0; i < n; i++)
			data[i] = sqrt(gsl_pow_2(result[2*i])+gsl_pow_2(result[2*i+1]))/n;
		break;
	case nsl_dft_result_real:
		for (i = 0; i < n; i++)
			data[i] = result[2*i];
		break;
	case nsl_dft_result_imag:
		for (i = 0; i < n; i++)
			data[i] = result[2*i+1];
		break;
	case nsl_dft_result_power:
		for (i = 0; i < n; i++)
			data[i] = (gsl_pow_2(result[2*i])+gsl_pow_2(result[2*i+1]))/n;
		break;
	case nsl_dft_result_phase:
		for (i = 0; i < n; i++)
			data[i] = -atan2(result[2*i+1],result[2*i]);
		break;
	case nsl_dft_result_dB:
		for (i = 0; i < n; i++)
			data[i] = 20.*log10(sqrt(gsl_pow_2(result[2*i])+gsl_pow_2(result[2*i+1]))/(double)n);
		break;
	case nsl_dft_result_squaremagnitude:
		for (i = 0; i < n; i++)
			data[i] = gsl_pow_2(result[2*i])+gsl_pow_2(result[2*i+1]);
		break;
	case nsl_dft_result_squareamplitude:
		for (i = 0; i < n; i++)
			data[i] = (gsl_pow_2(result[2*i])+gsl_pow_2(result[2*i+1]))/n/n;
		break;
	case nsl_dft_result_raw:
		break;
	}

	return 0;
}

