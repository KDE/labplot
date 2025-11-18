/*
	File                 : DecompositionTest.cpp
	Project              : LabPlot
	Description          : Tests for time series decomposition
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2025 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "DecompositionTest.h"
#include "backend/core/Project.h"
#include "backend/timeseriesanalysis/SeasonalDecomposition.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"

#include <QUndoStack>

// ##############################################################################
// ####################### SeasonalDecomposition ################################
// ##############################################################################
/*!
 * \brief create and add a new SeasonalDecomposition, undo and redo this step
 */
void DecompositionTest::testDecompositionInit() {
	Project project;

	auto* decomposition = new SeasonalDecomposition(QStringLiteral("decomposition"));
	project.addChild(decomposition);

	auto children = project.children<SeasonalDecomposition>();

	QCOMPARE(children.size(), 1);

	project.undoStack()->undo();
	children = project.children<SeasonalDecomposition>();
	QCOMPARE(children.size(), 0);

	project.undoStack()->redo();
	children = project.children<SeasonalDecomposition>();
	QCOMPARE(children.size(), 1);
}

/*!
 * \brief create and add a new SeasonalDecomposition, duplicate it and check the number of children
 */
void DecompositionTest::testDecompositionDuplicate() {
	Project project;

	auto* decomposition = new SeasonalDecomposition(QStringLiteral("decomposition"));
	project.addChild(decomposition);

	decomposition->duplicate();

	auto children = project.children<SeasonalDecomposition>();
	QCOMPARE(children.size(), 2);
}

/*!
 * \brief create and add a new SeasonalDecomposition, change the method and check the children
 */
void DecompositionTest::testDecompositionMethodChange() {
	Project project;

	auto* decomposition = new SeasonalDecomposition(QStringLiteral("decomposition"));
	project.addChild(decomposition);

	// the initial method is STL with three components: Trend, Seasonal, Residual
	const auto* w = decomposition->children<Worksheet>().constFirst();
	auto plotAreas = w->children<CartesianPlot>();
	QCOMPARE(plotAreas.size(), 4); // three components plus the origina data

	// change to MSTL which has two seasonal components by default
	decomposition->setMethod(SeasonalDecomposition::Method::MSTL);
	plotAreas = w->children<CartesianPlot>();
	QCOMPARE(plotAreas.size(), 5); // four components plus the original data

	// change back to STL
	decomposition->setMethod(SeasonalDecomposition::Method::STL);
	plotAreas = w->children<CartesianPlot>();
	QCOMPARE(plotAreas.size(), 4); // three components plus the original data
}

QTEST_MAIN(DecompositionTest)
