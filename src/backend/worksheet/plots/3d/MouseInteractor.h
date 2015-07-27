/***************************************************************************
    File                 : MouseInteractor.h
    Project              : LabPlot
    Description          : 3D plot mouse interactor
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

#ifndef PLOT3D_MOUSEINTERACTOR_H
#define PLOT3D_MOUSEINTERACTOR_H

#include <QObject>
#include <vtkInteractorStyleTrackballCamera.h>

class vtkProp;
class MouseInteractorBroadcaster : public QObject {
		Q_OBJECT
	public:
		void setObject(vtkProp* object);
	signals:
		void objectClicked(vtkProp*);
};

class MouseInteractor : public vtkInteractorStyleTrackballCamera {
	public:
		static MouseInteractor* New();
		vtkTypeMacro(MouseInteractor, vtkInteractorStyleTrackballCamera);

		virtual void OnLeftButtonDown();

		MouseInteractorBroadcaster broadcaster;
};

#endif