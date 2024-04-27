/*
	File                 : LiveDataTest.cpp
	Project              : LabPlot
	Description          : Tests for reading live data from files
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef LIVEDATATEST_H
#define LIVEDATATEST_H

#include "tests/CommonTest.h"

class LiveDataTest : public CommonTest {
	Q_OBJECT

private:
	void waitForSignal(QObject* sender, const char* signal);

private Q_SLOTS:
	void testReadWholeFile00();
	void testReadWholeFile01();
};

#endif // LIVEDATATEST_H
