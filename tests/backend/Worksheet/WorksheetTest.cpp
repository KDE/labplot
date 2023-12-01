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

QTEST_MAIN(WorksheetTest)
