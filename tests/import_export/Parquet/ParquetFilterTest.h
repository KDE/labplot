/*
	File                 : ParquetFilterTest.h
	Project              : LabPlot
	Description          : Tests for the Parquet/Arrow IPC/ORC import filter
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2026 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef PARQUETFILTERTEST_H
#define PARQUETFILTERTEST_H

#include "../../CommonMetaTest.h"

class ParquetFilterTest : public CommonMetaTest {
	Q_OBJECT

private Q_SLOTS:
	// Parquet import
	void testParquetBasicImport();
	void testParquetTypes();
	void testParquetNulls();
	void testParquetEmpty();
	void testParquetRowRange();
	void testParquetColumnRange();
	void testParquetPreview();

	// Arrow IPC import
	void testArrowIPCBasicImport();

	// ORC import
	void testORCBasicImport();
};

#endif
