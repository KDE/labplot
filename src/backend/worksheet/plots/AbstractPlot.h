/*
    File                 : AbstractPlot.h
    Project              : LabPlot
    Description          : Base class for plots of different types
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2009 Tilman Benkert (thzs@gmx.net)
    SPDX-FileCopyrightText: 2011-2017 Alexander Semke (alexander.semke@web.de)

*/

/***************************************************************************
 *                                                                         *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/

#ifndef ABSTRACTPLOT_H
#define ABSTRACTPLOT_H

#include "backend/worksheet/WorksheetElementContainer.h"
#include "backend/lib/macros.h"

class AbstractCoordinateSystem;
class PlotArea;
class TextLabel;
class AbstractPlotPrivate;

class AbstractPlot : public WorksheetElementContainer {
	Q_OBJECT

public:
	AbstractPlot(const QString &name, AspectType type);
	~AbstractPlot() override = default;

	void handleResize(double horizontalRatio, double verticalRatio, bool pageResize) override;
	AbstractCoordinateSystem* coordinateSystem(int index) const;
	QVector<AbstractCoordinateSystem*> coordinateSystems() const;
	PlotArea* plotArea();
	TextLabel* title();

	BASIC_D_ACCESSOR_DECL(double, horizontalPadding, HorizontalPadding)
	BASIC_D_ACCESSOR_DECL(double, verticalPadding, VerticalPadding)
	BASIC_D_ACCESSOR_DECL(double, rightPadding, RightPadding)
	BASIC_D_ACCESSOR_DECL(double, bottomPadding, BottomPadding)
	BASIC_D_ACCESSOR_DECL(bool, symmetricPadding, SymmetricPadding)

	typedef AbstractPlotPrivate Private;

protected:
	AbstractPlot(const QString&, AbstractPlotPrivate*, AspectType);
	QVector<AbstractCoordinateSystem*> m_coordinateSystems;
	PlotArea* m_plotArea{nullptr};
	TextLabel* m_title{nullptr};

private:
	void init();
	Q_DECLARE_PRIVATE(AbstractPlot)
};

#endif
