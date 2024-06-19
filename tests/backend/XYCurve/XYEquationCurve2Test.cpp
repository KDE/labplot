/*
	File                 : XYEquationCurve2Test.cpp
	Project              : LabPlot
	Description          : Tests for XYEquationCurve2
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Martin Marmsoler <martin.marmsoler@gmail.com>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "XYEquationCurve2Test.h"
#include "backend/core/Project.h"
#include "backend/core/column/Column.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/worksheet/plots/cartesian/XYCurve.h"
#include "backend/worksheet/plots/cartesian/XYEquationCurve.h"
#include "backend/worksheet/plots/cartesian/XYEquationCurve2.h"
#include "kdefrontend/dockwidgets/XYEquationCurve2Dock.h"

#include <QUndoStack>

void XYEquationCurve2Test::setCurves() {
	Project project;
	auto* ws = new Worksheet(QStringLiteral("Worksheet"));
	QVERIFY(ws != nullptr);
	project.addChild(ws);

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	p->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created
	QVERIFY(p != nullptr);
	ws->addChild(p);

	p->addChild(new XYEquationCurve(QLatin1String("eq")));

	auto equationCurves = p->children(AspectType::XYEquationCurve);
	QCOMPARE(equationCurves.count(), 1);
	auto* equationCurve = static_cast<XYEquationCurve*>(equationCurves.at(0));
	XYEquationCurve::EquationData data;
	data.count = 100;
	data.expression1 = QStringLiteral("x");
	data.expression2 = QString();
	data.min = QStringLiteral("1");
	data.max = QStringLiteral("100");
	data.type = XYEquationCurve::EquationType::Cartesian;
	equationCurve->setEquationData(data);

	QCOMPARE(equationCurve->xColumn()->rowCount(), data.count);

	p->addChild(new XYEquationCurve2(QLatin1String("eq2")));
	auto equationCurves2 = p->children(AspectType::XYEquationCurve2);
	QCOMPARE(equationCurves2.count(), 1);
	auto* eq2 = static_cast<XYEquationCurve2*>(equationCurves2.at(0));

	auto dock = XYEquationCurve2Dock(nullptr);
	dock.setupGeneral();
	dock.setCurves({eq2});

	eq2->setEquation(QStringLiteral("2*x"), {QStringLiteral("x")}, {equationCurve});

	{
		const auto* xColumn = eq2->xColumn();
		const auto* yColumn = eq2->yColumn();
		QCOMPARE(xColumn->rowCount(), data.count);
		QCOMPARE(yColumn->rowCount(), data.count);
		for (int i = 0; i < xColumn->rowCount(); i++) {
			VALUES_EQUAL(xColumn->valueAt(i), i + 1);
			VALUES_EQUAL(yColumn->valueAt(i), (i + 1) * 2);
		}
	}

	eq2->undoStack()->undo();

	QCOMPARE(eq2->xColumn()->rowCount(), 0);

	eq2->undoStack()->redo();

	{
		const auto* xColumn = eq2->xColumn();
		const auto* yColumn = eq2->yColumn();
		QCOMPARE(xColumn->rowCount(), data.count);
		QCOMPARE(yColumn->rowCount(), data.count);
		for (int i = 0; i < xColumn->rowCount(); i++) {
			VALUES_EQUAL(xColumn->valueAt(i), i + 1);
			VALUES_EQUAL(yColumn->valueAt(i), (i + 1) * 2);
		}
	}

	// Change data of equationCurve, equationCurve2 must update!
	data.expression1 = QStringLiteral("x^2");
	equationCurve->setEquationData(data);

	{
		const auto* xColumn = eq2->xColumn();
		const auto* yColumn = eq2->yColumn();
		QCOMPARE(xColumn->rowCount(), data.count);
		QCOMPARE(yColumn->rowCount(), data.count);
		for (int i = 0; i < xColumn->rowCount(); i++) {
			VALUES_EQUAL(xColumn->valueAt(i), i + 1);
			VALUES_EQUAL(yColumn->valueAt(i), 2 * (qPow(i + 1, 2)));
		}
	}
}

void XYEquationCurve2Test::removeCurves() {
	Project project;
	auto* ws = new Worksheet(QStringLiteral("Worksheet"));
	QVERIFY(ws != nullptr);
	project.addChild(ws);

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	p->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created
	QVERIFY(p != nullptr);
	ws->addChild(p);

	// Generate data and
	Spreadsheet* sheet = new Spreadsheet(QStringLiteral("Spreadsheet"), false);
	project.addChild(sheet);
	sheet->setColumnCount(2);
	sheet->setRowCount(11);
	sheet->column(0)->setColumnMode(AbstractColumn::ColumnMode::Double);
	sheet->column(1)->setColumnMode(AbstractColumn::ColumnMode::Double);

	for (int i = 0; i < sheet->rowCount(); i++) {
		sheet->column(0)->setValueAt(i, i);
		sheet->column(1)->setValueAt(i, 2 * i + 1);
	}

	auto* curve = new XYCurve(QStringLiteral("curve1"));
	p->addChild(curve);
	curve->setCoordinateSystemIndex(0);
	curve->setXColumn(sheet->column(0));
	curve->setYColumn(sheet->column(1));

	p->addChild(new XYEquationCurve2(QLatin1String("eq2")));
	auto equationCurves2 = p->children(AspectType::XYEquationCurve2);
	QCOMPARE(equationCurves2.count(), 1);
	auto* eq2 = static_cast<XYEquationCurve2*>(equationCurves2.at(0));

	auto dock = XYEquationCurve2Dock(nullptr);
	dock.setupGeneral();
	dock.setCurves({eq2});

	eq2->setEquation(QStringLiteral("2*x"), {QStringLiteral("x")}, {curve});

	{
		const auto* xColumn = eq2->xColumn();
		const auto* yColumn = eq2->yColumn();
		QCOMPARE(xColumn->rowCount(), 11);
		QCOMPARE(yColumn->rowCount(), 11);
		for (int i = 0; i < xColumn->rowCount(); i++) {
			VALUES_EQUAL(xColumn->valueAt(i), i);
			VALUES_EQUAL(yColumn->valueAt(i), 2 * (2 * i + 1));
		}
	}

	curve->remove();

	QCOMPARE(eq2->xColumn()->rowCount(), 0);
	QCOMPARE(eq2->equationData().count(), 1);
	QCOMPARE(eq2->equationData().at(0).curve(), nullptr);
	QCOMPARE(eq2->equationData().at(0).curvePath(), QStringLiteral("Project/Worksheet/plot/curve1"));
}
void XYEquationCurve2Test::removeColumnFromCurve() {
	Project project;
	auto* ws = new Worksheet(QStringLiteral("Worksheet"));
	QVERIFY(ws != nullptr);
	project.addChild(ws);

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	p->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created
	QVERIFY(p != nullptr);
	ws->addChild(p);

	p->addChild(new XYEquationCurve(QLatin1String("eq")));

	// Generate data and
	Spreadsheet* sheet = new Spreadsheet(QStringLiteral("Spreadsheet"), false);
	project.addChild(sheet);
	sheet->setColumnCount(2);
	sheet->setRowCount(11);
	sheet->column(0)->setColumnMode(AbstractColumn::ColumnMode::Double);
	sheet->column(1)->setColumnMode(AbstractColumn::ColumnMode::Double);

	for (int i = 0; i < sheet->rowCount(); i++) {
		sheet->column(0)->setValueAt(i, i);
		sheet->column(1)->setValueAt(i, 2 * i + 1);
	}

	auto* curve = new XYCurve(QStringLiteral("curve1"));
	p->addChild(curve);
	curve->setCoordinateSystemIndex(0);
	curve->setXColumn(sheet->column(0));
	curve->setYColumn(sheet->column(1));

	p->addChild(new XYEquationCurve2(QLatin1String("eq2")));
	auto equationCurves2 = p->children(AspectType::XYEquationCurve2);
	QCOMPARE(equationCurves2.count(), 1);
	auto* eq2 = static_cast<XYEquationCurve2*>(equationCurves2.at(0));

	auto dock = XYEquationCurve2Dock(nullptr);
	dock.setupGeneral();
	dock.setCurves({eq2});

	eq2->setEquation(QStringLiteral("2*x"), {QStringLiteral("x")}, {curve});

	sheet->column(1)->remove();

	QCOMPARE(eq2->xColumn()->rowCount(), 0);
}
void XYEquationCurve2Test::addColumnFromCurve() {
}
void XYEquationCurve2Test::removeCurveRenameAutomaticAdd() {
}
void XYEquationCurve2Test::saveLoad() {
}

QTEST_MAIN(XYEquationCurve2Test)
