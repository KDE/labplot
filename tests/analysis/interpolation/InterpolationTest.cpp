/*
	File                 : InterpolationTest.cpp
	Project              : LabPlot
	Description          : Tests for numerical interpolation
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2016-2023 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "InterpolationTest.h"
#include "backend/core/column/Column.h"
#include "backend/worksheet/plots/cartesian/XYInterpolationCurve.h"

// ##############################################################################

/*!
 * use a dataset and the 1d interpolationg grid for x that leads to the value of x that is bigger
 * than the maximal value in the data. make sure this case is handled correctly and we get a valid result.
 * s.a. https://invent.kde.org/education/labplot/-/issues/841
 */
void InterpolationTest::testRanges() {
	// data
	QVector<double> xData = {0.0001017011754038652,
							 0.0004975011506471159,
							 0.00102695699595614,
							 0.001966348318553697,
							 0.2242262396919622,
							 0.3605485906366224,
							 0.617114010419536,
							 1.017363907550924,
							 10.14814650334548};
	QVector<double> yData = {21.65343311865348,
							 21.59965712171775,
							 10.05153706441572,
							 2.836184922640735,
							 0.09417257716127328,
							 0.08249127667969287,
							 0.06333674921226348,
							 0.04736857950015065,
							 0.04597133454545185};

	// data source columns
	Column xDataColumn(QStringLiteral("x"), AbstractColumn::ColumnMode::Double);
	xDataColumn.replaceValues(0, xData);

	Column yDataColumn(QStringLiteral("y"), AbstractColumn::ColumnMode::Double);
	yDataColumn.replaceValues(0, yData);

	// interpolation curve
	XYInterpolationCurve curve(QStringLiteral("interpolation"));
	curve.setXDataColumn(&xDataColumn);
	curve.setYDataColumn(&yDataColumn);

	// prepare the interpolation
	// set the number of points to 27 which is leading for the original data to and for the grid points
	// defined via x_i = xmin + i * (xmax - xmin) / (npoints - 1) to a values that is bigger than xmax
	// because of issues with the representation of float numbers.
	auto data = curve.interpolationData();
	data.npoints = 27;
	curve.setInterpolationData(data);

	// perform the interpolation
	curve.recalculate();

	// check the results
	const auto& result = curve.result();
	QCOMPARE(result.available, true);
	QCOMPARE(result.valid, true);
}

QTEST_MAIN(InterpolationTest)
