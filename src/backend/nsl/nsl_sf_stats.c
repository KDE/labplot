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

const char* nsl_sf_stats_distribution_name[] = {i18n("Gaussian (Normal)"), i18n("Gaussian Tail"), i18n("Exponential"), i18n("Laplace"),
	i18n("Exponential Power"), i18n("Cauchy-Lorentz"), i18n("Rayleigh"), i18n("Rayleigh tail"), i18n("Landau"), i18n("Levy alpha-stable"),
	i18n("Levy skew alpha-stable"), i18n("Gamma"), i18n("Flat (uniform)"), i18n("Lognormal"), i18n("Chi-squared"), i18n("F"), i18n("t"),
	i18n("Beta"), i18n("Logistic"), i18n("Pareto"), i18n("Weibull"), i18n("Type-1 Gumbel"), i18n("Type-2 Gumbel"), i18n("Poisson"),
	i18n("Bernoulli"), i18n("Binomial"), i18n("Negative binomial"), i18n("Pascal"), i18n("Geometric"), i18n("Hypergeometric"), i18n("Logarithmic")};
const char* nsl_sf_stats_distribution_pic_name[] = {
	"gaussian", "gaussian_tail", "exponential", "laplace", "exponential_power", "cauchy_lorentz", "rayleigh",
	"rayleigh_tail", "landau","levy_alpha_stable", "levy_skew_alpha_stable","gamma", "flat",
	"lognormal", "chi_squared", "F", "t", "beta", "logistic",
	"pareto", "weibull", "gumbel_type_1", "gumbel_type_2", "poisson", "bernoulli",
	"binomial", "binomial_negative", "pascal", "geometric", "hypergeometric", "logarithmic"};
const char* nsl_sf_stats_distribution_equation[] = {
	"a/sqrt(2*pi)/s * exp(-((x-mu)/s)^2/2)", "Gaussian Tail", "Exponential",
	"a/(2*s) * exp(-fabs(x-mu)/s)", "Exp power", "a/pi * g/(g^2+(x-mu)^2)",
	"a * x/(s*s) * exp(-x*x/(s*s)/2)", "Rayleigh tail", "Landau",
	"Levy alpha-stable", "Levy-skew", "a * b^p/gamma(p)*x^(p-1)*exp(-b*x)",
	"Flat", "a/(sqrt(2*pi)*x*s) * exp(-( (log(x)-mu)/s )^2/2)", "a * pow(x,n/2.-1.)/pow(2, n/2.)/gamma(n/2.) * exp(-x/2.)",
	"F", "t", "Beta",
	"a/4/s * sech((x-mu)/2/s)**2", "Pareto", "a * k/l * ((x-mu)/l)^(k-1) * exp(-((x-mu)/l)^k)",
	"a/b * exp((x-mu)/b - exp((x-mu)/b))", "Gumbel2", "a * l^x/gamma(x+1) * exp(-l)",
	"Bernoulli", "Binomial", "Neg Binomial",
	"Pascal", "Geometric", "Hypergeo",
	"Logarithmic"};

/*TODO: "c*sqrt(2/pi) * x^2/a^3 * exp(-(x/a)^2/2)",	 	Maxwell-Boltzmann
"a * sqrt(g/(2*pi))/pow(x-mu, 1.5) * exp(-g/2./(x-mu))" 	Levy
"c * a/s*((x-mu)/s)^(-a-1) * exp(-((x-mu)/s)^(-a))", 		Frechet (inverse Weibull)
"a/2/s * sech(pi/2*(x-mu)/s)"};					Hyperbolic secant (sech)
*/
