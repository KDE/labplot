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
	// Continuous fixed - fixed amount of samples is processed starting from the beginning of the newly received data
	void testReadContinuousFixed00();
	void testReadContinuousFixed01();
	void testReadContinuousFixed02();

	// From End - fixed amount of samples is processed starting from the end of the newly received data
	void testReadFromEnd00();
	void testReadFromEnd01();
	void testReadFromEnd02();

	// Till the End - all newly received data is processed
	void testReadTillEnd00();
	void testReadTillEnd01();

	// Whole file - on every read the whole file is re-read completely and processed
	void testReadWholeFile00();
	void testReadWholeFile01();
};

#endif // LIVEDATATEST_H
