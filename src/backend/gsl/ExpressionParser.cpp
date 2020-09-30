/***************************************************************************
    File             : ExpressionParser.cpp
    Project          : LabPlot
    --------------------------------------------------------------------
    Copyright        : (C) 2014 Alexander Semke (alexander.semke@web.de)
    Copyright        : (C) 2014-2020 Stefan Gerlach (stefan.gerlach@uni.kn)
    Description      : C++ wrapper for the bison generated parser.

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

#include <QRegularExpression>

#include "backend/lib/macros.h"
#include "backend/gsl/ExpressionParser.h"

#include <klocalizedstring.h>

#include <gsl/gsl_version.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_const_mksa.h>
#include <gsl/gsl_const_num.h>

extern "C" {
#include "backend/gsl/parser.h"
}

ExpressionParser* ExpressionParser::m_instance{nullptr};

ExpressionParser::ExpressionParser() {
	init_table();
	initFunctions();
	initConstants();
}

void ExpressionParser::initFunctions() {
	//functions	(sync with functions.h!)
	for (int i = 0; _functions[i].name != nullptr; i++)
		m_functions << _functions[i].name;

	m_functionsGroups << i18n("Standard Mathematical functions");
	//https://www.gnu.org/software/gsl/doc/html/specfunc.html
	m_functionsGroups << i18n("Airy Functions and Derivatives");
	m_functionsGroups << i18n("Bessel Functions");
	m_functionsGroups << i18n("Clausen Functions");
	m_functionsGroups << i18n("Coulomb Functions");
// 	m_functionsGroups << i18n("Coupling Coefficients");
	m_functionsGroups << i18n("Dawson Function");
	m_functionsGroups << i18n("Debye Functions");
	m_functionsGroups << i18n("Dilogarithm");
//	m_functionsGroups << i18n("Elementary Operations");
	m_functionsGroups << i18n("Elliptic Integrals");
//	m_functionsGroups << i18n("Elliptic Functions (Jacobi)");
#ifndef _MSC_VER
	m_functionsGroups << i18n("Error Functions and Related Functions");
#else
	m_functionsGroups << i18n("Error Functions");
#endif
	m_functionsGroups << i18n("Exponential Functions");
	m_functionsGroups << i18n("Exponential Integrals");
	m_functionsGroups << i18n("Fermi-Dirac Function");
	m_functionsGroups << i18n("Gamma and Beta Functions");
	m_functionsGroups << i18n("Gegenbauer Functions");
#if (GSL_MAJOR_VERSION > 2) || (GSL_MAJOR_VERSION == 2) && (GSL_MINOR_VERSION >= 4)
	m_functionsGroups << i18n("Hermite Polynomials and Functions");
#endif
	m_functionsGroups << i18n("Hypergeometric Functions");
	m_functionsGroups << i18n("Laguerre Functions");
	m_functionsGroups << i18n("Lambert W Functions");
	m_functionsGroups << i18n("Legendre Functions and Spherical Harmonics");
	m_functionsGroups << i18n("Logarithm and Related Functions");
//	m_functionsGroups << i18n("Mathieu Functions");
	m_functionsGroups << i18n("Power Function");
	m_functionsGroups << i18n("Psi (Digamma) Function");
	m_functionsGroups << i18n("Synchrotron Functions");
	m_functionsGroups << i18n("Transport Functions");
	m_functionsGroups << i18n("Trigonometric Functions");
	m_functionsGroups << i18n("Zeta Functions");
	// GSL random distribution functions
	m_functionsGroups << i18n("Gaussian Distribution");
	m_functionsGroups << i18n("Exponential Distribution");
	m_functionsGroups << i18n("Laplace Distribution");
	m_functionsGroups << i18n("Exponential Power Distribution");
	m_functionsGroups << i18n("Cauchy Distribution");
	m_functionsGroups << i18n("Rayleigh Distribution");
	m_functionsGroups << i18n("Landau Distribution");
	m_functionsGroups << i18n("Gamma Distribution");
	m_functionsGroups << i18n("Flat (Uniform) Distribution");
	m_functionsGroups << i18n("Lognormal Distribution");
	m_functionsGroups << i18n("Chi-squared Distribution");
	m_functionsGroups << i18n("F-distribution");
	m_functionsGroups << i18n("t-distribution");
	m_functionsGroups << i18n("Beta Distribution");
	m_functionsGroups << i18n("Logistic Distribution");
	m_functionsGroups << i18n("Pareto Distribution");
	m_functionsGroups << i18n("Weibull Distribution");
	m_functionsGroups << i18n("Gumbel Distribution");
	m_functionsGroups << i18n("Poisson Distribution");
	m_functionsGroups << i18n("Bernoulli Distribution");
	m_functionsGroups << i18n("Binomial Distribution");
	m_functionsGroups << i18n("Pascal Distribution");
	m_functionsGroups << i18n("Geometric Distribution");
	m_functionsGroups << i18n("Hypergeometric Distribution");
	m_functionsGroups << i18n("Logarithmic Distribution");

	int index = 0;

	// Standard mathematical functions
	m_functionsNames << i18n("pseudo-random integer [0,RAND_MAX]");
	m_functionsNames << i18n("nonlinear additive feedback rng [0,RAND_MAX]");
	m_functionsNames << i18n("nonlinear additive feedback rng [0,1]");
	m_functionsNames << i18n("Smallest integral value not less");
	m_functionsNames << i18n("Absolute value");

	m_functionsNames << i18n("Base 10 logarithm");
	m_functionsNames << i18n("Power function [x^y]");
	m_functionsNames << i18n("Nonnegative square root");
	m_functionsNames << i18n("Sign function");
	m_functionsNames << i18n("Heavyside theta function");
	m_functionsNames << i18n("Harmonic number function");

#ifndef HAVE_WINDOWS
	m_functionsNames << i18n("Cube root");
	m_functionsNames << i18n("Extract the exponent");
	m_functionsNames << i18n("Round to an integer value");
	m_functionsNames << i18n("Round to the nearest integer");
	m_functionsNames << i18n("Round to the nearest integer");
#endif
	m_functionsNames << QString("log(1+x)");
	m_functionsNames << QString("x * 2^e");
	m_functionsNames << QString("x^n");
	m_functionsNames << QString("x^2");
	m_functionsNames << QString("x^3");
	m_functionsNames << QString("x^4");
	m_functionsNames << QString("x^5");
	m_functionsNames << QString("x^6");
	m_functionsNames << QString("x^7");
	m_functionsNames << QString("x^8");
	m_functionsNames << QString("x^9");

#ifndef HAVE_WINDOWS
	for (int i = 0; i < 27; i++)
#else
	for (int i = 0; i < 22; i++)
#endif
		m_functionsGroupIndex << index;

	// Airy Functions and Derivatives
	m_functionsNames << i18n("Airy function of the first kind");
	m_functionsNames << i18n("Airy function of the second kind");
	m_functionsNames << i18n("Scaled Airy function of the first kind");
	m_functionsNames << i18n("Scaled Airy function of the second kind");
	m_functionsNames << i18n("Airy function derivative of the first kind");
	m_functionsNames << i18n("Airy function derivative of the second kind");
	m_functionsNames << i18n("Scaled Airy function derivative of the first kind");
	m_functionsNames << i18n("Scaled Airy function derivative of the second kind");
	m_functionsNames << i18n("n-th zero of the Airy function of the first kind");
	m_functionsNames << i18n("n-th zero of the Airy function of the second kind");
	m_functionsNames << i18n("n-th zero of the Airy function derivative of the first kind");
	m_functionsNames << i18n("n-th zero of the Airy function derivative of the second kind");

	index++;
	for (int i = 0; i < 12; i++)
		m_functionsGroupIndex << index;

	// Bessel Functions
	m_functionsNames << i18n("Regular cylindrical Bessel function of zeroth order");
	m_functionsNames << i18n("Regular cylindrical Bessel function of first order");
	m_functionsNames << i18n("Regular cylindrical Bessel function of order n");
	m_functionsNames << i18n("Irregular cylindrical Bessel function of zeroth order");
	m_functionsNames << i18n("Irregular cylindrical Bessel function of first order");
	m_functionsNames << i18n("Irregular cylindrical Bessel function of order n");
	m_functionsNames << i18n("Regular modified cylindrical Bessel function of zeroth order");
	m_functionsNames << i18n("Regular modified cylindrical Bessel function of first order");
	m_functionsNames << i18n("Regular modified cylindrical Bessel function of order n");
	m_functionsNames << i18n("Scaled regular modified cylindrical Bessel function of zeroth order exp(-|x|) I0(x)");

	m_functionsNames << i18n("Scaled regular modified cylindrical Bessel function of first order exp(-|x|) I1(x)");
	m_functionsNames << i18n("Scaled regular modified cylindrical Bessel function of order n exp(-|x|) In(x)");
	m_functionsNames << i18n("Irregular modified cylindrical Bessel function of zeroth order");
	m_functionsNames << i18n("Irregular modified cylindrical Bessel function of first order");
	m_functionsNames << i18n("Irregular modified cylindrical Bessel function of order n");
	m_functionsNames << i18n("Scaled irregular modified cylindrical Bessel function of zeroth order exp(x) K0(x)");
	m_functionsNames << i18n("Scaled irregular modified cylindrical Bessel function of first order exp(x) K1(x)");
	m_functionsNames << i18n("Scaled irregular modified cylindrical Bessel function of order n exp(x) Kn(x)");
	m_functionsNames << i18n("Regular spherical Bessel function of zeroth order");
	m_functionsNames << i18n("Regular spherical Bessel function of first order");

	m_functionsNames << i18n("Regular spherical Bessel function of second order");
	m_functionsNames << i18n("Regular spherical Bessel function of order l");
	m_functionsNames << i18n("Irregular spherical Bessel function of zeroth order");
	m_functionsNames << i18n("Irregular spherical Bessel function of first order");
	m_functionsNames << i18n("Irregular spherical Bessel function of second order");
	m_functionsNames << i18n("Irregular spherical Bessel function of order l");
	m_functionsNames << i18n("Scaled regular modified spherical Bessel function of zeroth order, exp(-|x|) i0(x)");
	m_functionsNames << i18n("Scaled regular modified spherical Bessel function of first order, exp(-|x|) i1(x)");
	m_functionsNames << i18n("Scaled regular modified spherical Bessel function of second order, exp(-|x|) i2(x)");
	m_functionsNames << i18n("Scaled regular modified spherical Bessel function of order l, exp(-|x|) il(x)");

	m_functionsNames << i18n("Scaled irregular modified spherical Bessel function of zeroth order, exp(x) k0(x)");
	m_functionsNames << i18n("Scaled irregular modified spherical Bessel function of first order, exp(-|x|) k1(x)");
	m_functionsNames << i18n("Scaled irregular modified spherical Bessel function of second order, exp(-|x|) k2(x)");
	m_functionsNames << i18n("Scaled irregular modified spherical Bessel function of order l, exp(-|x|) kl(x)");
	m_functionsNames << i18n("Regular cylindrical Bessel function of fractional order");
	m_functionsNames << i18n("Irregular cylindrical Bessel function of fractional order");
	m_functionsNames << i18n("Regular modified Bessel function of fractional order");
	m_functionsNames << i18n("Scaled regular modified Bessel function of fractional order");
	m_functionsNames << i18n("Irregular modified Bessel function of fractional order");
	m_functionsNames << i18n("Logarithm of irregular modified Bessel function of fractional order");

	m_functionsNames << i18n("Scaled irregular modified Bessel function of fractional order");
	m_functionsNames << i18n("n-th positive zero of the Bessel function J0");
	m_functionsNames << i18n("n-th positive zero of the Bessel function J1");
	m_functionsNames << i18n("n-th positive zero of the Bessel function Jnu");

	index++;
	for (int i = 0; i < 44; i++)
		m_functionsGroupIndex << index;

	// Clausen Functions
	m_functionsNames << i18n("Clausen function");
	index++;
	m_functionsGroupIndex << index;

	// Coulomb Functions
	m_functionsNames << i18n("Lowest-order normalized hydrogenic bound state radial wavefunction");
	m_functionsNames << i18n("n-th normalized hydrogenic bound state radial wavefunction");

	index++;
	for (int i = 0; i < 2; i++)
		m_functionsGroupIndex << index;

	// Dawson Function
	m_functionsNames << i18n("Dawson integral");
	index++;
	m_functionsGroupIndex << index;

	// Debye Functions
	m_functionsNames << i18n("First-order Debye function");
	m_functionsNames << i18n("Second-order Debye function");
	m_functionsNames << i18n("Third-order Debye function");
	m_functionsNames << i18n("Fourth-order Debye function");
	m_functionsNames << i18n("Fifth-order Debye function");
	m_functionsNames << i18n("Sixth-order Debye function");

	index++;
	for (int i = 0; i < 6; i++)
		m_functionsGroupIndex << index;

	// Dilogarithm
	m_functionsNames << i18n("Dilogarithm for a real argument");
	index++;
	m_functionsGroupIndex << index;

	// Elliptic Integrals
	m_functionsNames << i18n("Legendre form of complete elliptic integral K");
	m_functionsNames << i18n("Legendre form of complete elliptic integral E");
	m_functionsNames << i18n("Legendre form of complete elliptic integral Pi");
	m_functionsNames << i18n("Legendre form of incomplete elliptic integral F");
	m_functionsNames << i18n("Legendre form of incomplete elliptic integral E");
	m_functionsNames << i18n("Legendre form of incomplete elliptic integral P");
	m_functionsNames << i18n("Legendre form of incomplete elliptic integral D");
	m_functionsNames << i18n("Carlson form of incomplete elliptic integral RC");
	m_functionsNames << i18n("Carlson form of incomplete elliptic integral RD");
	m_functionsNames << i18n("Carlson form of incomplete elliptic integral RF");
	m_functionsNames << i18n("Carlson form of incomplete elliptic integral RJ");

	index++;
	for (int i = 0; i < 11; i++)
		m_functionsGroupIndex << index;

	// Error Functions
	m_functionsNames << i18n("Error function");
	m_functionsNames << i18n("Complementary error function");
	m_functionsNames << i18n("Logarithm of complementary error function");
	m_functionsNames << i18n("Gaussian probability density function Z");
	m_functionsNames << i18n("Upper tail of the Gaussian probability function Q");
	m_functionsNames << i18n("Hazard function for the normal distribution Z/Q");
	int count = 6;
#ifndef _MSC_VER
	m_functionsNames << i18n("Underflow-compensating function exp(x^2) erfc(x) for real x");
	m_functionsNames << i18n("Imaginary error function erfi(x) = -i erf(ix) for real x");
	m_functionsNames << i18n("Imaginary part of Faddeeva's scaled complex error function w(x) = exp(-x^2) erfc(-ix) for real x");
	m_functionsNames << i18n("Dawson's integral D(z) = sqrt(pi)/2 * exp(-z^2) * erfi(z)");
	m_functionsNames << i18n("Voigt profile");
	count += 5;
#endif
	m_functionsNames << i18n("Pseudo-Voigt profile (same width)");
	count += 1;

	index++;
	for (int i = 0; i < count; i++)
		m_functionsGroupIndex << index;

	// Exponential Functions
	m_functionsNames << i18n("Exponential function");
	m_functionsNames << i18n("exponentiate x and multiply by y");
	m_functionsNames << QString("exp(x) - 1");
	m_functionsNames << QString("(exp(x)-1)/x");
	m_functionsNames << QString("2(exp(x)-1-x)/x^2");
	m_functionsNames << i18n("n-relative exponential");

	index++;
	for (int i = 0; i < 6; i++)
		m_functionsGroupIndex << index;

	// Exponential Integrals
	m_functionsNames << i18n("Exponential integral");
	m_functionsNames << i18n("Second order exponential integral");
	m_functionsNames << i18n("Exponential integral of order n");
	m_functionsNames << i18n("Exponential integral Ei");
	m_functionsNames << i18n("Hyperbolic integral Shi");
	m_functionsNames << i18n("Hyperbolic integral Chi");
	m_functionsNames << i18n("Third-order exponential integral");
	m_functionsNames << i18n("Sine integral");
	m_functionsNames << i18n("Cosine integral");
	m_functionsNames << i18n("Arctangent integral");

	index++;
	for (int i = 0; i < 10; i++)
		m_functionsGroupIndex << index;

	// Fermi-Dirac Function
	m_functionsNames << i18n("Complete Fermi-Dirac integral with index -1");
	m_functionsNames << i18n("Complete Fermi-Dirac integral with index 0");
	m_functionsNames << i18n("Complete Fermi-Dirac integral with index 1");
	m_functionsNames << i18n("Complete Fermi-Dirac integral with index 2");
	m_functionsNames << i18n("Complete Fermi-Dirac integral with integer index j");
	m_functionsNames << i18n("Complete Fermi-Dirac integral with index -1/2");
	m_functionsNames << i18n("Complete Fermi-Dirac integral with index 1/2");
	m_functionsNames << i18n("Complete Fermi-Dirac integral with index 3/2");
	m_functionsNames << i18n("Incomplete Fermi-Dirac integral with index zero");

	index++;
	for (int i = 0; i < 9; i++)
		m_functionsGroupIndex << index;

	// Gamma and Beta Functions
	m_functionsNames << i18n("Gamma function");
	m_functionsNames << i18n("Gamma function");
	m_functionsNames << i18n("Logarithm of the gamma function");
	m_functionsNames << i18n("Logarithm of the gamma function");
	m_functionsNames << i18n("Regulated gamma function");
	m_functionsNames << i18n("Reciprocal of the gamma function");
	m_functionsNames << i18n("Factorial n!");
	m_functionsNames << i18n("Double factorial n!!");
	m_functionsNames << i18n("Logarithm of the factorial");
	m_functionsNames << i18n("Logarithm of the double factorial");

	m_functionsNames << i18n("Combinatorial factor");
	m_functionsNames << i18n("Logarithm of the combinatorial factor");
	m_functionsNames << i18n("Taylor coefficient");
	m_functionsNames << i18n("Pochhammer symbol");
	m_functionsNames << i18n("Logarithm of the Pochhammer symbol");
	m_functionsNames << i18n("Relative Pochhammer symbol");
	m_functionsNames << i18n("Unnormalized incomplete gamma function");
	m_functionsNames << i18n("Normalized incomplete gamma function");
	m_functionsNames << i18n("Complementary normalized incomplete gamma function");
	m_functionsNames << i18n("Beta function");

	m_functionsNames << i18n("Logarithm of the beta function");
	m_functionsNames << i18n("Normalized incomplete beta function");

	index++;
	for (int i = 0; i < 22; i++)
		m_functionsGroupIndex << index;

	// Gegenbauer Functions
	m_functionsNames << i18n("Gegenbauer polynomial C_1");
	m_functionsNames << i18n("Gegenbauer polynomial C_2");
	m_functionsNames << i18n("Gegenbauer polynomial C_3");
	m_functionsNames << i18n("Gegenbauer polynomial C_n");

	index++;
	for (int i = 0; i < 4; i++)
		m_functionsGroupIndex << index;

#if (GSL_MAJOR_VERSION > 2) || (GSL_MAJOR_VERSION == 2) && (GSL_MINOR_VERSION >= 4)
	// Hermite Polynomials and Functions
	m_functionsNames << i18n("Hermite polynomials physicists version");
	m_functionsNames << i18n("Hermite polynomials probabilists version");
	m_functionsNames << i18n("Hermite functions");
	m_functionsNames << i18n("Derivatives of Hermite polynomials physicists version");
	m_functionsNames << i18n("Derivatives of Hermite polynomials probabilists version");
	m_functionsNames << i18n("Derivatives of Hermite functions");

	index++;
	for (int i = 0; i < 6; i++)
		m_functionsGroupIndex << index;
#endif

	// Hypergeometric Functions
	m_functionsNames << i18n("Hypergeometric function 0F1");
	m_functionsNames << i18n("Confluent hypergeometric function 1F1 for integer parameters");
	m_functionsNames << i18n("Confluent hypergeometric function 1F1 for general parameters");
	m_functionsNames << i18n("Confluent hypergeometric function U for integer parameters");
	m_functionsNames << i18n("Confluent hypergeometric function U");
	m_functionsNames << i18n("Gauss hypergeometric function 2F1");
	m_functionsNames << i18n("Gauss hypergeometric function 2F1 with complex parameters");
	m_functionsNames << i18n("Renormalized Gauss hypergeometric function 2F1");
	m_functionsNames << i18n("Renormalized Gauss hypergeometric function 2F1 with complex parameters");
	m_functionsNames << i18n("Hypergeometric function 2F0");

	index++;
	for (int i = 0; i < 10; i++)
		m_functionsGroupIndex << index;

	// Laguerre Functions
	m_functionsNames << i18n("generalized Laguerre polynomials L_1");
	m_functionsNames << i18n("generalized Laguerre polynomials L_2");
	m_functionsNames << i18n("generalized Laguerre polynomials L_3");

	index++;
	for (int i = 0; i < 3; i++)
		m_functionsGroupIndex << index;

	// Lambert W Functions
	m_functionsNames << i18n("Principal branch of the Lambert W function");
	m_functionsNames << i18n("Secondary real-valued branch of the Lambert W function");

	index++;
	for (int i = 0; i < 2; i++)
		m_functionsGroupIndex << index;

	// Legendre Functions and Spherical Harmonics
	m_functionsNames << i18n("Legendre polynomial P_1");
	m_functionsNames << i18n("Legendre polynomial P_2");
	m_functionsNames << i18n("Legendre polynomial P_3");
	m_functionsNames << i18n("Legendre polynomial P_l");
	m_functionsNames << i18n("Legendre function Q_0");
	m_functionsNames << i18n("Legendre function Q_1");
	m_functionsNames << i18n("Legendre function Q_l");
	m_functionsNames << i18n("Associated Legendre polynomial");
	m_functionsNames << i18n("Normalized associated Legendre polynomial");
	m_functionsNames << i18n("Irregular spherical conical function P^1/2");

	m_functionsNames << i18n("Regular spherical conical function P^(-1/2)");
	m_functionsNames << i18n("Conical function P^0");
	m_functionsNames << i18n("Conical function P^1");
	m_functionsNames << i18n("Regular spherical conical function P^(-1/2-l)");
	m_functionsNames << i18n("Regular cylindrical conical function P^(-m)");
	m_functionsNames << i18n("Zeroth radial eigenfunction of the Laplacian on the 3-dimensional hyperbolic space");
	m_functionsNames << i18n("First radial eigenfunction of the Laplacian on the 3-dimensional hyperbolic space");
	m_functionsNames << i18n("l-th radial eigenfunction of the Laplacian on the 3-dimensional hyperbolic space");

	index++;
	for (int i = 0; i < 18; i++)
		m_functionsGroupIndex << index;

	// Logarithm and Related Functions
	m_functionsNames << i18n("Logarithm");
	m_functionsNames << i18n("Logarithm of the magnitude");
	m_functionsNames << QString("log(1+x)");
	m_functionsNames << QString("log(1+x) - x");

	index++;
	for (int i = 0; i < 4; i++)
		m_functionsGroupIndex << index;

	// Power Function
	m_functionsNames << i18n("x^n for integer n with an error estimate");
	index++;
	m_functionsGroupIndex << index;

	// Psi (Digamma) Function
	m_functionsNames << i18n("Digamma function for positive integer n");
	m_functionsNames << i18n("Digamma function");
	m_functionsNames << i18n("Real part of the digamma function on the line 1+i y");
	m_functionsNames << i18n("Trigamma function psi' for positive integer n");
	m_functionsNames << i18n("Trigamma function psi'");
	m_functionsNames << i18n("Polygamma function psi^(n)");

	index++;
	for (int i = 0; i < 6; i++)
		m_functionsGroupIndex << index;

	// Synchrotron Functions
	m_functionsNames << i18n("First synchrotron function");
	m_functionsNames << i18n("Second synchrotron function");

	index++;
	for (int i = 0; i < 2; i++)
		m_functionsGroupIndex << index;

	// Transport Functions
	m_functionsNames << i18n("Transport function");
	m_functionsNames << i18n("Transport function");
	m_functionsNames << i18n("Transport function");
	m_functionsNames << i18n("Transport function");

	index++;
	for (int i = 0; i < 4; i++)
		m_functionsGroupIndex << index;

	// Trigonometric Functions
	m_functionsNames << i18n("Sine");
	m_functionsNames << i18n("Cosine");
	m_functionsNames << i18n("Tangent");
	m_functionsNames << i18n("Inverse sine");
	m_functionsNames << i18n("Inverse cosine");
	m_functionsNames << i18n("Inverse tangent");
	m_functionsNames << i18n("Inverse tangent using sign");
	m_functionsNames << i18n("Hyperbolic sine");
	m_functionsNames << i18n("Hyperbolic cosine");
	m_functionsNames << i18n("Hyperbolic tangent");
	m_functionsNames << i18n("Inverse hyperbolic cosine");
	m_functionsNames << i18n("Inverse hyperbolic sine");
	m_functionsNames << i18n("Inverse hyperbolic tangent");
	m_functionsNames << i18n("Secant");
	m_functionsNames << i18n("Cosecant");
	m_functionsNames << i18n("Cotangent");
	m_functionsNames << i18n("Inverse secant");
	m_functionsNames << i18n("Inverse cosecant");
	m_functionsNames << i18n("Inverse cotangent");
	m_functionsNames << i18n("Hyperbolic secant");
	m_functionsNames << i18n("Hyperbolic cosecant");
	m_functionsNames << i18n("Hyperbolic cotangent");
	m_functionsNames << i18n("Inverse hyperbolic secant");
	m_functionsNames << i18n("Inverse hyperbolic cosecant");
	m_functionsNames << i18n("Inverse hyperbolic cotangent");
	m_functionsNames << i18n("Sinc function sin(x)/x");
	m_functionsNames << QString("log(sinh(x))");
	m_functionsNames << QString("log(cosh(x))");
	m_functionsNames << i18n("Hypotenuse function");
	m_functionsNames << i18n("Three component hypotenuse function");
	m_functionsNames << i18n("restrict to [-pi,pi]");
	m_functionsNames << i18n("restrict to [0,2 pi]");

	index++;
	for (int i = 0; i < 32; i++)
		m_functionsGroupIndex << index;

	// Zeta Functions
	m_functionsNames << i18n("Riemann zeta function for integer n");
	m_functionsNames << i18n("Riemann zeta function");
	m_functionsNames << i18n("zeta(n)-1 for integer n");
	m_functionsNames << i18n("zeta(x)-1");
	m_functionsNames << i18n("Hurwitz zeta function");
	m_functionsNames << i18n("Eta function for integer n");
	m_functionsNames << i18n("Eta function");

	index++;
	for (int i = 0; i < 7; i++)
		m_functionsGroupIndex << index;

	// GSL Random Number Distributions: see https://www.gnu.org/software/gsl/doc/html/randist.html
	// Gaussian Distribution
	m_functionsNames << i18n("Probability density for a Gaussian distribution");
	m_functionsNames << i18n("Probability density for a unit Gaussian distribution");
	m_functionsNames << i18n("Cumulative distribution function P");
	m_functionsNames << i18n("Cumulative distribution function Q");
	m_functionsNames << i18n("Inverse cumulative distribution function P");
	m_functionsNames << i18n("Inverse cumulative distribution function Q");
	m_functionsNames << i18n("Cumulative unit distribution function P");
	m_functionsNames << i18n("Cumulative unit distribution function Q");
	m_functionsNames << i18n("Inverse cumulative unit distribution function P");
	m_functionsNames << i18n("Inverse cumulative unit distribution function Q");

	m_functionsNames << i18n("Probability density for Gaussian tail distribution");
	m_functionsNames << i18n("Probability density for unit Gaussian tail distribution");
	m_functionsNames << i18n("Probability density for a bivariate Gaussian distribution");

	index++;
	for (int i = 0; i < 13; i++)
		m_functionsGroupIndex << index;

	// Exponential Distribution
	m_functionsNames << i18n("Probability density for an exponential distribution");
	m_functionsNames << i18n("Cumulative distribution function P");
	m_functionsNames << i18n("Cumulative distribution function Q");
	m_functionsNames << i18n("Inverse cumulative distribution function P");
	m_functionsNames << i18n("Inverse cumulative distribution function Q");

	index++;
	for (int i = 0; i < 5; i++)
		m_functionsGroupIndex << index;

	// Laplace Distribution
	m_functionsNames << i18n("Probability density for a Laplace distribution");
	m_functionsNames << i18n("Cumulative distribution function P");
	m_functionsNames << i18n("Cumulative distribution function Q");
	m_functionsNames << i18n("Inverse cumulative distribution function P");
	m_functionsNames << i18n("Inverse cumulative distribution function Q");

	index++;
	for (int i = 0; i < 5; i++)
		m_functionsGroupIndex << index;

	// Exponential Power Distribution
	m_functionsNames << i18n("Probability density for an exponential power distribution");
	m_functionsNames << i18n("cumulative distribution function P");
	m_functionsNames << i18n("Cumulative distribution function Q");

	index++;
	for (int i = 0; i < 3; i++)
		m_functionsGroupIndex << index;

	// Cauchy Distribution
	m_functionsNames << i18n("Probability density for a Cauchy distribution");
	m_functionsNames << i18n("Cumulative distribution function P");
	m_functionsNames << i18n("Cumulative distribution function Q");
	m_functionsNames << i18n("Inverse cumulative distribution function P");
	m_functionsNames << i18n("Inverse cumulative distribution function Q");

	index++;
	for (int i = 0; i < 5; i++)
		m_functionsGroupIndex << index;

	// Rayleigh Distribution
	m_functionsNames << i18n("Probability density for a Rayleigh distribution");
	m_functionsNames << i18n("Cumulative distribution function P");
	m_functionsNames << i18n("Cumulative distribution function Q");
	m_functionsNames << i18n("Inverse cumulative distribution function P");
	m_functionsNames << i18n("Inverse cumulative distribution function Q");
	m_functionsNames << i18n("Probability density for a Rayleigh tail distribution");

	index++;
	for (int i = 0; i < 6; i++)
		m_functionsGroupIndex << index;

	// Landau Distribution
	m_functionsNames << i18n("Probability density for a Landau distribution");
	index++;
	m_functionsGroupIndex << index;

	// Gamma Distribution
	m_functionsNames << i18n("Probability density for a gamma distribution");
	m_functionsNames << i18n("Cumulative distribution function P");
	m_functionsNames << i18n("Cumulative distribution function Q");
	m_functionsNames << i18n("Inverse cumulative distribution function P");
	m_functionsNames << i18n("Inverse cumulative distribution function Q");

	index++;
	for (int i = 0; i < 5; i++)
		m_functionsGroupIndex << index;

	// Flat (Uniform) Distribution
	m_functionsNames << i18n("Probability density for a uniform distribution");
	m_functionsNames << i18n("Cumulative distribution function P");
	m_functionsNames << i18n("Cumulative distribution function Q");
	m_functionsNames << i18n("Inverse cumulative distribution function P");
	m_functionsNames << i18n("Inverse cumulative distribution function Q");

	index++;
	for (int i = 0; i < 5; i++)
		m_functionsGroupIndex << index;

	// Lognormal Distribution
	m_functionsNames << i18n("Probability density for a lognormal distribution");
	m_functionsNames << i18n("Cumulative distribution function P");
	m_functionsNames << i18n("Cumulative distribution function Q");
	m_functionsNames << i18n("Inverse cumulative distribution function P");
	m_functionsNames << i18n("Inverse cumulative distribution function Q");

	index++;
	for (int i = 0; i < 5; i++)
		m_functionsGroupIndex << index;

	// Chi-squared Distribution
	m_functionsNames << i18n("Probability density for a chi squared distribution");
	m_functionsNames << i18n("Cumulative distribution function P");
	m_functionsNames << i18n("Cumulative distribution function Q");
	m_functionsNames << i18n("Inverse cumulative distribution function P");
	m_functionsNames << i18n("Inverse cumulative distribution function Q");

	index++;
	for (int i = 0; i < 5; i++)
		m_functionsGroupIndex << index;

	// F-distribution
	m_functionsNames << i18n("Probability density for a F-distribution");
	m_functionsNames << i18n("Cumulative distribution function P");
	m_functionsNames << i18n("Cumulative distribution function Q");
	m_functionsNames << i18n("Inverse cumulative distribution function P");
	m_functionsNames << i18n("Inverse cumulative distribution function Q");

	index++;
	for (int i = 0; i < 5; i++)
		m_functionsGroupIndex << index;

	// t-distribution
	m_functionsNames << i18n("Probability density for a t-distribution");
	m_functionsNames << i18n("Cumulative distribution function P");
	m_functionsNames << i18n("Cumulative distribution function Q");
	m_functionsNames << i18n("Inverse cumulative distribution function P");
	m_functionsNames << i18n("Inverse cumulative distribution function Q");

	index++;
	for (int i = 0; i < 5; i++)
		m_functionsGroupIndex << index;

	// Beta Distribution
	m_functionsNames << i18n("Probability density for a beta distribution");
	m_functionsNames << i18n("Cumulative distribution function P");
	m_functionsNames << i18n("Cumulative distribution function Q");
	m_functionsNames << i18n("Inverse cumulative distribution function P");
	m_functionsNames << i18n("Inverse cumulative distribution function Q");

	index++;
	for (int i = 0; i < 5; i++)
		m_functionsGroupIndex << index;

	// Logistic Distribution
	m_functionsNames << i18n("Probability density for a logistic distribution");
	m_functionsNames << i18n("Cumulative distribution function P");
	m_functionsNames << i18n("Cumulative distribution function Q");
	m_functionsNames << i18n("Inverse cumulative distribution function P");
	m_functionsNames << i18n("Inverse cumulative distribution function Q");

	index++;
	for (int i = 0; i < 5; i++)
		m_functionsGroupIndex << index;

	// Pareto Distribution
	m_functionsNames << i18n("Probability density for a Pareto distribution");
	m_functionsNames << i18n("Cumulative distribution function P");
	m_functionsNames << i18n("Cumulative distribution function Q");
	m_functionsNames << i18n("Inverse cumulative distribution function P");
	m_functionsNames << i18n("Inverse cumulative distribution function Q");

	index++;
	for (int i = 0; i < 5; i++)
		m_functionsGroupIndex << index;

	// Weibull Distribution
	m_functionsNames << i18n("Probability density for a Weibull distribution");
	m_functionsNames << i18n("Cumulative distribution function P");
	m_functionsNames << i18n("Cumulative distribution function Q");
	m_functionsNames << i18n("Inverse cumulative distribution function P");
	m_functionsNames << i18n("Inverse cumulative distribution function Q");

	index++;
	for (int i = 0; i < 5; i++)
		m_functionsGroupIndex << index;

	// Gumbel Distribution
	m_functionsNames << i18n("Probability density for a Type-1 Gumbel distribution");
	m_functionsNames << i18n("Cumulative distribution function P");
	m_functionsNames << i18n("Cumulative distribution function Q");
	m_functionsNames << i18n("Inverse cumulative distribution function P");
	m_functionsNames << i18n("Inverse cumulative distribution function Q");
	m_functionsNames << i18n("Probability density for a Type-2 Gumbel distribution");
	m_functionsNames << i18n("Cumulative distribution function P");
	m_functionsNames << i18n("Cumulative distribution function Q");
	m_functionsNames << i18n("Inverse cumulative distribution function P");
	m_functionsNames << i18n("Inverse cumulative distribution function Q");

	index++;
	for (int i = 0; i < 10; i++)
		m_functionsGroupIndex << index;

	// Poisson Distribution
	m_functionsNames << i18n("Probability density for a Poisson distribution");
	m_functionsNames << i18n("Cumulative distribution function P");
	m_functionsNames << i18n("Cumulative distribution function Q");

	index++;
	for (int i = 0; i < 3; i++)
		m_functionsGroupIndex << index;

	// Bernoulli Distribution
	m_functionsNames << i18n("Probability density for a Bernoulli distribution");
	index++;
	m_functionsGroupIndex << index;

	// Binomial Distribution
	m_functionsNames << i18n("Probability density for a binomial distribution");
	m_functionsNames << i18n("Cumulative distribution function P");
	m_functionsNames << i18n("Cumulative distribution function Q");
	m_functionsNames << i18n("Probability density for a negative binomial distribution");
	m_functionsNames << i18n("Cumulative distribution function P");
	m_functionsNames << i18n("Cumulative distribution function Q");

	index++;
	for (int i = 0; i < 6; i++)
		m_functionsGroupIndex << index;

	// Pascal Distribution
	m_functionsNames << i18n("Probability density for a Pascal distribution");
	m_functionsNames << i18n("Cumulative distribution function P");
	m_functionsNames << i18n("Cumulative distribution function Q");

	index++;
	for (int i = 0; i < 3; i++)
		m_functionsGroupIndex << index;

	// Geometric Distribution
	m_functionsNames << i18n("Probability density for a geometric distribution");
	m_functionsNames << i18n("Cumulative distribution function P");
	m_functionsNames << i18n("Cumulative distribution function Q");

	index++;
	for (int i = 0; i < 3; i++)
		m_functionsGroupIndex << index;

	// Hypergeometric Distribution
	m_functionsNames << i18n("Probability density for a hypergeometric distribution");
	m_functionsNames << i18n("Cumulative distribution function P");
	m_functionsNames << i18n("Cumulative distribution function Q");

	index++;
	for (int i = 0; i < 3; i++)
		m_functionsGroupIndex << index;

	// Logarithmic Distribution
	m_functionsNames << i18n("Probability density for a logarithmic distribution");
	index++;
	m_functionsGroupIndex << index;
}

//TODO: decide whether we want to have i18n here in the backend part of the code
void ExpressionParser::initConstants() {
	for (int i = 0; _constants[i].name != nullptr; i++)
		m_constants << _constants[i].name;

	//groups
	m_constantsGroups << i18n("Mathematical constants");
	m_constantsGroups << i18n("Fundamental constants");
	m_constantsGroups << i18n("Astronomy and Astrophysics");
	m_constantsGroups << i18n("Atomic and Nuclear Physics");
	m_constantsGroups << i18n("Measurement of Time");
	m_constantsGroups << i18n("Imperial Units");
	m_constantsGroups << i18n("Speed and Nautical Units");
	m_constantsGroups << i18n("Printers Units");
	m_constantsGroups << i18n("Volume, Area and Length");
	m_constantsGroups << i18n("Mass and Weight");
	m_constantsGroups << i18n("Thermal Energy and Power");
	m_constantsGroups << i18n("Pressure");
	m_constantsGroups << i18n("Viscosity");
	m_constantsGroups << i18n("Light and Illumination");
	m_constantsGroups << i18n("Radioactivity");
	m_constantsGroups << i18n("Force and Energy");

	//Mathematical constants
	m_constantsNames << i18n("Base of exponentials");
	m_constantsValues << QString::number(M_E,'g',15);
	m_constantsUnits << QString();
	m_constantsNames << i18n("Pi");
	m_constantsValues << QString::number(M_PI,'g',15);
	m_constantsUnits << QString();
	m_constantsNames << i18n("Euler's constant");
	m_constantsValues << QString::number(M_EULER,'g',15);
	m_constantsUnits << QString();

	for (int i = 0; i < 3; i++)
		m_constantsGroupIndex << 0;

	//Fundamental constants
	m_constantsNames << i18n("Speed of light");
	m_constantsValues << QString::number(GSL_CONST_MKSA_SPEED_OF_LIGHT,'g',15);
	m_constantsUnits << "m / s";
	m_constantsNames << i18n("Vacuum permeability");
	m_constantsValues << QString::number(GSL_CONST_MKSA_VACUUM_PERMEABILITY,'g',15);
	m_constantsUnits << "kg m / A^2 s^2";
	m_constantsNames << i18n("Vacuum permittivity");
	m_constantsValues << QString::number(GSL_CONST_MKSA_VACUUM_PERMITTIVITY,'g',15);
	m_constantsUnits << "A^2 s^4 / kg m^3";
	m_constantsNames << i18n("Planck constant");
	m_constantsValues << QString::number(GSL_CONST_MKSA_PLANCKS_CONSTANT_H,'g',15);
	m_constantsUnits << "kg m^2 / s";
	m_constantsNames << i18n("Reduced Planck constant");
	m_constantsValues << QString::number(GSL_CONST_MKSA_PLANCKS_CONSTANT_HBAR,'g',15);
	m_constantsUnits << "kg m^2 / s";
	m_constantsNames << i18n("Avogadro constant");
	m_constantsValues << QString::number(GSL_CONST_NUM_AVOGADRO,'g',15);
	m_constantsUnits << "1 / mol";
	m_constantsNames << i18n("Faraday");
	m_constantsValues << QString::number(GSL_CONST_MKSA_FARADAY,'g',15);
	m_constantsUnits << "A s / mol";
	m_constantsNames << i18n("Boltzmann constant");
	m_constantsValues << QString::number(GSL_CONST_MKSA_BOLTZMANN,'g',15);
	m_constantsUnits << "kg m^2 / K s^2";
	m_constantsNames << i18n("Molar gas");
	m_constantsValues << QString::number(GSL_CONST_MKSA_MOLAR_GAS,'g',15);
	m_constantsUnits << "kg m^2 / K mol s^2";
	m_constantsNames << i18n("Standard gas volume");
	m_constantsValues << QString::number(GSL_CONST_MKSA_STANDARD_GAS_VOLUME,'g',15);
	m_constantsUnits << "m^3 / mol";
	m_constantsNames << i18n("Stefan-Boltzmann constant");
	m_constantsValues << QString::number(GSL_CONST_MKSA_STEFAN_BOLTZMANN_CONSTANT,'g',15);
	m_constantsUnits << "kg / K^4 s^3";
	m_constantsNames << i18n("Gauss");
	m_constantsValues << QString::number(GSL_CONST_MKSA_GAUSS,'g',15);
	m_constantsUnits << "kg / A s^2";

	for (int i = 0; i < 12; i++)
		m_constantsGroupIndex << 1;

	// Astronomy and Astrophysics
	m_constantsNames << i18n("Astronomical unit");
	m_constantsValues << QString::number(GSL_CONST_MKSA_ASTRONOMICAL_UNIT,'g',15);
	m_constantsUnits << "m";
	m_constantsNames << i18n("Gravitational constant");
	m_constantsValues << QString::number(GSL_CONST_MKSA_GRAVITATIONAL_CONSTANT,'g',15);
	m_constantsUnits << "m^3 / kg s^2";
	m_constantsNames << i18n("Light year");
	m_constantsValues << QString::number(GSL_CONST_MKSA_LIGHT_YEAR,'g',15);
	m_constantsUnits << "m";
	m_constantsNames << i18n("Parsec");
	m_constantsValues << QString::number(GSL_CONST_MKSA_PARSEC,'g',15);
	m_constantsUnits << "m";
	m_constantsNames << i18n("Gravitational acceleration");
	m_constantsValues << QString::number(GSL_CONST_MKSA_GRAV_ACCEL,'g',15);
	m_constantsUnits << "m / s^2";
	m_constantsNames << i18n("Solar mass");
	m_constantsValues << QString::number(GSL_CONST_MKSA_SOLAR_MASS,'g',15);
	m_constantsUnits << "kg";

	for (int i = 0; i < 6; i++)
		m_constantsGroupIndex << 2;

	// Atomic and Nuclear Physics;
	m_constantsNames << i18n("Charge of the electron");
	m_constantsValues << QString::number(GSL_CONST_MKSA_ELECTRON_CHARGE,'g',15);
	m_constantsUnits << "A s";
	m_constantsNames << i18n("Energy of 1 electron volt");
	m_constantsValues << QString::number(GSL_CONST_MKSA_ELECTRON_VOLT,'g',15);
	m_constantsUnits << "kg m^2 / s^2";
	m_constantsNames << i18n("Unified atomic mass");
	m_constantsValues << QString::number(GSL_CONST_MKSA_UNIFIED_ATOMIC_MASS,'g',15);
	m_constantsUnits << "kg";
	m_constantsNames << i18n("Mass of the electron");
	m_constantsValues << QString::number(GSL_CONST_MKSA_MASS_ELECTRON,'g',15);
	m_constantsUnits << "kg";
	m_constantsNames << i18n("Mass of the muon");
	m_constantsValues << QString::number(GSL_CONST_MKSA_MASS_MUON,'g',15);
	m_constantsUnits << "kg";
	m_constantsNames << i18n("Mass of the proton");
	m_constantsValues << QString::number(GSL_CONST_MKSA_MASS_PROTON,'g',15);
	m_constantsUnits << "kg";
	m_constantsNames << i18n("Mass of the neutron");
	m_constantsValues << QString::number(GSL_CONST_MKSA_MASS_NEUTRON,'g',15);
	m_constantsUnits << "kg";
	m_constantsNames << i18n("Electromagnetic fine structure constant");
	m_constantsValues << QString::number(GSL_CONST_NUM_FINE_STRUCTURE,'g',15);
	m_constantsUnits << QString();
	m_constantsNames << i18n("Rydberg constant");
	m_constantsValues << QString::number(GSL_CONST_MKSA_RYDBERG,'g',15);
	m_constantsUnits << "kg m^2 / s^2";
	m_constantsNames << i18n("Bohr radius");
	m_constantsValues << QString::number(GSL_CONST_MKSA_BOHR_RADIUS,'g',15);
	m_constantsUnits << "m";
	m_constantsNames << i18n("Length of 1 angstrom");
	m_constantsValues << QString::number(GSL_CONST_MKSA_ANGSTROM,'g',15);
	m_constantsUnits << "m";
	m_constantsNames << i18n("Area of 1 barn");
	m_constantsValues << QString::number(GSL_CONST_MKSA_BARN,'g',15);
	m_constantsUnits << "m^2";
	m_constantsNames << i18n("Bohr Magneton");
	m_constantsValues << QString::number(GSL_CONST_MKSA_BOHR_MAGNETON,'g',15);
	m_constantsUnits << "A m^2";
	m_constantsNames << i18n("Nuclear Magneton");
	m_constantsValues << QString::number(GSL_CONST_MKSA_NUCLEAR_MAGNETON,'g',15);
	m_constantsUnits << "A m^2";
	m_constantsNames << i18n("Magnetic moment of the electron [absolute value]");
	m_constantsValues << QString::number(GSL_CONST_MKSA_ELECTRON_MAGNETIC_MOMENT,'g',15);
	m_constantsUnits << "A m^2";
	m_constantsNames << i18n("Magnetic moment of the proton");
	m_constantsValues << QString::number(GSL_CONST_MKSA_PROTON_MAGNETIC_MOMENT,'g',15);
	m_constantsUnits << "A m^2";
	m_constantsNames << i18n("Thomson cross section");
	m_constantsValues << QString::number(GSL_CONST_MKSA_THOMSON_CROSS_SECTION,'g',15);
	m_constantsUnits << "m^2";
	m_constantsNames << i18n("Electric dipole moment of 1 Debye");
	m_constantsValues << QString::number(GSL_CONST_MKSA_DEBYE,'g',15);
	m_constantsUnits << "A s^2 / m^2";

	for (int i = 0; i < 18; i++)
		m_constantsGroupIndex << 3;

	// Measurement of Time
	m_constantsNames << i18n("Number of seconds in 1 minute");
	m_constantsValues << QString::number(GSL_CONST_MKSA_MINUTE,'g',15);
	m_constantsUnits << "s";
	m_constantsNames << i18n("Number of seconds in 1 hour");
	m_constantsValues << QString::number(GSL_CONST_MKSA_HOUR,'g',15);
	m_constantsUnits << "s";
	m_constantsNames << i18n("Number of seconds in 1 day");
	m_constantsValues << QString::number(GSL_CONST_MKSA_DAY,'g',15);
	m_constantsUnits << "s";
	m_constantsNames << i18n("Number of seconds in 1 week");
	m_constantsValues << QString::number(GSL_CONST_MKSA_WEEK,'g',15);
	m_constantsUnits << "s";

	for (int i = 0; i < 4; i++)
		m_constantsGroupIndex << 4;

	// Imperial Units
	m_constantsNames << i18n("Length of 1 inch");
	m_constantsValues << QString::number(GSL_CONST_MKSA_INCH,'g',15);
	m_constantsUnits << "m";
	m_constantsNames << i18n("Length of 1 foot");
	m_constantsValues << QString::number(GSL_CONST_MKSA_FOOT,'g',15);
	m_constantsUnits << "m";
	m_constantsNames << i18n("Length of 1 yard");
	m_constantsValues << QString::number(GSL_CONST_MKSA_YARD,'g',15);
	m_constantsUnits << "m";
	m_constantsNames << i18n("Length of 1 mile");
	m_constantsValues << QString::number(GSL_CONST_MKSA_MILE,'g',15);
	m_constantsUnits << "m";
	m_constantsNames << i18n("Length of 1/1000th of an inch");
	m_constantsValues << QString::number(GSL_CONST_MKSA_MIL,'g',15);
	m_constantsUnits << "m";

	for (int i = 0; i < 5; i++)
		m_constantsGroupIndex << 5;

	// Speed and Nautical Units
	m_constantsNames << i18n("Speed of 1 kilometer per hour");
	m_constantsValues << QString::number(GSL_CONST_MKSA_KILOMETERS_PER_HOUR,'g',15);
	m_constantsUnits << "m / s";
	m_constantsNames << i18n("Speed of 1 mile per hour");
	m_constantsValues << QString::number(GSL_CONST_MKSA_MILES_PER_HOUR,'g',15);
	m_constantsUnits << "m / s";
	m_constantsNames << i18n("Length of 1 nautical mile");
	m_constantsValues << QString::number(GSL_CONST_MKSA_NAUTICAL_MILE,'g',15);
	m_constantsUnits << "m";
	m_constantsNames << i18n("Length of 1 fathom");
	m_constantsValues << QString::number(GSL_CONST_MKSA_FATHOM,'g',15);
	m_constantsUnits << "m";
	m_constantsNames << i18n("Speed of 1 knot");
	m_constantsValues << QString::number(GSL_CONST_MKSA_KNOT,'g',15);
	m_constantsUnits << "m / s";

	for (int i = 0; i < 5; i++)
		m_constantsGroupIndex << 6;

	// Printers Units
	m_constantsNames << i18n("length of 1 printer's point [1/72 inch]");
	m_constantsValues << QString::number(GSL_CONST_MKSA_POINT,'g',15);
	m_constantsUnits << "m";
	m_constantsNames << i18n("length of 1 TeX point [1/72.27 inch]");
	m_constantsValues << QString::number(GSL_CONST_MKSA_TEXPOINT,'g',15);
	m_constantsUnits << "m";

	for (int i = 0; i < 2; i++)
		m_constantsGroupIndex << 7;

	// Volume, Area and Length
	m_constantsNames << i18n("Length of 1 micron");
	m_constantsValues << QString::number(GSL_CONST_MKSA_MICRON,'g',15);
	m_constantsUnits << "m";
	m_constantsNames << i18n("Area of 1 hectare");
	m_constantsValues << QString::number(GSL_CONST_MKSA_HECTARE,'g',15);
	m_constantsUnits << "m^2";
	m_constantsNames << i18n("Area of 1 acre");
	m_constantsValues << QString::number(GSL_CONST_MKSA_ACRE,'g',15);
	m_constantsUnits << "m^2";
	m_constantsNames << i18n("Volume of 1 liter");
	m_constantsValues << QString::number(GSL_CONST_MKSA_LITER,'g',15);
	m_constantsUnits << "m^3";
	m_constantsNames << i18n("Volume of 1 US gallon");
	m_constantsValues << QString::number(GSL_CONST_MKSA_US_GALLON,'g',15);
	m_constantsUnits << "m^3";
	m_constantsNames << i18n("Volume of 1 Canadian gallon");
	m_constantsValues << QString::number(GSL_CONST_MKSA_CANADIAN_GALLON,'g',15);
	m_constantsUnits << "m^3";
	m_constantsNames << i18n("Volume of 1 UK gallon");
	m_constantsValues << QString::number(GSL_CONST_MKSA_UK_GALLON,'g',15);
	m_constantsUnits << "m^3";
	m_constantsNames << i18n("Volume of 1 quart");
	m_constantsValues << QString::number(GSL_CONST_MKSA_QUART,'g',15);
	m_constantsUnits << "m^3";
	m_constantsNames << i18n("Volume of 1 pint");
	m_constantsValues << QString::number(GSL_CONST_MKSA_PINT,'g',15);
	m_constantsUnits << "m^3";

	for (int i = 0; i < 9; i++)
		m_constantsGroupIndex << 8;

	// Mass and Weight
	m_constantsNames << i18n("Mass of 1 pound");
	m_constantsValues << QString::number(GSL_CONST_MKSA_POUND_MASS,'g',15);
	m_constantsUnits << "kg";
	m_constantsNames << i18n("Mass of 1 ounce");
	m_constantsValues << QString::number(GSL_CONST_MKSA_OUNCE_MASS,'g',15);
	m_constantsUnits << "kg";
	m_constantsNames << i18n("Mass of 1 ton");
	m_constantsValues << QString::number(GSL_CONST_MKSA_TON,'g',15);
	m_constantsUnits << "kg";
	m_constantsNames << i18n("Mass of 1 metric ton [1000 kg]");
	m_constantsValues << QString::number(GSL_CONST_MKSA_METRIC_TON,'g',15);
	m_constantsUnits << "kg";
	m_constantsNames << i18n("Mass of 1 UK ton");
	m_constantsValues << QString::number(GSL_CONST_MKSA_UK_TON,'g',15);
	m_constantsUnits << "kg";
	m_constantsNames << i18n("Mass of 1 troy ounce");
	m_constantsValues << QString::number(GSL_CONST_MKSA_TROY_OUNCE,'g',15);
	m_constantsUnits << "kg";
	m_constantsNames << i18n("Mass of 1 carat");
	m_constantsValues << QString::number(GSL_CONST_MKSA_CARAT,'g',15);
	m_constantsUnits << "kg";
	m_constantsNames << i18n("Force of 1 gram weight");
	m_constantsValues << QString::number(GSL_CONST_MKSA_GRAM_FORCE,'g',15);
	m_constantsUnits << "kg m / s^2";
	m_constantsNames << i18n("Force of 1 pound weight");
	m_constantsValues << QString::number(GSL_CONST_MKSA_POUND_FORCE,'g',15);
	m_constantsUnits << "kg m / s^2";
	m_constantsNames << i18n("Force of 1 kilopound weight");
	m_constantsValues << QString::number(GSL_CONST_MKSA_KILOPOUND_FORCE,'g',15);
	m_constantsUnits << "kg m / s^2";
	m_constantsNames << i18n("Force of 1 poundal");
	m_constantsValues << QString::number(GSL_CONST_MKSA_POUNDAL,'g',15);
	m_constantsUnits << "kg m / s^2";

	for (int i = 0; i < 11; i++)
		m_constantsGroupIndex << 9;

	// Thermal Energy and Power
	m_constantsNames << i18n("Energy of 1 calorie");
	m_constantsValues << QString::number(GSL_CONST_MKSA_CALORIE,'g',15);
	m_constantsUnits << "kg m^2 / s^2";
	m_constantsNames << i18n("Energy of 1 British Thermal Unit");
	m_constantsValues << QString::number(GSL_CONST_MKSA_BTU,'g',15);
	m_constantsUnits << "kg m^2 / s^2";
	m_constantsNames << i18n("Energy of 1 Therm");
	m_constantsValues << QString::number(GSL_CONST_MKSA_THERM,'g',15);
	m_constantsUnits << "kg m^2 / s^2";
	m_constantsNames << i18n("Power of 1 horsepower");
	m_constantsValues << QString::number(GSL_CONST_MKSA_HORSEPOWER,'g',15);
	m_constantsUnits << "kg m^2 / s^3";

	for (int i = 0; i < 4; i++)
		m_constantsGroupIndex << 10;

	// Pressure
	m_constantsNames << i18n("Pressure of 1 bar");
	m_constantsValues << QString::number(GSL_CONST_MKSA_BAR,'g',15);
	m_constantsUnits << "kg / m s^2";
	m_constantsNames << i18n("Pressure of 1 standard atmosphere");
	m_constantsValues << QString::number(GSL_CONST_MKSA_STD_ATMOSPHERE,'g',15);
	m_constantsUnits << "kg / m s^2";
	m_constantsNames << i18n("Pressure of 1 torr");
	m_constantsValues << QString::number(GSL_CONST_MKSA_TORR,'g',15);
	m_constantsUnits << "kg / m s^2";
	m_constantsNames << i18n("Pressure of 1 meter of mercury");
	m_constantsValues << QString::number(GSL_CONST_MKSA_METER_OF_MERCURY,'g',15);
	m_constantsUnits << "kg / m s^2";
	m_constantsNames << i18n("Pressure of 1 inch of mercury");
	m_constantsValues << QString::number(GSL_CONST_MKSA_INCH_OF_MERCURY,'g',15);
	m_constantsUnits << "kg / m s^2";
	m_constantsNames << i18n("Pressure of 1 inch of water");
	m_constantsValues << QString::number(GSL_CONST_MKSA_INCH_OF_WATER,'g',15);
	m_constantsUnits << "kg / m s^2";
	m_constantsNames << i18n("Pressure of 1 pound per square inch");
	m_constantsValues << QString::number(GSL_CONST_MKSA_PSI,'g',15);
	m_constantsUnits << "kg / m s^2";

	for (int i = 0; i < 7; i++)
		m_constantsGroupIndex << 11;

	// Viscosity
	m_constantsNames << i18n("Dynamic viscosity of 1 poise");
	m_constantsValues << QString::number(GSL_CONST_MKSA_POISE,'g',15);
	m_constantsUnits << "kg / m s";
	m_constantsNames << i18n("Kinematic viscosity of 1 stokes");
	m_constantsValues << QString::number(GSL_CONST_MKSA_STOKES,'g',15);
	m_constantsUnits << "m^2 / s";

	for (int i = 0; i < 2; i++)
		m_constantsGroupIndex << 12;

	// Light and Illumination
	m_constantsNames << i18n("Luminance of 1 stilb");
	m_constantsValues << QString::number(GSL_CONST_MKSA_STILB,'g',15);
	m_constantsUnits << "cd / m^2";
	m_constantsNames << i18n("Luminous flux of 1 lumen");
	m_constantsValues << QString::number(GSL_CONST_MKSA_LUMEN,'g',15);
	m_constantsUnits << "cd sr";
	m_constantsNames << i18n("Illuminance of 1 lux");
	m_constantsValues << QString::number(GSL_CONST_MKSA_LUX,'g',15);
	m_constantsUnits << "cd sr / m^2";
	m_constantsNames << i18n("Illuminance of 1 phot");
	m_constantsValues << QString::number(GSL_CONST_MKSA_PHOT,'g',15);
	m_constantsUnits << "cd sr / m^2";
	m_constantsNames << i18n("Illuminance of 1 footcandle");
	m_constantsValues << QString::number(GSL_CONST_MKSA_FOOTCANDLE,'g',15);
	m_constantsUnits << "cd sr / m^2";
	m_constantsNames << i18n("Luminance of 1 lambert");
	m_constantsValues << QString::number(GSL_CONST_MKSA_LAMBERT,'g',15);
	m_constantsUnits << "cd sr / m^2";
	m_constantsNames << i18n("Luminance of 1 footlambert");
	m_constantsValues << QString::number(GSL_CONST_MKSA_FOOTLAMBERT,'g',15);
	m_constantsUnits << "cd sr / m^2";

	for (int i = 0; i < 7; i++)
		m_constantsGroupIndex << 13;

	// Radioactivity
	m_constantsNames << i18n("Activity of 1 curie");
	m_constantsValues << QString::number(GSL_CONST_MKSA_CURIE,'g',15);
	m_constantsUnits << "1 / s";
	m_constantsNames << i18n("Exposure of 1 roentgen");
	m_constantsValues << QString::number(GSL_CONST_MKSA_ROENTGEN,'g',15);
	m_constantsUnits << "A s / kg";
	m_constantsNames << i18n("Absorbed dose of 1 rad");
	m_constantsValues << QString::number(GSL_CONST_MKSA_RAD,'g',15);
	m_constantsUnits << "m^2 / s^2";

	for (int i = 0; i < 3; i++)
		m_constantsGroupIndex << 14;

	// Force and Energy
	m_constantsNames << i18n("SI unit of force");
	m_constantsValues << QString::number(GSL_CONST_MKSA_NEWTON,'g',15);
	m_constantsUnits << "kg m / s^2";
	m_constantsNames << i18n("Force of 1 Dyne");
	m_constantsValues << QString::number(GSL_CONST_MKSA_DYNE,'g',15);
	m_constantsUnits << "kg m / s^2";
	m_constantsNames << i18n("SI unit of energy");
	m_constantsValues << QString::number(GSL_CONST_MKSA_JOULE,'g',15);
	m_constantsUnits << "kg m^2 / s^2";
	m_constantsNames << i18n("Energy 1 erg");
	m_constantsValues << QString::number(GSL_CONST_MKSA_ERG,'g',15);
	m_constantsUnits << "kg m^2 / s^2";

	for (int i = 0; i < 4; i++)
		m_constantsGroupIndex << 15;
}

/**********************************************************************************/

ExpressionParser::~ExpressionParser() {
	delete_table();
}

ExpressionParser* ExpressionParser::getInstance() {
	if (!m_instance)
		m_instance = new ExpressionParser();

	return m_instance;
}

const QStringList& ExpressionParser::functions() {
	return m_functions;
}

const QStringList& ExpressionParser::functionsGroups() {
	return m_functionsGroups;
}

const QStringList& ExpressionParser::functionsNames() {
	return m_functionsNames;
}

const QVector<int>& ExpressionParser::functionsGroupIndices() {
	return m_functionsGroupIndex;
}

//TODO: number of function arguments (needed for auto fill)

const QStringList& ExpressionParser::constants() {
	return m_constants;
}

const QStringList& ExpressionParser::constantsGroups() {
	return m_constantsGroups;
}

const QStringList& ExpressionParser::constantsNames() {
	return m_constantsNames;
}

const QStringList& ExpressionParser::constantsValues() {
	return m_constantsValues;
}

const QStringList& ExpressionParser::constantsUnits() {
	return m_constantsUnits;
}

const QVector<int>& ExpressionParser::constantsGroupIndices() {
	return m_constantsGroupIndex;
}

bool ExpressionParser::isValid(const QString& expr, const QStringList& vars) {
	QDEBUG(Q_FUNC_INFO << ", expr:" << expr << ", vars:" << vars);
	gsl_set_error_handler_off();

	for (const auto& var: vars)
		assign_symbol(qPrintable(var), 0);

	SET_NUMBER_LOCALE
	parse(qPrintable(expr), qPrintable(numberLocale.name()));

	/*TODO: check if we accidentally remove used constants here */
	for (const auto& var: vars)
		remove_symbol(qPrintable(var));

	return !(parse_errors() > 0);
}

QStringList ExpressionParser::getParameter(const QString& expr, const QStringList& vars) {
	QDEBUG(Q_FUNC_INFO << ", variables:" << vars);
	QStringList parameters;

	QStringList strings = expr.split(QRegularExpression(QStringLiteral("\\W+")), QString::SkipEmptyParts);
	QDEBUG(Q_FUNC_INFO << ", found strings:" << strings);
	// RE for any number
#if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
	const QRegularExpression re(QRegularExpression::anchoredPattern(QStringLiteral("[0-9]*")));
#else
	const QRegularExpression re("\\A(?:" +  QStringLiteral("[0-9]*") + ")\\z");
#endif
	for (const QString& string: strings) {
		QDEBUG(string << ':' << constants().indexOf(string) << ' ' << functions().indexOf(string) << ' '
			       << vars.indexOf(string) << ' ' << re.match(string).hasMatch());
		// check if token is not a known constant/function/variable or number
		if (constants().indexOf(string) == -1 && functions().indexOf(string) == -1
		    && vars.indexOf(string) == -1 && re.match(string).hasMatch() == false)
			parameters << string;
	}
	parameters.removeDuplicates();
	QDEBUG(Q_FUNC_INFO << ", parameters found:" << parameters);

	return parameters;
}

/*
 * Evaluate cartesian expression returning true on success and false if parsing fails
 */
bool ExpressionParser::evaluateCartesian(const QString& expr, const QString& min, const QString& max,
		int count, QVector<double>* xVector, QVector<double>* yVector,
		const QStringList& paramNames, const QVector<double>& paramValues) {
	DEBUG(Q_FUNC_INFO << ", v1")
	gsl_set_error_handler_off();

	const Range<double> range{min, max};
	const double step = range.stepSize(count);

	for (int i = 0; i < paramNames.size(); ++i)
		assign_symbol(qPrintable(paramNames.at(i)), paramValues.at(i));

	SET_NUMBER_LOCALE
	for (int i = 0; i < count; i++) {
		const double x{ range.min() + step * i };
		assign_symbol("x", x);

		const double y{ parse(qPrintable(expr), qPrintable(numberLocale.name())) };
		if (parse_errors() > 0)
			return false;

		(*xVector)[i] = x;
		(*yVector)[i] = y;
	}

	return true;
}

bool ExpressionParser::evaluateCartesian(const QString& expr, const QString& min, const QString& max,
		int count, QVector<double>* xVector, QVector<double>* yVector) {
	DEBUG(Q_FUNC_INFO << ", v2")
	gsl_set_error_handler_off();

	const Range<double> range{min, max};
	const double step = range.stepSize(count);

	SET_NUMBER_LOCALE
	for (int i = 0; i < count; i++) {
		const double x{ range.min() + step * i };
		assign_symbol("x", x);

		const double y{ parse(qPrintable(expr), qPrintable(numberLocale.name())) };
		if (parse_errors() > 0)
			return false;

		(*xVector)[i] = x;
		(*yVector)[i] = y;
	}

	return true;
}

bool ExpressionParser::evaluateCartesian(const QString& expr, QVector<double>* xVector, QVector<double>* yVector) {
	DEBUG(Q_FUNC_INFO << ", v3")
	gsl_set_error_handler_off();

	SET_NUMBER_LOCALE
	for (int i = 0; i < xVector->count(); i++) {
		assign_symbol("x", xVector->at(i));
		const double y = parse(qPrintable(expr), qPrintable(numberLocale.name()));

		if (parse_errors() > 0)
			return false;

		(*yVector)[i] = y;
	}

	return true;
}

bool ExpressionParser::evaluateCartesian(const QString& expr, QVector<double>* xVector, QVector<double>* yVector,
		const QStringList& paramNames, const QVector<double>& paramValues) {
	DEBUG(Q_FUNC_INFO << ", v4")
	gsl_set_error_handler_off();

	for (int i = 0; i < paramNames.size(); ++i)
		assign_symbol(qPrintable(paramNames.at(i)), paramValues.at(i));

	SET_NUMBER_LOCALE
	for (int i = 0; i < xVector->count(); i++) {
		assign_symbol("x", xVector->at(i));

		const double y = parse(qPrintable(expr), qPrintable(numberLocale.name()));
		if (parse_errors() > 0)
			return false;

		(*yVector)[i] = y;
	}

	return true;
}

/*!
	evaluates multivariate function y=f(x_1, x_2, ...).
	Variable names (x_1, x_2, ...) are stored in \c vars.
	Data is stored in \c dataVectors.
 */
bool ExpressionParser::evaluateCartesian(const QString& expr, const QStringList& vars, const QVector<QVector<double>*>& xVectors, QVector<double>* yVector) {
	DEBUG(Q_FUNC_INFO << ", v5")
	Q_ASSERT(vars.size() == xVectors.size());
	gsl_set_error_handler_off();

	//determine the minimal size of involved vectors
	double minSize = INFINITY;
	for (auto* xVector : xVectors) {
		if (xVector->size() < minSize)
			minSize = xVector->size();
	}
	if (yVector->size() < minSize)
		minSize = yVector->size();

	// calculate values
	SET_NUMBER_LOCALE
	for (int i = 0; i < minSize; i++) {
		for (int n = 0; n < vars.size(); ++n)
			assign_symbol(qPrintable(vars.at(n)), xVectors.at(n)->at(i));

		const double y = parse(qPrintable(expr), qPrintable(numberLocale.name()));
		if (parse_errors() > 0)
			return false;

		(*yVector)[i] = y;
	}

	//if the y-vector is longer than the x-vector(s), set all exceeding elements to NAN
	for (int i = minSize; i < yVector->size(); ++i)
		(*yVector)[i] = NAN;

	return true;
}

bool ExpressionParser::evaluatePolar(const QString& expr, const QString& min, const QString& max,
		int count, QVector<double>* xVector, QVector<double>* yVector) {
	gsl_set_error_handler_off();

	const Range<double> range{min, max};
	const double step = range.stepSize(count);

	SET_NUMBER_LOCALE
	for (int i = 0; i < count; i++) {
		const double phi = range.min() + step * i;
		assign_symbol("phi", phi);

		const double r = parse(qPrintable(expr), qPrintable(numberLocale.name()));
		if (parse_errors() > 0)
			return false;

		(*xVector)[i] = r*cos(phi);
		(*yVector)[i] = r*sin(phi);
	}

	return true;
}

bool ExpressionParser::evaluateParametric(const QString& xexpr, const QString& yexpr, const QString& min, const QString& max,
		int count, QVector<double>* xVector, QVector<double>* yVector) {
	gsl_set_error_handler_off();

	const Range<double> range{min, max};
	const double step = range.stepSize(count);

	SET_NUMBER_LOCALE
	for (int i = 0; i < count; i++) {
		assign_symbol("t", range.min() + step * i);

		const double x = parse(qPrintable(xexpr), qPrintable(numberLocale.name()));
		if (parse_errors() > 0)
			return false;

		const double y = parse(qPrintable(yexpr), qPrintable(numberLocale.name()));
		if (parse_errors() > 0)
			return false;

		(*xVector)[i] = x;
		(*yVector)[i] = y;
	}

	return true;
}
