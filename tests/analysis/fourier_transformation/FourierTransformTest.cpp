/*
	File                 : FourierTransformTest.cpp
	Project              : LabPlot
	Description          : Tests for discrete Fourier transformation
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Martin Marmsoler <martin.marmsoler@gmail.com>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "FourierTransformTest.h"
#include "backend/core/column/Column.h"
#include "backend/worksheet/plots/cartesian/XYFourierTransformCurve.h"

#include <iostream>
//#include <math.h>

// ##############################################################################

// https://de.mathworks.com/help/matlab/ref/fft.html
void FourierTransformTest::fft() {
	constexpr int length_signal = 1500;
	constexpr double fs = 1000.0; // [Hz]

	// data
	QVector<double> time(length_signal);
	QVector<double> yData(length_signal);

	for (size_t i = 0; i < length_signal; i++) {
		const auto t = (double)i / fs;
		time[i] = t;
		yData[i] = 0.8 + 0.7 * sin(2.0 * M_PI * 50.0 * t) + sin(2.0 * M_PI * 120.0 * t);
	}

	// data source columns
	Column xDataColumn(QStringLiteral("time"), AbstractColumn::ColumnMode::Double);
	xDataColumn.replaceValues(0, time);

	Column yDataColumn(QStringLiteral("y"), AbstractColumn::ColumnMode::Double);
	yDataColumn.replaceValues(0, yData);

	XYFourierTransformCurve curve(QStringLiteral("fourier transform"));
	curve.setXDataColumn(&xDataColumn);
	curve.setYDataColumn(&yDataColumn);

	// prepare the fourier transform data
	auto data = curve.transformData();
	QCOMPARE(data.type, nsl_dft_result_amplitude);
	curve.setTransformData(data);

	// perform the calculation
	curve.recalculate();
	const auto& result = curve.result();

	// check the results
	QCOMPARE(result.available, true);
	QCOMPARE(result.valid, true);

	const auto* resultXDataColumn = curve.xColumn();
	const auto* resultYDataColumn = curve.yColumn();

	const int np = resultXDataColumn->rowCount();
	QCOMPARE(np, length_signal / 2);

	for (int i = 0; i < np; i++) {
		const auto x = resultXDataColumn->valueAt(i);
		auto y = resultYDataColumn->valueAt(i);

		// std::cout << i << ", x: " << x << ", y: " << std::endl;
		//  Three peaks:
		//  - first at zero
		//  - second at 50Hz
		//  - third at 120Hz
		if (x == 0)
			VALUES_EQUAL(y, 0.8);
		else if (x == 50)
			VALUES_EQUAL(y, 0.7);
		else if (x == 120)
			VALUES_EQUAL(y, 1.0);
		else
			QVERIFY(std::abs(y) < 1e-12);
	}
}

QTEST_MAIN(FourierTransformTest)
