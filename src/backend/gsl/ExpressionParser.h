#ifndef EXPRESSIONPARSER_H
#define EXPRESSIONPARSER_H

#include "backend/worksheet/plots/cartesian/XYEquationCurve.h"

class ExpressionParser{

public:
	static ExpressionParser* getInstance();

	bool isValid(const QString&, XYEquationCurve::EquationType);

private:
	ExpressionParser();
	~ExpressionParser();

	static ExpressionParser* instance;
};
#endif