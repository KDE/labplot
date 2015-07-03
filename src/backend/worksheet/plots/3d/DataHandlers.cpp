/***************************************************************************
    File                 : DataHandlers.cpp
    Project              : LabPlot
    Description          : 3D plot data handlers
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

#include "DataHandlers.h"

#include "backend/core/AbstractColumn.h"
#include "backend/matrix/Matrix.h"

#include <QDebug>

#include <vtkSphereSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkOBJReader.h>
#include <vtkSTLReader.h>
#include <vtkCellArray.h>
#include <vtkTriangle.h>

vtkSmartPointer<vtkActor> DemoDataHandler::actor() {
	vtkSmartPointer<vtkSphereSource> sphereSource = vtkSmartPointer<vtkSphereSource>::New();
	sphereSource->Update();
	vtkSmartPointer<vtkPolyDataMapper> sphereMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	sphereMapper->SetInputConnection(sphereSource->GetOutputPort());
	vtkSmartPointer<vtkActor> sphereActor = vtkSmartPointer<vtkActor>::New();
	sphereActor->GetProperty()->SetFrontfaceCulling(true);
	sphereActor->SetMapper(sphereMapper);

	return sphereActor;
}

////////////////////////////////////////////////////////////////////////////////

namespace {
	template<class TReader>
	vtkSmartPointer<vtkActor> createReader(const KUrl& path) {
		const QByteArray ascii = path.path().toAscii();
		vtkSmartPointer<TReader> reader = vtkSmartPointer<TReader>::New();
		reader->SetFileName(ascii.constData());
		reader->Update();

		//reader fails to read obj-files if the locale is not set to 'C'
		setlocale (LC_NUMERIC,"C");

		vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
		mapper->SetInputConnection(reader->GetOutputPort());

		vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
		actor->SetMapper(mapper);

		return actor;
	}
}

vtkSmartPointer<vtkActor> FileDataHandler::actor() {
	const QString& fileName = path.fileName();
	const QString& fileType = fileName.split('.').last().toLower();

	if (fileType == "obj") {
		return createReader<vtkOBJReader>(path);
	} else if (fileType == "stl") {
		return createReader<vtkSTLReader>(path);
	} else {
		return vtkSmartPointer<vtkActor>();
	}
}

void FileDataHandler::setFile(const KUrl& path) {
	this->path = path;
	emit parametersChanged();
}

////////////////////////////////////////////////////////////////////////////////

SpreadsheetDataHandler::SpreadsheetDataHandler()
	: xColumn(0)
	, yColumn(0)
	, zColumn(0) {

	for (int i = 0; i < 3; ++i) {
		nodeColumn[i] = 0;
	}
}

namespace {
	vtkSmartPointer<vtkActor> renderTriangles(vtkSmartPointer<vtkPoints>& points,
			vtkSmartPointer<vtkCellArray>& triangles) {

		qDebug() << Q_FUNC_INFO << "Amount of triangles:" << triangles->GetSize();

		vtkSmartPointer<vtkPolyData> polydata = vtkSmartPointer<vtkPolyData>::New();

		polydata->SetPoints(points);
		polydata->SetPolys(triangles);

		vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
		mapper->SetInputData(polydata);

		vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
		actor->SetMapper(mapper);

		return actor;
	}
}

vtkSmartPointer<vtkActor> SpreadsheetDataHandler::actor() {
	if (xColumn == 0 || yColumn == 0 || zColumn == 0) {
		return vtkSmartPointer<vtkActor>();
	}

	for (int i = 0; i < 3; ++i)
		if (nodeColumn[i] == 0) {
			qDebug() << Q_FUNC_INFO << "Node" << i << "== 0";
			return vtkSmartPointer<vtkActor>();
		}

	qDebug() << Q_FUNC_INFO << "Triangles rendering";
	vtkSmartPointer<vtkCellArray> triangles = vtkSmartPointer<vtkCellArray>::New();
	vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();

	const int numPoints = std::min(xColumn->rowCount(),
			std::min(yColumn->rowCount(), zColumn->rowCount()));

	for (int i = 0; i < numPoints; ++i) {
		const int x = static_cast<int>(xColumn->valueAt(i));
		const int y = static_cast<int>(yColumn->valueAt(i));
		const int z = static_cast<int>(zColumn->valueAt(i));

		points->InsertNextPoint(x, y, z);
	}

	const int numTrianges = std::min(nodeColumn[0]->rowCount(),
			std::min(nodeColumn[1]->rowCount(), nodeColumn[2]->rowCount()));

	for (int i = 0; i < numTrianges; ++i) {
		vtkSmartPointer<vtkTriangle> triangle = vtkSmartPointer<vtkTriangle>::New();
		const int id1 = static_cast<int>(nodeColumn[0]->valueAt(i));
		const int id2 = static_cast<int>(nodeColumn[1]->valueAt(i));
		const int id3 = static_cast<int>(nodeColumn[2]->valueAt(i));

		if (id1 < 1 || id2 < 1 || id3 < 1
				|| id1 > numPoints || id2 > numPoints || id3 > numPoints)
			// TODO: Return error
			continue;

		triangle->GetPointIds()->SetId(0, id1);
		triangle->GetPointIds()->SetId(1, id2);
		triangle->GetPointIds()->SetId(2, id3);

		triangles->InsertNextCell(triangle);
	}

	return renderTriangles(points, triangles);
}

void SpreadsheetDataHandler::setXColumn(AbstractColumn *column) {
	xColumn = column;
	emit parametersChanged();
}

void SpreadsheetDataHandler::setYColumn(AbstractColumn *column) {
	yColumn = column;
	emit parametersChanged();
}

void SpreadsheetDataHandler::setZColumn(AbstractColumn *column) {
	zColumn = column;
	emit parametersChanged();
}

void SpreadsheetDataHandler::setNodeColumn(int node, AbstractColumn *column) {
	if (node >= 0 && node < 3){
		nodeColumn[node] = column;
		emit parametersChanged();
	}
}

////////////////////////////////////////////////////////////////////////////////

MatrixDataHandler::MatrixDataHandler()
	: matrix(0) {
}

vtkSmartPointer<vtkActor> MatrixDataHandler::actor() {
	if (!matrix)
		return vtkSmartPointer<vtkActor>();

	qDebug() << Q_FUNC_INFO << "Triangles rendering";
	vtkSmartPointer<vtkCellArray> triangles = vtkSmartPointer<vtkCellArray>::New();
	vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();

	const double deltaX = (matrix->xEnd() - matrix->xStart()) / matrix->columnCount();
	const double deltaY = (matrix->yEnd() - matrix->yStart()) / matrix->rowCount();
	QVector<QVector<vtkIdType> > cellPoints(matrix->columnCount(),
			QVector<vtkIdType>(matrix->rowCount()));

	for (int x = 0; x < matrix->columnCount(); ++x) {
		for (int y = 0; y < matrix->rowCount(); ++y) {
			const double x_val = matrix->xStart() + deltaX * x;
			const double y_val = matrix->yStart() + deltaY * y;
			const double z_val = matrix->cell(x, y);
			cellPoints[x][y] = points->InsertNextPoint(x_val, y_val, z_val);
		}
	}

	for (int x = 0, max_x = cellPoints.size() - 1; x < max_x; ++x) {
		for (int y = 0, max_y = cellPoints[0].size() - 1; y < max_y; ++y) {
			const vtkIdType rectPoints[4] = {cellPoints[x][y], cellPoints[x +1][y],
				cellPoints[x + 1][y + 1], cellPoints[x][y + 1]};

			vtkSmartPointer<vtkTriangle> triangle1 = vtkSmartPointer<vtkTriangle>::New();
			triangle1->GetPointIds()->SetId(0, rectPoints[0]);
			triangle1->GetPointIds()->SetId(1, rectPoints[1]);
			triangle1->GetPointIds()->SetId(2, rectPoints[2]);

			vtkSmartPointer<vtkTriangle> triangle2 = vtkSmartPointer<vtkTriangle>::New();
			triangle2->GetPointIds()->SetId(0, rectPoints[2]);
			triangle2->GetPointIds()->SetId(1, rectPoints[3]);
			triangle2->GetPointIds()->SetId(2, rectPoints[0]);

			triangles->InsertNextCell(triangle1);
			triangles->InsertNextCell(triangle2);
		}
	}

	return renderTriangles(points, triangles);
}

void MatrixDataHandler::setMatrix(Matrix* matrix) {
	this->matrix = matrix;
	emit parametersChanged();
}