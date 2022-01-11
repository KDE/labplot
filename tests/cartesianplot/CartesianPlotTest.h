/*
    File                 : CartesianPlotTest.h
    Project              : LabPlot
    Description          : Tests for cartesian plots
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2022 Stefan Gerlach <stefan.gerlach@uni.kn>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef CARTESIANPLOTTEST_H
#define CARTESIANPLOTTEST_H

#include <QtTest>

class CartesianPlotTest : public QObject {
	Q_OBJECT

private Q_SLOTS:
	void initTestCase();

	// change data in spreadsheet source
	void changeData1();

};
#endif
