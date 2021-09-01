/*
    File                 : ParserTest.cpp
    Project              : LabPlot
    Description          : Tests for the Parser
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2020 Stefan Gerlach (stefan.gerlach@uni.kn)
*/

/***************************************************************************
 *                                                                         *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/

#include "ParserTest.h"

extern "C" {
#include "backend/gsl/parser.h"
}

#include <gsl/gsl_errno.h>
#include <gsl/gsl_const_mksa.h>

//**********************************************************
//****************** Function tests ************************
//**********************************************************

void ParserTest::testBasics() {
	const QVector<QPair<QString, double>> tests{ 
		{"42", 42.}, {"1.", 1.}, {"1+1", 2.}, {"1+2+3+4+5", 15.}, {"2*3", 6.}, { "3/2", 1.5}, {"2 -4 +6 -1 -1- 0 +8", 10.},
		{"1/3+1/3+1/3", 1.}, {"1.5 + 2.5", 4.}, {"4*2.5 + 8.5+1.5 / 3.0", 19.}, {"5.0005 + 0.0095", 5.01},
		{"pi", M_PI}, {"e", M_E}, {"e^1", M_E},  {"hbar", GSL_CONST_MKSA_PLANCKS_CONSTANT_HBAR},
		{" 1   +   1  ", 2.}, {"1-    2", -1.}, {"2*    2.5", 5.}, {"3 + 8/5 -1 -2*5", -6.4},
		{"(1)", 1.}, {"-(1)", -1.}, {"(1+1)", 2}, {"(sin(0))", 0.} ,{"(( ((2)) + 4))*((5))", 30.},
		{"2^2", 4.}, {"3**2", 9.}, {"1%1", 0.}, {"3%2", 1.},
		{"1.e-5", 1.e-5}, {"9.5E3", 9500.}, {"|1.5|", 1.5}, {"|-2.5|", 2.5}, {"0!", 1}, {"4!", 24}, {"-3!", -6.},
		{"exp(0)", 1.}, {"exp(1)", M_E}, {"sqrt(0)", 0.}, {"sin(0)", 0.}, {"cos(pi)", -1.}
	};

	for ( auto& expr: tests)
		QCOMPARE(parse(qPrintable(expr.first), "C"), expr.second);

	const QVector<QPair<QString, double>> testsFuzzy{ 
		{"(sin(pi))", 0.}
	};

	for ( auto& expr: testsFuzzy)
		FuzzyCompare(parse(qPrintable(expr.first), "C"), expr.second, 1.e-15);
}

void ParserTest::testErrors() {
	gsl_set_error_handler_off();	// do not crash

	const QVector<QString> testsNan{
		"", "a", "1+", "a+1", "&", "%", "+", "*", "/", "{1}", "{1*2}", "(1+1))", "a/0", "0/0", "1/0 + a",
		"sqrt(-1)", "log(-1)", "log(0)", "asin(2)"
	};

	for ( auto& expr: testsNan)
		QVERIFY(qIsNaN(parse(qPrintable(expr), "C")));
	
	const QVector<QString> testsInf{
		"1/0", "-1/0",  "1+1/0"
	};

	for ( auto& expr: testsInf)
		QVERIFY(qIsInf(parse(qPrintable(expr), "C")));
}

void ParserTest::testVariables() {
	assign_symbol("a", 1.);
	const QVector<QPair<QString, double>> tests{ 
		{"a", 1.}, {"a+1", 2.}, {"a+1.5", 2.5}, {"a!", 1.}
	};

	for ( auto& expr: tests)
		QCOMPARE(parse(qPrintable(expr.first), "C"), expr.second);

	assign_symbol("a", 0.);	// only vars set to zero get removed
	remove_symbol("a");
	for ( auto& expr: tests)
		QVERIFY(qIsNaN(parse(qPrintable(expr.first), "C")));

	// longer var name
	assign_symbol("sina", 1.5);
	const QVector<QPair<QString, double>> tests2{ 
		{"sina", 1.5}, {"sina+1", 2.5}, {"sina+1.5", 3.}, {"2*sina", 3.}
	};

	for ( auto& expr: tests2)
		QCOMPARE(parse(qPrintable(expr.first), "C"), expr.second);

	//parse_with_vars()
	parser_var vars[] = { {"x", 1.}, {"y", 2.}};
	QCOMPARE(parse_with_vars("x + y", vars, 2, "C"), 3.);
}

void ParserTest::testLocale() {
//TODO: locale test currently does not work on FreeBSD
#ifndef __FreeBSD__
	const QVector<QPair<QString, double>> tests{ 
		{"1,", 1.}, {"1,5", 1.5}, {"1+0,5", 1.5}, {"2*1,5", 3.}
	};

	for ( auto& expr: tests)
		QCOMPARE(parse(qPrintable(expr.first), "de_DE"), expr.second);
#endif
}

///////////// Performance ////////////////////////////////
// see https://github.com/ArashPartow/math-parser-benchmark-project

void ParserTest::testPerformance1() {
	const int N = 1e5;
	
	QBENCHMARK {
		for (int i = 0; i < N; i++) {
			const double x = i/100.;
			assign_symbol("x", i/100.);
			QCOMPARE(parse("x+1.", "C"), x+1.);
		}
	}
}

void ParserTest::testPerformance2() {
	const int N = 1e5;
	
	QBENCHMARK {
		for (int i = 0; i < N; i++) {
			assign_symbol("alpha", i/100.);
			QCOMPARE(parse("sin(alpha)^2 + cos(alpha)^2", "C"), 1.);
		}
	}
}

QTEST_MAIN(ParserTest)
