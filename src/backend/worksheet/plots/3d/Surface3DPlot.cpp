#include "Surface3DPlot.h"
#include "Surface3DPlotPrivate.h"
#include "backend/core/AbstractColumn.h"
#include "backend/matrix/Matrix.h"

#include <QtGraphs/QSurface3DSeries>

Surface3DPlot::Surface3DPlot(const QString& name)
	: WorksheetElement(name, new Surface3DPlotPrivate(this), AspectType::Surface3DPlot) {
	Q_D(Surface3DPlot);
	d->series = new QSurface3DSeries();
}

Surface3DPlot::~Surface3DPlot() {
	Q_D(Surface3DPlot);
	delete d->series;
}

Surface3DPlotPrivate::Surface3DPlotPrivate(Surface3DPlot* plot)
	: WorksheetElementPrivate(plot)
	, q(plot) {
}

QSurface3DSeries* Surface3DPlot::series() const {
	Q_D(const Surface3DPlot);
	return d->series;
}

// Accessor implementations
BASIC_D_READER_IMPL(Surface3DPlot, Surface3DPlot::DrawMode, drawMode, drawMode)
BASIC_D_READER_IMPL(Surface3DPlot, Surface3DPlot::DataSource, dataSource, dataSource)
BASIC_D_READER_IMPL(Surface3DPlot, bool, flatShading, flatShading)
BASIC_D_READER_IMPL(Surface3DPlot, bool, smooth, smooth)
BASIC_D_READER_IMPL(Surface3DPlot, QColor, color, color)

const AbstractColumn* Surface3DPlot::xColumn() const {
	Q_D(const Surface3DPlot);
	return d->xColumn;
}

const AbstractColumn* Surface3DPlot::yColumn() const {
	Q_D(const Surface3DPlot);
	return d->yColumn;
}

const AbstractColumn* Surface3DPlot::zColumn() const {
	Q_D(const Surface3DPlot);
	return d->zColumn;
}

const Matrix* Surface3DPlot::matrix() const {
	Q_D(const Surface3DPlot);
	return d->matrix;
}

void Surface3DPlot::setDrawMode(DrawMode mode) {
	Q_D(Surface3DPlot);
	if (d->drawMode != mode) {
		d->drawMode = mode;
		Q_EMIT drawModeChanged(mode);
	}
}

void Surface3DPlot::setDataSource(DataSource source) {
	Q_D(Surface3DPlot);
	if (d->dataSource != source) {
		d->dataSource = source;
		Q_EMIT sourceTypeChanged(source);
	}
}

void Surface3DPlot::setFlatShading(bool enable) {
	Q_D(Surface3DPlot);
	if (d->flatShading != enable) {
		d->flatShading = enable;
		Q_EMIT flatShadingChanged(enable);
	}
}

void Surface3DPlot::setSmooth(bool enable) {
	Q_D(Surface3DPlot);
	if (d->smooth != enable) {
		d->smooth = enable;
		Q_EMIT smoothChanged(enable);
	}
}

void Surface3DPlot::setColor(QColor color) {
	Q_D(Surface3DPlot);
	if (d->color != color) {
		d->color = color;
		Q_EMIT colorChanged(color);
	}
}

void Surface3DPlot::setXColumn(const AbstractColumn* column) {
	Q_D(Surface3DPlot);
	if (d->xColumn != column) {
		d->xColumn = column;
		Q_EMIT xColumnChanged(column);
	}
}

void Surface3DPlot::setYColumn(const AbstractColumn* column) {
	Q_D(Surface3DPlot);
	if (d->yColumn != column) {
		d->yColumn = column;
		Q_EMIT yColumnChanged(column);
	}
}

void Surface3DPlot::setZColumn(const AbstractColumn* column) {
	Q_D(Surface3DPlot);
	if (d->zColumn != column) {
		d->zColumn = column;
		Q_EMIT zColumnChanged(column);
	}
}

void Surface3DPlot::setMatrix(const Matrix* matrix) {
	Q_D(Surface3DPlot);
	if (d->matrix != matrix) {
		d->matrix = matrix;
		Q_EMIT matrixChanged(matrix);
	}
}
