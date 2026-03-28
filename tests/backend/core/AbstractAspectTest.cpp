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
#include "backend/core/Folder.h"
#include "backend/core/Project.h"
#include "backend/core/column/Column.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/worksheet/plots/cartesian/Histogram.h"
#include "backend/worksheet/plots/cartesian/XYEquationCurve.h"
#include "backend/worksheet/plots/cartesian/XYFitCurve.h"

#include "backend/lib/UndoStack.h"

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

void AbstractAspectTest::testAddChildUndoRedo() {
	Project project;

	auto* worksheet = new Worksheet(QStringLiteral("Worksheet"));
	project.addChild(worksheet);

	auto* plot = new CartesianPlot(QStringLiteral("plot"));
	worksheet->addChild(plot);

	auto* curve = new XYCurve(QStringLiteral("curve"));
	plot->addChild(curve);

	auto* undoStack = project.undoStack();

	// there should be 3 entries on the undo stack:
	// 1. add worksheet
	// 2. add plot
	// 3. add curve
	QCOMPARE(undoStack->count(), 3);

	// the number of entries should stay the same after undo/redo
	undoStack->undo();
	QCOMPARE(undoStack->count(), 3);
	undoStack->redo();
	QCOMPARE(undoStack->count(), 3);
}

void AbstractAspectTest::testAddChildUndoRedoTwice() {
	Project project;

	auto* worksheet = new Worksheet(QStringLiteral("Worksheet"));
	project.addChild(worksheet);

	auto* plot = new CartesianPlot(QStringLiteral("plot"));
	worksheet->addChild(plot);

	auto* undoStack = project.undoStack();

	// there should be 2 entries on the undo stack:
	// 1. add worksheet
	// 2. add plot
	QCOMPARE(undoStack->count(), 2);

	// undo the creation of the plot and of the worksheet and redo it again
	// the number of entries should stay the same after undo/redo
	undoStack->undo();
	QCOMPARE(undoStack->count(), 2);
	undoStack->undo();
	QCOMPARE(undoStack->count(), 2);
	undoStack->redo();
	QCOMPARE(undoStack->count(), 2);
	undoStack->redo();
	QCOMPARE(undoStack->count(), 2);

	// make sure the plot is visible after the redo steps
	QVERIFY(plot->isVisible());
}

void AbstractAspectTest::testDuplicateChildUndoRedo() {
	Project project;

	auto* worksheet = new Worksheet(QStringLiteral("Worksheet"));
	project.addChild(worksheet);

	auto* plot = new CartesianPlot(QStringLiteral("plot"));
	worksheet->addChild(plot);

	auto* curve = new XYCurve(QStringLiteral("curve"));
	plot->addChild(curve);
	curve->duplicate();

	auto* undoStack = project.undoStack();

	// there should be 4 entries on the undo stack:
	// 1. add worksheet
	// 2. add plot
	// 3. add curve
	// 4. duplicate of curve
	QCOMPARE(undoStack->count(), 4);

	// the number of entries should stay the same after undo/redo
	undoStack->undo();
	QCOMPARE(undoStack->count(), 4);
	undoStack->redo();
	QCOMPARE(undoStack->count(), 4);
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
	project.addChild(spreadsheet);

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

		if (childrenProject1.at(i)->path().contains(i18n("Project") + QStringLiteral("/Worksheet/plot/f(x)/x"))
			|| childrenProject1.at(i)->path().contains(i18n("Project") + QStringLiteral("/Worksheet/plot/f(x)/y")))
			continue; // The columns of the equation curve are not saved
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

	// move the worksheet behind the spreadsheet and check the order again
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

/*!
 * \brief AbstractAspectTest::reparentSimple
 * Move a leaf object (Spreadsheet) from the project root into a sub-folder.
 * Verify parent pointer and tree-model consistency.
 */
void AbstractAspectTest::reparentSimple() {
	Project project;
	AspectTreeModel treeModel(&project, this);

	auto* folder = new Folder(QStringLiteral("Folder"));
	project.addChild(folder);

	auto* spreadsheet = new Spreadsheet(QStringLiteral("Spreadsheet"));
	project.addChild(spreadsheet);

	// initial state: both folder and spreadsheet are direct children of the project
	QCOMPARE(project.childCount<AbstractAspect>(), 2);
	QCOMPARE(spreadsheet->parentAspect(), &project);
	QCOMPARE(treeModel.rowCount(treeModel.modelIndexOfAspect(&project)), 2);

	// move the spreadsheet into the folder
	spreadsheet->reparent(folder);

	QCOMPARE(project.childCount<AbstractAspect>(), 1);
	QCOMPARE(folder->childCount<AbstractAspect>(), 1);
	QCOMPARE(spreadsheet->parentAspect(), folder);
	QCOMPARE(folder->child<AbstractAspect>(0), spreadsheet);

	// verify tree model consistency
	QCOMPARE(treeModel.rowCount(treeModel.modelIndexOfAspect(&project)), 1);
	QCOMPARE(treeModel.rowCount(treeModel.modelIndexOfAspect(folder)), 1);
	const auto folderIdx = treeModel.modelIndexOfAspect(folder);
	QCOMPARE(treeModel.data(treeModel.index(0, 0, folderIdx)).toString(), QStringLiteral("Spreadsheet"));
}

/*!
 * \brief AbstractAspectTest::reparentSimpleUndoRedo
 * Move a leaf (Spreadsheet) into a sub-folder, undo, then redo.
 * Verify aspect tree and model consistency at each step.
 */
void AbstractAspectTest::reparentSimpleUndoRedo() {
	Project project;
	AspectTreeModel treeModel(&project, this);

	auto* folder = new Folder(QStringLiteral("Folder"));
	project.addChild(folder);

	auto* spreadsheet = new Spreadsheet(QStringLiteral("Spreadsheet"));
	project.addChild(spreadsheet);

	auto* undoStack = project.undoStack();

	// move the spreadsheet into the folder
	spreadsheet->reparent(folder);

	QCOMPARE(spreadsheet->parentAspect(), folder);
	QCOMPARE(project.childCount<AbstractAspect>(), 1);
	QCOMPARE(folder->childCount<AbstractAspect>(), 1);
	QCOMPARE(treeModel.rowCount(treeModel.modelIndexOfAspect(&project)), 1);
	QCOMPARE(treeModel.rowCount(treeModel.modelIndexOfAspect(folder)), 1);

	// undo
	undoStack->undo();

	QCOMPARE(spreadsheet->parentAspect(), &project);
	QCOMPARE(project.childCount<AbstractAspect>(), 2);
	QCOMPARE(folder->childCount<AbstractAspect>(), 0);
	QCOMPARE(treeModel.rowCount(treeModel.modelIndexOfAspect(&project)), 2);
	QCOMPARE(treeModel.rowCount(treeModel.modelIndexOfAspect(folder)), 0);

	// redo
	undoStack->redo();

	QCOMPARE(spreadsheet->parentAspect(), folder);
	QCOMPARE(project.childCount<AbstractAspect>(), 1);
	QCOMPARE(folder->childCount<AbstractAspect>(), 1);
	QCOMPARE(treeModel.rowCount(treeModel.modelIndexOfAspect(&project)), 1);
	QCOMPARE(treeModel.rowCount(treeModel.modelIndexOfAspect(folder)), 1);
	const auto folderIdx = treeModel.modelIndexOfAspect(folder);
	QCOMPARE(treeModel.data(treeModel.index(0, 0, folderIdx)).toString(), QStringLiteral("Spreadsheet"));
}

/*!
 * \brief AbstractAspectTest::reparentWithChildren
 * Move a Worksheet (which has child objects — a CartesianPlot with curves) into a
 * sub-folder. Verify that the whole subtree moves together and the tree model
 * (AspectTreeModel) stays consistent throughout.
 */
void AbstractAspectTest::reparentWithChildren() {
	Project project;
	AspectTreeModel treeModel(&project, this);

	auto* folder = new Folder(QStringLiteral("Folder"));
	project.addChild(folder);

	auto* worksheet = new Worksheet(QStringLiteral("Worksheet"));
	project.addChild(worksheet);

	auto* plot = new CartesianPlot(QStringLiteral("Plot"));
	worksheet->addChild(plot);

	auto* curve = new XYCurve(QStringLiteral("Curve"));
	plot->addChild(curve);

	// initial state
	QCOMPARE(project.childCount<AbstractAspect>(), 2); // folder + worksheet
	QCOMPARE(worksheet->parentAspect(), &project);

	// move the worksheet (with its subtree) into the folder
	worksheet->reparent(folder);

	// parent updated
	QCOMPARE(worksheet->parentAspect(), folder);

	// project now has only the folder as a direct child
	QCOMPARE(project.childCount<AbstractAspect>(), 1);

	// folder contains the worksheet
	QCOMPARE(folder->childCount<AbstractAspect>(), 1);
	QCOMPARE(folder->child<AbstractAspect>(0), worksheet);

	// worksheet's subtree is intact
	const auto plots = worksheet->children<CartesianPlot>(AbstractAspect::ChildIndexFlag::Recursive);
	QCOMPARE(plots.count(), 1);
	QCOMPARE(plots.at(0), plot);

	const auto curves = worksheet->children<XYCurve>(AbstractAspect::ChildIndexFlag::Recursive);
	QCOMPARE(curves.count(), 1);
	QCOMPARE(curves.at(0), curve);

	// verify tree model consistency
	QCOMPARE(treeModel.rowCount(treeModel.modelIndexOfAspect(&project)), 1);
	QCOMPARE(treeModel.rowCount(treeModel.modelIndexOfAspect(folder)), 1);
	const auto folderIdx = treeModel.modelIndexOfAspect(folder);
	QCOMPARE(treeModel.data(treeModel.index(0, 0, folderIdx)).toString(), QStringLiteral("Worksheet"));
}

/*!
 * \brief AbstractAspectTest::reparentWithChildrenUndoRedo
 * Move a Worksheet with children into a sub-folder, then undo the move (worksheet
 * returns to the project root), then redo (worksheet is back in the folder).
 * The AspectTreeModel is kept alive throughout to detect any begin/end imbalance
 * that would crash or corrupt the model.
 */
void AbstractAspectTest::reparentWithChildrenUndoRedo() {
	Project project;
	AspectTreeModel treeModel(&project, this);

	auto* folder = new Folder(QStringLiteral("Folder"));
	project.addChild(folder);

	auto* worksheet = new Worksheet(QStringLiteral("Worksheet"));
	project.addChild(worksheet);

	auto* plot = new CartesianPlot(QStringLiteral("Plot"));
	worksheet->addChild(plot);

	auto* curve = new XYCurve(QStringLiteral("Curve"));
	plot->addChild(curve);

	auto* undoStack = project.undoStack();

	// 3 commands on the undo stack: addFolder, addWorksheet, addPlot, addCurve
	const int stackDepthBeforeReparent = undoStack->count();

	// move the worksheet into the folder
	worksheet->reparent(folder);
	QCOMPARE(undoStack->count(), stackDepthBeforeReparent + 1);

	// post-move state
	QCOMPARE(worksheet->parentAspect(), folder);
	QCOMPARE(project.childCount<AbstractAspect>(), 1);
	QCOMPARE(folder->childCount<AbstractAspect>(), 1);
	QCOMPARE(treeModel.rowCount(treeModel.modelIndexOfAspect(&project)), 1);
	QCOMPARE(treeModel.rowCount(treeModel.modelIndexOfAspect(folder)), 1);

	// undo — worksheet should return to the project root
	undoStack->undo();

	QCOMPARE(worksheet->parentAspect(), &project);
	QCOMPARE(project.childCount<AbstractAspect>(), 2); // folder + worksheet
	QCOMPARE(folder->childCount<AbstractAspect>(), 0);
	QCOMPARE(treeModel.rowCount(treeModel.modelIndexOfAspect(&project)), 2);
	QCOMPARE(treeModel.rowCount(treeModel.modelIndexOfAspect(folder)), 0);

	// worksheet's subtree must still be intact after undo
	QCOMPARE(worksheet->children<CartesianPlot>(AbstractAspect::ChildIndexFlag::Recursive).count(), 1);
	QCOMPARE(worksheet->children<XYCurve>(AbstractAspect::ChildIndexFlag::Recursive).count(), 1);

	// redo — worksheet moves into the folder again
	undoStack->redo();

	QCOMPARE(worksheet->parentAspect(), folder);
	QCOMPARE(project.childCount<AbstractAspect>(), 1);
	QCOMPARE(folder->childCount<AbstractAspect>(), 1);
	QCOMPARE(folder->child<AbstractAspect>(0), worksheet);
	QCOMPARE(treeModel.rowCount(treeModel.modelIndexOfAspect(&project)), 1);
	QCOMPARE(treeModel.rowCount(treeModel.modelIndexOfAspect(folder)), 1);

	// subtree intact after redo
	QCOMPARE(worksheet->children<CartesianPlot>(AbstractAspect::ChildIndexFlag::Recursive).count(), 1);
	QCOMPARE(worksheet->children<XYCurve>(AbstractAspect::ChildIndexFlag::Recursive).count(), 1);
}

/*!
 * \brief AbstractAspectTest::reparentBetweenFolders
 * Move a Spreadsheet from one sub-folder to another sub-folder.
 */
void AbstractAspectTest::reparentBetweenFolders() {
	Project project;
	AspectTreeModel treeModel(&project, this);

	auto* folderA = new Folder(QStringLiteral("FolderA"));
	project.addChild(folderA);

	auto* folderB = new Folder(QStringLiteral("FolderB"));
	project.addChild(folderB);

	auto* spreadsheet = new Spreadsheet(QStringLiteral("Spreadsheet"));
	folderA->addChild(spreadsheet);

	// initial state
	QCOMPARE(folderA->childCount<AbstractAspect>(), 1);
	QCOMPARE(folderB->childCount<AbstractAspect>(), 0);
	QCOMPARE(treeModel.rowCount(treeModel.modelIndexOfAspect(folderA)), 1);
	QCOMPARE(treeModel.rowCount(treeModel.modelIndexOfAspect(folderB)), 0);

	// move from folderA to folderB
	spreadsheet->reparent(folderB);

	QCOMPARE(spreadsheet->parentAspect(), folderB);
	QCOMPARE(folderA->childCount<AbstractAspect>(), 0);
	QCOMPARE(folderB->childCount<AbstractAspect>(), 1);
	QCOMPARE(folderB->child<AbstractAspect>(0), spreadsheet);

	// model consistency
	QCOMPARE(treeModel.rowCount(treeModel.modelIndexOfAspect(folderA)), 0);
	QCOMPARE(treeModel.rowCount(treeModel.modelIndexOfAspect(folderB)), 1);
	const auto folderBIdx = treeModel.modelIndexOfAspect(folderB);
	QCOMPARE(treeModel.data(treeModel.index(0, 0, folderBIdx)).toString(), QStringLiteral("Spreadsheet"));

	// undo — spreadsheet goes back to folderA
	project.undoStack()->undo();

	QCOMPARE(spreadsheet->parentAspect(), folderA);
	QCOMPARE(folderA->childCount<AbstractAspect>(), 1);
	QCOMPARE(folderB->childCount<AbstractAspect>(), 0);
	QCOMPARE(treeModel.rowCount(treeModel.modelIndexOfAspect(folderA)), 1);
	QCOMPARE(treeModel.rowCount(treeModel.modelIndexOfAspect(folderB)), 0);

	// redo
	project.undoStack()->redo();

	QCOMPARE(spreadsheet->parentAspect(), folderB);
	QCOMPARE(folderA->childCount<AbstractAspect>(), 0);
	QCOMPARE(folderB->childCount<AbstractAspect>(), 1);
	QCOMPARE(treeModel.rowCount(treeModel.modelIndexOfAspect(folderA)), 0);
	QCOMPARE(treeModel.rowCount(treeModel.modelIndexOfAspect(folderB)), 1);
}

/*!
 * \brief AbstractAspectTest::reparentToProjectRoot
 * Move a Spreadsheet from a sub-folder back up to the project root.
 */
void AbstractAspectTest::reparentToProjectRoot() {
	Project project;
	AspectTreeModel treeModel(&project, this);

	auto* folder = new Folder(QStringLiteral("Folder"));
	project.addChild(folder);

	auto* spreadsheet = new Spreadsheet(QStringLiteral("Spreadsheet"));
	folder->addChild(spreadsheet);

	QCOMPARE(project.childCount<AbstractAspect>(), 1);
	QCOMPARE(folder->childCount<AbstractAspect>(), 1);
	QCOMPARE(treeModel.rowCount(treeModel.modelIndexOfAspect(&project)), 1);
	QCOMPARE(treeModel.rowCount(treeModel.modelIndexOfAspect(folder)), 1);

	// move spreadsheet up to the project root
	spreadsheet->reparent(&project);

	QCOMPARE(spreadsheet->parentAspect(), &project);
	QCOMPARE(project.childCount<AbstractAspect>(), 2); // folder + spreadsheet
	QCOMPARE(folder->childCount<AbstractAspect>(), 0);
	QCOMPARE(treeModel.rowCount(treeModel.modelIndexOfAspect(&project)), 2);
	QCOMPARE(treeModel.rowCount(treeModel.modelIndexOfAspect(folder)), 0);

	// undo — spreadsheet returns to the folder
	project.undoStack()->undo();

	QCOMPARE(spreadsheet->parentAspect(), folder);
	QCOMPARE(project.childCount<AbstractAspect>(), 1);
	QCOMPARE(folder->childCount<AbstractAspect>(), 1);
	QCOMPARE(treeModel.rowCount(treeModel.modelIndexOfAspect(&project)), 1);
	QCOMPARE(treeModel.rowCount(treeModel.modelIndexOfAspect(folder)), 1);
}

/*!
 * \brief AbstractAspectTest::reparentToNestedFolder
 * Move a Spreadsheet from the project root into a deeply nested folder (folder/subfolder).
 */
void AbstractAspectTest::reparentToNestedFolder() {
	Project project;
	AspectTreeModel treeModel(&project, this);

	auto* folder = new Folder(QStringLiteral("Folder"));
	project.addChild(folder);

	auto* subfolder = new Folder(QStringLiteral("Subfolder"));
	folder->addChild(subfolder);

	auto* spreadsheet = new Spreadsheet(QStringLiteral("Spreadsheet"));
	project.addChild(spreadsheet);

	QCOMPARE(project.childCount<AbstractAspect>(), 2); // folder + spreadsheet
	QCOMPARE(subfolder->childCount<AbstractAspect>(), 0);

	// move spreadsheet into the nested subfolder
	spreadsheet->reparent(subfolder);

	QCOMPARE(spreadsheet->parentAspect(), subfolder);
	QCOMPARE(project.childCount<AbstractAspect>(), 1);
	QCOMPARE(subfolder->childCount<AbstractAspect>(), 1);
	QCOMPARE(subfolder->child<AbstractAspect>(0), spreadsheet);

	// model consistency at every level
	QCOMPARE(treeModel.rowCount(treeModel.modelIndexOfAspect(&project)), 1);
	QCOMPARE(treeModel.rowCount(treeModel.modelIndexOfAspect(folder)), 1);
	QCOMPARE(treeModel.rowCount(treeModel.modelIndexOfAspect(subfolder)), 1);
	const auto subfolderIdx = treeModel.modelIndexOfAspect(subfolder);
	QCOMPARE(treeModel.data(treeModel.index(0, 0, subfolderIdx)).toString(), QStringLiteral("Spreadsheet"));

	// undo
	project.undoStack()->undo();

	QCOMPARE(spreadsheet->parentAspect(), &project);
	QCOMPARE(project.childCount<AbstractAspect>(), 2);
	QCOMPARE(subfolder->childCount<AbstractAspect>(), 0);
	QCOMPARE(treeModel.rowCount(treeModel.modelIndexOfAspect(&project)), 2);
	QCOMPARE(treeModel.rowCount(treeModel.modelIndexOfAspect(subfolder)), 0);
}

/*!
 * \brief AbstractAspectTest::reparentFolder
 * Move a Folder (containing a Spreadsheet) into another Folder.
 * This is the path exercised by processDropEvent for Folder-on-Folder drops.
 */
void AbstractAspectTest::reparentFolder() {
	Project project;
	AspectTreeModel treeModel(&project, this);

	auto* folderA = new Folder(QStringLiteral("FolderA"));
	project.addChild(folderA);

	auto* spreadsheet = new Spreadsheet(QStringLiteral("Spreadsheet"));
	folderA->addChild(spreadsheet);

	auto* folderB = new Folder(QStringLiteral("FolderB"));
	project.addChild(folderB);

	QCOMPARE(project.childCount<AbstractAspect>(), 2);
	QCOMPARE(folderA->childCount<AbstractAspect>(), 1);

	// move folderA (with its child spreadsheet) into folderB
	folderA->reparent(folderB);

	QCOMPARE(folderA->parentAspect(), folderB);
	QCOMPARE(project.childCount<AbstractAspect>(), 1); // only folderB
	QCOMPARE(folderB->childCount<AbstractAspect>(), 1); // folderA
	QCOMPARE(folderA->childCount<AbstractAspect>(), 1); // spreadsheet
	QCOMPARE(folderA->child<AbstractAspect>(0), spreadsheet);

	// model consistency
	QCOMPARE(treeModel.rowCount(treeModel.modelIndexOfAspect(&project)), 1);
	QCOMPARE(treeModel.rowCount(treeModel.modelIndexOfAspect(folderB)), 1);
	QCOMPARE(treeModel.rowCount(treeModel.modelIndexOfAspect(folderA)), 1);
	const auto folderAIdx = treeModel.modelIndexOfAspect(folderA);
	QCOMPARE(treeModel.data(treeModel.index(0, 0, folderAIdx)).toString(), QStringLiteral("Spreadsheet"));

	// undo — folderA returns to the project root
	project.undoStack()->undo();

	QCOMPARE(folderA->parentAspect(), &project);
	QCOMPARE(project.childCount<AbstractAspect>(), 2);
	QCOMPARE(folderB->childCount<AbstractAspect>(), 0);
	QCOMPARE(folderA->childCount<AbstractAspect>(), 1);
	QCOMPARE(treeModel.rowCount(treeModel.modelIndexOfAspect(&project)), 2);
	QCOMPARE(treeModel.rowCount(treeModel.modelIndexOfAspect(folderB)), 0);
	QCOMPARE(treeModel.rowCount(treeModel.modelIndexOfAspect(folderA)), 1);

	// redo
	project.undoStack()->redo();

	QCOMPARE(folderA->parentAspect(), folderB);
	QCOMPARE(project.childCount<AbstractAspect>(), 1);
	QCOMPARE(folderB->childCount<AbstractAspect>(), 1);
	QCOMPARE(folderA->childCount<AbstractAspect>(), 1);
	QCOMPARE(treeModel.rowCount(treeModel.modelIndexOfAspect(&project)), 1);
	QCOMPARE(treeModel.rowCount(treeModel.modelIndexOfAspect(folderB)), 1);
	QCOMPARE(treeModel.rowCount(treeModel.modelIndexOfAspect(folderA)), 1);
}

QTEST_MAIN(AbstractAspectTest)
