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
#include <vtkSphereSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkProperty.h>
#include <vtkRendererCollection.h>
#include <vtkOBJReader.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorStyleTrackballCamera.h>


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

void Plot3D::setVisualizationType(VisualizationType type){
	Q_D(Plot3D);
	d->visType = type;
	d->isChanged = true;
}

void Plot3D::setDataSource(DataSource source){
	Q_D(Plot3D);
	d->sourceType = source;
	d->isChanged = true;
}

void Plot3D::setFile(const KUrl& path){
	Q_D(Plot3D);
	d->path = path;
	d->isChanged = true;
}

void Plot3D::retransform(){
	Q_D(Plot3D);
	d->retransform();
	WorksheetElementContainer::retransform();
}

/////////////////////////////////////////////////////////////////////////////

Plot3DPrivate::Plot3DPrivate(Plot3D* owner, QGLContext *context)
	: AbstractPlotPrivate(owner)
	, q_ptr(owner)
	, context(context)
	, visType(Plot3D::VisualizationType_Triangles)
	, sourceType(Plot3D::DataSource_Empty)
	, isChanged(false){
}

Plot3DPrivate::~Plot3DPrivate(){
}

void Plot3DPrivate::init(){
	Q_Q(Plot3D);
	vtkItem = new QVTKGraphicsItem(context, q->plotArea()->graphicsItem());

	vtkGenericOpenGLRenderWindow *renderWindow = vtkItem->GetRenderWindow();

	renderer = vtkSmartPointer<vtkRenderer>::New();
	renderWindow->AddRenderer(renderer);

	renderer->SetBackground(1, 1, 1);

	vtkSmartPointer<vtkInteractorStyleTrackballCamera> style = vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();

	renderWindow->GetInteractor()->SetInteractorStyle(style);
}

void Plot3DPrivate::clearActors(){
	foreach(vtkActor *actor, actors){
		renderer->RemoveActor(actor);
	}
	actors.clear();
}

void Plot3DPrivate::addSphere(){
	vtkSmartPointer<vtkSphereSource> sphereSource = vtkSmartPointer<vtkSphereSource>::New();
	sphereSource->Update();
	vtkSmartPointer<vtkPolyDataMapper> sphereMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	sphereMapper->SetInputConnection(sphereSource->GetOutputPort());
	vtkSmartPointer<vtkActor> sphereActor = vtkSmartPointer<vtkActor>::New();
	sphereActor->GetProperty()->SetFrontfaceCulling(true);
	sphereActor->SetMapper(sphereMapper);
	renderer->AddActor(sphereActor);
	actors.push_back(sphereActor);
}

void Plot3DPrivate::readFromFile(){
	vtkSmartPointer<vtkOBJReader> reader = vtkSmartPointer<vtkOBJReader>::New();
	qDebug() << Q_FUNC_INFO << "Read from the file:" << path.path().toAscii();
	reader->SetFileName(path.path().toAscii().constData());
	reader->Update();
	vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  mapper->SetInputConnection(reader->GetOutputPort());

  vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
  actor->SetMapper(mapper);

	renderer->AddActor(actor);
	actors.push_back(actor);
}

void Plot3DPrivate::retransform(){
	prepareGeometryChange();
	setPos(rect.x()+rect.width()/2, rect.y()+rect.height()/2);

	//plotArea position is always (0, 0) in parent's coordinates, don't need to update here
	Q_Q(Plot3D);
	q->plotArea()->setRect(rect);

	if (isChanged){
		clearActors();
		if (sourceType == Plot3D::DataSource_Empty){
			qDebug() << Q_FUNC_INFO << "Add Sphere";
			addSphere();
		}else if(sourceType == Plot3D::DataSource_File){
			qDebug() << Q_FUNC_INFO << "Read file";
			readFromFile();
		}else if(sourceType == Plot3D::DataSource_Spreadsheet){
		}

		isChanged = false;
	}
	vtkItem->setGeometry(q->plotArea()->rect());

	WorksheetElementContainerPrivate::recalcShapeAndBoundingRect();
}