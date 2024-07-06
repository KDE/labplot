#include "Surface3DPlotArea.h"
#include "Surface3DPlotAreaPrivate.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/macros.h"
#include <Q3DSurface>
#include <QAbstract3DSeries>
#include <QXmlStreamAttributes>
#include <QtGraphs/Q3DSurface>
#include <qabstract3dgraph.h>

Surface3DPlotArea::Surface3DPlotArea(const QString& name)
    : WorksheetElement(name, new Surface3DPlotAreaPrivate(this), AspectType::SurfacePlot)
	, m_surface{new Q3DSurface()} {
}

Surface3DPlotArea::~Surface3DPlotArea() {
	delete m_surface;
}
// Spreadsheet slots
void Surface3DPlotArea::xColumnAboutToBeRemoved(const AbstractAspect*) {
	Q_D(Surface3DPlotArea);
	d->xColumn = nullptr;
}

void Surface3DPlotArea::yColumnAboutToBeRemoved(const AbstractAspect*) {
	Q_D(Surface3DPlotArea);
	d->yColumn = nullptr;
}

void Surface3DPlotArea::zColumnAboutToBeRemoved(const AbstractAspect*) {
	Q_D(Surface3DPlotArea);
	d->zColumn = nullptr;
}

void Surface3DPlotArea::firstNodeAboutToBeRemoved(const AbstractAspect*) {
	Q_D(Surface3DPlotArea);
	d->firstNode = nullptr;
}

void Surface3DPlotArea::secondNodeAboutToBeRemoved(const AbstractAspect*) {
	Q_D(Surface3DPlotArea);
	d->secondNode = nullptr;
}

void Surface3DPlotArea::thirdNodeAboutToBeRemoved(const AbstractAspect*) {
	Q_D(Surface3DPlotArea);
	d->thirdNode = nullptr;
}

// Matrix slots
void Surface3DPlotArea::matrixAboutToBeRemoved(const AbstractAspect*) {
	Q_D(Surface3DPlotArea);
	d->matrix = nullptr;
}

// ##############################################################################
// ##########################  getter methods  ##################################
// ##############################################################################

// General parameters
BASIC_SHARED_D_READER_IMPL(Surface3DPlotArea, Surface3DPlotArea::DataSource, dataSource, sourceType)
BASIC_SHARED_D_READER_IMPL(Surface3DPlotArea, Surface3DPlotArea::MeshType, meshType, meshType)
BASIC_SHARED_D_READER_IMPL(Surface3DPlotArea, Surface3DPlotArea::DrawMode, drawMode, drawMode)

BASIC_SHARED_D_READER_IMPL(Surface3DPlotArea, QColor, color, color)
BASIC_SHARED_D_READER_IMPL(Surface3DPlotArea, double, opacity, opacity)

BASIC_SHARED_D_READER_IMPL(Surface3DPlotArea, bool, flatShading, flatShading)
BASIC_SHARED_D_READER_IMPL(Surface3DPlotArea, bool, gridVisibility, gridVisibility)
BASIC_SHARED_D_READER_IMPL(Surface3DPlotArea, Surface3DPlotArea::ShadowQuality, shadowQuality, shadowQuality)
BASIC_SHARED_D_READER_IMPL(Surface3DPlotArea, bool, smooth, smooth)

// Matrix parameters
BASIC_SHARED_D_READER_IMPL(Surface3DPlotArea, const Matrix*, matrix, matrix)
const QString& Surface3DPlotArea::matrixPath() const {
	Q_D(const Surface3DPlotArea);
	return d->matrixPath;
}

// Spreadsheet parameters
BASIC_SHARED_D_READER_IMPL(Surface3DPlotArea, const AbstractColumn*, xColumn, xColumn)
BASIC_SHARED_D_READER_IMPL(Surface3DPlotArea, const AbstractColumn*, yColumn, yColumn)
BASIC_SHARED_D_READER_IMPL(Surface3DPlotArea, const AbstractColumn*, zColumn, zColumn)

BASIC_SHARED_D_READER_IMPL(Surface3DPlotArea, const AbstractColumn*, firstNode, firstNode)
BASIC_SHARED_D_READER_IMPL(Surface3DPlotArea, const AbstractColumn*, secondNode, secondNode)
BASIC_SHARED_D_READER_IMPL(Surface3DPlotArea, const AbstractColumn*, thirdNode, thirdNode)

const QString& Surface3DPlotArea::xColumnPath() const {
	Q_D(const Surface3DPlotArea);
	return d->xColumnPath;
}
const QString& Surface3DPlotArea::yColumnPath() const {
	Q_D(const Surface3DPlotArea);
	return d->yColumnPath;
}
const QString& Surface3DPlotArea::zColumnPath() const {
	Q_D(const Surface3DPlotArea);
	return d->zColumnPath;
}

const QString& Surface3DPlotArea::firstNodePath() const {
	Q_D(const Surface3DPlotArea);
	return d->firstNodePath;
}
const QString& Surface3DPlotArea::secondNodePath() const {
	Q_D(const Surface3DPlotArea);
	return d->secondNodePath;
}
const QString& Surface3DPlotArea::thirdNodePath() const {
	Q_D(const Surface3DPlotArea);
	return d->thirdNodePath;
}

void Surface3DPlotArea::show(bool visible) {
	if (m_surface) {
		m_surface->setVisible(visible);
	}
}

// ##############################################################################
// #################  setter methods and undo commands ##########################
// ##############################################################################

// General
STD_SETTER_CMD_IMPL_F_S(Surface3DPlotArea, SetDataSource, Surface3DPlotArea::DataSource, sourceType, generateData)
void Surface3DPlotArea::setDataSource(Surface3DPlotArea::DataSource sourceType) {
	Q_D(Surface3DPlotArea);
	if (sourceType != d->sourceType)
		exec(new Surface3DPlotAreaSetDataSourceCmd(d, sourceType, ki18n("%1: data source changed")));
}
STD_SETTER_CMD_IMPL_F_S(Surface3DPlotArea, SetDrawMode, Surface3DPlotArea::DrawMode, drawMode, recalc)
void Surface3DPlotArea::setDrawMode(Surface3DPlotArea::DrawMode drawMode) {
	Q_D(Surface3DPlotArea);
	if (drawMode != d->drawMode)
		exec(new Surface3DPlotAreaSetDrawModeCmd(d, drawMode, ki18n("%1: draw mode changed")));
}
STD_SETTER_CMD_IMPL_F_S(Surface3DPlotArea, SetMeshType, Surface3DPlotArea::MeshType, meshType, recalc)
void Surface3DPlotArea::setMeshType(Surface3DPlotArea::MeshType meshType) {
	Q_D(Surface3DPlotArea);
	if (meshType != d->meshType) {
		exec(new Surface3DPlotAreaSetMeshTypeCmd(d, meshType, ki18n("%1: draw mode changed")));
	}
}
STD_SETTER_CMD_IMPL_F_S(Surface3DPlotArea, SetColor, QColor, color, recalc)
void Surface3DPlotArea::setColor(QColor color) {
	Q_D(Surface3DPlotArea);
	if (color != d->color) {
		exec(new Surface3DPlotAreaSetColorCmd(d, color, ki18n("%1: color changed")));
	}
}
STD_SETTER_CMD_IMPL_F_S(Surface3DPlotArea, SetOpacity, double, opacity, recalc)
void Surface3DPlotArea::setOpacity(double opacity) {
	Q_D(Surface3DPlotArea);
	if (opacity != d->opacity) {
		exec(new Surface3DPlotAreaSetOpacityCmd(d, opacity, ki18n("%1: opacity changed")));
	}
}
STD_SETTER_CMD_IMPL_F_S(Surface3DPlotArea, SetFlatShading, bool, flatShading, recalc)
void Surface3DPlotArea::setFlatShading(bool flatShading) {
	Q_D(Surface3DPlotArea);
	if (flatShading != d->flatShading) {
		exec(new Surface3DPlotAreaSetFlatShadingCmd(d, flatShading, ki18n("%1: flat shading changed")));
	}
}
STD_SETTER_CMD_IMPL_F_S(Surface3DPlotArea, SetGridVisibility, bool, gridVisibility, recalc)
void Surface3DPlotArea::setGridVisibility(bool gridVisibility) {
	Q_D(Surface3DPlotArea);
	if (gridVisibility != d->gridVisibility) {
		exec(new Surface3DPlotAreaSetGridVisibilityCmd(d, gridVisibility, ki18n("%1: grid visibility changed")));
	}
}

STD_SETTER_CMD_IMPL_F_S(Surface3DPlotArea, SetShadowQuality, Surface3DPlotArea::ShadowQuality, shadowQuality, recalc)
void Surface3DPlotArea::setShadowQuality(Surface3DPlotArea::ShadowQuality shadowQuality) {
	Q_D(Surface3DPlotArea);
	if (shadowQuality != d->shadowQuality) {
		exec(new Surface3DPlotAreaSetShadowQualityCmd(d, shadowQuality, ki18n("%1: shadow quality changed")));
	}
}
STD_SETTER_CMD_IMPL_F_S(Surface3DPlotArea, SetSmooth, bool, smooth, recalc)
void Surface3DPlotArea::setSmooth(bool smooth) {
	Q_D(Surface3DPlotArea);
	if (smooth != d->smooth) {
		exec(new Surface3DPlotAreaSetSmoothCmd(d, smooth, ki18n("%1: smooth changed")));
	}
}
STD_SETTER_CMD_IMPL_F_S(Surface3DPlotArea, SetMatrix, const Matrix*, matrix, generateData)
void Surface3DPlotArea::setMatrix(const Matrix* matrix) {
    Q_D(Surface3DPlotArea);
    if (matrix != d->matrix) {
        exec(new Surface3DPlotAreaSetMatrixCmd(d, matrix, ki18n("%1: matrix changed")));
    }
}
STD_SETTER_CMD_IMPL_F_S(Surface3DPlotArea, SetXColumn, const AbstractColumn*, xColumn, generateData)
void Surface3DPlotArea::setXColumn(const AbstractColumn* xCol) {
    Q_D(Surface3DPlotArea);
    if (xCol != d->xColumn) {
        exec(new Surface3DPlotAreaSetXColumnCmd(d, xCol, ki18n("%1: X Column changed")));
    }
}
STD_SETTER_CMD_IMPL_F_S(Surface3DPlotArea, SetYColumn, const AbstractColumn*, yColumn, generateData)
void Surface3DPlotArea::setYColumn(const AbstractColumn* yCol) {
    Q_D(Surface3DPlotArea);
    if (yCol != d->yColumn) {
        exec(new Surface3DPlotAreaSetYColumnCmd(d, yCol, ki18n("%1: Y Column changed")));
    }
}
STD_SETTER_CMD_IMPL_F_S(Surface3DPlotArea, SetZColumn, const AbstractColumn*, zColumn, generateData)
void Surface3DPlotArea::setZColumn(const AbstractColumn* zCol) {
    Q_D(Surface3DPlotArea);
    if (zCol != d->zColumn) {
        exec(new Surface3DPlotAreaSetZColumnCmd(d, zCol, ki18n("%1: Z Column changed")));
    }
}
STD_SETTER_CMD_IMPL_F_S(Surface3DPlotArea, SetFirstNode, const AbstractColumn*, firstNode, generateData)
void Surface3DPlotArea::setFirstNode(const AbstractColumn* firstNode) {
    Q_D(Surface3DPlotArea);
    if (firstNode != d->firstNode) {
        exec(new Surface3DPlotAreaSetFirstNodeCmd(d, firstNode, ki18n("%1: first node changed")));
    }
}
STD_SETTER_CMD_IMPL_F_S(Surface3DPlotArea, SetSecondNode, const AbstractColumn*, secondNode, generateData)
void Surface3DPlotArea::setSecondNode(const AbstractColumn* secondNode) {
    Q_D(Surface3DPlotArea);
    if (secondNode != d->secondNode) {
        exec(new Surface3DPlotAreaSetYColumnCmd(d, secondNode, ki18n("%1: second node changed")));
    }
}
STD_SETTER_CMD_IMPL_F_S(Surface3DPlotArea, SetThirdNode, const AbstractColumn*, thirdNode, generateData)
void Surface3DPlotArea::setThirdNode(const AbstractColumn* thirdNode) {
    Q_D(Surface3DPlotArea);
    if (thirdNode != d->thirdNode) {
        exec(new Surface3DPlotAreaSetZColumnCmd(d, thirdNode, ki18n("%1: third node changed")));
    }
}

void Surface3DPlotArea::handleResize(double horizontalRatio, double verticalRatio, bool pageResize) {
}
void Surface3DPlotArea::retransform(){};

// #####################################################################
// ################### Private implementation ##########################
// #####################################################################
Surface3DPlotAreaPrivate::Surface3DPlotAreaPrivate(Surface3DPlotArea* owner)
	: WorksheetElementPrivate(owner)
	, q(owner) {
}

void Surface3DPlotAreaPrivate::retransform() {
}
void Surface3DPlotAreaPrivate::recalcShapeAndBoundingRect() {
}

void Surface3DPlotAreaPrivate::generateData() const {
    if (sourceType == Surface3DPlotArea::DataSource_Empty)
        generateDemoData();
    else if (sourceType == Surface3DPlotArea::DataSource_Spreadsheet)
        generateSpreadsheetData();
    else if (sourceType == Surface3DPlotArea::DataSource_Matrix)
        generateMatrixData();
}

void Surface3DPlotAreaPrivate::generateDemoData() const {
    qDebug() << Q_FUNC_INFO << q->name();

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

    // Additional steps to ensure the surface is displayed
    q->m_surface->axisX()->setLabelFormat(QLatin1String("%.1f"));
    q->m_surface->axisZ()->setLabelFormat(QLatin1String("%.1f"));
    q->m_surface->axisX()->setRange(-radius, radius);
    q->m_surface->axisY()->setRange(-radius, radius);
    q->m_surface->axisZ()->setRange(-radius, radius);
    // Adjust camera settings for better view

    q->m_surface->setCameraZoomLevel(100);
    q->m_surface->setMinCameraZoomLevel(50);
    q->m_surface->setMaxCameraZoomLevel(200);
    q->m_surface->setCameraXRotation(45);
    q->m_surface->setCameraYRotation(20);
}

void Surface3DPlotAreaPrivate::generateMatrixData() const {
    if (!matrix) {
        qDebug() << Q_FUNC_INFO << q->name() << "Matrix has not been set";
        return;
    }
    if (!matrix->rowCount())
        return;

    qDebug() << Q_FUNC_INFO << q->name() << "Matrix has been set!";
    QSurfaceDataArray* dataArray = new QSurfaceDataArray;
	dataArray->reserve(matrix->columnCount());

	const double deltaX = (matrix->xEnd() - matrix->xStart()) / matrix->columnCount();
	const double deltaY = (matrix->yEnd() - matrix->yStart()) / matrix->rowCount();

	for (int x = 0; x < matrix->columnCount(); ++x) {
		QSurfaceDataRow* newRow = new QSurfaceDataRow(matrix->rowCount());

		for (int y = 0; y < matrix->rowCount(); ++y) {
			const double x_val = matrix->xStart() + deltaX * x;
			const double y_val = matrix->yStart() + deltaY * y;
			const double z_val = matrix->cell<double>(x, y);

			(*newRow)[y].setPosition(QVector3D(x_val, y_val, z_val));
		}

		*dataArray << *newRow;
	}

	QSurface3DSeries* series = new QSurface3DSeries;
	series->dataProxy()->resetArray(*dataArray);
	series->setDrawMode(QSurface3DSeries::DrawSurfaceAndWireframe);
	series->setFlatShadingEnabled(true);

	// Add the series to the Q3DSurface
	q->m_surface->addSeries(series);
}

void Surface3DPlotAreaPrivate::generateSpreadsheetData() const {
    qDebug() << Q_FUNC_INFO;
    if (xColumn == nullptr || yColumn == nullptr || zColumn == nullptr) {
        return;
    }
    if (!xColumn->rowCount() || !yColumn->rowCount() || !zColumn->rowCount())
        return;

    // Create a QSurfaceDataArray to hold the data
    QSurfaceDataArray* dataArray = new QSurfaceDataArray;

    // Assuming xColumn, yColumn, and zColumn are vectors
    const int numPoints = std::min({xColumn->rowCount(), yColumn->rowCount(), zColumn->rowCount()});
    const int numRows = static_cast<int>(sqrt(numPoints)); // Assuming data forms a square grid
    const int numCols = numRows; // Same as numRows for square grid

    // Setup range
    QPair<float, float> xRange(INT_MAX, INT_MIN);
    QPair<float, float> yRange(INT_MAX, INT_MIN);
    QPair<float, float> zRange(INT_MAX, INT_MIN);

    // Populate the dataArray with points
    for (int i = 0; i < numRows; ++i) {
        QSurfaceDataRow* newRow = new QSurfaceDataRow(numCols);
        for (int j = 0; j < numCols; ++j) {
            int index = i * numCols + j;
            if (index >= numPoints)
                break; // Ensure we don't go out of bounds

            const float x = static_cast<float>(xColumn->valueAt(index));
            const float y = static_cast<float>(yColumn->valueAt(index));
            const float z = static_cast<float>(zColumn->valueAt(index));

            xRange.first = std::min(xRange.first, x);
            xRange.second = std::max(xRange.second, x);
            yRange.first = std::min(yRange.first, y);
            yRange.second = std::max(yRange.second, y);
            zRange.first = std::min(zRange.first, z);
            zRange.second = std::max(zRange.second, z);

            (*newRow)[j].setPosition(QVector3D(x, y, z));
        }
        *dataArray << *newRow;
    }

    // Create a QSurfaceDataProxy and set the data array
    QSurfaceDataProxy* proxy = new QSurfaceDataProxy();
    proxy->resetArray(*dataArray);

    // Create a QSurface3DSeries and set the proxy
    QSurface3DSeries* series = new QSurface3DSeries(proxy);
    q->m_surface->addSeries(series);

    // Additional steps to ensure the surface is displayed
    q->m_surface->axisX()->setRange(xRange.first, xRange.second);
    q->m_surface->axisY()->setRange(yRange.first, yRange.second);
    q->m_surface->axisZ()->setRange(zRange.first, zRange.second);

    // Adjust camera settings for better view
    q->m_surface->setCameraZoomLevel(100);
    q->m_surface->setMinCameraZoomLevel(50);
    q->m_surface->setMaxCameraZoomLevel(200);
    q->m_surface->setCameraXRotation(45);
    q->m_surface->setCameraYRotation(20);
} ////////////////////////////////////////////////////////////////////////////////

void Surface3DPlotAreaPrivate::saveSpreadsheetConfig(QXmlStreamWriter* writer) const {
    writer->writeStartElement("spreadsheet");
    WRITE_COLUMN(xColumn, xColumn);
    WRITE_COLUMN(yColumn, yColumn);
    WRITE_COLUMN(zColumn, zColumn);

    WRITE_COLUMN(firstNode, firstNode);
    WRITE_COLUMN(secondNode, secondNode);
    WRITE_COLUMN(thirdNode, thirdNode);
    writer->writeEndElement();
}

void Surface3DPlotAreaPrivate::saveMatrixConfig(QXmlStreamWriter* writer) const {
    writer->writeStartElement("matrix");
    writer->writeAttribute("matrixPath", matrix ? matrix->path() : QLatin1String(""));
    writer->writeEndElement();
}

bool Surface3DPlotAreaPrivate::loadSpreadsheetConfig(XmlStreamReader* reader) {
    const QXmlStreamAttributes& attribs = reader->attributes();
    QString str;
    Surface3DPlotAreaPrivate* d = this;
    READ_COLUMN(xColumn);
    READ_COLUMN(yColumn);
    READ_COLUMN(zColumn);

    READ_COLUMN(firstNode);
    READ_COLUMN(secondNode);
    READ_COLUMN(thirdNode);
    return true;
}

bool Surface3DPlotAreaPrivate::loadMatrixConfig(XmlStreamReader* reader) {
    const QXmlStreamAttributes& attribs = reader->attributes();
    matrixPath = attribs.value("matrixPath").toString();
    return true;
}
void Surface3DPlotAreaPrivate::recalc() {
    qDebug() << Q_FUNC_INFO;
    if (!q->m_surface->seriesList().isEmpty()) {
        QSurface3DSeries* series = q->m_surface->seriesList().first();

        series->setDrawMode(static_cast<QSurface3DSeries::DrawFlags>(drawMode));

        // Update the mesh type
        series->setMesh(static_cast<QAbstract3DSeries::Mesh>(meshType));

        // Update the color
        series->setBaseColor(color);

        // Update the opacity
        series->setBaseColor(QColor(color.red(), color.green(), color.blue(), static_cast<int>(opacity * 255)));

        // Update flat shading
        series->setFlatShadingEnabled(flatShading);

        // Update smoothness
        series->setMeshSmooth(smooth);
    }
    // Update grid visibility
    q->gridVisibilityChanged(gridVisibility);

    // Update shadow
    q->m_surface->setShadowQuality(static_cast<Q3DSurface::ShadowQuality>(shadowQuality));
}
