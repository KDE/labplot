/***************************************************************************
    File                 : nsl_filter.c
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

#include <stdio.h>
#include <math.h>
#include <gsl/gsl_fft_real.h>
#include <gsl/gsl_fft_halfcomplex.h>
#include "nsl_filter.h"

const char* nsl_filter_type_name[] = { "Low pass", "High pass", "Band pass", "Band reject" };
const char* nsl_filter_form_name[] = { "Ideal", "Butterworth", "Chebyshev type I", "Chebyshev type II" };
const char* nsl_filter_cutoff_unit_name[] = { "Frequency", "Fraction", "Index" };

int nsl_filter_transform(double data[], size_t n) {
	int status;
        /* TODO: use fftw3 if available */
/*#ifdef HAVE_FFTW3
      fftw_plan plan = fftw_plan_dft_r2c_1d(n, ydata, (fftw_complex *)ydata, FFTW_ESTIMATE);
      fftw_execute(plan);
      fftw_destroy_plan(plan);
#else*/
        gsl_fft_real_workspace *work = gsl_fft_real_workspace_alloc(n);
        gsl_fft_real_wavetable *real = gsl_fft_real_wavetable_alloc(n);

        /* double*, stride, size, wavetable, workspace */
	/* https://www.gnu.org/software/gsl/manual/html_node/Mixed_002dradix-FFT-routines-for-real-data.html */
        status = gsl_fft_real_transform(data, 1, n, real, work);
        gsl_fft_real_wavetable_free(real);
/*#endif*/

	return status;
}

int nsl_filter_backtransform(double data[], size_t n) {
	int status;

	gsl_fft_real_workspace *work = gsl_fft_real_workspace_alloc(n);
	gsl_fft_halfcomplex_wavetable *hc = gsl_fft_halfcomplex_wavetable_alloc(n);
	status = gsl_fft_halfcomplex_inverse(data, 1, n, hc, work);
	gsl_fft_halfcomplex_wavetable_free(hc);
	gsl_fft_real_workspace_free(work);

	return status;
}

int nsl_filter_apply(double data[], size_t n, nsl_filter_type type, nsl_filter_form form) {
	/*TODO*/
	return 0;
}
