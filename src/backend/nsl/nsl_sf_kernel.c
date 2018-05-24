/***************************************************************************
    File                 : nsl_sf_kernel.c
    Project              : LabPlot
    Description          : NSL special kernel functions
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

#include "nsl_sf_kernel.h"
#include <stdlib.h>
#include <math.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_randist.h>	/* Gaussian and Cauchy */

/* kernel on [-1,1] */

double nsl_sf_kernel_uniform(double u) {
	if(fabs(u) <= 1.0)
		return 0.5;
	return 0.0;
}

double nsl_sf_kernel_triangular(double u) {
	if(fabs(u) <= 1.0)
		return 1.0-fabs(u);
	return 0.0;
}

double nsl_sf_kernel_parabolic(double u) {
	if(fabs(u) <= 1.0)
		return 3./4.*(1.0-gsl_pow_2(u));
	return 0.0;
}

double nsl_sf_kernel_quartic(double u) {
	if(fabs(u) <= 1.0)
		return 15./16.*gsl_pow_2(1.0-gsl_pow_2(u));
	return 0.0;
}

double nsl_sf_kernel_triweight(double u) {
	if(fabs(u) <= 1.0)
		return 35./32.*gsl_pow_3(1.0-gsl_pow_2(u));
	return 0.0;
}

double nsl_sf_kernel_tricube(double u) {
	if(fabs(u) <= 1.0)
		return 70./81.*gsl_pow_3(1.0-gsl_pow_3(fabs(u)));
	return 0.0;
}

double nsl_sf_kernel_cosine(double u) {
	if(fabs(u) <= 1.0)
		return M_PI/4.*cos(M_PI/2.*u);
	return 0.0;
}

double nsl_sf_kernel_semicircle(double u) {
	if(fabs(u) < 1.0)
		return 2./M_PI*sqrt(1-gsl_pow_2(u));
	return 0.0;
}

/* kernel on (-inf,inf) */
double nsl_sf_kernel_gaussian(double u) {
	return gsl_ran_gaussian_pdf(u, 1.0);
}

double nsl_sf_kernel_cauchy(double u) {
	return gsl_ran_cauchy_pdf(u, 1.0);
}

double nsl_sf_kernel_logistic(double u) {
	return gsl_ran_logistic_pdf(u, 1.0);
}

double nsl_sf_kernel_picard(double u) {
	return 0.5*exp(-fabs(u));
}

double nsl_sf_kernel_sigmoid(double u) {
	return 1.0/M_PI/cosh(u);
}

double nsl_sf_kernel_silverman(double u) {
	return 1.0/2.0*exp(-fabs(u)/M_SQRT2)*sin(fabs(u)/M_SQRT2+M_PI/4.0);
}
