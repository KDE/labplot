/***************************************************************************
	File                 : AnovaTest.cpp
	Project              : LabPlot
	Description          : Tests for data correlation
	--------------------------------------------------------------------
	Copyright            : (C) 2019 Devanshu Agarwal (agarwaldevanshu8@gmail.com)
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,     *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                 *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/

#include "AnovaTest.h"
#include "backend/generalTest/HypothesisTest.h"

#include "backend/core/AbstractColumn.h"
#include "backend/core/column/Column.h"

void AnovaTest::oneWayAnova_data() {
	QTest::addColumn<QVector<QString>>("col1Data");
	QTest::addColumn<QVector<double>>("col2Data");
	QTest::addColumn<double>("fValue_expected");
	QTest::addColumn<double>("pValue_expected");

	// First Sample
	QVector<QString> col1Data = {"1", "1", "1", "1", "1", "1", "1", "1", "1", "1", "1", "1", "1", "1", "1", "1", "1", "1", "1", "1",
								 "2", "2", "2", "2", "2", "2", "2", "2", "2", "2", "2", "2", "2", "2", "2", "2", "2", "2", "2", "2",
								 "3", "3", "3", "3", "3", "3", "3", "3", "3", "3", "3", "3", "3", "3", "3", "3", "3", "3", "3", "3",
								 "4", "4", "4", "4", "4", "4", "4", "4", "4", "4", "4", "4", "4", "4", "4", "4", "4", "4", "4", "4",
								 "5", "5", "5", "5", "5", "5", "5", "5", "5", "5", "5", "5", "5", "5", "5", "5", "5", "5", "5", "5"
								};

	QVector<double> col2Data = {1, 43, 15, 40, 8, 17, 30, 34, 34, 26, 1, 7, 22, 30, 40, 15, 20, 9, 14, 15,
								6, 30, 15, 30, 12, 17, 21, 23, 20, 27, -19, -18, -8, -6, -6, -9, -17, -12, -11, -6,
								5, 8, 12, 19, 8, 15, 21, 28, 26, 27, -10, 6, 4, 3, 0, 4, 9, -5, 7, 13,
								38, 20, 20, 28, 11, 17, 15, 27, 24, 23, 28, 26, 34, 32, 24, 29, 30, 24, 34, 23,
								-5, -12, -15, -4, -2, -6, -2, -7, -10, -15, -13, -16, -23, -22, -9, -18, -17, -15, -14, -15
							   };
	double fValue_expected = 33.1288915411;
	double pValue_expected = 0;

	QTest::newRow("First Sample") << col1Data << col2Data << fValue_expected << pValue_expected;
}

void AnovaTest::oneWayAnova() {
	QFETCH(QVector<QString>, col1Data);
	QFETCH(QVector<double>, col2Data);
	QFETCH(double, fValue_expected);
	QFETCH(double, pValue_expected);

	Column* col1 = new Column("col1", AbstractColumn::ColumnMode::Text);
	Column* col2 = new Column("col2", AbstractColumn::ColumnMode::Numeric);

	col1->replaceTexts(0, col1Data);
	col2->replaceValues(0, col2Data);

	QVector<Column*> cols;
	cols << col1 << col2;

	HypothesisTest anovaTest("One Way Anova");
	anovaTest.setColumns(cols);

	int test;
	test = HypothesisTest::Anova;
	test |= HypothesisTest::OneWay;
	anovaTest.setTail(HypothesisTest::Two);

	bool categoricalVariable = true;
	bool equalVariance = true;

	anovaTest.performTest(test, categoricalVariable, equalVariance);

	double fValue = anovaTest.statisticValue()[0];
	double pValue = anovaTest.pValue()[0];

	QDEBUG("fValue is " << fValue);
	QDEBUG("pValue is: " << pValue);
	QDEBUG("fValue_expected is " << fValue_expected);
	QDEBUG("pValue_expected is: " << pValue_expected);

	FuzzyCompare(fValue, fValue_expected, 1.e-5);
	FuzzyCompare(pValue, pValue_expected, 1.e-5);
}

void AnovaTest::twoWayAnova_data() {
	QTest::addColumn<QVector<QString>>("col1Data");
	QTest::addColumn<QVector<QString>>("col2Data");
	QTest::addColumn<QVector<double>>("col3Data");
	QTest::addColumn<double>("fCol1Value_expected");
	QTest::addColumn<double>("fCol2Value_expected");
	QTest::addColumn<double>("fInteractionValue_expected");
	QTest::addColumn<double>("pCol1Value_expected");
	QTest::addColumn<double>("pCol2Value_expected");

	// First Sample
	// This data set is taken from: http://statweb.stanford.edu/~susan/courses/s141/exanova.pdf
	QVector<QString> col1Data = {"Super", "Super", "Super", "Super", "Super", "Super", "Super", "Super", "Super", "Super", "Super", "Super", "Best", "Best", "Best", "Best", "Best", "Best", "Best", "Best", "Best", "Best", "Best", "Best"};
	QVector<QString> col2Data = {"cold", "cold", "cold", "cold", "warm", "warm", "warm", "warm", "hot", "hot", "hot", "hot", "cold", "cold", "cold", "cold", "warm", "warm", "warm", "warm", "hot", "hot", "hot", "hot"};
	QVector<double> col3Data = {4, 5, 6, 5, 7, 9, 8, 12, 10, 12, 11, 9, 6, 6, 4, 4, 13, 15, 12, 12, 12, 13, 10, 13};
	double fCol1Value_expected =  9.80885214008;
	double fCol2Value_expected = 48.7193579767;
	double fInteractionValue_expected = 3.97227626459;
	double pCol1Value_expected =  0.005758;
	double pCol2Value_expected = 5.44e-08;
	//    double pInteractionValue_expected =  0.037224;

	QTest::newRow("First Sample") << col1Data << col2Data << col3Data <<
												 fCol1Value_expected << fCol2Value_expected << fInteractionValue_expected <<
												 pCol1Value_expected << pCol2Value_expected;
}

//TODO: check for pValue. In document probabilty is Pr(>F)
void AnovaTest::twoWayAnova() {
	QFETCH(QVector<QString>, col1Data);
	QFETCH(QVector<QString>, col2Data);
	QFETCH(QVector<double>, col3Data);
	QFETCH(double, fCol1Value_expected);
	QFETCH(double, fCol2Value_expected);
	QFETCH(double, fInteractionValue_expected);
	QFETCH(double, pCol1Value_expected);
	QFETCH(double, pCol2Value_expected);

	Column* col1 = new Column("col1", AbstractColumn::ColumnMode::Text);
	Column* col2 = new Column("col2", AbstractColumn::ColumnMode::Text);
	Column* col3 = new Column("col3", AbstractColumn::ColumnMode::Numeric);

	col1->replaceTexts(0, col1Data);
	col2->replaceTexts(0, col2Data);
	col3->replaceValues(0, col3Data);

	QVector<Column*> cols;
	cols << col1 << col2 << col3;

	HypothesisTest anovaTest("Two Way Anova");
	anovaTest.setColumns(cols);

	int test;
	test = HypothesisTest::Anova;
	test |= HypothesisTest::TwoWay;
	anovaTest.setTail(HypothesisTest::Two);

	anovaTest.performTest(test);
	double fCol1Value = anovaTest.statisticValue()[0];
	double fCol2Value = anovaTest.statisticValue()[1];
	double fInteractionValue = anovaTest.statisticValue()[2];

	double pCol1Value = anovaTest.pValue()[0];
	double pCol2Value = anovaTest.pValue()[1];

	QDEBUG("size of statistic value is " << anovaTest.statisticValue().size());
	QDEBUG("fCol1Value is " << fCol1Value);
	QDEBUG("fCol1Value_expected is " << fCol1Value_expected);
	QDEBUG("fCol2Value is " << fCol2Value);
	QDEBUG("fCol2Value_expected is " << fCol2Value_expected);
	QDEBUG("fInteractionValue is " << fInteractionValue);
	QDEBUG("fInteractionValue_expected is " << fInteractionValue_expected);

	QDEBUG("pCol1Value is " << pCol1Value);
	QDEBUG("pCol1Value_expected is " << pCol1Value_expected);
	QDEBUG("pCol2Value is " << pCol2Value);
	QDEBUG("pCol2Value_expected is " << pCol2Value_expected);

	FuzzyCompare(fCol1Value, fCol1Value_expected, 1.e-5);
	FuzzyCompare(fCol2Value, fCol2Value_expected, 1.e-5);
	FuzzyCompare(fInteractionValue, fInteractionValue_expected, 1.e-5);
	FuzzyCompare(pCol1Value, pCol1Value_expected, 1.e-5);
	FuzzyCompare(pCol2Value, pCol2Value_expected, 1.e-5);
}

QTEST_MAIN(AnovaTest)
