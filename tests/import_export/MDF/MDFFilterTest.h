/*
	File                 : MDFFilterTest.h
	Project              : LabPlot
	Description          : Tests for the MDF I/O-filter.
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022 Martin Marmsoler <martin.marmsoler@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MDFFILTERTEST_H
#define MDFFILTERTEST_H

#include "../../CommonTest.h"
#include <QtTest>

class MDFFilterTest : public CommonTest {
	Q_OBJECT

private Q_SLOTS:
    void testInvalidFile();
    void testValid();
};

#endif // MDFFILTERTEST_H
