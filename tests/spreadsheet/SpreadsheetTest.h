/***************************************************************************
    File                 : SpreadsheetTest.h
    Project              : LabPlot
    Description          : Tests for the Spreadsheet
    --------------------------------------------------------------------
    Copyright            : (C) 2020 Alexander Semke (alexander.semke@web.de)
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/

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
