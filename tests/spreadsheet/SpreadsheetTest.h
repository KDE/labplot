/*
    File                 : SpreadsheetTest.h
    Project              : LabPlot
    Description          : Tests for the Spreadsheet
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2020 Alexander Semke (alexander.semke@web.de)

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SPREADSHEETTEST_H
#define SPREADSHEETTEST_H

#include <QtTest>

class SpreadsheetTest : public QObject {
	Q_OBJECT

private slots:
	void initTestCase();

	//copy and paste

	//handling of different column modes
	void testCopyPasteColumnMode00();
	void testCopyPasteColumnMode01();
	void testCopyPasteColumnMode02();
	void testCopyPasteColumnMode03();
	void testCopyPasteColumnMode04();
	void testCopyPasteColumnMode05();

	//handling of spreadsheet size changes
	void testCopyPasteSizeChange00();
	void testCopyPasteSizeChange01();

	// sorting tests
	void testSortSingleNumeric1();
	void testSortSingleNumeric2();
	void testSortSingleInteger1();
	void testSortSingleInteger2();
	void testSortSingleBigInt1();
	void testSortSingleBigInt2();
	void testSortSingleText1();
	void testSortSingleText2();
	void testSortSingleDateTime1();
	void testSortSingleDateTime2();

	void testSortNumeric1();
	void testSortNumeric2();
	void testSortInteger1();
	void testSortInteger2();
	void testSortBigInt1();
	void testSortBigInt2();
	void testSortText1();
	void testSortText2();
	void testSortDateTime1();
	void testSortDateTime2();

	void testSortPerformanceNumeric1();
	void testSortPerformanceNumeric2();
};

#endif
