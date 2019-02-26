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

#include "nsl_filter.h"
#include "nsl_common.h"
#include "nsl_sf_poly.h"
#include <gsl/gsl_sf_pow_int.h>
#include <gsl/gsl_fft_real.h>
#include <gsl/gsl_fft_halfcomplex.h>
#ifdef HAVE_FFTW3
#include <fftw3.h>
#endif

const char* nsl_filter_type_name[] = { i18n("Low pass"), i18n("High pass"), i18n("Band pass"), i18n("Band reject") };
const char* nsl_filter_form_name[] = { i18n("Ideal"), i18n("Butterworth"), i18n("Chebyshev type I"), i18n("Chebyshev type II"), i18n("Legendre (Optimum L)"), i18n("Bessel (Thomson)") };
const char* nsl_filter_cutoff_unit_name[] = { i18n("Frequency"), i18n("Fraction"), i18n("Index") };

/* n - order, x = w/w0 */
double nsl_filter_gain_bessel(int n, double x) {
#ifdef _MSC_VER
	COMPLEX z0 = {0.0, 0.0};
	COMPLEX z = {0.0, (const double)x};
	double norm = cabs(nsl_sf_poly_reversed_bessel_theta(n, z));
	COMPLEX value = nsl_sf_poly_reversed_bessel_theta(n, z0);
	return creal(value)/norm;
#else
	return nsl_sf_poly_reversed_bessel_theta(n, 0)/cabs(nsl_sf_poly_reversed_bessel_theta(n, I*x));
#endif
}

int nsl_filter_apply(double data[], size_t n, nsl_filter_type type, nsl_filter_form form, int order, double cutindex, double bandwidth) {
	if (cutindex < 0) {
		printf("index for cutoff must be >= 0\n");
		return -1;
	}
	if (cutindex > n/2+1) {
		printf("index for cutoff must be <= n/2+1\n");
		return -1;
	}

	size_t i;
	double factor, centerindex = cutindex + bandwidth/2.;
	switch (type) {
	case nsl_filter_type_low_pass:
		switch (form) {
		case nsl_filter_form_ideal:
			for (i = (size_t)cutindex; i < n/2+1; i++)
				data[2*i] = data[2*i+1] = 0;
			break;
		case nsl_filter_form_butterworth:
			for (i = 0; i < n/2+1; i++) {
				factor = 1./sqrt(1. + gsl_sf_pow_int(i/cutindex, 2*order));
				data[2*i] *= factor;
				data[2*i+1] *= factor;
			}
			break;
		case nsl_filter_form_chebyshev_i:
			for (i = 0; i < n/2+1; i++) {
				factor = 1./sqrt(1. + gsl_sf_pow_int(nsl_sf_poly_chebyshev_T(order, i/cutindex), 2));
				data[2*i] *= factor;
				data[2*i+1] *= factor;
			}
			break;
		case nsl_filter_form_chebyshev_ii:
			for (i = 1; i < n/2+1; i++) {	/* i==0: factor=1 */
				factor = 1./sqrt(1. + 1./gsl_sf_pow_int(nsl_sf_poly_chebyshev_T(order, cutindex/i), 2));
				data[2*i] *= factor;
				data[2*i+1] *= factor;
			}
			break;
		case nsl_filter_form_legendre:
			for (i = 0; i < n/2+1; i++) {
				factor = 1./sqrt(1. + nsl_sf_poly_optimal_legendre_L(order, i*i/(cutindex*cutindex) ));
				data[2*i] *= factor;
				data[2*i+1] *= factor;
			}
			break;
		case nsl_filter_form_bessel:
			for (i = 0; i < n/2+1; i++) {
				factor = nsl_filter_gain_bessel(order, i/cutindex);
				data[2*i] *= factor;
				data[2*i+1] *= factor;
			}
			break;
		}
		break;
       case nsl_filter_type_high_pass:
		switch (form) {
		case nsl_filter_form_ideal:
			for (i = 0; i < cutindex; i++)
				data[2*i] = data[2*i+1] = 0;
			break;
		case nsl_filter_form_butterworth:
			data[0]=data[1]=0;
			for (i = 1; i < n/2+1; i++) {
				factor = 1./sqrt(1.+gsl_sf_pow_int(cutindex/i, 2*order));
				data[2*i] *= factor;
				data[2*i+1] *= factor;
			}
			break;
		case nsl_filter_form_chebyshev_i:
			data[0]=data[1]=0;
			for (i = 1; i < n/2+1; i++) {
				factor = 1./sqrt(1.+gsl_sf_pow_int(nsl_sf_poly_chebyshev_T(order, cutindex/i), 2));
				data[2*i] *= factor;
				data[2*i+1] *= factor;
			}
			break;
		case nsl_filter_form_chebyshev_ii:
			for (i = 0; i < n/2+1; i++) {
				factor = 1./sqrt(1.+1./gsl_sf_pow_int(nsl_sf_poly_chebyshev_T(order, i/cutindex), 2));
				data[2*i] *= factor;
				data[2*i+1] *= factor;
			}
			break;
		case nsl_filter_form_legendre:
			data[0]=data[1]=0;
			for (i = 1; i < n/2+1; i++) {
				factor = 1./sqrt(1. + nsl_sf_poly_optimal_legendre_L(order, cutindex*cutindex/(i*i) ));
				data[2*i] *= factor;
				data[2*i+1] *= factor;
			}
			break;
		case nsl_filter_form_bessel:
			data[0]=data[1]=0;
			for (i = 1; i < n/2+1; i++) {
				factor = nsl_filter_gain_bessel(order, cutindex/i);
				data[2*i] *= factor;
				data[2*i+1] *= factor;
			}
			break;
		}
		break;
	case nsl_filter_type_band_pass:
		switch (form) {
		case nsl_filter_form_ideal:
			for (i = 0; i < (size_t)cutindex; i++)
				data[2*i] = data[2*i+1] = 0;
			for (i = (size_t)(cutindex + bandwidth); i < n/2+1; i++)
				data[2*i] = data[2*i+1] = 0;
			break;
		case nsl_filter_form_butterworth:
			data[0]=data[1]=0;
			for (i = 1; i < n/2+1; i++) {
				factor = 1./sqrt(1.+gsl_sf_pow_int((i*i - centerindex*centerindex)/i/bandwidth, 2*order));
				data[2*i] *= factor;
				data[2*i+1] *= factor;
			}
			break;
		case nsl_filter_form_chebyshev_i:
			data[0]=data[1]=0;
			for (i = 1; i < n/2+1; i++) {
				factor = 1./sqrt(1.+gsl_sf_pow_int(nsl_sf_poly_chebyshev_T(order, (i*i - centerindex*centerindex)/i/bandwidth), 2));
				data[2*i] *= factor;
				data[2*i+1] *= factor;
			}
			break;
		case nsl_filter_form_chebyshev_ii:
			for (i = 0; i < n/2+1; i++) {
				factor = 1./sqrt(1.+1./gsl_sf_pow_int(nsl_sf_poly_chebyshev_T(order, i*bandwidth/(i*i - centerindex*centerindex)), 2));
				data[2*i] *= factor;
				data[2*i+1] *= factor;
			}
			break;
		case nsl_filter_form_legendre:
			data[0]=data[1]=0;
			for (i = 1; i < n/2+1; i++) {
				factor = 1./sqrt(1. + nsl_sf_poly_optimal_legendre_L(order,
								(i*i-2.*centerindex*centerindex+gsl_pow_4(centerindex)/(i*i))/gsl_pow_2(bandwidth) ));
				data[2*i] *= factor;
				data[2*i+1] *= factor;
			}
			break;
		case nsl_filter_form_bessel:
			data[0]=data[1]=0;
			for (i = 1; i < n/2+1; i++) {
				factor = nsl_filter_gain_bessel(order, (i*i - centerindex*centerindex)/i/bandwidth);
				data[2*i] *= factor;
				data[2*i+1] *= factor;
			}
			break;
		}
		break;
	case nsl_filter_type_band_reject:
		switch (form) {
		case nsl_filter_form_ideal:
			for (i = (size_t)cutindex; i < (size_t)(cutindex + bandwidth); i++)
				data[2*i] = data[2*i+1] = 0;
			break;
		case nsl_filter_form_butterworth:
			for (i = 0; i < n/2+1; i++) {
				factor = 1./sqrt(1.+gsl_sf_pow_int(i*bandwidth/(i*i - centerindex*centerindex), 2*order));
				data[2*i] *= factor;
				data[2*i+1] *= factor;
			}
			break;
		case nsl_filter_form_chebyshev_i:
			for (i = 0; i < n/2+1; i++) {
				factor = 1./sqrt(1.+gsl_sf_pow_int(nsl_sf_poly_chebyshev_T(order, i*bandwidth/(i*i - centerindex*centerindex)), 2));
				data[2*i] *= factor;
				data[2*i+1] *= factor;
			}
			break;
		case nsl_filter_form_chebyshev_ii:
			for (i = 1; i < n/2+1; i++) {	/* i==0: factor=1 */
				factor = 1./sqrt(1.+1./gsl_sf_pow_int(nsl_sf_poly_chebyshev_T(order, (i*i - centerindex*centerindex)/i/bandwidth), 2));
				data[2*i] *= factor;
				data[2*i+1] *= factor;
			}
			break;
		case nsl_filter_form_legendre:
			for (i = 0; i < n/2+1; i++) {
				factor = 1./sqrt(1. + nsl_sf_poly_optimal_legendre_L(order,
								gsl_pow_2(i*bandwidth)/gsl_pow_2(i*i-centerindex*centerindex)  ));
				data[2*i] *= factor;
				data[2*i+1] *= factor;
			}
			break;
		case nsl_filter_form_bessel:
			for (i = 0; i < n/2+1; i++) {
				if (i == centerindex)
					factor = 0;
				else
					factor = nsl_filter_gain_bessel(order, i*bandwidth/(i*i - centerindex*centerindex));
				data[2*i] *= factor;
				data[2*i+1] *= factor;
			}
			break;
		}
		break;
	}

	return 0;
}

void print_fdata(double data[], size_t n) {
	size_t i;
	for(i=0; i < 2*(n/2+1); i++)
		printf("%g ", data[i]);
	printf("\nreal: ");
	for(i=0; i < n/2+1; i++)
		printf("%g ", data[2*i]);
	printf("\nimag: ");
	for(i=0; i < n/2+1; i++)
		printf("%g ", data[2*i+1]);
	puts("");
}

int nsl_filter_fourier(double data[], size_t n, nsl_filter_type type, nsl_filter_form form, int order, int cutindex, int bandwidth) {
	/* 1. transform */
	double* fdata = (double*)malloc(2*n*sizeof(double));	/* contains re0,im0,re1,im1,re2,im2,... */
#ifdef HAVE_FFTW3
        fftw_plan plan = fftw_plan_dft_r2c_1d(n, data, (fftw_complex *) fdata, FFTW_ESTIMATE);
        fftw_execute(plan);
	fftw_destroy_plan(plan);
#else
	gsl_fft_real_wavetable *real = gsl_fft_real_wavetable_alloc(n);
	gsl_fft_real_workspace *work = gsl_fft_real_workspace_alloc(n);

	gsl_fft_real_transform(data, 1, n, real, work);
	gsl_fft_real_wavetable_free(real);

	gsl_fft_halfcomplex_unpack(data, fdata, 1, n);
#endif

	/* 2. apply filter */
	/*print_fdata(fdata, n);*/
	int status = nsl_filter_apply(fdata, n, type, form, order, cutindex, bandwidth);
	/*print_fdata(fdata, n);*/
	
	/* 3. back transform */
#ifdef HAVE_FFTW3
        plan = fftw_plan_dft_c2r_1d(n, (fftw_complex *) fdata, data, FFTW_ESTIMATE);
        fftw_execute(plan);
	fftw_destroy_plan(plan);
	/* normalize*/
	size_t i;
	for (i=0; i < n; i++)
		data[i] /= n;
#else
	gsl_fft_halfcomplex_wavetable *hc = gsl_fft_halfcomplex_wavetable_alloc(n);
	gsl_fft_halfcomplex_inverse(data, 1, n, hc, work);
	gsl_fft_halfcomplex_wavetable_free(hc);
	gsl_fft_real_workspace_free (work);
#endif
	free(fdata);

	return status;
}
