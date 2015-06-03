/***************************************************************************
    File                 : Plot3D.cpp
    Project              : LabPlot
    Description          : 3D plot
    --------------------------------------------------------------------
    Copyright            : (C) 2015 by Minh Ngo (minh@fedoraproject.org)

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

#include "Plot3D.h"
#include "Plot3DPrivate.h"
#include "backend/worksheet/plots/PlotArea.h"
#include "backend/worksheet/TextLabel.h"

#include <QDebug>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QGraphicsProxyWidget>
#include <QLineEdit>
#include <QPainter>
#include <QWidget>

#include <KIcon>

#include <vtk/QVTKWidget2.h>

Plot3D::Plot3D(const QString& name)
	: AbstractPlot(name, new Plot3DPrivate(this)){
	qDebug() << Q_FUNC_INFO;
	init();
}

Plot3D::Plot3D(const QString &name, Plot3DPrivate *dd)
	: AbstractPlot(name, dd){
	qDebug() << Q_FUNC_INFO;
	init();
}

void Plot3D::init(){
	m_plotArea = new PlotArea(name() + " plot area");
	addChild(m_plotArea);

	Q_D(Plot3D);
	d->init();
}

Plot3D::~Plot3D(){

}

QIcon Plot3D::icon() const{
	// TODO: Replace by some 3D chart
	return KIcon("office-chart-line");
}

void Plot3D::setRect(const QRectF &rect){
	Q_D(Plot3D);
	d->rect = rect;
	d->retransform();
}

/////////////////////////////////////////////////////////////////////////////

Plot3DPrivate::Plot3DPrivate(Plot3D* owner)
	: AbstractPlotPrivate(owner), q(owner){
}

void Plot3DPrivate::init(){
	QVTKWidget2 *w = new QVTKWidget2;

	m_proxyWidget = new QGraphicsProxyWidget(q->plotArea()->graphicsItem());
	m_proxyWidget->setWidget(w);
}

void Plot3DPrivate::retransform(){
	prepareGeometryChange();
	setPos(rect.x()+rect.width()/2, rect.y()+rect.height()/2);

	//plotArea position is always (0, 0) in parent's coordinates, don't need to update here
	q->plotArea()->setRect(rect);
	m_proxyWidget->setGeometry(q->plotArea()->rect());

	WorksheetElementContainerPrivate::recalcShapeAndBoundingRect();
}

void Plot3DPrivate::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget * widget){
	painter->fillRect(q->plotArea()->rect(), QBrush(Qt::red));
	WorksheetElementContainerPrivate::paint(painter, option, widget);
}