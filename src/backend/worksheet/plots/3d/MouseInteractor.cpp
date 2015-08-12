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
#include "Axes.h"

#include <QDebug>

#include <vtkObjectFactory.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartPointer.h>
#include <vtkPicker.h>
#include <vtkBoundingBox.h>
#include <vtkNew.h>
#include <vtkProp.h>

vtkStandardNewMacro(MouseInteractor);

MouseInteractor::MouseInteractor()
	: prevHovered(0)
	, prevClicked(0) {

}

vtkProp* MouseInteractor::getPickedObject(int* pos) {
	if (pos == 0)
		pos = GetInteractor()->GetEventPosition();
	
	vtkNew<vtkPicker> picker;
	picker->SetTolerance(0.0);
	picker->Pick(pos[0], pos[1], 0, GetDefaultRenderer());
	return picker->GetViewProp();
}

void MouseInteractor::OnMouseMove() {
	int pos[2];
	GetInteractor()->GetEventPosition(pos);
	vtkProp* object = getPickedObject(pos);
	if (object == 0)
		vtkInteractorStyleTrackballCamera::OnMouseMove();

	if (object != prevHovered) {
		prevHovered = object;
		emit broadcaster.objectHovered(prevHovered);
	}
}

void MouseInteractor::OnLeftButtonDown() {
	vtkProp* object = getPickedObject();
	if (object == 0)
		vtkInteractorStyleTrackballCamera::OnLeftButtonDown();
	if (object != prevClicked) {
		prevClicked = object;
		emit broadcaster.objectClicked(prevClicked);
	}
}