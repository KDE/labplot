#include "Bar3DPlot.h"
#include "Bar3DPlotPrivate.h"
#include "backend/core/AbstractColumn.h"

#include <KConfig>
#include <QtGraphs/QBar3DSeries>

Bar3DPlot::Bar3DPlot(const QString& name)
	: WorksheetElement(name, new Bar3DPlotPrivate(this), AspectType::Bar3DPlot) {
	Q_D(Bar3DPlot);
	d->series = new QBar3DSeries();
}

Bar3DPlot::~Bar3DPlot() {
	Q_D(Bar3DPlot);
	delete d->series;
}

Bar3DPlotPrivate::Bar3DPlotPrivate(Bar3DPlot* plot)
	: WorksheetElementPrivate(plot)
	, q(plot) {
}

QBar3DSeries* Bar3DPlot::series() const {
	Q_D(const Bar3DPlot);
	return d->series;
}

// Accessor implementations
BASIC_D_READER_IMPL(Bar3DPlot, QColor, color, color)

void Bar3DPlot::setColor(QColor color) {
	Q_D(Bar3DPlot);
	if (d->color != color) {
		d->color = color;
		Q_EMIT colorChanged(color);
	}
}

QVector<const AbstractColumn*> Bar3DPlot::dataColumns() const {
	Q_D(const Bar3DPlot);
	return d->dataColumns;
}

void Bar3DPlot::setDataColumns(const QVector<const AbstractColumn*>& columns) {
	Q_D(Bar3DPlot);
	if (d->dataColumns != columns) {
		d->dataColumns = columns;
		Q_EMIT dataColumnsChanged(columns);
	}
}

void Bar3DPlot::loadThemeConfig(const KConfig& config) {
	Q_UNUSED(config)
}

void Bar3DPlot::saveThemeConfig(const KConfig& config) {
	Q_UNUSED(config)
}

void Bar3DPlot::handleResize(double horizontalRatio, double verticalRatio, bool pageResize) {
	Q_UNUSED(horizontalRatio)
	Q_UNUSED(verticalRatio)
	Q_UNUSED(pageResize)
}

void Bar3DPlot::retransform() {
}
