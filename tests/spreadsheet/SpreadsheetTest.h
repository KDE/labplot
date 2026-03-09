/*
	File                 : SpreadsheetTest.h
	Project              : LabPlot
	Description          : Tests for the Spreadsheet
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2020-2025 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2022 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SPREADSHEETTEST_H
#define SPREADSHEETTEST_H

#include "../CommonMetaTest.h"

class Spreadsheet;

class SpreadsheetTest : public CommonMetaTest {
	Q_OBJECT

private Q_SLOTS:
	// copy and paste

	// handling of different column modes
	void testCopyPasteColumnMode00();
	void testCopyPasteColumnMode01();
	void testCopyPasteColumnMode02();
	void testCopyPasteColumnMode03();
	void testCopyPasteColumnMode04();
	void testCopyPasteColumnMode05();
	void testCopyPasteColumnMode06();
	void testCopyPasteColumnMode07();

	// handling of spreadsheet size changes
	void testCopyPasteSizeChange00();
	void testCopyPasteSizeChange01();

	void testCopyPasteUtf8();

	// sorting
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

	// drop/mask
	void testRemoveRowsWithMissingValues();
	void testMaskRowsWithMissingValues();

	// flattening
	void testFlatten00();
	void testFlatten01();
	void testFlatten02();
	void testFlatten03();

	// transposing
	void testTranspose00();
	void testTranspose01();
	void testTranspose02();

	// search&replace
	void testSearchSimple00();

	void testSearchExtended00();
	void testSearchExtended01();
	void testSearchExtended02();
	void testSearchExtended03();
	void testSearchFindAll();

	void testSearchReplaceNumeric();
	void testSearchReplaceText();
	void testSearchReplaceAll();

	// size changes
	void testInsertRows();
	void testRemoveRows();
	void testInsertColumns();
	void testRemoveColumns();
	void testRemoveColumns2();

	void testInsertRowsBegin(); // insert row at the beginning
	void testRemoveRowsBegin(); // remove first row

	void testInsertRowsSuppressUpdate();
	void testInsertColumnsSuppressUpdate();

	void testLinkSpreadsheetsUndoRedo();
	void testLinkSpreadsheetDeleteAdd();
	void testLinkSpreadsheetAddRow();
	void testLinkSpreadsheetRemoveRow();
	void testLinkSpreadsheetRecalculate();
	void testLinkSpreadsheetRecalculateRowCountChange();
	void testLinkSpreadsheetSaveLoad();
	void testLinkSpreadsheetsModelDockUpdateCheckInsertRows();
	void testLinkSpreadsheetsModelDockUpdateCheckRemoveRows();

	// statistics spreadsheet
	void testStatisticsSpreadsheetToggle();
	void testStatisticsSpreadsheetChangeMetrics();
	void testStatisticsSpreadsheetChildIndex();
	void testStatisticsSpreadsheetChildIndexAfterUndoRedo();

#ifdef HAVE_VECTOR_BLF
	void testLinkSpreadSheetImportBLF();
#endif // HAVE_VECTOR_BLF

	void testNaming();

	void testClearColumns();

private:
	Spreadsheet* createSearchReplaceSpreadsheet();
};

#endif
