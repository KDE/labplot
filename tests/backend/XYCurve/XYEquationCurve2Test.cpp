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
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
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
	data.min = QStringLiteral("0");
	data.max = QStringLiteral("10");
	data.type = XYEquationCurve::EquationType::Cartesian;
	equationCurve->setEquationData(data);

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
		const auto* yColumn = eq2->xColumn();
		QCOMPARE(xColumn->rowCount(), data.count);
		QCOMPARE(yColumn->rowCount(), data.count);
		for (int i = 0; i < xColumn->rowCount(); i++) {
			QCOMPARE(xColumn->valueAt(i), i);
			QCOMPARE(yColumn->valueAt(i), i * 2);
		}
	}

	eq2->undoStack()->undo();

	QCOMPARE(eq2->xColumn()->rowCount(), 0);

	eq2->undoStack()->redo();

	{
		const auto* xColumn = eq2->xColumn();
		const auto* yColumn = eq2->xColumn();
		QCOMPARE(xColumn->rowCount(), data.count);
		QCOMPARE(yColumn->rowCount(), data.count);
		for (int i = 0; i < xColumn->rowCount(); i++) {
			QCOMPARE(xColumn->valueAt(i), i);
			QCOMPARE(yColumn->valueAt(i), i * 2);
		}
	}
}
void XYEquationCurve2Test::removeCurves() {
}
void XYEquationCurve2Test::removeColumnFromCurve() {
}
void XYEquationCurve2Test::addColumnFromCurve() {
}
void XYEquationCurve2Test::removeCurveRenameAutomaticAdd() {
}
void XYEquationCurve2Test::saveLoad() {
}

QTEST_MAIN(XYEquationCurve2Test)
