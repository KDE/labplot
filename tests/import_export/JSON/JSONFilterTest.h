/*
    File                 : JSONFilterTest.h
    Project              : LabPlot
    Description          : Tests for the JSON I/O-filter.
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2018 Andrey Cygankov <craftplace.ms@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef JSONFILTERTEST_H
#define JSONFILTERTEST_H

#include <QtTest>

class JSONFilterTest : public QObject {
	Q_OBJECT

private Q_SLOTS:
	void initTestCase();

	void testArrayImport();
	void testObjectImport01();
	void testObjectImport02();
	void testObjectImport03();
	void testObjectImport04();
};


#endif
