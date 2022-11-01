/*
	File                 : FormulaTest.h
	Project              : LabPlot
	Description          : Tests for formula in spreadsheet
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FORMULATEST_H
#define FORMULATEST_H

#include <QtTest>

class FormulaTest : public QObject {
	Q_OBJECT

private Q_SLOTS:
	void initTestCase();

	void formula1();
};

#endif
