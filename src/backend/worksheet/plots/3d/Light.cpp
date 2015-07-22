/***************************************************************************
    File                 : Light.cpp
    Project              : LabPlot
    Description          : 3D plot light
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

#include "Light.h"
#include "LightPrivate.h"
#include "XmlAttributeReader.h"

#include "backend/lib/commandtemplates.h"
#include "backend/lib/macros.h"

#include <KIcon>
#include <KMenu>
#include <KLocale>

#include <vtkLight.h>
#include <vtkRenderer.h>

Light::Light(vtkRenderer* renderer, bool canRemove)
	: AbstractAspect(i18n(canRemove ? "Light" : "Main Light"))
	, d_ptr(new LightPrivate(renderer, this, canRemove)) {
}

Light::~Light() {
}

void Light::setRenderer(vtkRenderer* renderer) {
	Q_D(Light);
	d->renderer = renderer;
	d->init();
}

void Light::remove(){
	Q_D(Light);
	d->hide();
	emit removed();
	AbstractAspect::remove();
}

//##############################################################################
//##########################  getter methods  ##################################
//##############################################################################

BASIC_SHARED_D_READER_IMPL(Light, QVector3D, focalPoint, focalPoint)
BASIC_SHARED_D_READER_IMPL(Light, QVector3D, position, position)
BASIC_SHARED_D_READER_IMPL(Light, double, intensity, intensity)
BASIC_SHARED_D_READER_IMPL(Light, QColor, ambient, ambient)
BASIC_SHARED_D_READER_IMPL(Light, QColor, diffuse, diffuse)
BASIC_SHARED_D_READER_IMPL(Light, QColor, specular, specular)
BASIC_SHARED_D_READER_IMPL(Light, double, elevation, elevation)
BASIC_SHARED_D_READER_IMPL(Light, double, azimuth, azimuth)
BASIC_SHARED_D_READER_IMPL(Light, double, coneAngle, coneAngle)

//##############################################################################
//#################  setter methods and undo commands ##########################
//##############################################################################

STD_SETTER_CMD_IMPL_F_S(Light, SetFocalPoint, QVector3D, focalPoint, update)
STD_SETTER_IMPL(Light, FocalPoint, const QVector3D&, focalPoint, "%1: focal point changed")

STD_SETTER_CMD_IMPL_F_S(Light, SetPosition, QVector3D, position, update)
STD_SETTER_IMPL(Light, Position, const QVector3D&, position, "%1: position changed")

STD_SETTER_CMD_IMPL_F_S(Light, SetIntensity, double, intensity, update)
STD_SETTER_IMPL(Light, Intensity, double, intensity, "%1: intensity changed")

STD_SETTER_CMD_IMPL_F_S(Light, SetAmbient, QColor, ambient, update)
STD_SETTER_IMPL(Light, Ambient, const QColor&, ambient, "%1: ambient changed")

STD_SETTER_CMD_IMPL_F_S(Light, SetDiffuse, QColor, diffuse, update)
STD_SETTER_IMPL(Light, Diffuse, const QColor&, diffuse, "%1: diffuse changed")

STD_SETTER_CMD_IMPL_F_S(Light, SetSpecular, QColor, specular, update)
STD_SETTER_IMPL(Light, Specular, const QColor&, specular, "%1: specular changed")

STD_SETTER_CMD_IMPL_F_S(Light, SetElevation, double, elevation, update)
STD_SETTER_IMPL(Light, Elevation, double, elevation, "%1: elevation changed")

STD_SETTER_CMD_IMPL_F_S(Light, SetAzimuth, double, azimuth, update)
STD_SETTER_IMPL(Light, Azimuth, double, azimuth, "%1: azimuth changed")

STD_SETTER_CMD_IMPL_F_S(Light, SetConeAngle, double, coneAngle, update)
STD_SETTER_IMPL(Light, ConeAngle, double, coneAngle, "%1: coneAngle changed")

namespace {
	QString pointToString(const QVector3D& point) {
		return QString::number(point.x()) + "," + QString::number(point.y())
				+ "," + QString::number(point.z());
	}
}

void Light::save(QXmlStreamWriter* writer) const {
	Q_D(const Light);

	if (d->canRemove)
		writer->writeStartElement("light");
	else
		writer->writeStartElement("mainLight");

		writeBasicAttributes(writer);
		writer->writeAttribute("focalPoint", pointToString(d->focalPoint));
		writer->writeAttribute("position", pointToString(d->position));
		writer->writeAttribute("intensity", QString::number(d->intensity));
		writer->writeAttribute("ambient", d->ambient.name());
		writer->writeAttribute("diffuse", d->diffuse.name());
		writer->writeAttribute("specular", d->specular.name());
		writer->writeAttribute("elevation", QString::number(d->elevation));
		writer->writeAttribute("azimuth", QString::number(d->azimuth));
		writer->writeAttribute("coneAngle", QString::number(d->coneAngle));
	writer->writeEndElement();
}

bool Light::load(XmlStreamReader* reader) {
	Q_D(Light);
	const QXmlStreamAttributes& attribs = reader->attributes();
	XmlAttributeReader attributeReader(reader, attribs);
	attributeReader.checkAndLoadAttribute("focalPoint", d->focalPoint);
	attributeReader.checkAndLoadAttribute("position", d->position);
	attributeReader.checkAndLoadAttribute("intensity", d->intensity);
	attributeReader.checkAndLoadAttribute("ambient", d->ambient);
	attributeReader.checkAndLoadAttribute("diffuse", d->diffuse);
	attributeReader.checkAndLoadAttribute("specular", d->specular);
	attributeReader.checkAndLoadAttribute("elevation", d->elevation);
	attributeReader.checkAndLoadAttribute("azimuth", d->azimuth);
	attributeReader.checkAndLoadAttribute("coneAngle", d->coneAngle);
	return true;
}

QMenu* Light::createContextMenu() {
	// Reimplements createContextMenu to hide a delete button
	Q_D(const Light);
	if (d->canRemove)
		return AbstractAspect::createContextMenu();

	KMenu* menu = new KMenu();
	menu->addTitle(name());
	menu->addAction(KIcon("edit-rename"), i18n("Rename"), this, SIGNAL(renameRequested()));
	return menu;
}

////////////////////////////////////////////////////////////////////////////////

LightPrivate::LightPrivate(vtkRenderer* renderer, Light* parent, bool canRemove)
	: q(parent)
	, canRemove(canRemove)
	, renderer(renderer)
	, focalPoint(1.875, 0.6125, 0)
	, position(0.875, 1.6125, 1)
	, intensity(1.0)
	, ambient(Qt::white)
	, diffuse(Qt::white)
	, specular(Qt::white)
	, elevation(15)
	, azimuth(15)
	, coneAngle(15) {
}

LightPrivate::~LightPrivate() {
}

void LightPrivate::init() {
	if (!renderer)
		return;

	light = vtkSmartPointer<vtkLight>::New();
	light->SetFocalPoint(focalPoint.x(), focalPoint.y(), focalPoint.z());
	light->SetPosition(position.x(), position.y(), position.z());
	light->SetIntensity(intensity);
	light->SetAmbientColor(ambient.redF(), ambient.greenF(), ambient.blueF());
	light->SetDiffuseColor(diffuse.redF(), diffuse.greenF(), diffuse.blueF());
	light->SetSpecularColor(specular.redF(), specular.greenF(), specular.blueF());
	renderer->AddLight(light);
}

void LightPrivate::hide() {
	if (light && renderer)
		renderer->RemoveLight(light);
}

void LightPrivate::update() {
	hide();
	init();
	emit q->parametersChanged();
}

QString LightPrivate::name() const {
	return i18n("Light");
}