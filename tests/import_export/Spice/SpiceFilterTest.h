/*
	File                 : AsciiFilterTest.h
	Project              : LabPlot
	Description          : Tests for the ascii filter
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2017 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2024 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef SPICEFILTERTEST_H
#define SPICEFILTERTEST_H

#include "../../CommonMetaTest.h"

class SpiceFilterTest : public CommonMetaTest {
	Q_OBJECT

private Q_SLOTS:
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
	void DCTransfer();

	void FFT_From_TransientAnalysis();
};
#endif // SPICEFILTERTEST_H
