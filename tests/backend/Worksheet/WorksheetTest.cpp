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
#include "kdefrontend/GuiTools.h"

#include <QPen>

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

/*!
 * tests the replacement of the file extension when switching between the different formats during the export.
 */
// TODO: testing the code in GuiTools only. move it later to another location once we have tests for the common code in the frontend.
void WorksheetTest::exportReplaceExtension() {
	// from no extension to .pdf, no dot character in the path
	QString extension = QStringLiteral(".pdf");
#if defined(Q_OS_WIN)
	QString path = QStringLiteral("C:\\Users\\user\\test_dir\\Worksheet");
	QCOMPARE(GuiTools::replaceExtension(path, extension), QStringLiteral("C:\\Users\\user\\test_dir\\Worksheet.pdf"));
#else
	QString path = QStringLiteral("/home/user/test_dir/Worksheet");
	QCOMPARE(GuiTools::replaceExtension(path, extension), QStringLiteral("/home/user/test_dir/Worksheet.pdf"));
#endif

	// from .pdf to .svg, no dot character in the path
	extension = QStringLiteral(".svg");
#if defined(Q_OS_WIN)
	path = QStringLiteral("C:\\Users\\user\\test_dir\\Worksheet.pdf");
	QCOMPARE(GuiTools::replaceExtension(path, extension), QStringLiteral("C:\\Users\\user\\test_dir\\Worksheet.svg"));
#else
	path = QStringLiteral("/home/user/test_dir/Worksheet.pdf");
	QCOMPARE(GuiTools::replaceExtension(path, extension), QStringLiteral("/home/user/test_dir/Worksheet.svg"));
#endif

	// from no extension to .pdf, dot character in the path
	extension = QStringLiteral(".pdf");
#if defined(Q_OS_WIN)
	path = QStringLiteral("C:\\Users\\user\\test.dir\\Worksheet");
	QCOMPARE(GuiTools::replaceExtension(path, extension), QStringLiteral("C:\\Users\\user\\test.dir\\Worksheet.pdf"));
#else
	path = QStringLiteral("/home/user/test.dir/Worksheet");
	QCOMPARE(GuiTools::replaceExtension(path, extension), QStringLiteral("/home/user/test.dir/Worksheet.pdf"));
#endif

	// from .pdf to .svg, dot character in the path
	extension = QStringLiteral(".svg");
#if defined(Q_OS_WIN)
	path = QStringLiteral("C:\\Users\\user\\test.dir\\Worksheet.pdf");
	QCOMPARE(GuiTools::replaceExtension(path, extension), QStringLiteral("C:\\Users\\user\\test.dir\\Worksheet.svg"));
#else
	path = QStringLiteral("/home/user/test.dir/Worksheet.pdf");
	QCOMPARE(GuiTools::replaceExtension(path, extension), QStringLiteral("/home/user/test.dir/Worksheet.svg"));
#endif
}

QTEST_MAIN(WorksheetTest)
