/***************************************************************************
    File                 : CartesianPlot.cpp
    Project              : LabPlot/SciDAVis
    Description          : A plot containing decoration elements.
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

#include "worksheet/plots/cartesian/CartesianPlot.h"
#include "worksheet/plots/cartesian/CartesianCoordinateSystem.h"
#include "worksheet/plots/PlotArea.h"
/**
 * \class CartesianPlot
 * \brief A xy-plot.
 *
 * 
 */

CartesianPlot::CartesianPlot(const QString &name): AbstractPlot(name) {
}

CartesianPlot::CartesianPlot(const QString &name, WorksheetElementContainerPrivate *dd)
	: AbstractPlot(name, dd) {
}

void CartesianPlot::init(){
	graphicsItem()->setFlag(QGraphicsItem::ItemIsSelectable, true);
	m_coordinateSystem = new CartesianCoordinateSystem(name + "coordinate system");
//TODO 	addChild(m_coordinateSystem);
	m_coordinateSystem->setHidden(true);
	
	m_plotArea = new PlotArea(name + " plot area");
	addChild(m_plotArea);
	m_plotArea->setHidden(true);
	m_plotArea->setClippingEnabled(true);
	// TODO
}

CartesianPlot::~CartesianPlot() {
// TODO
}
