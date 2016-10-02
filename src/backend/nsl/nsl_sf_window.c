/***************************************************************************
    File                 : nsl_sf_window.c
    Project              : LabPlot
    Description          : NSL special window functions
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
#include <gsl/gsl_math.h>
#include <gsl/gsl_sf_trig.h>
#include "nsl_sf_window.h"

#define i18n(m) m

const char* nsl_sf_window_type_name[] = {i18n("rectangular (uniform)"), i18n("triangular"), i18n("triangular II (Bartlett)"), i18n("triangular III (Parzen)") ,
	i18n("Welch (parabolic)"), i18n("Hann (raised cosine)"), i18n("Hamming"), i18n("Blackman"), i18n("Nuttall"), i18n("Blackman-Nuttall"), i18n("Blackman-Harris"), i18n("Flat top"),
	i18n("Cosine"), i18n("Bartlett-Hann"), i18n("Lanczos")};

double nsl_sf_window(int i, int N, nsl_sf_window_type type) {
	double v=0.0;

	switch (type) {
	case nsl_sf_window_uniform:
		if (i >= 0 && i < N)
			v = 1.0;
		else
			v = 0.0;
		break;
	case nsl_sf_window_triangle:
		v = 1.0 - 2./N*fabs(i-(N-1)/2.);
		break;
	case nsl_sf_window_triangleII:
		v = 1.0 - 2./(N-1)*fabs(i-(N-1)/2.);
		break;
	case nsl_sf_window_triangleIII:
		v = 1.0 - 2./(N+1)*fabs(i-(N-1)/2.);
		break;
	case nsl_sf_window_welch:
		v = 1.0 - gsl_pow_2(2*(i-(N-1)/2.)/(N+1));
		break;
	case nsl_sf_window_hann:
		v = 0.5*(1. - cos(2.*M_PI*i/(N-1)));
		break;
	case nsl_sf_window_hamming:
		v = 0.54 - 0.46*cos(2.*M_PI*i/(N-1));
		break;
	case nsl_sf_window_blackman:
		v = 0.42 - 0.5*cos(2.*M_PI*i/(N-1)) + 0.08*cos(4.*M_PI*i/(N-1));
		break;
	case nsl_sf_window_nuttall:
		v = 0.355768 - 0.487396*cos(2.*M_PI*i/(N-1)) + 0.144232*cos(4.*M_PI*i/(N-1)) - 0.012604*cos(6.*M_PI*i/(N-1));
		break;
	case nsl_sf_window_blackman_nuttall:
		v = 0.3635819 - 0.4891775*cos(2.*M_PI*i/(N-1)) + 0.1365995*cos(4.*M_PI*i/(N-1)) - 0.0106411*cos(6.*M_PI*i/(N-1));
		break;
	case nsl_sf_window_blackman_harris:
		v = 0.35875 - 0.48829*cos(2.*M_PI*i/(N-1)) + 0.14128*cos(4.*M_PI*i/(N-1)) - 0.01168*cos(6.*M_PI*i/(N-1));
		break;
	case nsl_sf_window_flat_top:
		v = 1 - 1.93*cos(2.*M_PI*i/(N-1)) + 1.29*cos(4.*M_PI*i/(N-1)) - 0.388*cos(6.*M_PI*i/(N-1)) + 0.028*cos(8.*M_PI*i/(N-1));
		break;
	case nsl_sf_window_cosine:
		v = sin(M_PI*i/(N-1));  
		break;
	case nsl_sf_window_bartlett_hann:
		v = 0.62 - 0.48*fabs(i/(double)(N-1)-0.5) - 0.38*cos(2.*M_PI*i/(N-1));  
		break;
	case nsl_sf_window_lanczos:
		v = gsl_sf_sinc(2.*i/(N-1)-1.);
		break;
	}

	return v;
}
