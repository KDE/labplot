/*
	File                 : AxisTest3.cpp
	Project              : LabPlot
	Description          : Second tests for Axis
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022 Martin Marmsoler <martin.marmsoler@gmail.com>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "AxisTest3.h"
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

void AxisTest3::dateTime() {
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
	p->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created
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

void AxisTest3::dateTimeSpacing() {
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
	p->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created
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

void AxisTest3::dateTimeSpacingOffsetNonZero() {
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
	p->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created
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

void AxisTest3::dateTimeSpacingStartValueNonZero() {
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
	p->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created
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

void AxisTest3::numeric() {
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
	xCol->setColumnMode(AbstractColumn::ColumnMode::Double);
	xCol->replaceValues(-1, QVector<double>({1., 2., 5.}));

	auto* yCol = spreadsheetData->column(1);
	yCol->replaceValues(-1, QVector<double>({2., 3., 4.}));

	QCOMPARE(spreadsheetData->rowCount(), 3);
	QCOMPARE(spreadsheetData->columnCount(), 2);

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	p->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created
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

void AxisTest3::numericSpacing() {
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
	xCol->setColumnMode(AbstractColumn::ColumnMode::Double);
	xCol->replaceValues(-1, QVector<double>({1., 2., 5.}));

	auto* yCol = spreadsheetData->column(1);
	yCol->replaceValues(-1, QVector<double>({2., 3., 4.}));

	QCOMPARE(spreadsheetData->rowCount(), 3);
	QCOMPARE(spreadsheetData->columnCount(), 2);

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	p->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created
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

void AxisTest3::numericSpacingOffsetNonZero() {
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
	xCol->setColumnMode(AbstractColumn::ColumnMode::Double);
	xCol->replaceValues(-1, QVector<double>({1., 2., 5.}));

	auto* yCol = spreadsheetData->column(1);
	yCol->replaceValues(-1, QVector<double>({2., 3., 4.}));

	QCOMPARE(spreadsheetData->rowCount(), 3);
	QCOMPARE(spreadsheetData->columnCount(), 2);

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	p->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created
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

void AxisTest3::numericSpacingStartValueNonZero() {
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
	xCol->setColumnMode(AbstractColumn::ColumnMode::Double);
	xCol->replaceValues(-1, QVector<double>({1., 2., 5.}));

	auto* yCol = spreadsheetData->column(1);
	yCol->replaceValues(-1, QVector<double>({2., 3., 4.}));

	QCOMPARE(spreadsheetData->rowCount(), 3);
	QCOMPARE(spreadsheetData->columnCount(), 2);

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	p->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created
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
 * \brief AxisTest3::customColumnNumeric
 * Test setting a custom column as major tick once with the custom column values and
 * once with another column as ticks label
 */
void AxisTest3::customColumnNumeric() {
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
	p->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created
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
 * \brief AxisTest3::customColumnNumericMaxValues
 * The number of rows in custom column are higher than the maximum. Therefore
 * the number of ticks is limited
 */
void AxisTest3::customColumnNumericMaxValues() {
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
	p->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created
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

void AxisTest3::customColumnNonMonotonicColumnValues() {
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
	p->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created
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
 * \brief AxisTest3::customColumnNumericMaxValuesLimitedRange
 * The range is limited to 100-200 (max Range: 0-1000). But still 20 ticks shall be visible
 */
void AxisTest3::customColumnNumericMaxValuesLimitedRange() {
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
	p->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created
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
 * \brief AxisTest3::customColumnNumericMaxValuesLimitedRangeNotCompleteRange
 * Same as customColumnNumericMaxValuesLimitedRange() but in this case the range starts from -100, but the labels will start from 100
 */
void AxisTest3::customColumnNumericMaxValuesLimitedRangeNotCompleteRange() {
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
	p->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created
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
 * \brief AxisTest3::customColumnDateTime
 * Test setting a custom column as major tick once with the custom column values and
 * once with another column as ticks label
 */
void AxisTest3::customColumnDateTime() {
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
	p->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created
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

void AxisTest3::autoScaleLog10() {
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

void AxisTest3::autoScaleLog102() {
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

	{
		const auto s = xAxis->tickLabelStrings();
		QStringList expectedStrings{
			QStringLiteral("0.01"),
			QStringLiteral("0.03"),
			QStringLiteral("0.10"),
			QStringLiteral("0.32"),
			QStringLiteral("1.00"),
		};
		for (const auto& label: xAxis->tickLabelStrings()) {
			std::cout << label.toStdString() << ",";
		}
		std::cout << std::end
		COMPARE_STRING_VECTORS(xAxis->tickLabelStrings(), expectedStrings);
	}
}

QTEST_MAIN(AxisTest3)
