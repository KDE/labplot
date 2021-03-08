/***************************************************************************
    File                 : RangeTest.cpp
    Project              : LabPlot
    Description          : Tests for Range
    --------------------------------------------------------------------
    Copyright            : (C) 2021 Stefan Gerlach (stefan.gerlach@uni.kn)
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/

#include "RangeTest.h"
#include "backend/lib/Range.h"

//**********************************************************
//****************** Function tests ************************
//**********************************************************

void RangeTest::testTickCount() {
	const QVector<QPair<Range<double>, int>> tests{
		{{0., 1.}, 6}, {{0., 0.01}, 6}, {{0., 100.}, 6},
		{{0., 2.}, 5}, {{0., 3.}, 7}, {{0., 4.}, 5}, {{0., 5.}, 6},
		{{0., 6.}, 7}, {{0., 7.}, 8}, {{0., 8.}, 9}, {{0., 9.}, 10},
		{{1.6, 2.2}, 7}, {{1.3, 2.7}, 8}, {{1., 30.}, 7}
	};
	for ( auto& test: tests) {
		DEBUG(test.second);
		QCOMPARE(test.first.autoTickCount(), test.second);
	}
}

/*void ParserTest::testVariables() {
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
}*/

QTEST_MAIN(RangeTest)
