/*
	File                 : BinaryFilterTest.h
	Project              : LabPlot
	Description          : Tests for the binary filter
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef BINARYFILTERTEST_H
#define BINARYFILTERTEST_H

#include "../../CommonTest.h"

class BinaryFilterTest : public CommonTest {
	Q_OBJECT

private Q_SLOTS:
	void importInt8();
	void importInt16BE();
	void importInt32LE();
	void importInt32BE();
	void importInt64BE();

	void importFloatBE();
	void importDoubleBE();
	void importDoubleLE();

	void importDoubleMatrixBE();

	void benchIntImport_data();
	// this is called multiple times (warm-up of BENCHMARK)
	// see https://stackoverflow.com/questions/36916962/qtest-executes-test-case-twic
	void benchIntImport();
	void benchIntImport_cleanup(); // delete data

	void benchDoubleImport_data();
	// this is called multiple times (warm-up of BENCHMARK)
	// see https://stackoverflow.com/questions/36916962/qtest-executes-test-case-twic
	void benchDoubleImport();
	void benchDoubleImport_cleanup(); // delete data
private:
	QString benchDataFileName;
	const size_t lines = 1e6;
	static const int paths = 5;
};
#endif
