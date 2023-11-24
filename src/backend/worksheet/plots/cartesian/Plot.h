/*
	File                 : Plot.h
	Project              : LabPlot
	Description          : Base class for all plots like scatter plot, box plot, etc.
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2020-2023 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PLOT_H
#define PLOT_H

#include "backend/worksheet/WorksheetElement.h"
#include "backend/worksheet/plots/cartesian/CartesianCoordinateSystem.h"

class PlotPrivate;
class QPointF;

class Plot : public WorksheetElement {
	Q_OBJECT

public:
	virtual ~Plot();

	using Dimension = CartesianCoordinateSystem::Dimension;
	virtual bool minMax(const Dimension dim, const Range<int>& indexRange, Range<double>& r, bool includeErrorBars = true) const;
	virtual bool indicesMinMax(const Dimension dim, double v1, double v2, int& start, int& end) const;
	virtual double minimum(Dimension dim) const = 0;
	virtual double maximum(Dimension dim) const = 0;
	/*!
	 * \brief dataCount
	 * Number of elements in a specific direction
	 * \param dim
	 * \return Number of data or -1 if the plot is invalid (so it will not be considered for autoscale)
	 */
	virtual int dataCount(Dimension dim) const {
		Q_ASSERT(false);
		return -1;
	}
	virtual bool hasData() const = 0;
	bool activatePlot(QPointF mouseScenePos, double maxDist = -1);

	typedef PlotPrivate Private;

private:
	Q_DECLARE_PRIVATE(Plot)

protected:
	Plot(const QString&, PlotPrivate* dd, AspectType);
	PlotPrivate* const d_ptr;

Q_SIGNALS:
	void dataChanged(); // emitted when the data to be plotted was changed to re-adjust the parent plot area
	void updateLegendRequested();
};

#endif
