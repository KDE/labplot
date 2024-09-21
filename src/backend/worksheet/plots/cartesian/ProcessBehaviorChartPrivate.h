/*
	File                 : ProcessBehaviorChartPrivate.h
	Project              : LabPlot
	Description          : Private members of ProcessBehaviorChart
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PROCESSBEHAVIORCHARTPRIVATE_H
#define PROCESSBEHAVIORCHARTPRIVATE_H

#include "backend/worksheet/plots/cartesian/PlotPrivate.h"

class Column;
class ProcessBehaviorChart;

class ProcessBehaviorChartPrivate : public PlotPrivate {
public:
	explicit ProcessBehaviorChartPrivate(ProcessBehaviorChart* owner);
	~ProcessBehaviorChartPrivate() override;

	void retransform() override;
	void recalc();
	void recalcShapeAndBoundingRect() override;
	void updateControlLimits();

	ProcessBehaviorChart::Type type{ProcessBehaviorChart::Type::XmR};
	ProcessBehaviorChart::LimitsMetric limitsMetric{ProcessBehaviorChart::LimitsMetric::Average};

	XYCurve* dataCurve{nullptr};
	XYCurve* centerCurve{nullptr};
	XYCurve* upperLimitCurve{nullptr};
	XYCurve* lowerLimitCurve{nullptr};

	// column for the source data
	const AbstractColumn* dataColumn{nullptr};
	QString dataColumnPath;

	// columns used for the data curve in case other statistics is being plotted
	// and not the original data provided by the user in xDataColumn and yDataColumn above
	Column* xColumn{nullptr};
	QString xColumnPath;
	Column* yColumn{nullptr};
	QString yColumnPath;

	// columns for control limit lines
	Column* xCenterColumn{nullptr};
	QString xCenterColumnPath;
	Column* yCenterColumn{nullptr};
	QString yCenterColumnPath;

	Column* xUpperLimitColumn{nullptr};
	QString xUpperLimitColumnPath;
	Column* yUpperLimitColumn{nullptr};
	QString yUpperLimitColumnPath;

	Column* xLowerLimitColumn{nullptr};
	QString xLowerLimitColumnPath;
	Column* yLowerLimitColumn{nullptr};
	QString yLowerLimitColumnPath;

	int subgroupSize{5};

	ProcessBehaviorChart* const q;
};

#endif
