/***************************************************************************
    File                 : MouseInteractor.cpp
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

#include "MouseInteractor.h"

#include <vtkObjectFactory.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartPointer.h>
#include <vtkPicker.h>

void MouseInteractorBroadcaster::setObject(vtkProp* object) {
	emit objectClicked(object);
}

vtkStandardNewMacro(MouseInteractor);

void MouseInteractor::OnLeftButtonDown() {
	int* clickPos = GetInteractor()->GetEventPosition();
	
	vtkSmartPointer<vtkPicker> picker = vtkSmartPointer<vtkPicker>::New();
	picker->SetTolerance(0.0);
	picker->Pick(clickPos[0], clickPos[1], 0, GetDefaultRenderer());
	broadcaster.setObject(picker->GetViewProp());

	vtkInteractorStyleTrackballCamera::OnLeftButtonDown();
}