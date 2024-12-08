/*
	File                 : HeatmapTest.cpp
	Project              : LabPlot
	Description          : Tests for Heatmap
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Martin Marmsoler <martin.marmsoler@gmail.com>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "HeatmapTest.h"
#include "src/backend/core/Project.h"
#include "src/backend/core/column/Column.h"
#include "src/backend/matrix/Matrix.h"
#include "src/backend/spreadsheet/Spreadsheet.h"
#include "src/backend/worksheet/Worksheet.h"
#include "src/backend/worksheet/plots/cartesian/Heatmap.h"

#include <QUndoStack>

#define COMPARE_VALUES(xPosStart_, yPosStart_, xPosEnd_, yPosEnd_, value_)                                                                                     \
	do {                                                                                                                                                       \
		QCOMPARE(xPosStart, xPosStart_);                                                                                                                       \
		QCOMPARE(yPosStart, yPosStart_);                                                                                                                       \
		QCOMPARE(xPosEnd, xPosEnd_);                                                                                                                           \
		QCOMPARE(yPosEnd, yPosEnd_);                                                                                                                           \
		QCOMPARE(value, value_);                                                                                                                               \
	} while (false);

#define CONNECT_DATA_CHANGED                                                                                                                                   \
	do {                                                                                                                                                       \
		connect(hm, &Heatmap::dataChanged, [&dataChangedCounter] {                                                                                             \
			dataChangedCounter++;                                                                                                                              \
		});                                                                                                                                                    \
		connect(hm, &Heatmap::xDataChanged, [&dataChangedCounter] {                                                                                            \
			dataChangedCounter++;                                                                                                                              \
		});                                                                                                                                                    \
		connect(hm, &Heatmap::yDataChanged, [&dataChangedCounter] {                                                                                            \
			dataChangedCounter++;                                                                                                                              \
		});                                                                                                                                                    \
	} while (false);

/*!
 * \brief HeatmapTest::testSetMatrix
 * Testing setting a matrix
 * - Testing undo redo
 * - Testing set of column has no effect
 */
void HeatmapTest::testSetMatrix() {
	Project project;

	auto* ws = new Worksheet(QStringLiteral("Worksheet"));
	project.addChild(ws);

	auto* plot = new CartesianPlot(QStringLiteral("Plot"));
	ws->addChild(plot);

	Heatmap* hm = new Heatmap(QStringLiteral("HM"));
	plot->addChild(hm);

	hm->setDataSource(Heatmap::DataSource::Matrix);

	int dataChangedCounter = 0;
	CONNECT_DATA_CHANGED;

	auto* spreadsheet = new Spreadsheet(QStringLiteral("Spreadsheet"));
	auto columns = spreadsheet->children<Column>();
	QCOMPARE(columns.count(), 2);

	auto* xColumn = columns.at(0);
	auto* yColumn = columns.at(1);

	hm->setXColumn(xColumn);
	hm->setYColumn(yColumn);

	QCOMPARE(dataChangedCounter, 0); // Datasource set to matrix
	auto* matrix1 = new Matrix(QStringLiteral("Matrix1"));
	hm->setMatrix(matrix1);
	QCOMPARE(dataChangedCounter, 1);
	QCOMPARE(hm->matrix(), matrix1);

	xColumn->setValueAt(0, 5);
	QCOMPARE(dataChangedCounter, 1); // Datasource set to matrix
	yColumn->setValueAt(0, 3);
	QCOMPARE(dataChangedCounter, 1); // Datasource set to matrix

	auto* matrix2 = new Matrix(QStringLiteral("Matrix2"));
	hm->setMatrix(matrix2);
	QCOMPARE(hm->matrix(), matrix2);
	QCOMPARE(dataChangedCounter, 2);

	hm->undoStack()->undo();
	QCOMPARE(hm->matrix(), matrix1);
	QCOMPARE(dataChangedCounter, 3);

	hm->undoStack()->undo();
	QCOMPARE(hm->matrix(), nullptr);
	QCOMPARE(dataChangedCounter, 4);

	hm->undoStack()->redo();
	QCOMPARE(hm->matrix(), matrix1);
	QCOMPARE(dataChangedCounter, 5);

	matrix1->setCell(0, 0, 1.);
	QCOMPARE(dataChangedCounter, 6);
}

/*!
 * \brief HeatmapTest::testSetSpreadsheetColumn
 * Testing setting a column
 * - Testing undo redo
 * - Testing set of matrix has no effect
 */
void HeatmapTest::testSetSpreadsheetColumn() {
	Project project;

	auto* ws = new Worksheet(QStringLiteral("Worksheet"));
	project.addChild(ws);

	auto* plot = new CartesianPlot(QStringLiteral("Plot"));
	ws->addChild(plot);

	Heatmap* hm = new Heatmap(QStringLiteral("HM"));
	plot->addChild(hm);

	hm->setDataSource(Heatmap::DataSource::Spreadsheet);

	int dataChangedCounter = 0;
	CONNECT_DATA_CHANGED;

	auto* spreadsheet = new Spreadsheet(QStringLiteral("Spreadsheet"));
	project.addChild(spreadsheet);
	auto columns = spreadsheet->children<Column>();
	QCOMPARE(columns.count(), 2);

	auto* xColumn = columns.at(0);
	auto* yColumn = columns.at(1);

	QCOMPARE(dataChangedCounter, 0);
	hm->setXColumn(xColumn);
	QCOMPARE(hm->xColumn(), xColumn);
	QCOMPARE(dataChangedCounter, 1);
	hm->setYColumn(yColumn);
	QCOMPARE(hm->yColumn(), yColumn);
	QCOMPARE(dataChangedCounter, 2);

	auto* matrix1 = new Matrix(QStringLiteral("Matrix1"));
	hm->setMatrix(matrix1);
	QCOMPARE(dataChangedCounter, 2); // Datasource set to column

	xColumn->setValueAt(0, 5.);
	QCOMPARE(dataChangedCounter, 3);
	yColumn->setValueAt(0, 3.);
	QCOMPARE(dataChangedCounter, 4);

	hm->undoStack()->undo(); // yColumn->setValueAt(0, 3.);
	QCOMPARE(hm->matrix(), matrix1);
	QCOMPARE(dataChangedCounter, 5);

	hm->undoStack()->redo(); // xColumn->setValueAt(0, 5.);
	QCOMPARE(hm->matrix(), matrix1);
	QCOMPARE(dataChangedCounter, 6);

	spreadsheet = new Spreadsheet(QStringLiteral("Spreadsheet2"));
	columns = spreadsheet->children<Column>();
	QCOMPARE(columns.count(), 2);

	auto* xColumn2 = columns.at(0);
	auto* yColumn2 = columns.at(1);

	dataChangedCounter = 0; // reset

	hm->setXColumn(xColumn2);
	QCOMPARE(hm->xColumn(), xColumn2);
	QCOMPARE(dataChangedCounter, 1);
	hm->setYColumn(yColumn2);
	QCOMPARE(hm->yColumn(), yColumn2);
	QCOMPARE(dataChangedCounter, 2);

	hm->undoStack()->undo(); // hm->setYColumn(yColumn2);

	QCOMPARE(hm->xColumn(), xColumn2);
	QCOMPARE(hm->yColumn(), yColumn);

	matrix1->setCell(0, 0, 5.);
	QCOMPARE(dataChangedCounter, 3); // Does not have any effect
}

/*!
 * \brief HeatmapTest::testSetDataSource
 * Testing to set the datasource
 * - Undo redo testing
 */
void HeatmapTest::testSetDataSource() {
	Project project;

	auto* ws = new Worksheet(QStringLiteral("Worksheet"));
	project.addChild(ws);

	auto* plot = new CartesianPlot(QStringLiteral("Plot"));
	ws->addChild(plot);

	Heatmap* hm = new Heatmap(QStringLiteral("HM"));
	plot->addChild(hm);

	hm->setDataSource(Heatmap::DataSource::Spreadsheet);

	int dataChangedCounter = 0;
	CONNECT_DATA_CHANGED;

	auto* spreadsheet = new Spreadsheet(QStringLiteral("Spreadsheet"));
	project.addChild(spreadsheet);
	auto columns = spreadsheet->children<Column>();
	QCOMPARE(columns.count(), 2);

	auto* xColumn = columns.at(0);
	auto* yColumn = columns.at(1);

	QCOMPARE(dataChangedCounter, 0);
	hm->setXColumn(xColumn);
	QCOMPARE(hm->xColumn(), xColumn);
	QCOMPARE(dataChangedCounter, 1);
	hm->setYColumn(yColumn);
	QCOMPARE(hm->yColumn(), yColumn);
	QCOMPARE(dataChangedCounter, 2);

	auto* matrix1 = new Matrix(QStringLiteral("Matrix1"));
	project.addChild(matrix1);
	hm->setMatrix(matrix1);
	QCOMPARE(dataChangedCounter, 2);

	xColumn->setValueAt(0, 5.);
	QCOMPARE(dataChangedCounter, 3);
	yColumn->setValueAt(0, 3.);
	QCOMPARE(dataChangedCounter, 4);

	hm->setDataSource(Heatmap::DataSource::Matrix);
	QCOMPARE(hm->dataSource(), Heatmap::DataSource::Matrix);
	QCOMPARE(dataChangedCounter, 5);

	matrix1->setCell(0, 0, 5.);
	QCOMPARE(dataChangedCounter, 6);

	hm->undoStack()->undo(); // matrix1->setCell(0, 0, 5.);
	QCOMPARE(dataChangedCounter, 7);
	hm->undoStack()->undo(); // hm->setDataSource(Heatmap::DataSource::Matrix);
	QCOMPARE(dataChangedCounter, 8);
	QCOMPARE(hm->dataSource(), Heatmap::DataSource::Spreadsheet);

	hm->undoStack()->redo(); // hm->setDataSource(Heatmap::DataSource::Matrix);
	QCOMPARE(dataChangedCounter, 9);
	QCOMPARE(hm->dataSource(), Heatmap::DataSource::Matrix);
}

/*!
 * \brief HeatmapTest::testNumberBins
 * Testing changing number bins
 */
void HeatmapTest::testNumberBins() {
	Project project;

	auto* ws = new Worksheet(QStringLiteral("Worksheet"));
	project.addChild(ws);

	auto* plot = new CartesianPlot(QStringLiteral("Plot"));
	ws->addChild(plot);

	Heatmap* hm = new Heatmap(QStringLiteral("HM"));
	plot->addChild(hm);

	hm->setDataSource(Heatmap::DataSource::Spreadsheet);
	QCOMPARE(hm->drawEmpty(), false);
	hm->setXNumBins(5);
	hm->setYNumBins(5);
	QCOMPARE(hm->xNumBins(), 5);
	QCOMPARE(hm->yNumBins(), 5);

	int dataChangedCounter = 0;
	CONNECT_DATA_CHANGED;

	auto* spreadsheet = new Spreadsheet(QStringLiteral("Spreadsheet"));
	auto columns = spreadsheet->children<Column>();
	QCOMPARE(columns.count(), 2);

	auto* xColumn = columns.at(0);
	auto* yColumn = columns.at(1);

	QCOMPARE(dataChangedCounter, 0);
	hm->setXColumn(xColumn);
	QCOMPARE(hm->xColumn(), xColumn);
	QCOMPARE(dataChangedCounter, 1);
	hm->setYColumn(yColumn);
	QCOMPARE(hm->yColumn(), yColumn);
	QCOMPARE(dataChangedCounter, 2);

	spreadsheet->setRowCount(12);

	{
		auto range = plot->range(Dimension::X, 0);
		range.setAutoScale(false);
		range.setStart(0.);
		range.setEnd(10.);
		plot->setXRange(0, range);
	}

	{
		auto range = plot->range(Dimension::Y, 0);
		range.setAutoScale(false);
		range.setStart(0.);
		range.setEnd(100.);
		plot->setYRange(0, range);
	}

	CHECK_RANGE(plot, hm, Dimension::X, 0., 10.);
	CHECK_RANGE(plot, hm, Dimension::Y, 0., 100.);

	xColumn->setValueAt(0, 0.);
	yColumn->setValueAt(0, 0.);
	xColumn->setValueAt(1, 0.0);
	yColumn->setValueAt(1, 100.0); // Border values are included

	xColumn->setValueAt(2, 2.0);
	yColumn->setValueAt(2, 20.0);
	xColumn->setValueAt(3, 6.0);
	yColumn->setValueAt(3, 20.0);

	xColumn->setValueAt(4, 4.0); // center
	yColumn->setValueAt(4, 40.0); // center

	xColumn->setValueAt(5, 2.0);
	yColumn->setValueAt(5, 60.0);
	xColumn->setValueAt(6, 6.0);
	yColumn->setValueAt(6, 60.0);

	xColumn->setValueAt(7, 10.); // 10 Would be already in the next bin. But border values are included
	yColumn->setValueAt(7, 0.);
	xColumn->setValueAt(8, 10.0);
	yColumn->setValueAt(8, 100.0);

	xColumn->setValueAt(9, 4.5); // Testing Duplicates
	yColumn->setValueAt(9, 45.0); // Testing Duplicates
	xColumn->setValueAt(10, 9.0); // Testing Duplicates
	yColumn->setValueAt(10, 90.0); // Testing Duplicates
	xColumn->setValueAt(11, 6.5); // Testing Duplicates ( The corresponding y set is below the connect)

	int valueDrawnCounter = 0;
	connect(hm, &Heatmap::valueDrawn, [this, &valueDrawnCounter](double xPosStart, double yPosStart, double xPosEnd, double yPosEnd, double value) {
		switch (valueDrawnCounter) {
		case 0:
			COMPARE_VALUES(0.0, 0.0, 2.0, 20.0, 1.0);
			break;
		case 1:
			COMPARE_VALUES(8.0, 0.0, 10.0, 20.0, 1.0);
			break;
		case 2:
			COMPARE_VALUES(2.0, 20.0, 4.0, 40.0, 1.0);
			break;
		case 3:
			COMPARE_VALUES(6.0, 20.0, 8.0, 40.0, 1.0);
			break;
		case 4:
			COMPARE_VALUES(4.0, 40.0, 6.0, 60.0, 2.0);
			break;
		case 5:
			COMPARE_VALUES(2.0, 60.0, 4.0, 80.0, 1.0);
			break;
		case 6:
			COMPARE_VALUES(6.0, 60.0, 8.0, 80.0, 2.0);
			break;
		case 7:
			COMPARE_VALUES(0.0, 80.0, 2.0, 100.0, 1.0);
			break;
		case 8:
			COMPARE_VALUES(8.0, 80.0, 10.0, 100.0, 2.0);
			break;
		default:
			QVERIFY(false);
		}
		valueDrawnCounter++;
	});
	yColumn->setValueAt(11, 65.0); // Testing Duplicates
	disconnect(hm, &Heatmap::valueDrawn, nullptr, nullptr);

	// 5 Bins X
	// 0     2     4     6     8     10
	// |-----|-----|-----|-----|-----|   0
	// |  X  |     |     |     |  X  |   20  5
	// |     |  X  |     |  X  |     |   40  Bins
	// |     |     | XX  |     |     |   60  Y
	// |     |  X  |     | XX  |     |   80
	// |  X  |     |     |     | XX  |   100

	QCOMPARE(valueDrawnCounter, 9);

	QCOMPARE(plot->range(Dimension::X, hm->coordinateSystemIndex()).start(), 0);
	QCOMPARE(plot->range(Dimension::X, hm->coordinateSystemIndex()).end(), 10);
	QCOMPARE(plot->range(Dimension::Y, hm->coordinateSystemIndex()).start(), 0);
	QCOMPARE(plot->range(Dimension::Y, hm->coordinateSystemIndex()).end(), 100);

	// 3 Bins X
	// 0        3.3       6.6        10
	// |---------|---------|---------|   0
	// |   XX    |   XXX   |   X     |   50  2 Bins
	// |   XX    |   XX    |   XX    |   100 Y
	hm->setXNumBins(3);

	valueDrawnCounter = 0;
	connect(hm, &Heatmap::valueDrawn, [this, &valueDrawnCounter](double xPosStart, double yPosStart, double xPosEnd, double yPosEnd, double value) {
		switch (valueDrawnCounter) {
		case 0:
			COMPARE_VALUES(0., 0., 10. / 3., 50, 2.);
			break;
		case 1:
			COMPARE_VALUES(10. / 3., 0., 20. / 3., 50, 3.);
			break;
		case 2:
			COMPARE_VALUES(20. / 3., 0., 10, 50., 1.);
			break;

		case 3:
			COMPARE_VALUES(0., 50.0, 10. / 3., 100., 2.);
			break;
		case 4:
			COMPARE_VALUES(10. / 3., 50.0, 20. / 3., 100., 2.);
			break;
		case 5:
			COMPARE_VALUES(20. / 3., 50., 10., 100., 2.);
			break;
		}
		valueDrawnCounter++;
	});
	hm->setYNumBins(2);
	QCOMPARE(valueDrawnCounter, 6);
}

/*!
 * \brief HeatmapTest::indicesMinMaxMatrix
 */
void HeatmapTest::indicesMinMaxMatrix() {
	{
		Heatmap hm(QStringLiteral("Heatmap"));
		auto* matrix1 = new Matrix(10, 10, QStringLiteral("Matrix1"), AbstractColumn::ColumnMode::Double);
		hm.setMatrix(matrix1);
		matrix1->setXStart(7);
		matrix1->setXEnd(27);

		int start = -1;
		int end = -1;
		QCOMPARE(hm.indicesMinMaxMatrix(Dimension::X, 16, 19, start, end), true);
		QCOMPARE(start, 4);
		QCOMPARE(end, 6);
	}

	{
		Heatmap hm(QStringLiteral("Heatmap"));
		auto* matrix1 = new Matrix(10, 10, QStringLiteral("Matrix1"), AbstractColumn::ColumnMode::Double);
		hm.setMatrix(matrix1);
		matrix1->setXStart(27);
		matrix1->setXEnd(7);

		int start = -1;
		int end = -1;
		QCOMPARE(hm.indicesMinMaxMatrix(Dimension::X, 19, 16, start, end), true);
		QCOMPARE(start, 4);
		QCOMPARE(end, 5);
	}

	{
		Heatmap hm(QStringLiteral("Heatmap"));
		auto* matrix1 = new Matrix(10, 10, QStringLiteral("Matrix1"), AbstractColumn::ColumnMode::Double);
		hm.setMatrix(matrix1);
		matrix1->setXStart(7);
		matrix1->setXEnd(27);

		int start = -1;
		int end = -1;
		QCOMPARE(hm.indicesMinMaxMatrix(Dimension::X, 1, 50, start, end), true);
		QCOMPARE(start, 0);
		QCOMPARE(end, 9);
	}
}

void HeatmapTest::minMaxMatrix() {
}

void HeatmapTest::testFormat() {
	{
		std::vector<QColor> colorVector;
		for (int i = 0; i < 17; i++) {
			colorVector.push_back(QColor(5, 5, i));
		}
		Heatmap::Format format{0, 10, QStringLiteral("Test"), colorVector};

		QCOMPARE(format.index(-1), 0);
		QCOMPARE(format.index(0), 0);
		// QCOMPARE(format.index(5), ...); // TODO:
		// QCOMPARE(format.index(3.1239), ...) // TODO:
		QCOMPARE(format.index(10), colorVector.size() - 1);
		QCOMPARE(format.index(100), colorVector.size() - 1);
	}

	{
		std::vector<QColor> colorVector;
		for (int i = 0; i < 17; i++) {
			colorVector.push_back(QColor(5, 5, i));
		}
		Heatmap::Format format{10., -5., QStringLiteral("Test"), colorVector};

		QCOMPARE(format.index(-6), colorVector.size() - 1);
		QCOMPARE(format.index(-5), colorVector.size() - 1);
		//		QCOMPARE(format.index(5), ...);  TODO:
		//		QCOMPARE(format.index(3.1239), ...)  TODO:
		QCOMPARE(format.index(10), 0);
		QCOMPARE(format.index(100), 0);
	}
}

void HeatmapTest::testRepresentationMatrix() {
	Project project;

	auto* ws = new Worksheet(QStringLiteral("Worksheet"));
	project.addChild(ws);

	auto* plot = new CartesianPlot(QStringLiteral("Plot"));
	ws->addChild(plot);

	Heatmap* hm = new Heatmap(QStringLiteral("HM"));
	plot->addChild(hm);
	hm->setXNumBins(5);
	hm->setYNumBins(5);
	QCOMPARE(hm->xNumBins(), 5);
	QCOMPARE(hm->yNumBins(), 5);

	hm->setDataSource(Heatmap::DataSource::Matrix);

	int dataChangedCounter = 0;
	CONNECT_DATA_CHANGED;

	auto* matrix = new Matrix(5, 5, QStringLiteral("Matrix1"));
	project.addChild(matrix);
	hm->setMatrix(matrix);
	QCOMPARE(dataChangedCounter, 1);
	matrix->setXStart(1);
	QCOMPARE(dataChangedCounter, 2);
	matrix->setXEnd(11);
	QCOMPARE(dataChangedCounter, 3);
	matrix->setYStart(1);
	QCOMPARE(dataChangedCounter, 4);
	matrix->setYEnd(101);
	QCOMPARE(dataChangedCounter, 5);

	matrix->setXStart(0);
	matrix->setXEnd(10);
	matrix->setYStart(0);
	matrix->setYEnd(100);
	dataChangedCounter = 0; // reset

	{
		auto range = plot->range(Dimension::X, 0);
		range.setAutoScale(false);
		range.setStart(0.);
		range.setEnd(10.);
		plot->setXRange(0, range);
	}
	{
		auto range = plot->range(Dimension::Y, 0);
		range.setAutoScale(false);
		range.setStart(0.);
		range.setEnd(100.);
		plot->setYRange(0, range);
	}
	CHECK_RANGE(plot, hm, Dimension::X, 0., 10.);
	CHECK_RANGE(plot, hm, Dimension::Y, 0., 100.);

	QCOMPARE(matrix->rowCount(), 5);
	QCOMPARE(matrix->columnCount(), 5);

	matrix->setCell(0, 0, 1.0);
	QCOMPARE(dataChangedCounter, 1);
	matrix->setCell(0, 1, 2.0);
	QCOMPARE(dataChangedCounter, 2);
	matrix->setCell(0, 2, 3.0);
	matrix->setCell(0, 3, 4.0);
	matrix->setCell(0, 4, 5.0);

	matrix->setCell(1, 0, 6.0);
	matrix->setCell(1, 1, 7.0);
	matrix->setCell(1, 2, 8.0);
	matrix->setCell(1, 3, 9.0);
	matrix->setCell(1, 4, 10.0);

	matrix->setCell(2, 0, 11.0);
	matrix->setCell(2, 1, 12.0);
	matrix->setCell(2, 2, 13.0);
	matrix->setCell(2, 3, 14.0);
	matrix->setCell(2, 4, 15.0);

	matrix->setCell(3, 0, 16.0);
	matrix->setCell(3, 1, 17.0);
	matrix->setCell(3, 2, 18.0);
	matrix->setCell(3, 3, 19.0);
	matrix->setCell(3, 4, 20.0);

	matrix->setCell(4, 0, 21.0);
	matrix->setCell(4, 1, 22.0);
	matrix->setCell(4, 2, 23.0);
	matrix->setCell(4, 3, 24.0);

	int valueDrawnCounter = 0;
	connect(hm, &Heatmap::valueDrawn, [this, &valueDrawnCounter](double xPosStart, double yPosStart, double xPosEnd, double yPosEnd, double value) {
		switch (valueDrawnCounter) {
		case 0:
			COMPARE_VALUES(0.0, 0.0, 2.0, 20.0, 1.0); // (0, 0)
			break;
		case 1:
			COMPARE_VALUES(2.0, 0.0, 4.0, 20.0, 2.0); // (0, 1)
			break;
		case 2:
			COMPARE_VALUES(4.0, 0.0, 6.0, 20.0, 3.0); // (0, 2)
			break;
		case 3:
			COMPARE_VALUES(6.0, 0.0, 8.0, 20.0, 4.0); // (0, 3)
			break;
		case 4:
			COMPARE_VALUES(8.0, 0.0, 10.0, 20.0, 5.0); // (0, 4)
			break;

		case 5:
			COMPARE_VALUES(0.0, 20.0, 2.0, 40.0, 6.0); // (1, 0)
			break;
		case 6:
			COMPARE_VALUES(2.0, 20.0, 4.0, 40.0, 7.0); // (1, 1)
			break;
		case 7:
			COMPARE_VALUES(4.0, 20.0, 6.0, 40.0, 8.0); // (1, 2)
			break;
		case 8:
			COMPARE_VALUES(6.0, 20.0, 8.0, 40.0, 9.0); // (1, 3)
			break;
		case 9:
			COMPARE_VALUES(8.0, 20.0, 10.0, 40.0, 10.0); // (1, 4)
			break;

		case 10:
			COMPARE_VALUES(0.0, 40.0, 2.0, 60.0, 11.0); // (2, 0)
			break;
		case 11:
			COMPARE_VALUES(2.0, 40.0, 4.0, 60.0, 12.0); // (2, 1)
			break;
		case 12:
			COMPARE_VALUES(4.0, 40.0, 6.0, 60.0, 13.0); // (2, 2)
			break;
		case 13:
			COMPARE_VALUES(6.0, 40.0, 8.0, 60.0, 14.0); // (2, 3)
			break;
		case 14:
			COMPARE_VALUES(8.0, 40.0, 10.0, 60.0, 15.0); // (2, 4)
			break;

		case 15:
			COMPARE_VALUES(0.0, 60.0, 2.0, 80.0, 16.0); // (3, 0)
			break;
		case 16:
			COMPARE_VALUES(2.0, 60.0, 4.0, 80.0, 17.0); // (3, 1)
			break;
		case 17:
			COMPARE_VALUES(4.0, 60.0, 6.0, 80.0, 18.0); // (3, 2)
			break;
		case 18:
			COMPARE_VALUES(6.0, 60.0, 8.0, 80.0, 19.0); // (3, 3)
			break;
		case 19:
			COMPARE_VALUES(8.0, 60.0, 10.0, 80.0, 20.0); // (3, 4)
			break;

		case 20:
			COMPARE_VALUES(0.0, 80.0, 2.0, 100.0, 21.0); // (4, 0)
			break;
		case 21:
			COMPARE_VALUES(2.0, 80.0, 4.0, 100.0, 22.0); // (4, 1)
			break;
		case 22:
			COMPARE_VALUES(4.0, 80.0, 6.0, 100.0, 23.0); // (4, 2)
			break;
		case 23:
			COMPARE_VALUES(6.0, 80.0, 8.0, 100.0, 24.0); // (4, 3)
			break;
		case 24:
			COMPARE_VALUES(8.0, 80.0, 10.0, 100.0, 25.0); // (4, 4)
			break;
		}
		valueDrawnCounter++;
	});

	matrix->setCell(4, 4, 25.0);
	QCOMPARE(valueDrawnCounter, 25);
	disconnect(hm, &Heatmap::valueDrawn, nullptr, nullptr);

	QCOMPARE(plot->range(Dimension::X, hm->coordinateSystemIndex()).start(), 0);
	QCOMPARE(plot->range(Dimension::X, hm->coordinateSystemIndex()).end(), 10);
	QCOMPARE(plot->range(Dimension::Y, hm->coordinateSystemIndex()).start(), 0);
	QCOMPARE(plot->range(Dimension::Y, hm->coordinateSystemIndex()).end(), 100);

	{
		// first column is cut away completely, second column is half
		valueDrawnCounter = 0;
		connect(hm, &Heatmap::valueDrawn, [this, &valueDrawnCounter](double xPosStart, double yPosStart, double xPosEnd, double yPosEnd, double value) {
			switch (valueDrawnCounter) {
			case 0:
				COMPARE_VALUES(2.0, 0.0, 4.0, 20.0, 2.0); // (0, 1)
				break;
			case 1:
				COMPARE_VALUES(4.0, 0.0, 6.0, 20.0, 3.0); // (0, 2)
				break;
			case 2:
				COMPARE_VALUES(6.0, 0.0, 8.0, 20.0, 4.0); // (0, 3)
				break;

			case 3:
				COMPARE_VALUES(2.0, 20.0, 4.0, 40.0, 7.0); // (1, 1)
				break;
			case 4:
				COMPARE_VALUES(4.0, 20.0, 6.0, 40.0, 8.0); // (1, 2)
				break;
			case 5:
				COMPARE_VALUES(6.0, 20.0, 8.0, 40.0, 9.0); // (1, 3)
				break;

			case 6:
				COMPARE_VALUES(2.0, 40.0, 4.0, 60.0, 12.0); // (2, 1)
				break;
			case 7:
				COMPARE_VALUES(4.0, 40.0, 6.0, 60.0, 13.0); // (2, 2)
				break;
			case 8:
				COMPARE_VALUES(6.0, 40.0, 8.0, 60.0, 14.0); // (2, 3)
				break;

			case 9:
				COMPARE_VALUES(2.0, 60.0, 4.0, 80.0, 17.0); // (3, 1)
				break;
			case 10:
				COMPARE_VALUES(4.0, 60.0, 6.0, 80.0, 18.0); // (3, 2)
				break;
			case 11:
				COMPARE_VALUES(6.0, 60.0, 8.0, 80.0, 19.0); // (3, 3)
				break;

			case 12:
				COMPARE_VALUES(2.0, 80.0, 4.0, 100.0, 22.0); // (4, 1)
				break;
			case 13:
				COMPARE_VALUES(4.0, 80.0, 6.0, 100.0, 23.0); // (4, 2)
				break;
			case 14:
				COMPARE_VALUES(6.0, 80.0, 8.0, 100.0, 24.0); // (4, 3)
				break;
			}
			valueDrawnCounter++;
		});

		auto range = plot->range(Dimension::X, 0);
		range.setAutoScale(false);
		range.setStart(3.);
		range.setEnd(7.);
		plot->setXRange(0, range);

		QCOMPARE(valueDrawnCounter, 15);
		disconnect(hm, &Heatmap::valueDrawn, nullptr, nullptr);
	}

	// first row/column cut away completely, second row/column is half
	valueDrawnCounter = 0;
	connect(hm, &Heatmap::valueDrawn, [this, &valueDrawnCounter](double xPosStart, double yPosStart, double xPosEnd, double yPosEnd, double value) {
		switch (valueDrawnCounter) {
		case 0:
			COMPARE_VALUES(2.0, 20.0, 4.0, 40.0, 7.0); // (1, 1)
			break;
		case 1:
			COMPARE_VALUES(4.0, 20.0, 6.0, 40.0, 8.0); // (1, 2)
			break;
		case 2:
			COMPARE_VALUES(6.0, 20.0, 8.0, 40.0, 9.0); // (1, 3)
			break;

		case 3:
			COMPARE_VALUES(2.0, 40.0, 4.0, 60.0, 12.0); // (2, 1)
			break;
		case 4:
			COMPARE_VALUES(4.0, 40.0, 6.0, 60.0, 13.0); // (2, 2)
			break;
		case 5:
			COMPARE_VALUES(6.0, 40.0, 8.0, 60.0, 14.0); // (2, 3)
			break;

		case 6:
			COMPARE_VALUES(2.0, 60.0, 4.0, 80.0, 17.0); // (3, 1)
			break;
		case 7:
			COMPARE_VALUES(4.0, 60.0, 6.0, 80.0, 18.0); // (3, 2)
			break;
		case 8:
			COMPARE_VALUES(6.0, 60.0, 8.0, 80.0, 19.0); // (3, 3)
			break;
		}
		valueDrawnCounter++;
	});
	{
		auto range = plot->range(Dimension::Y, 0);
		range.setAutoScale(false);
		range.setStart(30.);
		range.setEnd(70.);
		plot->setYRange(0, range);
	}
	QCOMPARE(valueDrawnCounter, 9);
}

void HeatmapTest::testRepresentationSpreadsheet() {
	Project project;

	auto* ws = new Worksheet(QStringLiteral("Worksheet"));
	project.addChild(ws);

	auto* plot = new CartesianPlot(QStringLiteral("Plot"));
	ws->addChild(plot);

	Heatmap* hm = new Heatmap(QStringLiteral("HM"));
	plot->addChild(hm);

	hm->setDataSource(Heatmap::DataSource::Spreadsheet);
	QCOMPARE(hm->drawEmpty(), false);
	hm->setXNumBins(5);
	hm->setYNumBins(5);
	QCOMPARE(hm->xNumBins(), 5);
	QCOMPARE(hm->yNumBins(), 5);

	int dataChangedCounter = 0;
	CONNECT_DATA_CHANGED;

	auto* spreadsheet = new Spreadsheet(QStringLiteral("Spreadsheet"));
	auto columns = spreadsheet->children<Column>();
	QCOMPARE(columns.count(), 2);

	auto* xColumn = columns.at(0);
	auto* yColumn = columns.at(1);

	QCOMPARE(dataChangedCounter, 0);
	hm->setXColumn(xColumn);
	QCOMPARE(hm->xColumn(), xColumn);
	QCOMPARE(dataChangedCounter, 1);
	hm->setYColumn(yColumn);
	QCOMPARE(hm->yColumn(), yColumn);
	QCOMPARE(dataChangedCounter, 2);

	spreadsheet->setRowCount(7);

	xColumn->setValueAt(0, 0.);
	yColumn->setValueAt(0, 0.);
	xColumn->setValueAt(1, 10.);
	yColumn->setValueAt(1, 0.);
	xColumn->setValueAt(2, 0.0);
	yColumn->setValueAt(2, 100.0);
	xColumn->setValueAt(3, 2.0);
	yColumn->setValueAt(3, 20.0);
	xColumn->setValueAt(4, 6.0);
	yColumn->setValueAt(4, 20.0);
	xColumn->setValueAt(5, 2.0);
	yColumn->setValueAt(5, 60.0);
	xColumn->setValueAt(6, 6.0);
	yColumn->setValueAt(6, 60.0);
	xColumn->setValueAt(7, 4.0); // center
	yColumn->setValueAt(7, 40.0); // center
	xColumn->setValueAt(8, 10.0);
	yColumn->setValueAt(8, 100.0);
	xColumn->setValueAt(9, 4.5); // Testing Duplicates
	yColumn->setValueAt(9, 45.0); // Testing Duplicates
	xColumn->setValueAt(10, 9.0); // Testing Duplicates
	yColumn->setValueAt(10, 90.0); // Testing Duplicates
	xColumn->setValueAt(11, 6.5); // Testing Duplicates

	// |-----|-----|-----|-----|-----|
	// |  X  |     |     |     |  X  |
	// |     |  X  |     |  X  |     |
	// |     |     | XX  |     |     |
	// |     |  X  |     | XX  |     |
	// |  X  |     |     |     | XX  |

	int valueDrawnCounter = 0;
	connect(hm, &Heatmap::valueDrawn, [this, &valueDrawnCounter](double xPosStart, double yPosStart, double xPosEnd, double yPosEnd, double value) {
		switch (valueDrawnCounter) {
		// Row 0
		case 0:
			COMPARE_VALUES(0.0, 0.0, 2.0, 20.0, 1.0);
			break;
		case 1:
			COMPARE_VALUES(8.0, 0.0, 10.0, 20.0, 1.0);
			break;

		// Row 1
		case 2:
			COMPARE_VALUES(2.0, 20.0, 4.0, 40.0, 1.0);
			break;
		case 3:
			COMPARE_VALUES(6.0, 20.0, 8.0, 40.0, 1.0);
			break;

		// Row 2
		case 4:
			COMPARE_VALUES(4.0, 40.0, 6.0, 60.0, 2.0);
			break;

		// Row 3
		case 5:
			COMPARE_VALUES(2.0, 60.0, 4.0, 80.0, 1.0);
			break;
		case 6:
			COMPARE_VALUES(6.0, 60.0, 8.0, 80.0, 2.0);
			break;

		// Row 4
		case 7:
			COMPARE_VALUES(0.0, 80.0, 2.0, 100.0, 1.0);
			break;
		case 8:
			COMPARE_VALUES(8.0, 80.0, 10.0, 100.0, 2.0);
			break;
		}
		valueDrawnCounter++;
	});
	yColumn->setValueAt(11, 65.0); // Testing Duplicates
	QCOMPARE(valueDrawnCounter, 9);
	disconnect(hm, &Heatmap::valueDrawn, nullptr, nullptr);

	QCOMPARE(plot->range(Dimension::X, hm->coordinateSystemIndex()).start(), 0.);
	QCOMPARE(plot->range(Dimension::X, hm->coordinateSystemIndex()).end(), 10.);
	QCOMPARE(plot->range(Dimension::Y, hm->coordinateSystemIndex()).start(), 0.);
	QCOMPARE(plot->range(Dimension::Y, hm->coordinateSystemIndex()).end(), 100.);

	{
		auto range = plot->range(Dimension::X, 0);
		range.setAutoScale(false);
		range.setStart(3.);
		range.setEnd(7.);
		plot->setXRange(0, range);
	}

	// first are cut away completely, second row/column is half
	valueDrawnCounter = 0;
	connect(hm, &Heatmap::valueDrawn, [this, &valueDrawnCounter](double xPosStart, double yPosStart, double xPosEnd, double yPosEnd, double value) {
		switch (valueDrawnCounter) {
		case 0:
			COMPARE_VALUES(2.0, 20.0, 4.0, 40.0, 1.0);
			break;
		case 1:
			COMPARE_VALUES(6.0, 20.0, 8.0, 40.0, 1.0);
			break;
		case 2:
			COMPARE_VALUES(4.0, 40.0, 6.0, 60.0, 2.0);
			break;
		case 3:
			COMPARE_VALUES(2.0, 60.0, 4.0, 80.0, 1.0);
			break;
		case 4:
			COMPARE_VALUES(6.0, 60.0, 8.0, 80.0, 2.0);
			break;
		}
		valueDrawnCounter++;
	});
	{
		auto range = plot->range(Dimension::Y, 0);
		range.setAutoScale(false);
		range.setStart(30.);
		range.setEnd(70.);
		plot->setYRange(0, range);
	}
	QCOMPARE(valueDrawnCounter, 5);
}

/*!
 * Testing with drawEmpty() is true
 */
void HeatmapTest::testRepresentationSpreadsheetDrawEmpty() {
	Project project;

	auto* ws = new Worksheet(QStringLiteral("Worksheet"));
	project.addChild(ws);

	auto* plot = new CartesianPlot(QStringLiteral("Plot"));
	ws->addChild(plot);

	Heatmap* hm = new Heatmap(QStringLiteral("HM"));
	plot->addChild(hm);

	hm->setDataSource(Heatmap::DataSource::Spreadsheet);
	hm->setDrawEmpty(true);
	QCOMPARE(hm->drawEmpty(), true); // difference to testRepresentationSpreadsheet()
	hm->setXNumBins(5);
	hm->setYNumBins(5);
	QCOMPARE(hm->xNumBins(), 5);
	QCOMPARE(hm->yNumBins(), 5);

	int dataChangedCounter = 0;
	CONNECT_DATA_CHANGED;

	auto* spreadsheet = new Spreadsheet(QStringLiteral("Spreadsheet"));
	auto columns = spreadsheet->children<Column>();
	QCOMPARE(columns.count(), 2);

	auto* xColumn = columns.at(0);
	auto* yColumn = columns.at(1);

	QCOMPARE(dataChangedCounter, 0);
	hm->setXColumn(xColumn);
	QCOMPARE(hm->xColumn(), xColumn);
	QCOMPARE(dataChangedCounter, 1);
	hm->setYColumn(yColumn);
	QCOMPARE(hm->yColumn(), yColumn);
	QCOMPARE(dataChangedCounter, 2);

	spreadsheet->setRowCount(7);

	xColumn->setValueAt(0, 0.);
	yColumn->setValueAt(0, 0.);
	xColumn->setValueAt(1, 10.);
	yColumn->setValueAt(1, 0.);
	xColumn->setValueAt(2, 0.0);
	yColumn->setValueAt(2, 100.0);
	xColumn->setValueAt(3, 2.0);
	yColumn->setValueAt(3, 20.0);
	xColumn->setValueAt(4, 6.0);
	yColumn->setValueAt(4, 20.0);
	xColumn->setValueAt(5, 2.0);
	yColumn->setValueAt(5, 60.0);
	xColumn->setValueAt(6, 6.0);
	yColumn->setValueAt(6, 60.0);
	xColumn->setValueAt(7, 4.0); // center
	yColumn->setValueAt(7, 40.0); // center
	xColumn->setValueAt(8, 10.0);
	yColumn->setValueAt(8, 100.0);
	xColumn->setValueAt(9, 4.5); // Testing Duplicates
	yColumn->setValueAt(9, 45.0); // Testing Duplicates
	xColumn->setValueAt(10, 9.0); // Testing Duplicates
	yColumn->setValueAt(10, 90.0); // Testing Duplicates
	xColumn->setValueAt(11, 6.5); // Testing Duplicates

	// |-----|-----|-----|-----|-----|
	// |  X  |     |     |     |  X  |
	// |     |  X  |     |  X  |     |
	// |     |     | XX  |     |     |
	// |     |  X  |     | XX  |     |
	// |  X  |     |     |     | XX  |

	int valueDrawnCounter = 0;
	connect(hm, &Heatmap::valueDrawn, [this, &valueDrawnCounter](double xPosStart, double yPosStart, double xPosEnd, double yPosEnd, double value) {
		switch (valueDrawnCounter) {
		// Row 0
		case 0:
			COMPARE_VALUES(0.0, 0.0, 2.0, 20.0, 1.0);
			break;
		case 1:
			COMPARE_VALUES(2.0, 0.0, 4.0, 20.0, 0.0);
			break;
		case 2:
			COMPARE_VALUES(4.0, 0.0, 6.0, 20.0, 0.0);
			break;
		case 3:
			COMPARE_VALUES(6.0, 0.0, 8.0, 20.0, 0.0);
			break;
		case 4:
			COMPARE_VALUES(8.0, 0.0, 10.0, 20.0, 1.0);
			break;

		// Row 1
		case 5:
			COMPARE_VALUES(0.0, 20.0, 2.0, 40.0, 0.0);
			break;
		case 6:
			COMPARE_VALUES(2.0, 20.0, 4.0, 40.0, 1.0);
			break;
		case 7:
			COMPARE_VALUES(4.0, 20.0, 6.0, 40.0, 0.0);
			break;
		case 8:
			COMPARE_VALUES(6.0, 20.0, 8.0, 40.0, 1.0);
			break;
		case 9:
			COMPARE_VALUES(8.0, 20.0, 10.0, 40.0, 0.0);
			break;

		// Row 2
		case 10:
			COMPARE_VALUES(0.0, 40.0, 2.0, 60.0, 0.0);
			break;
		case 11:
			COMPARE_VALUES(2.0, 40.0, 4.0, 60.0, 0.0);
			break;
		case 12:
			COMPARE_VALUES(4.0, 40.0, 6.0, 60.0, 2.0);
			break;
		case 13:
			COMPARE_VALUES(6.0, 40.0, 8.0, 60.0, 0.0);
			break;
		case 14:
			COMPARE_VALUES(8.0, 40.0, 10.0, 60.0, 0.0);
			break;

		// Row 3
		case 15:
			COMPARE_VALUES(0.0, 60.0, 2.0, 80.0, 0.0);
			break;
		case 16:
			COMPARE_VALUES(2.0, 60.0, 4.0, 80.0, 1.0);
			break;
		case 17:
			COMPARE_VALUES(4.0, 60.0, 6.0, 80.0, 0.0);
			break;
		case 18:
			COMPARE_VALUES(6.0, 60.0, 8.0, 80.0, 2.0);
			break;
		case 19:
			COMPARE_VALUES(8.0, 60.0, 10.0, 80.0, 0.0);
			break;

		// Row 4
		case 20:
			COMPARE_VALUES(0.0, 80.0, 2.0, 100.0, 1.0);
			break;
		case 21:
			COMPARE_VALUES(2.0, 80.0, 4.0, 100.0, 0.0);
			break;
		case 22:
			COMPARE_VALUES(4.0, 80.0, 6.0, 100.0, 0.0);
			break;
		case 23:
			COMPARE_VALUES(6.0, 80.0, 8.0, 100.0, 0.0);
			break;
		case 24:
			COMPARE_VALUES(8.0, 80.0, 10.0, 100.0, 2.0);
			break;
		}
		valueDrawnCounter++;
	});
	yColumn->setValueAt(11, 65.0); // Testing Duplicates
	QCOMPARE(valueDrawnCounter, 25);
	disconnect(hm, &Heatmap::valueDrawn, nullptr, nullptr);

	QCOMPARE(plot->range(Dimension::X, hm->coordinateSystemIndex()).start(), 0);
	QCOMPARE(plot->range(Dimension::X, hm->coordinateSystemIndex()).end(), 10);
	QCOMPARE(plot->range(Dimension::Y, hm->coordinateSystemIndex()).start(), 0);
	QCOMPARE(plot->range(Dimension::Y, hm->coordinateSystemIndex()).end(), 100);

	{
		auto range = plot->range(Dimension::X, 0);
		range.setAutoScale(false);
		range.setStart(3.);
		range.setEnd(7.);
		plot->setXRange(0, range);
	}

	// first are cut away completely, second row/column is half
	valueDrawnCounter = 0;
	connect(hm, &Heatmap::valueDrawn, [this, &valueDrawnCounter](double xPosStart, double yPosStart, double xPosEnd, double yPosEnd, double value) {
		switch (valueDrawnCounter) {
		case 0:
			COMPARE_VALUES(2.0, 20.0, 4.0, 40.0, 1.0);
			break;
		case 1:
			COMPARE_VALUES(4.0, 20.0, 6.0, 40.0, 0.0);
			break;
		case 2:
			COMPARE_VALUES(6.0, 20.0, 8.0, 40.0, 1.0);
			break;

		case 3:
			COMPARE_VALUES(2.0, 40.0, 4.0, 60.0, 0.0);
			break;
		case 4:
			COMPARE_VALUES(4.0, 40.0, 6.0, 60.0, 2.0);
			break;
		case 5:
			COMPARE_VALUES(6.0, 40.0, 8.0, 60.0, 0.0);
			break;

		case 6:
			COMPARE_VALUES(2.0, 60.0, 4.0, 80.0, 1.0);
			break;
		case 7:
			COMPARE_VALUES(4.0, 60.0, 6.0, 80.0, 0.0);
			break;
		case 8:
			COMPARE_VALUES(6.0, 60.0, 8.0, 80.0, 2.0);
			break;
		}
		valueDrawnCounter++;
	});
	{
		auto range = plot->range(Dimension::Y, 0);
		range.setAutoScale(false);
		range.setStart(30.);
		range.setEnd(70.);
		plot->setYRange(0, range);
	}
	QCOMPARE(valueDrawnCounter, 9);
}

void HeatmapTest::testRepresentationSpreadsheetDrawZeroes() {
}

/*!
 * \brief HeatmapTest::testColorAutomatic
 * Automatic limits set to true
 */
void HeatmapTest::testColorAutomatic() {
	Project project;

	auto* ws = new Worksheet(QStringLiteral("Worksheet"));
	project.addChild(ws);

	auto* plot = new CartesianPlot(QStringLiteral("Plot"));
	ws->addChild(plot);

	Heatmap* hm = new Heatmap(QStringLiteral("HM"));
	plot->addChild(hm);

	hm->setDataSource(Heatmap::DataSource::Spreadsheet);
	QCOMPARE(hm->drawEmpty(), false);
	QCOMPARE(hm->xNumBins(), 5);
	QCOMPARE(hm->yNumBins(), 5);
	QCOMPARE(hm->automaticLimits(), true);
	// TODO
	//	hm->setFormatMin();
	//	hm->setFormatMax();

	auto* spreadsheet = new Spreadsheet(QStringLiteral("Spreadsheet"));
	auto columns = spreadsheet->children<Column>();
	QCOMPARE(columns.count(), 2);

	auto* xColumn = columns.at(0);
	auto* yColumn = columns.at(1);

	hm->setXColumn(xColumn);
	hm->setYColumn(yColumn);

	spreadsheet->setRowCount(12);

	xColumn->setValueAt(0, 0);
	yColumn->setValueAt(0, 0);
	xColumn->setValueAt(1, 0.0);
	yColumn->setValueAt(1, 100.0);

	xColumn->setValueAt(2, 2.0);
	yColumn->setValueAt(2, 20.0);
	xColumn->setValueAt(3, 6.0);
	yColumn->setValueAt(3, 20.0);

	xColumn->setValueAt(4, 4.0); // center
	yColumn->setValueAt(4, 40.0); // center

	xColumn->setValueAt(5, 2.0);
	yColumn->setValueAt(5, 60.0);
	xColumn->setValueAt(6, 6.0);
	yColumn->setValueAt(6, 60.0);

	xColumn->setValueAt(7, 10);
	yColumn->setValueAt(7, 0);
	xColumn->setValueAt(8, 10.0);
	yColumn->setValueAt(8, 100.0);

	xColumn->setValueAt(9, 4.5); // Testing Duplicates
	yColumn->setValueAt(9, 45.0); // Testing Duplicates
	xColumn->setValueAt(10, 9.0); // Testing Duplicates
	yColumn->setValueAt(10, 90.0); // Testing Duplicates
	xColumn->setValueAt(11, 6.5); // Testing Duplicates
	yColumn->setValueAt(11, 65.0); // Testing Duplicates

	QCOMPARE(hm->format().start, 1);
	QCOMPARE(hm->format().end, 2);

	// 5 Bins X
	// 0     2     4     6     8     10
	// |-----|-----|-----|-----|-----|   0
	// |  X  |     |     |     |  X  |   20  5
	// |     |  X  |     |  X  |     |   40  Bins
	// |     |     | XX  |     |     |   60  Y
	// |     |  X  |     | XX  |     |   80
	// |  X  |     |     |     | XX  |   100

	// 3 Bins X
	// 0        3.3       6.6        10
	// |---------|---------|---------|   0
	// |   XX    |   XXX   |   XX    |   50  2 Bins
	// |   X     |   XX    |   XX    |   100 Y
	hm->setXNumBins(3);
	hm->setYNumBins(2);
	QCOMPARE(hm->format().start, 1.);
	QCOMPARE(hm->format().end, 3.);

	hm->setAutomaticLimits(false);

	QCOMPARE(hm->format().start, 1.);
	QCOMPARE(hm->format().end, 3.);
}

void HeatmapTest::testColorManual() {
	Project project;

	auto* ws = new Worksheet(QStringLiteral("Worksheet"));
	project.addChild(ws);

	auto* plot = new CartesianPlot(QStringLiteral("Plot"));
	ws->addChild(plot);

	Heatmap* hm = new Heatmap(QStringLiteral("HM"));
	plot->addChild(hm);

	hm->setDataSource(Heatmap::DataSource::Spreadsheet);
	QCOMPARE(hm->drawEmpty(), false);
	QCOMPARE(hm->xNumBins(), 5);
	QCOMPARE(hm->yNumBins(), 5);
	hm->setAutomaticLimits(false);
	QCOMPARE(hm->automaticLimits(), false);
	hm->setFormatMin(-10);
	hm->setFormatMax(10);

	auto* spreadsheet = new Spreadsheet(QStringLiteral("Spreadsheet"));
	auto columns = spreadsheet->children<Column>();
	QCOMPARE(columns.count(), 2);

	auto* xColumn = columns.at(0);
	auto* yColumn = columns.at(1);

	int dataChangedCounter = 0;
	CONNECT_DATA_CHANGED;

	QCOMPARE(dataChangedCounter, 0);
	hm->setXColumn(xColumn);
	QCOMPARE(hm->xColumn(), xColumn);
	QCOMPARE(dataChangedCounter, 1);
	hm->setYColumn(yColumn);
	QCOMPARE(hm->yColumn(), yColumn);
	QCOMPARE(dataChangedCounter, 2);

	spreadsheet->setRowCount(12);

	xColumn->setValueAt(0, 0);
	yColumn->setValueAt(0, 0);
	xColumn->setValueAt(1, 0.0);
	yColumn->setValueAt(1, 100.0);

	xColumn->setValueAt(2, 2.0);
	yColumn->setValueAt(2, 20.0);
	xColumn->setValueAt(3, 6.0);
	yColumn->setValueAt(3, 20.0);

	xColumn->setValueAt(4, 4.0); // center
	yColumn->setValueAt(4, 40.0); // center

	xColumn->setValueAt(5, 2.0);
	yColumn->setValueAt(5, 60.0);
	xColumn->setValueAt(6, 6.0);
	yColumn->setValueAt(6, 60.0);

	xColumn->setValueAt(7, 10);
	yColumn->setValueAt(7, 0);
	xColumn->setValueAt(8, 10.0);
	yColumn->setValueAt(8, 100.0);

	xColumn->setValueAt(9, 4.5); // Testing Duplicates
	yColumn->setValueAt(9, 45.0); // Testing Duplicates
	xColumn->setValueAt(10, 9.0); // Testing Duplicates
	yColumn->setValueAt(10, 90.0); // Testing Duplicates
	xColumn->setValueAt(11, 6.5); // Testing Duplicates
	yColumn->setValueAt(11, 65.0); // Testing Duplicates

	QCOMPARE(hm->format().start, -10);
	QCOMPARE(hm->format().end, 10);

	// 5 Bins X
	// 0     2     4     6     8     10
	// |-----|-----|-----|-----|-----|   0
	// |  X  |     |     |     |  X  |   20  5
	// |     |  X  |     |  X  |     |   40  Bins
	// |     |     | XX  |     |     |   60  Y
	// |     |  X  |     | XX  |     |   80
	// |  X  |     |     |     | XX  |   100

	// 3 Bins X
	// 0        3.3       6.6        10
	// |---------|---------|---------|   0
	// |   XX    |   XXX   |   XX    |   50  2 Bins
	// |   X     |   XX    |   XX    |   100 Y

	// QCOMPARE(valueDrawnCounter, 9); // TODO:

	QCOMPARE(plot->range(Dimension::X, hm->coordinateSystemIndex()).start(), 0);
	QCOMPARE(plot->range(Dimension::X, hm->coordinateSystemIndex()).end(), 10);
	QCOMPARE(plot->range(Dimension::Y, hm->coordinateSystemIndex()).start(), 0);
	QCOMPARE(plot->range(Dimension::Y, hm->coordinateSystemIndex()).end(), 100);

	hm->setXNumBins(3);

	int valueDrawnCounter = 0;
	connect(hm, &Heatmap::valueDrawn, [this, &valueDrawnCounter](double xPosStart, double yPosStart, double xPosEnd, double yPosEnd, double value) {
		switch (valueDrawnCounter) {
		case 0:
			COMPARE_VALUES(0., 0., 3.3, 50, 2.0);
			break;
		case 1:
			COMPARE_VALUES(3.3, 0., 6.6, 50, 3.0);
			break;
		case 2:
			COMPARE_VALUES(6.6, 0., 10, 0., 2.0);
			break;

		case 3:
			COMPARE_VALUES(0., 50.0, 3.3, 100, 1.0);
			break;
		case 4:
			COMPARE_VALUES(3.3, 50.0, 6.6, 100, 2.0);
			break;
		case 5:
			COMPARE_VALUES(6.6, 50.0, 10, 100, 2.0);
			break;
		}
		valueDrawnCounter++;
	});
	hm->setYNumBins(2);
	QCOMPARE(valueDrawnCounter, 6);
	QCOMPARE(hm->format().start, -10);
	QCOMPARE(hm->format().end, 10);
}

void HeatmapTest::saveLoad() {
	QVERIFY(false);
}

void HeatmapTest::testMatrixNumBins() {
	QVERIFY(false);
	// Test if the matrixNumBins works as expected
}

QTEST_MAIN(HeatmapTest)
