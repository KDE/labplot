#include "ExpressionParserTest.h"
#include "backend/gsl/ExpressionParser.h"
#include "backend/gsl/functions.h"

void ExpressionParserTest::testFunctionArguments1() {
	const QString functionName(QStringLiteral("rand"));
	for (int i = 0; i < _number_functions; i++) {
		if (functionName == QLatin1String(_functions[i].name))
			QVERIFY(_functions[i].parameterFunction == nullptr);
	}

	QCOMPARE(ExpressionParser::parameters(functionName), QStringLiteral(""));
}

void ExpressionParserTest::testFunctionArguments2() {
	const QString functionName(QStringLiteral("if"));
	for (int i = 0; i < _number_functions; i++) {
		if (functionName == QLatin1String(_functions[i].name))
			QVERIFY(_functions[i].parameterFunction != nullptr);
	}

	QCOMPARE(ExpressionParser::parameters(functionName), QStringLiteral("(condition, trueValue, falseValue)"));
}

QTEST_MAIN(ExpressionParserTest)
