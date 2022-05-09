/*
	File                 : FITSFilterTest.h
	Project              : LabPlot
	Description          : Tests for the FITS filter
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef FITSFILTERTEST_H
#define FITSFILTERTEST_H

#include "../../CommonTest.h"
#include <QtTest>

class FITSFilterTest : public CommonTest {
	Q_OBJECT

private Q_SLOTS:
	void importFile1();

	/*	void benchDoubleImport_data();
		// this is called multiple times (warm-up of BENCHMARK)
		// see https://stackoverflow.com/questions/36916962/qtest-executes-test-case-twic
		void benchDoubleImport();
		void benchDoubleImport_cleanup();	// delete data
	*/
private:
	QString benchDataFileName;
	const size_t lines = 1e6;
	static const int paths = 5;
};
#endif
