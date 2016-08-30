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
#include <stdio.h>
#include "nsl_geom.h"
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
		double dist = nsl_geom_point_point_dist(xdata[i], ydata[i], xdata[key], ydata[key]);
		/* distance to last point */
		double lastdist = nsl_geom_point_point_dist(xdata[i], ydata[i], xdata[n-1], ydata[n-1]);
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

size_t nsl_geom_linesim_perpdist(const double xdata[], const double ydata[], const size_t n, const double eps, size_t index[]) {
	size_t nout=0, key=0, i;

	/*first point*/
	index[nout++] = 0;

	for(i=1; i < n-1; i++) {
		/* distance of point i to line key -- i+1 */
		double dist = nsl_geom_point_line_dist(xdata[key], ydata[key], xdata[i+1], ydata[i+1], xdata[i], ydata[i]);
		/*printf("%d: %g\n", i, dist);*/

		if(dist > eps) {	/* take it */
			index[nout++] = i;
			key = i;
			/*printf("%d: take it (key = %d)\n", i, key);*/
		} else {	/* ignore it */
			if(i+1 < n-1)	/* last point is taken anyway */
				index[nout++] = i+1; /* take next point in any case */
			/*printf("%d: ignore it (key = %d)\n", i, i+1);*/
			key = ++i;
		}
	}

	/* last point */
	index[nout++] = n-1;

	return nout;
}

size_t nsl_geom_linesim_perpdist_repeat(const double xdata[], const double ydata[], const size_t n, const double eps, const size_t repeat, size_t index[]) {
	size_t i, j, nout;
	double *xtmp = (double *) malloc(n*sizeof(double));
	double *ytmp = (double *) malloc(n*sizeof(double));
	size_t *tmpindex = (size_t *) malloc(n*sizeof(size_t));

	/* first round */
	nout = nsl_geom_linesim_perpdist(xdata, ydata, n, eps, index);
	/* repeats */
	for(i=0; i < repeat - 1; i++) {
		for(j=0; j < nout; j++) {
			xtmp[j] = xdata[index[j]];
			ytmp[j] = ydata[index[j]];
			tmpindex[j]= index[j];
			/*printf("%g %g\n", xtmp[j], ytmp[j]);*/
		}
		size_t tmpnout = nsl_geom_linesim_perpdist(xtmp, ytmp, nout, eps, tmpindex);
		for(j=0; j < tmpnout; j++) {
			index[j] = index[tmpindex[j]];
			/*printf("tmpindex[%d]: %d\n", j, tmpindex[j]);*/
		}

		if(tmpnout == nout)	/* return if nout does not change anymore */
			break;
		else
			nout = tmpnout;
	}

	free(tmpindex);
	free(xtmp);
	free(ytmp);

	return nout;
}

size_t nsl_geom_linesim_reumann_witkam(const double xdata[], const double ydata[], const size_t n, const double eps, size_t index[]) {
	size_t nout=0, key=0, key2=1, i;

	/*first  point*/
	index[nout++] = 0;

	for(i=2; i < n-1; i++) {
		/* distance to line key -- key2 */
		double dist = nsl_geom_point_line_dist(xdata[key], ydata[key], xdata[key2], ydata[key2], xdata[i], ydata[i]);
		/*printf("%d: %g\n", i, dist);*/

		if(dist > eps) {	/* take it */
			/*printf("%d: take it\n", i);*/
			key = i-1;
			key2 = i;
			index[nout++] = i-1;
		}
	}

	/* last point */
	index[nout++] = n-1;

	return nout;
}

size_t nsl_geom_linesim_opheim(const double xdata[], const double ydata[], const size_t n, const double mineps, const double maxeps, size_t index[]) {
	size_t nout=0, key=0, key2, i;

	/*first  point*/
	index[nout++] = 0;

	for(i=1; i < n-1; i++) {
		double dist, perpdist;
		do {	/* find key2 */
			dist = nsl_geom_point_point_dist(xdata[key], ydata[key], xdata[i], ydata[i]);
			/*printf("dist = %g: %d-%d\n", dist, key, i);*/
			i++;
		} while (dist < mineps);
		i--;
		if(key == i-1)		/*i+1 outside mineps */
			key2 = i;
		else
			key2 = i-1;	/* last point inside */
		/*printf("found key2 @%d\n", key2);*/

		do {	/* find next key */
			dist = nsl_geom_point_point_dist(xdata[key], ydata[key], xdata[i], ydata[i]);
			perpdist = nsl_geom_point_line_dist(xdata[key], ydata[key], xdata[key2], ydata[key2], xdata[i], ydata[i]);
			/*printf("dist = %g, perpdist=%g: %d\n", dist, perpdist, i);*/
			i++;
		} while (dist < maxeps && perpdist < mineps);
		i--;
		if(key == i-1)		/*i+1 outside */
			key = i;
		else {
			key = i-1;
			i--;
		}
		/*printf("new key: %d\n", key);*/
		index[nout++] = key;
	}

	/* last point */
	if (index[nout-1] != n-1)
		index[nout++] = n-1;

	return nout;
}
