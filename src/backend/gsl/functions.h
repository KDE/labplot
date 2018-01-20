/***************************************************************************
    File                 : functions.h
    Project              : LabPlot
    Description          : definition of functions
    --------------------------------------------------------------------
    Copyright            : (C) 2014 by Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2014-2017 by Stefan Gerlach (stefan.gerlach@uni.kn)

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

#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <gsl/gsl_version.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_sf.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_cdf.h>
#include "backend/nsl/nsl_sf_basic.h"

#ifdef _MSC_VER
/* avoid intrinsics */
#pragma function(ceil, floor)
#endif

/* sync with ExpressionParser.cpp */
struct func _functions[] = {
	/* standard functions */
	{"rand", nsl_sf_rand},
	{"random", nsl_sf_random},
	{"drand", nsl_sf_drand},
	/* math.h */
	{"ceil", (func_t)ceil},
	{"fabs", (func_t)fabs},
	{"log10", (func_t)log10},
	{"pow", (func_t)pow},
	{"sqrt", (func_t)sqrt},
	{"sgn", (func_t)nsl_sf_sgn},
	{"theta", (func_t)nsl_sf_theta},
#ifndef _WIN32
	{"cbrt", cbrt},
	{"logb", logb},
	{"rint", rint},
	{"round", round},
	{"trunc", trunc},
#endif
	/* GSL mathematical functions: see http://www.gnu.org/software/gsl/manual/gsl-ref.html#Mathematical-Functions */
	{"log1p", gsl_log1p},
	{"ldexp", nsl_sf_ldexp},
	{"powint", nsl_sf_powint},
	{"pow2", gsl_pow_2},
	{"pow3", gsl_pow_3},
	{"pow4", gsl_pow_4},
	{"pow5", gsl_pow_5},
	{"pow6", gsl_pow_6},
	{"pow7", gsl_pow_7},
	{"pow8", gsl_pow_8},
	{"pow9", gsl_pow_9},
	/* GSL special functions: see http://www.gnu.org/software/gsl/manual/html_node/Special-Functions.html */
	/* Airy Functions and Derivatives */
	{"Ai", nsl_sf_airy_Ai},
	{"Bi", nsl_sf_airy_Bi},
	{"Ais", nsl_sf_airy_Ais},
	{"Bis", nsl_sf_airy_Bis},
	{"Aid", nsl_sf_airy_Aid},
	{"Bid", nsl_sf_airy_Bid},
	{"Aids", nsl_sf_airy_Aids},
	{"Bids", nsl_sf_airy_Bids},
	{"Ai0", nsl_sf_airy_0_Ai},
	{"Bi0", nsl_sf_airy_0_Bi},
	{"Aid0", nsl_sf_airy_0_Aid},
	{"Bid0", nsl_sf_airy_0_Bid},
	/* Bessel Functions */
	{"J0", gsl_sf_bessel_J0},
	{"J1", gsl_sf_bessel_J1},
	{"Jn", nsl_sf_bessel_Jn},
	{"Y0", gsl_sf_bessel_Y0},
	{"Y1", gsl_sf_bessel_Y1},
	{"Yn", nsl_sf_bessel_Yn},
	{"I0", gsl_sf_bessel_I0},
	{"I1", gsl_sf_bessel_I1},
	{"In", nsl_sf_bessel_In},
	{"I0s", gsl_sf_bessel_I0_scaled},
	{"I1s", gsl_sf_bessel_I1_scaled},
	{"Ins", nsl_sf_bessel_Ins},
	{"K0", gsl_sf_bessel_K0},
	{"K1", gsl_sf_bessel_K1},
	{"Kn", nsl_sf_bessel_Kn},
	{"K0s", gsl_sf_bessel_K0_scaled},
	{"K1s", gsl_sf_bessel_K1_scaled},
	{"Kns", nsl_sf_bessel_Kns},
	{"j0", gsl_sf_bessel_j0},
	{"j1", gsl_sf_bessel_j1},
	{"j2", gsl_sf_bessel_j2},
	{"jl", nsl_sf_bessel_jl},
	{"y0", gsl_sf_bessel_y0},
	{"y1", gsl_sf_bessel_y1},
	{"y2", gsl_sf_bessel_y2},
	{"yl", nsl_sf_bessel_yl},
	{"i0s", gsl_sf_bessel_i0_scaled},
	{"i1s", gsl_sf_bessel_i1_scaled},
	{"i2s", gsl_sf_bessel_i2_scaled},
	{"ils", nsl_sf_bessel_ils},
	{"k0s", gsl_sf_bessel_k0_scaled},
	{"k1s", gsl_sf_bessel_k1_scaled},
	{"k2s", gsl_sf_bessel_k2_scaled},
	{"kls", nsl_sf_bessel_kls},
	{"Jnu", gsl_sf_bessel_Jnu},
	{"Ynu", gsl_sf_bessel_Ynu},
	{"Inu", gsl_sf_bessel_Inu},
	{"Inus", gsl_sf_bessel_Inu_scaled},
	{"Knu", gsl_sf_bessel_Knu},
	{"lnKnu", gsl_sf_bessel_lnKnu},
	{"Knus", gsl_sf_bessel_Knu_scaled},
	{"J0_0", nsl_sf_bessel_0_J0},
	{"J1_0", nsl_sf_bessel_0_J1},
	{"Jnu_0", nsl_sf_bessel_0_Jnu},
	/* Clausen functions */
	{"clausen", gsl_sf_clausen},
	/* Coulomb functions */
	{"hydrogenicR_1", gsl_sf_hydrogenicR_1},
	{"hydrogenicR", nsl_sf_hydrogenicR},
	{"dawson", gsl_sf_dawson},
	/* Debye functions */
	{"D1", gsl_sf_debye_1},
	{"D2", gsl_sf_debye_2},
	{"D3", gsl_sf_debye_3},
	{"D4", gsl_sf_debye_4},
	{"D5", gsl_sf_debye_5},
	{"D6", gsl_sf_debye_6},
	{"Li2", gsl_sf_dilog},
	/* Elliptic integrals */
	{"Kc", nsl_sf_ellint_Kc},
	{"Ec", nsl_sf_ellint_Ec},
	{"Pc", nsl_sf_ellint_Pc},
	{"F", nsl_sf_ellint_F},
	{"E", nsl_sf_ellint_E},
	{"P", nsl_sf_ellint_P},
	{"D", nsl_sf_ellint_D},
	{"RC", nsl_sf_ellint_RC},
	{"RD", nsl_sf_ellint_RD},
	{"RF", nsl_sf_ellint_RF},
	{"RJ", nsl_sf_ellint_RJ},
	/* Error functions */
	{"erf", gsl_sf_erf},
	{"erfc", gsl_sf_erfc},
	{"log_erfc", gsl_sf_log_erfc},
	{"erf_Z", gsl_sf_erf_Z},
	{"erf_Q", gsl_sf_erf_Q},
	{"hazard", gsl_sf_hazard},
	/* Exponential Functions */
	{"exp", gsl_sf_exp},
	{"exp_mult", gsl_sf_exp_mult},
	{"expm1", gsl_expm1},
	{"exprel", gsl_sf_exprel},
	{"exprel2", gsl_sf_exprel_2},
	{"expreln", nsl_sf_exprel_n},
	/* Exponential Integrals */
	{"E1", gsl_sf_expint_E1},
	{"E2", gsl_sf_expint_E2},
	{"En", gsl_sf_expint_En},
	{"Ei", gsl_sf_expint_Ei},
	{"Shi", gsl_sf_Shi},
	{"Chi", gsl_sf_Chi},
	{"Ei3", gsl_sf_expint_3},
	{"Si", gsl_sf_Si},
	{"Ci", gsl_sf_Ci},
	{"Atanint", gsl_sf_atanint},
	/* Fermi-Dirac Function */
	{"Fm1", gsl_sf_fermi_dirac_m1},
	{"F0", gsl_sf_fermi_dirac_0},
	{"F1", gsl_sf_fermi_dirac_1},
	{"F2", gsl_sf_fermi_dirac_2},
	{"Fj", nsl_sf_fermi_dirac_int},
	{"Fmhalf", gsl_sf_fermi_dirac_mhalf},
	{"Fhalf", gsl_sf_fermi_dirac_half},
	{"F3half", gsl_sf_fermi_dirac_3half},
	{"Finc0", gsl_sf_fermi_dirac_inc_0},
	/* Gamma and Beta Functions */
	{"gamma", gsl_sf_gamma},
	{"tgamma", gsl_sf_gamma},
	{"lgamma", gsl_sf_lngamma},
	{"lngamma", gsl_sf_lngamma},
	{"gammastar", gsl_sf_gammastar},
	{"gammainv", gsl_sf_gammainv},
	{"fact", nsl_sf_fact},
	{"doublefact", nsl_sf_doublefact},
	{"lnfact", nsl_sf_lnfact},
	{"lndoublefact", nsl_sf_lndoublefact},
	{"choose", nsl_sf_choose},
	{"lnchoose", nsl_sf_lnchoose},
	{"taylor", nsl_sf_taylorcoeff},
	{"poch", gsl_sf_poch},
	{"lnpoch", gsl_sf_lnpoch},
	{"pochrel", gsl_sf_pochrel},
	{"gammainc", gsl_sf_gamma_inc},
	{"gammaincQ", gsl_sf_gamma_inc_Q},
	{"gammaincP", gsl_sf_gamma_inc_P},
	{"beta", gsl_sf_beta},
	{"lnbeta", gsl_sf_lnbeta},
	{"betainc", gsl_sf_beta_inc},
	/* Gegenbauer Functions */
	{"C1", gsl_sf_gegenpoly_1},
	{"C2", gsl_sf_gegenpoly_2},
	{"C3", gsl_sf_gegenpoly_3},
	{"Cn", nsl_sf_gegenpoly_n},
#if (GSL_MAJOR_VERSION > 2) || (GSL_MAJOR_VERSION == 2) && (GSL_MINOR_VERSION >= 4)
	/* Hermite polynomials and functions */
	{"Hen", nsl_sf_hermite_prob},
	{"Hn", nsl_sf_hermite_phys},
	{"Hfn", nsl_sf_hermite_func},
	{"Hend", nsl_sf_hermite_prob_der},
	{"Hnd", nsl_sf_hermite_phys_der},
	{"Hfnd", nsl_sf_hermite_func_der},
#endif
	/* Hypergeometric Functions */
	{"hyperg_0F1", gsl_sf_hyperg_0F1},
	{"hyperg_1F1i", nsl_sf_hyperg_1F1i},
	{"hyperg_1F1", gsl_sf_hyperg_1F1},
	{"hyperg_Ui", nsl_sf_hyperg_Ui},
	{"hyperg_U", gsl_sf_hyperg_U},
	{"hyperg_2F1", gsl_sf_hyperg_2F1},
	{"hyperg_2F1c", gsl_sf_hyperg_2F1_conj},
	{"hyperg_2F1r", gsl_sf_hyperg_2F1_renorm},
	{"hyperg_2F1cr", gsl_sf_hyperg_2F1_conj_renorm},
	{"hyperg_2F0", gsl_sf_hyperg_2F0},
	/* Laguerre Functions */
	{"L1", gsl_sf_laguerre_1},
	{"L2", gsl_sf_laguerre_2},
	{"L3", gsl_sf_laguerre_3},
	/* Lambert W Functions */
	{"W0", gsl_sf_lambert_W0},
	{"Wm1", gsl_sf_lambert_Wm1},
	/* Legendre Functions and Spherical Harmonics */
	{"P1", gsl_sf_legendre_P1},
	{"P2", gsl_sf_legendre_P2},
	{"P3", gsl_sf_legendre_P3},
	{"Pl", nsl_sf_legendre_Pl},
	{"Q0", gsl_sf_legendre_Q0},
	{"Q1", gsl_sf_legendre_Q1},
	{"Ql", nsl_sf_legendre_Ql},
	{"Plm", nsl_sf_legendre_Plm},
	{"Pslm", nsl_sf_legendre_sphPlm},
	{"Phalf", gsl_sf_conicalP_half},
	{"Pmhalf", gsl_sf_conicalP_mhalf},
	{"Pc0", gsl_sf_conicalP_0},
	{"Pc1", gsl_sf_conicalP_1},
	{"Psr", nsl_sf_conicalP_sphreg},
	{"Pcr", nsl_sf_conicalP_cylreg},
	{"H3d0", gsl_sf_legendre_H3d_0},
	{"H3d1", gsl_sf_legendre_H3d_1},
	{"H3d", nsl_sf_legendre_H3d},
	/* Logarithm and Related Functions */
	{"log", gsl_sf_log},
	{"logabs", gsl_sf_log_abs},
	{"logp", gsl_sf_log_1plusx},
	{"logpm", gsl_sf_log_1plusx_mx},
	/* Power Function */
	{"gsl_powint", nsl_sf_powint},
	/* Psi (Digamma) Function */
	{"psiint", nsl_sf_psiint},
	{"psi", gsl_sf_psi},
	{"psi1piy", gsl_sf_psi_1piy},
	{"psi1int", nsl_sf_psi1int},
	{"psi1", gsl_sf_psi_1},
	{"psin", nsl_sf_psin},
	/* Synchrotron Functions */
	{"synchrotron1", gsl_sf_synchrotron_1},
	{"synchrotron2", gsl_sf_synchrotron_2},
	/* Transport Functions */
	{"J2", gsl_sf_transport_2},
	{"J3", gsl_sf_transport_3},
	{"J4", gsl_sf_transport_4},
	{"J5", gsl_sf_transport_5},
	/* Trigonometric Functions */
	{"sin", gsl_sf_sin},
	{"cos", gsl_sf_cos},
	{"tan", tan},
	{"asin", asin},
	{"acos", acos},
	{"atan", atan},
	{"atan2", atan2},
	{"sinh", sinh},
	{"cosh", cosh},
	{"tanh", tanh},
	{"acosh", gsl_acosh},
	{"asinh", gsl_asinh},
	{"atanh", gsl_atanh},
	{"sec", nsl_sf_sec},
	{"csc", nsl_sf_csc},
	{"cot", nsl_sf_cot},
	{"asec", nsl_sf_asec},
	{"acsc", nsl_sf_acsc},
	{"acot", nsl_sf_acot},
	{"sech", nsl_sf_sech},
	{"csch", nsl_sf_csch},
	{"coth", nsl_sf_coth},
	{"asech", nsl_sf_asech},
	{"acsch", nsl_sf_acsch},
	{"acoth", nsl_sf_acoth},
	{"sinc", gsl_sf_sinc},
	{"logsinh", gsl_sf_lnsinh},
	{"logcosh", gsl_sf_lncosh},
	{"hypot", gsl_sf_hypot},
	{"hypot3", gsl_hypot3},
	{"anglesymm", gsl_sf_angle_restrict_symm},
	{"anglepos", gsl_sf_angle_restrict_pos},
	/* Zeta Functions */
	{"zetaint", nsl_sf_zetaint},
	{"zeta", gsl_sf_zeta},
	{"zetam1int", nsl_sf_zetam1int},
	{"zetam1", gsl_sf_zetam1},
	{"hzeta", gsl_sf_hzeta},
	{"etaint", nsl_sf_etaint},
	{"eta", gsl_sf_eta},

	/* GSL Random Number Distributions: see http://www.gnu.org/software/gsl/manual/html_node/Random-Number-Distributions.html */
	/* Gaussian Distribution */
	{"gaussian", gsl_ran_gaussian_pdf},
	{"ugaussian", gsl_ran_ugaussian_pdf},
	{"gaussianP", gsl_cdf_gaussian_P},
	{"gaussianQ", gsl_cdf_gaussian_Q},
	{"gaussianPinv", gsl_cdf_gaussian_Pinv},
	{"gaussianQinv", gsl_cdf_gaussian_Qinv},
	{"ugaussianP", gsl_cdf_ugaussian_P},
	{"ugaussianQ", gsl_cdf_ugaussian_Q},
	{"ugaussianPinv", gsl_cdf_ugaussian_Pinv},
	{"ugaussianQinv", gsl_cdf_ugaussian_Qinv},
	{"gaussiantail", gsl_ran_gaussian_tail_pdf},
	{"ugaussiantail", gsl_ran_ugaussian_tail_pdf},
	{"gaussianbi", gsl_ran_bivariate_gaussian_pdf},
	/* Exponential Distribution */
	{"exponential", gsl_ran_exponential_pdf},
	{"exponentialP", gsl_cdf_exponential_P},
	{"exponentialQ", gsl_cdf_exponential_Q},
	{"exponentialPinv", gsl_cdf_exponential_Pinv},
	{"exponentialQinv", gsl_cdf_exponential_Qinv},
	/* Laplace Distribution */
	{"laplace", gsl_ran_laplace_pdf},
	{"laplaceP", gsl_cdf_laplace_P},
	{"laplaceQ", gsl_cdf_laplace_Q},
	{"laplacePinv", gsl_cdf_laplace_Pinv},
	{"laplaceQinv", gsl_cdf_laplace_Qinv},
	/* Exponential Power Distribution */
	{"exppow", gsl_ran_exppow_pdf},
	{"exppowP", gsl_cdf_exppow_P},
	{"exppowQ", gsl_cdf_exppow_Q},
	/* Cauchy Distribution */
	{"cauchy", gsl_ran_cauchy_pdf},
	{"cauchyP", gsl_cdf_cauchy_P},
	{"cauchyQ", gsl_cdf_cauchy_Q},
	{"cauchyPinv", gsl_cdf_cauchy_Pinv},
	{"cauchyQinv", gsl_cdf_cauchy_Qinv},
	/* Rayleigh Distribution */
	{"rayleigh", gsl_ran_rayleigh_pdf},
	{"rayleighP", gsl_cdf_rayleigh_P},
	{"rayleighQ", gsl_cdf_rayleigh_Q},
	{"rayleighPinv", gsl_cdf_rayleigh_Pinv},
	{"rayleighQinv", gsl_cdf_rayleigh_Qinv},
	{"rayleigh_tail", gsl_ran_rayleigh_tail_pdf},
	/* Landau Distribution */
	{"landau", gsl_ran_landau_pdf},
	/* Gamma Distribution */
	{"gammapdf", gsl_ran_gamma_pdf},
	{"gammaP", gsl_cdf_gamma_P},
	{"gammaQ", gsl_cdf_gamma_Q},
	{"gammaPinv", gsl_cdf_gamma_Pinv},
	{"gammaQinv", gsl_cdf_gamma_Qinv},
	/* Flat (Uniform) Distribution */
	{"flat", gsl_ran_flat_pdf},
	{"flatP", gsl_cdf_flat_P},
	{"flatQ", gsl_cdf_flat_Q},
	{"flatPinv", gsl_cdf_flat_Pinv},
	{"flatQinv", gsl_cdf_flat_Qinv},
	/* Lognormal Distribution */
	{"lognormal", gsl_ran_lognormal_pdf},
	{"lognormalP", gsl_cdf_lognormal_P},
	{"lognormalQ", gsl_cdf_lognormal_Q},
	{"lognormalPinv", gsl_cdf_lognormal_Pinv},
	{"lognormalQinv", gsl_cdf_lognormal_Qinv},
	/* Chi-squared Distribution */
	{"chisq", gsl_ran_chisq_pdf},
	{"chisqP", gsl_cdf_chisq_P},
	{"chisqQ", gsl_cdf_chisq_Q},
	{"chisqPinv", gsl_cdf_chisq_Pinv},
	{"chisqQinv", gsl_cdf_chisq_Qinv},
	/* F-distribution */
	{"fdist", gsl_ran_fdist_pdf},
	{"fdistP", gsl_cdf_fdist_P},
	{"fdistQ", gsl_cdf_fdist_Q},
	{"fdistPinv", gsl_cdf_fdist_Pinv},
	{"fdistQinv", gsl_cdf_fdist_Qinv},
	/* t-distribution */
	{"tdist", gsl_ran_tdist_pdf},
	{"tdistP", gsl_cdf_tdist_P},
	{"tdistQ", gsl_cdf_tdist_Q},
	{"tdistPinv", gsl_cdf_tdist_Pinv},
	{"tdistQinv", gsl_cdf_tdist_Qinv},
	/* Beta Distribution */
	{"betapdf", gsl_ran_beta_pdf},
	{"betaP", gsl_cdf_beta_P},
	{"betaQ", gsl_cdf_beta_Q},
	{"betaPinv", gsl_cdf_beta_Pinv},
	{"betaQinv", gsl_cdf_beta_Qinv},
	/* Logistic Distribution */
	{"logistic", gsl_ran_logistic_pdf},
	{"logisticP", gsl_cdf_logistic_P},
	{"logisticQ", gsl_cdf_logistic_Q},
	{"logisticPinv", gsl_cdf_logistic_Pinv},
	{"logisticQinv", gsl_cdf_logistic_Qinv},
	/* Pareto Distribution */
	{"pareto", gsl_ran_pareto_pdf},
	{"paretoP", gsl_cdf_pareto_P},
	{"paretoQ", gsl_cdf_pareto_Q},
	{"paretoPinv", gsl_cdf_pareto_Pinv},
	{"paretoQinv", gsl_cdf_pareto_Qinv},
	/* Weibull Distribution */
	{"weibull", gsl_ran_weibull_pdf},
	{"weibullP", gsl_cdf_weibull_P},
	{"weibullQ", gsl_cdf_weibull_Q},
	{"weibullPinv", gsl_cdf_weibull_Pinv},
	{"weibullQinv", gsl_cdf_weibull_Qinv},
	/* Gumbel Distribution */
	{"gumbel1", gsl_ran_gumbel1_pdf},
	{"gumbel1P", gsl_cdf_gumbel1_P},
	{"gumbel1Q", gsl_cdf_gumbel1_Q},
	{"gumbel1Pinv", gsl_cdf_gumbel1_Pinv},
	{"gumbel1Qinv", gsl_cdf_gumbel1_Qinv},
	{"gumbel2", gsl_ran_gumbel2_pdf},
	{"gumbel2P", gsl_cdf_gumbel2_P},
	{"gumbel2Q", gsl_cdf_gumbel2_Q},
	{"gumbel2Pinv", gsl_cdf_gumbel2_Pinv},
	{"gumbel2Qinv", gsl_cdf_gumbel2_Qinv},
	/* Poisson Distribution */
	{"poisson", nsl_sf_poisson},
	{"poissonP", gsl_cdf_poisson_P},
	{"poissonQ", gsl_cdf_poisson_Q},
	/* Bernoulli Distribution */
	{"bernoulli", nsl_sf_bernoulli},
	/* Binomial Distribution */
	{"binomial", nsl_sf_binomial},
	{"binomialP", gsl_cdf_binomial_P},
	{"binomialQ", gsl_cdf_binomial_Q},
	{"negative_binomial", nsl_sf_negative_binomial},
	{"negative_binomialP", gsl_cdf_negative_binomial_P},
	{"negative_binomialQ", gsl_cdf_negative_binomial_Q},
	/* Pascal Distribution */
	{"pascal", nsl_sf_pascal},
	{"pascalP", gsl_cdf_pascal_P},
	{"pascalQ", gsl_cdf_pascal_Q},
	/* Geometric Distribution */
	{"geometric", nsl_sf_geometric},
	{"geometricP", gsl_cdf_geometric_P},
	{"geometricQ", gsl_cdf_geometric_Q},
	/* Hypergeometric Distribution */
	{"hypergeometric", nsl_sf_hypergeometric},
	{"hypergeometricP", gsl_cdf_hypergeometric_P},
	{"hypergeometricQ", gsl_cdf_hypergeometric_Q},
	/* Logarithmic Distribution */
	{"logarithmic", nsl_sf_logarithmic},
	{0, 0}
};

#endif /*FUNCTIONS_H*/
