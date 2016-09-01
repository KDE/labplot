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

double main() {
	const double xdata[]={1,2,2.5,3,4,7,9,11,13,14};
	const double ydata[]={1,1,1,3,4,7,8,12,13,13};
	const size_t n=10;
	size_t index[n], i;

	const double eps5=0.6;
	printf("* simplification (Douglas Peucker)\n");
	size_t nout = nsl_geom_linesim_douglas_peucker(xdata, ydata, n, eps5, index);
	printf("nout = %d (error = %g)\n", nout, nsl_geom_linesim_positional_squared_error(xdata, ydata, n, index));

	for(i=0; i<nout; i++)
		printf("%d: %d\n", i, index[i]);

	const size_t np=2;
	printf("* n-th point\n");
	nout = nsl_geom_linesim_nthpoint(n, np, index);
	printf("nout = %d (error = %g)\n", nout, nsl_geom_linesim_positional_squared_error(xdata, ydata, n, index));

	for(i=0; i<nout; i++)
		printf("%d: %d\n", i, index[i]);

	const double eps=1.5;
	printf("* radial distance\n");
	nout = nsl_geom_linesim_raddist(xdata, ydata, n, eps, index);
	printf("nout = %d (error = %g)\n", nout, nsl_geom_linesim_positional_squared_error(xdata, ydata, n, index));

	for(i=0; i<nout; i++)
		printf("%d: %d\n", i, index[i]);

	const double eps2=0.5;
	const size_t repeat = 3;
	printf("* perpendicular distance (repeat = %d)\n", repeat);
	nout = nsl_geom_linesim_perpdist_repeat(xdata, ydata, n, eps2, repeat, index);
	printf("nout = %d (error = %g)\n", nout, nsl_geom_linesim_positional_squared_error(xdata, ydata, n, index));

	for(i=0; i<nout; i++)
		printf("%d: %d\n", i, index[i]);

	const double eps6=0.7;
	printf("* y distance (interpolation)\n", repeat);
	nout = nsl_geom_linesim_interp(xdata, ydata, n, eps6, index);
	printf("nout = %d (error = %g)\n", nout, nsl_geom_linesim_positional_squared_error(xdata, ydata, n, index));

	for(i=0; i<nout; i++)
		printf("%d: %d\n", i, index[i]);

	const double eps3=0.7;
	printf("* perp. distance (Reumann-Witkam)\n");
	nout = nsl_geom_linesim_reumann_witkam(xdata, ydata, n, eps3, index);
	printf("nout = %d (error = %g)\n", nout, nsl_geom_linesim_positional_squared_error(xdata, ydata, n, index));

	for(i=0; i<nout; i++)
		printf("%d: %d\n", i, index[i]);

	const double mineps=2.0;
	const double maxeps=7.0;
	printf("* perp. distance (Opheim)\n");
	nout = nsl_geom_linesim_opheim(xdata, ydata, n, mineps, maxeps, index);
	printf("nout = %d (error = %g)\n", nout, nsl_geom_linesim_positional_squared_error(xdata, ydata, n, index));

	for(i=0; i<nout; i++)
		printf("%d: %d\n", i, index[i]);

	const double eps4=0.5;
	const size_t region=5;
	printf("* simplification (Lang)\n");
	nout = nsl_geom_linesim_lang(xdata, ydata, n, eps4, region, index);
	printf("nout = %d (error = %g)\n", nout, nsl_geom_linesim_positional_squared_error(xdata, ydata, n, index));

	for(i=0; i<nout; i++)
		printf("%d: %d\n", i, index[i]);

}
