/*
	File                 : RunChart.cpp
	Project              : LabPlot
	Description          : Run Chart
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "RunChart.h"
#include "RunChartPrivate.h"
#include "backend/core/column/Column.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/macrosCurve.h"
#include "backend/lib/trace.h"
extern "C" {
#include "backend/nsl/nsl_pcm.h"
}
#include "backend/worksheet/Background.h"
#include "backend/worksheet/Line.h"
#include "backend/worksheet/plots/cartesian/Symbol.h"

#include <QMenu>
#include <QPainter>

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

#include <gsl/gsl_statistics.h>

CURVE_COLUMN_CONNECT(RunChart, Data, data, recalc)

/*!
 * \class RunChart
 * \brief This class implements the run chart (or run sequency plot) - a visualization showing
 * the provided data points together with the median/average of the data, commonly used to identify trends
 * or changes in the observation.
 *
 * To define the reference and to compare with, either median or the average values can be used.
 * The visual properties of the plotted line for the reference value and for the actual data can be modified
 * independently of each other.
 *
 * \ingroup CartesianPlots
 */
RunChart::RunChart(const QString& name)
	: Plot(name, new RunChartPrivate(this), AspectType::RunChart) {
	init();
}

RunChart::RunChart(const QString& name, RunChartPrivate* dd)
	: Plot(name, dd, AspectType::RunChart) {
	init();
}

// no need to delete the d-pointer here - it inherits from QGraphicsItem
// and is deleted during the cleanup in QGraphicsScene
RunChart::~RunChart() = default;

void RunChart::init() {
	Q_D(RunChart);

	KConfig config;
	KConfigGroup group = config.group(QStringLiteral("RunChart"));

	d->centerMetric = static_cast<RunChart::CenterMetric>(group.readEntry(QStringLiteral("CenterMetric"), static_cast<int>(RunChart::CenterMetric::Median)));

	// curve and columns for the data points
	d->dataCurve = new XYCurve(QStringLiteral("data"));
	d->dataCurve->setName(name(), AbstractAspect::NameHandling::UniqueNotRequired);
	d->dataCurve->setHidden(true);
	d->dataCurve->graphicsItem()->setParentItem(d);
	d->dataCurve->line()->init(group);
	d->dataCurve->line()->setStyle(Qt::SolidLine);
	d->dataCurve->symbol()->setStyle(Symbol::Style::Circle);
	d->dataCurve->background()->setPosition(Background::Position::No);

	d->xColumn = new Column(QStringLiteral("x"), AbstractColumn::ColumnMode::Integer);
	d->xColumn->setHidden(true);
	d->xColumn->setUndoAware(false);
	addChildFast(d->xColumn);

	// curve and columns for the central line
	d->centerCurve = new XYCurve(QStringLiteral("center"));
	d->centerCurve->setName(name(), AbstractAspect::NameHandling::UniqueNotRequired);
	d->centerCurve->setHidden(true);
	d->centerCurve->graphicsItem()->setParentItem(d);
	d->centerCurve->line()->init(group);
	d->centerCurve->line()->setStyle(Qt::SolidLine);
	d->centerCurve->symbol()->setStyle(Symbol::Style::NoSymbols);
	d->centerCurve->background()->setPosition(Background::Position::No);

	d->xCenterColumn = new Column(QStringLiteral("xCenter"), AbstractColumn::ColumnMode::Integer);
	d->xCenterColumn->setHidden(true);
	d->xCenterColumn->setUndoAware(false);
	addChildFast(d->xCenterColumn);
	d->centerCurve->setXColumn(d->xCenterColumn);

	d->yCenterColumn = new Column(QStringLiteral("yCenter"));
	d->yCenterColumn->setHidden(true);
	d->yCenterColumn->setUndoAware(false);
	addChildFast(d->yCenterColumn);
	d->centerCurve->setYColumn(d->yCenterColumn);

	// synchronize the names of the internal XYCurves with the name of the current plot
	// so we have the same name shown on the undo stack
	connect(this, &AbstractAspect::aspectDescriptionChanged, this, &RunChart::renameInternalCurves);

	// propage the visual changes to the parent
	connect(d->centerCurve, &XYCurve::changed, this, &RunChart::changed);
	connect(d->dataCurve, &XYCurve::changed, this, &RunChart::changed);
}

void RunChart::finalizeAdd() {
	Q_D(RunChart);
	WorksheetElement::finalizeAdd();
	addChildFast(d->centerCurve);
	addChildFast(d->dataCurve);
}

void RunChart::renameInternalCurves() {
	Q_D(RunChart);
	d->dataCurve->setName(name(), AbstractAspect::NameHandling::UniqueNotRequired);
	d->centerCurve->setName(name(), AbstractAspect::NameHandling::UniqueNotRequired);
}

/*!
  Returns an icon to be used in the project explorer.
  */
QIcon RunChart::icon() const {
	// TODO: set the correct icon
	return QIcon::fromTheme(QStringLiteral("view-object-histogram-linear"));
}

void RunChart::handleResize(double /*horizontalRatio*/, double /*verticalRatio*/, bool /*pageResize*/) {
	// TODO
}

void RunChart::setVisible(bool on) {
	Q_D(RunChart);
	beginMacro(on ? i18n("%1: set visible", name()) : i18n("%1: set invisible", name()));
	d->dataCurve->setVisible(on);
	d->centerCurve->setVisible(on);
	WorksheetElement::setVisible(on);
	endMacro();
}

/*!
 * override the default implementation to handle the visibility of the internal curves
 * and to set the z-value of the data curve to 1 higher than the z-value of the center curve.
 */
void RunChart::setZValue(qreal value) {
	Q_D(RunChart);
	d->centerCurve->setZValue(value);
	d->dataCurve->setZValue(value + 1);
}

// ##############################################################################
// ##########################  getter methods  ##################################
// ##############################################################################
//  general
BASIC_SHARED_D_READER_IMPL(RunChart, RunChart::CenterMetric, centerMetric, centerMetric)
BASIC_SHARED_D_READER_IMPL(RunChart, const AbstractColumn*, dataColumn, dataColumn)
BASIC_SHARED_D_READER_IMPL(RunChart, QString, dataColumnPath, dataColumnPath)

/*!
 * returns the number of index values used for x.
 */
int RunChart::xIndexCount() const {
	Q_D(const RunChart);
	if (!d->dataColumn)
		return 0;

	return d->dataColumn->rowCount();
}

// lines
Line* RunChart::dataLine() const {
	Q_D(const RunChart);
	return d->dataCurve->line();
}

Line* RunChart::centerLine() const {
	Q_D(const RunChart);
	return d->centerCurve->line();
}

// symbols
Symbol* RunChart::dataSymbol() const {
	Q_D(const RunChart);
	return d->dataCurve->symbol();
}

bool RunChart::minMax(const Dimension dim, const Range<int>& indexRange, Range<double>& r, bool /* includeErrorBars */) const {
	Q_D(const RunChart);
	return d->dataCurve->minMax(dim, indexRange, r, false);
}

double RunChart::minimum(const Dimension dim) const {
	Q_D(const RunChart);
	return d->dataCurve->minimum(dim);
}

double RunChart::maximum(const Dimension dim) const {
	Q_D(const RunChart);
	return d->dataCurve->maximum(dim);
}

bool RunChart::hasData() const {
	Q_D(const RunChart);
	return (d->dataColumn != nullptr);
}

bool RunChart::usingColumn(const AbstractColumn* column, bool) const {
	Q_D(const RunChart);
	return (d->dataColumn == column);
}

void RunChart::handleAspectUpdated(const QString& aspectPath, const AbstractAspect* aspect) {
	Q_D(RunChart);

	const auto column = dynamic_cast<const AbstractColumn*>(aspect);
	if (!column)
		return;

	if (d->dataColumn == column) // the column is the same and was just renamed -> update the column path
		d->dataColumnPath = aspectPath;
	else if (d->dataColumnPath == aspectPath) { // another column was renamed to the current path -> set and connect to the new column
		setUndoAware(false);
		setDataColumn(column);
		setUndoAware(true);
	}
}

QColor RunChart::color() const {
	Q_D(const RunChart);
	return d->dataCurve->color();
}

double RunChart::center() const {
	Q_D(const RunChart);
	return d->center;
}

XYCurve* RunChart::dataCurve() const {
	Q_D(const RunChart);
	return d->dataCurve;
}

// ##############################################################################
// #################  setter methods and undo commands ##########################
// ##############################################################################

// General
CURVE_COLUMN_SETTER_CMD_IMPL_F_S(RunChart, Data, data, recalc)
void RunChart::setDataColumn(const AbstractColumn* column) {
	Q_D(RunChart);
	if (column != d->dataColumn)
		exec(new RunChartSetDataColumnCmd(d, column, ki18n("%1: set data column")));
}

void RunChart::setDataColumnPath(const QString& path) {
	Q_D(RunChart);
	d->dataColumnPath = path;
}

STD_SETTER_CMD_IMPL_F_S(RunChart, SetLimitsMetric, RunChart::CenterMetric, centerMetric, recalc)
void RunChart::setCenterMetric(RunChart::CenterMetric centerMetric) {
	Q_D(RunChart);
	if (centerMetric != d->centerMetric)
		exec(new RunChartSetLimitsMetricCmd(d, centerMetric, ki18n("%1: set center metric")));
}

// ##############################################################################
// #################################  SLOTS  ####################################
// ##############################################################################
void RunChart::retransform() {
	D(RunChart);
	d->retransform();
}

void RunChart::recalc() {
	D(RunChart);
	d->recalc();
}

void RunChart::dataColumnAboutToBeRemoved(const AbstractAspect* aspect) {
	Q_D(RunChart);
	if (aspect == d->dataColumn) {
		d->dataColumn = nullptr;
		d->recalc();
		Q_EMIT dataChanged();
		Q_EMIT changed();
	}
}

// ##############################################################################
// ######################### Private implementation #############################
// ##############################################################################
RunChartPrivate::RunChartPrivate(RunChart* owner)
	: PlotPrivate(owner)
	, q(owner) {
	setFlag(QGraphicsItem::ItemIsSelectable, true);
	setAcceptHoverEvents(false);
}

RunChartPrivate::~RunChartPrivate() {
}

/*!
  called when the size of the plot or its data ranges (manual changes, zooming, etc.) were changed.
  recalculates the position of the scene points to be drawn.
  triggers the update of lines, drop lines, symbols etc.
*/
void RunChartPrivate::retransform() {
	const bool suppressed = suppressRetransform || q->isLoading();
	if (suppressed)
		return;

	if (!isVisible())
		return;

	PERFTRACE(name() + QLatin1String(Q_FUNC_INFO));
	dataCurve->retransform();
	centerCurve->retransform();
	recalcShapeAndBoundingRect();
}

/*!
 * called when the source data was changed, recalculates the plot.
 */
void RunChartPrivate::recalc() {
	PERFTRACE(name() + QLatin1String(Q_FUNC_INFO));
	if (!dataColumn) {
		center = 0.;
		xColumn->clear();
		xCenterColumn->clear();
		yCenterColumn->clear();
		Q_EMIT q->dataChanged();
		return;
	}

	// supress retransforms in all internal curves while modifying the data,
	// everything will be retransformend at the very end
	dataCurve->setSuppressRetransform(true);
	centerCurve->setSuppressRetransform(true);

	const int count = q->xIndexCount();
	const int xMin = 1;
	const int xMax = count;
	xColumn->clear();
	xColumn->resizeTo(count);
	for (int i = 0; i < count; ++i)
		xColumn->setIntegerAt(i, i + 1);

	dataCurve->setXColumn(xColumn);
	dataCurve->setYColumn(dataColumn);

	// min and max values for x
	xCenterColumn->setIntegerAt(0, xMin);
	xCenterColumn->setIntegerAt(1, xMax);

	// y value for the center line
	if (centerMetric == RunChart::CenterMetric::Median)
		center = static_cast<const Column*>(dataColumn)->statistics().median;
	else
		center = static_cast<const Column*>(dataColumn)->statistics().arithmeticMean;

	yCenterColumn->setValueAt(0, center);
	yCenterColumn->setValueAt(1, center);

	dataCurve->setSuppressRetransform(false);
	centerCurve->setSuppressRetransform(false);

	// emit dataChanged() in order to retransform everything with the new size/shape of the plot
	Q_EMIT q->dataChanged();
}

/*!
  recalculates the outer bounds and the shape of the curve.
  */
void RunChartPrivate::recalcShapeAndBoundingRect() {
	if (suppressRecalc)
		return;

	prepareGeometryChange();
	m_shape = QPainterPath();
	m_shape.addPath(dataCurve->graphicsItem()->shape());
	m_shape.addPath(centerCurve->graphicsItem()->shape());

	m_boundingRectangle = m_shape.boundingRect();
}

// ##############################################################################
// ##################  Serialization/Deserialization  ###########################
// ##############################################################################
//! Save as XML
void RunChart::save(QXmlStreamWriter* writer) const {
	Q_D(const RunChart);

	writer->writeStartElement(QStringLiteral("RunChart"));
	writeBasicAttributes(writer);
	writeCommentElement(writer);

	// general
	writer->writeStartElement(QStringLiteral("general"));
	WRITE_COLUMN(d->dataColumn, dataColumn);
	WRITE_COLUMN(d->xColumn, xColumn);
	WRITE_COLUMN(d->xCenterColumn, xCenterColumn);
	WRITE_COLUMN(d->yCenterColumn, yCenterColumn);
	writer->writeAttribute(QStringLiteral("centerMetric"), QString::number(static_cast<int>(d->centerMetric)));
	writer->writeAttribute(QStringLiteral("visible"), QString::number(d->isVisible()));
	writer->writeAttribute(QStringLiteral("legendVisible"), QString::number(d->legendVisible));
	writer->writeEndElement();

	// save the internal columns, above only the references to them were saved
	d->xColumn->save(writer);
	d->xCenterColumn->save(writer);
	d->yCenterColumn->save(writer);

	// save the internal curves
	// disconnect temporarily from renameInternalCurves so we can use unique names to be able to properly load the curves later
	disconnect(this, &AbstractAspect::aspectDescriptionChanged, this, &RunChart::renameInternalCurves);
	d->dataCurve->setName(QStringLiteral("data"));
	d->dataCurve->save(writer);
	d->centerCurve->setName(QStringLiteral("center"));
	d->centerCurve->save(writer);
	connect(this, &AbstractAspect::aspectDescriptionChanged, this, &RunChart::renameInternalCurves);

	writer->writeEndElement(); // close "RunChart" section
}

//! Load from XML
bool RunChart::load(XmlStreamReader* reader, bool preview) {
	Q_D(RunChart);

	if (!readBasicAttributes(reader))
		return false;

	QXmlStreamAttributes attribs;
	QString str;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == QLatin1String("RunChart"))
			break;

		if (!reader->isStartElement())
			continue;

		if (reader->name() == QLatin1String("comment")) {
			if (!readCommentElement(reader))
				return false;
		} else if (!preview && reader->name() == QLatin1String("general")) {
			attribs = reader->attributes();
			READ_COLUMN(dataColumn);
			READ_COLUMN(xColumn);
			READ_COLUMN(xCenterColumn);
			READ_COLUMN(yCenterColumn);
			READ_INT_VALUE("centerMetric", centerMetric, RunChart::CenterMetric);

			str = attribs.value(QStringLiteral("visible")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("visible"));
			else
				d->setVisible(str.toInt());
		} else if (reader->name() == QLatin1String("column")) {
			attribs = reader->attributes();
			bool rc = false;
			const auto& name = attribs.value(QStringLiteral("name"));
			if (name == QLatin1String("x"))
				rc = d->xColumn->load(reader, preview);
			else if (name == QLatin1String("xCenter"))
				rc = d->xCenterColumn->load(reader, preview);
			else if (name == QLatin1String("yCenter"))
				rc = d->yCenterColumn->load(reader, preview);

			if (!rc)
				return false;
		} else if (reader->name() == QLatin1String("xyCurve")) {
			attribs = reader->attributes();
			bool rc = false;
			if (attribs.value(QStringLiteral("name")) == QLatin1String("data"))
				rc = d->dataCurve->load(reader, preview);
			else if (attribs.value(QStringLiteral("name")) == QLatin1String("center"))
				rc = d->centerCurve->load(reader, preview);

			if (!rc)
				return false;
		} else { // unknown element
			reader->raiseUnknownElementWarning();
			if (!reader->skipToEndElement())
				return false;
		}
	}
	return true;
}

// ##############################################################################
// #########################  Theme management ##################################
// ##############################################################################
void RunChart::loadThemeConfig(const KConfig& config) {
	KConfigGroup group;
	if (config.hasGroup(QStringLiteral("Theme")))
		group = config.group(QStringLiteral("XYCurve")); // when loading from the theme config, use the same properties as for XYCurve
	else
		group = config.group(QStringLiteral("RunChart"));

	Q_D(RunChart);
	const auto* plot = d->m_plot;
	int index = plot->curveChildIndex(this);
	QColor themeColor = plot->themeColorPalette(index);

	d->suppressRecalc = true;

	d->dataCurve->line()->loadThemeConfig(group, themeColor);
	d->dataCurve->symbol()->loadThemeConfig(group, themeColor);

	themeColor = plot->themeColorPalette(index + 1);

	d->centerCurve->line()->loadThemeConfig(group, themeColor);
	d->centerCurve->symbol()->setStyle(Symbol::Style::NoSymbols);

	d->suppressRecalc = false;
	d->recalcShapeAndBoundingRect();
}

void RunChart::saveThemeConfig(const KConfig& config) {
	Q_D(const RunChart);
	KConfigGroup group = config.group(QStringLiteral("RunChart"));
	d->dataCurve->line()->saveThemeConfig(group);
	d->dataCurve->symbol()->saveThemeConfig(group);
	d->centerCurve->line()->saveThemeConfig(group);
}
