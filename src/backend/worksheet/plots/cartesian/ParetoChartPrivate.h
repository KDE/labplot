/*
	File                 : ParetoChartPrivate.h
	Project              : LabPlot
	Description          : Private members of ParetoChart
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PARETOCHARTPRIVATE_H
#define PARETOCHARTPRIVATE_H

#include "backend/worksheet/plots/cartesian/PlotPrivate.h"

class BarPlot;
class Column;
class ParetoChart;

class ParetoChartPrivate : public PlotPrivate {
public:
	explicit ParetoChartPrivate(ParetoChart* owner);
	~ParetoChartPrivate() override;

	void retransform() override;
	void recalc();
	void recalcShapeAndBoundingRect() override;

	BarPlot* barPlot{nullptr};
	XYCurve* linePlot{nullptr};

	// column for the original source data
	const AbstractColumn* dataColumn{nullptr};
	QString dataColumnPath;

	// column for the sorted data used in the bar plot
	Column* dataSortedColumn{nullptr};
	QString dataSortedColumnPath;

	// column for x and y values used in the line plot for the cumulative percentage
	Column* xColumn{nullptr};
	QString xColumnPath;
	Column* yColumn{nullptr};
	QString yColumnPath;

	ParetoChart* const q;

	double center{0.};
};

#endif
