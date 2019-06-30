///***************************************************************************
//    File                 : CorrelationTest.cpp
//    Project              : LabPlot
//    Description          : Tests for data correlation
//    --------------------------------------------------------------------
//    Copyright            : (C) 2019 Devanshu Agarwal (agarwaldevanshu8@gmail.com)
// ***************************************************************************/

///***************************************************************************
// *                                                                         *
// *  This program is free software; you can redistribute it and/or modify   *
// *  it under the terms of the GNU General Public License as published by   *
// *  the Free Software Foundation; either version 2 of the License, or      *
// *  (at your option) any later version.                                    *
// *                                                                         *
// *  This program is distributed in the hope that it will be useful,        *
// *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
// *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
// *  GNU General Public License for more details.                           *
// *                                                                         *
// *   You should have received a copy of the GNU General Public License     *
// *   along with this program; if not, write to the Free Software           *
// *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
// *   Boston, MA  02110-1301  USA                                           *
// *                                                                         *
// ***************************************************************************/

#include "TTestTest.h"
#include "backend/hypothesisTest/HypothesisTest.h"

#include "backend/core/AbstractColumn.h"
#include "backend/core/column/Column.h"

void TTestTest::twoSampleIndependent_data() {
	QTest::addColumn<QVector<double>>("col1Data");
	QTest::addColumn<QVector<double>>("col2Data");
	QTest::addColumn<double>("tValue_expected");
	QTest::addColumn<double>("pValue_expected");

	QVector<double> col1Data = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
	QVector<double> col2Data = {3, 1, 5, 4, 6, 4, 6, 2, 0, 5, 4, 5, 4, 3, 6, 6, 8, 5, 5, 4, 2, 5, 7, 5};
	QTest::newRow("ttest1") << col1Data << col2Data << -1.713 << 0.101;

	col1Data = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	col2Data = {42, 46, 43, 10, 55, 17, 26, 60, 62, 53, 37, 42, 33, 37, 41, 42, 19, 55, 54, 28, 20, 48, 85, 24, 56, 43, 59, 58, 52, 71, 62, 43, 54, 49, 57, 61, 33, 44, 46, 67, 43, 49, 57, 53};
	QTest::newRow("ttest2") << col1Data << col2Data << -2.26 << 0.028;
}

void TTestTest::twoSampleIndependent() {

	QFETCH(QVector<double>, col1Data);
	QFETCH(QVector<double>, col2Data);
	QFETCH(double, tValue_expected);
	QFETCH(double, pValue_expected);

	Column* col1 = new Column("col1", AbstractColumn::Numeric);
	Column* col2 = new Column("col2", AbstractColumn::Numeric);

	col1->replaceValues(0, col1Data);
	col2->replaceValues(0, col2Data);

	QVector<Column*> cols;
	cols << col1 << col2;

	HypothesisTest tTest("Test");
	tTest.setColumns(cols);

	HypothesisTest::Test test;
	test.type = HypothesisTest::Test::Type::TTest;
	test.subtype = HypothesisTest::Test::SubType::TwoSampleIndependent;
	test.tail = HypothesisTest::Test::Tail::Two;

	bool categoricalVariable = true;
	bool equalVariance = true;

	tTest.performTest(test, categoricalVariable, equalVariance);
	double tValue = tTest.statisticValue();
	double pValue = tTest.pValue();

	qDebug() << "tValue is " << tValue;
	qDebug() << "pValue is: " << pValue;
	qDebug() << "tValue_expected is " << tValue_expected;
	qDebug() << "pValue_expected is: " << pValue_expected;

	FuzzyCompare(tValue, tValue_expected, 0.1);
	FuzzyCompare(pValue, pValue_expected, 0.1);
}

QTEST_MAIN(TTestTest)
