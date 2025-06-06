/*
	File                 : CartesianPlotTest.h
	Project              : LabPlot
	Description          : Tests for cartesian plots
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-FileCopyrightText: 2022-2025 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef CARTESIANPLOTTEST_H
#define CARTESIANPLOTTEST_H

#include "../CommonTest.h"

class CartesianPlotTest : public CommonTest {
	Q_OBJECT

private Q_SLOTS:
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

	// initialize and add a child, undo and redo, save and load
	void infoElementInit();
	void insetPlotInit();
	void insetPlotSaveLoad();

	void axisFormat();
	void shiftLeftAutoScale();
	void shiftRightAutoScale();
	void shiftUpAutoScale();
	void shiftDownAutoScale();

	void rangeFormatYDataChanged();
	void rangeFormatXDataChanged();
	void rangeFormatNonDefaultRange();

	void invalidcSystem();

	void invalidStartValueLogScaling();

	void autoScaleFitCurveCalculation();

	void wheelEventCenterAxes();
	void wheelEventNotCenter();

	void wheelEventOutsideTopLeft();
	void wheelEventOutsideBottomRight();

	// checks after modifications in/on spreadsheet
	void spreadsheetRemoveRows();
	void spreadsheetInsertRows();
	void columnRemove();
	void columnRemoveSaveLoadRestore();
	void spreadsheetRemove();

	// handling of z-values on changes in the child hierarchy
	void zValueAfterAddMoveRemove();
};
#endif
