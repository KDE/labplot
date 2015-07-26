/***************************************************************************
    File                 : Surface3DPrivate.h
    Project              : LabPlot
    Description          : 3D surface class
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

#ifndef SURFACE3DPRIVATE_H
#define SURFACE3DPRIVATE_H

#include "Surface3D.h"
#include "DataHandlers.h"

class vtkProp;
class vtkRenderer;
class vtkProperty;
class Plot3D;

struct Surface3DPrivate {
	Surface3D* const q;

	vtkSmartPointer<vtkRenderer> renderer;
	Surface3D::VisualizationType visualizationType;
	Surface3D::DataSource sourceType;
	Surface3D::ColorFilling colorFilling;

	bool isSelected;
	DemoDataHandler *demoHandler;
	SpreadsheetDataHandler *spreadsheetHandler;
	MatrixDataHandler *matrixHandler;
	FileDataHandler *fileHandler;
	vtkSmartPointer<vtkActor> surfaceActor;
	vtkSmartPointer<vtkProperty> surfaceProperty;

	Surface3DPrivate(vtkRenderer* renderer, Surface3D *parent);
	void init();
	~Surface3DPrivate();
	QString name() const;
	void update();

	void hide();
};

#endif