/***************************************************************************
    File                 : functions.h
    Project              : LabPlot
    Description          : definition of functions
    --------------------------------------------------------------------
    Copyright            : (C) 2014 by Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2014-2018 by Stefan Gerlach (stefan.gerlach@uni.kn)

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
#ifdef HAVE_LIBCERF
#include <cerf.h>
#endif
#include "backend/nsl/nsl_sf_basic.h"

#ifdef _MSC_VER
/* avoid intrinsics */
#pragma function(ceil, floor)
#endif

/* sync with ExpressionParser.cpp */
struct func _functions[] = {
	/* standard functions */
	{"rand", (func_t)nsl_sf_rand},
	{"random", (func_t)nsl_sf_random},
	{"drand", (func_t)nsl_sf_drand},
	/* math.h */
	{"ceil", (func_t)ceil},
	{"fabs", (func_t)fabs},
	{"log10", (func_t)log10},
	{"pow", (func_t)pow},
	{"sqrt", (func_t)sqrt},
	{"sgn", (func_t)nsl_sf_sgn},
	{"theta", (func_t)nsl_sf_theta},
	{"harmonic", (func_t)nsl_sf_harmonic},
#ifndef _WIN32
	{"cbrt", (func_t)cbrt},
	{"logb", (func_t)logb},
	{"rint", (func_t)rint},
	{"round", (func_t)round},
	{"trunc", (func_t)trunc},
#endif
	/* GSL mathematical functions: see http://www.gnu.org/software/gsl/manual/gsl-ref.html#Mathematical-Functions */
	{"log1p", (func_t)gsl_log1p},
	{"ldexp", (func_t)nsl_sf_ldexp},
	{"powint", (func_t)nsl_sf_powint},
	{"pow2", (func_t)gsl_pow_2},
	{"pow3", (func_t)gsl_pow_3},
	{"pow4", (func_t)gsl_pow_4},
	{"pow5", (func_t)gsl_pow_5},
	{"pow6", (func_t)gsl_pow_6},
	{"pow7", (func_t)gsl_pow_7},
	{"pow8", (func_t)gsl_pow_8},
	{"pow9", (func_t)gsl_pow_9},
	/* GSL special functions: see http://www.gnu.org/software/gsl/manual/html_node/Special-Functions.html */
	/* Airy Functions and Derivatives */
	{"Ai", (func_t)nsl_sf_airy_Ai},
	{"Bi", (func_t)nsl_sf_airy_Bi},
	{"Ais", (func_t)nsl_sf_airy_Ais},
	{"Bis", (func_t)nsl_sf_airy_Bis},
	{"Aid", (func_t)nsl_sf_airy_Aid},
	{"Bid", (func_t)nsl_sf_airy_Bid},
	{"Aids", (func_t)nsl_sf_airy_Aids},
	{"Bids", (func_t)nsl_sf_airy_Bids},
	{"Ai0", (func_t)nsl_sf_airy_0_Ai},
	{"Bi0", (func_t)nsl_sf_airy_0_Bi},
	{"Aid0", (func_t)nsl_sf_airy_0_Aid},
	{"Bid0", (func_t)nsl_sf_airy_0_Bid},
	/* Bessel Functions */
	{"J0", (func_t)gsl_sf_bessel_J0},
	{"J1", (func_t)gsl_sf_bessel_J1},
	{"Jn", (func_t)nsl_sf_bessel_Jn},
	{"Y0", (func_t)gsl_sf_bessel_Y0},
	{"Y1", (func_t)gsl_sf_bessel_Y1},
	{"Yn", (func_t)nsl_sf_bessel_Yn},
	{"I0", (func_t)gsl_sf_bessel_I0},
	{"I1", (func_t)gsl_sf_bessel_I1},
	{"In", (func_t)nsl_sf_bessel_In},
	{"I0s", (func_t)gsl_sf_bessel_I0_scaled},
	{"I1s", (func_t)gsl_sf_bessel_I1_scaled},
	{"Ins", (func_t)nsl_sf_bessel_Ins},
	{"K0", (func_t)gsl_sf_bessel_K0},
	{"K1", (func_t)gsl_sf_bessel_K1},
	{"Kn", (func_t)nsl_sf_bessel_Kn},
	{"K0s", (func_t)gsl_sf_bessel_K0_scaled},
	{"K1s", (func_t)gsl_sf_bessel_K1_scaled},
	{"Kns", (func_t)nsl_sf_bessel_Kns},
	{"j0", (func_t)gsl_sf_bessel_j0},
	{"j1", (func_t)gsl_sf_bessel_j1},
	{"j2", (func_t)gsl_sf_bessel_j2},
	{"jl", (func_t)nsl_sf_bessel_jl},
	{"y0", (func_t)gsl_sf_bessel_y0},
	{"y1", (func_t)gsl_sf_bessel_y1},
	{"y2", (func_t)gsl_sf_bessel_y2},
	{"yl", (func_t)nsl_sf_bessel_yl},
	{"i0s", (func_t)gsl_sf_bessel_i0_scaled},
	{"i1s", (func_t)gsl_sf_bessel_i1_scaled},
	{"i2s", (func_t)gsl_sf_bessel_i2_scaled},
	{"ils", (func_t)nsl_sf_bessel_ils},
	{"k0s", (func_t)gsl_sf_bessel_k0_scaled},
	{"k1s", (func_t)gsl_sf_bessel_k1_scaled},
	{"k2s", (func_t)gsl_sf_bessel_k2_scaled},
	{"kls", (func_t)nsl_sf_bessel_kls},
	{"Jnu", (func_t)gsl_sf_bessel_Jnu},
	{"Ynu", (func_t)gsl_sf_bessel_Ynu},
	{"Inu", (func_t)gsl_sf_bessel_Inu},
	{"Inus", (func_t)gsl_sf_bessel_Inu_scaled},
	{"Knu", (func_t)gsl_sf_bessel_Knu},
	{"lnKnu", (func_t)gsl_sf_bessel_lnKnu},
	{"Knus", (func_t)gsl_sf_bessel_Knu_scaled},
	{"J0_0", (func_t)nsl_sf_bessel_0_J0},
	{"J1_0", (func_t)nsl_sf_bessel_0_J1},
	{"Jnu_0", (func_t)nsl_sf_bessel_0_Jnu},
	/* Clausen functions */
	{"clausen", (func_t)gsl_sf_clausen},
	/* Coulomb functions */
	{"hydrogenicR_1", (func_t)gsl_sf_hydrogenicR_1},
	{"hydrogenicR", (func_t)nsl_sf_hydrogenicR},
	{"dawson", (func_t)gsl_sf_dawson},
	/* Debye functions */
	{"D1", (func_t)gsl_sf_debye_1},
	{"D2", (func_t)gsl_sf_debye_2},
	{"D3", (func_t)gsl_sf_debye_3},
	{"D4", (func_t)gsl_sf_debye_4},
	{"D5", (func_t)gsl_sf_debye_5},
	{"D6", (func_t)gsl_sf_debye_6},
	{"Li2", (func_t)gsl_sf_dilog},
	/* Elliptic integrals */
	{"Kc", (func_t)nsl_sf_ellint_Kc},
	{"Ec", (func_t)nsl_sf_ellint_Ec},
	{"Pc", (func_t)nsl_sf_ellint_Pc},
	{"F", (func_t)nsl_sf_ellint_F},
	{"E", (func_t)nsl_sf_ellint_E},
	{"P", (func_t)nsl_sf_ellint_P},
	{"D", (func_t)nsl_sf_ellint_D},
	{"RC", (func_t)nsl_sf_ellint_RC},
	{"RD", (func_t)nsl_sf_ellint_RD},
	{"RF", (func_t)nsl_sf_ellint_RF},
	{"RJ", (func_t)nsl_sf_ellint_RJ},
	/* Error functions */
	{"erf", (func_t)gsl_sf_erf},
	{"erfc", (func_t)gsl_sf_erfc},
	{"log_erfc", (func_t)gsl_sf_log_erfc},
	{"erf_Z", (func_t)gsl_sf_erf_Z},
	{"erf_Q", (func_t)gsl_sf_erf_Q},
	{"hazard", (func_t)gsl_sf_hazard},
#ifdef HAVE_LIBCERF
	{"erfcx", (func_t)erfcx},
	{"erfi", (func_t)erfi},
	{"im_w_of_x", (func_t)im_w_of_x},
	{"dawson", (func_t)dawson},
#endif
#ifndef _MSC_VER
	{"voigt", (func_t)nsl_sf_voigt},
#endif
	/* Exponential Functions */
	{"exp", (func_t)gsl_sf_exp},
	{"exp_mult", (func_t)gsl_sf_exp_mult},
	{"expm1", (func_t)gsl_expm1},
	{"exprel", (func_t)gsl_sf_exprel},
	{"exprel2", (func_t)gsl_sf_exprel_2},
	{"expreln", (func_t)nsl_sf_exprel_n},
	/* Exponential Integrals */
	{"E1", (func_t)gsl_sf_expint_E1},
	{"E2", (func_t)gsl_sf_expint_E2},
	{"En", (func_t)gsl_sf_expint_En},
	{"Ei", (func_t)gsl_sf_expint_Ei},
	{"Shi", (func_t)gsl_sf_Shi},
	{"Chi", (func_t)gsl_sf_Chi},
	{"Ei3", (func_t)gsl_sf_expint_3},
	{"Si", (func_t)gsl_sf_Si},
	{"Ci", (func_t)gsl_sf_Ci},
	{"Atanint", (func_t)gsl_sf_atanint},
	/* Fermi-Dirac Function */
	{"Fm1", (func_t)gsl_sf_fermi_dirac_m1},
	{"F0", (func_t)gsl_sf_fermi_dirac_0},
	{"F1", (func_t)gsl_sf_fermi_dirac_1},
	{"F2", (func_t)gsl_sf_fermi_dirac_2},
	{"Fj", (func_t)nsl_sf_fermi_dirac_int},
	{"Fmhalf", (func_t)gsl_sf_fermi_dirac_mhalf},
	{"Fhalf", (func_t)gsl_sf_fermi_dirac_half},
	{"F3half", (func_t)gsl_sf_fermi_dirac_3half},
	{"Finc0", (func_t)gsl_sf_fermi_dirac_inc_0},
	/* Gamma and Beta Functions */
	{"gamma", (func_t)gsl_sf_gamma},
	{"tgamma", (func_t)gsl_sf_gamma},
	{"lgamma", (func_t)gsl_sf_lngamma},
	{"lngamma", (func_t)gsl_sf_lngamma},
	{"gammastar", (func_t)gsl_sf_gammastar},
	{"gammainv", (func_t)gsl_sf_gammainv},
	{"fact", (func_t)nsl_sf_fact},
	{"doublefact", (func_t)nsl_sf_doublefact},
	{"lnfact", (func_t)nsl_sf_lnfact},
	{"lndoublefact", (func_t)nsl_sf_lndoublefact},
	{"choose", (func_t)nsl_sf_choose},
	{"lnchoose", (func_t)nsl_sf_lnchoose},
	{"taylor", (func_t)nsl_sf_taylorcoeff},
	{"poch", (func_t)gsl_sf_poch},
	{"lnpoch", (func_t)gsl_sf_lnpoch},
	{"pochrel", (func_t)gsl_sf_pochrel},
	{"gammainc", (func_t)gsl_sf_gamma_inc},
	{"gammaincQ", (func_t)gsl_sf_gamma_inc_Q},
	{"gammaincP", (func_t)gsl_sf_gamma_inc_P},
	{"beta", (func_t)gsl_sf_beta},
	{"lnbeta", (func_t)gsl_sf_lnbeta},
	{"betainc", (func_t)gsl_sf_beta_inc},
	/* Gegenbauer Functions */
	{"C1", (func_t)gsl_sf_gegenpoly_1},
	{"C2", (func_t)gsl_sf_gegenpoly_2},
	{"C3", (func_t)gsl_sf_gegenpoly_3},
	{"Cn", (func_t)nsl_sf_gegenpoly_n},
#if (GSL_MAJOR_VERSION > 2) || (GSL_MAJOR_VERSION == 2) && (GSL_MINOR_VERSION >= 4)
	/* Hermite polynomials and functions */
	{"Hen", (func_t)nsl_sf_hermite_prob},
	{"Hn", (func_t)nsl_sf_hermite_phys},
	{"Hfn", (func_t)nsl_sf_hermite_func},
	{"Hend", (func_t)nsl_sf_hermite_prob_der},
	{"Hnd", (func_t)nsl_sf_hermite_phys_der},
	{"Hfnd", (func_t)nsl_sf_hermite_func_der},
#endif
	/* Hypergeometric Functions */
	{"hyperg_0F1", (func_t)gsl_sf_hyperg_0F1},
	{"hyperg_1F1i", (func_t)nsl_sf_hyperg_1F1i},
	{"hyperg_1F1", (func_t)gsl_sf_hyperg_1F1},
	{"hyperg_Ui", (func_t)nsl_sf_hyperg_Ui},
	{"hyperg_U", (func_t)gsl_sf_hyperg_U},
	{"hyperg_2F1", (func_t)gsl_sf_hyperg_2F1},
	{"hyperg_2F1c", (func_t)gsl_sf_hyperg_2F1_conj},
	{"hyperg_2F1r", (func_t)gsl_sf_hyperg_2F1_renorm},
	{"hyperg_2F1cr", (func_t)gsl_sf_hyperg_2F1_conj_renorm},
	{"hyperg_2F0", (func_t)gsl_sf_hyperg_2F0},
	/* Laguerre Functions */
	{"L1", (func_t)gsl_sf_laguerre_1},
	{"L2", (func_t)gsl_sf_laguerre_2},
	{"L3", (func_t)gsl_sf_laguerre_3},
	/* Lambert W Functions */
	{"W0", (func_t)gsl_sf_lambert_W0},
	{"Wm1", (func_t)gsl_sf_lambert_Wm1},
	/* Legendre Functions and Spherical Harmonics */
	{"P1", (func_t)gsl_sf_legendre_P1},
	{"P2", (func_t)gsl_sf_legendre_P2},
	{"P3", (func_t)gsl_sf_legendre_P3},
	{"Pl", (func_t)nsl_sf_legendre_Pl},
	{"Q0", (func_t)gsl_sf_legendre_Q0},
	{"Q1", (func_t)gsl_sf_legendre_Q1},
	{"Ql", (func_t)nsl_sf_legendre_Ql},
	{"Plm", (func_t)nsl_sf_legendre_Plm},
	{"Pslm", (func_t)nsl_sf_legendre_sphPlm},
	{"Phalf", (func_t)gsl_sf_conicalP_half},
	{"Pmhalf", (func_t)gsl_sf_conicalP_mhalf},
	{"Pc0", (func_t)gsl_sf_conicalP_0},
	{"Pc1", (func_t)gsl_sf_conicalP_1},
	{"Psr", (func_t)nsl_sf_conicalP_sphreg},
	{"Pcr", (func_t)nsl_sf_conicalP_cylreg},
	{"H3d0", (func_t)gsl_sf_legendre_H3d_0},
	{"H3d1", (func_t)gsl_sf_legendre_H3d_1},
	{"H3d", (func_t)nsl_sf_legendre_H3d},
	/* Logarithm and Related Functions */
	{"log", (func_t)gsl_sf_log},
	{"logabs", (func_t)gsl_sf_log_abs},
	{"logp", (func_t)gsl_sf_log_1plusx},
	{"logpm", (func_t)gsl_sf_log_1plusx_mx},
	/* Power Function */
	{"gsl_powint", (func_t)nsl_sf_powint},
	/* Psi (Digamma) Function */
	{"psiint", (func_t)nsl_sf_psiint},
	{"psi", (func_t)gsl_sf_psi},
	{"psi1piy", (func_t)gsl_sf_psi_1piy},
	{"psi1int", (func_t)nsl_sf_psi1int},
	{"psi1", (func_t)gsl_sf_psi_1},
	{"psin", (func_t)nsl_sf_psin},
	/* Synchrotron Functions */
	{"synchrotron1", (func_t)gsl_sf_synchrotron_1},
	{"synchrotron2", (func_t)gsl_sf_synchrotron_2},
	/* Transport Functions */
	{"J2", (func_t)gsl_sf_transport_2},
	{"J3", (func_t)gsl_sf_transport_3},
	{"J4", (func_t)gsl_sf_transport_4},
	{"J5", (func_t)gsl_sf_transport_5},
	/* Trigonometric Functions */
	{"sin", (func_t)gsl_sf_sin},
	{"cos", (func_t)gsl_sf_cos},
	{"tan", (func_t)tan},
	{"asin", (func_t)asin},
	{"acos", (func_t)acos},
	{"atan", (func_t)atan},
	{"atan2", (func_t)atan2},
	{"sinh", (func_t)sinh},
	{"cosh", (func_t)cosh},
	{"tanh", (func_t)tanh},
	{"acosh", (func_t)gsl_acosh},
	{"asinh", (func_t)gsl_asinh},
	{"atanh", (func_t)gsl_atanh},
	{"sec", (func_t)nsl_sf_sec},
	{"csc", (func_t)nsl_sf_csc},
	{"cot", (func_t)nsl_sf_cot},
	{"asec", (func_t)nsl_sf_asec},
	{"acsc", (func_t)nsl_sf_acsc},
	{"acot", (func_t)nsl_sf_acot},
	{"sech", (func_t)nsl_sf_sech},
	{"csch", (func_t)nsl_sf_csch},
	{"coth", (func_t)nsl_sf_coth},
	{"asech", (func_t)nsl_sf_asech},
	{"acsch", (func_t)nsl_sf_acsch},
	{"acoth", (func_t)nsl_sf_acoth},
	{"sinc", (func_t)gsl_sf_sinc},
	{"logsinh", (func_t)gsl_sf_lnsinh},
	{"logcosh", (func_t)gsl_sf_lncosh},
	{"hypot", (func_t)gsl_sf_hypot},
	{"hypot3", (func_t)gsl_hypot3},
	{"anglesymm", (func_t)gsl_sf_angle_restrict_symm},
	{"anglepos", (func_t)gsl_sf_angle_restrict_pos},
	/* Zeta Functions */
	{"zetaint", (func_t)nsl_sf_zetaint},
	{"zeta", (func_t)gsl_sf_zeta},
	{"zetam1int", (func_t)nsl_sf_zetam1int},
	{"zetam1", (func_t)gsl_sf_zetam1},
	{"hzeta", (func_t)gsl_sf_hzeta},
	{"etaint", (func_t)nsl_sf_etaint},
	{"eta", (func_t)gsl_sf_eta},

	/* GSL Random Number Distributions: see http://www.gnu.org/software/gsl/manual/html_node/Random-Number-Distributions.html */
	/* Gaussian Distribution */
	{"gaussian", (func_t)gsl_ran_gaussian_pdf},
	{"ugaussian", (func_t)gsl_ran_ugaussian_pdf},
	{"gaussianP", (func_t)gsl_cdf_gaussian_P},
	{"gaussianQ", (func_t)gsl_cdf_gaussian_Q},
	{"gaussianPinv", (func_t)gsl_cdf_gaussian_Pinv},
	{"gaussianQinv", (func_t)gsl_cdf_gaussian_Qinv},
	{"ugaussianP", (func_t)gsl_cdf_ugaussian_P},
	{"ugaussianQ", (func_t)gsl_cdf_ugaussian_Q},
	{"ugaussianPinv", (func_t)gsl_cdf_ugaussian_Pinv},
	{"ugaussianQinv", (func_t)gsl_cdf_ugaussian_Qinv},
	{"gaussiantail", (func_t)gsl_ran_gaussian_tail_pdf},
	{"ugaussiantail", (func_t)gsl_ran_ugaussian_tail_pdf},
	{"gaussianbi", (func_t)gsl_ran_bivariate_gaussian_pdf},
	/* Exponential Distribution */
	{"exponential", (func_t)gsl_ran_exponential_pdf},
	{"exponentialP", (func_t)gsl_cdf_exponential_P},
	{"exponentialQ", (func_t)gsl_cdf_exponential_Q},
	{"exponentialPinv", (func_t)gsl_cdf_exponential_Pinv},
	{"exponentialQinv", (func_t)gsl_cdf_exponential_Qinv},
	/* Laplace Distribution */
	{"laplace", (func_t)gsl_ran_laplace_pdf},
	{"laplaceP", (func_t)gsl_cdf_laplace_P},
	{"laplaceQ", (func_t)gsl_cdf_laplace_Q},
	{"laplacePinv", (func_t)gsl_cdf_laplace_Pinv},
	{"laplaceQinv", (func_t)gsl_cdf_laplace_Qinv},
	/* Exponential Power Distribution */
	{"exppow", (func_t)gsl_ran_exppow_pdf},
	{"exppowP", (func_t)gsl_cdf_exppow_P},
	{"exppowQ", (func_t)gsl_cdf_exppow_Q},
	/* Cauchy Distribution */
	{"cauchy", (func_t)gsl_ran_cauchy_pdf},
	{"cauchyP", (func_t)gsl_cdf_cauchy_P},
	{"cauchyQ", (func_t)gsl_cdf_cauchy_Q},
	{"cauchyPinv", (func_t)gsl_cdf_cauchy_Pinv},
	{"cauchyQinv", (func_t)gsl_cdf_cauchy_Qinv},
	/* Rayleigh Distribution */
	{"rayleigh", (func_t)gsl_ran_rayleigh_pdf},
	{"rayleighP", (func_t)gsl_cdf_rayleigh_P},
	{"rayleighQ", (func_t)gsl_cdf_rayleigh_Q},
	{"rayleighPinv", (func_t)gsl_cdf_rayleigh_Pinv},
	{"rayleighQinv", (func_t)gsl_cdf_rayleigh_Qinv},
	{"rayleigh_tail", (func_t)gsl_ran_rayleigh_tail_pdf},
	/* Landau Distribution */
	{"landau", (func_t)gsl_ran_landau_pdf},
	/* Gamma Distribution */
	{"gammapdf", (func_t)gsl_ran_gamma_pdf},
	{"gammaP", (func_t)gsl_cdf_gamma_P},
	{"gammaQ", (func_t)gsl_cdf_gamma_Q},
	{"gammaPinv", (func_t)gsl_cdf_gamma_Pinv},
	{"gammaQinv", (func_t)gsl_cdf_gamma_Qinv},
	/* Flat (Uniform) Distribution */
	{"flat", (func_t)gsl_ran_flat_pdf},
	{"flatP", (func_t)gsl_cdf_flat_P},
	{"flatQ", (func_t)gsl_cdf_flat_Q},
	{"flatPinv", (func_t)gsl_cdf_flat_Pinv},
	{"flatQinv", (func_t)gsl_cdf_flat_Qinv},
	/* Lognormal Distribution */
	{"lognormal", (func_t)gsl_ran_lognormal_pdf},
	{"lognormalP", (func_t)gsl_cdf_lognormal_P},
	{"lognormalQ", (func_t)gsl_cdf_lognormal_Q},
	{"lognormalPinv", (func_t)gsl_cdf_lognormal_Pinv},
	{"lognormalQinv", (func_t)gsl_cdf_lognormal_Qinv},
	/* Chi-squared Distribution */
	{"chisq", (func_t)gsl_ran_chisq_pdf},
	{"chisqP", (func_t)gsl_cdf_chisq_P},
	{"chisqQ", (func_t)gsl_cdf_chisq_Q},
	{"chisqPinv", (func_t)gsl_cdf_chisq_Pinv},
	{"chisqQinv", (func_t)gsl_cdf_chisq_Qinv},
	/* F-distribution */
	{"fdist", (func_t)gsl_ran_fdist_pdf},
	{"fdistP", (func_t)gsl_cdf_fdist_P},
	{"fdistQ", (func_t)gsl_cdf_fdist_Q},
	{"fdistPinv", (func_t)gsl_cdf_fdist_Pinv},
	{"fdistQinv", (func_t)gsl_cdf_fdist_Qinv},
	/* t-distribution */
	{"tdist", (func_t)gsl_ran_tdist_pdf},
	{"tdistP", (func_t)gsl_cdf_tdist_P},
	{"tdistQ", (func_t)gsl_cdf_tdist_Q},
	{"tdistPinv", (func_t)gsl_cdf_tdist_Pinv},
	{"tdistQinv", (func_t)gsl_cdf_tdist_Qinv},
	/* Beta Distribution */
	{"betapdf", (func_t)gsl_ran_beta_pdf},
	{"betaP", (func_t)gsl_cdf_beta_P},
	{"betaQ", (func_t)gsl_cdf_beta_Q},
	{"betaPinv", (func_t)gsl_cdf_beta_Pinv},
	{"betaQinv", (func_t)gsl_cdf_beta_Qinv},
	/* Logistic Distribution */
	{"logistic", (func_t)gsl_ran_logistic_pdf},
	{"logisticP", (func_t)gsl_cdf_logistic_P},
	{"logisticQ", (func_t)gsl_cdf_logistic_Q},
	{"logisticPinv", (func_t)gsl_cdf_logistic_Pinv},
	{"logisticQinv", (func_t)gsl_cdf_logistic_Qinv},
	/* Pareto Distribution */
	{"pareto", (func_t)gsl_ran_pareto_pdf},
	{"paretoP", (func_t)gsl_cdf_pareto_P},
	{"paretoQ", (func_t)gsl_cdf_pareto_Q},
	{"paretoPinv", (func_t)gsl_cdf_pareto_Pinv},
	{"paretoQinv", (func_t)gsl_cdf_pareto_Qinv},
	/* Weibull Distribution */
	{"weibull", (func_t)gsl_ran_weibull_pdf},
	{"weibullP", (func_t)gsl_cdf_weibull_P},
	{"weibullQ", (func_t)gsl_cdf_weibull_Q},
	{"weibullPinv", (func_t)gsl_cdf_weibull_Pinv},
	{"weibullQinv", (func_t)gsl_cdf_weibull_Qinv},
	/* Gumbel Distribution */
	{"gumbel1", (func_t)gsl_ran_gumbel1_pdf},
	{"gumbel1P", (func_t)gsl_cdf_gumbel1_P},
	{"gumbel1Q", (func_t)gsl_cdf_gumbel1_Q},
	{"gumbel1Pinv", (func_t)gsl_cdf_gumbel1_Pinv},
	{"gumbel1Qinv", (func_t)gsl_cdf_gumbel1_Qinv},
	{"gumbel2", (func_t)gsl_ran_gumbel2_pdf},
	{"gumbel2P", (func_t)gsl_cdf_gumbel2_P},
	{"gumbel2Q", (func_t)gsl_cdf_gumbel2_Q},
	{"gumbel2Pinv", (func_t)gsl_cdf_gumbel2_Pinv},
	{"gumbel2Qinv", (func_t)gsl_cdf_gumbel2_Qinv},
	/* Poisson Distribution */
	{"poisson", (func_t)nsl_sf_poisson},
	{"poissonP", (func_t)gsl_cdf_poisson_P},
	{"poissonQ", (func_t)gsl_cdf_poisson_Q},
	/* Bernoulli Distribution */
	{"bernoulli", (func_t)nsl_sf_bernoulli},
	/* Binomial Distribution */
	{"binomial", (func_t)nsl_sf_binomial},
	{"binomialP", (func_t)gsl_cdf_binomial_P},
	{"binomialQ", (func_t)gsl_cdf_binomial_Q},
	{"negative_binomial", (func_t)nsl_sf_negative_binomial},
	{"negative_binomialP", (func_t)gsl_cdf_negative_binomial_P},
	{"negative_binomialQ", (func_t)gsl_cdf_negative_binomial_Q},
	/* Pascal Distribution */
	{"pascal", (func_t)nsl_sf_pascal},
	{"pascalP", (func_t)gsl_cdf_pascal_P},
	{"pascalQ", (func_t)gsl_cdf_pascal_Q},
	/* Geometric Distribution */
	{"geometric", (func_t)nsl_sf_geometric},
	{"geometricP", (func_t)gsl_cdf_geometric_P},
	{"geometricQ", (func_t)gsl_cdf_geometric_Q},
	/* Hypergeometric Distribution */
	{"hypergeometric", (func_t)nsl_sf_hypergeometric},
	{"hypergeometricP", (func_t)gsl_cdf_hypergeometric_P},
	{"hypergeometricQ", (func_t)gsl_cdf_hypergeometric_Q},
	/* Logarithmic Distribution */
	{"logarithmic", (func_t)nsl_sf_logarithmic},
	{0, (func_t)0}
};

#endif /*FUNCTIONS_H*/
