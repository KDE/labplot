/*
	File                 : functions.cpp
	Project              : LabPlot
	Description          : definition of functions
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2014 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2014-2021 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "backend/nsl/nsl_sf_basic.h"
#include "parser.h"
#include "functions.h"

#include <KLocalizedString>

#include <gsl/gsl_cdf.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_sf.h>

#ifdef _MSC_VER
/* avoid intrinsics */
#pragma function(ceil, floor)
#endif

// Function definitions
double greaterThan(const double v1, const double v2);
double greaterEqualThan(const double v1, const double v2);
double lessThan(const double v1, const double v2);
double lessEqualThan(const double v1, const double v2);
double equal(const double v1, const double v2);
double ifCondition(const double condition, const double valueIfTrue, const double valueIfFalse);
double andFunction(const double v1, const double v2);
double orFunction(const double v1, const double v2);
double xorFunction(const double v1, const double v2);
double between(const double x, const double min, const double max);
double outside(const double x, const double min, const double max);
double equalEpsilon(const double v1, const double v2, const double epsilon);
double betweenIncluded(const double x, const double min, const double max);
double outsideIncluded(const double x, const double min, const double max);

// Parameter function definitions
QString parameterXE(int parameterIndex);
QString ifParameterNames(int parameterIndex);
QString nsl_sf_mathieuParameterNames(int parameterIndex);
QString betweenOutsideParameterNames(int parameterIndex);
QString equalEpsilonParameterNames(int parameterIndex);

QString FunctionGroupsToString(FunctionGroups group) {
	switch (group) {
	case FunctionGroups::StandardMathematicalFunctions:
		return i18n("Standard Mathematical Functions");
	case FunctionGroups::ComparisonFunctions:
		return i18n("Comparison Functions");
	case FunctionGroups::LogicalFunctions:
		return i18n("Logical Functions");
	case FunctionGroups::ColumnStatistics:
		return i18n("Column Statistics");
	case FunctionGroups::MovingStatistics:
		return i18n("Moving Statistics");
	case FunctionGroups::AiryFunctionsAndDerivatives:
		return i18n("Airy Functions and Derivatives");
	case FunctionGroups::BesselFunctions:
		return i18n("Bessel Functions");
	case FunctionGroups::ClausenFunctions:
		return i18n("Clausen Functions");
	case FunctionGroups::CoulombFunctions:
		return i18n("Coulomb Functions");
	case FunctionGroups::DawsonFunction:
		return i18n("Dawson Function");
	case FunctionGroups::DebyeFunctions:
		return i18n("Debye Functions");
	case FunctionGroups::Dilogarithm:
		return i18n("Dilogarithm");
	case FunctionGroups::EllipticIntegrals:
		return i18n("Elliptic Integrals");

#ifndef _MSC_VER
	case FunctionGroups::ErrorFunctions:
		return i18n("Error Functions and Related Functions");
#else
	case FunctionGroups::ErrorFunctions:
		return i18n("Error Functions");
#endif
	case FunctionGroups::ExponentialFunctions:
		return i18n("Exponential Functions");
	case FunctionGroups::ExponentialIntegrals:
		return i18n("Exponential Integrals");
	case FunctionGroups::FermiDiracFunction:
		return i18n("Fermi-Dirac Function");
	case FunctionGroups::GammaAndBetaFunctions:
		return i18n("Gamma and Beta Functions");
	case FunctionGroups::GegenbauerFunctions:
		return i18n("Gegenbauer Functions");
#if (GSL_MAJOR_VERSION > 2) || (GSL_MAJOR_VERSION == 2) && (GSL_MINOR_VERSION >= 4)
	case FunctionGroups::HermitePolynomialsAndFunctions:
		return i18n("Hermite Polynomials and Functions");
#endif
	case FunctionGroups::HypergeometricFunctions:
		return i18n("Hypergeometric Functions");
	case FunctionGroups::LaguerreFunctions:
		return i18n("Laguerre Functions");
	case FunctionGroups::LambertWFunctions:
		return i18n("Lambert W Functions");
	case FunctionGroups::LegendreFunctionsAndSphericalHarmonics:
		return i18n("Legendre Functions and Spherical Harmonics");
	case FunctionGroups::LogarithmAndRelatedFunctions:
		return i18n("Logarithm and Related Functions");
#if (GSL_MAJOR_VERSION >= 2)
	case FunctionGroups::MathieuFunctions:
		return i18n("Mathieu Functions");
#endif
	case FunctionGroups::PowerFunction:
		return i18n("Power Function");
	case FunctionGroups::PsiDigammaFunction:
		return i18n("Psi (Digamma) Function");
	case FunctionGroups::SynchrotronFunctions:
		return i18n("Synchrotron Functions");
	case FunctionGroups::TransportFunctions:
		return i18n("Transport Functions");
	case FunctionGroups::TrigonometricFunctions:
		return i18n("Trigonometric Functions");
	case FunctionGroups::ZetaFunctions:
		return i18n("Zeta Functions");
	case FunctionGroups::RandomNumberGenerator:
		return i18n("Random number generator");
	case FunctionGroups::GaussianDistribution:
		return i18n("Gaussian Distribution");
	case FunctionGroups::ExponentialDistribution:
		return i18n("Exponential Distribution");
	case FunctionGroups::LaplaceDistribution:
		return i18n("Laplace Distribution");
	case FunctionGroups::ExponentialPowerDistribution:
		return i18n("Exponential Power Distribution");
	case FunctionGroups::CauchyDistribution:
		return i18n("Cauchy Distribution");
	case FunctionGroups::RayleighDistribution:
		return i18n("Rayleigh Distribution");
	case FunctionGroups::LandauDistribution:
		return i18n("Landau Distribution");
	case FunctionGroups::GammaDistribution:
		return i18n("Gamma Distribution");
	case FunctionGroups::FlatUniformDistribution:
		return i18n("Flat (Uniform) Distribution");
	case FunctionGroups::LognormalDistribution:
		return i18n("Lognormal Distribution");
	case FunctionGroups::ChisquaredDistribution:
		return i18n("Chi-squared Distribution");
	case FunctionGroups::Fdistribution:
		return i18n("F-distribution");
	case FunctionGroups::Tdistribution:
		return i18n("t-distribution");
	case FunctionGroups::BetaDistribution:
		return i18n("Beta Distribution");
	case FunctionGroups::LogisticDistribution:
		return i18n("Logistic Distribution");
	case FunctionGroups::ParetoDistribution:
		return i18n("Pareto Distribution");
	case FunctionGroups::WeibullDistribution:
		return i18n("Weibull Distribution");
	case FunctionGroups::GumbelDistribution:
		return i18n("Gumbel Distribution");
	case FunctionGroups::PoissonDistribution:
		return i18n("Poisson Distribution");
	case FunctionGroups::BernoulliDistribution:
		return i18n("Bernoulli Distribution");
	case FunctionGroups::BinomialDistribution:
		return i18n("Binomial Distribution");
	case FunctionGroups::PascalDistribution:
		return i18n("Pascal Distribution");
	case FunctionGroups::GeometricDistribution:
		return i18n("Geometric Distribution");
	case FunctionGroups::HypergeometricDistribution:
		return i18n("Hypergeometric Distribution");
	case FunctionGroups::LogarithmicDistribution:
		return i18n("Logarithmic Distribution");
	case FunctionGroups::END:
		break;
	}
	return i18n("Unknown Function");
}

const char* colfun_size = "size";
const char* colfun_min = "min";
const char* colfun_max = "max";
const char* colfun_mean = "mean";
const char* colfun_median = "median";
const char* colfun_stdev = "stdev";
const char* colfun_var = "var";
const char* colfun_gm = "gm";
const char* colfun_hm = "hm";
const char* colfun_chm = "chm";
const char* colfun_mode = "mode";
const char* colfun_quartile1 = "quartile1";
const char* colfun_quartile3 = "quartile3";
const char* colfun_iqr = "iqr";
const char* colfun_percentile1 = "percentile1";
const char* colfun_percentile5 = "percentile5";
const char* colfun_percentile10 = "percentile10";
const char* colfun_percentile90 = "percentile90";
const char* colfun_percentile95 = "percentile95";
const char* colfun_percentile99 = "percentile99";
const char* colfun_trimean = "trimean";
const char* colfun_meandev = "meandev";
const char* colfun_meandevmedian = "meandevmedian";
const char* colfun_mediandev = "mediandev";
const char* colfun_skew = "skew";
const char* colfun_kurt = "kurt";
const char* colfun_entropy = "entropy";
const char* colfun_quantile = "quantile";
const char* colfun_percentile = "percentile";

const char* specialfun_cell = "cell";
const char* specialfun_ma = "ma";
const char* specialfun_mr = "mr";
const char* specialfun_smmin = "smmin";
const char* specialfun_smmax = "smmax";
const char* specialfun_sma = "sma";
const char* specialfun_smr = "smr";


// Values independend of the row index!!!
struct funs _const_functions[] = {
	// Important: When adding function here, implement it also in ColumnPrivate!
	{i18n("Size"), colfun_size, (func_t)nsl_sf_dummy, 1, nullptr, FunctionGroups::ColumnStatistics},
	{i18n("Minimum"), colfun_min, (func_t)nsl_sf_dummy, 1, nullptr, FunctionGroups::ColumnStatistics},
	{i18n("Maximum"), colfun_max, (func_t)nsl_sf_dummy, 1, nullptr, FunctionGroups::ColumnStatistics},
	{i18n("Arithmetic mean"), colfun_mean, (func_t)nsl_sf_dummy, 1, nullptr, FunctionGroups::ColumnStatistics},
	{i18n("Median"), colfun_median, (func_t)nsl_sf_dummy, 1, nullptr, FunctionGroups::ColumnStatistics},
	{i18n("Standard deviation"), colfun_stdev, (func_t)nsl_sf_dummy, 1, nullptr, FunctionGroups::ColumnStatistics},
	{i18n("Variance"), colfun_var, (func_t)nsl_sf_dummy, 1, nullptr, FunctionGroups::ColumnStatistics},
	{i18n("Geometric mean"), colfun_gm, (func_t)nsl_sf_dummy, 1, nullptr, FunctionGroups::ColumnStatistics},
	{i18n("Harmonic mean"), colfun_hm, (func_t)nsl_sf_dummy, 1, nullptr, FunctionGroups::ColumnStatistics},
	{i18n("Contraharmonic mean"), colfun_chm, (func_t)nsl_sf_dummy, 1, nullptr, FunctionGroups::ColumnStatistics},
	{i18n("Mode"), colfun_mode, (func_t)nsl_sf_dummy, 1, nullptr, FunctionGroups::ColumnStatistics},
	{i18n("First quartile"), colfun_quartile1, (func_t)nsl_sf_dummy, 1, nullptr, FunctionGroups::ColumnStatistics},
	{i18n("Third quartile"), colfun_quartile3, (func_t)nsl_sf_dummy, 1, nullptr, FunctionGroups::ColumnStatistics},
	{i18n("Interquartile range"), colfun_iqr, (func_t)nsl_sf_dummy, 1, nullptr, FunctionGroups::ColumnStatistics},
	{i18n("1st percentile"), colfun_percentile1, (func_t)nsl_sf_dummy, 1, nullptr, FunctionGroups::ColumnStatistics},
	{i18n("5th percentile"), colfun_percentile5, (func_t)nsl_sf_dummy, 1, nullptr, FunctionGroups::ColumnStatistics},
	{i18n("10th percentile"), colfun_percentile10, (func_t)nsl_sf_dummy, 1, nullptr, FunctionGroups::ColumnStatistics},
	{i18n("90th percentile"), colfun_percentile90, (func_t)nsl_sf_dummy, 1, nullptr, FunctionGroups::ColumnStatistics},
	{i18n("95th percentile"), colfun_percentile95, (func_t)nsl_sf_dummy, 1, nullptr, FunctionGroups::ColumnStatistics},
	{i18n("99th percentile"), colfun_percentile99, (func_t)nsl_sf_dummy, 1, nullptr, FunctionGroups::ColumnStatistics},
	{i18n("Trimean"), colfun_trimean, (func_t)nsl_sf_dummy, 1, nullptr, FunctionGroups::ColumnStatistics},
	{i18n("Mean absolute deviation"), colfun_meandev, (func_t)nsl_sf_dummy, 1, nullptr, FunctionGroups::ColumnStatistics},
	{i18n("Mean absolute deviation around median"), colfun_meandevmedian, (func_t)nsl_sf_dummy, 1, nullptr, FunctionGroups::ColumnStatistics},
	{i18n("Median absolute deviation"), colfun_mediandev, (func_t)nsl_sf_dummy, 1, nullptr, FunctionGroups::ColumnStatistics},
	{i18n("Skewness"), colfun_skew, (func_t)nsl_sf_dummy, 1, nullptr, FunctionGroups::ColumnStatistics},
	{i18n("Kurtosis"), colfun_kurt, (func_t)nsl_sf_dummy, 1, nullptr, FunctionGroups::ColumnStatistics},
	{i18n("Entropy"), colfun_entropy, (func_t)nsl_sf_dummy, 1, nullptr, FunctionGroups::ColumnStatistics},
	{i18n("Quantile"), colfun_quantile, (func_t)nsl_sf_dummy2, 2, nullptr, FunctionGroups::ColumnStatistics},
	{i18n("Percentile"), colfun_percentile, (func_t)nsl_sf_dummy2, 2, nullptr, FunctionGroups::ColumnStatistics},
};
const int _number_constfunctions = sizeof(_const_functions) / sizeof(funs);

// Special functions depending on variables
struct funs _special_functions[] = {
	// Moving Statistics
	// Important: when adding new function, implement them in Expressionhandler!
	{i18n("Cell"), specialfun_cell, (func_t)nsl_sf_dummy2, 2, nullptr, FunctionGroups::MovingStatistics},
	{i18n("Moving Average"), specialfun_ma, (func_t)nsl_sf_dummy, 1, nullptr, FunctionGroups::MovingStatistics},
	{i18n("Moving Range"), specialfun_mr, (func_t)nsl_sf_dummy, 1, nullptr, FunctionGroups::MovingStatistics},
	{i18n("Simple Moving Minimum"), specialfun_smmin, (func_t)nsl_sf_dummy, 2, nullptr, FunctionGroups::MovingStatistics},
	{i18n("Simple Moving Maximum"), specialfun_smmax, (func_t)nsl_sf_dummy, 2, nullptr, FunctionGroups::MovingStatistics},
	{i18n("Simple Moving Average"), specialfun_sma, (func_t)nsl_sf_dummy, 2, nullptr, FunctionGroups::MovingStatistics},
	{i18n("Simple Moving Range"), specialfun_smr, (func_t)nsl_sf_dummy, 2, nullptr, FunctionGroups::MovingStatistics},
	};
const int _number_specialfunctions = sizeof(_special_functions) / sizeof(funs);

/* list of functions (sync with ExpressionParser.cpp!) */
struct funs _functions[] = {
	// Standard Mathematical Functions
	{i18n("pseudo-random integer [0,RAND_MAX]"), "rand", (func_t)nsl_sf_rand, 0, nullptr, FunctionGroups::StandardMathematicalFunctions},
	{i18n("nonlinear additive feedback rng [0,RAND_MAX]"), "random", (func_t)nsl_sf_random, 0, nullptr, FunctionGroups::StandardMathematicalFunctions},
	{i18n("nonlinear additive feedback rng [0,1]"), "drand", (func_t)nsl_sf_drand, 0, nullptr, FunctionGroups::StandardMathematicalFunctions},
	{i18n("Smallest integral value not less"),
	 "ceil",
	 (func_t) static_cast<double (*)(double)>(&ceil),
	 1,
	 nullptr,
	 FunctionGroups::StandardMathematicalFunctions},
	{i18n("Absolute value"), "fabs", (func_t) static_cast<double (*)(double)>(&fabs), 1, nullptr, FunctionGroups::StandardMathematicalFunctions},

	{i18n("Base 10 logarithm"), "log10", (func_t) static_cast<double (*)(double)>(&log10), 1, nullptr, FunctionGroups::StandardMathematicalFunctions},
	{i18n("Power function [x^y]"), "pow", (func_t) static_cast<double (*)(double, double)>(&pow), 2, nullptr, FunctionGroups::StandardMathematicalFunctions},
	{i18n("Nonnegative square root"), "sqrt", (func_t) static_cast<double (*)(double)>(&sqrt), 1, nullptr, FunctionGroups::StandardMathematicalFunctions},
	{i18n("Sign function"), "sgn", (func_t)nsl_sf_sgn, 1, nullptr, FunctionGroups::StandardMathematicalFunctions},
	{i18n("Heavyside theta function"), "theta", (func_t)nsl_sf_theta, 1, nullptr, FunctionGroups::StandardMathematicalFunctions},
	{i18n("Harmonic number function"), "harmonic", (func_t)nsl_sf_harmonic, 1, nullptr, FunctionGroups::StandardMathematicalFunctions},

#ifndef HAVE_WINDOWS
	{i18n("Cube root"), "cbrt", (func_t) static_cast<double (*)(double)>(&cbrt), 1, nullptr, FunctionGroups::StandardMathematicalFunctions},
	{i18n("Extract the exponent"), "logb", (func_t) static_cast<double (*)(double)>(&logb), 1, nullptr, FunctionGroups::StandardMathematicalFunctions},
	{i18n("Round to an integer value"), "rint", (func_t) static_cast<double (*)(double)>(&rint), 1, nullptr, FunctionGroups::StandardMathematicalFunctions},
	{i18n("Round to the nearest integer"),
	 "round",
	 (func_t) static_cast<double (*)(double)>(&round),
	 1,
	 nullptr,
	 FunctionGroups::StandardMathematicalFunctions},
	{i18n("Round to the nearest integer"),
	 "trunc",
	 (func_t) static_cast<double (*)(double)>(&trunc),
	 1,
	 nullptr,
	 FunctionGroups::StandardMathematicalFunctions},
#endif
	{QStringLiteral("log(1+x)"), "log1p", (func_t)gsl_log1p, 1, nullptr, FunctionGroups::StandardMathematicalFunctions},
	{QStringLiteral("x * 2^e"), "ldexp", (func_t)nsl_sf_ldexp, 2, &parameterXE, FunctionGroups::StandardMathematicalFunctions},
	{QStringLiteral("x^y"), "powint", (func_t)nsl_sf_powint, 2, nullptr, FunctionGroups::StandardMathematicalFunctions},
	{QStringLiteral("x^2"), "pow2", (func_t)gsl_pow_2, 1, nullptr, FunctionGroups::StandardMathematicalFunctions},
	{QStringLiteral("x^3"), "pow3", (func_t)gsl_pow_3, 1, nullptr, FunctionGroups::StandardMathematicalFunctions},
	{QStringLiteral("x^4"), "pow4", (func_t)gsl_pow_4, 1, nullptr, FunctionGroups::StandardMathematicalFunctions},
	{QStringLiteral("x^5"), "pow5", (func_t)gsl_pow_5, 1, nullptr, FunctionGroups::StandardMathematicalFunctions},
	{QStringLiteral("x^6"), "pow6", (func_t)gsl_pow_6, 1, nullptr, FunctionGroups::StandardMathematicalFunctions},
	{QStringLiteral("x^7"), "pow7", (func_t)gsl_pow_7, 1, nullptr, FunctionGroups::StandardMathematicalFunctions},
	{QStringLiteral("x^8"), "pow8", (func_t)gsl_pow_8, 1, nullptr, FunctionGroups::StandardMathematicalFunctions},
	{QStringLiteral("x^9"), "pow9", (func_t)gsl_pow_9, 1, nullptr, FunctionGroups::StandardMathematicalFunctions},

	// Comparison Functions
	{i18n("greaterThan"), "greaterThan", (func_t)greaterThan, 2, nullptr, FunctionGroups::ComparisonFunctions},
	{i18n("lessThan"), "lessThan", (func_t)lessThan, 2, nullptr, FunctionGroups::ComparisonFunctions},
	{i18n("greaterEqualThan"), "greaterEqualThan", (func_t)greaterEqualThan, 2, nullptr, FunctionGroups::ComparisonFunctions},
	{i18n("lessEqualThan"), "lessEqualThan", (func_t)lessEqualThan, 2, nullptr, FunctionGroups::ComparisonFunctions},
	{i18n("equal"), "equal", (func_t)equal, 2, nullptr, FunctionGroups::ComparisonFunctions},
	{i18n("equal with epsilon"), "equalE", (func_t)equalEpsilon, 3, &equalEpsilonParameterNames, FunctionGroups::ComparisonFunctions},
	{i18n("between with boundaries included"), "between_inc", (func_t)betweenIncluded, 3, &betweenOutsideParameterNames, FunctionGroups::ComparisonFunctions},
	{i18n("outside with boundaries included"), "outside_inc", (func_t)outsideIncluded, 3, &betweenOutsideParameterNames, FunctionGroups::ComparisonFunctions},
	{i18n("between with boundaries excluded"), "between", (func_t)between, 3, &betweenOutsideParameterNames, FunctionGroups::ComparisonFunctions},
	{i18n("outside with boundaries excluded"), "outside", (func_t)outside, 3, &betweenOutsideParameterNames, FunctionGroups::ComparisonFunctions},

	// Logical
	{i18n("if(condition; ifTrue; ifFalse)"), "if", (func_t)ifCondition, 3, &ifParameterNames, FunctionGroups::LogicalFunctions},
	{i18n("and"), "and", (func_t)andFunction, 2, nullptr, FunctionGroups::LogicalFunctions},
	{i18n("or"), "or", (func_t)orFunction, 2, nullptr, FunctionGroups::LogicalFunctions},
	{i18n("xor"), "xor", (func_t)xorFunction, 2, nullptr, FunctionGroups::LogicalFunctions},

	// https://www.gnu.org/software/gsl/doc/html/specfunc.html
	// Airy Functions and Derivatives
	{i18n("Airy function of the first kind"), "Ai", (func_t)nsl_sf_airy_Ai, 1, nullptr, FunctionGroups::AiryFunctionsAndDerivatives},
	{i18n("Airy function of the second kind"), "Bi", (func_t)nsl_sf_airy_Bi, 1, nullptr, FunctionGroups::AiryFunctionsAndDerivatives},
	{i18n("Scaled Airy function of the first kind"), "Ais", (func_t)nsl_sf_airy_Ais, 1, nullptr, FunctionGroups::AiryFunctionsAndDerivatives},
	{i18n("Scaled Airy function of the second kind"), "Bis", (func_t)nsl_sf_airy_Bis, 1, nullptr, FunctionGroups::AiryFunctionsAndDerivatives},
	{i18n("Airy function derivative of the first kind"), "Aid", (func_t)nsl_sf_airy_Aid, 1, nullptr, FunctionGroups::AiryFunctionsAndDerivatives},
	{i18n("Airy function derivative of the second kind"), "Bid", (func_t)nsl_sf_airy_Bid, 1, nullptr, FunctionGroups::AiryFunctionsAndDerivatives},
	{i18n("Scaled Airy function derivative of the first kind"), "Aids", (func_t)nsl_sf_airy_Aids, 1, nullptr, FunctionGroups::AiryFunctionsAndDerivatives},
	{i18n("Scaled Airy function derivative of the second kind"), "Bids", (func_t)nsl_sf_airy_Bids, 1, nullptr, FunctionGroups::AiryFunctionsAndDerivatives},
	{i18n("n-th zero of the Airy function of the first kind"), "Ai0", (func_t)nsl_sf_airy_0_Ai, 1, nullptr, FunctionGroups::AiryFunctionsAndDerivatives},
	{i18n("n-th zero of the Airy function of the second kind"), "Bi0", (func_t)nsl_sf_airy_0_Bi, 1, nullptr, FunctionGroups::AiryFunctionsAndDerivatives},
	{i18n("n-th zero of the Airy function derivative of the first kind"),
	 "Aid0",
	 (func_t)nsl_sf_airy_0_Aid,
	 1,
	 nullptr,
	 FunctionGroups::AiryFunctionsAndDerivatives},
	{i18n("n-th zero of the Airy function derivative of the second kind"),
	 "Bid0",
	 (func_t)nsl_sf_airy_0_Bid,
	 1,
	 nullptr,
	 FunctionGroups::AiryFunctionsAndDerivatives},

	// Bessel Functions
	{i18n("Regular cylindrical Bessel function of zeroth order"), "J0", (func_t)gsl_sf_bessel_J0, 1, nullptr, FunctionGroups::BesselFunctions},
	{i18n("Regular cylindrical Bessel function of first order"), "J1", (func_t)gsl_sf_bessel_J1, 1, nullptr, FunctionGroups::BesselFunctions},
	{i18n("Regular cylindrical Bessel function of order n"), "Jn", (func_t)nsl_sf_bessel_Jn, 2, nullptr, FunctionGroups::BesselFunctions},
	{i18n("Irregular cylindrical Bessel function of zeroth order"), "Y0", (func_t)gsl_sf_bessel_Y0, 1, nullptr, FunctionGroups::BesselFunctions},
	{i18n("Irregular cylindrical Bessel function of first order"), "Y1", (func_t)gsl_sf_bessel_Y1, 1, nullptr, FunctionGroups::BesselFunctions},
	{i18n("Irregular cylindrical Bessel function of order n"), "Yn", (func_t)nsl_sf_bessel_Yn, 2, nullptr, FunctionGroups::BesselFunctions},
	{i18n("Regular modified cylindrical Bessel function of zeroth order"), "I0", (func_t)gsl_sf_bessel_I0, 1, nullptr, FunctionGroups::BesselFunctions},
	{i18n("Regular modified cylindrical Bessel function of first order"), "I1", (func_t)gsl_sf_bessel_I1, 1, nullptr, FunctionGroups::BesselFunctions},
	{i18n("Regular modified cylindrical Bessel function of order n"), "In", (func_t)nsl_sf_bessel_In, 2, nullptr, FunctionGroups::BesselFunctions},
	{i18n("Scaled regular modified cylindrical Bessel function of zeroth order exp(-|x|) I0(x)"),
	 "I0s",
	 (func_t)gsl_sf_bessel_I0_scaled,
	 1,
	 nullptr,
	 FunctionGroups::BesselFunctions},

	{i18n("Scaled regular modified cylindrical Bessel function of first order exp(-|x|) I1(x)"),
	 "I1s",
	 (func_t)gsl_sf_bessel_I1_scaled,
	 1,
	 nullptr,
	 FunctionGroups::BesselFunctions},
	{i18n("Scaled regular modified cylindrical Bessel function of order n exp(-|x|) In(x)"),
	 "Ins",
	 (func_t)nsl_sf_bessel_Ins,
	 2,
	 nullptr,
	 FunctionGroups::BesselFunctions},
	{i18n("Irregular modified cylindrical Bessel function of zeroth order"), "K0", (func_t)gsl_sf_bessel_K0, 1, nullptr, FunctionGroups::BesselFunctions},
	{i18n("Irregular modified cylindrical Bessel function of first order"), "K1", (func_t)gsl_sf_bessel_K1, 1, nullptr, FunctionGroups::BesselFunctions},
	{i18n("Irregular modified cylindrical Bessel function of order n"), "Kn", (func_t)nsl_sf_bessel_Kn, 2, nullptr, FunctionGroups::BesselFunctions},
	{i18n("Scaled irregular modified cylindrical Bessel function of zeroth order exp(x) K0(x)"),
	 "K0s",
	 (func_t)gsl_sf_bessel_K0_scaled,
	 1,
	 nullptr,
	 FunctionGroups::BesselFunctions},
	{i18n("Scaled irregular modified cylindrical Bessel function of first order exp(x) K1(x)"),
	 "K1s",
	 (func_t)gsl_sf_bessel_K1_scaled,
	 1,
	 nullptr,
	 FunctionGroups::BesselFunctions},
	{i18n("Scaled irregular modified cylindrical Bessel function of order n exp(x) Kn(x)"),
	 "Kns",
	 (func_t)nsl_sf_bessel_Kns,
	 2,
	 nullptr,
	 FunctionGroups::BesselFunctions},
	{i18n("Regular spherical Bessel function of zeroth order"), "j0", (func_t)gsl_sf_bessel_j0, 1, nullptr, FunctionGroups::BesselFunctions},
	{i18n("Regular spherical Bessel function of first order"), "j1", (func_t)gsl_sf_bessel_j1, 1, nullptr, FunctionGroups::BesselFunctions},

	{i18n("Regular spherical Bessel function of second order"), "j2", (func_t)gsl_sf_bessel_j2, 1, nullptr, FunctionGroups::BesselFunctions},
	{i18n("Regular spherical Bessel function of order l"), "jl", (func_t)nsl_sf_bessel_jl, 2, nullptr, FunctionGroups::BesselFunctions},
	{i18n("Irregular spherical Bessel function of zeroth order"), "y0", (func_t)gsl_sf_bessel_y0, 1, nullptr, FunctionGroups::BesselFunctions},
	{i18n("Irregular spherical Bessel function of first order"), "y1", (func_t)gsl_sf_bessel_y1, 1, nullptr, FunctionGroups::BesselFunctions},
	{i18n("Irregular spherical Bessel function of second order"), "y2", (func_t)gsl_sf_bessel_y2, 1, nullptr, FunctionGroups::BesselFunctions},
	{i18n("Irregular spherical Bessel function of order l"), "yl", (func_t)nsl_sf_bessel_yl, 2, nullptr, FunctionGroups::BesselFunctions},
	{i18n("Scaled regular modified spherical Bessel function of zeroth order, exp(-|x|) i0(x)"),
	 "i0s",
	 (func_t)gsl_sf_bessel_i0_scaled,
	 1,
	 nullptr,
	 FunctionGroups::BesselFunctions},
	{i18n("Scaled regular modified spherical Bessel function of first order, exp(-|x|) i1(x)"),
	 "i1s",
	 (func_t)gsl_sf_bessel_i1_scaled,
	 1,
	 nullptr,
	 FunctionGroups::BesselFunctions},
	{i18n("Scaled regular modified spherical Bessel function of second order, exp(-|x|) i2(x)"),
	 "i2s",
	 (func_t)gsl_sf_bessel_i2_scaled,
	 1,
	 nullptr,
	 FunctionGroups::BesselFunctions},
	{i18n("Scaled regular modified spherical Bessel function of order l, exp(-|x|) il(x)"),
	 "ils",
	 (func_t)nsl_sf_bessel_ils,
	 2,
	 nullptr,
	 FunctionGroups::BesselFunctions},

	{i18n("Scaled irregular modified spherical Bessel function of zeroth order, exp(x) k0(x)"),
	 "k0s",
	 (func_t)gsl_sf_bessel_k0_scaled,
	 1,
	 nullptr,
	 FunctionGroups::BesselFunctions},
	{i18n("Scaled irregular modified spherical Bessel function of first order, exp(-|x|) k1(x)"),
	 "k1s",
	 (func_t)gsl_sf_bessel_k1_scaled,
	 1,
	 nullptr,
	 FunctionGroups::BesselFunctions},
	{i18n("Scaled irregular modified spherical Bessel function of second order, exp(-|x|) k2(x)"),
	 "k2s",
	 (func_t)gsl_sf_bessel_k2_scaled,
	 1,
	 nullptr,
	 FunctionGroups::BesselFunctions},
	{i18n("Scaled irregular modified spherical Bessel function of order l, exp(-|x|) kl(x)"),
	 "kls",
	 (func_t)nsl_sf_bessel_kls,
	 2,
	 nullptr,
	 FunctionGroups::BesselFunctions},
	{i18n("Regular cylindrical Bessel function of fractional order"), "Jnu", (func_t)gsl_sf_bessel_Jnu, 2, nullptr, FunctionGroups::BesselFunctions},
	{i18n("Irregular cylindrical Bessel function of fractional order"), "Ynu", (func_t)gsl_sf_bessel_Ynu, 2, nullptr, FunctionGroups::BesselFunctions},
	{i18n("Regular modified Bessel function of fractional order"), "Inu", (func_t)gsl_sf_bessel_Inu, 2, nullptr, FunctionGroups::BesselFunctions},
	{i18n("Scaled regular modified Bessel function of fractional order"),
	 "Inus",
	 (func_t)gsl_sf_bessel_Inu_scaled,
	 2,
	 nullptr,
	 FunctionGroups::BesselFunctions},
	{i18n("Irregular modified Bessel function of fractional order"), "Knu", (func_t)gsl_sf_bessel_Knu, 2, nullptr, FunctionGroups::BesselFunctions},
	{i18n("Logarithm of irregular modified Bessel function of fractional order"),
	 "lnKnu",
	 (func_t)gsl_sf_bessel_lnKnu,
	 2,
	 nullptr,
	 FunctionGroups::BesselFunctions},

	{i18n("Scaled irregular modified Bessel function of fractional order"),
	 "Knus",
	 (func_t)gsl_sf_bessel_Knu_scaled,
	 2,
	 nullptr,
	 FunctionGroups::BesselFunctions},
	{i18n("n-th positive zero of the Bessel function J0"), "J0_0", (func_t)nsl_sf_bessel_0_J0, 1, nullptr, FunctionGroups::BesselFunctions},
	{i18n("n-th positive zero of the Bessel function J1"), "J1_0", (func_t)nsl_sf_bessel_0_J1, 1, nullptr, FunctionGroups::BesselFunctions},
	{i18n("n-th positive zero of the Bessel function Jnu"), "Jnu_0", (func_t)nsl_sf_bessel_0_Jnu, 2, nullptr, FunctionGroups::BesselFunctions},

	// Clausen Functions
	{i18n("Clausen function"), "clausen", (func_t)gsl_sf_clausen, 1, nullptr, FunctionGroups::ClausenFunctions},

	// Coulomb Functions
	{i18n("Lowest-order normalized hydrogenic bound state radial wavefunction"),
	 "hydrogenicR_1",
	 (func_t)gsl_sf_hydrogenicR_1,
	 2,
	 nullptr,
	 FunctionGroups::CoulombFunctions},
	{i18n("n-th normalized hydrogenic bound state radial wavefunction"),
	 "hydrogenicR",
	 (func_t)nsl_sf_hydrogenicR,
	 4,
	 nullptr,
	 FunctionGroups::CoulombFunctions},

	// Dawson Function
	{i18n("Dawson's integral D(z) = sqrt(pi)/2 * exp(-z^2) * erfi(z)"), "dawson", (func_t)nsl_sf_dawson, 1, nullptr, FunctionGroups::DawsonFunction},

	// Debye Functions
	{i18n("First-order Debye function"), "D1", (func_t)gsl_sf_debye_1, 1, nullptr, FunctionGroups::DebyeFunctions},
	{i18n("Second-order Debye function"), "D2", (func_t)gsl_sf_debye_2, 1, nullptr, FunctionGroups::DebyeFunctions},
	{i18n("Third-order Debye function"), "D3", (func_t)gsl_sf_debye_3, 1, nullptr, FunctionGroups::DebyeFunctions},
	{i18n("Fourth-order Debye function"), "D4", (func_t)gsl_sf_debye_4, 1, nullptr, FunctionGroups::DebyeFunctions},
	{i18n("Fifth-order Debye function"), "D5", (func_t)gsl_sf_debye_5, 1, nullptr, FunctionGroups::DebyeFunctions},
	{i18n("Sixth-order Debye function"), "D6", (func_t)gsl_sf_debye_6, 1, nullptr, FunctionGroups::DebyeFunctions},

	// Dilogarithm
	{i18n("Dilogarithm for a real argument"), "Li2", (func_t)gsl_sf_dilog, 1, nullptr, FunctionGroups::Dilogarithm},

	// Elliptic Integrals
	{i18n("Legendre form of complete elliptic integral K"), "Kc", (func_t)nsl_sf_ellint_Kc, 1, nullptr, FunctionGroups::EllipticIntegrals},
	{i18n("Legendre form of complete elliptic integral E"), "Ec", (func_t)nsl_sf_ellint_Ec, 1, nullptr, FunctionGroups::EllipticIntegrals},
	{i18n("Legendre form of complete elliptic integral Pi"), "Pc", (func_t)nsl_sf_ellint_Pc, 2, nullptr, FunctionGroups::EllipticIntegrals},
	{i18n("Legendre form of incomplete elliptic integral F"), "F", (func_t)nsl_sf_ellint_F, 2, nullptr, FunctionGroups::EllipticIntegrals},
	{i18n("Legendre form of incomplete elliptic integral E"), "E", (func_t)nsl_sf_ellint_E, 2, nullptr, FunctionGroups::EllipticIntegrals},
	{i18n("Legendre form of incomplete elliptic integral P"), "P", (func_t)nsl_sf_ellint_P, 3, nullptr, FunctionGroups::EllipticIntegrals},
	{i18n("Legendre form of incomplete elliptic integral D"), "D", (func_t)nsl_sf_ellint_D, 2, nullptr, FunctionGroups::EllipticIntegrals},
	{i18n("Carlson form of incomplete elliptic integral RC"), "RC", (func_t)nsl_sf_ellint_RC, 2, nullptr, FunctionGroups::EllipticIntegrals},
	{i18n("Carlson form of incomplete elliptic integral RD"), "RD", (func_t)nsl_sf_ellint_RD, 3, nullptr, FunctionGroups::EllipticIntegrals},
	{i18n("Carlson form of incomplete elliptic integral RF"), "RF", (func_t)nsl_sf_ellint_RF, 3, nullptr, FunctionGroups::EllipticIntegrals},
	{i18n("Carlson form of incomplete elliptic integral RJ"), "RJ", (func_t)nsl_sf_ellint_RJ, 4, nullptr, FunctionGroups::EllipticIntegrals},

	// Error Functions
	{i18n("Error function"), "erf", (func_t)gsl_sf_erf, 1, nullptr, FunctionGroups::ErrorFunctions},
	{i18n("Complementary error function"), "erfc", (func_t)gsl_sf_erfc, 1, nullptr, FunctionGroups::ErrorFunctions},
	{i18n("Logarithm of complementary error function"), "log_erfc", (func_t)gsl_sf_log_erfc, 1, nullptr, FunctionGroups::ErrorFunctions},
	{i18n("Gaussian probability density function Z"), "erf_Z", (func_t)gsl_sf_erf_Z, 1, nullptr, FunctionGroups::ErrorFunctions},
	{i18n("Upper tail of the Gaussian probability function Q"), "erf_Q", (func_t)gsl_sf_erf_Q, 1, nullptr, FunctionGroups::ErrorFunctions},
	{i18n("Hazard function for the normal distribution Z/Q"), "hazard", (func_t)gsl_sf_hazard, 1, nullptr, FunctionGroups::ErrorFunctions},
#ifndef _MSC_VER
	{i18n("Underflow-compensating function exp(x^2) erfc(x) for real x"), "erfcx", (func_t)nsl_sf_erfcx, 1, nullptr, FunctionGroups::ErrorFunctions},
	{i18n("Imaginary error function erfi(x) = -i erf(ix) for real x"), "erfi", (func_t)nsl_sf_erfi, 1, nullptr, FunctionGroups::ErrorFunctions},
	{i18n("Imaginary part of Faddeeva's scaled complex error function w(x) = exp(-x^2) erfc(-ix) for real x"),
	 "im_w_of_x",
	 (func_t)nsl_sf_im_w_of_x,
	 1,
	 nullptr,
	 FunctionGroups::ErrorFunctions},
	{i18n("Dawson's integral D(z) = sqrt(pi)/2 * exp(-z^2) * erfi(z)"), "dawson", (func_t)nsl_sf_dawson, 1, nullptr, FunctionGroups::ErrorFunctions},
	{i18n("Voigt profile"), "voigt", (func_t)nsl_sf_voigt, 3, nullptr, FunctionGroups::ErrorFunctions},
#endif
	{i18n("Pseudo-Voigt profile (same width)"), "pseudovoigt1", (func_t)nsl_sf_pseudovoigt1, 3, nullptr, FunctionGroups::ErrorFunctions},

	// Exponential Functions
	{i18n("Exponential function"), "exp", (func_t)gsl_sf_exp, 1, nullptr, FunctionGroups::ExponentialFunctions},
	{i18n("exponentiate x and multiply by y"), "exp_mult", (func_t)gsl_sf_exp_mult, 2, nullptr, FunctionGroups::ExponentialFunctions},
	{QStringLiteral("exp(x) - 1"), "expm1", (func_t)gsl_expm1, 1, nullptr, FunctionGroups::ExponentialFunctions},
	{QStringLiteral("(exp(x)-1)/x"), "exprel", (func_t)gsl_sf_exprel, 1, nullptr, FunctionGroups::ExponentialFunctions},
	{QStringLiteral("2(exp(x)-1-x)/x^2"), "exprel2", (func_t)gsl_sf_exprel_2, 1, nullptr, FunctionGroups::ExponentialFunctions},
	{i18n("n-relative exponential"), "expreln", (func_t)nsl_sf_exprel_n, 2, nullptr, FunctionGroups::ExponentialFunctions},

	// Exponential Integrals
	{i18n("Exponential integral"), "E1", (func_t)gsl_sf_expint_E1, 1, nullptr, FunctionGroups::ExponentialIntegrals},
	{i18n("Second order exponential integral"), "E2", (func_t)gsl_sf_expint_E2, 1, nullptr, FunctionGroups::ExponentialIntegrals},
	{i18n("Exponential integral of order n"), "En", (func_t)gsl_sf_expint_En, 2, nullptr, FunctionGroups::ExponentialIntegrals},
	{i18n("Exponential integral Ei"), "Ei", (func_t)gsl_sf_expint_Ei, 1, nullptr, FunctionGroups::ExponentialIntegrals},
	{i18n("Hyperbolic integral Shi"), "Shi", (func_t)gsl_sf_Shi, 1, nullptr, FunctionGroups::ExponentialIntegrals},
	{i18n("Hyperbolic integral Chi"), "Chi", (func_t)gsl_sf_Chi, 1, nullptr, FunctionGroups::ExponentialIntegrals},
	{i18n("Third-order exponential integral"), "Ei3", (func_t)gsl_sf_expint_3, 1, nullptr, FunctionGroups::ExponentialIntegrals},
	{i18n("Sine integral"), "Si", (func_t)gsl_sf_Si, 1, nullptr, FunctionGroups::ExponentialIntegrals},
	{i18n("Cosine integral"), "Ci", (func_t)gsl_sf_Ci, 1, nullptr, FunctionGroups::ExponentialIntegrals},
	{i18n("Arctangent integral"), "Atanint", (func_t)gsl_sf_atanint, 1, nullptr, FunctionGroups::ExponentialIntegrals},

	// Fermi-Dirac Function
	{i18n("Complete Fermi-Dirac integral with index -1"), "Fm1", (func_t)gsl_sf_fermi_dirac_m1, 1, nullptr, FunctionGroups::FermiDiracFunction},
	{i18n("Complete Fermi-Dirac integral with index 0"), "F0", (func_t)gsl_sf_fermi_dirac_0, 1, nullptr, FunctionGroups::FermiDiracFunction},
	{i18n("Complete Fermi-Dirac integral with index 1"), "F1", (func_t)gsl_sf_fermi_dirac_1, 1, nullptr, FunctionGroups::FermiDiracFunction},
	{i18n("Complete Fermi-Dirac integral with index 2"), "F2", (func_t)gsl_sf_fermi_dirac_2, 1, nullptr, FunctionGroups::FermiDiracFunction},
	{i18n("Complete Fermi-Dirac integral with integer index j"), "Fj", (func_t)nsl_sf_fermi_dirac_int, 2, nullptr, FunctionGroups::FermiDiracFunction},
	{i18n("Complete Fermi-Dirac integral with index -1/2"), "Fmhalf", (func_t)gsl_sf_fermi_dirac_mhalf, 1, nullptr, FunctionGroups::FermiDiracFunction},
	{i18n("Complete Fermi-Dirac integral with index 1/2"), "Fhalf", (func_t)gsl_sf_fermi_dirac_half, 1, nullptr, FunctionGroups::FermiDiracFunction},
	{i18n("Complete Fermi-Dirac integral with index 3/2"), "F3half", (func_t)gsl_sf_fermi_dirac_3half, 1, nullptr, FunctionGroups::FermiDiracFunction},
	{i18n("Incomplete Fermi-Dirac integral with index zero"), "Finc0", (func_t)gsl_sf_fermi_dirac_inc_0, 2, nullptr, FunctionGroups::FermiDiracFunction},

	// Gamma and Beta Functions
	{i18n("Gamma function"), "gamma", (func_t)gsl_sf_gamma, 1, nullptr, FunctionGroups::GammaAndBetaFunctions},
	{i18n("Gamma function"), "tgamma", (func_t)gsl_sf_gamma, 1, nullptr, FunctionGroups::GammaAndBetaFunctions},
	{i18n("Logarithm of the gamma function"), "lgamma", (func_t)gsl_sf_lngamma, 1, nullptr, FunctionGroups::GammaAndBetaFunctions},
	{i18n("Logarithm naturalis of the gamma function"), "lngamma", (func_t)gsl_sf_lngamma, 1, nullptr, FunctionGroups::GammaAndBetaFunctions},
	{i18n("Regulated gamma function"), "gammastar", (func_t)gsl_sf_gammastar, 1, nullptr, FunctionGroups::GammaAndBetaFunctions},
	{i18n("Reciprocal of the gamma function"), "gammainv", (func_t)gsl_sf_gammainv, 1, nullptr, FunctionGroups::GammaAndBetaFunctions},
	{i18n("Factorial n!"), "fact", (func_t)nsl_sf_fact, 1, nullptr, FunctionGroups::GammaAndBetaFunctions},
	{i18n("Double factorial n!!"), "doublefact", (func_t)nsl_sf_doublefact, 1, nullptr, FunctionGroups::GammaAndBetaFunctions},
	{i18n("Logarithm of the factorial"), "lnfact", (func_t)nsl_sf_lnfact, 1, nullptr, FunctionGroups::GammaAndBetaFunctions},
	{i18n("Logarithm of the double factorial"), "lndoublefact", (func_t)nsl_sf_lndoublefact, 1, nullptr, FunctionGroups::GammaAndBetaFunctions},

	{i18n("Combinatorial factor"), "choose", (func_t)nsl_sf_choose, 2, nullptr, FunctionGroups::GammaAndBetaFunctions},
	{i18n("Logarithm of the combinatorial factor"), "lnchoose", (func_t)nsl_sf_lnchoose, 2, nullptr, FunctionGroups::GammaAndBetaFunctions},
	{i18n("Taylor coefficient"), "taylor", (func_t)nsl_sf_taylorcoeff, 2, nullptr, FunctionGroups::GammaAndBetaFunctions},
	{i18n("Pochhammer symbol"), "poch", (func_t)gsl_sf_poch, 2, nullptr, FunctionGroups::GammaAndBetaFunctions},
	{i18n("Logarithm of the Pochhammer symbol"), "lnpoch", (func_t)gsl_sf_lnpoch, 2, nullptr, FunctionGroups::GammaAndBetaFunctions},
	{i18n("Relative Pochhammer symbol"), "pochrel", (func_t)gsl_sf_pochrel, 2, nullptr, FunctionGroups::GammaAndBetaFunctions},
	{i18n("Unnormalized incomplete gamma function"), "gammainc", (func_t)gsl_sf_gamma_inc, 2, nullptr, FunctionGroups::GammaAndBetaFunctions},
	{i18n("Normalized incomplete gamma function"), "gammaincQ", (func_t)gsl_sf_gamma_inc_Q, 2, nullptr, FunctionGroups::GammaAndBetaFunctions},
	{i18n("Complementary normalized incomplete gamma function"), "gammaincP", (func_t)gsl_sf_gamma_inc_P, 2, nullptr, FunctionGroups::GammaAndBetaFunctions},
	{i18n("Beta function"), "beta", (func_t)gsl_sf_beta, 2, nullptr, FunctionGroups::GammaAndBetaFunctions},

	{i18n("Logarithm of the beta function"), "lnbeta", (func_t)gsl_sf_lnbeta, 2, nullptr, FunctionGroups::GammaAndBetaFunctions},
	{i18n("Normalized incomplete beta function"), "betainc", (func_t)gsl_sf_beta_inc, 3, nullptr, FunctionGroups::GammaAndBetaFunctions},

	// Gegenbauer Functions
	{i18n("Gegenbauer polynomial C_1"), "C1", (func_t)gsl_sf_gegenpoly_1, 2, nullptr, FunctionGroups::GegenbauerFunctions},
	{i18n("Gegenbauer polynomial C_2"), "C2", (func_t)gsl_sf_gegenpoly_2, 2, nullptr, FunctionGroups::GegenbauerFunctions},
	{i18n("Gegenbauer polynomial C_3"), "C3", (func_t)gsl_sf_gegenpoly_3, 2, nullptr, FunctionGroups::GegenbauerFunctions},
	{i18n("Gegenbauer polynomial C_n"), "Cn", (func_t)nsl_sf_gegenpoly_n, 3, nullptr, FunctionGroups::GegenbauerFunctions},

#if (GSL_MAJOR_VERSION > 2) || (GSL_MAJOR_VERSION == 2) && (GSL_MINOR_VERSION >= 4)
	// Hermite Polynomials and Functions
	{i18n("Hermite polynomials physicists version"), "Hn", (func_t)nsl_sf_hermite, 2, nullptr, FunctionGroups::HermitePolynomialsAndFunctions},
	{i18n("Hermite polynomials probabilists version"), "Hen", (func_t)nsl_sf_hermite_prob, 2, nullptr, FunctionGroups::HermitePolynomialsAndFunctions},
	{i18n("Hermite functions"), "Hfn", (func_t)nsl_sf_hermite_func, 2, nullptr, FunctionGroups::HermitePolynomialsAndFunctions},
#if (GSL_MAJOR_VERSION > 2) || (GSL_MAJOR_VERSION == 2) && (GSL_MINOR_VERSION >= 6)
	{i18n("Hermite functions (fast version)"), "Hfnf", (func_t)nsl_sf_hermite_func_fast, 2, nullptr, FunctionGroups::HermitePolynomialsAndFunctions},
#endif
	{i18n("Derivatives of Hermite polynomials physicists version"),
	 "Hnd",
	 (func_t)nsl_sf_hermite_deriv,
	 3,
	 nullptr,
	 FunctionGroups::HermitePolynomialsAndFunctions},
	{i18n("Derivatives of Hermite polynomials probabilists version"),
	 "Hend",
	 (func_t)nsl_sf_hermite_prob_deriv,
	 3,
	 nullptr,
	 FunctionGroups::HermitePolynomialsAndFunctions},
	{i18n("Derivatives of Hermite functions"), "Hfnd", (func_t)nsl_sf_hermite_func_der, 3, nullptr, FunctionGroups::HermitePolynomialsAndFunctions},
#endif

	// Hypergeometric Functions
	{i18n("Hypergeometric function 0F1"), "hyperg_0F1", (func_t)gsl_sf_hyperg_0F1, 2, nullptr, FunctionGroups::HypergeometricFunctions},
	{i18n("Confluent hypergeometric function 1F1 for integer parameters"),
	 "hyperg_1F1i",
	 (func_t)nsl_sf_hyperg_1F1i,
	 3,
	 nullptr,
	 FunctionGroups::HypergeometricFunctions},
	{i18n("Confluent hypergeometric function 1F1 for general parameters"),
	 "hyperg_1F1",
	 (func_t)gsl_sf_hyperg_1F1,
	 3,
	 nullptr,
	 FunctionGroups::HypergeometricFunctions},
	{i18n("Confluent hypergeometric function U for integer parameters"),
	 "hyperg_Ui",
	 (func_t)nsl_sf_hyperg_Ui,
	 3,
	 nullptr,
	 FunctionGroups::HypergeometricFunctions},
	{i18n("Confluent hypergeometric function U"), "hyperg_U", (func_t)gsl_sf_hyperg_U, 3, nullptr, FunctionGroups::HypergeometricFunctions},
	{i18n("Gauss hypergeometric function 2F1"), "hyperg_2F1", (func_t)gsl_sf_hyperg_2F1, 4, nullptr, FunctionGroups::HypergeometricFunctions},
	{i18n("Gauss hypergeometric function 2F1 with complex parameters"),
	 "hyperg_2F1c",
	 (func_t)gsl_sf_hyperg_2F1_conj,
	 4,
	 nullptr,
	 FunctionGroups::HypergeometricFunctions},
	{i18n("Renormalized Gauss hypergeometric function 2F1"),
	 "hyperg_2F1r",
	 (func_t)gsl_sf_hyperg_2F1_renorm,
	 4,
	 nullptr,
	 FunctionGroups::HypergeometricFunctions},
	{i18n("Renormalized Gauss hypergeometric function 2F1 with complex parameters"),
	 "hyperg_2F1cr",
	 (func_t)gsl_sf_hyperg_2F1_conj_renorm,
	 4,
	 nullptr,
	 FunctionGroups::HypergeometricFunctions},
	{i18n("Hypergeometric function 2F0"), "hyperg_2F0", (func_t)gsl_sf_hyperg_2F0, 3, nullptr, FunctionGroups::HypergeometricFunctions},

	// Laguerre Functions
	{i18n("generalized Laguerre polynomials L_1"), "L1", (func_t)gsl_sf_laguerre_1, 2, nullptr, FunctionGroups::LaguerreFunctions},
	{i18n("generalized Laguerre polynomials L_2"), "L2", (func_t)gsl_sf_laguerre_2, 2, nullptr, FunctionGroups::LaguerreFunctions},
	{i18n("generalized Laguerre polynomials L_3"), "L3", (func_t)gsl_sf_laguerre_3, 2, nullptr, FunctionGroups::LaguerreFunctions},
	{i18n("generalized Laguerre polynomials L_n"), "Ln", (func_t)nsl_sf_laguerre_n, 3, nullptr, FunctionGroups::LaguerreFunctions},

	// Lambert W Functions
	{i18n("Principal branch of the Lambert W function"), "W0", (func_t)gsl_sf_lambert_W0, 1, nullptr, FunctionGroups::LambertWFunctions},
	{i18n("Secondary real-valued branch of the Lambert W function"), "Wm1", (func_t)gsl_sf_lambert_Wm1, 1, nullptr, FunctionGroups::LambertWFunctions},

	// Legendre Functions and Spherical Harmonics
	{i18n("Legendre polynomial P_1"), "P1", (func_t)gsl_sf_legendre_P1, 1, nullptr, FunctionGroups::LegendreFunctionsAndSphericalHarmonics},
	{i18n("Legendre polynomial P_2"), "P2", (func_t)gsl_sf_legendre_P2, 1, nullptr, FunctionGroups::LegendreFunctionsAndSphericalHarmonics},
	{i18n("Legendre polynomial P_3"), "P3", (func_t)gsl_sf_legendre_P3, 1, nullptr, FunctionGroups::LegendreFunctionsAndSphericalHarmonics},
	{i18n("Legendre polynomial P_l"), "Pl", (func_t)nsl_sf_legendre_Pl, 2, nullptr, FunctionGroups::LegendreFunctionsAndSphericalHarmonics},
	{i18n("Legendre function Q_0"), "Q0", (func_t)gsl_sf_legendre_Q0, 1, nullptr, FunctionGroups::LegendreFunctionsAndSphericalHarmonics},
	{i18n("Legendre function Q_1"), "Q1", (func_t)gsl_sf_legendre_Q1, 1, nullptr, FunctionGroups::LegendreFunctionsAndSphericalHarmonics},
	{i18n("Legendre function Q_l"), "Ql", (func_t)nsl_sf_legendre_Ql, 2, nullptr, FunctionGroups::LegendreFunctionsAndSphericalHarmonics},
	{i18n("Associated Legendre polynomial"), "Plm", (func_t)nsl_sf_legendre_Plm, 3, nullptr, FunctionGroups::LegendreFunctionsAndSphericalHarmonics},
	{i18n("Normalized associated Legendre polynomial"),
	 "Pslm",
	 (func_t)nsl_sf_legendre_sphPlm,
	 3,
	 nullptr,
	 FunctionGroups::LegendreFunctionsAndSphericalHarmonics},
	{i18n("Irregular spherical conical function P^1/2"),
	 "Phalf",
	 (func_t)gsl_sf_conicalP_half,
	 2,
	 nullptr,
	 FunctionGroups::LegendreFunctionsAndSphericalHarmonics},

	{i18n("Regular spherical conical function P^(-1/2)"),
	 "Pmhalf",
	 (func_t)gsl_sf_conicalP_mhalf,
	 2,
	 nullptr,
	 FunctionGroups::LegendreFunctionsAndSphericalHarmonics},
	{i18n("Conical function P^0"), "Pc0", (func_t)gsl_sf_conicalP_0, 2, nullptr, FunctionGroups::LegendreFunctionsAndSphericalHarmonics},
	{i18n("Conical function P^1"), "Pc1", (func_t)gsl_sf_conicalP_1, 2, nullptr, FunctionGroups::LegendreFunctionsAndSphericalHarmonics},
	{i18n("Regular spherical conical function P^(-1/2-l)"),
	 "Psr",
	 (func_t)nsl_sf_conicalP_sphreg,
	 3,
	 nullptr,
	 FunctionGroups::LegendreFunctionsAndSphericalHarmonics},
	{i18n("Regular cylindrical conical function P^(-m)"),
	 "Pcr",
	 (func_t)nsl_sf_conicalP_cylreg,
	 3,
	 nullptr,
	 FunctionGroups::LegendreFunctionsAndSphericalHarmonics},
	{i18n("Zeroth radial eigenfunction of the Laplacian on the 3-dimensional hyperbolic space"),
	 "H3d0",
	 (func_t)gsl_sf_legendre_H3d_0,
	 2,
	 nullptr,
	 FunctionGroups::LegendreFunctionsAndSphericalHarmonics},
	{i18n("First radial eigenfunction of the Laplacian on the 3-dimensional hyperbolic space"),
	 "H3d1",
	 (func_t)gsl_sf_legendre_H3d_1,
	 2,
	 nullptr,
	 FunctionGroups::LegendreFunctionsAndSphericalHarmonics},
	{i18n("l-th radial eigenfunction of the Laplacian on the 3-dimensional hyperbolic space"),
	 "H3d",
	 (func_t)nsl_sf_legendre_H3d,
	 3,
	 nullptr,
	 FunctionGroups::LegendreFunctionsAndSphericalHarmonics},

	// Logarithm and Related Functions
	{i18n("Logarithm"), "log", (func_t)gsl_sf_log, 1, nullptr, FunctionGroups::LogarithmAndRelatedFunctions},
	{i18n("Logarithm of the magnitude"), "logabs", (func_t)gsl_sf_log_abs, 1, nullptr, FunctionGroups::LogarithmAndRelatedFunctions},
	{QStringLiteral("log(1+x)"), "logp", (func_t)gsl_sf_log_1plusx, 1, nullptr, FunctionGroups::LogarithmAndRelatedFunctions},
	{QStringLiteral("log(1+x) - x"), "logpm", (func_t)gsl_sf_log_1plusx_mx, 1, nullptr, FunctionGroups::LogarithmAndRelatedFunctions},

#if (GSL_MAJOR_VERSION >= 2)
	// Mathieu Functions
	{i18n("Characteristic values a_n(q) of the Mathieu functions ce_n(q,x)"), "an", (func_t)nsl_sf_mathieu_a, 2, nullptr, FunctionGroups::MathieuFunctions},
	{i18n("Characteristic values b_n(q) of the Mathieu functions se_n(q,x)"), "bn", (func_t)nsl_sf_mathieu_b, 2, nullptr, FunctionGroups::MathieuFunctions},
	{i18n("Angular Mathieu functions ce_n(q,x)"), "ce", (func_t)nsl_sf_mathieu_ce, 3, nullptr, FunctionGroups::MathieuFunctions},
	{i18n("Angular Mathieu functions se_n(q,x)"), "se", (func_t)nsl_sf_mathieu_se, 3, nullptr, FunctionGroups::MathieuFunctions},
	{i18n("Radial j-th kind Mathieu functions Mc_n^{(j)}(q,x)"),
	 "Mc",
	 (func_t)nsl_sf_mathieu_Mc,
	 4,
	 nsl_sf_mathieuParameterNames,
	 FunctionGroups::MathieuFunctions},
	{i18n("Radial j-th kind Mathieu functions Ms_n^{(j)}(q,x)"),
	 "Ms",
	 (func_t)nsl_sf_mathieu_Ms,
	 4,
	 nsl_sf_mathieuParameterNames,
	 FunctionGroups::MathieuFunctions},
#endif

	// Power Function
	{i18n("x^n for integer n with an error estimate"), "gsl_powint", (func_t)nsl_sf_powint, 2, nullptr, FunctionGroups::PowerFunction},

	// Psi (Digamma) Function
	{i18n("Digamma function for positive integer n"), "psiint", (func_t)nsl_sf_psiint, 1, nullptr, FunctionGroups::PsiDigammaFunction},
	{i18n("Digamma function"), "psi", (func_t)gsl_sf_psi, 1, nullptr, FunctionGroups::PsiDigammaFunction},
	{i18n("Real part of the digamma function on the line 1+i y"), "psi1piy", (func_t)gsl_sf_psi_1piy, 1, nullptr, FunctionGroups::PsiDigammaFunction},
	{i18n("Trigamma function psi' for positive integer n"), "psi1int", (func_t)nsl_sf_psi1int, 1, nullptr, FunctionGroups::PsiDigammaFunction},
	{i18n("Trigamma function psi'"), "psi1", (func_t)gsl_sf_psi_1, 1, nullptr, FunctionGroups::PsiDigammaFunction},
	{i18n("Polygamma function psi^(n)"), "psin", (func_t)nsl_sf_psin, 2, nullptr, FunctionGroups::PsiDigammaFunction},

	// Synchrotron Functions
	{i18n("First synchrotron function"), "synchrotron1", (func_t)gsl_sf_synchrotron_1, 1, nullptr, FunctionGroups::SynchrotronFunctions},
	{i18n("Second synchrotron function"), "synchrotron2", (func_t)gsl_sf_synchrotron_2, 1, nullptr, FunctionGroups::SynchrotronFunctions},

	// Transport Functions
	{i18n("Transport function"), "J2", (func_t)gsl_sf_transport_2, 1, nullptr, FunctionGroups::TransportFunctions},
	{i18n("Transport function"), "J3", (func_t)gsl_sf_transport_3, 1, nullptr, FunctionGroups::TransportFunctions},
	{i18n("Transport function"), "J4", (func_t)gsl_sf_transport_4, 1, nullptr, FunctionGroups::TransportFunctions},
	{i18n("Transport function"), "J5", (func_t)gsl_sf_transport_5, 1, nullptr, FunctionGroups::TransportFunctions},

	// Trigonometric Functions
	{i18n("Sine"), "sin", (func_t)gsl_sf_sin, 1, nullptr, FunctionGroups::TrigonometricFunctions},
	{i18n("Cosine"), "cos", (func_t)gsl_sf_cos, 1, nullptr, FunctionGroups::TrigonometricFunctions},
	{i18n("Tangent"), "tan", (func_t) static_cast<double (*)(double)>(&tan), 1, nullptr, FunctionGroups::TrigonometricFunctions},
	{i18n("Inverse sine"), "asin", (func_t) static_cast<double (*)(double)>(&asin), 1, nullptr, FunctionGroups::TrigonometricFunctions},
	{i18n("Inverse cosine"), "acos", (func_t) static_cast<double (*)(double)>(&acos), 1, nullptr, FunctionGroups::TrigonometricFunctions},
	{i18n("Inverse tangent"), "atan", (func_t) static_cast<double (*)(double)>(&atan), 1, nullptr, FunctionGroups::TrigonometricFunctions},
	{i18n("Inverse tangent using sign"), "atan2", (func_t) static_cast<double (*)(double, double)>(&atan2), 2, nullptr, FunctionGroups::TrigonometricFunctions},
	{i18n("Hyperbolic sine"), "sinh", (func_t) static_cast<double (*)(double)>(&sinh), 1, nullptr, FunctionGroups::TrigonometricFunctions},
	{i18n("Hyperbolic cosine"), "cosh", (func_t) static_cast<double (*)(double)>(&cosh), 1, nullptr, FunctionGroups::TrigonometricFunctions},
	{i18n("Hyperbolic tangent"), "tanh", (func_t) static_cast<double (*)(double)>(&tanh), 1, nullptr, FunctionGroups::TrigonometricFunctions},
	{i18n("Inverse hyperbolic cosine"), "acosh", (func_t)gsl_acosh, 1, nullptr, FunctionGroups::TrigonometricFunctions},
	{i18n("Inverse hyperbolic sine"), "asinh", (func_t)gsl_asinh, 1, nullptr, FunctionGroups::TrigonometricFunctions},
	{i18n("Inverse hyperbolic tangent"), "atanh", (func_t)gsl_atanh, 1, nullptr, FunctionGroups::TrigonometricFunctions},
	{i18n("Secant"), "sec", (func_t)nsl_sf_sec, 1, nullptr, FunctionGroups::TrigonometricFunctions},
	{i18n("Cosecant"), "csc", (func_t)nsl_sf_csc, 1, nullptr, FunctionGroups::TrigonometricFunctions},
	{i18n("Cotangent"), "cot", (func_t)nsl_sf_cot, 1, nullptr, FunctionGroups::TrigonometricFunctions},
	{i18n("Inverse secant"), "asec", (func_t)nsl_sf_asec, 1, nullptr, FunctionGroups::TrigonometricFunctions},
	{i18n("Inverse cosecant"), "acsc", (func_t)nsl_sf_acsc, 1, nullptr, FunctionGroups::TrigonometricFunctions},
	{i18n("Inverse cotangent"), "acot", (func_t)nsl_sf_acot, 1, nullptr, FunctionGroups::TrigonometricFunctions},
	{i18n("Hyperbolic secant"), "sech", (func_t)nsl_sf_sech, 1, nullptr, FunctionGroups::TrigonometricFunctions},
	{i18n("Hyperbolic cosecant"), "csch", (func_t)nsl_sf_csch, 1, nullptr, FunctionGroups::TrigonometricFunctions},
	{i18n("Hyperbolic cotangent"), "coth", (func_t)nsl_sf_coth, 1, nullptr, FunctionGroups::TrigonometricFunctions},
	{i18n("Inverse hyperbolic secant"), "asech", (func_t)nsl_sf_asech, 1, nullptr, FunctionGroups::TrigonometricFunctions},
	{i18n("Inverse hyperbolic cosecant"), "acsch", (func_t)nsl_sf_acsch, 1, nullptr, FunctionGroups::TrigonometricFunctions},
	{i18n("Inverse hyperbolic cotangent"), "acoth", (func_t)nsl_sf_acoth, 1, nullptr, FunctionGroups::TrigonometricFunctions},
	{i18n("Sinc function sin(x)/x"), "sinc", (func_t)gsl_sf_sinc, 1, nullptr, FunctionGroups::TrigonometricFunctions},
	{QStringLiteral("log(sinh(x))"), "logsinh", (func_t)gsl_sf_lnsinh, 1, nullptr, FunctionGroups::TrigonometricFunctions},
	{QStringLiteral("log(cosh(x))"), "logcosh", (func_t)gsl_sf_lncosh, 1, nullptr, FunctionGroups::TrigonometricFunctions},
	{i18n("Hypotenuse function"), "hypot", (func_t)gsl_sf_hypot, 2, nullptr, FunctionGroups::TrigonometricFunctions},
	{i18n("Three component hypotenuse function"), "hypot3", (func_t)gsl_hypot3, 3, nullptr, FunctionGroups::TrigonometricFunctions},
	{i18n("restrict to [-pi,pi]"), "anglesymm", (func_t)gsl_sf_angle_restrict_symm, 1, nullptr, FunctionGroups::TrigonometricFunctions},
	{i18n("restrict to [0,2 pi]"), "anglepos", (func_t)gsl_sf_angle_restrict_pos, 1, nullptr, FunctionGroups::TrigonometricFunctions},

	// Zeta Functions
	{i18n("Riemann zeta function for integer n"), "zetaint", (func_t)nsl_sf_zetaint, 1, nullptr, FunctionGroups::ZetaFunctions},
	{i18n("Riemann zeta function"), "zeta", (func_t)gsl_sf_zeta, 1, nullptr, FunctionGroups::ZetaFunctions},
	{i18n("zeta(n)-1 for integer n"), "zetam1int", (func_t)nsl_sf_zetam1int, 1, nullptr, FunctionGroups::ZetaFunctions},
	{i18n("zeta(x)-1"), "zetam1", (func_t)gsl_sf_zetam1, 1, nullptr, FunctionGroups::ZetaFunctions},
	{i18n("Hurwitz zeta function"), "hzeta", (func_t)gsl_sf_hzeta, 2, nullptr, FunctionGroups::ZetaFunctions},
	{i18n("Eta function for integer n"), "etaint", (func_t)nsl_sf_etaint, 1, nullptr, FunctionGroups::ZetaFunctions},
	{i18n("Eta function"), "eta", (func_t)gsl_sf_eta, 1, nullptr, FunctionGroups::ZetaFunctions},

	// GSL Random Number Generators: see https://www.gnu.org/software/gsl/doc/html/randist.html
	// Random number generator
	{i18n("Gaussian random numbers"), "randgaussian", (func_t)nsl_sf_ran_gaussian, 1, nullptr, FunctionGroups::RandomNumberGenerator},
	{i18n("Exponential random numbers"), "randexponential", (func_t)nsl_sf_ran_exponential, 1, nullptr, FunctionGroups::RandomNumberGenerator},
	{i18n("Laplacian random numbers"), "randlaplace", (func_t)nsl_sf_ran_laplace, 1, nullptr, FunctionGroups::RandomNumberGenerator},
	{i18n("Cauchy/Lorentz random numbers"), "randcauchy", (func_t)nsl_sf_ran_cauchy, 1, nullptr, FunctionGroups::RandomNumberGenerator},
	{i18n("Rayleigh random numbers"), "randrayleigh", (func_t)nsl_sf_ran_rayleigh, 1, nullptr, FunctionGroups::RandomNumberGenerator},
	{i18n("Landau random numbers"), "randlandau", (func_t)nsl_sf_ran_landau, 0, nullptr, FunctionGroups::RandomNumberGenerator},
	{i18n("Levy alpha-stable random numbers"), "randlevy", (func_t)nsl_sf_ran_levy, 2, nullptr, FunctionGroups::RandomNumberGenerator},
	{i18n("Gamma random numbers"), "randgamma", (func_t)nsl_sf_ran_gamma, 2, nullptr, FunctionGroups::RandomNumberGenerator},
	{i18n("Flat random numbers"), "randflat", (func_t)nsl_sf_ran_flat, 2, nullptr, FunctionGroups::RandomNumberGenerator},
	{i18n("Lognormal random numbers"), "randlognormal", (func_t)nsl_sf_ran_lognormal, 2, nullptr, FunctionGroups::RandomNumberGenerator},

	{i18n("Chi-squared random numbers"), "randchisq", (func_t)nsl_sf_ran_chisq, 1, nullptr, FunctionGroups::RandomNumberGenerator},
	{i18n("t-distributed random numbers"), "randtdist", (func_t)nsl_sf_ran_tdist, 1, nullptr, FunctionGroups::RandomNumberGenerator},
	{i18n("Logistic random numbers"), "randlogistic", (func_t)nsl_sf_ran_logistic, 1, nullptr, FunctionGroups::RandomNumberGenerator},
	{i18n("Poisson random numbers"), "randpoisson", (func_t)nsl_sf_ran_poisson, 1, nullptr, FunctionGroups::RandomNumberGenerator},
	{i18n("Bernoulli random numbers"), "randbernoulli", (func_t)nsl_sf_ran_bernoulli, 1, nullptr, FunctionGroups::RandomNumberGenerator},
	{i18n("Binomial random numbers"), "randbinomial", (func_t)nsl_sf_ran_binomial, 2, nullptr, FunctionGroups::RandomNumberGenerator},

	// GSL Random Number Distributions: see https://www.gnu.org/software/gsl/doc/html/randist.html
	// Gaussian Distribution
	{i18n("Probability density for a Gaussian distribution"), "gaussian", (func_t)gsl_ran_gaussian_pdf, 2, nullptr, FunctionGroups::GaussianDistribution},
	{i18n("Probability density for a unit Gaussian distribution"),
	 "ugaussian",
	 (func_t)gsl_ran_ugaussian_pdf,
	 1,
	 nullptr,
	 FunctionGroups::GaussianDistribution},
	{i18n("Cumulative distribution function P"), "gaussianP", (func_t)gsl_cdf_gaussian_P, 2, nullptr, FunctionGroups::GaussianDistribution},
	{i18n("Cumulative distribution function Q"), "gaussianQ", (func_t)gsl_cdf_gaussian_Q, 2, nullptr, FunctionGroups::GaussianDistribution},
	{i18n("Inverse cumulative distribution function P"), "gaussianPinv", (func_t)gsl_cdf_gaussian_Pinv, 2, nullptr, FunctionGroups::GaussianDistribution},
	{i18n("Inverse cumulative distribution function Q"), "gaussianQinv", (func_t)gsl_cdf_gaussian_Qinv, 2, nullptr, FunctionGroups::GaussianDistribution},
	{i18n("Cumulative unit distribution function P"), "ugaussianP", (func_t)gsl_cdf_ugaussian_P, 1, nullptr, FunctionGroups::GaussianDistribution},
	{i18n("Cumulative unit distribution function Q"), "ugaussianQ", (func_t)gsl_cdf_ugaussian_Q, 1, nullptr, FunctionGroups::GaussianDistribution},
	{i18n("Inverse cumulative unit distribution function P"),
	 "ugaussianPinv",
	 (func_t)gsl_cdf_ugaussian_Pinv,
	 1,
	 nullptr,
	 FunctionGroups::GaussianDistribution},
	{i18n("Inverse cumulative unit distribution function Q"),
	 "ugaussianQinv",
	 (func_t)gsl_cdf_ugaussian_Qinv,
	 1,
	 nullptr,
	 FunctionGroups::GaussianDistribution},

	{i18n("Probability density for Gaussian tail distribution"),
	 "gaussiantail",
	 (func_t)gsl_ran_gaussian_tail_pdf,
	 3,
	 nullptr,
	 FunctionGroups::GaussianDistribution},
	{i18n("Probability density for unit Gaussian tail distribution"),
	 "ugaussiantail",
	 (func_t)gsl_ran_ugaussian_tail_pdf,
	 2,
	 nullptr,
	 FunctionGroups::GaussianDistribution},
	{i18n("Probability density for a bivariate Gaussian distribution"),
	 "gaussianbi",
	 (func_t)gsl_ran_bivariate_gaussian_pdf,
	 5,
	 nullptr,
	 FunctionGroups::GaussianDistribution},

	// Exponential Distribution
	{i18n("Probability density for an exponential distribution"),
	 "exponential",
	 (func_t)gsl_ran_exponential_pdf,
	 2,
	 nullptr,
	 FunctionGroups::ExponentialDistribution},
	{i18n("Cumulative distribution function P"), "exponentialP", (func_t)gsl_cdf_exponential_P, 2, nullptr, FunctionGroups::ExponentialDistribution},
	{i18n("Cumulative distribution function Q"), "exponentialQ", (func_t)gsl_cdf_exponential_Q, 2, nullptr, FunctionGroups::ExponentialDistribution},
	{i18n("Inverse cumulative distribution function P"),
	 "exponentialPinv",
	 (func_t)gsl_cdf_exponential_Pinv,
	 2,
	 nullptr,
	 FunctionGroups::ExponentialDistribution},
	{i18n("Inverse cumulative distribution function Q"),
	 "exponentialQinv",
	 (func_t)gsl_cdf_exponential_Qinv,
	 2,
	 nullptr,
	 FunctionGroups::ExponentialDistribution},

	// Laplace Distribution
	{i18n("Probability density for a Laplace distribution"), "laplace", (func_t)gsl_ran_laplace_pdf, 2, nullptr, FunctionGroups::LaplaceDistribution},
	{i18n("Cumulative distribution function P"), "laplaceP", (func_t)gsl_cdf_laplace_P, 2, nullptr, FunctionGroups::LaplaceDistribution},
	{i18n("Cumulative distribution function Q"), "laplaceQ", (func_t)gsl_cdf_laplace_Q, 2, nullptr, FunctionGroups::LaplaceDistribution},
	{i18n("Inverse cumulative distribution function P"), "laplacePinv", (func_t)gsl_cdf_laplace_Pinv, 2, nullptr, FunctionGroups::LaplaceDistribution},
	{i18n("Inverse cumulative distribution function Q"), "laplaceQinv", (func_t)gsl_cdf_laplace_Qinv, 2, nullptr, FunctionGroups::LaplaceDistribution},

	// Exponential Power Distribution
	{i18n("Probability density for an exponential power distribution"),
	 "exppow",
	 (func_t)gsl_ran_exppow_pdf,
	 3,
	 nullptr,
	 FunctionGroups::ExponentialPowerDistribution},
	{i18n("cumulative distribution function P"), "exppowP", (func_t)gsl_cdf_exppow_P, 3, nullptr, FunctionGroups::ExponentialPowerDistribution},
	{i18n("Cumulative distribution function Q"), "exppowQ", (func_t)gsl_cdf_exppow_Q, 3, nullptr, FunctionGroups::ExponentialPowerDistribution},

	// Cauchy Distribution
	{i18n("Probability density for a Cauchy distribution"), "cauchy", (func_t)gsl_ran_cauchy_pdf, 2, nullptr, FunctionGroups::CauchyDistribution},
	{i18n("Cumulative distribution function P"), "cauchyP", (func_t)gsl_cdf_cauchy_P, 2, nullptr, FunctionGroups::CauchyDistribution},
	{i18n("Cumulative distribution function Q"), "cauchyQ", (func_t)gsl_cdf_cauchy_Q, 2, nullptr, FunctionGroups::CauchyDistribution},
	{i18n("Inverse cumulative distribution function P"), "cauchyPinv", (func_t)gsl_cdf_cauchy_Pinv, 2, nullptr, FunctionGroups::CauchyDistribution},
	{i18n("Inverse cumulative distribution function Q"), "cauchyQinv", (func_t)gsl_cdf_cauchy_Qinv, 2, nullptr, FunctionGroups::CauchyDistribution},

	// Rayleigh Distribution
	{i18n("Probability density for a Rayleigh distribution"), "rayleigh", (func_t)gsl_ran_rayleigh_pdf, 2, nullptr, FunctionGroups::RayleighDistribution},
	{i18n("Cumulative distribution function P"), "rayleighP", (func_t)gsl_cdf_rayleigh_P, 2, nullptr, FunctionGroups::RayleighDistribution},
	{i18n("Cumulative distribution function Q"), "rayleighQ", (func_t)gsl_cdf_rayleigh_Q, 2, nullptr, FunctionGroups::RayleighDistribution},
	{i18n("Inverse cumulative distribution function P"), "rayleighPinv", (func_t)gsl_cdf_rayleigh_Pinv, 2, nullptr, FunctionGroups::RayleighDistribution},
	{i18n("Inverse cumulative distribution function Q"), "rayleighQinv", (func_t)gsl_cdf_rayleigh_Qinv, 2, nullptr, FunctionGroups::RayleighDistribution},
	{i18n("Probability density for a Rayleigh tail distribution"),
	 "rayleigh_tail",
	 (func_t)gsl_ran_rayleigh_tail_pdf,
	 3,
	 nullptr,
	 FunctionGroups::RayleighDistribution},

	// Landau Distribution
	{i18n("Probability density for a Landau distribution"), "landau", (func_t)gsl_ran_landau_pdf, 1, nullptr, FunctionGroups::LandauDistribution},

	// Gamma Distribution
	{i18n("Probability density for a gamma distribution"), "gammapdf", (func_t)gsl_ran_gamma_pdf, 3, nullptr, FunctionGroups::GammaDistribution},
	{i18n("Cumulative distribution function P"), "gammaP", (func_t)gsl_cdf_gamma_P, 3, nullptr, FunctionGroups::GammaDistribution},
	{i18n("Cumulative distribution function Q"), "gammaQ", (func_t)gsl_cdf_gamma_Q, 3, nullptr, FunctionGroups::GammaDistribution},
	{i18n("Inverse cumulative distribution function P"), "gammaPinv", (func_t)gsl_cdf_gamma_Pinv, 3, nullptr, FunctionGroups::GammaDistribution},
	{i18n("Inverse cumulative distribution function Q"), "gammaQinv", (func_t)gsl_cdf_gamma_Qinv, 3, nullptr, FunctionGroups::GammaDistribution},

	// Flat (Uniform) Distribution
	{i18n("Probability density for a uniform distribution"), "flat", (func_t)gsl_ran_flat_pdf, 3, nullptr, FunctionGroups::FlatUniformDistribution},
	{i18n("Cumulative distribution function P"), "flatP", (func_t)gsl_cdf_flat_P, 3, nullptr, FunctionGroups::FlatUniformDistribution},
	{i18n("Cumulative distribution function Q"), "flatQ", (func_t)gsl_cdf_flat_Q, 3, nullptr, FunctionGroups::FlatUniformDistribution},
	{i18n("Inverse cumulative distribution function P"), "flatPinv", (func_t)gsl_cdf_flat_Pinv, 3, nullptr, FunctionGroups::FlatUniformDistribution},
	{i18n("Inverse cumulative distribution function Q"), "flatQinv", (func_t)gsl_cdf_flat_Qinv, 3, nullptr, FunctionGroups::FlatUniformDistribution},

	// Lognormal Distribution
	{i18n("Probability density for a lognormal distribution"), "lognormal", (func_t)gsl_ran_lognormal_pdf, 3, nullptr, FunctionGroups::LognormalDistribution},
	{i18n("Cumulative distribution function P"), "lognormalP", (func_t)gsl_cdf_lognormal_P, 3, nullptr, FunctionGroups::LognormalDistribution},
	{i18n("Cumulative distribution function Q"), "lognormalQ", (func_t)gsl_cdf_lognormal_Q, 3, nullptr, FunctionGroups::LognormalDistribution},
	{i18n("Inverse cumulative distribution function P"), "lognormalPinv", (func_t)gsl_cdf_lognormal_Pinv, 3, nullptr, FunctionGroups::LognormalDistribution},
	{i18n("Inverse cumulative distribution function Q"), "lognormalQinv", (func_t)gsl_cdf_lognormal_Qinv, 3, nullptr, FunctionGroups::LognormalDistribution},

	// Chi-squared Distribution
	{i18n("Probability density for a chi squared distribution"), "chisq", (func_t)gsl_ran_chisq_pdf, 2, nullptr, FunctionGroups::ChisquaredDistribution},
	{i18n("Cumulative distribution function P"), "chisqP", (func_t)gsl_cdf_chisq_P, 2, nullptr, FunctionGroups::ChisquaredDistribution},
	{i18n("Cumulative distribution function Q"), "chisqQ", (func_t)gsl_cdf_chisq_Q, 2, nullptr, FunctionGroups::ChisquaredDistribution},
	{i18n("Inverse cumulative distribution function P"), "chisqPinv", (func_t)gsl_cdf_chisq_Pinv, 2, nullptr, FunctionGroups::ChisquaredDistribution},
	{i18n("Inverse cumulative distribution function Q"), "chisqQinv", (func_t)gsl_cdf_chisq_Qinv, 2, nullptr, FunctionGroups::ChisquaredDistribution},

	// F-distribution
	{i18n("Probability density for a F-distribution"), "fdist", (func_t)gsl_ran_fdist_pdf, 3, nullptr, FunctionGroups::Fdistribution},
	{i18n("Cumulative distribution function P"), "fdistP", (func_t)gsl_cdf_fdist_P, 3, nullptr, FunctionGroups::Fdistribution},
	{i18n("Cumulative distribution function Q"), "fdistQ", (func_t)gsl_cdf_fdist_Q, 3, nullptr, FunctionGroups::Fdistribution},
	{i18n("Inverse cumulative distribution function P"), "fdistPinv", (func_t)gsl_cdf_fdist_Pinv, 3, nullptr, FunctionGroups::Fdistribution},
	{i18n("Inverse cumulative distribution function Q"), "fdistQinv", (func_t)gsl_cdf_fdist_Qinv, 3, nullptr, FunctionGroups::Fdistribution},

	// t-distribution
	{i18n("Probability density for a t-distribution"), "tdist", (func_t)gsl_ran_tdist_pdf, 2, nullptr, FunctionGroups::Tdistribution},
	{i18n("Cumulative distribution function P"), "tdistP", (func_t)gsl_cdf_tdist_P, 2, nullptr, FunctionGroups::Tdistribution},
	{i18n("Cumulative distribution function Q"), "tdistQ", (func_t)gsl_cdf_tdist_Q, 2, nullptr, FunctionGroups::Tdistribution},
	{i18n("Inverse cumulative distribution function P"), "tdistPinv", (func_t)gsl_cdf_tdist_Pinv, 2, nullptr, FunctionGroups::Tdistribution},
	{i18n("Inverse cumulative distribution function Q"), "tdistQinv", (func_t)gsl_cdf_tdist_Qinv, 2, nullptr, FunctionGroups::Tdistribution},

	// Beta Distribution
	{i18n("Probability density for a beta distribution"), "betapdf", (func_t)gsl_ran_beta_pdf, 3, nullptr, FunctionGroups::BetaDistribution},
	{i18n("Cumulative distribution function P"), "betaP", (func_t)gsl_cdf_beta_P, 3, nullptr, FunctionGroups::BetaDistribution},
	{i18n("Cumulative distribution function Q"), "betaQ", (func_t)gsl_cdf_beta_Q, 3, nullptr, FunctionGroups::BetaDistribution},
	{i18n("Inverse cumulative distribution function P"), "betaPinv", (func_t)gsl_cdf_beta_Pinv, 3, nullptr, FunctionGroups::BetaDistribution},
	{i18n("Inverse cumulative distribution function Q"), "betaQinv", (func_t)gsl_cdf_beta_Qinv, 3, nullptr, FunctionGroups::BetaDistribution},

	// Logistic Distribution
	{i18n("Probability density for a logistic distribution"), "logistic", (func_t)gsl_ran_logistic_pdf, 2, nullptr, FunctionGroups::LogisticDistribution},
	{i18n("Cumulative distribution function P"), "logisticP", (func_t)gsl_cdf_logistic_P, 2, nullptr, FunctionGroups::LogisticDistribution},
	{i18n("Cumulative distribution function Q"), "logisticQ", (func_t)gsl_cdf_logistic_Q, 2, nullptr, FunctionGroups::LogisticDistribution},
	{i18n("Inverse cumulative distribution function P"), "logisticPinv", (func_t)gsl_cdf_logistic_Pinv, 2, nullptr, FunctionGroups::LogisticDistribution},
	{i18n("Inverse cumulative distribution function Q"), "logisticQinv", (func_t)gsl_cdf_logistic_Qinv, 2, nullptr, FunctionGroups::LogisticDistribution},

	// Pareto Distribution
	{i18n("Probability density for a Pareto distribution"), "pareto", (func_t)gsl_ran_pareto_pdf, 3, nullptr, FunctionGroups::ParetoDistribution},
	{i18n("Cumulative distribution function P"), "paretoP", (func_t)gsl_cdf_pareto_P, 3, nullptr, FunctionGroups::ParetoDistribution},
	{i18n("Cumulative distribution function Q"), "paretoQ", (func_t)gsl_cdf_pareto_Q, 3, nullptr, FunctionGroups::ParetoDistribution},
	{i18n("Inverse cumulative distribution function P"), "paretoPinv", (func_t)gsl_cdf_pareto_Pinv, 3, nullptr, FunctionGroups::ParetoDistribution},
	{i18n("Inverse cumulative distribution function Q"), "paretoQinv", (func_t)gsl_cdf_pareto_Qinv, 3, nullptr, FunctionGroups::ParetoDistribution},

	// Weibull Distribution
	{i18n("Probability density for a Weibull distribution"), "weibull", (func_t)gsl_ran_weibull_pdf, 3, nullptr, FunctionGroups::WeibullDistribution},
	{i18n("Cumulative distribution function P"), "weibullP", (func_t)gsl_cdf_weibull_P, 3, nullptr, FunctionGroups::WeibullDistribution},
	{i18n("Cumulative distribution function Q"), "weibullQ", (func_t)gsl_cdf_weibull_Q, 3, nullptr, FunctionGroups::WeibullDistribution},
	{i18n("Inverse cumulative distribution function P"), "weibullPinv", (func_t)gsl_cdf_weibull_Pinv, 3, nullptr, FunctionGroups::WeibullDistribution},
	{i18n("Inverse cumulative distribution function Q"), "weibullQinv", (func_t)gsl_cdf_weibull_Qinv, 3, nullptr, FunctionGroups::WeibullDistribution},

	// Gumbel Distribution
	{i18n("Probability density for a Type-1 Gumbel distribution"), "gumbel1", (func_t)gsl_ran_gumbel1_pdf, 3, nullptr, FunctionGroups::GumbelDistribution},
	{i18n("Cumulative distribution function P"), "gumbel1P", (func_t)gsl_cdf_gumbel1_P, 3, nullptr, FunctionGroups::GumbelDistribution},
	{i18n("Cumulative distribution function Q"), "gumbel1Q", (func_t)gsl_cdf_gumbel1_Q, 3, nullptr, FunctionGroups::GumbelDistribution},
	{i18n("Inverse cumulative distribution function P"), "gumbel1Pinv", (func_t)gsl_cdf_gumbel1_Pinv, 3, nullptr, FunctionGroups::GumbelDistribution},
	{i18n("Inverse cumulative distribution function Q"), "gumbel1Qinv", (func_t)gsl_cdf_gumbel1_Qinv, 3, nullptr, FunctionGroups::GumbelDistribution},
	{i18n("Probability density for a Type-2 Gumbel distribution"), "gumbel2", (func_t)gsl_ran_gumbel2_pdf, 3, nullptr, FunctionGroups::GumbelDistribution},
	{i18n("Cumulative distribution function P"), "gumbel2P", (func_t)gsl_cdf_gumbel2_P, 3, nullptr, FunctionGroups::GumbelDistribution},
	{i18n("Cumulative distribution function Q"), "gumbel2Q", (func_t)gsl_cdf_gumbel2_Q, 3, nullptr, FunctionGroups::GumbelDistribution},
	{i18n("Inverse cumulative distribution function P"), "gumbel2Pinv", (func_t)gsl_cdf_gumbel2_Pinv, 3, nullptr, FunctionGroups::GumbelDistribution},
	{i18n("Inverse cumulative distribution function Q"), "gumbel2Qinv", (func_t)gsl_cdf_gumbel2_Qinv, 3, nullptr, FunctionGroups::GumbelDistribution},

	// Poisson Distribution
	{i18n("Probability density for a Poisson distribution"), "poisson", (func_t)nsl_sf_poisson, 2, nullptr, FunctionGroups::PoissonDistribution},
	{i18n("Cumulative distribution function P"), "poissonP", (func_t)gsl_cdf_poisson_P, 2, nullptr, FunctionGroups::PoissonDistribution},
	{i18n("Cumulative distribution function Q"), "poissonQ", (func_t)gsl_cdf_poisson_Q, 2, nullptr, FunctionGroups::PoissonDistribution},

	// Bernoulli Distribution
	{i18n("Probability density for a Bernoulli distribution"), "bernoulli", (func_t)nsl_sf_bernoulli, 2, nullptr, FunctionGroups::BernoulliDistribution},

	// Binomial Distribution
	{i18n("Probability density for a binomial distribution"), "binomial", (func_t)nsl_sf_binomial, 3, nullptr, FunctionGroups::BinomialDistribution},
	{i18n("Cumulative distribution function P"), "binomialP", (func_t)gsl_cdf_binomial_P, 3, nullptr, FunctionGroups::BinomialDistribution},
	{i18n("Cumulative distribution function Q"), "binomialQ", (func_t)gsl_cdf_binomial_Q, 3, nullptr, FunctionGroups::BinomialDistribution},
	{i18n("Probability density for a negative binomial distribution"),
	 "negative_binomial",
	 (func_t)nsl_sf_negative_binomial,
	 3,
	 nullptr,
	 FunctionGroups::BinomialDistribution},
	{i18n("Cumulative distribution function P"), "negative_binomialP", (func_t)gsl_cdf_negative_binomial_P, 3, nullptr, FunctionGroups::BinomialDistribution},
	{i18n("Cumulative distribution function Q"), "negative_binomialQ", (func_t)gsl_cdf_negative_binomial_Q, 3, nullptr, FunctionGroups::BinomialDistribution},

	// Pascal Distribution
	{i18n("Probability density for a Pascal distribution"), "pascal", (func_t)nsl_sf_pascal, 3, nullptr, FunctionGroups::PascalDistribution},
	{i18n("Cumulative distribution function P"), "pascalP", (func_t)gsl_cdf_pascal_P, 3, nullptr, FunctionGroups::PascalDistribution},
	{i18n("Cumulative distribution function Q"), "pascalQ", (func_t)gsl_cdf_pascal_Q, 3, nullptr, FunctionGroups::PascalDistribution},

	// Geometric Distribution
	{i18n("Probability density for a geometric distribution"), "geometric", (func_t)nsl_sf_geometric, 2, nullptr, FunctionGroups::GeometricDistribution},
	{i18n("Cumulative distribution function P"), "geometricP", (func_t)gsl_cdf_geometric_P, 2, nullptr, FunctionGroups::GeometricDistribution},
	{i18n("Cumulative distribution function Q"), "geometricQ", (func_t)gsl_cdf_geometric_Q, 2, nullptr, FunctionGroups::GeometricDistribution},

	// Hypergeometric Distribution
	{i18n("Probability density for a hypergeometric distribution"),
	 "hypergeometric",
	 (func_t)nsl_sf_hypergeometric,
	 4,
	 nullptr,
	 FunctionGroups::HypergeometricDistribution},
	{i18n("Cumulative distribution function P"), "hypergeometricP", (func_t)gsl_cdf_hypergeometric_P, 4, nullptr, FunctionGroups::HypergeometricDistribution},
	{i18n("Cumulative distribution function Q"), "hypergeometricQ", (func_t)gsl_cdf_hypergeometric_Q, 4, nullptr, FunctionGroups::HypergeometricDistribution},

	// Logarithmic Distribution
	{i18n("Probability density for a logarithmic distribution"),
	 "logarithmic",
	 (func_t)nsl_sf_logarithmic,
	 2,
	 nullptr,
	 FunctionGroups::LogarithmicDistribution},
};

const int _number_functions = sizeof(_functions) / sizeof(funs);

// ########################################################################
// #### Function declarations ############################################
// ########################################################################

double greaterThan(const double v1, const double v2) {
	return v1 > v2;
}

double greaterEqualThan(const double v1, const double v2) {
	return v1 >= v2;
}

double lessThan(const double v1, const double v2) {
	return v1 < v2;
}

double lessEqualThan(const double v1, const double v2) {
	return v1 <= v2;
}

double equal(const double v1, const double v2) {
	return v1 == v2;
}

bool convertDoubleToBool(const double value) {
	return value != 0;
}

double ifCondition(const double condition, const double valueIfTrue, const double valueIfFalse) {
	if (convertDoubleToBool(condition))
		return valueIfTrue;
	return valueIfFalse;
}

double andFunction(const double v1, const double v2) {
	return (convertDoubleToBool(v1) && convertDoubleToBool(v2));
}

double orFunction(const double v1, const double v2) {
	return (convertDoubleToBool(v1) || convertDoubleToBool(v2));
}

double xorFunction(const double v1, const double v2) {
	if (convertDoubleToBool(v1) == convertDoubleToBool(v2))
		return 0;
	return 1;
}

double betweenIncluded(const double x, const double min, const double max) {
	if (x >= min && x <= max)
		return 1;
	return 0;
}

double outsideIncluded(const double x, const double min, const double max) {
	if (x <= min || x >= max)
		return 1;
	return 0;
}

double between(const double x, const double min, const double max) {
	if (x > min && x < max)
		return 1;
	return 0;
}

double outside(const double x, const double min, const double max) {
	if (x < min || x > max)
		return 1;
	return 0;
}

double equalEpsilon(const double v1, const double v2, const double epsilon) {
	if (fabs(v2 - v1) <= epsilon)
		return 1;
	return 0;
}

// ########################################################################
// #### Parameter Functions ###############################################
// ########################################################################

QString parameterXE(int parameterIndex) {
	switch (parameterIndex) {
	case 0:
		return QStringLiteral("x");
	case 1:
		return QStringLiteral("e");
	}
	return i18n("Invalid");
}

QString ifParameterNames(int parameterIndex) {
	switch (parameterIndex) {
	case 0:
		return i18n("condition");
	case 1:
		return i18n("trueValue");
	case 2:
		return i18n("falseValue");
	}
	return i18n("Invalid");
}

QString nsl_sf_mathieuParameterNames(int parameterIndex) {
	switch (parameterIndex) {
	case 0:
		return i18n("n");
	case 1:
		return i18n("j");
	case 2:
		return i18n("q");
	case 3:
		return i18n("x");
	}
	return i18n("Invalid");
}

QString betweenOutsideParameterNames(int parameterIndex) {
	switch (parameterIndex) {
	case 0:
		return i18n("x");
	case 1:
		return i18n("min");
	case 2:
		return i18n("max");
	}
	return i18n("Invalid");
}

QString equalEpsilonParameterNames(int parameterIndex) {
	switch (parameterIndex) {
	case 0:
		return i18n("v1");
	case 1:
		return i18n("v2");
	case 2:
		return i18n("ep");
	}
	return i18n("Invalid");
}
