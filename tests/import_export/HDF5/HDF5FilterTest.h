/*
	File                 : HDF5FilterTest.h
	Project              : LabPlot
	Description          : Tests for the HDF5 I/O-filter.
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2021-2022 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef HDF5FILTERTEST_H
#define HDF5FILTERTEST_H

#include "../../CommonTest.h"

class HDF5FilterTest : public CommonTest {
	Q_OBJECT

private Q_SLOTS:
	void testImportDouble();
	void testImportDoublePortion();
	void testImportInt();
	void testImportIntPortion();
	void testImportVLEN();
	void testImportVLENPortion();

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
