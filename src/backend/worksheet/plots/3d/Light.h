/***************************************************************************
    File                 : Light.h
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

#ifndef PLOT3D_LIGHT_H
#define PLOT3D_LIGHT_H

#include "backend/core/AbstractAspect.h"
#include "backend/lib/macros.h"

#include <QVector3D>
#include <QColor>

class vtkRenderer;

class LightPrivate;
class Light : public AbstractAspect {
		Q_OBJECT
		Q_DECLARE_PRIVATE(Light)
		Q_DISABLE_COPY(Light)
	public:
		Light(vtkRenderer* renderer = 0, bool canRemove = true);
		virtual ~Light();

		void setRenderer(vtkRenderer* renderer);

		virtual void save(QXmlStreamWriter*) const;
		virtual bool load(XmlStreamReader*);

		QMenu* createContextMenu();

		CLASS_D_ACCESSOR_DECL(QVector3D, focalPoint, FocalPoint);
		CLASS_D_ACCESSOR_DECL(QVector3D, position, Position);
		BASIC_D_ACCESSOR_DECL(double, intensity, Intensity); // From 1 to 0
		CLASS_D_ACCESSOR_DECL(QColor, ambient, Ambient);
		CLASS_D_ACCESSOR_DECL(QColor, diffuse, Diffuse);
		CLASS_D_ACCESSOR_DECL(QColor, specular, Specular);
		BASIC_D_ACCESSOR_DECL(double, elevation, Elevation);
		BASIC_D_ACCESSOR_DECL(double, azimuth, Azimuth);
		BASIC_D_ACCESSOR_DECL(double, coneAngle, ConeAngle);

		typedef Light BaseClass;
		typedef LightPrivate Private;

	signals:
		friend class LightSetFocalPointCmd;
		friend class LightSetPositionCmd;
		friend class LightSetIntensityCmd;
		friend class LightSetAmbientCmd;
		friend class LightSetDiffuseCmd;
		friend class LightSetSpecularCmd;
		friend class LightSetElevationCmd;
		friend class LightSetAzimuthCmd;
		friend class LightSetConeAngleCmd;
		void focalPointChanged(const QVector3D&);
		void positionChanged(const QVector3D&);
		void intensityChanged(double);
		void ambientChanged(const QColor&);
		void diffuseChanged(const QColor&);
		void specularChanged(const QColor&);
		void elevationChanged(double);
		void azimuthChanged(double);
		void coneAngleChanged(double);
		void parametersChanged();

	private:
		const QScopedPointer<LightPrivate> d_ptr;
};

#endif