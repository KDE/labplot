/*
	File                 : functions.h
	Project              : LabPlot
	Description          : definition of functions
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2014 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2014-2024 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "parserFunctionTypes.h"
#include <QString>
#include <functional>
#include <gsl/gsl_version.h>
#include <variant>

enum class FunctionGroups;

struct funs {
	std::function<QString(void)> description;
	const char* name;
	std::variant<func_t, func_t1, func_t2, func_t3, func_t4, func_t5, func_tPayload, func_t1Payload, func_t2Payload, func_t3Payload, func_t4Payload> fnct;
	int argc;
	std::function<QString(int)> parameterFunction; // can be also a nullptr. Check needed!
	FunctionGroups group;
};

struct funs0Payload {
	const char* name;
	func_tPayload fnct;
};

struct funs1Payload {
	const char* name;
	func_t1Payload fnct;
};

struct funs2Payload {
	const char* name;
	func_t2Payload fnct;
};

struct funs3Payload {
	const char* name;
	func_t3Payload fnct;
};

struct funs4Payload {
	const char* name;
	func_t4Payload fnct;
};

extern struct funs _functions[];
extern const int _number_functions;
extern struct funs _special_functions[];
extern const int _number_specialfunctions;

enum class FunctionGroups : int {
	StandardMathematicalFunctions,
	ComparisonFunctions,
	LogicalFunctions,
	ColumnStatistics,
	MovingStatistics,
	AiryFunctionsAndDerivatives,
	BesselFunctions,
	ClausenFunctions,
	CoulombFunctions,
	DawsonFunction,
	DebyeFunctions,
	Dilogarithm,
	EllipticIntegrals,
	ErrorFunctions,
	ExponentialFunctions,
	ExponentialIntegrals,
	FermiDiracFunction,
	GammaAndBetaFunctions,
	GegenbauerFunctions,
#if (GSL_MAJOR_VERSION > 2) || (GSL_MAJOR_VERSION == 2) && (GSL_MINOR_VERSION >= 4)
	HermitePolynomialsAndFunctions,
#endif
	HypergeometricFunctions,
	LaguerreFunctions,
	LambertWFunctions,
	LegendreFunctionsAndSphericalHarmonics,
	LogarithmAndRelatedFunctions,
#if (GSL_MAJOR_VERSION >= 2)
	MathieuFunctions,
#endif
	PowerFunction,
	PsiDigammaFunction,
	SynchrotronFunctions,
	TransportFunctions,
	TrigonometricFunctions,
	ZetaFunctions,
	RandomNumberGenerator,
	GaussianDistribution,
	ExponentialDistribution,
	LaplaceDistribution,
	ExponentialPowerDistribution,
	CauchyDistribution,
	RayleighDistribution,
	LandauDistribution,
	GammaDistribution,
	FlatUniformDistribution,
	LognormalDistribution,
	ChisquaredDistribution,
	Fdistribution,
	Tdistribution,
	BetaDistribution,
	LogisticDistribution,
	ParetoDistribution,
	WeibullDistribution,
	GumbelDistribution,
	PoissonDistribution,
	BernoulliDistribution,
	BinomialDistribution,
	PascalDistribution,
	GeometricDistribution,
	HypergeometricDistribution,
	LogarithmicDistribution,
	TriangularDistribution,
	// Not implemented
	// i18n("Coupling Coefficients")
	// i18n("Elementary Operations")
	// i18n("Elliptic Functions (Jacobi)")
	//---------------
	END
};

QString FunctionGroupsToString(FunctionGroups group);

extern const char* colfun_size;
extern const char* colfun_min;
extern const char* colfun_max;
extern const char* colfun_mean;
extern const char* colfun_median;
extern const char* colfun_stdev;
extern const char* colfun_var;
extern const char* colfun_gm;
extern const char* colfun_hm;
extern const char* colfun_chm;
extern const char* colfun_mode;
extern const char* colfun_quartile1;
extern const char* colfun_quartile3;
extern const char* colfun_iqr;
extern const char* colfun_percentile1;
extern const char* colfun_percentile5;
extern const char* colfun_percentile10;
extern const char* colfun_percentile90;
extern const char* colfun_percentile95;
extern const char* colfun_percentile99;
extern const char* colfun_trimean;
extern const char* colfun_meandev;
extern const char* colfun_meandevmedian;
extern const char* colfun_mediandev;
extern const char* colfun_skew;
extern const char* colfun_kurt;
extern const char* colfun_entropy;
extern const char* colfun_quantile;
extern const char* colfun_percentile;

extern const char* specialfun_cell;
extern const char* specialfun_ma;
extern const char* specialfun_mr;
extern const char* specialfun_smmin;
extern const char* specialfun_smmax;
extern const char* specialfun_sma;
extern const char* specialfun_smr;
extern const char* specialfun_psample;
extern const char* specialfun_rsample;

double andFunction(const double v1, const double v2);
double orFunction(const double v1, const double v2);
double notFunction(const double v);
double greaterThan(const double v1, const double v2);
double greaterEqualThan(const double v1, const double v2);
double lessThan(const double v1, const double v2);
double lessEqualThan(const double v1, const double v2);

#endif /*FUNCTIONS_H*/
