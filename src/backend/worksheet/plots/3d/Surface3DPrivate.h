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

#ifndef PLOT3D_SURFACE3DPRIVATE_H
#define PLOT3D_SURFACE3DPRIVATE_H

#include "Base3DPrivate.h"
#include "Surface3D.h"

#include <QColor>

#include <KUrl>

class vtkPolyData;

struct Surface3DPrivate : public Base3DPrivate {
	Surface3D* const q;

	// General properties
	Surface3D::VisualizationType visualizationType;
	Surface3D::DataSource sourceType;
	Surface3D::ColorFilling colorFilling;
	QColor color;
	double opacity;
	bool showXYProjection;
	bool showXZProjection;
	bool showYZProjection;

	// Matrix properties
	const Matrix* matrix;
	QString matrixPath;

	// Spreadsheet properties
	const AbstractColumn *xColumn;
	const AbstractColumn *yColumn;
	const AbstractColumn *zColumn;

	const AbstractColumn *firstNode;
	const AbstractColumn *secondNode;
	const AbstractColumn *thirdNode;

	QString xColumnPath;
	QString yColumnPath;
	QString zColumnPath;
	QString firstNodePath;
	QString secondNodePath;
	QString thirdNodePath;

	// FileData properties
	KUrl path;

	Surface3DPrivate(const QString& name, Surface3D *parent);
	~Surface3DPrivate();
	vtkSmartPointer<vtkPolyData> createData();
	vtkSmartPointer<vtkActor> modifyActor(vtkActor*);

	// Surface3D
	vtkSmartPointer<vtkPolyData> extractEdges(vtkPolyData* data) const;
	void makeColorElevation(vtkPolyData* polydata) const;

	vtkSmartPointer<vtkPolyData> generateData() const;
	// PolyData generation
	vtkSmartPointer<vtkPolyData> generateDemoData() const;
	vtkSmartPointer<vtkPolyData> generateFileData() const;
	vtkSmartPointer<vtkPolyData> generateMatrixData() const;
	vtkSmartPointer<vtkPolyData> generateSpreadsheetData() const;

	// Export
	void saveSpreadsheetConfig(QXmlStreamWriter*) const;
	void saveMatrixConfig(QXmlStreamWriter*) const;
	void saveFileDataConfig(QXmlStreamWriter*) const;

	// Import
	bool loadSpreadsheetConfig(XmlStreamReader*);
	bool loadMatrixConfig(XmlStreamReader*);
	bool loadFileDataConfig(XmlStreamReader*);
};

#endif