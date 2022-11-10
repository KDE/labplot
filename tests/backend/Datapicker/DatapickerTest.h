/*
	File                 : DatapickerTest.h
	Project              : LabPlot
	Description          : Tests for Datapicker
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022 Martin Marmsoler <martin.marmsoler@gmail.com>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef DATAPICKERTEST_H
#define DATAPICKERTEST_H

#include "../../CommonTest.h"

class DatapickerTest : public CommonTest {
	Q_OBJECT

private Q_SLOTS:
	void mapTypeToCartesian();
	void mapCartesianToType();
	void mapSceneToLogical();
	void cartesianMapping();
	void logarithmicNaturalXMapping();
	void logarithmicNaturalYMapping();
	void logarithmicNaturalXYMapping();
	void logarithmic10XMapping();
	void logarithmic10YMapping();
	void logarithmic10XYMapping();
	void referenceMove();
	void curvePointMove();
	void selectReferencePoint();
};

#endif // DATAPICKERTEST_H
