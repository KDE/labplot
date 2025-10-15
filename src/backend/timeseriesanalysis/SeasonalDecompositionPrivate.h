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
	// virtual ~SeasonalDecompositionPrivate();

	QString name() const;
	void recalc();

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

	SeasonalDecomposition::Method method{SeasonalDecomposition::Method::LOESS};
};

#endif
