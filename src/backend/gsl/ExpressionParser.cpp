/***************************************************************************
    File             : ExpressionParser.cpp
    Project          : LabPlot
    --------------------------------------------------------------------
    Copyright        : (C) 2014 Alexander Semke (alexander.semke@web.de)
    Description      : c++ wrapper for the bison generated parser.

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

#include "backend/gsl/ExpressionParser.h"
#include "backend/gsl/parser_extern.h"
#include "backend/gsl/parser_struct.h"

#include <gsl/gsl_errno.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_const_mksa.h>
#include <gsl/gsl_const_num.h>

ExpressionParser* ExpressionParser::instance = NULL;

ExpressionParser::ExpressionParser(){
	init_table();
	initFunctions();
	initConstants();
}

void ExpressionParser::initFunctions() {
	//functions
	for (int i = 0; _functions[i].name != 0; i++)
		m_functions << _functions[i].name;

	//http://www.gnu.org/software/gsl/manual/html_node/Special-Functions.html
	m_functionsGroups << "Airy Functions and Derivatives";
	m_functionsGroups << "Bessel Functions";
	m_functionsGroups << "Clausen Functions";
	m_functionsGroups << "Coulomb Functions";
// 	m_functionsGroups << "Coupling Coefficients";
	m_functionsGroups << "Dawson Function";
	m_functionsGroups << "Debye Functions";
	m_functionsGroups << "Dilogarithm";
	m_functionsGroups << "Elementary Operations";
	m_functionsGroups << "Elliptic Integrals";
	m_functionsGroups << "Elliptic Functions (Jacobi)";
	m_functionsGroups << "Error Functions";
	m_functionsGroups << "Exponential Functions";
	m_functionsGroups << "Exponential Integrals";
	m_functionsGroups << "Fermi-Dirac Function";
	m_functionsGroups << "Gamma and Beta Functions";
	m_functionsGroups << "Gegenbauer Functions";
	m_functionsGroups << "Hypergeometric Functions";
	m_functionsGroups << "Laguerre Functions";
	m_functionsGroups << "Lambert W Functions";
	m_functionsGroups << "Legendre Functions and Spherical Harmonics";
	m_functionsGroups << "Logarithm and Related Functions";
	m_functionsGroups << "Mathieu Functions";
	m_functionsGroups << "Power Function";
	m_functionsGroups << "Psi (Digamma) Function";
	m_functionsGroups << "Synchrotron Functions";
	m_functionsGroups << "Transport Functions:";
	m_functionsGroups << "Trigonometric Functions";
	m_functionsGroups << "Zeta Functions:";

	//TODO: fill function groups
}

//TODO: decide whether we want to have i18n here in the backend part of the code
void ExpressionParser::initConstants() {
	for (int i = 0; _constants[i].name != 0; i++)
		m_constants << _constants[i].name;

	//groups
	m_constantsGroups << "Mathematical constants";
	m_constantsGroups << "Fundamental constants";
	m_constantsGroups << "Astronomy and Astrophysics";
	m_constantsGroups << "Atomic and Nuclear Physics";
	m_constantsGroups << "Measurement of Time";
	m_constantsGroups << "Imperial Units";
	m_constantsGroups << "Speed and Nautical Units";
	m_constantsGroups << "Printers Units";
	m_constantsGroups << "Volume, Area and Length";
	m_constantsGroups << "Thermal Energy and Power";
	m_constantsGroups << "Pressure";
	m_constantsGroups << "Viscosity";
	m_constantsGroups << "Light and Illumination";
	m_constantsGroups << "Radioactivity";
	m_constantsGroups << "Force and Energy";

	//Mathematical constants
	m_constantsNames << "Euler constant";
	m_constantsValues << QString::number(M_E,'g',15); m_constantsUnits << "";
	m_constantsNames << "Pi";
	m_constantsValues << QString::number(M_PI,'g',15); m_constantsUnits << "";

	for(int i=0;i<2;i++)
		m_constantsGroupIndex << 0;

	//Fundamental constants
	m_constantsNames << "Speed of light";
	m_constantsValues << QString::number(GSL_CONST_MKSA_SPEED_OF_LIGHT); m_constantsUnits << "m / s";
	m_constantsNames << "Vaccuum permeability";
	m_constantsValues << QString::number(GSL_CONST_MKSA_VACUUM_PERMEABILITY); m_constantsUnits << "kg m / A^2 s^2";
	m_constantsNames << "Vaccuum permittivity";
	m_constantsValues << QString::number(GSL_CONST_MKSA_VACUUM_PERMITTIVITY); m_constantsUnits << "A^2 s^4 / kg m^3";
	m_constantsNames << "Plank constant";
	m_constantsValues << QString::number(GSL_CONST_MKSA_PLANCKS_CONSTANT_H); m_constantsUnits << "kg m^2 / s";
	m_constantsNames << "Reduced Plank constant";
	m_constantsValues << QString::number(GSL_CONST_MKSA_PLANCKS_CONSTANT_HBAR); m_constantsUnits << "kg m^2 / s";
	m_constantsNames << "Avogadro constant";
	m_constantsValues << QString::number(GSL_CONST_NUM_AVOGADRO); m_constantsUnits << "1 / mol";
	m_constantsNames << "Faraday";
	m_constantsValues << QString::number(GSL_CONST_MKSA_FARADAY); m_constantsUnits << "A s / mol";
	m_constantsNames << "Boltzman constant";
	m_constantsValues << QString::number(GSL_CONST_MKSA_BOLTZMANN); m_constantsUnits << "kg m^2 / K s^2";
	m_constantsNames << "Molar gas";
	m_constantsValues << QString::number(GSL_CONST_MKSA_MOLAR_GAS); m_constantsUnits << "kg m^2 / K mol s^2";
	m_constantsNames << "Standard gas volume";
	m_constantsValues << QString::number(GSL_CONST_MKSA_STANDARD_GAS_VOLUME); m_constantsUnits << "m^3 / mol";
	m_constantsNames << "Stefan-Boltzman constant";
	m_constantsValues << QString::number(GSL_CONST_MKSA_STEFAN_BOLTZMANN_CONSTANT); m_constantsUnits << "kg / K^4 s^3";
	m_constantsNames << "Gauss";
	m_constantsValues << QString::number(GSL_CONST_MKSA_GAUSS); m_constantsUnits << "kg / A s^2";

	for(int i=0;i<12;i++)
		m_constantsGroupIndex << 1;

	// Astronomy and Astrophysics
	m_constantsNames << "Astronomical unit";
	m_constantsValues << QString::number(GSL_CONST_MKSA_ASTRONOMICAL_UNIT); m_constantsUnits << "m";
	m_constantsNames << "Gravitational constant";
	m_constantsValues << QString::number(GSL_CONST_MKSA_GRAVITATIONAL_CONSTANT); m_constantsUnits << "m^3 / kg s^2";
	m_constantsNames << "Light year";
	m_constantsValues << QString::number(GSL_CONST_MKSA_LIGHT_YEAR); m_constantsUnits << "m";
	m_constantsNames << "Parsec";
	m_constantsValues << QString::number(GSL_CONST_MKSA_PARSEC); m_constantsUnits << "m";
	m_constantsNames << "Gravitational acceleration";
	m_constantsValues << QString::number(GSL_CONST_MKSA_GRAV_ACCEL); m_constantsUnits << "m / s^2";
	m_constantsNames << "Solar mass";
	m_constantsValues << QString::number(GSL_CONST_MKSA_SOLAR_MASS); m_constantsUnits << "kg";

	for(int i=0;i<6;i++)
		m_constantsGroupIndex << 2;

	// Atomic and Nuclear Physics;

	//TODO: complete
}

ExpressionParser::~ExpressionParser(){
	delete_table();
}

ExpressionParser* ExpressionParser::getInstance(){
	if (!instance)
		instance = new ExpressionParser();

	return instance;
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

bool ExpressionParser::isValid(const QString& expr, XYEquationCurve::EquationType type){
	char* data = expr.toLocal8Bit().data();
	if (type == XYEquationCurve::Cartesian) {
		char xVar[] = "x";
		double x = 0;
		assign_variable(xVar,x);
	} else if (type == XYEquationCurve::Polar) {
		char var[] = "phi";
		double phi = 0;
		assign_variable(var,phi);
	} else if (type == XYEquationCurve::Parametric) {
		char var[] = "t";
		double t = 0;
		assign_variable(var,t);
	}

	parse(data);
	return !(parse_errors()>0);
}

bool ExpressionParser::evaluateCartesian(const QString& expr, const QString& min, const QString& max,
										 int count, QVector<double>* xVector, QVector<double>* yVector) {
	double xMin = parse( min.toLocal8Bit().data() );
	double xMax = parse( max.toLocal8Bit().data() );
	double step = (xMax-xMin)/(double)(count-1);
	char* func = expr.toLocal8Bit().data();
// 	printf("fun = %s (%g,%g)\n",func,xMin,xMax);
	double x, y;
	char xVar[] = "x";
	gsl_set_error_handler_off();

	for(int i = 0;i < count; i++) {
		x = xMin + step*i;
		assign_variable(xVar,x);
		y = parse(func);
// 		printf("f(%g)=%g\n",x,y);

		if(parse_errors()>0)
			return false;

		(*xVector)[i] = x;
		if (finite(y))
			(*yVector)[i] = y;
		else
			(*yVector)[i] = NAN;
	}

	return true;
}

bool ExpressionParser::evaluatePolar(const QString& expr, const QString& min, const QString& max,
										 int count, QVector<double>* xVector, QVector<double>* yVector) {
	double minValue = parse( min.toLocal8Bit().data() );
	double maxValue = parse( max.toLocal8Bit().data() );
	double step = (maxValue-minValue)/(double)(count-1);
	char* func = expr.toLocal8Bit().data();
	double r, phi;
	char var[] = "phi";
	gsl_set_error_handler_off();

	for(int i = 0;i < count; i++) {
		phi = minValue + step*i;
		assign_variable(var,phi);
		r = parse(func);
		if(parse_errors()>0)
			return false;

		if (finite(r)) {
			(*xVector)[i] = r*cos(phi);
			(*yVector)[i] = r*sin(phi);
		} else {
			(*xVector)[i] = NAN;
			(*yVector)[i] = NAN;
		}
	}

	return true;
}

bool ExpressionParser::evaluateParametric(const QString& expr1, const QString& expr2, const QString& min, const QString& max,
										 int count, QVector<double>* xVector, QVector<double>* yVector) {
	double minValue = parse( min.toLocal8Bit().data() );
	double maxValue = parse( max.toLocal8Bit().data() );
	double step = (maxValue-minValue)/(double)(count-1);
	char* xFunc = expr1.toLocal8Bit().data();
	char* yFunc = expr2.toLocal8Bit().data();
	double x, y, t;
	char var[] = "t";
	gsl_set_error_handler_off();

	for(int i = 0;i < count; i++) {
		t = minValue + step*i;
		assign_variable(var,t);
		x = parse(xFunc);
		if(parse_errors()>0)
			return false;

		if (finite(x))
			(*xVector)[i] = x;
		else
			(*xVector)[i] = NAN;

		y = parse(yFunc);
		if(parse_errors()>0)
			return false;

		if (finite(y))
			(*yVector)[i] = y;
		else
			(*yVector)[i] = NAN;
	}

	return true;
}
