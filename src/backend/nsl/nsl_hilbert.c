/*
    File                 : nsl_hilbert.c
    Project              : LabPlot
    Description          : NSL Hilbert transform
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2021 Stefan Gerlach <stefan.gerlach@uni.kn>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#include "nsl_hilbert.h"
#include "nsl_common.h"
#include <gsl/gsl_fft_complex.h>
#include <gsl/gsl_fft_halfcomplex.h>
#ifdef HAVE_FFTW3
#include <fftw3.h>
#endif

const char* nsl_hilbert_result_type_name[] = {i18n("Imaginary part"), i18n("Envelope")};

/* algorithm from https://de.wikipedia.org/wiki/Hilbert-Transformation#Berechnung_%C3%BCber_Fouriertransformation */
int nsl_hilbert_transform(double data[], size_t stride, size_t n, nsl_hilbert_result_type type) {
	if (n < 2)	/* we need at least 2 points */
		return 1;

	/* 1. DFT of data: dft_transform returns gsl_halfcomplex (raw) */
	nsl_dft_transform(data, stride, n, 1, nsl_dft_result_raw);

	const size_t N = 2 * n;
	double* result = (double*)malloc(N*sizeof(double));
	gsl_fft_halfcomplex_unpack(data, result, stride, n);

	size_t i;
/*	for (i = 0; i < N; i++)
		printf(" %g", result[i]);
	printf("\n");
*/
	/* 2. manipulate values */
	for (i = 2; i < 2*ceil(n/2.); i++)
		result[i] *= 2;

/*	for (i = 0; i < N; i++)
		printf(" %g", result[i]);
	printf("\n");
*/
	for (i = n + 1; i < N; i++)
		result[i] = 0;

/*	for (i = 0; i < N; i++)
		printf(" %g", result[i]);
	printf("\n");
*/
	/* 3. back transform */
#ifdef HAVE_FFTW3
	fftw_complex* o = (fftw_complex*)malloc(N*sizeof(double));
	fftw_plan pb = fftw_plan_dft_1d(n, o, o, FFTW_BACKWARD, FFTW_ESTIMATE);

	fftw_execute_dft(pb, (fftw_complex*)result, (fftw_complex*)result);
	fftw_destroy_plan(pb);
	free(o);
#else
	gsl_fft_complex_workspace *work = gsl_fft_complex_workspace_alloc(n);
	gsl_fft_complex_wavetable *hc = gsl_fft_complex_wavetable_alloc(n);
	gsl_fft_complex_inverse(result, 1, n, hc, work);
	gsl_fft_complex_wavetable_free(hc);
	gsl_fft_complex_workspace_free(work);
#endif

/*	for (i = 0; i < N; i++)
#ifdef HAVE_FFTW3
		printf(" %g", result[i]/n);
#else
		printf(" %g", result[i]);
#endif
	printf("\n");
*/
	/* copy back (Hilbert transform is imag part) */
	switch (type) {
	case nsl_hilbert_result_imag:
		for (i = 0; i < n; i++)
#ifdef HAVE_FFTW3
			data[i] = result[2*i+1]/n;
#else
			data[i] = result[2*i+1];
#endif
		break;
	case nsl_hilbert_result_envelope:
		for (i = 0; i < n; i++)
#ifdef HAVE_FFTW3
			data[i] = hypot(result[2*i], result[2*i+1])/n;
#else
			data[i] = hypot(result[2*i], result[2*i+1]);
#endif
	}
		
	free(result);

	return 0;
}

