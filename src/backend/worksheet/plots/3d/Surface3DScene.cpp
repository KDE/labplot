/*
	File                 : Surface3DScene.cpp
	Project              : LabPlot
	Description          : 3D Surface Plot
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024-2026 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2024 Kuntal Bar <barkuntal6@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "Surface3DScene.h"
#include "Axis3D.h"
#include "Surface3DScenePrivate.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/macros.h"
#include "backend/worksheet/WorksheetElementPrivate.h"

#include <QGraphicsScene>
#include <QGraphicsProxyWidget>
#include <QtGraphsWidgets/Q3DSurfaceWidgetItem>
//#include <QtGraphs/QAbstract3DSeries>
//#include <QXmlStreamAttributes>

#include <KLocalizedString>

Surface3DScene::Surface3DScene(const QString& name)
	: Base3DPlot(name, new Surface3DScenePrivate(this), Base3DPlot::Surface, AspectType::Surface3DScene) {
	m_surface = new Q3DSurfaceWidgetItem();

	// Create QQuickWidget and set it on the WidgetItem
	auto* quickWidget = new QQuickWidget();
	m_surface->setWidget(quickWidget);

	// Apply initial rotation and zoom values to the widget
	Q_D(Surface3DScene);
	m_surface->setCameraXRotation(d->xRotation);
	m_surface->setCameraYRotation(d->yRotation);
	m_surface->setCameraZoomLevel(d->zoomLevel);

	// Only create default axes when NOT loading from file
	// Check parent too - if Scene is loading, axes come as separate children
	auto* parent = parentAspect();
	bool parentLoading = parent && parent->isLoading();

	if (!isLoading() && !parentLoading) {
		Axis3D* xAxis = new Axis3D(QStringLiteral("x-axis"), Axis3D::X);
		Axis3D* yAxis = new Axis3D(QStringLiteral("y-axis"), Axis3D::Y);
		Axis3D* zAxis = new Axis3D(QStringLiteral("z-axis"), Axis3D::Z);
		addChild(xAxis);
		addChild(yAxis);
		addChild(zAxis);
		m_surface->setAxisX(xAxis->m_axis);
		m_surface->setAxisY(yAxis->m_axis);
		m_surface->setAxisZ(zAxis->m_axis);
	}
}

void Surface3DScene::finalizeAdd() {
	WorksheetElement::finalizeAdd();
}

Surface3DScene::~Surface3DScene() {
}

void Surface3DScene::init(bool transform) {
	Q_D(Surface3DScene);

	// Create proxy widget and set as child of this graphics item
	if (m_surface && m_surface->widget()) {
		auto* widget = m_surface->widget();
		auto* proxy = new QGraphicsProxyWidget();
		proxy->setParentItem(static_cast<QGraphicsItem*>(d));
		proxy->setWidget(widget);

		// Size from rect
		QRectF rect = d->rect;
		proxy->setGeometry(QRectF(-rect.width()/2, -rect.height()/2, rect.width(), rect.height()));
		widget->resize(rect.width(), rect.height());
		widget->show();
	}

	if (transform)
		retransform();
}

void Surface3DScene::retransform() {
	Q_D(Surface3DScene);
	d->retransform();
}

void Surface3DScene::finalizeLoad() {
	// Connect axes to the 3D widget after load
	/*
	for (auto* child : children<Axis3D>()) {
		if (child->type() == Axis3D::X)
			m_surface->setAxisX(child->m_axis);
		else if (child->type() == Axis3D::Y)
			m_surface->setAxisY(child->m_axis);
		else if (child->type() == Axis3D::Z)
			m_surface->setAxisZ(child->m_axis);
	}*/
}
// Spreadsheet slots
void Surface3DScene::xColumnAboutToBeRemoved(const AbstractAspect*) {
	Q_D(Surface3DScene);
	d->xColumn = nullptr;
}

void Surface3DScene::yColumnAboutToBeRemoved(const AbstractAspect*) {
	Q_D(Surface3DScene);
	d->yColumn = nullptr;
}

void Surface3DScene::zColumnAboutToBeRemoved(const AbstractAspect*) {
	Q_D(Surface3DScene);
	d->zColumn = nullptr;
}

// Matrix slots
void Surface3DScene::matrixAboutToBeRemoved(const AbstractAspect*) {
	Q_D(Surface3DScene);
	d->matrix = nullptr;
}

// ##############################################################################
// ##########################  getter methods  ##################################
// ##############################################################################

// General parameters
BASIC_SHARED_D_READER_IMPL(Surface3DScene, Surface3DScene::DataSource, dataSource, sourceType)
BASIC_SHARED_D_READER_IMPL(Surface3DScene, Surface3DScene::DrawMode, drawMode, drawMode)

BASIC_SHARED_D_READER_IMPL(Surface3DScene, QColor, color, color)

BASIC_SHARED_D_READER_IMPL(Surface3DScene, bool, flatShading, flatShading)
BASIC_SHARED_D_READER_IMPL(Surface3DScene, bool, smooth, smooth)

// Matrix parameters
BASIC_SHARED_D_READER_IMPL(Surface3DScene, const Matrix*, matrix, matrix)
const QString& Surface3DScene::matrixPath() const {
	Q_D(const Surface3DScene);
	return d->matrixPath;
}

// Spreadsheet parameters
BASIC_SHARED_D_READER_IMPL(Surface3DScene, const AbstractColumn*, xColumn, xColumn)
BASIC_SHARED_D_READER_IMPL(Surface3DScene, const AbstractColumn*, yColumn, yColumn)
BASIC_SHARED_D_READER_IMPL(Surface3DScene, const AbstractColumn*, zColumn, zColumn)

const QString& Surface3DScene::xColumnPath() const {
	Q_D(const Surface3DScene);
	return d->xColumnPath;
}
const QString& Surface3DScene::yColumnPath() const {
	Q_D(const Surface3DScene);
	return d->yColumnPath;
}
const QString& Surface3DScene::zColumnPath() const {
	Q_D(const Surface3DScene);
	return d->zColumnPath;
}

void Surface3DScene::show(bool visible) {
	// Visibility is handled by Scene's QGraphicsProxyWidget
	Q_UNUSED(visible)
}

void Surface3DScene::recalc() {
	Q_D(Surface3DScene);
	d->recalc();
}

// ##############################################################################
// #################  setter methods and undo commands ##########################
// ##############################################################################

// General
STD_SETTER_CMD_IMPL_F_S(Surface3DScene, SetDataSource, Surface3DScene::DataSource, sourceType, recalc)
void Surface3DScene::setDataSource(Surface3DScene::DataSource sourceType) {
	Q_D(Surface3DScene);
	if (sourceType != d->sourceType)
		exec(new Surface3DSceneSetDataSourceCmd(d, sourceType, ki18n("%1: data source changed")));
}
STD_SETTER_CMD_IMPL_F_S(Surface3DScene, SetDrawMode, Surface3DScene::DrawMode, drawMode, updateDrawMode)
void Surface3DScene::setDrawMode(Surface3DScene::DrawMode drawMode) {
	Q_D(Surface3DScene);
	if (drawMode != d->drawMode)
		exec(new Surface3DSceneSetDrawModeCmd(d, drawMode, ki18n("%1: draw mode changed")));
}

STD_SETTER_CMD_IMPL_F_S(Surface3DScene, SetColor, QColor, color, updateColor)
void Surface3DScene::setColor(QColor color) {
	Q_D(Surface3DScene);
	if (color != d->color)
		exec(new Surface3DSceneSetColorCmd(d, color, ki18n("%1: color changed")));
}

STD_SETTER_CMD_IMPL_F_S(Surface3DScene, SetFlatShading, bool, flatShading, updateFlatShading)
void Surface3DScene::setFlatShading(bool flatShading) {
	Q_D(Surface3DScene);
	if (flatShading != d->flatShading)
		exec(new Surface3DSceneSetFlatShadingCmd(d, flatShading, ki18n("%1: flat shading changed")));
}

STD_SETTER_CMD_IMPL_F_S(Surface3DScene, SetSmooth, bool, smooth, updateSmoothMesh)
void Surface3DScene::setSmooth(bool smooth) {
	Q_D(Surface3DScene);
	if (smooth != d->smooth)
		exec(new Surface3DSceneSetSmoothCmd(d, smooth, ki18n("%1: smooth changed")));
}
STD_SETTER_CMD_IMPL_F_S(Surface3DScene, SetMatrix, const Matrix*, matrix, recalc)
void Surface3DScene::setMatrix(const Matrix* matrix) {
	Q_D(Surface3DScene);
	if (matrix != d->matrix)
		exec(new Surface3DSceneSetMatrixCmd(d, matrix, ki18n("%1: matrix changed")));
	if (matrix) {
		connect(matrix, &Matrix::dataChanged, this, &Surface3DScene::recalc);
		if (matrix->parentAspect())
			connect(matrix->parentAspect(), &AbstractAspect::childAspectAboutToBeRemoved, this, &Surface3DScene::matrixAboutToBeRemoved);
	}
}
STD_SETTER_CMD_IMPL_F_S(Surface3DScene, SetXColumn, const AbstractColumn*, xColumn, recalc)
void Surface3DScene::setXColumn(const AbstractColumn* xCol) {
	Q_D(Surface3DScene);
	if (xCol != d->xColumn)
		exec(new Surface3DSceneSetXColumnCmd(d, xCol, ki18n("%1: X Column changed")));
	if (xCol) {
		connect(xCol, &AbstractColumn::dataChanged, this, &Surface3DScene::recalc);
		if (xCol->parentAspect())
			connect(xCol->parentAspect(), &AbstractAspect::childAspectAboutToBeRemoved, this, &Surface3DScene::xColumnAboutToBeRemoved);
	}
}
STD_SETTER_CMD_IMPL_F_S(Surface3DScene, SetYColumn, const AbstractColumn*, yColumn, recalc)
void Surface3DScene::setYColumn(const AbstractColumn* yCol) {
	Q_D(Surface3DScene);
	if (yCol != d->yColumn)
		exec(new Surface3DSceneSetYColumnCmd(d, yCol, ki18n("%1: Y Column changed")));
	if (yCol) {
		connect(yCol, &AbstractColumn::dataChanged, this, &Surface3DScene::recalc);
		if (yCol->parentAspect())
			connect(yCol->parentAspect(), &AbstractAspect::childAspectAboutToBeRemoved, this, &Surface3DScene::yColumnAboutToBeRemoved);
	}
}
STD_SETTER_CMD_IMPL_F_S(Surface3DScene, SetZColumn, const AbstractColumn*, zColumn, recalc)
void Surface3DScene::setZColumn(const AbstractColumn* zCol) {
	Q_D(Surface3DScene);
	if (zCol != d->zColumn)
		exec(new Surface3DSceneSetZColumnCmd(d, zCol, ki18n("%1: Z Column changed")));
	if (zCol) {
		connect(zCol, &AbstractColumn::dataChanged, this, &Surface3DScene::recalc);
		if (zCol->parentAspect())
			connect(zCol->parentAspect(), &AbstractAspect::childAspectAboutToBeRemoved, this, &Surface3DScene::zColumnAboutToBeRemoved);
	}
}

// #####################################################################
// ################### Private implementation ##########################
// #####################################################################

Surface3DScenePrivate::Surface3DScenePrivate(Surface3DScene* owner)
	: Base3DPlotPrivate(owner)
	, q(owner)
	, sourceType(Surface3DScene::DataSource_Spreadsheet)
	, drawMode(Surface3DScene::DrawWireframeSurface)
	, color(Qt::green) {
}

void Surface3DScenePrivate::recalc() {
	if (sourceType == Surface3DScene::DataSource_Empty)
		generateDemoData();
	else if (sourceType == Surface3DScene::DataSource_Spreadsheet)
		generateSpreadsheetData();
	else if (sourceType == Surface3DScene::DataSource_Matrix)
		generateMatrixData();

	Q_EMIT q->changed();
}

void Surface3DScenePrivate::updateDrawMode() {
	qDebug() << Q_FUNC_INFO;
	QSurface3DSeries* series = q->m_surface->seriesList().first();
	series->setDrawMode(static_cast<QSurface3DSeries::DrawFlags>(drawMode));
	Q_EMIT q->changed();
}

void Surface3DScenePrivate::updateColor() {
	auto* series = q->m_surface->seriesList().first();
	if (!series)
		return;
	series->setBaseColor(color);
	Q_EMIT q->changed();
}

void Surface3DScenePrivate::updateFlatShading() {
	qDebug() << Q_FUNC_INFO;
	QSurface3DSeries* series = q->m_surface->seriesList().first();
	// QtGraphs API: use setShading() instead of setFlatShadingEnabled()
	series->setShading(flatShading ? QSurface3DSeries::Shading::Flat : QSurface3DSeries::Shading::Smooth);
	Q_EMIT q->changed();
}

void Surface3DScenePrivate::updateSmoothMesh() {
	qDebug() << Q_FUNC_INFO;
	QSurface3DSeries* series = q->m_surface->seriesList().first();
	// QtGraphs API: setShading() handles both flat and smooth
	series->setShading(smooth ? QSurface3DSeries::Shading::Smooth : QSurface3DSeries::Shading::Flat);
	Q_EMIT q->changed();
}

void Surface3DScenePrivate::generateDemoData() const {
	if (!q->m_surface->seriesList().empty())
		q->m_surface->removeSeries(q->m_surface->seriesList().first());

	auto* dataArray = new QSurfaceDataArray;
	dataArray->reserve(50);
	// Parameters for sphere
	int n = 50; // Number of points
	float radius = 10.0f; // Radius of the sphere

	for (int i = 0; i < n; ++i) {
		QSurfaceDataRow newRow;
		newRow.resize(n);
		float theta = i * M_PI / n; // Angle theta
		for (int j = 0; j < n; ++j) {
			float phi = j * 2 * M_PI / n; // Angle phi
			float x = radius * sin(theta) * cos(phi);
			float y = radius * sin(theta) * sin(phi);
			float z = radius * cos(theta);
			newRow[j].setPosition(QVector3D(x, y, z));
		}
		dataArray->append(newRow);
	}

	auto* proxy = new QSurfaceDataProxy();
	proxy->resetArray(*dataArray);

	auto* series = new QSurface3DSeries(proxy);
	q->m_surface->addSeries(series);
	q->m_surface->axisX()->setRange(-radius, radius);
	q->m_surface->axisY()->setRange(-radius, radius);
	q->m_surface->axisZ()->setRange(-radius, radius);
	Q_EMIT q->changed();
}

void Surface3DScenePrivate::generateMatrixData() const {
	if (!matrix) {
		qDebug() << Q_FUNC_INFO << q->name() << "Matrix has not been set";
		return;
	}
	if (!matrix->rowCount())
		return;
	if (!q->m_surface->seriesList().empty())
		q->m_surface->removeSeries(q->m_surface->seriesList().first());

	qDebug() << Q_FUNC_INFO << q->name() << "Matrix has been set!";
	auto* dataArray = new QSurfaceDataArray;
	dataArray->reserve(matrix->rowCount());

	const double deltaX = (matrix->xEnd() - matrix->xStart()) / matrix->rowCount();
	const double deltaY = (matrix->yEnd() - matrix->yStart()) / matrix->columnCount();

	for (int x = 0; x < matrix->rowCount(); ++x) {
		QSurfaceDataRow newRow;
		newRow.resize(matrix->columnCount());

		for (int y = 0; y < matrix->columnCount(); ++y) {
			const double x_val = matrix->xStart() + deltaX * x;
			const double y_val = matrix->yStart() + deltaY * y;
			const double z_val = matrix->cell<double>(x, y);
			newRow[y].setPosition(QVector3D(x_val, z_val, y_val));
		}

		dataArray->append(newRow);
	}

	auto* series = new QSurface3DSeries;
	series->dataProxy()->resetArray(*dataArray);

	// Add the series to the Q3DSurface
	q->m_surface->addSeries(series);
	Q_EMIT q->changed();
}

void Surface3DScenePrivate::generateSpreadsheetData() const {
	qDebug() << Q_FUNC_INFO;

	if (!q->m_surface) {
		qDebug() << "m_surface is null";
		return;
	}

	// Widget must exist to access seriesList - defer if not ready
	// The widget is created when added to QGraphicsScene in Scene::init
	if (!q->m_surface->widget()) {
		qDebug() << "Widget not ready, will be called again after widget is shown";
		return;
	}

	qDebug() << "Widget is ready, generating data";

	// Remove existing series if any
	if (!q->m_surface->seriesList().empty())
		q->m_surface->removeSeries(q->m_surface->seriesList().first());

	if (!xColumn || !yColumn || !zColumn) {
		qDebug() << "Missing columns:" << (xColumn ? "X OK" : "X NULL") << (yColumn ? "Y OK" : "Y NULL") << (zColumn ? "Z OK" : "Z NULL");
		return;
	}

	int xRowCount = xColumn->availableRowCount();
	int yRowCount = yColumn->availableRowCount();
	int zRowCount = zColumn->availableRowCount();

	qDebug() << "Row counts: X=" << xRowCount << "Y=" << yRowCount << "Z=" << zRowCount;

	if (xRowCount < 1 || yRowCount < 1 || zRowCount < 1 || xRowCount != yRowCount || yRowCount != zRowCount)
		return;
	int numPoints = xRowCount;
	int numRows = std::sqrt(numPoints);
	qDebug() << "numPoints=" << numPoints << "numRows=" << numRows << "check:" << (numRows * numRows);
	if (numRows * numRows != numPoints)
		return;
	auto dataArray = std::make_unique<QSurfaceDataArray>();
	dataArray->reserve(numRows);

	// Track min/max for axis ranges
	float minX = std::numeric_limits<float>::max();
	float maxX = std::numeric_limits<float>::lowest();
	float minY = std::numeric_limits<float>::max();
	float maxY = std::numeric_limits<float>::lowest();
	float minZ = std::numeric_limits<float>::max();
	float maxZ = std::numeric_limits<float>::lowest();

	// Insert data rows into the data array
	for (int y = 0; y < numRows; ++y) {
		QSurfaceDataRow dataRow;
		dataRow.resize(numRows);
		for (int x = 0; x < numRows; ++x) {
			int index = y * numRows + x;
			float xVal = xColumn->valueAt(index);
			float yVal = yColumn->valueAt(index);
			float zVal = zColumn->valueAt(index);
			dataRow[x] = QSurfaceDataItem(QVector3D(xVal, yVal, zVal));

			minX = std::min(minX, xVal);
			maxX = std::max(maxX, xVal);
			minY = std::min(minY, yVal);
			maxY = std::max(maxY, yVal);
			minZ = std::min(minZ, zVal);
			maxZ = std::max(maxZ, zVal);
		}
		dataArray->append(dataRow);
	}
	auto* dataProxy = new QSurfaceDataProxy();
	dataProxy->resetArray(std::move(*dataArray));
	auto* series = new QSurface3DSeries(dataProxy);
	q->m_surface->addSeries(series);

	// Set axis ranges based on data
	q->m_surface->axisX()->setRange(minX, maxX);
	q->m_surface->axisY()->setRange(minY, maxY);
	q->m_surface->axisZ()->setRange(minZ, maxZ);

	qDebug() << "Series added, total series count:" << q->m_surface->seriesList().size();
	qDebug() << "Axis ranges - X:" << minX << "to" << maxX << "Y:" << minY << "to" << maxY << "Z:" << minZ << "to" << maxZ;
	Q_EMIT q->changed();
}

// ##############################################################################
// ##################  Serialization/Deserialization  ###########################
// ##############################################################################

void Surface3DScenePrivate::saveSpreadsheetConfig(QXmlStreamWriter* writer) const {
	writer->writeStartElement("spreadsheet");
	WRITE_COLUMN(xColumn, xColumn);
	WRITE_COLUMN(yColumn, yColumn);
	WRITE_COLUMN(zColumn, zColumn);
	writer->writeEndElement();
}

void Surface3DScenePrivate::saveMatrixConfig(QXmlStreamWriter* writer) const {
	writer->writeStartElement("matrix");
	writer->writeAttribute("matrixPath", matrix ? matrix->path() : QLatin1String(""));
	writer->writeEndElement();
}

bool Surface3DScenePrivate::loadSpreadsheetConfig(XmlStreamReader* reader) {
	const auto& attribs = reader->attributes();
	QString str;
	Surface3DScenePrivate* d = this;
	READ_COLUMN(xColumn);
	READ_COLUMN(yColumn);
	READ_COLUMN(zColumn);

	return true;
}

bool Surface3DScenePrivate::loadMatrixConfig(XmlStreamReader* reader) {
	const auto& attribs = reader->attributes();
	matrixPath = attribs.value("matrixPath").toString();
	return true;
}
void Surface3DScene::save(QXmlStreamWriter* writer) const {
	Q_D(const Surface3DScene);

	writer->writeStartElement("surface3dplot");

	// Save basic attributes
	writeBasicAttributes(writer);

	writer->writeStartElement("general");
	writer->writeAttribute("color", d->color.name());
	writer->writeAttribute("sourceType", QString::number(d->sourceType));
	writer->writeAttribute("drawMode", QString::number(d->drawMode));
	writer->writeAttribute("flatShading", QString::number(d->flatShading));
	writer->writeAttribute("smooth", QString::number(d->smooth));
	writer->writeAttribute("xRotation", QString::number(d->xRotation));
	writer->writeAttribute("yRotation", QString::number(d->yRotation));
	writer->writeAttribute("theme", QString::number(d->theme));
	writer->writeAttribute("zoomLevel", QString::number(d->zoomLevel));
	writer->writeAttribute("shadowQuality", QString::number(d->shadowQuality));
	writer->writeEndElement(); // End of "general"

	if (!d->xColumnPath.isEmpty()) {
		writer->writeStartElement("column");
		writer->writeAttribute("path", d->xColumnPath);
		writer->writeEndElement(); // End of "column"
	}

	if (!d->yColumnPath.isEmpty()) {
		writer->writeStartElement("column");
		writer->writeAttribute("path", d->yColumnPath);
		writer->writeEndElement(); // End of "column"
	}

	if (!d->zColumnPath.isEmpty()) {
		writer->writeStartElement("column");
		writer->writeAttribute("path", d->zColumnPath);
		writer->writeEndElement(); // End of "column"
	}

	// Save matrix or spreadsheet config based on sourceType
	if (d->sourceType == Surface3DScene::DataSource::DataSource_Matrix) {
		d->saveMatrixConfig(writer);
	} else if (d->sourceType == Surface3DScene::DataSource::DataSource_Spreadsheet) {
		d->saveSpreadsheetConfig(writer);
	}

	writer->writeEndElement(); // End of "surface3dplot"
}

bool Surface3DScene::load(XmlStreamReader* reader, bool preview) {
	Q_D(Surface3DScene);

	// Reading basic attributes
	if (!readBasicAttributes(reader))
		return false;

	QXmlStreamAttributes attribs;
	QString str;

	while (!reader->atEnd()) {
		reader->readNext();

		if (reader->isEndElement() && reader->name() == QLatin1String("surface3dplot"))
			break;

		if (!reader->isStartElement())
			continue;

		if (!preview && reader->name() == QLatin1String("comment")) {
			if (!readCommentElement(reader))
				return false;
		} else if (!preview && reader->name() == QLatin1String("general")) {
			attribs = reader->attributes();

			READ_INT_VALUE("xRotation", xRotation, int);
			READ_INT_VALUE("yRotation", yRotation, int);
			READ_INT_VALUE("theme", theme, Base3DPlot::Theme);
			READ_INT_VALUE("zoomLevel", zoomLevel, int);
			READ_INT_VALUE("shadowQuality", shadowQuality, Base3DPlot::ShadowQuality);

			str = attribs.value(QStringLiteral("color")).toString();
			if (!str.isEmpty())
				d->color.setNamedColor(str);

			str = attribs.value(QStringLiteral("sourceType")).toString();
			if (!str.isEmpty())
				d->sourceType = static_cast<Surface3DScene::DataSource>(str.toInt());

			str = attribs.value(QStringLiteral("drawMode")).toString();
			if (!str.isEmpty())
				d->drawMode = static_cast<Surface3DScene::DrawMode>(str.toInt());

			READ_INT_VALUE("flatShading", flatShading, bool)
			READ_INT_VALUE("smooth", smooth, bool)
		} else if (reader->name() == QLatin1String("column")) {
			attribs = reader->attributes();

			str = attribs.value(QStringLiteral("path")).toString();
			if (attribs.hasAttribute("xColumn"))
				d->xColumnPath = str;
			else if (attribs.hasAttribute("yColumn"))
				d->yColumnPath = str;
			else if (attribs.hasAttribute("zColumn"))
				d->zColumnPath = str;
		} else if (reader->name() == QLatin1String("matrix")) {
			if (!d->loadMatrixConfig(reader))
				return false;
		} else if (reader->name() == QLatin1String("spreadsheet")) {
			if (!d->loadSpreadsheetConfig(reader))
				return false;
		} else { // Unknown element handling
			reader->raiseWarning(i18n("Unknown element '%1'", reader->name().toString()));
			if (!reader->skipToEndElement())
				return false;
		}
	}

	// Set up dataColumns if needed
	// d->dataColumns.resize(d->columnPaths.size());  // Adjust based on your implementation

	// Connect axes after all children are loaded
	finalizeLoad();

	return true;
}
