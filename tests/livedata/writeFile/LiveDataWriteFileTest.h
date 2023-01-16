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
#include <QProcess>

class LiveDataWriteFileTest : public CommonTest {
	Q_OBJECT

private Q_SLOTS:
	void init(); // Called before each test is executed
	void cleanup(); // Called after each test is executed

private Q_SLOTS:
	void testRefresh();

private:
	QProcess m_process;
};
#endif // LIVEDATAWRITEFILETEST_H
