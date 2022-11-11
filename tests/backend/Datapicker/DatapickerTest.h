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
	void mapCartesianToCartesian();
	void maplnXToCartesian();
	void maplnYToCartesian();
	void maplnXYToCartesian();
	void maplog10XToCartesian();
	void maplog10YToCartesian();
	void maplog10XYToCartesian();
	void mapPolarInRadiansToCartesian();
	void mapPolarInDegreeToCartesian();
	void mapCartesianToLinear();
	void mapCartesianToLnX();
	void mapCartesianToLnY();
	void mapCartesianToLnXY();
	void mapCartesianToLog10X();
	void mapCartesianToLog10Y();
	void mapCartesianToLog10XY();
	void mapCartesianToPolarInDegree();
	void mapCartesianToPolarInRadians();
	void linearMapping();
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
