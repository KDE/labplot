/*
	File                 : AsciiFilterTest.h
	Project              : LabPlot
	Description          : Tests for the ascii filter
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2017-2023 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2022 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef ASCIIFILTERTEST_H
#define ASCIIFILTERTEST_H

#include "../../CommonTest.h"
#include <QtTest>

class AsciiFilterTest : public CommonTest {
	Q_OBJECT

private Q_SLOTS:
	void initTestCase();

	// empty and sparse files
	void testEmptyFileAppend();
	void testEmptyFilePrepend();
	void testEmptyFileReplace();

	void testEmptyLines01();

	// Might happen, that not all columns are filled
	void testSparseFile01();
	void testSparseFile02();
	void testSparseFile03();

	// header handling
	void testHeaderDisabled();
	void testHeaderDisabledNoHeaderNames();
	void testHeaderEnabled();
	void testHeaderDisabledImportDefaultNameColumn2();
	void testHeaderDisabled2ColumnsImported3HeaderNames();
	void testHeaderEnabledCommenCharacter();
	void testHeaderEnabledStartRow();
	void testHeaderEnabledSpaces();
	void testHeaderDisabledDuplicateName();
	void testHeaderEnabledDuplicateName();
	void testHeaderEnabledColumnNaming();
	void testHeaderEnabledColumnNaming2();
	void testHeaderEnabledHeaderLine();
	void testHeaderEnabledStartRowEqualHeaderRow();

	// read ranges
	void testColumnRange00();
	void testColumnRange01();
	void testColumnRange02();
	void testColumnRange03();
	void testColumnRange04();
	void testColumnRange05();
	void testColumnRange06();

	void testRowRange00();
	void testRowRange01();
	void testRowRange02();

	void testRowColumnRange00();

	// different separators

	// qouted strings
	void testQuotedStrings00();
	void testQuotedStrings01();
	void testQuotedStrings02();
	void testQuotedStrings03();
	void testQuotedStrings04();
	void testQuotedStrings05();

	// different locales
	void testUtf8Cyrillic();

	// handling of NANs

	// automatically skip comments
	void testComments00();
	void testComments01();
	void testComments02();

	// datetime data
	void testDateTime00();
	void testDateTime01();
	void testDateTimeHex();

	// matrix import
	void testMatrixHeader();

	// check updates in the dependent objects after the data was modified by the import
	void spreadsheetFormulaUpdateAfterImport();
	void spreadsheetFormulaUpdateAfterImportWithColumnRestore();
	void plotUpdateAfterImport();
	void plotUpdateAfterImportWithColumnRestore();
	void plotUpdateAfterImportWithColumnRenaming();
	void plotUpdateAfterImportWithColumnRemove();

	// benchmarks
	void benchDoubleImport_data();
	// this is called multiple times (warm-up of BENCHMARK)
	// see https://stackoverflow.com/questions/36916962/qtest-executes-test-case-twic
	void benchDoubleImport();
	void benchDoubleImport_cleanup(); // delete data

	void testStartRow2();

	void testCreateIndex();

private:
	QString benchDataFileName;
	const size_t lines = 1e5;
	static const int paths = 5;
};
#endif
