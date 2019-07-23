/***************************************************************************
    File                 : CorrelationCoefficientTest.cpp
    Project              : LabPlot
    Description          : Unit Testing for Correlation Coefficient
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

#include "CorrelationCoefficientTest.h"
#include "backend/generalTest/CorrelationCoefficient.h"

#include "backend/core/AbstractColumn.h"
#include "backend/core/column/Column.h"

void CorrelationCoefficientTest::pearsonCoefficient_data() {
    QTest::addColumn<QVector<double>>("col1Data");
    QTest::addColumn<QVector<double>>("col2Data");
    QTest::addColumn<double>("correlationValue_expected");
    QTest::addColumn<double>("zValue_expected");

    // First Sample
    // This sample is taken from: http://learntech.uwe.ac.uk/da/Default.aspx?pageid=1442
    QVector<double> col1Data = {56, 56, 65, 65, 50, 25, 87, 44, 35};
    QVector<double> col2Data = {87, 91, 85, 91, 75, 28, 122, 66, 58};

    double correlationValue_expected = 0.96619424909;
    double zValue_expected = 0.;

    QTest::newRow("Sample 1") << col1Data << col2Data << correlationValue_expected << zValue_expected;

    // Second Sample
    // This sample is taken from:
    //      https://www.statisticshowto.datasciencecentral.com/probability-and-statistics/correlation-coefficient-formula/
    col1Data = {43, 21, 25, 42, 57, 59};
    col2Data = {99, 65, 79, 75, 87, 81};

    correlationValue_expected = 0.52980897305;
    zValue_expected = 0.;

    QTest::newRow("Sample 2") << col1Data << col2Data << correlationValue_expected << zValue_expected;

    // Third Sample
    // This sample is taken from:
    //      https://www.myaccountingcourse.com/financial-ratios/correlation-coefficient
    col1Data = {8, 8, 6, 5, 7, 6};
    col2Data = {81, 80, 75, 65, 91, 80};

    correlationValue_expected = 0.64755960039;
    zValue_expected = 0.;

    QTest::newRow("Sample 3") << col1Data << col2Data << correlationValue_expected << zValue_expected;
}

void CorrelationCoefficientTest::pearsonCoefficient() {
    QFETCH(QVector<double>, col1Data);
    QFETCH(QVector<double>, col2Data);
    QFETCH(double, correlationValue_expected);
    QFETCH(double, zValue_expected);

    Column* col1 = new Column("col1", AbstractColumn::Numeric);
    Column* col2 = new Column("col2", AbstractColumn::Numeric);

    col1->replaceValues(0, col1Data);
    col2->replaceValues(0, col2Data);

    QVector<Column*> cols;
    cols << col1 << col2;

    CorrelationCoefficient correlationCoefficientTest("Pearson's R");
    correlationCoefficientTest.setColumns(cols);

    CorrelationCoefficient::Test test;
    test = CorrelationCoefficient::Test::Pearson;

    bool categoricalVariable = false;

    correlationCoefficientTest.performTest(test, categoricalVariable);

    double correlationValue = correlationCoefficientTest.correlationValue();
    double zValue = correlationCoefficientTest.statisticValue()[0];

    QDEBUG("Correlation Value is " << correlationValue);
    QDEBUG("Correlation Value Expected is " << correlationValue_expected);
    QDEBUG("Z Value is: " << zValue);
    QDEBUG("Z Value Expected is: " << zValue_expected);

    FuzzyCompare(correlationValue, correlationValue_expected, 1.e-5);
    FuzzyCompare(zValue, zValue_expected);
}

void CorrelationCoefficientTest::kendallCoefficient_data() {
    QTest::addColumn<QVector<double>>("col1Values");
    QTest::addColumn<QVector<double>>("col2Values");
    QTest::addColumn<QVector<QString>>("col1Texts");
    QTest::addColumn<QVector<QString>>("col2Texts");
    QTest::addColumn<bool>("isDouble");
    QTest::addColumn<double>("correlationValue_expected");
    QTest::addColumn<double>("zValue_expected");

    // First Sample
    // This sample is taken from:
    //  https://www.statsdirect.com/help/nonparametric_methods/kendall_correlation.htm
    QVector<double> col1Values = {4, 10, 3, 1, 9, 2, 6, 7, 8, 5};
    QVector<double> col2Values = {5, 8, 6, 2, 10, 3, 9, 4, 7, 1};
    QVector<QString> col1Texts;
    QVector<QString> col2Texts;

    bool isDouble = true;

    double correlationValue_expected = 0.51111114025116;
    double zValue_expected = 2.05718265659;

    QTest::newRow("Sample 1") << col1Values << col2Values << col1Texts << col2Texts << isDouble << correlationValue_expected << zValue_expected;

    // Second Sample
    // This sample is taken from:
    //      https://www.statisticshowto.datasciencecentral.com/kendalls-tau/
    col1Texts = {"A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L"};
    col2Texts = {"A", "B", "D", "C", "F", "E", "H", "G", "J", "I", "L", "K"};
    col1Values = {};
    col2Values = {};

    correlationValue_expected = 0.84848484848;
    zValue_expected = 3.84676339286;

    QTest::newRow("Sample 2") << col1Values << col2Values << col1Texts << col2Texts << isDouble << correlationValue_expected << zValue_expected;
}

void CorrelationCoefficientTest::kendallCoefficient() {
    QFETCH(QVector<double>, col1Values);
    QFETCH(QVector<double>, col2Values);
    QFETCH(QVector<QString>, col1Texts);
    QFETCH(QVector<QString>, col2Texts);
    QFETCH(bool, isDouble);
    QFETCH(double, correlationValue_expected);
    QFETCH(double, zValue_expected);

    Column* col1;
    Column* col2;

    if (isDouble){
        col1 = new Column("col1", AbstractColumn::Numeric);
        col2 = new Column("col2", AbstractColumn::Numeric);

        col1->replaceValues(0, col1Values);
        col2->replaceValues(0, col2Values);
    } else {
        col1 = new Column("col1", AbstractColumn::Text);
        col2 = new Column("col2", AbstractColumn::Text);

        col1->replaceTexts(0, col1Texts);
        col2->replaceTexts(0, col2Texts);
    }

    QVector<Column*> cols;
    cols << col1 << col2;

    CorrelationCoefficient correlationCoefficientTest("Kendall's Tau");
    correlationCoefficientTest.setColumns(cols);

    CorrelationCoefficient::Test test;
    test = CorrelationCoefficient::Test::Kendall;

    bool categoricalVariable = false;
    correlationCoefficientTest.performTest(test, categoricalVariable);

    double correlationValue = correlationCoefficientTest.correlationValue();
    double zValue = correlationCoefficientTest.statisticValue()[0];

    QDEBUG("Correlation Value is " << correlationValue);
    QDEBUG("Correlation Value Expected is " << correlationValue_expected);
    QDEBUG("Z Value is: " << zValue);
    QDEBUG("Z Value Expected is: " << zValue_expected);

    FuzzyCompare(correlationValue, correlationValue_expected, 1.e-7);
    FuzzyCompare(zValue, zValue_expected, 1.e-7);
}

void CorrelationCoefficientTest::spearmanCoefficient_data() {
    QTest::addColumn<QVector<double>>("col1Data");
    QTest::addColumn<QVector<double>>("col2Data");
    QTest::addColumn<double>("correlationValue_expected");

    // First Sample
    // This sample is taken:
    //      https://statistics.laerd.com/statistical-guides/spearmans-rank-order-correlation-statistical-guide-2.php
    QVector<double> col1Data = {56, 75, 45, 71, 62, 64, 58, 80, 76, 61};
    QVector<double> col2Data = {66, 70, 40, 60, 65, 56, 59, 77, 67, 63};

    double correlationValue_expected = 0.67272727272;

    QTest::newRow("Sample 1") << col1Data << col2Data << correlationValue_expected;

    // Second Sample
    // This sample is taken from:
    //  https://en.wikipedia.org/wiki/Spearman%27s_rank_correlation_coefficient
    col1Data = {106, 86, 100, 101, 99, 103, 97, 113, 112, 110};
    col2Data = {7, 0, 27, 50, 28, 29, 20, 12, 6, 17};

    correlationValue_expected = -0.17575757575;

    QTest::newRow("Sample 2") << col1Data << col2Data << correlationValue_expected;

}

void CorrelationCoefficientTest::spearmanCoefficient() {
    QFETCH(QVector<double>, col1Data);
    QFETCH(QVector<double>, col2Data);
    QFETCH(double, correlationValue_expected);

    Column* col1 = new Column("col1", AbstractColumn::Numeric);
    Column* col2 = new Column("col2", AbstractColumn::Numeric);

    col1->replaceValues(0, col1Data);
    col2->replaceValues(0, col2Data);

    QVector<Column*> cols;
    cols << col1 << col2;

    CorrelationCoefficient correlationCoefficientTest("Spearman Rank");
    correlationCoefficientTest.setColumns(cols);

    CorrelationCoefficient::Test test;
    test = CorrelationCoefficient::Test::Spearman;

    bool categoricalVariable = false;

    correlationCoefficientTest.performTest(test, categoricalVariable);

    double correlationValue = correlationCoefficientTest.correlationValue();

    QDEBUG("Correlation Value is " << correlationValue);
    QDEBUG("Correlation Value Expected is " << correlationValue_expected);

    FuzzyCompare(correlationValue, correlationValue_expected, 1.e-5);
}

QTEST_MAIN(CorrelationCoefficientTest)
