/*
	File                 : BarPlotTest.cpp
	Project              : LabPlot
	Description          : Tests for BarPlot
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022-2023 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "BarPlotTest.h"
#include "backend/core/Project.h"
#include "backend/core/column/Column.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/BarPlot.h"
#include "backend/worksheet/plots/cartesian/LollipopPlot.h"

/*!
 * \brief one dataset, grouped
 */
void BarPlotTest::testRange01() {
	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	project.addChild(ws);

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	ws->addChild(p);

	auto* barPlot = new BarPlot(QStringLiteral("barplot"));
	p->addChild(barPlot);

	// set the data
	QVector<const AbstractColumn*> dataColumns;
	auto* c = new Column(QStringLiteral("data"));
	c->setValueAt(0, 3.);
	c->setValueAt(1, 6.);
	c->setValueAt(2, 9.);
	c->setValueAt(3, 12.);
	dataColumns << c;

	barPlot->setDataColumns(dataColumns);

	// check the min and max range values
	QCOMPARE(barPlot->minimum(Dimension::X), 0.0);
	QCOMPARE(barPlot->maximum(Dimension::X), 4.0);
	QCOMPARE(barPlot->minimum(Dimension::Y), 0.);
	QCOMPARE(barPlot->maximum(Dimension::Y), 12.);
}

/*!
 * \brief one dataset, grouped, with a negative value
 */
void BarPlotTest::testRange02() {
	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	project.addChild(ws);

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	ws->addChild(p);

	auto* barPlot = new BarPlot(QStringLiteral("barplot"));
	p->addChild(barPlot);

	// set the data
	QVector<const AbstractColumn*> dataColumns;
	auto* c = new Column(QStringLiteral("data"));
	c->setValueAt(0, 3.);
	c->setValueAt(1, -6.);
	c->setValueAt(2, 9.);
	c->setValueAt(3, 12.);
	dataColumns << c;

	barPlot->setDataColumns(dataColumns);

	// check the min and max range values
	QCOMPARE(barPlot->minimum(Dimension::X), 0.0);
	QCOMPARE(barPlot->maximum(Dimension::X), 4.0);
	QCOMPARE(barPlot->minimum(Dimension::Y), -6.);
	QCOMPARE(barPlot->maximum(Dimension::Y), 12.);
}

/*!
 * \brief two datasets, stacked
 */
void BarPlotTest::testRange03() {
	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	project.addChild(ws);

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	ws->addChild(p);

	auto* barPlot = new BarPlot(QStringLiteral("barplot"));
	barPlot->setType(BarPlot::Type::Stacked);
	p->addChild(barPlot);

	// set the data
	QVector<const AbstractColumn*> dataColumns;

	auto* c1 = new Column(QStringLiteral("data1"));
	c1->setValueAt(0, 3.);
	c1->setValueAt(1, 6.);
	c1->setValueAt(2, 9.);
	c1->setValueAt(3, 12.);
	dataColumns << c1;

	auto* c2 = new Column(QStringLiteral("data2"));
	c2->setValueAt(0, 2.);
	c2->setValueAt(1, 5.);
	c2->setValueAt(2, 8.);
	c2->setValueAt(3, 11.);
	dataColumns << c2;

	barPlot->setDataColumns(dataColumns);

	// check the min and max range values
	QCOMPARE(barPlot->minimum(Dimension::X), 0.);
	QCOMPARE(barPlot->maximum(Dimension::X), 4.);
	QCOMPARE(barPlot->minimum(Dimension::Y), 0.);
	QCOMPARE(barPlot->maximum(Dimension::Y), 23.);
}

/*!
 * \brief two datasets, stacked, with a negative value
 */
void BarPlotTest::testRange04() {
	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	project.addChild(ws);

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	ws->addChild(p);

	auto* barPlot = new BarPlot(QStringLiteral("barplot"));
	barPlot->setType(BarPlot::Type::Stacked);
	p->addChild(barPlot);

	// set the data
	QVector<const AbstractColumn*> dataColumns;

	auto* c1 = new Column(QStringLiteral("data1"));
	c1->setValueAt(0, 3.);
	c1->setValueAt(1, 6.);
	c1->setValueAt(2, 9.);
	c1->setValueAt(3, 12.);
	dataColumns << c1;

	auto* c2 = new Column(QStringLiteral("data2"));
	c2->setValueAt(0, 2.);
	c2->setValueAt(1, 5.);
	c2->setValueAt(2, 8.);
	c2->setValueAt(3, -11.);
	dataColumns << c2;

	barPlot->setDataColumns(dataColumns);

	// check the min and max range values
	QCOMPARE(barPlot->minimum(Dimension::X), 0.0);
	QCOMPARE(barPlot->maximum(Dimension::X), 4.0);
	QCOMPARE(barPlot->minimum(Dimension::Y), -11.);
	QCOMPARE(barPlot->maximum(Dimension::Y), 17.);
}

/*!
 * \brief two datasets, stacked 100%
 */
void BarPlotTest::testRange05() {
	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	project.addChild(ws);

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	ws->addChild(p);

	auto* barPlot = new BarPlot(QStringLiteral("barplot"));
	barPlot->setType(BarPlot::Type::Stacked_100_Percent);
	p->addChild(barPlot);

	// set the data
	QVector<const AbstractColumn*> dataColumns;

	auto* c1 = new Column(QStringLiteral("data1"));
	c1->setValueAt(0, 3.);
	c1->setValueAt(1, 6.);
	c1->setValueAt(2, 9.);
	c1->setValueAt(3, 12.);
	dataColumns << c1;

	auto* c2 = new Column(QStringLiteral("data2"));
	c2->setValueAt(0, 2.);
	c2->setValueAt(1, 5.);
	c2->setValueAt(2, 8.);
	c2->setValueAt(3, 11.);
	dataColumns << c2;

	barPlot->setDataColumns(dataColumns);

	// check the min and max range values
	QCOMPARE(barPlot->minimum(Dimension::X), 0.);
	QCOMPARE(barPlot->maximum(Dimension::X), 4.);
	QCOMPARE(barPlot->minimum(Dimension::Y), 0.);
	QCOMPARE(barPlot->maximum(Dimension::Y), 100.);
}

/*!
 * \brief lollipop plot with one dataset, horizontal
 */
void BarPlotTest::testRangeLollipopPlot01() {
	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	project.addChild(ws);

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	ws->addChild(p);

	auto* barPlot = new LollipopPlot(QStringLiteral("barplot"));
	p->addChild(barPlot);

	// set the data
	QVector<const AbstractColumn*> dataColumns;
	auto* c = new Column(QStringLiteral("data"));
	c->setValueAt(0, 3.);
	c->setValueAt(1, 6.);
	c->setValueAt(2, 9.);
	c->setValueAt(3, 12.);
	dataColumns << c;

	barPlot->setDataColumns(dataColumns);

	// check the min and max range values
	QCOMPARE(barPlot->minimum(Dimension::X), 0.0);
	QCOMPARE(barPlot->maximum(Dimension::X), 4.0);
	QCOMPARE(barPlot->minimum(Dimension::Y), 0.);
	QCOMPARE(barPlot->maximum(Dimension::Y), 12.);
}

/*!
 * \brief lollipop plot with one dataset, vertical
 */
void BarPlotTest::testRangeLollipopPlot02() {
	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	project.addChild(ws);

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	ws->addChild(p);

	auto* barPlot = new LollipopPlot(QStringLiteral("barplot"));
	barPlot->setOrientation(LollipopPlot::Orientation::Horizontal);
	p->addChild(barPlot);

	// set the data
	QVector<const AbstractColumn*> dataColumns;
	auto* c = new Column(QStringLiteral("data"));
	c->setValueAt(0, 3.);
	c->setValueAt(1, 6.);
	c->setValueAt(2, 9.);
	c->setValueAt(3, 12.);
	dataColumns << c;

	barPlot->setDataColumns(dataColumns);

	// check the min and max range values
	QCOMPARE(barPlot->minimum(Dimension::X), 0.0);
	QCOMPARE(barPlot->maximum(Dimension::X), 12.0);
	QCOMPARE(barPlot->minimum(Dimension::Y), 0.);
	QCOMPARE(barPlot->maximum(Dimension::Y), 4.0);
}

QTEST_MAIN(BarPlotTest)
