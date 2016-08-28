/***************************************************************************
    File                 : nsl_geom_linesim.c
    Project              : LabPlot
    Description          : NSL geometry line simplification functions
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
/*#include <stdio.h>*/
#include "nsl_geom_linesim.h"

size_t nsl_geom_linesim_nthpoint(const size_t n, const size_t step, size_t index[]) {
	size_t nout=0, i;

	/*first point*/
	index[nout++] = 0;
	
	for(i=1; i < n-1; i++)
		if(i%step == 0)
			index[nout++] = i;

	/* last point */
	index[nout++] = n-1;

	return nout;
}

size_t nsl_geom_linesim_raddist(const double xdata[], const double ydata[], const size_t n, const double eps, size_t index[]) {
	size_t nout=0, key=0, i;

	/*first point*/
	index[nout++] = 0;

	for(i=1; i < n-1; i++) {
		/* distance to key point */
		double dist = sqrt((xdata[key]-xdata[i])*(xdata[key]-xdata[i]) + (ydata[key]-ydata[i])*(ydata[key]-ydata[i]) );
		/* distance to last point */
		double lastdist = sqrt((xdata[n-1]-xdata[i])*(xdata[n-1]-xdata[i]) + (ydata[n-1]-ydata[i])*(ydata[n-1]-ydata[i]) );
		/*printf("%d: %g %g\n", i, dist, lastdist);*/

		if(dist > eps && lastdist > eps) {
			index[nout++] = i;
			key = i;
		}
	}

	/* last point */
	index[nout++] = n-1;

	return nout;
}
