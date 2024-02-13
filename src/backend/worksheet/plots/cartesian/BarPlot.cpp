/*
	File                 : BarPlot.cpp
	Project              : LabPlot
	Description          : Bar Plot
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022-2024 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "BarPlot.h"
#include "BarPlotPrivate.h"
#include "backend/core/AbstractColumn.h"
#include "backend/core/Settings.h"
#include "backend/core/column/Column.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/trace.h"
#include "backend/worksheet/Background.h"
#include "backend/worksheet/Line.h"
#include "backend/worksheet/plots/cartesian/CartesianCoordinateSystem.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/worksheet/plots/cartesian/ErrorBar.h"
#include "backend/worksheet/plots/cartesian/ErrorBarStyle.h"
#include "backend/worksheet/plots/cartesian/Value.h"
#include "tools/ImageTools.h"

#include <QActionGroup>
#include <QApplication>
#include <QGraphicsSceneMouseEvent>
#include <QMenu>
#include <QPainter>

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>

/**
 * \class BarPlot
 * \brief Box Plot
 */

BarPlot::BarPlot(const QString& name)
	: Plot(name, new BarPlotPrivate(this), AspectType::BarPlot) {
	init();
}

BarPlot::BarPlot(const QString& name, BarPlotPrivate* dd)
	: Plot(name, dd, AspectType::BarPlot) {
	init();
}

// no need to delete the d-pointer here - it inherits from QGraphicsItem
// and is deleted during the cleanup in QGraphicsScene
BarPlot::~BarPlot() = default;

void BarPlot::init() {
	Q_D(BarPlot);

	KConfig config;
	const auto& group = config.group(QStringLiteral("BarPlot"));

	// general
	d->type = (BarPlot::Type)group.readEntry(QStringLiteral("Type"), (int)BarPlot::Type::Grouped);
	d->orientation = (BarPlot::Orientation)group.readEntry(QStringLiteral("Orientation"), (int)BarPlot::Orientation::Vertical);
	d->widthFactor = group.readEntry(QStringLiteral("WidthFactor"), 1.0);

	// initial property objects that will be available even if no data column was set yet
	d->addBackground(group);
	d->addBorderLine(group);
	d->addValue(group);
	d->addErrorBar(group);
	d->addErrorBarStyle(group);
}

/*!
	Returns an icon to be used in the project explorer.
*/
QIcon BarPlot::icon() const {
	return QIcon::fromTheme(QStringLiteral("office-chart-bar"));
}

void BarPlot::initActions() {
	// Orientation
	auto* orientationActionGroup = new QActionGroup(this);
	orientationActionGroup->setExclusive(true);
	connect(orientationActionGroup, &QActionGroup::triggered, this, &BarPlot::orientationChangedSlot);

	orientationHorizontalAction = new QAction(QIcon::fromTheme(QStringLiteral("transform-move-horizontal")), i18n("Horizontal"), orientationActionGroup);
	orientationHorizontalAction->setCheckable(true);

	orientationVerticalAction = new QAction(QIcon::fromTheme(QStringLiteral("transform-move-vertical")), i18n("Vertical"), orientationActionGroup);
	orientationVerticalAction->setCheckable(true);
}

void BarPlot::initMenus() {
	this->initActions();

	// Orientation
	orientationMenu = new QMenu(i18n("Orientation"));
	orientationMenu->setIcon(QIcon::fromTheme(QStringLiteral("draw-cross")));
	orientationMenu->addAction(orientationHorizontalAction);
	orientationMenu->addAction(orientationVerticalAction);
}

QMenu* BarPlot::createContextMenu() {
	if (!orientationMenu)
		initMenus();

	QMenu* menu = WorksheetElement::createContextMenu();
	QAction* visibilityAction = this->visibilityAction();

	// Orientation
	Q_D(const BarPlot);
	if (d->orientation == Orientation::Horizontal)
		orientationHorizontalAction->setChecked(true);
	else
		orientationVerticalAction->setChecked(true);
	menu->insertMenu(visibilityAction, orientationMenu);
	menu->insertSeparator(visibilityAction);

	return menu;
}

void BarPlot::retransform() {
	Q_D(BarPlot);
	d->retransform();
}

void BarPlot::recalc() {
	Q_D(BarPlot);
	d->recalc();
}

void BarPlot::handleResize(double /*horizontalRatio*/, double /*verticalRatio*/, bool /*pageResize*/) {
}

/* ============================ getter methods ================= */
// general
BASIC_SHARED_D_READER_IMPL(BarPlot, QVector<const AbstractColumn*>, dataColumns, dataColumns)
BASIC_SHARED_D_READER_IMPL(BarPlot, BarPlot::Type, type, type)
BASIC_SHARED_D_READER_IMPL(BarPlot, BarPlot::Orientation, orientation, orientation)
BASIC_SHARED_D_READER_IMPL(BarPlot, double, widthFactor, widthFactor)
BASIC_SHARED_D_READER_IMPL(BarPlot, const AbstractColumn*, xColumn, xColumn)

QString& BarPlot::xColumnPath() const {
	D(BarPlot);
	return d->xColumnPath;
}

// box filling
Background* BarPlot::backgroundAt(int index) const {
	Q_D(const BarPlot);
	if (index < d->backgrounds.size())
		return d->backgrounds.at(index);
	else
		return nullptr;
}

// box border lines
Line* BarPlot::lineAt(int index) const {
	Q_D(const BarPlot);
	if (index < d->borderLines.size())
		return d->borderLines.at(index);
	else
		return nullptr;
}

// error bars
ErrorBar* BarPlot::errorBarAt(int index) const {
	Q_D(const BarPlot);
	if (index < d->errorBars.size())
		return d->errorBars.at(index);
	else
		return nullptr;
}

ErrorBarStyle* BarPlot::errorBarStyleAt(int index) const {
	Q_D(const BarPlot);
	if (index < d->errorBarStyles.size())
		return d->errorBarStyles.at(index);
	else
		return nullptr;
}

QVector<QString>& BarPlot::dataColumnPaths() const {
	D(BarPlot);
	return d->dataColumnPaths;
}

double BarPlot::minimum(const Dimension dim) const {
	Q_D(const BarPlot);
	switch (dim) {
	case Dimension::X:
		return d->xMin;
	case Dimension::Y:
		return d->yMin;
	}
	return NAN;
}

double BarPlot::maximum(const Dimension dim) const {
	Q_D(const BarPlot);
	switch (dim) {
	case Dimension::X:
		return d->xMax;
	case Dimension::Y:
		return d->yMax;
	}
	return NAN;
}

bool BarPlot::hasData() const {
	Q_D(const BarPlot);
	return !d->dataColumns.isEmpty();
}

bool BarPlot::usingColumn(const Column* column) const {
	Q_D(const BarPlot);

	if (d->xColumn == column)
		return true;

	for (auto* c : d->dataColumns) {
		if (c == column)
			return true;
	}

	return false;
}

void BarPlot::updateColumnDependencies(const AbstractColumn* column) {
	Q_D(const BarPlot);
	const QString& columnPath = column->path();
	const auto dataColumnPaths = d->dataColumnPaths;
	auto dataColumns = d->dataColumns;
	bool changed = false;

	for (int i = 0; i < dataColumnPaths.count(); ++i) {
		const auto& path = dataColumnPaths.at(i);

		if (path == columnPath) {
			dataColumns[i] = column;
			changed = true;
		}
	}

	if (changed) {
		setUndoAware(false);
		setDataColumns(dataColumns);
		setUndoAware(true);
	}
}

QColor BarPlot::color() const {
	Q_D(const BarPlot);
	if (d->backgrounds.size() > 0 && d->backgrounds.at(0)->enabled())
		return d->backgrounds.at(0)->firstColor();
	else if (d->borderLines.size() > 0 && d->borderLines.at(0)->style() != Qt::PenStyle::NoPen)
		return d->borderLines.at(0)->pen().color();
	return QColor();
}

// values
Value* BarPlot::value() const {
	Q_D(const BarPlot);
	return d->value;
}

/* ============================ setter methods and undo commands ================= */

// General
STD_SETTER_CMD_IMPL_F_S(BarPlot, SetXColumn, const AbstractColumn*, xColumn, recalc)
void BarPlot::setXColumn(const AbstractColumn* column) {
	Q_D(BarPlot);
	if (column != d->xColumn) {
		exec(new BarPlotSetXColumnCmd(d, column, ki18n("%1: set x column")));

		if (column) {
			// update the curve itself on changes
			connect(column, &AbstractColumn::dataChanged, this, &BarPlot::recalc);
			if (column->parentAspect())
				connect(column->parentAspect(), &AbstractAspect::childAspectAboutToBeRemoved, this, &BarPlot::dataColumnAboutToBeRemoved);

			connect(column, &AbstractColumn::dataChanged, this, &BarPlot::dataChanged);
			// TODO: add disconnect in the undo-function
		}
	}
}

STD_SETTER_CMD_IMPL_F_S(BarPlot, SetDataColumns, QVector<const AbstractColumn*>, dataColumns, recalc)
void BarPlot::setDataColumns(const QVector<const AbstractColumn*> columns) {
	Q_D(BarPlot);
	if (columns != d->dataColumns) {
		exec(new BarPlotSetDataColumnsCmd(d, columns, ki18n("%1: set data columns")));

		for (auto* column : columns) {
			if (!column)
				continue;

			// update the curve itself on changes
			connect(column, &AbstractColumn::dataChanged, this, &BarPlot::recalc);
			if (column->parentAspect())
				connect(column->parentAspect(), &AbstractAspect::childAspectAboutToBeRemoved, this, &BarPlot::dataColumnAboutToBeRemoved);
			// TODO: add disconnect in the undo-function

			connect(column, &AbstractColumn::dataChanged, this, &BarPlot::dataChanged);
			connect(column, &AbstractAspect::aspectDescriptionChanged, this, &Plot::appearanceChanged);
		}
	}
}

STD_SETTER_CMD_IMPL_F_S(BarPlot, SetType, BarPlot::Type, type, recalc)
void BarPlot::setType(BarPlot::Type type) {
	Q_D(BarPlot);
	if (type != d->type)
		exec(new BarPlotSetTypeCmd(d, type, ki18n("%1: set type")));
}

STD_SETTER_CMD_IMPL_F_S(BarPlot, SetOrientation, BarPlot::Orientation, orientation, recalc)
void BarPlot::setOrientation(BarPlot::Orientation orientation) {
	Q_D(BarPlot);
	if (orientation != d->orientation)
		exec(new BarPlotSetOrientationCmd(d, orientation, ki18n("%1: set orientation")));
}

STD_SETTER_CMD_IMPL_F_S(BarPlot, SetWidthFactor, double, widthFactor, recalc)
void BarPlot::setWidthFactor(double widthFactor) {
	Q_D(BarPlot);
	if (widthFactor != d->widthFactor)
		exec(new BarPlotSetWidthFactorCmd(d, widthFactor, ki18n("%1: width factor changed")));
}

// ##############################################################################
// #################################  SLOTS  ####################################
// ##############################################################################

void BarPlot::dataColumnAboutToBeRemoved(const AbstractAspect* aspect) {
	Q_D(BarPlot);
	for (int i = 0; i < d->dataColumns.size(); ++i) {
		if (aspect == d->dataColumns.at(i)) {
			d->dataColumns[i] = nullptr;
			d->retransform();
			break;
		}
	}
}

// ##############################################################################
// ######  SLOTs for changes triggered via QActions in the context menu  ########
// ##############################################################################
void BarPlot::orientationChangedSlot(QAction* action) {
	if (action == orientationHorizontalAction)
		this->setOrientation(Axis::Orientation::Horizontal);
	else
		this->setOrientation(Axis::Orientation::Vertical);
}

// ##############################################################################
// ####################### Private implementation ###############################
// ##############################################################################
BarPlotPrivate::BarPlotPrivate(BarPlot* owner)
	: PlotPrivate(owner)
	, q(owner) {
	setFlag(QGraphicsItem::ItemIsSelectable);
	setAcceptHoverEvents(false);
}

Background* BarPlotPrivate::addBackground(const KConfigGroup& group) {
	auto* background = new Background(QString());
	background->setPrefix(QLatin1String("Filling"));
	background->setEnabledAvailable(true);
	background->setHidden(true);
	q->addChild(background);

	if (!q->isLoading())
		background->init(group);

	q->connect(background, &Background::updateRequested, [=] {
		updatePixmap();
		Q_EMIT q->appearanceChanged();
	});

	backgrounds << background;

	return background;
}

Line* BarPlotPrivate::addBorderLine(const KConfigGroup& group) {
	auto* line = new Line(QString());
	line->setPrefix(QLatin1String("Border"));
	line->setHidden(true);
	q->addChild(line);
	if (!q->isLoading())
		line->init(group);

	q->connect(line, &Line::updatePixmapRequested, [=] {
		updatePixmap();
		Q_EMIT q->appearanceChanged();
	});

	q->connect(line, &Line::updateRequested, [=] {
		recalcShapeAndBoundingRect();
		Q_EMIT q->appearanceChanged();
	});

	borderLines << line;

	return line;
}

void BarPlotPrivate::addValue(const KConfigGroup& group) {
	value = new Value(QString());
	q->addChild(value);
	value->setHidden(true);
	value->setcenterPositionAvailable(true);
	if (!q->isLoading())
		value->init(group);

	q->connect(value, &Value::updatePixmapRequested, [=] {
		updatePixmap();
	});

	q->connect(value, &Value::updateRequested, [=] {
		updateValues();
	});
}

ErrorBar* BarPlotPrivate::addErrorBar(const KConfigGroup& group) {
	auto* errorBar = new ErrorBar(QString());
	q->addChild(errorBar);
	errorBar->setHidden(true);
	errorBar->init(group);
	q->connect(errorBar, &ErrorBar::updateRequested, [=] {
		updateErrorBars(errorBars.indexOf(errorBar));
	});

	errorBars << errorBar;

	return errorBar;
}

ErrorBarStyle* BarPlotPrivate::addErrorBarStyle(const KConfigGroup& group) {
	auto* errorBarStyle = new ErrorBarStyle(QString());
	q->addChild(errorBarStyle);
	errorBarStyle->setHidden(true);
	errorBarStyle->init(group);
	q->connect(errorBarStyle, &ErrorBarStyle::updateRequested, [=] {
		updateErrorBars(errorBarStyles.indexOf(errorBarStyle));
	});
	q->connect(errorBarStyle, &ErrorBarStyle::updatePixmapRequested, [=] {
		updatePixmap();
	});

	errorBarStyles << errorBarStyle;

	return errorBarStyle;
}

/*!
  called when the size of the plot or its data ranges (manual changes, zooming, etc.) were changed.
  recalculates the position of the scene points to be drawn.
  triggers the update of lines, drop lines, symbols etc.
*/
void BarPlotPrivate::retransform() {
	const bool suppressed = suppressRetransform || !isVisible() || q->isLoading();
	Q_EMIT trackRetransformCalled(suppressed);
	if (suppressed)
		return;

	PERFTRACE(name() + QLatin1String(Q_FUNC_INFO));

	const int count = dataColumns.size();
	if (!count || m_barLines.size() != count) {
		// no columns or recalc() was not called yet, nothing to do
		recalcShapeAndBoundingRect();
		return;
	}

	m_stackedBarPositiveOffsets.fill(0);
	m_stackedBarNegativeOffsets.fill(0);

	suppressRecalc = true;
	if (count) {
		if (orientation == BarPlot::Orientation::Vertical) {
			for (int i = 0; i < count; ++i) {
				if (dataColumns.at(i))
					verticalBarPlot(i);
			}
		} else {
			for (int i = 0; i < count; ++i) {
				if (dataColumns.at(i))
					horizontalBarPlot(i);
			}
		}
	}
	suppressRecalc = false;

	updateValues(); // this also calls recalcShapeAndBoundingRect()
}

/*!
 * called when the data columns or their values were changed
 * calculates the min and max values for x and y and calls dataChanged()
 * to trigger the retransform in the parent plot
 */
void BarPlotPrivate::recalc() {
	PERFTRACE(name() + QLatin1String(Q_FUNC_INFO));

	const int newSize = dataColumns.size();
	// resize the internal containers
	m_barLines.clear();
	m_barLines.resize(newSize);
	m_fillPolygons.clear();
	m_fillPolygons.resize(newSize);
	m_valuesPointsLogical.clear();
	m_valuesPointsLogical.resize(newSize);
	m_errorBarsPaths.clear();
	m_errorBarsPaths.resize(newSize);

	const double xMinOld = xMin;
	const double xMaxOld = xMax;
	const double yMinOld = yMin;
	const double yMaxOld = yMax;

	// bar properties
	int diff = newSize - backgrounds.size();
	if (diff > 0) {
		// one more bar needs to be added
		KConfig config;
		KConfigGroup group = config.group(QLatin1String("BarPlot"));
		const auto* plot = static_cast<const CartesianPlot*>(q->parentAspect());

		for (int i = 0; i < diff; ++i) {
			// box filling and border line
			auto* background = addBackground(group);
			auto* line = addBorderLine(group);
			addErrorBar(group);
			auto* errorBarStyle = addErrorBarStyle(group);

			if (plot) {
				const auto& themeColor = plot->themeColorPalette(backgrounds.count() - 1);
				background->setFirstColor(themeColor);
				line->setColor(themeColor);
				errorBarStyle->line()->setColor(themeColor);
			}
		}
	} else if (diff < 0) {
		// the last bar was deleted
		//		if (newSize != 0) {
		//			delete backgrounds.takeLast();
		//		}
	}

	// determine the number of bar groups that we need to draw.
	// this number is equal to the max number of non-empty
	// values in the provided datasets
	int barGroupsCount = 0;
	int columnIndex = 0;
	for (auto* column : qAsConst(dataColumns)) {
		int size = static_cast<const Column*>(column)->statistics().size;
		m_barLines[columnIndex].resize(size);
		m_fillPolygons[columnIndex].resize(size);
		if (size > barGroupsCount)
			barGroupsCount = size;

		++columnIndex;
	}

	m_stackedBarPositiveOffsets.resize(barGroupsCount);
	m_stackedBarNegativeOffsets.resize(barGroupsCount);

	m_stackedBar100PercentValues.resize(barGroupsCount);
	m_stackedBar100PercentValues.fill(0);

	// if an x-column was provided and it has less values than the count determined
	// above, we limit the number of bars to the number of values in the x-column
	if (xColumn) {
		int size = static_cast<const Column*>(xColumn)->statistics().size;
		if (size < barGroupsCount)
			barGroupsCount = size;
	}

	// calculate the new min and max values of the bar plot
	QVector<double> barMins(barGroupsCount);
	QVector<double> barMaxs(barGroupsCount);
	if (type == BarPlot::Type::Stacked) {
		for (auto* column : dataColumns) {
			int valueIndex = 0;
			for (int i = 0; i < column->rowCount(); ++i) {
				if (!column->isValid(i) || column->isMasked(i))
					continue;

				double value = column->valueAt(i);
				if (value > 0)
					barMaxs[valueIndex] += value;
				if (value < 0)
					barMins[valueIndex] += value;

				++valueIndex;
			}
		}
	} else if (type == BarPlot::Type::Stacked_100_Percent) {
		for (auto* column : dataColumns) {
			int valueIndex = 0;
			for (int i = 0; i < column->rowCount(); ++i) {
				if (!column->isValid(i) || column->isMasked(i))
					continue;

				m_stackedBar100PercentValues[valueIndex] += column->valueAt(i);
				++valueIndex;
			}
		}
	}

	// determine min and max values for x- and y-ranges.
	// the first group is placed between 0 and 1, the second one between 1 and 2, etc.
	if (orientation == BarPlot::Orientation::Vertical) {
		// min/max for x
		if (xColumn) {
			xMin = xColumn->minimum() - 0.5;
			xMax = xColumn->maximum() + 0.5;
		} else {
			xMin = 0.0;
			xMax = barGroupsCount;
		}

		// min/max for y
		yMin = 0;
		yMax = -INFINITY;
		switch (type) {
		case BarPlot::Type::Grouped: {
			for (auto* column : dataColumns) {
				double max = column->maximum();
				if (max > yMax)
					yMax = max;

				double min = column->minimum();
				if (min < yMin)
					yMin = min;
			}
			break;
		}
		case BarPlot::Type::Stacked: {
			if (!barMaxs.isEmpty())
				yMax = *std::max_element(barMaxs.constBegin(), barMaxs.constEnd());
			if (!barMins.isEmpty())
				yMin = *std::min_element(barMins.constBegin(), barMins.constEnd());
			break;
		}
		case BarPlot::Type::Stacked_100_Percent: {
			yMax = 100;
		}
		}

		// if there are no negative values, we plot
		// in the positive y-direction only and we start at y=0
		if (yMin > 0)
			yMin = 0;
	} else { // horizontal
		// min/max for x
		xMin = 0;
		xMax = -INFINITY;
		switch (type) {
		case BarPlot::Type::Grouped: {
			for (auto* column : dataColumns) {
				double max = column->maximum();
				if (max > xMax)
					xMax = max;

				double min = column->minimum();
				if (min < xMin)
					xMin = min;
			}
			break;
		}
		case BarPlot::Type::Stacked: {
			xMax = *std::max_element(barMaxs.constBegin(), barMaxs.constEnd());
			xMin = *std::min_element(barMins.constBegin(), barMins.constEnd());
			break;
		}
		case BarPlot::Type::Stacked_100_Percent: {
			xMax = 100;
		}
		}

		// if there are no negative values, we plot
		// in the positive x-direction only and we start at x=0
		if (xMin > 0)
			xMin = 0;

		// min/max for y
		if (xColumn) {
			yMin = xColumn->minimum() - 0.5;
			yMax = xColumn->maximum() + 0.5;
		} else {
			yMin = 0.0;
			yMax = barGroupsCount;
		}
	}

	// determine the width of a group and of the gaps around a group
	m_groupWidth = 1.0;
	if (xColumn && newSize != 0)
		m_groupWidth = (xColumn->maximum() - xColumn->minimum()) / newSize;

	m_groupGap = m_groupWidth * 0.1; // gap around a group - the gap between two neighbour groups is 2*m_groupGap

	// if the size of the plot has changed because of the actual
	// data changes or because of new plot settings, emit dataChanged()
	// in order to recalculate the data ranges in the parent plot area
	// and to retransform all its children.
	// Just call retransform() to update the plot only if the ranges didn't change.
	if (xMin != xMinOld || xMax != xMaxOld || yMin != yMinOld || yMax != yMaxOld)
		Q_EMIT q->dataChanged();
	else
		retransform();
}

void BarPlotPrivate::verticalBarPlot(int columnIndex) {
	PERFTRACE(name() + QLatin1String(Q_FUNC_INFO));

	const auto* column = static_cast<const Column*>(dataColumns.at(columnIndex));
	QVector<QLineF> lines; // four lines for one bar in logical coordinates
	QVector<QVector<QLineF>> barLines; // lines for all bars for one colum in scene coordinates
	QVector<QPointF> valuesPointsLogical;

	switch (type) {
	case BarPlot::Type::Grouped: {
		const double barGap = m_groupWidth * 0.1; // gap between two bars within a group
		const int barCount = dataColumns.size(); // number of bars within a group
		const double width = (m_groupWidth * widthFactor - 2 * m_groupGap - (barCount - 1) * barGap) / barCount; // bar width

		int valueIndex = 0;
		for (int i = 0; i < column->rowCount(); ++i) {
			if (!column->isValid(i) || column->isMasked(i))
				continue;

			const double value = column->valueAt(i);
			double x;

			if (xColumn)
				x = xColumn->valueAt(i);
			else
				x = m_groupGap + m_groupWidth * (1 - widthFactor) / 2
					+ valueIndex * m_groupWidth; // translate to the beginning of the group - 1st group is placed between 0 and 1, 2nd between 1 and 2, etc.

			x += (width + barGap) * columnIndex; // translate to the beginning of the bar within the current group

			lines.clear();
			lines << QLineF(x, value, x + width, value);
			lines << QLineF(x + width, value, x + width, 0);
			lines << QLineF(x + width, 0, x, 0);
			lines << QLineF(x, 0, x, value);

			valuesPointsLogical << QPointF(x + width / 2, value);

			barLines << q->cSystem->mapLogicalToScene(lines);
			updateFillingRect(columnIndex, valueIndex, lines);

			++valueIndex;
		}
		break;
	}
	case BarPlot::Type::Stacked: {
		const double width = m_groupWidth * widthFactor - 2 * m_groupGap; // bar width
		int valueIndex = 0;
		for (int i = 0; i < column->rowCount(); ++i) {
			if (!column->isValid(i) || column->isMasked(i))
				continue;

			const double value = column->valueAt(i);
			double offset;
			if (value > 0)
				offset = m_stackedBarPositiveOffsets.at(valueIndex);
			else
				offset = m_stackedBarNegativeOffsets.at(valueIndex);

			double x;
			if (xColumn)
				x = xColumn->valueAt(i);
			else
				x = m_groupGap + m_groupWidth * (1 - widthFactor) / 2 + valueIndex * m_groupWidth; // translate to the beginning of the group

			lines.clear();
			lines << QLineF(x, value + offset, x + width, value + offset);
			lines << QLineF(x + width, value + offset, x + width, offset);
			lines << QLineF(x + width, offset, x, offset);
			lines << QLineF(x, offset, x, value + offset);

			if (value > 0) {
				m_stackedBarPositiveOffsets[valueIndex] += value;
				valuesPointsLogical << QPointF(x + width / 2, m_stackedBarPositiveOffsets.at(valueIndex));
			} else {
				m_stackedBarNegativeOffsets[valueIndex] += value;
				valuesPointsLogical << QPointF(x + width / 2, m_stackedBarNegativeOffsets.at(valueIndex));
			}

			barLines << q->cSystem->mapLogicalToScene(lines);
			updateFillingRect(columnIndex, valueIndex, lines);

			++valueIndex;
		}
		break;
	}
	case BarPlot::Type::Stacked_100_Percent: {
		const double width = m_groupWidth * widthFactor - 2 * m_groupGap; // bar width
		int valueIndex = 0;

		for (int i = 0; i < column->rowCount(); ++i) {
			if (!column->isValid(i) || column->isMasked(i))
				continue;

			double value = column->valueAt(i);
			if (value < 0)
				continue;

			value = value * 100 / m_stackedBar100PercentValues.at(valueIndex);
			double offset = m_stackedBarPositiveOffsets.at(valueIndex);
			double x;

			if (xColumn)
				x = xColumn->valueAt(i);
			else
				x = m_groupGap + m_groupWidth * (1 - widthFactor) / 2 + valueIndex * m_groupWidth; // translate to the beginning of the group

			lines.clear();
			lines << QLineF(x, value + offset, x + width, value + offset);
			lines << QLineF(x + width, value + offset, x + width, offset);
			lines << QLineF(x + width, offset, x, offset);
			lines << QLineF(x, offset, x, value + offset);

			m_stackedBarPositiveOffsets[valueIndex] += value;

			valuesPointsLogical << QPointF(x + width / 2, m_stackedBarPositiveOffsets.at(valueIndex));

			barLines << q->cSystem->mapLogicalToScene(lines);
			updateFillingRect(columnIndex, valueIndex, lines);

			++valueIndex;
		}
	}
	}
	m_barLines[columnIndex] = barLines;
	m_valuesPointsLogical[columnIndex] = valuesPointsLogical;
	updateErrorBars(columnIndex);
}

void BarPlotPrivate::horizontalBarPlot(int columnIndex) {
	PERFTRACE(name() + QLatin1String(Q_FUNC_INFO));

	const auto* column = static_cast<const Column*>(dataColumns.at(columnIndex));
	QVector<QLineF> lines; // four lines for one bar in logical coordinates
	QVector<QVector<QLineF>> barLines; // lines for all bars for one colum in scene coordinates
	QVector<QPointF> valuesPointsLogical;

	switch (type) {
	case BarPlot::Type::Grouped: {
		const double barGap = m_groupWidth * 0.1; // gap between two bars within a group
		const int barCount = dataColumns.size(); // number of bars within a group
		const double width = (m_groupWidth * widthFactor - 2 * m_groupGap - (barCount - 1) * barGap) / barCount; // bar width

		int valueIndex = 0;
		for (int i = 0; i < column->rowCount(); ++i) {
			if (!column->isValid(i) || column->isMasked(i))
				continue;

			const double value = column->valueAt(i);
			double y;
			if (xColumn)
				y = xColumn->valueAt(i);
			else
				y = m_groupGap + m_groupWidth * (1 - widthFactor) / 2 + valueIndex * m_groupWidth; // translate to the beginning of the group

			y += (width + barGap) * columnIndex; // translate to the beginning of the bar within the current group

			lines.clear();
			lines << QLineF(value, y, value, y + width);
			lines << QLineF(value, y + width, 0, y + width);
			lines << QLineF(0, y + width, 0, y);
			lines << QLineF(0, y, value, y);

			valuesPointsLogical << QPointF(value, y + width / 2);

			barLines << q->cSystem->mapLogicalToScene(lines);
			updateFillingRect(columnIndex, valueIndex, lines);

			++valueIndex;
		}
		break;
	}
	case BarPlot::Type::Stacked: {
		const double width = m_groupWidth * widthFactor - 2 * m_groupGap; // bar width
		int valueIndex = 0;
		for (int i = 0; i < column->rowCount(); ++i) {
			if (!column->isValid(i) || column->isMasked(i))
				continue;

			const double value = column->valueAt(i);
			double offset;
			if (value > 0)
				offset = m_stackedBarPositiveOffsets.at(valueIndex);
			else
				offset = m_stackedBarNegativeOffsets.at(valueIndex);

			double y;
			if (xColumn)
				y = xColumn->valueAt(i);
			else
				y = m_groupGap + m_groupWidth * (1 - widthFactor) / 2 + valueIndex * m_groupWidth; // translate to the beginning of the group

			lines.clear();
			lines << QLineF(value + offset, y, value + offset, y + width);
			lines << QLineF(value + offset, y + width, offset, y + width);
			lines << QLineF(offset, y + width, offset, y);
			lines << QLineF(offset, y, value + offset, y);

			if (value > 0) {
				m_stackedBarPositiveOffsets[valueIndex] += value;
				valuesPointsLogical << QPointF(m_stackedBarPositiveOffsets.at(valueIndex), y + width / 2);
			} else {
				m_stackedBarNegativeOffsets[valueIndex] += value;
				valuesPointsLogical << QPointF(m_stackedBarNegativeOffsets.at(valueIndex), y + width / 2);
			}
			barLines << q->cSystem->mapLogicalToScene(lines);
			updateFillingRect(columnIndex, valueIndex, lines);

			++valueIndex;
		}
		break;
	}
	case BarPlot::Type::Stacked_100_Percent: {
		const double width = m_groupWidth * widthFactor - 2 * m_groupGap; // bar width
		int valueIndex = 0;
		for (int i = 0; i < column->rowCount(); ++i) {
			if (!column->isValid(i) || column->isMasked(i))
				continue;

			double value = column->valueAt(i);
			if (value < 0)
				continue;

			value = value * 100 / m_stackedBar100PercentValues.at(valueIndex);
			double offset = m_stackedBarPositiveOffsets.at(valueIndex);

			double y;
			if (xColumn)
				y = xColumn->valueAt(i);
			else
				y = m_groupGap + m_groupWidth * (1 - widthFactor) / 2 + valueIndex * m_groupWidth; // translate to the beginning of the group

			lines.clear();
			lines << QLineF(value + offset, y, value + offset, y + width);
			lines << QLineF(value + offset, y + width, offset, y + width);
			lines << QLineF(offset, y + width, offset, y);
			lines << QLineF(offset, y, value + offset, y);

			m_stackedBarPositiveOffsets[valueIndex] += value;
			valuesPointsLogical << QPointF(m_stackedBarPositiveOffsets.at(valueIndex), y + width / 2);

			barLines << q->cSystem->mapLogicalToScene(lines);
			updateFillingRect(columnIndex, valueIndex, lines);

			++valueIndex;
		}
	}
	}

	m_barLines[columnIndex] = barLines;
	m_valuesPointsLogical[columnIndex] = valuesPointsLogical;
	updateErrorBars(columnIndex);
}

void BarPlotPrivate::updateFillingRect(int columnIndex, int valueIndex, const QVector<QLineF>& lines) {
	const auto& unclippedLines = q->cSystem->mapLogicalToScene(lines, AbstractCoordinateSystem::MappingFlag::SuppressPageClipping);

	if (unclippedLines.isEmpty()) {
		m_fillPolygons[columnIndex][valueIndex] = QPolygonF();
		return;
	}

	// we have four unclipped lines for the box.
	// clip the points to the plot data rect and create a new polygon
	// out of them that will be filled out.
	QPolygonF polygon;
	const QRectF& dataRect = static_cast<CartesianPlot*>(q->parentAspect())->dataRect();
	int i = 0;
	for (const auto& line : unclippedLines) {
		// clip the first point of the line
		QPointF p1 = line.p1();
		if (p1.x() < dataRect.left())
			p1.setX(dataRect.left());
		else if (p1.x() > dataRect.right())
			p1.setX(dataRect.right());

		if (p1.y() < dataRect.top())
			p1.setY(dataRect.top());
		else if (p1.y() > dataRect.bottom())
			p1.setY(dataRect.bottom());

		// clip the second point of the line
		QPointF p2 = line.p2();
		if (p2.x() < dataRect.left())
			p2.setX(dataRect.left());
		else if (p2.x() > dataRect.right())
			p2.setX(dataRect.right());

		if (p2.y() < dataRect.top())
			p2.setY(dataRect.top());
		else if (p2.y() > dataRect.bottom())
			p2.setY(dataRect.bottom());

		if (i != unclippedLines.size() - 1)
			polygon << p1;
		else {
			// close the polygon for the last line
			polygon << p1;
			polygon << p2;
		}

		++i;
	}

	m_fillPolygons[columnIndex][valueIndex] = polygon;
}

void BarPlotPrivate::updateValues() {
	m_valuesPath = QPainterPath();
	m_valuesPoints.clear();
	m_valuesStrings.clear();

	if (value->type() == Value::NoValues) {
		recalcShapeAndBoundingRect();
		return;
	}

	// formatting and drawing of the value strings is independent of the data columns,
	// put all value points for the different data columns together here to process in the same way below
	QVector<QPointF> valuesPointsLogical;
	for (const auto& points : m_valuesPointsLogical)
		valuesPointsLogical << points;

	// determine the value string for all points that are currently visible in the plot
	auto visiblePoints = std::vector<bool>(valuesPointsLogical.count(), false);
	Points pointsScene;
	q->cSystem->mapLogicalToScene(valuesPointsLogical, pointsScene, visiblePoints);
	const auto& prefix = value->prefix();
	const auto& suffix = value->suffix();
	if (value->type() == Value::BinEntries) {
		for (int i = 0; i < valuesPointsLogical.count(); ++i) {
			if (!visiblePoints[i])
				continue;

			auto& point = valuesPointsLogical.at(i);
			if (orientation == BarPlot::Orientation::Vertical) {
				if (type == BarPlot::Type::Stacked_100_Percent)
					m_valuesStrings << prefix + QString::number(point.y(), value->numericFormat(), 1) + QLatin1String("%") + suffix;
				else
					m_valuesStrings << prefix + QString::number(point.y()) + suffix;
			} else {
				if (type == BarPlot::Type::Stacked_100_Percent)
					m_valuesStrings << prefix + QString::number(point.x(), value->numericFormat(), 1) + QLatin1String("%") + suffix;
				else
					m_valuesStrings << prefix + QString::number(point.x()) + suffix;
			}
		}
	} else if (value->type() == Value::CustomColumn) {
		const auto* valuesColumn = value->column();
		if (!valuesColumn) {
			recalcShapeAndBoundingRect();
			return;
		}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
		const int endRow = std::min(valuesPointsLogical.size(), static_cast<qsizetype>(valuesColumn->rowCount()));
#else
		const int endRow = std::min(valuesPointsLogical.size(), valuesColumn->rowCount());
#endif
		const auto xColMode = valuesColumn->columnMode();
		for (int i = 0; i < endRow; ++i) {
			if (!valuesColumn->isValid(i) || valuesColumn->isMasked(i))
				continue;

			switch (xColMode) {
			case AbstractColumn::ColumnMode::Double:
				if (type == BarPlot::Type::Stacked_100_Percent)
					m_valuesStrings << prefix + QString::number(valuesColumn->valueAt(i), value->numericFormat(), 1) + QString::fromStdString("%");
				else
					m_valuesStrings << prefix + QString::number(valuesColumn->valueAt(i), value->numericFormat(), value->precision()) + suffix;
				break;
			case AbstractColumn::ColumnMode::Integer:
			case AbstractColumn::ColumnMode::BigInt:
				m_valuesStrings << prefix + QString::number(valuesColumn->valueAt(i)) + suffix;
				break;
			case AbstractColumn::ColumnMode::Text:
				m_valuesStrings << prefix + valuesColumn->textAt(i) + suffix;
				break;
			case AbstractColumn::ColumnMode::DateTime:
			case AbstractColumn::ColumnMode::Month:
			case AbstractColumn::ColumnMode::Day:
				m_valuesStrings << prefix + valuesColumn->dateTimeAt(i).toString(value->dateTimeFormat()) + suffix;
				break;
			}
		}
	}

	// Calculate the coordinates where to paint the value strings.
	// The coordinates depend on the actual size of the string.
	QFontMetrics fm(value->font());
	qreal w;
	const qreal h = fm.ascent();
	int offset = value->distance();
	switch (value->position()) {
	case Value::Above:
		for (int i = 0; i < m_valuesStrings.size(); i++) {
			w = fm.boundingRect(m_valuesStrings.at(i)).width();
			const auto& point = pointsScene.at(i);

			if (orientation == BarPlot::Orientation::Vertical)
				m_valuesPoints << QPointF(point.x() - w / 2, point.y() - offset);
			else
				m_valuesPoints << QPointF(point.x(), point.y() - offset);
		}
		break;
	case Value::Center: {
		QVector<qreal> listBarWidth;
		for (int i = 0, j = 0; i < m_barLines.size(); i++) {
			auto& columnBarLines = m_barLines.at(i);

			for (int i = 0; i < columnBarLines.size(); i++, j++) { // loop over the different data columns
				if (visiblePoints.at(j) == true)
					listBarWidth.append(columnBarLines.at(i).at(1).length());
			}
		}
		for (int i = 0; i < m_valuesStrings.size(); i++) {
			w = fm.boundingRect(m_valuesStrings.at(i)).width();
			const auto& point = pointsScene.at(i);
			if (orientation == BarPlot::Orientation::Vertical)
				m_valuesPoints << QPointF(point.x() - w / 2,
										  point.y() + listBarWidth.at(i) / 2 + offset - Worksheet::convertToSceneUnits(1, Worksheet::Unit::Point));
			else
				m_valuesPoints << QPointF(point.x() - listBarWidth.at(i) / 2 - offset + h / 2 - w / 2, point.y() + h / 2);
		}
		break;
	}
	case Value::Under:
		for (int i = 0; i < m_valuesStrings.size(); i++) {
			w = fm.boundingRect(m_valuesStrings.at(i)).width();
			const auto& point = pointsScene.at(i);
			if (orientation == BarPlot::Orientation::Vertical)
				m_valuesPoints << QPointF(point.x() - w / 2, point.y() + offset + h / 2);
			else
				m_valuesPoints << QPointF(point.x(), point.y() + offset + h / 2);
		}
		break;
	case Value::Left:
		for (int i = 0; i < m_valuesStrings.size(); i++) {
			w = fm.boundingRect(m_valuesStrings.at(i)).width();
			const auto& point = pointsScene.at(i);
			if (orientation == BarPlot::Orientation::Vertical)
				m_valuesPoints << QPointF(point.x() - offset - w, point.y());
			else
				m_valuesPoints << QPointF(point.x() - offset - w, point.y() + h / 2);
		}
		break;
	case Value::Right:
		for (int i = 0; i < m_valuesStrings.size(); i++) {
			w = fm.boundingRect(m_valuesStrings.at(i)).width();
			const auto& point = pointsScene.at(i);
			if (orientation == BarPlot::Orientation::Vertical)
				m_valuesPoints << QPointF(point.x() + offset, point.y());
			else
				m_valuesPoints << QPointF(point.x() + offset, point.y() + h / 2);
		}
		break;
	}

	QTransform trafo;
	QPainterPath path;
	const double angle = value->rotationAngle();
	for (int i = 0; i < m_valuesPoints.size(); i++) {
		path = QPainterPath();
		path.addText(QPoint(0, 0), value->font(), m_valuesStrings.at(i));

		trafo.reset();
		trafo.translate(m_valuesPoints.at(i).x(), m_valuesPoints.at(i).y());
		if (angle != 0.)
			trafo.rotate(-angle);

		m_valuesPath.addPath(trafo.map(path));
	}

	recalcShapeAndBoundingRect();
}

void BarPlotPrivate::updateErrorBars(int columnIndex) {
	auto errorBarsPath = QPainterPath();

	QVector<QLineF> elines;
	auto* errorBar = errorBars.at(columnIndex);
	const auto& valuesPointsLogical = m_valuesPointsLogical.at(columnIndex);

	switch (errorBar->type()) {
	case ErrorBar::Type::NoError:
	case ErrorBar::Type::Poisson:
		break;
	case ErrorBar::Type::Symmetric: {
		int index = 0;
		if (orientation == BarPlot::Orientation::Vertical) {
			const auto* errorPlusColumn = errorBar->plusColumn();
			for (auto& point : valuesPointsLogical) {
				if (errorPlusColumn && errorPlusColumn->isValid(index) && !errorPlusColumn->isMasked(index)) {
					double error = errorPlusColumn->valueAt(index);
					if (error != 0.)
						elines << QLineF(point.x(), point.y() + error, point.x(), point.y() - error);
				}
				++index;
			}
		} else {
			const auto* errorMinusColumn = errorBar->minusColumn();
			for (auto& point : valuesPointsLogical) {
				if (errorMinusColumn && errorMinusColumn->isValid(index) && !errorMinusColumn->isMasked(index)) {
					double error = errorMinusColumn->valueAt(index);
					if (error != 0.)
						elines << QLineF(point.x() - error, point.y(), point.x() + error, point.y());
				}
				++index;
			}
		}
		break;
	}
	case ErrorBar::Type::Asymmetric: {
		int index = 0;
		if (orientation == BarPlot::Orientation::Vertical) {
			for (auto& point : valuesPointsLogical) {
				double errorPlus = 0.;
				double errorMinus = 0.;
				const auto* errorPlusColumn = errorBar->plusColumn();
				const auto* errorMinusColumn = errorBar->minusColumn();

				if (errorPlusColumn && errorPlusColumn->isValid(index) && !errorPlusColumn->isMasked(index))
					errorPlus = errorPlusColumn->valueAt(index);

				if (errorMinusColumn && errorMinusColumn->isValid(index) && !errorMinusColumn->isMasked(index))
					errorMinus = errorMinusColumn->valueAt(index);

				if (errorPlus != 0. || errorMinus != 0.)
					elines << QLineF(point.x(), point.y() - errorMinus, point.x(), point.y() + errorPlus);

				++index;
			}
		} else {
			for (auto& point : valuesPointsLogical) {
				double errorPlus = 0.;
				double errorMinus = 0.;
				const auto* errorPlusColumn = errorBar->plusColumn();
				const auto* errorMinusColumn = errorBar->minusColumn();

				if (errorPlusColumn && errorPlusColumn->isValid(index) && !errorPlusColumn->isMasked(index))
					errorPlus = errorPlusColumn->valueAt(index);

				if (errorMinusColumn && errorMinusColumn->isValid(index) && !errorMinusColumn->isMasked(index))
					errorMinus = errorMinusColumn->valueAt(index);

				if (errorPlus != 0. || errorMinus != 0.)
					elines << QLineF(point.x() - errorMinus, point.y(), point.x() + errorPlus, point.y());

				++index;
			}
		}
		break;
	}
	}

	// map the error bars to scene coordinates
	elines = q->cSystem->mapLogicalToScene(elines);

	// new painter path for the error bars
	for (const auto& line : qAsConst(elines)) {
		errorBarsPath.moveTo(line.p1());
		errorBarsPath.lineTo(line.p2());
	}

	// add caps for error bars
	const auto* errorBarStyle = errorBarStyles.at(columnIndex);
	const auto errorBarsCapSize = errorBarStyle->capSize();
	if (errorBarStyle->type() == ErrorBarStyle::Type::WithEnds) {
		if (orientation == BarPlot::Orientation::Vertical) {
			for (const auto& line : qAsConst(elines)) {
				const auto& p1 = line.p1();
				errorBarsPath.moveTo(QPointF(p1.x() - errorBarsCapSize / 2., p1.y()));
				errorBarsPath.lineTo(QPointF(p1.x() + errorBarsCapSize / 2., p1.y()));

				const auto& p2 = line.p2();
				errorBarsPath.moveTo(QPointF(p2.x() - errorBarsCapSize / 2., p2.y()));
				errorBarsPath.lineTo(QPointF(p2.x() + errorBarsCapSize / 2., p2.y()));
			}
		} else {
			for (const auto& line : qAsConst(elines)) {
				const auto& p1 = line.p1();
				errorBarsPath.moveTo(QPointF(p1.x(), p1.y() - errorBarsCapSize / 2.));
				errorBarsPath.lineTo(QPointF(p1.x(), p1.y() + errorBarsCapSize / 2.));

				const auto& p2 = line.p2();
				errorBarsPath.moveTo(QPointF(p2.x(), p2.y() - errorBarsCapSize / 2.));
				errorBarsPath.lineTo(QPointF(p2.x(), p2.y() + errorBarsCapSize / 2.));
			}
		}
	}

	m_errorBarsPaths[columnIndex] = errorBarsPath;
	recalcShapeAndBoundingRect();
}

/*!
  recalculates the outer bounds and the shape of the item.
*/
void BarPlotPrivate::recalcShapeAndBoundingRect() {
	if (suppressRecalc)
		return;

	prepareGeometryChange();
	m_shape = QPainterPath();

	int index = 0;
	for (const auto& columnBarLines : m_barLines) { // loop over the different data columns
		for (const auto& barLines : columnBarLines) { // loop over the bars for every data column
			QPainterPath barPath;
			for (const auto& line : barLines) { // loop over the four lines for every bar
				barPath.moveTo(line.p1());
				barPath.lineTo(line.p2());
			}

			if (index < borderLines.count()) { // TODO
				const auto& borderPen = borderLines.at(index)->pen();
				m_shape.addPath(WorksheetElement::shapeFromPath(barPath, borderPen));
			}
		}

		if (errorBars.at(index)->type() != ErrorBar::Type::NoError)
			m_shape.addPath(WorksheetElement::shapeFromPath(m_errorBarsPaths.at(index), errorBarStyles.at(index)->line()->pen()));

		++index;
	}

	if (value->type() != Value::NoValues)
		m_shape.addPath(m_valuesPath);

	m_boundingRectangle = m_shape.boundingRect();
	updatePixmap();
}

void BarPlotPrivate::updatePixmap() {
	PERFTRACE(name() + QLatin1String(Q_FUNC_INFO));
	QPixmap pixmap(m_boundingRectangle.width(), m_boundingRectangle.height());
	if (m_boundingRectangle.width() == 0. || m_boundingRectangle.height() == 0.) {
		m_pixmap = pixmap;
		m_hoverEffectImageIsDirty = true;
		m_selectionEffectImageIsDirty = true;
		return;
	}
	pixmap.fill(Qt::transparent);
	QPainter painter(&pixmap);
	painter.setRenderHint(QPainter::Antialiasing, true);
	painter.translate(-m_boundingRectangle.topLeft());

	draw(&painter);
	painter.end();

	m_pixmap = pixmap;
	m_hoverEffectImageIsDirty = true;
	m_selectionEffectImageIsDirty = true;
	Q_EMIT q->changed();
	update();
}

void BarPlotPrivate::draw(QPainter* painter) {
	PERFTRACE(name() + QLatin1String(Q_FUNC_INFO));

	int columnIndex = 0;
	for (const auto& columnBarLines : m_barLines) { // loop over the different data columns
		int valueIndex = 0;
		for (const auto& barLines : columnBarLines) { // loop over the bars for every data column
			// draw the box filling
			if (columnIndex < backgrounds.size()) { // TODO: remove this check later
				const auto* background = backgrounds.at(columnIndex);
				if (background->enabled()) {
					painter->setOpacity(background->opacity());
					painter->setPen(Qt::NoPen);
					const QPolygonF& polygon = m_fillPolygons.at(columnIndex).at(valueIndex);
					drawFillingPollygon(polygon, painter, background);
				}
			}

			// draw the border
			if (columnIndex < borderLines.size()) { // TODO: remove this check later
				const auto& borderPen = borderLines.at(columnIndex)->pen();
				const double borderOpacity = borderLines.at(columnIndex)->opacity();
				if (borderPen.style() != Qt::NoPen) {
					painter->setPen(borderPen);
					painter->setBrush(Qt::NoBrush);
					painter->setOpacity(borderOpacity);
					for (const auto& line : barLines) // loop over the four lines for every bar
						painter->drawLine(line);
				}
			}

			++valueIndex;
		}

		// draw error bars
		if (errorBars.at(columnIndex)->type() != ErrorBar::Type::NoError)
			errorBarStyles.at(columnIndex)->draw(painter, m_errorBarsPaths.at(columnIndex));

		++columnIndex;
	}

	// draw values
	value->draw(painter, m_valuesPoints, m_valuesStrings);
}

void BarPlotPrivate::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget*) {
	if (!isVisible())
		return;

	painter->setPen(Qt::NoPen);
	painter->setBrush(Qt::NoBrush);
	painter->setRenderHint(QPainter::SmoothPixmapTransform, true);

	if (Settings::group(QStringLiteral("Settings_Worksheet")).readEntry<bool>("DoubleBuffering", true))
		painter->drawPixmap(m_boundingRectangle.topLeft(), m_pixmap); // draw the cached pixmap (fast)
	else
		draw(painter); // draw directly again (slow)

	if (m_hovered && !isSelected() && !q->isPrinting()) {
		if (m_hoverEffectImageIsDirty) {
			QPixmap pix = m_pixmap;
			QPainter p(&pix);
			p.setCompositionMode(QPainter::CompositionMode_SourceIn); // source (shadow) pixels merged with the alpha channel of the destination (m_pixmap)
			p.fillRect(pix.rect(), QApplication::palette().color(QPalette::Shadow));
			p.end();

			m_hoverEffectImage = ImageTools::blurred(pix.toImage(), m_pixmap.rect(), 5);
			m_hoverEffectImageIsDirty = false;
		}

		painter->drawImage(m_boundingRectangle.topLeft(), m_hoverEffectImage, m_pixmap.rect());
		return;
	}

	if (isSelected() && !q->isPrinting()) {
		if (m_selectionEffectImageIsDirty) {
			QPixmap pix = m_pixmap;
			QPainter p(&pix);
			p.setCompositionMode(QPainter::CompositionMode_SourceIn);
			p.fillRect(pix.rect(), QApplication::palette().color(QPalette::Highlight));
			p.end();

			m_selectionEffectImage = ImageTools::blurred(pix.toImage(), m_pixmap.rect(), 5);
			m_selectionEffectImageIsDirty = false;
		}

		painter->drawImage(m_boundingRectangle.topLeft(), m_selectionEffectImage, m_pixmap.rect());
		return;
	}
}

// ##############################################################################
// ##################  Serialization/Deserialization  ###########################
// ##############################################################################
//! Save as XML
void BarPlot::save(QXmlStreamWriter* writer) const {
	Q_D(const BarPlot);

	writer->writeStartElement(QStringLiteral("barPlot"));
	writeBasicAttributes(writer);
	writeCommentElement(writer);

	// general
	writer->writeStartElement(QStringLiteral("general"));
	writer->writeAttribute(QStringLiteral("type"), QString::number(static_cast<int>(d->type)));
	writer->writeAttribute(QStringLiteral("orientation"), QString::number(static_cast<int>(d->orientation)));
	writer->writeAttribute(QStringLiteral("widthFactor"), QString::number(d->widthFactor));
	writer->writeAttribute(QStringLiteral("plotRangeIndex"), QString::number(m_cSystemIndex));
	writer->writeAttribute(QStringLiteral("xMin"), QString::number(d->xMin));
	writer->writeAttribute(QStringLiteral("xMax"), QString::number(d->xMax));
	writer->writeAttribute(QStringLiteral("yMin"), QString::number(d->yMin));
	writer->writeAttribute(QStringLiteral("yMax"), QString::number(d->yMax));
	writer->writeAttribute(QStringLiteral("visible"), QString::number(d->isVisible()));
	writer->writeAttribute(QStringLiteral("legendVisible"), QString::number(d->legendVisible));

	if (d->xColumn)
		writer->writeAttribute(QStringLiteral("xColumn"), d->xColumn->path());

	for (auto* column : d->dataColumns) {
		writer->writeStartElement(QStringLiteral("column"));
		writer->writeAttribute(QStringLiteral("path"), column->path());
		writer->writeEndElement();
	}
	writer->writeEndElement();

	// box filling
	for (auto* background : d->backgrounds)
		background->save(writer);

	// box border lines
	for (auto* line : d->borderLines)
		line->save(writer);

	// Values
	d->value->save(writer);

	writer->writeEndElement(); // close "BarPlot" section
}

//! Load from XML
bool BarPlot::load(XmlStreamReader* reader, bool preview) {
	Q_D(BarPlot);

	if (!readBasicAttributes(reader))
		return false;

	QXmlStreamAttributes attribs;
	QString str;
	bool firstBackgroundRead = false;
	bool firstBorderLineRead = false;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == QLatin1String("barPlot"))
			break;

		if (!reader->isStartElement())
			continue;

		if (!preview && reader->name() == QLatin1String("comment")) {
			if (!readCommentElement(reader))
				return false;
		} else if (!preview && reader->name() == QLatin1String("general")) {
			attribs = reader->attributes();

			READ_INT_VALUE("type", type, BarPlot::Type);
			READ_INT_VALUE("orientation", orientation, BarPlot::Orientation);
			READ_DOUBLE_VALUE("widthFactor", widthFactor);
			READ_INT_VALUE_DIRECT("plotRangeIndex", m_cSystemIndex, int);

			READ_DOUBLE_VALUE("xMin", xMin);
			READ_DOUBLE_VALUE("xMax", xMax);
			READ_DOUBLE_VALUE("yMin", yMin);
			READ_DOUBLE_VALUE("yMax", yMax);
			READ_COLUMN(xColumn);
			READ_INT_VALUE("legendVisible", legendVisible, bool);

			str = attribs.value(QStringLiteral("visible")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("visible"));
			else
				d->setVisible(str.toInt());
		} else if (reader->name() == QLatin1String("column")) {
			attribs = reader->attributes();

			str = attribs.value(QStringLiteral("path")).toString();
			if (!str.isEmpty())
				d->dataColumnPaths << str;
			// 			READ_COLUMN(dataColumn);
		} else if (!preview && reader->name() == QLatin1String("filling")) {
			if (!firstBackgroundRead) {
				auto* background = d->backgrounds.at(0);
				background->load(reader, preview);
				firstBackgroundRead = true;
			} else {
				auto* background = d->addBackground(KConfigGroup());
				background->load(reader, preview);
			}
		} else if (!preview && reader->name() == QLatin1String("border")) {
			if (!firstBorderLineRead) {
				auto* line = d->borderLines.at(0);
				line->load(reader, preview);
				firstBorderLineRead = true;
			} else {
				auto* line = d->addBorderLine(KConfigGroup());
				line->load(reader, preview);
			}
		} else if (!preview && reader->name() == QLatin1String("values")) {
			d->value->load(reader, preview);
		} else { // unknown element
			reader->raiseUnknownElementWarning();
			if (!reader->skipToEndElement())
				return false;
		}
	}

	d->dataColumns.resize(d->dataColumnPaths.size());

	return true;
}

// ##############################################################################
// #########################  Theme management ##################################
// ##############################################################################
void BarPlot::loadThemeConfig(const KConfig& config) {
	KConfigGroup group;
	if (config.hasGroup(QStringLiteral("Theme")))
		group = config.group(QStringLiteral("XYCurve")); // when loading from the theme config, use the same properties as for XYCurve
	else
		group = config.group(QStringLiteral("BarPlot"));

	const auto* plot = static_cast<const CartesianPlot*>(parentAspect());
	int index = plot->curveChildIndex(this);
	const QColor themeColor = plot->themeColorPalette(index);

	Q_D(BarPlot);
	d->suppressRecalc = true;

	// box filling
	for (int i = 0; i < d->backgrounds.count(); ++i) {
		auto* background = d->backgrounds.at(i);
		background->loadThemeConfig(group, plot->themeColorPalette(i));
	}

	// box border lines
	for (int i = 0; i < d->borderLines.count(); ++i) {
		auto* line = d->borderLines.at(i);
		line->loadThemeConfig(group, plot->themeColorPalette(i));
	}

	// Values
	d->value->loadThemeConfig(group, themeColor);

	d->suppressRecalc = false;
	d->recalcShapeAndBoundingRect();
}
