/*
    File                 : HilbertTransformTest.cpp
    Project              : LabPlot
    Description          : Tests for Hilbert transform
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2021 Stefan Gerlach (stefan.gerlach@uni.kn)

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "HilbertTransformTest.h"
#include "backend/core/column/Column.h"
#include "backend/worksheet/plots/cartesian/XYHilbertTransformCurve.h"

//##############################################################################

void HilbertTransformTest::test1() {
	// data
	QVector<int> xData = {1,2,3,4};
	QVector<double> yData = {1.,2.,3.,4.};

	//data source columns
	Column xDataColumn("x", AbstractColumn::ColumnMode::Integer);
	xDataColumn.replaceInteger(0, xData);

	Column yDataColumn("y", AbstractColumn::ColumnMode::Numeric);
	yDataColumn.replaceValues(0, yData);

	XYHilbertTransformCurve curve("Hilbert Trafo");
	curve.setXDataColumn(&xDataColumn);
	curve.setYDataColumn(&yDataColumn);

	//prepare the transform
	XYHilbertTransformCurve::TransformData data = curve.transformData();
	curve.setTransformData(data);

	//perform the transform
	const XYHilbertTransformCurve::TransformResult& result = curve.transformResult();

	//check the results
	QCOMPARE(result.available, true);
	QCOMPARE(result.valid, true);

	const AbstractColumn* resultXDataColumn = curve.xColumn();
	const AbstractColumn* resultYDataColumn = curve.yColumn();

	const int np = resultXDataColumn->rowCount();
	QCOMPARE(np, 4);

	for (int i = 0; i < np; i++)
		QCOMPARE(resultXDataColumn->valueAt(i), (double)i + 1);
	for (int i = 0; i < np; i++)
		DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(i));

	QCOMPARE(resultYDataColumn->valueAt(0), 1.);
	QCOMPARE(resultYDataColumn->valueAt(1), -1.);
	QCOMPARE(resultYDataColumn->valueAt(2), -1.);
	QCOMPARE(resultYDataColumn->valueAt(3), 1.);
}

void HilbertTransformTest::test2() {
	// data
	QVector<int> xData = {1,2,3,4,5,6,7};
	QVector<double> yData = {1.,2.,3.,4.,3.,2.,1.};

	//data source columns
	Column xDataColumn("x", AbstractColumn::ColumnMode::Integer);
	xDataColumn.replaceInteger(0, xData);

	Column yDataColumn("y", AbstractColumn::ColumnMode::Numeric);
	yDataColumn.replaceValues(0, yData);

	XYHilbertTransformCurve curve("Hilbert Trafo");
	curve.setXDataColumn(&xDataColumn);
	curve.setYDataColumn(&yDataColumn);

	//prepare the transform
	XYHilbertTransformCurve::TransformData data = curve.transformData();
	curve.setTransformData(data);

	//perform the transform
	const XYHilbertTransformCurve::TransformResult& result = curve.transformResult();

	//check the results
	QCOMPARE(result.available, true);
	QCOMPARE(result.valid, true);

	const AbstractColumn* resultXDataColumn = curve.xColumn();
	const AbstractColumn* resultYDataColumn = curve.yColumn();

	const int np = resultXDataColumn->rowCount();
	QCOMPARE(np, 7);

	for (int i = 0; i < np; i++)
		QCOMPARE(resultXDataColumn->valueAt(i), (double)i + 1);
	for (int i = 0; i < np; i++)
		DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(i));

	QCOMPARE(resultYDataColumn->valueAt(0), -0.736238855198572);
	QCOMPARE(resultYDataColumn->valueAt(1), -1.22454414518711);
	QCOMPARE(resultYDataColumn->valueAt(2), -1.29334051930247);
	FuzzyCompare(resultYDataColumn->valueAt(3), 0., 1.e-15);
	QCOMPARE(resultYDataColumn->valueAt(4), -resultYDataColumn->valueAt(2));
	QCOMPARE(resultYDataColumn->valueAt(5), -resultYDataColumn->valueAt(1));
	QCOMPARE(resultYDataColumn->valueAt(6), -resultYDataColumn->valueAt(0));
}

void HilbertTransformTest::test3() {
	// data
	QVector<int> xData = {1,2,3,4,5,6,7,8};
	QVector<double> yData = {1,2,3,4,5,6,7,8};

	//data source columns
	Column xDataColumn("x", AbstractColumn::ColumnMode::Integer);
	xDataColumn.replaceInteger(0, xData);

	Column yDataColumn("y", AbstractColumn::ColumnMode::Numeric);
	yDataColumn.replaceValues(0, yData);

	XYHilbertTransformCurve curve("Hilbert trafo");
	curve.setXDataColumn(&xDataColumn);
	curve.setYDataColumn(&yDataColumn);

	//prepare the transform
	XYHilbertTransformCurve::TransformData data = curve.transformData();

	//perform the transform
	curve.setTransformData(data);

	const XYHilbertTransformCurve::TransformResult& result = curve.transformResult();

	//check the results
	QCOMPARE(result.available, true);
	QCOMPARE(result.valid, true);

	const AbstractColumn* resultXDataColumn = curve.xColumn();
	const AbstractColumn* resultYDataColumn = curve.yColumn();

	const int np = resultXDataColumn->rowCount();
	QCOMPARE(np, 8);

	for (int i = 0; i < np; i++)
		QCOMPARE(resultXDataColumn->valueAt(i), (double)i + 1);
	for (int i = 0; i < np; i++)
		DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(i));

	QCOMPARE(resultYDataColumn->valueAt(0), 3.82842712474619);
	QCOMPARE(resultYDataColumn->valueAt(1), -1.);
	QCOMPARE(resultYDataColumn->valueAt(2), -1.);
	QCOMPARE(resultYDataColumn->valueAt(3), -1.82842712474619);
	QCOMPARE(resultYDataColumn->valueAt(4), -1.82842712474619);
	QCOMPARE(resultYDataColumn->valueAt(5), -1.);
	QCOMPARE(resultYDataColumn->valueAt(6), -1.);
	QCOMPARE(resultYDataColumn->valueAt(7), 3.82842712474619);
}

void HilbertTransformTest::test4() {
	// data
	QVector<int> xData = {1,2,3,4,5,6,7,8,9};
	QVector<double> yData = {1.,-1.,2.,-2.,3.,-3.,4.,-4.,5.};

	//data source columns
	Column xDataColumn("x", AbstractColumn::ColumnMode::Integer);
	xDataColumn.replaceInteger(0, xData);

	Column yDataColumn("y", AbstractColumn::ColumnMode::Numeric);
	yDataColumn.replaceValues(0, yData);

	XYHilbertTransformCurve curve("Hilbert trafo");
	curve.setXDataColumn(&xDataColumn);
	curve.setYDataColumn(&yDataColumn);

	//prepare the transform
	XYHilbertTransformCurve::TransformData data = curve.transformData();

	//perform the transform
	curve.setTransformData(data);

	const XYHilbertTransformCurve::TransformResult& result = curve.transformResult();

	//check the results
	QCOMPARE(result.available, true);
	QCOMPARE(result.valid, true);

	const AbstractColumn* resultXDataColumn = curve.xColumn();
	const AbstractColumn* resultYDataColumn = curve.yColumn();

	const int np = resultXDataColumn->rowCount();
	QCOMPARE(np, 9);

	for (int i = 0; i < np; i++)
		QCOMPARE(resultXDataColumn->valueAt(i), (double)i + 1);
	for (int i = 0; i < np; i++)
		DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(i));

	QCOMPARE(resultYDataColumn->valueAt(0), 5.73760166175338);
	QCOMPARE(resultYDataColumn->valueAt(1), -2.91301405896714);
	QCOMPARE(resultYDataColumn->valueAt(2), 2.9964917553466);
	QCOMPARE(resultYDataColumn->valueAt(3), -2.12747459905025);
	QCOMPARE(resultYDataColumn->valueAt(4), 1.6613081372851);
	QCOMPARE(resultYDataColumn->valueAt(5), -1.10190838316691);
	QCOMPARE(resultYDataColumn->valueAt(6), -0.146025447565372);
	QCOMPARE(resultYDataColumn->valueAt(7), 0.674355652492414);
	QCOMPARE(resultYDataColumn->valueAt(8), -4.78133471812782);
}

void HilbertTransformTest::testPerformance() {

	// data
	QVector<int> xData;
	QVector<double> yData;
#ifdef HAVE_FFTW3
	const int N = 1e6;
#else	// GSL is much slower
	const int N = 2e5;
#endif
	for (int i = 0;  i < N; i++) {
		xData.append(i);
		yData.append(i % 100);
	}

	//data source columns
	Column xDataColumn("x", AbstractColumn::ColumnMode::Integer);
	xDataColumn.replaceInteger(0, xData);

	Column yDataColumn("y", AbstractColumn::ColumnMode::Numeric);
	yDataColumn.replaceValues(0, yData);

	XYHilbertTransformCurve curve("Hilbert trafo");
	curve.setXDataColumn(&xDataColumn);
	curve.setYDataColumn(&yDataColumn);

	//prepare and perform the convolution
	XYHilbertTransformCurve::TransformData data = curve.transformData();
	QBENCHMARK {
		// triggers recalculate()
		curve.setTransformData(data);
	}

	//check the results
	const XYHilbertTransformCurve::TransformResult& result = curve.transformResult();

	QCOMPARE(result.available, true);
	QCOMPARE(result.valid, true);

	const AbstractColumn* resultXDataColumn = curve.xColumn();

	const int np = resultXDataColumn->rowCount();
	QCOMPARE(np, N);
}

QTEST_MAIN(HilbertTransformTest)
