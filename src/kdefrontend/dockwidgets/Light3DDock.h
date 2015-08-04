/***************************************************************************
    File                 : Light3DDock.h
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

#ifndef LIGHT3DDOCK_H
#define LIGHT3DDOCK_H

#include <QWidget>

#include "ui_light3ddock.h"

class Light;

class Light3DDock : public QWidget {
		Q_OBJECT
	public:
		explicit Light3DDock(QWidget* parent);
		void setLight(Light *light);

	private slots:
		void onFocalPointChanged(double);

		void onPositionChanged(double);

		void onIntensityChanged(double);
		void onAmbientChanged(const QColor&);
		void onDiffuseChanged(const QColor&);
		void onSpecularChanged(const QColor&);
		void onElevationChanged(int);
		void onAzimuthChanged(int);
		void onConeAngleChanged(int);

		void focalPointChanged(const QVector3D&);
		void positionChanged(const QVector3D&);
		void intensityChanged(double);
		void ambientChanged(const QColor&);
		void diffuseChanged(const QColor&);
		void specularChanged(const QColor&);
		void elevationChanged(double);
		void azimuthChanged(double);
		void coneAngleChanged(double);
	private:
		Ui::Light3DDock ui;
		QVector<QObject*> children;
		Light *light;
		bool m_initializing;
};


#endif