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

#include <gsl/gsl_errno.h>
#include <gsl/gsl_math.h>

ExpressionParser* ExpressionParser::instance = NULL;

ExpressionParser::ExpressionParser(){
	init_table();
}

ExpressionParser::~ExpressionParser(){
	delete_table();
}

ExpressionParser* ExpressionParser::getInstance(){
	if (!instance)
		instance = new ExpressionParser();

	return instance;
}

bool ExpressionParser::isValid(const QString& expr, XYEquationCurve::EquationType type){
	char* data = expr.toLocal8Bit().data();
	if (type == XYEquationCurve::Cartesian) {
		char xVar[] = "x";
		double x = 0;
		assign_variable(xVar,x);
	}

	parse(data);
	return !(parse_errors()>0);
}
