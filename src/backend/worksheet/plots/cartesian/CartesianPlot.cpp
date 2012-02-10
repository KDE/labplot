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
#include "worksheet/plots/cartesian/Axis.h"
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

class CartesianPlotPrivate:public AbstractPlotPrivate{
};

CartesianPlot::CartesianPlot(const QString &name): AbstractPlot(name) {
	init();
}

CartesianPlot::CartesianPlot(const QString &name, CartesianPlotPrivate *dd)
	: AbstractPlot(name, dd) {
	init();
}

void CartesianPlot::init(){
	Q_D(CartesianPlot);
	graphicsItem()->setFlag(QGraphicsItem::ItemIsSelectable, true);
	m_coordinateSystem = new CartesianCoordinateSystem(this);

	//TODO: Use default settings for left, top, width, height and for min/max for the coordinate system
	float x = Worksheet::convertToSceneUnits(2, Worksheet::Centimeter);
	float y = Worksheet::convertToSceneUnits(2, Worksheet::Centimeter);
	float w = Worksheet::convertToSceneUnits(10, Worksheet::Centimeter);
	float h = Worksheet::convertToSceneUnits(10, Worksheet::Centimeter);
	d->rect.setX(x);
	d->rect.setY(y);
	d->rect.setWidth(w);
	d->rect.setHeight(h);

	CartesianCoordinateSystem *cSystem = dynamic_cast<CartesianCoordinateSystem *>(m_coordinateSystem);
	QList<CartesianCoordinateSystem::Scale *> scales;
	scales << CartesianCoordinateSystem::Scale::createLinearScale(Interval<double>(SCALE_MIN, SCALE_MAX), x, x+w, 0, 1);
	cSystem ->setXScales(scales);
	scales.clear();
	scales << CartesianCoordinateSystem::Scale::createLinearScale(Interval<double>(SCALE_MIN, SCALE_MAX), y+h, y, 0, 1);
	cSystem ->setYScales(scales);
	
	m_plotArea = new PlotArea(name() + " plot area");
	addChild(m_plotArea);
// 	m_plotArea->setRect(QRectF(-0.05,-0.05,1.1, 1.1));
	m_plotArea->setRect(QRectF(0, 0, 1, 1));

	Axis *axis = new Axis("x axis 1", Axis::AxisHorizontal);
	addChild(axis);
	axis->setStart(0);
	axis->setEnd(1);
	axis->setMajorTicksDirection(Axis::ticksIn);
	axis->setMajorTicksNumber(6);
	axis->setMinorTicksDirection(Axis::ticksIn);
	axis->setMinorTicksNumber(1);
	axis->setLabelsOffset(QPointF(0,Worksheet::convertToSceneUnits(5.0, Worksheet::Point)));

	axis = new Axis("x axis 2", Axis::AxisHorizontal);
	addChild(axis);
	axis->setOffset(1);
	axis->setStart(0);
	axis->setEnd(1);
	axis->setMajorTicksDirection(Axis::ticksIn);
	axis->setMajorTicksNumber(6);
	axis->setMinorTicksDirection(Axis::ticksIn);
	axis->setMinorTicksNumber(1);
	axis->setLabelsPosition(Axis::NoLabels);
	axis->setLabelsOffset(QPointF(0,Worksheet::convertToSceneUnits(-5.0, Worksheet::Point)));
	
	axis = new Axis("y axis 1", Axis::AxisVertical);
	addChild(axis);
	axis->setStart(0);
	axis->setEnd(1);
	axis->setMajorTicksDirection(Axis::ticksIn);
	axis->setMajorTicksNumber(6);
	axis->setMinorTicksDirection(Axis::ticksIn);
	axis->setMinorTicksNumber(1);
	axis->setLabelsOffset(QPointF(Worksheet::convertToSceneUnits(-5.0, Worksheet::Point),0));
	
	axis = new Axis("y axis 2", Axis::AxisVertical);
	addChild(axis);
	axis->setStart(0);
	axis->setEnd(1);
	axis->setOffset(1);
	axis->setMajorTicksDirection(Axis::ticksIn);
	axis->setMajorTicksNumber(6);
	axis->setMinorTicksDirection(Axis::ticksIn);
	axis->setMinorTicksNumber(1);
	axis->setLabelsPosition(Axis::NoLabels);
	axis->setLabelsOffset(QPointF(Worksheet::convertToSceneUnits(5.0, Worksheet::Point),0));
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
	Q_D(CartesianPlot);
	d->rect=r;

	CartesianCoordinateSystem *cSystem = dynamic_cast<CartesianCoordinateSystem *>(m_coordinateSystem);
	QList<CartesianCoordinateSystem::Scale *> scales;
	scales << CartesianCoordinateSystem::Scale::createLinearScale(Interval<double>(SCALE_MIN, SCALE_MAX), r.x(), r.x()+r.width(), 0, 1);
	cSystem ->setXScales(scales);
	scales.clear();
	scales << CartesianCoordinateSystem::Scale::createLinearScale(Interval<double>(SCALE_MIN, SCALE_MAX), r.y()+r.height(), r.y(), 0, 1);
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

//################################################################
//################### Private implementation ##########################
//################################################################
