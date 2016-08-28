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
	const double ydata[]={1,1,1,3,4,7,9,11,13,13};
	const size_t n=10, np=2;
	size_t index[n];

	size_t nout = nsl_geom_linesim_nthpoint(n, np, index);

	printf("* n-th point\nnout = %d\n", nout);
	int i;
	for(i=0; i<nout; i++)
		printf("%d: %d\n", i, index[i]);

	const double eps=1.5;
	nout = nsl_geom_linesim_raddist(xdata, ydata, n, eps, index);

	printf("* radial distance\nnout = %d\n", nout);
	for(i=0; i<nout; i++)
		printf("%d: %d\n", i, index[i]);
}
