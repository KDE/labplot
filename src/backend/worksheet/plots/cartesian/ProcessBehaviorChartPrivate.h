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

	ProcessBehaviorChart::Type type;

	XYCurve* dataCurve{nullptr};
	XYCurve* centerCurve{nullptr};
	XYCurve* upperLimitCurve{nullptr};
	XYCurve* lowerLimitCurve{nullptr};

	const AbstractColumn* xDataColumn{nullptr};
	QString xDataColumnPath;
	const AbstractColumn* yDataColumn{nullptr};
	QString yDataColumnPath;

	Column* xIndexColumn{nullptr};

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

	int subgroupSize{1};

	ProcessBehaviorChart* const q;
};

#endif
