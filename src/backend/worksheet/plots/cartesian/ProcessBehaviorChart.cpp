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
ProcessBehaviorChart::ProcessBehaviorChart(const QString& name, bool loading)
	: Plot(name, new ProcessBehaviorChartPrivate(this), AspectType::ProcessBehaviorChart) {
	init(loading);
}

ProcessBehaviorChart::ProcessBehaviorChart(const QString& name, ProcessBehaviorChartPrivate* dd)
	: Plot(name, dd, AspectType::ProcessBehaviorChart) {
	init(false);
}

// no need to delete the d-pointer here - it inherits from QGraphicsItem
// and is deleted during the cleanup in QGraphicsScene
ProcessBehaviorChart::~ProcessBehaviorChart() = default;

void ProcessBehaviorChart::init(bool loading) {
	Q_D(ProcessBehaviorChart);

	// curve and columns for the data points
	d->dataCurve = new XYCurve(QStringLiteral("data"));
	d->dataCurve->setName(name(), AbstractAspect::NameHandling::UniqueNotRequired);
	d->dataCurve->setHidden(true);
	d->dataCurve->graphicsItem()->setParentItem(d);

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

	// border line for value labels
	d->labelsBorderLine = new Line(QStringLiteral("borderLine"));
	d->labelsBorderLine->setPrefix(QStringLiteral("Border"));
	d->labelsBorderLine->setCreateXmlElement(false);
	d->labelsBorderLine->setHidden(true);
	addChild(d->labelsBorderLine);

	d->upperLimitLabel = new TextLabel(QStringLiteral("upperLimitValue"));
	d->upperLimitLabel->setHidden(true);
	d->upperLimitLabel->setHorizontalAlignment(WorksheetElement::HorizontalAlignment::Left);

	d->centerLabel = new TextLabel(QStringLiteral("centerValue"));
	d->centerLabel->setHidden(true);
	d->centerLabel->setHorizontalAlignment(WorksheetElement::HorizontalAlignment::Left);
	// set alpha channel to 1 (solid) for background color (default is 0 for TextLabel)
	auto bgcolor = d->centerLabel->backgroundColor();
	// QDEBUG(Q_FUNC_INFO << ", COLOR = " << bgcolor)
	bgcolor.setAlphaF(1.0);
	d->centerLabel->setBackgroundColor(bgcolor);

	d->lowerLimitLabel = new TextLabel(QStringLiteral("lowerLimitValue"));
	d->lowerLimitLabel->setHidden(true);
	d->lowerLimitLabel->setHorizontalAlignment(WorksheetElement::HorizontalAlignment::Left);

	if (loading)
		return;

	// init the properties
	KConfig config;
	KConfigGroup group = config.group(QStringLiteral("ProcessBehaviorChart"));

	// general properties
	d->type = static_cast<Type>(group.readEntry(QStringLiteral("Type"), static_cast<int>(Type::XmR)));
	d->limitsType = static_cast<LimitsType>(group.readEntry(QStringLiteral("LimitsType"), static_cast<int>(LimitsType::Statistical)));
	d->sampleSize = group.readEntry(QStringLiteral("SampleSize"), 5);
	d->limitsMetric = static_cast<LimitsMetric>(group.readEntry(QStringLiteral("LimitsMetric"), static_cast<int>(LimitsMetric::Average)));
	// TODO: limit constraints and specifications?
	d->exactLimitsEnabled = group.readEntry(QStringLiteral("ExactLimitsEnabled"), true);

	d->dataCurve->line()->init(group);
	d->dataCurve->line()->setStyle(Qt::SolidLine);
	d->dataCurve->symbol()->setStyle(Symbol::Style::Circle);
	d->dataCurve->background()->setPosition(Background::Position::No);

	d->centerCurve->line()->init(group);
	d->centerCurve->line()->setStyle(Qt::SolidLine);
	d->centerCurve->symbol()->setStyle(Symbol::Style::NoSymbols);
	d->centerCurve->background()->setPosition(Background::Position::No);

	d->upperLimitCurve->line()->init(group);
	d->upperLimitCurve->line()->setStyle(Qt::DashLine);
	d->upperLimitCurve->symbol()->setStyle(Symbol::Style::NoSymbols);
	d->upperLimitCurve->background()->setPosition(Background::Position::No);
	d->upperLimitCurve->setLineType(XYCurve::LineType::MidpointHorizontal); // required for stair-step lines for P and U charts

	d->lowerLimitCurve->line()->init(group);
	d->lowerLimitCurve->line()->setStyle(Qt::DashLine);
	d->lowerLimitCurve->symbol()->setStyle(Symbol::Style::NoSymbols);
	d->lowerLimitCurve->background()->setPosition(Background::Position::No);
	d->lowerLimitCurve->setLineType(XYCurve::LineType::MidpointHorizontal); // required for stair-step lines for P and U charts

	// text labels for the labels
	d->labelsEnabled = group.readEntry(QStringLiteral("LabelsEnabled"), true);
	d->labelsAutoPrecision = group.readEntry(QStringLiteral("LabelsAutoPrecision"), false);
	d->labelsPrecision = group.readEntry(QStringLiteral("LabelsPrecision"), 2);
	const auto shape = static_cast<TextLabel::BorderShape>(group.readEntry(QStringLiteral("BorderShape"), (int)TextLabel::BorderShape::NoBorder));
	d->upperLimitLabel->setBorderShape(shape);
	d->centerLabel->setBorderShape(shape);
	d->lowerLimitLabel->setBorderShape(shape);
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
	addChildFast(d->upperLimitLabel);
	d->upperLimitLabel->setCoordinateBindingEnabled(true);
	d->upperLimitLabel->setParentGraphicsItem(graphicsItem());
	d->upperLimitLabel->graphicsItem()->setFlag(QGraphicsItem::ItemIsMovable, false);

	addChildFast(d->centerLabel);
	d->centerLabel->setCoordinateBindingEnabled(true);
	d->centerLabel->setParentGraphicsItem(graphicsItem());
	d->centerLabel->graphicsItem()->setFlag(QGraphicsItem::ItemIsMovable, false);

	addChildFast(d->lowerLimitLabel);
	d->lowerLimitLabel->setCoordinateBindingEnabled(true);
	d->lowerLimitLabel->setParentGraphicsItem(graphicsItem());
	d->lowerLimitLabel->graphicsItem()->setFlag(QGraphicsItem::ItemIsMovable, false);

	// synchronize the names of the internal XYCurves with the name of the current plot
	// so we have the same name shown on the undo stack
	connect(this, &AbstractAspect::aspectDescriptionChanged, this, &ProcessBehaviorChart::renameInternalCurves);

	// propagate the visual changes to the parent
	connect(d->centerCurve, &XYCurve::changed, this, &ProcessBehaviorChart::changed);
	connect(d->dataCurve, &XYCurve::changed, this, &ProcessBehaviorChart::changed);
	connect(d->upperLimitCurve, &XYCurve::changed, this, &ProcessBehaviorChart::changed);
	connect(d->lowerLimitCurve, &XYCurve::changed, this, &ProcessBehaviorChart::changed);

	// re-position the labels on text changes
	connect(d->upperLimitLabel, &TextLabel::textWrapperChanged, d->upperLimitLabel, &TextLabel::retransform);
	connect(d->centerLabel, &TextLabel::textWrapperChanged, d->centerLabel, &TextLabel::retransform);
	connect(d->lowerLimitLabel, &TextLabel::textWrapperChanged, d->lowerLimitLabel, &TextLabel::retransform);

	// notify the dock widget on changes in Line
	connect(d->labelsBorderLine, &Line::styleChanged, this, &ProcessBehaviorChart::labelsBorderStyleChanged);
	connect(d->labelsBorderLine, &Line::widthChanged, this, &ProcessBehaviorChart::labelsBorderWidthChanged);
	connect(d->labelsBorderLine, &Line::colorChanged, this, &ProcessBehaviorChart::labelsBorderColorChanged);
	connect(d->labelsBorderLine, &Line::opacityChanged, this, &ProcessBehaviorChart::labelsBorderOpacityChanged);
}

void ProcessBehaviorChart::renameInternalCurves() {
	Q_D(ProcessBehaviorChart);
	d->dataCurve->setUndoAware(false);
	d->centerCurve->setUndoAware(false);
	d->upperLimitCurve->setUndoAware(false);
	d->lowerLimitCurve->setUndoAware(false);
	d->dataCurve->setName(name(), AbstractAspect::NameHandling::UniqueNotRequired);
	d->centerCurve->setName(name(), AbstractAspect::NameHandling::UniqueNotRequired);
	d->upperLimitCurve->setName(name(), AbstractAspect::NameHandling::UniqueNotRequired);
	d->lowerLimitCurve->setName(name(), AbstractAspect::NameHandling::UniqueNotRequired);
	d->dataCurve->setUndoAware(true);
	d->centerCurve->setUndoAware(true);
	d->upperLimitCurve->setUndoAware(true);
	d->lowerLimitCurve->setUndoAware(true);
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
BASIC_SHARED_D_READER_IMPL(ProcessBehaviorChart, ProcessBehaviorChart::LimitsType, limitsType, limitsType)
BASIC_SHARED_D_READER_IMPL(ProcessBehaviorChart, ProcessBehaviorChart::LimitsMetric, limitsMetric, limitsMetric)
BASIC_SHARED_D_READER_IMPL(ProcessBehaviorChart, int, sampleSize, sampleSize)
BASIC_SHARED_D_READER_IMPL(ProcessBehaviorChart, double, maxUpperLimit, maxUpperLimit)
BASIC_SHARED_D_READER_IMPL(ProcessBehaviorChart, double, minLowerLimit, minLowerLimit)
BASIC_SHARED_D_READER_IMPL(ProcessBehaviorChart, double, centerSpecification, centerSpecification)
BASIC_SHARED_D_READER_IMPL(ProcessBehaviorChart, double, upperLimitSpecification, upperLimitSpecification)
BASIC_SHARED_D_READER_IMPL(ProcessBehaviorChart, double, lowerLimitSpecification, lowerLimitSpecification)
BASIC_SHARED_D_READER_IMPL(ProcessBehaviorChart, bool, exactLimitsEnabled, exactLimitsEnabled)
BASIC_SHARED_D_READER_IMPL(ProcessBehaviorChart, const AbstractColumn*, dataColumn, dataColumn)
BASIC_SHARED_D_READER_IMPL(ProcessBehaviorChart, QString, dataColumnPath, dataColumnPath)
BASIC_SHARED_D_READER_IMPL(ProcessBehaviorChart, const AbstractColumn*, data2Column, data2Column)
BASIC_SHARED_D_READER_IMPL(ProcessBehaviorChart, QString, data2ColumnPath, data2ColumnPath)

// labels
BASIC_SHARED_D_READER_IMPL(ProcessBehaviorChart, bool, labelsEnabled, labelsEnabled)
BASIC_SHARED_D_READER_IMPL(ProcessBehaviorChart, bool, labelsAutoPrecision, labelsAutoPrecision)
BASIC_SHARED_D_READER_IMPL(ProcessBehaviorChart, int, labelsPrecision, labelsPrecision)

QColor ProcessBehaviorChart::labelsFontColor() const {
	Q_D(const ProcessBehaviorChart);
	// using center label color
	// QDEBUG(Q_FUNC_INFO << ", font color =" << d->centerLabel->fontColor())
	return d->centerLabel->fontColor();
}

QColor ProcessBehaviorChart::labelsBackgroundColor() const {
	Q_D(const ProcessBehaviorChart);
	// using center label color
	// QDEBUG(Q_FUNC_INFO << ", background color =" << d->centerLabel->backgroundColor())
	return d->centerLabel->backgroundColor();
}

QFont ProcessBehaviorChart::labelsFont() const {
	Q_D(const ProcessBehaviorChart);
	// using center label font
	QTextEdit te(d->centerLabel->text().text);
	te.selectAll();
	// QDEBUG(Q_FUNC_INFO <<", FONT =" << te.currentCharFormat().font())
	return te.currentCharFormat().font();
}

TextLabel::BorderShape ProcessBehaviorChart::labelsBorderShape() const {
	Q_D(const ProcessBehaviorChart);
	return d->centerLabel->borderShape();
}

Line* ProcessBehaviorChart::labelsBorderLine() const {
	Q_D(const ProcessBehaviorChart);
	return d->labelsBorderLine;
}

/*!
 * returns the number of index labels used for x.
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
void ProcessBehaviorChart::setType(Type type) {
	Q_D(ProcessBehaviorChart);
	if (type != d->type)
		exec(new ProcessBehaviorChartSetTypeCmd(d, type, ki18n("%1: set type")));
}

STD_SETTER_CMD_IMPL_F_S(ProcessBehaviorChart, SetLimitsType, ProcessBehaviorChart::LimitsType, limitsType, recalc)
void ProcessBehaviorChart::setLimitsType(LimitsType limitsType) {
	Q_D(ProcessBehaviorChart);
	if (limitsType != d->limitsType)
		exec(new ProcessBehaviorChartSetLimitsTypeCmd(d, limitsType, ki18n("%1: set limits type")));
}

STD_SETTER_CMD_IMPL_F_S(ProcessBehaviorChart, SetLimitsMetric, ProcessBehaviorChart::LimitsMetric, limitsMetric, recalc)
void ProcessBehaviorChart::setLimitsMetric(LimitsMetric limitsMetric) {
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

STD_SETTER_CMD_IMPL_F_S(ProcessBehaviorChart, SetExactLimitsEnabled, bool, exactLimitsEnabled, recalc)
void ProcessBehaviorChart::setExactLimitsEnabled(bool enabled) {
	Q_D(ProcessBehaviorChart);
	if (enabled != d->exactLimitsEnabled) {
		KLocalizedString msg;
		enabled ? msg = ki18n("%1: enable exact limits") : msg = ki18n("%1: disable exact limits");
		exec(new ProcessBehaviorChartSetExactLimitsEnabledCmd(d, enabled, msg));
	}
}

STD_SETTER_CMD_IMPL_F_S(ProcessBehaviorChart, SetMaxUpperLimit, double, maxUpperLimit, recalc)
void ProcessBehaviorChart::setMaxUpperLimit(double maxUpperLimit) {
	Q_D(ProcessBehaviorChart);
	if (maxUpperLimit != d->maxUpperLimit)
		exec(new ProcessBehaviorChartSetMaxUpperLimitCmd(d, maxUpperLimit, ki18n("%1: set maximal upper limit")));
}

STD_SETTER_CMD_IMPL_F_S(ProcessBehaviorChart, SetMinLowerLimit, double, minLowerLimit, recalc)
void ProcessBehaviorChart::setMinLowerLimit(double minLowerLimit) {
	Q_D(ProcessBehaviorChart);
	if (minLowerLimit != d->minLowerLimit)
		exec(new ProcessBehaviorChartSetMinLowerLimitCmd(d, minLowerLimit, ki18n("%1: set minimal lower limit")));
}

STD_SETTER_CMD_IMPL_F_S(ProcessBehaviorChart, SetCenterSpecification, double, centerSpecification, updateSpecifications)
void ProcessBehaviorChart::setCenterSpecification(double centerSpecification) {
	Q_D(ProcessBehaviorChart);
	if (centerSpecification != d->centerSpecification)
		exec(new ProcessBehaviorChartSetCenterSpecificationCmd(d, centerSpecification, ki18n("%1: set center specification")));
}

STD_SETTER_CMD_IMPL_F_S(ProcessBehaviorChart, SetUpperLimitSpecification, double, upperLimitSpecification, updateSpecifications)
void ProcessBehaviorChart::setUpperLimitSpecification(double upperLimitSpecification) {
	Q_D(ProcessBehaviorChart);
	if (upperLimitSpecification != d->upperLimitSpecification)
		exec(new ProcessBehaviorChartSetUpperLimitSpecificationCmd(d, upperLimitSpecification, ki18n("%1: set upper limit specification")));
}

STD_SETTER_CMD_IMPL_F_S(ProcessBehaviorChart, SetLowerLimitSpecification, double, lowerLimitSpecification, updateSpecifications)
void ProcessBehaviorChart::setLowerLimitSpecification(double lowerLimitSpecification) {
	Q_D(ProcessBehaviorChart);
	if (lowerLimitSpecification != d->lowerLimitSpecification)
		exec(new ProcessBehaviorChartSetLowerLimitSpecificationCmd(d, lowerLimitSpecification, ki18n("%1: set lower limit specification")));
}

// labels
STD_SETTER_CMD_IMPL_F_S(ProcessBehaviorChart, SetLabelsEnabled, bool, labelsEnabled, updateLabels)
void ProcessBehaviorChart::setLabelsEnabled(bool enabled) {
	Q_D(ProcessBehaviorChart);
	if (enabled != d->labelsEnabled) {
		KLocalizedString msg;
		enabled ? msg = ki18n("%1: enable control labels") : msg = ki18n("%1: disable control labels");
		exec(new ProcessBehaviorChartSetLabelsEnabledCmd(d, enabled, msg));
	}
}

STD_SETTER_CMD_IMPL_F_S(ProcessBehaviorChart, SetLabelsAutoPrecision, bool, labelsAutoPrecision, updateLabels)
void ProcessBehaviorChart::setLabelsAutoPrecision(bool labelsAutoPrecision) {
	Q_D(ProcessBehaviorChart);
	if (labelsAutoPrecision != d->labelsAutoPrecision)
		exec(new ProcessBehaviorChartSetLabelsAutoPrecisionCmd(d, labelsAutoPrecision, ki18n("%1: set labels precision")));
}

STD_SETTER_CMD_IMPL_F_S(ProcessBehaviorChart, SetLabelsPrecision, int, labelsPrecision, updateLabels)
void ProcessBehaviorChart::setLabelsPrecision(int labelsPrecision) {
	Q_D(ProcessBehaviorChart);
	if (labelsPrecision != d->labelsPrecision)
		exec(new ProcessBehaviorChartSetLabelsPrecisionCmd(d, labelsPrecision, ki18n("%1: set labels precision")));
}

void ProcessBehaviorChart::setLabelsBorderShape(TextLabel::BorderShape shape) {
	Q_D(ProcessBehaviorChart);
	if (shape != d->centerLabel->borderShape()) {
		beginMacro(i18n("%1: set labels border shape", name()));
		d->centerLabel->setBorderShape(shape);
		d->upperLimitLabel->setBorderShape(shape);
		d->lowerLimitLabel->setBorderShape(shape);
		endMacro();
	}
}

void ProcessBehaviorChart::setLabelsFont(const QFont& font) {
	QDEBUG(Q_FUNC_INFO << ", FONT = " << font)
	Q_D(ProcessBehaviorChart);
	auto textWrapper = d->centerLabel->text();
	QTextEdit te(textWrapper.text);
	te.selectAll();
	if (font != te.font()) {
		beginMacro(i18n("%1: set labels font", name()));
		te.setFontFamily(font.family());
		te.setFontPointSize(font.pointSize());
		te.setFont(font);
		textWrapper.text = te.toHtml();
		d->centerLabel->setText(textWrapper);

		textWrapper = d->upperLimitLabel->text();
		te.setText(textWrapper.text);
		te.selectAll();
		te.setFontFamily(font.family());
		te.setFontPointSize(font.pointSize());
		te.setFont(font);
		textWrapper.text = te.toHtml();
		d->upperLimitLabel->setText(textWrapper);

		textWrapper = d->lowerLimitLabel->text();
		te.setText(textWrapper.text);
		te.selectAll();
		te.setFontFamily(font.family());
		te.setFontPointSize(font.pointSize());
		te.setFont(font);
		textWrapper.text = te.toHtml();
		d->lowerLimitLabel->setText(textWrapper);
		endMacro();
	}
}

void ProcessBehaviorChart::setLabelsFontColor(const QColor& color) {
	Q_D(ProcessBehaviorChart);
	auto textWrapper = d->centerLabel->text();
	QTextEdit te;
	te.setHtml(textWrapper.text);
	te.selectAll();
	if (color != te.textColor()) {
		beginMacro(i18n("%1: set labels background color", name()));
		te.setTextColor(color);
		textWrapper.text = te.toHtml();
		d->centerLabel->setText(textWrapper);
		d->centerLabel->setFontColor(color);

		textWrapper = d->upperLimitLabel->text();
		te.setText(textWrapper.text);
		te.selectAll();
		te.setTextColor(color);
		textWrapper.text = te.toHtml();
		d->upperLimitLabel->setText(textWrapper);
		d->upperLimitLabel->setFontColor(color);

		textWrapper = d->lowerLimitLabel->text();
		te.setHtml(textWrapper.text);
		te.selectAll();
		te.setTextColor(color);
		textWrapper.text = te.toHtml();
		d->lowerLimitLabel->setText(textWrapper);
		d->lowerLimitLabel->setFontColor(color);
		endMacro();
	}
}

void ProcessBehaviorChart::setLabelsBackgroundColor(const QColor& color) {
	Q_D(ProcessBehaviorChart);
	auto textWrapper = d->centerLabel->text();
	QTextEdit te;
	te.setHtml(textWrapper.text);
	te.selectAll();
	if (color != te.textBackgroundColor()) {
		beginMacro(i18n("%1: set labels background color", name()));
		te.setTextBackgroundColor(color);
		textWrapper.text = te.toHtml();
		d->centerLabel->setText(textWrapper);

		textWrapper = d->upperLimitLabel->text();
		te.setHtml(textWrapper.text);
		te.selectAll();
		te.setTextBackgroundColor(color);
		textWrapper.text = te.toHtml();
		d->upperLimitLabel->setText(textWrapper);

		textWrapper = d->lowerLimitLabel->text();
		te.setHtml(textWrapper.text);
		te.selectAll();
		te.setTextBackgroundColor(color);
		textWrapper.text = te.toHtml();
		d->lowerLimitLabel->setText(textWrapper);
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

void ProcessBehaviorChart::labelsBorderStyleChanged(Qt::PenStyle style) {
	Q_D(ProcessBehaviorChart);
	if (style != d->centerLabel->borderLine()->style()) {
		beginMacro(i18n("%1: set labels border style", name()));
		d->centerLabel->borderLine()->setStyle(style);
		d->upperLimitLabel->borderLine()->setStyle(style);
		d->lowerLimitLabel->borderLine()->setStyle(style);
		endMacro();
	}
}

void ProcessBehaviorChart::labelsBorderWidthChanged(double width) {
	Q_D(ProcessBehaviorChart);
	if (width != d->centerLabel->borderLine()->width()) {
		beginMacro(i18n("%1: set labels border width", name()));
		d->centerLabel->borderLine()->setWidth(width);
		d->upperLimitLabel->borderLine()->setWidth(width);
		d->lowerLimitLabel->borderLine()->setWidth(width);
		endMacro();
	}
}

void ProcessBehaviorChart::labelsBorderColorChanged(const QColor& color) {
	Q_D(ProcessBehaviorChart);
	if (color != d->centerLabel->borderLine()->color()) {
		beginMacro(i18n("%1: set labels border color", name()));
		d->centerLabel->borderLine()->setColor(color);
		d->upperLimitLabel->borderLine()->setColor(color);
		d->lowerLimitLabel->borderLine()->setColor(color);
		endMacro();
	}
}

void ProcessBehaviorChart::labelsBorderOpacityChanged(float opacity) {
	Q_D(ProcessBehaviorChart);
	if (opacity != d->centerLabel->borderLine()->opacity()) {
		beginMacro(i18n("%1: set labels border color", name()));
		d->centerLabel->borderLine()->setOpacity(opacity);
		d->upperLimitLabel->borderLine()->setOpacity(opacity);
		d->lowerLimitLabel->borderLine()->setOpacity(opacity);
		endMacro();
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

	// update the position of the value labels
	auto cs = q->plot()->coordinateSystem(q->coordinateSystemIndex());
	const auto& xRange = q->plot()->range(Dimension::X, cs->index(Dimension::X));
	double x = xRange.end();

	centerLabel->setUndoAware(false);
	upperLimitLabel->setUndoAware(false);
	lowerLimitLabel->setUndoAware(false);
	centerLabel->setPositionLogical(QPointF(x, center));
	upperLimitLabel->setPositionLogical(QPointF(x, upperLimit));
	lowerLimitLabel->setPositionLogical(QPointF(x, lowerLimit));
	centerLabel->retransform();
	upperLimitLabel->retransform();
	lowerLimitLabel->retransform();
	centerLabel->setUndoAware(true);
	upperLimitLabel->setUndoAware(true);
	lowerLimitLabel->setUndoAware(true);

	recalcShapeAndBoundingRect();
}

/*!
 * called when the source data was changed, recalculates the plot.
 */
void ProcessBehaviorChartPrivate::recalc() {
	PERFTRACE(name() + QLatin1String(Q_FUNC_INFO));
	const int count = q->xIndexCount();
	if (!dataColumn || ((type == ProcessBehaviorChart::Type::P || type == ProcessBehaviorChart::Type::U) && !data2Column) || count == 0) {
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

		// don't clear the labels while we're still loading and initilizing, the labels should keep their texts
		if (!q->isLoading()) {
			centerLabel->setText(QString());
			upperLimitLabel->setText(QString());
			lowerLimitLabel->setText(QString());
		}
		Q_EMIT q->dataChanged();
		Q_EMIT q->recalculated();

		// notify the dock widget if the sample size is bigger than the number of rows in the data column
		if (dataColumn && count == 0)
			Q_EMIT q->statusInfo(i18n("Not enough data provided."));

		return;
	}

	// supress retransforms in all internal curves while modifying the data,
	// everything will be retransformend at the very end
	dataCurve->setSuppressRetransform(true);
	centerCurve->setSuppressRetransform(true);
	upperLimitCurve->setSuppressRetransform(true);
	lowerLimitCurve->setSuppressRetransform(true);

	const int xMin = 1;
	const int xMax = count;
	xColumn->clear();
	xColumn->resizeTo(count);
	for (int i = 0; i < count; ++i)
		xColumn->setIntegerAt(i, i + 1);

	dataCurve->setXColumn(xColumn);

	// min and max labels for x
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

	updateLimitConstraints();
	updateControlLimits();

	dataCurve->setSuppressRetransform(false);
	centerCurve->setSuppressRetransform(false);
	upperLimitCurve->setSuppressRetransform(false);
	lowerLimitCurve->setSuppressRetransform(false);

	// emit dataChanged() in order to retransform everything with the new size/shape of the plot
	Q_EMIT q->dataChanged();
	Q_EMIT q->recalculated();
}

void ProcessBehaviorChartPrivate::updateLimitConstraints() {
	if (type == ProcessBehaviorChart::Type::mR || type == ProcessBehaviorChart::Type::R || type == ProcessBehaviorChart::Type::S
		|| type == ProcessBehaviorChart::Type::NP || type == ProcessBehaviorChart::Type::C || type == ProcessBehaviorChart::Type::U) {
		if (minLowerLimit != 0.) {
			minLowerLimit = 0.;
			Q_EMIT q->minLowerLimitChanged(0.);
		}
	}

	if (type == ProcessBehaviorChart::Type::NP) {
		if (maxUpperLimit > sampleSize) {
			maxUpperLimit = sampleSize;
			Q_EMIT q->maxUpperLimitChanged(maxUpperLimit);
		}
	} else if (type == ProcessBehaviorChart::Type::P) {
		if (minLowerLimit < 0. || minLowerLimit > 1.) {
			minLowerLimit = 0.;
			Q_EMIT q->minLowerLimitChanged(0.);
		}

		if (maxUpperLimit < 0. || maxUpperLimit > 1.) {
			maxUpperLimit = 1.;
			Q_EMIT q->maxUpperLimitChanged(1.);
		}
	}
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

	// determine the number of labels in the source data to be taken into account
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
			const double E2 = 3 / nsl_pcm_d2(2); // n = 2, two labels used to calculate the ranges
			upperLimit = mean + E2 * meanMovingRange;
			lowerLimit = mean - E2 * meanMovingRange;
		} else {
			// center line at the median of the data
			const double median = static_cast<const Column*>(dataColumn)->statistics().median;
			center = median;

			// upper and lower limits
			const double medianMovingRange = gsl_stats_median(movingRange.data(), 1, movingRange.size());
			const double E5 = 3 / nsl_pcm_d4(2); // n = 2, two labels used to calculate the ranges
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

	if (limitsType == ProcessBehaviorChart::LimitsType::Statistical) {
		// restrict the calculated limits to the min/max values for the current chart type:
		// for P and U chart limits are calculated for every individual point ("stair-step limits"),
		// for other charts straight lines are drawn
		if ((type == ProcessBehaviorChart::Type::P || type == ProcessBehaviorChart::Type::U) && exactLimitsEnabled) {
			for (int i = 0; i < yUpperLimitColumn->rowCount(); ++i) {
				if (yUpperLimitColumn->valueAt(i) > maxUpperLimit)
					yUpperLimitColumn->setValueAt(i, maxUpperLimit);
			}

			for (int i = 0; i < yLowerLimitColumn->rowCount(); ++i) {
				if (yLowerLimitColumn->valueAt(i) < minLowerLimit)
					yLowerLimitColumn->setValueAt(i, minLowerLimit);
			}

			upperLimitCurve->setLineType(XYCurve::LineType::MidpointHorizontal); // required for stair-step lines for P and U charts
			lowerLimitCurve->setLineType(XYCurve::LineType::MidpointHorizontal); // required for stair-step lines for P and U charts
		} else {
			if (lowerLimit < minLowerLimit)
				lowerLimit = minLowerLimit;
			if (upperLimit > maxUpperLimit)
				upperLimit = maxUpperLimit;

			yUpperLimitColumn->setValueAt(0, upperLimit);
			yUpperLimitColumn->setValueAt(1, upperLimit);
			yLowerLimitColumn->setValueAt(0, lowerLimit);
			yLowerLimitColumn->setValueAt(1, lowerLimit);
		}

		// show/hide the line for the lower limit depending on the chart type
		lowerLimitCurve->setUndoAware(false);
		if (type == ProcessBehaviorChart::Type::XmR || type == ProcessBehaviorChart::Type::XbarR || type == ProcessBehaviorChart::Type::XbarS)
			lowerLimitCurve->setVisible(true); // lower limit line is always visible
		else if (type == ProcessBehaviorChart::Type::mR || type == ProcessBehaviorChart::Type::R || type == ProcessBehaviorChart::Type::S
				 || type == ProcessBehaviorChart::Type::C) {
			if (lowerLimit == 0.)
				lowerLimitCurve->setVisible(false);
			else
				lowerLimitCurve->setVisible(true);
		}
		lowerLimitCurve->setUndoAware(true);

		yCenterColumn->setValueAt(0, center);
		yCenterColumn->setValueAt(1, center);

		updateLabels(); // update the texts in the value labels
	} else {
		// if "Specification" is selected, we use the values for center and limit lines that were specified by the user without any further checks
		// and just overwrite the values calculated above with the user-defined values
		updateSpecifications();
		// updateLabels() is called in updateModifications() above.
	}

	QDEBUG(Q_FUNC_INFO << ", center: " << center << " , upper limit: " << upperLimit << ", lower limit: " << lowerLimit);
}

void ProcessBehaviorChartPrivate::updateSpecifications() {
	center = centerSpecification;
	upperLimit = upperLimitSpecification;
	lowerLimit = lowerLimitSpecification;

	yCenterColumn->setValueAt(0, center);
	yCenterColumn->setValueAt(1, center);
	yUpperLimitColumn->setValueAt(0, upperLimit);
	yUpperLimitColumn->setValueAt(1, upperLimit);
	yLowerLimitColumn->setValueAt(0, lowerLimit);
	yLowerLimitColumn->setValueAt(1, lowerLimit);

	lowerLimitCurve->setUndoAware(false);
	lowerLimitCurve->setVisible(true);
	lowerLimitCurve->setUndoAware(true);

	updateLabels(); // update the texts in the value labels
}

/*!
 * updates the value labels, called after the values for center and limits were modified.
 */
void ProcessBehaviorChartPrivate::updateLabels() {
	// no need to update the labels during the load, the properties are set in label's load()
	if (!q->plot() || q->isLoading())
		return;

	centerLabel->setUndoAware(false);
	upperLimitLabel->setUndoAware(false);
	lowerLimitLabel->setUndoAware(false);

	const bool uniformLimitLabelsAvailable = !((type == ProcessBehaviorChart::Type::P || type == ProcessBehaviorChart::Type::U) && exactLimitsEnabled);
	const bool lowerLimitAvailable = q->lowerLimitAvailable();
	const bool validLimits = std::isfinite(center) && std::isfinite(upperLimit) && std::isfinite(lowerLimit);

	centerLabel->setVisible(labelsEnabled && validLimits);
	upperLimitLabel->setVisible(labelsEnabled && validLimits && uniformLimitLabelsAvailable);
	lowerLimitLabel->setVisible(labelsEnabled && validLimits && lowerLimitAvailable && uniformLimitLabelsAvailable);

	if (labelsEnabled && validLimits) {
		const auto numberLocale = QLocale();
		if (labelsAutoPrecision) {
			centerLabel->setText(numberLocale.toString(center));
			upperLimitLabel->setText(numberLocale.toString(upperLimit));
			if (lowerLimitAvailable)
				lowerLimitLabel->setText(numberLocale.toString(lowerLimit));
		} else {
			centerLabel->setText(numberLocale.toString(center, 'f', labelsPrecision));
			upperLimitLabel->setText(numberLocale.toString(upperLimit, 'f', labelsPrecision));
			if (lowerLimitAvailable)
				lowerLimitLabel->setText(numberLocale.toString(lowerLimit, 'f', labelsPrecision));
		}

		// update the position of the value labels
		auto cs = q->plot()->coordinateSystem(q->coordinateSystemIndex());
		const auto& xRange = q->plot()->range(Dimension::X, cs->index(Dimension::X));
		double x = xRange.end();
		centerLabel->setPositionLogical(QPointF(x, center));
		upperLimitLabel->setPositionLogical(QPointF(x, upperLimit));
		lowerLimitLabel->setPositionLogical(QPointF(x, lowerLimit));
	}

	centerLabel->setUndoAware(true);
	upperLimitLabel->setUndoAware(true);
	lowerLimitLabel->setUndoAware(true);

	recalcShapeAndBoundingRect();
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
	if (labelsEnabled) {
		const auto& linePen = labelsBorderLine->pen();
		auto path = WorksheetElement::shapeFromPath(centerLabel->graphicsItem()->mapToParent(centerLabel->graphicsItem()->shape()), linePen);
		m_shape.addPath(path);

		path = WorksheetElement::shapeFromPath(upperLimitLabel->graphicsItem()->mapToParent(upperLimitLabel->graphicsItem()->shape()), linePen);
		m_shape.addPath(path);

		path = WorksheetElement::shapeFromPath(lowerLimitLabel->graphicsItem()->mapToParent(lowerLimitLabel->graphicsItem()->shape()), linePen);
		m_shape.addPath(path);
	}

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
	writer->writeAttribute(QStringLiteral("limitsType"), QString::number(static_cast<int>(d->limitsType)));
	writer->writeAttribute(QStringLiteral("limitsMetric"), QString::number(static_cast<int>(d->limitsMetric)));
	writer->writeAttribute(QStringLiteral("sampleSize"), QString::number(d->sampleSize));
	writer->writeAttribute(QStringLiteral("minLowerLimit"), QString::number(d->minLowerLimit));
	writer->writeAttribute(QStringLiteral("maxUpperLimit"), QString::number(d->maxUpperLimit));
	writer->writeAttribute(QStringLiteral("exactLimitsEnabled"), QString::number(d->exactLimitsEnabled));
	writer->writeAttribute(QStringLiteral("centerSpecification"), QString::number(d->centerSpecification));
	writer->writeAttribute(QStringLiteral("lowerLimitSpecification"), QString::number(d->lowerLimitSpecification));
	writer->writeAttribute(QStringLiteral("upperLimitSpecification"), QString::number(d->upperLimitSpecification));
	writer->writeAttribute(QStringLiteral("visible"), QString::number(d->isVisible()));
	writer->writeAttribute(QStringLiteral("legendVisible"), QString::number(d->legendVisible));
	writer->writeEndElement();

	writer->writeStartElement(QStringLiteral("labels"));
	writer->writeAttribute(QStringLiteral("enabled"), QString::number(d->labelsEnabled));
	writer->writeAttribute(QStringLiteral("precision"), QString::number(d->labelsPrecision));
	writer->writeAttribute(QStringLiteral("autoPrecision"), QString::number(d->labelsAutoPrecision));
	d->labelsBorderLine->save(writer);
	writer->writeEndElement();

	d->centerLabel->save(writer);
	d->lowerLimitLabel->save(writer);
	d->upperLimitLabel->save(writer);

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
			READ_INT_VALUE("type", type, Type);
			READ_INT_VALUE("limitsType", limitsType, LimitsType);
			READ_INT_VALUE("limitsMetric", limitsMetric, LimitsMetric);
			READ_INT_VALUE("sampleSize", sampleSize, int);
			READ_DOUBLE_VALUE("minLowerLimit", minLowerLimit);
			READ_DOUBLE_VALUE("maxUpperLimit", maxUpperLimit);
			READ_INT_VALUE("exactLimitsEnabled", exactLimitsEnabled, bool);
			READ_DOUBLE_VALUE("centerSpecification", centerSpecification);
			READ_DOUBLE_VALUE("lowerLimitSpecification", lowerLimitSpecification);
			READ_DOUBLE_VALUE("upperLimitSpecification", upperLimitSpecification);
			READ_INT_VALUE("legendVisible", legendVisible, bool);

			str = attribs.value(QStringLiteral("visible")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("visible"));
			else
				d->setVisible(str.toInt());
		} else if (!preview && reader->name() == QLatin1String("labels")) {
			attribs = reader->attributes();
			READ_INT_VALUE("enabled", labelsEnabled, bool);
			READ_INT_VALUE("precision", labelsPrecision, int);
			READ_INT_VALUE("autoPrecision", labelsAutoPrecision, bool);
			d->labelsBorderLine->load(reader, preview);
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
		} else if (reader->name() == QLatin1String("textLabel")) {
			attribs = reader->attributes();
			bool rc = false;
			if (attribs.value(QStringLiteral("name")) == QLatin1String("centerValue"))
				rc = d->centerLabel->load(reader, preview);
			else if (attribs.value(QStringLiteral("name")) == QLatin1String("lowerLimitValue"))
				rc = d->lowerLimitLabel->load(reader, preview);
			else if (attribs.value(QStringLiteral("name")) == QLatin1String("upperLimitValue"))
				rc = d->upperLimitLabel->load(reader, preview);

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
	d->upperLimitCurve->line()->setStyle(Qt::DashLine);
	d->upperLimitCurve->symbol()->setStyle(Symbol::Style::NoSymbols);

	d->lowerLimitCurve->line()->loadThemeConfig(group, themeColor);
	d->lowerLimitCurve->line()->setStyle(Qt::DashLine);
	d->lowerLimitCurve->symbol()->setStyle(Symbol::Style::NoSymbols);

	d->centerLabel->loadThemeConfig(config);
	d->upperLimitLabel->loadThemeConfig(config);
	d->lowerLimitLabel->loadThemeConfig(config);

	group = config.group(QStringLiteral("CartesianPlot"));
	d->labelsBorderLine->loadThemeConfig(group);

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
