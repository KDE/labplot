/*
	File                 : ProcessBehaviorChart.cpp
	Project              : LabPlot
	Description          : ProcessBehaviorChart
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

/*!
  \class ProcessBehaviorChart
  \brief

  \ingroup worksheet
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
#include "backend/worksheet/plots/cartesian/Symbol.h"

#include <QMenu>
#include <QPainter>

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

#include <gsl/gsl_statistics.h>

CURVE_COLUMN_CONNECT(ProcessBehaviorChart, Data, data, recalc)

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

	// setUndoAware(false);

	KConfig config;
	KConfigGroup group = config.group(QStringLiteral("ProcessBehaviorChart"));

	// curve and columns for the data points
	d->dataCurve = new XYCurve(QStringLiteral("data"));
	d->dataCurve->setName(name(), AbstractAspect::NameHandling::UniqueNotRequired);
	d->dataCurve->setHidden(true);
	d->dataCurve->graphicsItem()->setParentItem(d);
	d->dataCurve->line()->init(group);
	d->dataCurve->line()->setStyle(Qt::SolidLine);
	d->dataCurve->symbol()->setStyle(Symbol::Style::Circle);
	d->dataCurve->background()->setPosition(Background::Position::No);

	d->xColumn = new Column(QStringLiteral("x"));
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

	d->xCenterColumn = new Column(QStringLiteral("xCenter"));
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

	d->xUpperLimitColumn = new Column(QStringLiteral("xUpperLimit"));
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

	d->xLowerLimitColumn = new Column(QStringLiteral("xLowerLimit"));
	d->xLowerLimitColumn->setHidden(true);
	d->xLowerLimitColumn->setUndoAware(false);
	addChildFast(d->xLowerLimitColumn);
	d->lowerLimitCurve->setXColumn(d->xLowerLimitColumn);

	d->yLowerLimitColumn = new Column(QStringLiteral("yLowerLimit"));
	d->yLowerLimitColumn->setHidden(true);
	d->yLowerLimitColumn->setUndoAware(false);
	addChildFast(d->yLowerLimitColumn);
	d->lowerLimitCurve->setYColumn(d->yLowerLimitColumn);

	// synchronize the names of the internal XYCurves with the name of the current plot
	// so we have the same name shown on the undo stack
	connect(this, &AbstractAspect::aspectDescriptionChanged, this, &ProcessBehaviorChart::renameInternalCurves);
}

void ProcessBehaviorChart::finalizeAdd() {
	Q_D(ProcessBehaviorChart);
	WorksheetElement::finalizeAdd();
	addChildFast(d->centerCurve);
	addChildFast(d->upperLimitCurve);
	addChildFast(d->lowerLimitCurve);
	addChildFast(d->dataCurve);
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

// ##############################################################################
// ##########################  getter methods  ##################################
// ##############################################################################
//  general
BASIC_SHARED_D_READER_IMPL(ProcessBehaviorChart, ProcessBehaviorChart::Type, type, type)
BASIC_SHARED_D_READER_IMPL(ProcessBehaviorChart, ProcessBehaviorChart::LimitsMetric, limitsMetric, limitsMetric)
BASIC_SHARED_D_READER_IMPL(ProcessBehaviorChart, int, subgroupSize, subgroupSize)
BASIC_SHARED_D_READER_IMPL(ProcessBehaviorChart, const AbstractColumn*, dataColumn, dataColumn)
BASIC_SHARED_D_READER_IMPL(ProcessBehaviorChart, QString, dataColumnPath, dataColumnPath)

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

bool ProcessBehaviorChart::usingColumn(const Column* column) const {
	Q_D(const ProcessBehaviorChart);
	return (d->dataColumn == column);
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

STD_SETTER_CMD_IMPL_F_S(ProcessBehaviorChart, SetSubgroupSize, int, subgroupSize, recalc)
void ProcessBehaviorChart::setSubgroupSize(int subgroupSize) {
	Q_D(ProcessBehaviorChart);
	if (subgroupSize != d->subgroupSize)
		exec(new ProcessBehaviorChartSetSubgroupSizeCmd(d, subgroupSize, ki18n("%1: set subgroup size")));
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
		CURVE_COLUMN_REMOVED(data);
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
	const bool suppressed = suppressRetransform || q->isLoading();
	if (suppressed)
		return;

	if (!isVisible())
		return;

	PERFTRACE(name() + QLatin1String(Q_FUNC_INFO));
	dataCurve->retransform();
	centerCurve->retransform();
	upperLimitCurve->retransform();
	lowerLimitCurve->retransform();
	recalcShapeAndBoundingRect();
}

/*!
 * called when the source data was changed, recalculates the plot.
 */
void ProcessBehaviorChartPrivate::recalc() {
	PERFTRACE(name() + QLatin1String(Q_FUNC_INFO));

	if (!dataColumn) {
		xCenterColumn->clear();
		yCenterColumn->clear();
		xUpperLimitColumn->clear();
		yUpperLimitColumn->clear();
		xLowerLimitColumn->clear();
		yLowerLimitColumn->clear();
		Q_EMIT q->dataChanged();
		return;
	}

	int count = 0;
	if (type == ProcessBehaviorChart::Type::XmR || type == ProcessBehaviorChart::Type::mR)
		count = dataColumn->rowCount();
	else
		count = dataColumn->rowCount() / subgroupSize;

	const double xMin = 1.;
	const double xMax = count;
	xColumn->clear();
	xColumn->resizeTo(count);
	for (int i = 0; i < count; ++i)
		xColumn->setValueAt(i, i + 1);

	dataCurve->setXColumn(xColumn);

	// min and max values for x
	xCenterColumn->setValueAt(0, xMin);
	xCenterColumn->setValueAt(1, xMax);
	xUpperLimitColumn->setValueAt(0, xMin);
	xUpperLimitColumn->setValueAt(1, xMax);
	xLowerLimitColumn->setValueAt(0, xMin);
	xLowerLimitColumn->setValueAt(1, xMax);

	updateControlLimits();

	// emit dataChanged() in order to retransform everything with the new size/shape of the plot
	Q_EMIT q->dataChanged();
}

/*!
 * conventions and definitions taken from Wheeler's "Making Use of Data"
 */
void ProcessBehaviorChartPrivate::updateControlLimits() {
	PERFTRACE(name() + QLatin1String(Q_FUNC_INFO));
	double center = 0.;
	double upperLimit = 0.;
	double lowerLimit = 0.;
	const int count = dataColumn->rowCount();

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
			const double E2 = 2.66; // 2.66 is 3 / d2 for n = 2 (two values used to calculate the ranges)
			upperLimit = mean + E2 * meanMovingRange;
			lowerLimit = mean - E2 * meanMovingRange;
		} else {
			// center line at the median of the data
			const double median = static_cast<const Column*>(dataColumn)->statistics().median;
			center = median;

			// upper and lower limits
			const double medianMovingRange = gsl_stats_median(movingRange.data(), 1, movingRange.size());
			const double E5 = 3.145; // 3.145 is 3 / d4 for n = 2 (two values used to calculate the ranges)
			upperLimit = median + E5 * medianMovingRange;
			lowerLimit = median - E5 * medianMovingRange;
		}

		if (lowerLimit < 0.)
			lowerLimit = 0.;

		// plotted data - original data
		dataCurve->setYColumn(dataColumn);

		break;
	}
	case ProcessBehaviorChart::Type::mR: {
		yColumn->clear();
		yColumn->resizeTo(count);

		// calculate the mean moving ranges
		for (int i = 1; i < count; ++i) {
			if (dataColumn->isValid(i) && !dataColumn->isMasked(i) && dataColumn->isValid(i - 1) && !dataColumn->isMasked(i - 1))
				yColumn->setValueAt(i - 1, std::abs(dataColumn->valueAt(i) - dataColumn->valueAt(i - 1)));
		}

		if (limitsMetric == ProcessBehaviorChart::LimitsMetric::Average) {
			// center line
			const double meanMovingRange = yColumn->statistics().arithmeticMean;
			center = meanMovingRange;

			// upper and lower limits
			upperLimit = 3.268 * meanMovingRange; // D4 = 3.268 for n = 2 // TODO: 267(page) or 268(appendix)?
			lowerLimit = 0.; // D3 = 0 for n = 2
		} else { // median
			// center line
			const double medianMovingRange = yColumn->statistics().median;
			center = medianMovingRange;

			// upper and lower limits
			upperLimit = 3.866 * medianMovingRange; // D6 = 3.866 for n = 2
			lowerLimit = 0.; // D5 = 0 for n = 2
		}

		// plotted data - moving ranges
		dataCurve->setYColumn(yColumn);

		break;
	}
	case ProcessBehaviorChart::Type::XbarR: {
		yColumn->clear();
		yColumn->resizeTo(subgroupSize);

		// calculate the mean for each subgroup
		int groupIndex = 0;
		for (int i = 0; i < count; i += subgroupSize) {
			double sum = 0.0;
			int j = 0;
			for (; j < subgroupSize && (i + j) < count; ++j) {
				if (dataColumn->isValid(i + j) && !dataColumn->isMasked(i + j))
					sum += dataColumn->valueAt(i + j);
			}

			yColumn->setValueAt(groupIndex, sum / subgroupSize);
			++groupIndex;
		}

		// Calculate the range for each subgroups
		std::vector<double> subgroupRanges;
		for (int i = 0; i < count; i += subgroupSize) {
			double minVal = dataColumn->valueAt(i);
			double maxVal = dataColumn->valueAt(i);
			for (int j = 1; j < subgroupSize && (i + j) < count; ++j) {
				if (dataColumn->isValid(i) && !dataColumn->isMasked(i)) {
					double val = dataColumn->valueAt(i + j);
					if (val < minVal)
						minVal = val;
					if (val > maxVal)
						maxVal = val;
				}
			}
			subgroupRanges.push_back(maxVal - minVal);
		}

		// center line at the mean of subgroup means ("grand average")
		const double meanOfMeans = yColumn->statistics().arithmeticMean;
		center = meanOfMeans;

		// upper and lower limits - the mean of means plus/minus normalized mean range
		if (limitsMetric == ProcessBehaviorChart::LimitsMetric::Average) {
			const double meanRange = gsl_stats_mean(subgroupRanges.data(), 1, subgroupRanges.size());
			const double A2 = nsl_pcm_A2(subgroupSize);
			upperLimit = meanOfMeans + A2 * meanRange;
			lowerLimit = meanOfMeans - A2 * meanRange;
		} else { // median
			const double medianRange = gsl_stats_median(subgroupRanges.data(), 1, subgroupRanges.size());
			const double A4 = nsl_pcm_A4(subgroupSize);
			upperLimit = meanOfMeans + A4 * medianRange;
			lowerLimit = meanOfMeans - A4 * medianRange;
		}

		// plotted data - means of subroups
		dataCurve->setYColumn(yColumn);

		break;
	}
	case ProcessBehaviorChart::Type::R: {
		yColumn->clear();
		yColumn->resizeTo(subgroupSize);

		// Calculate the range of subgroups
		int groupIndex = 0;
		for (int i = 0; i < count; i += subgroupSize) {
			double minVal = dataColumn->valueAt(i);
			double maxVal = dataColumn->valueAt(i);
			for (int j = 1; j < subgroupSize && (i + j) < count; ++j) {
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
			const double D3 = nsl_pcm_D3(subgroupSize);
			const double D4 = nsl_pcm_D4(subgroupSize);
			upperLimit = D4 * meanRange;
			lowerLimit = D3 * meanRange;
		} else { // median
			// center line at the median range
			const double medianRange = yColumn->statistics().median;
			center = medianRange;

			// upper and lower limits
			const double D5 = nsl_pcm_D5(subgroupSize);
			const double D6 = nsl_pcm_D6(subgroupSize);
			upperLimit = D6 * medianRange;
			lowerLimit = D5 * medianRange;
		}

		// plotted data - subroup ranges
		dataCurve->setYColumn(yColumn);

		break;
	}
	case ProcessBehaviorChart::Type::XbarS: { // chart based on the means and standard deviations for each subgroup
		// Calculate the mean of subgroups
		int groupIndex = 0;
		for (int i = 0; i < count; i += subgroupSize) {
			double sum = 0.0;
			for (int j = 0; j < subgroupSize && (i + j) < count; ++j) {
				if (dataColumn->isValid(i) && !dataColumn->isMasked(i))
					sum += dataColumn->valueAt(i + j);
			}

			yColumn->setValueAt(groupIndex, sum / subgroupSize);
			++groupIndex;
		}

		// Calculate the standard deviations of subgroups
		std::vector<double> subgroupStdDevs;
		for (int i = 0; i < count; i += subgroupSize) {
			std::vector<double> subgroup;
			for (int j = 0; j < subgroupSize && (i + j) < count; ++j) {
				if (dataColumn->isValid(i) && !dataColumn->isMasked(i))
					subgroup.push_back(dataColumn->valueAt(i + j));
			}
			const double stddev = gsl_stats_sd(subgroup.data(), 1, subgroup.size());
			subgroupStdDevs.push_back(stddev);
		}

		// center line at the mean of means
		const double meanOfMeans = yColumn->statistics().arithmeticMean;
		center = meanOfMeans;

		// upper and lower limits
		const double meanStdDev = gsl_stats_mean(subgroupStdDevs.data(), 1, subgroupStdDevs.size());
		const double A3 = nsl_pcm_A3(subgroupSize);
		upperLimit = meanOfMeans + A3 * meanStdDev;
		lowerLimit = meanOfMeans - A3 * meanStdDev;

		// plotted data - subroup means
		dataCurve->setYColumn(yColumn);

		break;
	}
	case ProcessBehaviorChart::Type::S: {
		yColumn->clear();
		yColumn->resizeTo(subgroupSize);

		// Calculate the standard deviation for each subgroup
		int groupIndex = 0;
		for (int i = 0; i < count; i += subgroupSize) {
			std::vector<double> subgroup;
			for (int j = 0; j < subgroupSize && (i + j) < count; ++j) {
				if (dataColumn->isValid(i + j) && !dataColumn->isMasked(i + j))
					subgroup.push_back(dataColumn->valueAt(i + j));
			}
			const double stddev = gsl_stats_sd(subgroup.data(), 1, subgroup.size());
			yColumn->setValueAt(groupIndex, stddev);
			++groupIndex;
		}

		// center line
		const double meanStdDev = yColumn->statistics().arithmeticMean;
		center = meanStdDev;

		// upper and lower limits
		const double B3 = nsl_pcm_B3(subgroupSize);
		const double B4 = nsl_pcm_B4(subgroupSize);
		upperLimit = B4 * meanStdDev;
		lowerLimit = B3 * meanStdDev;

		// plotted data - subroup standard deviations
		dataCurve->setYColumn(yColumn);

		break;
	}
	}

	yCenterColumn->setValueAt(0, center);
	yCenterColumn->setValueAt(1, center);
	yUpperLimitColumn->setValueAt(0, upperLimit);
	yUpperLimitColumn->setValueAt(1, upperLimit);
	yLowerLimitColumn->setValueAt(0, lowerLimit);
	yLowerLimitColumn->setValueAt(1, lowerLimit);
}

/*!
  recalculates the outer bounds and the shape of the curve.
  */
void ProcessBehaviorChartPrivate::recalcShapeAndBoundingRect() {
	if (suppressRecalc)
		return;

	prepareGeometryChange();
	m_shape = QPainterPath();
	m_shape.addPath(dataCurve->graphicsItem()->shape());
	m_shape.addPath(centerCurve->graphicsItem()->shape());
	m_shape.addPath(upperLimitCurve->graphicsItem()->shape());
	m_shape.addPath(lowerLimitCurve->graphicsItem()->shape());

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
	writer->writeAttribute(QStringLiteral("subgroupSize"), QString::number(d->subgroupSize));
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
			READ_INT_VALUE("subgroupSize", subgroupSize, int);
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

	const auto* plot = static_cast<const CartesianPlot*>(parentAspect());
	int index = plot->curveChildIndex(this);
	QColor themeColor = plot->themeColorPalette(index);

	Q_D(ProcessBehaviorChart);
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
