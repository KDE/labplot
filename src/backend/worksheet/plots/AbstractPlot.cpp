/***************************************************************************
    File                 : AbstractPlot.cpp
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

#include "worksheet/plots/AbstractPlot.h"
#include "worksheet/plots/PlotArea.h"
#include "worksheet/plots/AbstractCoordinateSystem.h"
#include "worksheet/WorksheetElementContainerPrivate.h"
#include <QDebug>
/**
 * \class AbstractPlot
 * \brief Second level container in a Worksheet for logical grouping
 *
 * TODO: decide the exact role of AbstractPlot 
 *
 */ 

AbstractPlot::AbstractPlot(const QString &name):WorksheetElementContainer(name){
	init();
}

AbstractPlot::AbstractPlot(const QString &name, WorksheetElementContainerPrivate *dd)
	: WorksheetElementContainer(name, dd){
	init();
}

void AbstractPlot::init(){
	graphicsItem()->setFlag(QGraphicsItem::ItemIsMovable, true);
	graphicsItem()->setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);
	graphicsItem()->setFlag(QGraphicsItem::ItemIsSelectable, true);
// TODO
}


AbstractPlot::~AbstractPlot() {
// TODO
}

PlotArea* AbstractPlot::plotArea(){
	return m_plotArea;
}

AbstractCoordinateSystem* AbstractPlot::coordinateSystem() const{
	return m_coordinateSystem;
}

ScalableTextLabel* AbstractPlot::title(){
	return m_title;
}

void AbstractPlot::handlePageResize(double horizontalRatio, double verticalRatio){
	m_coordinateSystem->handlePageResize(horizontalRatio, verticalRatio);
	m_plotArea->handlePageResize(horizontalRatio, verticalRatio);
	WorksheetElementContainer::handlePageResize(horizontalRatio, verticalRatio);
}

//################################################################
//################### Private implementation ##########################
//################################################################

