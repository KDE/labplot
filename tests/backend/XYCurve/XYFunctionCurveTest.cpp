/*
	File                 : XYFunctionCurveTest.cpp
	Project              : LabPlot
	Description          : Tests for XYFunctionCurve
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Martin Marmsoler <martin.marmsoler@gmail.com>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "XYFunctionCurveTest.h"
#include "backend/core/Project.h"
#include "backend/core/column/Column.h"
#include "backend/datasources/filters/AsciiFilter.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/worksheet/plots/cartesian/XYCurve.h"
#include "backend/worksheet/plots/cartesian/XYEquationCurve.h"
#include "backend/worksheet/plots/cartesian/XYFunctionCurve.h"
#include "backend/worksheet/plots/cartesian/XYFunctionCurvePrivate.h"
#include "frontend/dockwidgets/XYFunctionCurveDock.h"

#include <QUndoStack>

void XYFunctionCurveTest::setCurves() {
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

	p->addChild(new XYFunctionCurve(QLatin1String("eq2")));
	auto functionCurves = p->children(AspectType::XYFunctionCurve);
	QCOMPARE(functionCurves.count(), 1);
	auto* functionCurve = static_cast<XYFunctionCurve*>(functionCurves.at(0));

	auto dock = XYFunctionCurveDock(nullptr);
	dock.setupGeneral();
	dock.setCurves({functionCurve});

	functionCurve->setFunction(QStringLiteral("2*x"), {QStringLiteral("x")}, {equationCurve});

	{
		const auto* xColumn = functionCurve->xColumn();
		const auto* yColumn = functionCurve->yColumn();
		QCOMPARE(xColumn->rowCount(), data.count);
		QCOMPARE(yColumn->rowCount(), data.count);
		for (int i = 0; i < xColumn->rowCount(); i++) {
			VALUES_EQUAL(xColumn->valueAt(i), i + 1.);
			VALUES_EQUAL(yColumn->valueAt(i), (i + 1.) * 2.);
		}
	}

	functionCurve->undoStack()->undo();

	QCOMPARE(functionCurve->xColumn()->rowCount(), 0);

	functionCurve->undoStack()->redo();

	{
		const auto* xColumn = functionCurve->xColumn();
		const auto* yColumn = functionCurve->yColumn();
		QCOMPARE(xColumn->rowCount(), data.count);
		QCOMPARE(yColumn->rowCount(), data.count);
		for (int i = 0; i < xColumn->rowCount(); i++) {
			VALUES_EQUAL(xColumn->valueAt(i), i + 1.);
			VALUES_EQUAL(yColumn->valueAt(i), (i + 1.) * 2.);
		}
	}

	// Change data of equationCurve, equationCurve2 must update!
	data.expression1 = QStringLiteral("x^2");
	equationCurve->setEquationData(data);

	{
		const auto* xColumn = functionCurve->xColumn();
		const auto* yColumn = functionCurve->yColumn();
		QCOMPARE(xColumn->rowCount(), data.count);
		QCOMPARE(yColumn->rowCount(), data.count);
		for (int i = 0; i < xColumn->rowCount(); i++) {
			VALUES_EQUAL(xColumn->valueAt(i), i + 1.);
			VALUES_EQUAL(yColumn->valueAt(i), 2 * (qPow(i + 1., 2.)));
		}
	}
}

void XYFunctionCurveTest::removeCurves() {
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

	p->addChild(new XYFunctionCurve(QLatin1String("eq2")));
	auto functionCurves = p->children(AspectType::XYFunctionCurve);
	QCOMPARE(functionCurves.count(), 1);
	auto* functionCurve = static_cast<XYFunctionCurve*>(functionCurves.at(0));

	auto dock = XYFunctionCurveDock(nullptr);
	dock.setupGeneral();
	dock.setCurves({functionCurve});

	functionCurve->setFunction(QStringLiteral("2*x"), {QStringLiteral("x")}, {curve});

	{
		const auto* xColumn = functionCurve->xColumn();
		const auto* yColumn = functionCurve->yColumn();
		QCOMPARE(xColumn->rowCount(), 11);
		QCOMPARE(yColumn->rowCount(), 11);
		for (int i = 0; i < xColumn->rowCount(); i++) {
			VALUES_EQUAL(xColumn->valueAt(i), (double)i);
			VALUES_EQUAL(yColumn->valueAt(i), 2 * (2 * i + 1.));
		}
	}

	curve->remove();

	QCOMPARE(functionCurve->xColumn()->rowCount(), 0);
	QCOMPARE(functionCurve->functionData().count(), 1);
	QCOMPARE(functionCurve->functionData().at(0).curve(), nullptr);
	QCOMPARE(functionCurve->functionData().at(0).curvePath(), i18n("Project") + QStringLiteral("/Worksheet/plot/curve1"));
}

void XYFunctionCurveTest::removeColumnFromCurve() {
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

	p->addChild(new XYFunctionCurve(QLatin1String("eq2")));
	auto functionCurves = p->children(AspectType::XYFunctionCurve);
	QCOMPARE(functionCurves.count(), 1);
	auto* functionCurve = static_cast<XYFunctionCurve*>(functionCurves.at(0));

	auto dock = XYFunctionCurveDock(nullptr);
	dock.setupGeneral();
	dock.setCurves({functionCurve});

	functionCurve->setFunction(QStringLiteral("2*x"), {QStringLiteral("x")}, {curve});

	sheet->column(1)->remove();

	QCOMPARE(functionCurve->xColumn()->rowCount(), 0);
}

void XYFunctionCurveTest::removeCurveRenameAutomaticAdd() {
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

	p->addChild(new XYFunctionCurve(QLatin1String("eq2")));
	auto functionCurves = p->children(AspectType::XYFunctionCurve);
	QCOMPARE(functionCurves.count(), 1);
	auto* functionCurve = static_cast<XYFunctionCurve*>(functionCurves.at(0));

	auto dock = XYFunctionCurveDock(nullptr);
	dock.setupGeneral();
	dock.setCurves({functionCurve});

	functionCurve->setFunction(QStringLiteral("2*x"), {QStringLiteral("x")}, {equationCurve});

	{
		const auto* xColumn = functionCurve->xColumn();
		const auto* yColumn = functionCurve->yColumn();
		QCOMPARE(xColumn->rowCount(), data.count);
		QCOMPARE(yColumn->rowCount(), data.count);
		for (int i = 0; i < xColumn->rowCount(); i++) {
			VALUES_EQUAL(xColumn->valueAt(i), i + 1.);
			VALUES_EQUAL(yColumn->valueAt(i), (i + 1.) * 2.);
		}
	}

	equationCurve->remove();

	QCOMPARE(functionCurve->functionData().at(0).curve(), nullptr);
	QCOMPARE(functionCurve->functionData().at(0).curvePath(), i18n("Project") + QStringLiteral("/Worksheet/plot/eq"));

	p->addChild(new XYEquationCurve(QLatin1String("eqDifferent"))); // different name than eq!
	auto eqs = p->children(AspectType::XYEquationCurve);
	QCOMPARE(eqs.count(), 1);
	auto* equationCurveNew = static_cast<XYEquationCurve*>(eqs.at(0));
	data.expression1 = QStringLiteral("x^2");
	equationCurveNew->setEquationData(data);

	QCOMPARE(functionCurve->functionData().at(0).curve(), nullptr);
	QCOMPARE(functionCurve->functionData().at(0).curvePath(), i18n("Project") + QStringLiteral("/Worksheet/plot/eq"));

	equationCurveNew->setName(QStringLiteral("eq"));

	QCOMPARE(functionCurve->functionData().at(0).curve(), equationCurveNew);
	QCOMPARE(functionCurve->functionData().at(0).curvePath(), i18n("Project") + QStringLiteral("/Worksheet/plot/eq"));

	{
		const auto* xColumn = functionCurve->xColumn();
		const auto* yColumn = functionCurve->yColumn();
		QCOMPARE(xColumn->rowCount(), data.count);
		QCOMPARE(yColumn->rowCount(), data.count);
		for (int i = 0; i < xColumn->rowCount(); i++) {
			VALUES_EQUAL(xColumn->valueAt(i), i + 1.);
			VALUES_EQUAL(yColumn->valueAt(i), 2 * qPow(i + 1., 2.));
		}
	}
}

void XYFunctionCurveTest::saveLoad() {
	QString savePath;
	{
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

		p->addChild(new XYFunctionCurve(QLatin1String("eq2")));
		auto functionCurves = p->children(AspectType::XYFunctionCurve);
		QCOMPARE(functionCurves.count(), 1);
		auto* functionCurve = static_cast<XYFunctionCurve*>(functionCurves.at(0));

		auto dock = XYFunctionCurveDock(nullptr);
		dock.setupGeneral();
		dock.setCurves({functionCurve});

		functionCurve->setFunction(QStringLiteral("2*z"), {QStringLiteral("z")}, {equationCurve});

		{
			const auto* xColumn = functionCurve->xColumn();
			const auto* yColumn = functionCurve->yColumn();
			QCOMPARE(xColumn->rowCount(), data.count);
			QCOMPARE(yColumn->rowCount(), data.count);
			for (int i = 0; i < xColumn->rowCount(); i++) {
				VALUES_EQUAL(xColumn->valueAt(i), i + 1.);
				VALUES_EQUAL(yColumn->valueAt(i), (i + 1.) * 2.);
			}
		}
		SAVE_PROJECT("TestXYFunctionCurveSaveLoad");
	}

	{
		Project project;
		QCOMPARE(project.load(savePath), true);

		const auto* ws = project.child<Worksheet>(0);
		QVERIFY(ws);
		const auto* p = ws->child<CartesianPlot>(0);
		QVERIFY(p);
		const auto* functionCurve = p->child<XYFunctionCurve>(0);
		QVERIFY(functionCurve);
		auto size = functionCurve->d_func()->m_logicalPoints.size();
		if (size < 100)
			DEBUG("WARNING: m_logicalPoints not restored yet. (" << size << " < 100)")
		const auto* eq = p->child<XYEquationCurve>(0);
		QVERIFY(eq);

		const auto& data = functionCurve->functionData();
		QCOMPARE(data.length(), 1);
		QCOMPARE(data.at(0).curvePath(), i18n("Project") + QStringLiteral("/Worksheet/plot/eq"));
		QCOMPARE(data.at(0).curve(), eq);
		QCOMPARE(data.at(0).variableName(), QStringLiteral("z"));

		QCOMPARE(functionCurve->function(), QStringLiteral("2*z"));

		{
			const auto* xColumn = functionCurve->xColumn();
			const auto* yColumn = functionCurve->yColumn();
			QVERIFY(xColumn);
			QVERIFY(yColumn);
			QCOMPARE(xColumn->rowCount(), 100);
			QCOMPARE(yColumn->rowCount(), 100);
			for (int i = 0; i < xColumn->rowCount(); i++) {
				VALUES_EQUAL(xColumn->valueAt(i), i + 1.);
				VALUES_EQUAL(yColumn->valueAt(i), (i + 1.) * 2.);
			}
		}

		// Check that the logical points are really set
		QCOMPARE(functionCurve->d_func()->m_logicalPoints.length(), 100);
	}
}

void XYFunctionCurveTest::importData() {
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

	QCOMPARE(sheet->column(0)->name(), QStringLiteral("1"));
	QCOMPARE(sheet->column(1)->name(), QStringLiteral("2"));

	auto* curve = new XYCurve(QStringLiteral("curve1"));
	p->addChild(curve);
	curve->setCoordinateSystemIndex(0);
	curve->setXColumn(sheet->column(0));
	curve->setYColumn(sheet->column(1));

	p->addChild(new XYFunctionCurve(QLatin1String("eq2")));
	auto functionCurves = p->children(AspectType::XYFunctionCurve);
	QCOMPARE(functionCurves.count(), 1);
	auto* functionCurve = static_cast<XYFunctionCurve*>(functionCurves.at(0));

	auto dock = XYFunctionCurveDock(nullptr);
	dock.setupGeneral();
	dock.setCurves({functionCurve});

	functionCurve->setFunction(QStringLiteral("2*x"), {QStringLiteral("x")}, {curve});

	{
		const auto* xColumn = functionCurve->xColumn();
		const auto* yColumn = functionCurve->yColumn();
		QCOMPARE(xColumn->rowCount(), 11);
		QCOMPARE(yColumn->rowCount(), 11);
		for (int i = 0; i < xColumn->rowCount(); i++) {
			VALUES_EQUAL(xColumn->valueAt(i), (double)i);
			VALUES_EQUAL(yColumn->valueAt(i), 2 * (2 * i + 1.));
		}
	}

	QStringList fileContent = {
		QStringLiteral("1,2"),
		QStringLiteral("7,99"),
		QStringLiteral("17,3"),
	};
	QString savePath;
	SAVE_FILE("testStartRowSkipRow", fileContent);

	AsciiFilter filter;
	filter.readDataFromFile(savePath, sheet, AbstractFileFilter::ImportMode::Replace);

	{
		const auto* xColumn = functionCurve->xColumn();
		const auto* yColumn = functionCurve->yColumn();
		QCOMPARE(xColumn->rowCount(), 2);
		QCOMPARE(yColumn->rowCount(), 2);
		VALUES_EQUAL(xColumn->valueAt(0), 7.0);
		VALUES_EQUAL(yColumn->valueAt(0), 99.0 * 2.0);
		VALUES_EQUAL(xColumn->valueAt(1), 17.0);
		VALUES_EQUAL(yColumn->valueAt(1), 3.0 * 2.0);
	}
}

void XYFunctionCurveTest::importDataComplexDependency() {
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

	QCOMPARE(sheet->column(0)->name(), QStringLiteral("1"));
	QCOMPARE(sheet->column(1)->name(), QStringLiteral("2"));

	auto* curve = new XYCurve(QStringLiteral("curve1"));
	p->addChild(curve);
	curve->setCoordinateSystemIndex(0);
	curve->setXColumn(sheet->column(0));
	curve->setYColumn(sheet->column(1));

	p->addChild(new XYFunctionCurve(QLatin1String("eq2")));
	auto functionCurves = p->children(AspectType::XYFunctionCurve);
	QCOMPARE(functionCurves.count(), 1);
	auto* functionCurve1 = static_cast<XYFunctionCurve*>(functionCurves.at(0));
	// First function curve check
	{
		auto dock = XYFunctionCurveDock(nullptr);
		dock.setupGeneral();
		dock.setCurves({functionCurve1});

		functionCurve1->setFunction(QStringLiteral("2*x"), {QStringLiteral("x")}, {curve});

		{
			const auto* xColumn = functionCurve1->xColumn();
			const auto* yColumn = functionCurve1->yColumn();
			QCOMPARE(xColumn->rowCount(), 11);
			QCOMPARE(yColumn->rowCount(), 11);
			for (int i = 0; i < xColumn->rowCount(); i++) {
				VALUES_EQUAL(xColumn->valueAt(i), (double)i);
				VALUES_EQUAL(yColumn->valueAt(i), 2 * (2 * i + 1.));
			}
		}
	}

	p->addChild(new XYFunctionCurve(QLatin1String("eq3")));
	functionCurves = p->children(AspectType::XYFunctionCurve);
	QCOMPARE(functionCurves.count(), 2);
	auto* functionCurve2 = static_cast<XYFunctionCurve*>(functionCurves.at(1));
	QCOMPARE(functionCurve2->name(), QLatin1String("eq3"));
	// First function curve check
	{
		auto dock = XYFunctionCurveDock(nullptr);
		dock.setupGeneral();
		dock.setCurves({functionCurve2});

		functionCurve2->setFunction(QStringLiteral("3*x + 5*y"), {QStringLiteral("x"), QStringLiteral("y")}, {functionCurve1, curve});

		{
			const auto* xColumn = functionCurve2->xColumn();
			const auto* yColumn = functionCurve2->yColumn();
			QCOMPARE(xColumn->rowCount(), 11);
			QCOMPARE(yColumn->rowCount(), 11);
			for (int i = 0; i < xColumn->rowCount(); i++) {
				VALUES_EQUAL(xColumn->valueAt(i), (double)i);
				const double valueCurve = (2. * i + 1.);
				const double valueEq2 = 2. * valueCurve;
				VALUES_EQUAL(yColumn->valueAt(i), 3. * valueEq2 + 5. * valueCurve);
			}
		}
	}

	QStringList fileContent = {
		QStringLiteral("1,2"),
		QStringLiteral("7,99"),
		QStringLiteral("17,3"),
	};
	QString savePath;
	SAVE_FILE("testStartRowSkipRow", fileContent);

	AsciiFilter filter;
	auto properties = filter.properties();
	properties.automaticSeparatorDetection = true;
	properties.headerEnabled = true;
	properties.headerLine = 1;
	properties.startRow = 1;
	properties.intAsDouble = false;
	filter.setProperties(properties);

	filter.readDataFromFile(savePath, sheet, AbstractFileFilter::ImportMode::Replace);

	{
		const auto* xColumn = functionCurve1->xColumn();
		const auto* yColumn = functionCurve1->yColumn();
		QCOMPARE(xColumn->rowCount(), 2);
		QCOMPARE(yColumn->rowCount(), 2);
		VALUES_EQUAL(xColumn->valueAt(0), 7.);
		VALUES_EQUAL(yColumn->valueAt(0), 99. * 2.);
		VALUES_EQUAL(xColumn->valueAt(1), 17.0);
		VALUES_EQUAL(yColumn->valueAt(1), 3. * 2.);
	}

	{
		const auto* xColumn = functionCurve2->xColumn();
		const auto* yColumn = functionCurve2->yColumn();
		QCOMPARE(xColumn->rowCount(), 2);
		QCOMPARE(yColumn->rowCount(), 2);
		VALUES_EQUAL(xColumn->valueAt(0), 7.);
		VALUES_EQUAL(yColumn->valueAt(0), 3. * (99. * 2.) + 5. * 99.);
		VALUES_EQUAL(xColumn->valueAt(1), 17.);
		VALUES_EQUAL(yColumn->valueAt(1), 3. * (3. * 2.) + 5. * 3.);
	}
}

QTEST_MAIN(XYFunctionCurveTest)
