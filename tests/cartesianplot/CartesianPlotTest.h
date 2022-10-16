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
	void changeData2();
	void changeData3();
	void changeData4();
	void changeData5();
	void changeData6();

	// check deleting curve
	void deleteCurveAutoscale();
	void deleteCurveNoAutoscale();

	void invisibleCurveAutoscale();
	void invisibleCurveNoAutoscale();

	void equationCurveEquationChangedAutoScale();
	void equationCurveEquationChangedNoAutoScale();

	void undoInfoElement();

	void axisFormat();
	void shiftLeftAutoScale();
	void shiftRightAutoScale();
	void shiftUpAutoScale();
	void shiftDownAutoScale();

	void rangeFormatYDataChanged();
	void rangeFormatXDataChanged();
};
#endif
