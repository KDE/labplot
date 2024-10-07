/*
	File                 : DifferentiationTest.cpp
	Project              : LabPlot
	Description          : Tests for numerical differentiation
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2018-2022 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "DifferentiationTest.h"
#include "backend/core/column/Column.h"
#include "backend/worksheet/plots/cartesian/XYDifferentiationCurve.h"

// ##############################################################################

void DifferentiationTest::testLinear() {
	// data
	QVector<int> xData = {1, 2, 3, 4};
	QVector<double> yData = {1., 2., 3., 4.};

	// data source columns
	Column xDataColumn(QStringLiteral("x"), AbstractColumn::ColumnMode::Integer);
	xDataColumn.replaceInteger(0, xData);

	Column yDataColumn(QStringLiteral("y"), AbstractColumn::ColumnMode::Double);
	yDataColumn.replaceValues(0, yData);

	XYDifferentiationCurve differentiationCurve(QStringLiteral("differentiation"));
	differentiationCurve.setXDataColumn(&xDataColumn);
	differentiationCurve.setYDataColumn(&yDataColumn);

	// prepare the differentiation
	XYDifferentiationCurve::DifferentiationData differentiationData = differentiationCurve.differentiationData();
	differentiationCurve.setDifferentiationData(differentiationData);

	// perform the differentiation
	differentiationCurve.recalculate();
	const XYDifferentiationCurve::DifferentiationResult& differentiationResult = differentiationCurve.differentiationResult();

	// check the results
	QCOMPARE(differentiationResult.available, true);
	QCOMPARE(differentiationResult.valid, true);

	const AbstractColumn* resultXDataColumn = differentiationCurve.xColumn();
	const AbstractColumn* resultYDataColumn = differentiationCurve.yColumn();

	const int np = resultXDataColumn->rowCount();
	QCOMPARE(np, 4);

	for (int i = 0; i < np; i++)
		QCOMPARE(resultXDataColumn->valueAt(i), (double)i + 1);

	QCOMPARE(resultYDataColumn->valueAt(0), 1.);
	QCOMPARE(resultYDataColumn->valueAt(1), 1.);
	QCOMPARE(resultYDataColumn->valueAt(2), 1.);
	QCOMPARE(resultYDataColumn->valueAt(3), 1.);
}

void DifferentiationTest::testLinearNonEquidistant() {
	// data
	QVector<double> xData = {1., 1.5, 3., 5.};
	QVector<double> yData = {1., 1.5, 3., 5.};

	// data source columns
	Column xDataColumn(QStringLiteral("x"), AbstractColumn::ColumnMode::Double);
	xDataColumn.replaceValues(0, xData);

	Column yDataColumn(QStringLiteral("y"), AbstractColumn::ColumnMode::Double);
	yDataColumn.replaceValues(0, yData);

	XYDifferentiationCurve differentiationCurve(QStringLiteral("differentiation"));
	differentiationCurve.setXDataColumn(&xDataColumn);
	differentiationCurve.setYDataColumn(&yDataColumn);

	// prepare the differentiation
	XYDifferentiationCurve::DifferentiationData differentiationData = differentiationCurve.differentiationData();
	differentiationCurve.setDifferentiationData(differentiationData);

	// perform the differentiation
	differentiationCurve.recalculate();
	const XYDifferentiationCurve::DifferentiationResult& differentiationResult = differentiationCurve.differentiationResult();

	// check the results
	QCOMPARE(differentiationResult.available, true);
	QCOMPARE(differentiationResult.valid, true);

	const AbstractColumn* resultXDataColumn = differentiationCurve.xColumn();
	const AbstractColumn* resultYDataColumn = differentiationCurve.yColumn();

	const int np = resultXDataColumn->rowCount();
	QCOMPARE(np, 4);

	for (int i = 0; i < np; i++)
		QCOMPARE(resultXDataColumn->valueAt(i), xData[i]);

	QCOMPARE(resultYDataColumn->valueAt(0), 1.);
	QCOMPARE(resultYDataColumn->valueAt(1), 1.);
	QCOMPARE(resultYDataColumn->valueAt(2), 1.);
	QCOMPARE(resultYDataColumn->valueAt(3), 1.);
}

void DifferentiationTest::testQuadratic() {
	// data
	QVector<double> xData = {1., 2., 3., 4.};
	QVector<double> yData = {1., 4., 9., 16.};

	// data source columns
	Column xDataColumn(QStringLiteral("x"), AbstractColumn::ColumnMode::Double);
	xDataColumn.replaceValues(0, xData);

	Column yDataColumn(QStringLiteral("y"), AbstractColumn::ColumnMode::Double);
	yDataColumn.replaceValues(0, yData);

	XYDifferentiationCurve differentiationCurve(QStringLiteral("differentiation"));
	differentiationCurve.setXDataColumn(&xDataColumn);
	differentiationCurve.setYDataColumn(&yDataColumn);

	// prepare the differentiation
	XYDifferentiationCurve::DifferentiationData differentiationData = differentiationCurve.differentiationData();
	differentiationCurve.setDifferentiationData(differentiationData);

	// perform the differentiation
	differentiationCurve.recalculate();
	const XYDifferentiationCurve::DifferentiationResult& differentiationResult = differentiationCurve.differentiationResult();

	// check the results
	QCOMPARE(differentiationResult.available, true);
	QCOMPARE(differentiationResult.valid, true);

	const AbstractColumn* resultXDataColumn = differentiationCurve.xColumn();
	const AbstractColumn* resultYDataColumn = differentiationCurve.yColumn();

	const int np = resultXDataColumn->rowCount();
	QCOMPARE(np, 4);

	for (int i = 0; i < np; i++)
		QCOMPARE(resultXDataColumn->valueAt(i), xData[i]);

	QCOMPARE(resultYDataColumn->valueAt(0), 2.);
	QCOMPARE(resultYDataColumn->valueAt(1), 4.);
	QCOMPARE(resultYDataColumn->valueAt(2), 6.);
	QCOMPARE(resultYDataColumn->valueAt(3), 8.);
}

void DifferentiationTest::testQuadraticNonEquidistant() {
	// data
	QVector<double> xData = {1., 1.5, 3., 5.};
	QVector<double> yData = {1., 2.25, 9., 25.};

	// data source columns
	Column xDataColumn(QStringLiteral("x"), AbstractColumn::ColumnMode::Double);
	xDataColumn.replaceValues(0, xData);

	Column yDataColumn(QStringLiteral("y"), AbstractColumn::ColumnMode::Double);
	yDataColumn.replaceValues(0, yData);

	XYDifferentiationCurve differentiationCurve(QStringLiteral("differentiation"));
	differentiationCurve.setXDataColumn(&xDataColumn);
	differentiationCurve.setYDataColumn(&yDataColumn);

	// prepare the differentiation
	XYDifferentiationCurve::DifferentiationData differentiationData = differentiationCurve.differentiationData();
	differentiationCurve.setDifferentiationData(differentiationData);

	// perform the differentiation
	differentiationCurve.recalculate();
	const XYDifferentiationCurve::DifferentiationResult& differentiationResult = differentiationCurve.differentiationResult();

	// check the results
	QCOMPARE(differentiationResult.available, true);
	QCOMPARE(differentiationResult.valid, true);

	const AbstractColumn* resultXDataColumn = differentiationCurve.xColumn();
	const AbstractColumn* resultYDataColumn = differentiationCurve.yColumn();

	const int np = resultXDataColumn->rowCount();
	QCOMPARE(np, 4);

	for (int i = 0; i < np; i++)
		QCOMPARE(resultXDataColumn->valueAt(i), xData[i]);

	QCOMPARE(resultYDataColumn->valueAt(0), 2.);
	QCOMPARE(resultYDataColumn->valueAt(1), 3.);
	QCOMPARE(resultYDataColumn->valueAt(2), 6.);
	QCOMPARE(resultYDataColumn->valueAt(3), 10.);
}

void DifferentiationTest::testQuadraticSecondOrder() {
	// data
	QVector<double> xData = {1., 2., 3., 4.};
	QVector<double> yData = {1., 4., 9., 16.};

	// data source columns
	Column xDataColumn(QStringLiteral("x"), AbstractColumn::ColumnMode::Double);
	xDataColumn.replaceValues(0, xData);

	Column yDataColumn(QStringLiteral("y"), AbstractColumn::ColumnMode::Double);
	yDataColumn.replaceValues(0, yData);

	XYDifferentiationCurve differentiationCurve(QStringLiteral("differentiation"));
	differentiationCurve.setXDataColumn(&xDataColumn);
	differentiationCurve.setYDataColumn(&yDataColumn);

	// prepare the differentiation
	XYDifferentiationCurve::DifferentiationData differentiationData = differentiationCurve.differentiationData();
	differentiationData.derivOrder = nsl_diff_deriv_order_second;
	differentiationCurve.setDifferentiationData(differentiationData);

	// perform the differentiation
	differentiationCurve.recalculate();
	const XYDifferentiationCurve::DifferentiationResult& differentiationResult = differentiationCurve.differentiationResult();

	// check the results
	QCOMPARE(differentiationResult.available, true);
	QCOMPARE(differentiationResult.valid, true);

	const AbstractColumn* resultXDataColumn = differentiationCurve.xColumn();
	const AbstractColumn* resultYDataColumn = differentiationCurve.yColumn();

	const int np = resultXDataColumn->rowCount();
	QCOMPARE(np, 4);

	for (int i = 0; i < np; i++)
		QCOMPARE(resultXDataColumn->valueAt(i), xData[i]);

	QCOMPARE(resultYDataColumn->valueAt(0), 2.);
	QCOMPARE(resultYDataColumn->valueAt(1), 2.);
	QCOMPARE(resultYDataColumn->valueAt(2), 2.);
	QCOMPARE(resultYDataColumn->valueAt(3), 2.);
}

void DifferentiationTest::testCubicSecondOrder() {
	// data
	QVector<double> xData = {1., 2., 3., 4.};
	QVector<double> yData = {1., 8., 27., 64.};

	// data source columns
	Column xDataColumn(QStringLiteral("x"), AbstractColumn::ColumnMode::Double);
	xDataColumn.replaceValues(0, xData);

	Column yDataColumn(QStringLiteral("y"), AbstractColumn::ColumnMode::Double);
	yDataColumn.replaceValues(0, yData);

	XYDifferentiationCurve differentiationCurve(QStringLiteral("differentiation"));
	differentiationCurve.setXDataColumn(&xDataColumn);
	differentiationCurve.setYDataColumn(&yDataColumn);

	// prepare the differentiation
	XYDifferentiationCurve::DifferentiationData differentiationData = differentiationCurve.differentiationData();
	differentiationData.derivOrder = nsl_diff_deriv_order_second;
	differentiationCurve.setDifferentiationData(differentiationData);

	// perform the differentiation
	differentiationCurve.recalculate();
	const XYDifferentiationCurve::DifferentiationResult& differentiationResult = differentiationCurve.differentiationResult();

	// check the results
	QCOMPARE(differentiationResult.available, true);
	QCOMPARE(differentiationResult.valid, true);

	const AbstractColumn* resultXDataColumn = differentiationCurve.xColumn();
	const AbstractColumn* resultYDataColumn = differentiationCurve.yColumn();

	const int np = resultXDataColumn->rowCount();
	QCOMPARE(np, 4);

	for (int i = 0; i < np; i++)
		QCOMPARE(resultXDataColumn->valueAt(i), xData[i]);

	QCOMPARE(resultYDataColumn->valueAt(0), 6.);
	QCOMPARE(resultYDataColumn->valueAt(1), 12.);
	QCOMPARE(resultYDataColumn->valueAt(2), 18.);
	QCOMPARE(resultYDataColumn->valueAt(3), 24.);
}

void DifferentiationTest::testCubicThirdOrder() {
	// data
	QVector<double> xData = {1., 2., 3., 4., 5.};
	QVector<double> yData = {1., 8., 27., 64., 125.};

	// data source columns
	Column xDataColumn(QStringLiteral("x"), AbstractColumn::ColumnMode::Double);
	xDataColumn.replaceValues(0, xData);

	Column yDataColumn(QStringLiteral("y"), AbstractColumn::ColumnMode::Double);
	yDataColumn.replaceValues(0, yData);

	XYDifferentiationCurve differentiationCurve(QStringLiteral("differentiation"));
	differentiationCurve.setXDataColumn(&xDataColumn);
	differentiationCurve.setYDataColumn(&yDataColumn);

	// prepare the differentiation
	XYDifferentiationCurve::DifferentiationData differentiationData = differentiationCurve.differentiationData();
	differentiationData.derivOrder = nsl_diff_deriv_order_third;
	differentiationCurve.setDifferentiationData(differentiationData);

	// perform the differentiation
	differentiationCurve.recalculate();
	const XYDifferentiationCurve::DifferentiationResult& differentiationResult = differentiationCurve.differentiationResult();

	// check the results
	QCOMPARE(differentiationResult.available, true);
	QCOMPARE(differentiationResult.valid, true);

	const AbstractColumn* resultXDataColumn = differentiationCurve.xColumn();
	const AbstractColumn* resultYDataColumn = differentiationCurve.yColumn();

	const int np = resultXDataColumn->rowCount();
	QCOMPARE(np, 5);

	for (int i = 0; i < np; i++)
		QCOMPARE(resultXDataColumn->valueAt(i), xData[i]);

	QCOMPARE(resultYDataColumn->valueAt(0), 6.);
	QCOMPARE(resultYDataColumn->valueAt(1), 6.);
	QCOMPARE(resultYDataColumn->valueAt(2), 6.);
	QCOMPARE(resultYDataColumn->valueAt(3), 6.);
	QCOMPARE(resultYDataColumn->valueAt(4), 6.);
}

void DifferentiationTest::testLinearDuplicateX() {
	// data
	QVector<int> xData = {1, 1, 2, 2, 3, 4, 4, 4, 5, 5};
	QVector<double> yData = {1., 2., 3., 4., 5., 6., 7., 8., 9., 10.};

	// data source columns
	Column xDataColumn(QStringLiteral("x"), AbstractColumn::ColumnMode::Integer);
	xDataColumn.replaceInteger(0, xData);

	Column yDataColumn(QStringLiteral("y"), AbstractColumn::ColumnMode::Double);
	yDataColumn.replaceValues(0, yData);

	XYDifferentiationCurve differentiationCurve(QStringLiteral("differentiation"));
	differentiationCurve.setXDataColumn(&xDataColumn);
	differentiationCurve.setYDataColumn(&yDataColumn);

	// prepare the differentiation
	XYDifferentiationCurve::DifferentiationData differentiationData = differentiationCurve.differentiationData();
	differentiationCurve.setDifferentiationData(differentiationData);

	// perform the differentiation
	differentiationCurve.recalculate();
	const XYDifferentiationCurve::DifferentiationResult& differentiationResult = differentiationCurve.differentiationResult();

	/*for (int i = 0; i < 5; i++)
		QCOMPARE(xDataColumn.valueAt(i), (double)i + 1);

	QCOMPARE(yDataColumn.valueAt(0), 1.5);
	QCOMPARE(yDataColumn.valueAt(1), 3.5);
	QCOMPARE(yDataColumn.valueAt(2), 5.);
	QCOMPARE(yDataColumn.valueAt(3), 7.);
	QCOMPARE(yDataColumn.valueAt(4), 9.5);
	*/

	// check the results
	QCOMPARE(differentiationResult.available, true);
	QCOMPARE(differentiationResult.valid, true);

	const AbstractColumn* resultXDataColumn = differentiationCurve.xColumn();
	const AbstractColumn* resultYDataColumn = differentiationCurve.yColumn();

	const int np = resultXDataColumn->rowCount();
	QCOMPARE(np, 5);

	for (int i = 0; i < np; i++)
		QCOMPARE(resultXDataColumn->valueAt(i), (double)i + 1);

	QCOMPARE(resultYDataColumn->valueAt(0), 2.25);
	QCOMPARE(resultYDataColumn->valueAt(1), 1.75);
	QCOMPARE(resultYDataColumn->valueAt(2), 3.);
	QCOMPARE(resultYDataColumn->valueAt(3), 2.5);
	QCOMPARE(resultYDataColumn->valueAt(4), -1.5);
}

void DifferentiationTest::testRecalculation() {
	// data
	QVector<int> xData = {1, 2, 3, 4};
	QVector<double> yData = {1., 2., 3., 4.};

		   // data source columns
	Column xDataColumn(QStringLiteral("x"), AbstractColumn::ColumnMode::Integer);
	xDataColumn.replaceInteger(0, xData);

	Column yDataColumn(QStringLiteral("y"), AbstractColumn::ColumnMode::Double);
	yDataColumn.replaceValues(0, yData);

	XYDifferentiationCurve differentiationCurve(QStringLiteral("differentiation"));
	differentiationCurve.setXDataColumn(&xDataColumn);
	differentiationCurve.setYDataColumn(&yDataColumn);

		   // prepare the differentiation
	XYDifferentiationCurve::DifferentiationData differentiationData = differentiationCurve.differentiationData();
	differentiationCurve.setDifferentiationData(differentiationData);

	// perform the differentiation
	//differentiationCurve.recalculate();
	const XYDifferentiationCurve::DifferentiationResult& differentiationResult = differentiationCurve.differentiationResult();

		   // check the results
	QCOMPARE(differentiationResult.available, true);
	QCOMPARE(differentiationResult.valid, true);

	const AbstractColumn* resultXDataColumn = differentiationCurve.xColumn();
	const AbstractColumn* resultYDataColumn = differentiationCurve.yColumn();

	const int np = resultXDataColumn->rowCount();
	QCOMPARE(np, 4);

	for (int i = 0; i < np; i++)
		QCOMPARE(resultXDataColumn->valueAt(i), (double)i + 1);

	QCOMPARE(resultYDataColumn->valueAt(0), 1.);
	QCOMPARE(resultYDataColumn->valueAt(1), 1.);
	QCOMPARE(resultYDataColumn->valueAt(2), 1.);
	QCOMPARE(resultYDataColumn->valueAt(3), 1.);

	yDataColumn.replaceValues(0, {4, 3, 2, 1});
	// Automatic recalculation done

	for (int i = 0; i < np; i++)
		QCOMPARE(resultXDataColumn->valueAt(i), (double)i + 1);
	QCOMPARE(resultYDataColumn->valueAt(0), -1.);
	QCOMPARE(resultYDataColumn->valueAt(1), -1.);
	QCOMPARE(resultYDataColumn->valueAt(2), -1.);
	QCOMPARE(resultYDataColumn->valueAt(3), -1.);
}

QTEST_MAIN(DifferentiationTest)
