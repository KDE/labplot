/***************************************************************************
    File                 : Curve3DPrivate.h
    Project              : LabPlot
    Description          : 3D curve class
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

#ifndef CURVE3DPRIVATE_H
#define CURVE3DPRIVATE_H

#include <vtkSmartPointer.h>

class vtkRenderer;
class vtkActor;
class vtkProperty;

class Curve3D;
class AbstractColumn;
struct Curve3DPrivate {
	Curve3D* const q;

	bool isSelected;
	vtkSmartPointer<vtkRenderer> renderer;
	const AbstractColumn* xColumn;
	const AbstractColumn* yColumn;
	const AbstractColumn* zColumn;
	QString xColumnPath;
	QString yColumnPath;
	QString zColumnPath;
	float pointRadius;
	bool showVertices;
	bool isClosed;
	vtkSmartPointer<vtkActor> curveActor;
	vtkSmartPointer<vtkProperty> curveProperty;

	Curve3DPrivate(vtkRenderer* renderer, Curve3D* parent);
	void init();
	~Curve3DPrivate();
	QString name() const;
	void update();

	void hide();
};

#endif