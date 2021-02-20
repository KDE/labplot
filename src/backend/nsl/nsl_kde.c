/***************************************************************************
    File                 : nsl_kde.c
    Project              : LabPlot
    Description          : NSL functions for the kernel density estimation
    --------------------------------------------------------------------
    Copyright            : (C) 2021 by Alexander Semke (alexander.semke@web.de)

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

#include <gsl/gsl_statistics.h>
#include <gsl/gsl_sort.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_math.h>

double nsl_kde_gauss_kernel(double x) {
	return exp(-(gsl_pow_2(x)/2)) / (M_SQRT2*sqrt(M_PI));
}

double nsl_kde(double* data, double x, double h, size_t n) {
	double density = 0;
	for (size_t i=0; i < n; i++)
		density += gsl_ran_gaussian_pdf((data[i] - x)/h, 1.) / (n * h);

	return density;
}

double nsl_kde_normal_dist_bandwith(double* data, int n) {
	gsl_sort(data, 1, n);
	double sigma = gsl_stats_sd(data, 1, n);
	double iqr = gsl_stats_quantile_from_sorted_data(data, 1, n, 0.75) -
				gsl_stats_quantile_from_sorted_data(data, 1, n, 0.25);

	return 0.9 * GSL_MIN(sigma, iqr/1.34) * pow(n, -0.2);
}


