/*
	File                 : RunChartPrivate.h
	Project              : LabPlot
	Description          : Private members of RunChart
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RUNCHARTPRIVATE_H
#define RUNCHARTPRIVATE_H

#include "backend/worksheet/plots/cartesian/PlotPrivate.h"

class Column;
class RunChart;

class RunChartPrivate : public PlotPrivate {
public:
	explicit RunChartPrivate(RunChart* owner);
	~RunChartPrivate() override;

	void retransform() override;
	void recalc();
	void recalcShapeAndBoundingRect() override;

	RunChart::CenterMetric centerMetric{RunChart::CenterMetric::Median};

	XYCurve* dataCurve{nullptr};
	XYCurve* centerCurve{nullptr};

	// column for the source data
	const AbstractColumn* dataColumn{nullptr};
	QString dataColumnPath;

	// column for the index on x
	Column* xColumn{nullptr};
	QString xColumnPath;

	// columns for control limit lines
	Column* xCenterColumn{nullptr};
	QString xCenterColumnPath;
	Column* yCenterColumn{nullptr};
	QString yCenterColumnPath;

	RunChart* const q;

	double center{0.};
};

#endif
