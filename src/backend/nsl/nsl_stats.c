/***************************************************************************
    File                 : nsl_stats.c
    Project              : LabPlot
    Description          : NSL statistics functions
    --------------------------------------------------------------------
    Copyright            : (C) 2016-2017 by Stefan Gerlach (stefan.gerlach@uni.kn)

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

#include "nsl_stats.h"
#include <math.h>
#include <float.h>
#include <gsl/gsl_sort.h>
#include <gsl/gsl_cdf.h>

double nsl_stats_minimum(const double data[], const size_t n, size_t *index) {
	size_t i;

	double min = data[0];
	if (index != NULL) 
		*index = 0;

	for (i = 1; i < n; i++) {
		if (data[i] < min) {
			min = data[i];
			if (index != NULL) 
				*index = i;
		}
	}

	return min;
}

double nsl_stats_maximum(const double data[], const size_t n, size_t *index) {
	size_t i;

	double max = data[0];
	if (index != NULL) 
		*index = 0;

	for (i = 1; i < n; i++) {
		if (data[i] > max) {
			 max = data[i];
			 if (index != NULL) 
				*index = i;
		}
	}

	return max;
}

double nsl_stats_median(double data[], size_t stride, size_t n, nsl_stats_quantile_type type) {
	gsl_sort(data, stride, n);
	return nsl_stats_median_sorted(data, stride, n, type);
}

double nsl_stats_median_sorted(const double sorted_data[], size_t stride, size_t n, nsl_stats_quantile_type type) {
	return nsl_stats_quantile_sorted(sorted_data, stride, n, 0.5, type);
}

double nsl_stats_median_from_sorted_data(const double sorted_data[], size_t stride, size_t n) {
	return nsl_stats_median_sorted(sorted_data, stride, n, nsl_stats_quantile_type7);
}

double nsl_stats_quantile(double data[], size_t stride, size_t n, double p, nsl_stats_quantile_type type) {
	gsl_sort(data, stride, n);
	return nsl_stats_quantile_sorted(data, stride, n, p, type);
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
#ifdef _WIN32
			return d[((int)round(n*p)-1)*stride];
#else
			return d[(lrint(n*p)-1)*stride];
#endif
	case nsl_stats_quantile_type4:
		if(p < 1./n)
			return d[0];
		else if (p == 1.0)
			return d[(n-1)*stride];
		else {
			int i = (int)floor(n*p);
			return d[(i-1)*stride]+(n*p-i)*(d[i*stride]-d[(i-1)*stride]);	
		}
	case nsl_stats_quantile_type5:
		if(p < 0.5/n)
			return d[0];
		else if (p >= (n-0.5)/n)
			return d[(n-1)*stride];
		else {
			int i = (int)floor(n*p+0.5);
			return d[(i-1)*stride]+(n*p+0.5-i)*(d[i*stride]-d[(i-1)*stride]);	
		}
	case nsl_stats_quantile_type6:
		if(p < 1./(n+1.))
			return d[0];
		else if (p > n/(n+1.))
			return d[(n-1)*stride];
		else {
			int i = (int)floor((n+1)*p);
			return d[(i-1)*stride]+((n+1)*p-i)*(d[i*stride]-d[(i-1)*stride]);	
		}
	case nsl_stats_quantile_type7:	// = gsl_stats_quantile_from_sorted_data(d, stride, n, p);
		if (p == 1.0)
                        return d[(n-1)*stride];
                else {
			int i = (int)floor((n-1)*p+1);
			return d[(i-1)*stride]+((n-1)*p+1-i)*(d[i*stride]-d[(i-1)*stride]);	
		}
	case nsl_stats_quantile_type8:
		if (p < 2./3./(n+1./3.))
			return d[0];
		else if (p >= (n-1./3.)/(n+1./3.))
			return d[(n-1)*stride];
		else {
			int i = (int)floor((n+1./3.)*p+1./3.);
			return d[(i-1)*stride]+((n+1./3.)*p+1./3.-i)*(d[i*stride]-d[(i-1)*stride]);	
		}
	case nsl_stats_quantile_type9:
		if (p < 5./8./(n+1./4.))
			return d[0];
		else if (p >= (n-3./8.)/(n+1./4.))
			return d[(n-1)*stride];
		else {
			int i = (int)floor((n+1./4.)*p+3./8.);
			return d[(i-1)*stride]+((n+1./4.)*p+3./8.-i)*(d[i*stride]-d[(i-1)*stride]);	
		}
	}

	return 0;
}

double nsl_stats_quantile_from_sorted_data(const double sorted_data[], size_t stride, size_t n, double p) {
        return nsl_stats_quantile_sorted(sorted_data, stride, n, p, nsl_stats_quantile_type7);
}

/* R^2 and adj. R^2 */
double nsl_stats_rsquare(double sse, double sst) {
	return 1. - sse/sst;
}
double nsl_stats_rsquareAdj(double rsquare, size_t np, size_t dof, int version) {
	size_t n = np + dof;
	// see https://stats.stackexchange.com/questions/48703/what-is-the-adjusted-r-squared-formula-in-lm-in-r-and-how-should-it-be-interpret
	switch (version) {
	case 2:
		return 1. - (1. - rsquare) * (n - 1.)/dof;
	default:
		return 1. - (1. - rsquare) * (n - 1.)/(dof - 1.);
	}
}

/* t distribution */
double nsl_stats_tdist_t(double parameter, double error) {
	if (error > 0)
		return parameter/error;
	else
		return DBL_MAX;
}

double nsl_stats_tdist_p(double t, double dof) {
	double p = 2. * gsl_cdf_tdist_Q(fabs(t), dof);
	if (p < 1.e-9)
		p = 0;
	return p;
}
double nsl_stats_tdist_margin(double alpha, double dof, double error) {
	return gsl_cdf_tdist_Pinv(1. - alpha/2., dof) * error;
}

/* chi^2 distribution */
double nsl_stats_chisq_p(double t, double dof) {
	double p = gsl_cdf_chisq_Q(t, dof);
	if (p < 1.e-9)
		p = 0;
	return p;
}

/* F distribution */
double nsl_stats_fdist_F(double sst, double rms, unsigned int np, int version) {
	switch (version) {
	case 2:
		if (np > 1)	// scale accourding R
			sst /= np;
		break;
	default:
		if (np > 2)     // scale according NIST reference
			sst /= (np-1);
	}
	return sst/rms;
}
double nsl_stats_fdist_p(double F, size_t np, double dof) {
	double p = gsl_cdf_fdist_Q(F, (double)np, dof);
	if (p < 1.e-9)
		p = 0;
	return p;
}

/* log-likelihood */
double nsl_stats_logLik(double sse, size_t n) {
	double ll = -(double)n/2.*log(sse/n) - n/2.*log(2*M_PI) -n/2.;
	return ll;
}

/* Akaike information criterion */
double nsl_stats_aic(double sse, size_t n, size_t np, int version) {
	switch (version) {
	case 2:
		return n * log(sse/n) + 2. * np;	// reduced formula
	case 3: {
		double aic = n * log(sse/n) + 2. * np;
		if (n < 40 * np)	// bias correction
			aic += 2. * np * (np + 1.)/(n - np - 1.);
		return aic;
	}
	default:
		return n * log(sse/n) + 2. * (np+1) + n*log(2.*M_PI) + n;	// complete formula used in R
	}
}
double nsl_stats_aicc(double sse, size_t n, size_t np, int version) {
	double aic;
	switch (version) {
	case 2:
		aic = n * log(sse/n) + 2. * np;
		break;
	default:
		aic = n * log(sse/n) + 2. * (np+1) + n*log(2.*M_PI) + n;
	}
	return aic + 2. * np * (np + 1.)/(n - np - 1.);
}

/* Bayasian information criterion */
double nsl_stats_bic(double sse, size_t n, size_t np, int version) {
	switch (version) {
	case 2:
		return n * log(sse/n) + np * log((double)n); // reduced formula
	default:
		return n * log(sse/n) + (np+1) * log((double)n) + n + n*log(2.*M_PI); // complete formula used in R
	}
}
