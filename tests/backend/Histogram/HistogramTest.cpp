/*
	File                 : HistogramTest.cpp
	Project              : LabPlot
	Description          : Tests for Histogram
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Martin Marmsoler <martin.marmsoler@gmail.com>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "HistogramTest.h"
#include "backend/core/Project.h"
#include "backend/core/column/Column.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/Histogram.h"

void HistogramTest::testColumnRemoved() {
	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	project.addChild(ws);

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	ws->addChild(p);

	auto* histogram = new Histogram(QStringLiteral("histogram"));
	p->addChild(histogram);

	auto* c = new Column(QStringLiteral("TestColumn"));
	project.addChild(c);

	histogram->setDataColumn(c);
	c->setName(QStringLiteral("NewName"));
	QCOMPARE(histogram->dataColumnPath(), QStringLiteral("Project/NewName"));

	c->remove();

	QCOMPARE(histogram->dataColumn(), nullptr);
	QCOMPARE(histogram->dataColumnPath(), QStringLiteral("Project/NewName"));

	c->setName(QStringLiteral("Another new name")); // Shall not lead to a crash

	QCOMPARE(histogram->dataColumn(), nullptr);
	QCOMPARE(histogram->dataColumnPath(), QStringLiteral("Project/NewName"));
}

QTEST_MAIN(HistogramTest)
