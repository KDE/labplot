/*
	File                 : functions.cpp
	Project              : LabPlot
	Description          : definition of functions
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2014 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2014-2024 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "functions.h"
#include "backend/nsl/nsl_math.h"
#include "backend/nsl/nsl_sf_basic.h"
#include "parser.h"
#include "parserFunctionTypes.h"

#include <KLocalizedString>

#include <gsl/gsl_cdf.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_sf.h>

namespace Parsing {

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
double notFunction(const double v);
double between(const double x, const double min, const double max);
double outside(const double x, const double min, const double max);
double equalEpsilon(const double v1, const double v2, const double epsilon);
double betweenIncluded(const double x, const double min, const double max);
double outsideIncluded(const double x, const double min, const double max);
double roundn(const double v, const double precision);

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
	case FunctionGroups::TriangularDistribution:
		return i18n("Triangular Distribution");
	case FunctionGroups::END:
		break;
	}
	return i18n("Unknown Function");
}

const char* specialfun_cell = "cell";
const char* specialfun_cell_default_value = "cell_with_default";
const char* specialfun_ma = "ma";
const char* specialfun_mr = "mr";
const char* specialfun_smmin = "smmin";
const char* specialfun_smmax = "smmax";
const char* specialfun_sma = "sma";
const char* specialfun_smr = "smr";
const char* specialfun_psample = "psample";
const char* specialfun_rsample = "rsample";

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
const char* cell_curr_column = "cell_curr_column";
const char* cell_curr_column_default = "cell_curr_column_with_default";

// clang-format off

	// Special functions depending on variables
	struct funs _special_functions[] = {
		// Moving Statistics
		// Important: when adding new function, implement them in Expressionhandler or somewhere else!
		{[]() { return i18n("Cell (index; variable)"); }, specialfun_cell, func_tValueVariablePayload(), 2, nullptr, FunctionGroups::MovingStatistics},
		{[]() { return i18n("Cell (index; default_value; variable)"); }, specialfun_cell_default_value, func_t2ValueVariablePayload(), 3, nullptr, FunctionGroups::MovingStatistics},
		{[]() { return i18n("Moving Average"); }, specialfun_ma, func_tVariablePayload(), 1, nullptr, FunctionGroups::MovingStatistics},
		{[]() { return i18n("Moving Range"); }, specialfun_mr, func_tVariablePayload(), 1, nullptr, FunctionGroups::MovingStatistics},
		{[]() { return i18n("Simple Moving Minimum"); }, specialfun_smmin, func_tValueVariablePayload(), 2, nullptr, FunctionGroups::MovingStatistics},
		{[]() { return i18n("Simple Moving Maximum"); }, specialfun_smmax, func_tValueVariablePayload(), 2, nullptr, FunctionGroups::MovingStatistics},
		{[]() { return i18n("Simple Moving Average"); }, specialfun_sma, func_tValueVariablePayload(), 2, nullptr, FunctionGroups::MovingStatistics},
		{[]() { return i18n("Simple Moving Range"); }, specialfun_smr, func_tValueVariablePayload(), 2, nullptr, FunctionGroups::MovingStatistics},
		{[]() { return i18n("Period sample"); }, specialfun_psample, func_tValueVariablePayload(), 2, nullptr, FunctionGroups::MovingStatistics},
		{[]() { return i18n("Random sample"); }, specialfun_rsample, func_tVariablePayload(), 1, nullptr, FunctionGroups::MovingStatistics},

		{[]() { return i18n("Current column cell (index)"); }, cell_curr_column, func_tValuePayload(), 1, nullptr, FunctionGroups::MovingStatistics},
		{[]() { return i18n("Current column cell (index; default_value)"); }, cell_curr_column_default, func_t2ValuePayload(), 2, nullptr, FunctionGroups::MovingStatistics},

		// Values independent of the row index!!!
		// Important: When adding function here, implement it somewhere. For example column functions are implemented in ColumnPrivate!
		{[]() { return i18n("Size"); }, colfun_size, func_tVariablePayload(), 1, nullptr, FunctionGroups::ColumnStatistics},
		{[]() { return i18n("Minimum"); }, colfun_min, func_tVariablePayload(), 1, nullptr, FunctionGroups::ColumnStatistics},
		{[]() { return i18n("Maximum"); }, colfun_max, func_tVariablePayload(), 1, nullptr, FunctionGroups::ColumnStatistics},
		{[]() { return i18n("Arithmetic mean"); }, colfun_mean, func_tVariablePayload(), 1, nullptr, FunctionGroups::ColumnStatistics},
		{[]() { return i18n("Median"); }, colfun_median, func_tVariablePayload(), 1, nullptr, FunctionGroups::ColumnStatistics},
		{[]() { return i18n("Standard deviation"); }, colfun_stdev, func_tVariablePayload(), 1, nullptr, FunctionGroups::ColumnStatistics},
		{[]() { return i18n("Variance"); }, colfun_var, func_tVariablePayload(), 1, nullptr, FunctionGroups::ColumnStatistics},
		{[]() { return i18n("Geometric mean"); }, colfun_gm, func_tVariablePayload(), 1, nullptr, FunctionGroups::ColumnStatistics},
		{[]() { return i18n("Harmonic mean"); }, colfun_hm, func_tVariablePayload(), 1, nullptr, FunctionGroups::ColumnStatistics},
		{[]() { return i18n("Contraharmonic mean"); }, colfun_chm, func_tVariablePayload(), 1, nullptr, FunctionGroups::ColumnStatistics},
		{[]() { return i18n("Mode"); }, colfun_mode, func_tVariablePayload(), 1, nullptr, FunctionGroups::ColumnStatistics},
		{[]() { return i18n("First quartile"); }, colfun_quartile1, func_tVariablePayload(), 1, nullptr, FunctionGroups::ColumnStatistics},
		{[]() { return i18n("Third quartile"); }, colfun_quartile3, func_tVariablePayload(), 1, nullptr, FunctionGroups::ColumnStatistics},
		{[]() { return i18n("Interquartile range"); }, colfun_iqr, func_tVariablePayload(), 1, nullptr, FunctionGroups::ColumnStatistics},
		{[]() { return i18n("1st percentile"); }, colfun_percentile1, func_tVariablePayload(), 1, nullptr, FunctionGroups::ColumnStatistics},
		{[]() { return i18n("5th percentile"); }, colfun_percentile5, func_tVariablePayload(), 1, nullptr, FunctionGroups::ColumnStatistics},
		{[]() { return i18n("10th percentile"); }, colfun_percentile10, func_tVariablePayload(), 1, nullptr, FunctionGroups::ColumnStatistics},
		{[]() { return i18n("90th percentile"); }, colfun_percentile90, func_tVariablePayload(), 1, nullptr, FunctionGroups::ColumnStatistics},
		{[]() { return i18n("95th percentile"); }, colfun_percentile95, func_tVariablePayload(), 1, nullptr, FunctionGroups::ColumnStatistics},
		{[]() { return i18n("99th percentile"); }, colfun_percentile99, func_tVariablePayload(), 1, nullptr, FunctionGroups::ColumnStatistics},
		{[]() { return i18n("Trimean"); }, colfun_trimean, func_tVariablePayload(), 1, nullptr, FunctionGroups::ColumnStatistics},
		{[]() { return i18n("Mean absolute deviation"); }, colfun_meandev, func_tVariablePayload(), 1, nullptr, FunctionGroups::ColumnStatistics},
		{[]() { return i18n("Mean absolute deviation around median"); }, colfun_meandevmedian, func_tVariablePayload(), 1, nullptr, FunctionGroups::ColumnStatistics},
		{[]() { return i18n("Median absolute deviation"); }, colfun_mediandev, func_tVariablePayload(), 1, nullptr, FunctionGroups::ColumnStatistics},
		{[]() { return i18n("Skewness"); }, colfun_skew, func_tVariablePayload(), 1, nullptr, FunctionGroups::ColumnStatistics},
		{[]() { return i18n("Kurtosis"); }, colfun_kurt, func_tVariablePayload(), 1, nullptr, FunctionGroups::ColumnStatistics},
		{[]() { return i18n("Entropy"); }, colfun_entropy, func_tVariablePayload(), 1, nullptr, FunctionGroups::ColumnStatistics},
		{[]() { return i18n("Quantile"); }, colfun_quantile, func_tValueVariablePayload(), 2, nullptr, FunctionGroups::ColumnStatistics},
		{[]() { return i18n("Percentile"); }, colfun_percentile, func_tValueVariablePayload(), 2, nullptr, FunctionGroups::ColumnStatistics},
	};
	const int _number_specialfunctions = sizeof(_special_functions) / sizeof(funs);

	/* list of functions (description must be unique!) */
	struct funs _functions[] = {
		// Standard Mathematical Functions
		{[]() { return i18n("pseudo-random integer [0, RAND_MAX]"); }, "rand", nsl_sf_rand, 0, nullptr, FunctionGroups::StandardMathematicalFunctions},
		{[]() { return i18n("nonlinear additive feedback rng [0, RAND_MAX]"); }, "random", nsl_sf_random, 0, nullptr, FunctionGroups::StandardMathematicalFunctions},
		{[]() { return i18n("nonlinear additive feedback rng [0, 1]"); }, "drand", nsl_sf_drand, 0, nullptr, FunctionGroups::StandardMathematicalFunctions},
		{[]() { return i18n("Smallest integral value not less"); }, "ceil", static_cast<double (*)(double)>(&ceil), 1, nullptr, FunctionGroups::StandardMathematicalFunctions},
		{[]() { return i18n("Absolute value"); }, "fabs", static_cast<double (*)(double)>(&fabs), 1, nullptr, FunctionGroups::StandardMathematicalFunctions},

		{[]() { return i18n("Base 10 logarithm"); }, "log10", static_cast<double (*)(double)>(&log10), 1, nullptr, FunctionGroups::StandardMathematicalFunctions},
		{[]() { return i18n("Power function [x^y]"); }, "pow", static_cast<double (*)(double, double)>(&pow), 2, nullptr, FunctionGroups::StandardMathematicalFunctions},
		{[]() { return i18n("Nonnegative square root"); }, "sqrt", static_cast<double (*)(double)>(&sqrt), 1, nullptr, FunctionGroups::StandardMathematicalFunctions},
		{[]() { return i18n("Sign function"); }, "sgn", nsl_sf_sgn, 1, nullptr, FunctionGroups::StandardMathematicalFunctions},
		{[]() { return i18n("Heavyside theta function"); }, "theta", nsl_sf_theta, 1, nullptr, FunctionGroups::StandardMathematicalFunctions},
		{[]() { return i18n("Harmonic number function"); }, "harmonic", nsl_sf_harmonic, 1, nullptr, FunctionGroups::StandardMathematicalFunctions},

		{[]() { return i18n("Cube root"); }, "cbrt", static_cast<double (*)(double)>(&cbrt), 1, nullptr, FunctionGroups::StandardMathematicalFunctions},
		{[]() { return i18n("Extract the exponent"); }, "logb", static_cast<double (*)(double)>(&logb), 1, nullptr, FunctionGroups::StandardMathematicalFunctions},
		{[]() { return i18n("Round to an integer value"); }, "rint", static_cast<double (*)(double)>(&rint), 1, nullptr, FunctionGroups::StandardMathematicalFunctions},
		{[]() { return i18n("Round to the nearest integer"); }, "round", static_cast<double (*)(double)>(&round), 1, nullptr, FunctionGroups::StandardMathematicalFunctions},
		{[]() { return i18n("Round to the nearest integer"); }, "trunc", static_cast<double (*)(double)>(&trunc), 1, nullptr, FunctionGroups::StandardMathematicalFunctions},
		{[]() { return i18n("Round to y decimal places"); }, "roundn", static_cast<double (*)(double, double)>(&roundn), 2, nullptr, FunctionGroups::StandardMathematicalFunctions},
		{[]() { return QStringLiteral("log(1+x)"); }, "log1p", gsl_log1p, 1, nullptr, FunctionGroups::StandardMathematicalFunctions},
		{[]() { return QStringLiteral("x * 2^e"); }, "ldexp", nsl_sf_ldexp, 2, &parameterXE, FunctionGroups::StandardMathematicalFunctions},
		{[]() { return QStringLiteral("x^y"); }, "powint", nsl_sf_powint, 2, nullptr, FunctionGroups::StandardMathematicalFunctions},
		{[]() { return QStringLiteral("x^2"); }, "pow2", gsl_pow_2, 1, nullptr, FunctionGroups::StandardMathematicalFunctions},
		{[]() { return QStringLiteral("x^3"); }, "pow3", gsl_pow_3, 1, nullptr, FunctionGroups::StandardMathematicalFunctions},
		{[]() { return QStringLiteral("x^4"); }, "pow4", gsl_pow_4, 1, nullptr, FunctionGroups::StandardMathematicalFunctions},
		{[]() { return QStringLiteral("x^5"); }, "pow5", gsl_pow_5, 1, nullptr, FunctionGroups::StandardMathematicalFunctions},
		{[]() { return QStringLiteral("x^6"); }, "pow6", gsl_pow_6, 1, nullptr, FunctionGroups::StandardMathematicalFunctions},
		{[]() { return QStringLiteral("x^7"); }, "pow7", gsl_pow_7, 1, nullptr, FunctionGroups::StandardMathematicalFunctions},
		{[]() { return QStringLiteral("x^8"); }, "pow8", gsl_pow_8, 1, nullptr, FunctionGroups::StandardMathematicalFunctions},
		{[]() { return QStringLiteral("x^9"); }, "pow9", gsl_pow_9, 1, nullptr, FunctionGroups::StandardMathematicalFunctions},

		// Comparison Functions
		{[]() { return i18n("greaterThan"); }, "greaterThan", greaterThan, 2, nullptr, FunctionGroups::ComparisonFunctions},
		{[]() { return i18n("lessThan"); }, "lessThan", lessThan, 2, nullptr, FunctionGroups::ComparisonFunctions},
		{[]() { return i18n("greaterEqualThan"); }, "greaterEqualThan", greaterEqualThan, 2, nullptr, FunctionGroups::ComparisonFunctions},
		{[]() { return i18n("lessEqualThan"); }, "lessEqualThan", lessEqualThan, 2, nullptr, FunctionGroups::ComparisonFunctions},
		{[]() { return i18n("equal"); }, "equal", equal, 2, nullptr, FunctionGroups::ComparisonFunctions},
		{[]() { return i18n("equal with epsilon"); }, "equalE", equalEpsilon, 3, &equalEpsilonParameterNames, FunctionGroups::ComparisonFunctions},
		{[]() { return i18n("between with boundaries included"); }, "between_inc", betweenIncluded, 3, &betweenOutsideParameterNames, FunctionGroups::ComparisonFunctions},
		{[]() { return i18n("outside with boundaries included"); }, "outside_inc", outsideIncluded, 3, &betweenOutsideParameterNames, FunctionGroups::ComparisonFunctions},
		{[]() { return i18n("between with boundaries excluded"); }, "between", between, 3, &betweenOutsideParameterNames, FunctionGroups::ComparisonFunctions},
		{[]() { return i18n("outside with boundaries excluded"); }, "outside", outside, 3, &betweenOutsideParameterNames, FunctionGroups::ComparisonFunctions},

		// Logical
		{[]() { return i18n("if(condition; ifTrue; ifFalse)"); }, "if", ifCondition, 3, &ifParameterNames, FunctionGroups::LogicalFunctions},
		{[]() { return i18n("and"); }, "and", andFunction, 2, nullptr, FunctionGroups::LogicalFunctions},
		{[]() { return i18n("or"); }, "or", orFunction, 2, nullptr, FunctionGroups::LogicalFunctions},
		{[]() { return i18n("xor"); }, "xor", xorFunction, 2, nullptr, FunctionGroups::LogicalFunctions},
		{[]() { return i18n("not"); }, "not", notFunction, 1, nullptr, FunctionGroups::LogicalFunctions},

		// https://www.gnu.org/software/gsl/doc/html/specfunc.html
		// Airy Functions and Derivatives
		{[]() { return i18n("Airy function of the first kind"); }, "Ai", nsl_sf_airy_Ai, 1, nullptr, FunctionGroups::AiryFunctionsAndDerivatives},
		{[]() { return i18n("Airy function of the second kind"); }, "Bi", nsl_sf_airy_Bi, 1, nullptr, FunctionGroups::AiryFunctionsAndDerivatives},
		{[]() { return i18n("Scaled Airy function of the first kind"); }, "Ais", nsl_sf_airy_Ais, 1, nullptr, FunctionGroups::AiryFunctionsAndDerivatives},
		{[]() { return i18n("Scaled Airy function of the second kind"); }, "Bis", nsl_sf_airy_Bis, 1, nullptr, FunctionGroups::AiryFunctionsAndDerivatives},
		{[]() { return i18n("Airy function derivative of the first kind"); }, "Aid", nsl_sf_airy_Aid, 1, nullptr, FunctionGroups::AiryFunctionsAndDerivatives},
		{[]() { return i18n("Airy function derivative of the second kind"); }, "Bid", nsl_sf_airy_Bid, 1, nullptr, FunctionGroups::AiryFunctionsAndDerivatives},
		{[]() { return i18n("Scaled Airy function derivative of the first kind"); }, "Aids", nsl_sf_airy_Aids, 1, nullptr, FunctionGroups::AiryFunctionsAndDerivatives},
		{[]() { return i18n("Scaled Airy function derivative of the second kind"); }, "Bids", nsl_sf_airy_Bids, 1, nullptr, FunctionGroups::AiryFunctionsAndDerivatives},
		{[]() { return i18n("n-th zero of the Airy function of the first kind"); }, "Ai0", nsl_sf_airy_0_Ai, 1, nullptr, FunctionGroups::AiryFunctionsAndDerivatives},
		{[]() { return i18n("n-th zero of the Airy function of the second kind"); }, "Bi0", nsl_sf_airy_0_Bi, 1, nullptr, FunctionGroups::AiryFunctionsAndDerivatives},
		{[]() { return i18n("n-th zero of the Airy function derivative of the first kind"); }, "Aid0", nsl_sf_airy_0_Aid, 1, nullptr, FunctionGroups::AiryFunctionsAndDerivatives},
		{[]() { return i18n("n-th zero of the Airy function derivative of the second kind"); }, "Bid0", nsl_sf_airy_0_Bid, 1, nullptr, FunctionGroups::AiryFunctionsAndDerivatives},

		// Bessel Functions
		{[]() { return i18n("Regular cylindrical Bessel function of zeroth order"); }, "J0", gsl_sf_bessel_J0, 1, nullptr, FunctionGroups::BesselFunctions},
		{[]() { return i18n("Regular cylindrical Bessel function of first order"); }, "J1", gsl_sf_bessel_J1, 1, nullptr, FunctionGroups::BesselFunctions},
		{[]() { return i18n("Regular cylindrical Bessel function of order n"); }, "Jn", nsl_sf_bessel_Jn, 2, nullptr, FunctionGroups::BesselFunctions},
		{[]() { return i18n("Irregular cylindrical Bessel function of zeroth order"); }, "Y0", gsl_sf_bessel_Y0, 1, nullptr, FunctionGroups::BesselFunctions},
		{[]() { return i18n("Irregular cylindrical Bessel function of first order"); }, "Y1", gsl_sf_bessel_Y1, 1, nullptr, FunctionGroups::BesselFunctions},
		{[]() { return i18n("Irregular cylindrical Bessel function of order n"); }, "Yn", nsl_sf_bessel_Yn, 2, nullptr, FunctionGroups::BesselFunctions},
		{[]() { return i18n("Regular modified cylindrical Bessel function of zeroth order"); }, "I0", gsl_sf_bessel_I0, 1, nullptr, FunctionGroups::BesselFunctions},
		{[]() { return i18n("Regular modified cylindrical Bessel function of first order"); }, "I1", gsl_sf_bessel_I1, 1, nullptr, FunctionGroups::BesselFunctions},
		{[]() { return i18n("Regular modified cylindrical Bessel function of order n"); }, "In", nsl_sf_bessel_In, 2, nullptr, FunctionGroups::BesselFunctions},
		{[]() { return i18n("Scaled regular modified cylindrical Bessel function of zeroth order exp(-|x|) I0(x)"); }, "I0s", gsl_sf_bessel_I0_scaled, 1, nullptr, FunctionGroups::BesselFunctions},

		{[]() { return i18n("Scaled regular modified cylindrical Bessel function of first order exp(-|x|) I1(x)"); }, "I1s", gsl_sf_bessel_I1_scaled, 1, nullptr, FunctionGroups::BesselFunctions},
		{[]() { return i18n("Scaled regular modified cylindrical Bessel function of order n exp(-|x|) In(x)"); }, "Ins", nsl_sf_bessel_Ins, 2, nullptr, FunctionGroups::BesselFunctions},
		{[]() { return i18n("Irregular modified cylindrical Bessel function of zeroth order"); }, "K0", gsl_sf_bessel_K0, 1, nullptr, FunctionGroups::BesselFunctions},
		{[]() { return i18n("Irregular modified cylindrical Bessel function of first order"); }, "K1", gsl_sf_bessel_K1, 1, nullptr, FunctionGroups::BesselFunctions},
		{[]() { return i18n("Irregular modified cylindrical Bessel function of order n"); }, "Kn", nsl_sf_bessel_Kn, 2, nullptr, FunctionGroups::BesselFunctions},
		{[]() { return i18n("Scaled irregular modified cylindrical Bessel function of zeroth order exp(x) K0(x)"); }, "K0s", gsl_sf_bessel_K0_scaled, 1, nullptr, FunctionGroups::BesselFunctions},
		{[]() { return i18n("Scaled irregular modified cylindrical Bessel function of first order exp(x) K1(x)"); }, "K1s", gsl_sf_bessel_K1_scaled, 1, nullptr, FunctionGroups::BesselFunctions},
		{[]() { return i18n("Scaled irregular modified cylindrical Bessel function of order n exp(x) Kn(x)"); }, "Kns", nsl_sf_bessel_Kns, 2, nullptr, FunctionGroups::BesselFunctions},
		{[]() { return i18n("Regular spherical Bessel function of zeroth order"); }, "j0", gsl_sf_bessel_j0, 1, nullptr, FunctionGroups::BesselFunctions},
		{[]() { return i18n("Regular spherical Bessel function of first order"); }, "j1", gsl_sf_bessel_j1, 1, nullptr, FunctionGroups::BesselFunctions},

		{[]() { return i18n("Regular spherical Bessel function of second order"); }, "j2", gsl_sf_bessel_j2, 1, nullptr, FunctionGroups::BesselFunctions},
		{[]() { return i18n("Regular spherical Bessel function of order l"); }, "jl", nsl_sf_bessel_jl, 2, nullptr, FunctionGroups::BesselFunctions},
		{[]() { return i18n("Irregular spherical Bessel function of zeroth order"); }, "y0", gsl_sf_bessel_y0, 1, nullptr, FunctionGroups::BesselFunctions},
		{[]() { return i18n("Irregular spherical Bessel function of first order"); }, "y1", gsl_sf_bessel_y1, 1, nullptr, FunctionGroups::BesselFunctions},
		{[]() { return i18n("Irregular spherical Bessel function of second order"); }, "y2", gsl_sf_bessel_y2, 1, nullptr, FunctionGroups::BesselFunctions},
		{[]() { return i18n("Irregular spherical Bessel function of order l"); }, "yl", nsl_sf_bessel_yl, 2, nullptr, FunctionGroups::BesselFunctions},
		{[]() { return i18n("Scaled regular modified spherical Bessel function of zeroth order, exp(-|x|) i0(x)"); }, "i0s", gsl_sf_bessel_i0_scaled, 1, nullptr, FunctionGroups::BesselFunctions},
		{[]() { return i18n("Scaled regular modified spherical Bessel function of first order, exp(-|x|) i1(x)"); }, "i1s", gsl_sf_bessel_i1_scaled, 1, nullptr, FunctionGroups::BesselFunctions},
		{[]() { return i18n("Scaled regular modified spherical Bessel function of second order, exp(-|x|) i2(x)"); }, "i2s", gsl_sf_bessel_i2_scaled, 1, nullptr, FunctionGroups::BesselFunctions},
		{[]() { return i18n("Scaled regular modified spherical Bessel function of order l, exp(-|x|) il(x)"); }, "ils", nsl_sf_bessel_ils, 2, nullptr, FunctionGroups::BesselFunctions},

		{[]() { return i18n("Scaled irregular modified spherical Bessel function of zeroth order, exp(x) k0(x)"); }, "k0s", gsl_sf_bessel_k0_scaled, 1, nullptr, FunctionGroups::BesselFunctions},
		{[]() { return i18n("Scaled irregular modified spherical Bessel function of first order, exp(-|x|) k1(x)"); }, "k1s", gsl_sf_bessel_k1_scaled, 1, nullptr, FunctionGroups::BesselFunctions},
		{[]() { return i18n("Scaled irregular modified spherical Bessel function of second order, exp(-|x|) k2(x)"); }, "k2s", gsl_sf_bessel_k2_scaled, 1, nullptr, FunctionGroups::BesselFunctions},
		{[]() { return i18n("Scaled irregular modified spherical Bessel function of order l, exp(-|x|) kl(x)"); }, "kls", nsl_sf_bessel_kls, 2, nullptr, FunctionGroups::BesselFunctions},
		{[]() { return i18n("Regular cylindrical Bessel function of fractional order"); }, "Jnu", gsl_sf_bessel_Jnu, 2, nullptr, FunctionGroups::BesselFunctions},
		{[]() { return i18n("Irregular cylindrical Bessel function of fractional order"); }, "Ynu", gsl_sf_bessel_Ynu, 2, nullptr, FunctionGroups::BesselFunctions},
		{[]() { return i18n("Regular modified Bessel function of fractional order"); }, "Inu", gsl_sf_bessel_Inu, 2, nullptr, FunctionGroups::BesselFunctions},
		{[]() { return i18n("Scaled regular modified Bessel function of fractional order"); }, "Inus", gsl_sf_bessel_Inu_scaled, 2, nullptr, FunctionGroups::BesselFunctions},
		{[]() { return i18n("Irregular modified Bessel function of fractional order"); }, "Knu", gsl_sf_bessel_Knu, 2, nullptr, FunctionGroups::BesselFunctions},
		{[]() { return i18n("Logarithm of irregular modified Bessel function of fractional order"); }, "lnKnu", gsl_sf_bessel_lnKnu, 2, nullptr, FunctionGroups::BesselFunctions},

		{[]() { return i18n("Scaled irregular modified Bessel function of fractional order"); }, "Knus", gsl_sf_bessel_Knu_scaled, 2, nullptr, FunctionGroups::BesselFunctions},
		{[]() { return i18n("n-th positive zero of the Bessel function J0"); }, "J0_0", nsl_sf_bessel_0_J0, 1, nullptr, FunctionGroups::BesselFunctions},
		{[]() { return i18n("n-th positive zero of the Bessel function J1"); }, "J1_0", nsl_sf_bessel_0_J1, 1, nullptr, FunctionGroups::BesselFunctions},
		{[]() { return i18n("n-th positive zero of the Bessel function Jnu"); }, "Jnu_0", nsl_sf_bessel_0_Jnu, 2, nullptr, FunctionGroups::BesselFunctions},

		// Clausen Functions
		{[]() { return i18n("Clausen function"); }, "clausen", gsl_sf_clausen, 1, nullptr, FunctionGroups::ClausenFunctions},

		// Coulomb Functions
		{[]() { return i18n("Lowest-order normalized hydrogenic bound state radial wavefunction"); }, "hydrogenicR_1", gsl_sf_hydrogenicR_1, 2, nullptr, FunctionGroups::CoulombFunctions},
		{[]() { return i18n("n-th normalized hydrogenic bound state radial wavefunction"); }, "hydrogenicR", nsl_sf_hydrogenicR, 4, nullptr, FunctionGroups::CoulombFunctions},

		// Dawson Function
		{[]() { return i18n("Dawson's integral D(z) = sqrt(pi)/2 * exp(-z^2) * erfi(z)"); }, "dawson", nsl_sf_dawson, 1, nullptr, FunctionGroups::DawsonFunction},

		// Debye Functions
		{[]() { return i18n("First-order Debye function"); }, "D1", gsl_sf_debye_1, 1, nullptr, FunctionGroups::DebyeFunctions},
		{[]() { return i18n("Second-order Debye function"); }, "D2", gsl_sf_debye_2, 1, nullptr, FunctionGroups::DebyeFunctions},
		{[]() { return i18n("Third-order Debye function"); }, "D3", gsl_sf_debye_3, 1, nullptr, FunctionGroups::DebyeFunctions},
		{[]() { return i18n("Fourth-order Debye function"); }, "D4", gsl_sf_debye_4, 1, nullptr, FunctionGroups::DebyeFunctions},
		{[]() { return i18n("Fifth-order Debye function"); }, "D5", gsl_sf_debye_5, 1, nullptr, FunctionGroups::DebyeFunctions},
		{[]() { return i18n("Sixth-order Debye function"); }, "D6", gsl_sf_debye_6, 1, nullptr, FunctionGroups::DebyeFunctions},

		// Dilogarithm
		{[]() { return i18n("Dilogarithm for a real argument"); }, "Li2", gsl_sf_dilog, 1, nullptr, FunctionGroups::Dilogarithm},

		// Elliptic Integrals
		{[]() { return i18n("Legendre form of complete elliptic integral K"); }, "Kc", nsl_sf_ellint_Kc, 1, nullptr, FunctionGroups::EllipticIntegrals},
		{[]() { return i18n("Legendre form of complete elliptic integral E"); }, "Ec", nsl_sf_ellint_Ec, 1, nullptr, FunctionGroups::EllipticIntegrals},
		{[]() { return i18n("Legendre form of complete elliptic integral Pi"); }, "Pc", nsl_sf_ellint_Pc, 2, nullptr, FunctionGroups::EllipticIntegrals},
		{[]() { return i18n("Legendre form of incomplete elliptic integral F"); }, "F", nsl_sf_ellint_F, 2, nullptr, FunctionGroups::EllipticIntegrals},
		{[]() { return i18n("Legendre form of incomplete elliptic integral E"); }, "E", nsl_sf_ellint_E, 2, nullptr, FunctionGroups::EllipticIntegrals},
		{[]() { return i18n("Legendre form of incomplete elliptic integral P"); }, "P", nsl_sf_ellint_P, 3, nullptr, FunctionGroups::EllipticIntegrals},
		{[]() { return i18n("Legendre form of incomplete elliptic integral D"); }, "D", nsl_sf_ellint_D, 2, nullptr, FunctionGroups::EllipticIntegrals},
		{[]() { return i18n("Carlson form of incomplete elliptic integral RC"); }, "RC", nsl_sf_ellint_RC, 2, nullptr, FunctionGroups::EllipticIntegrals},
		{[]() { return i18n("Carlson form of incomplete elliptic integral RD"); }, "RD", nsl_sf_ellint_RD, 3, nullptr, FunctionGroups::EllipticIntegrals},
		{[]() { return i18n("Carlson form of incomplete elliptic integral RF"); }, "RF", nsl_sf_ellint_RF, 3, nullptr, FunctionGroups::EllipticIntegrals},
		{[]() { return i18n("Carlson form of incomplete elliptic integral RJ"); }, "RJ", nsl_sf_ellint_RJ, 4, nullptr, FunctionGroups::EllipticIntegrals},

		// Error Functions
		{[]() { return i18n("Error function"); }, "erf", gsl_sf_erf, 1, nullptr, FunctionGroups::ErrorFunctions},
		{[]() { return i18n("Complementary error function"); }, "erfc", gsl_sf_erfc, 1, nullptr, FunctionGroups::ErrorFunctions},
		{[]() { return i18n("Logarithm of complementary error function"); }, "log_erfc", gsl_sf_log_erfc, 1, nullptr, FunctionGroups::ErrorFunctions},
		{[]() { return i18n("Gaussian probability density function Z"); }, "erf_Z", gsl_sf_erf_Z, 1, nullptr, FunctionGroups::ErrorFunctions},
		{[]() { return i18n("Upper tail of the Gaussian probability function Q"); }, "erf_Q", gsl_sf_erf_Q, 1, nullptr, FunctionGroups::ErrorFunctions},
		{[]() { return i18n("Hazard function for the normal distribution Z/Q"); }, "hazard", gsl_sf_hazard, 1, nullptr, FunctionGroups::ErrorFunctions},
	#ifndef _MSC_VER
		{[]() { return i18n("Underflow-compensating function exp(x^2) erfc(x) for real x"); }, "erfcx", nsl_sf_erfcx, 1, nullptr, FunctionGroups::ErrorFunctions},
		{[]() { return i18n("Imaginary error function erfi(x) = -i erf(ix) for real x"); }, "erfi", nsl_sf_erfi, 1, nullptr, FunctionGroups::ErrorFunctions},
		{[]() { return i18n("Imaginary part of Faddeeva's scaled complex error function w(x) = exp(-x^2) erfc(-ix) for real x"); }, "im_w_of_x", nsl_sf_im_w_of_x, 1, nullptr, FunctionGroups::ErrorFunctions},
		{[]() { return i18n("Dawson's integral D(z) = sqrt(pi)/2 * exp(-z^2) * erfi(z)"); }, "dawson", nsl_sf_dawson, 1, nullptr, FunctionGroups::ErrorFunctions},
		{[]() { return i18n("Voigt profile"); }, "voigt", nsl_sf_voigt, 3, nullptr, FunctionGroups::ErrorFunctions},
	#endif
		{[]() { return i18n("Pseudo-Voigt profile (same width)"); }, "pseudovoigt1", nsl_sf_pseudovoigt1, 3, nullptr, FunctionGroups::ErrorFunctions},

		// Exponential Functions
		{[]() { return i18n("Exponential function"); }, "exp", gsl_sf_exp, 1, nullptr, FunctionGroups::ExponentialFunctions},
		{[]() { return i18n("exponentiate x and multiply by y"); }, "exp_mult", gsl_sf_exp_mult, 2, nullptr, FunctionGroups::ExponentialFunctions},
		{[]() { return QStringLiteral("exp(x) - 1"); }, "expm1", gsl_expm1, 1, nullptr, FunctionGroups::ExponentialFunctions},
		{[]() { return QStringLiteral("(exp(x)-1)/x"); }, "exprel", gsl_sf_exprel, 1, nullptr, FunctionGroups::ExponentialFunctions},
		{[]() { return QStringLiteral("2(exp(x)-1-x)/x^2"); }, "exprel2", gsl_sf_exprel_2, 1, nullptr, FunctionGroups::ExponentialFunctions},
		{[]() { return i18n("n-relative exponential"); }, "expreln", nsl_sf_exprel_n, 2, nullptr, FunctionGroups::ExponentialFunctions},

		// Exponential Integrals
		{[]() { return i18n("Exponential integral"); }, "E1", gsl_sf_expint_E1, 1, nullptr, FunctionGroups::ExponentialIntegrals},
		{[]() { return i18n("Second order exponential integral"); }, "E2", gsl_sf_expint_E2, 1, nullptr, FunctionGroups::ExponentialIntegrals},
		{[]() { return i18n("Exponential integral of order n"); }, "En", [](double v1, double v2) { return gsl_sf_expint_En(v1, v2); },  2, nullptr, FunctionGroups::ExponentialIntegrals},
		{[]() { return i18n("Exponential integral Ei"); }, "Ei", gsl_sf_expint_Ei, 1, nullptr, FunctionGroups::ExponentialIntegrals},
		{[]() { return i18n("Hyperbolic integral Shi"); }, "Shi", gsl_sf_Shi, 1, nullptr, FunctionGroups::ExponentialIntegrals},
		{[]() { return i18n("Hyperbolic integral Chi"); }, "Chi", gsl_sf_Chi, 1, nullptr, FunctionGroups::ExponentialIntegrals},
		{[]() { return i18n("Third-order exponential integral"); }, "Ei3", gsl_sf_expint_3, 1, nullptr, FunctionGroups::ExponentialIntegrals},
		{[]() { return i18n("Sine integral"); }, "Si", gsl_sf_Si, 1, nullptr, FunctionGroups::ExponentialIntegrals},
		{[]() { return i18n("Cosine integral"); }, "Ci", gsl_sf_Ci, 1, nullptr, FunctionGroups::ExponentialIntegrals},
		{[]() { return i18n("Arctangent integral"); }, "Atanint", gsl_sf_atanint, 1, nullptr, FunctionGroups::ExponentialIntegrals},

		// Fermi-Dirac Function
		{[]() { return i18n("Complete Fermi-Dirac integral with index -1"); }, "Fm1", gsl_sf_fermi_dirac_m1, 1, nullptr, FunctionGroups::FermiDiracFunction},
		{[]() { return i18n("Complete Fermi-Dirac integral with index 0"); }, "F0", gsl_sf_fermi_dirac_0, 1, nullptr, FunctionGroups::FermiDiracFunction},
		{[]() { return i18n("Complete Fermi-Dirac integral with index 1"); }, "F1", gsl_sf_fermi_dirac_1, 1, nullptr, FunctionGroups::FermiDiracFunction},
		{[]() { return i18n("Complete Fermi-Dirac integral with index 2"); }, "F2", gsl_sf_fermi_dirac_2, 1, nullptr, FunctionGroups::FermiDiracFunction},
		{[]() { return i18n("Complete Fermi-Dirac integral with integer index j"); }, "Fj", nsl_sf_fermi_dirac_int, 2, nullptr, FunctionGroups::FermiDiracFunction},
		{[]() { return i18n("Complete Fermi-Dirac integral with index -1/2"); }, "Fmhalf", gsl_sf_fermi_dirac_mhalf, 1, nullptr, FunctionGroups::FermiDiracFunction},
		{[]() { return i18n("Complete Fermi-Dirac integral with index 1/2"); }, "Fhalf", gsl_sf_fermi_dirac_half, 1, nullptr, FunctionGroups::FermiDiracFunction},
		{[]() { return i18n("Complete Fermi-Dirac integral with index 3/2"); }, "F3half", gsl_sf_fermi_dirac_3half, 1, nullptr, FunctionGroups::FermiDiracFunction},
		{[]() { return i18n("Incomplete Fermi-Dirac integral with index zero"); }, "Finc0", gsl_sf_fermi_dirac_inc_0, 2, nullptr, FunctionGroups::FermiDiracFunction},

		// Gamma and Beta Functions
		{[]() { return i18n("Gamma function"); }, "gamma", gsl_sf_gamma, 1, nullptr, FunctionGroups::GammaAndBetaFunctions},
		{[]() { return i18n("Gamma function"); }, "tgamma", gsl_sf_gamma, 1, nullptr, FunctionGroups::GammaAndBetaFunctions},
		{[]() { return i18n("Logarithm of the gamma function"); }, "lgamma", gsl_sf_lngamma, 1, nullptr, FunctionGroups::GammaAndBetaFunctions},
		{[]() { return i18n("Logarithm naturalis of the gamma function"); }, "lngamma", gsl_sf_lngamma, 1, nullptr, FunctionGroups::GammaAndBetaFunctions},
		{[]() { return i18n("Regulated gamma function"); }, "gammastar", gsl_sf_gammastar, 1, nullptr, FunctionGroups::GammaAndBetaFunctions},
		{[]() { return i18n("Reciprocal of the gamma function"); }, "gammainv", gsl_sf_gammainv, 1, nullptr, FunctionGroups::GammaAndBetaFunctions},
		{[]() { return i18n("Factorial n!"); }, "fact", nsl_sf_fact, 1, nullptr, FunctionGroups::GammaAndBetaFunctions},
		{[]() { return i18n("Double factorial n!!"); }, "doublefact", nsl_sf_doublefact, 1, nullptr, FunctionGroups::GammaAndBetaFunctions},
		{[]() { return i18n("Logarithm of the factorial"); }, "lnfact", nsl_sf_lnfact, 1, nullptr, FunctionGroups::GammaAndBetaFunctions},
		{[]() { return i18n("Logarithm of the double factorial"); }, "lndoublefact", nsl_sf_lndoublefact, 1, nullptr, FunctionGroups::GammaAndBetaFunctions},

		{[]() { return i18n("Combinatorial factor"); }, "choose", nsl_sf_choose, 2, nullptr, FunctionGroups::GammaAndBetaFunctions},
		{[]() { return i18n("Logarithm of the combinatorial factor"); }, "lnchoose", nsl_sf_lnchoose, 2, nullptr, FunctionGroups::GammaAndBetaFunctions},
		{[]() { return i18n("Taylor coefficient"); }, "taylor", nsl_sf_taylorcoeff, 2, nullptr, FunctionGroups::GammaAndBetaFunctions},
		{[]() { return i18n("Pochhammer symbol"); }, "poch", gsl_sf_poch, 2, nullptr, FunctionGroups::GammaAndBetaFunctions},
		{[]() { return i18n("Logarithm of the Pochhammer symbol"); }, "lnpoch", gsl_sf_lnpoch, 2, nullptr, FunctionGroups::GammaAndBetaFunctions},
		{[]() { return i18n("Relative Pochhammer symbol"); }, "pochrel", gsl_sf_pochrel, 2, nullptr, FunctionGroups::GammaAndBetaFunctions},
		{[]() { return i18n("Unnormalized incomplete gamma function"); }, "gammainc", gsl_sf_gamma_inc, 2, nullptr, FunctionGroups::GammaAndBetaFunctions},
		{[]() { return i18n("Normalized incomplete gamma function"); }, "gammaincQ", gsl_sf_gamma_inc_Q, 2, nullptr, FunctionGroups::GammaAndBetaFunctions},
		{[]() { return i18n("Complementary normalized incomplete gamma function"); }, "gammaincP", gsl_sf_gamma_inc_P, 2, nullptr, FunctionGroups::GammaAndBetaFunctions},
		{[]() { return i18n("Beta function"); }, "beta", gsl_sf_beta, 2, nullptr, FunctionGroups::GammaAndBetaFunctions},

		{[]() { return i18n("Logarithm of the beta function"); }, "lnbeta", gsl_sf_lnbeta, 2, nullptr, FunctionGroups::GammaAndBetaFunctions},
		{[]() { return i18n("Normalized incomplete beta function"); }, "betainc", gsl_sf_beta_inc, 3, nullptr, FunctionGroups::GammaAndBetaFunctions},

		// Gegenbauer Functions
		{[]() { return i18n("Gegenbauer polynomial C_1"); }, "C1", gsl_sf_gegenpoly_1, 2, nullptr, FunctionGroups::GegenbauerFunctions},
		{[]() { return i18n("Gegenbauer polynomial C_2"); }, "C2", gsl_sf_gegenpoly_2, 2, nullptr, FunctionGroups::GegenbauerFunctions},
		{[]() { return i18n("Gegenbauer polynomial C_3"); }, "C3", gsl_sf_gegenpoly_3, 2, nullptr, FunctionGroups::GegenbauerFunctions},
		{[]() { return i18n("Gegenbauer polynomial C_n"); }, "Cn", nsl_sf_gegenpoly_n, 3, nullptr, FunctionGroups::GegenbauerFunctions},

	#if (GSL_MAJOR_VERSION > 2) || (GSL_MAJOR_VERSION == 2) && (GSL_MINOR_VERSION >= 4)
		// Hermite Polynomials and Functions
		{[]() { return i18n("Hermite polynomials physicists version"); }, "Hn", nsl_sf_hermite, 2, nullptr, FunctionGroups::HermitePolynomialsAndFunctions},
		{[]() { return i18n("Hermite polynomials probabilists version"); }, "Hen", nsl_sf_hermite_prob, 2, nullptr, FunctionGroups::HermitePolynomialsAndFunctions},
		{[]() { return i18n("Hermite functions"); }, "Hfn", nsl_sf_hermite_func, 2, nullptr, FunctionGroups::HermitePolynomialsAndFunctions},
	#if (GSL_MAJOR_VERSION > 2) || (GSL_MAJOR_VERSION == 2) && (GSL_MINOR_VERSION >= 6)
		{[]() { return i18n("Hermite functions (fast version)"); }, "Hfnf", nsl_sf_hermite_func_fast, 2, nullptr, FunctionGroups::HermitePolynomialsAndFunctions},
	#endif
		{[]() { return i18n("Derivatives of Hermite polynomials physicists version"); }, "Hnd", nsl_sf_hermite_deriv, 3, nullptr, FunctionGroups::HermitePolynomialsAndFunctions},
		{[]() { return i18n("Derivatives of Hermite polynomials probabilists version"); }, "Hend", nsl_sf_hermite_prob_deriv, 3, nullptr, FunctionGroups::HermitePolynomialsAndFunctions},
		{[]() { return i18n("Derivatives of Hermite functions"); }, "Hfnd", nsl_sf_hermite_func_der, 3, nullptr, FunctionGroups::HermitePolynomialsAndFunctions},
	#endif

		// Hypergeometric Functions
		{[]() { return i18n("Hypergeometric function 0F1"); }, "hyperg_0F1", gsl_sf_hyperg_0F1, 2, nullptr, FunctionGroups::HypergeometricFunctions},
		{[]() { return i18n("Confluent hypergeometric function 1F1 for integer parameters"); }, "hyperg_1F1i", nsl_sf_hyperg_1F1i, 3, nullptr, FunctionGroups::HypergeometricFunctions},
		{[]() { return i18n("Confluent hypergeometric function 1F1 for general parameters"); }, "hyperg_1F1", gsl_sf_hyperg_1F1, 3, nullptr, FunctionGroups::HypergeometricFunctions},
		{[]() { return i18n("Confluent hypergeometric function U for integer parameters"); }, "hyperg_Ui", nsl_sf_hyperg_Ui, 3, nullptr, FunctionGroups::HypergeometricFunctions},
		{[]() { return i18n("Confluent hypergeometric function U"); }, "hyperg_U", gsl_sf_hyperg_U, 3, nullptr, FunctionGroups::HypergeometricFunctions},
		{[]() { return i18n("Gauss hypergeometric function 2F1"); }, "hyperg_2F1", gsl_sf_hyperg_2F1, 4, nullptr, FunctionGroups::HypergeometricFunctions},
		{[]() { return i18n("Gauss hypergeometric function 2F1 with complex parameters"); }, "hyperg_2F1c", gsl_sf_hyperg_2F1_conj, 4, nullptr, FunctionGroups::HypergeometricFunctions},
		{[]() { return i18n("Renormalized Gauss hypergeometric function 2F1"); }, "hyperg_2F1r", gsl_sf_hyperg_2F1_renorm, 4, nullptr, FunctionGroups::HypergeometricFunctions},
		{[]() { return i18n("Renormalized Gauss hypergeometric function 2F1 with complex parameters"); }, "hyperg_2F1cr", gsl_sf_hyperg_2F1_conj_renorm, 4, nullptr, FunctionGroups::HypergeometricFunctions},
		{[]() { return i18n("Hypergeometric function 2F0"); }, "hyperg_2F0", gsl_sf_hyperg_2F0, 3, nullptr, FunctionGroups::HypergeometricFunctions},

		// Laguerre Functions
		{[]() { return i18n("generalized Laguerre polynomials L_1"); }, "L1", gsl_sf_laguerre_1, 2, nullptr, FunctionGroups::LaguerreFunctions},
		{[]() { return i18n("generalized Laguerre polynomials L_2"); }, "L2", gsl_sf_laguerre_2, 2, nullptr, FunctionGroups::LaguerreFunctions},
		{[]() { return i18n("generalized Laguerre polynomials L_3"); }, "L3", gsl_sf_laguerre_3, 2, nullptr, FunctionGroups::LaguerreFunctions},
		{[]() { return i18n("generalized Laguerre polynomials L_n"); }, "Ln", nsl_sf_laguerre_n, 3, nullptr, FunctionGroups::LaguerreFunctions},

		// Lambert W Functions
		{[]() { return i18n("Principal branch of the Lambert W function"); }, "W0", gsl_sf_lambert_W0, 1, nullptr, FunctionGroups::LambertWFunctions},
		{[]() { return i18n("Secondary real-valued branch of the Lambert W function"); }, "Wm1", gsl_sf_lambert_Wm1, 1, nullptr, FunctionGroups::LambertWFunctions},

		// Legendre Functions and Spherical Harmonics
		{[]() { return i18n("Legendre polynomial P_1"); }, "P1", gsl_sf_legendre_P1, 1, nullptr, FunctionGroups::LegendreFunctionsAndSphericalHarmonics},
		{[]() { return i18n("Legendre polynomial P_2"); }, "P2", gsl_sf_legendre_P2, 1, nullptr, FunctionGroups::LegendreFunctionsAndSphericalHarmonics},
		{[]() { return i18n("Legendre polynomial P_3"); }, "P3", gsl_sf_legendre_P3, 1, nullptr, FunctionGroups::LegendreFunctionsAndSphericalHarmonics},
		{[]() { return i18n("Legendre polynomial P_l"); }, "Pl", nsl_sf_legendre_Pl, 2, nullptr, FunctionGroups::LegendreFunctionsAndSphericalHarmonics},
		{[]() { return i18n("Legendre function Q_0"); }, "Q0", gsl_sf_legendre_Q0, 1, nullptr, FunctionGroups::LegendreFunctionsAndSphericalHarmonics},
		{[]() { return i18n("Legendre function Q_1"); }, "Q1", gsl_sf_legendre_Q1, 1, nullptr, FunctionGroups::LegendreFunctionsAndSphericalHarmonics},
		{[]() { return i18n("Legendre function Q_l"); }, "Ql", nsl_sf_legendre_Ql, 2, nullptr, FunctionGroups::LegendreFunctionsAndSphericalHarmonics},
		{[]() { return i18n("Associated Legendre polynomial"); }, "Plm", nsl_sf_legendre_Plm, 3, nullptr, FunctionGroups::LegendreFunctionsAndSphericalHarmonics},
		{[]() { return i18n("Normalized associated Legendre polynomial"); }, "Pslm", nsl_sf_legendre_sphPlm, 3, nullptr, FunctionGroups::LegendreFunctionsAndSphericalHarmonics},
		{[]() { return i18n("Irregular spherical conical function P^1/2"); }, "Phalf", gsl_sf_conicalP_half, 2, nullptr, FunctionGroups::LegendreFunctionsAndSphericalHarmonics},

		{[]() { return i18n("Regular spherical conical function P^(-1/2)"); }, "Pmhalf", gsl_sf_conicalP_mhalf, 2, nullptr, FunctionGroups::LegendreFunctionsAndSphericalHarmonics},
		{[]() { return i18n("Conical function P^0"); }, "Pc0", gsl_sf_conicalP_0, 2, nullptr, FunctionGroups::LegendreFunctionsAndSphericalHarmonics},
		{[]() { return i18n("Conical function P^1"); }, "Pc1", gsl_sf_conicalP_1, 2, nullptr, FunctionGroups::LegendreFunctionsAndSphericalHarmonics},
		{[]() { return i18n("Regular spherical conical function P^(-1/2-l)"); }, "Psr", nsl_sf_conicalP_sphreg, 3, nullptr, FunctionGroups::LegendreFunctionsAndSphericalHarmonics},
		{[]() { return i18n("Regular cylindrical conical function P^(-m)"); }, "Pcr", nsl_sf_conicalP_cylreg, 3, nullptr, FunctionGroups::LegendreFunctionsAndSphericalHarmonics},
		{[]() { return i18n("Zeroth radial eigenfunction of the Laplacian on the 3-dimensional hyperbolic space"); }, "H3d0", gsl_sf_legendre_H3d_0, 2, nullptr, FunctionGroups::LegendreFunctionsAndSphericalHarmonics},
		{[]() { return i18n("First radial eigenfunction of the Laplacian on the 3-dimensional hyperbolic space"); }, "H3d1", gsl_sf_legendre_H3d_1, 2, nullptr, FunctionGroups::LegendreFunctionsAndSphericalHarmonics},
		{[]() { return i18n("l-th radial eigenfunction of the Laplacian on the 3-dimensional hyperbolic space"); }, "H3d", nsl_sf_legendre_H3d, 3, nullptr, FunctionGroups::LegendreFunctionsAndSphericalHarmonics},

		// Logarithm and Related Functions
		{[]() { return i18n("Logarithm"); }, "log", gsl_sf_log, 1, nullptr, FunctionGroups::LogarithmAndRelatedFunctions},
		{[]() { return i18n("Base 2 logarithm"); }, "log2", static_cast<double (*)(double)>(&log2), 1, nullptr, FunctionGroups::LogarithmAndRelatedFunctions},
		{[]() { return i18n("Logarithm of the magnitude"); }, "logabs", gsl_sf_log_abs, 1, nullptr, FunctionGroups::LogarithmAndRelatedFunctions},
		{[]() { return QStringLiteral("log(1+x)"); }, "logp", gsl_sf_log_1plusx, 1, nullptr, FunctionGroups::LogarithmAndRelatedFunctions},
		{[]() { return QStringLiteral("log(1+x) - x"); }, "logpm", gsl_sf_log_1plusx_mx, 1, nullptr, FunctionGroups::LogarithmAndRelatedFunctions},

	#if (GSL_MAJOR_VERSION >= 2)
		// Mathieu Functions
		{[]() { return i18n("Characteristic values a_n(q) of the Mathieu functions ce_n(q, x)"); }, "an", nsl_sf_mathieu_a, 2, nullptr, FunctionGroups::MathieuFunctions},
		{[]() { return i18n("Characteristic values b_n(q) of the Mathieu functions se_n(q, x)"); }, "bn", nsl_sf_mathieu_b, 2, nullptr, FunctionGroups::MathieuFunctions},
		{[]() { return i18n("Angular Mathieu functions ce_n(q, x)"); }, "ce", nsl_sf_mathieu_ce, 3, nullptr, FunctionGroups::MathieuFunctions},
		{[]() { return i18n("Angular Mathieu functions se_n(q, x)"); }, "se", nsl_sf_mathieu_se, 3, nullptr, FunctionGroups::MathieuFunctions},
		{[]() { return i18n("Radial j-th kind Mathieu functions Mc_n^{(j)}(q, x)"); }, "Mc", nsl_sf_mathieu_Mc, 4, nsl_sf_mathieuParameterNames, FunctionGroups::MathieuFunctions},
		{[]() { return i18n("Radial j-th kind Mathieu functions Ms_n^{(j)}(q, x)"); }, "Ms", nsl_sf_mathieu_Ms, 4, nsl_sf_mathieuParameterNames, FunctionGroups::MathieuFunctions},
	#endif

		// Power Function
		{[]() { return i18n("x^n for integer n with an error estimate"); }, "gsl_powint", nsl_sf_powint, 2, nullptr, FunctionGroups::PowerFunction},

		// Psi (Digamma) Function
		{[]() { return i18n("Digamma function for positive integer n"); }, "psiint", nsl_sf_psiint, 1, nullptr, FunctionGroups::PsiDigammaFunction},
		{[]() { return i18n("Digamma function"); }, "psi", gsl_sf_psi, 1, nullptr, FunctionGroups::PsiDigammaFunction},
		{[]() { return i18n("Real part of the digamma function on the line 1+i y"); }, "psi1piy", gsl_sf_psi_1piy, 1, nullptr, FunctionGroups::PsiDigammaFunction},
		{[]() { return i18n("Trigamma function psi' for positive integer n"); }, "psi1int", nsl_sf_psi1int, 1, nullptr, FunctionGroups::PsiDigammaFunction},
		{[]() { return i18n("Trigamma function psi'"); }, "psi1", gsl_sf_psi_1, 1, nullptr, FunctionGroups::PsiDigammaFunction},
		{[]() { return i18n("Polygamma function psi^(n)"); }, "psin", nsl_sf_psin, 2, nullptr, FunctionGroups::PsiDigammaFunction},

		// Synchrotron Functions
		{[]() { return i18n("First synchrotron function"); }, "synchrotron1", gsl_sf_synchrotron_1, 1, nullptr, FunctionGroups::SynchrotronFunctions},
		{[]() { return i18n("Second synchrotron function"); }, "synchrotron2", gsl_sf_synchrotron_2, 1, nullptr, FunctionGroups::SynchrotronFunctions},

		// Transport Functions
		{[]() { return i18n("Transport function J2"); }, "J2", gsl_sf_transport_2, 1, nullptr, FunctionGroups::TransportFunctions},
		{[]() { return i18n("Transport function J3"); }, "J3", gsl_sf_transport_3, 1, nullptr, FunctionGroups::TransportFunctions},
		{[]() { return i18n("Transport function J4"); }, "J4", gsl_sf_transport_4, 1, nullptr, FunctionGroups::TransportFunctions},
		{[]() { return i18n("Transport function J5"); }, "J5", gsl_sf_transport_5, 1, nullptr, FunctionGroups::TransportFunctions},

		// Trigonometric Functions
		{[]() { return i18n("Sine"); }, "sin", gsl_sf_sin, 1, nullptr, FunctionGroups::TrigonometricFunctions},
		{[]() { return i18n("Cosine"); }, "cos", gsl_sf_cos, 1, nullptr, FunctionGroups::TrigonometricFunctions},
		{[]() { return i18n("Tangent"); }, "tan", static_cast<double (*)(double)>(&tan), 1, nullptr, FunctionGroups::TrigonometricFunctions},
		{[]() { return i18n("Inverse sine"); }, "asin", static_cast<double (*)(double)>(&asin), 1, nullptr, FunctionGroups::TrigonometricFunctions},
		{[]() { return i18n("Inverse cosine"); }, "acos", static_cast<double (*)(double)>(&acos), 1, nullptr, FunctionGroups::TrigonometricFunctions},
		{[]() { return i18n("Inverse tangent"); }, "atan", static_cast<double (*)(double)>(&atan), 1, nullptr, FunctionGroups::TrigonometricFunctions},
		{[]() { return i18n("Inverse tangent using sign"); }, "atan2", static_cast<double (*)(double, double)>(&atan2), 2, nullptr, FunctionGroups::TrigonometricFunctions},
		{[]() { return i18n("Hyperbolic sine"); }, "sinh", static_cast<double (*)(double)>(&sinh), 1, nullptr, FunctionGroups::TrigonometricFunctions},
		{[]() { return i18n("Hyperbolic cosine"); }, "cosh", static_cast<double (*)(double)>(&cosh), 1, nullptr, FunctionGroups::TrigonometricFunctions},
		{[]() { return i18n("Hyperbolic tangent"); }, "tanh", static_cast<double (*)(double)>(&tanh), 1, nullptr, FunctionGroups::TrigonometricFunctions},
		{[]() { return i18n("Inverse hyperbolic cosine"); }, "acosh", gsl_acosh, 1, nullptr, FunctionGroups::TrigonometricFunctions},
		{[]() { return i18n("Inverse hyperbolic sine"); }, "asinh", gsl_asinh, 1, nullptr, FunctionGroups::TrigonometricFunctions},
		{[]() { return i18n("Inverse hyperbolic tangent"); }, "atanh", gsl_atanh, 1, nullptr, FunctionGroups::TrigonometricFunctions},
		{[]() { return i18n("Secant"); }, "sec", nsl_sf_sec, 1, nullptr, FunctionGroups::TrigonometricFunctions},
		{[]() { return i18n("Cosecant"); }, "csc", nsl_sf_csc, 1, nullptr, FunctionGroups::TrigonometricFunctions},
		{[]() { return i18n("Cotangent"); }, "cot", nsl_sf_cot, 1, nullptr, FunctionGroups::TrigonometricFunctions},
		{[]() { return i18n("Inverse secant"); }, "asec", nsl_sf_asec, 1, nullptr, FunctionGroups::TrigonometricFunctions},
		{[]() { return i18n("Inverse cosecant"); }, "acsc", nsl_sf_acsc, 1, nullptr, FunctionGroups::TrigonometricFunctions},
		{[]() { return i18n("Inverse cotangent"); }, "acot", nsl_sf_acot, 1, nullptr, FunctionGroups::TrigonometricFunctions},
		{[]() { return i18n("Hyperbolic secant"); }, "sech", nsl_sf_sech, 1, nullptr, FunctionGroups::TrigonometricFunctions},
		{[]() { return i18n("Hyperbolic cosecant"); }, "csch", nsl_sf_csch, 1, nullptr, FunctionGroups::TrigonometricFunctions},
		{[]() { return i18n("Hyperbolic cotangent"); }, "coth", nsl_sf_coth, 1, nullptr, FunctionGroups::TrigonometricFunctions},
		{[]() { return i18n("Inverse hyperbolic secant"); }, "asech", nsl_sf_asech, 1, nullptr, FunctionGroups::TrigonometricFunctions},
		{[]() { return i18n("Inverse hyperbolic cosecant"); }, "acsch", nsl_sf_acsch, 1, nullptr, FunctionGroups::TrigonometricFunctions},
		{[]() { return i18n("Inverse hyperbolic cotangent"); }, "acoth", nsl_sf_acoth, 1, nullptr, FunctionGroups::TrigonometricFunctions},
		{[]() { return i18n("Sinc function sin(x)/x"); }, "sinc", gsl_sf_sinc, 1, nullptr, FunctionGroups::TrigonometricFunctions},
		{[]() { return QStringLiteral("log(sinh(x))"); }, "logsinh", gsl_sf_lnsinh, 1, nullptr, FunctionGroups::TrigonometricFunctions},
		{[]() { return QStringLiteral("log(cosh(x))"); }, "logcosh", gsl_sf_lncosh, 1, nullptr, FunctionGroups::TrigonometricFunctions},
		{[]() { return i18n("Hypotenuse function"); }, "hypot", gsl_sf_hypot, 2, nullptr, FunctionGroups::TrigonometricFunctions},
		{[]() { return i18n("Three component hypotenuse function"); }, "hypot3", gsl_hypot3, 3, nullptr, FunctionGroups::TrigonometricFunctions},
		{[]() { return i18n("restrict to [-pi, pi]"); }, "anglesymm", gsl_sf_angle_restrict_symm, 1, nullptr, FunctionGroups::TrigonometricFunctions},
		{[]() { return i18n("restrict to [0, 2 pi]"); }, "anglepos", gsl_sf_angle_restrict_pos, 1, nullptr, FunctionGroups::TrigonometricFunctions},

		// Zeta Functions
		{[]() { return i18n("Riemann zeta function for integer n"); }, "zetaint", nsl_sf_zetaint, 1, nullptr, FunctionGroups::ZetaFunctions},
		{[]() { return i18n("Riemann zeta function"); }, "zeta", gsl_sf_zeta, 1, nullptr, FunctionGroups::ZetaFunctions},
		{[]() { return i18n("zeta(n)-1 for integer n"); }, "zetam1int", nsl_sf_zetam1int, 1, nullptr, FunctionGroups::ZetaFunctions},
		{[]() { return i18n("zeta(x)-1"); }, "zetam1", gsl_sf_zetam1, 1, nullptr, FunctionGroups::ZetaFunctions},
		{[]() { return i18n("Hurwitz zeta function"); }, "hzeta", gsl_sf_hzeta, 2, nullptr, FunctionGroups::ZetaFunctions},
		{[]() { return i18n("Eta function for integer n"); }, "etaint", nsl_sf_etaint, 1, nullptr, FunctionGroups::ZetaFunctions},
		{[]() { return i18n("Eta function"); }, "eta", gsl_sf_eta, 1, nullptr, FunctionGroups::ZetaFunctions},

		// Random number generator
		// GSL Random Number Generators: see https://www.gnu.org/software/gsl/doc/html/randist.html
		{[]() { return i18n("Gaussian random numbers"); }, "randgaussian", nsl_sf_ran_gaussian, 1, nullptr, FunctionGroups::RandomNumberGenerator},
		{[]() { return i18n("Exponential random numbers"); }, "randexponential", nsl_sf_ran_exponential, 1, nullptr, FunctionGroups::RandomNumberGenerator},
		{[]() { return i18n("Laplacian random numbers"); }, "randlaplace", nsl_sf_ran_laplace, 1, nullptr, FunctionGroups::RandomNumberGenerator},
		{[]() { return i18n("Cauchy/Lorentz random numbers"); }, "randcauchy", nsl_sf_ran_cauchy, 1, nullptr, FunctionGroups::RandomNumberGenerator},
		{[]() { return i18n("Rayleigh random numbers"); }, "randrayleigh", nsl_sf_ran_rayleigh, 1, nullptr, FunctionGroups::RandomNumberGenerator},
		{[]() { return i18n("Landau random numbers"); }, "randlandau", nsl_sf_ran_landau, 0, nullptr, FunctionGroups::RandomNumberGenerator},
		{[]() { return i18n("Levy alpha-stable random numbers"); }, "randlevy", nsl_sf_ran_levy, 2, nullptr, FunctionGroups::RandomNumberGenerator},
		{[]() { return i18n("Gamma random numbers"); }, "randgamma", nsl_sf_ran_gamma, 2, nullptr, FunctionGroups::RandomNumberGenerator},
		{[]() { return i18n("Flat random numbers"); }, "randflat", nsl_sf_ran_flat, 2, nullptr, FunctionGroups::RandomNumberGenerator},
		{[]() { return i18n("Lognormal random numbers"); }, "randlognormal", nsl_sf_ran_lognormal, 2, nullptr, FunctionGroups::RandomNumberGenerator},

		{[]() { return i18n("Chi-squared random numbers"); }, "randchisq", nsl_sf_ran_chisq, 1, nullptr, FunctionGroups::RandomNumberGenerator},
		{[]() { return i18n("t-distributed random numbers"); }, "randtdist", nsl_sf_ran_tdist, 1, nullptr, FunctionGroups::RandomNumberGenerator},
		{[]() { return i18n("Logistic random numbers"); }, "randlogistic", nsl_sf_ran_logistic, 1, nullptr, FunctionGroups::RandomNumberGenerator},
		{[]() { return i18n("Poisson random numbers"); }, "randpoisson", nsl_sf_ran_poisson, 1, nullptr, FunctionGroups::RandomNumberGenerator},
		{[]() { return i18n("Bernoulli random numbers"); }, "randbernoulli", nsl_sf_ran_bernoulli, 1, nullptr, FunctionGroups::RandomNumberGenerator},
		{[]() { return i18n("Binomial random numbers"); }, "randbinomial", nsl_sf_ran_binomial, 2, nullptr, FunctionGroups::RandomNumberGenerator},

		// NSL Random Number Generators
		{[]() { return i18n("Triangular random numbers"); }, "randtriangular", nsl_sf_ran_triangular, 3, nullptr, FunctionGroups::RandomNumberGenerator},

		// GSL Random Number Distributions: see https://www.gnu.org/software/gsl/doc/html/randist.html
		// Gaussian Distribution
		{[]() { return i18n("Probability density for a Gaussian distribution"); }, "gaussian", gsl_ran_gaussian_pdf, 2, nullptr, FunctionGroups::GaussianDistribution},
		{[]() { return i18n("Probability density for a unit Gaussian distribution"); }, "ugaussian", gsl_ran_ugaussian_pdf, 1, nullptr, FunctionGroups::GaussianDistribution},
		{[]() { return i18n("Cumulative distribution function P"); }, "gaussianP", gsl_cdf_gaussian_P, 2, nullptr, FunctionGroups::GaussianDistribution},
		{[]() { return i18n("Cumulative distribution function Q"); }, "gaussianQ", gsl_cdf_gaussian_Q, 2, nullptr, FunctionGroups::GaussianDistribution},
		{[]() { return i18n("Inverse cumulative distribution function P"); }, "gaussianPinv", gsl_cdf_gaussian_Pinv, 2, nullptr, FunctionGroups::GaussianDistribution},
		{[]() { return i18n("Inverse cumulative distribution function Q"); }, "gaussianQinv", gsl_cdf_gaussian_Qinv, 2, nullptr, FunctionGroups::GaussianDistribution},
		{[]() { return i18n("Cumulative unit distribution function P"); }, "ugaussianP", gsl_cdf_ugaussian_P, 1, nullptr, FunctionGroups::GaussianDistribution},
		{[]() { return i18n("Cumulative unit distribution function Q"); }, "ugaussianQ", gsl_cdf_ugaussian_Q, 1, nullptr, FunctionGroups::GaussianDistribution},
		{[]() { return i18n("Inverse cumulative unit distribution function P"); }, "ugaussianPinv", gsl_cdf_ugaussian_Pinv, 1, nullptr, FunctionGroups::GaussianDistribution},
		{[]() { return i18n("Inverse cumulative unit distribution function Q"); }, "ugaussianQinv", gsl_cdf_ugaussian_Qinv, 1, nullptr, FunctionGroups::GaussianDistribution},

		{[]() { return i18n("Probability density for Gaussian tail distribution"); }, "gaussiantail", gsl_ran_gaussian_tail_pdf, 3, nullptr, FunctionGroups::GaussianDistribution},
		{[]() { return i18n("Probability density for unit Gaussian tail distribution"); }, "ugaussiantail", gsl_ran_ugaussian_tail_pdf, 2, nullptr, FunctionGroups::GaussianDistribution},
		{[]() { return i18n("Probability density for a bivariate Gaussian distribution"); }, "gaussianbi", gsl_ran_bivariate_gaussian_pdf, 5, nullptr, FunctionGroups::GaussianDistribution},

		// Exponential Distribution
		{[]() { return i18n("Probability density for an exponential distribution"); }, "exponential", gsl_ran_exponential_pdf, 2, nullptr, FunctionGroups::ExponentialDistribution},
		{[]() { return i18n("Cumulative exponential distribution function P"); }, "exponentialP", gsl_cdf_exponential_P, 2, nullptr, FunctionGroups::ExponentialDistribution},
		{[]() { return i18n("Cumulative exponential distribution function Q"); }, "exponentialQ", gsl_cdf_exponential_Q, 2, nullptr, FunctionGroups::ExponentialDistribution},
		{[]() { return i18n("Inverse cumulative exponential distribution function P"); }, "exponentialPinv", gsl_cdf_exponential_Pinv, 2, nullptr, FunctionGroups::ExponentialDistribution},
		{[]() { return i18n("Inverse cumulative exponential distribution function Q"); }, "exponentialQinv", gsl_cdf_exponential_Qinv, 2, nullptr, FunctionGroups::ExponentialDistribution},

		// Laplace Distribution
		{[]() { return i18n("Probability density for a Laplace distribution"); }, "laplace", gsl_ran_laplace_pdf, 2, nullptr, FunctionGroups::LaplaceDistribution},
		{[]() { return i18n("Cumulative Laplace distribution function P"); }, "laplaceP", gsl_cdf_laplace_P, 2, nullptr, FunctionGroups::LaplaceDistribution},
		{[]() { return i18n("Cumulative Laplace distribution function Q"); }, "laplaceQ", gsl_cdf_laplace_Q, 2, nullptr, FunctionGroups::LaplaceDistribution},
		{[]() { return i18n("Inverse cumulative Laplace distribution function P"); }, "laplacePinv", gsl_cdf_laplace_Pinv, 2, nullptr, FunctionGroups::LaplaceDistribution},
		{[]() { return i18n("Inverse cumulative Laplace distribution function Q"); }, "laplaceQinv", gsl_cdf_laplace_Qinv, 2, nullptr, FunctionGroups::LaplaceDistribution},

		// Exponential Power Distribution
		{[]() { return i18n("Probability density for an exponential power distribution"); }, "exppow", gsl_ran_exppow_pdf, 3, nullptr, FunctionGroups::ExponentialPowerDistribution},
		{[]() { return i18n("cumulative exponential power distribution function P"); }, "exppowP", gsl_cdf_exppow_P, 3, nullptr, FunctionGroups::ExponentialPowerDistribution},
		{[]() { return i18n("Cumulative exponential power distribution function Q"); }, "exppowQ", gsl_cdf_exppow_Q, 3, nullptr, FunctionGroups::ExponentialPowerDistribution},

		// Cauchy Distribution
		{[]() { return i18n("Probability density for a Cauchy distribution"); }, "cauchy", gsl_ran_cauchy_pdf, 2, nullptr, FunctionGroups::CauchyDistribution},
		{[]() { return i18n("Cumulative Cauchy distribution function P"); }, "cauchyP", gsl_cdf_cauchy_P, 2, nullptr, FunctionGroups::CauchyDistribution},
		{[]() { return i18n("Cumulative Cauchy distribution function Q"); }, "cauchyQ", gsl_cdf_cauchy_Q, 2, nullptr, FunctionGroups::CauchyDistribution},
		{[]() { return i18n("Inverse cumulative Cauchy distribution function P"); }, "cauchyPinv", gsl_cdf_cauchy_Pinv, 2, nullptr, FunctionGroups::CauchyDistribution},
		{[]() { return i18n("Inverse cumulative Cauchy distribution function Q"); }, "cauchyQinv", gsl_cdf_cauchy_Qinv, 2, nullptr, FunctionGroups::CauchyDistribution},

		// Rayleigh Distribution
		{[]() { return i18n("Probability density for a Rayleigh distribution"); }, "rayleigh", gsl_ran_rayleigh_pdf, 2, nullptr, FunctionGroups::RayleighDistribution},
		{[]() { return i18n("Cumulative Rayleigh distribution function P"); }, "rayleighP", gsl_cdf_rayleigh_P, 2, nullptr, FunctionGroups::RayleighDistribution},
		{[]() { return i18n("Cumulative Rayleigh distribution function Q"); }, "rayleighQ", gsl_cdf_rayleigh_Q, 2, nullptr, FunctionGroups::RayleighDistribution},
		{[]() { return i18n("Inverse cumulative Rayleigh distribution function P"); }, "rayleighPinv", gsl_cdf_rayleigh_Pinv, 2, nullptr, FunctionGroups::RayleighDistribution},
		{[]() { return i18n("Inverse cumulative Rayleigh distribution function Q"); }, "rayleighQinv", gsl_cdf_rayleigh_Qinv, 2, nullptr, FunctionGroups::RayleighDistribution},
		{[]() { return i18n("Probability density for a Rayleigh tail distribution"); }, "rayleigh_tail", gsl_ran_rayleigh_tail_pdf, 3, nullptr, FunctionGroups::RayleighDistribution},

		// Landau Distribution
		{[]() { return i18n("Probability density for a Landau distribution"); }, "landau", gsl_ran_landau_pdf, 1, nullptr, FunctionGroups::LandauDistribution},

		// Gamma Distribution
		{[]() { return i18n("Probability density for a gamma distribution"); }, "gammapdf", gsl_ran_gamma_pdf, 3, nullptr, FunctionGroups::GammaDistribution},
		{[]() { return i18n("Cumulative gamma distribution function P"); }, "gammaP", gsl_cdf_gamma_P, 3, nullptr, FunctionGroups::GammaDistribution},
		{[]() { return i18n("Cumulative gamma distribution function Q"); }, "gammaQ", gsl_cdf_gamma_Q, 3, nullptr, FunctionGroups::GammaDistribution},
		{[]() { return i18n("Inverse cumulative gamma distribution function P"); }, "gammaPinv", gsl_cdf_gamma_Pinv, 3, nullptr, FunctionGroups::GammaDistribution},
		{[]() { return i18n("Inverse cumulative gamma distribution function Q"); }, "gammaQinv", gsl_cdf_gamma_Qinv, 3, nullptr, FunctionGroups::GammaDistribution},

		// Flat (Uniform) Distribution
		{[]() { return i18n("Probability density for a uniform distribution"); }, "flat", gsl_ran_flat_pdf, 3, nullptr, FunctionGroups::FlatUniformDistribution},
		{[]() { return i18n("Cumulative uniform distribution function P"); }, "flatP", gsl_cdf_flat_P, 3, nullptr, FunctionGroups::FlatUniformDistribution},
		{[]() { return i18n("Cumulative uniform distribution function Q"); }, "flatQ", gsl_cdf_flat_Q, 3, nullptr, FunctionGroups::FlatUniformDistribution},
		{[]() { return i18n("Inverse cumulative uniform distribution function P"); }, "flatPinv", gsl_cdf_flat_Pinv, 3, nullptr, FunctionGroups::FlatUniformDistribution},
		{[]() { return i18n("Inverse cumulative uniform distribution function Q"); }, "flatQinv", gsl_cdf_flat_Qinv, 3, nullptr, FunctionGroups::FlatUniformDistribution},

		// Lognormal Distribution
		{[]() { return i18n("Probability density for a lognormal distribution"); }, "lognormal", gsl_ran_lognormal_pdf, 3, nullptr, FunctionGroups::LognormalDistribution},
		{[]() { return i18n("Cumulative lognormal distribution function P"); }, "lognormalP", gsl_cdf_lognormal_P, 3, nullptr, FunctionGroups::LognormalDistribution},
		{[]() { return i18n("Cumulative lognormal distribution function Q"); }, "lognormalQ", gsl_cdf_lognormal_Q, 3, nullptr, FunctionGroups::LognormalDistribution},
		{[]() { return i18n("Inverse cumulative lognormal distribution function P"); }, "lognormalPinv", gsl_cdf_lognormal_Pinv, 3, nullptr, FunctionGroups::LognormalDistribution},
		{[]() { return i18n("Inverse cumulative lognormal distribution function Q"); }, "lognormalQinv", gsl_cdf_lognormal_Qinv, 3, nullptr, FunctionGroups::LognormalDistribution},

		// Chi-squared Distribution
		{[]() { return i18n("Probability density for a chi squared distribution"); }, "chisq", gsl_ran_chisq_pdf, 2, nullptr, FunctionGroups::ChisquaredDistribution},
		{[]() { return i18n("Cumulative chi squared distribution function P"); }, "chisqP", gsl_cdf_chisq_P, 2, nullptr, FunctionGroups::ChisquaredDistribution},
		{[]() { return i18n("Cumulative chi squared distribution function Q"); }, "chisqQ", gsl_cdf_chisq_Q, 2, nullptr, FunctionGroups::ChisquaredDistribution},
		{[]() { return i18n("Inverse cumulative chi squared distribution function P"); }, "chisqPinv", gsl_cdf_chisq_Pinv, 2, nullptr, FunctionGroups::ChisquaredDistribution},
		{[]() { return i18n("Inverse cumulative chi squared distribution function Q"); }, "chisqQinv", gsl_cdf_chisq_Qinv, 2, nullptr, FunctionGroups::ChisquaredDistribution},

		// F-distribution
		{[]() { return i18n("Probability density for a F-distribution"); }, "fdist", gsl_ran_fdist_pdf, 3, nullptr, FunctionGroups::Fdistribution},
		{[]() { return i18n("Cumulative F-distribution function P"); }, "fdistP", gsl_cdf_fdist_P, 3, nullptr, FunctionGroups::Fdistribution},
		{[]() { return i18n("Cumulative F-distribution function Q"); }, "fdistQ", gsl_cdf_fdist_Q, 3, nullptr, FunctionGroups::Fdistribution},
		{[]() { return i18n("Inverse cumulative F-distribution function P"); }, "fdistPinv", gsl_cdf_fdist_Pinv, 3, nullptr, FunctionGroups::Fdistribution},
		{[]() { return i18n("Inverse cumulative F-distribution function Q"); }, "fdistQinv", gsl_cdf_fdist_Qinv, 3, nullptr, FunctionGroups::Fdistribution},

		// t-distribution
		{[]() { return i18n("Probability density for a t-distribution"); }, "tdist", gsl_ran_tdist_pdf, 2, nullptr, FunctionGroups::Tdistribution},
		{[]() { return i18n("Cumulative t-distribution function P"); }, "tdistP", gsl_cdf_tdist_P, 2, nullptr, FunctionGroups::Tdistribution},
		{[]() { return i18n("Cumulative t-distribution function Q"); }, "tdistQ", gsl_cdf_tdist_Q, 2, nullptr, FunctionGroups::Tdistribution},
		{[]() { return i18n("Inverse cumulative t-distribution function P"); }, "tdistPinv", gsl_cdf_tdist_Pinv, 2, nullptr, FunctionGroups::Tdistribution},
		{[]() { return i18n("Inverse cumulative t-distribution function Q"); }, "tdistQinv", gsl_cdf_tdist_Qinv, 2, nullptr, FunctionGroups::Tdistribution},

		// Beta Distribution
		{[]() { return i18n("Probability density for a beta distribution"); }, "betapdf", gsl_ran_beta_pdf, 3, nullptr, FunctionGroups::BetaDistribution},
		{[]() { return i18n("Cumulative beta distribution function P"); }, "betaP", gsl_cdf_beta_P, 3, nullptr, FunctionGroups::BetaDistribution},
		{[]() { return i18n("Cumulative beta distribution function Q"); }, "betaQ", gsl_cdf_beta_Q, 3, nullptr, FunctionGroups::BetaDistribution},
		{[]() { return i18n("Inverse cumulative beta distribution function P"); }, "betaPinv", gsl_cdf_beta_Pinv, 3, nullptr, FunctionGroups::BetaDistribution},
		{[]() { return i18n("Inverse cumulative beta distribution function Q"); }, "betaQinv", gsl_cdf_beta_Qinv, 3, nullptr, FunctionGroups::BetaDistribution},

		// Logistic Distribution
		{[]() { return i18n("Probability density for a logistic distribution"); }, "logistic", gsl_ran_logistic_pdf, 2, nullptr, FunctionGroups::LogisticDistribution},
		{[]() { return i18n("Cumulative logistic distribution function P"); }, "logisticP", gsl_cdf_logistic_P, 2, nullptr, FunctionGroups::LogisticDistribution},
		{[]() { return i18n("Cumulative logistic distribution function Q"); }, "logisticQ", gsl_cdf_logistic_Q, 2, nullptr, FunctionGroups::LogisticDistribution},
		{[]() { return i18n("Inverse cumulative logistic distribution function P"); }, "logisticPinv", gsl_cdf_logistic_Pinv, 2, nullptr, FunctionGroups::LogisticDistribution},
		{[]() { return i18n("Inverse cumulative logistic distribution function Q"); }, "logisticQinv", gsl_cdf_logistic_Qinv, 2, nullptr, FunctionGroups::LogisticDistribution},

		// Pareto Distribution
		{[]() { return i18n("Probability density for a Pareto distribution"); }, "pareto", gsl_ran_pareto_pdf, 3, nullptr, FunctionGroups::ParetoDistribution},
		{[]() { return i18n("Cumulative Pareto distribution function P"); }, "paretoP", gsl_cdf_pareto_P, 3, nullptr, FunctionGroups::ParetoDistribution},
		{[]() { return i18n("Cumulative Pareto distribution function Q"); }, "paretoQ", gsl_cdf_pareto_Q, 3, nullptr, FunctionGroups::ParetoDistribution},
		{[]() { return i18n("Inverse cumulative Pareto distribution function P"); }, "paretoPinv", gsl_cdf_pareto_Pinv, 3, nullptr, FunctionGroups::ParetoDistribution},
		{[]() { return i18n("Inverse cumulative Pareto distribution function Q"); }, "paretoQinv", gsl_cdf_pareto_Qinv, 3, nullptr, FunctionGroups::ParetoDistribution},

		// Weibull Distribution
		{[]() { return i18n("Probability density for a Weibull distribution"); }, "weibull", gsl_ran_weibull_pdf, 3, nullptr, FunctionGroups::WeibullDistribution},
		{[]() { return i18n("Cumulative Weibull distribution function P"); }, "weibullP", gsl_cdf_weibull_P, 3, nullptr, FunctionGroups::WeibullDistribution},
		{[]() { return i18n("Cumulative Weibull distribution function Q"); }, "weibullQ", gsl_cdf_weibull_Q, 3, nullptr, FunctionGroups::WeibullDistribution},
		{[]() { return i18n("Inverse cumulative Weibull distribution function P"); }, "weibullPinv", gsl_cdf_weibull_Pinv, 3, nullptr, FunctionGroups::WeibullDistribution},
		{[]() { return i18n("Inverse cumulative Weibull distribution function Q"); }, "weibullQinv", gsl_cdf_weibull_Qinv, 3, nullptr, FunctionGroups::WeibullDistribution},

		// Gumbel Distribution
		{[]() { return i18n("Probability density for a Type-1 Gumbel distribution"); }, "gumbel1", gsl_ran_gumbel1_pdf, 3, nullptr, FunctionGroups::GumbelDistribution},
		{[]() { return i18n("Cumulative Type-1 Gumbel distribution function P"); }, "gumbel1P", gsl_cdf_gumbel1_P, 3, nullptr, FunctionGroups::GumbelDistribution},
		{[]() { return i18n("Cumulative Type-1 Gumbel distribution function Q"); }, "gumbel1Q", gsl_cdf_gumbel1_Q, 3, nullptr, FunctionGroups::GumbelDistribution},
		{[]() { return i18n("Inverse cumulative Type-1 Gumbel distribution function P"); }, "gumbel1Pinv", gsl_cdf_gumbel1_Pinv, 3, nullptr, FunctionGroups::GumbelDistribution},
		{[]() { return i18n("Inverse cumulative Type-1 Gumbel distribution function Q"); }, "gumbel1Qinv", gsl_cdf_gumbel1_Qinv, 3, nullptr, FunctionGroups::GumbelDistribution},
		{[]() { return i18n("Probability density for a Type-2 Gumbel distribution"); }, "gumbel2", gsl_ran_gumbel2_pdf, 3, nullptr, FunctionGroups::GumbelDistribution},
		{[]() { return i18n("Cumulative Type-2 Gumbel distribution function P"); }, "gumbel2P", gsl_cdf_gumbel2_P, 3, nullptr, FunctionGroups::GumbelDistribution},
		{[]() { return i18n("Cumulative Type-2 Gumbel distribution function Q"); }, "gumbel2Q", gsl_cdf_gumbel2_Q, 3, nullptr, FunctionGroups::GumbelDistribution},
		{[]() { return i18n("Inverse cumulative Type-2 Gumbel distribution function P"); }, "gumbel2Pinv", gsl_cdf_gumbel2_Pinv, 3, nullptr, FunctionGroups::GumbelDistribution},
		{[]() { return i18n("Inverse cumulative Type-2 Gumbel distribution function Q"); }, "gumbel2Qinv", gsl_cdf_gumbel2_Qinv, 3, nullptr, FunctionGroups::GumbelDistribution},

		// Poisson Distribution
		{[]() { return i18n("Probability density for a Poisson distribution"); }, "poisson", nsl_sf_poisson, 2, nullptr, FunctionGroups::PoissonDistribution},
		{[]() { return i18n("Cumulative Poisson distribution function P"); }, "poissonP", [](double v1, double v2) { return gsl_cdf_poisson_P(v1, v2); },  2, nullptr, FunctionGroups::PoissonDistribution},
		{[]() { return i18n("Cumulative Poisson distribution function Q"); }, "poissonQ", [](double v1, double v2) { return gsl_cdf_poisson_Q(v1, v2); },  2, nullptr, FunctionGroups::PoissonDistribution},

		// Bernoulli Distribution
		{[]() { return i18n("Probability density for a Bernoulli distribution"); }, "bernoulli", nsl_sf_bernoulli, 2, nullptr, FunctionGroups::BernoulliDistribution},

		// Binomial Distribution
		{[]() { return i18n("Probability density for a binomial distribution"); }, "binomial", nsl_sf_binomial, 3, nullptr, FunctionGroups::BinomialDistribution},
		{[]() { return i18n("Cumulative binomial distribution function P"); }, "binomialP", gsl_cdf_binomial_P, 3, nullptr, FunctionGroups::BinomialDistribution},
		{[]() { return i18n("Cumulative binomial distribution function Q"); }, "binomialQ", gsl_cdf_binomial_Q, 3, nullptr, FunctionGroups::BinomialDistribution},
		{[]() { return i18n("Probability density for a negative binomial distribution"); }, "negative_binomial", nsl_sf_negative_binomial, 3, nullptr, FunctionGroups::BinomialDistribution},
		{[]() { return i18n("Cumulative negative binomial distribution function P"); }, "negative_binomialP", gsl_cdf_negative_binomial_P, 3, nullptr, FunctionGroups::BinomialDistribution},
		{[]() { return i18n("Cumulative negative binomial distribution function Q"); }, "negative_binomialQ", gsl_cdf_negative_binomial_Q, 3, nullptr, FunctionGroups::BinomialDistribution},

		// Pascal Distribution
		{[]() { return i18n("Probability density for a Pascal distribution"); }, "pascal", nsl_sf_pascal, 3, nullptr, FunctionGroups::PascalDistribution},
		{[]() { return i18n("Cumulative Pascal distribution function P"); }, "pascalP", gsl_cdf_pascal_P, 3, nullptr, FunctionGroups::PascalDistribution},
		{[]() { return i18n("Cumulative Pascal distribution function Q"); }, "pascalQ", gsl_cdf_pascal_Q, 3, nullptr, FunctionGroups::PascalDistribution},

		// Geometric Distribution
		{[]() { return i18n("Probability density for a geometric distribution"); }, "geometric", nsl_sf_geometric, 2, nullptr, FunctionGroups::GeometricDistribution},
		{[]() { return i18n("Cumulative geometric distribution function P"); }, "geometricP", gsl_cdf_geometric_P, 2, nullptr, FunctionGroups::GeometricDistribution},
		{[]() { return i18n("Cumulative geometric distribution function Q"); }, "geometricQ", gsl_cdf_geometric_Q, 2, nullptr, FunctionGroups::GeometricDistribution},

		// Hypergeometric Distribution
		{[]() { return i18n("Probability density for a hypergeometric distribution"); }, "hypergeometric", nsl_sf_hypergeometric, 4, nullptr, FunctionGroups::HypergeometricDistribution},
		{[]() { return i18n("Cumulative hypergeometric distribution function P"); }, "hypergeometricP", gsl_cdf_hypergeometric_P, 4, nullptr, FunctionGroups::HypergeometricDistribution},
		{[]() { return i18n("Cumulative hypergeometric distribution function Q"); }, "hypergeometricQ", gsl_cdf_hypergeometric_Q, 4, nullptr, FunctionGroups::HypergeometricDistribution},

		// Logarithmic Distribution
		{[]() { return i18n("Probability density for a logarithmic distribution"); }, "logarithmic", nsl_sf_logarithmic, 2, nullptr, FunctionGroups::LogarithmicDistribution},

		// Non-GSL Distributions
		// Triangular Distributions
		{[]() { return i18n("Probability density for a triangular distribution"); }, "triangular", nsl_sf_triangular, 4, nullptr, FunctionGroups::TriangularDistribution},
		{[]() { return i18n("Cumulative triangular distribution function P"); }, "triangularP", nsl_sf_triangular_P, 4, nullptr, FunctionGroups::TriangularDistribution},
		{[]() { return i18n("Cumulative triangular distribution function Q"); }, "triangularQ", nsl_sf_triangular_Q, 4, nullptr, FunctionGroups::TriangularDistribution},
		{[]() { return i18n("p-th quantile for triangular distribution"); }, "triangularQuantile", nsl_sf_triangular_Quantile, 4, nullptr, FunctionGroups::TriangularDistribution},
	};

// clang-format on

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

double notFunction(const double v) {
	if (convertDoubleToBool(v))
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

double roundn(const double v, const double precision) {
	return nsl_math_round_places(v, static_cast<int>(precision));
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
} // namespace Parsing
