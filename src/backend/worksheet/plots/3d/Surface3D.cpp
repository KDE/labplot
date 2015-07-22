/***************************************************************************
    File                 : Surface3D.cpp
    Project              : LabPlot
    Description          : 3D surface class
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

#include "Surface3D.h"
#include "Surface3DPrivate.h"
#include "Plot3D.h"
#include "XmlAttributeReader.h"
#include "backend/lib/commandtemplates.h"

#include <QDebug>

#include <KLocale>

#include <vtkRenderer.h>
#include <vtkProperty.h>

Surface3D::Surface3D(vtkRenderer* renderer)
	: AbstractAspect(i18n("Surface"))
	, d_ptr(new Surface3DPrivate(renderer, this)) {
	Q_D(Surface3D);
	if (renderer)
		d->init();
}

void Surface3D::setRenderer(vtkRenderer* renderer) {
	Q_D(Surface3D);
	d->renderer = renderer;
	if (renderer)
		d->init();
}

void Surface3D::highlight(bool pred) {
	Q_D(Surface3D);

	if (pred && !d->isSelected) {
		d->isSelected = pred;
		vtkProperty *prop = d->surfaceActor->GetProperty();
		d->surfaceProperty->DeepCopy(prop);
		prop->SetColor(1.0, 0.0, 0.0);
		prop->SetDiffuse(1.0);
		prop->SetSpecular(0.0);
	} else if (d->isSelected && !pred) {
		d->isSelected = pred;
		d->surfaceActor->GetProperty()->DeepCopy(d->surfaceProperty);
	}
}

Surface3D::~Surface3D() {
}

bool Surface3D::operator==(vtkProp* prop) const {
	Q_D(const Surface3D);
	return dynamic_cast<vtkProp*>(d->surfaceActor.Get()) == prop;
}

bool Surface3D::operator!=(vtkProp* prop) const {
	return !operator==(prop);
}

DemoDataHandler& Surface3D::demoDataHandler() {
	Q_D(Surface3D);
	return *d->demoHandler;
}

SpreadsheetDataHandler& Surface3D::spreadsheetDataHandler() {
	Q_D(Surface3D);
	return *d->spreadsheetHandler;
}

MatrixDataHandler& Surface3D::matrixDataHandler() {
	Q_D(Surface3D);
	return *d->matrixHandler;
}

FileDataHandler& Surface3D::fileDataHandler() {
	Q_D(Surface3D);
	return *d->fileHandler;
}

void Surface3D::remove(){
	Q_D(Surface3D);
	d->hide();
	emit removed();
	AbstractAspect::remove();
}

void Surface3D::update() {
	Q_D(Surface3D);
	d->update();
}

void Surface3D::show(bool pred) {
	Q_D(Surface3D);
	if (d->surfaceActor) {
		d->surfaceActor->SetVisibility(pred);
		emit parametersChanged();
	}
}

bool Surface3D::isVisible() const {
	Q_D(const Surface3D);
	if (!d->surfaceActor)
		return false;
	return d->surfaceActor->GetVisibility() != 0;
}

//##############################################################################
//##########################  getter methods  ##################################
//##############################################################################

BASIC_SHARED_D_READER_IMPL(Surface3D, Surface3D::VisualizationType, visualizationType, visualizationType)
BASIC_SHARED_D_READER_IMPL(Surface3D, Surface3D::DataSource, dataSource, sourceType)

//##############################################################################
//#################  setter methods and undo commands ##########################
//##############################################################################

STD_SETTER_CMD_IMPL_F_S(Surface3D, SetVisualizationType, Surface3D::VisualizationType, visualizationType, update)
STD_SETTER_IMPL(Surface3D, VisualizationType, Surface3D::VisualizationType, visualizationType, "%1: visualization type changed")

STD_SETTER_CMD_IMPL_F_S(Surface3D, SetDataSource, Surface3D::DataSource, sourceType, update)
STD_SETTER_IMPL(Surface3D, DataSource, Surface3D::DataSource, sourceType, "%1: data source type changed")

////////////////////////////////////////////////////////////////////////////////

Surface3DPrivate::Surface3DPrivate(vtkRenderer* renderer, Surface3D *parent)
	: q(parent)
	, renderer(renderer)
	, visualizationType(Surface3D::VisualizationType_Triangles)
	, sourceType(Surface3D::Surface3D::DataSource_Empty)
	, isSelected(false)
	, demoHandler(new DemoDataHandler)
	, spreadsheetHandler(new SpreadsheetDataHandler)
	, matrixHandler(new MatrixDataHandler)
	, fileHandler(new FileDataHandler)
	, surfaceProperty(vtkProperty::New()) {
}

void Surface3DPrivate::init() {
	q->addChild(demoHandler);
	demoHandler->setHidden(true);
	
	q->addChild(spreadsheetHandler);
	spreadsheetHandler->setHidden(true);
	
	q->addChild(matrixHandler);
	matrixHandler->setHidden(true);

	q->addChild(fileHandler);
	fileHandler->setHidden(true);

	update();
	q->connect(demoHandler, SIGNAL(parametersChanged()), SLOT(update()));
	q->connect(spreadsheetHandler, SIGNAL(parametersChanged()), SLOT(update()));
	q->connect(matrixHandler, SIGNAL(parametersChanged()), SLOT(update()));
	q->connect(fileHandler, SIGNAL(parametersChanged()), SLOT(update()));
}

Surface3DPrivate::~Surface3DPrivate() {
	hide();
}

QString Surface3DPrivate::name() const {
	return i18n("3D Surface");
}

void Surface3DPrivate::hide() {
	if (surfaceActor && renderer) {
		renderer->RemoveActor(surfaceActor);
	}
}

void Surface3DPrivate::update() {
	if (!renderer)
		return;

	hide();
	if (sourceType == Surface3D::DataSource_Empty) {
		surfaceActor = demoHandler->actor(visualizationType);
	} else if (sourceType == Surface3D::DataSource_File) {
		surfaceActor = fileHandler->actor(visualizationType);
	} else if (sourceType == Surface3D::DataSource_Matrix) {
		surfaceActor = matrixHandler->actor(visualizationType);
	} else if (sourceType == Surface3D::DataSource_Spreadsheet) {
		surfaceActor = spreadsheetHandler->actor(visualizationType);
	}

	renderer->AddActor(surfaceActor);
	emit q->parametersChanged();
	emit q->visibilityChanged(true);
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################
//! Save as XML
void Surface3D::save(QXmlStreamWriter* writer) const {
	Q_D(const Surface3D);

	writer->writeStartElement("surface3d");
		writer->writeAttribute("visualizationType", QString::number(d->visualizationType));
		writer->writeAttribute("sourceType", QString::number(d->sourceType));
		writeBasicAttributes(writer);
		writeCommentElement(writer);
		d->spreadsheetHandler->save(writer);
		d->matrixHandler->save(writer);
		d->fileHandler->save(writer);
	writer->writeEndElement();
}


//! Load from XML
bool Surface3D::load(XmlStreamReader* reader) {
	Q_D(Surface3D);

	const QXmlStreamAttributes& attribs = reader->attributes();
	XmlAttributeReader attributeReader(reader, attribs);
	attributeReader.checkAndLoadAttribute<VisualizationType>("visualizationType", d->visualizationType);
	attributeReader.checkAndLoadAttribute<DataSource>("sourceType", d->sourceType);

	if(!readBasicAttributes(reader)){
		return false;
	}

	while(!reader->atEnd()){
		reader->readNext();
		const QStringRef& sectionName = reader->name();
		if (reader->isEndElement() && sectionName == "surface3d")
			break;

		if (reader->isEndElement())
			continue;

		if (sectionName == "comment") {
			if (!readCommentElement(reader))
				return false;
		} else if (sectionName == "matrix") {
			qDebug() << Q_FUNC_INFO << "Load matrix";
			if (!d->matrixHandler->load(reader))
				return false;
		} else if (sectionName == "spreadsheet") {
			qDebug() << Q_FUNC_INFO << "Load spreadsheet";
			if (!d->spreadsheetHandler->load(reader))
				return false;
		} else if (sectionName == "file") {
			qDebug() << Q_FUNC_INFO << "Load file";
			if (!d->fileHandler->load(reader))
				return false;
		}
	}

	return true;
}