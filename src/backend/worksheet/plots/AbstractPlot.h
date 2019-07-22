/***************************************************************************
    File                 : AbstractPlot.h
    Project              : LabPlot
    Description          : Base class for plots of different types
    --------------------------------------------------------------------
    Copyright            : (C) 2009 Tilman Benkert (thzs@gmx.net)
    Copyright            : (C) 2011-2017 by Alexander Semke (alexander.semke@web.de)

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
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
	~AbstractPlot() override= default;

	void handleResize(double horizontalRatio, double verticalRatio, bool pageResize) override;
	AbstractCoordinateSystem* coordinateSystem() const;
	PlotArea* plotArea();
	TextLabel* title();

	BASIC_D_ACCESSOR_DECL(float, horizontalPadding, HorizontalPadding)
	BASIC_D_ACCESSOR_DECL(float, verticalPadding, VerticalPadding)
	BASIC_D_ACCESSOR_DECL(double, rightPadding, RightPadding)
	BASIC_D_ACCESSOR_DECL(double, bottomPadding, BottomPadding)
	BASIC_D_ACCESSOR_DECL(bool, symmetricPadding, SymmetricPadding)

	typedef AbstractPlotPrivate Private;

protected:
	AbstractPlot(const QString&, AbstractPlotPrivate*, AspectType);
	AbstractCoordinateSystem* m_coordinateSystem{nullptr};
	PlotArea* m_plotArea{nullptr};
	TextLabel* m_title{nullptr};

private:
	void init();
	Q_DECLARE_PRIVATE(AbstractPlot)
};

#endif
