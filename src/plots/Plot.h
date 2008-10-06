/***************************************************************************
    File                 : Plot.h
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Stefan Gerlach, Alexander Semke
    Email (use @ for *)  : stefan.gerlach*uni-konstanz.de, alexander.semke*web.de
    Description          : generic plot class
                           
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
#ifndef PLOT_H
#define PLOT_H

#include <QBrush>

#include "../elements/Axis.h"
#include "../elements/Label.h"
#include "../elements/Legend.h"
#include "../elements/Point.h"
#include "../elements/Range.h"
#include "../elements/Set.h"
#include "../elements/Symbol.h"

class Legend;

class Plot{
public:
	enum PlotType {PLOT2D,PLOTSURFACE,PLOT3D,PLOTGRASS,PLOTVTK,PLOTPIE,PLOTPOLAR,PLOTTERNARY,PLOTQWT3D};

	Plot();
	virtual ~Plot();

	Plot::PlotType plotType() const;
	void addSet(Set);
	QString getTicLabel(const int, const int, const QString, const double) const;

	virtual void draw(QPainter *p, const int w, const int h) =0;
	void drawStyle(QPainter*, const Style*, const QPolygonF, const int xmin, const int xmax, const int ymin, const int ymax);

	QList<Axis>* axes();

 	void resetRanges();
	Label* titleLabel();
 	Legend* legend();

	/*
	 void setRange(Range *,int i)();
	Range* getRange(int i) { return &range[i];}

	void setActRange(Range *,int i)();
	Range* ActRange(int i) { return &actrange[i];}

	 void setRanges(Range *)();
	Range* Ranges() { return range;}

	void setActRanges(Range *)();
	Range* ActRanges() { return actrange;}

	void draw(QPainter *p, const int w, const int h) const ;
*/

protected:
	Plot::PlotType m_plotType;
	QList<Set> list_Sets;			//!< data set list
	QList<Axis> list_Axes;			//!< axes list
	Point p1, p2;				//!< plotting area	(0..1)
	Label m_titleLabel;
 	Legend m_legend;
	QList<Range> list_setRanges;				//!< list of set ranges
	QList<Range> list_plotRanges;			//!< list of plot ranges
	QBrush backgroundBrush, areaBackgroundBrush;
	Point position, size;
	bool transparent;
};

#endif // PLOT_H
