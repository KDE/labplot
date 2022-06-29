/*
	File                 : ColumnTest.h
	Project              : LabPlot
	Description          : Tests for Column
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2021 Martin Marmsoler <martin.marmsoler@gmail.com>
	SPDX-FileCopyrightText: 2022 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef COLUMNTEST_H
#define COLUMNTEST_H

#include "../../CommonTest.h"

class ColumnTest : public CommonTest {
	Q_OBJECT

private Q_SLOTS:
	void doubleMinimum();
	void doubleMaximum();
	void integerMinimum();
	void integerMaximum();
	void bigIntMinimum();
	void bigIntMaximum();

	void statisticsDouble();		// only positive double values
	void statisticsDoubleNegative();	// contains negative values (> -100)
	void statisticsDoubleBigNegative();	// contains big negative values (<= -100)
	void statisticsDoubleZero();		// contains zero value
	void statisticsInt();			// only positive integer values
	void statisticsIntNegative();		// contains negative values (> -100)
	void statisticsIntBigNegative();	// contains big negative values (<= -100)
	void statisticsIntZero();		// contains zero value
	void statisticsIntOverflow();		// check overflow of integer
	void statisticsBigInt();		// big ints

	void loadDoubleFromProject();
	void loadIntegerFromProject();
	void loadBigIntegerFromProject();
	void loadTextFromProject();
	void loadDateTimeFromProject();
	void saveLoadDateTime();
};

#endif // COLUMNTEST_H
