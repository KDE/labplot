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

void ExpressionParserTest::testUniques() {
	QSet<QString> names;
	for (int i = 0; i < _number_functions; i++) {
		const QString name = QLatin1String(_functions[i].name);
		// Dawson is in two groups available with the same function access
		QVERIFY2(!names.contains(name) || name == QStringLiteral("dawson"), qPrintable(name));
		names.insert(name);
	}
}
QTEST_MAIN(ExpressionParserTest)
