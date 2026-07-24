/*
	File                 : WorksheetTest.cpp
	Project              : LabPlot
	Description          : Tests for Worksheets
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Martin Marmsoler <martin.marmsoler@gmail.com>

 SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "WorksheetTest.h"

#include "backend/core/Project.h"
#include "backend/worksheet/Line.h"
#include "backend/worksheet/TreeModel.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/WorksheetPrivate.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/worksheet/plots/cartesian/XYCurve.h"
#include "frontend/GuiTools.h"

#include <QGraphicsItem>
#include <QPen>

void WorksheetTest::cursorCurveColor() {
	Project project;

	auto* worksheet = new Worksheet(QStringLiteral("Worksheet"));
	project.addChild(worksheet);

	auto* plot = new CartesianPlot(QStringLiteral("plot123"));
	worksheet->addChild(plot);

	auto* curve1 = new XYCurve(QStringLiteral("Curve1"));
	plot->addChild(curve1);
	auto* curve2 = new XYCurve(QStringLiteral("Curve2"));
	plot->addChild(curve2);

	curve2->line()->color();
	QVERIFY(curve1->line()->pen().color() != curve2->line()->pen().color());

	const auto* treemodel = worksheet->cursorModel();

	// Row0: X Value
	// Row1: Plot
	QCOMPARE(treemodel->rowCount(), 2);

	QCOMPARE(treemodel->data(treemodel->index(0, (int)WorksheetPrivate::TreeModelColumn::PLOTNAME), Qt::DisplayRole).toString(), QStringLiteral("X"));
	const auto& plotIndex = treemodel->index(1, (int)WorksheetPrivate::TreeModelColumn::PLOTNAME);

	// Row2: Curve1 (Plot as parent index)
	// Row3: Curve2 (Plot as parent index)
	QCOMPARE(treemodel->rowCount(plotIndex), 2);
	QCOMPARE(treemodel->data(plotIndex, Qt::DisplayRole).toString(), QStringLiteral("plot123"));
	QCOMPARE(treemodel->data(treemodel->index(0, (int)WorksheetPrivate::TreeModelColumn::SIGNALNAME, plotIndex), Qt::DisplayRole).toString(), curve1->name());
	QCOMPARE(treemodel->data(treemodel->index(1, (int)WorksheetPrivate::TreeModelColumn::SIGNALNAME, plotIndex), Qt::DisplayRole).toString(), curve2->name());

	{
		QColor color = curve1->line()->pen().color();
		color.setAlpha(worksheet->d_func()->cursorTreeModelCurveBackgroundAlpha);
		QCOMPARE(treemodel->data(treemodel->index(0, (int)WorksheetPrivate::TreeModelColumn::SIGNALNAME, plotIndex), Qt::BackgroundRole).value<QColor>(),
				 color);
	}
	{
		QColor color = curve2->line()->pen().color();
		color.setAlpha(worksheet->d_func()->cursorTreeModelCurveBackgroundAlpha);
		QCOMPARE(treemodel->data(treemodel->index(1, (int)WorksheetPrivate::TreeModelColumn::SIGNALNAME, plotIndex), Qt::BackgroundRole).value<QColor>(),
				 color);
	}
}

#define PLOT_MODEL_INDEX(row) treemodel->index(row, (int)WorksheetPrivate::TreeModelColumn::PLOTNAME)
#define C1_MODEL_INDEX treemodel->index(0, (int)WorksheetPrivate::TreeModelColumn::CURSOR0)
#define CURVE_NAME_MODEL_INDEX(plotIndex, curveRow) treemodel->index(curveRow, (int)WorksheetPrivate::TreeModelColumn::SIGNALNAME, plotIndex)
#define CURVE_C1_MODEL_INDEX(plotIndex, curveRow) treemodel->index(curveRow, (int)WorksheetPrivate::TreeModelColumn::CURSOR0, plotIndex)

void WorksheetTest::cursorNotAllPlotsVisible() {
	Project project;

	auto* worksheet = new Worksheet(QStringLiteral("Worksheet"));
	project.addChild(worksheet);

	auto* plot = new CartesianPlot(QStringLiteral("plot1"));
	worksheet->addChild(plot);
	auto* curve1 = new XYCurve(QStringLiteral("Curve1"));
	plot->addChild(curve1);
	auto* curve2 = new XYCurve(QStringLiteral("Curve2"));
	plot->addChild(curve2);

	auto* plot2 = new CartesianPlot(QStringLiteral("plot2"));
	worksheet->addChild(plot2);
	auto* curve1plot2 = new XYCurve(QStringLiteral("Curve3"));
	plot2->addChild(curve1plot2);
	auto* curve2plot2 = new XYCurve(QStringLiteral("Curve4"));
	plot2->addChild(curve2plot2);

	auto* plot3 = new CartesianPlot(QStringLiteral("plot3"));
	worksheet->addChild(plot3);
	auto* curve1plot3 = new XYCurve(QStringLiteral("Curve5"));
	plot3->addChild(curve1plot3);

	const auto* treemodel = worksheet->cursorModel();

	// Row0: X Value
	// Row1: Plot
	// Row1: Plot
	// Row1: Plot
	QCOMPARE(treemodel->rowCount(), 4);

	QCOMPARE(treemodel->data(treemodel->index(0, (int)WorksheetPrivate::TreeModelColumn::PLOTNAME), Qt::DisplayRole).toString(), QStringLiteral("X"));
	QCOMPARE(treemodel->data(PLOT_MODEL_INDEX(1), Qt::DisplayRole).toString(), QStringLiteral("plot1"));
	QCOMPARE(treemodel->data(PLOT_MODEL_INDEX(2), Qt::DisplayRole).toString(), QStringLiteral("plot2"));
	QCOMPARE(treemodel->data(PLOT_MODEL_INDEX(3), Qt::DisplayRole).toString(), QStringLiteral("plot3"));

	QCOMPARE(treemodel->rowCount(PLOT_MODEL_INDEX(1)), 2);
	QCOMPARE(treemodel->data(CURVE_NAME_MODEL_INDEX(PLOT_MODEL_INDEX(1), 0), Qt::DisplayRole).toString(), curve1->name());
	QCOMPARE(treemodel->data(CURVE_NAME_MODEL_INDEX(PLOT_MODEL_INDEX(1), 1), Qt::DisplayRole).toString(), curve2->name());

	QCOMPARE(treemodel->rowCount(PLOT_MODEL_INDEX(2)), 2);
	QCOMPARE(treemodel->data(CURVE_NAME_MODEL_INDEX(PLOT_MODEL_INDEX(2), 0), Qt::DisplayRole).toString(), curve1plot2->name());
	QCOMPARE(treemodel->data(CURVE_NAME_MODEL_INDEX(PLOT_MODEL_INDEX(2), 1), Qt::DisplayRole).toString(), curve2plot2->name());

	QCOMPARE(treemodel->rowCount(PLOT_MODEL_INDEX(3)), 1);
	QCOMPARE(treemodel->data(CURVE_NAME_MODEL_INDEX(PLOT_MODEL_INDEX(3), 0), Qt::DisplayRole).toString(), curve1plot3->name());

	plot2->setVisible(false);

	QCOMPARE(treemodel->rowCount(), 3);

	QCOMPARE(treemodel->data(treemodel->index(0, (int)WorksheetPrivate::TreeModelColumn::PLOTNAME), Qt::DisplayRole).toString(), QStringLiteral("X"));
	QCOMPARE(treemodel->data(PLOT_MODEL_INDEX(1), Qt::DisplayRole).toString(), QStringLiteral("plot1"));
	QCOMPARE(treemodel->data(PLOT_MODEL_INDEX(2), Qt::DisplayRole).toString(), QStringLiteral("plot3"));

	QCOMPARE(treemodel->rowCount(PLOT_MODEL_INDEX(1)), 2);
	QCOMPARE(treemodel->data(CURVE_NAME_MODEL_INDEX(PLOT_MODEL_INDEX(1), 0), Qt::DisplayRole).toString(), curve1->name());
	QCOMPARE(treemodel->data(CURVE_NAME_MODEL_INDEX(PLOT_MODEL_INDEX(1), 1), Qt::DisplayRole).toString(), curve2->name());

	QCOMPARE(treemodel->rowCount(PLOT_MODEL_INDEX(2)), 1);
	QCOMPARE(treemodel->data(CURVE_NAME_MODEL_INDEX(PLOT_MODEL_INDEX(2), 0), Qt::DisplayRole).toString(), curve1plot3->name());
}

void WorksheetTest::applyCursorToSelected() {
	Project project;

	auto* worksheet = new Worksheet(QStringLiteral("Worksheet"));
	project.addChild(worksheet);

	auto* plot = new CartesianPlot(QStringLiteral("plot1"));
	worksheet->addChild(plot);
	auto* curve1 = new XYCurve(QStringLiteral("Curve1"));
	plot->addChild(curve1);
	auto* curve2 = new XYCurve(QStringLiteral("Curve2"));
	plot->addChild(curve2);

	auto* plot2 = new CartesianPlot(QStringLiteral("plot2"));
	worksheet->addChild(plot2);
	auto* curve1plot2 = new XYCurve(QStringLiteral("Curve3"));
	plot2->addChild(curve1plot2);
	auto* curve2plot2 = new XYCurve(QStringLiteral("Curve4"));
	plot2->addChild(curve2plot2);

	auto* plot3 = new CartesianPlot(QStringLiteral("plot3"));
	worksheet->addChild(plot3);
	auto* curve1plot3 = new XYCurve(QStringLiteral("Curve5"));
	plot3->addChild(curve1plot3);

	const auto* treemodel = worksheet->cursorModel();

	QCOMPARE(treemodel->rowCount(), 4);
	QCOMPARE(treemodel->data(treemodel->index(0, (int)WorksheetPrivate::TreeModelColumn::PLOTNAME), Qt::DisplayRole).toString(), QStringLiteral("X"));
	QCOMPARE(treemodel->data(PLOT_MODEL_INDEX(1), Qt::DisplayRole).toString(), QStringLiteral("plot1"));
	QCOMPARE(treemodel->data(PLOT_MODEL_INDEX(2), Qt::DisplayRole).toString(), QStringLiteral("plot2"));
	QCOMPARE(treemodel->data(PLOT_MODEL_INDEX(3), Qt::DisplayRole).toString(), QStringLiteral("plot3"));
	QCOMPARE(treemodel->rowCount(PLOT_MODEL_INDEX(1)), 2);
	QCOMPARE(treemodel->data(CURVE_NAME_MODEL_INDEX(PLOT_MODEL_INDEX(1), 0), Qt::DisplayRole).toString(), curve1->name());
	QCOMPARE(treemodel->data(CURVE_NAME_MODEL_INDEX(PLOT_MODEL_INDEX(1), 1), Qt::DisplayRole).toString(), curve2->name());
	QCOMPARE(treemodel->rowCount(PLOT_MODEL_INDEX(2)), 2);
	QCOMPARE(treemodel->data(CURVE_NAME_MODEL_INDEX(PLOT_MODEL_INDEX(2), 0), Qt::DisplayRole).toString(), curve1plot2->name());
	QCOMPARE(treemodel->data(CURVE_NAME_MODEL_INDEX(PLOT_MODEL_INDEX(2), 1), Qt::DisplayRole).toString(), curve2plot2->name());
	QCOMPARE(treemodel->rowCount(PLOT_MODEL_INDEX(3)), 1);
	QCOMPARE(treemodel->data(CURVE_NAME_MODEL_INDEX(PLOT_MODEL_INDEX(3), 0), Qt::DisplayRole).toString(), curve1plot3->name());

	plot->mousePressCursorModeSignal(0, QPointF(0.3, 0.)); // Trigger position change

	QCOMPARE(treemodel->data(C1_MODEL_INDEX, Qt::DisplayRole).toDouble(), 0.3);

	worksheet->setCartesianPlotCursorMode(Worksheet::CartesianPlotActionMode::ApplyActionToSelection);

	QCOMPARE(treemodel->rowCount(), 3);

	QCOMPARE(treemodel->data(PLOT_MODEL_INDEX(0), Qt::DisplayRole).toString(), QStringLiteral("plot1"));
	QCOMPARE(treemodel->data(PLOT_MODEL_INDEX(1), Qt::DisplayRole).toString(), QStringLiteral("plot2"));
	QCOMPARE(treemodel->data(PLOT_MODEL_INDEX(2), Qt::DisplayRole).toString(), QStringLiteral("plot3"));

	QCOMPARE(treemodel->rowCount(PLOT_MODEL_INDEX(0)), 3);
	QCOMPARE(treemodel->data(CURVE_NAME_MODEL_INDEX(PLOT_MODEL_INDEX(0), 0), Qt::DisplayRole).toString(), QStringLiteral("X"));
	QCOMPARE(treemodel->data(CURVE_C1_MODEL_INDEX(PLOT_MODEL_INDEX(0), 0), Qt::DisplayRole).toDouble(), 0.3);
	QCOMPARE(treemodel->data(CURVE_NAME_MODEL_INDEX(PLOT_MODEL_INDEX(0), 1), Qt::DisplayRole).toString(), curve1->name());
	QCOMPARE(treemodel->data(CURVE_NAME_MODEL_INDEX(PLOT_MODEL_INDEX(0), 2), Qt::DisplayRole).toString(), curve2->name());

	QCOMPARE(treemodel->rowCount(PLOT_MODEL_INDEX(1)), 3);
	QCOMPARE(treemodel->data(CURVE_NAME_MODEL_INDEX(PLOT_MODEL_INDEX(1), 0), Qt::DisplayRole).toString(), QStringLiteral("X"));
	QCOMPARE(treemodel->data(CURVE_C1_MODEL_INDEX(PLOT_MODEL_INDEX(1), 0), Qt::DisplayRole).toDouble(), 0.3);
	QCOMPARE(treemodel->data(CURVE_NAME_MODEL_INDEX(PLOT_MODEL_INDEX(1), 1), Qt::DisplayRole).toString(), curve1plot2->name());
	QCOMPARE(treemodel->data(CURVE_NAME_MODEL_INDEX(PLOT_MODEL_INDEX(1), 2), Qt::DisplayRole).toString(), curve2plot2->name());

	QCOMPARE(treemodel->rowCount(PLOT_MODEL_INDEX(2)), 2);
	QCOMPARE(treemodel->data(CURVE_NAME_MODEL_INDEX(PLOT_MODEL_INDEX(2), 0), Qt::DisplayRole).toString(), QStringLiteral("X"));
	QCOMPARE(treemodel->data(CURVE_C1_MODEL_INDEX(PLOT_MODEL_INDEX(2), 0), Qt::DisplayRole).toDouble(), 0.3);
	QCOMPARE(treemodel->data(CURVE_NAME_MODEL_INDEX(PLOT_MODEL_INDEX(2), 1), Qt::DisplayRole).toString(), curve1plot3->name());

	plot->mousePressCursorModeSignal(0, QPointF(0.7, 0.));

	QCOMPARE(treemodel->rowCount(), 3);
	QCOMPARE(treemodel->data(PLOT_MODEL_INDEX(0), Qt::DisplayRole).toString(), QStringLiteral("plot1"));
	QCOMPARE(treemodel->data(PLOT_MODEL_INDEX(1), Qt::DisplayRole).toString(), QStringLiteral("plot2"));
	QCOMPARE(treemodel->data(PLOT_MODEL_INDEX(2), Qt::DisplayRole).toString(), QStringLiteral("plot3"));
	QCOMPARE(treemodel->data(CURVE_C1_MODEL_INDEX(PLOT_MODEL_INDEX(0), 0), Qt::DisplayRole).toDouble(), 0.7); // Only plot 1 changed
	QCOMPARE(treemodel->data(CURVE_C1_MODEL_INDEX(PLOT_MODEL_INDEX(1), 0), Qt::DisplayRole).toDouble(), 0.3);
	QCOMPARE(treemodel->data(CURVE_C1_MODEL_INDEX(PLOT_MODEL_INDEX(2), 0), Qt::DisplayRole).toDouble(), 0.3);
}

/*!
 * tests the replacement of the file extension when switching between the different formats during the export.
 */
// TODO: testing the code in GuiTools only. move it later to another location once we have tests for the common code in the frontend.
void WorksheetTest::exportReplaceExtension() {
	// from no extension to .pdf, no dot character in the path
	QString extension = QStringLiteral(".pdf");
#if defined(Q_OS_WIN)
	QString path = QStringLiteral("C:\\Users\\user\\test_dir\\Worksheet");
	QCOMPARE(GuiTools::replaceExtension(path, extension), QStringLiteral("C:\\Users\\user\\test_dir\\Worksheet.pdf"));
#else
	QString path = QStringLiteral("/home/user/test_dir/Worksheet");
	QCOMPARE(GuiTools::replaceExtension(path, extension), QStringLiteral("/home/user/test_dir/Worksheet.pdf"));
#endif

	// from .pdf to .svg, no dot character in the path
	extension = QStringLiteral(".svg");
#if defined(Q_OS_WIN)
	path = QStringLiteral("C:\\Users\\user\\test_dir\\Worksheet.pdf");
	QCOMPARE(GuiTools::replaceExtension(path, extension), QStringLiteral("C:\\Users\\user\\test_dir\\Worksheet.svg"));
#else
	path = QStringLiteral("/home/user/test_dir/Worksheet.pdf");
	QCOMPARE(GuiTools::replaceExtension(path, extension), QStringLiteral("/home/user/test_dir/Worksheet.svg"));
#endif

	// from no extension to .pdf, dot character in the path
	extension = QStringLiteral(".pdf");
#if defined(Q_OS_WIN)
	path = QStringLiteral("C:\\Users\\user\\test.dir\\Worksheet");
	QCOMPARE(GuiTools::replaceExtension(path, extension), QStringLiteral("C:\\Users\\user\\test.dir\\Worksheet.pdf"));
#else
	path = QStringLiteral("/home/user/test.dir/Worksheet");
	QCOMPARE(GuiTools::replaceExtension(path, extension), QStringLiteral("/home/user/test.dir/Worksheet.pdf"));
#endif

	// from .pdf to .svg, dot character in the path
	extension = QStringLiteral(".svg");
#if defined(Q_OS_WIN)
	path = QStringLiteral("C:\\Users\\user\\test.dir\\Worksheet.pdf");
	QCOMPARE(GuiTools::replaceExtension(path, extension), QStringLiteral("C:\\Users\\user\\test.dir\\Worksheet.svg"));
#else
	path = QStringLiteral("/home/user/test.dir/Worksheet.pdf");
	QCOMPARE(GuiTools::replaceExtension(path, extension), QStringLiteral("/home/user/test.dir/Worksheet.svg"));
#endif
}

/*!
 * tests the handling of z-values on changes in the child hierarchy.
 */
void WorksheetTest::zValueAfterAddMoveRemove() {
	auto* worksheet = new Worksheet(QStringLiteral("Worksheet"));

	auto* plot1 = new CartesianPlot(QStringLiteral("plot1"));
	worksheet->addChild(plot1);

	auto* plot2 = new CartesianPlot(QStringLiteral("plot2"));
	worksheet->addChild(plot2);

	auto* plot3 = new CartesianPlot(QStringLiteral("plot3"));
	worksheet->addChild(plot3);

	// after the objects were added, the order of children is
	// * plot1
	// * plot2
	// * plot3
	int zValuePlot1 = plot1->graphicsItem()->zValue();
	int zValuePlot2 = plot2->graphicsItem()->zValue();
	int zValuePlot3 = plot3->graphicsItem()->zValue();
	QVERIFY(zValuePlot1 < zValuePlot2);
	QVERIFY(zValuePlot2 < zValuePlot3);

	// move plot2 up
	worksheet->moveChild(plot2, -1);

	// after the move, the order of children is
	// * plot2
	// * plot1
	// * plot3
	zValuePlot1 = plot1->graphicsItem()->zValue();
	zValuePlot2 = plot2->graphicsItem()->zValue();
	zValuePlot3 = plot3->graphicsItem()->zValue();
	QVERIFY(zValuePlot2 < zValuePlot1);
	QVERIFY(zValuePlot1 < zValuePlot3);

	// move plot1 down
	worksheet->moveChild(plot1, 1);

	// after the move, the order of children is
	// * plot2
	// * plot3
	// * plot1
	zValuePlot1 = plot1->graphicsItem()->zValue();
	zValuePlot2 = plot2->graphicsItem()->zValue();
	zValuePlot3 = plot3->graphicsItem()->zValue();
	QVERIFY(zValuePlot2 < zValuePlot3);
	QVERIFY(zValuePlot3 < zValuePlot1);

	// remove plot3
	worksheet->removeChild(plot3);

	// after the remove, the order of children is
	// * plot2
	// * plot1
	zValuePlot1 = plot1->graphicsItem()->zValue();
	zValuePlot2 = plot2->graphicsItem()->zValue();
	QVERIFY(zValuePlot2 < zValuePlot1);
}

QTEST_MAIN(WorksheetTest)
