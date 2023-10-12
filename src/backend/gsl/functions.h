/*
	File                 : functions.h
	Project              : LabPlot
	Description          : definition of functions
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2014 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2014-2021 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <QString>
#include <gsl/gsl_version.h>

enum class FunctionGroups;

struct funs {
	QString description;
	const char* name;
#ifdef _MSC_VER /* MSVC needs void argument */
	double (*fnct)(void);
#else
	double (*fnct)();
#endif
	int argc;
	QString (*parameterFunction)(int); // can be also a nullptr. Check needed!
	FunctionGroups group;
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
	// Not implemented
	// i18n("Coupling Coefficients")
	// i18n("Elementary Operations")
	// i18n("Elliptic Functions (Jacobi)")

	Special,

	//---------------
	END
};

QString FunctionGroupsToString(FunctionGroups group);

#endif /*FUNCTIONS_H*/
