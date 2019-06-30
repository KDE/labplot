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

void TTestTest::twoSampleIndependent() {
	QVector<double> col1Data = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
	QVector<double> col2Data = {3, 1, 5, 4, 6, 4, 6, 2, 0, 5, 4, 5, 4, 3, 6, 6, 8, 5, 5, 4, 2, 5, 7, 5};

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

	FuzzyCompare(tValue, -1.713, 1.e-2);
	FuzzyCompare(pValue, 0.101, 1.e-2);
}


void TTestTest::twoSampleIndependent_data() {
}

QTEST_MAIN(TTestTest)
