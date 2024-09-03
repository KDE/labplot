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

CURVE_COLUMN_CONNECT(ProcessBehaviorChart, XData, xData, recalc)
CURVE_COLUMN_CONNECT(ProcessBehaviorChart, YData, yData, recalc)

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

	// curve for the data points
	d->dataCurve = new XYCurve(QStringLiteral("data"));
	d->dataCurve->setName(name(), AbstractAspect::NameHandling::UniqueNotRequired);
	d->dataCurve->setHidden(true);
	d->dataCurve->graphicsItem()->setParentItem(d);
	d->dataCurve->line()->init(group);
	d->dataCurve->line()->setStyle(Qt::SolidLine);
	d->dataCurve->symbol()->setStyle(Symbol::Style::Circle);
	d->dataCurve->background()->setPosition(Background::Position::No);

	// curve for the central line
	d->centerCurve = new XYCurve(QStringLiteral("center"));
	d->centerCurve->setName(name(), AbstractAspect::NameHandling::UniqueNotRequired);
	d->centerCurve->setHidden(true);
	d->centerCurve->graphicsItem()->setParentItem(d);
	d->centerCurve->line()->init(group);
	d->centerCurve->line()->setStyle(Qt::SolidLine);
	d->centerCurve->symbol()->setStyle(Symbol::Style::NoSymbols);
	d->centerCurve->background()->setPosition(Background::Position::No);

	// curve for the upper and lower limit lines
	d->upperLimitCurve = new XYCurve(QStringLiteral("upper limit"));
	d->upperLimitCurve->setName(name(), AbstractAspect::NameHandling::UniqueNotRequired);
	d->upperLimitCurve->setHidden(true);
	d->upperLimitCurve->graphicsItem()->setParentItem(d);
	d->upperLimitCurve->line()->init(group);
	d->upperLimitCurve->line()->setStyle(Qt::SolidLine);
	d->upperLimitCurve->symbol()->setStyle(Symbol::Style::NoSymbols);
	d->upperLimitCurve->background()->setPosition(Background::Position::No);

	d->lowerLimitCurve = new XYCurve(QStringLiteral("lower limit"));
	d->lowerLimitCurve->setName(name(), AbstractAspect::NameHandling::UniqueNotRequired);
	d->lowerLimitCurve->setHidden(true);
	d->lowerLimitCurve->graphicsItem()->setParentItem(d);
	d->lowerLimitCurve->line()->init(group);
	d->lowerLimitCurve->line()->setStyle(Qt::SolidLine);
	d->lowerLimitCurve->symbol()->setStyle(Symbol::Style::NoSymbols);
	d->lowerLimitCurve->background()->setPosition(Background::Position::No);

	// synchronize the names of the internal XYCurves with the name of the current q-q plot
	// so we have the same name shown on the undo stack
	connect(this, &AbstractAspect::aspectDescriptionChanged, [this] {
		Q_D(ProcessBehaviorChart);
		d->centerCurve->setName(name(), AbstractAspect::NameHandling::UniqueNotRequired);
		d->upperLimitCurve->setName(name(), AbstractAspect::NameHandling::UniqueNotRequired);
		d->lowerLimitCurve->setName(name(), AbstractAspect::NameHandling::UniqueNotRequired);
	});
}

void ProcessBehaviorChart::finalizeAdd() {
	Q_D(ProcessBehaviorChart);
	WorksheetElement::finalizeAdd();
	addChildFast(d->dataCurve);
	addChildFast(d->centerCurve);
	addChildFast(d->upperLimitCurve);
	addChildFast(d->lowerLimitCurve);
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
BASIC_SHARED_D_READER_IMPL(ProcessBehaviorChart, const AbstractColumn*, xDataColumn, xDataColumn)
BASIC_SHARED_D_READER_IMPL(ProcessBehaviorChart, QString, xDataColumnPath, xDataColumnPath)
BASIC_SHARED_D_READER_IMPL(ProcessBehaviorChart, const AbstractColumn*, yDataColumn, yDataColumn)
BASIC_SHARED_D_READER_IMPL(ProcessBehaviorChart, QString, yDataColumnPath, yDataColumnPath)

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
		// TODO
		/*
		Range referenceRange(r);
		Range percentilesRange(r);
		bool rc = true;
		rc = d->referenceCurve->minMax(dim, indexRange, referenceRange, false);
		if (!rc)
			return false;

		rc = d->percentilesCurve->minMax(dim, indexRange, percentilesRange, false);
		if (!rc)
			return false;

		r.setStart(std::min(referenceRange.start(), percentilesRange.start()));
		r.setEnd(std::max(referenceRange.end(), percentilesRange.end()));
		*/
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
	// TODO
		// return std::min(d->referenceCurve->minimum(dim), d->percentilesCurve->minimum(dim));
		return NAN;
	}
	return NAN;
}

double ProcessBehaviorChart::maximum(const Dimension dim) const {
	Q_D(const ProcessBehaviorChart);
	switch (dim) {
	case Dimension::X:
		return d->dataCurve->maximum(dim);
	case Dimension::Y:
	// TODO
		// return std::max(d->referenceCurve->maximum(dim), d->percentilesCurve->maximum(dim));
		return NAN;
	}
	return NAN;
}

bool ProcessBehaviorChart::hasData() const {
	Q_D(const ProcessBehaviorChart);
	return (d->yDataColumn != nullptr);
}

bool ProcessBehaviorChart::usingColumn(const Column* column) const {
	Q_D(const ProcessBehaviorChart);
	return (d->xDataColumn == column || d->yDataColumn == column);
}

void ProcessBehaviorChart::handleAspectUpdated(const QString& aspectPath, const AbstractAspect* aspect) {
	Q_D(ProcessBehaviorChart);

	const auto column = dynamic_cast<const AbstractColumn*>(aspect);
	if (!column)
		return;

	if (d->xDataColumn == column) // the column is the same and was just renamed -> update the column path
		d->xDataColumnPath = aspectPath;
	else if (d->xDataColumnPath == aspectPath) { // another column was renamed to the current path -> set and connect to the new column
		setUndoAware(false);
		setXDataColumn(column);
		setUndoAware(true);
	}

	if (d->yDataColumn == column) // the column is the same and was just renamed -> update the column path
		d->yDataColumnPath = aspectPath;
	else if (d->yDataColumnPath == aspectPath) { // another column was renamed to the current path -> set and connect to the new column
		setUndoAware(false);
		setYDataColumn(column);
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
CURVE_COLUMN_SETTER_CMD_IMPL_F_S(ProcessBehaviorChart, XData, xData, recalc)
void ProcessBehaviorChart::setXDataColumn(const AbstractColumn* column) {
	Q_D(ProcessBehaviorChart);
	if (column != d->xDataColumn)
		exec(new ProcessBehaviorChartSetXDataColumnCmd(d, column, ki18n("%1: set x data column")));
}

void ProcessBehaviorChart::setXDataColumnPath(const QString& path) {
	Q_D(ProcessBehaviorChart);
	d->xDataColumnPath = path;
}

CURVE_COLUMN_SETTER_CMD_IMPL_F_S(ProcessBehaviorChart, YData, yData, recalc)
void ProcessBehaviorChart::setYDataColumn(const AbstractColumn* column) {
	Q_D(ProcessBehaviorChart);
	if (column != d->yDataColumn)
		exec(new ProcessBehaviorChartSetYDataColumnCmd(d, column, ki18n("%1: set y data column")));
}

void ProcessBehaviorChart::setYDataColumnPath(const QString& path) {
	Q_D(ProcessBehaviorChart);
	d->yDataColumnPath = path;
}

STD_SETTER_CMD_IMPL_F_S(ProcessBehaviorChart, SetType, ProcessBehaviorChart::Type, type, recalc)
void ProcessBehaviorChart::setType(ProcessBehaviorChart::Type type) {
	Q_D(ProcessBehaviorChart);
	if (type != d->type)
		exec(new ProcessBehaviorChartSetTypeCmd(d, type, ki18n("%1: set type")));
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

void ProcessBehaviorChart::xDataColumnAboutToBeRemoved(const AbstractAspect* aspect) {
	Q_D(ProcessBehaviorChart);
	if (aspect == d->xDataColumn) {
		d->xDataColumn = nullptr;
		CURVE_COLUMN_REMOVED(xData);
	}
}

void ProcessBehaviorChart::yDataColumnAboutToBeRemoved(const AbstractAspect* aspect) {
	Q_D(ProcessBehaviorChart);
	if (aspect == d->yDataColumn) {
		d->yDataColumn = nullptr;
		CURVE_COLUMN_REMOVED(yData);
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

	if (!yDataColumn) {
		xCenterColumn->clear();
		yCenterColumn->clear();
		xUpperLimitColumn->clear();
		yUpperLimitColumn->clear();
		xLowerLimitColumn->clear();
		yLowerLimitColumn->clear();
		Q_EMIT q->dataChanged();
		return;
	}

	// if (!xDataColumn) {
	// 	// no column for x provided, use the index for x
	// 	xDataColumn->clear();
	// 	const int count = yDataColumn->rowCount();
	// 	// TODO xDataColumn->setRowsCount(count);
	// 	for (int i = 0; i < count; ++i)
	// 		xDataColumn->setValueAt(i, i + 1);
	// }

	// min and max values for x
	const auto& statistics = static_cast<const Column*>(xDataColumn)->statistics();
	const double xMin = statistics.minimum;
	const double xMax = statistics.maximum;
	xCenterColumn->setValueAt(0, xMin);
	xCenterColumn->setValueAt(1, xMax);
	xUpperLimitColumn->setValueAt(0, xMin);
	xUpperLimitColumn->setValueAt(1, xMax);
	xLowerLimitColumn->setValueAt(0, xMin);
	xLowerLimitColumn->setValueAt(1, xMax);

	updateControlLimits();

	// Q_EMIT dataChanged() in order to retransform everything with the new size/shape of the plot
	Q_EMIT q->dataChanged();
}

/*
#include <vector>
#include <cmath>
#include <gsl/gsl_statistics.h>

void ProcessBehaviorChart::recalcMeanAndStandardDeviation() {
    if (data.empty()) {
        mean = 0.0;
        standardDeviation = 0.0;
        return;
    }

    double sum = 0.0;
    for (double value : data) {
        sum += value;
    }
    mean = sum / data.size();

    double varianceSum = 0.0;
    for (double value : data) {
        varianceSum += (value - mean) * (value - mean);
    }
    standardDeviation = std::sqrt(varianceSum / data.size());
}
*/

/*
Explanation:
XmR: Calculates the mean moving range and sets the control limits based on the mean and the mean moving range.
XbarR: Calculates the mean of subgroups and the range of subgroups, then sets the control limits based on these values.
XbarS: Calculates the mean of subgroups and the standard deviation of subgroups, then sets the control limits based on these values.
P: Calculates the proportion of defectives and sets the control limits based on this proportion.
NP: Calculates the number of defectives and sets the control limits based on this number.
C: Calculates the average number of defects per unit and sets the control limits based on this average.
U: Calculates the average number of defects per unit and sets the control limits based on this average.
This implementation assumes that the necessary constants (like d2 and c4) are known and used correctly for the given subgroup sizes. Adjust the constants as needed based on the actual subgroup sizes and control chart requirements.
*/
void ProcessBehaviorChartPrivate::updateControlLimits() {
	switch (type) {
	case ProcessBehaviorChart::Type::XmR: {
		const auto& statistics = static_cast<const Column*>(xDataColumn)->statistics();

		// center line
		const double mean = statistics.arithmeticMean;
		yCenterColumn->setValueAt(0, mean);
		yCenterColumn->setValueAt(1, mean);

		// calculate the mean moving range
		std::vector<double> movingRange;
		for (int i = 1; i < yDataColumn->rowCount(); ++i)
			movingRange.push_back(std::abs(yDataColumn->valueAt(i) - yDataColumn->valueAt(i - 1)));

		const double meanMovingRange = gsl_stats_mean(movingRange.data(), 1, movingRange.size());

		// upper and lower limits
		const double upperLimit = statistics.arithmeticMean + 3. * meanMovingRange / 1.128;
		const double lowerLimit = statistics.arithmeticMean - 3. * meanMovingRange / 1.128;
		yUpperLimitColumn->setValueAt(0, upperLimit);
		yUpperLimitColumn->setValueAt(1, upperLimit);
		yLowerLimitColumn->setValueAt(0, lowerLimit);
		yLowerLimitColumn->setValueAt(1, lowerLimit);
		break;
	}
	case ProcessBehaviorChart::Type::mR: {
		// calculate the mean moving range
		std::vector<double> movingRange;
		for (int i = 1; i < yDataColumn->rowCount(); ++i)
			movingRange.push_back(std::abs(yDataColumn->valueAt(i) - yDataColumn->valueAt(i - 1)));

		const double meanMovingRange = gsl_stats_mean(movingRange.data(), 1, movingRange.size());

		// center line
		yCenterColumn->setValueAt(0, meanMovingRange);
		yCenterColumn->setValueAt(1, meanMovingRange);

		// upper and lower limits
		const double upperLimit = 0;
		const double lowerLimit = 3.2665 * meanMovingRange;
		yUpperLimitColumn->setValueAt(0, upperLimit);
		yUpperLimitColumn->setValueAt(1, upperLimit);
		yLowerLimitColumn->setValueAt(0, lowerLimit);
		yLowerLimitColumn->setValueAt(1, lowerLimit);
		break;
	}
	case ProcessBehaviorChart::Type::XbarR: {
		// Calculate the mean of subgroups
		std::vector<double> subgroupMeans;
		for (int i = 0; i < yDataColumn->rowCount(); i += subgroupSize) {
			double sum = 0.0;
			for (int j = 0; j < subgroupSize && (i + j) < yDataColumn->rowCount(); ++j) {
				sum += yDataColumn->valueAt(i + j);
			}
			subgroupMeans.push_back(sum / subgroupSize);
		}

		const double meanOfMeans = gsl_stats_mean(subgroupMeans.data(), 1, subgroupMeans.size());
		yCenterColumn->setValueAt(0, meanOfMeans);
		yCenterColumn->setValueAt(1, meanOfMeans);

		// Calculate the range of subgroups
		std::vector<double> subgroupRanges;
		for (int i = 0; i < yDataColumn->rowCount(); i += subgroupSize) {
			double minVal = yDataColumn->valueAt(i);
			double maxVal = yDataColumn->valueAt(i);
			for (int j = 1; j < subgroupSize && (i + j) < yDataColumn->rowCount(); ++j) {
				double val = yDataColumn->valueAt(i + j);
				if (val < minVal) minVal = val;
				if (val > maxVal) maxVal = val;
			}
			subgroupRanges.push_back(maxVal - minVal);
		}

		const double meanRange = gsl_stats_mean(subgroupRanges.data(), 1, subgroupRanges.size());
		const double d2 = 2.326; // d2 constant for subgroup size 5
		const double upperLimit = meanOfMeans + 3. * meanRange / d2;
		const double lowerLimit = meanOfMeans - 3. * meanRange / d2;
		yUpperLimitColumn->setValueAt(0, upperLimit);
		yUpperLimitColumn->setValueAt(1, upperLimit);
		yLowerLimitColumn->setValueAt(0, lowerLimit);
		yLowerLimitColumn->setValueAt(1, lowerLimit);
		break;
	}
	case ProcessBehaviorChart::Type::XbarS: {
		// Calculate the mean of subgroups
		std::vector<double> subgroupMeans;
		for (int i = 0; i < yDataColumn->rowCount(); i += subgroupSize) {
			double sum = 0.0;
			for (int j = 0; j < subgroupSize && (i + j) < yDataColumn->rowCount(); ++j) {
				sum += yDataColumn->valueAt(i + j);
			}
			subgroupMeans.push_back(sum / subgroupSize);
		}

		const double meanOfMeans = gsl_stats_mean(subgroupMeans.data(), 1, subgroupMeans.size());
		yCenterColumn->setValueAt(0, meanOfMeans);
		yCenterColumn->setValueAt(1, meanOfMeans);

		// Calculate the standard deviation of subgroups
		std::vector<double> subgroupStdDevs;
		for (int i = 0; i < yDataColumn->rowCount(); i += subgroupSize) {
			std::vector<double> subgroup;
			for (int j = 0; j < subgroupSize && (i + j) < yDataColumn->rowCount(); ++j) {
				subgroup.push_back(yDataColumn->valueAt(i + j));
			}
			const double stddev = gsl_stats_sd(subgroup.data(), 1, subgroup.size());
			subgroupStdDevs.push_back(stddev);
		}

		const double meanStdDev = gsl_stats_mean(subgroupStdDevs.data(), 1, subgroupStdDevs.size());
		const double c4 = 0.94; // c4 constant for subgroup size 5
		const double upperLimit = meanOfMeans + 3. * meanStdDev / c4;
		const double lowerLimit = meanOfMeans - 3. * meanStdDev / c4;
		yUpperLimitColumn->setValueAt(0, upperLimit);
		yUpperLimitColumn->setValueAt(1, upperLimit);
		yLowerLimitColumn->setValueAt(0, lowerLimit);
		yLowerLimitColumn->setValueAt(1, lowerLimit);
		break;
	}
	case ProcessBehaviorChart::Type::P: {
		// Calculate the proportion of defectives
		double totalDefectives = 0.0;
		for (int i = 0; i < yDataColumn->rowCount(); ++i) {
			totalDefectives += yDataColumn->valueAt(i);
		}
		const double pBar = totalDefectives / yDataColumn->rowCount();
		yCenterColumn->setValueAt(0, pBar);
		yCenterColumn->setValueAt(1, pBar);

		// Calculate the control limits
		const double upperLimit = pBar + 3. * std::sqrt(pBar * (1 - pBar) / yDataColumn->rowCount());
		const double lowerLimit = pBar - 3. * std::sqrt(pBar * (1 - pBar) / yDataColumn->rowCount());
		yUpperLimitColumn->setValueAt(0, upperLimit);
		yUpperLimitColumn->setValueAt(1, upperLimit);
		yLowerLimitColumn->setValueAt(0, lowerLimit);
		yLowerLimitColumn->setValueAt(1, lowerLimit);
		break;
	}
	case ProcessBehaviorChart::Type::NP: {
		// Calculate the number of defectives
		double totalDefectives = 0.0;
		for (int i = 0; i < yDataColumn->rowCount(); ++i) {
			totalDefectives += yDataColumn->valueAt(i);
		}
		const double npBar = totalDefectives;
		yCenterColumn->setValueAt(0, npBar);
		yCenterColumn->setValueAt(1, npBar);

		// Calculate the control limits
		const double upperLimit = npBar + 3. * std::sqrt(npBar * (1 - npBar / yDataColumn->rowCount()));
		const double lowerLimit = npBar - 3. * std::sqrt(npBar * (1 - npBar / yDataColumn->rowCount()));
		yUpperLimitColumn->setValueAt(0, upperLimit);
		yUpperLimitColumn->setValueAt(1, upperLimit);
		yLowerLimitColumn->setValueAt(0, lowerLimit);
		yLowerLimitColumn->setValueAt(1, lowerLimit);
		break;
	}
	case ProcessBehaviorChart::Type::C: {
		// Calculate the average number of defects per unit
		double totalDefects = 0.0;
		for (int i = 0; i < yDataColumn->rowCount(); ++i) {
			totalDefects += yDataColumn->valueAt(i);
		}
		const double cBar = totalDefects / yDataColumn->rowCount();
		yCenterColumn->setValueAt(0, cBar);
		yCenterColumn->setValueAt(1, cBar);

		// Calculate the control limits
		const double upperLimit = cBar + 3. * std::sqrt(cBar);
		const double lowerLimit = cBar - 3. * std::sqrt(cBar);
		yUpperLimitColumn->setValueAt(0, upperLimit);
		yUpperLimitColumn->setValueAt(1, upperLimit);
		yLowerLimitColumn->setValueAt(0, lowerLimit);
		yLowerLimitColumn->setValueAt(1, lowerLimit);
		break;
	}
	case ProcessBehaviorChart::Type::U: {
		// Calculate the average number of defects per unit
		double totalDefects = 0.0;
		for (int i = 0; i < yDataColumn->rowCount(); ++i) {
			totalDefects += yDataColumn->valueAt(i);
		}
		const double uBar = totalDefects / yDataColumn->rowCount();
		yCenterColumn->setValueAt(0, uBar);
		yCenterColumn->setValueAt(1, uBar);

		// Calculate the control limits
		const double upperLimit = uBar + 3. * std::sqrt(uBar / yDataColumn->rowCount());
		const double lowerLimit = uBar - 3. * std::sqrt(uBar / yDataColumn->rowCount());
		yUpperLimitColumn->setValueAt(0, upperLimit);
		yUpperLimitColumn->setValueAt(1, upperLimit);
		yLowerLimitColumn->setValueAt(0, lowerLimit);
		yLowerLimitColumn->setValueAt(1, lowerLimit);
		break;
	}
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
	WRITE_COLUMN(d->xDataColumn, xDataColumn);
	WRITE_COLUMN(d->yDataColumn, yDataColumn);
	WRITE_COLUMN(d->xCenterColumn, xCenterColumn);
	WRITE_COLUMN(d->yCenterColumn, yCenterColumn);
	WRITE_COLUMN(d->xUpperLimitColumn, xUpperLimitColumn);
	WRITE_COLUMN(d->yUpperLimitColumn, yUpperLimtColumn);
	WRITE_COLUMN(d->xLowerLimitColumn, xLowerLimitColumn);
	WRITE_COLUMN(d->yLowerLimitColumn, yLowerLimitColumn);
	writer->writeAttribute(QStringLiteral("type"), QString::number(static_cast<int>(d->type)));
	writer->writeAttribute(QStringLiteral("visible"), QString::number(d->isVisible()));
	writer->writeAttribute(QStringLiteral("legendVisible"), QString::number(d->legendVisible));
	writer->writeEndElement();

	// save the internal columns, above only the references to them were saved
	d->xCenterColumn->save(writer);
	d->yCenterColumn->save(writer);
	d->xUpperLimitColumn->save(writer);
	d->yUpperLimitColumn->save(writer);
	d->xLowerLimitColumn->save(writer);
	d->yLowerLimitColumn->save(writer);

	// save the internal curves
	d->dataCurve->save(writer);
	d->centerCurve->save(writer);
	d->upperLimitCurve->save(writer);
	d->lowerLimitCurve->save(writer);

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
			READ_COLUMN(xDataColumn);
			READ_COLUMN(yDataColumn);
			READ_COLUMN(xCenterColumn);
			READ_COLUMN(yCenterColumn);
			READ_COLUMN(xUpperLimitColumn);
			READ_COLUMN(yUpperLimitColumn);
			READ_COLUMN(xLowerLimitColumn);
			READ_COLUMN(yLowerLimitColumn);
			READ_INT_VALUE("type", type, ProcessBehaviorChart::Type);
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
			if (name == QLatin1String("xData"))
				rc = const_cast<AbstractColumn*>(d->xDataColumn)->load(reader, preview);
			else if (name == QLatin1String("yData"))
				rc = const_cast<AbstractColumn*>(d->yDataColumn)->load(reader, preview);
			else if (name == QLatin1String("xCenterLine"))
				rc = d->xCenterColumn->load(reader, preview);
			else if (name == QLatin1String("yCenterLine"))
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
			else if (attribs.value(QStringLiteral("name")) == QLatin1String("upper limit"))
				rc = d->upperLimitCurve->load(reader, preview);
			else if (attribs.value(QStringLiteral("name")) == QLatin1String("lower limit"))
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
	const QColor themeColor = plot->themeColorPalette(index);

	Q_D(ProcessBehaviorChart);
	d->suppressRecalc = true;

/*
	d->referenceCurve->line()->loadThemeConfig(group, themeColor);
	d->percentilesCurve->line()->setStyle(Qt::NoPen);
	d->percentilesCurve->symbol()->loadThemeConfig(group, themeColor);
*/
	d->suppressRecalc = false;
	d->recalcShapeAndBoundingRect();
}

void ProcessBehaviorChart::saveThemeConfig(const KConfig& config) {
	Q_D(const ProcessBehaviorChart);
	KConfigGroup group = config.group(QStringLiteral("ProcessBehaviorChart"));
//	d->referenceCurve->line()->saveThemeConfig(group);
//	d->percentilesCurve->symbol()->saveThemeConfig(group);
}
