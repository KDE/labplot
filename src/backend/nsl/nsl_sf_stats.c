/*
    File                 : nsl_sf_stats.c
    Project              : LabPlot
    Description          : NSL special statistics functions
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2016-2017 Stefan Gerlach <stefan.gerlach@uni.kn>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#include "nsl_sf_stats.h"
#include "nsl_common.h"

const char* nsl_sf_stats_distribution_name[] = {i18n("Gaussian (Normal)"), i18n("Gaussian Tail"), i18n("Exponential"), i18n("Laplace"),
	i18n("Exponential Power"), i18n("Cauchy-Lorentz (Breit-Wigner)"), i18n("Rayleigh"), i18n("Rayleigh Tail"), i18n("Landau"), i18n("Levy alpha-stable"),
	i18n("Levy skew alpha-stable"), i18n("Gamma"), i18n("Flat (uniform)"), i18n("Log-normal (Galton)"), i18n("Chi-squared"), i18n("F (Fisher-Snedecor)"),
	i18n("Student's t"), i18n("Beta"), i18n("Logistic (sech-squared)"), i18n("Pareto"), i18n("Weibull"), i18n("Gumbel Type-1 (maximum value; log-Weibull)"),
	i18n("Gumbel Type-2"), i18n("Poisson"), i18n("Bernoulli"), i18n("Binomial"), i18n("Negative binomial"), i18n("Pascal"), i18n("Geometric"),
	i18n("Hypergeometric"), i18n("Logarithmic"), i18n("Maxwell-Boltzmann"), i18n("Hyperbolic secant (sech)"), i18n("Levy"), i18n("Frechet (inverse Weibull)")};
const char* nsl_sf_stats_distribution_pic_name[] = {
	"gaussian", "gaussian_tail", "exponential", "laplace", "exponential_power", "cauchy_lorentz", "rayleigh", "rayleigh_tail", "landau",
	"levy_alpha_stable", "levy_skew_alpha_stable","gamma", "flat", "lognormal", "chi_squared", "fdist", "students_t", "beta", "logistic",
	"pareto", "weibull", "gumbel1", "gumbel2", "poisson", "bernoulli", "binomial", "negative_binomial", "pascal", "geometric",
	"hypergeometric", "logarithmic", "maxwell_boltzmann", "sech", "levy", "frechet"};
const char* nsl_sf_stats_distribution_equation[] = {
	"a/sqrt(2*pi)/s * exp(-((x-mu)/s)^2/2)", "2*A/sqrt(2*pi)/s/erfc(a/sqrt(2)/s) * exp(-((x-mu)/s)^2/2) * theta(x-mu-a)", "a*l*exp(-l*(x-mu))",
	"a/(2*s) * exp(-fabs((x-mu)/s))", "a/(2*s*gamma(1+1/b))*exp(-fabs((x-mu)/s)^b)", "a/pi * g/(g^2 + (x-mu)^2)",
	"a * x/s^2 * exp(-(x/s)^2/2)", "a*x/s^2 * exp((mu^2-x^2)/2/s^2)", "a*landau(x)",
	"Levy alpha-stable", "Levy-skew", "a/gamma(k)/t^k * x^(k-1)*exp(-x/t)",
	"A/(b-a)*theta(b-x)*theta(x-a)", "a/sqrt(2*pi)/x/s * exp(-( (log(x)-mu)/s )^2/2)", "a * x^(n/2.-1.)/2^(n/2.)/gamma(n/2.) * exp(-x/2.)",
	"a * fdist(x, n1, n2)", "a*gamma((n+1)/2)/sqrt(pi*n)/gamma(n/2) * (1+x^2/n)^(-(n+1)/2)", "A*gamma(a+b)/gamma(a)/gamma(b) * x^(a-1) * (1-x)^(b-1)",
	"a/4/s * sech((x-mu)/2/s)**2", "a/b * (b/x)^(a+1) * theta(x-b)", "a * k/l * ((x-mu)/l)^(k-1) * exp(-((x-mu)/l)^k)",
	"a/s * exp(-(x-mu)/s - b*exp(-(x-mu)/s))", "A*a*b * (x-mu)^(-a-1) * exp(-b*(x-mu)^(-a))", "a * l^x/gamma(x+1) * exp(-l)",
	"Bernoulli", "a * binomial(x, p, n)", "a * negative_binomial(x, p, n)",
	"a * pascal(x, p, n)", "a * geometric(x, p)", "a * hypergeometric(x, n1, n2, t)",
	"a * logarithmic(x, p)", "a*sqrt(2/pi) * x^2/s^3 * exp(-(x/s)^2/2)", "a/2/s * sech(pi/2*(x-mu)/s)",
	"a * sqrt(g/(2*pi))/pow(x-mu, 1.5) * exp(-g/2./(x-mu))", "a * g/s*((x-mu)/s)^(-g-1) * exp(-((x-mu)/s)^(-g))"};

