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
#include "backend/worksheet/plots/3d/Light.h"
#include "ui_light3ddock.h"

class Light3DDock : public QWidget {
		Q_OBJECT
	public:
		explicit Light3DDock(QWidget* parent);
		void setLight(Light *light);

	private slots:

	private:
		Ui::Light3DDock ui;
		Light *light;
};


#endif