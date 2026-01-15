/*
	File                 : ParserTest.cpp
	Project              : LabPlot
	Description          : Tests for the Parser
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2020 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ParserTest.h"

#include "backend/gsl/Parser.h"

#include <backend/lib/Range.h>
#include <cstdlib>
#include <gsl/gsl_const_mksa.h>
#include <gsl/gsl_errno.h>

using namespace Parsing;

//**********************************************************
//****************** Function tests ************************
//**********************************************************

void ParserTest::testBasics() {
	Parsing::Parser parser(false);
	const QVector<QPair<QString, double>> tests{{QStringLiteral("42"), 42.},
												{QStringLiteral("1."), 1.},
												{QStringLiteral("1+1"), 2.},
												{QStringLiteral("1+2+3+4+5"), 15.},
												{QStringLiteral("2*3"), 6.},
												{QStringLiteral("3/2"), 1.5},
												{QStringLiteral("2 -4 +6 -1 -1- 0 +8"), 10.},
												{QStringLiteral("1/3+1/3+1/3"), 1.},
												{QStringLiteral("1.5 + 2.5"), 4.},
												{QStringLiteral("4*2.5 + 8.5+1.5 / 3.0"), 19.},
												{QStringLiteral("5.0005 + 0.0095"), 5.01},
												{QStringLiteral("pi"), M_PI},
												{QStringLiteral("e"), M_E},
												{QStringLiteral("e^1"), M_E},
												{QStringLiteral("hbar"), GSL_CONST_MKSA_PLANCKS_CONSTANT_HBAR},
												{QStringLiteral(" 1   +   1  "), 2.},
												{QStringLiteral("1-    2"), -1.},
												{QStringLiteral("2*    2.5"), 5.},
												{QStringLiteral("3 + 8/5 -1 -2*5"), -6.4},
												{QStringLiteral("(1)"), 1.},
												{QStringLiteral("-(1)"), -1.},
												{QStringLiteral("(1+1)"), 2},
												{QStringLiteral("(sin(0))"), 0.},
												{QStringLiteral("(( ((2)) + 4))*((5))"), 30.},
												{QStringLiteral("2^2"), 4.},
												{QStringLiteral("3**2"), 9.},
												{QStringLiteral("1%1"), 0.},
												{QStringLiteral("3%2"), 1.},
												{QStringLiteral("1.e-5"), 1.e-5},
												{QStringLiteral("9.5E3"), 9500.},
												{QStringLiteral("|1.5|"), 1.5},
												{QStringLiteral("|-2.5|"), 2.5},
												{QStringLiteral("0!"), 1},
												{QStringLiteral("4!"), 24},
												{QStringLiteral("-3!"), -6.},
												{QStringLiteral("exp(0)"), 1.},
												{QStringLiteral("exp(1)"), M_E},
												{QStringLiteral("sqrt(0)"), 0.},
												{QStringLiteral("sin(0)"), 0.},
												{QStringLiteral("cos(pi)"), -1.}};

	for (auto& expr : tests)
		QCOMPARE(parser.parse(qPrintable(expr.first), "C"), expr.second);

	const QVector<QPair<QString, double>> testsFuzzy{{QStringLiteral("(sin(pi))"), 0.}};

	for (const auto& expr : testsFuzzy)
		FuzzyCompare(parser.parse(qPrintable(expr.first), "C"), expr.second, 1.e-15);
}

void ParserTest::testFunction0Arguments() {
	// Function with no arguments

	Parsing::Parser parser(false);
	const auto res1 = parser.parse(qPrintable(QStringLiteral("rand()")), "C");
	QVERIFY(res1 >= 0. && res1 <= (double)RAND_MAX);

	const auto res2 = parser.parse(qPrintable(QStringLiteral("rand()")), "C");
	QVERIFY(res2 >= 0. && res2 <= (double)RAND_MAX);

	QVERIFY(res1 != res2); // It is really unlikely that they are equal
}

void ParserTest::testFunction1Argument() {
	Parsing::Parser parser(false);
	FuzzyCompare(parser.parse(qPrintable(QStringLiteral("sin(pi)")), "C"), 0., 1.e-15);

	VALUES_EQUAL(parser.parse(qPrintable(QStringLiteral("fabs(-5.324)")), "C"), 5.324);
}

void ParserTest::testFunction2Arguments() {
	Parsing::Parser parser(false);

	VALUES_EQUAL(parser.parse(qPrintable(QStringLiteral("atan2(0.1;0.2)")), "C"), 0.46364761);
	VALUES_EQUAL(parser.parse(qPrintable(QStringLiteral("atan2(0.1,0.2)")), "C"), 0.46364761);
}

void ParserTest::testFunction3Arguments() {
	Parsing::Parser parser(false);

	VALUES_EQUAL(parser.parse(qPrintable(QStringLiteral("if(1;-5.0;3.2)")), "C"), -5.);
	VALUES_EQUAL(parser.parse(qPrintable(QStringLiteral("if(0;-5.0;3.2)")), "C"), 3.2);
	// With comma
	VALUES_EQUAL(parser.parse(qPrintable(QStringLiteral("if(1,-5.2,3.7)")), "C"), -5.2);
	VALUES_EQUAL(parser.parse(qPrintable(QStringLiteral("if(0,-5.7,3.5)")), "C"), 3.5);
}

void ParserTest::testErrors() {
	Parsing::Parser parser(false);
	gsl_set_error_handler_off(); // do not crash

	const QVector<QString> testsNan{QString(),
									QStringLiteral("a"),
									QStringLiteral("1+"),
									QStringLiteral("a+1"),
									QStringLiteral("&"),
									QStringLiteral("%"),
									QStringLiteral("+"),
									QStringLiteral("*"),
									QStringLiteral("/"),
									QStringLiteral("{1}"),
									QStringLiteral("{1*2}"),
									QStringLiteral("(1+1))"),
									QStringLiteral("a/0"),
									QStringLiteral("0/0"),
									QStringLiteral("1/0 + a"),
									QStringLiteral("sqrt(-1)"),
									QStringLiteral("log(-1)"),
									QStringLiteral("log(0)"),
									QStringLiteral("asin(2)")};

	for (const auto& expr : testsNan)
		QVERIFY(std::isnan(parser.parse(qPrintable(expr), "C")));

	const QVector<QString> testsInf{QStringLiteral("1/0"), QStringLiteral("-1/0"), QStringLiteral("1+1/0")};

	for (const auto& expr : testsInf)
		QVERIFY(std::isinf(parser.parse(qPrintable(expr), "C")));
}

void ParserTest::testVariables() {
	Parsing::Parser parser(false);
	parser.assign_symbol("a", 1.);
	const QVector<QPair<QString, double>> tests{{QStringLiteral("a"), 1.},
												{QStringLiteral("a+1"), 2.},
												{QStringLiteral("a+1.5"), 2.5},
												{QStringLiteral("a!"), 1.}};

	for (auto& expr : tests)
		QCOMPARE(parser.parse(qPrintable(expr.first), "C"), expr.second);

	parser.assign_symbol("a", 0.); // only vars set to zero get removed
	parser.remove_symbol("a");
	for (auto& expr : tests)
		QVERIFY(std::isnan(parser.parse(qPrintable(expr.first), "C")));

	// longer var name
	parser.assign_symbol("sina", 1.5);
	const QVector<QPair<QString, double>> tests2{{QStringLiteral("sina"), 1.5},
												 {QStringLiteral("sina+1"), 2.5},
												 {QStringLiteral("sina+1.5"), 3.},
												 {QStringLiteral("2*sina"), 3.}};

	for (const auto& expr : tests2)
		QCOMPARE(parser.parse(qPrintable(expr.first), "C"), expr.second);

	// parse_with_vars()
	parser_var vars[] = {{"x", 1.}, {"y", 2.}};
	QCOMPARE(parser.parse_with_vars("x + y", vars, 2, "C"), 3.);
}

void ParserTest::testLocale() {
// TODO: locale test currently does not work on FreeBSD
#ifndef __FreeBSD__
	Parsing::Parser parser(false);
	const QVector<QPair<QString, double>> tests{{QStringLiteral("1,"), 1.},
												{QStringLiteral("1,5"), 1.5},
												{QStringLiteral("1+0,5"), 1.5},
												{QStringLiteral("2*1,5"), 3.}};

	for (const auto& expr : tests)
		QCOMPARE(parser.parse(qPrintable(expr.first), "de_DE"), expr.second);
#endif
}

///////////// Performance ////////////////////////////////
// see https://github.com/ArashPartow/math-parser-benchmark-project

void ParserTest::testPerformance1() {
	const int N = 1e5;

	Parsing::Parser parser(false);

	QBENCHMARK {
		for (int i = 0; i < N; i++) {
			const double x = i / 100.;
			parser.assign_symbol("x", i / 100.);
			QCOMPARE(parser.parse("x+1.", "C"), x + 1.);
		}
	}
}

void ParserTest::testPerformance2() {
	const int N = 1e5;

	Parsing::Parser parser(false);
	QBENCHMARK {
		for (int i = 0; i < N; i++) {
			parser.assign_symbol("alpha", i / 100.);
			QCOMPARE(parser.parse("sin(alpha)^2 + cos(alpha)^2", "C"), 1.);
		}
	}
}

void ParserTest::testRangeParsing() {
	Range<double> r(QString(QStringLiteral("0")), QString(QStringLiteral("2*pi")));
	QCOMPARE(r.start(), 0.);
	VALUES_EQUAL(r.end(), 2. * M_PI);
}

QTEST_MAIN(ParserTest)
