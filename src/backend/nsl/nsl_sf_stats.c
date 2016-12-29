/***************************************************************************
    File                 : nsl_sf_stats.c
    Project              : LabPlot
    Description          : NSL special statistics functions
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

#include "nsl_common.h"
#include "nsl_sf_stats.h"

const char* nsl_sf_stats_distribution_name[] = {i18n("Gaussian (Normal) distribution"), i18n("Gaussian Tail Distribution"), i18n("Exponential Distribution"),
	i18n("Laplace Distribution"), i18n("Exponential Power Distribution"), i18n("Cauchy-Lorentz Distribution"), i18n("Rayleigh Distribution"),
	i18n("Rayleigh Tail Distribution"), i18n("Landau Distribution"), i18n("Levy alpha-stable Distribution"), i18n("Levy skew alpha-stable Distribution"),
	i18n("Gamma Distribution"), i18n("Flat (Uniform) Distribution"), i18n("Lognormal Distribution"), i18n("Chi-squared Distribution"),
	i18n("F-distribution"), i18n("t-distribution"), i18n("Beta Distribution"), i18n("Logistic Distribution"), i18n("Pareto Distribution"),
	i18n("Weibull Distribution"), i18n("Type-1 Gumbel Distribution"), i18n("Type-2 Gumbel Distribution"), i18n("Poisson Distribution"),
	i18n("Bernoulli Distribution"), i18n("Binomial Distribution"), i18n("Negative Binomial Distribution"), i18n("Pascal Distribution"),
	i18n("Geometric Distribution"), i18n("Hypergeometric Distribution"), i18n("Logarithmic Distribution")};
const char* nsl_sf_stats_distribution_pic_name[] = {
	"gaussian", "gaussian_tail", "exponential", "laplace", "exponential_power", "cauchy_lorentz", "rayleigh",
	"rayleigh_tail", "landau","levy_alpha_stable", "levy_skew_alpha_stable","gamma", "flat",
	"lognormal", "chi_squared", "F", "t", "beta", "logistic",
	"pareto", "weibull", "gumbel_type_1", "gumbel_type_2", "poisson", "bernoulli",
	"binomial", "binomial_negative", "pascal", "geometric", "hypergeometric", "logarithmic"};
