/***************************************************************************
    File                 : Light3DDock.cpp
    Project              : LabPlot
    Description          : widget for 3D Light properties
    --------------------------------------------------------------------
    Copyright            : (C) 2015 Minh Ngo (minh@fedoraproject.org)

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

#include "Light3DDock.h"
#include "DockHelpers.h"
#include "backend/worksheet/plots/3d/Light.h"

using namespace DockHelpers;

Light3DDock::Light3DDock(QWidget* parent)
	: QWidget(parent)
	, light(0)
	, m_initializing(false) {
	ui.setupUi(this);

	connect(ui.sbXFocalPoint, SIGNAL(valueChanged(double)), SLOT(onFocalPointChanged(double)));

	connect(ui.sbXPosition, SIGNAL(valueChanged(double)), SLOT(onPositionChanged(double)));

	connect(ui.sbLightIntensity, SIGNAL(valueChanged(double)), SLOT(onIntensityChanged(double)));

	connect(ui.kcbLightAmbientColor, SIGNAL(changed(const QColor&)), SLOT(onAmbientChanged(const QColor&)));
	connect(ui.kcbLightDiffuseColor, SIGNAL(changed(const QColor&)), SLOT(onDiffuseChanged(const QColor&)));
	connect(ui.kcbLightSpecularColor, SIGNAL(changed(const QColor&)), SLOT(onSpecularChanged(const QColor&)));

	connect(ui.sbLightElevation, SIGNAL(valueChanged(int)), SLOT(onElevationChanged(int)));
	connect(ui.sbLightAzimuth, SIGNAL(valueChanged(int)), SLOT(onAzimuthChanged(int)));
	connect(ui.sbLightConeAngle, SIGNAL(valueChanged(int)), SLOT(onConeAngleChanged(int)));
}

void Light3DDock::onFocalPointChanged(double) {
	const Lock lock(m_initializing);
	light->setFocalPoint(QVector3D(ui.sbXFocalPoint->value(), ui.sbYFocalPoint->value(), ui.sbZFocalPoint->value()));
}

void Light3DDock::onPositionChanged(double) {
	const Lock lock(m_initializing);
	light->setPosition(QVector3D(ui.sbXPosition->value(), ui.sbYPosition->value(), ui.sbZPosition->value()));
}

void Light3DDock::onIntensityChanged(double value) {
	const Lock lock(m_initializing);
	light->setIntensity(value);
}

void Light3DDock::onAmbientChanged(const QColor& color) {
	const Lock lock(m_initializing);
	light->setAmbient(color);
}

void Light3DDock::onDiffuseChanged(const QColor& color) {
	const Lock lock(m_initializing);
	light->setDiffuse(color);
}

void Light3DDock::onSpecularChanged(const QColor& color) {
	const Lock lock(m_initializing);
	light->setSpecular(color);
}

void Light3DDock::onElevationChanged(int elevation) {
	const Lock lock(m_initializing);
	light->setElevation(elevation);
}

void Light3DDock::onAzimuthChanged(int azimuth) {
	const Lock lock(m_initializing);
	light->setAzimuth(azimuth);
}

void Light3DDock::onConeAngleChanged(int angle) {
	const Lock lock(m_initializing);
	light->setConeAngle(angle);
}

void Light3DDock::setLight(Light *light) {
	if (this->light)
		this->light->disconnect(this);

	this->light = light;

	{
	const SignalBlocker blocker(this);
	const QVector3D& fp = light->focalPoint();
	focalPointChanged(fp);

	const QVector3D& pos = light->position();
	positionChanged(pos);

	intensityChanged(light->intensity());
	ambientChanged(light->ambient());
	diffuseChanged(light->diffuse());
	specularChanged(light->specular());

	elevationChanged(light->elevation());
	azimuthChanged(light->azimuth());
	coneAngleChanged(light->coneAngle());
	}

	connect(light, SIGNAL(focalPointChanged(const QVector3D&)), SLOT(focalPointChanged(const QVector3D&)));
	connect(light, SIGNAL(positionChanged(const QVector3D&)), SLOT(positionChanged(const QVector3D&)));

	connect(light, SIGNAL(intensityChanged(double)), SLOT(intensityChanged(double)));

	connect(light, SIGNAL(ambientChanged(const QColor&)), SLOT(ambientChanged(const QColor&)));
	connect(light, SIGNAL(diffuseChanged(const QColor&)), SLOT(diffuseChanged(const QColor&)));
	connect(light, SIGNAL(specularChanged(const QColor&)), SLOT(specularChanged(const QColor&)));

	connect(light, SIGNAL(elevationChanged(double)), SLOT(elevationChanged(double)));
	connect(light, SIGNAL(azimuthChanged(double)), SLOT(azimuthChanged(double)));
	connect(light, SIGNAL(coneAngleChanged(double)), SLOT(coneAngleChanged(double)));
}

void Light3DDock::focalPointChanged(const QVector3D& fp) {
	if (m_initializing)
		return;
	ui.sbXFocalPoint->setValue(fp.x());
	ui.sbYFocalPoint->setValue(fp.y());
	ui.sbZFocalPoint->setValue(fp.z());
}

void Light3DDock::positionChanged(const QVector3D& pos) {
	if (m_initializing)
		return;
	ui.sbXPosition->setValue(pos.x());
	ui.sbYPosition->setValue(pos.y());
	ui.sbZPosition->setValue(pos.z());
}

void Light3DDock::intensityChanged(double val) {
	if (m_initializing)
		return;
	ui.sbLightIntensity->setValue(val);
}

void Light3DDock::ambientChanged(const QColor& color) {
	if (m_initializing)
		return;
	ui.kcbLightAmbientColor->setColor(color);
}

void Light3DDock::diffuseChanged(const QColor& color) {
	if (m_initializing)
		return;
	ui.kcbLightDiffuseColor->setColor(color);
}

void Light3DDock::specularChanged(const QColor& color) {
	if (m_initializing)
		return;
	ui.kcbLightSpecularColor->setColor(color);
}

void Light3DDock::elevationChanged(double val) {
	if (m_initializing)
		return;
	ui.sbLightElevation->setValue(val);
}

void Light3DDock::azimuthChanged(double val) {
	if (m_initializing)
		return;
	ui.sbLightAzimuth->setValue(val);
}

void Light3DDock::coneAngleChanged(double val) {
	if (m_initializing)
		return;
	ui.sbLightConeAngle->setValue(val);
}