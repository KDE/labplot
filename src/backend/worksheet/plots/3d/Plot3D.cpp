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
#include <QGraphicsView>
#include <QPainter>
#include <QWidget>

#include <KIcon>

#include <QVTKGraphicsItem.h>
#include <vtkSmartPointer.h>
#include <vtkSphereSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkRenderer.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkProperty.h>


Plot3D::Plot3D(const QString& name, QGLContext *context)
	: AbstractPlot(name, new Plot3DPrivate(this, context)){
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

Plot3DPrivate::Plot3DPrivate(Plot3D* owner, QGLContext *context)
	: AbstractPlotPrivate(owner), q(owner), context(context){
}

Plot3DPrivate::~Plot3DPrivate(){
}

void Plot3DPrivate::init(){
	vtkItem = new QVTKGraphicsItem(context, q->plotArea()->graphicsItem());

	vtkGenericOpenGLRenderWindow *renderWindow = vtkItem->GetRenderWindow();

	vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
	renderWindow->AddRenderer(renderer);

	renderer->SetBackground(1, 1, 1);

	vtkSmartPointer<vtkSphereSource> sphereSource = vtkSmartPointer<vtkSphereSource>::New();
	sphereSource->Update();
	vtkSmartPointer<vtkPolyDataMapper> sphereMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	sphereMapper->SetInputConnection(sphereSource->GetOutputPort());
	vtkSmartPointer<vtkActor> sphereActor = vtkSmartPointer<vtkActor>::New();
	sphereActor->GetProperty()->SetFrontfaceCulling(true);
	sphereActor->SetMapper(sphereMapper);
	renderer->AddActor(sphereActor);
}

void Plot3DPrivate::retransform(){
	prepareGeometryChange();
	setPos(rect.x()+rect.width()/2, rect.y()+rect.height()/2);

	//plotArea position is always (0, 0) in parent's coordinates, don't need to update here
	q->plotArea()->setRect(rect);
	vtkItem->setGeometry(q->plotArea()->rect());

	WorksheetElementContainerPrivate::recalcShapeAndBoundingRect();
}