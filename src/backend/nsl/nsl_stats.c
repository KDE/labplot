/***************************************************************************
    File                 : nsl_stats.c
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

#include <math.h>
#include <gsl/gsl_sort.h>
#include "nsl_stats.h"

double nsl_stats_median(double data[], size_t stride, size_t n, nsl_stats_quantile_type type) {
	gsl_sort(data, stride, n);
	return nsl_stats_median_sorted(data,stride,n,type);
}

double nsl_stats_median_sorted(const double sorted_data[], size_t stride, size_t n, nsl_stats_quantile_type type) {
	return nsl_stats_quantile_sorted(sorted_data,stride,n,0.5,type);
}

double nsl_stats_median_from_sorted_data(const double sorted_data[], size_t stride, size_t n) {
	return nsl_stats_median_sorted(sorted_data,stride,n,nsl_stats_quantile_type7);
}

double nsl_stats_quantile(double data[], size_t stride, size_t n, double p, nsl_stats_quantile_type type) {
	gsl_sort(data, stride, n);
	return nsl_stats_quantile_sorted(data,stride,n,p,type);
}

double nsl_stats_quantile_sorted(const double d[], size_t stride, size_t n, double p, nsl_stats_quantile_type type) {

	switch(type) {
	case nsl_stats_quantile_type1:
		if (p == 0.0)
			return d[0];
		else
			return d[((int)ceil(n*p)-1)*stride];
	case nsl_stats_quantile_type2:
		if (p == 0.0)
                        return d[0];
		else if (p == 1.0)
                        return d[(n-1)*stride];
		else
			return (d[((int)ceil(n*p)-1)*stride]+d[((int)ceil(n*p+1)-1)*stride])/2.;
	case nsl_stats_quantile_type3:
		if(p <= 0.5/n)
			return d[0];
		else
			return d[(lrint(n*p)-1)*stride];
	case nsl_stats_quantile_type4:
		if(p < 1./n)
			return d[0];
		else if (p == 1.0)
			return d[(n-1)*stride];
		else {
			int i = floor(n*p);
			return d[(i-1)*stride]+(n*p-i)*(d[i*stride]-d[(i-1)*stride]);	
		}
	case nsl_stats_quantile_type5:
		if(p < 0.5/n)
			return d[0];
		else if (p >= (n-0.5)/n)
			return d[(n-1)*stride];
		else {
			int i = floor(n*p+0.5);
			return d[(i-1)*stride]+(n*p+0.5-i)*(d[i*stride]-d[(i-1)*stride]);	
		}
	case nsl_stats_quantile_type6:
		if(p < 1./(n+1.))
			return d[0];
		else if (p > n/(n+1.))
			return d[(n-1)*stride];
		else {
			int i = floor((n+1)*p);
			return d[(i-1)*stride]+((n+1)*p-i)*(d[i*stride]-d[(i-1)*stride]);	
		}
	case nsl_stats_quantile_type7:
		if (p == 1.0)
                        return d[(n-1)*stride];
                else {
			int i = floor((n-1)*p+1);
			return d[(i-1)*stride]+((n-1)*p+1-i)*(d[i*stride]-d[(i-1)*stride]);	
		}
	case nsl_stats_quantile_type8:
		if (p < 2./3./(n+1./3.))
			return d[0];
		else if (p >= (n-1./3.)/(n+1./3.))
			return d[(n-1)*stride];
		else {
			int i = floor((n+1./3.)*p+1./3.);
			return d[(i-1)*stride]+((n+1./3.)*p+1./3.-i)*(d[i*stride]-d[(i-1)*stride]);	
		}
	case nsl_stats_quantile_type9:
		if (p < 5./8./(n+1./4.))
			return d[0];
		else if (p >= (n-3./8.)/(n+1./4.))
			return d[(n-1)*stride];
		else {
			int i = floor((n+1./4.)*p+3./8.);
			return d[(i-1)*stride]+((n+1./4.)*p+3./8.-i)*(d[i*stride]-d[(i-1)*stride]);	
		}
	}

	return 0;
}

double nsl_stats_quantile_from_sorted_data(const double sorted_data[], size_t stride, size_t n, double p) {
        return nsl_stats_quantile_sorted(sorted_data, stride, n, p, nsl_stats_quantile_type7);
}

