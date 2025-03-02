/*
	File                 : ProcessBehaviorChart.cpp
	Project              : LabPlot
	Description          : ProcessBehaviorChart
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024-2025 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ProcessBehaviorChart.h"
#include "ProcessBehaviorChartPrivate.h"
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
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/worksheet/plots/cartesian/Symbol.h"

#include <QMenu>
#include <QPainter>

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

#include <gsl/gsl_statistics.h>

CURVE_COLUMN_CONNECT(ProcessBehaviorChart, Data, data, recalc)
CURVE_COLUMN_CONNECT(ProcessBehaviorChart, Data2, data2, recalc)

/*!
 * \class ProcessBehaviorChart
 * \brief This class implements the process behavior chart.
 *
 * The sub-types XmR, mR, XbarR, R, XbarS, S, P, NP, C, U are implemented and the implementation follows
 * the conventions used in the book "Making Sense of Data", Donald J. Wheeler.
 * The visual properties of the plotted line for the controll limits and for the actual data can be modified
 * independently of each other.
 *
 * \ingroup CartesianPlots
 */
ProcessBehaviorChart::ProcessBehaviorChart(const QString& name)
	: Plot(name, new ProcessBehaviorChartPrivate(this), AspectType::ProcessBehaviorChart) {
	init();
}

ProcessBehaviorChart::ProcessBehaviorChart(const QString& name, ProcessBehaviorChartPrivate* dd)
	: Plot(name, dd, AspectType::ProcessBehaviorChart) {
	init();
}

// no need to delete the d-pointer here - it inherits from QGraphicsItem
// and is deleted during the cleanup in QGraphicsScene
ProcessBehaviorChart::~ProcessBehaviorChart() = default;

void ProcessBehaviorChart::init() {
	Q_D(ProcessBehaviorChart);

	KConfig config;
	KConfigGroup group = config.group(QStringLiteral("ProcessBehaviorChart"));

	// general properties
	d->type = static_cast<ProcessBehaviorChart::Type>(group.readEntry(QStringLiteral("Type"), static_cast<int>(ProcessBehaviorChart::Type::XmR)));
	d->sampleSize = group.readEntry(QStringLiteral("SampleSize"), 5);
	d->limitsMetric = static_cast<ProcessBehaviorChart::LimitsMetric>(
		group.readEntry(QStringLiteral("LimitsMetric"), static_cast<int>(ProcessBehaviorChart::LimitsMetric::Average)));
	d->negativeLowerLimitEnabled = group.readEntry(QStringLiteral("NegativeLowerLimitEnabled"), false);
	d->exactLimitsEnabled = group.readEntry(QStringLiteral("ExactLimitsEnabled"), true);

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

	d->yColumn = new Column(QStringLiteral("y"));
	d->yColumn->setHidden(true);
	d->yColumn->setUndoAware(false);
	addChildFast(d->yColumn);

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

	// curve and columns for the upper and lower limit lines
	d->upperLimitCurve = new XYCurve(QStringLiteral("upperLimit"));
	d->upperLimitCurve->setName(name(), AbstractAspect::NameHandling::UniqueNotRequired);
	d->upperLimitCurve->setHidden(true);
	d->upperLimitCurve->graphicsItem()->setParentItem(d);
	d->upperLimitCurve->line()->init(group);
	d->upperLimitCurve->line()->setStyle(Qt::SolidLine);
	d->upperLimitCurve->symbol()->setStyle(Symbol::Style::NoSymbols);
	d->upperLimitCurve->background()->setPosition(Background::Position::No);
	d->upperLimitCurve->setLineType(XYCurve::LineType::MidpointHorizontal); // required for stair-step lines for P and U charts

	d->xUpperLimitColumn = new Column(QStringLiteral("xUpperLimit"), AbstractColumn::ColumnMode::Integer);
	d->xUpperLimitColumn->setHidden(true);
	d->xUpperLimitColumn->setUndoAware(false);
	addChildFast(d->xUpperLimitColumn);
	d->upperLimitCurve->setXColumn(d->xUpperLimitColumn);

	d->yUpperLimitColumn = new Column(QStringLiteral("yUpperLimit"));
	d->yUpperLimitColumn->setHidden(true);
	d->yUpperLimitColumn->setUndoAware(false);
	addChildFast(d->yUpperLimitColumn);
	d->upperLimitCurve->setYColumn(d->yUpperLimitColumn);

	d->lowerLimitCurve = new XYCurve(QStringLiteral("lowerLimit"));
	d->lowerLimitCurve->setName(name(), AbstractAspect::NameHandling::UniqueNotRequired);
	d->lowerLimitCurve->setHidden(true);
	d->lowerLimitCurve->graphicsItem()->setParentItem(d);
	d->lowerLimitCurve->line()->init(group);
	d->lowerLimitCurve->line()->setStyle(Qt::SolidLine);
	d->lowerLimitCurve->symbol()->setStyle(Symbol::Style::NoSymbols);
	d->lowerLimitCurve->background()->setPosition(Background::Position::No);
	d->lowerLimitCurve->setLineType(XYCurve::LineType::MidpointHorizontal);

	d->xLowerLimitColumn = new Column(QStringLiteral("xLowerLimit"), AbstractColumn::ColumnMode::Integer);
	d->xLowerLimitColumn->setHidden(true);
	d->xLowerLimitColumn->setUndoAware(false);
	addChildFast(d->xLowerLimitColumn);
	d->lowerLimitCurve->setXColumn(d->xLowerLimitColumn);

	d->yLowerLimitColumn = new Column(QStringLiteral("yLowerLimit"));
	d->yLowerLimitColumn->setHidden(true);
	d->yLowerLimitColumn->setUndoAware(false);
	addChildFast(d->yLowerLimitColumn);
	d->lowerLimitCurve->setYColumn(d->yLowerLimitColumn);

	// text labels for the values
	d->valuesEnabled = group.readEntry(QStringLiteral("ValuesEnabled"), true);

	d->upperLimitValueLabel = new TextLabel(QStringLiteral("upper"));
	d->upperLimitValueLabel->setHidden(true);
	d->upperLimitValueLabel->setBorderShape(TextLabel::BorderShape::LeftPointingRectangle);
	d->upperLimitValueLabel->setHorizontalAlignment(WorksheetElement::HorizontalAlignment::Left);

	d->centerValueLabel = new TextLabel(QStringLiteral("center"));
	d->centerValueLabel->setHidden(true);
	d->centerValueLabel->setBorderShape(TextLabel::BorderShape::LeftPointingRectangle);
	d->centerValueLabel->setHorizontalAlignment(WorksheetElement::HorizontalAlignment::Left);

	d->lowerLimitValueLabel = new TextLabel(QStringLiteral("lower"));
	d->lowerLimitValueLabel->setHidden(true);
	d->lowerLimitValueLabel->setBorderShape(TextLabel::BorderShape::LeftPointingRectangle);
	d->lowerLimitValueLabel->setHorizontalAlignment(WorksheetElement::HorizontalAlignment::Left);

	// synchronize the names of the internal XYCurves with the name of the current plot
	// so we have the same name shown on the undo stack
	connect(this, &AbstractAspect::aspectDescriptionChanged, this, &ProcessBehaviorChart::renameInternalCurves);

	// propagate the visual changes to the parent
	connect(d->centerCurve, &XYCurve::changed, this, &ProcessBehaviorChart::changed);
	connect(d->dataCurve, &XYCurve::changed, this, &ProcessBehaviorChart::changed);
	connect(d->upperLimitCurve, &XYCurve::changed, this, &ProcessBehaviorChart::changed);
	connect(d->lowerLimitCurve, &XYCurve::changed, this, &ProcessBehaviorChart::changed);
}

void ProcessBehaviorChart::finalizeAdd() {
	Q_D(ProcessBehaviorChart);
	WorksheetElement::finalizeAdd();

	// curves
	addChildFast(d->centerCurve);
	addChildFast(d->upperLimitCurve);
	addChildFast(d->lowerLimitCurve);
	addChildFast(d->dataCurve);

	// labels
	addChildFast(d->upperLimitValueLabel);
	d->upperLimitValueLabel->setCoordinateBindingEnabled(true);
	d->upperLimitValueLabel->setParentGraphicsItem(graphicsItem());
	d->upperLimitValueLabel->graphicsItem()->setFlag(QGraphicsItem::ItemIsMovable, false);

	addChildFast(d->centerValueLabel);
	d->centerValueLabel->setCoordinateBindingEnabled(true);
	d->centerValueLabel->setParentGraphicsItem(graphicsItem());
	d->centerValueLabel->graphicsItem()->setFlag(QGraphicsItem::ItemIsMovable, false);

	addChildFast(d->lowerLimitValueLabel);
	d->lowerLimitValueLabel->setCoordinateBindingEnabled(true);
	d->lowerLimitValueLabel->setParentGraphicsItem(graphicsItem());
	d->lowerLimitValueLabel->graphicsItem()->setFlag(QGraphicsItem::ItemIsMovable, false);
}

void ProcessBehaviorChart::renameInternalCurves() {
	Q_D(ProcessBehaviorChart);
	d->dataCurve->setName(name(), AbstractAspect::NameHandling::UniqueNotRequired);
	d->centerCurve->setName(name(), AbstractAspect::NameHandling::UniqueNotRequired);
	d->upperLimitCurve->setName(name(), AbstractAspect::NameHandling::UniqueNotRequired);
	d->lowerLimitCurve->setName(name(), AbstractAspect::NameHandling::UniqueNotRequired);
}

/*!
  Returns an icon to be used in the project explorer.
  */
QIcon ProcessBehaviorChart::icon() const {
	// TODO: set the correct icon
	return QIcon::fromTheme(QStringLiteral("view-object-histogram-linear"));
}

void ProcessBehaviorChart::handleResize(double /*horizontalRatio*/, double /*verticalRatio*/, bool /*pageResize*/) {
	// TODO
}

void ProcessBehaviorChart::setVisible(bool on) {
	Q_D(ProcessBehaviorChart);
	beginMacro(on ? i18n("%1: set visible", name()) : i18n("%1: set invisible", name()));
	d->dataCurve->setVisible(on);
	d->centerCurve->setVisible(on);
	d->upperLimitCurve->setVisible(on);
	d->lowerLimitCurve->setVisible(on);
	WorksheetElement::setVisible(on);
	endMacro();
}

/*!
 * override the default implementation to handle the visibility of the internal curves
 * and to set the z-value of the data curve to 1 higher than the z-value of the other curves.
 */
void ProcessBehaviorChart::setZValue(qreal value) {
	Q_D(ProcessBehaviorChart);
	d->centerCurve->setZValue(value);
	d->upperLimitCurve->setZValue(value);
	d->lowerLimitCurve->setZValue(value);
	d->dataCurve->setZValue(value + 1);
}

// ##############################################################################
// ##########################  getter methods  ##################################
// ##############################################################################
//  general
BASIC_SHARED_D_READER_IMPL(ProcessBehaviorChart, ProcessBehaviorChart::Type, type, type)
BASIC_SHARED_D_READER_IMPL(ProcessBehaviorChart, ProcessBehaviorChart::LimitsMetric, limitsMetric, limitsMetric)
BASIC_SHARED_D_READER_IMPL(ProcessBehaviorChart, int, sampleSize, sampleSize)
BASIC_SHARED_D_READER_IMPL(ProcessBehaviorChart, bool, negativeLowerLimitEnabled, negativeLowerLimitEnabled)
BASIC_SHARED_D_READER_IMPL(ProcessBehaviorChart, bool, exactLimitsEnabled, exactLimitsEnabled)
BASIC_SHARED_D_READER_IMPL(ProcessBehaviorChart, const AbstractColumn*, dataColumn, dataColumn)
BASIC_SHARED_D_READER_IMPL(ProcessBehaviorChart, QString, dataColumnPath, dataColumnPath)
BASIC_SHARED_D_READER_IMPL(ProcessBehaviorChart, const AbstractColumn*, data2Column, data2Column)
BASIC_SHARED_D_READER_IMPL(ProcessBehaviorChart, QString, data2ColumnPath, data2ColumnPath)

// values
BASIC_SHARED_D_READER_IMPL(ProcessBehaviorChart, bool, valuesEnabled, valuesEnabled)

QColor ProcessBehaviorChart::valuesFontColor() const {
	Q_D(const ProcessBehaviorChart);
	return d->centerValueLabel->fontColor();
}

QColor ProcessBehaviorChart::valuesBackgroundColor() const {
	Q_D(const ProcessBehaviorChart);
	return d->centerValueLabel->backgroundColor();
}

QFont ProcessBehaviorChart::valuesFont() const {
	Q_D(const ProcessBehaviorChart);
	return d->centerValueLabel->teXFont();
}

TextLabel::BorderShape ProcessBehaviorChart::valuesBorderShape() const {
	Q_D(const ProcessBehaviorChart);
	return d->centerValueLabel->borderShape();
}

QPen ProcessBehaviorChart::valuesBorderPen() const {
	Q_D(const ProcessBehaviorChart);
	return d->centerValueLabel->borderPen();
}

qreal ProcessBehaviorChart::valuesBorderOpacity() const {
	Q_D(const ProcessBehaviorChart);
	return d->centerValueLabel->borderOpacity();
}

/*!
 * returns the number of index values used for x.
 */
int ProcessBehaviorChart::xIndexCount() const {
	Q_D(const ProcessBehaviorChart);
	if (!d->dataColumn)
		return 0;

	int count = d->dataColumn->rowCount();
	// subract the remainder to handle complete samples only for chart types where one point per sample is plotted
	if (d->type == ProcessBehaviorChart::Type::XbarR || d->type == ProcessBehaviorChart::Type::R || d->type == ProcessBehaviorChart::Type::XbarS
		|| d->type == ProcessBehaviorChart::Type::S) {
		const int remainder = count % d->sampleSize;
		if (remainder > 0)
			count -= remainder;

		count = count / d->sampleSize;
	}

	return count;
}

// lines
Line* ProcessBehaviorChart::dataLine() const {
	Q_D(const ProcessBehaviorChart);
	return d->dataCurve->line();
}

Line* ProcessBehaviorChart::centerLine() const {
	Q_D(const ProcessBehaviorChart);
	return d->centerCurve->line();
}

Line* ProcessBehaviorChart::upperLimitLine() const {
	Q_D(const ProcessBehaviorChart);
	return d->upperLimitCurve->line();
}

Line* ProcessBehaviorChart::lowerLimitLine() const {
	Q_D(const ProcessBehaviorChart);
	return d->lowerLimitCurve->line();
}

// symbols
Symbol* ProcessBehaviorChart::dataSymbol() const {
	Q_D(const ProcessBehaviorChart);
	return d->dataCurve->symbol();
}

bool ProcessBehaviorChart::minMax(const Dimension dim, const Range<int>& indexRange, Range<double>& r, bool /* includeErrorBars */) const {
	Q_D(const ProcessBehaviorChart);

	switch (dim) {
	case Dimension::X:
		return d->dataCurve->minMax(dim, indexRange, r, false);
	case Dimension::Y: {
		Range upperLimitRange(r);
		bool rc = d->upperLimitCurve->minMax(dim, indexRange, upperLimitRange, false);
		if (!rc)
			return false;

		Range lowerLimitRange(r);
		rc = d->lowerLimitCurve->minMax(dim, indexRange, lowerLimitRange, false);
		if (!rc)
			return false;

		Range dataRange(r);
		rc = d->dataCurve->minMax(dim, indexRange, dataRange, false);
		if (!rc)
			return false;

		r.setStart(std::min(dataRange.start(), lowerLimitRange.start()));
		r.setEnd(std::max(dataRange.end(), upperLimitRange.end()));

		return true;
	}
	}
	return false;
}

double ProcessBehaviorChart::minimum(const Dimension dim) const {
	Q_D(const ProcessBehaviorChart);
	switch (dim) {
	case Dimension::X:
		return d->dataCurve->minimum(dim);
	case Dimension::Y:
		return d->lowerLimitCurve->minimum(dim);
	}
	return NAN;
}

double ProcessBehaviorChart::maximum(const Dimension dim) const {
	Q_D(const ProcessBehaviorChart);
	switch (dim) {
	case Dimension::X:
		return d->dataCurve->maximum(dim);
	case Dimension::Y:
		return std::max(d->dataCurve->maximum(dim), d->upperLimitCurve->maximum(dim));
	}
	return NAN;
}

bool ProcessBehaviorChart::hasData() const {
	Q_D(const ProcessBehaviorChart);
	return (d->dataColumn != nullptr);
}

bool ProcessBehaviorChart::usingColumn(const AbstractColumn* column, bool) const {
	Q_D(const ProcessBehaviorChart);
	return (d->dataColumn == column || d->data2Column == column);
}

void ProcessBehaviorChart::handleAspectUpdated(const QString& aspectPath, const AbstractAspect* aspect) {
	Q_D(ProcessBehaviorChart);

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

QColor ProcessBehaviorChart::color() const {
	Q_D(const ProcessBehaviorChart);
	return d->dataCurve->color();
}

double ProcessBehaviorChart::center() const {
	Q_D(const ProcessBehaviorChart);
	return d->center;
}

double ProcessBehaviorChart::upperLimit() const {
	Q_D(const ProcessBehaviorChart);
	return d->upperLimit;
}

double ProcessBehaviorChart::lowerLimit() const {
	Q_D(const ProcessBehaviorChart);
	return d->lowerLimit;
}

XYCurve* ProcessBehaviorChart::dataCurve() const {
	Q_D(const ProcessBehaviorChart);
	return d->dataCurve;
}

bool ProcessBehaviorChart::lowerLimitAvailable() const {
	Q_D(const ProcessBehaviorChart);
	return d->lowerLimitCurve->isVisible();
}

// ##############################################################################
// #################  setter methods and undo commands ##########################
// ##############################################################################

// General
CURVE_COLUMN_SETTER_CMD_IMPL_F_S(ProcessBehaviorChart, Data, data, recalc)
void ProcessBehaviorChart::setDataColumn(const AbstractColumn* column) {
	Q_D(ProcessBehaviorChart);
	if (column != d->dataColumn)
		exec(new ProcessBehaviorChartSetDataColumnCmd(d, column, ki18n("%1: set data column")));
}

void ProcessBehaviorChart::setDataColumnPath(const QString& path) {
	Q_D(ProcessBehaviorChart);
	d->dataColumnPath = path;
}

CURVE_COLUMN_SETTER_CMD_IMPL_F_S(ProcessBehaviorChart, Data2, data2, recalc)
void ProcessBehaviorChart::setData2Column(const AbstractColumn* column) {
	Q_D(ProcessBehaviorChart);
	if (column != d->data2Column)
		exec(new ProcessBehaviorChartSetData2ColumnCmd(d, column, ki18n("%1: set data column")));
}

void ProcessBehaviorChart::setData2ColumnPath(const QString& path) {
	Q_D(ProcessBehaviorChart);
	d->data2ColumnPath = path;
}

STD_SETTER_CMD_IMPL_F_S(ProcessBehaviorChart, SetType, ProcessBehaviorChart::Type, type, recalc)
void ProcessBehaviorChart::setType(ProcessBehaviorChart::Type type) {
	Q_D(ProcessBehaviorChart);
	if (type != d->type)
		exec(new ProcessBehaviorChartSetTypeCmd(d, type, ki18n("%1: set type")));
}

STD_SETTER_CMD_IMPL_F_S(ProcessBehaviorChart, SetLimitsMetric, ProcessBehaviorChart::LimitsMetric, limitsMetric, recalc)
void ProcessBehaviorChart::setLimitsMetric(ProcessBehaviorChart::LimitsMetric limitsMetric) {
	Q_D(ProcessBehaviorChart);
	if (limitsMetric != d->limitsMetric)
		exec(new ProcessBehaviorChartSetLimitsMetricCmd(d, limitsMetric, ki18n("%1: set limits metric")));
}

STD_SETTER_CMD_IMPL_F_S(ProcessBehaviorChart, SetSampleSize, int, sampleSize, recalc)
void ProcessBehaviorChart::setSampleSize(int sampleSize) {
	Q_D(ProcessBehaviorChart);
	if (sampleSize != d->sampleSize)
		exec(new ProcessBehaviorChartSetSampleSizeCmd(d, sampleSize, ki18n("%1: set sample size")));
}

STD_SETTER_CMD_IMPL_F_S(ProcessBehaviorChart, SetNegativeLowerLimitEnabled, bool, negativeLowerLimitEnabled, recalc)
void ProcessBehaviorChart::setNegativeLowerLimitEnabled(bool enabled) {
	Q_D(ProcessBehaviorChart);
	if (enabled != d->negativeLowerLimitEnabled)
		exec(new ProcessBehaviorChartSetNegativeLowerLimitEnabledCmd(d, enabled, ki18n("%1: change negative lower limit")));
}

STD_SETTER_CMD_IMPL_F_S(ProcessBehaviorChart, SetExactLimitsEnabled, bool, exactLimitsEnabled, recalc)
void ProcessBehaviorChart::setExactLimitsEnabled(bool enabled) {
	Q_D(ProcessBehaviorChart);
	if (enabled != d->exactLimitsEnabled)
		exec(new ProcessBehaviorChartSetExactLimitsEnabledCmd(d, enabled, ki18n("%1: change exact limits")));
}

// values
STD_SETTER_CMD_IMPL_F_S(ProcessBehaviorChart, SetValuesEnabled, bool, valuesEnabled, updateValueLabels)
void ProcessBehaviorChart::setValuesEnabled(bool enabled) {
	Q_D(ProcessBehaviorChart);
	if (enabled != d->valuesEnabled)
		exec(new ProcessBehaviorChartSetValuesEnabledCmd(d, enabled, ki18n("%1: enable control values")));
}

void ProcessBehaviorChart::setValuesBorderShape(TextLabel::BorderShape shape) {
	Q_D(ProcessBehaviorChart);
	if (shape != d->centerValueLabel->borderShape()) {
		beginMacro(i18n("%1: set values border shape", name()));
		d->centerValueLabel->setBorderShape(shape);
		d->upperLimitValueLabel->setBorderShape(shape);
		d->lowerLimitValueLabel->setBorderShape(shape);
		endMacro();
	}
}


// ##############################################################################
// #################################  SLOTS  ####################################
// ##############################################################################
void ProcessBehaviorChart::retransform() {
	D(ProcessBehaviorChart);
	d->retransform();
}

void ProcessBehaviorChart::recalc() {
	D(ProcessBehaviorChart);
	d->recalc();
}

void ProcessBehaviorChart::dataColumnAboutToBeRemoved(const AbstractAspect* aspect) {
	Q_D(ProcessBehaviorChart);
	if (aspect == d->dataColumn) {
		d->dataColumn = nullptr;
		d->recalc();
		Q_EMIT dataChanged();
		Q_EMIT changed();
	}
}

void ProcessBehaviorChart::data2ColumnAboutToBeRemoved(const AbstractAspect* aspect) {
	Q_D(ProcessBehaviorChart);
	if (aspect == d->data2Column) {
		d->data2Column = nullptr;
		d->recalc();
		Q_EMIT dataChanged();
		Q_EMIT changed();
	}
}

// ##############################################################################
// ######################### Private implementation #############################
// ##############################################################################
ProcessBehaviorChartPrivate::ProcessBehaviorChartPrivate(ProcessBehaviorChart* owner)
	: PlotPrivate(owner)
	, q(owner) {
	setFlag(QGraphicsItem::ItemIsSelectable, true);
	setAcceptHoverEvents(false);
}

ProcessBehaviorChartPrivate::~ProcessBehaviorChartPrivate() {
}

/*!
  called when the size of the plot or its data ranges (manual changes, zooming, etc.) were changed.
  recalculates the position of the scene points to be drawn.
  triggers the update of lines, drop lines, symbols etc.
*/
void ProcessBehaviorChartPrivate::retransform() {
	if (suppressRetransform || q->isLoading())
		return;

	if (!isVisible())
		return;

	PERFTRACE(name() + QLatin1String(Q_FUNC_INFO));

	// curves
	dataCurve->retransform();
	centerCurve->retransform();
	upperLimitCurve->retransform();
	lowerLimitCurve->retransform();

	// values
	// update the position of the value labels
	auto cs = q->plot()->coordinateSystem(q->coordinateSystemIndex());
	const auto& xRange = q->plot()->range(Dimension::X, cs->index(Dimension::X));
	double x = xRange.end();
	centerValueLabel->setPositionLogical(QPointF(x, center));
	upperLimitValueLabel->setPositionLogical(QPointF(x, upperLimit));
	lowerLimitValueLabel->setPositionLogical(QPointF(x, lowerLimit));
	centerValueLabel->retransform();
	upperLimitValueLabel->retransform();
	lowerLimitValueLabel->retransform();

	recalcShapeAndBoundingRect();
}

/*!
 * called when the source data was changed, recalculates the plot.
 */
void ProcessBehaviorChartPrivate::recalc() {
	PERFTRACE(name() + QLatin1String(Q_FUNC_INFO));
	if (!dataColumn || ((type == ProcessBehaviorChart::Type::P || type == ProcessBehaviorChart::Type::U) && !data2Column)) {
		center = 0.;
		upperLimit = 0.;
		lowerLimit = 0.;
		xColumn->clear();
		yColumn->clear();
		xCenterColumn->clear();
		yCenterColumn->clear();
		xUpperLimitColumn->clear();
		yUpperLimitColumn->clear();
		xLowerLimitColumn->clear();
		yLowerLimitColumn->clear();
		centerValueLabel->setText(QString());
		upperLimitValueLabel->setText(QString());
		lowerLimitValueLabel->setText(QString());
		Q_EMIT q->dataChanged();
		return;
	}

	// supress retransforms in all internal curves while modifying the data,
	// everything will be retransformend at the very end
	dataCurve->setSuppressRetransform(true);
	centerCurve->setSuppressRetransform(true);
	upperLimitCurve->setSuppressRetransform(true);
	lowerLimitCurve->setSuppressRetransform(true);

	const int count = q->xIndexCount();
	const int xMin = 1;
	const int xMax = count;
	xColumn->clear();
	xColumn->resizeTo(count);
	for (int i = 0; i < count; ++i)
		xColumn->setIntegerAt(i, i + 1);

	dataCurve->setXColumn(xColumn);

	// min and max values for x
	xCenterColumn->setIntegerAt(0, xMin);
	xCenterColumn->setIntegerAt(1, xMax);

	// for P and U chart exact limits are calculated for every individual point ("stair-step limits"),
	// if exactLimits is true. Straight lines are drawn for limits otherwise.
	if ((type == ProcessBehaviorChart::Type::P || type == ProcessBehaviorChart::Type::U) && exactLimitsEnabled) {
		for (int i = 0; i < count; ++i) {
			xUpperLimitColumn->setIntegerAt(i, i + 1);
			xLowerLimitColumn->setIntegerAt(i, i + 1);
		}
	} else {
		xUpperLimitColumn->resizeTo(2);
		xLowerLimitColumn->resizeTo(2);
		xUpperLimitColumn->setIntegerAt(0, xMin);
		xUpperLimitColumn->setIntegerAt(1, xMax);
		xLowerLimitColumn->setIntegerAt(0, xMin);
		xLowerLimitColumn->setIntegerAt(1, xMax);
	}

	updateControlLimits();

	dataCurve->setSuppressRetransform(false);
	centerCurve->setSuppressRetransform(false);
	upperLimitCurve->setSuppressRetransform(false);
	lowerLimitCurve->setSuppressRetransform(false);

	// emit dataChanged() in order to retransform everything with the new size/shape of the plot
	Q_EMIT q->dataChanged();
}

/*!
 * conventions and definitions taken from Wheeler's book "Making Sense of Data"
 */
void ProcessBehaviorChartPrivate::updateControlLimits() {
	PERFTRACE(name() + QLatin1String(Q_FUNC_INFO));
	center = 0.;
	upperLimit = 0.;
	lowerLimit = 0.;
	yColumn->clear();
	yColumn->resizeTo(xColumn->rowCount());
	Q_EMIT q->statusInfo(QString()); // reset the previous info message

	// determine the number of values in the source data to be taken into account
	int count = dataColumn->rowCount();
	if (type == ProcessBehaviorChart::Type::XbarR || type == ProcessBehaviorChart::Type::R || type == ProcessBehaviorChart::Type::XbarS
		|| type == ProcessBehaviorChart::Type::S) {
		const int remainder = count % sampleSize;
		if (remainder > 0) {
			count -= remainder; // subract the remainder to handle complete samples only
			Q_EMIT q->statusInfo(i18n("The last sample is incomplete and was omitted."));
		}
	}

	switch (type) {
	case ProcessBehaviorChart::Type::XmR: {
		// calculate the mean moving range
		std::vector<double> movingRange;
		for (int i = 1; i < count; ++i) {
			if (dataColumn->isValid(i) && !dataColumn->isMasked(i) && dataColumn->isValid(i - 1) && !dataColumn->isMasked(i - 1))
				movingRange.push_back(std::abs(dataColumn->valueAt(i) - dataColumn->valueAt(i - 1)));
		}

		if (limitsMetric == ProcessBehaviorChart::LimitsMetric::Average) {
			// center line at the mean of the data
			const double mean = static_cast<const Column*>(dataColumn)->statistics().arithmeticMean;
			center = mean;

			// upper and lower limits
			const double meanMovingRange = gsl_stats_mean(movingRange.data(), 1, movingRange.size());
			const double E2 = 3 / nsl_pcm_d2(2); // n = 2, two values used to calculate the ranges
			upperLimit = mean + E2 * meanMovingRange;
			lowerLimit = mean - E2 * meanMovingRange;
		} else {
			// center line at the median of the data
			const double median = static_cast<const Column*>(dataColumn)->statistics().median;
			center = median;

			// upper and lower limits
			const double medianMovingRange = gsl_stats_median(movingRange.data(), 1, movingRange.size());
			const double E5 = 3 / nsl_pcm_d4(2); // n = 2, two values used to calculate the ranges
			upperLimit = median + E5 * medianMovingRange;
			lowerLimit = median - E5 * medianMovingRange;
		}

		// plotted data - original data
		dataCurve->setYColumn(dataColumn);

		break;
	}
	case ProcessBehaviorChart::Type::mR: {
		// calculate the mean moving ranges
		for (int i = 1; i < count; ++i) {
			if (dataColumn->isValid(i) && !dataColumn->isMasked(i) && dataColumn->isValid(i - 1) && !dataColumn->isMasked(i - 1))
				yColumn->setValueAt(i, std::abs(dataColumn->valueAt(i) - dataColumn->valueAt(i - 1)));
		}

		if (limitsMetric == ProcessBehaviorChart::LimitsMetric::Average) {
			// center line
			const double meanMovingRange = yColumn->statistics().arithmeticMean;
			center = meanMovingRange;

			// upper and lower limits
			const double D3 = nsl_pcm_D3(2);
			const double D4 = nsl_pcm_D4(2);
			upperLimit = D4 * meanMovingRange;
			lowerLimit = D3 * meanMovingRange;
		} else { // median
			// center line
			const double medianMovingRange = yColumn->statistics().median;
			center = medianMovingRange;

			// upper and lower limits
			const double D5 = nsl_pcm_D5(2);
			const double D6 = nsl_pcm_D6(2);
			upperLimit = D6 * medianMovingRange;
			lowerLimit = D5 * medianMovingRange;
		}

		// plotted data - moving ranges
		dataCurve->setYColumn(yColumn);

		break;
	}
	case ProcessBehaviorChart::Type::XbarR: {
		// calculate the mean for each sample
		int groupIndex = 0;
		for (int i = 0; i < count; i += sampleSize) {
			double sum = 0.0;
			int j = 0;
			for (; j < sampleSize && (i + j) < count; ++j) {
				if (dataColumn->isValid(i + j) && !dataColumn->isMasked(i + j))
					sum += dataColumn->valueAt(i + j);
			}

			yColumn->setValueAt(groupIndex, sum / sampleSize);
			++groupIndex;
		}

		// Calculate the range for each sample
		std::vector<double> sampleRanges;
		for (int i = 0; i < count; i += sampleSize) {
			double minVal = dataColumn->valueAt(i);
			double maxVal = dataColumn->valueAt(i);
			for (int j = 1; j < sampleSize && (i + j) < count; ++j) {
				if (dataColumn->isValid(i) && !dataColumn->isMasked(i)) {
					double val = dataColumn->valueAt(i + j);
					if (val < minVal)
						minVal = val;
					if (val > maxVal)
						maxVal = val;
				}
			}
			sampleRanges.push_back(maxVal - minVal);
		}

		// center line at the mean of sample means ("grand average")
		const double meanOfMeans = yColumn->statistics().arithmeticMean;
		center = meanOfMeans;

		// upper and lower limits - the mean of means plus/minus normalized mean range
		if (limitsMetric == ProcessBehaviorChart::LimitsMetric::Average) {
			const double meanRange = gsl_stats_mean(sampleRanges.data(), 1, sampleRanges.size());
			const double A2 = nsl_pcm_A2(sampleSize);
			upperLimit = meanOfMeans + A2 * meanRange;
			lowerLimit = meanOfMeans - A2 * meanRange;
		} else { // median
			const double medianRange = gsl_stats_median(sampleRanges.data(), 1, sampleRanges.size());
			const double A4 = nsl_pcm_A4(sampleSize);
			upperLimit = meanOfMeans + A4 * medianRange;
			lowerLimit = meanOfMeans - A4 * medianRange;
		}

		// plotted data - means of subroups
		dataCurve->setYColumn(yColumn);

		break;
	}
	case ProcessBehaviorChart::Type::R: {
		// Calculate the range of samples
		int groupIndex = 0;
		for (int i = 0; i < count; i += sampleSize) {
			double minVal = dataColumn->valueAt(i);
			double maxVal = dataColumn->valueAt(i);
			for (int j = 1; j < sampleSize && (i + j) < count; ++j) {
				if (dataColumn->isValid(i) && !dataColumn->isMasked(i)) {
					double val = dataColumn->valueAt(i + j);
					if (val < minVal)
						minVal = val;
					if (val > maxVal)
						maxVal = val;
				}
			}

			yColumn->setValueAt(groupIndex, maxVal - minVal);
			++groupIndex;
		}

		if (limitsMetric == ProcessBehaviorChart::LimitsMetric::Average) {
			// center line at the average range
			const double meanRange = yColumn->statistics().arithmeticMean;
			center = meanRange;

			// upper and lower limits
			const double D3 = nsl_pcm_D3(sampleSize);
			const double D4 = nsl_pcm_D4(sampleSize);
			upperLimit = D4 * meanRange;
			lowerLimit = D3 * meanRange;
		} else { // median
			// center line at the median range
			const double medianRange = yColumn->statistics().median;
			center = medianRange;

			// upper and lower limits
			const double D5 = nsl_pcm_D5(sampleSize);
			const double D6 = nsl_pcm_D6(sampleSize);
			upperLimit = D6 * medianRange;
			lowerLimit = D5 * medianRange;
		}

		// plotted data - subroup ranges
		dataCurve->setYColumn(yColumn);

		break;
	}
	case ProcessBehaviorChart::Type::XbarS: { // chart based on the means and standard deviations for each sample
		// Calculate the mean of samples
		int groupIndex = 0;
		for (int i = 0; i < count; i += sampleSize) {
			double sum = 0.0;
			for (int j = 0; j < sampleSize && (i + j) < count; ++j) {
				if (dataColumn->isValid(i) && !dataColumn->isMasked(i))
					sum += dataColumn->valueAt(i + j);
			}

			yColumn->setValueAt(groupIndex, sum / sampleSize);
			++groupIndex;
		}

		// Calculate the standard deviations of samples
		std::vector<double> sampleStdDevs;
		for (int i = 0; i < count; i += sampleSize) {
			std::vector<double> sample;
			for (int j = 0; j < sampleSize && (i + j) < count; ++j) {
				if (dataColumn->isValid(i) && !dataColumn->isMasked(i))
					sample.push_back(dataColumn->valueAt(i + j));
			}
			const double stddev = gsl_stats_sd(sample.data(), 1, sample.size());
			sampleStdDevs.push_back(stddev);
		}

		// center line at the mean of means
		const double meanOfMeans = yColumn->statistics().arithmeticMean;
		center = meanOfMeans;

		// upper and lower limits
		const double meanStdDev = gsl_stats_mean(sampleStdDevs.data(), 1, sampleStdDevs.size());
		const double A3 = nsl_pcm_A3(sampleSize);
		upperLimit = meanOfMeans + A3 * meanStdDev;
		lowerLimit = meanOfMeans - A3 * meanStdDev;

		// plotted data - subroup means
		dataCurve->setYColumn(yColumn);

		break;
	}
	case ProcessBehaviorChart::Type::S: {
		// Calculate the standard deviation for each sample
		int groupIndex = 0;
		for (int i = 0; i < count; i += sampleSize) {
			std::vector<double> sample;
			for (int j = 0; j < sampleSize && (i + j) < count; ++j) {
				if (dataColumn->isValid(i + j) && !dataColumn->isMasked(i + j))
					sample.push_back(dataColumn->valueAt(i + j));
			}
			const double stddev = gsl_stats_sd(sample.data(), 1, sample.size());
			yColumn->setValueAt(groupIndex, stddev);
			++groupIndex;
		}

		// center line
		const double meanStdDev = yColumn->statistics().arithmeticMean;
		center = meanStdDev;

		// upper and lower limits
		const double B3 = nsl_pcm_B3(sampleSize);
		const double B4 = nsl_pcm_B4(sampleSize);
		upperLimit = B4 * meanStdDev;
		lowerLimit = B3 * meanStdDev;

		// plotted data - sample standard deviations
		dataCurve->setYColumn(yColumn);

		break;
	}
	case ProcessBehaviorChart::Type::P: {
		// calculate the proportions
		double total = 0.;
		double totalSampleSize = 0.;
		for (int i = 0; i < count; ++i) {
			if (dataColumn->isValid(i) && !dataColumn->isMasked(i) && data2Column->isValid(i) && !data2Column->isMasked(i)) {
				yColumn->setValueAt(i, dataColumn->valueAt(i) / data2Column->valueAt(i));
				total += dataColumn->valueAt(i);
				totalSampleSize += data2Column->valueAt(i);
			}
		}

		// center line
		const double pbar = (totalSampleSize) ? total / totalSampleSize : 0;
		center = pbar;

		// upper and lower limits
		if (exactLimitsEnabled) {
			for (int i = 0; i < count; ++i) {
				if (dataColumn->isValid(i) && !dataColumn->isMasked(i) && data2Column->isValid(i) && !data2Column->isMasked(i)) {
					const double distance = 3. * std::sqrt(pbar * (1 - pbar) / data2Column->valueAt(i));
					yUpperLimitColumn->setValueAt(i, pbar + distance);
					yLowerLimitColumn->setValueAt(i, pbar - distance);
				}
			}
		} else {
			double nMean = static_cast<const Column*>(data2Column)->statistics().arithmeticMean;
			const double distance = 3. * std::sqrt(pbar * (1 - pbar) / nMean);
			upperLimit = pbar + distance;
			lowerLimit = pbar - distance;
		}

		// plotted data - proportions
		dataCurve->setYColumn(yColumn);

		break;
	}
	case ProcessBehaviorChart::Type::NP: {
		// Calculate the total number of defectives
		double totalDefectivesCount = 0.0;
		for (int i = 0; i < count; ++i) {
			if (dataColumn->isValid(i) && !dataColumn->isMasked(i))
				totalDefectivesCount += dataColumn->valueAt(i);
		}

		// center
		const double size = static_cast<const Column*>(dataColumn)->statistics().size;
		const double pBar = totalDefectivesCount / (sampleSize * size);
		const double npBar = sampleSize * pBar;
		center = npBar;

		// upper and lower limits
		const double distance = 3. * std::sqrt(npBar * (1 - pBar));
		upperLimit = npBar + distance;
		lowerLimit = npBar - distance;

		// plotted data - original data
		dataCurve->setYColumn(dataColumn);

		break;
	}
	case ProcessBehaviorChart::Type::C: {
		// center
		center = static_cast<const Column*>(dataColumn)->statistics().arithmeticMean;

		// upper and lower limits
		upperLimit = center + 3 * std::sqrt(center);
		lowerLimit = center - 3 * std::sqrt(center);

		// plotted data - original data
		dataCurve->setYColumn(dataColumn);
		break;
	}
	case ProcessBehaviorChart::Type::U: {
		// calculate the ratios
		double total = 0.;
		double totalSampleSize = 0.;
		for (int i = 0; i < count; ++i) {
			if (dataColumn->isValid(i) && !dataColumn->isMasked(i) && data2Column->isValid(i) && !data2Column->isMasked(i)) {
				yColumn->setValueAt(i, dataColumn->valueAt(i) / data2Column->valueAt(i));
				total += dataColumn->valueAt(i);
				totalSampleSize += data2Column->valueAt(i);
			}
		}

		// center line
		const double ubar = (totalSampleSize) ? total / totalSampleSize : 0;
		center = ubar;

		// upper and lower limits
		if (exactLimitsEnabled) {
			for (int i = 0; i < count; ++i) {
				if (dataColumn->isValid(i) && !dataColumn->isMasked(i) && data2Column->isValid(i) && !data2Column->isMasked(i)) {
					const double distance = 3. * std::sqrt(ubar / data2Column->valueAt(i));
					yUpperLimitColumn->setValueAt(i, ubar + distance);
					yLowerLimitColumn->setValueAt(i, ubar - distance);
				}
			}
		} else {
			double nMean = static_cast<const Column*>(data2Column)->statistics().arithmeticMean;
			const double distance = 3. * std::sqrt(ubar / nMean);
			upperLimit = ubar + distance;
			lowerLimit = ubar - distance;
		}

		// plotted data - proportions
		dataCurve->setYColumn(yColumn);
	}
	}

	QDEBUG(Q_FUNC_INFO << ", center: " << center << " , upper limit: " << upperLimit << ", lower limit: " << lowerLimit);

	// further restrict the lower limit if it becomes negative
	if (type == ProcessBehaviorChart::Type::XmR || type == ProcessBehaviorChart::Type::XbarR || type == ProcessBehaviorChart::Type::XbarS) {
		// restrict the lower limit to 0, the curve for the lower limit line is always visible
		if (lowerLimit < 0. && !negativeLowerLimitEnabled)
			lowerLimit = 0.;

		lowerLimitCurve->setVisible(true);
	} else if (type == ProcessBehaviorChart::Type::mR || type == ProcessBehaviorChart::Type::R || type == ProcessBehaviorChart::Type::S
			   || type == ProcessBehaviorChart::Type::C) {
		// restrict the lower limit to 0, hide the curve for the lower limit line
		if (lowerLimit < 0.) {
			lowerLimit = 0.;
			lowerLimitCurve->setVisible(false);
		} else
			lowerLimitCurve->setVisible(true);
	}

	yCenterColumn->setValueAt(0, center);
	yCenterColumn->setValueAt(1, center);

	// for P and U chart limits are calculated for every individual point ("stair-step limits"),
	// for other charts straight lines are drawn
	if ((type == ProcessBehaviorChart::Type::P || type == ProcessBehaviorChart::Type::U) && exactLimitsEnabled) {
		upperLimitCurve->setLineType(XYCurve::LineType::MidpointHorizontal); // required for stair-step lines for P and U charts
		lowerLimitCurve->setLineType(XYCurve::LineType::MidpointHorizontal); // required for stair-step lines for P and U charts
	} else {
		yUpperLimitColumn->setValueAt(0, upperLimit);
		yUpperLimitColumn->setValueAt(1, upperLimit);
		yLowerLimitColumn->setValueAt(0, lowerLimit);
		yLowerLimitColumn->setValueAt(1, lowerLimit);
	}

	// update the texts in the value labels
	updateValueLabels();
}

void ProcessBehaviorChartPrivate::updateValueLabels() {
	centerValueLabel->setVisible(valuesEnabled);
	upperLimitValueLabel->setVisible(valuesEnabled);
	lowerLimitValueLabel->setVisible(valuesEnabled);

	if (valuesEnabled) {
		const auto numberLocale = QLocale();
		centerValueLabel->setText(numberLocale.toString(center));
		upperLimitValueLabel->setText(numberLocale.toString(upperLimit));
		lowerLimitValueLabel->setText(numberLocale.toString(lowerLimit));

		// update the position of the value labels
		auto cs = q->plot()->coordinateSystem(q->coordinateSystemIndex());
		const auto& xRange = q->plot()->range(Dimension::X, cs->index(Dimension::X));
		double x = xRange.end();
		centerValueLabel->setPositionLogical(QPointF(x, center));
		upperLimitValueLabel->setPositionLogical(QPointF(x, upperLimit));
		lowerLimitValueLabel->setPositionLogical(QPointF(x, lowerLimit));
	}
}

/*!
  recalculates the outer bounds and the shape of the curve.
  */
void ProcessBehaviorChartPrivate::recalcShapeAndBoundingRect() {
	if (suppressRecalc)
		return;

	prepareGeometryChange();
	m_shape = QPainterPath();

	// curves
	m_shape.addPath(dataCurve->graphicsItem()->shape());
	m_shape.addPath(centerCurve->graphicsItem()->shape());
	m_shape.addPath(upperLimitCurve->graphicsItem()->shape());
	m_shape.addPath(lowerLimitCurve->graphicsItem()->shape());

	// labels
	m_shape.addPath(centerValueLabel->graphicsItem()->shape());
	m_shape.addPath(upperLimitValueLabel->graphicsItem()->shape());
	m_shape.addPath(lowerLimitValueLabel->graphicsItem()->shape());

	m_boundingRectangle = m_shape.boundingRect();
}

// ##############################################################################
// ##################  Serialization/Deserialization  ###########################
// ##############################################################################
//! Save as XML
void ProcessBehaviorChart::save(QXmlStreamWriter* writer) const {
	Q_D(const ProcessBehaviorChart);

	writer->writeStartElement(QStringLiteral("ProcessBehaviorChart"));
	writeBasicAttributes(writer);
	writeCommentElement(writer);

	// general
	writer->writeStartElement(QStringLiteral("general"));
	WRITE_COLUMN(d->dataColumn, dataColumn);
	WRITE_COLUMN(d->data2Column, data2Column);
	WRITE_COLUMN(d->xColumn, xColumn);
	WRITE_COLUMN(d->yColumn, yColumn);
	WRITE_COLUMN(d->xCenterColumn, xCenterColumn);
	WRITE_COLUMN(d->yCenterColumn, yCenterColumn);
	WRITE_COLUMN(d->xUpperLimitColumn, xUpperLimitColumn);
	WRITE_COLUMN(d->yUpperLimitColumn, yUpperLimtColumn);
	WRITE_COLUMN(d->xLowerLimitColumn, xLowerLimitColumn);
	WRITE_COLUMN(d->yLowerLimitColumn, yLowerLimitColumn);
	writer->writeAttribute(QStringLiteral("type"), QString::number(static_cast<int>(d->type)));
	writer->writeAttribute(QStringLiteral("limitsMetric"), QString::number(static_cast<int>(d->limitsMetric)));
	writer->writeAttribute(QStringLiteral("sampleSize"), QString::number(d->sampleSize));
	writer->writeAttribute(QStringLiteral("negativeLowerLimitEnabled"), QString::number(d->negativeLowerLimitEnabled));
	writer->writeAttribute(QStringLiteral("exactLimitsEnabled"), QString::number(d->exactLimitsEnabled));
	writer->writeAttribute(QStringLiteral("visible"), QString::number(d->isVisible()));
	writer->writeAttribute(QStringLiteral("legendVisible"), QString::number(d->legendVisible));
	writer->writeEndElement();

	// save the internal columns, above only the references to them were saved
	d->xColumn->save(writer);
	d->yColumn->save(writer);
	d->xCenterColumn->save(writer);
	d->yCenterColumn->save(writer);
	d->xUpperLimitColumn->save(writer);
	d->yUpperLimitColumn->save(writer);
	d->xLowerLimitColumn->save(writer);
	d->yLowerLimitColumn->save(writer);

	// save the internal curves
	// disconnect temporarily from renameInternalCurves so we can use unique names to be able to properly load the curves later
	disconnect(this, &AbstractAspect::aspectDescriptionChanged, this, &ProcessBehaviorChart::renameInternalCurves);
	d->dataCurve->setName(QStringLiteral("data"));
	d->dataCurve->save(writer);
	d->centerCurve->setName(QStringLiteral("center"));
	d->centerCurve->save(writer);
	d->upperLimitCurve->setName(QStringLiteral("upperLimit"));
	d->upperLimitCurve->save(writer);
	d->lowerLimitCurve->setName(QStringLiteral("lowerLimit"));
	d->lowerLimitCurve->save(writer);
	connect(this, &AbstractAspect::aspectDescriptionChanged, this, &ProcessBehaviorChart::renameInternalCurves);

	writer->writeEndElement(); // close "ProcessBehaviorChart" section
}

//! Load from XML
bool ProcessBehaviorChart::load(XmlStreamReader* reader, bool preview) {
	Q_D(ProcessBehaviorChart);

	if (!readBasicAttributes(reader))
		return false;

	QXmlStreamAttributes attribs;
	QString str;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == QLatin1String("ProcessBehaviorChart"))
			break;

		if (!reader->isStartElement())
			continue;

		if (reader->name() == QLatin1String("comment")) {
			if (!readCommentElement(reader))
				return false;
		} else if (!preview && reader->name() == QLatin1String("general")) {
			attribs = reader->attributes();
			READ_COLUMN(dataColumn);
			READ_COLUMN(data2Column);
			READ_COLUMN(xColumn);
			READ_COLUMN(yColumn);
			READ_COLUMN(xCenterColumn);
			READ_COLUMN(yCenterColumn);
			READ_COLUMN(xUpperLimitColumn);
			READ_COLUMN(yUpperLimitColumn);
			READ_COLUMN(xLowerLimitColumn);
			READ_COLUMN(yLowerLimitColumn);
			READ_INT_VALUE("type", type, ProcessBehaviorChart::Type);
			READ_INT_VALUE("limitsMetric", limitsMetric, ProcessBehaviorChart::LimitsMetric);
			READ_INT_VALUE("sampleSize", sampleSize, int);
			READ_INT_VALUE("negativeLowerLimitEnabled", negativeLowerLimitEnabled, bool);
			READ_INT_VALUE("exactLimitsEnabled", exactLimitsEnabled, bool);
			READ_INT_VALUE("legendVisible", legendVisible, bool);

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
			else if (name == QLatin1String("y"))
				rc = d->yColumn->load(reader, preview);
			else if (name == QLatin1String("xCenter"))
				rc = d->xCenterColumn->load(reader, preview);
			else if (name == QLatin1String("yCenter"))
				rc = d->yCenterColumn->load(reader, preview);
			else if (name == QLatin1String("xUpperLimit"))
				rc = d->xUpperLimitColumn->load(reader, preview);
			else if (name == QLatin1String("yUpperLimit"))
				rc = d->yUpperLimitColumn->load(reader, preview);
			else if (name == QLatin1String("xLowerLimit"))
				rc = d->xLowerLimitColumn->load(reader, preview);
			else if (name == QLatin1String("yLowerLimit"))
				rc = d->yLowerLimitColumn->load(reader, preview);

			if (!rc)
				return false;
		} else if (reader->name() == QLatin1String("xyCurve")) {
			attribs = reader->attributes();
			bool rc = false;
			if (attribs.value(QStringLiteral("name")) == QLatin1String("data"))
				rc = d->dataCurve->load(reader, preview);
			else if (attribs.value(QStringLiteral("name")) == QLatin1String("center"))
				rc = d->centerCurve->load(reader, preview);
			else if (attribs.value(QStringLiteral("name")) == QLatin1String("upperLimit"))
				rc = d->upperLimitCurve->load(reader, preview);
			else if (attribs.value(QStringLiteral("name")) == QLatin1String("lowerLimit"))
				rc = d->lowerLimitCurve->load(reader, preview);

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
void ProcessBehaviorChart::loadThemeConfig(const KConfig& config) {
	KConfigGroup group;
	if (config.hasGroup(QStringLiteral("Theme")))
		group = config.group(QStringLiteral("XYCurve")); // when loading from the theme config, use the same properties as for XYCurve
	else
		group = config.group(QStringLiteral("ProcessBehaviorChart"));

	Q_D(ProcessBehaviorChart);
	const auto* plot = d->m_plot;
	int index = plot->curveChildIndex(this);
	QColor themeColor = plot->themeColorPalette(index);

	d->suppressRecalc = true;

	d->dataCurve->line()->loadThemeConfig(group, themeColor);
	d->dataCurve->symbol()->loadThemeConfig(group, themeColor);

	themeColor = plot->themeColorPalette(index + 1);

	d->centerCurve->line()->loadThemeConfig(group, themeColor);
	d->centerCurve->symbol()->setStyle(Symbol::Style::NoSymbols);

	d->upperLimitCurve->line()->loadThemeConfig(group, themeColor);
	d->upperLimitCurve->symbol()->setStyle(Symbol::Style::NoSymbols);

	d->lowerLimitCurve->line()->loadThemeConfig(group, themeColor);
	d->lowerLimitCurve->symbol()->setStyle(Symbol::Style::NoSymbols);

	d->centerValueLabel->loadThemeConfig(config);
	d->upperLimitValueLabel->loadThemeConfig(config);
	d->lowerLimitValueLabel->loadThemeConfig(config);

	d->suppressRecalc = false;
	d->recalcShapeAndBoundingRect();
}

void ProcessBehaviorChart::saveThemeConfig(const KConfig& config) {
	Q_D(const ProcessBehaviorChart);
	KConfigGroup group = config.group(QStringLiteral("ProcessBehaviorChart"));
	d->dataCurve->line()->saveThemeConfig(group);
	d->dataCurve->symbol()->saveThemeConfig(group);
	d->centerCurve->line()->saveThemeConfig(group);
	d->upperLimitCurve->line()->saveThemeConfig(group);
	d->lowerLimitCurve->line()->saveThemeConfig(group);
}
