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
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "src/backend/core/Time.h"
#include "src/backend/worksheet/Line.h"
#include "src/backend/worksheet/TextLabel.h"
#include "src/backend/worksheet/WorksheetElement.h"
#include "src/backend/worksheet/plots/cartesian/Axis.h" // already included in CartesianPlot
#include "src/backend/worksheet/plots/cartesian/AxisPrivate.h"
#include "src/frontend/dockwidgets/AxisDock.h" // access ui elements
#include "src/frontend/widgets/LabelWidget.h"
#include "src/frontend/widgets/LineWidget.h"

#include <QUndoStack>

#define CHECK_AXIS_LABELS(currentTickValues, expectedTickValues)                                                                                               \
	{                                                                                                                                                          \
		/* To check if retransform ticks was called at the correct time */                                                                                     \
		QCOMPARE(currentTickValues.length(), expectedTickValues.length());                                                                                     \
		for (int i = 0; i < expectedTickValues.length(); i++)                                                                                                  \
			QCOMPARE(currentTickValues.at(i), expectedTickValues.at(i));                                                                                       \
	}

void AxisTest::axisLine() {
	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	QVERIFY(ws != nullptr);
	project.addChild(ws);

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	p->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created
	QVERIFY(p != nullptr);
	ws->addChild(p);

	auto axes = p->children<Axis>();
	QCOMPARE(axes.count(), 2);
	QCOMPARE(axes.at(0)->name(), QStringLiteral("x"));
	QCOMPARE(axes.at(1)->name(), QStringLiteral("y"));

	auto* xAxis = axes.at(0);
	auto* yAxis1 = axes.at(1);

	const auto dataRect = p->dataRect();
	const auto bottomLeft = dataRect.bottomLeft();
	const auto topLeft = dataRect.topLeft();
	const auto bottomRight = dataRect.bottomRight();

	{
		auto* axis = yAxis1;

		QCOMPARE(axis->offset(), 0);
		QCOMPARE(axis->position(), Axis::Position::Left);

		const auto& linePath = axis->d_func()->linePath;
		QCOMPARE(linePath.isEmpty(), false);
		QCOMPARE(linePath.elementCount(), 2);

		auto element = linePath.elementAt(0);
		QCOMPARE(element.type, QPainterPath::MoveToElement);
		QCOMPARE(element.x, bottomLeft.x());
		QCOMPARE(element.y, bottomLeft.y());
		element = linePath.elementAt(1);
		QCOMPARE(element.type, QPainterPath::LineToElement);
		QCOMPARE(element.x, topLeft.x());
		QCOMPARE(element.y, topLeft.y());
	}

	{
		auto* axis = xAxis;

		QCOMPARE(axis->offset(), 0);
		QCOMPARE(axis->position(), Axis::Position::Bottom);

		const auto& linePath = axis->d_func()->linePath;
		QCOMPARE(linePath.isEmpty(), false);
		QCOMPARE(linePath.elementCount(), 2);

		auto element = linePath.elementAt(0);
		QCOMPARE(element.type, QPainterPath::MoveToElement);
		QCOMPARE(element.x, bottomLeft.x());
		QCOMPARE(element.y, bottomLeft.y());
		element = linePath.elementAt(1);
		QCOMPARE(element.type, QPainterPath::LineToElement);
		QCOMPARE(element.x, bottomRight.x());
		QCOMPARE(element.y, bottomRight.y());
	}

	yAxis1->copy();
	p->paste();

	axes = p->children<Axis>();
	QCOMPARE(axes.count(), 3);
	QCOMPARE(axes.at(0)->name(), QStringLiteral("x"));
	QCOMPARE(axes.at(1)->name(), QStringLiteral("y"));
	QCOMPARE(axes.at(1), yAxis1);
	QVERIFY(axes.at(2)->name().startsWith(QLatin1Char('y')));

	auto yAxis2 = axes.at(2);

	{
		auto* axis = yAxis2;

		QCOMPARE(axis->offset(), 0);
		QCOMPARE(axis->position(), Axis::Position::Left);

		const auto& linePath = axis->d_func()->linePath;
		QCOMPARE(linePath.isEmpty(), false);
		QCOMPARE(linePath.elementCount(), 2);

		auto element = linePath.elementAt(0);
		QCOMPARE(element.type, QPainterPath::MoveToElement);
		QCOMPARE(element.x, bottomLeft.x());
		QCOMPARE(element.y, bottomLeft.y());
		element = linePath.elementAt(1);
		QCOMPARE(element.type, QPainterPath::LineToElement);
		QCOMPARE(element.x, topLeft.x());
		QCOMPARE(element.y, topLeft.y());
	}
}

void AxisTest::majorTicksAutoNumberEnableDisable() {
	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	QVERIFY(ws != nullptr);
	project.addChild(ws);

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	p->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created
	QVERIFY(p != nullptr);
	ws->addChild(p);

	auto axes = p->children<Axis>();
	QCOMPARE(axes.count(), 2);
	QCOMPARE(axes.at(0)->name(), QStringLiteral("x"));
	QCOMPARE(axes.at(1)->name(), QStringLiteral("y"));

	AxisDock axisDock(nullptr);

	auto* xAxis = axes.at(0);
	QCOMPARE(xAxis->majorTicksNumber(), 6); // Default number created by autonumbering
	QCOMPARE(xAxis->majorTicksAutoNumber(), true);

	{
		QVector<double> expectedTickValues = {0, 0.2, 0.4, 0.6, 0.8, 1.0};
		CHECK_AXIS_LABELS(xAxis->tickLabelValues(), expectedTickValues);
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
		CHECK_AXIS_LABELS(xAxis->tickLabelValues(), expectedTickValues);
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
		CHECK_AXIS_LABELS(xAxis->tickLabelValues(), expectedTickValues);
	}

	QCOMPARE(axisDock.ui.cbMajorTicksAutoNumber->isChecked(), true);
	QCOMPARE(axisDock.ui.sbMajorTicksNumber->isEnabled(), false);
	QCOMPARE(axisDock.ui.sbMajorTicksNumber->value(), 6);

	project.undoStack()->redo();
	QCOMPARE(xAxis->majorTicksNumber(), 5);
	QCOMPARE(xAxis->majorTicksAutoNumber(), false);

	{
		QVector<double> expectedTickValues = {0, 0.25, 0.5, 0.75, 1};
		CHECK_AXIS_LABELS(xAxis->tickLabelValues(), expectedTickValues);
	}

	xAxis->setMajorTicksAutoNumber(true);
	QCOMPARE(xAxis->majorTicksNumber(), 6);
	QCOMPARE(xAxis->majorTicksAutoNumber(), true);

	{
		QVector<double> expectedTickValues = {0, 0.2, 0.4, 0.6, 0.8, 1.0};
		CHECK_AXIS_LABELS(xAxis->tickLabelValues(), expectedTickValues);
	}

	// Check that undo/redo works for setting autonumber enable/disable
	project.undoStack()->undo();
	QCOMPARE(xAxis->majorTicksNumber(), 5);
	QCOMPARE(xAxis->majorTicksAutoNumber(), false);

	{
		QVector<double> expectedTickValues = {0, 0.25, 0.5, 0.75, 1};
		CHECK_AXIS_LABELS(xAxis->tickLabelValues(), expectedTickValues);
	}

	project.undoStack()->redo();
	QCOMPARE(xAxis->majorTicksNumber(), 6);
	QCOMPARE(xAxis->majorTicksAutoNumber(), true);

	{
		QVector<double> expectedTickValues = {0, 0.2, 0.4, 0.6, 0.8, 1.0};
		CHECK_AXIS_LABELS(xAxis->tickLabelValues(), expectedTickValues);
	}
}

void AxisTest::minorTicksAutoNumberEnableDisable() {
	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	QVERIFY(ws != nullptr);
	project.addChild(ws);

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	p->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created
	QVERIFY(p != nullptr);
	ws->addChild(p);

	auto axes = p->children<Axis>();
	QCOMPARE(axes.count(), 2);
	QCOMPARE(axes.at(0)->name(), QStringLiteral("x"));
	QCOMPARE(axes.at(1)->name(), QStringLiteral("y"));

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

void AxisTest::majorTicksStartValue() {
	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	QVERIFY(ws != nullptr);
	project.addChild(ws);

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	p->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created
	QVERIFY(p != nullptr);
	ws->addChild(p);

	auto axes = p->children<Axis>();
	QCOMPARE(axes.count(), 2);
	QCOMPARE(axes.at(0)->name(), QStringLiteral("x"));
	QCOMPARE(axes.at(1)->name(), QStringLiteral("y"));

	AxisDock axisDock(nullptr);

	auto* xAxis = axes.at(0);

	{
		QVector<double> expectedTickValues = {0, 0.2, 0.4, 0.6, 0.8, 1.0};
		CHECK_AXIS_LABELS(xAxis->tickLabelValues(), expectedTickValues);
	}

	// To check also if the dock shows the correct values
	axisDock.setAxes({xAxis});

	QCOMPARE(axisDock.ui.cbMajorTicksStartType->currentIndex(), 1); // by default offset is used

	QCOMPARE(xAxis->majorTicksStartType(), Axis::TicksStartType::Offset);

	xAxis->setMajorTickStartValue(0.1); // does not affect anything, but just that the ticklabels are different to the offset when setting

	xAxis->setMajorTicksStartType(Axis::TicksStartType::Absolute);

	QCOMPARE(axisDock.ui.cbMajorTicksStartType->currentIndex(), 0);

	QCOMPARE(xAxis->majorTicksStartType(), Axis::TicksStartType::Absolute);
	{
		QVector<double> expectedTickValues = {0.1, 0.4, 0.7, 1.0}; // starting now from 0.1
		CHECK_AXIS_LABELS(xAxis->tickLabelValues(), expectedTickValues);
	}

	xAxis->setMajorTickStartValue(0.2);
	QCOMPARE(axisDock.ui.sbMajorTickStartValue->value(), 0.2);

	{
		QVector<double> expectedTickValues = {0.2, 0.4, 0.6, 0.8, 1.0}; // starting now from 0.2
		CHECK_AXIS_LABELS(xAxis->tickLabelValues(), expectedTickValues);
	}

	project.undoStack()->undo();

	QCOMPARE(xAxis->majorTicksStartType(), Axis::TicksStartType::Absolute);

	{
		QVector<double> expectedTickValues = {0.1, 0.4, 0.7, 1.0}; // starting now from 0.1
		CHECK_AXIS_LABELS(xAxis->tickLabelValues(), expectedTickValues);
	}

	project.undoStack()->undo();

	QCOMPARE(xAxis->majorTicksStartType(), Axis::TicksStartType::Offset);

	// by default the offset is zero, so we are starting again from the begining
	{
		QVector<double> expectedTickValues = {0, 0.2, 0.4, 0.6, 0.8, 1.0};
		CHECK_AXIS_LABELS(xAxis->tickLabelValues(), expectedTickValues);
	}
}

void AxisTest::TestSetCoordinateSystem() {
	// Test if the range stored in the Axis gets updated when a new coordinatesystemindex is set
	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	QVERIFY(ws != nullptr);
	project.addChild(ws);

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	p->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axes are created
	QVERIFY(p != nullptr);

	ws->addChild(p);

	auto axes = p->children<Axis>();
	QCOMPARE(axes.count(), 2);
	auto yAxis = axes.at(1);
	QCOMPARE(yAxis->name(), QStringLiteral("y"));
	{
		auto range = yAxis->range();
		QCOMPARE(range.start(), 0);
		QCOMPARE(range.end(), 1);
	}

	// Add new coordinatesystem to the plot
	p->addYRange(Range<double>(5, 100));
	QCOMPARE(p->rangeCount(Dimension::X), 1);
	QCOMPARE(p->rangeCount(Dimension::Y), 2);
	p->addCoordinateSystem();
	QCOMPARE(p->coordinateSystemCount(), 2);
	auto cSystem = p->coordinateSystem(1);
	cSystem->setIndex(Dimension::X, 0);
	cSystem->setIndex(Dimension::Y, 1);

	// Change CoordinatesystemIndex of the axis
	yAxis->setCoordinateSystemIndex(1);

	{
		auto range = yAxis->range();
		QCOMPARE(range.start(), 5);
		QCOMPARE(range.end(), 100);
	}
}

void AxisTest::TestSetRange() {
	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	QVERIFY(ws != nullptr);
	project.addChild(ws);
	auto* p = new CartesianPlot(QStringLiteral("plot"));
	p->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axes are created
	QVERIFY(p != nullptr);
	ws->addChild(p);
	auto axes = p->children<Axis>();
	QCOMPARE(axes.count(), 2);
	auto xAxis = axes.at(0);
	QCOMPARE(xAxis->name(), QStringLiteral("x"));

	// This does not work anymore, because isNumeric() is depending on the
	// CoordinateSystem which is not available when using creating the object
	// TODO: find a way to sync AxisPrivate::Range with the CartesianPlotRange
	// (Not only the start/end, but also the format and the scale!)
	// Then this can be used again
	// Axis xAxis(QStringLiteral("x"), Axis::Orientation::Horizontal);

	auto arange = xAxis->range();
	// different to default values!
	Range<double> r(5, 11, RangeT::Format::DateTime, RangeT::Scale::Log10);
	QVERIFY(arange.start() != r.start());
	QVERIFY(arange.end() != r.end());
	QVERIFY(arange.format() != r.format());
	QVERIFY(arange.scale() != r.scale());

	xAxis->setRange(r);
	arange = xAxis->range();
	QCOMPARE(arange.start(), 5);
	QCOMPARE(arange.end(), 11);
	QCOMPARE(arange.format(), RangeT::Format::DateTime);
	QCOMPARE(arange.scale(), RangeT::Scale::Log10);

	xAxis->setStart(1);
	arange = xAxis->range();
	QCOMPARE(arange.start(), 1);
	QCOMPARE(arange.end(), 11);
	QCOMPARE(arange.format(), RangeT::Format::DateTime);
	QCOMPARE(arange.scale(), RangeT::Scale::Log10);

	xAxis->setEnd(23);
	arange = xAxis->range();
	QCOMPARE(arange.start(), 1);
	QCOMPARE(arange.end(), 23);
	QCOMPARE(arange.format(), RangeT::Format::DateTime);
	QCOMPARE(arange.scale(), RangeT::Scale::Log10);

	xAxis->setRange(-10, 10);
	arange = xAxis->range();
	QCOMPARE(arange.start(), -10);
	QCOMPARE(arange.end(), 10);
	QCOMPARE(arange.format(), RangeT::Format::DateTime);
	QCOMPARE(arange.scale(), RangeT::Scale::Log10);
}

void AxisTest::TestAddingHorizontalAxis() {
	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	QVERIFY(ws != nullptr);
	project.addChild(ws);

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	p->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axes are created
	QVERIFY(p != nullptr);

	ws->addChild(p);

	p->addHorizontalAxis(); // should not crash
}

void AxisTest::TestAddingVerticalAxis() {
	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	QVERIFY(ws != nullptr);
	project.addChild(ws);

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	p->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axes are created
	QVERIFY(p != nullptr);

	ws->addChild(p);

	p->addVerticalAxis(); // should not crash
}

void AxisTest::tickLabelRepresentationAutomatic() {
	QLocale::setDefault(QLocale::C); // . as decimal separator
	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	QVERIFY(ws != nullptr);
	project.addChild(ws);

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	p->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created
	QVERIFY(p != nullptr);
	ws->addChild(p);

	auto axes = p->children<Axis>();
	QCOMPARE(axes.count(), 2);
	QCOMPARE(axes.at(0)->name(), QStringLiteral("x"));
	QCOMPARE(axes.at(1)->name(), QStringLiteral("y"));
	auto* yAxis = static_cast<Axis*>(axes.at(1));

	AxisDock dock(nullptr);
	dock.setAxes({yAxis});

	QCOMPARE(dock.ui.chkLabelsFormatAuto->isChecked(), true);
	QCOMPARE(dock.ui.cbLabelsFormat->isEnabled(), false);

	{
		QCOMPARE(yAxis->labelsFormat(), Axis::LabelsFormat::Decimal);
		QStringList expectedStrings{QStringLiteral("0.0"),
									QStringLiteral("0.2"),
									QStringLiteral("0.4"),
									QStringLiteral("0.6"),
									QStringLiteral("0.8"),
									QStringLiteral("1.0")};
		COMPARE_STRING_VECTORS(yAxis->tickLabelStrings(), expectedStrings);
	}

	p->setRangeDefault(Dimension::Y, Range<double>(0, 1e6));

	{
		QCOMPARE(yAxis->labelsFormat(), Axis::LabelsFormat::Scientific);
		QStringList expectedStrings{
			QStringLiteral("0"),
			AxisPrivate::createScientificRepresentation(QStringLiteral("2"), QStringLiteral("5")),
			AxisPrivate::createScientificRepresentation(QStringLiteral("4"), QStringLiteral("5")),
			AxisPrivate::createScientificRepresentation(QStringLiteral("6"), QStringLiteral("5")),
			AxisPrivate::createScientificRepresentation(QStringLiteral("8"), QStringLiteral("5")),
			AxisPrivate::createScientificRepresentation(QStringLiteral("1"), QStringLiteral("6")),
		};
		COMPARE_STRING_VECTORS(yAxis->tickLabelStrings(), expectedStrings);
	}

	p->setRangeDefault(Dimension::Y, Range<double>(0, 1));

	{
		QCOMPARE(yAxis->labelsFormat(), Axis::LabelsFormat::Decimal);
		QStringList expectedStrings{QStringLiteral("0.0"),
									QStringLiteral("0.2"),
									QStringLiteral("0.4"),
									QStringLiteral("0.6"),
									QStringLiteral("0.8"),
									QStringLiteral("1.0")};
		COMPARE_STRING_VECTORS(yAxis->tickLabelStrings(), expectedStrings);
	}
}

void AxisTest::tickLabelRepresentationManual() {
	QLocale::setDefault(QLocale::English); // . as decimal separator
	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	QVERIFY(ws != nullptr);
	project.addChild(ws);

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	p->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created
	QVERIFY(p != nullptr);
	ws->addChild(p);

	auto axes = p->children<Axis>();
	QCOMPARE(axes.count(), 2);
	QCOMPARE(axes.at(0)->name(), QStringLiteral("x"));
	QCOMPARE(axes.at(1)->name(), QStringLiteral("y"));
	auto* yAxis = static_cast<Axis*>(axes.at(1));

	AxisDock dock(nullptr);
	dock.setAxes({yAxis});

	QCOMPARE(dock.ui.chkLabelsFormatAuto->isChecked(), true);
	QCOMPARE(dock.ui.cbLabelsFormat->isEnabled(), false);

	yAxis->setLabelsFormatAuto(false);

	QCOMPARE(dock.ui.chkLabelsFormatAuto->isChecked(), false);
	QCOMPARE(dock.ui.cbLabelsFormat->isEnabled(), true);

	{
		// Check if applied also when settings
		AxisDock dock2(nullptr);
		dock2.setAxes({yAxis});
		QCOMPARE(dock2.ui.chkLabelsFormatAuto->isChecked(), false);
		QCOMPARE(dock2.ui.cbLabelsFormat->isEnabled(), true);
	}

	{
		QCOMPARE(yAxis->labelsFormat(), Axis::LabelsFormat::Decimal);
		QStringList expectedStrings{QStringLiteral("0.0"),
									QStringLiteral("0.2"),
									QStringLiteral("0.4"),
									QStringLiteral("0.6"),
									QStringLiteral("0.8"),
									QStringLiteral("1.0")};
		COMPARE_STRING_VECTORS(yAxis->tickLabelStrings(), expectedStrings);
	}

	p->setRangeDefault(Dimension::Y, Range<double>(0, 1e6));

	{
		QCOMPARE(yAxis->labelsFormat(), Axis::LabelsFormat::Decimal);
		QStringList expectedStrings{QStringLiteral("0"),
									QStringLiteral("200,000"),
									QStringLiteral("400,000"),
									QStringLiteral("600,000"),
									QStringLiteral("800,000"),
									QStringLiteral("1,000,000")};
		COMPARE_STRING_VECTORS(yAxis->tickLabelStrings(), expectedStrings);
	}

	p->setRangeDefault(Dimension::Y, Range<double>(0, 1));

	{
		QCOMPARE(yAxis->labelsFormat(), Axis::LabelsFormat::Decimal);
		QStringList expectedStrings{QStringLiteral("0.0"),
									QStringLiteral("0.2"),
									QStringLiteral("0.4"),
									QStringLiteral("0.6"),
									QStringLiteral("0.8"),
									QStringLiteral("1.0")};
		COMPARE_STRING_VECTORS(yAxis->tickLabelStrings(), expectedStrings);
	}
}

// TODO: write test switching between numeric and datetime

#define CHECK_TITLE_COLOR(color_)                                                                                                                              \
	QCOMPARE(a->title()->fontColor(), color_);                                                                                                                 \
	QCOMPARE(dock.labelWidget->ui.kcbFontColor->color(), color_);

#define CHECK_MAJOR_TICKS_LINE_COLOR(color_)                                                                                                                   \
	QCOMPARE(a->majorTicksLine()->color(), color_);                                                                                                            \
	QCOMPARE(dock.majorTicksLineWidget->ui.kcbColor->color(), color_);

#define CHECK_MINOR_TICKS_LINE_COLOR(color_)                                                                                                                   \
	QCOMPARE(a->minorTicksLine()->color(), color_);                                                                                                            \
	QCOMPARE(dock.minorTicksLineWidget->ui.kcbColor->color(), color_);

#define CHECK_LINE_COLOR(color_)                                                                                                                               \
	QCOMPARE(a->line()->color(), color_);                                                                                                                      \
	QCOMPARE(dock.lineWidget->ui.kcbColor->color(), color_);

#define CHECK_TICK_LABLES_COLOR(color_)                                                                                                                        \
	QCOMPARE(a->labelsColor(), color_);                                                                                                                        \
	QCOMPARE(dock.ui.kcbLabelsFontColor->color(), color_);

#define CHECK_COMMON_COLOR(color_)                                                                                                                             \
	CHECK_TITLE_COLOR(color_);                                                                                                                                 \
	CHECK_MAJOR_TICKS_LINE_COLOR(color_);                                                                                                                      \
	CHECK_MINOR_TICKS_LINE_COLOR(color_);                                                                                                                      \
	CHECK_LINE_COLOR(color_);                                                                                                                                  \
	CHECK_TICK_LABLES_COLOR(color_);                                                                                                                           \
	QCOMPARE(dock.ui.kcbAxisColor->color(), color_);

#define CREATE_PROJECT                                                                                                                                         \
	Project project;                                                                                                                                           \
	auto* ws = new Worksheet(QStringLiteral("worksheet"));                                                                                                     \
	QVERIFY(ws != nullptr);                                                                                                                                    \
	project.addChild(ws);                                                                                                                                      \
                                                                                                                                                               \
	auto* p = new CartesianPlot(QStringLiteral("plot"));                                                                                                       \
	p->setType(CartesianPlot::Type::TwoAxes); /* Otherwise no axis are created */                                                                              \
	QVERIFY(p != nullptr);                                                                                                                                     \
	ws->addChild(p);                                                                                                                                           \
                                                                                                                                                               \
	auto axes = p->children<Axis>();                                                                                                                           \
	QCOMPARE(axes.count(), 2);                                                                                                                                 \
	QCOMPARE(axes.at(0)->name(), QStringLiteral("x"));                                                                                                         \
	QCOMPARE(axes.at(1)->name(), QStringLiteral("y"));                                                                                                         \
	auto a = axes.at(0);                                                                                                                                       \
	AxisDock dock(nullptr);                                                                                                                                    \
	dock.setAxes({a});                                                                                                                                         \
	CHECK_COMMON_COLOR(Qt::black);

void AxisTest::setAxisColor() {
	CREATE_PROJECT

	// set axis color
	dock.ui.kcbAxisColor->setColor(Qt::red);
	CHECK_COMMON_COLOR(Qt::red);

	// undo/redo
	a->undoStack()->undo();
	CHECK_COMMON_COLOR(Qt::black);
	a->undoStack()->redo();
	CHECK_COMMON_COLOR(Qt::red);
}

void AxisTest::setTitleColor() {
	CREATE_PROJECT

	// change title color
	dock.labelWidget->ui.kcbFontColor->setColor(Qt::green);
	CHECK_TITLE_COLOR(Qt::green);
	QCOMPARE(dock.ui.kcbAxisColor->color(), Qt::transparent);
	CHECK_MAJOR_TICKS_LINE_COLOR(Qt::black);
	CHECK_MINOR_TICKS_LINE_COLOR(Qt::black);
	CHECK_LINE_COLOR(Qt::black);
	CHECK_TICK_LABLES_COLOR(Qt::black);

	a->undoStack()->undo();
	CHECK_COMMON_COLOR(Qt::black);

	a->undoStack()->redo();
	CHECK_TITLE_COLOR(Qt::green);
	QCOMPARE(dock.ui.kcbAxisColor->color(), Qt::transparent);
	CHECK_MAJOR_TICKS_LINE_COLOR(Qt::black);
	CHECK_MINOR_TICKS_LINE_COLOR(Qt::black);
	CHECK_LINE_COLOR(Qt::black);
	CHECK_TICK_LABLES_COLOR(Qt::black);
}

void AxisTest::setMajorTickColor() {
	CREATE_PROJECT

	// change title color
	dock.majorTicksLineWidget->setColor(Qt::green);
	CHECK_MAJOR_TICKS_LINE_COLOR(Qt::green);
	QCOMPARE(dock.ui.kcbAxisColor->color(), Qt::transparent);
	CHECK_TITLE_COLOR(Qt::black);
	CHECK_MINOR_TICKS_LINE_COLOR(Qt::black);
	CHECK_LINE_COLOR(Qt::black);
	CHECK_TICK_LABLES_COLOR(Qt::black);

	a->undoStack()->undo();
	CHECK_COMMON_COLOR(Qt::black);

	a->undoStack()->redo();
	CHECK_MAJOR_TICKS_LINE_COLOR(Qt::green);
	QCOMPARE(dock.ui.kcbAxisColor->color(), Qt::transparent);
	CHECK_TITLE_COLOR(Qt::black);
	CHECK_MINOR_TICKS_LINE_COLOR(Qt::black);
	CHECK_LINE_COLOR(Qt::black);
	CHECK_TICK_LABLES_COLOR(Qt::black);
}

void AxisTest::setMinorTickColor() {
	CREATE_PROJECT

	// change title color
	dock.minorTicksLineWidget->setColor(Qt::green);
	CHECK_MINOR_TICKS_LINE_COLOR(Qt::green);
	QCOMPARE(dock.ui.kcbAxisColor->color(), Qt::transparent);
	CHECK_TITLE_COLOR(Qt::black);
	CHECK_MAJOR_TICKS_LINE_COLOR(Qt::black);
	CHECK_LINE_COLOR(Qt::black);
	CHECK_TICK_LABLES_COLOR(Qt::black);

	a->undoStack()->undo();
	CHECK_COMMON_COLOR(Qt::black);

	a->undoStack()->redo();
	CHECK_MINOR_TICKS_LINE_COLOR(Qt::green);
	QCOMPARE(dock.ui.kcbAxisColor->color(), Qt::transparent);
	CHECK_TITLE_COLOR(Qt::black);
	CHECK_MAJOR_TICKS_LINE_COLOR(Qt::black);
	CHECK_LINE_COLOR(Qt::black);
	CHECK_TICK_LABLES_COLOR(Qt::black);
}

void AxisTest::setLineColor() {
	CREATE_PROJECT

	// change title color
	dock.lineWidget->setColor(Qt::green);
	CHECK_LINE_COLOR(Qt::green);
	QCOMPARE(dock.ui.kcbAxisColor->color(), Qt::transparent);
	CHECK_TITLE_COLOR(Qt::black);
	CHECK_MAJOR_TICKS_LINE_COLOR(Qt::black);
	CHECK_MINOR_TICKS_LINE_COLOR(Qt::black);
	CHECK_TICK_LABLES_COLOR(Qt::black);

	a->undoStack()->undo();
	CHECK_COMMON_COLOR(Qt::black);

	a->undoStack()->redo();
	CHECK_LINE_COLOR(Qt::green);
	QCOMPARE(dock.ui.kcbAxisColor->color(), Qt::transparent);
	CHECK_TITLE_COLOR(Qt::black);
	CHECK_MAJOR_TICKS_LINE_COLOR(Qt::black);
	CHECK_MINOR_TICKS_LINE_COLOR(Qt::black);
	CHECK_TICK_LABLES_COLOR(Qt::black);
}

void AxisTest::setTickLabelColor() {
	CREATE_PROJECT

	// change title color
	dock.ui.kcbLabelsFontColor->setColor(Qt::green);
	CHECK_TICK_LABLES_COLOR(Qt::green);
	QCOMPARE(dock.ui.kcbAxisColor->color(), Qt::transparent);
	CHECK_TITLE_COLOR(Qt::black);
	CHECK_MAJOR_TICKS_LINE_COLOR(Qt::black);
	CHECK_MINOR_TICKS_LINE_COLOR(Qt::black);
	CHECK_LINE_COLOR(Qt::black);

	a->undoStack()->undo();
	CHECK_COMMON_COLOR(Qt::black);

	a->undoStack()->redo();
	CHECK_TICK_LABLES_COLOR(Qt::green);
	QCOMPARE(dock.ui.kcbAxisColor->color(), Qt::transparent);
	CHECK_TITLE_COLOR(Qt::black);
	CHECK_MAJOR_TICKS_LINE_COLOR(Qt::black);
	CHECK_MINOR_TICKS_LINE_COLOR(Qt::black);
	CHECK_LINE_COLOR(Qt::black);
}

void AxisTest::automaticTicNumberUpdateDockMajorTicks() {
	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	QVERIFY(ws != nullptr);
	project.addChild(ws);

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	p->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created
	QVERIFY(p != nullptr);
	ws->addChild(p);

	auto axes = p->children<Axis>();
	QCOMPARE(axes.count(), 2);
	QCOMPARE(axes.at(0)->name(), QStringLiteral("x"));
	QCOMPARE(axes.at(1)->name(), QStringLiteral("y"));
	auto* yAxis = static_cast<Axis*>(axes.at(1));
	auto* xAxis = static_cast<Axis*>(axes.at(0));

	AxisDock dock(nullptr);

	dock.setAxes({xAxis, yAxis});
	dock.ui.cbMajorTicksAutoNumber->setChecked(false);
	dock.ui.sbMajorTicksNumber->setValue(10);

	// Check majorticks numbers of the axes
	QCOMPARE(xAxis->majorTicksNumber(), 10);
	QCOMPARE(xAxis->majorTicksAutoNumber(), false);
	QCOMPARE(yAxis->majorTicksNumber(), 10);
	QCOMPARE(xAxis->majorTicksAutoNumber(), false);
	QCOMPARE(dock.ui.cbMajorTicksAutoNumber->isChecked(), false);

	dock.setAxes({xAxis, yAxis}); // Another time
	dock.ui.cbMajorTicksAutoNumber->setChecked(true);

	QCOMPARE(xAxis->majorTicksNumber(), 6);
	QCOMPARE(xAxis->majorTicksAutoNumber(), true);
	QCOMPARE(yAxis->majorTicksNumber(), 6);
	QCOMPARE(xAxis->majorTicksAutoNumber(), true);
	QCOMPARE(dock.ui.sbMajorTicksNumber->value(), 6);
}

void AxisTest::automaticTicNumberUpdateDockMinorTicks() {
	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	QVERIFY(ws != nullptr);
	project.addChild(ws);

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	p->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created
	QVERIFY(p != nullptr);
	ws->addChild(p);

	auto axes = p->children<Axis>();
	QCOMPARE(axes.count(), 2);
	QCOMPARE(axes.at(0)->name(), QStringLiteral("x"));
	QCOMPARE(axes.at(1)->name(), QStringLiteral("y"));
	auto* yAxis = static_cast<Axis*>(axes.at(1));
	auto* xAxis = static_cast<Axis*>(axes.at(0));

	AxisDock dock(nullptr);
	dock.setAxes({xAxis, yAxis});
	dock.ui.cbMinorTicksAutoNumber->setChecked(false);
	dock.ui.sbMinorTicksNumber->setValue(10);

	// Check minorticks numbers of the axes
	QCOMPARE(xAxis->minorTicksNumber(), 10);
	QCOMPARE(xAxis->minorTicksAutoNumber(), false);
	QCOMPARE(yAxis->minorTicksNumber(), 10);
	QCOMPARE(xAxis->minorTicksAutoNumber(), false);
	QCOMPARE(dock.ui.cbMinorTicksAutoNumber->isChecked(), false);

	dock.setAxes({xAxis, yAxis}); // Another time
	QCOMPARE(dock.ui.cbMinorTicksAutoNumber->isChecked(), false);
	dock.ui.cbMinorTicksAutoNumber->setChecked(true);

	// 1 is the default value for automatic
	QCOMPARE(xAxis->minorTicksNumber(), 1);
	QCOMPARE(xAxis->minorTicksAutoNumber(), true);
	QCOMPARE(yAxis->minorTicksNumber(), 1);
	QCOMPARE(xAxis->minorTicksAutoNumber(), true);
	QCOMPARE(dock.ui.sbMinorTicksNumber->value(), 1);
}

void AxisTest::columnLabelValues() {
	QLocale::setDefault(QLocale::C); // . as decimal separator
	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	QVERIFY(ws != nullptr);
	project.addChild(ws);

	Spreadsheet* spreadsheet = new Spreadsheet(QStringLiteral("test"), false);
	spreadsheet->setColumnCount(2);
	spreadsheet->setRowCount(3);
	project.addChild(spreadsheet);

	auto* xCol = spreadsheet->column(0);
	xCol->replaceValues(0, QVector<double>({1, 2, 3}));

	auto* yCol = spreadsheet->column(1);
	yCol->replaceValues(0, QVector<double>({2, 3, 4}));

	QCOMPARE(spreadsheet->rowCount(), 3);
	QCOMPARE(spreadsheet->columnCount(), 2);

	yCol->addValueLabel(2, QStringLiteral("Status 1"));
	yCol->addValueLabel(3, QStringLiteral("Status 2"));
	yCol->addValueLabel(4, QStringLiteral("Status 3"));

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	p->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created
	QVERIFY(p != nullptr);
	ws->addChild(p);

	auto* curve = new XYCurve(QStringLiteral("xy-curve"));
	curve->setXColumn(xCol);
	curve->setYColumn(yCol);
	p->addChild(curve);

	auto axes = p->children<Axis>();
	QCOMPARE(axes.count(), 2);
	QCOMPARE(axes.at(0)->name(), QStringLiteral("x"));
	QCOMPARE(axes.at(1)->name(), QStringLiteral("y"));
	auto* yAxis = static_cast<Axis*>(axes.at(1));
	// auto* xAxis = static_cast<Axis*>(axes.at(0));

	{
		QCOMPARE(yAxis->labelsFormat(), Axis::LabelsFormat::Decimal);
		const auto v = yAxis->tickLabelStrings();
		QStringList expectedStrings{
			QStringLiteral("2.0"),
			QStringLiteral("2.5"),
			QStringLiteral("3.0"),
			QStringLiteral("3.5"),
			QStringLiteral("4.0"),
		};
		COMPARE_STRING_VECTORS(yAxis->tickLabelStrings(), expectedStrings);
	}

	{
		yAxis->setMajorTicksType(Axis::TicksType::ColumnLabels);
		yAxis->setMajorTicksColumn(yCol);

		QStringList expectedStrings{
			QStringLiteral("Status 1"),
			QStringLiteral("Status 2"),
			QStringLiteral("Status 3"),
		};
		COMPARE_STRING_VECTORS(yAxis->tickLabelStrings(), expectedStrings);
	}
}

/*!
 * \brief AxisTest::columnLabelValuesMaxValues
 * Same as columnLabelValuesMaxValues() with the difference
 * that more columnLabels are available than the maximum number of ticks allowed
 * in the axis. This leads to a limited representation of ticks/labels
 */
void AxisTest::columnLabelValuesMaxValues() {
	constexpr int valueLabelsCount = 1000;
	QLocale::setDefault(QLocale::C); // . as decimal separator
	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	QVERIFY(ws != nullptr);
	project.addChild(ws);

	Spreadsheet* spreadsheet = new Spreadsheet(QStringLiteral("test"), false);
	spreadsheet->setColumnCount(2);
	spreadsheet->setRowCount(3);
	project.addChild(spreadsheet);

	auto* xCol = spreadsheet->column(0);
	xCol->replaceValues(-1, QVector<double>({1., 100.}));

	auto* yCol = spreadsheet->column(1);
	yCol->replaceValues(-1, QVector<double>({1., 1000.}));

	QCOMPARE(spreadsheet->rowCount(), 2);
	QCOMPARE(spreadsheet->columnCount(), 2);

	for (int i = 0; i <= valueLabelsCount; i++) {
		yCol->addValueLabel(i, QStringLiteral("Status ") + QString::number(i));
	}

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	p->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created
	QVERIFY(p != nullptr);
	ws->addChild(p);

	auto* curve = new XYCurve(QStringLiteral("xy-curve"));
	curve->setXColumn(xCol);
	curve->setYColumn(yCol);
	p->addChild(curve);

	auto axes = p->children<Axis>();
	QCOMPARE(axes.count(), 2);
	QCOMPARE(axes.at(0)->name(), QStringLiteral("x"));
	QCOMPARE(axes.at(1)->name(), QStringLiteral("y"));
	auto* yAxis = static_cast<Axis*>(axes.at(1));
	// auto* xAxis = static_cast<Axis*>(axes.at(0));

	{
		yAxis->setMajorTicksType(Axis::TicksType::ColumnLabels);
		yAxis->setMajorTicksColumn(yCol);

		const auto v = yAxis->tickLabelStrings();
		QStringList expectedStrings;
		for (int i = 0; i <= valueLabelsCount; i += valueLabelsCount / (Axis::maxNumberMajorTicksCustomColumn() - 1))
			expectedStrings.push_back(QStringLiteral("Status ") + QString::number(i));

		COMPARE_STRING_VECTORS(yAxis->tickLabelStrings(), expectedStrings);
	}
}

void AxisTest::customTextLabels() {
	QLocale::setDefault(QLocale::C); // . as decimal separator
	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	QVERIFY(ws != nullptr);
	project.addChild(ws);

	Spreadsheet* spreadsheetData = new Spreadsheet(QStringLiteral("data"), false);
	spreadsheetData->setColumnCount(2);
	spreadsheetData->setRowCount(3);
	project.addChild(spreadsheetData);

	auto* xCol = spreadsheetData->column(0);
	xCol->replaceValues(0, QVector<double>({1, 2, 3}));

	auto* yCol = spreadsheetData->column(1);
	yCol->replaceValues(0, QVector<double>({2, 3, 4}));

	QCOMPARE(spreadsheetData->rowCount(), 3);
	QCOMPARE(spreadsheetData->columnCount(), 2);

	Spreadsheet* spreadsheetLabels = new Spreadsheet(QStringLiteral("data"), false);
	spreadsheetLabels->setColumnCount(1);
	spreadsheetLabels->setRowCount(3);
	project.addChild(spreadsheetLabels);
	auto* labelsCol = spreadsheetLabels->column(0);
	labelsCol->setColumnMode(AbstractColumn::ColumnMode::Text);
	labelsCol->replaceTexts(-1, QVector<QString>({QStringLiteral("A"), QStringLiteral("B"), QStringLiteral("C")}));

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	p->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created
	QVERIFY(p != nullptr);
	ws->addChild(p);

	auto* curve = new XYCurve(QStringLiteral("xy-curve"));
	curve->setXColumn(xCol);
	curve->setYColumn(yCol);
	p->addChild(curve);

	auto axes = p->children<Axis>();
	QCOMPARE(axes.count(), 2);
	QCOMPARE(axes.at(0)->name(), QStringLiteral("x"));
	QCOMPARE(axes.at(1)->name(), QStringLiteral("y"));

	auto* xAxis = static_cast<Axis*>(axes.at(0));
	xAxis->setMajorTicksNumber(3, false);
	xAxis->setLabelsTextType(Axis::LabelsTextType::CustomValues);
	xAxis->setLabelsTextColumn(labelsCol);

	{
		const auto v = xAxis->tickLabelStrings();
		QStringList expectedStrings{
			QStringLiteral("A"),
			QStringLiteral("B"),
			QStringLiteral("C"),
		};
		COMPARE_STRING_VECTORS(xAxis->tickLabelStrings(), expectedStrings);
	}
}

QTEST_MAIN(AxisTest)
