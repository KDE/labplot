/***************************************************************************
File                 : MatioFilterTest.cpp
Project              : LabPlot
Description          : Tests for the Matio I/O-filter.
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

#include "MatioFilterTest.h"
#include "backend/datasources/filters/MatioFilter.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/matrix/Matrix.h"

#include <KLocalizedString>

void MatioFilterTest::testImportDouble() {
	Spreadsheet spreadsheet("test", false);
	MatioFilter filter;

	const QString& fileName = QFINDTESTDATA(QLatin1String("data/testdouble_4.2c_SOL2.mat"));
	filter.setCurrentVarName(QLatin1String("testdouble"));
	const auto mode = AbstractFileFilter::ImportMode::Replace;
	filter.readDataFromFile(fileName, &spreadsheet, mode);

	QCOMPARE(spreadsheet.columnCount(), 1);
	QCOMPARE(spreadsheet.rowCount(), 9);
	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Numeric);
	QCOMPARE(spreadsheet.column(0)->plotDesignation(), AbstractColumn::PlotDesignation::X);
	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("Column 1"));
	
	//for (int i = 0; i < spreadsheet.rowCount(); i++)
	//	DEBUG(std::setprecision(15) << spreadsheet.column(0)->valueAt(i))

	QCOMPARE(spreadsheet.column(0)->valueAt(0), 0);
	QCOMPARE(spreadsheet.column(0)->valueAt(1), M_PI/4.);
	QCOMPARE(spreadsheet.column(0)->valueAt(2), M_PI/2.);
	QCOMPARE(spreadsheet.column(0)->valueAt(3), 3.*M_PI/4.);
	QCOMPARE(spreadsheet.column(0)->valueAt(4), M_PI);
	QCOMPARE(spreadsheet.column(0)->valueAt(5), 5.*M_PI/4.);
	QCOMPARE(spreadsheet.column(0)->valueAt(6), 3.*M_PI/2.);
	QCOMPARE(spreadsheet.column(0)->valueAt(7), 7.*M_PI/4.);
	QCOMPARE(spreadsheet.column(0)->valueAt(8), 2.*M_PI);

	//DEBUG(Q_FUNC_INFO << ", value = " << spreadsheet.column(0)->valueAt(0))

}

void MatioFilterTest::testImportSpreadsheet() {
	Spreadsheet spreadsheet("test", false);
	MatioFilter filter;

	const QString& fileName = QFINDTESTDATA(QLatin1String("data/testmatrix_7.4_GLNX86.mat"));
	filter.setCurrentVarName(QLatin1String("testmatrix"));
	const auto mode = AbstractFileFilter::ImportMode::Replace;
	filter.readDataFromFile(fileName, &spreadsheet, mode);

	QCOMPARE(spreadsheet.columnCount(), 3);
	QCOMPARE(spreadsheet.rowCount(), 5);
	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Numeric);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Numeric);
	QCOMPARE(spreadsheet.column(2)->columnMode(), AbstractColumn::ColumnMode::Numeric);

	QCOMPARE(spreadsheet.column(0)->plotDesignation(), AbstractColumn::PlotDesignation::X);
	QCOMPARE(spreadsheet.column(1)->plotDesignation(), AbstractColumn::PlotDesignation::Y);
	QCOMPARE(spreadsheet.column(2)->plotDesignation(), AbstractColumn::PlotDesignation::Y);

	//QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("sepallength"));
	//QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("sepalwidth"));
	//QCOMPARE(spreadsheet.column(2)->name(), QLatin1String("petallength"));

	QCOMPARE(spreadsheet.column(0)->valueAt(0), 1);
	QCOMPARE(spreadsheet.column(0)->valueAt(1), 2);
	QCOMPARE(spreadsheet.column(0)->valueAt(2), 3);
	QCOMPARE(spreadsheet.column(0)->valueAt(3), 4);
	QCOMPARE(spreadsheet.column(0)->valueAt(4), 5);
	QCOMPARE(spreadsheet.column(1)->valueAt(0), 2);
	QCOMPARE(spreadsheet.column(1)->valueAt(1), 0);
	QCOMPARE(spreadsheet.column(1)->valueAt(2), 0);
	QCOMPARE(spreadsheet.column(1)->valueAt(3), 0);
	QCOMPARE(spreadsheet.column(1)->valueAt(4), 0);
	QCOMPARE(spreadsheet.column(2)->valueAt(0), 3);
	QCOMPARE(spreadsheet.column(2)->valueAt(1), 0);
	QCOMPARE(spreadsheet.column(2)->valueAt(2), 0);
	QCOMPARE(spreadsheet.column(2)->valueAt(3), 0);
	QCOMPARE(spreadsheet.column(2)->valueAt(4), 0);

	//DEBUG(Q_FUNC_INFO << ", value = " << matrix.column(0)->valueAt(0))

}

void MatioFilterTest::testImportMatrix() {
	Matrix matrix("test", false);
	MatioFilter filter;

	const QString& fileName = QFINDTESTDATA(QLatin1String("data/testmatrix_7.4_GLNX86.mat"));
	filter.setCurrentVarName(QLatin1String("testmatrix"));
	const auto mode = AbstractFileFilter::ImportMode::Replace;
	filter.readDataFromFile(fileName, &matrix, mode);

	QCOMPARE(matrix.columnCount(), 3);
	QCOMPARE(matrix.rowCount(), 5);
	//QCOMPARE(matrix.column(0)->columnMode(), AbstractColumn::ColumnMode::Numeric);
	//QCOMPARE(matrix.column(1)->columnMode(), AbstractColumn::ColumnMode::Numeric);
	//QCOMPARE(matrix.column(2)->columnMode(), AbstractColumn::ColumnMode::Numeric);

	//QCOMPARE(matrix.column(0)->plotDesignation(), AbstractColumn::PlotDesignation::X);
	//QCOMPARE(matrix.column(1)->plotDesignation(), AbstractColumn::PlotDesignation::Y);
	//QCOMPARE(matrix.column(2)->plotDesignation(), AbstractColumn::PlotDesignation::Y);

	QCOMPARE(matrix.cell<double>(0, 0), 1.);
	QCOMPARE(matrix.cell<double>(1, 0), 2);
	QCOMPARE(matrix.cell<double>(2, 0), 3);
	QCOMPARE(matrix.cell<double>(3, 0), 4);
	QCOMPARE(matrix.cell<double>(4, 0), 5);
	QCOMPARE(matrix.cell<double>(0, 1), 2);
	QCOMPARE(matrix.cell<double>(1, 1), 0);
	QCOMPARE(matrix.cell<double>(2, 1), 0);
	QCOMPARE(matrix.cell<double>(3, 1), 0);
	QCOMPARE(matrix.cell<double>(4, 1), 0);
	QCOMPARE(matrix.cell<double>(0, 2), 3);
	QCOMPARE(matrix.cell<double>(1, 2), 0);
	QCOMPARE(matrix.cell<double>(2, 2), 0);
	QCOMPARE(matrix.cell<double>(3, 2), 0);
	QCOMPARE(matrix.cell<double>(4, 2), 0);
}

QTEST_MAIN(MatioFilterTest)
