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

#ifndef PLOT3D_CURVE3DPRIVATE_H
#define PLOT3D_CURVE3DPRIVATE_H

#include "Base3DPrivate.h"
class Curve3D;
class AbstractColumn;
struct Curve3DPrivate : public Base3DPrivate {
	Curve3D* const q;

	const AbstractColumn* xColumn;
	const AbstractColumn* yColumn;
	const AbstractColumn* zColumn;
	QString xColumnPath;
	QString yColumnPath;
	QString zColumnPath;
	float pointRadius;
	bool showEdges;
	bool isClosed;

	Curve3DPrivate(const QString& name, Curve3D* parent);
	~Curve3DPrivate();
	vtkSmartPointer<vtkPolyData> createData();
	vtkSmartPointer<vtkActor> modifyActor(vtkActor*);
};

#endif