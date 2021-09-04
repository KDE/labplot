/*
    SPDX-FileCopyrightText: 2012 Massachusetts Institute of Technology
    SPDX-License-Identifier: MIT
*/

/* Available at: http://ab-initio.mit.edu/Faddeeva

   Header file for Faddeeva.c; see Faddeeva.cc for more information. */

#ifndef FADDEEVA_H
#define FADDEEVA_H 1

// Require C99 complex-number support
#include <complex.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

// compute w(z) = exp(-z^2) erfc(-iz) [ Faddeeva / scaled complex error func ]
extern double complex Faddeeva_w(double complex z,double relerr);
extern double Faddeeva_w_im(double x); // special-case code for Im[w(x)] of real x

// Various functions that we can compute with the help of w(z)

// compute erfcx(z) = exp(z^2) erfc(z)
extern double complex Faddeeva_erfcx(double complex z, double relerr);
extern double Faddeeva_erfcx_re(double x); // special case for real x

// compute erf(z), the error function of complex arguments
extern double complex Faddeeva_erf(double complex z, double relerr);
extern double Faddeeva_erf_re(double x); // special case for real x

// compute erfi(z) = -i erf(iz), the imaginary error function
extern double complex Faddeeva_erfi(double complex z, double relerr);
extern double Faddeeva_erfi_re(double x); // special case for real x

// compute erfc(z) = 1 - erf(z), the complementary error function
extern double complex Faddeeva_erfc(double complex z, double relerr);
extern double Faddeeva_erfc_re(double x); // special case for real x

// compute Dawson(z) = sqrt(pi)/2  *  exp(-z^2) * erfi(z)
extern double complex Faddeeva_Dawson(double complex z, double relerr);
extern double Faddeeva_Dawson_re(double x); // special case for real x

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // FADDEEVA_H
