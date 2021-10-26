/*
    File                 : ProjectImportTest.h
    Project              : LabPlot
    Description          : Tests for project imports
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2018 Alexander Semke <alexander.semke@web.de>
    SPDX-FileCopyrightText: 2021 Stefan Gerlach <stefan.gerlach@uni.kn>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef PROJECTIMPORTTEST_H
#define PROJECTIMPORTTEST_H

#include <QtTest>

class MultiRangeTest : public QObject {
	Q_OBJECT

private slots:
	void initTestCase();

	//import of LabPlot projects
	void applyActionToSelection_CurveSelected_ZoomSelection();

	void zoomXSelection_AllRanges();
	void zoomXSelection_SingleRange();
	void zoomYSelection_AllRanges();
	void zoomYSelection_SingleRange();
	void zoomSelection_AllRanges();
	void zoomSelection_SingleRange();
};
#endif
