/*
	File                 : AxisTest.cpp
	Project              : LabPlot
	Description          : Tests for Axis
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022 Martin Marmsoler <martin.marmsoler@gmail.com>
	SPDX-FileCopyrightText: 2022-2025 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "AxisTest.h"
#include "backend/core/Project.h"
#include "backend/core/column/Column.h"
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

// TODO: write test switching between numeric and datetime

#define CHECK_AXIS_LABELS(currentTickValues, expectedTickValues)                                                                                               \
	{                                                                                                                                                          \
		/* To check if retransform ticks was called at the correct time */                                                                                     \
		QCOMPARE(currentTickValues.length(), expectedTickValues.length());                                                                                     \
		for (int i = 0; i < expectedTickValues.length(); i++)                                                                                                  \
			QCOMPARE(currentTickValues.at(i), expectedTickValues.at(i));                                                                                       \
	}

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

void AxisTest::axisLine() {
	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	QVERIFY(ws != nullptr);
	project.addChild(ws);

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	QVERIFY(p != nullptr);
	p->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created
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

void AxisTest::minorTicksAutoNumberEnableDisable() {
	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	QVERIFY(ws != nullptr);
	project.addChild(ws);

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	QVERIFY(p != nullptr);
	p->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created
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
	QVERIFY(p != nullptr);
	p->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created
	ws->addChild(p);

	auto axes = p->children<Axis>();
	QCOMPARE(axes.count(), 2);
	QCOMPARE(axes.at(0)->name(), QStringLiteral("x"));
	QCOMPARE(axes.at(1)->name(), QStringLiteral("y"));

	AxisDock axisDock(nullptr);

	auto* xAxis = axes.at(0);
	QCOMPARE(xAxis->majorTicksAutoNumber(), true);
	xAxis->setMajorTicksNumber(5); // Expected might different because of auto

	{
		QVector<double> expectedTickValues = {0, 0.2, 0.4, 0.6, 0.8, 1.0};
		CHECK_AXIS_LABELS(xAxis->tickLabelValues(), expectedTickValues);
	}

	// To check also if the dock shows the correct values
	axisDock.setAxes({xAxis});

	QCOMPARE(axisDock.ui.cbMajorTicksStartType->currentIndex(), 1); // by default offset is used

	QCOMPARE(xAxis->majorTicksStartType(), Axis::TicksStartType::Offset);

	xAxis->setMajorTickStartValue(0.5); // does not affect anything, but just that the ticklabels are different to the offset when setting

	xAxis->setMajorTicksStartType(Axis::TicksStartType::Absolute);

	QCOMPARE(axisDock.ui.cbMajorTicksStartType->currentIndex(), 0);

	QCOMPARE(xAxis->majorTicksStartType(), Axis::TicksStartType::Absolute);
	{
		QVector<double> expectedTickValues = {0.5, 0.6, 0.7, 0.8, 0.9, 1.0}; // starting now from 0.5
		CHECK_AXIS_LABELS(xAxis->tickLabelValues(), expectedTickValues);
	}

	xAxis->setMajorTickStartValue(0.7);
	QCOMPARE(axisDock.ui.sbMajorTickStartValue->value(), 0.7);

	{
		QVector<double> expectedTickValues = {0.7, 0.75, 0.8, 0.85, 0.9, 0.95, 1.0}; // starting now from 0.7
		CHECK_AXIS_LABELS(xAxis->tickLabelValues(), expectedTickValues);
	}

	project.undoStack()->undo();

	QCOMPARE(xAxis->majorTicksStartType(), Axis::TicksStartType::Absolute);

	{
		QVector<double> expectedTickValues = {0.5, 0.6, 0.7, 0.8, 0.9, 1.0}; // starting again from 0.5
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
	QVERIFY(p != nullptr);
	p->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axes are created
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
	QVERIFY(p != nullptr);
	p->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axes are created
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
	QVERIFY(p != nullptr);
	p->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axes are created
	ws->addChild(p);

	p->addHorizontalAxis(); // should not crash
}

void AxisTest::TestAddingVerticalAxis() {
	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	QVERIFY(ws != nullptr);
	project.addChild(ws);

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	QVERIFY(p != nullptr);
	p->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axes are created
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
	QVERIFY(p != nullptr);
	p->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created
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
	QVERIFY(p != nullptr);
	p->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created
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
	QVERIFY(p != nullptr);
	p->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created
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
	QVERIFY(p != nullptr);
	p->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created
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

/*!
 * checks the spacing of the ticks that is automatically calculated and set
 * after the switch from "number" to "spacing" ticks type in the dock widget.
 */
void AxisTest::tickSpacingUpdateDockMajorTicks() {
	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	QVERIFY(ws != nullptr);
	project.addChild(ws);

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	QVERIFY(p != nullptr);
	p->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created
	ws->addChild(p);

	auto axes = p->children<Axis>();
	auto* xAxis = static_cast<Axis*>(axes.at(0));

	AxisDock dock(nullptr);
	dock.setAxes({xAxis});
	dock.ui.cbMajorTicksType->setCurrentIndex(dock.ui.cbMajorTicksType->findData((int)Axis::TicksType::Spacing));
	dock.ui.cbMinorTicksType->setCurrentIndex(dock.ui.cbMinorTicksType->findData((int)Axis::TicksType::Spacing));

	// initially, the spacing is not set (equal to zero) and after the switch to "spacing"
	// it's adjusted to the current spacing determined by the total number of the ticks:
	// * for major ticks, for the range 0-1 and the initial number of ticks is 6 and it corresponds to spacing = 0.2
	// * for minor ticks, the spacing between the major ticks is 0.2 and the number of minor ticks is 1 which corresponds to spacing = 0.1
	QCOMPARE(xAxis->majorTicksSpacing(), 0.2);
	QCOMPARE(xAxis->minorTicksSpacing(), 0.1);
}

void AxisTest::testComputeMajorTickStart() {
	int majorTickCount = 5;
	double spacing = 0;
	AxisPrivate::calculateAutoParameters(majorTickCount, Range<double>(-0.7, 0.8, RangeT::Format::Numeric, RangeT::Scale::Linear), spacing);

	majorTickCount = 5;
	AxisPrivate::calculateAutoParameters(majorTickCount, Range<double>(0.3, 1.0, RangeT::Format::Numeric, RangeT::Scale::Linear), spacing);

	majorTickCount = 5;
	AxisPrivate::calculateAutoParameters(majorTickCount, Range<double>(0.1, 1.0, RangeT::Format::Numeric, RangeT::Scale::Linear), spacing);

	majorTickCount = 5;
	AxisPrivate::calculateAutoParameters(majorTickCount, Range<double>(-250., 250., RangeT::Format::Numeric, RangeT::Scale::Linear), spacing);

	majorTickCount = 7;
	AxisPrivate::calculateAutoParameters(majorTickCount, Range<double>(-250., 250., RangeT::Format::Numeric, RangeT::Scale::Linear), spacing);
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
	yAxis->setMajorTicksNumber(5);
	// auto* xAxis = static_cast<Axis*>(axes.at(0));

	{
		QCOMPARE(yAxis->labelsFormat(), Axis::LabelsFormat::Decimal);
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
 * Same as columnLabelValues() with the difference
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
	p->setNiceExtend(true);
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

/*!
 * \brief AxisTest::columnLabelValuesMoreTicksThanLabels
 * Draw all ticks regardsless of their position, because we have only a few
 */
void AxisTest::columnLabelValuesMoreTicksThanLabels() {
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
	yCol->replaceValues(0, QVector<double>({3, 5, 10}));

	QCOMPARE(spreadsheet->rowCount(), 3);
	QCOMPARE(spreadsheet->columnCount(), 2);

	yCol->addValueLabel(0, QStringLiteral("Status 1"));
	yCol->addValueLabel(1, QStringLiteral("Status 2"));
	yCol->addValueLabel(2, QStringLiteral("Status 3"));
	yCol->addValueLabel(3, QStringLiteral("Status 4"));
	yCol->addValueLabel(4, QStringLiteral("Status 5"));
	// yCol->addValueLabel(5, QStringLiteral("Status 6")); Gap
	// yCol->addValueLabel(6, QStringLiteral("Status 7")); Gap
	yCol->addValueLabel(7, QStringLiteral("Status 8"));
	yCol->addValueLabel(8, QStringLiteral("Status 9"));
	yCol->addValueLabel(9, QStringLiteral("Status 10"));
	yCol->addValueLabel(10, QStringLiteral("Status 11"));

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

	{
		yAxis->setMajorTicksType(Axis::TicksType::ColumnLabels);
		yAxis->setMajorTicksColumn(yCol);

		QStringList expectedStrings{
			QStringLiteral("Status 4"),
			QStringLiteral("Status 5"),
			// QStringLiteral("Status 6"),
			// QStringLiteral("Status 7"),
			QStringLiteral("Status 8"),
			QStringLiteral("Status 9"),
			QStringLiteral("Status 10"),
			QStringLiteral("Status 11"),
		};
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
	xAxis->setMajorTicksAutoNumber(false);
	xAxis->setMajorTicksNumber(3);
	xAxis->setLabelsTextType(Axis::LabelsTextType::CustomValues);
	xAxis->setLabelsTextColumn(labelsCol);

	{
		QStringList expectedStrings{
			QStringLiteral("A"),
			QStringLiteral("B"),
			QStringLiteral("C"),
		};
		COMPARE_STRING_VECTORS(xAxis->tickLabelStrings(), expectedStrings);
	}
}

/*!
 * \brief AxisTest::customTextLabelsMoreTicksThanLabels
 * Draw all ticks regardsless of their position, because we have only a few
 */
void AxisTest::customTextLabelsMoreTicksThanLabels() {
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
	xCol->replaceValues(0, QVector<double>({3, 5, 10}));

	auto* yCol = spreadsheetData->column(1);
	yCol->replaceValues(0, QVector<double>({3, 5, 10}));

	QCOMPARE(spreadsheetData->rowCount(), 3);
	QCOMPARE(spreadsheetData->columnCount(), 2);

	Spreadsheet* spreadsheetLabels = new Spreadsheet(QStringLiteral("data"), false);
	spreadsheetLabels->setColumnCount(2);
	spreadsheetLabels->setRowCount(11);
	project.addChild(spreadsheetLabels);
	auto* valuesCol = spreadsheetLabels->column(0);
	valuesCol->setColumnMode(AbstractColumn::ColumnMode::Double);
	valuesCol->replaceValues(-1, QVector<double>({0., 1., 2., 3., 4., 7., 8., 9., 10.}));
	VALUES_EQUAL(valuesCol->valueAt(1), 1.);

	auto* labelsCol = spreadsheetLabels->column(1);
	labelsCol->setColumnMode(AbstractColumn::ColumnMode::Text);
	labelsCol->replaceTexts(-1,
							QVector<QString>({QStringLiteral("A"),
											  QStringLiteral("B"),
											  QStringLiteral("C"),
											  QStringLiteral("D"),
											  QStringLiteral("E"),
											  QStringLiteral("H"),
											  QStringLiteral("I"),
											  QStringLiteral("J"),
											  QStringLiteral("K")}));

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
	xAxis->setMajorTicksAutoNumber(false);
	xAxis->setMajorTicksNumber(100);
	xAxis->setMajorTicksType(Axis::TicksType::CustomColumn);
	xAxis->setMajorTicksColumn(valuesCol);
	xAxis->setLabelsTextType(Axis::LabelsTextType::CustomValues);
	xAxis->setLabelsTextColumn(labelsCol);

	{
		const auto v = xAxis->tickLabelStrings();
		QStringList expectedStrings{
			QStringLiteral("D"),
			QStringLiteral("E"),
			// QStringLiteral("F"), gap
			// QStringLiteral("G"), gap
			QStringLiteral("H"),
			QStringLiteral("I"),
			QStringLiteral("J"),
			QStringLiteral("K"),
		};
		COMPARE_STRING_VECTORS(xAxis->tickLabelStrings(), expectedStrings);
	}
}

void AxisTest::dateTime() {
	QLocale::setDefault(QLocale::C); // use . as decimal separator
	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	QVERIFY(ws != nullptr);
	project.addChild(ws);

	Spreadsheet* spreadsheetData = new Spreadsheet(QStringLiteral("data"), false);
	spreadsheetData->setColumnCount(2);
	spreadsheetData->setRowCount(3);
	project.addChild(spreadsheetData);

	auto* xCol = spreadsheetData->column(0);
	xCol->setColumnMode(AbstractColumn::ColumnMode::DateTime);
	QDateTime dt1 = QDateTime::fromString(QStringLiteral("2017-07-24T00:00:00Z"), Qt::ISODate);
	QDateTime dt2 = QDateTime::fromString(QStringLiteral("2017-08-24T00:00:00Z"), Qt::ISODate);
	QDateTime dt3 = QDateTime::fromString(QStringLiteral("2017-09-24T00:00:00Z"), Qt::ISODate);
	xCol->replaceDateTimes(-1, QVector<QDateTime>({dt1, dt2, dt3}));

	auto* yCol = spreadsheetData->column(1);
	yCol->replaceValues(-1, QVector<double>({2., 3., 4.}));

	QCOMPARE(spreadsheetData->rowCount(), 3);
	QCOMPARE(spreadsheetData->columnCount(), 2);

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	p->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axes are created
	p->setNiceExtend(false);
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
	xAxis->setMajorTicksAutoNumber(false);
	xAxis->setMajorTicksNumber(3);
	QCOMPARE(xAxis->range().start(), dt1.toMSecsSinceEpoch());
	QCOMPARE(xAxis->range().end(), dt3.toMSecsSinceEpoch());
	QCOMPARE(xAxis->majorTicksType(), Axis::TicksType::TotalNumber);
	QCOMPARE(xAxis->majorTicksNumber(), 3);
	xAxis->setLabelsDateTimeFormat(QStringLiteral("yyyy-MM-dd hh:mm:ss"));
	QCOMPARE(xAxis->labelsTextType(), Axis::LabelsTextType::PositionValues);

	{
		const auto v = xAxis->tickLabelStrings();
		QStringList expectedStrings{
			QStringLiteral("2017-07-24 00:00:00"),
			QStringLiteral("2017-08-24 00:00:00"),
			QStringLiteral("2017-09-24 00:00:00"),
		};
		COMPARE_STRING_VECTORS(xAxis->tickLabelStrings(), expectedStrings);
	}
}

void AxisTest::dateTimeSpacing() {
	QLocale::setDefault(QLocale::C); // use . as decimal separator
	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	QVERIFY(ws != nullptr);
	project.addChild(ws);

	Spreadsheet* spreadsheetData = new Spreadsheet(QStringLiteral("data"), false);
	spreadsheetData->setColumnCount(2);
	spreadsheetData->setRowCount(3);
	project.addChild(spreadsheetData);

	auto* xCol = spreadsheetData->column(0);
	xCol->setColumnMode(AbstractColumn::ColumnMode::DateTime);
	QDateTime dt1 = QDateTime::fromString(QStringLiteral("2017-07-24T00:00:00Z"), Qt::ISODate);
	QDateTime dt2 = QDateTime::fromString(QStringLiteral("2017-11-24T12:03:00Z"), Qt::ISODate);
	QDateTime dt3 = QDateTime::fromString(QStringLiteral("2017-12-24T00:05:03Z"), Qt::ISODate);
	xCol->replaceDateTimes(-1, QVector<QDateTime>({dt1, dt2, dt3}));

	auto* yCol = spreadsheetData->column(1);
	yCol->replaceValues(-1, QVector<double>({2., 3., 4.}));

	QCOMPARE(spreadsheetData->rowCount(), 3);
	QCOMPARE(spreadsheetData->columnCount(), 2);

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	p->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axes are created
	p->setNiceExtend(false);
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
	xAxis->setMajorTicksSpacing(DateTime::createValue(0, 1, 0, 0, 0, 0, 0)); // 1 month
	QCOMPARE(xAxis->range().start(), dt1.toMSecsSinceEpoch());
	QCOMPARE(xAxis->range().end(), dt3.toMSecsSinceEpoch());
	xAxis->setMajorTicksType(Axis::TicksType::Spacing);
	// QCOMPARE(xAxis->majorTicksNumber(), 3);
	xAxis->setLabelsDateTimeFormat(QStringLiteral("yyyy-MM-dd hh:mm:ss"));
	QCOMPARE(xAxis->labelsTextType(), Axis::LabelsTextType::PositionValues);
	QCOMPARE(xAxis->majorTicksStartType(), Axis::TicksStartType::Offset);
	QCOMPARE(xAxis->majorTickStartOffset(), 0);
	QCOMPARE(xAxis->majorTickStartValue(), 0);

	{
		const auto v = xAxis->tickLabelStrings();
		QStringList expectedStrings{
			QStringLiteral("2017-07-24 00:00:00"),
			QStringLiteral("2017-08-24 00:00:00"),
			QStringLiteral("2017-09-24 00:00:00"),
			QStringLiteral("2017-10-24 00:00:00"),
			QStringLiteral("2017-11-24 00:00:00"),
			QStringLiteral("2017-12-24 00:00:00"),
		};
		COMPARE_STRING_VECTORS(xAxis->tickLabelStrings(), expectedStrings);

		QCOMPARE(xAxis->minorTicksAutoNumber(), true);
		QCOMPARE(xAxis->minorTicksNumber(), 1);
		QCOMPARE(xAxis->d_func()->minorTickPoints.size(), 5); // Between every major tick
	}
}

void AxisTest::dateTimeSpacingOffsetNonZero() {
	QLocale::setDefault(QLocale::C); // use . as decimal separator
	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	QVERIFY(ws != nullptr);
	project.addChild(ws);

	Spreadsheet* spreadsheetData = new Spreadsheet(QStringLiteral("data"), false);
	spreadsheetData->setColumnCount(2);
	spreadsheetData->setRowCount(3);
	project.addChild(spreadsheetData);

	auto* xCol = spreadsheetData->column(0);
	xCol->setColumnMode(AbstractColumn::ColumnMode::DateTime);
	QDateTime dt1 = QDateTime::fromString(QStringLiteral("2017-07-24T00:00:00Z"), Qt::ISODate);
	QDateTime dt2 = QDateTime::fromString(QStringLiteral("2017-11-24T12:03:00Z"), Qt::ISODate);
	QDateTime dt3 = QDateTime::fromString(QStringLiteral("2019-12-24T00:05:03Z"), Qt::ISODate);
	xCol->replaceDateTimes(-1, QVector<QDateTime>({dt1, dt2, dt3}));

	auto* yCol = spreadsheetData->column(1);
	yCol->replaceValues(-1, QVector<double>({2., 3., 4.}));

	QCOMPARE(spreadsheetData->rowCount(), 3);
	QCOMPARE(spreadsheetData->columnCount(), 2);

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	p->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axes are created
	p->setNiceExtend(false);
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
	xAxis->setMajorTicksSpacing(DateTime::createValue(0, 1, 0, 0, 0, 0, 0)); // 1 month
	QCOMPARE(xAxis->range().start(), dt1.toMSecsSinceEpoch());
	QCOMPARE(xAxis->range().end(), dt3.toMSecsSinceEpoch());
	xAxis->setMajorTicksType(Axis::TicksType::Spacing);
	// QCOMPARE(xAxis->majorTicksNumber(), 3);
	xAxis->setLabelsDateTimeFormat(QStringLiteral("yyyy-MM-dd hh:mm:ss"));
	QCOMPARE(xAxis->labelsTextType(), Axis::LabelsTextType::PositionValues);
	QCOMPARE(xAxis->majorTicksStartType(), Axis::TicksStartType::Offset);
	xAxis->setMajorTickStartOffset(DateTime::createValue(1, 2, 3, 4, 5, 6, 7));
	QVERIFY(xAxis->majorTickStartOffset() > 0);
	QCOMPARE(xAxis->majorTickStartValue(), 0);

	{
		const auto v = xAxis->tickLabelStrings();
		QStringList expectedStrings{
			QStringLiteral("2018-09-27 04:05:06"),
			QStringLiteral("2018-10-27 04:05:06"),
			QStringLiteral("2018-11-27 04:05:06"),
			QStringLiteral("2018-12-27 04:05:06"),
			QStringLiteral("2019-01-27 04:05:06"),
			QStringLiteral("2019-02-27 04:05:06"),
			QStringLiteral("2019-03-27 04:05:06"),
			QStringLiteral("2019-04-27 04:05:06"),
			QStringLiteral("2019-05-27 04:05:06"),
			QStringLiteral("2019-06-27 04:05:06"),
			QStringLiteral("2019-07-27 04:05:06"),
			QStringLiteral("2019-08-27 04:05:06"),
			QStringLiteral("2019-09-27 04:05:06"),
			QStringLiteral("2019-10-27 04:05:06"),
			QStringLiteral("2019-11-27 04:05:06"),
		};
		COMPARE_STRING_VECTORS(xAxis->tickLabelStrings(), expectedStrings);
	}
}

void AxisTest::dateTimeSpacingStartValueNonZero() {
	QLocale::setDefault(QLocale::C); // use . as decimal separator
	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	QVERIFY(ws != nullptr);
	project.addChild(ws);

	Spreadsheet* spreadsheetData = new Spreadsheet(QStringLiteral("data"), false);
	spreadsheetData->setColumnCount(2);
	spreadsheetData->setRowCount(3);
	project.addChild(spreadsheetData);

	auto* xCol = spreadsheetData->column(0);
	xCol->setColumnMode(AbstractColumn::ColumnMode::DateTime);
	QDateTime dt1 = QDateTime::fromString(QStringLiteral("2017-07-24T00:00:00Z"), Qt::ISODate);
	QDateTime dt2 = QDateTime::fromString(QStringLiteral("2017-11-24T12:03:00Z"), Qt::ISODate);
	QDateTime dt3 = QDateTime::fromString(QStringLiteral("2019-03-01T00:00:00Z"), Qt::ISODate);
	xCol->replaceDateTimes(-1, QVector<QDateTime>({dt1, dt2, dt3}));

	auto* yCol = spreadsheetData->column(1);
	yCol->replaceValues(-1, QVector<double>({2., 3., 4.}));

	QCOMPARE(spreadsheetData->rowCount(), 3);
	QCOMPARE(spreadsheetData->columnCount(), 2);

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	p->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axes are created
	p->setNiceExtend(false);
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
	xAxis->setMajorTicksSpacing(DateTime::createValue(0, 1, 0, 0, 0, 0, 0)); // 1 month
	QCOMPARE(xAxis->range().start(), dt1.toMSecsSinceEpoch());
	QCOMPARE(xAxis->range().end(), dt3.toMSecsSinceEpoch());
	xAxis->setMajorTicksType(Axis::TicksType::Spacing);
	// QCOMPARE(xAxis->majorTicksNumber(), 3);
	xAxis->setLabelsDateTimeFormat(QStringLiteral("yyyy-MM-dd hh:mm:ss"));
	QCOMPARE(xAxis->labelsTextType(), Axis::LabelsTextType::PositionValues);
	xAxis->setMajorTicksStartType(Axis::TicksStartType::Absolute);
	xAxis->setMajorTickStartValue(QDateTime::fromString(QStringLiteral("2018-09-27T16:05:06Z"), Qt::ISODate).toMSecsSinceEpoch());
	QVERIFY(xAxis->majorTickStartValue() > 0);
	{
		const auto v = xAxis->tickLabelStrings();
		QStringList expectedStrings{
			QStringLiteral("2018-09-27 16:05:06"),
			QStringLiteral("2018-10-27 16:05:06"),
			QStringLiteral("2018-11-27 16:05:06"),
			QStringLiteral("2018-12-27 16:05:06"),
			QStringLiteral("2019-01-27 16:05:06"),
			QStringLiteral("2019-02-27 16:05:06"),
		};
		COMPARE_STRING_VECTORS(xAxis->tickLabelStrings(), expectedStrings);
	}
}

void AxisTest::numeric() {
	QLocale::setDefault(QLocale::C); // use . as decimal separator
	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	QVERIFY(ws != nullptr);
	project.addChild(ws);

	Spreadsheet* spreadsheetData = new Spreadsheet(QStringLiteral("data"), false);
	spreadsheetData->setColumnCount(2);
	spreadsheetData->setRowCount(3);
	project.addChild(spreadsheetData);

	auto* xCol = spreadsheetData->column(0);
	xCol->setColumnMode(AbstractColumn::ColumnMode::Double);
	xCol->replaceValues(-1, QVector<double>({1., 2., 5.}));

	auto* yCol = spreadsheetData->column(1);
	yCol->replaceValues(-1, QVector<double>({2., 3., 4.}));

	QCOMPARE(spreadsheetData->rowCount(), 3);
	QCOMPARE(spreadsheetData->columnCount(), 2);

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	p->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axes are created
	p->setNiceExtend(false);
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
	xAxis->setMajorTicksAutoNumber(false);
	xAxis->setMajorTicksNumber(3);
	QCOMPARE(xAxis->range().start(), 1.);
	QCOMPARE(xAxis->range().end(), 5.);
	QCOMPARE(xAxis->majorTicksType(), Axis::TicksType::TotalNumber);
	QCOMPARE(xAxis->majorTicksNumber(), 3);
	QCOMPARE(xAxis->labelsTextType(), Axis::LabelsTextType::PositionValues);

	{
		const auto v = xAxis->tickLabelStrings();
		QStringList expectedStrings{
			QStringLiteral("1"),
			QStringLiteral("3"),
			QStringLiteral("5"),
		};
		COMPARE_STRING_VECTORS(xAxis->tickLabelStrings(), expectedStrings);
	}
}

void AxisTest::numericSpacing() {
	QLocale::setDefault(QLocale::C); // use . as decimal separator
	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	QVERIFY(ws != nullptr);
	project.addChild(ws);

	Spreadsheet* spreadsheetData = new Spreadsheet(QStringLiteral("data"), false);
	spreadsheetData->setColumnCount(2);
	spreadsheetData->setRowCount(3);
	project.addChild(spreadsheetData);

	auto* xCol = spreadsheetData->column(0);
	xCol->setColumnMode(AbstractColumn::ColumnMode::Double);
	xCol->replaceValues(-1, QVector<double>({1., 2., 5.}));

	auto* yCol = spreadsheetData->column(1);
	yCol->replaceValues(-1, QVector<double>({2., 3., 4.}));

	QCOMPARE(spreadsheetData->rowCount(), 3);
	QCOMPARE(spreadsheetData->columnCount(), 2);

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	p->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axes are created
	p->setNiceExtend(false);
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
	xAxis->setMajorTicksSpacing(0.5);
	QCOMPARE(xAxis->range().start(), 1.);
	QCOMPARE(xAxis->range().end(), 5.);
	xAxis->setMajorTicksType(Axis::TicksType::Spacing);
	// QCOMPARE(xAxis->majorTicksNumber(), 3);
	QCOMPARE(xAxis->labelsTextType(), Axis::LabelsTextType::PositionValues);
	QCOMPARE(xAxis->majorTicksStartType(), Axis::TicksStartType::Offset);
	QCOMPARE(xAxis->majorTickStartOffset(), 0.);
	QCOMPARE(xAxis->majorTickStartValue(), 0.);

	{
		const auto v = xAxis->tickLabelStrings();
		QStringList expectedStrings{
			QStringLiteral("1.0"),
			QStringLiteral("1.5"),
			QStringLiteral("2.0"),
			QStringLiteral("2.5"),
			QStringLiteral("3.0"),
			QStringLiteral("3.5"),
			QStringLiteral("4.0"),
			QStringLiteral("4.5"),
			QStringLiteral("5.0"),
		};
		COMPARE_STRING_VECTORS(xAxis->tickLabelStrings(), expectedStrings);
	}
}

void AxisTest::numericSpacingOffsetNonZero() {
	QLocale::setDefault(QLocale::C); // use . as decimal separator
	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	QVERIFY(ws != nullptr);
	project.addChild(ws);

	Spreadsheet* spreadsheetData = new Spreadsheet(QStringLiteral("data"), false);
	spreadsheetData->setColumnCount(2);
	spreadsheetData->setRowCount(3);
	project.addChild(spreadsheetData);

	auto* xCol = spreadsheetData->column(0);
	xCol->setColumnMode(AbstractColumn::ColumnMode::Double);
	xCol->replaceValues(-1, QVector<double>({1., 2., 5.}));

	auto* yCol = spreadsheetData->column(1);
	yCol->replaceValues(-1, QVector<double>({2., 3., 4.}));

	QCOMPARE(spreadsheetData->rowCount(), 3);
	QCOMPARE(spreadsheetData->columnCount(), 2);

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	p->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axes are created
	p->setNiceExtend(false);
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
	xAxis->setMajorTicksSpacing(0.5);
	QCOMPARE(xAxis->range().start(), 1.);
	QCOMPARE(xAxis->range().end(), 5.);
	xAxis->setMajorTicksType(Axis::TicksType::Spacing);
	// QCOMPARE(xAxis->majorTicksNumber(), 3);
	QCOMPARE(xAxis->labelsTextType(), Axis::LabelsTextType::PositionValues);
	QCOMPARE(xAxis->majorTicksStartType(), Axis::TicksStartType::Offset);
	xAxis->setMajorTickStartOffset(1.2);
	QVERIFY(xAxis->majorTickStartOffset() > 0);
	QCOMPARE(xAxis->majorTickStartValue(), 0.);

	{
		const auto v = xAxis->tickLabelStrings();
		QStringList expectedStrings{
			QStringLiteral("2.2"),
			QStringLiteral("2.7"),
			QStringLiteral("3.2"),
			QStringLiteral("3.7"),
			QStringLiteral("4.2"),
			QStringLiteral("4.7"),
		};
		COMPARE_STRING_VECTORS(xAxis->tickLabelStrings(), expectedStrings);
	}
}

void AxisTest::numericSpacingStartValueNonZero() {
	QLocale::setDefault(QLocale::C); // use . as decimal separator
	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	QVERIFY(ws != nullptr);
	project.addChild(ws);

	Spreadsheet* spreadsheetData = new Spreadsheet(QStringLiteral("data"), false);
	spreadsheetData->setColumnCount(2);
	spreadsheetData->setRowCount(3);
	project.addChild(spreadsheetData);

	auto* xCol = spreadsheetData->column(0);
	xCol->setColumnMode(AbstractColumn::ColumnMode::Double);
	xCol->replaceValues(-1, QVector<double>({1., 2., 5.}));

	auto* yCol = spreadsheetData->column(1);
	yCol->replaceValues(-1, QVector<double>({2., 3., 4.}));

	QCOMPARE(spreadsheetData->rowCount(), 3);
	QCOMPARE(spreadsheetData->columnCount(), 2);

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	p->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axes are created
	p->setNiceExtend(false);
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
	xAxis->setMajorTicksSpacing(0.7);
	QCOMPARE(xAxis->range().start(), 1.);
	QCOMPARE(xAxis->range().end(), 5.);
	xAxis->setMajorTicksType(Axis::TicksType::Spacing);
	// QCOMPARE(xAxis->majorTicksNumber(), 3);
	QCOMPARE(xAxis->labelsTextType(), Axis::LabelsTextType::PositionValues);
	xAxis->setMajorTicksStartType(Axis::TicksStartType::Absolute);
	xAxis->setMajorTickStartValue(1.7);
	QVERIFY(xAxis->majorTickStartValue() > 0);
	{
		QStringList expectedStrings{
			QStringLiteral("1.7"),
			QStringLiteral("2.4"),
			QStringLiteral("3.1"),
			QStringLiteral("3.8"),
			QStringLiteral("4.5"),
		};
		COMPARE_STRING_VECTORS(xAxis->tickLabelStrings(), expectedStrings);
	}
}

/*!
 * \brief AxisTest::customColumnNumeric
 * Test setting a custom column as major tick once with the custom column values and
 * once with another column as ticks label
 */
void AxisTest::customColumnNumeric() {
	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	QVERIFY(ws != nullptr);
	project.addChild(ws);
	ws->setPageRect(QRectF(0, 0, 300, 300));
	ws->setLayoutBottomMargin(0);
	ws->setLayoutTopMargin(0);
	ws->setLayoutRightMargin(0);
	ws->setLayoutLeftMargin(0);

	Spreadsheet* spreadsheetData = new Spreadsheet(QStringLiteral("data"), false);
	spreadsheetData->setColumnCount(2);
	spreadsheetData->setRowCount(3);
	project.addChild(spreadsheetData);
	auto* xCol = spreadsheetData->column(0);
	xCol->setColumnMode(AbstractColumn::ColumnMode::Double);
	xCol->replaceValues(-1, QVector<double>({1., 2., 5.}));
	auto* yCol = spreadsheetData->column(1);
	yCol->replaceValues(-1, QVector<double>({2., 3., 4.}));

	Spreadsheet* spreadsheetLabels = new Spreadsheet(QStringLiteral("labels"), false);
	spreadsheetLabels->setColumnCount(2);
	spreadsheetLabels->setRowCount(3);
	project.addChild(spreadsheetLabels);
	auto* posCol = spreadsheetLabels->column(0);
	posCol->setColumnMode(AbstractColumn::ColumnMode::Double);
	posCol->replaceValues(-1, QVector<double>({1.7, 2.2, 2.5}));
	auto* labelsCol = spreadsheetLabels->column(1);
	labelsCol->setColumnMode(AbstractColumn::ColumnMode::Text);
	labelsCol->replaceTexts(-1, QVector<QString>({QStringLiteral("first"), QStringLiteral("second"), QStringLiteral("third")}));

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	p->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axes are created
	p->setNiceExtend(false);
	QVERIFY(p != nullptr);
	p->setBottomPadding(0);
	p->setHorizontalPadding(0);
	p->setRightPadding(0);
	p->setVerticalPadding(0);
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
	QCOMPARE(xAxis->range().start(), 1.);
	QCOMPARE(xAxis->range().end(), 5.);
	xAxis->setMajorTicksType(Axis::TicksType::CustomColumn);
	xAxis->setMajorTicksColumn(posCol);
	QCOMPARE(xAxis->labelsTextType(), Axis::LabelsTextType::PositionValues);
	{
		const auto v = xAxis->tickLabelStrings();
		QStringList expectedStrings{
			QStringLiteral("1.7"),
			QStringLiteral("2.2"),
			QStringLiteral("2.5"),
		};
		COMPARE_STRING_VECTORS(xAxis->tickLabelStrings(), expectedStrings);
	}

	xAxis->setLabelsTextType(Axis::LabelsTextType::CustomValues);
	xAxis->setLabelsTextColumn(labelsCol);
	{
		QStringList expectedStrings{
			QStringLiteral("first"),
			QStringLiteral("second"),
			QStringLiteral("third"),
		};
		COMPARE_STRING_VECTORS(xAxis->tickLabelStrings(), expectedStrings);
	}
	QVERIFY(p->dataRect().width() > 0.);
	QVERIFY(p->dataRect().height() > 0.);
	QCOMPARE(xAxis->d_func()->majorTickPoints.size(), 3);
	VALUES_EQUAL(xAxis->d_func()->majorTickPoints.at(0).x(), p->dataRect().x() + p->dataRect().width() * (1.7 - 1.) / (5. - 1.));
	VALUES_EQUAL(xAxis->d_func()->majorTickPoints.at(1).x(), p->dataRect().x() + p->dataRect().width() * (2.2 - 1.) / (5. - 1.));
	VALUES_EQUAL(xAxis->d_func()->majorTickPoints.at(2).x(), p->dataRect().x() + p->dataRect().width() * (2.5 - 1.) / (5. - 1.));
}

/*!
 * \brief AxisTest::customColumnNumericMaxValues
 * The number of rows in custom column are higher than the maximum. Therefore
 * the number of ticks is limited
 */
void AxisTest::customColumnNumericMaxValues() {
	constexpr int rowCountCustomColumn = 1000;

	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	QVERIFY(ws != nullptr);
	project.addChild(ws);
	ws->setPageRect(QRectF(0, 0, 300, 300));
	ws->setLayoutBottomMargin(0);
	ws->setLayoutTopMargin(0);
	ws->setLayoutRightMargin(0);
	ws->setLayoutLeftMargin(0);

	Spreadsheet* spreadsheetData = new Spreadsheet(QStringLiteral("data"), false);
	spreadsheetData->setColumnCount(2);
	spreadsheetData->setRowCount(3);
	project.addChild(spreadsheetData);
	auto* xCol = spreadsheetData->column(0);
	xCol->setColumnMode(AbstractColumn::ColumnMode::Double);
	xCol->replaceValues(-1, QVector<double>({0., 1000.}));
	auto* yCol = spreadsheetData->column(1);
	yCol->replaceValues(-1, QVector<double>({0., 1000.}));

	Spreadsheet* spreadsheetLabels = new Spreadsheet(QStringLiteral("labels"), false);
	spreadsheetLabels->setColumnCount(2);
	spreadsheetLabels->setRowCount(3);
	project.addChild(spreadsheetLabels);
	auto* posCol = spreadsheetLabels->column(0);
	posCol->setColumnMode(AbstractColumn::ColumnMode::Integer);
	QVector<int> posValues;
	QVector<QString> customLabels;
	for (int i = 0; i <= rowCountCustomColumn; i++) {
		posValues.push_back(i);
		customLabels.push_back(QStringLiteral("Some text") + QString::number(i));
	}
	posCol->replaceInteger(-1, posValues);
	auto* labelsCol = spreadsheetLabels->column(1);
	labelsCol->setColumnMode(AbstractColumn::ColumnMode::Text);
	labelsCol->replaceTexts(-1, customLabels);

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	p->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axes are created
	p->setNiceExtend(false);
	QVERIFY(p != nullptr);
	p->setBottomPadding(0);
	p->setHorizontalPadding(0);
	p->setRightPadding(0);
	p->setVerticalPadding(0);
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
	QCOMPARE(xAxis->range().start(), 0.);
	QCOMPARE(xAxis->range().end(), 1000.);
	xAxis->setMajorTicksType(Axis::TicksType::CustomColumn);
	xAxis->setMajorTicksColumn(posCol);
	QCOMPARE(xAxis->majorTicksAutoNumber(), true);
	QCOMPARE(xAxis->labelsTextType(), Axis::LabelsTextType::PositionValues);
	{
		const auto v = xAxis->tickLabelStrings();
		QStringList expectedStrings;
		for (int i = 0; i <= rowCountCustomColumn; i += rowCountCustomColumn / (Axis::maxNumberMajorTicksCustomColumn() - 1))
			expectedStrings.push_back(QString::number(i));

		COMPARE_STRING_VECTORS(xAxis->tickLabelStrings(), expectedStrings);
	}

	xAxis->setLabelsTextType(Axis::LabelsTextType::CustomValues);
	xAxis->setLabelsTextColumn(labelsCol);
	{
		QStringList expectedStrings;
		for (int i = 0; i <= rowCountCustomColumn; i += rowCountCustomColumn / (Axis::maxNumberMajorTicksCustomColumn() - 1))
			expectedStrings.push_back(QStringLiteral("Some text") + QString::number(i));
		COMPARE_STRING_VECTORS(xAxis->tickLabelStrings(), expectedStrings);
	}
	QVERIFY(p->dataRect().width() > 0.);
	QVERIFY(p->dataRect().height() > 0.);
	QCOMPARE(xAxis->d_func()->majorTickPoints.size(), Axis::maxNumberMajorTicksCustomColumn());
	for (int i = 0; i < Axis::maxNumberMajorTicksCustomColumn(); i++) {
		const double xVal = ((double)i * rowCountCustomColumn / (Axis::maxNumberMajorTicksCustomColumn() - 1) - 0.) / (1000. - 0.);
		VALUES_EQUAL(xAxis->d_func()->majorTickPoints.at(i).x(), p->dataRect().x() + p->dataRect().width() * xVal);
	}
}

void AxisTest::customColumnNonMonotonicColumnValues() {
	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	QVERIFY(ws != nullptr);
	project.addChild(ws);
	ws->setPageRect(QRectF(0, 0, 300, 300));
	ws->setLayoutBottomMargin(0);
	ws->setLayoutTopMargin(0);
	ws->setLayoutRightMargin(0);
	ws->setLayoutLeftMargin(0);

	Spreadsheet* spreadsheetData = new Spreadsheet(QStringLiteral("data"), false);
	spreadsheetData->setColumnCount(2);
	spreadsheetData->setRowCount(3);
	project.addChild(spreadsheetData);
	auto* xCol = spreadsheetData->column(0);
	xCol->setColumnMode(AbstractColumn::ColumnMode::Double);
	xCol->replaceValues(-1, QVector<double>({0., 1000.}));
	auto* yCol = spreadsheetData->column(1);
	yCol->replaceValues(-1, QVector<double>({0., 1000.}));

	Spreadsheet* spreadsheetLabels = new Spreadsheet(QStringLiteral("labels"), false);
	spreadsheetLabels->setColumnCount(2);
	spreadsheetLabels->setRowCount(3);
	project.addChild(spreadsheetLabels);
	auto* posCol = spreadsheetLabels->column(0);
	posCol->setColumnMode(AbstractColumn::ColumnMode::Double);
	QVector<double> posValues({
		0., 5., 3., 800., 500., 300., 200., 1., 6., 2., 900., 787., 333., 128., 999., 650., 11., 14., 18., 20., 576., 238., 239.,
	});
	for (int i = 100; i < 200; i++)
		posValues.push_back(i); // Add more posValues which are between 100 and 200

	posCol->replaceValues(-1, posValues);
	QVERIFY(posCol->rowCount() > Axis::maxNumberMajorTicksCustomColumn());

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	p->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axes are created
	p->setNiceExtend(false);
	QVERIFY(p != nullptr);
	p->setBottomPadding(0);
	p->setHorizontalPadding(0);
	p->setRightPadding(0);
	p->setVerticalPadding(0);
	ws->addChild(p);

	auto* curve = new XYCurve(QStringLiteral("xy-curve"));
	curve->setXColumn(xCol);
	curve->setYColumn(yCol);
	p->addChild(curve);
	p->enableAutoScale(Dimension::X, 0, false);
	auto r = p->range(Dimension::X, 0);
	r.setStart(100.);
	r.setEnd(200.);
	p->setRange(Dimension::X, 0, r);

	auto axes = p->children<Axis>();
	QCOMPARE(axes.count(), 2);
	QCOMPARE(axes.at(0)->name(), QStringLiteral("x"));
	QCOMPARE(axes.at(1)->name(), QStringLiteral("y"));

	auto* xAxis = static_cast<Axis*>(axes.at(0));
	QCOMPARE(xAxis->range().start(), 100.);
	QCOMPARE(xAxis->range().end(), 200.);
	xAxis->setMajorTicksType(Axis::TicksType::CustomColumn);
	xAxis->setMajorTicksColumn(posCol);
	QCOMPARE(xAxis->majorTicksAutoNumber(), true);
	QCOMPARE(xAxis->labelsTextType(), Axis::LabelsTextType::PositionValues);
	{
		const auto v = xAxis->tickLabelStrings();
		QStringList expectedStrings;
		for (int i = 100; i <= 200.; i += (200. - 100.) / (Axis::maxNumberMajorTicksCustomColumn() - 1))
			expectedStrings.push_back(QString::number(i));

		COMPARE_STRING_VECTORS(xAxis->tickLabelStrings(), expectedStrings);
	}
}

/*!
 * \brief AxisTest::customColumnNumericMaxValuesLimitedRange
 * The range is limited to 100-200 (max Range: 0-1000). But still 20 ticks shall be visible
 */
void AxisTest::customColumnNumericMaxValuesLimitedRange() {
	constexpr int rowCountCustomColumn = 1000;

	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	QVERIFY(ws != nullptr);
	project.addChild(ws);
	ws->setPageRect(QRectF(0, 0, 300, 300));
	ws->setLayoutBottomMargin(0);
	ws->setLayoutTopMargin(0);
	ws->setLayoutRightMargin(0);
	ws->setLayoutLeftMargin(0);

	Spreadsheet* spreadsheetData = new Spreadsheet(QStringLiteral("data"), false);
	spreadsheetData->setColumnCount(2);
	spreadsheetData->setRowCount(3);
	project.addChild(spreadsheetData);
	auto* xCol = spreadsheetData->column(0);
	xCol->setColumnMode(AbstractColumn::ColumnMode::Double);
	xCol->replaceValues(-1, QVector<double>({0., 1000.}));
	auto* yCol = spreadsheetData->column(1);
	yCol->replaceValues(-1, QVector<double>({0., 1000.}));

	Spreadsheet* spreadsheetLabels = new Spreadsheet(QStringLiteral("labels"), false);
	spreadsheetLabels->setColumnCount(2);
	spreadsheetLabels->setRowCount(3);
	project.addChild(spreadsheetLabels);
	auto* posCol = spreadsheetLabels->column(0);
	posCol->setColumnMode(AbstractColumn::ColumnMode::Integer);
	QVector<int> posValues;
	QVector<QString> customLabels;
	for (int i = 0; i < rowCountCustomColumn; i++) {
		posValues.push_back(i);
		customLabels.push_back(QStringLiteral("Some text") + QString::number(i));
	}
	posCol->replaceInteger(-1, posValues);
	auto* labelsCol = spreadsheetLabels->column(1);
	labelsCol->setColumnMode(AbstractColumn::ColumnMode::Text);
	labelsCol->replaceTexts(-1, customLabels);

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	p->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axes are created
	p->setNiceExtend(false);
	QVERIFY(p != nullptr);
	p->setBottomPadding(0);
	p->setHorizontalPadding(0);
	p->setRightPadding(0);
	p->setVerticalPadding(0);
	ws->addChild(p);

	auto* curve = new XYCurve(QStringLiteral("xy-curve"));
	curve->setXColumn(xCol);
	curve->setYColumn(yCol);
	p->addChild(curve);
	p->enableAutoScale(Dimension::X, 0, false);
	auto r = p->range(Dimension::X, 0);
	r.setStart(100.);
	r.setEnd(200.);
	p->setRange(Dimension::X, 0, r);

	auto axes = p->children<Axis>();
	QCOMPARE(axes.count(), 2);
	QCOMPARE(axes.at(0)->name(), QStringLiteral("x"));
	QCOMPARE(axes.at(1)->name(), QStringLiteral("y"));

	auto* xAxis = static_cast<Axis*>(axes.at(0));
	QCOMPARE(xAxis->range().start(), 100.);
	QCOMPARE(xAxis->range().end(), 200.);
	xAxis->setMajorTicksType(Axis::TicksType::CustomColumn);
	xAxis->setMajorTicksColumn(posCol);
	QCOMPARE(xAxis->majorTicksAutoNumber(), true);
	QCOMPARE(xAxis->labelsTextType(), Axis::LabelsTextType::PositionValues);
	{
		QStringList expectedStrings;
		for (int i = 100; i <= 200.; i += (200. - 100.) / (Axis::maxNumberMajorTicksCustomColumn() - 1))
			expectedStrings.push_back(QString::number(i));

		COMPARE_STRING_VECTORS(xAxis->tickLabelStrings(), expectedStrings);
	}

	xAxis->setLabelsTextType(Axis::LabelsTextType::CustomValues);
	xAxis->setLabelsTextColumn(labelsCol);
	{
		QStringList expectedStrings;
		for (int i = 100; i <= 200.; i += (200. - 100.) / (Axis::maxNumberMajorTicksCustomColumn() - 1))
			expectedStrings.push_back(QStringLiteral("Some text") + QString::number(i));
		COMPARE_STRING_VECTORS(xAxis->tickLabelStrings(), expectedStrings);
	}
	QVERIFY(p->dataRect().width() > 0.);
	QVERIFY(p->dataRect().height() > 0.);
	QCOMPARE(xAxis->d_func()->majorTickPoints.size(), Axis::maxNumberMajorTicksCustomColumn());
	for (int i = 0; i < Axis::maxNumberMajorTicksCustomColumn(); i++) {
		const double xValExpected = (100. + (double)i * (200. - 100.) / (Axis::maxNumberMajorTicksCustomColumn() - 1));
		const double posExpected = p->dataRect().x() + p->dataRect().width() * (xValExpected - 100.) / (200. - 100.);
		const double pos = xAxis->d_func()->majorTickPoints.at(i).x();
		VALUES_EQUAL(pos, posExpected);
	}

	r = p->range(Dimension::X, 0);
	r.setStart(100.);
	r.setEnd(110.);
	p->setRange(Dimension::X, 0, r);

	{
		QStringList expectedStrings;
		for (int i = 100; i <= 110.; i += (110. - 100.) / 10) // maximum 10 labels are visible because not more labels exist in this range
			expectedStrings.push_back(QStringLiteral("Some text") + QString::number(i));
		COMPARE_STRING_VECTORS(xAxis->tickLabelStrings(), expectedStrings);
	}
}

/*!
 * \brief AxisTest::customColumnNumericMaxValuesLimitedRangeNotCompleteRange
 * Same as customColumnNumericMaxValuesLimitedRange() but in this case the range starts from -100, but the labels will start from 100
 */
void AxisTest::customColumnNumericMaxValuesLimitedRangeNotCompleteRange() {
	constexpr int rowCountCustomColumn = 1000;

	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	QVERIFY(ws != nullptr);
	project.addChild(ws);
	ws->setPageRect(QRectF(0, 0, 300, 300));
	ws->setLayoutBottomMargin(0);
	ws->setLayoutTopMargin(0);
	ws->setLayoutRightMargin(0);
	ws->setLayoutLeftMargin(0);

	Spreadsheet* spreadsheetData = new Spreadsheet(QStringLiteral("data"), false);
	spreadsheetData->setColumnCount(2);
	spreadsheetData->setRowCount(3);
	project.addChild(spreadsheetData);
	auto* xCol = spreadsheetData->column(0);
	xCol->setColumnMode(AbstractColumn::ColumnMode::Double);
	xCol->replaceValues(-1, QVector<double>({0., 1000.}));
	auto* yCol = spreadsheetData->column(1);
	yCol->replaceValues(-1, QVector<double>({0., 1000.}));

	Spreadsheet* spreadsheetLabels = new Spreadsheet(QStringLiteral("labels"), false);
	spreadsheetLabels->setColumnCount(2);
	spreadsheetLabels->setRowCount(3);
	project.addChild(spreadsheetLabels);
	auto* posCol = spreadsheetLabels->column(0);
	posCol->setColumnMode(AbstractColumn::ColumnMode::Integer);
	QVector<int> posValues;
	QVector<QString> customLabels;
	for (int i = 0; i < rowCountCustomColumn; i++) {
		posValues.push_back(i);
		customLabels.push_back(QStringLiteral("Some text") + QString::number(i));
	}
	posCol->replaceInteger(-1, posValues);
	auto* labelsCol = spreadsheetLabels->column(1);
	labelsCol->setColumnMode(AbstractColumn::ColumnMode::Text);
	labelsCol->replaceTexts(-1, customLabels);

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	p->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axes are created
	p->setNiceExtend(false);
	QVERIFY(p != nullptr);
	p->setBottomPadding(0);
	p->setHorizontalPadding(0);
	p->setRightPadding(0);
	p->setVerticalPadding(0);
	ws->addChild(p);

	auto* curve = new XYCurve(QStringLiteral("xy-curve"));
	curve->setXColumn(xCol);
	curve->setYColumn(yCol);
	p->addChild(curve);
	p->enableAutoScale(Dimension::X, 0, false);
	auto r = p->range(Dimension::X, 0);
	r.setStart(-100);
	r.setEnd(200.);
	p->setRange(Dimension::X, 0, r);

	auto axes = p->children<Axis>();
	QCOMPARE(axes.count(), 2);
	QCOMPARE(axes.at(0)->name(), QStringLiteral("x"));
	QCOMPARE(axes.at(1)->name(), QStringLiteral("y"));

	auto* xAxis = static_cast<Axis*>(axes.at(0));
	QCOMPARE(xAxis->range().start(), -100.);
	QCOMPARE(xAxis->range().end(), 200.);
	xAxis->setMajorTicksType(Axis::TicksType::CustomColumn);
	xAxis->setMajorTicksColumn(posCol);
	QCOMPARE(xAxis->majorTicksAutoNumber(), true);
	QCOMPARE(xAxis->labelsTextType(), Axis::LabelsTextType::PositionValues);

	// |          |          |          |
	//-100        0         100        200
	//            ^
	//       labels begin here.

	{
		const auto v = xAxis->tickLabelStrings();
		QStringList expectedStrings;
		for (int i = 0; i <= 200.; i += (200. - 0.) / (Axis::maxNumberMajorTicksCustomColumn() - 1))
			expectedStrings.push_back(QString::number(i));

		COMPARE_STRING_VECTORS(xAxis->tickLabelStrings(), expectedStrings);
	}

	xAxis->setLabelsTextType(Axis::LabelsTextType::CustomValues);
	xAxis->setLabelsTextColumn(labelsCol);
	{
		QStringList expectedStrings;
		for (int i = 0; i <= 200.; i += (200. - 0.) / (Axis::maxNumberMajorTicksCustomColumn() - 1))
			expectedStrings.push_back(QStringLiteral("Some text") + QString::number(i));
		COMPARE_STRING_VECTORS(xAxis->tickLabelStrings(), expectedStrings);
	}
	QVERIFY(p->dataRect().width() > 0.);
	QVERIFY(p->dataRect().height() > 0.);
	QCOMPARE(xAxis->d_func()->majorTickPoints.size(), Axis::maxNumberMajorTicksCustomColumn());
	for (int i = 0; i < Axis::maxNumberMajorTicksCustomColumn(); i++) {
		const double xValExpected = (0. + (double)i * (200. - 0.) / (Axis::maxNumberMajorTicksCustomColumn() - 1));
		const double posExpected = p->dataRect().x() + p->dataRect().width() * (xValExpected - (-100.)) / (200. - (-100.));
		const double pos = xAxis->d_func()->majorTickPoints.at(i).x();
		VALUES_EQUAL(pos, posExpected);
	}
}

/*!
 * \brief AxisTest::customColumnDateTime
 * Test setting a custom column as major tick once with the custom column values and
 * once with another column as ticks label
 */
void AxisTest::customColumnDateTime() {
	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	QVERIFY(ws != nullptr);
	project.addChild(ws);
	ws->setPageRect(QRectF(0, 0, 300, 300));
	ws->setLayoutBottomMargin(0);
	ws->setLayoutTopMargin(0);
	ws->setLayoutRightMargin(0);
	ws->setLayoutLeftMargin(0);

	Spreadsheet* spreadsheetData = new Spreadsheet(QStringLiteral("data"), false);
	spreadsheetData->setColumnCount(2);
	spreadsheetData->setRowCount(3);
	project.addChild(spreadsheetData);
	auto* xCol = spreadsheetData->column(0);
	xCol->setColumnMode(AbstractColumn::ColumnMode::DateTime);
	QDateTime dt1 = QDateTime::fromString(QStringLiteral("2017-07-24T00:00:00Z"), Qt::ISODate);
	QDateTime dt2 = QDateTime::fromString(QStringLiteral("2017-07-25T00:00:00Z"), Qt::ISODate);
	QDateTime dt3 = QDateTime::fromString(QStringLiteral("2019-07-26T00:00:00Z"), Qt::ISODate);
	xCol->replaceDateTimes(-1, QVector<QDateTime>({dt1, dt2, dt3}));
	auto* yCol = spreadsheetData->column(1);
	yCol->replaceValues(-1, QVector<double>({2., 3., 4.}));

	Spreadsheet* spreadsheetLabels = new Spreadsheet(QStringLiteral("labels"), false);
	spreadsheetLabels->setColumnCount(2);
	spreadsheetLabels->setRowCount(3);
	project.addChild(spreadsheetLabels);
	auto* posCol = spreadsheetLabels->column(0);
	posCol->setColumnMode(AbstractColumn::ColumnMode::DateTime);
	QDateTime dt1Label = QDateTime::fromString(QStringLiteral("2017-07-24T11:03:02Z"), Qt::ISODate);
	QDateTime dt2Label = QDateTime::fromString(QStringLiteral("2017-07-24T15:30:00Z"), Qt::ISODate);
	QDateTime dt3Label = QDateTime::fromString(QStringLiteral("2019-07-25T13:25:00Z"), Qt::ISODate);
	posCol->replaceDateTimes(-1, QVector<QDateTime>({dt1Label, dt2Label, dt3Label}));
	auto* labelsCol = spreadsheetLabels->column(1);
	labelsCol->setColumnMode(AbstractColumn::ColumnMode::Text);
	labelsCol->replaceTexts(-1, QVector<QString>({QStringLiteral("first"), QStringLiteral("second"), QStringLiteral("third")}));

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	p->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axes are created
	p->setNiceExtend(false);
	QVERIFY(p != nullptr);
	p->setBottomPadding(0);
	p->setHorizontalPadding(0);
	p->setRightPadding(0);
	p->setVerticalPadding(0);
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
	QCOMPARE(xAxis->range().start(), dt1.toMSecsSinceEpoch());
	QCOMPARE(xAxis->range().end(), dt3.toMSecsSinceEpoch());
	xAxis->setMajorTicksType(Axis::TicksType::CustomColumn);
	xAxis->setMajorTicksColumn(posCol);

	QCOMPARE(xAxis->labelsDateTimeFormat(), QStringLiteral("yyyy-MM-dd hh:mm:ss.zzz"));
	QCOMPARE(xAxis->labelsTextType(), Axis::LabelsTextType::PositionValues);
	{
		QStringList expectedStrings{
			QStringLiteral("2017-07-24 11:03:02.000"),
			QStringLiteral("2017-07-24 15:30:00.000"),
			QStringLiteral("2019-07-25 13:25:00.000"),
		};
		COMPARE_STRING_VECTORS(xAxis->tickLabelStrings(), expectedStrings);
	}

	xAxis->setLabelsTextType(Axis::LabelsTextType::CustomValues);
	xAxis->setLabelsTextColumn(labelsCol);
	{
		QStringList expectedStrings{
			QStringLiteral("first"),
			QStringLiteral("second"),
			QStringLiteral("third"),
		};
		COMPARE_STRING_VECTORS(xAxis->tickLabelStrings(), expectedStrings);
	}

	QVERIFY(p->dataRect().width() > 0.);
	QVERIFY(p->dataRect().height() > 0.);
	QCOMPARE(xAxis->d_func()->majorTickPoints.size(), 3);
	const auto span = dt3.toMSecsSinceEpoch() - dt1.toMSecsSinceEpoch();
	VALUES_EQUAL(xAxis->d_func()->majorTickPoints.at(0).x(),
				 p->dataRect().x() + p->dataRect().width() * (dt1Label.toMSecsSinceEpoch() - dt1.toMSecsSinceEpoch()) / span);
	VALUES_EQUAL(xAxis->d_func()->majorTickPoints.at(1).x(),
				 p->dataRect().x() + p->dataRect().width() * (dt2Label.toMSecsSinceEpoch() - dt1.toMSecsSinceEpoch()) / span);
	VALUES_EQUAL(xAxis->d_func()->majorTickPoints.at(2).x(),
				 p->dataRect().x() + p->dataRect().width() * (dt3Label.toMSecsSinceEpoch() - dt1.toMSecsSinceEpoch()) / span);
}

void AxisTest::autoScaleLog10() {
	QLocale::setDefault(QLocale::C); // use . as decimal separator
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
	QCOMPARE(axes.at(0)->name(), QStringLiteral("x"));
	QCOMPARE(axes.at(1)->name(), QStringLiteral("y"));
	auto* xAxis = static_cast<Axis*>(axes.at(0));
	xAxis->setMajorTicksNumber(4);
	QCOMPARE(xAxis->scale(), RangeT::Scale::Linear);
	QCOMPARE(xAxis->rangeScale(), true);

	auto range = p->range(Dimension::X, 0);
	range.setStart(10);
	range.setEnd(10000);
	p->setNiceExtend(false);
	p->setRange(Dimension::X, 0, range);

	{
		QStringList expectedStrings{
			QStringLiteral("2000"),
			QStringLiteral("4000"),
			QStringLiteral("6000"),
			QStringLiteral("8000"),
			QStringLiteral("10000"),
		};
		COMPARE_STRING_VECTORS(xAxis->tickLabelStrings(), expectedStrings);
	}

	p->enableAutoScale(Dimension::X, 0, false, true);
	range = p->range(Dimension::X, 0);
	range.setScale(RangeT::Scale::Log10);
	xAxis->setMajorTicksNumber(3);
	p->setRange(Dimension::X, 0, range);

	QCOMPARE(xAxis->range(), range);
	QCOMPARE(xAxis->scale(), RangeT::Scale::Log10);

	{
		QStringList expectedStrings{
			QStringLiteral("10"),
			QStringLiteral("100"),
			QStringLiteral("1000"),
			QStringLiteral("10000"),
		};
		COMPARE_STRING_VECTORS(xAxis->tickLabelStrings(), expectedStrings);
	}
}

void AxisTest::autoScaleLog102() {
	QLocale::setDefault(QLocale::C); // use . as decimal separator
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
	QCOMPARE(axes.at(0)->name(), QStringLiteral("x"));
	QCOMPARE(axes.at(1)->name(), QStringLiteral("y"));
	auto* xAxis = static_cast<Axis*>(axes.at(0));
	xAxis->setMajorTicksNumber(4);
	QCOMPARE(xAxis->scale(), RangeT::Scale::Linear);
	QCOMPARE(xAxis->rangeScale(), true);
	xAxis->setLabelsAutoPrecision(false);
	xAxis->setLabelsPrecision(2);

	auto range = p->range(Dimension::X, 0);
	range.setStart(0);
	range.setEnd(1);
	p->setRange(Dimension::X, 0, range);
	p->setNiceExtend(false);
	p->enableAutoScale(Dimension::X, 0, false, true);
	p->setRangeScale(Dimension::X, 0, RangeT::Scale::Log10); // use different method

	QCOMPARE(xAxis->range(), p->range(Dimension::X, 0));
	QCOMPARE(xAxis->scale(), RangeT::Scale::Log10);

	VALUES_EQUAL(xAxis->range().start(), 0.01);
	QCOMPARE(xAxis->range().end(), 1.0);

	QStringList expectedStrings{
		QStringLiteral("0.01"),
		QStringLiteral("0.03"),
		QStringLiteral("0.10"),
		QStringLiteral("0.32"),
		QStringLiteral("1.00"),
	};
	COMPARE_STRING_VECTORS(xAxis->tickLabelStrings(), expectedStrings);
}

void AxisTest::autoScaleLog102Vertical() {
	QLocale::setDefault(QLocale::C); // use . as decimal separator
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
	QCOMPARE(axes.at(0)->name(), QStringLiteral("x"));
	QCOMPARE(axes.at(1)->name(), QStringLiteral("y"));
	auto* yAxis = static_cast<Axis*>(axes.at(1));
	yAxis->setMajorTicksNumber(4);
	QCOMPARE(yAxis->scale(), RangeT::Scale::Linear);
	QCOMPARE(yAxis->rangeScale(), true);
	yAxis->setLabelsAutoPrecision(false);
	yAxis->setLabelsPrecision(2);

	auto range = p->range(Dimension::Y, 0);
	range.setStart(0);
	range.setEnd(1);
	p->setRange(Dimension::Y, 0, range);
	p->setNiceExtend(false);
	p->enableAutoScale(Dimension::Y, 0, false, true);
	p->setRangeScale(Dimension::Y, 0, RangeT::Scale::Log10); // use different method

	QCOMPARE(yAxis->range(), p->range(Dimension::Y, 0));
	QCOMPARE(yAxis->scale(), RangeT::Scale::Log10);

	VALUES_EQUAL(yAxis->range().start(), 0.01);
	QCOMPARE(yAxis->range().end(), 1.0);

	QStringList expectedStrings{
		QStringLiteral("0.01"),
		QStringLiteral("0.03"),
		QStringLiteral("0.10"),
		QStringLiteral("0.32"),
		QStringLiteral("1.00"),
	};
	COMPARE_STRING_VECTORS(yAxis->tickLabelStrings(), expectedStrings);
}

void AxisTest::colorBar() {
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

	auto* xAxis = static_cast<Axis*>(axes.at(0));
	QCOMPARE(xAxis->colorBar(), false);

	int counter = 0;
	connect(xAxis, &Axis::trackRetransformCalled, [this, &counter](bool suppress) {
		QCOMPARE(suppress, false);
		counter++;
	});
	QCOMPARE(xAxis->setColorBar(true));
	QCOMPARE(counter, 1);

	// Nothing happens, because no Heatmap is set
	auto* hm = new Heatmap("Heatmap");
	format.min = 0;
	format.max = 100;
	hm->setFormat(format);
	xAxis->setHeatmap(hm);
	QCOMPARE(counter, 2);

	{
		const auto v = xAxis->tickLabelStrings();
		QStringList expectedStrings{
			QStringLiteral("0"),
			QStringLiteral("50"),
			QStringLiteral("100"),
		};
		COMPARE_STRING_VECTORS(xAxis->tickLabelStrings(), expectedStrings);
	}

	// Check that color bar is drawn
}

QTEST_MAIN(AxisTest)
