/***************************************************************************
    File                 : IntegrationTest.cpp
    Project              : LabPlot
    Description          : Tests for numerical integration
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

	Column yDataColumn("y", AbstractColumn::ColumnMode::Numeric);
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
