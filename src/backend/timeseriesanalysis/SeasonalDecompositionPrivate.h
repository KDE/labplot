/*
	File                 : SeasonalDecompositionPrivate.h
	Project              : LabPlot
	Description          : Private members of SeasonalDecomposition.
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2025 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SEASONALDECOMPOSITIONPRIVATE_H
#define SEASONALDECOMPOSITIONPRIVATE_H

#include "backend/timeseriesanalysis/SeasonalDecomposition.h"

class CartesianPlot;
class SeasonalDecomposition;
class XYCurve;
class Worksheet;

class SeasonalDecompositionPrivate {
public:
	explicit SeasonalDecompositionPrivate(SeasonalDecomposition*);

	QString name() const;
	void recalc();
	void recalcDecomposition();

	SeasonalDecomposition* const q;

	// data source
	const AbstractColumn* xColumn{nullptr};
	const AbstractColumn* yColumn{nullptr};
	QString xColumnPath;
	QString yColumnPath;

	// visualization
	Worksheet* worksheet{nullptr};

	// plot areas
	CartesianPlot* plotAreaOriginal{nullptr};
	CartesianPlot* plotAreaTrend{nullptr};
	CartesianPlot* plotAreaSeasonal{nullptr};
	CartesianPlot* plotAreaResidual{nullptr};

	// curves
	XYCurve* curveOriginal{nullptr};
	XYCurve* curveTrend{nullptr};
	XYCurve* curveSeasonal{nullptr};
	XYCurve* curveResidual{nullptr};

	// spreadsheet and columns to hold the result y-data
	Spreadsheet* resultSpreadsheet{nullptr};
	Column* columnTrend{nullptr};
	Column* columnSeasonal{nullptr};
	Column* columnResidual{nullptr};

	SeasonalDecomposition::Method method{SeasonalDecomposition::Method::STL};

	// STL parameters
	int stlPeriod{7};
	bool stlRobust{true};

	int stlSeasonalLength{7};
	int stlTrendLength{3};
	bool stlTrendLengthAuto{true};
	int stlLowPassLength{3};
	bool stlLowPassLengthAuto{true};

	int stlSeasonalDegree{0};
	int stlTrendDegree{1};
	int stlLowPassDegree{1};

	int stlSeasonalJump{1};
	bool stlSeasonalJumpAuto{true};
	int stlTrendJump{1};
	bool stlTrendJumpAuto{true};
	int stlLowPassJump{1};
	bool stlLowPassJumpAuto{true};

	// MSTL parameters
	std::vector<size_t> mstlPeriods{24, 168};
	double mstlLambda{0.5};
	int mstlIterations{2};

private:
	void reset(const QString&) const;
	std::vector<double> yDataVector; // valid values from the sources y-column, used in the calculation of the decomposition
};

#endif
