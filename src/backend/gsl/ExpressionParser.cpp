#include "backend/gsl/ExpressionParser.h"
#include "backend/gsl/parser_extern.h"

#include <gsl/gsl_errno.h>
#include <gsl/gsl_math.h>

#include <QString>

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
