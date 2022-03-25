/*
    File                 : AsciiFilterTest.h
    Project              : LabPlot
    Description          : Tests for the ascii filter
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2017 Alexander Semke <alexander.semke@web.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef SPICEFILTERTEST_H
#define SPICEFILTERTEST_H

#include <QtTest>

class SpiceFilterTest : public QObject {
	Q_OBJECT

private Q_SLOTS:
	void initTestCase();

	void NgSpiceAsciiFileToBinaryFilter();
	void NgSpiceBinaryFileToAsciiFilter();

	// StartRow = 0
	void NgSpiceDCAscii();
	void NgSpiceDCBinary();
	void NgSpiceACAscii();
	void NgSpiceACBinary();

	void NgSpiceDCAsciiStartRowNotZero();
	void NgSpiceDCBinaryStartRowNotZero();
	void NgSpiceACAsciiStartRowNotZero();
	void NgSpiceACBinaryStartRowNotZero();

	void NgSpiceDCBinaryBulkReadNumberLines();
	void NgSpiceACBinaryBulkReadNumberLines();

	void LtSpiceACBinary();
	void LtSpiceTranBinary();
	void LtSpiceTranDoubleBinary();

	void LtSpiceWakeup();
};
#endif // SPICEFILTERTEST_H
