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
#include "DataHandlersPrivate.h"
#include "Surface3D.h"
#include "XmlAttributeReader.h"

#include "backend/lib/commandtemplates.h"
#include "backend/lib/macros.h"

#include "backend/core/AbstractColumn.h"
#include "backend/matrix/Matrix.h"

#include <QDebug>

#include <KLocale>

#include <vtkSphereSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkOBJReader.h>
#include <vtkSTLReader.h>
#include <vtkCellArray.h>
#include <vtkTriangle.h>
#include <vtkLookupTable.h>
#include <vtkPointData.h>
#include <vtkExtractEdges.h>
#include <vtkAlgorithm.h>

IDataHandler::IDataHandler(): AbstractAspect(i18n("Data handler")) {
}

void IDataHandler::update() {
	emit parametersChanged();
}

void IDataHandler::makeColorElevation(vtkPolyData* polydata) {
		double bounds[6];
		polydata->GetBounds(bounds);

		// Find min and max z
		double minz = bounds[4];
		double maxz = bounds[5];

		vtkSmartPointer<vtkLookupTable> colorLookupTable = vtkSmartPointer<vtkLookupTable>::New();
		colorLookupTable->SetTableRange(minz, maxz);
		colorLookupTable->Build();

		vtkSmartPointer<vtkUnsignedCharArray> colors = vtkSmartPointer<vtkUnsignedCharArray>::New();
		colors->SetNumberOfComponents(3);
		colors->SetName("Colors");

		for(int i = 0; i < polydata->GetNumberOfPoints(); ++i) {
			double p[3];
			polydata->GetPoint(i,p);

			double dcolor[3];
			colorLookupTable->GetColor(p[2], dcolor);
			unsigned char color[3];
			for(unsigned int j = 0; j < 3; j++)
				color[j] = static_cast<unsigned char>(255.0 * dcolor[j]);

			colors->InsertNextTupleValue(color);
		}

		polydata->GetPointData()->SetScalars(colors);
}

vtkSmartPointer<vtkActor> IDataHandler::actor(Surface3D::VisualizationType type, bool coloredElevation) {
	vtkSmartPointer<vtkPolyData> data = generateData();
	if (type == Surface3D::Surface3D::VisualizationType_Wireframe)
		data = extractEdges(data);
	else if (coloredElevation)
		makeColorElevation(data);

	return mapData(data);
}

vtkSmartPointer<vtkPolyData> IDataHandler::extractEdges(vtkPolyData* data) const {
	vtkSmartPointer<vtkExtractEdges> edges = vtkSmartPointer<vtkExtractEdges>::New();
	edges->SetInputData(data);
	edges->Update();
	return vtkSmartPointer<vtkPolyData>(edges->GetOutput());
}

vtkSmartPointer<vtkActor> IDataHandler::mapData(vtkPolyData* data) {
	//reader fails to read obj-files if the locale is not set to 'C'
	setlocale(LC_NUMERIC, "C");

	vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->SetInputData(data);
	vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
	actor->SetMapper(mapper);
	return actor;
}

////////////////////////////////////////////////////////////////////////////////

DemoDataHandler::DemoDataHandler(){
}

vtkSmartPointer<vtkPolyData> DemoDataHandler::generateData() {
	vtkSmartPointer<vtkSphereSource> sphereSource = vtkSmartPointer<vtkSphereSource>::New();
	sphereSource->Update();
	return vtkSmartPointer<vtkPolyData>(sphereSource->GetOutput());
}

////////////////////////////////////////////////////////////////////////////////

FileDataHandler::FileDataHandler()
	: IDataHandler()
	, d_ptr(new FileDataHandlerPrivate(this)) {
}

FileDataHandler::~FileDataHandler() {
}

void FileDataHandler::save(QXmlStreamWriter* writer) const {
	Q_D(const FileDataHandler);

	writer->writeStartElement("file");
		writer->writeAttribute("url", d->path.path());
	writer->writeEndElement();
}

bool FileDataHandler::load(XmlStreamReader* reader) {
	Q_D(FileDataHandler);
	const QXmlStreamAttributes& attribs = reader->attributes();
	XmlAttributeReader attributeReader(reader, attribs);
	attributeReader.checkAndLoadAttribute("url", d->path);
	return true;
}

namespace {
	template<class TReader>
	vtkSmartPointer<vtkPolyData> createReader(const KUrl& path) {
		const QByteArray ascii = path.path().toAscii();
		vtkSmartPointer<TReader> reader = vtkSmartPointer<TReader>::New();
		reader->SetFileName(ascii.constData());
		reader->Update();
		return vtkSmartPointer<vtkPolyData>(reader->GetOutput());
	}
}

vtkSmartPointer<vtkPolyData> FileDataHandler::generateData() {
	Q_D(FileDataHandler);
	const QString& fileName = d->path.fileName();
	const QString& fileType = fileName.split('.').last().toLower();
	if (fileType == "obj") {
		return createReader<vtkOBJReader>(d->path);
	} else if (fileType == "stl") {
		return createReader<vtkSTLReader>(d->path);
	} else {
		return vtkSmartPointer<vtkPolyData>();
	}
}

CLASS_SHARED_D_READER_IMPL(FileDataHandler, KUrl, file, path)

STD_SETTER_CMD_IMPL_F_S(FileDataHandler, SetFile, KUrl, path, update)
STD_SETTER_IMPL(FileDataHandler, File, const KUrl&, path, "%1: file path changed")

////////////////////////////////////////////////////////////////////////////////

SpreadsheetDataHandler::SpreadsheetDataHandler()
	: IDataHandler()
	, d_ptr(new SpreadsheetDataHandlerPrivate(this)) {
}

SpreadsheetDataHandler::~SpreadsheetDataHandler() {
}

BASIC_SHARED_D_READER_IMPL(SpreadsheetDataHandler, const AbstractColumn*, xColumn, xColumn)
BASIC_SHARED_D_READER_IMPL(SpreadsheetDataHandler, const AbstractColumn*, yColumn, yColumn)
BASIC_SHARED_D_READER_IMPL(SpreadsheetDataHandler, const AbstractColumn*, zColumn, zColumn)

BASIC_SHARED_D_READER_IMPL(SpreadsheetDataHandler, const AbstractColumn*, firstNode, firstNode)
BASIC_SHARED_D_READER_IMPL(SpreadsheetDataHandler, const AbstractColumn*, secondNode, secondNode)
BASIC_SHARED_D_READER_IMPL(SpreadsheetDataHandler, const AbstractColumn*, thirdNode, thirdNode)

const QString& SpreadsheetDataHandler::xColumnPath() const { Q_D(const SpreadsheetDataHandler); return d->xColumnPath; }
const QString& SpreadsheetDataHandler::yColumnPath() const { Q_D(const SpreadsheetDataHandler); return d->yColumnPath; }
const QString& SpreadsheetDataHandler::zColumnPath() const { Q_D(const SpreadsheetDataHandler); return d->zColumnPath; }

const QString& SpreadsheetDataHandler::firstNodePath() const { Q_D(const SpreadsheetDataHandler); return d->firstNodePath; }
const QString& SpreadsheetDataHandler::secondNodePath() const { Q_D(const SpreadsheetDataHandler); return d->secondNodePath; }
const QString& SpreadsheetDataHandler::thirdNodePath() const { Q_D(const SpreadsheetDataHandler); return d->thirdNodePath; }

void SpreadsheetDataHandler::save(QXmlStreamWriter* writer) const {
	Q_D(const SpreadsheetDataHandler);

	writer->writeStartElement("spreadsheet");
		WRITE_COLUMN(d->xColumn, xColumn);
		WRITE_COLUMN(d->yColumn, yColumn);
		WRITE_COLUMN(d->zColumn, zColumn);

		WRITE_COLUMN(d->firstNode, firstNode);
		WRITE_COLUMN(d->secondNode, secondNode);
		WRITE_COLUMN(d->thirdNode, thirdNode);
	writer->writeEndElement();
}

bool SpreadsheetDataHandler::load(XmlStreamReader* reader) {
	Q_D(SpreadsheetDataHandler);
	const QXmlStreamAttributes& attribs = reader->attributes();
	QString str;
	READ_COLUMN(xColumn);
	READ_COLUMN(yColumn);
	READ_COLUMN(zColumn);

	READ_COLUMN(firstNode);
	READ_COLUMN(secondNode);
	READ_COLUMN(thirdNode);
	return true;
}

namespace {
	vtkSmartPointer<vtkPolyData> renderTriangles(vtkSmartPointer<vtkPoints>& points,
			vtkSmartPointer<vtkCellArray>& triangles) {

		qDebug() << Q_FUNC_INFO << "Amount of triangles:" << triangles->GetNumberOfCells();

		vtkSmartPointer<vtkPolyData> polydata = vtkSmartPointer<vtkPolyData>::New();

		polydata->SetPoints(points);
		polydata->SetPolys(triangles);

		return polydata;
	}
}

vtkSmartPointer<vtkPolyData> SpreadsheetDataHandler::generateData() {
	Q_D(SpreadsheetDataHandler);
	const AbstractColumn* const xColumn = d->xColumn;
	const AbstractColumn* const yColumn = d->yColumn;
	const AbstractColumn* const zColumn = d->zColumn;
	const AbstractColumn* const firstNode = d->firstNode;
	const AbstractColumn* const secondNode = d->secondNode;
	const AbstractColumn* const thirdNode = d->thirdNode;

	if (xColumn == 0 || yColumn == 0 || zColumn == 0
			|| firstNode == 0 || secondNode == 0 || thirdNode == 0) {
		return vtkSmartPointer<vtkPolyData>();
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

	const int numTrianges = std::min(firstNode->rowCount(),
			std::min(secondNode->rowCount(), thirdNode->rowCount()));

	for (int i = 0; i < numTrianges; ++i) {
		vtkSmartPointer<vtkTriangle> triangle = vtkSmartPointer<vtkTriangle>::New();
		const int id1 = static_cast<int>(firstNode->valueAt(i));
		const int id2 = static_cast<int>(secondNode->valueAt(i));
		const int id3 = static_cast<int>(thirdNode->valueAt(i));

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

STD_SETTER_CMD_IMPL_F_S(SpreadsheetDataHandler, SetXColumn, const AbstractColumn*, xColumn, update)
STD_SETTER_COLUMN_IMPL(SpreadsheetDataHandler, XColumn, xColumn, "%1: X column changed")

STD_SETTER_CMD_IMPL_F_S(SpreadsheetDataHandler, SetYColumn, const AbstractColumn*, yColumn, update)
STD_SETTER_COLUMN_IMPL(SpreadsheetDataHandler, YColumn, yColumn, "%1: Y column changed")

STD_SETTER_CMD_IMPL_F_S(SpreadsheetDataHandler, SetZColumn, const AbstractColumn*, zColumn, update)
STD_SETTER_COLUMN_IMPL(SpreadsheetDataHandler, ZColumn, zColumn, "%1: Z column changed")

STD_SETTER_CMD_IMPL_F_S(SpreadsheetDataHandler, SetFirstNode, const AbstractColumn*, firstNode, update)
STD_SETTER_COLUMN_IMPL(SpreadsheetDataHandler, FirstNode, firstNode, "%1: First node changed")

STD_SETTER_CMD_IMPL_F_S(SpreadsheetDataHandler, SetSecondNode, const AbstractColumn*, secondNode, update)
STD_SETTER_COLUMN_IMPL(SpreadsheetDataHandler, SecondNode, secondNode, "%1: Second node changed")

STD_SETTER_CMD_IMPL_F_S(SpreadsheetDataHandler, SetThirdNode, const AbstractColumn*, thirdNode, update)
STD_SETTER_COLUMN_IMPL(SpreadsheetDataHandler, ThirdNode, thirdNode, "%1: Third node changed")

void SpreadsheetDataHandler::xColumnAboutToBeRemoved(const AbstractAspect*) {
	Q_D(SpreadsheetDataHandler);
	d->xColumn = 0;
}

void SpreadsheetDataHandler::yColumnAboutToBeRemoved(const AbstractAspect*) {
	Q_D(SpreadsheetDataHandler);
	d->yColumn = 0;
}

void SpreadsheetDataHandler::zColumnAboutToBeRemoved(const AbstractAspect*) {
	Q_D(SpreadsheetDataHandler);
	d->zColumn = 0;
}

void SpreadsheetDataHandler::firstNodeAboutToBeRemoved(const AbstractAspect*) {
	Q_D(SpreadsheetDataHandler);
	d->firstNode = 0;
}

void SpreadsheetDataHandler::secondNodeAboutToBeRemoved(const AbstractAspect*) {
	Q_D(SpreadsheetDataHandler);
	d->secondNode = 0;
}

void SpreadsheetDataHandler::thirdNodeAboutToBeRemoved(const AbstractAspect*) {
	Q_D(SpreadsheetDataHandler);
	d->thirdNode = 0;
}

////////////////////////////////////////////////////////////////////////////////

MatrixDataHandler::MatrixDataHandler()
	: IDataHandler()
	, d_ptr(new MatrixDataHandlerPrivate(this)) {
}

MatrixDataHandler::~MatrixDataHandler() {
}

void MatrixDataHandler::save(QXmlStreamWriter* writer) const {
	Q_D(const MatrixDataHandler);

	writer->writeStartElement("matrix");
		writer->writeAttribute("matrixPath", d->matrix ? d->matrix->path() : "");
	writer->writeEndElement();
}

bool MatrixDataHandler::load(XmlStreamReader* reader) {
	Q_D(MatrixDataHandler);
	const QXmlStreamAttributes& attribs = reader->attributes();
	d->matrixPath = attribs.value("matrixPath").toString();
	return true;
}

vtkSmartPointer<vtkPolyData> MatrixDataHandler::generateData() {
	Q_D(MatrixDataHandler);
	const Matrix * const matrix = d->matrix;
	if (!matrix)
		return vtkSmartPointer<vtkPolyData>();

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

BASIC_SHARED_D_READER_IMPL(MatrixDataHandler, const Matrix*, matrix, matrix)
const QString& MatrixDataHandler::matrixPath() const { Q_D(const MatrixDataHandler); return d->matrixPath; }

STD_SETTER_CMD_IMPL_F_S(MatrixDataHandler, SetMatrix, const Matrix*, matrix, update)
STD_SETTER_MATRIX_IMPL(MatrixDataHandler, Matrix, matrix, "%1: Matrix changed")

void MatrixDataHandler::matrixAboutToBeRemoved(const AbstractAspect*) {
	Q_D(MatrixDataHandler);
	d->matrix = 0;
}

////////////////////////////////////////////////////////////////////////////////

template<typename TParent>
BaseDataHandlerPrivate<TParent>::BaseDataHandlerPrivate(TParent* parent)
	: q(parent) {
}

template<typename TParent>
void BaseDataHandlerPrivate<TParent>::update() {
	q->update();
}

template<typename TParent>
QString BaseDataHandlerPrivate<TParent>::name() const {
	return "";
}

////////////////////////////////////////////////////////////////////////////////

MatrixDataHandlerPrivate::MatrixDataHandlerPrivate(MatrixDataHandler* parent)
	: BaseDataHandlerPrivate<MatrixDataHandler>(parent)
	, matrix(0) {
}

////////////////////////////////////////////////////////////////////////////////

SpreadsheetDataHandlerPrivate::SpreadsheetDataHandlerPrivate(SpreadsheetDataHandler* parent)
	: BaseDataHandlerPrivate<SpreadsheetDataHandler>(parent)
	, xColumn(0)
	, yColumn(0)
	, zColumn(0)
	, firstNode(0)
	, secondNode(0)
	, thirdNode(0) {
}

////////////////////////////////////////////////////////////////////////////////

FileDataHandlerPrivate::FileDataHandlerPrivate(FileDataHandler* parent)
	: BaseDataHandlerPrivate<FileDataHandler>(parent) {
}