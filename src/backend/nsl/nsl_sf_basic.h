/*
	File                 : nsl_sf_basic.h
	Project              : LabPlot
	Description          : NSL special basic functions
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2017-2021 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef NSL_SF_BASIC_H
#define NSL_SF_BASIC_H

#include "nsl_complex.h"
#include <gsl/gsl_version.h>
#include <stdint.h> /* fixed size int types */

/* random functions */
double nsl_sf_rand(void);
double nsl_sf_random(void);
double nsl_sf_drand(void);

/* signum function */
double nsl_sf_sgn(double x);
/* Heavyside theta function */
double nsl_sf_theta(double x);

/* non-standard function */
double nsl_sf_exp10(double x);

/* non-uniform random number generation */
double nsl_sf_ran_gaussian(double sigma);
double nsl_sf_ran_exponential(double mu);
double nsl_sf_ran_laplace(double a);
double nsl_sf_ran_cauchy(double a);
double nsl_sf_ran_rayleigh(double sigma);
double nsl_sf_ran_landau(void);
double nsl_sf_ran_levy(double c, double alpha);
double nsl_sf_ran_gamma(double a, double b);
double nsl_sf_ran_flat(double a, double b);
double nsl_sf_ran_lognormal(double zeta, double sigma);
double nsl_sf_ran_chisq(double nu);
double nsl_sf_ran_tdist(double nu);
double nsl_sf_ran_logistic(double a);

double nsl_sf_ran_poisson(double mu);
double nsl_sf_ran_bernoulli(double p);
double nsl_sf_ran_binomial(double p, double n);

/* log2(x) for integer value x */
int nsl_sf_log2_int(unsigned int x);
int nsl_sf_log2_int0(unsigned int x);
int nsl_sf_log2_int2(int x);
int nsl_sf_log2_int3(uint64_t x);
int nsl_sf_log2p1_int(int x);
int nsl_sf_log2_longlong(unsigned long long x);

/* more trig. functions */
double nsl_sf_sec(double x);
double nsl_sf_csc(double x);
double nsl_sf_cot(double x);
double nsl_sf_asec(double x);
double nsl_sf_acsc(double x);
double nsl_sf_acot(double x);
double nsl_sf_sech(double x);
double nsl_sf_csch(double x);
double nsl_sf_coth(double x);
double nsl_sf_asech(double x);
double nsl_sf_acsch(double x);
double nsl_sf_acoth(double x);

/* harmonic numbers (extended to non-integers) */
double nsl_sf_harmonic(double x);

/* error function and related wrapper */
double nsl_sf_erfcx(double x);
double nsl_sf_erfi(double x);
double nsl_sf_im_w_of_x(double x);
#if !defined(_MSC_VER)
double nsl_sf_im_w_of_z(COMPLEX z);
#endif
double nsl_sf_dawson(double x);
double nsl_sf_voigt(double x, double sigma, double gamma);
double nsl_sf_pseudovoigt(double x, double eta, double sigma, double gamma);
/* same width pseudo Voigt*/
double nsl_sf_pseudovoigt1(double x, double eta, double w);

/* wrapper for GSL functions with integer parameters */
/* mathematical functions */
double nsl_sf_ldexp(double x, double expo);
double nsl_sf_powint(double x, double n);
/* Airy functions */
double nsl_sf_airy_Ai(double x);
double nsl_sf_airy_Bi(double x);
double nsl_sf_airy_Ais(double x);
double nsl_sf_airy_Bis(double x);
double nsl_sf_airy_Aid(double x);
double nsl_sf_airy_Bid(double x);
double nsl_sf_airy_Aids(double x);
double nsl_sf_airy_Bids(double x);
double nsl_sf_airy_0_Ai(double s);
double nsl_sf_airy_0_Bi(double s);
double nsl_sf_airy_0_Aid(double s);
double nsl_sf_airy_0_Bid(double s);
/* Bessel functions */
double nsl_sf_bessel_Jn(double n, double x);
double nsl_sf_bessel_Yn(double n, double x);
double nsl_sf_bessel_In(double n, double x);
double nsl_sf_bessel_Ins(double n, double x);
double nsl_sf_bessel_Kn(double n, double x);
double nsl_sf_bessel_Kns(double n, double x);
double nsl_sf_bessel_jl(double l, double x);
double nsl_sf_bessel_yl(double l, double x);
double nsl_sf_bessel_ils(double l, double x);
double nsl_sf_bessel_kls(double l, double x);
double nsl_sf_bessel_0_J0(double s);
double nsl_sf_bessel_0_J1(double s);
double nsl_sf_bessel_0_Jnu(double nu, double s);

double nsl_sf_hydrogenicR(double n, double l, double z, double r);
/* elliptic integrals */
double nsl_sf_ellint_Kc(double x);
double nsl_sf_ellint_Ec(double x);
double nsl_sf_ellint_Pc(double x, double n);
double nsl_sf_ellint_F(double phi, double k);
double nsl_sf_ellint_E(double phi, double k);
double nsl_sf_ellint_P(double phi, double k, double n);
double nsl_sf_ellint_D(double phi, double k);
double nsl_sf_ellint_RC(double x, double y);
double nsl_sf_ellint_RD(double x, double y, double z);
double nsl_sf_ellint_RF(double x, double y, double z);
double nsl_sf_ellint_RJ(double x, double y, double z, double p);

double nsl_sf_exprel_n(double n, double x);
double nsl_sf_fermi_dirac_int(double j, double x);
/* Gamma */
double nsl_sf_fact(double n);
double nsl_sf_doublefact(double n);
double nsl_sf_lnfact(double n);
double nsl_sf_lndoublefact(double n);
double nsl_sf_choose(double n, double m);
double nsl_sf_lnchoose(double n, double m);
double nsl_sf_taylorcoeff(double n, double x);

double nsl_sf_gegenpoly_n(double n, double l, double x);

#if (GSL_MAJOR_VERSION > 2) || (GSL_MAJOR_VERSION == 2) && (GSL_MINOR_VERSION >= 4)
double nsl_sf_hermite_prob(double n, double x);
double nsl_sf_hermite_func(double n, double x);
double nsl_sf_hermite_func_der(double m, double n, double x);
double nsl_sf_hermite(double n, double x);
double nsl_sf_hermite_deriv(double m, double n, double x);
double nsl_sf_hermite_prob_deriv(double m, double n, double x);
#if (GSL_MAJOR_VERSION > 2) || (GSL_MAJOR_VERSION == 2) && (GSL_MINOR_VERSION >= 6)
double nsl_sf_hermite_func_fast(double n, double x);
#endif
#endif

double nsl_sf_hyperg_1F1i(double m, double n, double x);
double nsl_sf_hyperg_Ui(double m, double n, double x);
double nsl_sf_laguerre_n(double n, double a, double x);

double nsl_sf_legendre_Pl(double l, double x);
double nsl_sf_legendre_Ql(double l, double x);
double nsl_sf_legendre_Plm(double l, double m, double x);
double nsl_sf_legendre_sphPlm(double l, double m, double x);
double nsl_sf_conicalP_sphreg(double l, double L, double x);
double nsl_sf_conicalP_cylreg(double m, double l, double x);
double nsl_sf_legendre_H3d(double l, double L, double e);

#if (GSL_MAJOR_VERSION >= 2)
double nsl_sf_mathieu_a(double n, double q);
double nsl_sf_mathieu_b(double n, double q);
double nsl_sf_mathieu_ce(double n, double q, double x);
double nsl_sf_mathieu_se(double n, double q, double x);
double nsl_sf_mathieu_Mc(double j, double n, double q, double x);
double nsl_sf_mathieu_Ms(double j, double n, double q, double x);
#endif

double nsl_sf_psiint(double n);
double nsl_sf_psi1int(double n);
double nsl_sf_psin(double n, double x);

double nsl_sf_zetaint(double n);
double nsl_sf_zetam1int(double n);
double nsl_sf_etaint(double n);

/* random number distributions */
double nsl_sf_poisson(double k, double m);
double nsl_sf_bernoulli(double k, double p);
double nsl_sf_binomial(double k, double p, double n);
double nsl_sf_negative_binomial(double k, double p, double n);
double nsl_sf_pascal(double k, double p, double n);
double nsl_sf_geometric(double k, double p);
double nsl_sf_hypergeometric(double k, double n1, double n2, double t);
double nsl_sf_logarithmic(double k, double p);

#endif /* NSL_SF_BASIC_H */
