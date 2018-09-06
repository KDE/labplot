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
#include <gsl/gsl_cblas.h>
#include <gsl/gsl_fft_halfcomplex.h>
#ifdef HAVE_FFTW3
#include <fftw3.h>
#endif
#include "backend/nsl/nsl_stats.h"

const char* nsl_conv_direction_name[] = {i18n("forward (convolution)"), i18n("backward (deconvolution)")};
const char* nsl_conv_method_name[] = {i18n("auto"), i18n("direct"), i18n("FFT")};
const char* nsl_conv_type_name[] = {i18n("linear (zero-padded)"), i18n("circular")};

int nsl_conv_convolution_direction(double s[], size_t n, double r[], size_t m, nsl_conv_direction_type dir, nsl_conv_method_type method, nsl_conv_type_type type,
		int normalize, int wrap, double out[]) {
	if (dir == nsl_conv_direction_forward)
		return nsl_conv_convolution(s, n, r, m, method, type, normalize, wrap, out);
	else
		return nsl_conv_deconvolution(s, n, r, m, type, normalize, wrap, out);
}

int nsl_conv_convolution(double s[], size_t n, double r[], size_t m, nsl_conv_method_type method, nsl_conv_type_type type, int normalize, int wrap, double out[]) {
	if (method == nsl_conv_method_direct || (method == nsl_conv_method_auto && GSL_MAX_INT(n,m) <= NSL_CONV_METHOD_BORDER)) {
		if (type == nsl_conv_type_linear)
			return nsl_conv_linear_direct(s, n, r, m, normalize, wrap, out);
		else if (type == nsl_conv_type_circular)
			return nsl_conv_circular_direct(s, n, r, m, normalize, wrap, out);
	} else {
		return nsl_conv_fft_type(s, n, r, m, nsl_conv_direction_forward, type, out);
	}

	return 0;
}

int nsl_conv_deconvolution(double s[], size_t n, double r[], size_t m, nsl_conv_type_type type, int normalize, int wrap, double out[]) {
	/* only supported by FFT method */
	return nsl_conv_fft_type(s, n, r, m, nsl_conv_direction_backward, type, out);
}

int nsl_conv_linear_direct(double s[], size_t n, double r[], size_t m, int normalize, int wrap, double out[]) {
	size_t i, j, wi = 0;
	double norm = 1;
	if (normalize)
		norm = cblas_dnrm2(m, r, 1);
	if (wrap)
		nsl_stats_maximum(r, m, &wi);

	for (j = 0; j < n + m - 1; j++) {
		int index;
		double res = 0;
		for (i = 0; i < n; i++) {
			index = j - i;
			if (index >= 0 && index < (int)m)
				res += s[i] * r[index]/norm;
		}
		index = j - wi;
		if (index < 0)
			index += n+m-1;
		out[index] = res;
	}

	return 0;
}

int nsl_conv_circular_direct(double s[], size_t n, double r[], size_t m, int normalize, int wrap, double out[]) {
	size_t i, j, size = GSL_MAX(n,m), wi = 0;
	double norm = 1;
	if (normalize)
		norm = cblas_dnrm2(m, r, 1);
	if (wrap)
		nsl_stats_maximum(r, m, &wi);

	for (j = 0; j < size; j++) {
		int index;
		double res = 0;
		for (i = 0; i < n; i++) {
			index = j-i;
			if (index < 0)
				index += size;
			if (index < (int)m)
				res += s[i]*r[index]/norm;
		}
		index = j - wi;
		if (index < 0)
			index += size;
		out[index] = res;
	}

	return 0;
}

int nsl_conv_fft_type(double s[], size_t n, double r[], size_t m, nsl_conv_direction_type dir, nsl_conv_type_type type, double out[]) {
	size_t i, size;
	if (type == nsl_conv_type_linear)
		size = n + m - 1;
	else	// circular
		size = GSL_MAX(n, m);

	// zero-padded arrays
	double *stmp = (double*)malloc(size*sizeof(double));
	double *rtmp = (double*)malloc(size*sizeof(double));
	if (stmp == NULL || rtmp == NULL) {
		puts("ERROR: zero-padding data");
		return -1;
	}
	for (i = 0; i < n; i++)
		stmp[i] = s[i];
	for (i = n; i < size; i++)
		stmp[i] = 0;
	for (i = 0; i < m; i++)
		rtmp[i] = r[i];
	for (i = m; i < size; i++)
		rtmp[i] = 0;

	int status = nsl_conv_fft(stmp, rtmp, size, dir, out);

	return status;
}

#ifdef HAVE_FFTW3
int nsl_conv_fft_FFTW(double sin[], double rin[], size_t n, nsl_conv_direction_type dir, double out[]) {
	size_t i;
	const size_t size = 2*(n/2+1);
	double* in = (double*)malloc(size*sizeof(double));
	fftw_plan rpf = fftw_plan_dft_r2c_1d(n, in, (fftw_complex*)in, FFTW_ESTIMATE);

	/* zero-pad input */
	double *s = realloc(sin, size*sizeof(double));
	if (s == NULL) {
		printf("ERROR: zero-padding data");
		free(sin);
		free(rin);
		return -1;
	}
	double *r = realloc(rin, size*sizeof(double));
	if (r == NULL) {
		printf("ERROR: zero-padding data");
		free(s);
		free(rin);
		return -1;
	}
	for (i = n; i < size; i++) {
		s[i] = 0;
		r[i] = 0;
	}

	fftw_execute_dft_r2c(rpf, s, (fftw_complex*)s);
	fftw_execute_dft_r2c(rpf, r, (fftw_complex*)r);
	fftw_destroy_plan(rpf);

	// multiply/divide
	if (dir == nsl_conv_direction_forward) {
		for (i = 0; i < size; i += 2) {
			double re = s[i]*r[i] - s[i+1]*r[i+1];
			double im = s[i]*r[i+1] + s[i+1]*r[i];

			s[i] = re;
			s[i+1] = im;
		}
	} else {
		for (i = 0; i < size; i += 2) {
			double tmp = r[i]*r[i] + r[i+1]*r[i+1];
			double re = (s[i]*r[i] + s[i+1]*r[i+1])/tmp;
			double im = (s[i+1]*r[i] - s[i]*r[i+1])/tmp;

			s[i] = re;
			s[i+1] = im;
		}
	}

	// back transform
	double* o = (double*)malloc(size*sizeof(double));
	fftw_plan rpb = fftw_plan_dft_c2r_1d(n, (fftw_complex*)o, o, FFTW_ESTIMATE);
	fftw_execute_dft_c2r(rpb, (fftw_complex*)s, s);
	fftw_destroy_plan(rpb);

	for (i = 0; i < n; i++)
		out[i] = s[i]/n;

	free(s);
	free(r);

	return 0;
}
#endif

int nsl_conv_fft_GSL(double s[], double r[], size_t n, nsl_conv_direction_type dir, double out[]) {
	gsl_fft_real_workspace *work = gsl_fft_real_workspace_alloc(n);
	gsl_fft_real_wavetable *real = gsl_fft_real_wavetable_alloc(n);

	/* FFT s and r */
	gsl_fft_real_transform(s, 1, n, real, work);
	gsl_fft_real_transform(r, 1, n, real, work);
	gsl_fft_real_wavetable_free(real);

	size_t i;
	/* calculate halfcomplex product/quotient depending on direction */
	if (dir == nsl_conv_direction_forward) {
		out[0] = s[0]*r[0];
		for (i = 1; i < n; i++) {
			if (i%2) { /* Re */
				out[i] = s[i]*r[i];
				if (i < n-1) /* when n is even last value is real */
					out[i] -= s[i+1]*r[i+1];
			} else 	/* Im */
				out[i] = s[i-1]*r[i] + s[i]*r[i-1];
		}
	} else {
		out[0] = s[0]/r[0];
		for (i = 1; i < n; i++) {
			if (i%2) { /* Re */
				if (i == n-1) /* when n is even last value is real */
					out[i] = s[i]/r[i];
				else
					out[i] = (s[i]*r[i] + s[i+1]*r[i+1])/(r[i]*r[i] + r[i+1]*r[i+1]);
			} else 	/* Im */
				out[i] = (s[i]*r[i-1] - s[i-1]*r[i])/(r[i-1]*r[i-1] + r[i]*r[i]);
		}
	}

	/* back transform */
	gsl_fft_halfcomplex_wavetable *hc = gsl_fft_halfcomplex_wavetable_alloc(n);
	gsl_fft_halfcomplex_inverse(out, 1, n, hc, work);
	gsl_fft_halfcomplex_wavetable_free(hc);
	gsl_fft_real_workspace_free(work);

	return 0;
}

int nsl_conv_fft(double s[], double r[], size_t n, nsl_conv_direction_type dir, double out[]) {
	int status;
#ifdef HAVE_FFTW3
	return nsl_conv_fft_FFTW(s, r, n, dir, out);
	/* s and r handled in function */
#else
	status = nsl_conv_fft_GSL(s, r, n, dir, out);
	free(s);
	free(r);
#endif

	return status;
}
