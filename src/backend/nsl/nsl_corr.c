/*
	File                 : nsl_corr.c
	Project              : LabPlot
	Description          : NSL discrete correlation functions
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2018 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "nsl_corr.h"
#include "nsl_common.h"
#include <gsl/gsl_cblas.h>
#include <gsl/gsl_fft_halfcomplex.h>
#ifdef HAVE_FFTW3
#include <fftw3.h>
#endif

const char* nsl_corr_type_name[] = {i18n("Linear (Zero-padded)"), i18n("Circular")};
const char* nsl_corr_norm_name[] = {i18n("None"), i18n("Biased"), i18n("Unbiased"), i18n("Coeff")};

int nsl_corr_correlation(double s[], size_t n, double r[], size_t m, nsl_corr_type_type type, nsl_corr_norm_type normalize, double out[]) {
	return nsl_corr_fft_type(s, n, r, m, type, normalize, out);
}

int nsl_corr_fft_type(double s[], size_t n, double r[], size_t m, nsl_corr_type_type type, nsl_corr_norm_type normalize, double out[]) {
	size_t i, size, N = GSL_MAX(n, m), maxlag = N - 1;
	if (type == nsl_corr_type_linear)
		size = maxlag + N;
	else // circular
		size = N;

	size_t oldsize = size;
#ifdef HAVE_FFTW3
	// already zero-pad here for FFT method and FFTW r2c
	size = 2 * (size / 2 + 1);
#endif

	// zero-padded arrays
	double* stmp = (double*)malloc(size * sizeof(double));
	if (stmp == NULL) {
		printf("nsl_corr_fft_type(): ERROR allocating memory for 'stmp'!\n");
		return -1;
	}

	double* rtmp = (double*)malloc(size * sizeof(double));
	if (rtmp == NULL) {
		free(stmp);
		printf("nsl_corr_fft_type(): ERROR allocating memory for 'rtmp'!\n");
		return -1;
	}

	if (type == nsl_corr_type_linear) {
		for (i = 0; i < maxlag; i++)
			stmp[i] = 0.;
		for (i = 0; i < n; i++)
			stmp[i + maxlag] = s[i];
		for (i = n + maxlag; i < size; i++)
			stmp[i] = 0;
		for (i = 0; i < m; i++)
			rtmp[i] = r[i];
		for (i = m; i < size; i++)
			rtmp[i] = 0;
	} else { // circular
		for (i = 0; i < n; i++)
			stmp[i] = s[i];
		for (i = n; i < N; i++)
			stmp[i] = 0.;
		for (i = 0; i < m; i++)
			rtmp[i] = r[i];
		for (i = m; i < N; i++)
			rtmp[i] = 0.;
	}

	int status;
#ifdef HAVE_FFTW3 // already wraps output
	status = nsl_corr_fft_FFTW(stmp, rtmp, oldsize, out);
#else // GSL
	status = nsl_corr_fft_GSL(stmp, rtmp, size, out);
#endif

	free(stmp);
	free(rtmp);

	/* normalization */
	switch (normalize) {
	case nsl_corr_norm_none:
		break;
	case nsl_corr_norm_biased:
		for (i = 0; i < oldsize; i++)
			out[i] = out[i] / N;
		break;
	case nsl_corr_norm_unbiased:
		for (i = 0; i < oldsize; i++) {
			size_t norm = i < oldsize / 2 ? i + 1 : oldsize - i;
			out[i] = out[i] / norm;
		}
		break;
	case nsl_corr_norm_coeff: {
		double snorm = cblas_dnrm2((int)n, s, 1);
		double rnorm = cblas_dnrm2((int)m, r, 1);
		for (i = 0; i < oldsize; i++)
			out[i] = out[i] / snorm / rnorm;
		break;
	}
	}

	// reverse array for circular type
	if (type == nsl_corr_type_circular) {
		for (i = 0; i < N / 2; i++) {
			double tmp = out[i];
			out[i] = out[N - i - 1];
			out[N - i - 1] = tmp;
		}
	}

	return status;
}

#ifdef HAVE_FFTW3
int nsl_corr_fft_FFTW(double s[], double r[], size_t n, double out[]) {
	if (n == 0)
		return -1;

	const size_t size = 2 * (n / 2 + 1);
	double* in = (double*)malloc(size * sizeof(double));
	fftw_plan rpf = fftw_plan_dft_r2c_1d((int)n, in, (fftw_complex*)in, FFTW_ESTIMATE);

	fftw_execute_dft_r2c(rpf, s, (fftw_complex*)s);
	fftw_execute_dft_r2c(rpf, r, (fftw_complex*)r);
	fftw_destroy_plan(rpf);

	size_t i;

	// multiply
	for (i = 0; i < size; i += 2) {
		double re = s[i] * r[i] + s[i + 1] * r[i + 1];
		double im = s[i + 1] * r[i] - s[i] * r[i + 1];

		s[i] = re;
		s[i + 1] = im;
	}

	// back transform
	double* o = (double*)malloc(size * sizeof(double));
	fftw_plan rpb = fftw_plan_dft_c2r_1d((int)n, (fftw_complex*)o, o, FFTW_ESTIMATE);
	fftw_execute_dft_c2r(rpb, (fftw_complex*)s, s);
	fftw_destroy_plan(rpb);

	for (i = 0; i < n; i++)
		out[i] = s[i] / n;

	free(in);
	free(o);
	return 0;
}
#endif

int nsl_corr_fft_GSL(double s[], double r[], size_t n, double out[]) {
	gsl_fft_real_workspace* work = gsl_fft_real_workspace_alloc(n);
	gsl_fft_real_wavetable* real = gsl_fft_real_wavetable_alloc(n);

	/* FFT s and r */
	gsl_fft_real_transform(s, 1, n, real, work);
	gsl_fft_real_transform(r, 1, n, real, work);
	gsl_fft_real_wavetable_free(real);

	size_t i;
	/* calculate halfcomplex product */
	out[0] = s[0] * r[0];
	for (i = 1; i < n; i++) {
		if (i % 2) { /* Re */
			out[i] = s[i] * r[i];
			if (i < n - 1) /* when n is even last value is real */
				out[i] += s[i + 1] * r[i + 1];
		} else /* Im */
			out[i] = s[i] * r[i - 1] - s[i - 1] * r[i];
	}

	/* back transform */
	gsl_fft_halfcomplex_wavetable* hc = gsl_fft_halfcomplex_wavetable_alloc(n);
	gsl_fft_halfcomplex_inverse(out, 1, n, hc, work);
	gsl_fft_halfcomplex_wavetable_free(hc);
	gsl_fft_real_workspace_free(work);

	return 0;
}
