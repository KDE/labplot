/***************************************************************************
    File                 : nsl_geom_linesim_morse_test.c
    Project              : LabPlot
    Description          : NSL line simplification morse test
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
#include <sys/time.h>
#include "nsl_geom_linesim.h"

#define FILENAME "morse_code.dat"
#define N 152000
#define NOUT 15200

int main() {
	FILE *file;
	if((file = fopen(FILENAME, "r")) == NULL) {
		printf("ERROR reading %s. Giving up.\n", FILENAME);
		return -1;
	}

	double *xdata, *ydata;
	size_t index[N], i;

	xdata = (double *)malloc(N*sizeof(double));
	ydata = (double *)malloc(N*sizeof(double));

	for(i=0; i<N; i++)
		fscanf(file,"%lf %lf", &xdata[i], &ydata[i]);

	printf("automatic tol clip_diag_perpoint = %g\n", nsl_geom_linesim_clip_diag_perpoint(xdata, ydata, N));
	printf("automatic tol clip_area_perpoint = %g\n", nsl_geom_linesim_clip_area_perpoint(xdata, ydata, N));
	printf("automatic tol avg_dist = %g\n", nsl_geom_linesim_avg_dist_perpoint(xdata, ydata, N));

	printf("* simplification (Douglas Peucker variant) nout = %zu\n", NOUT);

	struct timeval time1, time2;
	gettimeofday(&time1, NULL);
	double tolout = nsl_geom_linesim_douglas_peucker_variant(xdata, ydata, N, NOUT, index);
	gettimeofday(&time2, NULL);
	printf("run time : %llu ms\n", 1000 * (time2.tv_sec - time1.tv_sec) + (time2.tv_usec - time1.tv_usec) / 1000);

	printf("maxtol = %g (pos. error = %g, area error = %g)\n", tolout, nsl_geom_linesim_positional_squared_error(xdata, ydata, N, index), nsl_geom_linesim_area_error(xdata, ydata, N, index));

	free(xdata);
	free(ydata);

}
