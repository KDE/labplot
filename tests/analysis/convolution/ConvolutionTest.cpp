/***************************************************************************
    File                 : ConvolutionTest.cpp
    Project              : LabPlot
    Description          : Tests for data convolution
    --------------------------------------------------------------------
    Copyright            : (C) 2018 Stefan Gerlach (stefan.gerlach@uni.kn)
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

#include "ConvolutionTest.h"
#include "backend/core/column/Column.h"
#include "backend/worksheet/plots/cartesian/XYConvolutionCurve.h"

extern "C" {
#include "backend/nsl/nsl_sf_stats.h"
#include "backend/nsl/nsl_stats.h"
}

void ConvolutionTest::initTestCase() {
	// needed in order to have the signals triggered by SignallingUndoCommand, see LabPlot.cpp
	//TODO: redesign/remove this
	qRegisterMetaType<const AbstractAspect*>("const AbstractAspect*");
	qRegisterMetaType<const AbstractColumn*>("const AbstractColumn*");
}

//##############################################################################
//TODO: without x

void ConvolutionTest::test1() {
	// data
	QVector<int> xData = {0,1,2,3};
	QVector<double> yData = {1.,2.,3.,4.};
	QVector<double> y2Data = {0,1.,.5};

	//data source columns
	Column xDataColumn("x", AbstractColumn::Integer);
	xDataColumn.replaceInteger(0, xData);

	Column yDataColumn("y", AbstractColumn::Numeric);
	yDataColumn.replaceValues(0, yData);

	Column y2DataColumn("y2", AbstractColumn::Numeric);
	y2DataColumn.replaceValues(0, y2Data);

	XYConvolutionCurve convolutionCurve("convolution");
	convolutionCurve.setXDataColumn(&xDataColumn);
	convolutionCurve.setYDataColumn(&yDataColumn);
	convolutionCurve.setY2DataColumn(&y2DataColumn);

	//prepare the convolution
	XYConvolutionCurve::ConvolutionData convolutionData = convolutionCurve.convolutionData();
	//TODO: options
	//fitData.modelCategory = nsl_fit_model_basic;
	//...
	convolutionCurve.setConvolutionData(convolutionData);

	//perform the convolution
	convolutionCurve.recalculate();
	const XYConvolutionCurve::ConvolutionResult& convolutionResult = convolutionCurve.convolutionResult();

	//check the results
	QCOMPARE(convolutionResult.available, true);
	QCOMPARE(convolutionResult.valid, true);

	const AbstractColumn* resultXDataColumn = convolutionCurve.xColumn();
	const AbstractColumn* resultYDataColumn = convolutionCurve.yColumn();

	const int np = resultXDataColumn->rowCount();
	QCOMPARE(np, 6);

	for (int i = 0; i < 4; i++)	//TODO: other values are zero!
		QCOMPARE(resultXDataColumn->valueAt(i), (double)i);

	QCOMPARE(resultYDataColumn->valueAt(0), 0.);
	QCOMPARE(resultYDataColumn->valueAt(1), 1.);
	QCOMPARE(resultYDataColumn->valueAt(2), 2.5);
	QCOMPARE(resultYDataColumn->valueAt(3), 4.);
	QCOMPARE(resultYDataColumn->valueAt(4), 5.5);
	QCOMPARE(resultYDataColumn->valueAt(5), 2.);

}

QTEST_MAIN(ConvolutionTest)
