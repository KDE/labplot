#include "ExpressionParserTest.h"
#include "backend/gsl/ExpressionParser.h"
#include "backend/gsl/functions.h"

using namespace Parser;

namespace {
func_t1 getFunction1(const QString& s) {
	const QString functionName(s);
	for (int i = 0; i < _number_functions; i++) {
		if (functionName == QLatin1String(_functions[i].name)) {
			if (_functions[i].argc == 1)
				return std::get<func_t1>(_functions[i].fnct);
		}
	}
	return nullptr;
}

func_t2 getFunction2(const QString& s) {
	const QString functionName(s);
	for (int i = 0; i < _number_functions; i++) {
		if (functionName == QLatin1String(_functions[i].name)) {
			if (_functions[i].argc == 2)
				return std::get<func_t2>(_functions[i].fnct);
		}
	}
	return nullptr;
}

func_t3 getFunction3(const QString& s) {
	const QString functionName(s);
	for (int i = 0; i < _number_functions; i++) {
		if (functionName == QLatin1String(_functions[i].name)) {
			if (_functions[i].argc == 3)
				return std::get<func_t3>(_functions[i].fnct);
		}
	}
	return nullptr;
}
} // anonymous namespace

void ExpressionParserTest::testIsValid() {
	const QString expr = QStringLiteral("cell(5; x)");
	const QStringList vars = {QStringLiteral("x")};

	QCOMPARE(ExpressionParser::isValid(expr, vars), true); // should not crash
}

void ExpressionParserTest::testIsValidStdev() {
	const QString expr = QStringLiteral("stdev(x)");
	const QStringList vars = {QStringLiteral("x")};

	QCOMPARE(ExpressionParser::isValid(expr, vars), true);
}

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

	QCOMPARE(ExpressionParser::parameters(functionName), QStringLiteral("(condition; trueValue; falseValue)"));
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

void ExpressionParserTest::testnotFunction() {
	auto fnct = getFunction1(QStringLiteral("not"));
	QVERIFY(fnct);

	QCOMPARE(fnct(1), 0);
	QCOMPARE(fnct(0), 1);
	QCOMPARE(fnct(-1), 0); // According to C/C++ standard. -1 is true and !-1 is false
	QCOMPARE(fnct(2), 0);
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

void ExpressionParserTest::testRoundn() {
	auto fnct = getFunction2(QStringLiteral("roundn"));
	QVERIFY(fnct);

	QCOMPARE(fnct(3.1415, 2), 3.14); // round down
	QCOMPARE(fnct(10.2397281298423, 5), 10.23973); // roundup
	QCOMPARE(fnct(10000.1, 5), 10000.1);
	QCOMPARE(fnct(123.45, -1), 120.);
	QCOMPARE(fnct(1.45, 1), 1.5);
	QCOMPARE(fnct(-1.45, 1), -1.5);
	QCOMPARE(fnct(-1.44, 1), -1.4);
	QCOMPARE(fnct(-123.45, 1), -123.5);
	QCOMPARE(fnct(-123.44, 1), -123.4);
	QCOMPARE(fnct(-123.45, -1), -120);
}

void ExpressionParserTest::testSpecialFunctions() {
	auto fnct = getFunction1(QStringLiteral("cbrt"));
	QVERIFY(fnct);

	QCOMPARE(fnct(8.), 2.);
	QCOMPARE(fnct(-8.), -2.);

	fnct = getFunction1(QStringLiteral("logb"));
	QVERIFY(fnct);

	QCOMPARE(fnct(0.1), -4.);
	QCOMPARE(fnct(0.4), -2.);
	QCOMPARE(fnct(0.5), -1.);
	QCOMPARE(fnct(8.), 3.);
	QCOMPARE(fnct(10.), 3.);
	QCOMPARE(fnct(100.), 6.);

	fnct = getFunction1(QStringLiteral("rint"));
	QVERIFY(fnct);

	QCOMPARE(fnct(0.), 0.);
	QCOMPARE(fnct(0.5), 0.);
	QCOMPARE(fnct(1.49), 1.);
	QCOMPARE(fnct(-0.5), 0.);
	QCOMPARE(fnct(-0.99), -1.);
	QCOMPARE(fnct(-1.5), -2.);

	fnct = getFunction1(QStringLiteral("round"));
	QVERIFY(fnct);

	QCOMPARE(fnct(0.), 0.);
	QCOMPARE(fnct(0.5), 1.);
	QCOMPARE(fnct(1.49), 1.);
	QCOMPARE(fnct(-0.5), -1.);
	QCOMPARE(fnct(-0.99), -1.);
	QCOMPARE(fnct(-1.5), -2.);

	fnct = getFunction1(QStringLiteral("trunc"));
	QVERIFY(fnct);

	QCOMPARE(fnct(0.), 0.);
	QCOMPARE(fnct(0.5), 0.);
	QCOMPARE(fnct(1.49), 1.);
	QCOMPARE(fnct(-0.5), -0.);
	QCOMPARE(fnct(-0.99), -0.);
	QCOMPARE(fnct(-1.5), -1.);
}

void ExpressionParserTest::testevaluateCartesian() {
	const QString expr = QStringLiteral("x+y");
	const QStringList vars = {QStringLiteral("x"), QStringLiteral("y")};

	QVector<QVector<double>*> xVectors;

	xVectors << new QVector<double>({1., 2., 3.}); // x
	xVectors << new QVector<double>({4., 5., 6., 9.}); // y
	QVector<double> yVector({101., 123., 345., 239., 1290., 43290., 238., 342., 823., 239.});
	ExpressionParser::tryEvaluateCartesian(expr, vars, xVectors, &yVector);

	QVector<double> ref({5., 7., 9.});
	QCOMPARE(yVector.size(), 10);
	COMPARE_DOUBLE_VECTORS_AT_LEAST_LENGTH(yVector, ref);
	QCOMPARE(yVector.at(3), NAN);
	QCOMPARE(yVector.at(4), NAN);
	QCOMPARE(yVector.at(5), NAN);
	QCOMPARE(yVector.at(6), NAN);
	QCOMPARE(yVector.at(7), NAN);
	QCOMPARE(yVector.at(8), NAN);
	QCOMPARE(yVector.at(9), NAN);
}

void ExpressionParserTest::testevaluateCartesianConstExpr() {
	const QString expr = QStringLiteral("5 + 5");
	const QStringList vars = {QStringLiteral("x"), QStringLiteral("y")};

	QVector<QVector<double>*> xVectors;

	xVectors << new QVector<double>({1., 2., 3.}); // x
	xVectors << new QVector<double>({4., 5., 6., 9.}); // y
	QVector<double> yVector({101., 123., 345., 239., 1290., 43290., 238., 342., 823., 239.});
	ExpressionParser::tryEvaluateCartesian(expr, vars, xVectors, &yVector);

	QCOMPARE(yVector.size(), 10);
	// All yVector rows are filled
	for (const auto v : yVector)
		QCOMPARE(v, 5. + 5.);
}

void ExpressionParserTest::testEvaluateAnd() {
	const QString expr = QStringLiteral("x && y");
	const QStringList vars = {QStringLiteral("x"), QStringLiteral("y")};

	QVector<QVector<double>*> xVectors;

	xVectors << new QVector<double>({1., 0., 1., 0.}); // x
	xVectors << new QVector<double>({0., 0., 1., 1.}); // y
	QVector<double> yVector({
		5.,
		5.,
		5.,
		5.,
	}); // random value
	ExpressionParser::tryEvaluateCartesian(expr, vars, xVectors, &yVector);

	QCOMPARE(yVector.size(), 4);
	QCOMPARE(yVector.at(0), 0.);
	QCOMPARE(yVector.at(1), 0.);
	QCOMPARE(yVector.at(2), 1.);
	QCOMPARE(yVector.at(3), 0.);
}

void ExpressionParserTest::testEvaluateOr() {
	const QString expr = QStringLiteral("x || y");
	const QStringList vars = {QStringLiteral("x"), QStringLiteral("y")};

	QVector<QVector<double>*> xVectors;

	xVectors << new QVector<double>({1., 0., 1., 0.}); // x
	xVectors << new QVector<double>({0., 0., 1., 1.}); // y
	QVector<double> yVector({
		5.,
		5.,
		5.,
		5.,
	}); // random value
	ExpressionParser::tryEvaluateCartesian(expr, vars, xVectors, &yVector);

	QCOMPARE(yVector.size(), 4);
	QCOMPARE(yVector.at(0), 1.);
	QCOMPARE(yVector.at(1), 0.);
	QCOMPARE(yVector.at(2), 1.);
	QCOMPARE(yVector.at(3), 1.);
}

void ExpressionParserTest::testEvaluateNot() {
	const QString expr = QStringLiteral("!x");
	const QStringList vars = {QStringLiteral("x")};

	QVector<QVector<double>*> xVectors;

	xVectors << new QVector<double>({1., 0., -1., 2.}); // x
	QVector<double> yVector({5., 5., 5., 5.}); // random value
	ExpressionParser::tryEvaluateCartesian(expr, vars, xVectors, &yVector);

	QCOMPARE(yVector.size(), 4);
	QCOMPARE(yVector.at(0), 0.);
	QCOMPARE(yVector.at(1), 1.);
	QCOMPARE(yVector.at(2), 0.); // According to C/C++ standard, -1 is true for bool and therefore the negated is false
	QCOMPARE(yVector.at(3), 0.);
}

void ExpressionParserTest::testEvaluateLogicalExpression() {
	const QString expr = QStringLiteral("!x || y && x");
	const QStringList vars = {QStringLiteral("x"), QStringLiteral("y")};

	QVector<QVector<double>*> xVectors;

	xVectors << new QVector<double>({0., 1., 0., 1.}); // x
	xVectors << new QVector<double>({0., 0., 1., 1.}); // y
	QVector<double> yVector({
		5.,
		5.,
		5.,
		5.,
	}); // random value
	ExpressionParser::tryEvaluateCartesian(expr, vars, xVectors, &yVector);

	// if or evaluated first:
	// x y
	// 0 0 : 0
	// 1 0 : 0
	// 0 1 : 0
	// 1 1 : 1

	// if and evaluated first:
	// x y
	// 0 0 : 1
	// 1 0 : 0
	// 0 1 : 1
	// 1 1 : 1

	// And must be evaluated before or!
	QCOMPARE(yVector.size(), 4);
	QCOMPARE(yVector.at(0), 1.);
	QCOMPARE(yVector.at(1), 0.);
	QCOMPARE(yVector.at(2), 1.);
	QCOMPARE(yVector.at(3), 1.);
}

void ExpressionParserTest::testevaluateGreaterThan() {
	const QString expr = QStringLiteral("x > y");
	const QStringList vars = {QStringLiteral("x"), QStringLiteral("y")};

	QVector<QVector<double>*> xVectors;

	xVectors << new QVector<double>({0., 1., 0., 1., -1., -std::numeric_limits<double>::infinity(), 1, std::numeric_limits<double>::infinity(), 7.}); // x
	xVectors << new QVector<double>({0., 0., 1., 1., -2., std::numeric_limits<double>::infinity(), std::nan("0"), 1e9, 7.}); // y
	QVector<double> yVector({5., 5., 5., 5., 5., 5., 5., 5., 5.}); // random value
	ExpressionParser::tryEvaluateCartesian(expr, vars, xVectors, &yVector);

	QCOMPARE(yVector.size(), 9);
	QCOMPARE(yVector.at(0), 0.);
	QCOMPARE(yVector.at(1), 1.);
	QCOMPARE(yVector.at(2), 0.);
	QCOMPARE(yVector.at(3), 0.);
	QCOMPARE(yVector.at(4), 1.);
	QCOMPARE(yVector.at(5), 0.);
	QCOMPARE(yVector.at(6), 0.);
	QCOMPARE(yVector.at(7), 1.);
	QCOMPARE(yVector.at(8), 0.);
}

void ExpressionParserTest::testevaluateLessThan() {
	const QString expr = QStringLiteral("x < y");
	const QStringList vars = {QStringLiteral("x"), QStringLiteral("y")};

	QVector<QVector<double>*> xVectors;

	xVectors << new QVector<double>({0., 1., 0., 1., -1., -std::numeric_limits<double>::infinity(), 1, std::numeric_limits<double>::infinity(), 7.}); // x
	xVectors << new QVector<double>({0., 0., 1., 1., -2., std::numeric_limits<double>::infinity(), std::nan("0"), 1e9, 7.}); // y
	QVector<double> yVector({5., 5., 5., 5., 5., 5., 5., 5., 5.}); // random value
	ExpressionParser::tryEvaluateCartesian(expr, vars, xVectors, &yVector);

	QCOMPARE(yVector.size(), 9);
	QCOMPARE(yVector.at(0), 0.);
	QCOMPARE(yVector.at(1), 0.);
	QCOMPARE(yVector.at(2), 1.);
	QCOMPARE(yVector.at(3), 0.);
	QCOMPARE(yVector.at(4), 0.);
	QCOMPARE(yVector.at(5), 1.);
	QCOMPARE(yVector.at(6), 0.);
	QCOMPARE(yVector.at(7), 0.);
	QCOMPARE(yVector.at(8), 0.);
}

void ExpressionParserTest::testevaluateLessEqualThan() {
	const QString expr = QStringLiteral("x <= y");
	const QStringList vars = {QStringLiteral("x"), QStringLiteral("y")};

	QVector<QVector<double>*> xVectors;

	xVectors << new QVector<double>({0., 1., 0., 1., -1., -std::numeric_limits<double>::infinity(), 1, std::numeric_limits<double>::infinity(), 7.}); // x
	xVectors << new QVector<double>({0., 0., 1., 1., -2., std::numeric_limits<double>::infinity(), std::nan("0"), 1e9, 7.}); // y
	QVector<double> yVector({5., 5., 5., 5., 5., 5., 5., 5., 5.}); // random value
	ExpressionParser::tryEvaluateCartesian(expr, vars, xVectors, &yVector);

	QCOMPARE(yVector.size(), 9);
	QCOMPARE(yVector.at(0), 1.);
	QCOMPARE(yVector.at(1), 0.);
	QCOMPARE(yVector.at(2), 1.);
	QCOMPARE(yVector.at(3), 1.);
	QCOMPARE(yVector.at(4), 0.);
	QCOMPARE(yVector.at(5), 1.);
	QCOMPARE(yVector.at(6), 0.);
	QCOMPARE(yVector.at(7), 0.);
	QCOMPARE(yVector.at(8), 1.);
}

void ExpressionParserTest::testevaluateGreaterEqualThan() {
	const QString expr = QStringLiteral("x >= y");
	const QStringList vars = {QStringLiteral("x"), QStringLiteral("y")};

	QVector<QVector<double>*> xVectors;

	xVectors << new QVector<double>({0., 1., 0., 1., -1., -std::numeric_limits<double>::infinity(), 1, std::numeric_limits<double>::infinity(), 7.}); // x
	xVectors << new QVector<double>({0., 0., 1., 1., -2., std::numeric_limits<double>::infinity(), std::nan("0"), 1e9, 7.}); // y
	QVector<double> yVector({5., 5., 5., 5., 5., 5., 5., 5., 5.}); // random value
	ExpressionParser::tryEvaluateCartesian(expr, vars, xVectors, &yVector);

	QCOMPARE(yVector.size(), 9);
	QCOMPARE(yVector.at(0), 1.);
	QCOMPARE(yVector.at(1), 1.);
	QCOMPARE(yVector.at(2), 0.);
	QCOMPARE(yVector.at(3), 1.);
	QCOMPARE(yVector.at(4), 1.);
	QCOMPARE(yVector.at(5), 0.);
	QCOMPARE(yVector.at(6), 0.);
	QCOMPARE(yVector.at(7), 1.);
	QCOMPARE(yVector.at(8), 1.);
}

void ExpressionParserTest::testBenchmark() {
	const QString expr = QStringLiteral("atan2(x;y) + sqrt(x)");
	const QStringList vars = {QStringLiteral("x"), QStringLiteral("y")};

	const int values = 3000;

	{
		QVector<QVector<double>*> xVectors;

		auto* x = new QVector<double>(values);
		auto* y = new QVector<double>(values);
		for (int i = 0; i < values; i++) {
			if (i % 2 == 0) {
				x->operator[](i) = 5.;
				y->operator[](i) = 2.;
			} else {
				x->operator[](i) = 24.;
				y->operator[](i) = 22.;
			}
		}

		xVectors << x; // x
		xVectors << y; // y
		QVector<double> yVector(values); // random value
		QBENCHMARK { ExpressionParser::tryEvaluateCartesian(expr, vars, xVectors, &yVector, true); }
		// QCOMPARE(QLatin1String(Parser::lastErrorMessage()), QStringLiteral(""));

		QCOMPARE(yVector.size(), values);
		for (int i = 0; i < values; i++) {
			if (i % 2 == 0) {
				VALUES_EQUAL(yVector.at(i), 3.42635792718232);
			} else {
				VALUES_EQUAL(yVector.at(i), 5.72782854435533);
			}
		}
	}

	{
		QVector<QVector<double>*> xVectors;

		auto* x = new QVector<double>(values);
		auto* y = new QVector<double>(values);
		for (int i = 0; i < values; i++) {
			if (i % 2 == 0) {
				x->operator[](i) = 5.;
				y->operator[](i) = 2.;
			} else {
				x->operator[](i) = 24.;
				y->operator[](i) = 22.;
			}
		}

		xVectors << x; // x
		xVectors << y; // y
		QVector<double> yVector(values); // random value
		QBENCHMARK { ExpressionParser::tryEvaluateCartesian(expr, vars, xVectors, &yVector, false); }
		// QCOMPARE(QLatin1String(Parser::lastErrorMessage()), QStringLiteral(""));

		QCOMPARE(yVector.size(), values);
		for (int i = 0; i < values; i++) {
			if (i % 2 == 0) {
				VALUES_EQUAL(yVector.at(i), 3.42635792718232);
			} else {
				VALUES_EQUAL(yVector.at(i), 5.72782854435533);
			}
		}
	}
}

// This is not implemented. It uses always the smallest rowCount
// Does not matter if the variable is used in the expression or not
// void ExpressionParserTest::testevaluateCartesianConstExpr2() {
//	const QString expr = QStringLiteral("5 + y");
//	const QStringList vars = {QStringLiteral("x"), QStringLiteral("y")};

//	QVector<QVector<double>*> xVectors;

//	xVectors << new QVector<double>({1., 2., 3.}); // x
//	xVectors << new QVector<double>({4., 5., 6., 9.}); // y
//	QVector<double> yVector({101., 123., 345., 239., 1290., 43290., 238., 342., 823., 239.});
//	ExpressionParser::tryEvaluateCartesian(expr, vars, xVectors, &yVector);

//	QVector<double> ref({9, 10, 11, 14}); // 5 + y
//	QCOMPARE(yVector.size(), 10);
//	COMPARE_DOUBLE_VECTORS_AT_LEAST_LENGTH(yVector, ref);
//	QCOMPARE(yVector.at(4), NAN);
//	QCOMPARE(yVector.at(5), NAN);
//	QCOMPARE(yVector.at(6), NAN);
//	QCOMPARE(yVector.at(7), NAN);
//	QCOMPARE(yVector.at(8), NAN);
//	QCOMPARE(yVector.at(9), NAN);
//}

void ExpressionParserTest::testLog2() {
	auto fnct = getFunction1(QStringLiteral("log2"));
	QVERIFY(fnct);

	QCOMPARE(fnct(2), 1.);
	QCOMPARE(fnct(10), 3.32192809489);
}

QTEST_MAIN(ExpressionParserTest)
