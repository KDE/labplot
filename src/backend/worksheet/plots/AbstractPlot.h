/***************************************************************************
    File                 : AbstractPlot.h
    Project              : LabPlot/SciDAVis
    Description          : Second level container in a Worksheet for logical grouping
    --------------------------------------------------------------------
    Copyright            : (C) 2009 Tilman Benkert (thzs*gmx.net)
                           (replace * with @ in the email addresses) 
                           
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

#include "worksheet/WorksheetElementContainer.h"

class AbstractCoordinateSystem;
class PlotArea;

class AbstractPlot: public WorksheetElementContainer {
	Q_OBJECT

	public:
		AbstractPlot(const QString &name);
		~AbstractPlot();

		AbstractCoordinateSystem* coordinateSystem() const;
		PlotArea* plotArea();
		// TODO add abstract methods interface (such as transform(...))

	protected:
		AbstractPlot(const QString &name, WorksheetElementContainerPrivate *dd);
		AbstractCoordinateSystem* m_coordinateSystem;
		PlotArea* m_plotArea;
};


#endif

