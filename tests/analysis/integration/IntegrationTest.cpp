/*
    File                 : IntegrationTest.cpp
    Project              : LabPlot
    Description          : Tests for numerical integration
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2018 Stefan Gerlach <stefan.gerlach@uni.kn>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "IntegrationTest.h"
#include "backend/core/column/Column.h"
#include "backend/worksheet/plots/cartesian/XYIntegrationCurve.h"

//##############################################################################

void IntegrationTest::testLinear() {
	// data
	QVector<int> xData = {1,2,3,4};
	QVector<double> yData = {1.,2.,3.,4.};

	//data source columns
	Column xDataColumn("x", AbstractColumn::ColumnMode::Integer);
	xDataColumn.replaceInteger(0, xData);

	Column yDataColumn("y", AbstractColumn::ColumnMode::Double);
	yDataColumn.replaceValues(0, yData);

	XYIntegrationCurve integrationCurve("integration");
	integrationCurve.setXDataColumn(&xDataColumn);
	integrationCurve.setYDataColumn(&yDataColumn);

	//prepare the integration
	XYIntegrationCurve::IntegrationData integrationData = integrationCurve.integrationData();
	integrationCurve.setIntegrationData(integrationData);

	//perform the integration
	integrationCurve.recalculate();
	const XYIntegrationCurve::IntegrationResult& integrationResult = integrationCurve.integrationResult();

	//check the results
	QCOMPARE(integrationResult.available, true);
	QCOMPARE(integrationResult.valid, true);

	const AbstractColumn* resultXDataColumn = integrationCurve.xColumn();
	const AbstractColumn* resultYDataColumn = integrationCurve.yColumn();

	const int np = resultXDataColumn->rowCount();
	QCOMPARE(np, 4);

	for (int i = 0; i < np; i++)
		QCOMPARE(resultXDataColumn->valueAt(i), (double)i + 1);

	QCOMPARE(resultYDataColumn->valueAt(0), 0.);
	QCOMPARE(resultYDataColumn->valueAt(1), 1.5);
	QCOMPARE(resultYDataColumn->valueAt(2), 4.);
	QCOMPARE(resultYDataColumn->valueAt(3), 7.5);
}

QTEST_MAIN(IntegrationTest)
