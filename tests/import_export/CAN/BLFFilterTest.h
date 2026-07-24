/*
	File                 : BLFFilterTest.h
	Project              : LabPlot
	Description          : Tests for the BLF filter
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Martin Marmsoler <martin.marmsoler@gmail.com>

	SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef BLFFILTERTEST_H
#define BLFFILTERTEST_H

#include "../../CommonMetaTest.h"

namespace Vector {
namespace BLF {
struct CanMessage2;
} // namespace BLF
} // namespace Vector

class BLFFilterTest : public CommonMetaTest {
	Q_OBJECT

private Q_SLOTS:
#ifdef HAVE_VECTOR_BLF
	void testInvalidBLF();
	void testNotFoundBLF();
	void testInvalidDBC();

	void testTimeNative();

	void testValidBLFValidDBCSingleMessageBigEndian();
	void testUsePreviousValueBigEndian();
	void testUseNANBigEndian();

	void testUseNANLittleEndian();
	void testUsePreviousValueLittleEndian();
	void testBigNumberNotByteAlignedLittleEndian();

private:
	// Helper functions
	void createDBCFile(const QString& filename, const std::string& content);
	Vector::BLF::CanMessage2* createCANMessage(uint32_t id, uint64_t timestamp, const std::vector<uint8_t>& data);
	void createBLFFile(const QString& filename, const QVector<Vector::BLF::CanMessage2*>& messages);
#endif
};
#endif // BLFFILTERTEST_H
