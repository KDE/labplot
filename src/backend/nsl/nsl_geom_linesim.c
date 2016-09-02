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
#include <stdlib.h>
#include <float.h>
#include "nsl_sort.h"
#include "nsl_stats.h"
#include "nsl_geom.h"
#include "nsl_geom_linesim.h"

const char* nsl_geom_linesim_type_name[] = {"Douglas-Peucker", "n-th point", "radial distance", "perpendicular distance", 
	"Interpolation", "Visvalingam-Whyatt", "Reumann-Witkam", "Opheim", "Lang"};

/*********** error calculation functions *********/

double nsl_geom_linesim_positional_error(const double xdata[], const double ydata[], const size_t n, const size_t index[]) {
	double dist=0;
	size_t i=0, j;	/* i: index of index[] */
	do {
		/*for every point not in index[] calculate distance to line*/
		/*printf("i=%d (index[i]-index[i+1]=%d-%d)\n", i , index[i], index[i+1]);*/
		for(j=1;j<index[i+1]-index[i];j++) {
			/*printf("i=%d: j=%d\n", i, j);*/
			dist += nsl_geom_point_line_dist(xdata[index[i]], ydata[index[i]], xdata[index[i+1]], ydata[index[i+1]], xdata[index[i]+j], ydata[index[i]+j]);
			/*printf("dist = %g\n", dist);*/
		}
		i++;
	} while(index[i] != n-1);
	
	return dist/(double)n;
}

double nsl_geom_linesim_positional_squared_error(const double xdata[], const double ydata[], const size_t n, const size_t index[]) {
	double dist=0;
	size_t i=0, j;	/* i: index of index[] */
	do {
		/*for every point not in index[] calculate distance to line*/
		/*printf("i=%d (index[i]-index[i+1]=%d-%d)\n", i , index[i], index[i+1]);*/
		for(j=1;j<index[i+1]-index[i];j++) {
			/*printf("i=%d: j=%d\n", i, j);*/
			dist += pow(nsl_geom_point_line_dist(xdata[index[i]], ydata[index[i]], xdata[index[i+1]], ydata[index[i+1]], xdata[index[i]+j], ydata[index[i]+j]), 2);
			/*printf("dist = %g\n", dist);*/
		}
		i++;
	} while(index[i] != n-1);
	
	return dist/(double)n;
}

double nsl_geom_linesim_area_error(const double xdata[], const double ydata[], const size_t n, const size_t index[]) {
	double area=0;

	size_t i=0, j;	/* i: index of index[] */
	do {
		/* for every point not in index[] calculate area */
		/*printf("i=%d (index[i]-index[i+1]=%d-%d)\n", i , index[i], index[i+1]);*/
		for(j=1;j<index[i+1]-index[i];j++) {
			/*printf("j=%d: area between %d %d %d\n", j, index[i]+j-1, index[i]+j, index[i+1]);*/
			area += nsl_geom_three_point_area(xdata[index[i]+j-1], ydata[index[i]+j-1], xdata[index[i]+j], ydata[index[i]+j], xdata[index[i+1]], ydata[index[i+1]]);
			/*printf("area = %g\n", area);*/
		}
		i++;
	} while(index[i] != n-1);
	

	return area/(double)n;
}

double nsl_geom_linesim_eps(const double xdata[], const double ydata[], const size_t n) {
	double dx = nsl_stats_maximum(xdata, n, NULL) - nsl_stats_minimum(xdata, n, NULL);
	double dy = nsl_stats_maximum(ydata, n, NULL) - nsl_stats_minimum(ydata, n, NULL);
	double d = sqrt(dx*dx+dy*dy);
	double eps = d/1000.0;	/* "small" */
	
	return eps;
}

/*********** simplification algorithms *********/

void nsl_geom_linesim_douglas_peucker_step(const double xdata[], const double ydata[], const size_t start, const size_t end, size_t *nout, const double eps, size_t index[]) {
	/*printf("DP: %d - %d\n", start, end);*/

	size_t i, nkey=start;
	double dist, maxdist=0;
	/* search for key (biggest perp. distance) */
	for(i=start+1; i < end; i++) {
		dist = nsl_geom_point_line_dist(xdata[start], ydata[start], xdata[end], ydata[end], xdata[i], ydata[i]);
		(dist > maxdist) ? maxdist=dist, nkey=i : 0;
	}
	/*printf("maxdist = %g @ i = %d\n", maxdist, nkey);*/

	if(maxdist > eps) {
		/*printf("take %d\n", nkey);*/
		index[(*nout)++]=nkey;
		if(nkey-start > 1)
			nsl_geom_linesim_douglas_peucker_step(xdata, ydata, start, nkey, nout, eps, index);
		if(end-nkey > 1)
			nsl_geom_linesim_douglas_peucker_step(xdata, ydata, nkey, end, nout, eps, index);
	}

	/*printf("nout=%d\n", *nout);*/
}

size_t nsl_geom_linesim_douglas_peucker(const double xdata[], const double ydata[], const size_t n, const double eps, size_t index[]) {
	size_t nout=0;

	/*first point*/
	index[nout++] = 0;

	nsl_geom_linesim_douglas_peucker_step(xdata, ydata, 0, n-1, &nout, eps, index);

	/* last point */
	if (index[nout-1] != n-1)
		index[nout++] = n-1;

	/* sort array index */
	nsl_sort_size_t(index, nout);

	return nout;
}
size_t nsl_geom_linesim_douglas_peucker_auto(const double xdata[], const double ydata[], const size_t n, size_t index[]) {
	double eps = nsl_geom_linesim_eps(xdata, ydata, n);
	return  nsl_geom_linesim_douglas_peucker(xdata, ydata, n, eps, index);
}

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
size_t nsl_geom_linesim_raddist_auto(const double xdata[], const double ydata[], const size_t n, size_t index[]) {
	double eps = nsl_geom_linesim_eps(xdata, ydata, n);
	return  nsl_geom_linesim_raddist(xdata, ydata, n, eps, index);
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
size_t nsl_geom_linesim_perpdist_auto(const double xdata[], const double ydata[], const size_t n, size_t index[]) {
	double eps = nsl_geom_linesim_eps(xdata, ydata, n);
	return  nsl_geom_linesim_perpdist(xdata, ydata, n, eps, index);
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

size_t nsl_geom_linesim_interp(const double xdata[], const double ydata[], const size_t n, const double eps, size_t index[]) {
	size_t nout=0, i;

	/*first  point*/
	index[nout++] = 0;

	size_t key=0;
	for(i=1; i < n-1; i++) {
		/*printf("%d: %d-%d\n", i, key, i+1);*/
		double dist = nsl_geom_point_line_dist_y(xdata[key], ydata[key], xdata[i+1], ydata[i+1], xdata[i], ydata[i]);
		/*printf("%d: dist = %g\n", i, dist);*/
		if(dist > eps) {
			/*printf("take it %d\n", i);*/
			index[nout++] = key = i;
		}
	}

	/* last point */
	index[nout++] = n-1;

	return nout;
}
size_t nsl_geom_linesim_interp_auto(const double xdata[], const double ydata[], const size_t n, size_t index[]) {
	double eps = nsl_geom_linesim_eps(xdata, ydata, n);
	return  nsl_geom_linesim_interp(xdata, ydata, n, eps, index);
}

size_t nsl_geom_linesim_visvalingam_whyatt(const double xdata[], const double ydata[], const size_t n, const double eps, size_t index[]) {
	size_t i, nout=n;

	double *area = (double *) malloc((n-2)*sizeof(double));	/* area associated with every point */
	for(i=1;i<n-1;i++) {
		area[i-1] = nsl_geom_three_point_area(xdata[i-1], ydata[i-1], xdata[i], ydata[i], xdata[i+1], ydata[i+1]);
		index[i] = i;
	}
	index[n-1]=n-1;

	double minarea;
	size_t minindex;
	while ( (minarea = nsl_stats_minimum(area, n-2, &minindex)) < eps) {

		/* remove point minindex */
		/*printf("removing point %d (minarea = %g) \n", minindex+1, minarea);*/
		index[minindex+1] = 0;
		area[minindex] = DBL_MAX;
		double tmparea;
		if(minindex>0) {
			tmparea = nsl_geom_three_point_area(xdata[minindex-1], ydata[minindex-1], xdata[minindex], ydata[minindex], xdata[minindex+1], ydata[minindex+1]);
			if(tmparea > area[minindex-1])	/* take largest value new and old area */
				area[minindex-1] = tmparea;
		}
		if(minindex<n-2) {
			tmparea = nsl_geom_three_point_area(xdata[minindex], ydata[minindex], xdata[minindex+1], ydata[minindex+1], xdata[minindex+2], ydata[minindex+2]);
			if(tmparea > area[minindex+1])	/* take largest value new and old area */
				area[minindex+1] = tmparea;

		}
		nout--;
	};

	/*for(i=0;i<n;i++)
		printf("INDEX = %d\n", index[i]);
	*/
	/* condens index */
	i=1;
	size_t newi=1;
	while (newi < n-1) {
		while (index[newi] == 0)
			newi++;
		index[i++] = index[newi++];
	};

	/*for(i=0;i<nout;i++)
		printf("INDEX2 = %d\n", index[i]);
	*/

	free(area);
	return nout;
}
size_t nsl_geom_linesim_visvalingam_whyatt_auto(const double xdata[], const double ydata[], const size_t n, size_t index[]) {
	/* TODO: calculate area eps */
	/*double eps = nsl_geom_linesim_eps(xdata, ydata, n);
	return  nsl_geom_linesim_visvalingam_whyatt(xdata, ydata, n, eps, index);
	*/
	printf("nsl_geom_linesim_visvalingam_whyatt_auto(): Not implemented yet\n");
	return 0;
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
size_t nsl_geom_linesim_reumann_witkam_auto(const double xdata[], const double ydata[], const size_t n, size_t index[]) {
	double eps = nsl_geom_linesim_eps(xdata, ydata, n);
	return  nsl_geom_linesim_reumann_witkam(xdata, ydata, n, eps, index);
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
size_t nsl_geom_linesim_opheim_auto(const double xdata[], const double ydata[], const size_t n, size_t index[]) {
	/* TODO: calculate min and max tolerance */
	/*double eps = nsl_geom_linesim_eps(xdata, ydata, n);
	return  nsl_geom_linesim_visvalingam_whyatt(xdata, ydata, n, eps, index);
	*/
	printf("nsl_geom_linesim_opheim_auto(): Not implemented yet\n");
	return 0;
}

size_t nsl_geom_linesim_lang(const double xdata[], const double ydata[], const size_t n, const double eps, const size_t region, size_t index[]) {
	size_t nout=0, key=0, i, j;

	/*first  point*/
	index[nout++] = 0;

	double dist, maxdist;
	for(i=1; i<n-1; i++) {
		size_t tmpregion=region;
		if(key+tmpregion > n-1)	/* end of data set */
			tmpregion = n-1-key;

		do {
			maxdist=0;
			for (j=1; j < tmpregion; j++) {
				dist = nsl_geom_point_line_dist(xdata[key], ydata[key], xdata[key+tmpregion], ydata[key+tmpregion], xdata[key+j], ydata[key+j]);
				/*printf("%d: dist (%d to %d-%d) = %g\n", j, key+j, key, key+tmpregion, dist);*/
				if(dist > maxdist)
					maxdist = dist;
			}
			/*printf("eps = %g maxdist = %g\n", eps, maxdist);*/
			tmpregion--;
			/*printf("region = %d\n", tmpregion);*/
		} while (maxdist>eps && tmpregion>0);
		i += tmpregion;
		index[nout++] = key = i;
		/*printf("take it (%d) key=%d\n", i, key);*/
	}

	/* last point */
	if (index[nout-1] != n-1)
		index[nout++] = n-1;

	return nout;
}
size_t nsl_geom_linesim_lang_auto(const double xdata[], const double ydata[], const size_t n, size_t index[]) {
	/* TODO: calculate  region */
	double eps = nsl_geom_linesim_eps(xdata, ydata, n);
	/*return  nsl_geom_linesim_visvalingam_whyatt(xdata, ydata, n, eps, index);
	*/
	printf("nsl_geom_linesim_lang_auto(): Not implemented yet\n");
	return 0;
}

