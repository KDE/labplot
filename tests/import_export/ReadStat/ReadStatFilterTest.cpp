/*
    File                 : ReadStatFilterTest.cpp
    Project              : LabPlot
    Description          : Tests for the ReadStat I/O-filter.
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2021 Stefan Gerlach <stefan.gerlach@uni.kn>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ReadStatFilterTest.h"
#include "backend/datasources/filters/ReadStatFilter.h"
#include "backend/spreadsheet/Spreadsheet.h"

#include <KLocalizedString>

void ReadStatFilterTest::testDTAImport() {
	Spreadsheet spreadsheet("test", false);
	ReadStatFilter filter;

	const QString& fileName = QFINDTESTDATA(QLatin1String("data/iris.dta"));
	const auto mode = AbstractFileFilter::ImportMode::Replace;
	filter.readDataFromFile(fileName, &spreadsheet, mode);

	QCOMPARE(spreadsheet.columnCount(), 5);
	QCOMPARE(spreadsheet.rowCount(), 150);
	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(2)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(3)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(4)->columnMode(), AbstractColumn::ColumnMode::Text);

	QCOMPARE(spreadsheet.column(0)->plotDesignation(), AbstractColumn::PlotDesignation::X);
	QCOMPARE(spreadsheet.column(1)->plotDesignation(), AbstractColumn::PlotDesignation::Y);
	QCOMPARE(spreadsheet.column(2)->plotDesignation(), AbstractColumn::PlotDesignation::Y);
	QCOMPARE(spreadsheet.column(3)->plotDesignation(), AbstractColumn::PlotDesignation::Y);
	QCOMPARE(spreadsheet.column(4)->plotDesignation(), AbstractColumn::PlotDesignation::Y);

	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("sepallength"));
	QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("sepalwidth"));
	QCOMPARE(spreadsheet.column(2)->name(), QLatin1String("petallength"));
	QCOMPARE(spreadsheet.column(3)->name(), QLatin1String("petalwidth"));
	QCOMPARE(spreadsheet.column(4)->name(), QLatin1String("species"));

	FuzzyCompare(spreadsheet.column(0)->valueAt(0), 5.1, 1.e-6);	// 5.09999990463
	FuzzyCompare(spreadsheet.column(0)->valueAt(1), 4.9, 1.e-7);	// 4.90000009537
	FuzzyCompare(spreadsheet.column(0)->valueAt(2), 4.7, 1.e-7);	// 4.69999980927

	QCOMPARE(spreadsheet.column(1)->valueAt(0), 3.5);
	FuzzyCompare(spreadsheet.column(2)->valueAt(0), 1.4, 1.e-7);	// 1.39999997616
	FuzzyCompare(spreadsheet.column(3)->valueAt(0), 0.2, 1.e-7);	// 0.20000000298
	QCOMPARE(spreadsheet.column(4)->textAt(0), QLatin1String("setosa"));

	DEBUG(Q_FUNC_INFO << ", value = " << spreadsheet.column(0)->valueAt(149))
	DEBUG(Q_FUNC_INFO << ", value = " << spreadsheet.column(2)->valueAt(149))
	DEBUG(Q_FUNC_INFO << ", value = " << spreadsheet.column(3)->valueAt(149))

	FuzzyCompare(spreadsheet.column(0)->valueAt(149), 5.9, 1.e-7);	// 5.90000009536743
	QCOMPARE(spreadsheet.column(1)->valueAt(149), 3.);
	FuzzyCompare(spreadsheet.column(2)->valueAt(149), 5.1, 1.e-7);	// 5.09999990463257
	FuzzyCompare(spreadsheet.column(3)->valueAt(149), 1.8, 1.e-7);	// 1.79999995231628
	QCOMPARE(spreadsheet.column(4)->textAt(149), QLatin1String("virginica"));

}

void ReadStatFilterTest::testSASImport() {
	Spreadsheet spreadsheet("test", false);
	ReadStatFilter filter;

	const QString& fileName = QFINDTESTDATA(QLatin1String("data/iris.sas7bdat"));
	const auto mode = AbstractFileFilter::ImportMode::Replace;
	filter.readDataFromFile(fileName, &spreadsheet, mode);

	QCOMPARE(spreadsheet.columnCount(), 5);
	QCOMPARE(spreadsheet.rowCount(), 150);
	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(2)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(3)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(4)->columnMode(), AbstractColumn::ColumnMode::Text);

	QCOMPARE(spreadsheet.column(0)->plotDesignation(), AbstractColumn::PlotDesignation::X);
	QCOMPARE(spreadsheet.column(1)->plotDesignation(), AbstractColumn::PlotDesignation::Y);
	QCOMPARE(spreadsheet.column(2)->plotDesignation(), AbstractColumn::PlotDesignation::Y);
	QCOMPARE(spreadsheet.column(3)->plotDesignation(), AbstractColumn::PlotDesignation::Y);
	QCOMPARE(spreadsheet.column(4)->plotDesignation(), AbstractColumn::PlotDesignation::Y);

	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("Sepal_Length : BEST"));
	QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("Sepal_Width : BEST"));
	QCOMPARE(spreadsheet.column(2)->name(), QLatin1String("Petal_Length : BEST"));
	QCOMPARE(spreadsheet.column(3)->name(), QLatin1String("Petal_Width : BEST"));
	QCOMPARE(spreadsheet.column(4)->name(), QLatin1String("Species : $"));

	QCOMPARE(spreadsheet.column(0)->valueAt(0), 5.1);
	QCOMPARE(spreadsheet.column(0)->valueAt(1), 4.9);
	QCOMPARE(spreadsheet.column(0)->valueAt(2), 4.7);

	QCOMPARE(spreadsheet.column(1)->valueAt(0), 3.5);
	QCOMPARE(spreadsheet.column(2)->valueAt(0), 1.4);
	QCOMPARE(spreadsheet.column(3)->valueAt(0), 0.2);
	QCOMPARE(spreadsheet.column(4)->textAt(0), QLatin1String("setosa"));

	QCOMPARE(spreadsheet.column(0)->valueAt(149), 5.9);
	QCOMPARE(spreadsheet.column(1)->valueAt(149), 3.);
	QCOMPARE(spreadsheet.column(2)->valueAt(149), 5.1);
	QCOMPARE(spreadsheet.column(3)->valueAt(149), 1.8);
	QCOMPARE(spreadsheet.column(4)->textAt(149), QLatin1String("virgin"));

}

void ReadStatFilterTest::testSAVImport() {
	Spreadsheet spreadsheet("test", false);
	ReadStatFilter filter;

	const QString& fileName = QFINDTESTDATA(QLatin1String("data/iris.sav"));
	const auto mode = AbstractFileFilter::ImportMode::Replace;
	filter.readDataFromFile(fileName, &spreadsheet, mode);

	QCOMPARE(spreadsheet.columnCount(), 5);
	QCOMPARE(spreadsheet.rowCount(), 150);
	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(2)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(3)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(4)->columnMode(), AbstractColumn::ColumnMode::Double);

	QCOMPARE(spreadsheet.column(0)->plotDesignation(), AbstractColumn::PlotDesignation::X);
	QCOMPARE(spreadsheet.column(1)->plotDesignation(), AbstractColumn::PlotDesignation::Y);
	QCOMPARE(spreadsheet.column(2)->plotDesignation(), AbstractColumn::PlotDesignation::Y);
	QCOMPARE(spreadsheet.column(3)->plotDesignation(), AbstractColumn::PlotDesignation::Y);
	QCOMPARE(spreadsheet.column(4)->plotDesignation(), AbstractColumn::PlotDesignation::Y);

	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("Sepal.Length"));
	QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("Sepal.Width"));
	QCOMPARE(spreadsheet.column(2)->name(), QLatin1String("Petal.Length"));
	QCOMPARE(spreadsheet.column(3)->name(), QLatin1String("Petal.Width"));
	QCOMPARE(spreadsheet.column(4)->name(), QLatin1String("Species : labels0"));

	QCOMPARE(spreadsheet.column(0)->valueAt(0), 5.1);
	QCOMPARE(spreadsheet.column(0)->valueAt(1), 4.9);
	QCOMPARE(spreadsheet.column(0)->valueAt(2), 4.7);

	QCOMPARE(spreadsheet.column(1)->valueAt(0), 3.5);
	QCOMPARE(spreadsheet.column(2)->valueAt(0), 1.4);
	QCOMPARE(spreadsheet.column(3)->valueAt(0), 0.2);
	QCOMPARE(spreadsheet.column(4)->valueAt(0), 1);

	QCOMPARE(spreadsheet.column(0)->valueAt(149), 5.9);
	QCOMPARE(spreadsheet.column(1)->valueAt(149), 3.);
	QCOMPARE(spreadsheet.column(2)->valueAt(149), 5.1);
	QCOMPARE(spreadsheet.column(3)->valueAt(149), 1.8);
	QCOMPARE(spreadsheet.column(4)->valueAt(149), 3);

	//check value label
	QCOMPARE(spreadsheet.column(4)->valueLabels().value(1), QLatin1String("setosa"));
	QCOMPARE(spreadsheet.column(4)->valueLabels().value(2), QLatin1String("versicolor"));
	QCOMPARE(spreadsheet.column(4)->valueLabels().value(3), QLatin1String("virginica"));

}

void ReadStatFilterTest::testPORImport() {
	Spreadsheet spreadsheet("test", false);
	ReadStatFilter filter;

	const QString& fileName = QFINDTESTDATA(QLatin1String("data/sample.por"));
	const auto mode = AbstractFileFilter::ImportMode::Replace;
	filter.readDataFromFile(fileName, &spreadsheet, mode);

	QCOMPARE(spreadsheet.columnCount(), 7);
	QCOMPARE(spreadsheet.rowCount(), 5);
	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Text);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(2)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(3)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(4)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(5)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(6)->columnMode(), AbstractColumn::ColumnMode::Double);

	QCOMPARE(spreadsheet.column(0)->plotDesignation(), AbstractColumn::PlotDesignation::X);
	QCOMPARE(spreadsheet.column(1)->plotDesignation(), AbstractColumn::PlotDesignation::Y);
	QCOMPARE(spreadsheet.column(2)->plotDesignation(), AbstractColumn::PlotDesignation::Y);
	QCOMPARE(spreadsheet.column(3)->plotDesignation(), AbstractColumn::PlotDesignation::Y);
	QCOMPARE(spreadsheet.column(4)->plotDesignation(), AbstractColumn::PlotDesignation::Y);
	QCOMPARE(spreadsheet.column(5)->plotDesignation(), AbstractColumn::PlotDesignation::Y);
	QCOMPARE(spreadsheet.column(6)->plotDesignation(), AbstractColumn::PlotDesignation::Y);

	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("MYCHAR"));
	QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("MYNUM"));
	QCOMPARE(spreadsheet.column(2)->name(), QLatin1String("MYDATE"));
	QCOMPARE(spreadsheet.column(3)->name(), QLatin1String("DTIME"));
	QCOMPARE(spreadsheet.column(4)->name(), QLatin1String("MYLABL : labels0"));
	QCOMPARE(spreadsheet.column(5)->name(), QLatin1String("MYORD : labels1"));
	QCOMPARE(spreadsheet.column(6)->name(), QLatin1String("MYTIME"));

	QCOMPARE(spreadsheet.column(0)->textAt(0), QLatin1String("a"));
	QCOMPARE(spreadsheet.column(0)->textAt(1), QLatin1String("b"));
	QCOMPARE(spreadsheet.column(0)->textAt(2), QLatin1String("c"));
	QCOMPARE(spreadsheet.column(0)->textAt(3), QLatin1String("d"));
	QCOMPARE(spreadsheet.column(0)->textAt(4), QLatin1String("e"));

	QCOMPARE(spreadsheet.column(1)->valueAt(0), 1.1);
	QCOMPARE(spreadsheet.column(1)->valueAt(1), 1.2);
	QCOMPARE(spreadsheet.column(1)->valueAt(2), -1000.3);
	QCOMPARE(spreadsheet.column(1)->valueAt(3), -1.4);
	QCOMPARE(spreadsheet.column(1)->valueAt(4), 1000.3);

	QCOMPARE(spreadsheet.column(2)->valueAt(0), 1.3744944e10);
	QCOMPARE(spreadsheet.column(2)->valueAt(1), 9.3901248e9);
	QCOMPARE(spreadsheet.column(2)->valueAt(2), 1.190376e10);
	QCOMPARE(spreadsheet.column(2)->valueAt(3), 6.8256e6);
	QCOMPARE(spreadsheet.column(2)->valueAt(4), qQNaN());

	QCOMPARE(spreadsheet.column(3)->valueAt(0), 1.374498061e10);
	QCOMPARE(spreadsheet.column(3)->valueAt(1), 9.39016141e9);
	QCOMPARE(spreadsheet.column(3)->valueAt(2), 1.190376e10);
	QCOMPARE(spreadsheet.column(3)->valueAt(3), 6.8256e6);
	QCOMPARE(spreadsheet.column(3)->valueAt(4), qQNaN());

	QCOMPARE(spreadsheet.column(4)->valueAt(0), 1);
	QCOMPARE(spreadsheet.column(4)->valueAt(1), 2);
	QCOMPARE(spreadsheet.column(4)->valueAt(2), 1);
	QCOMPARE(spreadsheet.column(4)->valueAt(3), 2);
	QCOMPARE(spreadsheet.column(4)->valueAt(4), 1);

	QCOMPARE(spreadsheet.column(5)->valueAt(0), 1);
	QCOMPARE(spreadsheet.column(5)->valueAt(1), 2);
	QCOMPARE(spreadsheet.column(5)->valueAt(2), 3);
	QCOMPARE(spreadsheet.column(5)->valueAt(3), 1);
	QCOMPARE(spreadsheet.column(5)->valueAt(4), 1);

	QCOMPARE(spreadsheet.column(6)->valueAt(0), 36610);
	QCOMPARE(spreadsheet.column(6)->valueAt(1), 83410);
	QCOMPARE(spreadsheet.column(6)->valueAt(2), 0);
	QCOMPARE(spreadsheet.column(6)->valueAt(3), 58210);
	QCOMPARE(spreadsheet.column(6)->valueAt(4), qQNaN());

	//check value label
	QCOMPARE(spreadsheet.column(4)->valueLabels().value(1), QLatin1String("Male"));
	QCOMPARE(spreadsheet.column(4)->valueLabels().value(2), QLatin1String("Female"));
	QCOMPARE(spreadsheet.column(5)->valueLabels().value(1), QLatin1String("low"));
	QCOMPARE(spreadsheet.column(5)->valueLabels().value(2), QLatin1String("medium"));
	QCOMPARE(spreadsheet.column(5)->valueLabels().value(3), QLatin1String("high"));
}

void ReadStatFilterTest::testXPTImport() {
	Spreadsheet spreadsheet("test", false);
	ReadStatFilter filter;

	const QString& fileName = QFINDTESTDATA(QLatin1String("data/sample.xpt"));
	const auto mode = AbstractFileFilter::ImportMode::Replace;
	filter.readDataFromFile(fileName, &spreadsheet, mode);

	QCOMPARE(spreadsheet.columnCount(), 7);
	QCOMPARE(spreadsheet.rowCount(), 5);
	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Text);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(2)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(3)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(4)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(5)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(6)->columnMode(), AbstractColumn::ColumnMode::Double);

	QCOMPARE(spreadsheet.column(0)->plotDesignation(), AbstractColumn::PlotDesignation::X);
	QCOMPARE(spreadsheet.column(1)->plotDesignation(), AbstractColumn::PlotDesignation::Y);
	QCOMPARE(spreadsheet.column(2)->plotDesignation(), AbstractColumn::PlotDesignation::Y);
	QCOMPARE(spreadsheet.column(3)->plotDesignation(), AbstractColumn::PlotDesignation::Y);
	QCOMPARE(spreadsheet.column(4)->plotDesignation(), AbstractColumn::PlotDesignation::Y);
	QCOMPARE(spreadsheet.column(5)->plotDesignation(), AbstractColumn::PlotDesignation::Y);
	QCOMPARE(spreadsheet.column(6)->plotDesignation(), AbstractColumn::PlotDesignation::Y);

	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("MYCHAR : $1"));
	QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("MYNUM : BEST12"));
	QCOMPARE(spreadsheet.column(2)->name(), QLatin1String("MYDATE : YYMMDD10"));
	QCOMPARE(spreadsheet.column(3)->name(), QLatin1String("DTIME : DATETIME"));
	QCOMPARE(spreadsheet.column(4)->name(), QLatin1String("MYLABL : BEST12"));
	QCOMPARE(spreadsheet.column(5)->name(), QLatin1String("MYORD : BEST12"));
	QCOMPARE(spreadsheet.column(6)->name(), QLatin1String("MYTIME : TIME20.3"));

	QCOMPARE(spreadsheet.column(0)->textAt(0), QLatin1String("a"));
	QCOMPARE(spreadsheet.column(0)->textAt(1), QLatin1String("b"));
	QCOMPARE(spreadsheet.column(0)->textAt(2), QLatin1String("c"));
	QCOMPARE(spreadsheet.column(0)->textAt(3), QLatin1String("d"));
	QCOMPARE(spreadsheet.column(0)->textAt(4), QLatin1String("e"));

	QCOMPARE(spreadsheet.column(1)->valueAt(0), 1.1);
	QCOMPARE(spreadsheet.column(1)->valueAt(1), 1.2);
	QCOMPARE(spreadsheet.column(1)->valueAt(2), -1000.3);
	QCOMPARE(spreadsheet.column(1)->valueAt(3), -1.4);
	QCOMPARE(spreadsheet.column(1)->valueAt(4), 1000.3);

	QCOMPARE(spreadsheet.column(2)->valueAt(0), 21310);
	QCOMPARE(spreadsheet.column(2)->valueAt(1), -29093);
	QCOMPARE(spreadsheet.column(2)->valueAt(2), 0);
	QCOMPARE(spreadsheet.column(2)->valueAt(3), -137696);
	QCOMPARE(spreadsheet.column(2)->valueAt(4), qQNaN());

	QCOMPARE(spreadsheet.column(3)->valueAt(0), 1.84122061e9);
	QCOMPARE(spreadsheet.column(3)->valueAt(1), -2.51359859e9);
	QCOMPARE(spreadsheet.column(3)->valueAt(2), 0);
	QCOMPARE(spreadsheet.column(3)->valueAt(3), -1.18969344e10);
	QCOMPARE(spreadsheet.column(3)->valueAt(4), qQNaN());

	QCOMPARE(spreadsheet.column(4)->valueAt(0), 1);
	QCOMPARE(spreadsheet.column(4)->valueAt(1), 2);
	QCOMPARE(spreadsheet.column(4)->valueAt(2), 1);
	QCOMPARE(spreadsheet.column(4)->valueAt(3), 2);
	QCOMPARE(spreadsheet.column(4)->valueAt(4), 1);

	QCOMPARE(spreadsheet.column(5)->valueAt(0), 1);
	QCOMPARE(spreadsheet.column(5)->valueAt(1), 2);
	QCOMPARE(spreadsheet.column(5)->valueAt(2), 3);
	QCOMPARE(spreadsheet.column(5)->valueAt(3), 1);
	QCOMPARE(spreadsheet.column(5)->valueAt(4), 1);

	QCOMPARE(spreadsheet.column(6)->valueAt(0), 36610);
	QCOMPARE(spreadsheet.column(6)->valueAt(1), 83410);
	QCOMPARE(spreadsheet.column(6)->valueAt(2), 0);
	QCOMPARE(spreadsheet.column(6)->valueAt(3), 58210);
	QCOMPARE(spreadsheet.column(6)->valueAt(4), qQNaN());

	// no value label
}

QTEST_MAIN(ReadStatFilterTest)
