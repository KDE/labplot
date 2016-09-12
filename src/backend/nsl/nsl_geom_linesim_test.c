/***************************************************************************
    File                 : nsl_geom_linesim_test.c
    Project              : LabPlot
    Description          : NSL line simplification test
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
#include "nsl_geom_linesim.h"

int main() {
	const double xdata[]={1,2,2.5,3,4,7,9,11,13,14};
	const double ydata[]={1,1,1,3,4,7,8,12,13,13};
	const size_t n=10;
	size_t index[n], i;

	printf("automatic tol clip_diag_perpoint = %g\n", nsl_geom_linesim_clip_diag_perpoint(xdata, ydata, n));
	printf("automatic tol clip_area_perpoint = %g\n", nsl_geom_linesim_clip_area_perpoint(xdata, ydata, n));
	printf("automatic tol avg_dist = %g\n", nsl_geom_linesim_avg_dist_perpoint(xdata, ydata, n));

	const double tol5=0.6;
	printf("* simplification (Douglas Peucker)\n");
	size_t nout = nsl_geom_linesim_douglas_peucker(xdata, ydata, n, tol5, index);
	printf("nout = %d (pos. error = %g, area error = %g)\n", nout, nsl_geom_linesim_positional_squared_error(xdata, ydata, n, index), nsl_geom_linesim_area_error(xdata, ydata, n, index));

	for(i=0; i<nout; i++)
		printf("%d: %d\n", i, index[i]);

	const size_t no=6;
	printf("* simplification (Douglas Peucker variant) nout = %zu\n", no);
	double tolout = nsl_geom_linesim_douglas_peucker_variant(xdata, ydata, n, no, index);
	printf("maxtol = %g (pos. error = %g, area error = %g)\n", tolout, nsl_geom_linesim_positional_squared_error(xdata, ydata, n, index), nsl_geom_linesim_area_error(xdata, ydata, n, index));

	for(i=0; i<no; i++)
		printf("%d: %d\n", i, index[i]);

	const size_t np=2;
	printf("* n-th point\n");
	nout = nsl_geom_linesim_nthpoint(n, np, index);
	printf("nout = %d (pos. error = %g, area error = %g)\n", nout, nsl_geom_linesim_positional_squared_error(xdata, ydata, n, index), nsl_geom_linesim_area_error(xdata, ydata, n, index));

	for(i=0; i<nout; i++)
		printf("%d: %d\n", i, index[i]);

	const double tol=1.5;
	printf("* radial distance\n");
	nout = nsl_geom_linesim_raddist(xdata, ydata, n, tol, index);
	printf("nout = %d (pos. error = %g, area error = %g)\n", nout, nsl_geom_linesim_positional_squared_error(xdata, ydata, n, index), nsl_geom_linesim_area_error(xdata, ydata, n, index));

	for(i=0; i<nout; i++)
		printf("%d: %d\n", i, index[i]);

	const double tol2=0.5;
	const size_t repeat = 3;
	printf("* perpendicular distance (repeat = %d)\n", repeat);
	nout = nsl_geom_linesim_perpdist_repeat(xdata, ydata, n, tol2, repeat, index);
	printf("nout = %d (pos. error = %g, area error = %g)\n", nout, nsl_geom_linesim_positional_squared_error(xdata, ydata, n, index), nsl_geom_linesim_area_error(xdata, ydata, n, index));

	for(i=0; i<nout; i++)
		printf("%d: %d\n", i, index[i]);

	const double tol6=0.7;
	printf("* y distance (interpolation)\n", repeat);
	nout = nsl_geom_linesim_interp(xdata, ydata, n, tol6, index);
	printf("nout = %d (pos. error = %g, area error = %g)\n", nout, nsl_geom_linesim_positional_squared_error(xdata, ydata, n, index), nsl_geom_linesim_area_error(xdata, ydata, n, index));

	for(i=0; i<nout; i++)
		printf("%d: %d\n", i, index[i]);

	const double tol7=1.6;
	printf("* minimum area (Visvalingam-Whyatt)\n", repeat);
	nout = nsl_geom_linesim_visvalingam_whyatt(xdata, ydata, n, tol7, index);
	printf("nout = %d (pos. error = %g, area error = %g)\n", nout, nsl_geom_linesim_positional_squared_error(xdata, ydata, n, index), nsl_geom_linesim_area_error(xdata, ydata, n, index));

	for(i=0; i<nout; i++)
		printf("%d: %d\n", i, index[i]);
	return 0;

	const double tol3=0.7;
	printf("* perp. distance (Reumann-Witkam)\n");
	nout = nsl_geom_linesim_reumann_witkam(xdata, ydata, n, tol3, index);
	printf("nout = %d (pos. error = %g, area error = %g)\n", nout, nsl_geom_linesim_positional_squared_error(xdata, ydata, n, index), nsl_geom_linesim_area_error(xdata, ydata, n, index));

	for(i=0; i<nout; i++)
		printf("%d: %d\n", i, index[i]);

	const double mintol=2.0;
	const double maxtol=7.0;
	printf("* perp. distance (Opheim)\n");
	nout = nsl_geom_linesim_opheim(xdata, ydata, n, mintol, maxtol, index);
	printf("nout = %d (pos. error = %g, area error = %g)\n", nout, nsl_geom_linesim_positional_squared_error(xdata, ydata, n, index), nsl_geom_linesim_area_error(xdata, ydata, n, index));

	for(i=0; i<nout; i++)
		printf("%d: %d\n", i, index[i]);

	const double tol4=0.5;
	const size_t region=5;
	printf("* simplification (Lang)\n");
	nout = nsl_geom_linesim_lang(xdata, ydata, n, tol4, region, index);
	printf("nout = %d (pos. error = %g, area error = %g)\n", nout, nsl_geom_linesim_positional_squared_error(xdata, ydata, n, index), nsl_geom_linesim_area_error(xdata, ydata, n, index));

	for(i=0; i<nout; i++)
		printf("%d: %d\n", i, index[i]);

}
