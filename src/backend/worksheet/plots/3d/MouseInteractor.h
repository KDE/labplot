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

class Axes;
class vtkProp;
class MouseInteractorBroadcaster : public QObject {
		Q_OBJECT
		friend class MouseInteractor;
	signals:
		void objectHovered(vtkProp*);
		void objectClicked(vtkProp*);
		void axesHovered();
		void axesClicked();
};

class MouseInteractor : public vtkInteractorStyleTrackballCamera {
	public:
		MouseInteractor();
		static MouseInteractor* New();
		vtkTypeMacro(MouseInteractor, vtkInteractorStyleTrackballCamera);
		void setAxes(Axes* axes);

		virtual void OnMouseMove();
		virtual void OnLeftButtonDown();

		MouseInteractorBroadcaster broadcaster;

	private:
		vtkProp* getPickedObject(int* pos = 0);
		bool isAxesPicked(int* pos);

	private:
		void* prevHovered;
		void* prevClicked;
		Axes* axes;
};

#endif