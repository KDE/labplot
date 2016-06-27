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
#include "nsl_sf_window.h"

const char* nsl_sf_window_type_name[] = {"rectangular (uniform)", "triangular", "Welch (parabolic)" };

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
		v = 1.0 - 2./(N-1)*fabs(i-(N-1)/2.);
		break;
	case nsl_sf_window_welch:
		v= 1.0 - gsl_pow_2(2*(i-(N-1)/2.)/(N+1));
	}

	return v;
}
