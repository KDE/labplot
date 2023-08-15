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

	virtual bool minMax(const CartesianCoordinateSystem::Dimension dim, const Range<int>& indexRange, Range<double>& r, bool includeErrorBars = true) const;
	virtual double minimum(CartesianCoordinateSystem::Dimension dim) const = 0;
	virtual double maximum(CartesianCoordinateSystem::Dimension dim) const = 0;

	virtual bool hasData() const = 0;

	virtual bool activatePlot(QPointF mouseScenePos, double maxDist = -1) = 0;
	virtual void setHover(bool on) = 0;

	typedef PlotPrivate Private;

private:
	Q_DECLARE_PRIVATE(WorksheetElement)

protected:
	Plot(const QString&, PlotPrivate* dd, AspectType);
	PlotPrivate* const d_ptr;

Q_SIGNALS:
	void dataChanged(); // emitted when the data to be plotted was changed to re-adjust the parent plot area
	void updateLegendRequested();
};

#endif
