/***************************************************************************
    File                 : Base3DPrivate.h
    Project              : LabPlot
    Description          : Base classes for 3D objects
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

#ifndef PLOT3D_BASE3DPRIVATE_H
#define PLOT3D_BASE3DPRIVATE_H

#include <vtkSmartPointer.h>

class vtkActor;
class vtkProperty;
class vtkRenderer;
class vtkProp3D;

class Base3D;
struct Base3DPrivate {
	Base3D *baseParent;
	bool isHighlighted;
	bool isSelected;
	vtkSmartPointer<vtkRenderer> renderer;
	vtkSmartPointer<vtkProp3D> actor;
	vtkSmartPointer<vtkProperty> property;
	Base3DPrivate(vtkRenderer *renderer, Base3D *baseParent);
	virtual ~Base3DPrivate();

	virtual void init();
	void update();
	virtual void createActor() = 0;
	void hide();
	vtkProperty* getProperty() const;
};

#endif