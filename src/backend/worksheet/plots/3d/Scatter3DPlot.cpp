#include "Scatter3DPlot.h"
#include "Scatter3DPlotPrivate.h"
#include "backend/core/AbstractColumn.h"

#include <QtGraphs/QScatter3DSeries>

Scatter3DPlot::Scatter3DPlot(const QString& name)
	: WorksheetElement(name, new Scatter3DPlotPrivate(this), AspectType::Scatter3DPlot) {
	Q_D(Scatter3DPlot);
	d->series = new QScatter3DSeries();
}

Scatter3DPlot::~Scatter3DPlot() {
	Q_D(Scatter3DPlot);
	delete d->series;
}

Scatter3DPlotPrivate::Scatter3DPlotPrivate(Scatter3DPlot* plot)
	: WorksheetElementPrivate(plot)
	, q(plot) {
}

QScatter3DSeries* Scatter3DPlot::series() const {
	Q_D(const Scatter3DPlot);
	return d->series;
}

// Accessor implementations
BASIC_D_READER_IMPL(Scatter3DPlot, Scatter3DPlot::PointStyle, pointStyle, pointStyle)
BASIC_D_READER_IMPL(Scatter3DPlot, QColor, color, color)

const AbstractColumn* Scatter3DPlot::xColumn() const {
	Q_D(const Scatter3DPlot);
	return d->xColumn;
}

const AbstractColumn* Scatter3DPlot::yColumn() const {
	Q_D(const Scatter3DPlot);
	return d->yColumn;
}

const AbstractColumn* Scatter3DPlot::zColumn() const {
	Q_D(const Scatter3DPlot);
	return d->zColumn;
}

void Scatter3DPlot::setPointStyle(PointStyle style) {
	Q_D(Scatter3DPlot);
	if (d->pointStyle != style) {
		d->pointStyle = style;
		Q_EMIT pointStyleChanged(style);
	}
}

void Scatter3DPlot::setColor(QColor color) {
	Q_D(Scatter3DPlot);
	if (d->color != color) {
		d->color = color;
		Q_EMIT colorChanged(color);
	}
}

void Scatter3DPlot::setXColumn(const AbstractColumn* column) {
	Q_D(Scatter3DPlot);
	if (d->xColumn != column) {
		d->xColumn = column;
		Q_EMIT xColumnChanged(column);
	}
}

void Scatter3DPlot::setYColumn(const AbstractColumn* column) {
	Q_D(Scatter3DPlot);
	if (d->yColumn != column) {
		d->yColumn = column;
		Q_EMIT yColumnChanged(column);
	}
}

void Scatter3DPlot::setZColumn(const AbstractColumn* column) {
	Q_D(Scatter3DPlot);
	if (d->zColumn != column) {
		d->zColumn = column;
		Q_EMIT zColumnChanged(column);
	}
}
