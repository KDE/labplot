/***************************************************************************
    File                 : CartesianPlot.cpp
    Project              : LabPlot/SciDAVis
    Description          : A plot containing decoration elements.
    --------------------------------------------------------------------
    Copyright            : (C) 2009 Tilman Benkert (thzs*gmx.net)
    Copyright            : (C) 2011-2012 by Alexander Semke (alexander.semke*web.de)
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
#include "worksheet/Worksheet.h"

#ifdef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
#include <QIcon>
#else
#include "KIcon"
#endif

#define SCALE_MIN CartesianCoordinateSystem::Scale::LIMIT_MIN
#define SCALE_MAX CartesianCoordinateSystem::Scale::LIMIT_MAX

/**
 * \class CartesianPlot
 * \brief A xy-plot.
 *
 * 
 */

CartesianPlot::CartesianPlot(const QString &name): AbstractPlot(name) {
	init();
}

CartesianPlot::CartesianPlot(const QString &name, WorksheetElementContainerPrivate *dd)
	: AbstractPlot(name, dd) {
	init();
}

void CartesianPlot::init(){
	graphicsItem()->setFlag(QGraphicsItem::ItemIsSelectable, true);
	m_coordinateSystem = new CartesianCoordinateSystem(this);
	
	m_plotArea = new PlotArea(name() + " plot area");
	addChild(m_plotArea);
	m_plotArea->setHidden(true);
	m_plotArea->setClippingEnabled(true);
}

CartesianPlot::~CartesianPlot() {
// TODO
}

/*!
	Returns an icon to be used in the project explorer.
*/
QIcon CartesianPlot::icon() const{
	QIcon ico;
#ifdef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
	ico.addPixmap(QPixmap(":/graph.xpm"));
#else
	ico = KIcon("office-chart-line");
#endif
	return ico;
}

void CartesianPlot::setRect(const QRectF& r){
	m_rect=r;

	CartesianCoordinateSystem *cSystem = dynamic_cast<CartesianCoordinateSystem *>(m_coordinateSystem);
	QList<CartesianCoordinateSystem::Scale *> scales;
	scales << CartesianCoordinateSystem::Scale::createLinearScale(Interval<double>(SCALE_MIN, SCALE_MAX), r.x(), r.x()+r.width(), 0, 10);
	cSystem ->setXScales(scales);
	scales.clear();
	scales << CartesianCoordinateSystem::Scale::createLinearScale(Interval<double>(SCALE_MIN, SCALE_MAX), r.y()+r.height(), r.y(), 1, 10);
	cSystem ->setYScales(scales);
	
	m_plotArea->retransform();
	retransform();
	graphicsItem()->update();
	
	//TODO trigger an update of the view
// 	AbstractAspect * parent = parentAspect();
// 	while (parent) {
// 		Worksheet *worksheet = qobject_cast<Worksheet *>(parent);
// 		if (worksheet)
// 			orksheet->requestUpdate();
// 
// 		parent = parent->parentAspect();
// 	}
}
