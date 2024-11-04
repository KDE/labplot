/*
	File                 : ParserTest.cpp
	Project              : LabPlot
	Description          : Tests for the Parser
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2020 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ParserTest.h"

#include "backend/gsl/parser.h"

#include <gsl/gsl_const_mksa.h>
#include <gsl/gsl_errno.h>

using namespace Parser;

//**********************************************************
//****************** Function tests ************************
//**********************************************************

void ParserTest::testBasics() {
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
		QCOMPARE(parse(qPrintable(expr.first), "C"), expr.second);

	const QVector<QPair<QString, double>> testsFuzzy{{QStringLiteral("(sin(pi))"), 0.}};

	for (const auto& expr : testsFuzzy)
		FuzzyCompare(parse(qPrintable(expr.first), "C"), expr.second, 1.e-15);
}

void ParserTest::testErrors() {
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

	for (auto& expr : testsNan)
		QVERIFY(std::isnan(parse(qPrintable(expr), "C")));

	const QVector<QString> testsInf{QStringLiteral("1/0"), QStringLiteral("-1/0"), QStringLiteral("1+1/0")};

	for (auto& expr : testsInf)
		QVERIFY(std::isinf(parse(qPrintable(expr), "C")));
}

void ParserTest::testVariables() {
	assign_symbol("a", 1.);
	const QVector<QPair<QString, double>> tests{{QStringLiteral("a"), 1.},
												{QStringLiteral("a+1"), 2.},
												{QStringLiteral("a+1.5"), 2.5},
												{QStringLiteral("a!"), 1.}};

	for (auto& expr : tests)
		QCOMPARE(parse(qPrintable(expr.first), "C"), expr.second);

	assign_symbol("a", 0.); // only vars set to zero get removed
	remove_symbol("a");
	for (auto& expr : tests)
		QVERIFY(std::isnan(parse(qPrintable(expr.first), "C")));

	// longer var name
	assign_symbol("sina", 1.5);
	const QVector<QPair<QString, double>> tests2{{QStringLiteral("sina"), 1.5},
												 {QStringLiteral("sina+1"), 2.5},
												 {QStringLiteral("sina+1.5"), 3.},
												 {QStringLiteral("2*sina"), 3.}};

	for (auto& expr : tests2)
		QCOMPARE(parse(qPrintable(expr.first), "C"), expr.second);

	// parse_with_vars()
	parser_var vars[] = {{"x", 1.}, {"y", 2.}};
	QCOMPARE(parse_with_vars("x + y", vars, 2, "C"), 3.);
}

void ParserTest::testLocale() {
// TODO: locale test currently does not work on FreeBSD
#ifndef __FreeBSD__
	const QVector<QPair<QString, double>> tests{{QStringLiteral("1,"), 1.},
												{QStringLiteral("1,5"), 1.5},
												{QStringLiteral("1+0,5"), 1.5},
												{QStringLiteral("2*1,5"), 3.}};

	for (auto& expr : tests)
		QCOMPARE(parse(qPrintable(expr.first), "de_DE"), expr.second);
#endif
}

///////////// Performance ////////////////////////////////
// see https://github.com/ArashPartow/math-parser-benchmark-project

void ParserTest::testPerformance1() {
	const int N = 1e5;

	QBENCHMARK {
		for (int i = 0; i < N; i++) {
			const double x = i / 100.;
			assign_symbol("x", i / 100.);
			QCOMPARE(parse("x+1.", "C"), x + 1.);
		}
	}
}

void ParserTest::testPerformance2() {
	const int N = 1e5;

	QBENCHMARK {
		for (int i = 0; i < N; i++) {
			assign_symbol("alpha", i / 100.);
			QCOMPARE(parse("sin(alpha)^2 + cos(alpha)^2", "C"), 1.);
		}
	}
}

QTEST_MAIN(ParserTest)
