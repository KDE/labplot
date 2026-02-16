/*
	File                 : Plot.h
	Project              : LabPlot
	Description          : Base class for all plots like scatter plot, box plot, etc.
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2020-2026 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PLOT_H
#define PLOT_H

#include "backend/lib/macros.h"
#include "backend/worksheet/WorksheetElement.h"
#include "backend/worksheet/plots/cartesian/CartesianCoordinateSystem.h"

class AbstractColumn;
class Column;
class PlotPrivate;
class QPointF;

#ifdef SDK
#include "labplot_export.h"
class LABPLOT_EXPORT Plot : public WorksheetElement {
#else
class Plot : public WorksheetElement {
#endif
	Q_OBJECT

public:
	virtual ~Plot();

	enum class PlotType {
		// basic plots
		Line,
		LineHorizontalStep,
		LineVerticalStep,
		LineSpline,
		Scatter,
		ScatterYError,
		ScatterXYError,
		LineSymbol,
		LineSymbol2PointSegment,
		LineSymbol3PointSegment,

		Formula,

		// statistical plots
		Histogram,
		BoxPlot,
		KDEPlot,
		QQPlot,

		// bar plots
		BarPlot,
		LollipopPlot,

		// continuous improvement plots
		ProcessBehaviorChart,
		RunChart,
		ParetoChart
	};

	BASIC_D_ACCESSOR_DECL(bool, legendVisible, LegendVisible)
	using Dimension = CartesianCoordinateSystem::Dimension;
	virtual bool minMax(const Dimension dim, const Range<int>& indexRange, Range<double>& rOut, bool includeErrorBars = true) const = 0;
	virtual double minimum(Dimension dim) const = 0;
	virtual double maximum(Dimension dim) const = 0;
	virtual bool indicesMinMax(const Dimension dim, double v1, double v2, int& start, int& end) const = 0;
	virtual int dataCount(Dimension dim) const = 0;
	virtual bool hasData() const = 0;
	bool activatePlot(QPointF mouseScenePos, double maxDist = -1);
	virtual QColor color() const = 0; // Color of the plot. If the plot consists multiple colors, return the main Color (This is used in the cursor dock as
									  // background color for example)
	virtual bool usingColumn(const AbstractColumn*, bool indirect = true) const = 0;

	typedef PlotPrivate Private;

private:
	Q_DECLARE_PRIVATE(Plot)

protected:
	Plot(const QString&, PlotPrivate* dd, AspectType);
	PlotPrivate* const d_ptr;

	friend class CartesianPlot;
	friend class CartesianPlotPrivate;
	friend class Spreadsheet;
	virtual void recalc() = 0;

Q_SIGNALS:
	void dataChanged(); // emitted when the data to be plotted was changed to re-adjust the parent plot area
	void appearanceChanged();
	void legendVisibleChanged(bool);
};

#endif
