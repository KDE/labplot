/*
	File                 : AxisTest.cpp
	Project              : LabPlot
	Description          : Tests for Axis
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022 Martin Marmsoler <martin.marmsoler@gmail.com>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "AxisTest.h"
#include "backend/core/Project.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "src/backend/worksheet/WorksheetElement.h"
#include "src/backend/worksheet/plots/cartesian/Axis.h" // already included in CartesianPlot
#include "src/backend/worksheet/plots/cartesian/AxisPrivate.h"

#include <QUndoStack>

#define private public
#include "src/kdefrontend/dockwidgets/AxisDock.h" // to be able to access the ui elements
#undef private

#define CHECK_AXIS_LABELS(expectedTickValues)                                                                                                                  \
	{                                                                                                                                                          \
		/* To check if retransform ticks was called at the correct time */                                                                                     \
		auto tickLabelValues = xAxis->tickLabelValues();                                                                                            \
		QCOMPARE(tickLabelValues.length(), expectedTickValues.length());                                                                                      \
		for (int i = 0; i < expectedTickValues.length(); i++)                                                                                                    \
			QCOMPARE(tickLabelValues.at(i), expectedTickValues.at(i));                                                                                        \
	}

void AxisTest::majorTicksAutoNumberEnableDisable() {
	Project project;
	auto* ws = new Worksheet("worksheet");
	QVERIFY(ws != nullptr);
	project.addChild(ws);

	auto* p = new CartesianPlot("plot");
	p->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created
	QVERIFY(p != nullptr);
	ws->addChild(p);

	auto axes = p->children<Axis>();
	QCOMPARE(axes.count(), 2);
	QCOMPARE(axes.at(0)->name(), "x");
	QCOMPARE(axes.at(1)->name(), "y");

	AxisDock axisDock(nullptr);

	auto* xAxis = axes.at(0);
	QCOMPARE(xAxis->majorTicksNumber(), 6); // Default number created by autonumbering
	QCOMPARE(xAxis->majorTicksAutoNumber(), true);

	{
		QVector<double> expectedTickValues = {0, 0.2, 0.4, 0.6, 0.8, 1.0};
		CHECK_AXIS_LABELS(expectedTickValues);
	}

	// To check also if the dock shows the correct values
	axisDock.setAxes({xAxis});

	QCOMPARE(axisDock.ui.cbMajorTicksAutoNumber->isChecked(), true);
	QCOMPARE(axisDock.ui.sbMajorTicksNumber->isEnabled(), false);

	// Not possible, because sbMajorTicksNumber is disabled
	// test it nevertless
	xAxis->setMajorTicksNumber(5);
	QCOMPARE(xAxis->majorTicksNumber(), 5);
	QCOMPARE(xAxis->majorTicksAutoNumber(), false);

	{
		QVector<double> expectedTickValues = {0, 0.25, 0.5, 0.75, 1};
		CHECK_AXIS_LABELS(expectedTickValues);
	}

	QCOMPARE(axisDock.ui.cbMajorTicksAutoNumber->isChecked(), false);
	QCOMPARE(axisDock.ui.sbMajorTicksNumber->isEnabled(), true);
	QCOMPARE(axisDock.ui.sbMajorTicksNumber->value(), 5);

	// Check that undo/redo works for setting manual ticknumber
	project.undoStack()->undo();
	QCOMPARE(xAxis->majorTicksNumber(), 6);
	QCOMPARE(xAxis->majorTicksAutoNumber(), true);

	{
		QVector<double> expectedTickValues = {0, 0.2, 0.4, 0.6, 0.8, 1.0};
		CHECK_AXIS_LABELS(expectedTickValues);
	}

	QCOMPARE(axisDock.ui.cbMajorTicksAutoNumber->isChecked(), true);
	QCOMPARE(axisDock.ui.sbMajorTicksNumber->isEnabled(), false);
	QCOMPARE(axisDock.ui.sbMajorTicksNumber->value(), 6);

	project.undoStack()->redo();
	QCOMPARE(xAxis->majorTicksNumber(), 5);
	QCOMPARE(xAxis->majorTicksAutoNumber(), false);

	{
		QVector<double> expectedTickValues = {0, 0.25, 0.5, 0.75, 1};
		CHECK_AXIS_LABELS(expectedTickValues);
	}

	xAxis->setMajorTicksAutoNumber(true);
	QCOMPARE(xAxis->majorTicksNumber(), 6);
	QCOMPARE(xAxis->majorTicksAutoNumber(), true);

	{
		QVector<double> expectedTickValues = {0, 0.2, 0.4, 0.6, 0.8, 1.0};
		CHECK_AXIS_LABELS(expectedTickValues);
	}

	// Check that undo/redo works for setting autonumber enable/disable
	project.undoStack()->undo();
	QCOMPARE(xAxis->majorTicksNumber(), 5);
	QCOMPARE(xAxis->majorTicksAutoNumber(), false);

	{
		QVector<double> expectedTickValues = {0, 0.25, 0.5, 0.75, 1};
		CHECK_AXIS_LABELS(expectedTickValues);
	}

	project.undoStack()->redo();
	QCOMPARE(xAxis->majorTicksNumber(), 6);
	QCOMPARE(xAxis->majorTicksAutoNumber(), true);

	{
		QVector<double> expectedTickValues = {0, 0.2, 0.4, 0.6, 0.8, 1.0};
		CHECK_AXIS_LABELS(expectedTickValues);
	}
}

void AxisTest::minorTicksAutoNumberEnableDisable() {
	Project project;
	auto* ws = new Worksheet("worksheet");
	QVERIFY(ws != nullptr);
	project.addChild(ws);

	auto* p = new CartesianPlot("plot");
	p->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created
	QVERIFY(p != nullptr);
	ws->addChild(p);

	auto axes = p->children<Axis>();
	QCOMPARE(axes.count(), 2);
	QCOMPARE(axes.at(0)->name(), "x");
	QCOMPARE(axes.at(1)->name(), "y");

	AxisDock axisDock(nullptr);

	auto* xAxis = axes.at(0);
	QCOMPARE(xAxis->minorTicksNumber(), 1); // Default number created by autonumbering
	QCOMPARE(xAxis->minorTicksAutoNumber(), true);

	// To check also if the dock shows the correct values
	axisDock.setAxes({xAxis});

	QCOMPARE(axisDock.ui.cbMinorTicksAutoNumber->isChecked(), true);
	QCOMPARE(axisDock.ui.sbMinorTicksNumber->isEnabled(), false);

	// Not possible, because sbminorTicksNumber is disabled
	// test it nevertless
	xAxis->setMinorTicksNumber(5);
	QCOMPARE(xAxis->minorTicksNumber(), 5);
	QCOMPARE(xAxis->minorTicksAutoNumber(), false);

	QCOMPARE(axisDock.ui.cbMinorTicksAutoNumber->isChecked(), false);
	QCOMPARE(axisDock.ui.sbMinorTicksNumber->isEnabled(), true);
	QCOMPARE(axisDock.ui.sbMinorTicksNumber->value(), 5);

	// Check that undo/redo works for setting manual ticknumber
	project.undoStack()->undo();
	QCOMPARE(xAxis->minorTicksNumber(), 1);
	QCOMPARE(xAxis->minorTicksAutoNumber(), true);

	QCOMPARE(axisDock.ui.cbMinorTicksAutoNumber->isChecked(), true);
	QCOMPARE(axisDock.ui.sbMinorTicksNumber->isEnabled(), false);
	QCOMPARE(axisDock.ui.sbMinorTicksNumber->value(), 1);

	project.undoStack()->redo();
	QCOMPARE(xAxis->minorTicksNumber(), 5);
	QCOMPARE(xAxis->minorTicksAutoNumber(), false);

	xAxis->setMinorTicksAutoNumber(true);
	QCOMPARE(xAxis->minorTicksNumber(), 1);
	QCOMPARE(xAxis->minorTicksAutoNumber(), true);

	// Check that undo/redo works for setting autonumber enable/disable
	project.undoStack()->undo();
	QCOMPARE(xAxis->minorTicksNumber(), 5);
	QCOMPARE(xAxis->minorTicksAutoNumber(), false);

	project.undoStack()->redo();
	QCOMPARE(xAxis->minorTicksNumber(), 1);
	QCOMPARE(xAxis->minorTicksAutoNumber(), true);
}

// TODO: check that retransformTicks is called correctly

QTEST_MAIN(AxisTest)
