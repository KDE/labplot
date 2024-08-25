/*
	File                 : DatapickerTest.h
	Project              : LabPlot
	Description          : Tests for Datapicker
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022 Martin Marmsoler <martin.marmsoler@gmail.com>
	SPDX-FileCopyrightText: 2022 Alexander Semke <alexander.semke@web.de>

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
	void referenceMoveKeyPress();
	void curvePointMove();
	void curvePointMoveUndoRedo();
	void selectReferencePoint();

	void imageAxisPointsChanged();

	void datapickerDateTime();

	void datapickerDeleteCurvePoint();

	void datapickerImageLoadImageAbsolute();
	void datapickerImageLoadImageRelative();
	void datapickerImageLoadImageEmbeddAbsolute();
	void datapickerImageLoadImageEmbeddAbsoluteUndoRedo();
	void datapickerImageLoadImageEmbeddRelative();
	void datapickerImageLoadImageEmbeddRelativeUndoRedo();
	void datapickerImageClipboard();
	void datapickerImageClipboardSelectImageFromPath();

	void saveLoad();
};

#endif // DATAPICKERTEST_H
