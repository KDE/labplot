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

#include "nsl_geom_linesim.h"
#include "nsl_geom.h"
#include "nsl_common.h"
#include "nsl_sort.h"
#include "nsl_stats.h"

const char* nsl_geom_linesim_type_name[] = {i18n("Douglas-Peucker (number)"), i18n("Douglas-Peucker (tolerance)"), i18n("Visvalingam-Whyatt"), i18n("Reumann-Witkam"), i18n("perpendicular distance"), i18n("n-th point"),
	i18n("radial distance"), i18n("Interpolation"), i18n("Opheim"), i18n("Lang")};

/*********** error calculation functions *********/

double nsl_geom_linesim_positional_error(const double xdata[], const double ydata[], const size_t n, const size_t index[]) {
	double dist = 0;
	size_t i = 0, j;	/* i: index of index[] */
	do {
		/*for every point not in index[] calculate distance to line*/
		/*printf("i=%d (index[i]-index[i+1]=%d-%d)\n", i , index[i], index[i+1]);*/
		for (j = 1; j < index[i+1]-index[i]; j++) {
			/*printf("i=%d: j=%d\n", i, j);*/
			dist += nsl_geom_point_line_dist(xdata[index[i]], ydata[index[i]], xdata[index[i+1]], ydata[index[i+1]], xdata[index[i]+j], ydata[index[i]+j]);
			/*printf("dist = %g\n", dist);*/
		}
		i++;
	} while(index[i] != n-1);

	return dist/(double)n;
}

double nsl_geom_linesim_positional_squared_error(const double xdata[], const double ydata[], const size_t n, const size_t index[]) {
	double dist = 0;
	size_t i = 0, j;	/* i: index of index[] */
	do {
		/*for every point not in index[] calculate distance to line*/
		/*printf("i=%d (index[i]-index[i+1]=%d-%d)\n", i , index[i], index[i+1]);*/
		for (j = 1; j < index[i+1]-index[i]; j++) {
			/*printf("i=%d: j=%d\n", i, j);*/
			dist += pow(nsl_geom_point_line_dist(xdata[index[i]], ydata[index[i]], xdata[index[i+1]], ydata[index[i+1]], xdata[index[i]+j], ydata[index[i]+j]), 2);
			/*printf("dist = %g\n", dist);*/
		}
		i++;
	} while(index[i] != n-1);

	return dist/(double)n;
}

double nsl_geom_linesim_area_error(const double xdata[], const double ydata[], const size_t n, const size_t index[]) {
	double area = 0;

	size_t i = 0, j;	/* i: index of index[] */
	do {
		/* for every point not in index[] calculate area */
		/*printf("i=%d (index[i]-index[i+1]=%d-%d)\n", i , index[i], index[i+1]);*/
		for (j = 1; j < index[i+1]-index[i]; j++) {
			/*printf("j=%d: area between %d %d %d\n", j, index[i]+j-1, index[i]+j, index[i+1]);*/
			area += nsl_geom_three_point_area(xdata[index[i]+j-1], ydata[index[i]+j-1], xdata[index[i]+j], ydata[index[i]+j], xdata[index[i+1]], ydata[index[i+1]]);
			/*printf("area = %g\n", area);*/
		}
		i++;
	} while(index[i] != n-1);


	return area/(double)n;
}

double nsl_geom_linesim_clip_diag_perpoint(const double xdata[], const double ydata[], const size_t n) {
	double dx = nsl_stats_maximum(xdata, n, NULL) - nsl_stats_minimum(xdata, n, NULL);
	double dy = nsl_stats_maximum(ydata, n, NULL) - nsl_stats_minimum(ydata, n, NULL);
	double d = sqrt(dx*dx+dy*dy);

	return d/(double)n;	/* per point */
}

double nsl_geom_linesim_clip_area_perpoint(const double xdata[], const double ydata[], const size_t n) {
	double dx = nsl_stats_maximum(xdata, n, NULL) - nsl_stats_minimum(xdata, n, NULL);
	double dy = nsl_stats_maximum(ydata, n, NULL) - nsl_stats_minimum(ydata, n, NULL);
	double A = dx*dy;

	return A/(double)n;	/* per point */
}

double nsl_geom_linesim_avg_dist_perpoint(const double xdata[], const double ydata[], const size_t n) {
	double dist = 0;
	size_t i;
	for (i = 0; i < n-1; i++) {
		double dx = xdata[i+1] - xdata[i];
		double dy = ydata[i+1] - ydata[i];
		dist += sqrt(dx*dx + dy*dy);
	}
	dist /= (double)n;

	return dist;
}

/*********** simplification algorithms *********/

void nsl_geom_linesim_douglas_peucker_step(const double xdata[], const double ydata[], const size_t start, const size_t end, size_t *nout, const double tol, size_t index[]) {
	/*printf("DP: %d - %d\n", start, end);*/

	size_t i, nkey = start;
	double maxdist = 0;
	/* search for key (biggest perp. distance) */
	for (i = start+1; i < end; i++) {
		double dist = nsl_geom_point_line_dist(xdata[start], ydata[start], xdata[end], ydata[end], xdata[i], ydata[i]);
		if (dist > maxdist) {
			maxdist = dist;
			nkey = i;
		}
	}
	/*printf("maxdist = %g @ i = %zu\n", maxdist, nkey);*/

	if (maxdist > tol) {
		/*printf("take %d\n", nkey);*/
		index[(*nout)++] = nkey;
		if (nkey-start > 1)
			nsl_geom_linesim_douglas_peucker_step(xdata, ydata, start, nkey, nout, tol, index);
		if (end-nkey > 1)
			nsl_geom_linesim_douglas_peucker_step(xdata, ydata, nkey, end, nout, tol, index);
	}

	/*printf("nout=%d\n", *nout);*/
}

size_t nsl_geom_linesim_douglas_peucker(const double xdata[], const double ydata[], const size_t n, const double tol, size_t index[]) {
	size_t nout = 0;

	/*first point*/
	index[nout++] = 0;

	nsl_geom_linesim_douglas_peucker_step(xdata, ydata, 0, n-1, &nout, tol, index);

	/* last point */
	if (index[nout-1] != n-1)
		index[nout++] = n-1;

	/* sort array index */
	nsl_sort_size_t(index, nout);

	return nout;
}
size_t nsl_geom_linesim_douglas_peucker_auto(const double xdata[], const double ydata[], const size_t n, size_t index[]) {
	double tol = nsl_geom_linesim_clip_diag_perpoint(xdata, ydata, n);
	return nsl_geom_linesim_douglas_peucker(xdata, ydata, n, tol, index);
}

/*
 * Douglas-Peucker variant:
 * The key of all egdes of the current simplified line is calculated and only the
 * largest is added. This is repeated until nout is reached.
 * */
double nsl_geom_linesim_douglas_peucker_variant(const double xdata[], const double ydata[], const size_t n, const size_t nout, size_t index[]) {
	size_t i;
	if (nout >= n) {	/* use all points */
		for (i = 0; i < n; i++)
			index[i] = i;
		return 0;
	}

	/* set first and last point in index (other indizes not initialized) */
	size_t ncount = 0;
	index[ncount++] = 0;
	index[ncount++] = n-1;

	if (nout <= 2)	/* use only first and last point (perp. dist is zero) */
		return 0.0;

	double *dist = (double *)malloc(n * sizeof(double));
	if (dist == NULL) {
		/* printf("nsl_geom_linesim_douglas_peucker_variant(): ERROR allocating memory for 'dist'!\n"); */
		return DBL_MAX;
	}

	double *maxdist = (double *)malloc(nout * sizeof(double));	/* max dist per edge */
	if (maxdist == NULL) {
		/* printf("nsl_geom_linesim_douglas_peucker_variant(): ERROR allocating memory for 'maxdist'!\n"); */
		free(dist);
		return DBL_MAX;
	}

	for (i = 0; i < n; i++) {	/* initialize  dist */
		dist[i] = nsl_geom_point_line_dist(xdata[0], ydata[0], xdata[n-1], ydata[n-1], xdata[i], ydata[i]);
		/* printf("nsl_geom_linesim_douglas_peucker_variant(): %zu dist = %g\n", i, dist[i]); */
	}
	for (i = 0; i < nout; i++)
		maxdist[i] = 0;

	double newmaxdist = 0;
	do {
		/* printf("nsl_geom_linesim_douglas_peucker_variant(): NEXT ITERATION\n"); */
		size_t key = 0;

		/* find edge of maximum */
		size_t maxindex;
		nsl_stats_maximum(maxdist, ncount, &maxindex);
		/* printf("nsl_geom_linesim_douglas_peucker_variant(): found edge with max dist at index %zu\n", maxindex); */
		/*newmaxdist = nsl_stats_maximum(dist, n, &key);*/
		newmaxdist = 0;
		for (i = index[maxindex]+1; i < index[maxindex+1]; i++) {
			/* printf("nsl_geom_linesim_douglas_peucker_variant(): iterate i=%zu\n", i); */
			if (dist[i] > newmaxdist) {
				newmaxdist = dist[i];
				key = i;
			}
		}

		/* printf("nsl_geom_linesim_douglas_peucker_variant(): found key %zu (dist = %g)\n", key, newmaxdist); */
		ncount++;
		dist[key] = 0;

		/* find index of previous key */
		size_t previndex = 0;
		while (index[previndex+1] < key)
			previndex++;
		/* printf("nsl_geom_linesim_douglas_peucker_variant(): previndex = %zu (update keys %zu - %zu)\n", previndex, index[previndex], index[previndex+1]); */

		size_t v;
		/* printf("nsl_geom_linesim_douglas_peucker_variant(): ncount = %zu, previndex = %zu\n", ncount, previndex); */
		/* update dist[]. no update on last key */
		if (ncount < nout) {
			/* shift maxdist */
			for (v = ncount; v > previndex; v--)
				maxdist[v] = maxdist[v-1];

			double tmpmax = 0;
			for (v = index[previndex]+1; v < key; v++) {
				/* printf("nsl_geom_linesim_douglas_peucker_variant(): %zu in %zu - %zu", v, index[previndex], key); */
				dist[v] = nsl_geom_point_line_dist(xdata[index[previndex]], ydata[index[previndex]], xdata[key], ydata[key],
					xdata[v], ydata[v]);
				if (dist[v] > tmpmax)
					tmpmax = dist[v];

				/* printf(" dist = %g\n", dist[v]); */
			}
			maxdist[previndex] = tmpmax;

			tmpmax = 0;
			for (v = key+1; v < index[previndex+1]; v++) {
				/* printf("nsl_geom_linesim_douglas_peucker_variant(): %zu in %zu - %zu", v, key, index[previndex+1]); */
				dist[v] = nsl_geom_point_line_dist(xdata[key], ydata[key], xdata[index[previndex+1]], ydata[index[previndex+1]],
					xdata[v], ydata[v]);
				if (dist[v] > tmpmax)
					tmpmax = dist[v];
				/* printf(" dist = %g\n", dist[v]); */
			}
			maxdist[previndex+1] = tmpmax;
		}

		/* put into index array */
		for (v = ncount; v > previndex+1; v--)
			index[v] = index[v-1];
		index[previndex+1] = key;
	} while (ncount < nout);

	free(maxdist);
	free(dist);

	return newmaxdist;
}

size_t nsl_geom_linesim_nthpoint(const size_t n, const int step, size_t index[]) {
	if (step < 1) {
		printf("step size must be > 0 (given: %d)\n", step);
		return 0;
	}

	size_t i, nout = 0;

	/*first point*/
	index[nout++] = 0;

	for (i = 1; i < n-1; i++)
		if (i%step == 0)
			index[nout++] = i;

	/* last point */
	index[nout++] = n-1;

	return nout;
}

size_t nsl_geom_linesim_raddist(const double xdata[], const double ydata[], const size_t n, const double tol, size_t index[]) {
	size_t i, nout = 0, key = 0;

	/*first point*/
	index[nout++] = 0;

	for (i = 1; i < n-1; i++) {
		/* distance to key point */
		double dist = nsl_geom_point_point_dist(xdata[i], ydata[i], xdata[key], ydata[key]);
		/* distance to last point */
		double lastdist = nsl_geom_point_point_dist(xdata[i], ydata[i], xdata[n-1], ydata[n-1]);
		/*printf("%d: %g %g\n", i, dist, lastdist);*/

		if (dist > tol && lastdist > tol) {
			index[nout++] = i;
			key = i;
		}
	}

	/* last point */
	index[nout++] = n-1;

	return nout;
}
size_t nsl_geom_linesim_raddist_auto(const double xdata[], const double ydata[], const size_t n, size_t index[]) {
	double tol = 10.*nsl_geom_linesim_clip_diag_perpoint(xdata, ydata, n);
	return nsl_geom_linesim_raddist(xdata, ydata, n, tol, index);
}

size_t nsl_geom_linesim_perpdist(const double xdata[], const double ydata[], const size_t n, const double tol, size_t index[]) {
	size_t nout = 0, key = 0, i;

	/*first point*/
	index[nout++] = 0;

	for (i = 1; i < n-1; i++) {
		/* distance of point i to line key -- i+1 */
		double dist = nsl_geom_point_line_dist(xdata[key], ydata[key], xdata[i+1], ydata[i+1], xdata[i], ydata[i]);
		/*printf("%d: %g\n", i, dist);*/

		if (dist > tol) {	/* take it */
			index[nout++] = i;
			key = i;
			/*printf("%d: take it (key = %d)\n", i, key);*/
		} else {	/* ignore it */
			if (i+1 < n-1)	/* last point is taken anyway */
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
	double tol = nsl_geom_linesim_clip_diag_perpoint(xdata, ydata, n);
	return nsl_geom_linesim_perpdist(xdata, ydata, n, tol, index);
}

size_t nsl_geom_linesim_perpdist_repeat(const double xdata[], const double ydata[], const size_t n, const double tol, const size_t repeat, size_t index[]) {
	size_t i, j, nout;
	double *xtmp = (double *) malloc(n*sizeof(double));
	if (xtmp == NULL) {
		printf("nsl_geom_linesim_perpdist_repeat(): ERROR allocating memory for 'xtmp'!\n");
		return 0;
	}

	double *ytmp = (double *) malloc(n*sizeof(double));
	if (ytmp == NULL) {
		printf("nsl_geom_linesim_perpdist_repeat(): ERROR allocating memory for 'ytmp'!\n");
		free(xtmp);
		return 0;
	}

	size_t *tmpindex = (size_t *) malloc(n*sizeof(size_t));
	if (tmpindex == NULL) {
		printf("nsl_geom_linesim_perpdist_repeat(): ERROR allocating memory for 'tmpindex'!\n");
		free(xtmp);
		free(ytmp);
		return 0;
	}

	/* first round */
	nout = nsl_geom_linesim_perpdist(xdata, ydata, n, tol, index);
	/* repeats */
	for (i = 0; i < repeat - 1; i++) {
		for (j = 0; j < nout; j++) {
			xtmp[j] = xdata[index[j]];
			ytmp[j] = ydata[index[j]];
			tmpindex[j]= index[j];
			/*printf("%g %g\n", xtmp[j], ytmp[j]);*/
		}
		size_t tmpnout = nsl_geom_linesim_perpdist(xtmp, ytmp, nout, tol, tmpindex);
		for (j = 0; j < tmpnout; j++) {
			index[j] = index[tmpindex[j]];
			/*printf("tmpindex[%d]: %d\n", j, tmpindex[j]);*/
		}

		if (tmpnout == nout)	/* return if nout does not change anymore */
			break;
		else
			nout = tmpnout;
	}

	free(tmpindex);
	free(xtmp);
	free(ytmp);

	return nout;
}

size_t nsl_geom_linesim_interp(const double xdata[], const double ydata[], const size_t n, const double tol, size_t index[]) {
	size_t i, nout = 0;

	/*first  point*/
	index[nout++] = 0;

	size_t key = 0;
	for (i = 1; i < n-1; i++) {
		/*printf("%d: %d-%d\n", i, key, i+1);*/
		double dist = nsl_geom_point_line_dist_y(xdata[key], ydata[key], xdata[i+1], ydata[i+1], xdata[i], ydata[i]);
		/*printf("%d: dist = %g\n", i, dist);*/
		if (dist > tol) {
			/*printf("take it %d\n", i);*/
			index[nout++] = key = i;
		}
	}

	/* last point */
	index[nout++] = n-1;

	return nout;
}
size_t nsl_geom_linesim_interp_auto(const double xdata[], const double ydata[], const size_t n, size_t index[]) {
	double tol = nsl_geom_linesim_clip_diag_perpoint(xdata, ydata, n);
	return nsl_geom_linesim_interp(xdata, ydata, n, tol, index);
}

size_t nsl_geom_linesim_visvalingam_whyatt(const double xdata[], const double ydata[], const size_t n, const double tol, size_t index[]) {
	if (n < 3)	/* we need at least three points */
		return 0;

	size_t i, nout = n;
	double *area = (double *) malloc((n-2)*sizeof(double));	/* area associated with every point */
	if (area == NULL) {
		printf("nsl_geom_linesim_visvalingam_whyatt(): ERROR allocating memory!\n");
		return 0;
	}
	for (i = 0; i < n; i++)
		index[i] = i;
	for (i = 1; i < n-1; i++)
		area[i-1] = nsl_geom_three_point_area(xdata[i-1], ydata[i-1], xdata[i], ydata[i], xdata[i+1], ydata[i+1]);

	size_t minindex;
	while ( nsl_stats_minimum(area, n-2, &minindex) < tol && nout > 2) {
	/* double minarea;
	while ( (minarea = nsl_stats_minimum(area, n-2, &minindex)) < tol && nout > 2) { */
		/*for (i=0; i < n-2; i++)
			if (area[i]<DBL_MAX)
				printf("area[%zu] = %g\n", i, area[i]);
		*/
		/* remove point minindex */
		/*printf("removing point %zu (minindex = %zu, minarea = %g) nout=%zu\n", minindex+1, minindex, minarea, nout-1);*/
		index[minindex+1] = 0;
		area[minindex] = DBL_MAX;
		double tmparea;
		/* update area of neigbor points */
		size_t before = minindex, after = minindex+2;	/* find index before and after */
		while (index[before] == 0 && before > 0)
			before--;
		while (after < n+1 && index[after] == 0)
			after++;
		if (minindex > 0) {	/*before */
			if (before > 0) {
				size_t beforebefore=before-1;
				while (index[beforebefore] == 0 && beforebefore > 0)
					beforebefore--;
				/*printf("recalculate area[%zu] from %zu %zu %zu\n", before-1, beforebefore, before, after);*/
				tmparea = nsl_geom_three_point_area(xdata[beforebefore], ydata[beforebefore], xdata[before], ydata[before], xdata[after], ydata[after]);
				if (tmparea > area[before-1])	/* take largest value of new and old area */
					area[before-1] = tmparea;
			}
		}
		if (minindex < n-3) {	/* after */
			if (after < n-1) {
				size_t afterafter = after+1;
				while (afterafter < n-1 && index[afterafter] == 0)
					afterafter++;
				/*printf("recalculate area[%zu] from %zu %zu %zu\n",after-1, before, after, afterafter);*/
				tmparea = nsl_geom_three_point_area(xdata[before], ydata[before], xdata[after], ydata[after], xdata[afterafter], ydata[afterafter]);
				if (tmparea > area[after-1])	/* take largest value of new and old area */
					area[after-1] = tmparea;
			}
		}
		nout--;
	};

	/*for(i=0;i<n;i++)
		printf("INDEX = %d\n", index[i]);
	*/

	/* condens index */
	i = 1;
	size_t newi = 1;
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
	double tol = nsl_geom_linesim_clip_area_perpoint(xdata, ydata, n);

	return nsl_geom_linesim_visvalingam_whyatt(xdata, ydata, n, tol, index);
}

size_t nsl_geom_linesim_reumann_witkam(const double xdata[], const double ydata[], const size_t n, const double tol, size_t index[]) {
	size_t i, nout = 0, key = 0, key2 = 1;

	/*first  point*/
	index[nout++] = 0;

	for (i = 2; i < n-1; i++) {
		/* distance to line key -- key2 */
		double dist = nsl_geom_point_line_dist(xdata[key], ydata[key], xdata[key2], ydata[key2], xdata[i], ydata[i]);
		/*printf("%d: %g\n", i, dist);*/

		if (dist > tol) {	/* take it */
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
	double tol = nsl_geom_linesim_clip_diag_perpoint(xdata, ydata, n);
	return nsl_geom_linesim_reumann_witkam(xdata, ydata, n, tol, index);
}

size_t nsl_geom_linesim_opheim(const double xdata[], const double ydata[], const size_t n, const double mintol, const double maxtol, size_t index[]) {
	size_t i, nout = 0, key = 0, key2;

	/*first  point*/
	index[nout++] = 0;

	for (i = 1; i < n-1; i++) {
		double dist, perpdist;
		do {	/* find key2 */
			dist = nsl_geom_point_point_dist(xdata[key], ydata[key], xdata[i], ydata[i]);
			/*printf("dist = %g: %d-%d\n", dist, key, i);*/
			i++;
		} while (dist < mintol);
		i--;
		if (key == i-1)		/*i+1 outside mintol */
			key2 = i;
		else
			key2 = i-1;	/* last point inside */
		/*printf("found key2 @%d\n", key2);*/

		do {	/* find next key */
			dist = nsl_geom_point_point_dist(xdata[key], ydata[key], xdata[i], ydata[i]);
			perpdist = nsl_geom_point_line_dist(xdata[key], ydata[key], xdata[key2], ydata[key2], xdata[i], ydata[i]);
			/*printf("dist = %g, perpdist=%g: %d\n", dist, perpdist, i);*/
			i++;
		} while (dist < maxtol && perpdist < mintol);
		i--;
		if (key == i-1)		/*i+1 outside */
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
	double mintol = 10.*nsl_geom_linesim_clip_diag_perpoint(xdata, ydata, n);
	/* TODO: calculate max tolerance ? */
	double maxtol = 5.*mintol;

	return nsl_geom_linesim_opheim(xdata, ydata, n, mintol, maxtol, index);
}

size_t nsl_geom_linesim_lang(const double xdata[], const double ydata[], const size_t n, const double tol, const size_t region, size_t index[]) {
	size_t i, j, nout = 0, key = 0;

	/*first  point*/
	index[nout++] = 0;

	double dist, maxdist;
	for (i = 1; i < n-1; i++) {
		size_t tmpregion=region;
		if (key+tmpregion > n-1)	/* end of data set */
			tmpregion = n-1-key;

		do {
			maxdist = 0;
			for (j = 1; j < tmpregion; j++) {
				dist = nsl_geom_point_line_dist(xdata[key], ydata[key], xdata[key+tmpregion], ydata[key+tmpregion], xdata[key+j], ydata[key+j]);
				/*printf("%d: dist (%d to %d-%d) = %g\n", j, key+j, key, key+tmpregion, dist);*/
				if (dist > maxdist)
					maxdist = dist;
			}
			/*printf("tol = %g maxdist = %g\n", tol, maxdist);*/
			tmpregion--;
			/*printf("region = %d\n", tmpregion);*/
		} while (maxdist>tol && tmpregion>0);
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
	double tol = nsl_geom_linesim_clip_diag_perpoint(xdata, ydata, n);
	/* TODO: calculate search region */
	size_t region = 0;
	printf("nsl_geom_linesim_lang_auto(): Not implemented yet\n");

	return nsl_geom_linesim_lang(xdata, ydata, n, tol, region, index);
}

