/*
	File                 : Surface3DPlot.cpp
	Project              : LabPlot
	Description          : Surface3DPlot
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Kuntal Bar <barkuntal6@gmail.com>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "Surface3DPlot.h"
#include "Axis3D.h"
#include "MouseInteractor.h"
#include "Surface3DPlotPrivate.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/macros.h"
#include <Q3DSurface>
#include <QAbstract3DSeries>
#include <QXmlStreamAttributes>

Surface3DPlot::Surface3DPlot(const QString& name)
	: Base3DArea(name, new Surface3DPlotPrivate(this), Base3DArea::Surface, AspectType::Surface3DPlot) {
	m_surface = new Q3DSurface();
	Axis3D* xAxis = new Axis3D(QStringLiteral("x-axis"), Axis3D::X);
	Axis3D* yAxis = new Axis3D(QStringLiteral("y-axis"), Axis3D::Y);
	Axis3D* zAxis = new Axis3D(QStringLiteral("z-axis"), Axis3D::Z);
	addChild(xAxis);
	addChild(yAxis);
	addChild(zAxis);
	m_surface->setAxisX(xAxis->m_axis);
	m_surface->setAxisY(yAxis->m_axis);
	m_surface->setAxisZ(zAxis->m_axis);
	m_surface->setActiveInputHandler(new MouseInteractor());
}

Surface3DPlot::~Surface3DPlot() {
}
// Spreadsheet slots
void Surface3DPlot::xColumnAboutToBeRemoved(const AbstractAspect*) {
	Q_D(Surface3DPlot);
	d->xColumn = nullptr;
}

void Surface3DPlot::yColumnAboutToBeRemoved(const AbstractAspect*) {
	Q_D(Surface3DPlot);
	d->yColumn = nullptr;
}

void Surface3DPlot::zColumnAboutToBeRemoved(const AbstractAspect*) {
	Q_D(Surface3DPlot);
	d->zColumn = nullptr;
}

// Matrix slots
void Surface3DPlot::matrixAboutToBeRemoved(const AbstractAspect*) {
	Q_D(Surface3DPlot);
	d->matrix = nullptr;
}

// ##############################################################################
// ##########################  getter methods  ##################################
// ##############################################################################

// General parameters
BASIC_SHARED_D_READER_IMPL(Surface3DPlot, Surface3DPlot::DataSource, dataSource, sourceType)
BASIC_SHARED_D_READER_IMPL(Surface3DPlot, Surface3DPlot::DrawMode, drawMode, drawMode)

BASIC_SHARED_D_READER_IMPL(Surface3DPlot, QColor, color, color)

BASIC_SHARED_D_READER_IMPL(Surface3DPlot, bool, flatShading, flatShading)
BASIC_SHARED_D_READER_IMPL(Surface3DPlot, bool, smooth, smooth)

// Matrix parameters
BASIC_SHARED_D_READER_IMPL(Surface3DPlot, const Matrix*, matrix, matrix)
const QString& Surface3DPlot::matrixPath() const {
	Q_D(const Surface3DPlot);
	return d->matrixPath;
}

// Spreadsheet parameters
BASIC_SHARED_D_READER_IMPL(Surface3DPlot, const AbstractColumn*, xColumn, xColumn)
BASIC_SHARED_D_READER_IMPL(Surface3DPlot, const AbstractColumn*, yColumn, yColumn)
BASIC_SHARED_D_READER_IMPL(Surface3DPlot, const AbstractColumn*, zColumn, zColumn)

const QString& Surface3DPlot::xColumnPath() const {
	Q_D(const Surface3DPlot);
	return d->xColumnPath;
}
const QString& Surface3DPlot::yColumnPath() const {
	Q_D(const Surface3DPlot);
	return d->yColumnPath;
}
const QString& Surface3DPlot::zColumnPath() const {
	Q_D(const Surface3DPlot);
	return d->zColumnPath;
}

void Surface3DPlot::show(bool visible) {
	if (m_surface)
		m_surface->setVisible(visible);
}

void Surface3DPlot::recalc() {
	Q_D(Surface3DPlot);
	d->recalc();
}

// ##############################################################################
// #################  setter methods and undo commands ##########################
// ##############################################################################

// General
STD_SETTER_CMD_IMPL_F_S(Surface3DPlot, SetDataSource, Surface3DPlot::DataSource, sourceType, recalc)
void Surface3DPlot::setDataSource(Surface3DPlot::DataSource sourceType) {
	Q_D(Surface3DPlot);
	if (sourceType != d->sourceType)
		exec(new Surface3DPlotSetDataSourceCmd(d, sourceType, ki18n("%1: data source changed")));
}
STD_SETTER_CMD_IMPL_F_S(Surface3DPlot, SetDrawMode, Surface3DPlot::DrawMode, drawMode, updateDrawMode)
void Surface3DPlot::setDrawMode(Surface3DPlot::DrawMode drawMode) {
	Q_D(Surface3DPlot);
	if (drawMode != d->drawMode)
		exec(new Surface3DPlotSetDrawModeCmd(d, drawMode, ki18n("%1: draw mode changed")));
}

STD_SETTER_CMD_IMPL_F_S(Surface3DPlot, SetColor, QColor, color, updateColor)
void Surface3DPlot::setColor(QColor color) {
	Q_D(Surface3DPlot);
	if (color != d->color)
		exec(new Surface3DPlotSetColorCmd(d, color, ki18n("%1: color changed")));
}

STD_SETTER_CMD_IMPL_F_S(Surface3DPlot, SetFlatShading, bool, flatShading, updateFlatShading)
void Surface3DPlot::setFlatShading(bool flatShading) {
	Q_D(Surface3DPlot);
	if (flatShading != d->flatShading)
		exec(new Surface3DPlotSetFlatShadingCmd(d, flatShading, ki18n("%1: flat shading changed")));
}

STD_SETTER_CMD_IMPL_F_S(Surface3DPlot, SetSmooth, bool, smooth, updateSmoothMesh)
void Surface3DPlot::setSmooth(bool smooth) {
	Q_D(Surface3DPlot);
	if (smooth != d->smooth)
		exec(new Surface3DPlotSetSmoothCmd(d, smooth, ki18n("%1: smooth changed")));
}
STD_SETTER_CMD_IMPL_F_S(Surface3DPlot, SetMatrix, const Matrix*, matrix, recalc)
void Surface3DPlot::setMatrix(const Matrix* matrix) {
	Q_D(Surface3DPlot);
	if (matrix != d->matrix)
		exec(new Surface3DPlotSetMatrixCmd(d, matrix, ki18n("%1: matrix changed")));
	if (matrix) {
		connect(matrix, &Matrix::dataChanged, this, &Surface3DPlot::recalc);
		if (matrix->parentAspect())
			connect(matrix->parentAspect(), &AbstractAspect::childAspectAboutToBeRemoved, this, &Surface3DPlot::matrixAboutToBeRemoved);
	}
}
STD_SETTER_CMD_IMPL_F_S(Surface3DPlot, SetXColumn, const AbstractColumn*, xColumn, recalc)
void Surface3DPlot::setXColumn(const AbstractColumn* xCol) {
	Q_D(Surface3DPlot);
	if (xCol != d->xColumn)
		exec(new Surface3DPlotSetXColumnCmd(d, xCol, ki18n("%1: X Column changed")));
	if (xCol) {
		connect(xCol, &AbstractColumn::dataChanged, this, &Surface3DPlot::recalc);
		if (xCol->parentAspect())
			connect(xCol->parentAspect(), &AbstractAspect::childAspectAboutToBeRemoved, this, &Surface3DPlot::xColumnAboutToBeRemoved);
	}
}
STD_SETTER_CMD_IMPL_F_S(Surface3DPlot, SetYColumn, const AbstractColumn*, yColumn, recalc)
void Surface3DPlot::setYColumn(const AbstractColumn* yCol) {
	Q_D(Surface3DPlot);
	if (yCol != d->yColumn)
		exec(new Surface3DPlotSetYColumnCmd(d, yCol, ki18n("%1: Y Column changed")));
	if (yCol) {
		connect(yCol, &AbstractColumn::dataChanged, this, &Surface3DPlot::recalc);
		if (yCol->parentAspect())
			connect(yCol->parentAspect(), &AbstractAspect::childAspectAboutToBeRemoved, this, &Surface3DPlot::yColumnAboutToBeRemoved);
	}
}
STD_SETTER_CMD_IMPL_F_S(Surface3DPlot, SetZColumn, const AbstractColumn*, zColumn, recalc)
void Surface3DPlot::setZColumn(const AbstractColumn* zCol) {
	Q_D(Surface3DPlot);
	if (zCol != d->zColumn)
		exec(new Surface3DPlotSetZColumnCmd(d, zCol, ki18n("%1: Z Column changed")));
	if (zCol) {
		connect(zCol, &AbstractColumn::dataChanged, this, &Surface3DPlot::recalc);
		if (zCol->parentAspect())
			connect(zCol->parentAspect(), &AbstractAspect::childAspectAboutToBeRemoved, this, &Surface3DPlot::zColumnAboutToBeRemoved);
	}
}

// #####################################################################
// ################### Private implementation ##########################
// #####################################################################

Surface3DPlotPrivate::Surface3DPlotPrivate(Surface3DPlot* owner)
	: Base3DAreaPrivate(owner)
	, q(owner)
	, sourceType(Surface3DPlot::DataSource_Spreadsheet)
	, drawMode(Surface3DPlot::DrawWireframeSurface)
	, color(Qt::green) {
}

void Surface3DPlotPrivate::recalc() {
	if (sourceType == Surface3DPlot::DataSource_Empty)
		generateDemoData();
	else if (sourceType == Surface3DPlot::DataSource_Spreadsheet)
		generateSpreadsheetData();
	else if (sourceType == Surface3DPlot::DataSource_Matrix)
		generateMatrixData();

	Q_EMIT q->changed();
}

void Surface3DPlotPrivate::updateDrawMode() {
	qDebug() << Q_FUNC_INFO;
	QSurface3DSeries* series = q->m_surface->seriesList().first();
	series->setDrawMode(static_cast<QSurface3DSeries::DrawFlags>(drawMode));
	q->m_surface->update(); // or q->m_surface->repaint();
	Q_EMIT q->changed();
}

void Surface3DPlotPrivate::updateColor() {
	auto* series = q->m_surface->seriesList().first();
	if (!series)
		return;
	series->setBaseColor(color);
	Q_EMIT q->changed();
}

void Surface3DPlotPrivate::updateFlatShading() {
	qDebug() << Q_FUNC_INFO;
	QSurface3DSeries* series = q->m_surface->seriesList().first();
	series->setFlatShadingEnabled(flatShading);
	Q_EMIT q->changed();
}

void Surface3DPlotPrivate::updateSmoothMesh() {
	qDebug() << Q_FUNC_INFO;
	QSurface3DSeries* series = q->m_surface->seriesList().first();
	series->setMeshSmooth(smooth);
	Q_EMIT q->changed();
}

void Surface3DPlotPrivate::generateDemoData() const {
	if (!q->m_surface->seriesList().empty())
		q->m_surface->removeSeries(q->m_surface->seriesList().first());
	QSurfaceDataArray* dataArray = new QSurfaceDataArray;
	dataArray->reserve(50);
	// Parameters for sphere
	int n = 50; // Number of points
	float radius = 10.0f; // Radius of the sphere

	for (int i = 0; i < n; ++i) {
		QSurfaceDataRow* newRow = new QSurfaceDataRow(n);
		float theta = i * M_PI / n; // Angle theta
		for (int j = 0; j < n; ++j) {
			float phi = j * 2 * M_PI / n; // Angle phi
			float x = radius * sin(theta) * cos(phi);
			float y = radius * sin(theta) * sin(phi);
			float z = radius * cos(theta);
			(*newRow)[j].setPosition(QVector3D(x, y, z));
		}
		*dataArray << *newRow;
	}

	QSurfaceDataProxy* proxy = new QSurfaceDataProxy();
	proxy->resetArray(*dataArray);

	QSurface3DSeries* series = new QSurface3DSeries(proxy);
	q->m_surface->addSeries(series);
	q->m_surface->axisX()->setRange(-radius, radius);
	q->m_surface->axisY()->setRange(-radius, radius);
	q->m_surface->axisZ()->setRange(-radius, radius);
	Q_EMIT q->changed();
}

void Surface3DPlotPrivate::generateMatrixData() const {
	if (!matrix) {
		qDebug() << Q_FUNC_INFO << q->name() << "Matrix has not been set";
		return;
	}
	if (!matrix->rowCount())
		return;
	if (!q->m_surface->seriesList().empty())
		q->m_surface->removeSeries(q->m_surface->seriesList().first());

	qDebug() << Q_FUNC_INFO << q->name() << "Matrix has been set!";
	QSurfaceDataArray* dataArray = new QSurfaceDataArray;
	dataArray->reserve(matrix->rowCount());

	const double deltaX = (matrix->xEnd() - matrix->xStart()) / matrix->rowCount();
	const double deltaY = (matrix->yEnd() - matrix->yStart()) / matrix->columnCount();

	for (int x = 0; x < matrix->rowCount(); ++x) {
		QSurfaceDataRow* newRow = new QSurfaceDataRow(matrix->columnCount());

		for (int y = 0; y < matrix->columnCount(); ++y) {
			const double x_val = matrix->xStart() + deltaX * x;
			const double y_val = matrix->yStart() + deltaY * y;
			const double z_val = matrix->cell<double>(x, y);
			(*newRow)[y].setPosition(QVector3D(x_val, z_val, y_val));
		}

		*dataArray << *newRow;
	}

	QSurface3DSeries* series = new QSurface3DSeries;
	series->dataProxy()->resetArray(*dataArray);

	// Add the series to the Q3DSurface
	q->m_surface->addSeries(series);
	Q_EMIT q->changed();
}

void Surface3DPlotPrivate::generateSpreadsheetData() const {
	qDebug() << Q_FUNC_INFO;
	if (!q->m_surface->seriesList().empty())
		q->m_surface->removeSeries(q->m_surface->seriesList().first());

	if (!xColumn || !yColumn || !zColumn)
		return;

	int xRowCount = xColumn->availableRowCount();
	int yRowCount = yColumn->availableRowCount();
	int zRowCount = zColumn->availableRowCount();

	if (xRowCount < 1 || yRowCount < 1 || zRowCount < 1 || xRowCount != yRowCount || yRowCount != zRowCount)
		return;
	int numPoints = xRowCount; // Assuming xRowCount, yRowCount, and zRowCount are the same
	int numRows = std::sqrt(numPoints); // Assuming square grid; adjust if not square
	if (numRows * numRows != numPoints)
		return;
	auto dataArray = std::make_unique<QSurfaceDataArray>();
	dataArray->reserve(numRows);
	// Insert data rows into the data array
	for (int y = 0; y < numRows; ++y) {
		auto dataRow = std::make_unique<QSurfaceDataRow>(numRows);
		for (int x = 0; x < numRows; ++x) {
			int index = y * numRows + x;
			float xVal = xColumn->valueAt(index);
			float yVal = yColumn->valueAt(index);
			float zVal = zColumn->valueAt(index);
			(*dataRow)[x] = QSurfaceDataItem(QVector3D(xVal, yVal, zVal));
		}
		dataArray->append(*dataRow.release());
	}
	QSurfaceDataProxy* dataProxy = new QSurfaceDataProxy();
	dataProxy->resetArray(*dataArray.release());
	QSurface3DSeries* series = new QSurface3DSeries(dataProxy);
	q->m_surface->addSeries(series);
	Q_EMIT q->changed();
}

////////////////////////////////////////////////////////////////////////////////

void Surface3DPlotPrivate::saveSpreadsheetConfig(QXmlStreamWriter* writer) const {
	writer->writeStartElement("spreadsheet");
	WRITE_COLUMN(xColumn, xColumn);
	WRITE_COLUMN(yColumn, yColumn);
	WRITE_COLUMN(zColumn, zColumn);
	writer->writeEndElement();
}

void Surface3DPlotPrivate::saveMatrixConfig(QXmlStreamWriter* writer) const {
	writer->writeStartElement("matrix");
	writer->writeAttribute("matrixPath", matrix ? matrix->path() : QLatin1String(""));
	writer->writeEndElement();
}

bool Surface3DPlotPrivate::loadSpreadsheetConfig(XmlStreamReader* reader) {
	const QXmlStreamAttributes& attribs = reader->attributes();
	QString str;
	Surface3DPlotPrivate* d = this;
	READ_COLUMN(xColumn);
	READ_COLUMN(yColumn);
	READ_COLUMN(zColumn);

	return true;
}

bool Surface3DPlotPrivate::loadMatrixConfig(XmlStreamReader* reader) {
	const QXmlStreamAttributes& attribs = reader->attributes();
	matrixPath = attribs.value("matrixPath").toString();
	return true;
}
