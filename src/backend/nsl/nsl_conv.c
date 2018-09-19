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
const char* nsl_conv_type_name[] = {i18n("linear (zero-padded)"), i18n("circular")};
const char* nsl_conv_method_name[] = {i18n("auto"), i18n("direct"), i18n("FFT")};
const char* nsl_conv_norm_name[] = {i18n("none"), i18n("Euclidean"), i18n("sum")};
const char* nsl_conv_wrap_name[] = {i18n("none"), i18n("maximum"), i18n("center (acausal)")};
const char* nsl_conv_kernel_name[] = {i18n("sliding average"), i18n("triangular smooth"), i18n("pseudo-Gaussian smooth"), i18n("first derivative"), i18n("smooth first derivative"),
	i18n("second derivative"), i18n("third derivative"), i18n("fourth derivative"), i18n("Gaussian"), i18n("Lorentzian") };

int nsl_conv_standard_kernel(double k[], size_t n, nsl_conv_kernel_type type) {
	size_t i;

	int validn = 1;
	switch (type) {
	case nsl_conv_kernel_avg:
		for (i = 0; i < n; i++)
			k[i] = 1.;
		break;
	case nsl_conv_kernel_smooth_triangle:
		for (i = 0; i < n/2; i++)
			k[i] = i + 1.;
		for (i = n/2; i < n; i++)
			k[i] = (double)(n - i);
		break;
	case nsl_conv_kernel_smooth_gaussian:
		switch (n) {
		case 5:
			k[0]=1;k[1]=4;k[2]=6;k[3]=4;k[4]=1;
			break;
		case 7:
			k[0]=1;k[1]=4;k[2]=8;k[3]=10;k[4]=8;k[5]=4;k[6]=1;
			break;
		case 9:
			k[0]=1;k[1]=4;k[2]=9;k[3]=14;k[4]=17;k[5]=14;k[6]=9;k[7]=4;k[8]=1;
			break;
		default:
			validn = 0;
		}
		break;
	case nsl_conv_kernel_first_derivative:
		if (n == 2) {
			k[0]=-1;k[1]=1;
		} else
			validn = 0;
		break;
	case nsl_conv_kernel_smooth_first_derivative:
		if (n%2) {
			for (i = 0; i < n; i++)
				k[i] = (int)(i - n/2);
		} else
			validn = 0;
		break;
	case nsl_conv_kernel_second_derivative:
		if (n == 3) {
			k[0]=1;k[1]=-2;k[2]=1;
		} else
			validn = 0;
		break;
	case nsl_conv_kernel_third_derivative:
		if (n == 4) {
			k[0]=1;k[1]=-3;k[2]=3;k[3]=-1;
		} else
			validn = 0;
		break;
	case nsl_conv_kernel_fourth_derivative:
		if (n == 5) {
			k[0]=1;k[1]=-4;k[2]=6;k[3]=-4;k[4]=1;
		} else
			validn = 0;
		break;
	case nsl_conv_kernel_gaussian: {
		double s = n/5.;	/* relative width */
		for (i = 0;i < n; i++) {
			double x = i - (n-1.)/2.;
			k[i] = 1./sqrt(2.*M_PI)/s * exp(-x*x/2./s/s);
		}
		break;
	}
	case nsl_conv_kernel_lorentzian: {
		double s = n/5.;	/* relative width */
		for (i = 0;i < n; i++) {
			double x = i - (n-1.)/2.;
			k[i] = s/M_PI/(s*s + x*x);
		}
		break;
	}
	}

	if (!validn) {
		printf("ERROR: kernel size %zu not supported for kernel %s\n", n, nsl_conv_kernel_name[type]);
		return -1;
	}

	/* debug */
	printf("[");
	for (i = 0; i < n; i++)
		printf("%g ", k[i]);
	puts("]");

	return 0;
}

int nsl_conv_convolution_direction(double s[], size_t n, double r[], size_t m, nsl_conv_direction_type dir, nsl_conv_type_type type, nsl_conv_method_type method,
		nsl_conv_norm_type normalize, nsl_conv_wrap_type wrap, double out[]) {
	if (dir == nsl_conv_direction_forward)
		return nsl_conv_convolution(s, n, r, m, type, method, normalize, wrap, out);
	else
		return nsl_conv_deconvolution(s, n, r, m, type, normalize, wrap, out);
}

int nsl_conv_convolution(double s[], size_t n, double r[], size_t m, nsl_conv_type_type type, nsl_conv_method_type method, nsl_conv_norm_type normalize, nsl_conv_wrap_type wrap, double out[]) {
	if (method == nsl_conv_method_direct || (method == nsl_conv_method_auto && GSL_MAX_INT(n, m) <= NSL_CONV_METHOD_BORDER)) {
		if (type == nsl_conv_type_linear)
			return nsl_conv_linear_direct(s, n, r, m, normalize, wrap, out);
		else if (type == nsl_conv_type_circular)
			return nsl_conv_circular_direct(s, n, r, m, normalize, wrap, out);
	} else {
		return nsl_conv_fft_type(s, n, r, m, nsl_conv_direction_forward, type, normalize, wrap, out);
	}

	return 0;
}

int nsl_conv_deconvolution(double s[], size_t n, double r[], size_t m, nsl_conv_type_type type, nsl_conv_norm_type normalize, nsl_conv_wrap_type wrap, double out[]) {
	/* only supported by FFT method */
	return nsl_conv_fft_type(s, n, r, m, nsl_conv_direction_backward, type, normalize, wrap, out);
}

int nsl_conv_linear_direct(double s[], size_t n, double r[], size_t m, nsl_conv_norm_type normalize, nsl_conv_wrap_type wrap, double out[]) {
	size_t i, j, size = n + m - 1, wi = 0;
	double norm = 1;
	if (normalize == nsl_conv_norm_euclidean) {
		if ((norm = cblas_dnrm2((int)m, r, 1)) == 0)
			norm = 1.;
	} else if (normalize == nsl_conv_norm_sum) {
		if ((norm = cblas_dasum((int)m, r, 1)) == 0)
			norm = 1.;
	}

	if (wrap == nsl_conv_wrap_max)
		nsl_stats_maximum(r, m, &wi);
	else if (wrap == nsl_conv_wrap_center)
		wi = m/2;

	for (j = 0; j < size; j++) {
		int index;	// can be negative
		double res = 0;
		for (i = 0; i < n; i++) {
			index = (int)(j - i);
			if (index >= 0 && index < (int)m)
				res += s[i] * r[index]/norm;
		}
		index = (int)(j - wi);
		if (index < 0)
			index += (int)size;
		out[index] = res;
	}

	return 0;
}

int nsl_conv_circular_direct(double s[], size_t n, double r[], size_t m, nsl_conv_norm_type normalize, nsl_conv_wrap_type wrap, double out[]) {
	size_t i, j, size = GSL_MAX(n,m), wi = 0;
	double norm = 1;
	if (normalize == nsl_conv_norm_euclidean) {
		if ((norm = cblas_dnrm2((int)m, r, 1)) == 0)
			norm = 1.;
	} else if (normalize == nsl_conv_norm_sum) {
		if ((norm = cblas_dasum((int)m, r, 1)) == 0)
			norm = 1.;
	}

	if (wrap == nsl_conv_wrap_max)
		nsl_stats_maximum(r, m, &wi);
	else if (wrap == nsl_conv_wrap_center)
		wi = m/2;

	for (j = 0; j < size; j++) {
		int index;	// can be negative
		double res = 0;
		for (i = 0; i < n; i++) {
			index = (int)(j - i);
			if (index < 0)
				index += (int)size;
			if (index < (int)m)
				res += s[i]*r[index]/norm;
		}
		index = (int)(j - wi);
		if (index < 0)
			index += (int)size;
		out[index] = res;
	}

	return 0;
}

int nsl_conv_fft_type(double s[], size_t n, double r[], size_t m, nsl_conv_direction_type dir, nsl_conv_type_type type, nsl_conv_norm_type normalize, nsl_conv_wrap_type wrap, double out[]) {
	size_t i, size, wi = 0;
	if (type == nsl_conv_type_linear)
		size = n + m - 1;
	else	// circular
		size = GSL_MAX(n, m);

	double norm = 1.;
	if (normalize == nsl_conv_norm_euclidean) {
		if ((norm = cblas_dnrm2((int)m, r, 1)) == 0)
			norm = 1.;
	} else if (normalize == nsl_conv_norm_sum) {
		if ((norm = cblas_dasum((int)m, r, 1)) == 0)
			norm = 1.;
	}

	if (wrap == nsl_conv_wrap_max)
		nsl_stats_maximum(r, m, &wi);
	else if (wrap == nsl_conv_wrap_center)
		wi = m/2;

#ifdef HAVE_FFTW3
	// already zero-pad here for FFT method and FFTW r2c
	size_t oldsize = size;
	size = 2*(size/2+1);
#endif

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
	for (i = 0; i < m; i++) {
		rtmp[i] = r[i]/norm;	/* norm response */
	}
	for (i = m; i < size; i++)
		rtmp[i] = 0;

	int status;
#ifdef HAVE_FFTW3	// already wraps output
	status = nsl_conv_fft_FFTW(stmp, rtmp, oldsize, dir, wi, out);
#else	// GSL
	status = nsl_conv_fft_GSL(stmp, rtmp, size, dir, out);
	for (i = 0; i < size; i++) {	// wrap output
		size_t index = (i + wi) % size;
		stmp[i] = out[index];
	}
	memcpy(out, stmp, size * sizeof(double));
#endif

	free(stmp);
	free(rtmp);

	return status;
}

#ifdef HAVE_FFTW3
int nsl_conv_fft_FFTW(double s[], double r[], size_t n, nsl_conv_direction_type dir, size_t wi, double out[]) {
	size_t i;
	const size_t size = 2*(n/2+1);
	double* in = (double*)malloc(size*sizeof(double));
	fftw_plan rpf = fftw_plan_dft_r2c_1d(n, in, (fftw_complex*)in, FFTW_ESTIMATE);

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
			double norm = r[i]*r[i] + r[i+1]*r[i+1];
			if (norm < DBL_MIN)
				norm = 1.;
			double re = (s[i]*r[i] + s[i+1]*r[i+1])/norm;
			double im = (s[i+1]*r[i] - s[i]*r[i+1])/norm;

			s[i] = re;
			s[i+1] = im;
		}
	}

	// back transform
	double* o = (double*)malloc(size*sizeof(double));
	fftw_plan rpb = fftw_plan_dft_c2r_1d(n, (fftw_complex*)o, o, FFTW_ESTIMATE);
	fftw_execute_dft_c2r(rpb, (fftw_complex*)s, s);
	fftw_destroy_plan(rpb);

	for (i = 0; i < n; i++) {
		size_t index = (i + wi) % n;
		out[i] = s[index]/n;
	}

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
				else {
					double norm = r[i]*r[i] + r[i+1]*r[i+1];
					if (norm < DBL_MIN)
						norm = 1.;
					out[i] = (s[i]*r[i] + s[i+1]*r[i+1])/norm;
				}
			} else { 	/* Im */
				double norm = r[i-1]*r[i-1] + r[i]*r[i];
				if (norm < DBL_MIN)
					norm = 1.;
				out[i] = (s[i]*r[i-1] - s[i-1]*r[i])/norm;
			}
		}
	}

	/* back transform */
	gsl_fft_halfcomplex_wavetable *hc = gsl_fft_halfcomplex_wavetable_alloc(n);
	gsl_fft_halfcomplex_inverse(out, 1, n, hc, work);
	gsl_fft_halfcomplex_wavetable_free(hc);
	gsl_fft_real_workspace_free(work);

	return 0;
}

