/***************************************************************************
    File                 : LightPrivate.h
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

#ifndef PLOT3D_LIGHTPRIVATE_H
#define PLOT3D_LIGHTPRIVATE_H

#include <QVector3D>
#include <QColor>

#include <vtkSmartPointer.h>

class vtkRenderer;
class vtkLight;

class Light;
struct LightPrivate {
	Light* const q;

	vtkSmartPointer<vtkRenderer> renderer;
	vtkSmartPointer<vtkLight> light;

	QVector3D focalPoint;
	QVector3D position;
	double intensity;
	QColor ambient;
	QColor diffuse;
	QColor specular;
	double elevation;
	double azimuth;
	double coneAngle;

	LightPrivate(vtkRenderer* renderer, Light* parent);
	~LightPrivate();

	void init();
	void hide();
	void update();

	QString name() const;
};

#endif