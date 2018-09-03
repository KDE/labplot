/***************************************************************************
    File                 : nsl_conv.c
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

#include "nsl_conv.h"
#include "nsl_common.h"
#include <gsl/gsl_fft_halfcomplex.h>

const char* nsl_conv_direction_name[] = {i18n("forward (convolution)"), i18n("backward (deconvolution)")};
const char* nsl_conv_type_name[] = {i18n("linear (zero-padded)"), i18n("circular")};

int nsl_conv_convolution(double sig[], size_t n, double res[], size_t m, nsl_conv_direction_type dir) {
	/* TODO: use different versions */
	return nsl_conv_linear_fft(sig, n, res, m, dir);
}

/* adapted from SciDAVis */
/* TODO: implement own mixed-radix version */
int nsl_conv_linear_fft(double sig[], size_t n, double inres[], size_t m, nsl_conv_direction_type dir) {

	double* res = (double*)malloc(n * sizeof(double));
	if (res == NULL) {
		printf("nsl_conv_convolution(): ERROR allocating memory!\n");
		return -1;
	}
	memset(res, 0, n * sizeof(double));

	size_t i, m2 = m/2;
	for (i = 0; i < m2; i++) {	// store the response in wrap around order (see Numerical Recipes)
		res[i] = inres[m2+i];
		res[n-m2+i] = inres[i];
	}

	if (m2%2 == 1)
		res[m2] = inres[m-1];

	/* FFT both */
	/* TODO: use mixed radix versions */
	gsl_fft_real_radix2_transform(res, 1, n);
	gsl_fft_real_radix2_transform(sig, 1, n);

	double re, im, size;
	for (i = 0; i < n/2; i++) {	/* multiply/divide FFTs */
		if (i == 0 || i == n/2 - 1) {
			if (dir == nsl_conv_direction_forward)
				sig[i] = res[i]*sig[i];
			else {
				if (res[i] > 0)
					sig[i] = sig[i]/res[i];
				else
					sig[i] = 0.;
			}
		} else {
			if (dir == nsl_conv_direction_forward) {
				re = res[i]*sig[i] - res[n-i]*sig[n-i];
				im = res[i]*sig[n-i] + res[n-i]*sig[i];
			} else {
				size = res[i]*res[i] + res[n-i]*res[n-i];
				re = res[i]*sig[i] + res[n-i]*sig[n-i];
				im = res[i]*sig[n-i] - res[n-i]*sig[i];
				if (size > 0) {
					re /= size;
					im /= size;
				}
			}

			sig[i] = re;
			sig[n-i] = im;
		}
	}
	free(res);

	/* inverse FFT */
	/* TODO: use mixed radix version */
	gsl_fft_halfcomplex_radix2_inverse(sig, 1, n);

	return 0;
}
