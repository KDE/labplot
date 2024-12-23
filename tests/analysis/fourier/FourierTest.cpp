/*
	File                 : FourierTest.cpp
	Project              : LabPlot
	Description          : Tests for fourier filtering
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022 Martin Marmsoler <martin.marmsoler@gmail.com>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "FourierTest.h"
#include "backend/core/Project.h"
#include "backend/core/column/Column.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/worksheet/plots/cartesian/XYFourierFilterCurve.h"
#include "backend/worksheet/plots/cartesian/XYAnalysisCurvePrivate.h"
#include "backend/worksheet/plots/cartesian/XYCurvePrivate.h"

// ##############################################################################

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

void FourierTest::addCurve() {
	Project project;

	auto* ws = new Worksheet(QStringLiteral("Worksheet"));
	project.addChild(ws);

	auto* plot = new CartesianPlot(QStringLiteral("plot"));
	ws->addChild(plot);

	auto* sheet = new Spreadsheet(QStringLiteral("sheet"));
	project.addChild(sheet);
	sheet->setColumnCount(2);
	sheet->column(0)->setName(QStringLiteral("x"));
	sheet->column(1)->setName(QStringLiteral("y"));

	auto* curve = new XYCurve(QStringLiteral("curve"));
	plot->addChild(curve);

	curve->d_func()->setSelected(true);
	QCOMPARE(plot->currentCurve(), curve);

	plot->addFourierFilterCurve(); // Should not crash and curve should be assigned accordingly

	const auto& analysisCurves = plot->children<XYAnalysisCurve>();
	QCOMPARE(analysisCurves.count(), 1);

	QCOMPARE(analysisCurves.at(0)->d_func()->dataSourceCurve, curve);
}

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
	filterData.autoRange = true; // use complete data range
	filterData.cutoff = 100; // [Hz]
	// filterData.cutoff2; // Not relevant for lowpass filter
	filterData.order = 1;
	filterData.unit = nsl_filter_cutoff_unit_frequency;
	// filterData.unit2; // Not relevant for lowpass filter
	// filterData.xRange; // Not relevant because autoRange is true
	curve.setFilterData(filterData);

	// perform the differentiation
	curve.recalculate();
	const XYFourierFilterCurve::FilterResult& results = curve.filterResult();

	QVERIFY(results.available);
	QVERIFY(results.valid);

	// This does not work yet, because the implementation uses two side filtering,
	// considering knowing of future values (values after the current processed index)
	// Octave is using an IIR filter
	//	const auto xVec = *(curve.d_func()->xVector);
	//	const auto yVec = *(curve.d_func()->yVector);
	//	WRITE_RESULT_DATA(xData, yData, filteredDataRef, yVec);

	//	COMPARE_DOUBLE_VECTORS(xVec, xData);
	//	COMPARE_DOUBLE_VECTORS(yVec, filteredDataRef);
}

QTEST_MAIN(FourierTest)
