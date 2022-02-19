/*
	File                 : TextLabelTest.cpp
	Project              : LabPlot
	Description          : Tests for TextLabel
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "TextLabelTest.h"
#include "backend/worksheet/TextLabel.h"
#include "backend/worksheet/TextLabelPrivate.h"
#include "backend/core/Project.h"
#include "backend/lib/trace.h"

void TextLabelTest::addPlot() {
	Project project;
	auto* ws = new Worksheet("worksheet");
	QVERIFY(ws != nullptr);
	project.addChild(ws);

	auto* p = new CartesianPlot("plot");
	QVERIFY(p != nullptr);
	ws->addChild(p);

	auto *l = new TextLabel("Label");
	QVERIFY(l != nullptr);
	l->setText(QString("TextLabelText"));
	ws->addChild(l);

	QCOMPARE(l->text().mode, TextLabel::Mode::Text);
	QCOMPARE(l->text().text.contains(QLatin1String("Text")), true);
	QCOMPARE(l->text().text.contains(QLatin1String(" color:")), true);
	QCOMPARE(l->text().text.contains(QLatin1String(" background-color:")), true);

	QCOMPARE(l->fontColor(), Qt::black);
	QCOMPARE(l->backgroundColor(), Qt::white);

	// add title?
	
	// add axes?
	// check axis label
}

QTEST_MAIN(TextLabelTest)
