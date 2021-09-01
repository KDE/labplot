/*
    File                 : RangeTest.cpp
    Project              : LabPlot
    Description          : Tests for Range
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2021 Stefan Gerlach (stefan.gerlach@uni.kn)
*/

/***************************************************************************
 *                                                                         *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/

#include "RangeTest.h"
#include "backend/lib/Range.h"

//**********************************************************
//****************** Function tests ************************
//**********************************************************

void RangeTest::testNiceExtend() {
	QVector< QPair<Range<double>, Range<double>> > tests{
		{{0.,.95},{0.,1.}}, {{0,.91},{0.,1.}}, {{0,.9},{0.,.9}},
		{{0.,.85},{0.,.9}}, {{0,.81},{0.,.9}}, {{0,.71},{0.,.8}},
		{{0,.46},{0.,.5}}, {{0.,.41},{0.,.45}}, {{0,.36},{0.,.4}},
		{{0,.21},{0.,.25}}, {{0,.19},{0.,.2}}, {{0.,.17},{0.,.18}},
		{{0,.15},{0.,.16}}, {{0.,.13},{0.,.14}}, {{0,995.},{0, 1000.}}
	};
	QVector< QPair<Range<double>, Range<double>> > tests2{	// QCOMPARE is too strict
		{{0.,.61},{0.,.7}}, {{0,.51},{0.,.6}}, {{0,.31},{0.,.35}}, {{0.,.26},{0.,.3}}
	};

	for (auto& test : tests) {
		DEBUG(Q_FUNC_INFO << ", " << test.first.toStdString())
		test.first.niceExtend();
		//DEBUG(std::setprecision(19) << test.first.start() << " == " << test.second.start())
		//DEBUG(std::setprecision(19) << test.first.end() << " == " << test.second.end())
		QCOMPARE(test.first, test.second);
	}
	for (auto& test : tests2) {
		DEBUG(Q_FUNC_INFO << ", " << test.first.toStdString())
		test.first.niceExtend();
		FuzzyCompare(test.first.start(), test.second.start(), DBL_EPSILON);
		FuzzyCompare(test.first.end(), test.second.end(), 1.e-15);
	}
}


void RangeTest::testTickCount() {

	const QVector<QPair<Range<double>, int>> tests{
		{{0., 1.}, 6}, {{0., 0.01}, 6}, {{0., 100.}, 6},
		{{0., 2.}, 5}, {{0., 3.}, 4}, {{0., 4.}, 5}, {{0., 5.}, 6},
		{{0., 6.}, 4}, {{0., 7.}, 8}, {{0., 8.}, 5}, {{0., 9.}, 4},
		{{1.6, 2.2}, 4}, {{1.3, 2.7}, 8}, {{0., 3.2}, 5}, {{0.4, 2.}, 5},
		{{0.5, 2.5}, 5}
		// size=29,41 should not happen when nice extended
	};

	for (auto& test : tests) {
		DEBUG(test.second);
		QCOMPARE(test.first.autoTickCount(), test.second);
	}
}

///////////// Performance ////////////////////////////////
/*
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
