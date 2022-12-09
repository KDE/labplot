/*
	File                 : FourierTest.cpp
	Project              : LabPlot
	Description          : Tests for fourier filtering
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022 Martin Marmsoler <martin.marmsoler@gmail.com>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "FourierTest.h"
#include "backend/core/column/Column.h"
#include "backend/worksheet/plots/cartesian/XYFourierFilterCurve.h"
#include "backend/worksheet/plots/cartesian/XYFourierFilterCurvePrivate.h"

//##############################################################################

const QString dataPath = QStringLiteral("data/"); // relative path

#define READ_DATA(filename)                                                                                                                                    \
	auto filepath = QFINDTESTDATA(dataPath + filename);                                                                                                        \
	QVector<QVector<double>> data;                                                                                                                             \
	do {                                                                                                                                                       \
		QFile f(filepath);                                                                                                                                     \
		QCOMPARE(f.open(QIODevice::ReadOnly), true);                                                                                                           \
                                                                                                                                                               \
		while (!f.atEnd()) {                                                                                                                                   \
			const QString line = QLatin1String(f.readLine().simplified());                                                                                     \
			const auto entries = line.split(QLatin1Char(','));                                                                                                 \
			QVector<double> values;                                                                                                                            \
			for (const auto& entry : entries) {                                                                                                                \
				bool ok;                                                                                                                                       \
				double v = entry.toDouble(&ok);                                                                                                                \
				QVERIFY(ok);                                                                                                                                   \
				values.append(v);                                                                                                                              \
			}                                                                                                                                                  \
			data.append(values);                                                                                                                               \
		}                                                                                                                                                      \
		QVERIFY(data.count() > 0);                                                                                                                             \
		/* Check that all rows have the same number of values */                                                                                               \
		const auto length = data.at(0).length();                                                                                                               \
		for (const auto& v : data)                                                                                                                             \
			QCOMPARE(v.length(), length);                                                                                                                      \
	} while (false);

/*!
 * Writing the results to a file to validate them
 */
#define WRITE_RESULT_DATA(xData, yData, filteredDataRef, filteredData)                                                                                         \
	do {                                                                                                                                                       \
		QFile f(filepath + QStringLiteral("_testOutput.csv"));                                                                                                 \
		QCOMPARE(f.open(QIODevice::WriteOnly), true);                                                                                                          \
                                                                                                                                                               \
		f.write("x,y,filtered_reference, filtered_calculated\n"); /* header */                                                                                 \
		for (int i = 0; i < xData.length(); i++) {                                                                                                             \
			f.write(QStringLiteral("%1,%2,%3,%4\n").arg(xData.at(i)).arg(yData.at(i)).arg(filteredDataRef.at(i)).arg(filteredData.at(i)).toLatin1());          \
		}                                                                                                                                                      \
	} while (false);

void FourierTest::lowPassButterWorth() {
	const QString filename = QStringLiteral("butterworth.csv");
	READ_DATA(filename);

	// data
	const auto xData = data.at(0);
	const auto yData = data.at(1);
	const auto filteredDataRef = data.at(2);

	// data source columns
	Column xDataColumn(QStringLiteral("x"), AbstractColumn::ColumnMode::Double);
	xDataColumn.replaceValues(0, xData);

	Column yDataColumn(QStringLiteral("y"), AbstractColumn::ColumnMode::Double);
	yDataColumn.replaceValues(0, yData);

	XYFourierFilterCurve curve(QStringLiteral("fourierFiltering"));
	curve.setXDataColumn(&xDataColumn);
	curve.setYDataColumn(&yDataColumn);

	// prepare the filtering
	auto filterData = curve.filterData();
	filterData.type = nsl_filter_type_low_pass;
	filterData.form = nsl_filter_form_butterworth;
	filterData.autoRange;
	filterData.cutoff = 100; // [Hz]
	filterData.cutoff2;
	filterData.order;
	filterData.unit;
	filterData.unit2;
	filterData.xRange;
	curve.setFilterData(filterData);

	// perform the differentiation
	curve.recalculate();
	const XYFourierFilterCurve::FilterResult& results = curve.filterResult();

	QVERIFY(results.available);
	QVERIFY(results.valid);

	const auto xVec = *(curve.d_func()->xVector);
	const auto yVec = *(curve.d_func()->yVector);
	WRITE_RESULT_DATA(xData, yData, filteredDataRef, yVec);

	COMPARE_DOUBLE_VECTORS(xVec, xData);
	COMPARE_DOUBLE_VECTORS(yVec, filteredDataRef);
}

QTEST_MAIN(FourierTest)
