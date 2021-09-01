/*
    File                 : ConvolutionTest.cpp
    Project              : LabPlot
    Description          : Tests for data convolution
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2018 Stefan Gerlach (stefan.gerlach@uni.kn)

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ConvolutionTest.h"
#include "backend/core/column/Column.h"
#include "backend/worksheet/plots/cartesian/XYConvolutionCurve.h"

//##############################################################################

void ConvolutionTest::testLinear() {
	// data
	QVector<int> xData = {1,2,3,4};
	QVector<double> yData = {1.,2.,3.,4.};
	QVector<double> y2Data = {0,1.,.5};

	//data source columns
	Column xDataColumn("x", AbstractColumn::ColumnMode::Integer);
	xDataColumn.replaceInteger(0, xData);

	Column yDataColumn("y", AbstractColumn::ColumnMode::Numeric);
	yDataColumn.replaceValues(0, yData);

	Column y2DataColumn("y2", AbstractColumn::ColumnMode::Numeric);
	y2DataColumn.replaceValues(0, y2Data);

	XYConvolutionCurve curve("convolution");
	curve.setXDataColumn(&xDataColumn);
	curve.setYDataColumn(&yDataColumn);
	curve.setY2DataColumn(&y2DataColumn);

	//prepare the convolution
	XYConvolutionCurve::ConvolutionData convolutionData = curve.convolutionData();
	curve.setConvolutionData(convolutionData);

	//perform the convolution
	curve.recalculate();
	const XYConvolutionCurve::ConvolutionResult& result = curve.convolutionResult();

	//check the results
	QCOMPARE(result.available, true);
	QCOMPARE(result.valid, true);

	const AbstractColumn* resultXDataColumn = curve.xColumn();
	const AbstractColumn* resultYDataColumn = curve.yColumn();

	const int np = resultXDataColumn->rowCount();
	QCOMPARE(np, 6);

	for (int i = 0; i < np; i++)
		QCOMPARE(resultXDataColumn->valueAt(i), (double)i + 1);

	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(0));
	FuzzyCompare(resultYDataColumn->valueAt(0), 0., 1.e-15);
	QCOMPARE(resultYDataColumn->valueAt(1), 1.);
	QCOMPARE(resultYDataColumn->valueAt(2), 2.5);
	QCOMPARE(resultYDataColumn->valueAt(3), 4.);
	QCOMPARE(resultYDataColumn->valueAt(4), 5.5);
	QCOMPARE(resultYDataColumn->valueAt(5), 2.);
}

void ConvolutionTest::testLinear2() {
	// data
	QVector<int> xData = {1,2,3};
	QVector<double> yData = {1.,2.,3.};
	QVector<double> y2Data = {0,1.,.5};

	//data source columns
	Column xDataColumn("x", AbstractColumn::ColumnMode::Integer);
	xDataColumn.replaceInteger(0, xData);

	Column yDataColumn("y", AbstractColumn::ColumnMode::Numeric);
	yDataColumn.replaceValues(0, yData);

	Column y2DataColumn("y2", AbstractColumn::ColumnMode::Numeric);
	y2DataColumn.replaceValues(0, y2Data);

	XYConvolutionCurve curve("convolution");
	curve.setXDataColumn(&xDataColumn);
	curve.setYDataColumn(&yDataColumn);
	curve.setY2DataColumn(&y2DataColumn);

	//prepare the convolution
	XYConvolutionCurve::ConvolutionData convolutionData = curve.convolutionData();
	curve.setConvolutionData(convolutionData);

	//perform the convolution
	curve.recalculate();
	const XYConvolutionCurve::ConvolutionResult& result = curve.convolutionResult();

	//check the results
	QCOMPARE(result.available, true);
	QCOMPARE(result.valid, true);

	const AbstractColumn* resultXDataColumn = curve.xColumn();
	const AbstractColumn* resultYDataColumn = curve.yColumn();

	const int np = resultXDataColumn->rowCount();
	QCOMPARE(np, 5);

	for (int i = 0; i < np; i++)
		QCOMPARE(resultXDataColumn->valueAt(i), (double)i+1);

	QCOMPARE(resultYDataColumn->valueAt(0), 0.);
	QCOMPARE(resultYDataColumn->valueAt(1), 1.);
	QCOMPARE(resultYDataColumn->valueAt(2), 2.5);
	QCOMPARE(resultYDataColumn->valueAt(3), 4.);
	QCOMPARE(resultYDataColumn->valueAt(4), 1.5);
}

void ConvolutionTest::testLinear_noX() {
	// data
	QVector<double> yData = {1.,2.,3.,4.};
	QVector<double> y2Data = {0,1.,.5};

	//data source columns
	Column yDataColumn("y", AbstractColumn::ColumnMode::Numeric);
	yDataColumn.replaceValues(0, yData);

	Column y2DataColumn("y2", AbstractColumn::ColumnMode::Numeric);
	y2DataColumn.replaceValues(0, y2Data);

	XYConvolutionCurve curve("convolution");
	curve.setYDataColumn(&yDataColumn);
	curve.setY2DataColumn(&y2DataColumn);

	//prepare the convolution
	XYConvolutionCurve::ConvolutionData convolutionData = curve.convolutionData();
	curve.setConvolutionData(convolutionData);

	//perform the convolution
	curve.recalculate();
	const XYConvolutionCurve::ConvolutionResult& result = curve.convolutionResult();

	//check the results
	QCOMPARE(result.available, true);
	QCOMPARE(result.valid, true);

	const AbstractColumn* resultXDataColumn = curve.xColumn();
	const AbstractColumn* resultYDataColumn = curve.yColumn();

	const int np = resultXDataColumn->rowCount();
	QCOMPARE(np, 6);

	for (int i = 0; i < np; i++)
		QCOMPARE(resultXDataColumn->valueAt(i), (double)i);

	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(0));
	FuzzyCompare(resultYDataColumn->valueAt(0), 0., 1.e-15);
	QCOMPARE(resultYDataColumn->valueAt(1), 1.);
	QCOMPARE(resultYDataColumn->valueAt(2), 2.5);
	QCOMPARE(resultYDataColumn->valueAt(3), 4.);
	QCOMPARE(resultYDataColumn->valueAt(4), 5.5);
	QCOMPARE(resultYDataColumn->valueAt(5), 2.);
}

void ConvolutionTest::testLinear_swapped() {
	// data
	QVector<int> xData = {1,2,3,4};
	QVector<double> yData = {0,1.,.5};
	QVector<double> y2Data = {1.,2.,3.,4.};

	//data source columns
	Column xDataColumn("x", AbstractColumn::ColumnMode::Integer);
	xDataColumn.replaceInteger(0, xData);

	Column yDataColumn("y", AbstractColumn::ColumnMode::Numeric);
	yDataColumn.replaceValues(0, yData);

	Column y2DataColumn("y2", AbstractColumn::ColumnMode::Numeric);
	y2DataColumn.replaceValues(0, y2Data);

	XYConvolutionCurve curve("convolution");
	curve.setXDataColumn(&xDataColumn);
	curve.setYDataColumn(&yDataColumn);
	curve.setY2DataColumn(&y2DataColumn);

	//prepare the convolution
	XYConvolutionCurve::ConvolutionData convolutionData = curve.convolutionData();
	curve.setConvolutionData(convolutionData);

	//perform the convolution
	curve.recalculate();
	const XYConvolutionCurve::ConvolutionResult& result = curve.convolutionResult();

	//check the results
	QCOMPARE(result.available, true);
	QCOMPARE(result.valid, true);

	const AbstractColumn* resultXDataColumn = curve.xColumn();
	const AbstractColumn* resultYDataColumn = curve.yColumn();

	const int np = resultXDataColumn->rowCount();
	QCOMPARE(np, 6);

	for (int i = 0; i < np; i++)
		QCOMPARE(resultXDataColumn->valueAt(i), (double)i+1);

	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(0));
	FuzzyCompare(resultYDataColumn->valueAt(0), 0., 1.e-15);
	QCOMPARE(resultYDataColumn->valueAt(1), 1.);
	QCOMPARE(resultYDataColumn->valueAt(2), 2.5);
	QCOMPARE(resultYDataColumn->valueAt(3), 4.);
	QCOMPARE(resultYDataColumn->valueAt(4), 5.5);
	QCOMPARE(resultYDataColumn->valueAt(5), 2.);
}

void ConvolutionTest::testLinear_swapped_noX() {
	// data
	QVector<double> yData = {0,1.,.5};
	QVector<double> y2Data = {1.,2.,3.,4.};

	//data source columns
	Column yDataColumn("y", AbstractColumn::ColumnMode::Numeric);
	yDataColumn.replaceValues(0, yData);

	Column y2DataColumn("y2", AbstractColumn::ColumnMode::Numeric);
	y2DataColumn.replaceValues(0, y2Data);

	XYConvolutionCurve curve("convolution");
	curve.setYDataColumn(&yDataColumn);
	curve.setY2DataColumn(&y2DataColumn);

	//prepare the convolution
	XYConvolutionCurve::ConvolutionData convolutionData = curve.convolutionData();
	curve.setConvolutionData(convolutionData);

	//perform the convolution
	curve.recalculate();
	const XYConvolutionCurve::ConvolutionResult& result = curve.convolutionResult();

	//check the results
	QCOMPARE(result.available, true);
	QCOMPARE(result.valid, true);

	const AbstractColumn* resultXDataColumn = curve.xColumn();
	const AbstractColumn* resultYDataColumn = curve.yColumn();

	const int np = resultXDataColumn->rowCount();
	QCOMPARE(np, 6);

	for (int i = 0; i < np; i++)
		QCOMPARE(resultXDataColumn->valueAt(i), (double)i);

	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(0));
	FuzzyCompare(resultYDataColumn->valueAt(0), 0., 1.e-15);
	QCOMPARE(resultYDataColumn->valueAt(1), 1.);
	QCOMPARE(resultYDataColumn->valueAt(2), 2.5);
	QCOMPARE(resultYDataColumn->valueAt(3), 4.);
	QCOMPARE(resultYDataColumn->valueAt(4), 5.5);
	QCOMPARE(resultYDataColumn->valueAt(5), 2.);
}

void ConvolutionTest::testLinear_norm() {
	// data
	QVector<double> yData = {1.,2.,3.,4.};
	QVector<double> y2Data = {0,1.,.5};

	//data source columns
	Column yDataColumn("y", AbstractColumn::ColumnMode::Numeric);
	yDataColumn.replaceValues(0, yData);

	Column y2DataColumn("y2", AbstractColumn::ColumnMode::Numeric);
	y2DataColumn.replaceValues(0, y2Data);

	XYConvolutionCurve curve("convolution");
	curve.setYDataColumn(&yDataColumn);
	curve.setY2DataColumn(&y2DataColumn);

	//prepare the convolution
	XYConvolutionCurve::ConvolutionData convolutionData = curve.convolutionData();
	convolutionData.normalize = nsl_conv_norm_euclidean;
	curve.setConvolutionData(convolutionData);

	//perform the convolution
	curve.recalculate();
	const XYConvolutionCurve::ConvolutionResult& result = curve.convolutionResult();

	//check the results
	QCOMPARE(result.available, true);
	QCOMPARE(result.valid, true);

	const AbstractColumn* resultXDataColumn = curve.xColumn();
	const AbstractColumn* resultYDataColumn = curve.yColumn();

	const int np = resultXDataColumn->rowCount();
	QCOMPARE(np, 6);

	for (int i = 0; i < np; i++)
		QCOMPARE(resultXDataColumn->valueAt(i), (double)i);

	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(0));
	FuzzyCompare(resultYDataColumn->valueAt(0), 0., 1.e-15);
	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(1));
	QCOMPARE(resultYDataColumn->valueAt(1), 0.894427190999916);
	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(2));
	QCOMPARE(resultYDataColumn->valueAt(2), 2.23606797749979);
	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(3));
	QCOMPARE(resultYDataColumn->valueAt(3), 3.57770876399966);
	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(4));
	QCOMPARE(resultYDataColumn->valueAt(4), 4.91934955049954);
	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(5));
	QCOMPARE(resultYDataColumn->valueAt(5), 1.78885438199983);
}

void ConvolutionTest::testLinear_swapped_norm() {
	// data
	QVector<double> yData = {0,1.,.5};
	QVector<double> y2Data = {1.,2.,3.,4.};

	//data source columns
	Column yDataColumn("y", AbstractColumn::ColumnMode::Numeric);
	yDataColumn.replaceValues(0, yData);

	Column y2DataColumn("y2", AbstractColumn::ColumnMode::Numeric);
	y2DataColumn.replaceValues(0, y2Data);

	XYConvolutionCurve curve("convolution");
	curve.setYDataColumn(&yDataColumn);
	curve.setY2DataColumn(&y2DataColumn);

	//prepare the convolution
	XYConvolutionCurve::ConvolutionData convolutionData = curve.convolutionData();
	convolutionData.normalize = nsl_conv_norm_euclidean;
	curve.setConvolutionData(convolutionData);

	//perform the convolution
	curve.recalculate();
	const XYConvolutionCurve::ConvolutionResult& result = curve.convolutionResult();

	//check the results
	QCOMPARE(result.available, true);
	QCOMPARE(result.valid, true);

	const AbstractColumn* resultXDataColumn = curve.xColumn();
	const AbstractColumn* resultYDataColumn = curve.yColumn();

	const int np = resultXDataColumn->rowCount();
	QCOMPARE(np, 6);

	for (int i = 0; i < np; i++)
		QCOMPARE(resultXDataColumn->valueAt(i), (double)i);

	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(0));
	QCOMPARE(resultYDataColumn->valueAt(0), 0.);
	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(1));
	QCOMPARE(resultYDataColumn->valueAt(1), 0.182574185835055);
	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(2));
	QCOMPARE(resultYDataColumn->valueAt(2), 0.456435464587638);
	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(3));
	QCOMPARE(resultYDataColumn->valueAt(3), 0.730296743340221);
	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(4));
	QCOMPARE(resultYDataColumn->valueAt(4), 1.0041580220928);
	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(5));
	QCOMPARE(resultYDataColumn->valueAt(5), 0.365148371670111);
}

void ConvolutionTest::testLinear_wrapMax() {
	// data
	QVector<double> yData = {1.,2.,3.,4.};
	QVector<double> y2Data = {0,1.,.5};

	//data source columns
	Column yDataColumn("y", AbstractColumn::ColumnMode::Numeric);
	yDataColumn.replaceValues(0, yData);

	Column y2DataColumn("y2", AbstractColumn::ColumnMode::Numeric);
	y2DataColumn.replaceValues(0, y2Data);

	XYConvolutionCurve curve("convolution");
	curve.setYDataColumn(&yDataColumn);
	curve.setY2DataColumn(&y2DataColumn);

	//prepare the convolution
	XYConvolutionCurve::ConvolutionData convolutionData = curve.convolutionData();
	convolutionData.wrap = nsl_conv_wrap_max;
	curve.setConvolutionData(convolutionData);

	//perform the convolution
	curve.recalculate();
	const XYConvolutionCurve::ConvolutionResult& result = curve.convolutionResult();

	//check the results
	QCOMPARE(result.available, true);
	QCOMPARE(result.valid, true);

	const AbstractColumn* resultXDataColumn = curve.xColumn();
	const AbstractColumn* resultYDataColumn = curve.yColumn();

	const int np = resultXDataColumn->rowCount();
	QCOMPARE(np, 6);

	for (int i = 0; i < np; i++)
		QCOMPARE(resultXDataColumn->valueAt(i), (double)i);

	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(0));
	QCOMPARE(resultYDataColumn->valueAt(0), 1.);
	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(1));
	QCOMPARE(resultYDataColumn->valueAt(1), 2.5);
	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(2));
	QCOMPARE(resultYDataColumn->valueAt(2), 4.);
	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(3));
	QCOMPARE(resultYDataColumn->valueAt(3), 5.5);
	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(4));
	QCOMPARE(resultYDataColumn->valueAt(4), 2.);
	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(5));
	FuzzyCompare(resultYDataColumn->valueAt(5), 0., 1.e-15);
}

void ConvolutionTest::testLinear_swapped_wrapMax() {
	// data
	QVector<double> yData = {0,1.,.5};
	QVector<double> y2Data = {1.,2.,3.,4.};

	//data source columns
	Column yDataColumn("y", AbstractColumn::ColumnMode::Numeric);
	yDataColumn.replaceValues(0, yData);

	Column y2DataColumn("y2", AbstractColumn::ColumnMode::Numeric);
	y2DataColumn.replaceValues(0, y2Data);

	XYConvolutionCurve curve("convolution");
	curve.setYDataColumn(&yDataColumn);
	curve.setY2DataColumn(&y2DataColumn);

	//prepare the convolution
	XYConvolutionCurve::ConvolutionData convolutionData = curve.convolutionData();
	convolutionData.wrap = nsl_conv_wrap_max;
	curve.setConvolutionData(convolutionData);

	//perform the convolution
	curve.recalculate();
	const XYConvolutionCurve::ConvolutionResult& result = curve.convolutionResult();

	//check the results
	QCOMPARE(result.available, true);
	QCOMPARE(result.valid, true);

	const AbstractColumn* resultXDataColumn = curve.xColumn();
	const AbstractColumn* resultYDataColumn = curve.yColumn();

	const int np = resultXDataColumn->rowCount();
	QCOMPARE(np, 6);

	for (int i = 0; i < np; i++)
		QCOMPARE(resultXDataColumn->valueAt(i), (double)i);

	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(0));
	QCOMPARE(resultYDataColumn->valueAt(0), 4.);
	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(1));
	QCOMPARE(resultYDataColumn->valueAt(1), 5.5);
	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(2));
	QCOMPARE(resultYDataColumn->valueAt(2), 2.);
	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(3));
	FuzzyCompare(resultYDataColumn->valueAt(3), 0., 1.e-15);
	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(4));
	QCOMPARE(resultYDataColumn->valueAt(4), 1.);
	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(5));
	QCOMPARE(resultYDataColumn->valueAt(5), 2.5);
}

void ConvolutionTest::testLinear_wrapCenter() {
	// data
	QVector<double> yData = {1.,2.,3.,4.};
	QVector<double> y2Data = {0,1.,.5};

	//data source columns
	Column yDataColumn("y", AbstractColumn::ColumnMode::Numeric);
	yDataColumn.replaceValues(0, yData);

	Column y2DataColumn("y2", AbstractColumn::ColumnMode::Numeric);
	y2DataColumn.replaceValues(0, y2Data);

	XYConvolutionCurve curve("convolution");
	curve.setYDataColumn(&yDataColumn);
	curve.setY2DataColumn(&y2DataColumn);

	//prepare the convolution
	XYConvolutionCurve::ConvolutionData convolutionData = curve.convolutionData();
	convolutionData.wrap = nsl_conv_wrap_center;
	curve.setConvolutionData(convolutionData);

	//perform the convolution
	curve.recalculate();
	const XYConvolutionCurve::ConvolutionResult& result = curve.convolutionResult();

	//check the results
	QCOMPARE(result.available, true);
	QCOMPARE(result.valid, true);

	const AbstractColumn* resultXDataColumn = curve.xColumn();
	const AbstractColumn* resultYDataColumn = curve.yColumn();

	const int np = resultXDataColumn->rowCount();
	QCOMPARE(np, 6);

	for (int i = 0; i < np; i++)
		QCOMPARE(resultXDataColumn->valueAt(i), (double)i);

	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(0));
	QCOMPARE(resultYDataColumn->valueAt(0), 1.);
	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(1));
	QCOMPARE(resultYDataColumn->valueAt(1), 2.5);
	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(2));
	QCOMPARE(resultYDataColumn->valueAt(2), 4.);
	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(3));
	QCOMPARE(resultYDataColumn->valueAt(3), 5.5);
	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(4));
	QCOMPARE(resultYDataColumn->valueAt(4), 2.);
	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(5));
	FuzzyCompare(resultYDataColumn->valueAt(5), 0., 1.e-15);
}

void ConvolutionTest::testLinear_swapped_wrapCenter() {
	// data
	QVector<double> yData = {0,1.,.5};
	QVector<double> y2Data = {1.,2.,3.,4.};

	//data source columns
	Column yDataColumn("y", AbstractColumn::ColumnMode::Numeric);
	yDataColumn.replaceValues(0, yData);

	Column y2DataColumn("y2", AbstractColumn::ColumnMode::Numeric);
	y2DataColumn.replaceValues(0, y2Data);

	XYConvolutionCurve curve("convolution");
	curve.setYDataColumn(&yDataColumn);
	curve.setY2DataColumn(&y2DataColumn);

	//prepare the convolution
	XYConvolutionCurve::ConvolutionData convolutionData = curve.convolutionData();
	convolutionData.wrap = nsl_conv_wrap_max;
	curve.setConvolutionData(convolutionData);

	//perform the convolution
	curve.recalculate();
	const XYConvolutionCurve::ConvolutionResult& result = curve.convolutionResult();

	//check the results
	QCOMPARE(result.available, true);
	QCOMPARE(result.valid, true);

	const AbstractColumn* resultXDataColumn = curve.xColumn();
	const AbstractColumn* resultYDataColumn = curve.yColumn();

	const int np = resultXDataColumn->rowCount();
	QCOMPARE(np, 6);

	for (int i = 0; i < np; i++)
		QCOMPARE(resultXDataColumn->valueAt(i), (double)i);

	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(0));
	QCOMPARE(resultYDataColumn->valueAt(0), 4.);
	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(1));
	QCOMPARE(resultYDataColumn->valueAt(1), 5.5);
	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(2));
	QCOMPARE(resultYDataColumn->valueAt(2), 2.);
	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(3));
	FuzzyCompare(resultYDataColumn->valueAt(3), 0., 1.e-15);
	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(4));
	QCOMPARE(resultYDataColumn->valueAt(4), 1.);
	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(5));
	QCOMPARE(resultYDataColumn->valueAt(5), 2.5);
}

////////////////// circular tests ////////////////////////////////////////////////////////////////

void ConvolutionTest::testCircular() {
	// data
	QVector<int> xData = {1,2,3,4};
	QVector<double> yData = {1.,2.,3.,4.};
	QVector<double> y2Data = {0,1.,.5};

	//data source columns
	Column xDataColumn("x", AbstractColumn::ColumnMode::Integer);
	xDataColumn.replaceInteger(0, xData);

	Column yDataColumn("y", AbstractColumn::ColumnMode::Numeric);
	yDataColumn.replaceValues(0, yData);

	Column y2DataColumn("y2", AbstractColumn::ColumnMode::Numeric);
	y2DataColumn.replaceValues(0, y2Data);

	XYConvolutionCurve curve("convolution");
	curve.setXDataColumn(&xDataColumn);
	curve.setYDataColumn(&yDataColumn);
	curve.setY2DataColumn(&y2DataColumn);

	//prepare the convolution
	XYConvolutionCurve::ConvolutionData convolutionData = curve.convolutionData();
	convolutionData.type = nsl_conv_type_circular;
	curve.setConvolutionData(convolutionData);

	//perform the convolution
	curve.recalculate();
	const XYConvolutionCurve::ConvolutionResult& result = curve.convolutionResult();

	//check the results
	QCOMPARE(result.available, true);
	QCOMPARE(result.valid, true);

	const AbstractColumn* resultXDataColumn = curve.xColumn();
	const AbstractColumn* resultYDataColumn = curve.yColumn();

	const int np = resultXDataColumn->rowCount();
	QCOMPARE(np, 4);

	for (int i = 0; i < np; i++)
		QCOMPARE(resultXDataColumn->valueAt(i), (double)i+1);

	QCOMPARE(resultYDataColumn->valueAt(0), 5.5);
	QCOMPARE(resultYDataColumn->valueAt(1), 3.);
	QCOMPARE(resultYDataColumn->valueAt(2), 2.5);
	QCOMPARE(resultYDataColumn->valueAt(3), 4.);
}

void ConvolutionTest::testCircular2() {
	// data
	QVector<int> xData = {1,2,3};
	QVector<double> yData = {1.,2.,3.};
	QVector<double> y2Data = {0,1.,.5};

	//data source columns
	Column xDataColumn("x", AbstractColumn::ColumnMode::Integer);
	xDataColumn.replaceInteger(0, xData);

	Column yDataColumn("y", AbstractColumn::ColumnMode::Numeric);
	yDataColumn.replaceValues(0, yData);

	Column y2DataColumn("y2", AbstractColumn::ColumnMode::Numeric);
	y2DataColumn.replaceValues(0, y2Data);

	XYConvolutionCurve curve("convolution");
	curve.setXDataColumn(&xDataColumn);
	curve.setYDataColumn(&yDataColumn);
	curve.setY2DataColumn(&y2DataColumn);

	//prepare the convolution
	XYConvolutionCurve::ConvolutionData convolutionData = curve.convolutionData();
	convolutionData.type = nsl_conv_type_circular;
	curve.setConvolutionData(convolutionData);

	//perform the convolution
	curve.recalculate();
	const XYConvolutionCurve::ConvolutionResult& result = curve.convolutionResult();

	//check the results
	QCOMPARE(result.available, true);
	QCOMPARE(result.valid, true);

	const AbstractColumn* resultXDataColumn = curve.xColumn();
	const AbstractColumn* resultYDataColumn = curve.yColumn();

	const int np = resultXDataColumn->rowCount();
	QCOMPARE(np, 3);

	for (int i = 0; i < np; i++)
		QCOMPARE(resultXDataColumn->valueAt(i), (double)i+1);

	QCOMPARE(resultYDataColumn->valueAt(0), 4.);
	QCOMPARE(resultYDataColumn->valueAt(1), 2.5);
	QCOMPARE(resultYDataColumn->valueAt(2), 2.5);
}

void ConvolutionTest::testCircular_noX() {
	// data
	QVector<double> yData = {1.,2.,3.,4.};
	QVector<double> y2Data = {0,1.,.5};

	//data source columns
	Column yDataColumn("y", AbstractColumn::ColumnMode::Numeric);
	yDataColumn.replaceValues(0, yData);

	Column y2DataColumn("y2", AbstractColumn::ColumnMode::Numeric);
	y2DataColumn.replaceValues(0, y2Data);

	XYConvolutionCurve curve("convolution");
	curve.setYDataColumn(&yDataColumn);
	curve.setY2DataColumn(&y2DataColumn);

	//prepare the convolution
	XYConvolutionCurve::ConvolutionData convolutionData = curve.convolutionData();
	convolutionData.type = nsl_conv_type_circular;
	curve.setConvolutionData(convolutionData);

	//perform the convolution
	curve.recalculate();
	const XYConvolutionCurve::ConvolutionResult& result = curve.convolutionResult();

	//check the results
	QCOMPARE(result.available, true);
	QCOMPARE(result.valid, true);

	const AbstractColumn* resultXDataColumn = curve.xColumn();
	const AbstractColumn* resultYDataColumn = curve.yColumn();

	const int np = resultXDataColumn->rowCount();
	QCOMPARE(np, 4);

	for (int i = 0; i < np; i++)
		QCOMPARE(resultXDataColumn->valueAt(i), (double)i);

	QCOMPARE(resultYDataColumn->valueAt(0), 5.5);
	QCOMPARE(resultYDataColumn->valueAt(1), 3.);
	QCOMPARE(resultYDataColumn->valueAt(2), 2.5);
	QCOMPARE(resultYDataColumn->valueAt(3), 4.);
}

void ConvolutionTest::testCircular_swapped() {
	// data
	QVector<int> xData = {1,2,3,4};
	QVector<double> yData = {0,1.,.5};
	QVector<double> y2Data = {1.,2.,3.,4.};

	//data source columns
	Column xDataColumn("x", AbstractColumn::ColumnMode::Integer);
	xDataColumn.replaceInteger(0, xData);

	Column yDataColumn("y", AbstractColumn::ColumnMode::Numeric);
	yDataColumn.replaceValues(0, yData);

	Column y2DataColumn("y2", AbstractColumn::ColumnMode::Numeric);
	y2DataColumn.replaceValues(0, y2Data);

	XYConvolutionCurve curve("convolution");
	curve.setXDataColumn(&xDataColumn);
	curve.setYDataColumn(&yDataColumn);
	curve.setY2DataColumn(&y2DataColumn);

	//prepare the convolution
	XYConvolutionCurve::ConvolutionData convolutionData = curve.convolutionData();
	convolutionData.type = nsl_conv_type_circular;
	curve.setConvolutionData(convolutionData);

	//perform the convolution
	curve.recalculate();
	const XYConvolutionCurve::ConvolutionResult& result = curve.convolutionResult();

	//check the results
	QCOMPARE(result.available, true);
	QCOMPARE(result.valid, true);

	const AbstractColumn* resultXDataColumn = curve.xColumn();
	const AbstractColumn* resultYDataColumn = curve.yColumn();

	const int np = resultXDataColumn->rowCount();
	QCOMPARE(np, 4);

	for (int i = 0; i < np; i++)
		QCOMPARE(resultXDataColumn->valueAt(i), (double)i+1);

	QCOMPARE(resultYDataColumn->valueAt(0), 5.5);
	QCOMPARE(resultYDataColumn->valueAt(1), 3.);
	QCOMPARE(resultYDataColumn->valueAt(2), 2.5);
	QCOMPARE(resultYDataColumn->valueAt(3), 4.);
}

void ConvolutionTest::testCircular_swapped_noX() {
	// data
	QVector<double> yData = {0,1.,.5};
	QVector<double> y2Data = {1.,2.,3.,4.};

	//data source columns
	Column yDataColumn("y", AbstractColumn::ColumnMode::Numeric);
	yDataColumn.replaceValues(0, yData);

	Column y2DataColumn("y2", AbstractColumn::ColumnMode::Numeric);
	y2DataColumn.replaceValues(0, y2Data);

	XYConvolutionCurve curve("convolution");
	curve.setYDataColumn(&yDataColumn);
	curve.setY2DataColumn(&y2DataColumn);

	//prepare the convolution
	XYConvolutionCurve::ConvolutionData convolutionData = curve.convolutionData();
	convolutionData.type = nsl_conv_type_circular;
	curve.setConvolutionData(convolutionData);

	//perform the convolution
	curve.recalculate();
	const XYConvolutionCurve::ConvolutionResult& result = curve.convolutionResult();

	//check the results
	QCOMPARE(result.available, true);
	QCOMPARE(result.valid, true);

	const AbstractColumn* resultXDataColumn = curve.xColumn();
	const AbstractColumn* resultYDataColumn = curve.yColumn();

	const int np = resultXDataColumn->rowCount();
	QCOMPARE(np, 4);

	for (int i = 0; i < np; i++)
		QCOMPARE(resultXDataColumn->valueAt(i), (double)i);

	QCOMPARE(resultYDataColumn->valueAt(0), 5.5);
	QCOMPARE(resultYDataColumn->valueAt(1), 3.);
	QCOMPARE(resultYDataColumn->valueAt(2), 2.5);
	QCOMPARE(resultYDataColumn->valueAt(3), 4.);
}

void ConvolutionTest::testCircular_norm() {
	// data
	QVector<double> yData = {1.,2.,3.,4.};
	QVector<double> y2Data = {0,1.,.5};

	//data source columns
	Column yDataColumn("y", AbstractColumn::ColumnMode::Numeric);
	yDataColumn.replaceValues(0, yData);

	Column y2DataColumn("y2", AbstractColumn::ColumnMode::Numeric);
	y2DataColumn.replaceValues(0, y2Data);

	XYConvolutionCurve curve("convolution");
	curve.setYDataColumn(&yDataColumn);
	curve.setY2DataColumn(&y2DataColumn);

	//prepare the convolution
	XYConvolutionCurve::ConvolutionData convolutionData = curve.convolutionData();
	convolutionData.type = nsl_conv_type_circular;
	convolutionData.normalize = nsl_conv_norm_euclidean;
	curve.setConvolutionData(convolutionData);

	//perform the convolution
	curve.recalculate();
	const XYConvolutionCurve::ConvolutionResult& result = curve.convolutionResult();

	//check the results
	QCOMPARE(result.available, true);
	QCOMPARE(result.valid, true);

	const AbstractColumn* resultXDataColumn = curve.xColumn();
	const AbstractColumn* resultYDataColumn = curve.yColumn();

	const int np = resultXDataColumn->rowCount();
	QCOMPARE(np, 4);

	for (int i = 0; i < np; i++)
		QCOMPARE(resultXDataColumn->valueAt(i), (double)i);

	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(0));
	QCOMPARE(resultYDataColumn->valueAt(0), 4.91934955049954);
	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(1));
	QCOMPARE(resultYDataColumn->valueAt(1), 2.68328157299975);
	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(2));
	QCOMPARE(resultYDataColumn->valueAt(2), 2.23606797749979);
	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(3));
	QCOMPARE(resultYDataColumn->valueAt(3), 3.57770876399966);
}

void ConvolutionTest::testCircular_swapped_norm() {
	// data
	QVector<double> yData = {0,1.,.5};
	QVector<double> y2Data = {1.,2.,3.,4.};

	//data source columns
	Column yDataColumn("y", AbstractColumn::ColumnMode::Numeric);
	yDataColumn.replaceValues(0, yData);

	Column y2DataColumn("y2", AbstractColumn::ColumnMode::Numeric);
	y2DataColumn.replaceValues(0, y2Data);

	XYConvolutionCurve curve("convolution");
	curve.setYDataColumn(&yDataColumn);
	curve.setY2DataColumn(&y2DataColumn);

	//prepare the convolution
	XYConvolutionCurve::ConvolutionData convolutionData = curve.convolutionData();
	convolutionData.type = nsl_conv_type_circular;
	convolutionData.normalize = nsl_conv_norm_euclidean;
	curve.setConvolutionData(convolutionData);

	//perform the convolution
	curve.recalculate();
	const XYConvolutionCurve::ConvolutionResult& result = curve.convolutionResult();

	//check the results
	QCOMPARE(result.available, true);
	QCOMPARE(result.valid, true);

	const AbstractColumn* resultXDataColumn = curve.xColumn();
	const AbstractColumn* resultYDataColumn = curve.yColumn();

	const int np = resultXDataColumn->rowCount();
	QCOMPARE(np, 4);

	for (int i = 0; i < np; i++)
		QCOMPARE(resultXDataColumn->valueAt(i), (double)i);

	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(0));
	QCOMPARE(resultYDataColumn->valueAt(0), 1.0041580220928);
	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(1));
	QCOMPARE(resultYDataColumn->valueAt(1), 0.547722557505166);
	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(2));
	QCOMPARE(resultYDataColumn->valueAt(2), 0.456435464587638);
	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(3));
	QCOMPARE(resultYDataColumn->valueAt(3), 0.730296743340221);
}

void ConvolutionTest::testCircular_wrapMax() {
	// data
	QVector<double> yData = {1.,2.,3.,4.};
	QVector<double> y2Data = {0,1.,.5};

	//data source columns
	Column yDataColumn("y", AbstractColumn::ColumnMode::Numeric);
	yDataColumn.replaceValues(0, yData);

	Column y2DataColumn("y2", AbstractColumn::ColumnMode::Numeric);
	y2DataColumn.replaceValues(0, y2Data);

	XYConvolutionCurve curve("convolution");
	curve.setYDataColumn(&yDataColumn);
	curve.setY2DataColumn(&y2DataColumn);

	//prepare the convolution
	XYConvolutionCurve::ConvolutionData convolutionData = curve.convolutionData();
	convolutionData.type = nsl_conv_type_circular;
	convolutionData.wrap = nsl_conv_wrap_max;
	curve.setConvolutionData(convolutionData);

	//perform the convolution
	curve.recalculate();
	const XYConvolutionCurve::ConvolutionResult& result = curve.convolutionResult();

	//check the results
	QCOMPARE(result.available, true);
	QCOMPARE(result.valid, true);

	const AbstractColumn* resultXDataColumn = curve.xColumn();
	const AbstractColumn* resultYDataColumn = curve.yColumn();

	const int np = resultXDataColumn->rowCount();
	QCOMPARE(np, 4);

	for (int i = 0; i < np; i++)
		QCOMPARE(resultXDataColumn->valueAt(i), (double)i);

	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(0));
	QCOMPARE(resultYDataColumn->valueAt(0), 3.);
	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(1));
	QCOMPARE(resultYDataColumn->valueAt(1), 2.5);
	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(2));
	QCOMPARE(resultYDataColumn->valueAt(2), 4.);
	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(3));
	QCOMPARE(resultYDataColumn->valueAt(3), 5.5);
}

void ConvolutionTest::testCircular_swapped_wrapMax() {
	// data
	QVector<double> yData = {0,1.,.5};
	QVector<double> y2Data = {1.,2.,3.,4.};

	//data source columns
	Column yDataColumn("y", AbstractColumn::ColumnMode::Numeric);
	yDataColumn.replaceValues(0, yData);

	Column y2DataColumn("y2", AbstractColumn::ColumnMode::Numeric);
	y2DataColumn.replaceValues(0, y2Data);

	XYConvolutionCurve curve("convolution");
	curve.setYDataColumn(&yDataColumn);
	curve.setY2DataColumn(&y2DataColumn);

	//prepare the convolution
	XYConvolutionCurve::ConvolutionData convolutionData = curve.convolutionData();
	convolutionData.type = nsl_conv_type_circular;
	convolutionData.wrap = nsl_conv_wrap_max;
	curve.setConvolutionData(convolutionData);

	//perform the convolution
	curve.recalculate();
	const XYConvolutionCurve::ConvolutionResult& result = curve.convolutionResult();

	//check the results
	QCOMPARE(result.available, true);
	QCOMPARE(result.valid, true);

	const AbstractColumn* resultXDataColumn = curve.xColumn();
	const AbstractColumn* resultYDataColumn = curve.yColumn();

	const int np = resultXDataColumn->rowCount();
	QCOMPARE(np, 4);

	for (int i = 0; i < np; i++)
		QCOMPARE(resultXDataColumn->valueAt(i), (double)i);

	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(0));
	QCOMPARE(resultYDataColumn->valueAt(0), 4.);
	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(1));
	QCOMPARE(resultYDataColumn->valueAt(1), 5.5);
	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(2));
	QCOMPARE(resultYDataColumn->valueAt(2), 3.);
	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(3));
	QCOMPARE(resultYDataColumn->valueAt(3), 2.5);
}

void ConvolutionTest::testCircular_wrapCenter() {
	// data
	QVector<double> yData = {1.,2.,3.,4.};
	QVector<double> y2Data = {0,1.,.5};

	//data source columns
	Column yDataColumn("y", AbstractColumn::ColumnMode::Numeric);
	yDataColumn.replaceValues(0, yData);

	Column y2DataColumn("y2", AbstractColumn::ColumnMode::Numeric);
	y2DataColumn.replaceValues(0, y2Data);

	XYConvolutionCurve curve("convolution");
	curve.setYDataColumn(&yDataColumn);
	curve.setY2DataColumn(&y2DataColumn);

	//prepare the convolution
	XYConvolutionCurve::ConvolutionData convolutionData = curve.convolutionData();
	convolutionData.type = nsl_conv_type_circular;
	convolutionData.wrap = nsl_conv_wrap_center;
	curve.setConvolutionData(convolutionData);

	//perform the convolution
	curve.recalculate();
	const XYConvolutionCurve::ConvolutionResult& result = curve.convolutionResult();

	//check the results
	QCOMPARE(result.available, true);
	QCOMPARE(result.valid, true);

	const AbstractColumn* resultXDataColumn = curve.xColumn();
	const AbstractColumn* resultYDataColumn = curve.yColumn();

	const int np = resultXDataColumn->rowCount();
	QCOMPARE(np, 4);

	for (int i = 0; i < np; i++)
		QCOMPARE(resultXDataColumn->valueAt(i), (double)i);

	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(0));
	QCOMPARE(resultYDataColumn->valueAt(0), 3.);
	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(1));
	QCOMPARE(resultYDataColumn->valueAt(1), 2.5);
	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(2));
	QCOMPARE(resultYDataColumn->valueAt(2), 4.);
	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(3));
	QCOMPARE(resultYDataColumn->valueAt(3), 5.5);
}

void ConvolutionTest::testCircular_swapped_wrapCenter() {
	// data
	QVector<double> yData = {0,1.,.5};
	QVector<double> y2Data = {1.,2.,3.,4.};

	//data source columns
	Column yDataColumn("y", AbstractColumn::ColumnMode::Numeric);
	yDataColumn.replaceValues(0, yData);

	Column y2DataColumn("y2", AbstractColumn::ColumnMode::Numeric);
	y2DataColumn.replaceValues(0, y2Data);

	XYConvolutionCurve curve("convolution");
	curve.setYDataColumn(&yDataColumn);
	curve.setY2DataColumn(&y2DataColumn);

	//prepare the convolution
	XYConvolutionCurve::ConvolutionData convolutionData = curve.convolutionData();
	convolutionData.type = nsl_conv_type_circular;
	convolutionData.wrap = nsl_conv_wrap_center;
	curve.setConvolutionData(convolutionData);

	//perform the convolution
	curve.recalculate();
	const XYConvolutionCurve::ConvolutionResult& result = curve.convolutionResult();

	//check the results
	QCOMPARE(result.available, true);
	QCOMPARE(result.valid, true);

	const AbstractColumn* resultXDataColumn = curve.xColumn();
	const AbstractColumn* resultYDataColumn = curve.yColumn();

	const int np = resultXDataColumn->rowCount();
	QCOMPARE(np, 4);

	for (int i = 0; i < np; i++)
		QCOMPARE(resultXDataColumn->valueAt(i), (double)i);

	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(0));
	QCOMPARE(resultYDataColumn->valueAt(0), 2.5);
	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(1));
	QCOMPARE(resultYDataColumn->valueAt(1), 4.);
	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(2));
	QCOMPARE(resultYDataColumn->valueAt(2), 5.5);
	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(3));
	QCOMPARE(resultYDataColumn->valueAt(3), 3.);
}

//////////////// Deconvolution tests /////////////////////

void ConvolutionTest::testLinearDeconv() {
	// data
	QVector<double> yData = {0., 1., 2.5, 4., 5.5, 2.};
	QVector<double> y2Data = {0,1.,.5};

	//data source columns
	Column yDataColumn("y", AbstractColumn::ColumnMode::Numeric);
	yDataColumn.replaceValues(0, yData);

	Column y2DataColumn("y2", AbstractColumn::ColumnMode::Numeric);
	y2DataColumn.replaceValues(0, y2Data);

	XYConvolutionCurve curve("convolution");
	curve.setYDataColumn(&yDataColumn);
	curve.setY2DataColumn(&y2DataColumn);

	//prepare the convolution
	XYConvolutionCurve::ConvolutionData convolutionData = curve.convolutionData();
	convolutionData.direction = nsl_conv_direction_backward;
	curve.setConvolutionData(convolutionData);

	//perform the convolution
	curve.recalculate();
	const XYConvolutionCurve::ConvolutionResult& result = curve.convolutionResult();

	//check the results
	QCOMPARE(result.available, true);
	QCOMPARE(result.valid, true);

	const AbstractColumn* resultXDataColumn = curve.xColumn();
	const AbstractColumn* resultYDataColumn = curve.yColumn();

	const int np = resultXDataColumn->rowCount();
	QCOMPARE(np, 4);

	for (int i = 0; i < np; i++)
		QCOMPARE(resultXDataColumn->valueAt(i), (double)i);

	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(0));
	QCOMPARE(resultYDataColumn->valueAt(0), 1.);
	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(1));
	QCOMPARE(resultYDataColumn->valueAt(1), 2.);
	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(2));
	QCOMPARE(resultYDataColumn->valueAt(2), 3.);
	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(3));
	QCOMPARE(resultYDataColumn->valueAt(3), 4.);
}

void ConvolutionTest::testLinearDeconv2() {
	// data
	QVector<double> yData = {0., 1., 2.5, 4., 1.5};
	QVector<double> y2Data = {0,1.,.5};

	//data source columns
	Column yDataColumn("y", AbstractColumn::ColumnMode::Numeric);
	yDataColumn.replaceValues(0, yData);

	Column y2DataColumn("y2", AbstractColumn::ColumnMode::Numeric);
	y2DataColumn.replaceValues(0, y2Data);

	XYConvolutionCurve curve("convolution");
	curve.setYDataColumn(&yDataColumn);
	curve.setY2DataColumn(&y2DataColumn);

	//prepare the convolution
	XYConvolutionCurve::ConvolutionData convolutionData = curve.convolutionData();
	convolutionData.direction = nsl_conv_direction_backward;
	curve.setConvolutionData(convolutionData);

	//perform the convolution
	curve.recalculate();
	const XYConvolutionCurve::ConvolutionResult& result = curve.convolutionResult();

	//check the results
	QCOMPARE(result.available, true);
	QCOMPARE(result.valid, true);

	const AbstractColumn* resultXDataColumn = curve.xColumn();
	const AbstractColumn* resultYDataColumn = curve.yColumn();

	const int np = resultXDataColumn->rowCount();
	QCOMPARE(np, 3);

	for (int i = 0; i < np; i++)
		QCOMPARE(resultXDataColumn->valueAt(i), (double)i);

	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(0));
	QCOMPARE(resultYDataColumn->valueAt(0), 1.);
	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(1));
	QCOMPARE(resultYDataColumn->valueAt(1), 2.);
	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(2));
	QCOMPARE(resultYDataColumn->valueAt(2), 3.);
}

void ConvolutionTest::testLinearDeconv_swapped() {
	// data
	QVector<double> yData = {0., 1., 2.5, 4., 5.5, 2.};
	QVector<double> y2Data = {1, 2, 3, 4};

	//data source columns
	Column yDataColumn("y", AbstractColumn::ColumnMode::Numeric);
	yDataColumn.replaceValues(0, yData);

	Column y2DataColumn("y2", AbstractColumn::ColumnMode::Numeric);
	y2DataColumn.replaceValues(0, y2Data);

	XYConvolutionCurve curve("convolution");
	curve.setYDataColumn(&yDataColumn);
	curve.setY2DataColumn(&y2DataColumn);

	//prepare the convolution
	XYConvolutionCurve::ConvolutionData convolutionData = curve.convolutionData();
	convolutionData.direction = nsl_conv_direction_backward;
	curve.setConvolutionData(convolutionData);

	//perform the convolution
	curve.recalculate();
	const XYConvolutionCurve::ConvolutionResult& result = curve.convolutionResult();

	//check the results
	QCOMPARE(result.available, true);
	QCOMPARE(result.valid, true);

	const AbstractColumn* resultXDataColumn = curve.xColumn();
	const AbstractColumn* resultYDataColumn = curve.yColumn();

	const int np = resultXDataColumn->rowCount();
	QCOMPARE(np, 3);

	for (int i = 0; i < np; i++)
		QCOMPARE(resultXDataColumn->valueAt(i), (double)i);

	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(0));
	FuzzyCompare(resultYDataColumn->valueAt(0), 0., 1.e-15);
	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(1));
	QCOMPARE(resultYDataColumn->valueAt(1), 1.);
	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(2));
	QCOMPARE(resultYDataColumn->valueAt(2), 0.5);
}

void ConvolutionTest::testLinearDeconv2_swapped() {
	// data
	QVector<double> yData = {0., 1., 2.5, 4., 1.5};
	QVector<double> y2Data = {1, 2, 3};

	//data source columns
	Column yDataColumn("y", AbstractColumn::ColumnMode::Numeric);
	yDataColumn.replaceValues(0, yData);

	Column y2DataColumn("y2", AbstractColumn::ColumnMode::Numeric);
	y2DataColumn.replaceValues(0, y2Data);

	XYConvolutionCurve curve("convolution");
	curve.setYDataColumn(&yDataColumn);
	curve.setY2DataColumn(&y2DataColumn);

	//prepare the convolution
	XYConvolutionCurve::ConvolutionData convolutionData = curve.convolutionData();
	convolutionData.direction = nsl_conv_direction_backward;
	curve.setConvolutionData(convolutionData);

	//perform the convolution
	curve.recalculate();
	const XYConvolutionCurve::ConvolutionResult& result = curve.convolutionResult();

	//check the results
	QCOMPARE(result.available, true);
	QCOMPARE(result.valid, true);

	const AbstractColumn* resultXDataColumn = curve.xColumn();
	const AbstractColumn* resultYDataColumn = curve.yColumn();

	const int np = resultXDataColumn->rowCount();
	QCOMPARE(np, 3);

	for (int i = 0; i < np; i++)
		QCOMPARE(resultXDataColumn->valueAt(i), (double)i);

	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(0));
	FuzzyCompare(resultYDataColumn->valueAt(0), 0., 1.e-15);
	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(1));
	QCOMPARE(resultYDataColumn->valueAt(1), 1.);
	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(2));
	QCOMPARE(resultYDataColumn->valueAt(2), 0.5);
}

void ConvolutionTest::testLinearDeconv_norm() {
	// data
	QVector<double> yData = {0, 0.894427190999916, 2.23606797749979, 3.57770876399966, 4.91934955049954, 1.78885438199983};
	QVector<double> y2Data = {0,1.,.5};

	//data source columns
	Column yDataColumn("y", AbstractColumn::ColumnMode::Numeric);
	yDataColumn.replaceValues(0, yData);

	Column y2DataColumn("y2", AbstractColumn::ColumnMode::Numeric);
	y2DataColumn.replaceValues(0, y2Data);

	XYConvolutionCurve curve("convolution");
	curve.setYDataColumn(&yDataColumn);
	curve.setY2DataColumn(&y2DataColumn);

	//prepare the convolution
	XYConvolutionCurve::ConvolutionData convolutionData = curve.convolutionData();
	convolutionData.direction = nsl_conv_direction_backward;
	convolutionData.normalize = nsl_conv_norm_euclidean;
	curve.setConvolutionData(convolutionData);

	//perform the convolution
	curve.recalculate();
	const XYConvolutionCurve::ConvolutionResult& result = curve.convolutionResult();

	//check the results
	QCOMPARE(result.available, true);
	QCOMPARE(result.valid, true);

	const AbstractColumn* resultXDataColumn = curve.xColumn();
	const AbstractColumn* resultYDataColumn = curve.yColumn();

	const int np = resultXDataColumn->rowCount();
	QCOMPARE(np, 4);

	for (int i = 0; i < np; i++)
		QCOMPARE(resultXDataColumn->valueAt(i), (double)i);

	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(0));
	QCOMPARE(resultYDataColumn->valueAt(0), 1.);
	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(1));
	QCOMPARE(resultYDataColumn->valueAt(1), 2.);
	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(2));
	QCOMPARE(resultYDataColumn->valueAt(2), 3.);
	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(3));
	QCOMPARE(resultYDataColumn->valueAt(3), 4.);
}

void ConvolutionTest::testCircularDeconv() {
	// data
	QVector<double> yData = {5.5, 3., 2.5, 4.};
	QVector<double> y2Data = {0,1.,.5};

	//data source columns
	Column yDataColumn("y", AbstractColumn::ColumnMode::Numeric);
	yDataColumn.replaceValues(0, yData);

	Column y2DataColumn("y2", AbstractColumn::ColumnMode::Numeric);
	y2DataColumn.replaceValues(0, y2Data);

	XYConvolutionCurve curve("convolution");
	curve.setYDataColumn(&yDataColumn);
	curve.setY2DataColumn(&y2DataColumn);

	//prepare the convolution
	XYConvolutionCurve::ConvolutionData convolutionData = curve.convolutionData();
	convolutionData.direction = nsl_conv_direction_backward;
	convolutionData.type = nsl_conv_type_circular;
	curve.setConvolutionData(convolutionData);

	//perform the convolution
	curve.recalculate();
	const XYConvolutionCurve::ConvolutionResult& result = curve.convolutionResult();

	//check the results
	QCOMPARE(result.available, true);
	QCOMPARE(result.valid, true);

	const AbstractColumn* resultXDataColumn = curve.xColumn();
	const AbstractColumn* resultYDataColumn = curve.yColumn();

	const int np = resultXDataColumn->rowCount();
	QCOMPARE(np, 4);

	for (int i = 0; i < np; i++)
		QCOMPARE(resultXDataColumn->valueAt(i), (double)i);

	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(0));
	QCOMPARE(resultYDataColumn->valueAt(0), 1.);
	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(1));
	QCOMPARE(resultYDataColumn->valueAt(1), 2.);
	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(2));
	QCOMPARE(resultYDataColumn->valueAt(2), 3.);
	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(3));
	QCOMPARE(resultYDataColumn->valueAt(3), 4.);
}

void ConvolutionTest::testCircularDeconv2() {
	// data
	QVector<double> yData = {4., 2.5, 2.5};
	QVector<double> y2Data = {0,1.,.5};

	//data source columns
	Column yDataColumn("y", AbstractColumn::ColumnMode::Numeric);
	yDataColumn.replaceValues(0, yData);

	Column y2DataColumn("y2", AbstractColumn::ColumnMode::Numeric);
	y2DataColumn.replaceValues(0, y2Data);

	XYConvolutionCurve curve("convolution");
	curve.setYDataColumn(&yDataColumn);
	curve.setY2DataColumn(&y2DataColumn);

	//prepare the convolution
	XYConvolutionCurve::ConvolutionData convolutionData = curve.convolutionData();
	convolutionData.direction = nsl_conv_direction_backward;
	convolutionData.type = nsl_conv_type_circular;
	curve.setConvolutionData(convolutionData);

	//perform the convolution
	curve.recalculate();
	const XYConvolutionCurve::ConvolutionResult& result = curve.convolutionResult();

	//check the results
	QCOMPARE(result.available, true);
	QCOMPARE(result.valid, true);

	const AbstractColumn* resultXDataColumn = curve.xColumn();
	const AbstractColumn* resultYDataColumn = curve.yColumn();

	const int np = resultXDataColumn->rowCount();
	QCOMPARE(np, 3);

	for (int i = 0; i < np; i++)
		QCOMPARE(resultXDataColumn->valueAt(i), (double)i);

	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(0));
	QCOMPARE(resultYDataColumn->valueAt(0), 1.);
	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(1));
	QCOMPARE(resultYDataColumn->valueAt(1), 2.);
	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(2));
	QCOMPARE(resultYDataColumn->valueAt(2), 3.);
}

void ConvolutionTest::testCircularDeconv_norm() {
	// data
	QVector<double> yData = {4.91934955049954, 2.68328157299975, 2.23606797749979, 3.57770876399966};
	QVector<double> y2Data = {0,1.,.5};

	//data source columns
	Column yDataColumn("y", AbstractColumn::ColumnMode::Numeric);
	yDataColumn.replaceValues(0, yData);

	Column y2DataColumn("y2", AbstractColumn::ColumnMode::Numeric);
	y2DataColumn.replaceValues(0, y2Data);

	XYConvolutionCurve curve("convolution");
	curve.setYDataColumn(&yDataColumn);
	curve.setY2DataColumn(&y2DataColumn);

	//prepare the convolution
	XYConvolutionCurve::ConvolutionData convolutionData = curve.convolutionData();
	convolutionData.direction = nsl_conv_direction_backward;
	convolutionData.type = nsl_conv_type_circular;
	convolutionData.normalize = nsl_conv_norm_euclidean;
	curve.setConvolutionData(convolutionData);

	//perform the convolution
	curve.recalculate();
	const XYConvolutionCurve::ConvolutionResult& result = curve.convolutionResult();

	//check the results
	QCOMPARE(result.available, true);
	QCOMPARE(result.valid, true);

	const AbstractColumn* resultXDataColumn = curve.xColumn();
	const AbstractColumn* resultYDataColumn = curve.yColumn();

	const int np = resultXDataColumn->rowCount();
	QCOMPARE(np, 4);

	for (int i = 0; i < np; i++)
		QCOMPARE(resultXDataColumn->valueAt(i), (double)i);

	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(0));
	QCOMPARE(resultYDataColumn->valueAt(0), 1.);
	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(1));
	QCOMPARE(resultYDataColumn->valueAt(1), 2.);
	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(2));
	QCOMPARE(resultYDataColumn->valueAt(2), 3.);
	DEBUG(std::setprecision(15) << resultYDataColumn->valueAt(3));
	QCOMPARE(resultYDataColumn->valueAt(3), 4.);
}

void ConvolutionTest::testPerformance() {
	// data
	QVector<double> yData;
#ifdef HAVE_FFTW3
	const int N = 3e6;
#else	// GSL is much slower
	const int N = 5e4;
#endif
	for (int i = 0;  i < N; i++)
		yData.append(i % 100);
	QVector<double> y2Data = {0,1.,.5};

	//data source columns
	Column yDataColumn("y", AbstractColumn::ColumnMode::Numeric);
	yDataColumn.replaceValues(0, yData);

	Column y2DataColumn("y2", AbstractColumn::ColumnMode::Numeric);
	y2DataColumn.replaceValues(0, y2Data);

	XYConvolutionCurve curve("convolution");
	curve.setYDataColumn(&yDataColumn);
	curve.setY2DataColumn(&y2DataColumn);

	//prepare and perform the convolution
	XYConvolutionCurve::ConvolutionData data = curve.convolutionData();
	QBENCHMARK {
		// triggers recalculate()
		curve.setConvolutionData(data);
	}

	//check the results
	const XYConvolutionCurve::ConvolutionResult& result = curve.convolutionResult();

	QCOMPARE(result.available, true);
	QCOMPARE(result.valid, true);

	const AbstractColumn* resultXDataColumn = curve.xColumn();

	const int np = resultXDataColumn->rowCount();
	QCOMPARE(np, N + 2);
}

QTEST_MAIN(ConvolutionTest)
