/*
	File                 : ExpressionParserTest.h
	Project              : LabPlot
	Description          : Tests for ExpressionParser
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023-2025 Martin Marmsoler <martin.marmsoler@gmail.com>
	SPDX-FileCopyrightText: 2026 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ExpressionParserTest.h"
#include "backend/gsl/ExpressionParser.h"
#include "backend/gsl/functions.h"

#include <QDate>
#include <QDateTime>
#include <QTime>

using namespace Parsing;

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

void ExpressionParserTest::testIsInValid() {
	const QString expr = QStringLiteral("cell");
	const QStringList vars = {};

	QCOMPARE(ExpressionParser::isValid(expr, vars), false); // should not crash and should be invalid
}

void ExpressionParserTest::testIsInValid2() {
	const QString expr = QStringLiteral("cell_isdk"); // Not a valid symbol
	const QStringList vars = {};

	QCOMPARE(ExpressionParser::isValid(expr, vars), false); // should not crash and should be invalid
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

	QCOMPARE(ExpressionParser::parameters(functionName), i18n("(%1; %2; %3)", i18n("condition"), i18n("trueValue"), i18n("falseValue")));
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

void ExpressionParserTest::testTodayFunction() {
	double result = todayFunction();
	double expected = QDate(1900, 1, 1).daysTo(QDate::currentDate());
	QCOMPARE(result, expected);
}

void ExpressionParserTest::testNowFunction() {
	double result = nowFunction();
	auto now = QDateTime::currentDateTime();
	double expected = double(QDateTime(QDate(1900, 1, 1), QTime(0, 0, 0, 0)).msecsTo(now)) / 86400000.0;
	QVERIFY(qAbs(result - expected) < 1e-3);
}

void ExpressionParserTest::testDateFunction() {
	double result = dateFunction(2026, 4, 19);
	double expected = double(QDate(1900, 1, 1).daysTo(QDate(2026, 4, 19)));
	QCOMPARE(result, expected);
}

void ExpressionParserTest::testDatedifFunction() {
	// Test days difference
	double start = dateFunction(2026, 4, 15);
	double end = dateFunction(2026, 4, 19);
	QCOMPARE(datedifFunction(start, end, 0), 4.0); // 4 days

	// Test months difference
	start = dateFunction(2026, 1, 1);
	end = dateFunction(2026, 4, 1);
	QCOMPARE(datedifFunction(start, end, 1), 3.0); // 3 months

	// Test years difference
	start = dateFunction(2020, 4, 19);
	end = dateFunction(2026, 4, 19);
	QCOMPARE(datedifFunction(start, end, 2), 6.0); // 6 years
}

void ExpressionParserTest::testEomonthFunction() {
	// Test end of current month
	double start = dateFunction(2026, 4, 19);
	QCOMPARE(eomonthFunction(start, 0), dateFunction(2026, 4, 30)); // April 30, 2026

	// Test end of next month
	QCOMPARE(eomonthFunction(start, 1), dateFunction(2026, 5, 31)); // May 31, 2026

	// Test end of previous month
	QCOMPARE(eomonthFunction(start, -1), dateFunction(2026, 3, 31)); // March 31, 2026

	// Test February (non-leap year)
	start = dateFunction(2025, 1, 15);
	QCOMPARE(eomonthFunction(start, 1), dateFunction(2025, 2, 28)); // February 28, 2025

	// Test February (leap year)
	start = dateFunction(2024, 1, 15);
	QCOMPARE(eomonthFunction(start, 1), dateFunction(2024, 2, 29)); // February 29, 2024
}

void ExpressionParserTest::testWeekdayFunction() {
	// Test Monday (April 20, 2026 is a Monday)
	double date = dateFunction(2026, 4, 20);
	QCOMPARE(weekdayFunction(date), 0.0); // Monday = 0

	// Test Tuesday
	date = dateFunction(2026, 4, 21);
	QCOMPARE(weekdayFunction(date), 1.0); // Tuesday = 1

	// Test Wednesday
	date = dateFunction(2026, 4, 22);
	QCOMPARE(weekdayFunction(date), 2.0); // Wednesday = 2

	// Test Thursday
	date = dateFunction(2026, 4, 23);
	QCOMPARE(weekdayFunction(date), 3.0); // Thursday = 3

	// Test Friday
	date = dateFunction(2026, 4, 24);
	QCOMPARE(weekdayFunction(date), 4.0); // Friday = 4

	// Test Saturday
	date = dateFunction(2026, 4, 25);
	QCOMPARE(weekdayFunction(date), 5.0); // Saturday = 5

	// Test Sunday (today)
	date = dateFunction(2026, 4, 19);
	QCOMPARE(weekdayFunction(date), 6.0); // Sunday = 6
}

void ExpressionParserTest::testNetworkdaysFunction() {
	// Test single day - Monday
	double start = dateFunction(2026, 4, 20); // Monday
	double end = dateFunction(2026, 4, 20); // Monday
	QCOMPARE(networkdaysFunction(start, end), 1.0); // 1 working day

	// Test single day - Saturday
	start = dateFunction(2026, 4, 25); // Saturday
	end = dateFunction(2026, 4, 25); // Saturday
	QCOMPARE(networkdaysFunction(start, end), 0.0); // 0 working days

	// Test week: Monday to Friday
	start = dateFunction(2026, 4, 20); // Monday
	end = dateFunction(2026, 4, 24); // Friday
	QCOMPARE(networkdaysFunction(start, end), 5.0); // 5 working days

	// Test week: Monday to Sunday (includes weekend)
	start = dateFunction(2026, 4, 20); // Monday
	end = dateFunction(2026, 4, 26); // Sunday
	QCOMPARE(networkdaysFunction(start, end), 5.0); // 5 working days (Mon-Fri)

	// Test two weeks
	start = dateFunction(2026, 4, 20); // Monday
	end = dateFunction(2026, 5, 3); // Sunday (2 weeks later)
	QCOMPARE(networkdaysFunction(start, end), 10.0); // 10 working days
}

void ExpressionParserTest::testYearFunction() {
	double date = dateFunction(2026, 4, 19);
	QCOMPARE(yearFunction(date), 2026.0);

	date = dateFunction(2000, 1, 1);
	QCOMPARE(yearFunction(date), 2000.0);

	date = dateFunction(2050, 12, 31);
	QCOMPARE(yearFunction(date), 2050.0);
}

void ExpressionParserTest::testMonthFunction() {
	double date = dateFunction(2026, 4, 19);
	QCOMPARE(monthFunction(date), 4.0);

	date = dateFunction(2026, 1, 1);
	QCOMPARE(monthFunction(date), 1.0);

	date = dateFunction(2026, 12, 31);
	QCOMPARE(monthFunction(date), 12.0);
}

void ExpressionParserTest::testDayFunction() {
	double date = dateFunction(2026, 4, 19);
	QCOMPARE(dayFunction(date), 19.0);

	date = dateFunction(2026, 4, 1);
	QCOMPARE(dayFunction(date), 1.0);

	date = dateFunction(2026, 4, 30);
	QCOMPARE(dayFunction(date), 30.0);
}

void ExpressionParserTest::testWeeknumFunction() {
	// April 19, 2026 is Sunday of week 16
	double date = dateFunction(2026, 4, 19);
	QCOMPARE(weeknumFunction(date), 16.0);

	// January 1, 2026 is Thursday of week 1
	date = dateFunction(2026, 1, 1);
	QCOMPARE(weeknumFunction(date), 1.0);

	// First day of 2026 should be in week 1
	date = dateFunction(2026, 1, 4);
	QVERIFY(weeknumFunction(date) >= 1.0);
}

void ExpressionParserTest::testTodayExpression() {
	const QString expr = QStringLiteral("today()");
	const QStringList vars = {};
	QVector<QVector<double>*> xVectors;
	QVector<double> yVector({0.});
	auto* parser = ExpressionParser::getInstance();
	parser->tryEvaluateCartesian(expr, vars, xVectors, &yVector);

	QCOMPARE(yVector.size(), 1);
	QCOMPARE(yVector.at(0), todayFunction());
}

void ExpressionParserTest::testNowExpression() {
	const QString expr = QStringLiteral("now()");
	const QStringList vars = {};
	QVector<QVector<double>*> xVectors;
	QVector<double> yVector({0.});
	auto* parser = ExpressionParser::getInstance();
	parser->tryEvaluateCartesian(expr, vars, xVectors, &yVector);

	QCOMPARE(yVector.size(), 1);
	QVERIFY(qAbs(yVector.at(0) - nowFunction()) < 1e-3);
}

void ExpressionParserTest::testDateExpression() {
	const QString expr = QStringLiteral("date(2026; 4; 19)");
	const QStringList vars = {};
	QVector<QVector<double>*> xVectors;
	QVector<double> yVector({0.});
	auto* parser = ExpressionParser::getInstance();
	parser->tryEvaluateCartesian(expr, vars, xVectors, &yVector);

	QCOMPARE(yVector.size(), 1);
	QCOMPARE(yVector.at(0), dateFunction(2026, 4, 19));
}

void ExpressionParserTest::testDatedifExpression() {
	const QString expr = QStringLiteral("datedif(date(2026; 4; 15); date(2026; 4; 19); 0)");
	const QStringList vars = {};
	QVector<QVector<double>*> xVectors;
	QVector<double> yVector({0.});
	auto* parser = ExpressionParser::getInstance();
	parser->tryEvaluateCartesian(expr, vars, xVectors, &yVector);

	QCOMPARE(yVector.size(), 1);
	QCOMPARE(yVector.at(0), 4.0); // 4 days difference
}

void ExpressionParserTest::testEomonthExpression() {
	const QString expr = QStringLiteral("eomonth(date(2026; 4; 19); 0)");
	const QStringList vars = {};
	QVector<QVector<double>*> xVectors;
	QVector<double> yVector({0.});
	auto* parser = ExpressionParser::getInstance();
	parser->tryEvaluateCartesian(expr, vars, xVectors, &yVector);

	QCOMPARE(yVector.size(), 1);
	QCOMPARE(yVector.at(0), dateFunction(2026, 4, 30)); // End of April 2026
}

void ExpressionParserTest::testWeekdayExpression() {
	const QString expr = QStringLiteral("weekday(date(2026; 4; 20))"); // Monday
	const QStringList vars = {};
	QVector<QVector<double>*> xVectors;
	QVector<double> yVector({0.});
	auto* parser = ExpressionParser::getInstance();
	parser->tryEvaluateCartesian(expr, vars, xVectors, &yVector);

	QCOMPARE(yVector.size(), 1);
	QCOMPARE(yVector.at(0), 0.0); // Monday = 0
}

void ExpressionParserTest::testNetworkdaysExpression() {
	const QString expr = QStringLiteral("networkdays(date(2026; 4; 20); date(2026; 4; 24))"); // Mon-Fri
	const QStringList vars = {};
	QVector<QVector<double>*> xVectors;
	QVector<double> yVector({0.});
	auto* parser = ExpressionParser::getInstance();
	parser->tryEvaluateCartesian(expr, vars, xVectors, &yVector);

	QCOMPARE(yVector.size(), 1);
	QCOMPARE(yVector.at(0), 5.0); // 5 working days
}

void ExpressionParserTest::testYearExpression() {
	const QString expr = QStringLiteral("year(date(2026; 4; 19))");
	const QStringList vars = {};
	QVector<QVector<double>*> xVectors;
	QVector<double> yVector({0.});
	auto* parser = ExpressionParser::getInstance();
	parser->tryEvaluateCartesian(expr, vars, xVectors, &yVector);

	QCOMPARE(yVector.size(), 1);
	QCOMPARE(yVector.at(0), 2026.0);
}

void ExpressionParserTest::testMonthExpression() {
	const QString expr = QStringLiteral("month(date(2026; 4; 19))");
	const QStringList vars = {};
	QVector<QVector<double>*> xVectors;
	QVector<double> yVector({0.});
	auto* parser = ExpressionParser::getInstance();
	parser->tryEvaluateCartesian(expr, vars, xVectors, &yVector);

	QCOMPARE(yVector.size(), 1);
	QCOMPARE(yVector.at(0), 4.0);
}

void ExpressionParserTest::testDayExpression() {
	const QString expr = QStringLiteral("day(date(2026; 4; 19))");
	const QStringList vars = {};
	QVector<QVector<double>*> xVectors;
	QVector<double> yVector({0.});
	auto* parser = ExpressionParser::getInstance();
	parser->tryEvaluateCartesian(expr, vars, xVectors, &yVector);

	QCOMPARE(yVector.size(), 1);
	QCOMPARE(yVector.at(0), 19.0);
}

void ExpressionParserTest::testWeeknumExpression() {
	const QString expr = QStringLiteral("weeknum(date(2026; 4; 19))");
	const QStringList vars = {};
	QVector<QVector<double>*> xVectors;
	QVector<double> yVector({0.});
	auto* parser = ExpressionParser::getInstance();
	parser->tryEvaluateCartesian(expr, vars, xVectors, &yVector);

	QCOMPARE(yVector.size(), 1);
	QCOMPARE(yVector.at(0), 16.0); // April 19, 2026 is in week 16
}

void ExpressionParserTest::testDateTimeVariableExpression() {
	const QString expr = QStringLiteral("day(x) + month(x) * 100 + year(x) * 10000");
	const QStringList vars = {QStringLiteral("x")};
	QVector<QVector<double>*> xVectors;

	double dateTimeValue = dateFunction(2026, 4, 19);

	auto* xVector = new QVector<double>({dateTimeValue});
	xVectors << xVector;
	QVector<double> yVector({0.});
	auto* parser = ExpressionParser::getInstance();
	parser->tryEvaluateCartesian(expr, vars, xVectors, &yVector);

	QCOMPARE(yVector.size(), 1);
	QCOMPARE(yVector.at(0), 20260419.0);

	delete xVector;
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
	auto* parser = ExpressionParser::getInstance();
	parser->tryEvaluateCartesian(expr, vars, xVectors, &yVector);

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
	auto* parser = ExpressionParser::getInstance();
	parser->tryEvaluateCartesian(expr, vars, xVectors, &yVector);

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
	auto* parser = ExpressionParser::getInstance();
	parser->tryEvaluateCartesian(expr, vars, xVectors, &yVector);

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
	auto* parser = ExpressionParser::getInstance();
	parser->tryEvaluateCartesian(expr, vars, xVectors, &yVector);

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
	auto* parser = ExpressionParser::getInstance();
	parser->tryEvaluateCartesian(expr, vars, xVectors, &yVector);

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
	auto* parser = ExpressionParser::getInstance();
	parser->tryEvaluateCartesian(expr, vars, xVectors, &yVector);

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
	auto* parser = ExpressionParser::getInstance();
	parser->tryEvaluateCartesian(expr, vars, xVectors, &yVector);

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
	auto* parser = ExpressionParser::getInstance();
	parser->tryEvaluateCartesian(expr, vars, xVectors, &yVector);

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
	auto* parser = ExpressionParser::getInstance();
	parser->tryEvaluateCartesian(expr, vars, xVectors, &yVector);

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
	auto* parser = ExpressionParser::getInstance();
	parser->tryEvaluateCartesian(expr, vars, xVectors, &yVector);

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
		QBENCHMARK {
			auto* parser = ExpressionParser::getInstance();
			parser->tryEvaluateCartesian(expr, vars, xVectors, &yVector, true);
		}
		// QCOMPARE(QLatin1String(Parsing::lastErrorMessage()), QStringLiteral(""));

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
		QBENCHMARK {
			auto* parser = ExpressionParser::getInstance();
			parser->tryEvaluateCartesian(expr, vars, xVectors, &yVector, false);
		}
		// QCOMPARE(QLatin1String(Parsing::lastErrorMessage()), QStringLiteral(""));

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
//	auto* parser = ExpressionParser::getInstance();
// parser->tryEvaluateCartesian(expr, vars, xVectors, &yVector);

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

void ExpressionParserTest::testPolarCircle() {
	auto* parser = ExpressionParser::getInstance();
	constexpr auto numElements = 100;
	QVector<double> xVector(numElements);
	QVector<double> yVector(numElements);
	// Constant radius 5.7
	QVERIFY(parser->tryEvaluatePolar(QStringLiteral("5.7"), QStringLiteral("0.0"), QStringLiteral("6.28"), numElements, &xVector, &yVector));

	QCOMPARE(xVector.size(), numElements);
	QCOMPARE(yVector.size(), numElements);

	for (int i = 0; i < 100; i++)
		VALUES_EQUAL(sqrt(xVector.at(i) * xVector.at(i) + yVector.at(i) * yVector.at(i)), 5.7);
}

void ExpressionParserTest::testPolarSpiral() {
	auto* parser = ExpressionParser::getInstance();
	constexpr auto numElements = 100;
	QVector<double> xVector(numElements);
	QVector<double> yVector(numElements);

	// Radius increases with every iteration by 1/100
	QVERIFY(parser->tryEvaluatePolar(QStringLiteral("i/100"), QStringLiteral("0.0"), QStringLiteral("6.28"), numElements, &xVector, &yVector));

	QCOMPARE(xVector.size(), numElements);
	QCOMPARE(yVector.size(), numElements);

	for (int i = 0; i < numElements; i++)
		VALUES_EQUAL(sqrt(xVector.at(i) * xVector.at(i) + yVector.at(i) * yVector.at(i)), (i + 1) / 100.);
}

QTEST_MAIN(ExpressionParserTest)
