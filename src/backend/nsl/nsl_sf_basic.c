/***************************************************************************
    File                 : nsl_sf_basic.c
    Project              : LabPlot
    Description          : NSL special basic functions
    --------------------------------------------------------------------
    Copyright            : (C) 2018 by Stefan Gerlach (stefan.gerlach@uni.kn)

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

#include "nsl_sf_basic.h"
#include <stdlib.h>
#include <math.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_sf.h>
#include <gsl/gsl_randist.h>
#ifdef HAVE_LIBCERF
#include <cerf.h>
#elif !defined(_MSC_VER)
#include "Faddeeva.h"
#endif

/* stdlib.h */
double nsl_sf_rand(void) { return rand(); }
#if defined(HAVE_RANDOM_FUNCTION)
double nsl_sf_random(void) { return random(); }
double nsl_sf_drand(void) { return random()/(double)RAND_MAX; }
#else
double nsl_sf_random(void) { return rand(); }
double nsl_sf_drand(void) { return rand()/(double)RAND_MAX; }
#endif

double nsl_sf_sgn(double x) {
#ifndef _WIN32
	return copysign(1.0, x);
#else
	if (x > 0)
		return 1;
	else if (x < 0)
		return -1;
	else
		return 0;
#endif
}

double nsl_sf_theta(double x) {
	if (x >= 0)
		return 1;
	else
		return 0;
}

/*
 * source: https://stackoverflow.com/questions/11376288/fast-computing-of-log2-for-64-bit-integers
 * source: http://graphics.stanford.edu/~seander/bithacks.html#IntegerLogLookup
 */
#ifdef _MSC_VER
#include <intrin.h>
#endif
int nsl_sf_log2_int(unsigned int x) {
#ifdef _MSC_VER
	return (int) (__lzcnt(x));
#else
	return (int) (8*sizeof (unsigned int) - __builtin_clz(x) - 1);
#endif
}
int nsl_sf_log2_longlong(unsigned long long x) {
#ifdef _MSC_VER
	return (int) (__lzcnt64(x));
#else
	return (int) (8*sizeof (unsigned long long) - __builtin_clzll(x) - 1);
#endif
}
int nsl_sf_log2_int2(int x) {
	const signed char LogTable256[256] = {
		-1,0,1,1,2,2,2,2,3,3,3,3,3,3,3,3,
		4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
		5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
		5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
		6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
		6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
		6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
		6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
		7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
		7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
		7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
		7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
		7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
		7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
		7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
		7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7
	};

	unsigned int r;     // r will be lg(v)
	unsigned int t, tt; // temporaries
	if ((tt = x >> 16))
		r = (t = tt >> 8) ? 24 + LogTable256[t] : 16 + LogTable256[tt];
	else
		r = (t = x >> 8) ? 8 + LogTable256[t] : LogTable256[x];

	return r;
}
int nsl_sf_log2_int3(uint64_t value) {
	const int tab64[64] = {
		63,  0, 58,  1, 59, 47, 53,  2,
		60, 39, 48, 27, 54, 33, 42,  3,
		61, 51, 37, 40, 49, 18, 28, 20,
		55, 30, 34, 11, 43, 14, 22,  4,
		62, 57, 46, 52, 38, 26, 32, 41,
		50, 36, 17, 19, 29, 10, 13, 21,
		56, 45, 25, 31, 35, 16,  9, 12,
		44, 24, 15,  8, 23,  7,  6,  5};

	value |= value >> 1;
	value |= value >> 2;
	value |= value >> 4;
	value |= value >> 8;
	value |= value >> 16;
	value |= value >> 32;

	return tab64[((uint64_t)((value - (value >> 1))*0x07EDD5E59A4E28C2)) >> 58];
}
int nsl_sf_log2p1_int(int x) {
	// fastest method
	return nsl_sf_log2_int(x) + 1;
	//TODO: why is this so slow
	//return (int)log2(x) + 1;
}

double nsl_sf_sec(double x) { return 1./cos(x); }
double nsl_sf_csc(double x) { return 1./sin(x); }
double nsl_sf_cot(double x) { return 1./tan(x); }
double nsl_sf_asec(double x) { return acos(1./x); }
double nsl_sf_acsc(double x) { return asin(1./x); }
double nsl_sf_acot(double x) {
	if (x > 0)
		return atan(1./x);
	else
		return atan(1./x) + M_PI;
}
double nsl_sf_sech(double x) { return 1./cosh(x); }
double nsl_sf_csch(double x) { return 1./sinh(x); }
double nsl_sf_coth(double x) { return 1./tanh(x); }
double nsl_sf_asech(double x) { return gsl_acosh(1./x); }
double nsl_sf_acsch(double x) { return gsl_asinh(1./x); }
double nsl_sf_acoth(double x) { return gsl_atanh(1./x); }

double nsl_sf_harmonic(double x) {
	// check if x is a negative integer
	if (x < 0 && !gsl_fcmp(round(x) - x, 0., 1.e-16))
		return GSL_POSINF;

	return gsl_sf_psi(x + 1) + M_EULER;
}

/* error functions and related */
double nsl_sf_erfcx(double x) {
#ifdef HAVE_LIBCERF
	return erfcx(x);
#elif defined(_MSC_VER)
	return 0.;	// not supported yet
#else
	return Faddeeva_erfcx_re(x);
#endif
}

double nsl_sf_erfi(double x) {
#ifdef HAVE_LIBCERF
	return erfi(x);
#elif defined(_MSC_VER)
	return 0.;	// not supported yet
#else
	return Faddeeva_erfi_re(x);
#endif
}

double nsl_sf_im_w_of_x(double x) {
#ifdef HAVE_LIBCERF
	return im_w_of_x(x);
#elif defined(_MSC_VER)
	return 0.;	// not supported yet
#else
	return Faddeeva_w_im(x);
#endif
}

#if !defined(_MSC_VER)
double nsl_sf_im_w_of_z(COMPLEX z) {
#ifdef HAVE_LIBCERF
	return cimag(w_of_z(z));
#else
	return cimag(Faddeeva_w(z, 0));
#endif
}
#endif

double nsl_sf_dawson(double x) {
#ifdef HAVE_LIBCERF
	return dawson(x);
#elif defined(_MSC_VER)
	return 0.;	// not supported yet
#else
	return Faddeeva_Dawson_re(x);
#endif
}

double nsl_sf_voigt(double x, double sigma, double gamma) {
#ifdef HAVE_LIBCERF
	return voigt(x, sigma, gamma);
#elif defined(_MSC_VER)
	return 0.;	// not supported yet
#else
	COMPLEX z = (x + I*gamma)/(sqrt(2.)*sigma);
	return creal(Faddeeva_w(z, 0))/(sqrt(2.*M_PI)*sigma);
#endif
}

double nsl_sf_pseudovoigt(double x, double eta, double sigma, double gamma) {
	if (sigma == 0 || gamma == 0)
		return 0;
	//TODO: what if eta < 0 or > 1?

	return (1. - eta) * gsl_ran_gaussian_pdf(x, sigma) + eta * gsl_ran_cauchy_pdf(x, gamma);
}

double nsl_sf_pseudovoigt1(double x, double eta, double w) {
	// 2w - FWHM, sigma_G = w/sqrt(2ln(2))
	return nsl_sf_pseudovoigt(x, eta, w/sqrt(2.*log(2.)), w);
}

/* wrapper for GSL functions with integer parameters */
#define MODE GSL_PREC_DOUBLE
/* mathematical functions */
double nsl_sf_ldexp(double x, double expo) { return gsl_ldexp(x, (int)round(expo)); }
double nsl_sf_powint(double x, double n) { return gsl_sf_pow_int(x, (int)round(n)); }
/* Airy functions */
double nsl_sf_airy_Ai(double x) { return gsl_sf_airy_Ai(x, MODE); }
double nsl_sf_airy_Bi(double x) { return gsl_sf_airy_Bi(x, MODE); }
double nsl_sf_airy_Ais(double x) { return gsl_sf_airy_Ai_scaled(x, MODE); }
double nsl_sf_airy_Bis(double x) { return gsl_sf_airy_Bi_scaled(x, MODE); }
double nsl_sf_airy_Aid(double x) { return gsl_sf_airy_Ai_deriv(x, MODE); }
double nsl_sf_airy_Bid(double x) { return gsl_sf_airy_Bi_deriv(x, MODE); }
double nsl_sf_airy_Aids(double x) { return gsl_sf_airy_Ai_deriv_scaled(x, MODE); }
double nsl_sf_airy_Bids(double x) { return gsl_sf_airy_Bi_deriv_scaled(x, MODE); }
double nsl_sf_airy_0_Ai(double s) { return gsl_sf_airy_zero_Ai((unsigned int)round(s)); }
double nsl_sf_airy_0_Bi(double s) { return gsl_sf_airy_zero_Bi((unsigned int)round(s)); }
double nsl_sf_airy_0_Aid(double s) { return gsl_sf_airy_zero_Ai_deriv((unsigned int)round(s)); }
double nsl_sf_airy_0_Bid(double s) { return gsl_sf_airy_zero_Bi_deriv((unsigned int)round(s)); }
/* Bessel functions */
double nsl_sf_bessel_Jn(double n, double x) { return gsl_sf_bessel_Jn((int)round(n), x); }
double nsl_sf_bessel_Yn(double n, double x) { return gsl_sf_bessel_Yn((int)round(n), x); }
double nsl_sf_bessel_In(double n, double x) { return gsl_sf_bessel_In((int)round(n), x); }
double nsl_sf_bessel_Ins(double n, double x) { return gsl_sf_bessel_In_scaled((int)round(n), x); }
double nsl_sf_bessel_Kn(double n, double x) { return gsl_sf_bessel_Kn((int)round(n), x); }
double nsl_sf_bessel_Kns(double n, double x) { return gsl_sf_bessel_Kn_scaled((int)round(n), x); }
double nsl_sf_bessel_jl(double l, double x) { return gsl_sf_bessel_jl((int)round(l), x); }
double nsl_sf_bessel_yl(double l, double x) { return gsl_sf_bessel_yl((int)round(l), x); }
double nsl_sf_bessel_ils(double l, double x) { return gsl_sf_bessel_il_scaled((int)round(l), x); }
double nsl_sf_bessel_kls(double l, double x) { return gsl_sf_bessel_kl_scaled((int)round(l), x); }
double nsl_sf_bessel_0_J0(double s) { return gsl_sf_bessel_zero_J0((unsigned int)round(s)); }
double nsl_sf_bessel_0_J1(double s) { return gsl_sf_bessel_zero_J1((unsigned int)round(s)); }
double nsl_sf_bessel_0_Jnu(double nu, double s) { return gsl_sf_bessel_zero_Jnu(nu, (unsigned int)round(s)); }

double nsl_sf_hydrogenicR(double n, double l, double z, double r) { return gsl_sf_hydrogenicR((int)round(n), (int)round(l), z, r); }
/* elliptic integrals */
double nsl_sf_ellint_Kc(double x) { return gsl_sf_ellint_Kcomp(x, MODE); }
double nsl_sf_ellint_Ec(double x) { return gsl_sf_ellint_Ecomp(x, MODE); }
double nsl_sf_ellint_Pc(double x, double n) { return gsl_sf_ellint_Pcomp(x, n, MODE); }
double nsl_sf_ellint_F(double phi, double k) { return gsl_sf_ellint_F(phi, k, MODE); }
double nsl_sf_ellint_E(double phi, double k) { return gsl_sf_ellint_E(phi, k, MODE); }
double nsl_sf_ellint_P(double phi, double k, double n) { return gsl_sf_ellint_P(phi, k, n, MODE); }
double nsl_sf_ellint_D(double phi, double k) {
#if GSL_MAJOR_VERSION >= 2
	return gsl_sf_ellint_D(phi,k,MODE);
#else
	return gsl_sf_ellint_D(phi,k,0.0,MODE);
#endif
}
double nsl_sf_ellint_RC(double x, double y) { return gsl_sf_ellint_RC(x, y, MODE); }
double nsl_sf_ellint_RD(double x, double y, double z) { return gsl_sf_ellint_RD(x, y, z, MODE); }
double nsl_sf_ellint_RF(double x, double y, double z) { return gsl_sf_ellint_RF(x, y, z, MODE); }
double nsl_sf_ellint_RJ(double x, double y, double z, double p) { return gsl_sf_ellint_RJ(x, y, z, p, MODE); }

double nsl_sf_exprel_n(double n, double x) { return gsl_sf_exprel_n((int)round(n), x); }
double nsl_sf_fermi_dirac_int(double j, double x) { return gsl_sf_fermi_dirac_int((int)round(j), x); }
/* Gamma */
double nsl_sf_fact(double n) { return gsl_sf_fact((unsigned int)round(n)); }
double nsl_sf_doublefact(double n) { return gsl_sf_doublefact((unsigned int)round(n)); }
double nsl_sf_lnfact(double n) { return gsl_sf_lnfact((unsigned int)round(n)); }
double nsl_sf_lndoublefact(double n) { return gsl_sf_lndoublefact((unsigned int)round(n)); }
double nsl_sf_choose(double n, double m) { return gsl_sf_choose((unsigned int)round(n), (unsigned int)round(m)); }
double nsl_sf_lnchoose(double n, double m) { return gsl_sf_lnchoose((unsigned int)round(n), (unsigned int)round(m)); }
double nsl_sf_taylorcoeff(double n, double x) { return gsl_sf_taylorcoeff((int)round(n), x); }

double nsl_sf_gegenpoly_n(double n, double l, double x) { return gsl_sf_gegenpoly_n((int)round(n), l, x); }

#if (GSL_MAJOR_VERSION > 2) || (GSL_MAJOR_VERSION == 2) && (GSL_MINOR_VERSION >= 4)
double nsl_sf_hermite_prob(double n, double x) { return gsl_sf_hermite_prob(round(n), x); }
double nsl_sf_hermite_phys(double n, double x) { return gsl_sf_hermite_phys(round(n), x); }
double nsl_sf_hermite_func(double n, double x) { return gsl_sf_hermite_func(round(n), x); }
double nsl_sf_hermite_prob_der(double m, double n, double x) { return gsl_sf_hermite_prob_der(round(m), round(n), x); }
double nsl_sf_hermite_phys_der(double m, double n, double x) { return gsl_sf_hermite_phys_der(round(m), round(n), x); }
double nsl_sf_hermite_func_der(double m, double n, double x) { return gsl_sf_hermite_func_der(round(m), round(n), x); }
#endif

double nsl_sf_hyperg_1F1i(double m, double n, double x) { return gsl_sf_hyperg_1F1_int((int)round(m), (int)round(n), x); }
double nsl_sf_hyperg_Ui(double m, double n, double x) { return gsl_sf_hyperg_U_int((int)round(m), (int)round(n), x); }
double nsl_sf_laguerre_n(double n, double a, double x) { return gsl_sf_laguerre_n((int)round(n), a, x); }

double nsl_sf_legendre_Pl(double l, double x) { return gsl_sf_legendre_Pl((int)round(l), x); }
double nsl_sf_legendre_Ql(double l, double x) { return gsl_sf_legendre_Ql((int)round(l), x); }
double nsl_sf_legendre_Plm(double l, double m, double x) { return gsl_sf_legendre_Plm((int)round(l), (int)round(m), x); }
double nsl_sf_legendre_sphPlm(double l, double m, double x) { return gsl_sf_legendre_sphPlm((int)round(l), (int)round(m), x); }
double nsl_sf_conicalP_sphreg(double l, double L, double x) { return gsl_sf_conicalP_sph_reg((int)round(l), L, x); }
double nsl_sf_conicalP_cylreg(double m, double l, double x) { return gsl_sf_conicalP_sph_reg((int)round(m), l, x); }
double nsl_sf_legendre_H3d(double l,  double L, double e) { return gsl_sf_legendre_H3d((int)round(l), L, e); }

double nsl_sf_psiint(double n) { return gsl_sf_psi_int((int)round(n)); }
double nsl_sf_psi1int(double n) { return gsl_sf_psi_1_int((int)round(n)); }
double nsl_sf_psin(double n,  double x) { return gsl_sf_psi_n((int)round(n), x); }

double nsl_sf_zetaint(double n) { return gsl_sf_zeta_int((int)round(n)); }
double nsl_sf_zetam1int(double n) { return gsl_sf_zetam1_int((int)round(n)); }
double nsl_sf_etaint(double n) { return gsl_sf_eta_int((int)round(n)); }

/* random number distributions */
double nsl_sf_poisson(double k, double m) { return gsl_ran_poisson_pdf((unsigned int)round(k), m); }
double nsl_sf_bernoulli(double k, double p) { return gsl_ran_bernoulli_pdf((unsigned int)round(k), p); }
double nsl_sf_binomial(double k, double p, double n) { return gsl_ran_binomial_pdf((unsigned int)round(k), p, (unsigned int)round(n)); }
double nsl_sf_negative_binomial(double k, double p, double n) { return gsl_ran_negative_binomial_pdf((unsigned int)round(k), p, n); }
double nsl_sf_pascal(double k, double p, double n) { return gsl_ran_pascal_pdf((unsigned int)round(k), p, (unsigned int)round(n)); }
double nsl_sf_geometric(double k, double p) { return gsl_ran_geometric_pdf((unsigned int)round(k), p); }
double nsl_sf_hypergeometric(double k, double n1, double n2, double t) {
	return gsl_ran_hypergeometric_pdf((unsigned int)round(k), (unsigned int)round(n1), (unsigned int)round(n2), (unsigned int)round(t));
}
double nsl_sf_logarithmic(double k, double p) { return gsl_ran_logarithmic_pdf((unsigned int)round(k), p); }
