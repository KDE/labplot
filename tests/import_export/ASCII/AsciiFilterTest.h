/*
    File                 : AsciiFilterTest.h
    Project              : LabPlot
    Description          : Tests for the ascii filter
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2017 Alexander Semke <alexander.semke@web.de>
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
	//empty and sparse files
	void testEmptyFileAppend();
	void testEmptyFilePrepend();
	void testEmptyFileReplace();

	void testEmptyLines01();

	void testSparseFile01();
	void testSparseFile02();
	void testSparseFile03();

	//header handling
	void testHeader01();
	void testHeader02();
	void testHeader03();
	void testHeader04();
	void testHeader05();
	void testHeader06();
	void testHeader07();
	void testHeader07a();
	void testHeader08();

	//read ranges
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

	//different separators

	//qouted strings
	void testQuotedStrings00();
	void testQuotedStrings01();
	void testQuotedStrings02();
	void testQuotedStrings03();

	//different locales

	//handling of NANs

	//automatically skip comments
	void testComments00();
	void testComments01();
	void testComments02();

	//datetime data
	void testDateTime00();
	void testDateTimeHex();

// benchmarks

	void benchDoubleImport_data();
	// this is called multiple times (warm-up of BENCHMARK)
	// see https://stackoverflow.com/questions/36916962/qtest-executes-test-case-twic
	void benchDoubleImport();
	void benchDoubleImport_cleanup();	// delete data

private:
	QString benchDataFileName;
	const size_t lines = 1e6;
	static const int paths = 5;
};
#endif
