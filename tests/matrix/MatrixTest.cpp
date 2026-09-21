/*
	File                 : MatrixTest.cpp
	Project              : LabPlot
	Description          : Tests for the Matrix
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "MatrixTest.h"
#include "backend/core/Project.h"
#include "backend/datasources/filters/AsciiFilter.h"
#include "backend/matrix/Matrix.h"
#include "backend/matrix/MatrixModel.h"
#include "frontend/dockwidgets/MatrixDock.h"
#include "frontend/matrix/MatrixView.h"

void MatrixTest::testLoadSaveNoData() {
	constexpr auto rowCount = 10;
	constexpr auto columnCount = 5;
	QString savePath;

	{
		Project project;
		project.setSaveData(false);
		auto* m = new Matrix(rowCount, columnCount, QStringLiteral("Test"));
		project.addChild(m);

		QCOMPARE(m->rowCount(), rowCount);
		QCOMPARE(m->columnCount(), columnCount);

		for (int r = 0; r < rowCount; r++) {
			for (int c = 0; c < columnCount; c++) {
				m->setCell(r, c, (double)(r * columnCount + c));
			}
		}
		SAVE_PROJECT("testLoadSaveNoData");
	}

	{
		Project project;
		QVERIFY(project.load(savePath));

		const auto& matrices = project.children<Matrix>();
		QCOMPARE(matrices.size(), 1);
		const auto* matrix = matrices.at(0);
		QCOMPARE(matrix->name(), QStringLiteral("Test"));
		QCOMPARE(matrix->rowCount(), 0);
		QCOMPARE(matrix->columnCount(), 0);
	}
}

void MatrixTest::testLoadSaveWithData() {
	constexpr auto rowCount = 10;
	constexpr auto columnCount = 5;
	QString savePath;

	{
		Project project;
		project.setSaveData(true);
		auto* m = new Matrix(rowCount, columnCount, QStringLiteral("Test"));
		project.addChild(m);

		QCOMPARE(m->rowCount(), rowCount);
		QCOMPARE(m->columnCount(), columnCount);

		for (int r = 0; r < rowCount; r++) {
			for (int c = 0; c < columnCount; c++) {
				m->setCell(r, c, (double)(r * columnCount + c));
			}
		}
		SAVE_PROJECT("testLoadSaveNoData");
	}

	{
		Project project;
		QVERIFY(project.load(savePath));

		const auto& matrices = project.children<Matrix>();
		QCOMPARE(matrices.size(), 1);
		const auto* matrix = matrices.at(0);
		QCOMPARE(matrix->name(), QStringLiteral("Test"));
		QCOMPARE(matrix->rowCount(), rowCount);
		QCOMPARE(matrix->columnCount(), columnCount);

		for (int r = 0; r < rowCount; r++) {
			for (int c = 0; c < columnCount; c++) {
				VALUES_EQUAL(matrix->cell<double>(r, c), r * columnCount + c);
			}
		}
	}
}

//**********************************************************
//********** Check different formulas **********************
//**********************************************************
#define INIT_MATRIX                                                                                                                                            \
	Matrix m(QStringLiteral("test matrix"), false);                                                                                                            \
	QStringList variableNames;                                                                                                                                 \
	variableNames << QStringLiteral("x") << QStringLiteral("y");

/*!
   formula "1"
*/
void MatrixTest::formula1() {
	INIT_MATRIX

	m.setFormula(QStringLiteral("1"));
	// TODO: currently only via MatrixFunctionDialog
	// m.updateFormula();

	const int rows = 10;
	const int cols = 10;

	QCOMPARE(m.columnCount(), cols);
	QCOMPARE(m.rowCount(), rows);

	// values
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			QCOMPARE(m.cell<double>(i, j), 0);
		}
	}
}

//**********************************************************
//********** dock related tests *****************************
//**********************************************************
/*!
 * \brief MatrixTest::testAsciiImportDockRowCountUpdate
 * After importing ascii data into a matrix, the row count determined during the import is applied
 * directly to the underlying data vectors, bypassing insertRows()/removeRows(). Because of that,
 * the MatrixDock, which relies on Matrix::rowCountChanged() to keep its row count spinbox in sync,
 * needs to be notified explicitly in Matrix::finalizeImport(). Otherwise, the dock would still show
 * the row count that was set before the import (the default 10x10 dimensions for a freshly created matrix).
 */
void MatrixTest::testAsciiImportDockRowCountUpdate() {
	Project project;
	auto* matrix = new Matrix(QStringLiteral("test"), false);
	project.addChild(matrix);

	MatrixDock dock(nullptr);
	dock.setMatrices({matrix});

	QCOMPARE(dock.ui.sbRowCount->isEnabled(), true);
	QCOMPARE(dock.ui.sbRowCount->value(), matrix->rowCount());
	QCOMPARE(dock.ui.sbColumnCount->value(), matrix->columnCount());

	// matrix data only supports the Double column mode, so the values need a decimal point
	// to be auto-detected as Double instead of Integer
	QStringList fileContent = {
		QStringLiteral("1.0 2.0 3.0"),
		QStringLiteral("4.0 5.0 6.0"),
		QStringLiteral("7.0 8.0 9.0"),
	};
	QString savePath;
	SAVE_FILE("testfile", fileContent);

	AsciiFilter filter;
	auto p = filter.properties();
	p.automaticSeparatorDetection = true;
	p.separator = QStringLiteral(" ");
	p.headerEnabled = false;
	filter.setProperties(p);

	filter.readDataFromFile(savePath, matrix, AbstractFileFilter::ImportMode::Replace);
	QVERIFY(filter.lastError().isEmpty());

	QCOMPARE(matrix->rowCount(), 3);
	QCOMPARE(matrix->columnCount(), 3);

	QCOMPARE(dock.ui.sbRowCount->value(), 3);
	QCOMPARE(dock.ui.sbColumnCount->value(), 3);
}

/*!
 * \brief MatrixTest::testAsciiImportModelRowCountUpdate
 * The actual data grid shown for a matrix (MatrixView) is backed by MatrixModel, a QAbstractItemModel.
 * During the ascii import, the row count is only known once all the data was read and the underlying
 * data vectors were resized directly, bypassing insertRows()/removeRows(). Matrix::setSuppressDataChangedSignal(),
 * which brackets the whole import, needs to trigger a full model reset in MatrixModel so an attached view
 * (QTableView in MatrixView) is actually notified and re-queries the row and column count after the import.
 * Note: MatrixModel::rowCount()/columnCount() always forward to Matrix::rowCount()/columnCount() directly, so
 * checking their return values alone would not catch a missing view notification; a QSignalSpy on the
 * standard modelReset() signal is used instead to verify that views are actually told to refresh.
 */
void MatrixTest::testAsciiImportModelRowCountUpdate() {
	Project project;
	auto* matrix = new Matrix(QStringLiteral("test"), false);
	project.addChild(matrix);

	// matrix->view() creates the MatrixView together with its MatrixModel and, importantly,
	// hooks the model up to Matrix::m_model, exactly like opening the matrix in the GUI does.
	// A standalone MatrixModel constructed directly would not be reachable from
	// Matrix::setSuppressDataChangedSignal() and would not exercise the fix.
	auto* view = static_cast<MatrixView*>(matrix->view());
	auto* model = view->model();
	QSignalSpy resetSpy(model, &QAbstractItemModel::modelReset);

	// matrix data only supports the Double column mode, so the values need a decimal point
	// to be auto-detected as Double instead of Integer
	QStringList fileContent = {
		QStringLiteral("1.0 2.0 3.0"),
		QStringLiteral("4.0 5.0 6.0"),
		QStringLiteral("7.0 8.0 9.0"),
	};
	QString savePath;
	SAVE_FILE("testfile", fileContent);

	AsciiFilter filter;
	auto p = filter.properties();
	p.automaticSeparatorDetection = true;
	p.separator = QStringLiteral(" ");
	p.headerEnabled = false;
	filter.setProperties(p);

	filter.readDataFromFile(savePath, matrix, AbstractFileFilter::ImportMode::Replace);
	QVERIFY(filter.lastError().isEmpty());

	QCOMPARE(matrix->rowCount(), 3);
	QCOMPARE(matrix->columnCount(), 3);

	// an attached view must have been notified to refresh, otherwise it keeps showing the old dimensions
	QVERIFY(resetSpy.count() > 0);
	QCOMPARE(model->rowCount(), 3);
	QCOMPARE(model->columnCount(), 3);
}

QTEST_MAIN(MatrixTest)
