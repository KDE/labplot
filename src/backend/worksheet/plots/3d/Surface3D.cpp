/***************************************************************************
    File                 : Surface3D.cpp
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

#include "Surface3D.h"
#include "Surface3DPrivate.h"
#include "Plot3D.h"
#include "XmlAttributeReader.h"

#include "backend/lib/commandtemplates.h"

#include "backend/core/AbstractColumn.h"
#include "backend/matrix/Matrix.h"

#include <QDebug>

#include <KLocale>

#include <cmath>

#include <vtkProperty.h>
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
#include <vtkDecimatePro.h>
#include <vtkNew.h>

Surface3D::Surface3D()
	: Base3D(new Surface3DPrivate(i18n("Surface"), this)) {
}

Surface3D::~Surface3D() {
}

// Spreadsheet slots
void Surface3D::xColumnAboutToBeRemoved(const AbstractAspect*) {
	Q_D(Surface3D);
	d->xColumn = 0;
}

void Surface3D::yColumnAboutToBeRemoved(const AbstractAspect*) {
	Q_D(Surface3D);
	d->yColumn = 0;
}

void Surface3D::zColumnAboutToBeRemoved(const AbstractAspect*) {
	Q_D(Surface3D);
	d->zColumn = 0;
}

void Surface3D::firstNodeAboutToBeRemoved(const AbstractAspect*) {
	Q_D(Surface3D);
	d->firstNode = 0;
}

void Surface3D::secondNodeAboutToBeRemoved(const AbstractAspect*) {
	Q_D(Surface3D);
	d->secondNode = 0;
}

void Surface3D::thirdNodeAboutToBeRemoved(const AbstractAspect*) {
	Q_D(Surface3D);
	d->thirdNode = 0;
}

// Matrix slots
void Surface3D::matrixAboutToBeRemoved(const AbstractAspect*) {
	Q_D(Surface3D);
	d->matrix = 0;
}

//##############################################################################
//##########################  getter methods  ##################################
//##############################################################################

// General parameters
BASIC_SHARED_D_READER_IMPL(Surface3D, Surface3D::VisualizationType, visualizationType, visualizationType)
BASIC_SHARED_D_READER_IMPL(Surface3D, Surface3D::DataSource, dataSource, sourceType)
BASIC_SHARED_D_READER_IMPL(Surface3D, Surface3D::ColorFilling, colorFilling, colorFilling)
BASIC_SHARED_D_READER_IMPL(Surface3D, QColor, color, color)
BASIC_SHARED_D_READER_IMPL(Surface3D, double, opacity, opacity)
BASIC_SHARED_D_READER_IMPL(Surface3D, bool, showXYProjection, showXYProjection)
BASIC_SHARED_D_READER_IMPL(Surface3D, bool, showXZProjection, showXZProjection)
BASIC_SHARED_D_READER_IMPL(Surface3D, bool, showYZProjection, showYZProjection)

// Matrix parameters
BASIC_SHARED_D_READER_IMPL(Surface3D, const Matrix*, matrix, matrix)
const QString& Surface3D::matrixPath() const { Q_D(const Surface3D); return d->matrixPath; }

// Spreadsheet parameters
BASIC_SHARED_D_READER_IMPL(Surface3D, const AbstractColumn*, xColumn, xColumn)
BASIC_SHARED_D_READER_IMPL(Surface3D, const AbstractColumn*, yColumn, yColumn)
BASIC_SHARED_D_READER_IMPL(Surface3D, const AbstractColumn*, zColumn, zColumn)

BASIC_SHARED_D_READER_IMPL(Surface3D, const AbstractColumn*, firstNode, firstNode)
BASIC_SHARED_D_READER_IMPL(Surface3D, const AbstractColumn*, secondNode, secondNode)
BASIC_SHARED_D_READER_IMPL(Surface3D, const AbstractColumn*, thirdNode, thirdNode)

const QString& Surface3D::xColumnPath() const { Q_D(const Surface3D); return d->xColumnPath; }
const QString& Surface3D::yColumnPath() const { Q_D(const Surface3D); return d->yColumnPath; }
const QString& Surface3D::zColumnPath() const { Q_D(const Surface3D); return d->zColumnPath; }

const QString& Surface3D::firstNodePath() const { Q_D(const Surface3D); return d->firstNodePath; }
const QString& Surface3D::secondNodePath() const { Q_D(const Surface3D); return d->secondNodePath; }
const QString& Surface3D::thirdNodePath() const { Q_D(const Surface3D); return d->thirdNodePath; }

// FileData parameters
CLASS_SHARED_D_READER_IMPL(Surface3D, KUrl, file, path)

//##############################################################################
//#################  setter methods and undo commands ##########################
//##############################################################################

// General parameters
STD_SETTER_CMD_IMPL_F_S(Surface3D, SetVisualizationType, Surface3D::VisualizationType, visualizationType, update)
STD_SETTER_IMPL(Surface3D, VisualizationType, Surface3D::VisualizationType, visualizationType, "%1: visualization type changed")

STD_SETTER_CMD_IMPL_F_S(Surface3D, SetDataSource, Surface3D::DataSource, sourceType, update)
STD_SETTER_IMPL(Surface3D, DataSource, Surface3D::DataSource, sourceType, "%1: data source type changed")

STD_SETTER_CMD_IMPL_F_S(Surface3D, SetColorFilling, Surface3D::ColorFilling, colorFilling, update)
STD_SETTER_IMPL(Surface3D, ColorFilling, Surface3D::ColorFilling, colorFilling, "%1: color filling type changed")

STD_SETTER_CMD_IMPL_F_S(Surface3D, SetColor, QColor, color, update)
STD_SETTER_IMPL(Surface3D, Color, const QColor&, color, "%1: color changed")

STD_SETTER_CMD_IMPL_F_S(Surface3D, SetOpacity, double, opacity, update)
STD_SETTER_IMPL(Surface3D, Opacity, double, opacity, "%1: opacity changed")

STD_SETTER_CMD_IMPL_F_S(Surface3D, SetShowXYProjection, bool, showXYProjection, update)
STD_SETTER_IMPL(Surface3D, ShowXYProjection, bool, showXYProjection, "%1: show XY projection changed")

STD_SETTER_CMD_IMPL_F_S(Surface3D, SetShowXZProjection, bool, showXZProjection, update)
STD_SETTER_IMPL(Surface3D, ShowXZProjection, bool, showXZProjection, "%1: show XZ projection changed")

STD_SETTER_CMD_IMPL_F_S(Surface3D, SetShowYZProjection, bool, showYZProjection, update)
STD_SETTER_IMPL(Surface3D, ShowYZProjection, bool, showYZProjection, "%1: show YZ projection changed")


// Matrix parameters
STD_SETTER_CMD_IMPL_F_S(Surface3D, SetMatrix, const Matrix*, matrix, update)
STD_SETTER_MATRIX_IMPL(Surface3D, Matrix, matrix, "%1: Matrix changed")


// Spreadsheet parameters
STD_SETTER_CMD_IMPL_F_S(Surface3D, SetXColumn, const AbstractColumn*, xColumn, update)
STD_SETTER_COLUMN_IMPL(Surface3D, XColumn, xColumn, "%1: X column changed")

STD_SETTER_CMD_IMPL_F_S(Surface3D, SetYColumn, const AbstractColumn*, yColumn, update)
STD_SETTER_COLUMN_IMPL(Surface3D, YColumn, yColumn, "%1: Y column changed")

STD_SETTER_CMD_IMPL_F_S(Surface3D, SetZColumn, const AbstractColumn*, zColumn, update)
STD_SETTER_COLUMN_IMPL(Surface3D, ZColumn, zColumn, "%1: Z column changed")

STD_SETTER_CMD_IMPL_F_S(Surface3D, SetFirstNode, const AbstractColumn*, firstNode, update)
STD_SETTER_COLUMN_IMPL(Surface3D, FirstNode, firstNode, "%1: First node changed")

STD_SETTER_CMD_IMPL_F_S(Surface3D, SetSecondNode, const AbstractColumn*, secondNode, update)
STD_SETTER_COLUMN_IMPL(Surface3D, SecondNode, secondNode, "%1: Second node changed")

STD_SETTER_CMD_IMPL_F_S(Surface3D, SetThirdNode, const AbstractColumn*, thirdNode, update)
STD_SETTER_COLUMN_IMPL(Surface3D, ThirdNode, thirdNode, "%1: Third node changed")

// FileData parameters
STD_SETTER_CMD_IMPL_F_S(Surface3D, SetFile, KUrl, path, update)
STD_SETTER_IMPL(Surface3D, File, const KUrl&, path, "%1: file path changed")


//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################
//! Save as XML
void Surface3D::save(QXmlStreamWriter* writer) const {
	Q_D(const Surface3D);

	writer->writeStartElement("surface3d");
		writer->writeAttribute("visualizationType", QString::number(d->visualizationType));
		writer->writeAttribute("sourceType", QString::number(d->sourceType));
		writer->writeAttribute("colorFilling", QString::number(d->colorFilling));
		writer->writeAttribute("color", d->color.name());
		writer->writeAttribute("opacity", QString::number(d->opacity));
		writer->writeAttribute("showXYProjection", QString::number(d->showXYProjection));
		writer->writeAttribute("showXZProjection", QString::number(d->showXZProjection));
		writer->writeAttribute("showYZProjection", QString::number(d->showYZProjection));
		writeBasicAttributes(writer);
		writeCommentElement(writer);
		d->saveSpreadsheetConfig(writer);
		d->saveMatrixConfig(writer);
		d->saveFileDataConfig(writer);
	writer->writeEndElement();
}


//! Load from XML
bool Surface3D::load(XmlStreamReader* reader) {
	Q_D(Surface3D);

	const QXmlStreamAttributes& attribs = reader->attributes();
	XmlAttributeReader attributeReader(reader, attribs);
	attributeReader.checkAndLoadAttribute<VisualizationType>("visualizationType", d->visualizationType);
	attributeReader.checkAndLoadAttribute<DataSource>("sourceType", d->sourceType);
	attributeReader.checkAndLoadAttribute<ColorFilling>("colorFilling", d->colorFilling);
	attributeReader.checkAndLoadAttribute("color", d->color);
	attributeReader.checkAndLoadAttribute("opacity", d->opacity);
	attributeReader.checkAndLoadAttribute("showXYProjection", d->showXYProjection);
	attributeReader.checkAndLoadAttribute("showXZProjection", d->showXZProjection);
	attributeReader.checkAndLoadAttribute("showYZProjection", d->showYZProjection);

	if(!readBasicAttributes(reader)){
		return false;
	}

	while(!reader->atEnd()){
		reader->readNext();
		const QStringRef& sectionName = reader->name();
		if (reader->isEndElement() && sectionName == "surface3d")
			break;

		if (reader->isEndElement())
			continue;

		if (sectionName == "comment") {
			if (!readCommentElement(reader))
				return false;
		} else if (sectionName == "matrix") {
			qDebug() << Q_FUNC_INFO << "Load matrix";
			if (!d->loadMatrixConfig(reader))
				return false;
		} else if (sectionName == "spreadsheet") {
			qDebug() << Q_FUNC_INFO << "Load spreadsheet";
			if (!d->loadSpreadsheetConfig(reader))
				return false;
		} else if (sectionName == "file") {
			qDebug() << Q_FUNC_INFO << "Load file";
			if (!d->loadFileDataConfig(reader))
				return false;
		}
	}

	return true;
}

//##############################################################################
//##############################  Private class  ###############################
//##############################################################################

Surface3DPrivate::Surface3DPrivate(const QString& name, Surface3D *parent)
	: Base3DPrivate(name, parent)
	, q(parent)
	// General properties
	, visualizationType(Surface3D::VisualizationType_Triangles)
	, sourceType(Surface3D::DataSource_Empty)
	, colorFilling(Surface3D::ColorFilling_Empty)
	, color(Qt::gray)
	, opacity(0.5)
	, showXYProjection(false)
	, showXZProjection(false)
	, showYZProjection(false)
	// Matrix properties
	, matrix(0)
	// Spreadsheet properties
	, xColumn(0)
	, yColumn(0)
	, zColumn(0)
	, firstNode(0)
	, secondNode(0)
	, thirdNode(0){
}

Surface3DPrivate::~Surface3DPrivate() {
}

vtkSmartPointer<vtkPolyData> Surface3DPrivate::createData() const {
	vtkSmartPointer<vtkPolyData> data = generateData();

	if (visualizationType == Surface3D::Surface3D::VisualizationType_Wireframe)
		data = extractEdges(data);

	if (colorFilling == Surface3D::ColorFilling_ElevationLevel)
		makeColorElevation(data);

	return data;
}

void Surface3DPrivate::modifyActor(vtkRenderer* renderer, vtkActor* actor) const {
	Q_UNUSED(renderer);
	if (colorFilling == Surface3D::ColorFilling_SolidColor) {
		vtkProperty* prop = actor->GetProperty();
		prop->SetColor(color.redF(), color.greenF(), color.blueF());
		prop->SetOpacity(opacity);
	} else {
		actor->SetProperty(vtkProperty::New());
	}
}

////////////////////////////////////////////////////////////////////////////////

vtkSmartPointer<vtkPolyData> Surface3DPrivate::extractEdges(vtkPolyData* data) const {
	vtkSmartPointer<vtkExtractEdges> edges = vtkSmartPointer<vtkExtractEdges>::New();
	edges->SetInputData(data);
	edges->Update();
	return vtkSmartPointer<vtkPolyData>(edges->GetOutput());
}

void Surface3DPrivate::makeColorElevation(vtkPolyData* polydata) const {
	double bounds[6];
	polydata->GetBounds(bounds);

	// Find min and max z
	const double minz = bounds[4];
	const double maxz = bounds[5];

	vtkNew<vtkLookupTable> colorLookupTable;
	colorLookupTable->SetTableRange(minz, maxz);
	colorLookupTable->Build();

	vtkSmartPointer<vtkUnsignedCharArray> colors = vtkSmartPointer<vtkUnsignedCharArray>::New();
	colors->SetNumberOfComponents(3);
	colors->SetName("Colors");

	for(int i = 0; i < polydata->GetNumberOfPoints(); ++i) {
		double p[3];
		polydata->GetPoint(i, p);

		double dcolor[3];
		colorLookupTable->GetColor(p[2], dcolor);
		unsigned char color[3];
		for(unsigned int j = 0; j < 3; ++j)
			color[j] = static_cast<unsigned char>(255.0 * dcolor[j]);

		colors->InsertNextTupleValue(color);
	}

	polydata->GetPointData()->SetScalars(colors);
}

////////////////////////////////////////////////////////////////////////////////

vtkSmartPointer<vtkPolyData> Surface3DPrivate::generateData() const {
	if (sourceType == Surface3D::DataSource_Empty)
		return generateDemoData();
	else if (sourceType == Surface3D::DataSource_File)
		return generateFileData();
	else if (sourceType == Surface3D::DataSource_Matrix)
		return generateMatrixData();
	else
		return generateSpreadsheetData();
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

	vtkSmartPointer<vtkPolyData> renderTriangles(vtkSmartPointer<vtkPoints>& points,
			vtkSmartPointer<vtkCellArray>& triangles) {

		qDebug() << Q_FUNC_INFO << "Amount of triangles:" << triangles->GetNumberOfCells();

		vtkSmartPointer<vtkPolyData> polydata = vtkSmartPointer<vtkPolyData>::New();

		polydata->SetPoints(points);
		polydata->SetPolys(triangles);

		return polydata;
	}
}

vtkSmartPointer<vtkPolyData> Surface3DPrivate::generateDemoData() const {
	vtkSmartPointer<vtkSphereSource> sphereSource = vtkSmartPointer<vtkSphereSource>::New();
	sphereSource->Update();
	return vtkSmartPointer<vtkPolyData>(sphereSource->GetOutput());
}

vtkSmartPointer<vtkPolyData> Surface3DPrivate::generateFileData() const {
	const QString& fileName = path.fileName();
	const QString& fileType = fileName.split('.').last().toLower();
	if (fileType == "obj") {
		return createReader<vtkOBJReader>(path);
	} else if (fileType == "stl") {
		return createReader<vtkSTLReader>(path);
	} else {
		return vtkSmartPointer<vtkPolyData>();
	}
}

namespace {
	vtkSmartPointer<vtkTriangle> createTriangle(vtkIdType p1, vtkIdType p2, vtkIdType p3) {
		vtkSmartPointer<vtkTriangle> triangle = vtkSmartPointer<vtkTriangle>::New();
		triangle->GetPointIds()->SetId(0, p1);
		triangle->GetPointIds()->SetId(1, p2);
		triangle->GetPointIds()->SetId(2, p3);
		return triangle;
	}
}

vtkSmartPointer<vtkPolyData> Surface3DPrivate::generateMatrixData() const {
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

			triangles->InsertNextCell(createTriangle(rectPoints[0], rectPoints[1], rectPoints[2]));
			triangles->InsertNextCell(createTriangle(rectPoints[2], rectPoints[3], rectPoints[0]));
		}
	}

	return renderTriangles(points, triangles);
}

vtkSmartPointer<vtkPolyData> Surface3DPrivate::generateSpreadsheetData() const {
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
		const int id1 = static_cast<int>(firstNode->valueAt(i));
		const int id2 = static_cast<int>(secondNode->valueAt(i));
		const int id3 = static_cast<int>(thirdNode->valueAt(i));

		triangles->InsertNextCell(createTriangle(id1, id2, id3));
	}

	return renderTriangles(points, triangles);
}

////////////////////////////////////////////////////////////////////////////////

void Surface3DPrivate::saveSpreadsheetConfig(QXmlStreamWriter* writer) const {
	writer->writeStartElement("spreadsheet");
		WRITE_COLUMN(xColumn, xColumn);
		WRITE_COLUMN(yColumn, yColumn);
		WRITE_COLUMN(zColumn, zColumn);

		WRITE_COLUMN(firstNode, firstNode);
		WRITE_COLUMN(secondNode, secondNode);
		WRITE_COLUMN(thirdNode, thirdNode);
	writer->writeEndElement();
}

void Surface3DPrivate::saveMatrixConfig(QXmlStreamWriter* writer) const {
	writer->writeStartElement("matrix");
		writer->writeAttribute("matrixPath", matrix ? matrix->path() : "");
	writer->writeEndElement();
}

void Surface3DPrivate::saveFileDataConfig(QXmlStreamWriter* writer) const {
	writer->writeStartElement("file");
	writer->writeAttribute("url", path.path());
	writer->writeEndElement();
}

bool Surface3DPrivate::loadSpreadsheetConfig(XmlStreamReader* reader) {
	const QXmlStreamAttributes& attribs = reader->attributes();
	QString str;
	Surface3DPrivate* d = this;
	READ_COLUMN(xColumn);
	READ_COLUMN(yColumn);
	READ_COLUMN(zColumn);

	READ_COLUMN(firstNode);
	READ_COLUMN(secondNode);
	READ_COLUMN(thirdNode);
	return true;
}

bool Surface3DPrivate::loadMatrixConfig(XmlStreamReader* reader) {
	const QXmlStreamAttributes& attribs = reader->attributes();
	matrixPath = attribs.value("matrixPath").toString();
	return true;
}

bool Surface3DPrivate::loadFileDataConfig(XmlStreamReader* reader) {
	const QXmlStreamAttributes& attribs = reader->attributes();
	XmlAttributeReader attributeReader(reader, attribs);
	attributeReader.checkAndLoadAttribute("url", path);
	return true;
}
