/*
	File                 : AsciiFilterTest.h
	Project              : LabPlot
	Description          : Tests for the ascii filter
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2017-2023 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2022-2024 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef ASCIIFILTERTEST_H
#define ASCIIFILTERTEST_H

#include "../../CommonMetaTest.h"

class AsciiFilterTest : public CommonMetaTest {
	Q_OBJECT

private Q_SLOTS:
	void initialization();
	void lineCount();

	// column modes
	void read_HeaderEnabled_tooLessColumnModes();
	void read_HeaderEnabled_tooManyColumnModes();

	void read_HeaderDisabled_tooLessColumnModes();
	void read_HeaderDisabled_tooManyColumnModes();

	void read_HeaderDisabled_NotMatchingImport();
	void read_HeaderDisabled_tooLessColumnNames();

	void singleColumn();
	void singleColumnSimplifyWhitespaceEnabled();

	// empty and sparse files
	void testEmptyFileAppend();
	void testEmptyFilePrepend();
	void testEmptyFileReplace();

	void testEmptyLines01();

	void testSparseFile01();
	void testSparseFile02();
	void testSparseFile03();

	void testFileEndingWithoutLinebreak();

	// header handling
	void testHeaderDisabled();
	void intAsDouble();
	void testFirstLineHeader();
	void testMissingParts();
	void testMissingPartsSkip();
	void testImportSingleColumn();
	void commaSeparatedWhiteSpace();
	void tooManyHeaders();
	void testHeaderLine2DataLine4();
	void testHeaderLine2DataLine4_CommentLine();
	// void testHeader08();
	void testHeaderDuplicateNames_HeaderDisabled();
	void testHeaderDuplicateNames_HeaderEnabled();
	void testHeaderTwoImportsReverseColumnNames();
	void testHeader11a();
	void testHeader12();

	// read ranges
	void testColumnRange00();
	void testCreateIndex();
	void testCreateIndexAndTimestamp();
	void testStartColumn();
	void testStartColumn_IndexColumn();
	void testLastColumnOnly();
	void testWrongColumnRange_StartLargerEnd();
	void testWrongColumnRange();
	void testWrongColumnRange_IndexColumn();

	void testRowRange00();
	void testRowRange_EndRowLargerThanContent();
	void testRowRange02();

	void testRowColumnRange00();

	// different separators
	void testSpaceSeparator();
	void testSpaceSeparatorSimplifyWhiteSpace();

	// qouted strings
	void testQuotedStrings00();
	void testQuotedStrings01();
	void testQuotedStrings02();
	void testQuotedStrings03();
	void testQuotedStrings04();
	void testIvalidFile_Json();

	// different locales and encodings
	void testUtf8Cyrillic();
	void testUtf16NotSupported();

	// handling of NANs

	// automatically skip comments
	void testMultilineComment();
	void testComments01();
	void testComments02();

	// datetime data
	void testDateTime00();
	void testDateTimeDefaultDateTimeFormat();
	void testDateTimeAutodetect();
	void testDateTimeHex();

	// matrix import
	void testMatrixHeader();

	void testAppendRows();
	void keepLast();

	void testAppendColumns();
	void testPrependColumns();

	void testCommaAsDecimalSeparator();

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
	// see https://stackoverflow.com/questions/36916962/qtest-executes-test-case-twice
	void benchDoubleImport();
	void benchDoubleImport_cleanup(); // delete data
	void benchMarkCompare_SimplifyWhiteSpace();

	void determineSeparator();
	void determineColumns();
	void determineColumnsWhiteSpaces();

	void deleteSpreadsheet();
	void saveLoad();

	void bufferReader();

private:
	QString benchDataFileName;
	const size_t lines = 1e5;
	static const int paths = 5;
};
#endif
