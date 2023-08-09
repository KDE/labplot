#include "ExpressionParserTest.h"
#include "backend/gsl/ExpressionParser.h"
#include "backend/gsl/functions.h"

namespace {
func_t2 getFunction2(const QString& s) {
	const QString functionName(s);
	for (int i = 0; i < _number_functions; i++) {
		if (functionName == QLatin1String(_functions[i].name)) {
			if (_functions[i].argc == 2)
				return (func_t2)_functions[i].fnct;
		}
	}
	return nullptr;
}

func_t3 getFunction3(const QString& s) {
	const QString functionName(s);
	for (int i = 0; i < _number_functions; i++) {
		if (functionName == QLatin1String(_functions[i].name)) {
			if (_functions[i].argc == 3)
				return (func_t3)_functions[i].fnct;
		}
	}
	return nullptr;
}
} // anonymous namespace

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

void ExpressionParserTest::testgreaterThan() {
	auto fnct = getFunction2(QStringLiteral("greaterThan"));
	QVERIFY(fnct);

	QCOMPARE(fnct(1, 5), 0);
	QCOMPARE(fnct(4.99, 5), 0);
	QCOMPARE(fnct(5, 5), 0);
	QCOMPARE(fnct(5.000001, 5), 1);
	QCOMPARE(fnct(10, 5), 1);
	QCOMPARE(fnct(0, -0), 0);
}
void ExpressionParserTest::testgreaterEqualThan() {
	auto fnct = getFunction2(QStringLiteral("greaterEqualThan"));
	QVERIFY(fnct);

	QCOMPARE(fnct(1, 5), 0);
	QCOMPARE(fnct(4.99, 5), 0);
	QCOMPARE(fnct(5, 5), 1);
	QCOMPARE(fnct(5.000001, 5), 1);
	QCOMPARE(fnct(10, 5), 1);
	QCOMPARE(fnct(0, -0), 1);
}
void ExpressionParserTest::testlessThan() {
	auto fnct = getFunction2(QStringLiteral("lessThan"));
	QVERIFY(fnct);

	QCOMPARE(fnct(1, 5), 1);
	QCOMPARE(fnct(4.99, 5), 1);
	QCOMPARE(fnct(5, 5), 0);
	QCOMPARE(fnct(5.000001, 0), 0);
	QCOMPARE(fnct(10, 5), 0);
	QCOMPARE(fnct(0, -0), 0);
}
void ExpressionParserTest::testlessEqualThan() {
	auto fnct = getFunction2(QStringLiteral("lessEqualThan"));
	QVERIFY(fnct);

	QCOMPARE(fnct(1, 5), 1);
	QCOMPARE(fnct(4.99, 5), 1);
	QCOMPARE(fnct(5, 5), 1);
	QCOMPARE(fnct(5.000001, 0), 0);
	QCOMPARE(fnct(10, 5), 0);
	QCOMPARE(fnct(0, -0), 1);
}
void ExpressionParserTest::testequal() {
	auto fnct = getFunction2(QStringLiteral("equal"));
	QVERIFY(fnct);

	QCOMPARE(fnct(1, 5), 0);
	QCOMPARE(fnct(4.99, 5), 0);
	QCOMPARE(fnct(5, 5), 1);
	QCOMPARE(fnct(5.000001, 0), 0);
	QCOMPARE(fnct(10, 5), 0);
	QCOMPARE(fnct(0, -0), 1);
}

void ExpressionParserTest::testifCondition() {
	auto fnct = getFunction3(QStringLiteral("if"));
	QVERIFY(fnct);

	QCOMPARE(fnct(1, 1, 5), 1);
	QCOMPARE(fnct(0, 1, 5), 5);

	QCOMPARE(fnct(1.1, 1, 5), 1);
	QCOMPARE(fnct(-1, 1, 5), 1);
}
void ExpressionParserTest::testandFunction() {
	auto fnct = getFunction2(QStringLiteral("and"));
	QVERIFY(fnct);

	QCOMPARE(fnct(1, 1), 1);
	QCOMPARE(fnct(0, 1), 0);
	QCOMPARE(fnct(1, 0), 0);
	QCOMPARE(fnct(0, 0), 0);
	QCOMPARE(fnct(2, 5), 1);
}
void ExpressionParserTest::testorFunction() {
	auto fnct = getFunction2(QStringLiteral("or"));
	QVERIFY(fnct);

	QCOMPARE(fnct(1, 1), 1);
	QCOMPARE(fnct(0, 1), 1);
	QCOMPARE(fnct(1, 0), 1);
	QCOMPARE(fnct(0, 0), 0);
	QCOMPARE(fnct(2, 5), 1);
	QCOMPARE(fnct(0, 2.3345), 1);
}
void ExpressionParserTest::testxorFunction() {
	auto fnct = getFunction2(QStringLiteral("xor"));
	QVERIFY(fnct);

	QCOMPARE(fnct(1, 1), 0);
	QCOMPARE(fnct(0, 1), 1);
	QCOMPARE(fnct(1, 0), 1);
	QCOMPARE(fnct(0, 0), 0);
	QCOMPARE(fnct(2, 5), 0);
	QCOMPARE(fnct(0, 2.3345), 1);
	QCOMPARE(fnct(2.3345, 2.3345), 0);
	QCOMPARE(fnct(2.3345, 0), 1);
}
void ExpressionParserTest::testbetweenIncluded() {
	auto fnct = getFunction3(QStringLiteral("between_inc"));
	QVERIFY(fnct);

	QCOMPARE(fnct(0.99, 1, 5), 0);
	QCOMPARE(fnct(1, 1, 5), 1);
	QCOMPARE(fnct(2, 1, 5), 1);
	QCOMPARE(fnct(5, 1, 5), 1);
	QCOMPARE(fnct(5.11, 1, 5), 0);
}
void ExpressionParserTest::testoutsideIncluded() {
	auto fnct = getFunction3(QStringLiteral("outside_inc"));
	QVERIFY(fnct);

	QCOMPARE(fnct(0.99, 1, 5), 1);
	QCOMPARE(fnct(1, 1, 5), 1);
	QCOMPARE(fnct(2, 1, 5), 0);
	QCOMPARE(fnct(5, 1, 5), 1);
	QCOMPARE(fnct(5.11, 1, 5), 1);
}
void ExpressionParserTest::testbetween() {
	auto fnct = getFunction3(QStringLiteral("between"));
	QVERIFY(fnct);

	QCOMPARE(fnct(0.99, 1, 5), 0);
	QCOMPARE(fnct(1, 1, 5), 0);
	QCOMPARE(fnct(2, 1, 5), 1);
	QCOMPARE(fnct(5, 1, 5), 0);
	QCOMPARE(fnct(5.11, 1, 5), 0);
}
void ExpressionParserTest::testoutside() {
	auto fnct = getFunction3(QStringLiteral("outside"));
	QVERIFY(fnct);

	QCOMPARE(fnct(0.99, 1, 5), 1);
	QCOMPARE(fnct(1, 1, 5), 0);
	QCOMPARE(fnct(2, 1, 5), 0);
	QCOMPARE(fnct(5, 1, 5), 0);
	QCOMPARE(fnct(5.11, 1, 5), 1);
}
void ExpressionParserTest::testequalEpsilon() {
	auto fnct = getFunction3(QStringLiteral("equalE"));
	QVERIFY(fnct);

	QCOMPARE(fnct(0.99, 1, 5), 1);
	QCOMPARE(fnct(1, 1, 5), 1);
	QCOMPARE(fnct(2, 1, 5), 1);
	QCOMPARE(fnct(5, 1, 5), 1);
	QCOMPARE(fnct(5.11, 1, 5), 1);

	QCOMPARE(fnct(5.11, 5.3, 0.1), 0);
	QCOMPARE(fnct(-5.11, 5.3, 0.2), 0);
	QCOMPARE(fnct(-5.11, -5.3, 0.2), 1);
}

QTEST_MAIN(ExpressionParserTest)
