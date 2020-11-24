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
#include "backend/nsl/nsl_sf_basic.h"

#ifdef _MSC_VER
/* avoid intrinsics */
#pragma function(ceil, floor)
#endif

/* list of functions (sync with ExpressionParser.cpp!) */
struct funs _functions[] = {
	/* standard functions */
	{"rand", (func_t)nsl_sf_rand, 0},
	{"random", (func_t)nsl_sf_random, 0},
	{"drand", (func_t)nsl_sf_drand, 0},
	/* math.h */
	{"ceil", (func_t)ceil, 1},
	{"fabs", (func_t)fabs, 1},
	{"log10", (func_t)log10, 1},
	{"pow", (func_t)pow, 2},
	{"sqrt", (func_t)sqrt, 1},
	{"sgn", (func_t)nsl_sf_sgn, 1},
	{"theta", (func_t)nsl_sf_theta, 1},
	{"harmonic", (func_t)nsl_sf_harmonic, 1},
#ifndef _WIN32
	{"cbrt", (func_t)cbrt, 1},
	{"logb", (func_t)logb, 1},
	{"rint", (func_t)rint, 1},
	{"round", (func_t)round, 1},
	{"trunc", (func_t)trunc, 1},
#endif
	/* GSL mathematical functions: see https://www.gnu.org/software/gsl/doc/html/math.html */
	{"log1p", (func_t)gsl_log1p, 1},
	{"ldexp", (func_t)nsl_sf_ldexp, 2},
	{"powint", (func_t)nsl_sf_powint, 2},
	{"pow2", (func_t)gsl_pow_2, 1},
	{"pow3", (func_t)gsl_pow_3, 1},
	{"pow4", (func_t)gsl_pow_4, 1},
	{"pow5", (func_t)gsl_pow_5, 1},
	{"pow6", (func_t)gsl_pow_6, 1},
	{"pow7", (func_t)gsl_pow_7, 1},
	{"pow8", (func_t)gsl_pow_8, 1},
	{"pow9", (func_t)gsl_pow_9, 1},
	/* GSL special functions: see https://www.gnu.org/software/gsl/doc/html/specfunc.html */
	/* Airy Functions and Derivatives */
	{"Ai", (func_t)nsl_sf_airy_Ai, 1},
	{"Bi", (func_t)nsl_sf_airy_Bi, 1},
	{"Ais", (func_t)nsl_sf_airy_Ais, 1},
	{"Bis", (func_t)nsl_sf_airy_Bis, 1},
	{"Aid", (func_t)nsl_sf_airy_Aid, 1},
	{"Bid", (func_t)nsl_sf_airy_Bid, 1},
	{"Aids", (func_t)nsl_sf_airy_Aids, 1},
	{"Bids", (func_t)nsl_sf_airy_Bids, 1},
	{"Ai0", (func_t)nsl_sf_airy_0_Ai, 1},
	{"Bi0", (func_t)nsl_sf_airy_0_Bi, 1 },
	{"Aid0", (func_t)nsl_sf_airy_0_Aid, 1},
	{"Bid0", (func_t)nsl_sf_airy_0_Bid, 1},
	/* Bessel Functions */
	{"J0", (func_t)gsl_sf_bessel_J0, 1},
	{"J1", (func_t)gsl_sf_bessel_J1, 1},
	{"Jn", (func_t)nsl_sf_bessel_Jn, 2},
	{"Y0", (func_t)gsl_sf_bessel_Y0, 1},
	{"Y1", (func_t)gsl_sf_bessel_Y1, 1},
	{"Yn", (func_t)nsl_sf_bessel_Yn, 2},
	{"I0", (func_t)gsl_sf_bessel_I0, 1},
	{"I1", (func_t)gsl_sf_bessel_I1, 1},
	{"In", (func_t)nsl_sf_bessel_In, 2},
	{"I0s", (func_t)gsl_sf_bessel_I0_scaled, 1},
	{"I1s", (func_t)gsl_sf_bessel_I1_scaled, 1},
	{"Ins", (func_t)nsl_sf_bessel_Ins, 2},
	{"K0", (func_t)gsl_sf_bessel_K0, 1},
	{"K1", (func_t)gsl_sf_bessel_K1, 1},
	{"Kn", (func_t)nsl_sf_bessel_Kn, 2},
	{"K0s", (func_t)gsl_sf_bessel_K0_scaled, 1},
	{"K1s", (func_t)gsl_sf_bessel_K1_scaled, 1},
	{"Kns", (func_t)nsl_sf_bessel_Kns, 2},
	{"j0", (func_t)gsl_sf_bessel_j0, 1},
	{"j1", (func_t)gsl_sf_bessel_j1, 1},
	{"j2", (func_t)gsl_sf_bessel_j2, 1},
	{"jl", (func_t)nsl_sf_bessel_jl, 2},
	{"y0", (func_t)gsl_sf_bessel_y0, 1},
	{"y1", (func_t)gsl_sf_bessel_y1, 1},
	{"y2", (func_t)gsl_sf_bessel_y2, 1},
	{"yl", (func_t)nsl_sf_bessel_yl, 2},
	{"i0s", (func_t)gsl_sf_bessel_i0_scaled, 1},
	{"i1s", (func_t)gsl_sf_bessel_i1_scaled, 1},
	{"i2s", (func_t)gsl_sf_bessel_i2_scaled, 1},
	{"ils", (func_t)nsl_sf_bessel_ils, 2},
	{"k0s", (func_t)gsl_sf_bessel_k0_scaled, 1},
	{"k1s", (func_t)gsl_sf_bessel_k1_scaled, 1},
	{"k2s", (func_t)gsl_sf_bessel_k2_scaled, 1},
	{"kls", (func_t)nsl_sf_bessel_kls, 2},
	{"Jnu", (func_t)gsl_sf_bessel_Jnu, 2},
	{"Ynu", (func_t)gsl_sf_bessel_Ynu, 2},
	{"Inu", (func_t)gsl_sf_bessel_Inu, 2},
	{"Inus", (func_t)gsl_sf_bessel_Inu_scaled, 2},
	{"Knu", (func_t)gsl_sf_bessel_Knu, 2},
	{"lnKnu", (func_t)gsl_sf_bessel_lnKnu, 2},
	{"Knus", (func_t)gsl_sf_bessel_Knu_scaled, 2},
	{"J0_0", (func_t)nsl_sf_bessel_0_J0, 1},
	{"J1_0", (func_t)nsl_sf_bessel_0_J1, 1},
	{"Jnu_0", (func_t)nsl_sf_bessel_0_Jnu, 2},
	/* Clausen functions */
	{"clausen", (func_t)gsl_sf_clausen, 1},
	/* Coulomb functions */
	{"hydrogenicR_1", (func_t)gsl_sf_hydrogenicR_1, 2},
	{"hydrogenicR", (func_t)nsl_sf_hydrogenicR, 4},
	{"dawson", (func_t)gsl_sf_dawson, 1},
	/* Debye functions */
	{"D1", (func_t)gsl_sf_debye_1, 1},
	{"D2", (func_t)gsl_sf_debye_2, 1},
	{"D3", (func_t)gsl_sf_debye_3, 1},
	{"D4", (func_t)gsl_sf_debye_4, 1},
	{"D5", (func_t)gsl_sf_debye_5, 1},
	{"D6", (func_t)gsl_sf_debye_6, 1},
	{"Li2", (func_t)gsl_sf_dilog, 1},
	/* Elliptic integrals */
	{"Kc", (func_t)nsl_sf_ellint_Kc, 1},
	{"Ec", (func_t)nsl_sf_ellint_Ec, 1},
	{"Pc", (func_t)nsl_sf_ellint_Pc, 2},
	{"F", (func_t)nsl_sf_ellint_F, 2},
	{"E", (func_t)nsl_sf_ellint_E, 2},
	{"P", (func_t)nsl_sf_ellint_P, 3},
	{"D", (func_t)nsl_sf_ellint_D, 2},
	{"RC", (func_t)nsl_sf_ellint_RC, 2},
	{"RD", (func_t)nsl_sf_ellint_RD, 3},
	{"RF", (func_t)nsl_sf_ellint_RF, 3},
	{"RJ", (func_t)nsl_sf_ellint_RJ, 4},
	/* Error functions */
	{"erf", (func_t)gsl_sf_erf, 1},
	{"erfc", (func_t)gsl_sf_erfc, 1},
	{"log_erfc", (func_t)gsl_sf_log_erfc, 1},
	{"erf_Z", (func_t)gsl_sf_erf_Z, 1},
	{"erf_Q", (func_t)gsl_sf_erf_Q, 1},
	{"hazard", (func_t)gsl_sf_hazard, 1},
#ifndef _MSC_VER
	{"erfcx", (func_t)nsl_sf_erfcx, 1},
	{"erfi", (func_t)nsl_sf_erfi, 1},
	{"im_w_of_x", (func_t)nsl_sf_im_w_of_x, 1},
	{"dawson", (func_t)nsl_sf_dawson, 1},
	{"voigt", (func_t)nsl_sf_voigt, 3},
#endif
	{"pseudovoigt1", (func_t)nsl_sf_pseudovoigt1, 3},
	/* Exponential Functions */
	{"exp", (func_t)gsl_sf_exp, 1},
	{"exp_mult", (func_t)gsl_sf_exp_mult, 2},
	{"expm1", (func_t)gsl_expm1, 1},
	{"exprel", (func_t)gsl_sf_exprel, 1},
	{"exprel2", (func_t)gsl_sf_exprel_2, 1},
	{"expreln", (func_t)nsl_sf_exprel_n, 2},
	/* Exponential Integrals */
	{"E1", (func_t)gsl_sf_expint_E1, 1},
	{"E2", (func_t)gsl_sf_expint_E2, 1},
	{"En", (func_t)gsl_sf_expint_En, 2},
	{"Ei", (func_t)gsl_sf_expint_Ei, 1},
	{"Shi", (func_t)gsl_sf_Shi, 1},
	{"Chi", (func_t)gsl_sf_Chi, 1},
	{"Ei3", (func_t)gsl_sf_expint_3, 1},
	{"Si", (func_t)gsl_sf_Si, 1},
	{"Ci", (func_t)gsl_sf_Ci, 1},
	{"Atanint", (func_t)gsl_sf_atanint, 1},
	/* Fermi-Dirac Function */
	{"Fm1", (func_t)gsl_sf_fermi_dirac_m1, 1},
	{"F0", (func_t)gsl_sf_fermi_dirac_0, 1},
	{"F1", (func_t)gsl_sf_fermi_dirac_1, 1},
	{"F2", (func_t)gsl_sf_fermi_dirac_2, 1},
	{"Fj", (func_t)nsl_sf_fermi_dirac_int, 2},
	{"Fmhalf", (func_t)gsl_sf_fermi_dirac_mhalf, 1},
	{"Fhalf", (func_t)gsl_sf_fermi_dirac_half, 1},
	{"F3half", (func_t)gsl_sf_fermi_dirac_3half, 1},
	{"Finc0", (func_t)gsl_sf_fermi_dirac_inc_0, 2},
	/* Gamma and Beta Functions */
	{"gamma", (func_t)gsl_sf_gamma, 1},
	{"tgamma", (func_t)gsl_sf_gamma, 1},
	{"lgamma", (func_t)gsl_sf_lngamma, 1},
	{"lngamma", (func_t)gsl_sf_lngamma, 1},
	{"gammastar", (func_t)gsl_sf_gammastar, 1},
	{"gammainv", (func_t)gsl_sf_gammainv, 1},
	{"fact", (func_t)nsl_sf_fact, 1},
	{"doublefact", (func_t)nsl_sf_doublefact, 1},
	{"lnfact", (func_t)nsl_sf_lnfact, 1},
	{"lndoublefact", (func_t)nsl_sf_lndoublefact, 1},
	{"choose", (func_t)nsl_sf_choose, 2},
	{"lnchoose", (func_t)nsl_sf_lnchoose, 2},
	{"taylor", (func_t)nsl_sf_taylorcoeff, 2},
	{"poch", (func_t)gsl_sf_poch, 2},
	{"lnpoch", (func_t)gsl_sf_lnpoch, 2},
	{"pochrel", (func_t)gsl_sf_pochrel, 2},
	{"gammainc", (func_t)gsl_sf_gamma_inc, 2},
	{"gammaincQ", (func_t)gsl_sf_gamma_inc_Q, 2},
	{"gammaincP", (func_t)gsl_sf_gamma_inc_P, 2},
	{"beta", (func_t)gsl_sf_beta, 2},
	{"lnbeta", (func_t)gsl_sf_lnbeta, 2},
	{"betainc", (func_t)gsl_sf_beta_inc, 3},
	/* Gegenbauer Functions */
	{"C1", (func_t)gsl_sf_gegenpoly_1, 2},
	{"C2", (func_t)gsl_sf_gegenpoly_2, 2},
	{"C3", (func_t)gsl_sf_gegenpoly_3, 2},
	{"Cn", (func_t)nsl_sf_gegenpoly_n, 3},
#if (GSL_MAJOR_VERSION > 2) || (GSL_MAJOR_VERSION == 2) && (GSL_MINOR_VERSION >= 4)
	/* Hermite polynomials and functions */
	{"Hn", (func_t)nsl_sf_hermite, 2},
	{"Hen", (func_t)nsl_sf_hermite_prob, 2},
	{"Hfn", (func_t)nsl_sf_hermite_func, 2},
#if (GSL_MAJOR_VERSION > 2) || (GSL_MAJOR_VERSION == 2) && (GSL_MINOR_VERSION >= 6)
	{"Hfnf", (func_t)nsl_sf_hermite_func_fast, 2},
#endif
	{"Hnd", (func_t)nsl_sf_hermite_deriv, 3},
	{"Hend", (func_t)nsl_sf_hermite_prob_deriv, 3},
	{"Hfnd", (func_t)nsl_sf_hermite_func_der, 3},
#endif
	/* Hypergeometric Functions */
	{"hyperg_0F1", (func_t)gsl_sf_hyperg_0F1, 2},
	{"hyperg_1F1i", (func_t)nsl_sf_hyperg_1F1i, 3},
	{"hyperg_1F1", (func_t)gsl_sf_hyperg_1F1, 3},
	{"hyperg_Ui", (func_t)nsl_sf_hyperg_Ui, 3},
	{"hyperg_U", (func_t)gsl_sf_hyperg_U, 3},
	{"hyperg_2F1", (func_t)gsl_sf_hyperg_2F1, 4},
	{"hyperg_2F1c", (func_t)gsl_sf_hyperg_2F1_conj, 4},
	{"hyperg_2F1r", (func_t)gsl_sf_hyperg_2F1_renorm, 4},
	{"hyperg_2F1cr", (func_t)gsl_sf_hyperg_2F1_conj_renorm, 4},
	{"hyperg_2F0", (func_t)gsl_sf_hyperg_2F0, 3},
	/* Laguerre Functions */
	{"L1", (func_t)gsl_sf_laguerre_1, 2},
	{"L2", (func_t)gsl_sf_laguerre_2, 2},
	{"L3", (func_t)gsl_sf_laguerre_3, 2},
	{"Ln", (func_t)nsl_sf_laguerre_n, 3},
	/* Lambert W Functions */
	{"W0", (func_t)gsl_sf_lambert_W0, 1},
	{"Wm1", (func_t)gsl_sf_lambert_Wm1, 1},
	/* Legendre Functions and Spherical Harmonics */
	{"P1", (func_t)gsl_sf_legendre_P1, 1},
	{"P2", (func_t)gsl_sf_legendre_P2, 1},
	{"P3", (func_t)gsl_sf_legendre_P3, 1},
	{"Pl", (func_t)nsl_sf_legendre_Pl, 2},
	{"Q0", (func_t)gsl_sf_legendre_Q0, 1},
	{"Q1", (func_t)gsl_sf_legendre_Q1, 1},
	{"Ql", (func_t)nsl_sf_legendre_Ql, 2},
	{"Plm", (func_t)nsl_sf_legendre_Plm, 3},
	{"Pslm", (func_t)nsl_sf_legendre_sphPlm, 3},
	{"Phalf", (func_t)gsl_sf_conicalP_half, 2},
	{"Pmhalf", (func_t)gsl_sf_conicalP_mhalf, 2},
	{"Pc0", (func_t)gsl_sf_conicalP_0, 2},
	{"Pc1", (func_t)gsl_sf_conicalP_1, 2},
	{"Psr", (func_t)nsl_sf_conicalP_sphreg, 3},
	{"Pcr", (func_t)nsl_sf_conicalP_cylreg, 3},
	{"H3d0", (func_t)gsl_sf_legendre_H3d_0, 2},
	{"H3d1", (func_t)gsl_sf_legendre_H3d_1, 2},
	{"H3d", (func_t)nsl_sf_legendre_H3d, 3},
	/* Logarithm and Related Functions */
	{"log", (func_t)gsl_sf_log, 1},
	{"logabs", (func_t)gsl_sf_log_abs, 1},
	{"logp", (func_t)gsl_sf_log_1plusx, 1},
	{"logpm", (func_t)gsl_sf_log_1plusx_mx, 1},
	/* Mathieu Functions */
#if (GSL_MAJOR_VERSION >= 2)
	{"an", (func_t)nsl_sf_mathieu_a, 2},
	{"bn", (func_t)nsl_sf_mathieu_b, 2},
	{"cen", (func_t)nsl_sf_mathieu_ce, 3},
	{"sen", (func_t)nsl_sf_mathieu_se, 3},
	{"Mc", (func_t)nsl_sf_mathieu_Mc, 4},
	{"Ms", (func_t)nsl_sf_mathieu_Ms, 4},
#endif
	/* Power Function */
	{"gsl_powint", (func_t)nsl_sf_powint, 2},
	/* Psi (Digamma) Function */
	{"psiint", (func_t)nsl_sf_psiint, 1},
	{"psi", (func_t)gsl_sf_psi, 1},
	{"psi1piy", (func_t)gsl_sf_psi_1piy, 1},
	{"psi1int", (func_t)nsl_sf_psi1int, 1},
	{"psi1", (func_t)gsl_sf_psi_1, 1},
	{"psin", (func_t)nsl_sf_psin, 2},
	/* Synchrotron Functions */
	{"synchrotron1", (func_t)gsl_sf_synchrotron_1, 1},
	{"synchrotron2", (func_t)gsl_sf_synchrotron_2, 1},
	/* Transport Functions */
	{"J2", (func_t)gsl_sf_transport_2, 1},
	{"J3", (func_t)gsl_sf_transport_3, 1},
	{"J4", (func_t)gsl_sf_transport_4, 1},
	{"J5", (func_t)gsl_sf_transport_5, 1},
	/* Trigonometric Functions */
	{"sin", (func_t)gsl_sf_sin, 1},
	{"cos", (func_t)gsl_sf_cos, 1},
	{"tan", (func_t)tan, 1},
	{"asin", (func_t)asin, 1},
	{"acos", (func_t)acos, 1},
	{"atan", (func_t)atan, 1},
	{"atan2", (func_t)atan2, 2},
	{"sinh", (func_t)sinh, 1},
	{"cosh", (func_t)cosh, 1},
	{"tanh", (func_t)tanh, 1},
	{"acosh", (func_t)gsl_acosh, 1},
	{"asinh", (func_t)gsl_asinh, 1},
	{"atanh", (func_t)gsl_atanh, 1},
	{"sec", (func_t)nsl_sf_sec, 1},
	{"csc", (func_t)nsl_sf_csc, 1},
	{"cot", (func_t)nsl_sf_cot, 1},
	{"asec", (func_t)nsl_sf_asec, 1},
	{"acsc", (func_t)nsl_sf_acsc, 1},
	{"acot", (func_t)nsl_sf_acot, 1},
	{"sech", (func_t)nsl_sf_sech, 1},
	{"csch", (func_t)nsl_sf_csch, 1},
	{"coth", (func_t)nsl_sf_coth, 1},
	{"asech", (func_t)nsl_sf_asech, 1},
	{"acsch", (func_t)nsl_sf_acsch, 1},
	{"acoth", (func_t)nsl_sf_acoth, 1},
	{"sinc", (func_t)gsl_sf_sinc, 1},
	{"logsinh", (func_t)gsl_sf_lnsinh, 1},
	{"logcosh", (func_t)gsl_sf_lncosh, 1},
	{"hypot", (func_t)gsl_sf_hypot, 2},
	{"hypot3", (func_t)gsl_hypot3, 3},
	{"anglesymm", (func_t)gsl_sf_angle_restrict_symm, 1},
	{"anglepos", (func_t)gsl_sf_angle_restrict_pos, 1},
	/* Zeta Functions */
	{"zetaint", (func_t)nsl_sf_zetaint, 1},
	{"zeta", (func_t)gsl_sf_zeta, 1},
	{"zetam1int", (func_t)nsl_sf_zetam1int, 1},
	{"zetam1", (func_t)gsl_sf_zetam1, 1},
	{"hzeta", (func_t)gsl_sf_hzeta, 2},
	{"etaint", (func_t)nsl_sf_etaint, 1},
	{"eta", (func_t)gsl_sf_eta, 1},

	/* GSL Random Number Distributions: see https://www.gnu.org/software/gsl/doc/html/randist.html */
	/* Gaussian Distribution */
	{"gaussian", (func_t)gsl_ran_gaussian_pdf, 2},
	{"ugaussian", (func_t)gsl_ran_ugaussian_pdf, 1},
	{"gaussianP", (func_t)gsl_cdf_gaussian_P, 2},
	{"gaussianQ", (func_t)gsl_cdf_gaussian_Q, 2},
	{"gaussianPinv", (func_t)gsl_cdf_gaussian_Pinv, 2},
	{"gaussianQinv", (func_t)gsl_cdf_gaussian_Qinv, 2},
	{"ugaussianP", (func_t)gsl_cdf_ugaussian_P, 1},
	{"ugaussianQ", (func_t)gsl_cdf_ugaussian_Q, 1},
	{"ugaussianPinv", (func_t)gsl_cdf_ugaussian_Pinv, 1},
	{"ugaussianQinv", (func_t)gsl_cdf_ugaussian_Qinv, 1},
	{"gaussiantail", (func_t)gsl_ran_gaussian_tail_pdf, 3},
	{"ugaussiantail", (func_t)gsl_ran_ugaussian_tail_pdf, 2},
	{"gaussianbi", (func_t)gsl_ran_bivariate_gaussian_pdf, 5},
	/* Exponential Distribution */
	{"exponential", (func_t)gsl_ran_exponential_pdf, 2},
	{"exponentialP", (func_t)gsl_cdf_exponential_P, 2},
	{"exponentialQ", (func_t)gsl_cdf_exponential_Q, 2},
	{"exponentialPinv", (func_t)gsl_cdf_exponential_Pinv, 2},
	{"exponentialQinv", (func_t)gsl_cdf_exponential_Qinv, 2},
	/* Laplace Distribution */
	{"laplace", (func_t)gsl_ran_laplace_pdf, 2},
	{"laplaceP", (func_t)gsl_cdf_laplace_P, 2},
	{"laplaceQ", (func_t)gsl_cdf_laplace_Q, 2},
	{"laplacePinv", (func_t)gsl_cdf_laplace_Pinv, 2},
	{"laplaceQinv", (func_t)gsl_cdf_laplace_Qinv, 2},
	/* Exponential Power Distribution */
	{"exppow", (func_t)gsl_ran_exppow_pdf, 3},
	{"exppowP", (func_t)gsl_cdf_exppow_P, 3},
	{"exppowQ", (func_t)gsl_cdf_exppow_Q, 3},
	/* Cauchy Distribution */
	{"cauchy", (func_t)gsl_ran_cauchy_pdf, 2},
	{"cauchyP", (func_t)gsl_cdf_cauchy_P, 2},
	{"cauchyQ", (func_t)gsl_cdf_cauchy_Q, 2},
	{"cauchyPinv", (func_t)gsl_cdf_cauchy_Pinv, 2},
	{"cauchyQinv", (func_t)gsl_cdf_cauchy_Qinv, 2},
	/* Rayleigh Distribution */
	{"rayleigh", (func_t)gsl_ran_rayleigh_pdf, 2},
	{"rayleighP", (func_t)gsl_cdf_rayleigh_P, 2},
	{"rayleighQ", (func_t)gsl_cdf_rayleigh_Q, 2},
	{"rayleighPinv", (func_t)gsl_cdf_rayleigh_Pinv, 2},
	{"rayleighQinv", (func_t)gsl_cdf_rayleigh_Qinv, 2},
	{"rayleigh_tail", (func_t)gsl_ran_rayleigh_tail_pdf, 3},
	/* Landau Distribution */
	{"landau", (func_t)gsl_ran_landau_pdf, 1},
	/* Gamma Distribution */
	{"gammapdf", (func_t)gsl_ran_gamma_pdf, 3},
	{"gammaP", (func_t)gsl_cdf_gamma_P, 3},
	{"gammaQ", (func_t)gsl_cdf_gamma_Q, 3},
	{"gammaPinv", (func_t)gsl_cdf_gamma_Pinv, 3},
	{"gammaQinv", (func_t)gsl_cdf_gamma_Qinv, 3},
	/* Flat (Uniform) Distribution */
	{"flat", (func_t)gsl_ran_flat_pdf, 3},
	{"flatP", (func_t)gsl_cdf_flat_P, 3},
	{"flatQ", (func_t)gsl_cdf_flat_Q, 3},
	{"flatPinv", (func_t)gsl_cdf_flat_Pinv, 3},
	{"flatQinv", (func_t)gsl_cdf_flat_Qinv, 3},
	/* Lognormal Distribution */
	{"lognormal", (func_t)gsl_ran_lognormal_pdf, 3},
	{"lognormalP", (func_t)gsl_cdf_lognormal_P, 3},
	{"lognormalQ", (func_t)gsl_cdf_lognormal_Q, 3},
	{"lognormalPinv", (func_t)gsl_cdf_lognormal_Pinv, 3},
	{"lognormalQinv", (func_t)gsl_cdf_lognormal_Qinv, 3},
	/* Chi-squared Distribution */
	{"chisq", (func_t)gsl_ran_chisq_pdf, 2},
	{"chisqP", (func_t)gsl_cdf_chisq_P, 2},
	{"chisqQ", (func_t)gsl_cdf_chisq_Q, 2},
	{"chisqPinv", (func_t)gsl_cdf_chisq_Pinv, 2},
	{"chisqQinv", (func_t)gsl_cdf_chisq_Qinv, 2},
	/* F-distribution */
	{"fdist", (func_t)gsl_ran_fdist_pdf, 3},
	{"fdistP", (func_t)gsl_cdf_fdist_P, 3},
	{"fdistQ", (func_t)gsl_cdf_fdist_Q, 3},
	{"fdistPinv", (func_t)gsl_cdf_fdist_Pinv, 3},
	{"fdistQinv", (func_t)gsl_cdf_fdist_Qinv, 3},
	/* t-distribution */
	{"tdist", (func_t)gsl_ran_tdist_pdf, 2},
	{"tdistP", (func_t)gsl_cdf_tdist_P, 2},
	{"tdistQ", (func_t)gsl_cdf_tdist_Q, 2},
	{"tdistPinv", (func_t)gsl_cdf_tdist_Pinv, 2},
	{"tdistQinv", (func_t)gsl_cdf_tdist_Qinv, 2},
	/* Beta Distribution */
	{"betapdf", (func_t)gsl_ran_beta_pdf, 3},
	{"betaP", (func_t)gsl_cdf_beta_P, 3},
	{"betaQ", (func_t)gsl_cdf_beta_Q, 3},
	{"betaPinv", (func_t)gsl_cdf_beta_Pinv, 3},
	{"betaQinv", (func_t)gsl_cdf_beta_Qinv, 3},
	/* Logistic Distribution */
	{"logistic", (func_t)gsl_ran_logistic_pdf, 2},
	{"logisticP", (func_t)gsl_cdf_logistic_P, 2},
	{"logisticQ", (func_t)gsl_cdf_logistic_Q, 2},
	{"logisticPinv", (func_t)gsl_cdf_logistic_Pinv, 2},
	{"logisticQinv", (func_t)gsl_cdf_logistic_Qinv, 2},
	/* Pareto Distribution */
	{"pareto", (func_t)gsl_ran_pareto_pdf, 3},
	{"paretoP", (func_t)gsl_cdf_pareto_P, 3},
	{"paretoQ", (func_t)gsl_cdf_pareto_Q, 3},
	{"paretoPinv", (func_t)gsl_cdf_pareto_Pinv, 3},
	{"paretoQinv", (func_t)gsl_cdf_pareto_Qinv, 3},
	/* Weibull Distribution */
	{"weibull", (func_t)gsl_ran_weibull_pdf, 3},
	{"weibullP", (func_t)gsl_cdf_weibull_P, 3},
	{"weibullQ", (func_t)gsl_cdf_weibull_Q, 3},
	{"weibullPinv", (func_t)gsl_cdf_weibull_Pinv, 3},
	{"weibullQinv", (func_t)gsl_cdf_weibull_Qinv, 3},
	/* Gumbel Distribution */
	{"gumbel1", (func_t)gsl_ran_gumbel1_pdf, 3},
	{"gumbel1P", (func_t)gsl_cdf_gumbel1_P, 3},
	{"gumbel1Q", (func_t)gsl_cdf_gumbel1_Q, 3},
	{"gumbel1Pinv", (func_t)gsl_cdf_gumbel1_Pinv, 3},
	{"gumbel1Qinv", (func_t)gsl_cdf_gumbel1_Qinv, 3},
	{"gumbel2", (func_t)gsl_ran_gumbel2_pdf, 3},
	{"gumbel2P", (func_t)gsl_cdf_gumbel2_P, 3},
	{"gumbel2Q", (func_t)gsl_cdf_gumbel2_Q, 3},
	{"gumbel2Pinv", (func_t)gsl_cdf_gumbel2_Pinv, 3},
	{"gumbel2Qinv", (func_t)gsl_cdf_gumbel2_Qinv, 3},
	/* Poisson Distribution */
	{"poisson", (func_t)nsl_sf_poisson, 2},
	{"poissonP", (func_t)gsl_cdf_poisson_P, 2},
	{"poissonQ", (func_t)gsl_cdf_poisson_Q, 2},
	/* Bernoulli Distribution */
	{"bernoulli", (func_t)nsl_sf_bernoulli, 2},
	/* Binomial Distribution */
	{"binomial", (func_t)nsl_sf_binomial, 3},
	{"binomialP", (func_t)gsl_cdf_binomial_P, 3},
	{"binomialQ", (func_t)gsl_cdf_binomial_Q, 3},
	{"negative_binomial", (func_t)nsl_sf_negative_binomial, 3},
	{"negative_binomialP", (func_t)gsl_cdf_negative_binomial_P, 3},
	{"negative_binomialQ", (func_t)gsl_cdf_negative_binomial_Q, 3},
	/* Pascal Distribution */
	{"pascal", (func_t)nsl_sf_pascal, 3},
	{"pascalP", (func_t)gsl_cdf_pascal_P, 3},
	{"pascalQ", (func_t)gsl_cdf_pascal_Q, 3},
	/* Geometric Distribution */
	{"geometric", (func_t)nsl_sf_geometric, 2},
	{"geometricP", (func_t)gsl_cdf_geometric_P, 2},
	{"geometricQ", (func_t)gsl_cdf_geometric_Q, 2},
	/* Hypergeometric Distribution */
	{"hypergeometric", (func_t)nsl_sf_hypergeometric, 4},
	{"hypergeometricP", (func_t)gsl_cdf_hypergeometric_P, 4},
	{"hypergeometricQ", (func_t)gsl_cdf_hypergeometric_Q, 4},
	/* Logarithmic Distribution */
	{"logarithmic", (func_t)nsl_sf_logarithmic, 2},
	{0, (func_t)0, 0}
};

#endif /*FUNCTIONS_H*/
