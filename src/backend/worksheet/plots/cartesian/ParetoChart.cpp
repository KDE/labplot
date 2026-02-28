/*
	File                 : ParetoChart.cpp
	Project              : LabPlot
	Description          : Run Chart
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2025-2026 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

/*!
  \class ParetoChart
  \brief

  \ingroup worksheet
  */
#include "ParetoChart.h"
#include "ParetoChartPrivate.h"
#include "backend/core/column/Column.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/macrosCurve.h"
#include "backend/lib/trace.h"
#include "backend/worksheet/Background.h"
#include "backend/worksheet/Line.h"
#include "backend/worksheet/plots/cartesian/BarPlot.h"
#include "backend/worksheet/plots/cartesian/Symbol.h"

#include <QIcon>

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>

// #include <gsl/gsl_statistics.h>

CURVE_COLUMN_CONNECT(ParetoChart, Data, data, recalc)

ParetoChart::ParetoChart(const QString& name)
	: Plot(name, new ParetoChartPrivate(this), AspectType::ParetoChart) {
	init();
}

ParetoChart::ParetoChart(const QString& name, ParetoChartPrivate* dd)
	: Plot(name, dd, AspectType::ParetoChart) {
	init();
}

// no need to delete the d-pointer here - it inherits from QGraphicsItem
// and is deleted during the cleanup in QGraphicsScene
ParetoChart::~ParetoChart() = default;

void ParetoChart::init() {
	Q_D(ParetoChart);

	KConfig config;
	KConfigGroup group = config.group(QStringLiteral("ParetoChart"));

	// column for the sorted frequencies used in the bar plot
	d->frequenciesColumn = new Column(QStringLiteral("frequencies"), AbstractColumn::ColumnMode::Integer);
	d->frequenciesColumn->setHidden(true);
	d->frequenciesColumn->setUndoAware(false);
	addChildFast(d->frequenciesColumn);

	// column with the text represenation of the sorted frequencies, used in the horizontal axis of the parent plot area
	d->labelsColumn = new Column(QStringLiteral("labels"), AbstractColumn::ColumnMode::Text);
	addChildFast(d->labelsColumn);

	// column for x and y values used in the line plot for the cumulative percentage
	d->xColumn = new Column(QStringLiteral("x"), AbstractColumn::ColumnMode::Double);
	d->xColumn->setHidden(true);
	d->xColumn->setUndoAware(false);
	addChildFast(d->xColumn);

	d->yColumn = new Column(QStringLiteral("y"), AbstractColumn::ColumnMode::Double);
	d->yColumn->setHidden(true);
	d->yColumn->setUndoAware(false);
	addChildFast(d->yColumn);

	// bar plot for the frequencies
	d->barPlot = new BarPlot(QString());
	d->barPlot->setHidden(true);
	d->barPlot->setOrientation(BarPlot::Orientation::Vertical);
	d->barPlot->graphicsItem()->setParentItem(d);

	// line plot for the cumulative percentage values
	d->linePlot = new XYCurve(QStringLiteral("data"));
	d->linePlot->setName(name(), AbstractAspect::NameHandling::UniqueNotRequired);
	d->linePlot->setHidden(true);
	d->linePlot->graphicsItem()->setParentItem(d);
	d->linePlot->line()->setStyle(Qt::SolidLine);
	d->linePlot->symbol()->setStyle(Symbol::Style::Circle);
	d->linePlot->setValuesType(XYCurve::ValuesType::Y);
	d->linePlot->setValuesPosition(XYCurve::ValuesPosition::Right);
	d->linePlot->setValuesDistance(Worksheet::convertToSceneUnits(10, Worksheet::Unit::Point));
	d->linePlot->setValuesSuffix(QStringLiteral("%"));

	// set the columns in the plots
	d->barPlot->setDataColumns({d->frequenciesColumn});
	d->linePlot->setXColumn(d->xColumn);
	d->linePlot->setYColumn(d->yColumn);

	// synchronize the names of the internal XYCurves with the name of the current plot
	// so we have the same name shown on the undo stack
	connect(this, &AbstractAspect::aspectDescriptionChanged, this, &ParetoChart::renameInternalCurves);
}

void ParetoChart::finalizeAdd() {
	Q_D(ParetoChart);
	WorksheetElement::finalizeAdd();
	addChildFast(d->barPlot);
	addChildFast(d->linePlot);
	d->linePlot->setCoordinateSystemIndex(1); // assign to the second y-range going from 0 to 100%
}

void ParetoChart::renameInternalCurves() {
	Q_D(ParetoChart);
	d->barPlot->setName(name(), AbstractAspect::NameHandling::UniqueNotRequired);
	d->linePlot->setName(name(), AbstractAspect::NameHandling::UniqueNotRequired);
}

/*!
  Returns an icon to be used in the project explorer.
  */
QIcon ParetoChart::icon() const {
	// TODO: set the correct icon
	return QIcon::fromTheme(QStringLiteral("view-object-histogram-linear"));
}

void ParetoChart::handleResize(double /*horizontalRatio*/, double /*verticalRatio*/, bool /*pageResize*/) {
	// TODO
}

void ParetoChart::setVisible(bool on) {
	Q_D(ParetoChart);
	beginMacro(on ? i18n("%1: set visible", name()) : i18n("%1: set invisible", name()));
	d->barPlot->setVisible(on);
	d->linePlot->setVisible(on);
	WorksheetElement::setVisible(on);
	endMacro();
}

// ##############################################################################
// ##########################  getter methods  ##################################
// ##############################################################################
//  general
BASIC_SHARED_D_READER_IMPL(ParetoChart, const AbstractColumn*, dataColumn, dataColumn)
BASIC_SHARED_D_READER_IMPL(ParetoChart, QString, dataColumnPath, dataColumnPath)
BASIC_SHARED_D_READER_IMPL(ParetoChart, const AbstractColumn*, labelsColumn, labelsColumn)
BASIC_SHARED_D_READER_IMPL(ParetoChart, QString, labelsColumnPath, labelsColumnPath)

/*!
 * returns the number of index values used for x.
 */
int ParetoChart::xIndexCount() const {
	Q_D(const ParetoChart);
	if (!d->dataColumn)
		return 0;

	return d->frequenciesColumn->rowCount();
}

Line* ParetoChart::barLine() const {
	Q_D(const ParetoChart);
	return d->barPlot->lineAt(0);
}

Background* ParetoChart::barBackground() const {
	Q_D(const ParetoChart);
	return d->barPlot->backgroundAt(0);
}

Line* ParetoChart::line() const {
	Q_D(const ParetoChart);
	return d->linePlot->line();
}

Symbol* ParetoChart::symbol() const {
	Q_D(const ParetoChart);
	return d->linePlot->symbol();
}

bool ParetoChart::indicesMinMax(Dimension, double, double, int& start, int& end) const {
	start = 0;
	end = xIndexCount() - 1;
	return true;
}

bool ParetoChart::multiMinMax(int csIndex, Dimension dim, const Range<int>& indexRange, Range<double>& r, bool /* includeErrorBars */) const {
	Q_D(const ParetoChart);
	if (csIndex == 0 && dim == Dimension::Y)
		return d->barPlot->minMax(dim, indexRange, r, false);
	else
		return d->linePlot->minMax(dim, indexRange, r, false);
}

bool ParetoChart::minMax(Dimension dim, const Range<int>& indexRange, Range<double>& r, bool /* includeErrorBars */) const {
	Q_D(const ParetoChart);
	return d->linePlot->minMax(dim, indexRange, r, false);
}

double ParetoChart::minimum(Dimension dim) const {
	Q_D(const ParetoChart);
	return d->linePlot->minimum(dim);
}

double ParetoChart::maximum(Dimension dim) const {
	Q_D(const ParetoChart);
	return d->linePlot->maximum(dim);
}

bool ParetoChart::hasData() const {
	Q_D(const ParetoChart);
	return (d->dataColumn != nullptr);
}

int ParetoChart::dataCount(Dimension) const {
	Q_D(const ParetoChart);
	if (!d->dataColumn)
		return -1;
	return d->dataColumn->rowCount();
}

bool ParetoChart::usingColumn(const AbstractColumn* column, bool) const {
	Q_D(const ParetoChart);
	return (d->dataColumn == column);
}

void ParetoChart::handleAspectUpdated(const QString& aspectPath, const AbstractAspect* aspect) {
	Q_D(ParetoChart);

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

QColor ParetoChart::color() const {
	Q_D(const ParetoChart);
	return d->linePlot->color();
}

// ##############################################################################
// #################  setter methods and undo commands ##########################
// ##############################################################################

// General
CURVE_COLUMN_SETTER_CMD_IMPL_F_S(ParetoChart, Data, data, recalc)
void ParetoChart::setDataColumn(const AbstractColumn* column) {
	Q_D(ParetoChart);
	if (column != d->dataColumn)
		exec(new ParetoChartSetDataColumnCmd(d, column, ki18n("%1: set data column")));
}

void ParetoChart::setDataColumnPath(const QString& path) {
	Q_D(ParetoChart);
	d->dataColumnPath = path;
}

// ##############################################################################
// #################################  SLOTS  ####################################
// ##############################################################################
void ParetoChart::retransform() {
	D(ParetoChart);
	d->retransform();
}

void ParetoChart::recalc() {
	D(ParetoChart);
	d->recalc();
}

void ParetoChart::dataColumnAboutToBeRemoved(const AbstractAspect* aspect) {
	Q_D(ParetoChart);
	if (aspect == d->dataColumn) {
		d->dataColumn = nullptr;
		CURVE_COLUMN_REMOVED(data);
	}
}

// ##############################################################################
// ######################### Private implementation #############################
// ##############################################################################
ParetoChartPrivate::ParetoChartPrivate(ParetoChart* owner)
	: PlotPrivate(owner)
	, q(owner) {
	setFlag(QGraphicsItem::ItemIsSelectable, true);
	setAcceptHoverEvents(false);
}

ParetoChartPrivate::~ParetoChartPrivate() {
}

/*!
  called when the size of the plot or its data ranges (manual changes, zooming, etc.) were changed.
  recalculates the position of the scene points to be drawn.
  triggers the update of lines, drop lines, symbols etc.
*/
void ParetoChartPrivate::retransform() {
	const bool suppressed = suppressRetransform || q->isLoading();
	if (suppressed)
		return;

	if (!isVisible())
		return;

	PERFTRACE(name() + QLatin1String(Q_FUNC_INFO));
	barPlot->retransform();
	linePlot->retransform();
	recalcShapeAndBoundingRect();
}

/*!
 * called when the source data was changed, recalculates the plot.
 */
void ParetoChartPrivate::recalc() {
	PERFTRACE(name() + QLatin1String(Q_FUNC_INFO));
	if (!dataColumn) {
		frequenciesColumn->clear();
		xColumn->clear();
		yColumn->clear();
		Q_EMIT q->dataChanged();
		return;
	}

	// supress retransforms in all internal curves while modifying the data,
	// everything will be retransformend at the very end
	barPlot->setSuppressRetransform(true);
	linePlot->setSuppressRetransform(true);

	// sort the frequencies and the accompanying labels and calculate the total sum of frequencies
	const auto& frequencies = static_cast<const Column*>(dataColumn)->frequencies();
	const int count = frequencies.count();
	QVector<double> xData(count);
	QVector<double> yData(count);

	auto i = frequencies.constBegin();
	QVector<QPair<QString, int>> pairs;
	int row = 0;
	int totalSumOfFrequencies = 0;
	while (i != frequencies.constEnd()) {
		pairs << QPair<QString, int>(i.key(), i.value());
		xData[row] = 0.5 + row;
		totalSumOfFrequencies += i.value();
		++row;
		++i;
	}

	std::sort(pairs.begin(), pairs.end(), [](QPair<QString, int> a, QPair<QString, int> b) {
		return a.second > b.second;
	});

	QVector<int> frequenciesData;
	QVector<QString> labelsData;
	for (const auto& pair : pairs) {
		labelsData << pair.first;
		frequenciesData << pair.second;
	}

	// calculate the cumulative values
	int sum = 0;
	row = 0;
	for (auto value : frequenciesData) {
		sum += value;
		if (totalSumOfFrequencies != 0)
			yData[row] = (double)sum / totalSumOfFrequencies * 100;
		++row;
	}

	frequenciesColumn->setIntegers(frequenciesData);
	labelsColumn->setText(labelsData);
	xColumn->setValues(xData);
	yColumn->setValues(yData);

	barPlot->setSuppressRetransform(false);
	linePlot->setSuppressRetransform(false);

	// emit dataChanged() in order to retransform everything with the new size/shape of the plot
	Q_EMIT q->dataChanged();
}

/*!
  recalculates the outer bounds and the shape of the curve.
  */
void ParetoChartPrivate::recalcShapeAndBoundingRect() {
	if (suppressRecalc)
		return;

	prepareGeometryChange();
	m_shape = QPainterPath();
	m_shape.addPath(barPlot->graphicsItem()->shape());
	m_shape.addPath(linePlot->graphicsItem()->shape());

	m_boundingRectangle = m_shape.boundingRect();
}

// ##############################################################################
// ##################  Serialization/Deserialization  ###########################
// ##############################################################################
//! Save as XML
void ParetoChart::save(QXmlStreamWriter* writer) const {
	Q_D(const ParetoChart);

	writer->writeStartElement(QStringLiteral("ParetoChart"));
	writeBasicAttributes(writer);
	writeCommentElement(writer);

	// general
	writer->writeStartElement(QStringLiteral("general"));
	WRITE_COLUMN(d->dataColumn, dataColumn);
	WRITE_COLUMN(d->frequenciesColumn, frequenciesColumn);
	WRITE_COLUMN(d->xColumn, xColumn);
	WRITE_COLUMN(d->yColumn, yColumn);
	writer->writeAttribute(QStringLiteral("visible"), QString::number(d->isVisible()));
	writer->writeAttribute(QStringLiteral("legendVisible"), QString::number(d->legendVisible));
	writer->writeEndElement();

	// save the internal columns, above only the references to them were saved
	d->frequenciesColumn->save(writer);
	d->xColumn->save(writer);
	d->yColumn->save(writer);

	// save the internal curves
	// disconnect temporarily from renameInternalCurves so we can use unique names to be able to properly load the curves later
	disconnect(this, &AbstractAspect::aspectDescriptionChanged, this, &ParetoChart::renameInternalCurves);
	d->barPlot->setName(QStringLiteral("barPlot"));
	d->barPlot->save(writer);
	d->linePlot->setName(QStringLiteral("linePlot"));
	d->linePlot->save(writer);
	connect(this, &AbstractAspect::aspectDescriptionChanged, this, &ParetoChart::renameInternalCurves);

	writer->writeEndElement(); // close "ParetoChart" section
}

//! Load from XML
bool ParetoChart::load(XmlStreamReader* reader, bool preview) {
	Q_D(ParetoChart);

	if (!readBasicAttributes(reader))
		return false;

	QXmlStreamAttributes attribs;
	QString str;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == QLatin1String("ParetoChart"))
			break;

		if (!reader->isStartElement())
			continue;

		if (reader->name() == QLatin1String("comment")) {
			if (!readCommentElement(reader))
				return false;
		} else if (!preview && reader->name() == QLatin1String("general")) {
			attribs = reader->attributes();
			READ_COLUMN(dataColumn);
			READ_COLUMN(frequenciesColumn);
			READ_COLUMN(xColumn);
			READ_COLUMN(yColumn);

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
				rc = d->xColumn->load(reader, preview);
			else if (name == QLatin1String("frequencies"))
				rc = d->frequenciesColumn->load(reader, preview);

			if (!rc)
				return false;
		} else if (reader->name() == QLatin1String("barPlot")) {
			d->barPlot->setIsLoading(true);
			if (!d->barPlot->load(reader, preview))
				return false;
		} else if (reader->name() == QLatin1String("linePlot")) {
			d->linePlot->setIsLoading(true);
			if (!d->linePlot->load(reader, preview))
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
void ParetoChart::loadThemeConfig(const KConfig& config) {
	KConfigGroup group;
	if (config.hasGroup(QStringLiteral("Theme")))
		group = config.group(QStringLiteral("XYCurve")); // when loading from the theme config, use the same properties as for XYCurve
	else
		group = config.group(QStringLiteral("ParetoChart"));

	const auto* plot = static_cast<const CartesianPlot*>(parentAspect());
	int index = plot->curveChildIndex(this);
	QColor themeColor = plot->plotColor(index);

	Q_D(ParetoChart);
	d->suppressRecalc = true;
	d->barPlot->loadThemeConfig(config); // TODO
	d->linePlot->line()->loadThemeConfig(group, themeColor);
	d->linePlot->symbol()->loadThemeConfig(group, themeColor);

	themeColor = plot->plotColor(index + 1);

	d->suppressRecalc = false;
	d->recalcShapeAndBoundingRect();
}

void ParetoChart::saveThemeConfig(const KConfig& config) {
	Q_D(const ParetoChart);
	KConfigGroup group = config.group(QStringLiteral("ParetoChart"));
	// TODO: d->barPlot->saveThemeConfig(group);
	d->linePlot->line()->saveThemeConfig(group);
	d->linePlot->symbol()->saveThemeConfig(group);
}
