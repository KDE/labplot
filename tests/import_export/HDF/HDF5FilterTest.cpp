/***************************************************************************
File                 : HDF5FilterTest.cpp
Project              : LabPlot
Description          : Tests for the HDF5 I/O-filter.
--------------------------------------------------------------------
--------------------------------------------------------------------
Copyright            : (C) 2021 Stefan Gerlach (stefan.gerlach@uni.kn)

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

#include "HDF5FilterTest.h"
#include "backend/datasources/filters/HDF5Filter.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/matrix/Matrix.h"

#include <KLocalizedString>

void HDF5FilterTest::testImportDouble() {
	Spreadsheet spreadsheet("test", false);
	HDF5Filter filter;

	const QString& fileName = QFINDTESTDATA(QLatin1String("data/hdf5_test.h5"));
	filter.setCurrentDataSetName(QLatin1String("/arrays/2D float array"));
	const auto mode = AbstractFileFilter::ImportMode::Replace;
	filter.readDataFromFile(fileName, &spreadsheet, mode);

	QCOMPARE(spreadsheet.columnCount(), 50);
	QCOMPARE(spreadsheet.rowCount(), 100);
	for (int i = 0; i < spreadsheet.columnCount(); i++) {
		QCOMPARE(spreadsheet.column(i)->columnMode(), AbstractColumn::ColumnMode::Numeric);
		QCOMPARE(spreadsheet.column(i)->name(), QLatin1String("2D float array_") + QString::number(i+1));
	}

	QCOMPARE(spreadsheet.column(0)->plotDesignation(), AbstractColumn::PlotDesignation::X);
	for (int i = 1; i < spreadsheet.columnCount(); i++)
		QCOMPARE(spreadsheet.column(i)->plotDesignation(), AbstractColumn::PlotDesignation::Y);

	for (int i = 0; i < 4; i++)
	for (int j = 0; j < 4; j++)
		DEBUG(std::setprecision(15) << spreadsheet.column(j)->valueAt(i))

	QCOMPARE(spreadsheet.column(0)->valueAt(0), 0.000123456804431044);
	QCOMPARE(spreadsheet.column(1)->valueAt(0), 0.001234499970451);
	QCOMPARE(spreadsheet.column(2)->valueAt(0), 0);
	QCOMPARE(spreadsheet.column(3)->valueAt(0), 0);
	QCOMPARE(spreadsheet.column(0)->valueAt(1), 0);
	QCOMPARE(spreadsheet.column(1)->valueAt(1), 0.899999976158142);
	QCOMPARE(spreadsheet.column(2)->valueAt(1), 1.70000004768372);
	QCOMPARE(spreadsheet.column(3)->valueAt(1), 2.59999990463257);
	QCOMPARE(spreadsheet.column(0)->valueAt(2), 0);
	QCOMPARE(spreadsheet.column(1)->valueAt(2), 1.70000004768372);
	QCOMPARE(spreadsheet.column(2)->valueAt(2), 3.5);
	QCOMPARE(spreadsheet.column(3)->valueAt(2), 5.19999980926514);
	QCOMPARE(spreadsheet.column(0)->valueAt(3), 0.170000001788139);
	QCOMPARE(spreadsheet.column(1)->valueAt(3), 2.59999990463257);
	QCOMPARE(spreadsheet.column(2)->valueAt(3), 3.5);
	QCOMPARE(spreadsheet.column(3)->valueAt(3), 7.80000019073486);
}

void HDF5FilterTest::testImportDoublePortion() {
	Spreadsheet spreadsheet("test", false);
	HDF5Filter filter;

	const QString& fileName = QFINDTESTDATA(QLatin1String("data/hdf5_test.h5"));
	filter.setCurrentDataSetName(QLatin1String("/arrays/2D float array"));
	const auto mode = AbstractFileFilter::ImportMode::Replace;
	// set start/end row/col
	filter.setStartRow(2);
	filter.setEndRow(3);
	filter.setStartColumn(2);
	filter.setEndColumn(3);
	filter.readDataFromFile(fileName, &spreadsheet, mode);

	QCOMPARE(spreadsheet.columnCount(), 2);
	QCOMPARE(spreadsheet.rowCount(), 2);
	for (int i = 0; i < 2; i++) {
		QCOMPARE(spreadsheet.column(i)->columnMode(), AbstractColumn::ColumnMode::Numeric);
		QCOMPARE(spreadsheet.column(i)->name(), QLatin1String("2D float array_") + QString::number(i+1));
	}

	QCOMPARE(spreadsheet.column(0)->plotDesignation(), AbstractColumn::PlotDesignation::X);
	QCOMPARE(spreadsheet.column(1)->plotDesignation(), AbstractColumn::PlotDesignation::Y);

	for (int i = 0; i < 2; i++)
	for (int j = 0; j < 2; j++)
		DEBUG(std::setprecision(15) << spreadsheet.column(j)->valueAt(i))

	QCOMPARE(spreadsheet.column(0)->valueAt(0), 0.899999976158142);
	QCOMPARE(spreadsheet.column(0)->valueAt(1), 1.70000004768372);
	QCOMPARE(spreadsheet.column(1)->valueAt(0), 1.70000004768372);
	QCOMPARE(spreadsheet.column(1)->valueAt(1), 3.5);
}

void HDF5FilterTest::testImportInt() {
	Spreadsheet spreadsheet("test", false);
	HDF5Filter filter;

	const QString& fileName = QFINDTESTDATA(QLatin1String("data/hdf5_test.h5"));
	filter.setCurrentDataSetName(QLatin1String("/arrays/2D int array"));
	const auto mode = AbstractFileFilter::ImportMode::Replace;
	filter.readDataFromFile(fileName, &spreadsheet, mode);

	QCOMPARE(spreadsheet.columnCount(), 50);
	QCOMPARE(spreadsheet.rowCount(), 100);
	for (int i = 0; i < spreadsheet.columnCount(); i++) {
		QCOMPARE(spreadsheet.column(i)->columnMode(), AbstractColumn::ColumnMode::Integer);
		QCOMPARE(spreadsheet.column(i)->name(), QLatin1String("2D int array_") + QString::number(i+1));
	}

	QCOMPARE(spreadsheet.column(0)->plotDesignation(), AbstractColumn::PlotDesignation::X);
	for (int i = 1; i < spreadsheet.columnCount(); i++)
		QCOMPARE(spreadsheet.column(i)->plotDesignation(), AbstractColumn::PlotDesignation::Y);

	QCOMPARE(spreadsheet.column(0)->valueAt(0), 1000);
	QCOMPARE(spreadsheet.column(1)->valueAt(0), 1001);
	QCOMPARE(spreadsheet.column(2)->valueAt(0), 1002);
	QCOMPARE(spreadsheet.column(3)->valueAt(0), 1003);
	QCOMPARE(spreadsheet.column(0)->valueAt(1), 1100);
	QCOMPARE(spreadsheet.column(1)->valueAt(1), 1101);
	QCOMPARE(spreadsheet.column(2)->valueAt(1), 1102);
	QCOMPARE(spreadsheet.column(3)->valueAt(1), 1103);
	QCOMPARE(spreadsheet.column(0)->valueAt(2), 1200);
	QCOMPARE(spreadsheet.column(1)->valueAt(2), 1207);
	QCOMPARE(spreadsheet.column(2)->valueAt(2), 1202);
	QCOMPARE(spreadsheet.column(3)->valueAt(2), 1203);
	QCOMPARE(spreadsheet.column(0)->valueAt(3), 1300);
	QCOMPARE(spreadsheet.column(1)->valueAt(3), 1301);
	QCOMPARE(spreadsheet.column(2)->valueAt(3), 1302);
	QCOMPARE(spreadsheet.column(3)->valueAt(3), 1303);
}

void HDF5FilterTest::testImportIntPortion() {
	Spreadsheet spreadsheet("test", false);
	HDF5Filter filter;

	const QString& fileName = QFINDTESTDATA(QLatin1String("data/hdf5_test.h5"));
	filter.setCurrentDataSetName(QLatin1String("/arrays/2D int array"));
	const auto mode = AbstractFileFilter::ImportMode::Replace;
	// set start/end row/col
	filter.setStartRow(2);
	filter.setEndRow(3);
	filter.setStartColumn(2);
	filter.setEndColumn(3);
	filter.readDataFromFile(fileName, &spreadsheet, mode);

	QCOMPARE(spreadsheet.columnCount(), 2);
	QCOMPARE(spreadsheet.rowCount(), 2);
	for (int i = 0; i < 2; i++) {
		QCOMPARE(spreadsheet.column(i)->columnMode(), AbstractColumn::ColumnMode::Integer);
		QCOMPARE(spreadsheet.column(i)->name(), QLatin1String("2D int array_") + QString::number(i+1));
	}

	QCOMPARE(spreadsheet.column(0)->plotDesignation(), AbstractColumn::PlotDesignation::X);
	QCOMPARE(spreadsheet.column(1)->plotDesignation(), AbstractColumn::PlotDesignation::Y);

	QCOMPARE(spreadsheet.column(0)->valueAt(0), 1101);
	QCOMPARE(spreadsheet.column(1)->valueAt(0), 1102);
	QCOMPARE(spreadsheet.column(0)->valueAt(1), 1207);
	QCOMPARE(spreadsheet.column(1)->valueAt(1), 1202);
}

QTEST_MAIN(HDF5FilterTest)
