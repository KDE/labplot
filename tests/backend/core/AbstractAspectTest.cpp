/*
	File                 : AbstractAspectTest.cpp
	Project              : LabPlot
	Description          : Tests for AbstractAspect
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022 Martin Marmsoler <martin.marmsoler@gmail.com>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "AbstractAspectTest.h"
#include "backend/core/Project.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/worksheet/plots/cartesian/XYEquationCurve.h"

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
		//				qDebug() << "i: " << i;
		//				qDebug() << childrenWorksheet1.at(i)->name() << ", type: " << AbstractAspect::typeName(childrenWorksheet1.at(i)->type())
		//						 << ", path: " << childrenWorksheet1.at(i)->path();
		//				qDebug() << childrenWorksheet2.at(i)->name() << ", type: " << AbstractAspect::typeName(childrenWorksheet2.at(i)->type())
		//						 << ", path: " << childrenWorksheet2.at(i)->path();
		QVERIFY(childrenWorksheet1.at(i)->type() == childrenWorksheet2.at(i)->type());
		if (childrenWorksheet1.at(i)->type() == AspectType::AbstractAspect)
			continue; // unique will change the triggered for those aspects, and therefore when changing for the second from "1" to "2" it happens that "2"
					  // already exists and therefore it receives "3"
		QVERIFY(childrenWorksheet1.at(i)->name() == childrenWorksheet2.at(i)->name());
		QVERIFY(childrenWorksheet1.at(i)->uuid() != childrenWorksheet2.at(i)->uuid());
	}
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
	SAVE_PROJECT("testLinkSpreadsheetSaveLoad")

	Project project2;
	QCOMPARE(project2.load(savePath), true);

	const auto& childrenProject1 =
		project.children(AspectType::AbstractAspect, {AbstractAspect::ChildIndexFlag::IncludeHidden, AbstractAspect::ChildIndexFlag::Recursive});
	const auto& childrenProject2 =
		project2.children(AspectType::AbstractAspect, {AbstractAspect::ChildIndexFlag::IncludeHidden, AbstractAspect::ChildIndexFlag::Recursive});

	QCOMPARE(childrenProject1.count(), childrenProject2.count());

	for (int i = 0; i < childrenProject1.count(); i++) {
		//				qDebug() << "i: " << i;
		//				qDebug() << childrenProject1.at(i)->name() << ", type: " << AbstractAspect::typeName(childrenProject1.at(i)->type())
		//						 << ", path: " << childrenProject1.at(i)->path() << ", UUid: " << childrenProject1.at(i)->uuid();
		//				qDebug() << childrenProject2.at(i)->name() << ", type: " << AbstractAspect::typeName(childrenProject2.at(i)->type())
		//						 << ", path: " << childrenProject2.at(i)->path() << ", UUid: " << childrenProject2.at(i)->uuid();
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

QTEST_MAIN(AbstractAspectTest)
