/***************************************************************************
    File                 : nsl_stats.h
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

#ifndef NSL_STATS_H
#define NSL_STATS_H

#include <stdlib.h>

/* estimation types of quantile (see https://en.wikipedia.org/wiki/Quantile,
 * https://docs.scipy.org/doc/scipy/reference/generated/scipy.stats.mstats.mquantiles.html) */
#define NSL_STATS_QUANTILE_TYPE_COUNT 9
typedef enum {nsl_stats_quantile_type1=1, nsl_stats_quantile_type2, nsl_stats_quantile_type3, nsl_stats_quantile_type4, 
		nsl_stats_quantile_type5, nsl_stats_quantile_type6, nsl_stats_quantile_type7, nsl_stats_quantile_type8,
		nsl_stats_quantile_type9} nsl_stats_quantile_type;

/* minimum value of data array 
	data - data array
	n - size of data array
	index - index of minimum value	(not used if NULL)
*/
double nsl_stats_minimum(const double data[], const size_t n, size_t *index);
/* maximum value of data array 
	data - data array
	n - size of data array
	index - index of maximum value (not used if NULL)
*/
double nsl_stats_maximum(const double data[], const size_t n, size_t *index);

/* median from unsorted data. data will be sorted! */
double nsl_stats_median(double data[], size_t stride, size_t n, nsl_stats_quantile_type type);
/* median from sorted data */
double nsl_stats_median_sorted(const double sorted_data[], size_t stride, size_t n, nsl_stats_quantile_type type);
/* GSL legacy function */
double nsl_stats_median_from_sorted_data(const double sorted_data[], size_t stride, size_t n);

/* quantile from unsorted data. data will be sorted! */
double nsl_stats_quantile(double data[], size_t stride, size_t n, double p, nsl_stats_quantile_type type);
/* quantile from sorted data */
double nsl_stats_quantile_sorted(const double sorted_data[], size_t stride, size_t n, double p, nsl_stats_quantile_type type);
/* GSL legacy function */
double nsl_stats_quantile_from_sorted_data(const double sorted_data[], size_t stride, size_t n, double p);

/* R^2 */
double nsl_stats_rsquare(double sse, double sst);
/* adj. R^2 default version=1 */
double nsl_stats_rsquareAdj(double rsquare, size_t np, size_t dof, int version);

/* t distribution */
double nsl_stats_tdist_t(double parameter, double error);
/* p value */
double nsl_stats_tdist_p(double t, double dof);
/* margin (half of confidence interval) */
double nsl_stats_tdist_margin(double alpha, double dof, double error);

/* chi^2 distribution */
double nsl_stats_chisq_p(double t, double dof);

/* F distribution */
double nsl_stats_fdist_F(double rsquare, size_t np, size_t dof);
/* p value */
double nsl_stats_fdist_p(double F, size_t np, double dof);

/* log-likelihood */
double nsl_stats_logLik(double sse, size_t n);

/* Akaike information criterion (AIC) */
double nsl_stats_aic(double sse, size_t n, size_t np, int version);
/* bias-corrected version */
double nsl_stats_aicc(double sse, size_t n, size_t np, int version);

/* Schwarz Bayesian information criterion (BIC, SBC, SBIC) */
double nsl_stats_bic(double sse, size_t n, size_t np, int version);

#endif /* NSL_STATS_H */
