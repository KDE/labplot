/***************************************************************************
    File                 : nsl_stats_test.c
    Project              : LabPlot
    Description          : NSL statistics functions
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

#include <stdio.h>
#include <gsl/gsl_statistics_double.h>
#include "nsl_stats.h"

int main() {
	const double data[]={1,1,1,3,4,7,9,11,13,13};
	const int size=10;

	double v0,v10,v20,v25,v30,v40,v50,med,v60,v70,v75,v80,v90,v100;
	int i;
	printf("Quantile sorted:\n");
	for(i=1;i<10;i++) {
		nsl_stats_quantile_type type = i;
		v0 = nsl_stats_quantile_sorted(data, 1, size, 0.0, type);
		v10 = nsl_stats_quantile_sorted(data, 1, size, 0.1, type);
		v20 = nsl_stats_quantile_sorted(data, 1, size, 0.2, type);
		v25 = nsl_stats_quantile_sorted(data, 1, size, 0.25, type);
		v30 = nsl_stats_quantile_sorted(data, 1, size, 0.3, type);
		v40 = nsl_stats_quantile_sorted(data, 1, size, 0.4, type);
		v50 = nsl_stats_quantile_sorted(data, 1, size, 0.5, type);
		med = nsl_stats_median_sorted(data, 1, size, type);
		v60 = nsl_stats_quantile_sorted(data, 1, size, 0.6, type);
		v70 = nsl_stats_quantile_sorted(data, 1, size, 0.7, type);
		v75 = nsl_stats_quantile_sorted(data, 1, size, 0.75, type);
		v80 = nsl_stats_quantile_sorted(data, 1, size, 0.8, type);
		v90 = nsl_stats_quantile_sorted(data, 1, size, 0.9, type);
		v100 = nsl_stats_quantile_sorted(data, 1, size, 1.0, type);

		printf("%d: %g %g %g %g %g %g %g |%g| %g %g %g %g %g %g\n", type, v0,v10,v20,v25,v30,v40,v50,med,v60,v70,v75,v80,v90,v100);
	}
	double data2[]={3,7,11,1,13,1,9,1,13,4};
	printf("Quantile unsorted:\n");
	for(i=1;i<10;i++) {
		nsl_stats_quantile_type type = i;
		v0 = nsl_stats_quantile(data2, 1, size, 0.0, type);
		v10 = nsl_stats_quantile(data2, 1, size, 0.1, type);
		v20 = nsl_stats_quantile(data2, 1, size, 0.2, type);
		v25 = nsl_stats_quantile(data2, 1, size, 0.25, type);
		v30 = nsl_stats_quantile(data2, 1, size, 0.3, type);
		v40 = nsl_stats_quantile(data2, 1, size, 0.4, type);
		v50 = nsl_stats_quantile(data2, 1, size, 0.5, type);
		med = nsl_stats_median(data2, 1, size, type);
		v60 = nsl_stats_quantile(data2, 1, size, 0.6, type);
		v70 = nsl_stats_quantile(data2, 1, size, 0.7, type);
		v75 = nsl_stats_quantile(data2, 1, size, 0.75, type);
		v80 = nsl_stats_quantile(data2, 1, size, 0.8, type);
		v90 = nsl_stats_quantile(data2, 1, size, 0.9, type);
		v100 = nsl_stats_quantile(data2, 1, size, 1.0, type);

		printf("%d: %g %g %g %g %g %g %g |%g| %g %g %g %g %g %g\n", type, v0,v10,v20,v25,v30,v40,v50,med,v60,v70,v75,v80,v90,v100);
	}

	v0 = gsl_stats_quantile_from_sorted_data(data, 1, size, 0.0);
	v10 = gsl_stats_quantile_from_sorted_data(data, 1, size, 0.1);
	v20 = gsl_stats_quantile_from_sorted_data(data, 1, size, 0.2);
	v25 = gsl_stats_quantile_from_sorted_data(data, 1, size, 0.25);
	v30 = gsl_stats_quantile_from_sorted_data(data, 1, size, 0.3);
	v40 = gsl_stats_quantile_from_sorted_data(data, 1, size, 0.4);
	v50 = gsl_stats_quantile_from_sorted_data(data, 1, size, 0.5);
	v60 = gsl_stats_quantile_from_sorted_data(data, 1, size, 0.6);
	v70 = gsl_stats_quantile_from_sorted_data(data, 1, size, 0.7);
	v75 = gsl_stats_quantile_from_sorted_data(data, 1, size, 0.75);
	v80 = gsl_stats_quantile_from_sorted_data(data, 1, size, 0.8);
	v90 = gsl_stats_quantile_from_sorted_data(data, 1, size, 0.9);
	v100 = gsl_stats_quantile_from_sorted_data(data, 1, size, 1.0);
	printf("\nGSL: %g %g %g %g %g %g %g %g %g %g %g %g %g\n", v0,v10,v20,v25,v30,v40,v50,v60,v70,v75,v80,v90,v100);
}
