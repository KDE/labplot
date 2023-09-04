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

QTEST_MAIN(StatisticalPlotsTest)
