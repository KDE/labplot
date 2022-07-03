/*
	File                 : TextLabelTest.cpp
	Project              : LabPlot
	Description          : Tests for TextLabel
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "BarPlotTest.h"
#include "backend/core/Project.h"
#include "backend/core/column/Column.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/BarPlot.h"

/*!
 * \brief one dataset, grouped
 */
void BarPlotTest::testRange01() {
	Project project;
	auto* ws = new Worksheet("worksheet");
	project.addChild(ws);

	auto* p = new CartesianPlot("plot");
	ws->addChild(p);

	auto* barPlot = new BarPlot("barplot");
	ws->addChild(barPlot);

	// set the data
	QVector<const AbstractColumn*> dataColumns;
	auto* c = new Column("data");
	c->setValueAt(0, 3.);
	c->setValueAt(1, 6.);
	c->setValueAt(2, 9.);
	c->setValueAt(3, 12.);
	dataColumns << c;

	barPlot->setDataColumns(dataColumns);

	// check the min and max range values
	QCOMPARE(barPlot->xMinimum(), 0.0);
	QCOMPARE(barPlot->xMaximum(), 5.0);
	QCOMPARE(barPlot->yMinimum(), 0.);
	QCOMPARE(barPlot->yMaximum(), 12.);
}

/*!
 * \brief one dataset, grouped, with a negative value
 */
void BarPlotTest::testRange02() {
	Project project;
	auto* ws = new Worksheet("worksheet");
	project.addChild(ws);

	auto* p = new CartesianPlot("plot");
	ws->addChild(p);

	auto* barPlot = new BarPlot("barplot");
	ws->addChild(barPlot);

	// set the data
	QVector<const AbstractColumn*> dataColumns;
	auto* c = new Column("data");
	c->setValueAt(0, 3.);
	c->setValueAt(1, -6.);
	c->setValueAt(2, 9.);
	c->setValueAt(3, 12.);
	dataColumns << c;

	barPlot->setDataColumns(dataColumns);

	// check the min and max range values
	QCOMPARE(barPlot->xMinimum(), 0.0);
	QCOMPARE(barPlot->xMaximum(), 5.0);
	QCOMPARE(barPlot->yMinimum(), -6.);
	QCOMPARE(barPlot->yMaximum(), 12.);
}

QTEST_MAIN(BarPlotTest)
