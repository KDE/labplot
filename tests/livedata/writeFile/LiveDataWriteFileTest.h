/*
	File                 : LiveDataWriteFileTest.cpp
	Project              : LabPlot
	Description          : Tests for Livedata with files
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022 Martin Marmsoler <martin.marmsoler@gmail.com>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef LIVEDATAWRITEFILETEST_H
#define LIVEDATAWRITEFILETEST_H

#include "tests/CommonTest.h"

class LiveDataWriteFileTest : public CommonTest {
	Q_OBJECT

private Q_SLOTS:
	void testRefresh();
};
#endif // LIVEDATAWRITEFILETEST_H
