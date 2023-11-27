/*
	File                 : StatisticalPlotsTest.cpp
	Project              : LabPlot
	Description          : Tests for statistical plots like Q-Q plot, KDE plot, etc.
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "StatisticalPlotsTest.h"
#include "backend/core/Project.h"
#include "backend/core/column/Column.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/Histogram.h"
#include "backend/worksheet/plots/cartesian/KDEPlot.h"
#include "backend/worksheet/plots/cartesian/QQPlot.h"

#include <gsl/gsl_randist.h>
#include <gsl/gsl_rng.h>

#include <QUndoStack>

// ##############################################################################
// ############################## KDE Plot ######################################
// ##############################################################################

/*!
 * \brief create and add a new KDEPlot, undo and redo this step
 */
void StatisticalPlotsTest::testKDEPlotInit() {
	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	project.addChild(ws);

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	ws->addChild(p);

	auto* kdePlot = new KDEPlot(QStringLiteral("kdeplot"));
	p->addChild(kdePlot);

	auto children = p->children<KDEPlot>();

	QCOMPARE(children.size(), 1);

	project.undoStack()->undo();
	children = p->children<KDEPlot>();
	QCOMPARE(children.size(), 0);

	// TODO: crash!!!
	// project.undoStack()->redo();
	// children = p->children<KDEPlot>();
	// QCOMPARE(children.size(), 1);
}

/*!
 * \brief create and add a new KDEPlot, duplicate it and check the number of children
 */
void StatisticalPlotsTest::testKDEPlotDuplicate() {
	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	project.addChild(ws);

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	ws->addChild(p);

	auto* kdePlot = new KDEPlot(QStringLiteral("kdeplot"));
	p->addChild(kdePlot);

	kdePlot->duplicate();

	auto children = p->children<KDEPlot>();
	QCOMPARE(children.size(), 2);
}

/*!
 * \brief create a KDE plot for 3 values check the plot ranges.
 */
void StatisticalPlotsTest::testKDEPlotRange() {
	// prepare the data
	Spreadsheet sheet(QStringLiteral("test"), false);
	sheet.setColumnCount(1);
	sheet.setRowCount(100);
	auto* column = sheet.column(0);
	column->setValueAt(0, 2.);
	column->setValueAt(1, 4.);
	column->setValueAt(2, 6.);

	// prepare the worksheet + plot
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	auto* p = new CartesianPlot(QStringLiteral("plot"));
	ws->addChild(p);

	auto* kdePlot = new KDEPlot(QStringLiteral("kdeplot"));
	kdePlot->setKernelType(nsl_kernel_gauss);
	kdePlot->setBandwidthType(nsl_kde_bandwidth_custom);
	kdePlot->setBandwidth(0.3);
	p->addChild(kdePlot);
	kdePlot->setDataColumn(column);

	// validate with R via:
	// data <- c(2,4,6);
	// kd <- density(data,kernel="gaussian", bw=0.3)
	// plot(kd, col='blue', lwd=2)

	// check the x-range of the plot which should be [1, 7] (subtract/add 3 sigmas from/to min and max, respectively).
	const auto& rangeX = p->range(Dimension::X);
	QCOMPARE(rangeX.start(), 1);
	QCOMPARE(rangeX.end(), 7);

	// check the y-range of the plot which should be [0, 0.45]
	const auto& rangeY = p->range(Dimension::Y);
	QCOMPARE(rangeY.start(), 0.);
	QCOMPARE(rangeY.end(), 0.45);
}

// ##############################################################################
// ############################## Q-Q Plot ######################################
// ##############################################################################

/*!
 * \brief create and add a new QQPlot, undo and redo this step
 */
void StatisticalPlotsTest::testQQPlotInit() {
	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	project.addChild(ws);

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	ws->addChild(p);

	auto* qqPlot = new QQPlot(QStringLiteral("qqplot"));
	p->addChild(qqPlot);

	auto children = p->children<QQPlot>();

	QCOMPARE(children.size(), 1);

	project.undoStack()->undo();
	children = p->children<QQPlot>();
	QCOMPARE(children.size(), 0);

	// TODO: crash!!!
	// project.undoStack()->redo();
	// children = p->children<QQPlot>();
	// QCOMPARE(children.size(), 1);
}

/*!
 * \brief create and add a new QQPlot, duplicate it and check the number of children
 */
void StatisticalPlotsTest::testQQPlotDuplicate() {
	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	project.addChild(ws);

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	ws->addChild(p);

	auto* qqPlot = new QQPlot(QStringLiteral("qqplot"));
	p->addChild(qqPlot);

	qqPlot->duplicate();

	auto children = p->children<QQPlot>();
	QCOMPARE(children.size(), 2);
}

/*!
 * \brief create QQPlot for 100 normaly distributed values and check the plot ranges.
 */
void StatisticalPlotsTest::testQQPlotRange() {
	// prepare the data
	Spreadsheet sheet(QStringLiteral("test"), false);
	sheet.setColumnCount(1);
	sheet.setRowCount(100);
	auto* column = sheet.column(0);

	// create a generator chosen by the environment variable GSL_RNG_TYPE
	gsl_rng_env_setup();
	const gsl_rng_type* T = gsl_rng_default;
	gsl_rng* r = gsl_rng_alloc(T);
	const double sigma = 1.;

	for (int i = 0; i < 100; ++i)
		column->setValueAt(i, gsl_ran_gaussian(r, sigma));

	// prepare the worksheet + plot
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	auto* p = new CartesianPlot(QStringLiteral("plot"));
	ws->addChild(p);

	auto* qqPlot = new QQPlot(QStringLiteral("qqplot"));
	p->addChild(qqPlot);
	qqPlot->setDataColumn(column);

	// check the x-range of the plot which should be [-2.5, 2.5] for the theoretical quantiles
	const auto& range = p->range(Dimension::X);
	QCOMPARE(range.start(), -2.5);
	QCOMPARE(range.end(), 2.5);
}

void StatisticalPlotsTest::testHistogramColumnRemoved() {
	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	project.addChild(ws);

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	ws->addChild(p);

	auto* histogram = new Histogram(QStringLiteral("histogram"));
	p->addChild(histogram);

	auto* c = new Column(QStringLiteral("TestColumn"));
	project.addChild(c);

	histogram->setDataColumn(c);
	c->setName(QStringLiteral("NewName"));
	QCOMPARE(histogram->dataColumnPath(), QStringLiteral("Project/NewName"));

	c->remove();

	QCOMPARE(histogram->dataColumn(), nullptr);
	QCOMPARE(histogram->dataColumnPath(), QStringLiteral("Project/NewName"));

	c->setName(QStringLiteral("Another new name")); // Shall not lead to a crash

	QCOMPARE(histogram->dataColumn(), nullptr);
	QCOMPARE(histogram->dataColumnPath(), QStringLiteral("Project/NewName"));
}

QTEST_MAIN(StatisticalPlotsTest)
