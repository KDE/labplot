/*
	File                 : ProcessBehaviorChartPrivate.h
	Project              : LabPlot
	Description          : Private members of ProcessBehaviorChart
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024-2025 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PROCESSBEHAVIORCHARTPRIVATE_H
#define PROCESSBEHAVIORCHARTPRIVATE_H

#include "backend/worksheet/plots/cartesian/PlotPrivate.h"

class Column;
class ProcessBehaviorChart;
class TextLabel;

class ProcessBehaviorChartPrivate : public PlotPrivate {
public:
	explicit ProcessBehaviorChartPrivate(ProcessBehaviorChart* owner);
	~ProcessBehaviorChartPrivate() override;

	void retransform() override;
	void recalc();
	void recalcShapeAndBoundingRect() override;
	void updateLimitConstraints();
	void updateControlLimits();
	void updateLabels();

	ProcessBehaviorChart::Type type{ProcessBehaviorChart::Type::XmR};
	ProcessBehaviorChart::LimitsMetric limitsMetric{ProcessBehaviorChart::LimitsMetric::Average};

	XYCurve* dataCurve{nullptr};
	XYCurve* centerCurve{nullptr};
	XYCurve* upperLimitCurve{nullptr};
	XYCurve* lowerLimitCurve{nullptr};

	// columns for the source data
	const AbstractColumn* dataColumn{nullptr}; // column with the data containing the actual variables/measures/attributes
	QString dataColumnPath;

	const AbstractColumn* data2Column{nullptr}; // additional input like the sample sizes for the P chart
	QString data2ColumnPath;

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

	// labels for control limit values
	bool labelsEnabled{true};
	int labelsPrecision{2};
	bool labelsAutoPrecision{true};
	Line* labelsBorderLine{nullptr};
	TextLabel* upperLimitLabel{nullptr};
	TextLabel* centerLabel{nullptr};
	TextLabel* lowerLimitLabel{nullptr};

	int sampleSize{5};
	double maxUpperLimit{INFINITY};
	double minLowerLimit{-INFINITY};
	bool exactLimitsEnabled{true};

	ProcessBehaviorChart* const q;

	double center{0.};
	double upperLimit{0.};
	double lowerLimit{0.};
};

#endif
