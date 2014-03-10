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

ExpressionParser* ExpressionParser::instance = NULL;

ExpressionParser::ExpressionParser(){
	init_table();

	for (int i = 0; constants[i].name != 0; i++)
		m_constants << constants[i].name;

	for (int i = 0; arith_fncts[i].fname != 0; i++)
		m_functions << arith_fncts[i].fname;
}

ExpressionParser::~ExpressionParser(){
	delete_table();
}

ExpressionParser* ExpressionParser::getInstance(){
	if (!instance)
		instance = new ExpressionParser();

	return instance;
}

const QStringList& ExpressionParser::functionsList() {
	return m_functions;
}

const QStringList& ExpressionParser::constantsList() {
	return m_constants;
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
