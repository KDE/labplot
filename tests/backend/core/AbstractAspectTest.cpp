/*
	File                 : AbstractAspectTest.cpp
	Project              : LabPlot
	Description          : Tests for AbstractAspect
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Martin Marmsoler <martin.marmsoler@gmail.com>
	SPDX-FileCopyrightText: 2023-2025 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "AbstractAspectTest.h"
#include "backend/core/AspectTreeModel.h"
#include "backend/core/Project.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/worksheet/plots/cartesian/Histogram.h"
#include "backend/worksheet/plots/cartesian/XYEquationCurve.h"
#include "backend/worksheet/plots/cartesian/XYFitCurve.h"

#include <QUndoStack>

void AbstractAspectTest::name() {
	Project project;

	const QString initialName = QStringLiteral("Worksheet");
	const QString secondName = QStringLiteral("New name");
	const QString thirdName = QStringLiteral("Another new name");

	auto* worksheet = new Worksheet(QStringLiteral("Worksheet"));
	project.addChild(worksheet);

	int aspectDescriptionAboutToChangeCounter = 0;
	connect(&project,
			&AbstractAspect::aspectDescriptionAboutToChange,
			[worksheet, &aspectDescriptionAboutToChangeCounter, initialName, secondName, thirdName](const AbstractAspect* aspect) {
				QCOMPARE(aspect, worksheet);
				switch (aspectDescriptionAboutToChangeCounter) {
				case 0: {
					QCOMPARE(aspect->name(), initialName);
					break;
				}
				case 1: {
					QCOMPARE(aspect->name(), secondName);
					break;
				}
				case 2: {
					QCOMPARE(aspect->name(), thirdName);
					break;
				}
				case 3: {
					QCOMPARE(aspect->name(), secondName);
					break;
				}
				case 4: {
					QCOMPARE(aspect->name(), initialName);
					break;
				}
				case 5: {
					QCOMPARE(aspect->name(), secondName);
					break;
				}
				}
				aspectDescriptionAboutToChangeCounter++;
			});

	int aspectDescriptionChangedCounter = 0;
	connect(&project,
			&AbstractAspect::aspectDescriptionChanged,
			[worksheet, &aspectDescriptionChangedCounter, initialName, secondName, thirdName](const AbstractAspect* aspect) {
				QCOMPARE(aspect, worksheet);
				switch (aspectDescriptionChangedCounter) {
				case 0: {
					QCOMPARE(aspect->name(), secondName);
					break;
				}
				case 1: {
					QCOMPARE(aspect->name(), thirdName);
					break;
				}
				case 2: {
					QCOMPARE(aspect->name(), secondName);
					break;
				}
				case 3: {
					QCOMPARE(aspect->name(), initialName);
					break;
				}
				case 4: {
					QCOMPARE(aspect->name(), secondName);
					break;
				}
				case 5: {
					QCOMPARE(aspect->name(), thirdName);
					break;
				}
				}
				aspectDescriptionChangedCounter++;
			});

	worksheet->setName(secondName);
	worksheet->setName(thirdName);

	worksheet->undoStack()->undo();
	QCOMPARE(worksheet->name(), secondName);
	worksheet->undoStack()->undo();
	QCOMPARE(worksheet->name(), initialName);

	worksheet->undoStack()->redo();
	QCOMPARE(worksheet->name(), secondName);
	worksheet->undoStack()->redo();
	QCOMPARE(worksheet->name(), thirdName);

	QCOMPARE(aspectDescriptionAboutToChangeCounter, 6);
	QCOMPARE(aspectDescriptionChangedCounter, 6);
}

void AbstractAspectTest::copyPaste() {
	Project project;

	auto* worksheet = new Worksheet(QStringLiteral("Worksheet"));
	project.addChild(worksheet);

	auto* plot = new CartesianPlot(QStringLiteral("plot"));
	worksheet->addChild(plot);
	plot->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created
	plot->setNiceExtend(false);

	auto* equationCurve{new XYEquationCurve(QStringLiteral("f(x)"))};
	equationCurve->setCoordinateSystemIndex(plot->defaultCoordinateSystemIndex());
	plot->addChild(equationCurve);

	XYEquationCurve::EquationData data;
	data.min = QStringLiteral("0");
	data.max = QStringLiteral("1");
	data.count = 10;
	data.expression1 = QStringLiteral("x");
	equationCurve->setEquationData(data);
	equationCurve->recalculate();

	worksheet->copy();
	project.paste();

	const auto& worksheets = project.children(AspectType::Worksheet);
	QCOMPARE(worksheets.count(), 2);
	QVERIFY(worksheets.at(0)->uuid() != worksheets.at(1)->uuid());

	const auto& childrenWorksheet1 =
		worksheets.at(0)->children(AspectType::AbstractAspect, {AbstractAspect::ChildIndexFlag::IncludeHidden, AbstractAspect::ChildIndexFlag::Recursive});
	const auto& childrenWorksheet2 =
		worksheets.at(1)->children(AspectType::AbstractAspect, {AbstractAspect::ChildIndexFlag::IncludeHidden, AbstractAspect::ChildIndexFlag::Recursive});

	QCOMPARE(childrenWorksheet1.count(), childrenWorksheet2.count());

	for (int i = 0; i < childrenWorksheet1.count(); i++) {
		QVERIFY(childrenWorksheet1.at(i)->type() == childrenWorksheet2.at(i)->type());
		if (childrenWorksheet1.at(i)->type() == AspectType::AbstractAspect)
			continue; // unique will change the triggered for those aspects, and therefore when changing for the second from "1" to "2" it happens that "2"
					  // already exists and therefore it receives "3"
		QVERIFY(childrenWorksheet1.at(i)->name() == childrenWorksheet2.at(i)->name());
		QVERIFY(childrenWorksheet1.at(i)->uuid() != childrenWorksheet2.at(i)->uuid());
	}
}

/*!
 * check copy&paste (duplicate) of a XYFitCurve with the data source type "Spreadsheet",
 * the pointers to the data source columns must be properly restored after the duplication.
 */
void AbstractAspectTest::pasteFitCurveSourceSpreadsheet() {
	Project project;

	auto* spreadsheet = new Spreadsheet(QStringLiteral("Spreadsheet"));
	spreadsheet->setColumnCount(2);

	auto* worksheet = new Worksheet(QStringLiteral("Worksheet"));
	project.addChild(worksheet);

	auto* plot = new CartesianPlot(QStringLiteral("plot"));
	worksheet->addChild(plot);

	auto* fitCurve = new XYFitCurve(QStringLiteral("fit"));
	fitCurve->setDataSourceType(XYAnalysisCurve::DataSourceType::Spreadsheet);
	fitCurve->setXDataColumn(spreadsheet->column(0));
	fitCurve->setYDataColumn(spreadsheet->column(1));
	plot->addChild(fitCurve);

	fitCurve->copy();
	plot->paste();

	// checks
	const auto& fitCurves = project.children<XYFitCurve>(AbstractAspect::ChildIndexFlag::Recursive);
	QCOMPARE(fitCurves.count(), 2);
	auto* fitCurveCopy = fitCurves.at(1);
	QCOMPARE(fitCurveCopy->dataSourceType(), XYAnalysisCurve::DataSourceType::Spreadsheet);
	QCOMPARE(fitCurveCopy->xDataColumn(), spreadsheet->column(0));
	QCOMPARE(fitCurveCopy->yDataColumn(), spreadsheet->column(1));
}

/*!
 * check copy&paste (duplicate) of a XYFitCurve with the data source type "Curve",
 * the pointer to the data source curve must be properly restored after the duplication.
 */
void AbstractAspectTest::pasteFitCurveSourceCurve() {
	Project project;

	auto* worksheet = new Worksheet(QStringLiteral("Worksheet"));
	project.addChild(worksheet);

	auto* plot = new CartesianPlot(QStringLiteral("plot"));
	worksheet->addChild(plot);

	auto* xyCurve = new XYCurve(QStringLiteral("xy-curve"));
	plot->addChild(xyCurve);

	auto* fitCurve = new XYFitCurve(QStringLiteral("fit"));
	fitCurve->setDataSourceType(XYAnalysisCurve::DataSourceType::Curve);
	fitCurve->setDataSourceCurve(xyCurve);
	plot->addChild(fitCurve);

	fitCurve->copy();
	plot->paste();

	// checks
	const auto& fitCurves = project.children<XYFitCurve>(AbstractAspect::ChildIndexFlag::Recursive);
	QCOMPARE(fitCurves.count(), 2);
	auto* fitCurveCopy = fitCurves.at(1);
	QCOMPARE(fitCurveCopy->dataSourceType(), XYAnalysisCurve::DataSourceType::Curve);
	QCOMPARE(fitCurveCopy->dataSourceCurve(), xyCurve);
}

/*!
 * check copy&paste (duplicate) of a XYFitCurve with the data source type "Histogram",
 * the pointer to the data source histogram must be properly restored after the duplication.
 */
void AbstractAspectTest::pasteFitCurveSourceHistogram() {
	Project project;

	auto* worksheet = new Worksheet(QStringLiteral("Worksheet"));
	project.addChild(worksheet);

	auto* plot = new CartesianPlot(QStringLiteral("plot"));
	worksheet->addChild(plot);

	auto* histogram = new Histogram(QStringLiteral("histogram"));
	plot->addChild(histogram);

	auto* fitCurve = new XYFitCurve(QStringLiteral("fit"));
	fitCurve->setDataSourceType(XYAnalysisCurve::DataSourceType::Histogram);
	fitCurve->setDataSourceHistogram(histogram);
	plot->addChild(fitCurve);

	fitCurve->copy();
	plot->paste();

	// checks
	const auto& fitCurves = project.children<XYFitCurve>(AbstractAspect::ChildIndexFlag::Recursive);
	QCOMPARE(fitCurves.count(), 2);
	auto* fitCurveCopy = fitCurves.at(1);
	QCOMPARE(fitCurveCopy->dataSourceType(), XYAnalysisCurve::DataSourceType::Histogram);
	QCOMPARE(fitCurveCopy->dataSourceHistogram(), histogram);
}

void AbstractAspectTest::saveLoad() {
	Project project;

	auto* worksheet = new Worksheet(QStringLiteral("Worksheet"));
	project.addChild(worksheet);

	auto* plot = new CartesianPlot(QStringLiteral("plot"));
	worksheet->addChild(plot);
	plot->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created
	plot->setNiceExtend(false);

	auto* equationCurve{new XYEquationCurve(QStringLiteral("f(x)"))};
	equationCurve->setCoordinateSystemIndex(plot->defaultCoordinateSystemIndex());
	plot->addChild(equationCurve);

	XYEquationCurve::EquationData data;
	data.min = QStringLiteral("0");
	data.max = QStringLiteral("1");
	data.count = 10;
	data.expression1 = QStringLiteral("x");
	equationCurve->setEquationData(data);
	equationCurve->recalculate();

	QString savePath;
	SAVE_PROJECT("testLinkSpreadsheetSaveLoad");

	Project project2;
	QCOMPARE(project2.load(savePath), true);

	const auto& childrenProject1 =
		project.children(AspectType::AbstractAspect, {AbstractAspect::ChildIndexFlag::IncludeHidden, AbstractAspect::ChildIndexFlag::Recursive});
	const auto& childrenProject2 =
		project2.children(AspectType::AbstractAspect, {AbstractAspect::ChildIndexFlag::IncludeHidden, AbstractAspect::ChildIndexFlag::Recursive});

	QCOMPARE(childrenProject1.count(), childrenProject2.count());

	for (int i = 0; i < childrenProject1.count(); i++) {
		QVERIFY(childrenProject1.at(i)->type() == childrenProject2.at(i)->type());
		if (childrenProject1.at(i)->type() == AspectType::AbstractAspect)
			continue; // Usually they don't implement the write basic function and therefore they cannot have the same uuid

		QVERIFY(childrenProject1.at(i)->name() == childrenProject2.at(i)->name());

		if (childrenProject1.at(i)->path().contains(QStringLiteral("Project/Worksheet/plot/f(x)/x"))
			|| childrenProject1.at(i)->path().contains(QStringLiteral("Project/Worksheet/plot/f(x)/y")))
			continue; // The columns of the quation curve are not saved
		QVERIFY(childrenProject1.at(i)->uuid() == childrenProject2.at(i)->uuid());
	}
}

void AbstractAspectTest::moveUp() {
	Project project;

	AspectTreeModel treemodel(&project, this);

	auto* worksheet = new Worksheet(QStringLiteral("Worksheet"));
	project.addChild(worksheet);

	auto* spreadsheet = new Spreadsheet(QStringLiteral("Spreadsheet"));
	project.addChild(spreadsheet);

	// check the order of children
	QCOMPARE(project.child<AbstractAspect>(0), worksheet);
	QCOMPARE(project.child<AbstractAspect>(1), spreadsheet);

	// move the spreadsheet in front of the worksheet and check the order again
	spreadsheet->moveUp();
	QCOMPARE(project.child<AbstractAspect>(0), spreadsheet);
	QCOMPARE(project.child<AbstractAspect>(1), worksheet);

	spreadsheet->undoStack()->undo();

	QCOMPARE(project.child<AbstractAspect>(0), worksheet);
	QCOMPARE(project.child<AbstractAspect>(1), spreadsheet);

	spreadsheet->undoStack()->redo();

	QCOMPARE(project.child<AbstractAspect>(0), spreadsheet);
	QCOMPARE(project.child<AbstractAspect>(1), worksheet);
}

void AbstractAspectTest::moveDown() {
	Project project;

	AspectTreeModel treemodel(&project, this);

	auto* worksheet = new Worksheet(QStringLiteral("Worksheet"));
	project.addChild(worksheet);

	auto* spreadsheet = new Spreadsheet(QStringLiteral("Spreadsheet"));
	project.addChild(spreadsheet);

	// check the order of children
	QCOMPARE(project.child<AbstractAspect>(0), worksheet);
	QCOMPARE(project.child<AbstractAspect>(1), spreadsheet);

	// move the worksheet behing the speadsheet and check the order again
	worksheet->moveDown();
	QCOMPARE(project.child<AbstractAspect>(0), spreadsheet);
	QCOMPARE(project.child<AbstractAspect>(1), worksheet);

	spreadsheet->undoStack()->undo();

	QCOMPARE(project.child<AbstractAspect>(0), worksheet);
	QCOMPARE(project.child<AbstractAspect>(1), spreadsheet);

	spreadsheet->undoStack()->redo();

	QCOMPARE(project.child<AbstractAspect>(0), spreadsheet);
	QCOMPARE(project.child<AbstractAspect>(1), worksheet);
}

/*!
 * \brief AbstractAspectTest::moveUpDown
 * Move up/down with a treemodel connected
 */
void AbstractAspectTest::moveUpDown() {
	Project project;

	AspectTreeModel treemodel(&project, this);

	auto* worksheet = new Worksheet(QStringLiteral("Worksheet"));
	project.addChild(worksheet);

	auto* spreadsheet = new Spreadsheet(QStringLiteral("Spreadsheet"));
	project.addChild(spreadsheet);

	auto* spreadsheet2 = new Spreadsheet(QStringLiteral("Spreadsheet2"));
	project.addChild(spreadsheet2);

	// check the order of children
	QCOMPARE(project.child<AbstractAspect>(0), worksheet);
	QCOMPARE(project.child<AbstractAspect>(1), spreadsheet);
	QCOMPARE(project.child<AbstractAspect>(2), spreadsheet2);

	spreadsheet2->moveUp();
	QCOMPARE(project.child<AbstractAspect>(0), worksheet);
	QCOMPARE(project.child<AbstractAspect>(1), spreadsheet2);
	QCOMPARE(project.child<AbstractAspect>(2), spreadsheet);

	spreadsheet2->moveUp();
	QCOMPARE(project.child<AbstractAspect>(0), spreadsheet2);
	QCOMPARE(project.child<AbstractAspect>(1), worksheet);
	QCOMPARE(project.child<AbstractAspect>(2), spreadsheet);

	spreadsheet2->moveDown();
	QCOMPARE(project.child<AbstractAspect>(0), worksheet);
	QCOMPARE(project.child<AbstractAspect>(1), spreadsheet2);
	QCOMPARE(project.child<AbstractAspect>(2), spreadsheet);

	spreadsheet->undoStack()->undo();

	QCOMPARE(project.child<AbstractAspect>(0), spreadsheet2);
	QCOMPARE(project.child<AbstractAspect>(1), worksheet);
	QCOMPARE(project.child<AbstractAspect>(2), spreadsheet);

	spreadsheet->undoStack()->redo();

	QCOMPARE(project.child<AbstractAspect>(0), worksheet);
	QCOMPARE(project.child<AbstractAspect>(1), spreadsheet2);
	QCOMPARE(project.child<AbstractAspect>(2), spreadsheet);
}

QTEST_MAIN(AbstractAspectTest)
