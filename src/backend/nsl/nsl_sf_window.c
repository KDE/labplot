/*
    File                 : nsl_sf_window.c
    Project              : LabPlot
    Description          : NSL special window functions
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2016-2017 Stefan Gerlach (stefan.gerlach@uni.kn)
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#include "nsl_sf_window.h"
#include "nsl_common.h"
#include <gsl/gsl_math.h>
#include <gsl/gsl_sf_trig.h>

const char* nsl_sf_window_type_name[] = {i18n("rectangular (uniform)"), i18n("triangular"), i18n("triangular II (Bartlett)"), i18n("triangular III (Parzen)") ,
	i18n("Welch (parabolic)"), i18n("Hann (raised cosine)"), i18n("Hamming"), i18n("Blackman"), i18n("Nuttall"), i18n("Blackman-Nuttall"), i18n("Blackman-Harris"),
	i18n("Flat top"), i18n("Cosine"), i18n("Bartlett-Hann"), i18n("Lanczos")};

int nsl_sf_apply_window(double data[], size_t N, nsl_sf_window_type type) {
	if (N == 0)
		return -1;

	size_t i;
	switch (type) {
	case nsl_sf_window_uniform:
		/* do nothing */
		break;
	case nsl_sf_window_triangle:
		for (i = 0; i < N; i++)
			data[i] = 1.0 - 2./N*fabs(i-(N-1)/2.);
		break;
	case nsl_sf_window_triangleII:
		for (i = 0; i < N; i++)
			data[i] = 1.0 - 2./(N-1)*fabs(i-(N-1)/2.);
		break;
	case nsl_sf_window_triangleIII:
		for (i = 0; i < N; i++)
			data[i] = 1.0 - 2./(N+1)*fabs(i-(N-1)/2.);
		break;
	case nsl_sf_window_welch:
		for (i = 0; i < N; i++)
			data[i] = 1.0 - gsl_pow_2(2*(i-(N-1)/2.)/(N+1));
		break;
	case nsl_sf_window_hann:
		for (i = 0; i < N; i++)
			data[i] = 0.5*(1. - cos(2.*M_PI*i/(N-1)));
		break;
	case nsl_sf_window_hamming:
		for (i = 0; i < N; i++)
			data[i] = 0.54 - 0.46*cos(2.*M_PI*i/(N-1));
		break;
	case nsl_sf_window_blackman:
		for (i = 0; i < N; i++)
			data[i] = 0.42 - 0.5*cos(2.*M_PI*i/(N-1)) + 0.08*cos(4.*M_PI*i/(N-1));
		break;
	case nsl_sf_window_nuttall:
		for (i = 0; i < N; i++)
			data[i] = 0.355768 - 0.487396*cos(2.*M_PI*i/(N-1)) + 0.144232*cos(4.*M_PI*i/(N-1)) - 0.012604*cos(6.*M_PI*i/(N-1));
		break;
	case nsl_sf_window_blackman_nuttall:
		for (i = 0; i < N; i++)
			data[i] = 0.3635819 - 0.4891775*cos(2.*M_PI*i/(N-1)) + 0.1365995*cos(4.*M_PI*i/(N-1)) - 0.0106411*cos(6.*M_PI*i/(N-1));
		break;
	case nsl_sf_window_blackman_harris:
		for (i = 0; i < N; i++)
			data[i] = 0.35875 - 0.48829*cos(2.*M_PI*i/(N-1)) + 0.14128*cos(4.*M_PI*i/(N-1)) - 0.01168*cos(6.*M_PI*i/(N-1));
		break;
	case nsl_sf_window_flat_top:
		for (i = 0; i < N; i++)
			data[i] = 1 - 1.93*cos(2.*M_PI*i/(N-1)) + 1.29*cos(4.*M_PI*i/(N-1)) - 0.388*cos(6.*M_PI*i/(N-1)) + 0.028*cos(8.*M_PI*i/(N-1));
		break;
	case nsl_sf_window_cosine:
		for (i = 0; i < N; i++)
			data[i] = sin(M_PI*i/(N-1));  
		break;
	case nsl_sf_window_bartlett_hann:
		for (i = 0; i < N; i++)
			data[i] = 0.62 - 0.48*fabs(i/(double)(N-1)-0.5) - 0.38*cos(2.*M_PI*i/(N-1));  
		break;
	case nsl_sf_window_lanczos:
		for (i = 0; i < N; i++)
			data[i] = gsl_sf_sinc(2.*i/(N-1)-1.);
		break;
	}

	return 0;
}
