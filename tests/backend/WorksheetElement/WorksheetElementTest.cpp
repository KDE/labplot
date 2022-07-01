/*
	File                 : XYCurveTest.cpp
	Project              : LabPlot
	Description          : Tests for XYCurve
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2021 Martin Marmsoler <martin.marmsoler@gmail.com>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "WorksheetElementTest.h"
#include "backend/core/Project.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/worksheet/plots/PlotArea.h"

#define private public
#define protected public
// To be able to access private members of the WorksheetElement
#include "backend/worksheet/WorksheetElement.h"
#include "backend/worksheet/plots/cartesian/ReferenceRange.h"
#undef protected
#undef private

#include "backend/worksheet/plots/cartesian/ReferenceRangePrivate.h"


#include "QGraphicsItem"

void WorksheetElementTest::referenceRange() {
	Project project;
	auto* ws = new Worksheet("worksheet");
	QVERIFY(ws != nullptr);
	project.addChild(ws);

	auto* p = new CartesianPlot("plot");
	QVERIFY(p != nullptr);
	ws->addChild(p);

	auto* rr = new ReferenceRange(p, "reference range");
	QVERIFY(rr != nullptr);
	p->addChild(rr);

	QDEBUG("Plot rect: " << rr->parentRect());

	auto rrPrivate = rr->d_func();
	//QCOMPARE(rr->cSystem, p->coordinateSystem(0));
	QVERIFY(rrPrivate != nullptr);

	// Calculation in init function of the reference range
	// To check if the values get not changed during initialization
	// 1/2 - 0.1/2 = 0.45
	QCOMPARE(rrPrivate->positionLogical, QPointF(0.5, 0.5));
	QCOMPARE(rrPrivate->positionLogicalStart, QPointF(0.45, 0.45));
	// 1/2 + 0.1/2 = 0.55
	QCOMPARE(rrPrivate->positionLogicalEnd, QPointF(0.55, 0.55));

	QPointF currPos = rrPrivate->position.point;
	auto logicalPos = p->coordinateSystem(0)->mapSceneToLogical(currPos);
	QCOMPARE(logicalPos, QPointF(0.5, 0.5));
	QCOMPARE(rrPrivate->prevPositionLogical, logicalPos);

	auto newLogicalPos = QPointF(0.7, 0.5);
	auto newPos = p->coordinateSystem(0)->mapLogicalToScene({newLogicalPos}).at(0);
	auto change = QGraphicsItem::ItemPositionChange;
	rrPrivate->itemChange(change, newPos);

	QCOMPARE(rrPrivate->positionLogical, QPointF(0.7, 0.5));
	QCOMPARE(rrPrivate->positionLogicalStart, QPointF(0.65, 0.65));
	QCOMPARE(rrPrivate->positionLogicalEnd, QPointF(0.75, 0.75));
}


QTEST_MAIN(WorksheetElementTest)
