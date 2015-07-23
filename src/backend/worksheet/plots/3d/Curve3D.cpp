/***************************************************************************
    File                 : Curve3D.cpp
    Project              : LabPlot
    Description          : 3D curve class
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

#include "Curve3D.h"
#include "Curve3DPrivate.h"
#include "XmlAttributeReader.h"

#include "backend/lib/commandtemplates.h"
#include "backend/lib/macros.h"
#include "backend/core/AbstractColumn.h"

#include <KLocale>

#include <vtkActor.h>
#include <vtkRenderer.h>
#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>

Curve3D::Curve3D(vtkRenderer* renderer)
	: AbstractAspect(i18n("Curve 3D"))
	, d_ptr(new Curve3DPrivate(renderer, this)) {
	Q_D(Curve3D);
	if (renderer)
		d->init();
}

Curve3D::~Curve3D() {
}

void Curve3D::setRenderer(vtkRenderer* renderer) {
	Q_D(Curve3D);
	// TODO: Move to the base class between Curve3D and Surface3D
	d->renderer = renderer;
	if (renderer)
		d->init();
}

void Curve3D::highlight(bool pred) {
	Q_D(Curve3D);

	// TODO: Move to the base class between Curve3D and Surface3D
	if (pred && !d->isSelected) {
		d->isSelected = pred;
		vtkProperty *prop = d->curveActor->GetProperty();
		d->curveProperty->DeepCopy(prop);
		prop->SetColor(1.0, 0.0, 0.0);
		prop->SetDiffuse(1.0);
		prop->SetSpecular(0.0);
	} else if (d->isSelected && !pred) {
		d->isSelected = pred;
		d->curveActor->GetProperty()->DeepCopy(d->curveProperty);
	}
}

void Curve3D::save(QXmlStreamWriter* writer) const {
	Q_D(const Curve3D);

	writer->writeStartElement("curve3d");
		WRITE_COLUMN(d->xColumn, xColumn);
		WRITE_COLUMN(d->yColumn, yColumn);
		WRITE_COLUMN(d->zColumn, zColumn);
		writer->writeAttribute("pointRadius", QString::number(d->pointRadius));
		writer->writeAttribute("showEdges", QString::number(d->showEdges));
		writer->writeAttribute("isClosed", QString::number(d->isClosed));
		writeBasicAttributes(writer);
		writeCommentElement(writer);
	writer->writeEndElement();
}

bool Curve3D::load(XmlStreamReader* reader) {
	Q_D(Curve3D);
	const QXmlStreamAttributes& attribs = reader->attributes();
	QString str;
	READ_COLUMN(xColumn);
	READ_COLUMN(yColumn);
	READ_COLUMN(zColumn);
	XmlAttributeReader attributeReader(reader, attribs);
	attributeReader.checkAndLoadAttribute("pointRadius", d->pointRadius);
	attributeReader.checkAndLoadAttribute("showEdges", d->showEdges);
	attributeReader.checkAndLoadAttribute("isClosed", d->isClosed);

	if(!readBasicAttributes(reader)){
		return false;
	}

	while(!reader->atEnd()){
		reader->readNext();
		const QStringRef& sectionName = reader->name();
		if (reader->isEndElement() && sectionName == "curve3d")
			break;

		if (reader->isEndElement())
			continue;

		if (sectionName == "comment") {
			if (!readCommentElement(reader))
				return false;
		}
	}

	return true;
}

// TODO: Move to the base class between Curve3D and Surface3D
bool Curve3D::operator==(vtkProp* prop) const {
	Q_D(const Curve3D);
	return dynamic_cast<vtkProp*>(d->curveActor.Get()) == prop;
}

bool Curve3D::operator!=(vtkProp* prop) const {
	return !operator==(prop);
}

//##############################################################################
//##########################  getter methods  ##################################
//##############################################################################

BASIC_SHARED_D_READER_IMPL(Curve3D, const AbstractColumn*, xColumn, xColumn)
BASIC_SHARED_D_READER_IMPL(Curve3D, const AbstractColumn*, yColumn, yColumn)
BASIC_SHARED_D_READER_IMPL(Curve3D, const AbstractColumn*, zColumn, zColumn)

const QString& Curve3D::xColumnPath() const { Q_D(const Curve3D); return d->xColumnPath; }
const QString& Curve3D::yColumnPath() const { Q_D(const Curve3D); return d->yColumnPath; }
const QString& Curve3D::zColumnPath() const { Q_D(const Curve3D); return d->zColumnPath; }

BASIC_SHARED_D_READER_IMPL(Curve3D, float, pointRadius, pointRadius)
BASIC_SHARED_D_READER_IMPL(Curve3D, bool, showEdges, showEdges)
BASIC_SHARED_D_READER_IMPL(Curve3D, bool, isClosed, isClosed)

//##############################################################################
//#################  setter methods and undo commands ##########################
//##############################################################################

STD_SETTER_CMD_IMPL_F_S(Curve3D, SetXColumn, const AbstractColumn*, xColumn, update)
STD_SETTER_COLUMN_IMPL(Curve3D, XColumn, xColumn, "%1: X column changed")

STD_SETTER_CMD_IMPL_F_S(Curve3D, SetYColumn, const AbstractColumn*, yColumn, update)
STD_SETTER_COLUMN_IMPL(Curve3D, YColumn, yColumn, "%1: Y column changed")

STD_SETTER_CMD_IMPL_F_S(Curve3D, SetZColumn, const AbstractColumn*, zColumn, update)
STD_SETTER_COLUMN_IMPL(Curve3D, ZColumn, zColumn, "%1: Z column changed")

STD_SETTER_CMD_IMPL_F_S(Curve3D, SetPointRadius, float, pointRadius, update)
STD_SETTER_IMPL(Curve3D, PointRadius, float, pointRadius, "%1: point radius changed")

STD_SETTER_CMD_IMPL_F_S(Curve3D, SetShowEdges, bool, showEdges, update)
STD_SETTER_IMPL(Curve3D, ShowEdges, bool, showEdges, "%1: show edges flag changed")

STD_SETTER_CMD_IMPL_F_S(Curve3D, SetIsClosed, bool, isClosed, update)
STD_SETTER_IMPL(Curve3D, IsClosed, bool, isClosed, "%1: closed flag changed")

void Curve3D::xColumnAboutToBeRemoved(const AbstractAspect*) {
	Q_D(Curve3D);
	d->xColumn = 0;
}

void Curve3D::yColumnAboutToBeRemoved(const AbstractAspect*) {
	Q_D(Curve3D);
	d->yColumn = 0;
}

void Curve3D::zColumnAboutToBeRemoved(const AbstractAspect*) {
	Q_D(Curve3D);
	d->zColumn = 0;
}

// TODO: Move to the base class between Curve3D and Surface3D
void Curve3D::remove() {
	Q_D(Curve3D);
	d->hide();
	emit removed();
	AbstractAspect::remove();
}

void Curve3D::update() {
	Q_D(Curve3D);
	d->update();
}

void Curve3D::show(bool pred) {
	Q_D(Curve3D);
	if (d->curveActor) {
		d->curveActor->SetVisibility(pred);
		emit parametersChanged();
	}
}

bool Curve3D::isVisible() const {
	Q_D(const Curve3D);
	if (!d->curveActor)
		return false;
	return d->curveActor->GetVisibility() != 0;
}

////////////////////////////////////////////////////////////////////////////////

Curve3DPrivate::Curve3DPrivate(vtkRenderer* renderer, Curve3D* parent)
	: q(parent)
	, isSelected(false)
	, renderer(renderer)
	, xColumn(0)
	, yColumn(0)
	, zColumn(0)
	, pointRadius(20.0)
	, showEdges(true)
	, isClosed(false)
	, curveProperty(vtkProperty::New()) {
}

void Curve3DPrivate::init() {
	update();
}

Curve3DPrivate::~Curve3DPrivate() {
}

QString Curve3DPrivate::name() const {
	return i18n("Curve 3D");
}

void Curve3DPrivate::update() {
	if (!renderer)
		return;

	if (curveActor)
		renderer->RemoveActor(curveActor);

	if (xColumn == 0 || yColumn == 0 || zColumn == 0)
		return;

	vtkSmartPointer<vtkPolyData> pdata = vtkSmartPointer<vtkPolyData>::New();
	
	vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
	vtkSmartPointer<vtkCellArray> vertices = vtkSmartPointer<vtkCellArray>::New();

	// TODO: Remove duplicate in DataHandlers
	const int numPoints = std::min(xColumn->rowCount(),
			std::min(yColumn->rowCount(), zColumn->rowCount()));
	if (numPoints == 0)
		return;

	vtkIdType pid[1];
	if (showEdges) {
		points->SetNumberOfPoints(numPoints);
		vertices->InsertNextCell(isClosed ? numPoints + 1 : numPoints);
	}
	for (int i = 0; i < numPoints; ++i) {
		const int x = static_cast<int>(xColumn->valueAt(i));
		const int y = static_cast<int>(yColumn->valueAt(i));
		const int z = static_cast<int>(zColumn->valueAt(i));

		if (showEdges) {
			points->SetPoint(i, x, y, z);
			vertices->InsertCellPoint(i);
		} else {
			pid[0] = points->InsertNextPoint(x, y, z);
			vertices->InsertNextCell(1, pid);
		}
	}

	if (showEdges && isClosed)
		vertices->InsertCellPoint(0);

	pdata->SetPoints(points);
	if (showEdges)
		pdata->SetLines(vertices);
	else
		pdata->SetVerts(vertices);

	vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->SetInputData(pdata);

	curveActor = vtkSmartPointer<vtkActor>::New();
	curveActor->SetMapper(mapper);
	curveActor->GetProperty()->SetPointSize(pointRadius);
	curveActor->GetProperty()->SetLineWidth(pointRadius);

	renderer->AddActor(curveActor);
	emit q->parametersChanged();
	emit q->visibilityChanged(true);
}

// TODO: Move to the base class between Curve3D and Surface3D
void Curve3DPrivate::hide() {
	if (curveActor && renderer)
		renderer->RemoveActor(curveActor);
}