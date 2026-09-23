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
 * After importing ascii data into a matrix, the row count in the dock must be corrct
 */
void MatrixTest::testAsciiImportDockRowCountUpdate() {
	Project project;
	auto* matrix = new Matrix(QStringLiteral("test"), false);
	project.addChild(matrix);

	MatrixDock dock(nullptr);
	dock.setMatrices({matrix});

	auto* view = static_cast<MatrixView*>(matrix->view());
	auto* model = view->model();

	QCOMPARE(dock.ui.sbRowCount->isEnabled(), true);
	QCOMPARE(dock.ui.sbRowCount->value(), matrix->rowCount());
	QCOMPARE(dock.ui.sbColumnCount->value(), matrix->columnCount());

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
	p.intAsDouble = true; // Matrix supports only double values
	filter.setProperties(p);

	filter.readDataFromFile(savePath, matrix, AbstractFileFilter::ImportMode::Replace);
	QVERIFY(filter.lastError().isEmpty());

	QCOMPARE(matrix->rowCount(), 3);
	QCOMPARE(matrix->columnCount(), 3);

	// Dock update
	QCOMPARE(dock.ui.sbRowCount->value(), 3);
	QCOMPARE(dock.ui.sbColumnCount->value(), 3);

	// Model updated as well
	QCOMPARE(model->rowCount(), 3);
	QCOMPARE(model->columnCount(), 3);
}

QTEST_MAIN(MatrixTest)
