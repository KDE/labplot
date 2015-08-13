/***************************************************************************
    File                 : Base3D.cpp
    Project              : LabPlot
    Description          : Base class for 3D objects
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

#include "Base3D.h"
#include "Base3DPrivate.h"

#include <QDebug>

#include <KIcon>

#include <cmath>

#include <vtkRenderer.h>
#include <vtkProperty.h>
#include <vtkActor.h>
#include <vtkAssembly.h>
#include <vtkProp3DCollection.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkBoundingBox.h>
#include <vtkPlane.h>
#include <vtkNew.h>
#include <vtkClipPolyData.h>
#include <vtkAlgorithmOutput.h>

Base3D::Base3D(Base3DPrivate* priv)
	: AbstractAspect(priv->name())
	, d_ptr(priv) {
	Q_D(Base3D);
	d->update();
}

Base3D::~Base3D() {
}

QIcon Base3D::icon() const {
	return KIcon("xy-curve");
}

void Base3D::setRenderer(vtkRenderer* renderer) {
	Q_D(Base3D);
	d->renderer = renderer;
	if (renderer) {
		d->renderer->AddViewProp(d->actor);
		d->update();
	}
}

void Base3D::setXScaling(Plot3D::Scaling scaling) {
	Q_D(Base3D);
	d->xScaling = scaling;
	d->updateScaling();
}

void Base3D::setYScaling(Plot3D::Scaling scaling) {
	Q_D(Base3D);
	d->yScaling = scaling;
	d->updateScaling();
}

void Base3D::setZScaling(Plot3D::Scaling scaling) {
	Q_D(Base3D);
	d->zScaling = scaling;
	d->updateScaling();
}

void Base3D::highlight(bool pred) {
	Q_D(Base3D);
	vtkProperty *prop = d->actor->GetProperty();
	if (pred && !d->isHighlighted) {
		d->isHighlighted = true;
		if (!d->isSelected)
			d->property->DeepCopy(prop);
		prop->SetColor(1.0, 1.0, 0.0);
		prop->SetDiffuse(1.0);
		prop->SetSpecular(0.0);
	} else if (d->isHighlighted && !pred) {
		d->isHighlighted = false;
		prop->DeepCopy(d->property);
		if (d->isSelected) {
			d->isSelected = false;
			select(true);
		}
	}
}

void Base3D::select(bool pred) {
	Q_D(Base3D);
	vtkProperty *prop = d->actor->GetProperty();
	if (pred && !d->isSelected) {
		d->isSelected = true;
		if (!d->isHighlighted)
			d->property->DeepCopy(prop);
		prop->SetColor(1.0, 0.0, 0.0);
		prop->SetDiffuse(1.0);
		prop->SetSpecular(0.0);
	} else if (d->isSelected && !pred) {
		d->isSelected = false;
		d->isHighlighted = false;
		prop->DeepCopy(d->property);
	}
}

bool Base3D::operator==(vtkProp* prop) const {
	Q_D(const Base3D);
	return dynamic_cast<vtkProp*>(d->actor.GetPointer()) == prop;
}

bool Base3D::operator!=(vtkProp* prop) const {
	return !operator==(prop);
}

void Base3D::show(bool pred) {
	Q_D(Base3D);
	d->actor->SetVisibility(pred);
	emit parametersChanged();
}

bool Base3D::isVisible() const {
	Q_D(const Base3D);
	return d->actor->GetVisibility() != 0;
}

void Base3D::remove() {
	Q_D(Base3D);
	d->renderer->RemoveViewProp(d->actor);
	emit removed();
	AbstractAspect::remove();
}

void Base3D::recover() {
	Q_D(Base3D);
	if (!d->renderer->HasViewProp(d->actor)) {
		d->renderer->AddViewProp(d->actor);
	}
}

////////////////////////////////////////////////////////////////////////////////

Base3DPrivate::Base3DPrivate(const QString& name, Base3D* baseParent, vtkActor* actor)
	: xScaling(Plot3D::Scaling_Linear)
	, yScaling(Plot3D::Scaling_Linear)
	, zScaling(Plot3D::Scaling_Linear)
	, baseParent(baseParent)
	, aspectName(name)
	, isHighlighted(false)
	, isSelected(false)
	, renderer(0)
	, actor(actor == 0 ? vtkActor::New() : actor)
	, property(vtkProperty::New()){
}

const QString& Base3DPrivate::name() const {
	return aspectName;
}

Base3DPrivate::~Base3DPrivate() {
}

void Base3DPrivate::updateBounds() {
	updateBounds(actor);
}

vtkSmartPointer<vtkPolyData> Base3DPrivate::createData() const {
	return 0;
}

void Base3DPrivate::modifyActor(vtkRenderer* renderer, vtkActor* actor) const {
	Q_UNUSED(renderer);
	Q_UNUSED(actor);
}

void Base3DPrivate::objectScaled(vtkActor* actor) const {
	Q_UNUSED(actor);
}

void Base3DPrivate::updateBounds(vtkActor* actor) const {
	Q_UNUSED(actor);
}

void Base3DPrivate::updateScaling(bool notify) {
	if (!isInitialized())
		return;

	objectScaled(actor);

	scaledPolyData = scale(polyData);
	// Maps scaledPolyData to actor
	mapData(scaledPolyData);
	if (notify) {
		emit baseParent->parametersChanged();
		emit baseParent->visibilityChanged(true);
	}
}

void Base3DPrivate::update() {
	if (!isInitialized())
		return;

	polyData = createData();

	updateScaling(false);
	modifyActor(renderer, actor);

	if (actor) {
		// Initially actor is in the unclicked state
		property->DeepCopy(actor->GetProperty());
		emit baseParent->parametersChanged();
		emit baseParent->visibilityChanged(true);
	}
}

void Base3DPrivate::getBounds(double bounds[6]) const {
	vtkPropCollection* actors = renderer->GetViewProps();
	actors->InitTraversal();

	vtkBoundingBox bb;
	if (actors->GetNumberOfItems() > 1) {
		vtkProp* actor = 0;
		while ((actor = actors->GetNextProp()) != 0) {
			if (actor == this->actor.GetPointer())
				continue;

			bb.AddBounds(actor->GetBounds());
		}

		bb.GetBounds(bounds);
	} else {
		bounds[0] = bounds[2] = bounds[4] = -1;
		bounds[1] = bounds[3] = bounds[5] = 1;
	}
}

bool Base3DPrivate::isInitialized() const {
	return renderer;
}

void Base3DPrivate::mapData(vtkPolyData* data) {
	//reader fails to read obj-files if the locale is not set to 'C'
	setlocale(LC_NUMERIC, "C");

	vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->SetInputData(data);
	actor->SetMapper(mapper);
}

namespace {
	double self(double val) { return val; }
}

vtkSmartPointer<vtkPolyData> Base3DPrivate::scale(vtkPolyData* data) {
	if (data == 0)
		return 0;

	if (xScaling == Plot3D::Scaling_Linear && yScaling == Plot3D::Scaling_Linear
			&& zScaling == Plot3D::Scaling_Linear)
		return data;

	vtkSmartPointer<vtkPolyData> result = vtkSmartPointer<vtkPolyData>::New();
	result->DeepCopy(data);

	vtkNew<vtkPlane> planeX;
	planeX->SetOrigin(0.00000001, 0, 0);
	planeX->SetNormal(1, 0, 0);

	vtkNew<vtkClipPolyData> clipperX;
	clipperX->SetClipFunction(planeX.GetPointer());

	vtkNew<vtkPlane> planeY;
	planeY->SetOrigin(0, 0.00000001, 0);
	planeY->SetNormal(0, 1, 0);

	vtkNew<vtkClipPolyData> clipperY;
	clipperY->SetClipFunction(planeY.GetPointer());

	vtkNew<vtkPlane> planeZ;
	planeZ->SetOrigin(0, 0, 0.00000001);
	planeZ->SetNormal(0, 0, 1);

	vtkNew<vtkClipPolyData> clipperZ;
	clipperZ->SetClipFunction(planeZ.GetPointer());

	if (xScaling != Plot3D::Scaling_Linear) {
		clipperX->SetInputData(result);
		clipperX->Update();
		result = clipperX->GetOutput();
	}

	if (yScaling != Plot3D::Scaling_Linear) {
		clipperY->SetInputData(result);
		clipperY->Update();
		result = clipperY->GetOutput();
	}

	if (zScaling != Plot3D::Scaling_Linear) {
		clipperZ->SetInputData(result);
		clipperZ->Update();
		result = clipperZ->GetOutput();
	}

	double (*scaleX)(double);
	double (*scaleY)(double);
	double (*scaleZ)(double);

	if (xScaling == Plot3D::Scaling_Ln)
		scaleX = log;
	else if (xScaling == Plot3D::Scaling_Log10)
		scaleX = log10;
	else if (xScaling == Plot3D::Scaling_Log2)
		scaleX = log2;
	else
		scaleX = self;

	if (yScaling == Plot3D::Scaling_Ln)
		scaleY = log;
	else if (yScaling == Plot3D::Scaling_Log10)
		scaleY = log10;
	else if (yScaling == Plot3D::Scaling_Log2)
		scaleY = log2;
	else
		scaleY = self;

	if (zScaling == Plot3D::Scaling_Ln)
		scaleZ = log;
	else if (zScaling == Plot3D::Scaling_Log10)
		scaleZ = log10;
	else if (zScaling == Plot3D::Scaling_Log2)
		scaleZ = log2;
	else
		scaleZ = self;

	scale(result, scaleX, scaleY, scaleZ);
	return result;
}

void Base3DPrivate::scale(vtkPolyData* data, double (*scaleX)(double),
		double (*scaleY)(double),  double (*scaleZ)(double)) {
	vtkPoints* points = data->GetPoints();
	double point[3];
	for (vtkIdType i = 0, max = points->GetNumberOfPoints(); i < max; ++i) {
		points->GetPoint(i, point);
		point[0] = scaleX(point[0]);
		point[1] = scaleY(point[1]);
		point[2] = scaleZ(point[2]);
		points->SetPoint(i, point);
	}

}