/*
    File                 : ProjectImportTest.h
    Project              : LabPlot
    Description          : Tests for project imports
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2018 Alexander Semke (alexander.semke@web.de)
*/

/***************************************************************************
 *                                                                         *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/
#ifndef PROJECTIMPORTTEST_H
#define PROJECTIMPORTTEST_H

#include <QtTest>

class MultiRangeTest : public QObject {
	Q_OBJECT

private slots:
	void initTestCase();

	//import of LabPlot projects
	void testApplyActionToSelection_CurveSelected_ZoomSelection();

	void testZoomSelection_SingleRange();
	void testZoomXSelection_AllRanges();
	void testZoomXSelection_SingleRange();
	void testZoomYSelection_AllRanges();
	void testZoomYSelection_SingleRange();
};
#endif
