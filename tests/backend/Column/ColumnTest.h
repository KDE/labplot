/*
	File                 : ColumnTest.h
	Project              : LabPlot
	Description          : Tests for Column
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2021 Martin Marmsoler <martin.marmsoler@gmail.com>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef COLUMNTEST_H
#define COLUMNTEST_H

#include "../../CommonTest.h"

class ColumnTest : public CommonTest {
	Q_OBJECT

private Q_SLOTS:

	void loadDoubleFromProject();
	void loadIntegerFromProject();
	void loadBigIntegerFromProject();
	void loadTextFromProject();
	void loadDateTimeFromProject();
	void doubleMinimum();
	void doubleMaximum();
	void integerMinimum();
	void integerMaximum();
	void bigIntMinimum();
	void bigIntMaximum();
	void saveLoadDateTime();

};

#endif // COLUMNTEST_H
