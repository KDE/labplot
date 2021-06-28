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

	QCOMPARE(spreadsheet.columnCount(), 5);
	QCOMPARE(spreadsheet.rowCount(), 3);
	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Numeric);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Numeric);
	QCOMPARE(spreadsheet.column(2)->columnMode(), AbstractColumn::ColumnMode::Numeric);
	QCOMPARE(spreadsheet.column(3)->columnMode(), AbstractColumn::ColumnMode::Numeric);
	QCOMPARE(spreadsheet.column(4)->columnMode(), AbstractColumn::ColumnMode::Numeric);

	QCOMPARE(spreadsheet.column(0)->plotDesignation(), AbstractColumn::PlotDesignation::X);
	QCOMPARE(spreadsheet.column(1)->plotDesignation(), AbstractColumn::PlotDesignation::Y);
	QCOMPARE(spreadsheet.column(2)->plotDesignation(), AbstractColumn::PlotDesignation::Y);
	QCOMPARE(spreadsheet.column(3)->plotDesignation(), AbstractColumn::PlotDesignation::Y);
	QCOMPARE(spreadsheet.column(4)->plotDesignation(), AbstractColumn::PlotDesignation::Y);

	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("Column 1"));
	QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("Column 2"));
	QCOMPARE(spreadsheet.column(2)->name(), QLatin1String("Column 3"));
	QCOMPARE(spreadsheet.column(3)->name(), QLatin1String("Column 4"));
	QCOMPARE(spreadsheet.column(4)->name(), QLatin1String("Column 5"));

	QCOMPARE(spreadsheet.column(0)->valueAt(0), 1);
	QCOMPARE(spreadsheet.column(0)->valueAt(1), 2);
	QCOMPARE(spreadsheet.column(0)->valueAt(2), 3);
	QCOMPARE(spreadsheet.column(1)->valueAt(0), 2);
	QCOMPARE(spreadsheet.column(1)->valueAt(1), 0);
	QCOMPARE(spreadsheet.column(1)->valueAt(2), 0);
	QCOMPARE(spreadsheet.column(2)->valueAt(0), 3);
	QCOMPARE(spreadsheet.column(2)->valueAt(1), 0);
	QCOMPARE(spreadsheet.column(2)->valueAt(2), 0);
	QCOMPARE(spreadsheet.column(3)->valueAt(0), 4);
	QCOMPARE(spreadsheet.column(3)->valueAt(1), 0);
	QCOMPARE(spreadsheet.column(3)->valueAt(2), 0);
	QCOMPARE(spreadsheet.column(4)->valueAt(0), 5);
	QCOMPARE(spreadsheet.column(4)->valueAt(1), 0);
	QCOMPARE(spreadsheet.column(4)->valueAt(2), 0);

	//DEBUG(Q_FUNC_INFO << ", value = " << matrix.column(0)->valueAt(0))
}

// same test but with start/end row/col
void MatioFilterTest::testImportSpreadsheetPortion() {
	Spreadsheet spreadsheet("test", false);
	MatioFilter filter;

	const QString& fileName = QFINDTESTDATA(QLatin1String("data/testmatrix_7.4_GLNX86.mat"));
	filter.setCurrentVarName(QLatin1String("testmatrix"));
	const auto mode = AbstractFileFilter::ImportMode::Replace;
	// set start/end row/col
	filter.setStartRow(2);
	filter.setEndRow(3);
	filter.setStartColumn(2);
	filter.setEndColumn(3);
	filter.readDataFromFile(fileName, &spreadsheet, mode);

	QCOMPARE(spreadsheet.columnCount(), 2);
	QCOMPARE(spreadsheet.rowCount(), 2);
	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Numeric);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Numeric);

	QCOMPARE(spreadsheet.column(0)->plotDesignation(), AbstractColumn::PlotDesignation::X);
	QCOMPARE(spreadsheet.column(1)->plotDesignation(), AbstractColumn::PlotDesignation::Y);

	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("Column 1"));
	QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("Column 2"));

	QCOMPARE(spreadsheet.column(0)->valueAt(0), 0);
	QCOMPARE(spreadsheet.column(0)->valueAt(1), 0);
	QCOMPARE(spreadsheet.column(1)->valueAt(0), 0);
	QCOMPARE(spreadsheet.column(1)->valueAt(1), 0);

	//DEBUG(Q_FUNC_INFO << ", value = " << matrix.column(0)->valueAt(0))
}

void MatioFilterTest::testImportMatrix() {
	Matrix matrix("test", false);
	MatioFilter filter;

	const QString& fileName = QFINDTESTDATA(QLatin1String("data/testmatrix_7.4_GLNX86.mat"));
	filter.setCurrentVarName(QLatin1String("testmatrix"));
	const auto mode = AbstractFileFilter::ImportMode::Replace;
	filter.readDataFromFile(fileName, &matrix, mode);

	QCOMPARE(matrix.columnCount(), 5);
	QCOMPARE(matrix.rowCount(), 3);
	//QCOMPARE(matrix.column(0)->columnMode(), AbstractColumn::ColumnMode::Numeric);
	//QCOMPARE(matrix.column(1)->columnMode(), AbstractColumn::ColumnMode::Numeric);
	//QCOMPARE(matrix.column(2)->columnMode(), AbstractColumn::ColumnMode::Numeric);

	//QCOMPARE(matrix.column(0)->plotDesignation(), AbstractColumn::PlotDesignation::X);
	//QCOMPARE(matrix.column(1)->plotDesignation(), AbstractColumn::PlotDesignation::Y);
	//QCOMPARE(matrix.column(2)->plotDesignation(), AbstractColumn::PlotDesignation::Y);

	QCOMPARE(matrix.cell<double>(0, 0), 1);
	QCOMPARE(matrix.cell<double>(1, 0), 2);
	QCOMPARE(matrix.cell<double>(2, 0), 3);
	QCOMPARE(matrix.cell<double>(0, 1), 2);
	QCOMPARE(matrix.cell<double>(1, 1), 0);
	QCOMPARE(matrix.cell<double>(2, 1), 0);
	QCOMPARE(matrix.cell<double>(0, 2), 3);
	QCOMPARE(matrix.cell<double>(1, 2), 0);
	QCOMPARE(matrix.cell<double>(2, 2), 0);
	QCOMPARE(matrix.cell<double>(0, 3), 4);
	QCOMPARE(matrix.cell<double>(1, 3), 0);
	QCOMPARE(matrix.cell<double>(2, 3), 0);
	QCOMPARE(matrix.cell<double>(0, 4), 5);
	QCOMPARE(matrix.cell<double>(1, 4), 0);
	QCOMPARE(matrix.cell<double>(2, 4), 0);
}

void MatioFilterTest::testImportSparse() {
	Spreadsheet spreadsheet("test", false);
	MatioFilter filter;

	const QString& fileName = QFINDTESTDATA(QLatin1String("data/testsparse_6.5.1_GLNX86.mat"));
	filter.setCurrentVarName(QLatin1String("testsparse"));
	const auto mode = AbstractFileFilter::ImportMode::Replace;
	filter.readDataFromFile(fileName, &spreadsheet, mode);

	QCOMPARE(spreadsheet.columnCount(), 5);
	QCOMPARE(spreadsheet.rowCount(), 3);
	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Numeric);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Numeric);
	QCOMPARE(spreadsheet.column(2)->columnMode(), AbstractColumn::ColumnMode::Numeric);
	QCOMPARE(spreadsheet.column(3)->columnMode(), AbstractColumn::ColumnMode::Numeric);
	QCOMPARE(spreadsheet.column(4)->columnMode(), AbstractColumn::ColumnMode::Numeric);

	QCOMPARE(spreadsheet.column(0)->plotDesignation(), AbstractColumn::PlotDesignation::X);
	QCOMPARE(spreadsheet.column(1)->plotDesignation(), AbstractColumn::PlotDesignation::Y);
	QCOMPARE(spreadsheet.column(2)->plotDesignation(), AbstractColumn::PlotDesignation::Y);
	QCOMPARE(spreadsheet.column(3)->plotDesignation(), AbstractColumn::PlotDesignation::Y);
	QCOMPARE(spreadsheet.column(4)->plotDesignation(), AbstractColumn::PlotDesignation::Y);

	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("Column 1"));
	QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("Column 2"));
	QCOMPARE(spreadsheet.column(2)->name(), QLatin1String("Column 3"));
	QCOMPARE(spreadsheet.column(3)->name(), QLatin1String("Column 4"));
	QCOMPARE(spreadsheet.column(4)->name(), QLatin1String("Column 5"));

	QCOMPARE(spreadsheet.column(0)->valueAt(0), 1);
	QCOMPARE(spreadsheet.column(0)->valueAt(1), 2);
	QCOMPARE(spreadsheet.column(0)->valueAt(2), 3);
	QCOMPARE(spreadsheet.column(1)->valueAt(0), 2);
	QCOMPARE(spreadsheet.column(1)->valueAt(1), 0);
	QCOMPARE(spreadsheet.column(1)->valueAt(2), 0);
	QCOMPARE(spreadsheet.column(2)->valueAt(0), 3);
	QCOMPARE(spreadsheet.column(2)->valueAt(1), 0);
	QCOMPARE(spreadsheet.column(2)->valueAt(2), 0);
	QCOMPARE(spreadsheet.column(3)->valueAt(0), 4);
	QCOMPARE(spreadsheet.column(3)->valueAt(1), 0);
	QCOMPARE(spreadsheet.column(3)->valueAt(2), 0);
	QCOMPARE(spreadsheet.column(4)->valueAt(0), 5);
	QCOMPARE(spreadsheet.column(4)->valueAt(1), 0);
	QCOMPARE(spreadsheet.column(4)->valueAt(2), 0);

	//DEBUG(Q_FUNC_INFO << ", value = " << matrix.column(0)->valueAt(0))
}

void MatioFilterTest::testImportLogicalSparse() {
	Matrix matrix("test", false);
	MatioFilter filter;

	const QString& fileName = QFINDTESTDATA(QLatin1String("data/logical_sparse.mat"));
	filter.setCurrentVarName(QLatin1String("sp_log_5_4"));
	const auto mode = AbstractFileFilter::ImportMode::Replace;
	filter.readDataFromFile(fileName, &matrix, mode);

	QCOMPARE(matrix.columnCount(), 4);
	QCOMPARE(matrix.rowCount(), 5);

	QCOMPARE(matrix.cell<int>(0, 0), 1);
	QCOMPARE(matrix.cell<int>(1, 0), 0);
	QCOMPARE(matrix.cell<int>(2, 0), 0);
	QCOMPARE(matrix.cell<int>(3, 0), 0);
	QCOMPARE(matrix.cell<int>(4, 0), 0);
	QCOMPARE(matrix.cell<int>(0, 1), 1);
	QCOMPARE(matrix.cell<int>(1, 1), 0);
	QCOMPARE(matrix.cell<int>(2, 1), 0);
	QCOMPARE(matrix.cell<int>(3, 1), 0);
	QCOMPARE(matrix.cell<int>(4, 1), 0);
	QCOMPARE(matrix.cell<int>(0, 2), 1);
	QCOMPARE(matrix.cell<int>(1, 2), 1);
	QCOMPARE(matrix.cell<int>(2, 2), 1);
	QCOMPARE(matrix.cell<int>(3, 2), 0);
	QCOMPARE(matrix.cell<int>(4, 2), 0);
	QCOMPARE(matrix.cell<int>(0, 3), 0);
	QCOMPARE(matrix.cell<int>(1, 3), 0);
	QCOMPARE(matrix.cell<int>(2, 3), 0);
	QCOMPARE(matrix.cell<int>(3, 3), 0);
	QCOMPARE(matrix.cell<int>(4, 3), 0);
}

void MatioFilterTest::testImportLogicalSparsePortion() {
	Spreadsheet spreadsheet("test", false);
	MatioFilter filter;

	const QString& fileName = QFINDTESTDATA(QLatin1String("data/logical_sparse.mat"));
	filter.setCurrentVarName(QLatin1String("sp_log_5_4"));
	const auto mode = AbstractFileFilter::ImportMode::Replace;
	// set start/end row/col
	filter.setStartRow(2);
	filter.setEndRow(3);
	filter.setStartColumn(2);
	filter.setEndColumn(3);
	filter.readDataFromFile(fileName, &spreadsheet, mode);

	QCOMPARE(spreadsheet.columnCount(), 2);
	QCOMPARE(spreadsheet.rowCount(), 2);
	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Integer);

	QCOMPARE(spreadsheet.column(0)->plotDesignation(), AbstractColumn::PlotDesignation::X);
	QCOMPARE(spreadsheet.column(1)->plotDesignation(), AbstractColumn::PlotDesignation::Y);

	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("Column 1"));
	QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("Column 2"));

	QCOMPARE(spreadsheet.column(0)->valueAt(0), 0);
	QCOMPARE(spreadsheet.column(0)->valueAt(1), 0);
	QCOMPARE(spreadsheet.column(1)->valueAt(0), 1);
	QCOMPARE(spreadsheet.column(1)->valueAt(1), 1);
}

void MatioFilterTest::testImportSparseComplex() {
	Matrix matrix("test", false);
	MatioFilter filter;

	const QString& fileName = QFINDTESTDATA(QLatin1String("data/testsparsecomplex_7.4_GLNX86.mat"));
	filter.setCurrentVarName(QLatin1String("testsparsecomplex"));
	const auto mode = AbstractFileFilter::ImportMode::Replace;
	filter.readDataFromFile(fileName, &matrix, mode);

	QCOMPARE(matrix.columnCount(), 10);
	QCOMPARE(matrix.rowCount(), 3);

	/*DEBUG(Q_FUNC_INFO << ", value = " << matrix.cell<double>(1, 1)) */
	QCOMPARE(matrix.cell<double>(0, 0), 1.);
	QCOMPARE(matrix.cell<double>(1, 0), 2.);
	QCOMPARE(matrix.cell<double>(2, 0), 3.);
	QCOMPARE(matrix.cell<double>(0, 1), 1.);
	QCOMPARE(matrix.cell<double>(1, 1), 0.);
	QCOMPARE(matrix.cell<double>(2, 1), 0.);
	QCOMPARE(matrix.cell<double>(0, 2), 2.);
	QCOMPARE(matrix.cell<double>(0, 3), 0.);
	QCOMPARE(matrix.cell<double>(0, 4), 3.);
	QCOMPARE(matrix.cell<double>(0, 5), 0.);
	QCOMPARE(matrix.cell<double>(0, 6), 4.);
	QCOMPARE(matrix.cell<double>(0, 7), 0.);
	QCOMPARE(matrix.cell<double>(0, 8), 5.);
	QCOMPARE(matrix.cell<double>(0, 9), 0.);
}

void MatioFilterTest::testImportStruct() {
	Spreadsheet spreadsheet("test", false);
	MatioFilter filter;

	const QString& fileName = QFINDTESTDATA(QLatin1String("data/teststruct_7.4_GLNX86.mat"));
	filter.setCurrentVarName(QLatin1String("teststruct"));
	const auto mode = AbstractFileFilter::ImportMode::Replace;
	filter.readDataFromFile(fileName, &spreadsheet, mode);

	QCOMPARE(spreadsheet.columnCount(), 4);
	QCOMPARE(spreadsheet.rowCount(), 3);
	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Text);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Numeric);
	QCOMPARE(spreadsheet.column(2)->columnMode(), AbstractColumn::ColumnMode::Numeric);
	QCOMPARE(spreadsheet.column(3)->columnMode(), AbstractColumn::ColumnMode::Numeric);

	QCOMPARE(spreadsheet.column(0)->plotDesignation(), AbstractColumn::PlotDesignation::X);
	QCOMPARE(spreadsheet.column(1)->plotDesignation(), AbstractColumn::PlotDesignation::Y);
	QCOMPARE(spreadsheet.column(2)->plotDesignation(), AbstractColumn::PlotDesignation::Y);
	QCOMPARE(spreadsheet.column(3)->plotDesignation(), AbstractColumn::PlotDesignation::Y);

	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("stringfield"));
	QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("doublefield"));
	QCOMPARE(spreadsheet.column(2)->name(), QLatin1String("complexfield - Re"));
	QCOMPARE(spreadsheet.column(3)->name(), QLatin1String("complexfield - Im"));

	QCOMPARE(spreadsheet.column(0)->textAt(0), QLatin1String("Rats live on no evil star."));
	for (int i = 1; i < 4; i++) {
		QCOMPARE(spreadsheet.column(i)->valueAt(0), M_SQRT2);
		QCOMPARE(spreadsheet.column(i)->valueAt(1), M_E);
		QCOMPARE(spreadsheet.column(i)->valueAt(2), M_PI);
	}
}

void MatioFilterTest::testImportStructPortion() {
	Spreadsheet spreadsheet("test", false);
	MatioFilter filter;

	const QString& fileName = QFINDTESTDATA(QLatin1String("data/teststruct_7.4_GLNX86.mat"));
	filter.setCurrentVarName(QLatin1String("teststruct"));
	const auto mode = AbstractFileFilter::ImportMode::Replace;
	// set start/end row/col
	filter.setStartRow(2);
	filter.setEndRow(3);
	filter.setStartColumn(2);
	filter.setEndColumn(3);
	filter.readDataFromFile(fileName, &spreadsheet, mode);

	QCOMPARE(spreadsheet.columnCount(), 3);
	QCOMPARE(spreadsheet.rowCount(), 2);
	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Numeric);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Numeric);
	QCOMPARE(spreadsheet.column(2)->columnMode(), AbstractColumn::ColumnMode::Numeric);

	QCOMPARE(spreadsheet.column(0)->plotDesignation(), AbstractColumn::PlotDesignation::X);
	QCOMPARE(spreadsheet.column(1)->plotDesignation(), AbstractColumn::PlotDesignation::Y);
	QCOMPARE(spreadsheet.column(2)->plotDesignation(), AbstractColumn::PlotDesignation::Y);

	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("doublefield"));
	QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("complexfield - Re"));
	QCOMPARE(spreadsheet.column(2)->name(), QLatin1String("complexfield - Im"));

	for (int i = 0; i < 2; i++) {
		QCOMPARE(spreadsheet.column(i)->valueAt(0), M_E);
		QCOMPARE(spreadsheet.column(i)->valueAt(1), M_PI);
	}
}

void MatioFilterTest::testImportCell() {
	Spreadsheet spreadsheet("test", false);
	MatioFilter filter;

	const QString& fileName = QFINDTESTDATA(QLatin1String("data/testcell_6.1_SOL2.mat"));
	filter.setCurrentVarName(QLatin1String("testcell"));
	const auto mode = AbstractFileFilter::ImportMode::Replace;
	filter.readDataFromFile(fileName, &spreadsheet, mode);

	QCOMPARE(spreadsheet.columnCount(), 4);
	QCOMPARE(spreadsheet.rowCount(), 3);
	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Text);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Numeric);
	QCOMPARE(spreadsheet.column(2)->columnMode(), AbstractColumn::ColumnMode::Numeric);
	QCOMPARE(spreadsheet.column(3)->columnMode(), AbstractColumn::ColumnMode::Numeric);

	QCOMPARE(spreadsheet.column(0)->plotDesignation(), AbstractColumn::PlotDesignation::X);
	QCOMPARE(spreadsheet.column(1)->plotDesignation(), AbstractColumn::PlotDesignation::Y);
	QCOMPARE(spreadsheet.column(2)->plotDesignation(), AbstractColumn::PlotDesignation::Y);
	QCOMPARE(spreadsheet.column(3)->plotDesignation(), AbstractColumn::PlotDesignation::Y);

	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("Column 1"));
	QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("Column 2"));
	QCOMPARE(spreadsheet.column(2)->name(), QLatin1String("Column 3"));
	QCOMPARE(spreadsheet.column(3)->name(), QLatin1String("Column 4"));

	QCOMPARE(spreadsheet.column(0)->textAt(1), QLatin1String());
	QCOMPARE(spreadsheet.column(0)->textAt(2), QLatin1String());
	QCOMPARE(spreadsheet.column(0)->textAt(3), QLatin1String());
	QCOMPARE(spreadsheet.column(1)->valueAt(0), 1.);
	QCOMPARE(spreadsheet.column(1)->valueAt(1), qQNaN());
	QCOMPARE(spreadsheet.column(1)->valueAt(2), qQNaN());
	QCOMPARE(spreadsheet.column(2)->valueAt(0), 1.);
	QCOMPARE(spreadsheet.column(2)->valueAt(1), 2.);
	QCOMPARE(spreadsheet.column(2)->valueAt(2), qQNaN());
	QCOMPARE(spreadsheet.column(3)->valueAt(0), 1.);
	QCOMPARE(spreadsheet.column(3)->valueAt(1), 2.);
	QCOMPARE(spreadsheet.column(3)->valueAt(2), 3.);
}

void MatioFilterTest::testImportEmptyCell() {
	Spreadsheet spreadsheet("test", false);
	MatioFilter filter;

	const QString& fileName = QFINDTESTDATA(QLatin1String("data/testemptycell_7.4_GLNX86.mat"));
	filter.setCurrentVarName(QLatin1String("testemptycell"));
	const auto mode = AbstractFileFilter::ImportMode::Replace;
	filter.readDataFromFile(fileName, &spreadsheet, mode);

	QCOMPARE(spreadsheet.columnCount(), 5);
	QCOMPARE(spreadsheet.rowCount(), 1);
	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Numeric);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Numeric);
	QCOMPARE(spreadsheet.column(2)->columnMode(), AbstractColumn::ColumnMode::Numeric);
	QCOMPARE(spreadsheet.column(3)->columnMode(), AbstractColumn::ColumnMode::Numeric);
	QCOMPARE(spreadsheet.column(4)->columnMode(), AbstractColumn::ColumnMode::Numeric);

	QCOMPARE(spreadsheet.column(0)->plotDesignation(), AbstractColumn::PlotDesignation::X);
	QCOMPARE(spreadsheet.column(1)->plotDesignation(), AbstractColumn::PlotDesignation::Y);
	QCOMPARE(spreadsheet.column(2)->plotDesignation(), AbstractColumn::PlotDesignation::Y);
	QCOMPARE(spreadsheet.column(3)->plotDesignation(), AbstractColumn::PlotDesignation::Y);
	QCOMPARE(spreadsheet.column(4)->plotDesignation(), AbstractColumn::PlotDesignation::Y);

	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("Column 1"));
	QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("Column 2"));
	QCOMPARE(spreadsheet.column(2)->name(), QLatin1String("Column 3"));
	QCOMPARE(spreadsheet.column(3)->name(), QLatin1String("Column 4"));
	QCOMPARE(spreadsheet.column(4)->name(), QLatin1String("Column 5"));

	QCOMPARE(spreadsheet.column(0)->valueAt(0), 1);
	QCOMPARE(spreadsheet.column(1)->valueAt(0), 2);
	QCOMPARE(spreadsheet.column(2)->valueAt(0), qQNaN());
	QCOMPARE(spreadsheet.column(3)->valueAt(0), qQNaN());
	QCOMPARE(spreadsheet.column(4)->valueAt(0), 3);
}

QTEST_MAIN(MatioFilterTest)
