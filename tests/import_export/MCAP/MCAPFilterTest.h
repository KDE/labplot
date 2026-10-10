/*
	File                 : MCAPFilterTest.h
	Project              : LabPlot
	Description          : Tests for the JSON I/O-filter.
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2018 Andrey Cygankov <craftplace.ms@gmail.com>
	SPDX-FileCopyrightText: 2024 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MCAPFILTERTEST_H
#define MCAPFILTERTEST_H

#include "../../CommonTest.h"

class MCAPFilterTest : public CommonTest {
	Q_OBJECT

private Q_SLOTS:
	void testArrayImport();
	void testExport();
	void testImportWithoutValidTopics();
	void testImportWrongFile();
};

#endif
