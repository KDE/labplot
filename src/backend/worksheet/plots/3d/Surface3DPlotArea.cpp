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
    : WorksheetElementContainer(name, new Surface3DPlotAreaPrivate(this), AspectType::SurfacePlot)
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
BASIC_SHARED_D_READER_IMPL(Surface3DPlotArea, int, zoomLevel, zoomLevel)
BASIC_SHARED_D_READER_IMPL(Surface3DPlotArea, int, xRotation, xRotation)
BASIC_SHARED_D_READER_IMPL(Surface3DPlotArea, int, yRotation, yRotation)
BASIC_SHARED_D_READER_IMPL(Surface3DPlotArea, Surface3DPlotArea::Theme, theme, theme)


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

void Surface3DPlotArea::setZoomLevel(int zoomLevel) {
    Q_D(Surface3DPlotArea);
    m_surface->setCameraZoomLevel(zoomLevel);
    d->zoomLevel = zoomLevel;
    Q_EMIT zoomChanged(zoomLevel);
}
void Surface3DPlotArea::setXRotation(int value) {
    Q_D(Surface3DPlotArea);
    m_surface->setCameraXRotation(value);
    d->xRotation = value;
    Q_EMIT xRotationChanged(value);
}
void Surface3DPlotArea::setYRotation(int value) {
    Q_D(Surface3DPlotArea);
    m_surface->setCameraYRotation(value);
    d->yRotation = value;
    Q_EMIT yRotationChanged(value);
}
void Surface3DPlotArea::setTheme(Surface3DPlotArea::Theme theme) {
    Q_D(Surface3DPlotArea);
    m_surface->activeTheme()->setType(static_cast<Q3DTheme::Theme>(theme));
    d->theme = theme;
    Q_EMIT themeChanged(theme);
}
class Surface3DPlotAreaSetRectCmd : public QUndoCommand {
public:
    Surface3DPlotAreaSetRectCmd(Surface3DPlotAreaPrivate* private_obj, const QRectF& rect)
        : m_private(private_obj)
        , m_rect(rect) {
        setText(i18n("%1: change geometry rect", m_private->name()));
    }

    void redo() override {
        // 		const double horizontalRatio = m_rect.width() / m_private->rect.width();
        // 		const double verticalRatio = m_rect.height() / m_private->rect.height();

        qSwap(m_private->rect, m_rect);

        // 		m_private->q->handleResize(horizontalRatio, verticalRatio, false);
        m_private->retransform();
        Q_EMIT m_private->q->rectChanged(m_private->rect);
    }

    void undo() override {
        redo();
    }

private:
    Surface3DPlotAreaPrivate* m_private;
    QRectF m_rect;
};
void Surface3DPlotArea::setRect(const QRectF& rect) {
    Q_D(Surface3DPlotArea);
    if (rect != d->rect)
        exec(new Surface3DPlotAreaSetRectCmd(d, rect));
}
class Surface3DPlotAreaSetPrevRectCmd : public QUndoCommand {
public:
    Surface3DPlotAreaSetPrevRectCmd(Surface3DPlotAreaPrivate* private_obj, const QRectF& rect)
        : m_private(private_obj)
        , m_rect(rect) {
        setText(i18n("%1: change geometry rect", m_private->name()));
    }

    void redo() override {
        if (m_initilized) {
            qSwap(m_private->rect, m_rect);
            m_private->retransform();
            Q_EMIT m_private->q->rectChanged(m_private->rect);
        } else {
            // this function is called for the first time,
            // nothing to do, we just need to remember what the previous rect was
            // which has happened already in the constructor.
            m_initilized = true;
        }
    }

    void undo() override {
        redo();
    }

private:
    Surface3DPlotAreaPrivate* m_private;
    QRectF m_rect;
    bool m_initilized{false};
};

void Surface3DPlotArea::setPrevRect(const QRectF& prevRect) {
    Q_D(Surface3DPlotArea);
    exec(new Surface3DPlotAreaSetPrevRectCmd(d, prevRect));
}

void Surface3DPlotArea::handleResize(double horizontalRatio, double verticalRatio, bool pageResize) {
}
void Surface3DPlotArea::retransform() {
}

// #####################################################################
// ################### Private implementation ##########################
// #####################################################################
Surface3DPlotAreaPrivate::Surface3DPlotAreaPrivate(Surface3DPlotArea* owner)
    : WorksheetElementContainerPrivate(owner)
	, q(owner) {
}

void Surface3DPlotAreaPrivate::retransform() {
    const bool suppress = suppressRetransform || q->isLoading();
    trackRetransformCalled(suppress);

    if (suppress)
        return;

    prepareGeometryChange();
    setPos(rect.x() + rect.width() / 2, rect.y() + rect.height() / 2);

    q->setRect(rect);

    WorksheetElementContainerPrivate::recalcShapeAndBoundingRect();

    q->WorksheetElementContainer::retransform();
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
    if (!xColumn || !yColumn || !zColumn)
        return;
    if (!xColumn->rowCount() || !yColumn->rowCount() || !zColumn->rowCount())
        return;
    auto dataArray = std::make_unique<QSurfaceDataArray>();
    qDebug() << "Start generating points";

    int numPoints = std::min({xColumn->availableRowCount(), yColumn->availableRowCount(), zColumn->availableRowCount()});
    qDebug() << numPoints;

    for (int i = 0; i < numPoints - 1; i += 2) {
        auto dataRow = std::make_unique<QSurfaceDataRow>();

        int xVal1 = xColumn->valueAt(i);
        int yVal1 = yColumn->valueAt(i);
        int zVal1 = zColumn->valueAt(i);
        dataRow->append(QSurfaceDataItem(xVal1, yVal1, zVal1));

        int xVal2 = xColumn->valueAt(i + 1);
        int yVal2 = yColumn->valueAt(i + 1);
        int zVal2 = zColumn->valueAt(i + 1);
        dataRow->append(QSurfaceDataItem(xVal2, yVal2, zVal2));

        dataArray->append(*dataRow.release());
    }

    QSurfaceDataProxy* dataProxy = new QSurfaceDataProxy();
    dataProxy->resetArray(*dataArray.release());

    QSurface3DSeries* series = new QSurface3DSeries(dataProxy);
    q->m_surface->addSeries(series);

    qDebug() << "Data generation complete";
} ////////////////////////////////////////////////////////////////////////////////

void Surface3DPlotAreaPrivate::saveSpreadsheetConfig(QXmlStreamWriter* writer) const {
    writer->writeStartElement("spreadsheet");
    WRITE_COLUMN(xColumn, xColumn);
    WRITE_COLUMN(yColumn, yColumn);
    WRITE_COLUMN(zColumn, zColumn);

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
