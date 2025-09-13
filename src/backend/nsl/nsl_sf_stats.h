/*
	File                 : nsl_sf_stats.h
	Project              : LabPlot
	Description          : NSL special statistics functions
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2016-2024 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef NSL_SF_STATS_H
#define NSL_SF_STATS_H

#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
#define __BEGIN_DECLS extern "C" {
#define __END_DECLS }
#else
#define __BEGIN_DECLS /* empty */
#define __END_DECLS /* empty */
#endif
__BEGIN_DECLS

#include <stdbool.h>

#define NSL_SF_STATS_DISTRIBUTION_COUNT 36
/* ordered as defined in GSL random number distributions */
/* order is fixed, so append new distros at the end */
typedef enum {
	/* RNG distributions */
	nsl_sf_stats_gaussian,
	nsl_sf_stats_gaussian_tail,
	nsl_sf_stats_exponential,
	nsl_sf_stats_laplace,
	nsl_sf_stats_exponential_power,
	nsl_sf_stats_cauchy_lorentz,
	nsl_sf_stats_rayleigh,
	nsl_sf_stats_rayleigh_tail,
	nsl_sf_stats_landau,
	nsl_sf_stats_levy_alpha_stable,
	nsl_sf_stats_levy_skew_alpha_stable,
	nsl_sf_stats_gamma,
	nsl_sf_stats_flat,
	nsl_sf_stats_lognormal,
	nsl_sf_stats_chi_squared,
	nsl_sf_stats_fdist,
	nsl_sf_stats_tdist,
	nsl_sf_stats_beta,
	nsl_sf_stats_logistic,
	nsl_sf_stats_pareto,
	nsl_sf_stats_weibull,
	nsl_sf_stats_gumbel1,
	nsl_sf_stats_gumbel2,
	nsl_sf_stats_poisson,
	nsl_sf_stats_bernoulli,
	nsl_sf_stats_binomial,
	nsl_sf_stats_negative_binomial,
	nsl_sf_stats_pascal,
	nsl_sf_stats_geometric,
	nsl_sf_stats_hypergeometric,
	nsl_sf_stats_logarithmic,
	/* other distributions */
	nsl_sf_stats_maxwell_boltzmann,
	nsl_sf_stats_sech,
	nsl_sf_stats_levy,
	nsl_sf_stats_frechet,
	nsl_sf_stats_triangular
} nsl_sf_stats_distribution;
/*TODO: CDF, SF, ... ? */

extern const char* nsl_sf_stats_distribution_name[];
extern const char* nsl_sf_stats_distribution_pic_name[];
extern const char* nsl_sf_stats_distribution_equation[];

/* distribution supports random number generation? */
bool nsl_sf_stats_distribution_supports_RNG(nsl_sf_stats_distribution);
/* distribution supports maximum likelihood estimation? */
bool nsl_sf_stats_distribution_supports_ML(nsl_sf_stats_distribution);

__END_DECLS

#endif /* NSL_SF_STATS_H */
