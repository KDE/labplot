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

#include <vtkRenderer.h>
#include <vtkProperty.h>
#include <vtkActor.h>

Base3D::Base3D(const QString& name, Base3DPrivate* priv)
	: AbstractAspect(name)
	, d_ptr(priv) {
	Q_D(Base3D);
	if (d->renderer)
		d->init();
}

Base3D::~Base3D() {

}

void Base3D::setRenderer(vtkRenderer* renderer) {
	Q_D(Base3D);
	d->renderer = renderer;
	if (renderer)
		d->init();
}

void Base3D::highlight(bool pred) {
	Q_D(Base3D);
	if (pred && !d->isSelected) {
		d->isSelected = pred;
		vtkProperty *prop = d->actor->GetProperty();
		d->property->DeepCopy(prop);
		prop->SetColor(1.0, 0.0, 0.0);
		prop->SetDiffuse(1.0);
		prop->SetSpecular(0.0);
	} else if (d->isSelected && !pred) {
		d->isSelected = pred;
		d->actor->GetProperty()->DeepCopy(d->property);
	}
}

bool Base3D::operator==(vtkProp* prop) const {
	Q_D(const Base3D);
	return dynamic_cast<vtkProp*>(d->actor.Get()) == prop;
}

bool Base3D::operator!=(vtkProp* prop) const {
	return !operator==(prop);
}

void Base3D::show(bool pred) {
	Q_D(Base3D);
	if (d->actor) {
		d->actor->SetVisibility(pred);
		emit parametersChanged();
	}
}

bool Base3D::isVisible() const {
	Q_D(const Base3D);
	if (!d->actor)
		return false;
	return d->actor->GetVisibility() != 0;
}

void Base3D::update() {
	Q_D(Base3D);
	d->update();
}

void Base3D::remove() {
	Q_D(Base3D);
	d->hide();
	emit removed();
	AbstractAspect::remove();
}


////////////////////////////////////////////////////////////////////////////////

Base3DPrivate::Base3DPrivate(vtkRenderer *renderer)
	: isSelected(false)
	, renderer(renderer)
	, property(vtkProperty::New()) {
}

Base3DPrivate::~Base3DPrivate() {
}

void Base3DPrivate::hide() {
	if (actor && renderer)
		renderer->RemoveActor(actor);
}
