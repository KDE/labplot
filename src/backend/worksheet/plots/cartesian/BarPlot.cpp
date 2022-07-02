/*
	File                 : BarPlot.cpp
	Project              : LabPlot
	Description          : Bar Plot
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "BarPlot.h"
#include "BarPlotPrivate.h"
#include "backend/core/AbstractColumn.h"
#include "backend/core/column/Column.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/trace.h"
#include "backend/worksheet/Background.h"
#include "backend/worksheet/plots/cartesian/CartesianCoordinateSystem.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
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
	: WorksheetElement(name, new BarPlotPrivate(this), AspectType::BarPlot) {
	init();
}

BarPlot::BarPlot(const QString& name, BarPlotPrivate* dd)
	: WorksheetElement(name, dd, AspectType::BarPlot) {
	init();
}

// no need to delete the d-pointer here - it inherits from QGraphicsItem
// and is deleted during the cleanup in QGraphicsScene
BarPlot::~BarPlot() = default;

void BarPlot::init() {
	Q_D(BarPlot);

	KConfig config;
	KConfigGroup group = config.group("BarPlot");

	d->xColumn = nullptr;

	// general
	d->type = (BarPlot::Type)group.readEntry("Type", (int)BarPlot::Type::Grouped);
	d->orientation = (BarPlot::Orientation)group.readEntry("Orientation", (int)BarPlot::Orientation::Vertical);
	d->widthFactor = group.readEntry("WidthFactor", 1.0);

	d->widthFactors << d->widthFactor;

	// box filling

	// create the initial background object that will be available even if not data column was set yet
	auto* background = new Background(QString());
	background->setPrefix(QLatin1String("Filling"));
	background->setEnabledAvailable(true);
	background->setHidden(true);
	addChild(background);
	background->init(group);
	connect(background, &Background::updateRequested, [=] {
		d->updatePixmap();
	});
	d->backgrounds << background;

	// box border
	d->borderPen = QPen(group.readEntry("BorderColor", QColor(Qt::black)),
						group.readEntry("BorderWidth", Worksheet::convertToSceneUnits(1.0, Worksheet::Unit::Point)),
						(Qt::PenStyle)group.readEntry("BorderStyle", (int)Qt::SolidLine));
	d->borderOpacity = group.readEntry("BorderOpacity", 1.0);
}

/*!
	Returns an icon to be used in the project explorer.
*/
QIcon BarPlot::icon() const {
	return QIcon::fromTheme(QLatin1String("office-chart-bar"));
}

void BarPlot::initActions() {
	visibilityAction = new QAction(QIcon::fromTheme("view-visible"), i18n("Visible"), this);
	visibilityAction->setCheckable(true);
	connect(visibilityAction, &QAction::triggered, this, &BarPlot::visibilityChangedSlot);

	// Orientation
	auto* orientationActionGroup = new QActionGroup(this);
	orientationActionGroup->setExclusive(true);
	connect(orientationActionGroup, &QActionGroup::triggered, this, &BarPlot::orientationChangedSlot);

	orientationHorizontalAction = new QAction(QIcon::fromTheme(QLatin1String("transform-move-horizontal")), i18n("Horizontal"), orientationActionGroup);
	orientationHorizontalAction->setCheckable(true);

	orientationVerticalAction = new QAction(QIcon::fromTheme(QLatin1String("transform-move-vertical")), i18n("Vertical"), orientationActionGroup);
	orientationVerticalAction->setCheckable(true);
}

void BarPlot::initMenus() {
	this->initActions();

	// Orientation
	orientationMenu = new QMenu(i18n("Orientation"));
	orientationMenu->setIcon(QIcon::fromTheme(QLatin1String("draw-cross")));
	orientationMenu->addAction(orientationHorizontalAction);
	orientationMenu->addAction(orientationVerticalAction);
}

QMenu* BarPlot::createContextMenu() {
	if (!orientationMenu)
		initMenus();

	QMenu* menu = WorksheetElement::createContextMenu();
	QAction* firstAction = menu->actions().at(1); // skip the first action because of the "title-action"

	// Visibility
	visibilityAction->setChecked(isVisible());
	menu->insertAction(firstAction, visibilityAction);

	// Orientation
	Q_D(const BarPlot);
	if (d->orientation == Orientation::Horizontal)
		orientationHorizontalAction->setChecked(true);
	else
		orientationVerticalAction->setChecked(true);
	menu->insertMenu(firstAction, orientationMenu);
	menu->insertSeparator(firstAction);

	return menu;
}

QGraphicsItem* BarPlot::graphicsItem() const {
	return d_ptr;
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

bool BarPlot::activateCurve(QPointF mouseScenePos, double maxDist) {
	Q_D(BarPlot);
	return d->activateCurve(mouseScenePos, maxDist);
}

void BarPlot::setHover(bool on) {
	Q_D(BarPlot);
	d->setHover(on);
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

// box
// filling
Background* BarPlot::backgroundAt(int index) const {
	Q_D(const BarPlot);
	if (index < d->backgrounds.size())
		return d->backgrounds.at(index);
	else
		return nullptr;
}

// border
BASIC_SHARED_D_READER_IMPL(BarPlot, QPen, borderPen, borderPen)
BASIC_SHARED_D_READER_IMPL(BarPlot, qreal, borderOpacity, borderOpacity)

QVector<QString>& BarPlot::dataColumnPaths() const {
	D(BarPlot);
	return d->dataColumnPaths;
}

double BarPlot::xMinimum() const {
	D(BarPlot);
	return d->xMin;
}

double BarPlot::xMaximum() const {
	D(BarPlot);
	return d->xMax;
}

double BarPlot::yMinimum() const {
	D(BarPlot);
	return d->yMin;
}

double BarPlot::yMaximum() const {
	D(BarPlot);
	return d->yMax;
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
			connect(column->parentAspect(), &AbstractAspect::aspectAboutToBeRemoved, this, &BarPlot::dataColumnAboutToBeRemoved);

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
			connect(column->parentAspect(), &AbstractAspect::aspectAboutToBeRemoved, this, &BarPlot::dataColumnAboutToBeRemoved);
			// TODO: add disconnect in the undo-function

			connect(column, &AbstractColumn::dataChanged, this, &BarPlot::dataChanged);
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

// box border
STD_SETTER_CMD_IMPL_F_S(BarPlot, SetBorderPen, QPen, borderPen, recalcShapeAndBoundingRect)
void BarPlot::setBorderPen(const QPen& pen) {
	Q_D(BarPlot);
	if (pen != d->borderPen)
		exec(new BarPlotSetBorderPenCmd(d, pen, ki18n("%1: set border pen")));
}

STD_SETTER_CMD_IMPL_F_S(BarPlot, SetBorderOpacity, qreal, borderOpacity, updatePixmap)
void BarPlot::setBorderOpacity(qreal opacity) {
	Q_D(BarPlot);
	if (opacity != d->borderOpacity)
		exec(new BarPlotSetBorderOpacityCmd(d, opacity, ki18n("%1: set border opacity")));
}

//##############################################################################
//#################################  SLOTS  ####################################
//##############################################################################

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

//##############################################################################
//######  SLOTs for changes triggered via QActions in the context menu  ########
//##############################################################################
void BarPlot::orientationChangedSlot(QAction* action) {
	if (action == orientationHorizontalAction)
		this->setOrientation(Axis::Orientation::Horizontal);
	else
		this->setOrientation(Axis::Orientation::Vertical);
}

void BarPlot::visibilityChangedSlot() {
	Q_D(const BarPlot);
	this->setVisible(!d->isVisible());
}

//##############################################################################
//####################### Private implementation ###############################
//##############################################################################
BarPlotPrivate::BarPlotPrivate(BarPlot* owner)
	: WorksheetElementPrivate(owner)
	, q(owner) {
	setFlag(QGraphicsItem::ItemIsSelectable);
	setAcceptHoverEvents(false);
}

bool BarPlotPrivate::activateCurve(QPointF mouseScenePos, double /*maxDist*/) {
	if (!isVisible())
		return false;

	return shape().contains(mouseScenePos);
}

void BarPlotPrivate::setHover(bool on) {
	if (on == m_hovered)
		return; // don't update if state not changed

	m_hovered = on;
	on ? Q_EMIT q->hovered() : Q_EMIT q->unhovered();
	update();
}

/*!
  called when the size of the plot or its data ranges (manual changes, zooming, etc.) were changed.
  recalculates the position of the scene points to be drawn.
  triggers the update of lines, drop lines, symbols etc.
*/
void BarPlotPrivate::retransform() {
	if (m_suppressRetransform || !isVisible() || q->isLoading())
		return;

	PERFTRACE(name() + Q_FUNC_INFO);

	const int count = dataColumns.size();
	if (!count || m_barLines.size() != count) {
		// no columns or recalc() was not called yet, nothing to do
		recalcShapeAndBoundingRect();
		return;
	}

	m_stackedBarPositiveOffsets.fill(0);
	m_stackedBarNegativeOffsets.fill(0);

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

	recalcShapeAndBoundingRect();
}

/*!
 * called when the data columns or their values were changed
 * calculates the min and max values for x and y and calls dataChanged()
 * to trigger the retransform in the parent plot
 */
void BarPlotPrivate::recalc() {
	PERFTRACE(name() + Q_FUNC_INFO);

	const int newSize = dataColumns.size();
	// resize the internal containers
	m_barLines.clear();
	m_barLines.resize(newSize);
	m_fillPolygons.clear();
	m_fillPolygons.resize(newSize);

	// bar properties
	int diff = newSize - backgrounds.size();
	if (diff > 0) {
		// one more bar needs to be added
		KConfig config;
		KConfigGroup group = config.group(QLatin1String("BarPlot"));
		const auto* plot = static_cast<const CartesianPlot*>(q->parentAspect());

		for (int i = 0; i < diff; ++i) {
			widthFactors << group.readEntry("WidthFactor", 1.0);

			// box filling
			auto* background = new Background(QString());
			background->setPrefix(QLatin1String("Filling"));
			background->setEnabledAvailable(true);
			background->setHidden(true);
			q->addChild(background);
			background->init(group);
			q->connect(background, &Background::updateRequested, [=] {
				updatePixmap();
			});

			backgrounds << background;

			if (plot)
				background->setFirstColor(plot->themeColorPalette(backgrounds.count() - 1));
		}
	} else if (diff < 0){
		// the last bar was deleted
//		if (newSize != 0) {
//			widthFactors.takeLast();
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
	}

	if (orientation == BarPlot::Orientation::Vertical) {
		// min/max for x
		if (xColumn) {
			xMin = xColumn->minimum() - 0.5;
			xMax = xColumn->maximum() + 0.5;
		} else {
			xMin = 0.0;
			xMax = barGroupsCount + 1.0;
		}

		// min/max for y
		yMin = 0;
		yMax = -INFINITY;
		if (type == BarPlot::Type::Grouped) {
			for (auto* column : dataColumns) {
				double max = column->maximum();
				if (max > yMax)
					yMax = max;

				double min = column->minimum();
				if (min < yMin)
					yMin = min;
			}
		} else { // stacked bar plots
			yMax = *std::max_element(barMaxs.constBegin(), barMaxs.constEnd());
			yMin = *std::min_element(barMins.constBegin(), barMins.constEnd());
		}

		// if there are no negative values, we plot
		// in the positive y-direction only and we start at y=0
		if (yMin > 0)
			yMin = 0;
	} else { // horizontal
		// min/max for x
		xMin = 0;
		xMax = -INFINITY;
		if (type == BarPlot::Type::Grouped) {
			for (auto* column : dataColumns) {
				double max = column->maximum();
				if (max > xMax)
					xMax = max;

				double min = column->minimum();
				if (min < xMin)
					xMin = min;
			}

		} else { // stacked bar plots
			xMax = *std::max_element(barMaxs.constBegin(), barMaxs.constEnd());
			xMin = *std::min_element(barMins.constBegin(), barMins.constEnd());
		}

		// if there are no negative values, we plot
		// in the positive x-direction only and we start at x=0
		if (xMin > 0)
			xMin = 0;

		// min/max for y
		if (xColumn) {
			yMin = xColumn->minimum() - 1.0;
			yMax = xColumn->maximum() + 1.0;
		} else {
			yMin = 0.0;
			yMax = barGroupsCount + 1.0;
		}
	}

	// determine the width of a group and of the gaps around a group
	m_groupWidth = 1.0;
	if (xColumn && newSize != 0)
		m_groupWidth = (xColumn->maximum() - xColumn->minimum())/newSize;

	m_groupGap = m_groupWidth*0.15*widthFactor; // gap around a group - the gap between two neighbour groups is 2*m_groupGap

	// the size of the bar plots changed because of the actual
	// data changes or because of new bar plot settings.
	// Q_EMIT dataChanged() in order to recalculate everything
	// in the parent plot with the new size/shape of the boxplot
	Q_EMIT q->dataChanged();
}

void BarPlotPrivate::verticalBarPlot(int columnIndex) {
	PERFTRACE(name() + Q_FUNC_INFO);

	const auto* column = static_cast<const Column*>(dataColumns.at(columnIndex));
	QVector<QLineF> lines; // four lines for one bar in logical coordinates
	QVector<QVector<QLineF>> barLines; // lines for all bars for one colum in scene coordinates

	if (type == BarPlot::Type::Grouped) {
		const double barGap = m_groupWidth*0.1*widthFactor; // gap between two bars within a group
		const int barCount = dataColumns.size(); // number of bars within a group
		const double width = (m_groupWidth*widthFactor - 2*m_groupGap - (barCount - 1)*barGap) / barCount; // bar width

		int valueIndex = 0;
		for (int i = 0; i < column->rowCount(); ++i) {
			if (!column->isValid(i) || column->isMasked(i))
				continue;

			const double value = column->valueAt(i);
			double x;

			if (xColumn)
				x = xColumn->valueAt(i);
			else
				x = valueIndex + m_groupWidth;

			x += -m_groupWidth*0.5*widthFactor + m_groupGap + (width + barGap)*columnIndex;

			lines.clear();
			lines << QLineF(x, value, x + width, value);
			lines << QLineF(x + width, value, x + width, 0);
			lines << QLineF(x + width, 0, x, 0);
			lines << QLineF(x, 0, x, value);

			barLines << q->cSystem->mapLogicalToScene(lines);
			updateFillingRect(columnIndex, valueIndex, lines);

			++valueIndex;
		}
	} else { // stacked bar plot
		const double width = 1*widthFactor - 2*m_groupGap; // bar width
		int valueIndex = 0;
		for (int i = 0; i < column->rowCount(); ++i) {
			if (!column->isValid(i) || column->isMasked(i))
				continue;

			const double value = column->valueAt(i);
			double offset;
			if (value > 0)
				offset = m_stackedBarPositiveOffsets[valueIndex];
			else
				offset = m_stackedBarNegativeOffsets[valueIndex];

			double x;

			if (xColumn)
				x = xColumn->valueAt(i);
			else
				x = valueIndex + m_groupWidth;

			x += -0.5*widthFactor + m_groupGap;

			lines.clear();
			lines << QLineF(x, value + offset, x + width, value + offset);
			lines << QLineF(x + width, value + offset, x + width, offset);
			lines << QLineF(x + width, offset, x, offset);
			lines << QLineF(x, offset, x, value + offset);

			if (value > 0)
				m_stackedBarPositiveOffsets[valueIndex] += value;
			else
				m_stackedBarNegativeOffsets[valueIndex] += value;

			barLines << q->cSystem->mapLogicalToScene(lines);
			updateFillingRect(columnIndex, valueIndex, lines);

			++valueIndex;
		}
	}

	m_barLines[columnIndex] = barLines;
}

void BarPlotPrivate::horizontalBarPlot(int columnIndex) {
	PERFTRACE(name() + Q_FUNC_INFO);

	const auto* column = static_cast<const Column*>(dataColumns.at(columnIndex));
	QVector<QLineF> lines; // four lines for one bar in logical coordinates
	QVector<QVector<QLineF>> barLines; // lines for all bars for one colum in scene coordinates

	if (type == BarPlot::Type::Grouped) {
		const double barGap = m_groupWidth*0.1*widthFactor; // gap between two bars within a group
		const int barCount = dataColumns.size(); // number of bars within a group
		const double width = (m_groupWidth*widthFactor - 2*m_groupGap - (barCount - 1)*barGap) / barCount; // bar width

		int valueIndex = 0;
		for (int i = 0; i < column->rowCount(); ++i) {
			if (!column->isValid(i) || column->isMasked(i))
				continue;

			const double value = column->valueAt(i);
			double y;

			if (xColumn)
				y = xColumn->valueAt(i);
			else
				y = valueIndex + m_groupWidth;

			y += -m_groupWidth*0.5*widthFactor + m_groupGap + (width + barGap)*columnIndex;

			lines.clear();
			lines << QLineF(value, y, value, y + width);
			lines << QLineF(value, y + width, 0, y + width);
			lines << QLineF(0, y + width, 0, y);
			lines << QLineF(0, y, value, y);

			barLines << q->cSystem->mapLogicalToScene(lines);
			updateFillingRect(columnIndex, valueIndex, lines);

			++valueIndex;
		}
	} else { // stacked bar plot
		const double width = 1*widthFactor - 2*m_groupGap; // bar width
		int valueIndex = 0;
		for (int i = 0; i < column->rowCount(); ++i) {
			if (!column->isValid(i) || column->isMasked(i))
				continue;

			const double value = column->valueAt(i);
			double offset;
			if (value > 0)
				offset = m_stackedBarPositiveOffsets[valueIndex];
			else
				offset = m_stackedBarNegativeOffsets[valueIndex];

			double y;

			if (xColumn)
				y = xColumn->valueAt(i);
			else
				y = valueIndex + m_groupWidth;

			y += -0.5*widthFactor + m_groupGap;

			lines.clear();
			lines << QLineF(value + offset, y, value + offset, y + width);
			lines << QLineF(value + offset, y + width, offset, y + width);
			lines << QLineF(offset, y + width, offset, y);
			lines << QLineF(offset, y, value + offset, y);

			if (value > 0)
				m_stackedBarPositiveOffsets[valueIndex] += value;
			else
				m_stackedBarNegativeOffsets[valueIndex] += value;

			barLines << q->cSystem->mapLogicalToScene(lines);
			updateFillingRect(columnIndex, valueIndex, lines);

			++valueIndex;
		}
	}

	m_barLines[columnIndex] = barLines;
}

void BarPlotPrivate::updateFillingRect(int columnIndex, int valueIndex, const QVector<QLineF>& lines) {
	const auto& unclippedLines = q->cSystem->mapLogicalToScene(lines, AbstractCoordinateSystem::MappingFlag::SuppressPageClipping);

	if (unclippedLines.isEmpty()) {
		m_fillPolygons[columnIndex][valueIndex]  = QPolygonF();
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

/*!
	Returns the outer bounds of the item as a rectangle.
 */
QRectF BarPlotPrivate::boundingRect() const {
	return m_boundingRectangle;
}

/*!
	Returns the shape of this item as a QPainterPath in local coordinates.
*/
QPainterPath BarPlotPrivate::shape() const {
	return m_barPlotShape;
}

/*!
  recalculates the outer bounds and the shape of the item.
*/
void BarPlotPrivate::recalcShapeAndBoundingRect() {
	prepareGeometryChange();
	m_barPlotShape = QPainterPath();

	for (const auto& columnBarLines : m_barLines) { // loop over the different data columns
		for (const auto& barLines : columnBarLines) { // loop over the bars for every data column
			QPainterPath barPath;
			for (const auto& line : barLines) { // loop over the four lines for every bar
				barPath.moveTo(line.p1());
				barPath.lineTo(line.p2());
			}
			m_barPlotShape.addPath(WorksheetElement::shapeFromPath(barPath, borderPen));
		}
	}

	m_boundingRectangle = m_barPlotShape.boundingRect();
	updatePixmap();
}

void BarPlotPrivate::updatePixmap() {
	PERFTRACE(name() + Q_FUNC_INFO);
	QPixmap pixmap(m_boundingRectangle.width(), m_boundingRectangle.height());
	if (m_boundingRectangle.width() == 0 || m_boundingRectangle.height() == 0) {
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
	update();
}

void BarPlotPrivate::draw(QPainter* painter) {
	PERFTRACE(name() + Q_FUNC_INFO);

	int columnIndex = 0;
	for (const auto& columnBarLines : m_barLines) { // loop over the different data columns
		int valueIndex = 0;
		for (const auto& barLines : columnBarLines) { // loop over the bars for every data column
			// draw the box filling
			if (columnIndex < backgrounds.size()) { // TODO: remove this check later
				auto* background = backgrounds.at(columnIndex);
				if (background->enabled()) {
					painter->setOpacity(background->opacity());
					painter->setPen(Qt::SolidLine);
					drawFilling(painter, columnIndex, valueIndex);
				}
			}

			// draw the border
			if (borderPen.style() != Qt::NoPen) {
				painter->setPen(borderPen);
				painter->setBrush(Qt::NoBrush);
				painter->setOpacity(borderOpacity);
				for (const auto& line : barLines) // loop over the four lines for every bar
					painter->drawLine(line);
			}

			++valueIndex;
		}
		++columnIndex;
	}
}

void BarPlotPrivate::drawFilling(QPainter* painter, int columnIndex, int valueIndex) {
	PERFTRACE(name() + Q_FUNC_INFO);

	const QPolygonF& polygon = m_fillPolygons.at(columnIndex).at(valueIndex);
	const QRectF& rect = polygon.boundingRect();

	const auto* background = backgrounds.at(columnIndex);

	if (background->type() == Background::Type::Color) {
		switch (background->colorStyle()) {
		case Background::ColorStyle::SingleColor: {
			painter->setBrush(QBrush(background->firstColor()));
			break;
		}
		case Background::ColorStyle::HorizontalLinearGradient: {
			QLinearGradient linearGrad(rect.topLeft(), rect.topRight());
			linearGrad.setColorAt(0, background->firstColor());
			linearGrad.setColorAt(1, background->secondColor());
			painter->setBrush(QBrush(linearGrad));
			break;
		}
		case Background::ColorStyle::VerticalLinearGradient: {
			QLinearGradient linearGrad(rect.topLeft(), rect.bottomLeft());
			linearGrad.setColorAt(0, background->firstColor());
			linearGrad.setColorAt(1, background->secondColor());
			painter->setBrush(QBrush(linearGrad));
			break;
		}
		case Background::ColorStyle::TopLeftDiagonalLinearGradient: {
			QLinearGradient linearGrad(rect.topLeft(), rect.bottomRight());
			linearGrad.setColorAt(0, background->firstColor());
			linearGrad.setColorAt(1, background->secondColor());
			painter->setBrush(QBrush(linearGrad));
			break;
		}
		case Background::ColorStyle::BottomLeftDiagonalLinearGradient: {
			QLinearGradient linearGrad(rect.bottomLeft(), rect.topRight());
			linearGrad.setColorAt(0, background->firstColor());
			linearGrad.setColorAt(1, background->secondColor());
			painter->setBrush(QBrush(linearGrad));
			break;
		}
		case Background::ColorStyle::RadialGradient: {
			QRadialGradient radialGrad(rect.center(), rect.width() / 2);
			radialGrad.setColorAt(0, background->firstColor());
			radialGrad.setColorAt(1, background->secondColor());
			painter->setBrush(QBrush(radialGrad));
			break;
		}
		}
	} else if (background->type() == Background::Type::Image) {
		if (!background->fileName().trimmed().isEmpty()) {
			QPixmap pix(background->fileName());
			switch (background->imageStyle()) {
			case Background::ImageStyle::ScaledCropped:
				pix = pix.scaled(rect.size().toSize(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
				painter->setBrush(QBrush(pix));
				painter->setBrushOrigin(pix.size().width() / 2, pix.size().height() / 2);
				break;
			case Background::ImageStyle::Scaled:
				pix = pix.scaled(rect.size().toSize(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
				painter->setBrush(QBrush(pix));
				painter->setBrushOrigin(pix.size().width() / 2, pix.size().height() / 2);
				break;
			case Background::ImageStyle::ScaledAspectRatio:
				pix = pix.scaled(rect.size().toSize(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
				painter->setBrush(QBrush(pix));
				painter->setBrushOrigin(pix.size().width() / 2, pix.size().height() / 2);
				break;
			case Background::ImageStyle::Centered: {
				QPixmap backpix(rect.size().toSize());
				backpix.fill();
				QPainter p(&backpix);
				p.drawPixmap(QPointF(0, 0), pix);
				p.end();
				painter->setBrush(QBrush(backpix));
				painter->setBrushOrigin(-pix.size().width() / 2, -pix.size().height() / 2);
				break;
			}
			case Background::ImageStyle::Tiled:
				painter->setBrush(QBrush(pix));
				break;
			case Background::ImageStyle::CenterTiled:
				painter->setBrush(QBrush(pix));
				painter->setBrushOrigin(pix.size().width() / 2, pix.size().height() / 2);
			}
		}
	} else if (background->type() == Background::Type::Pattern)
		painter->setBrush(QBrush(background->firstColor(), background->brushStyle()));

	painter->drawPolygon(polygon);
}

void BarPlotPrivate::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget*) {
	if (!isVisible())
		return;

	painter->setPen(Qt::NoPen);
	painter->setBrush(Qt::NoBrush);
	painter->setRenderHint(QPainter::SmoothPixmapTransform, true);

	if (KSharedConfig::openConfig()->group("Settings_Worksheet").readEntry<bool>("DoubleBuffering", true))
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

void BarPlotPrivate::contextMenuEvent(QGraphicsSceneContextMenuEvent* event) {
	q->createContextMenu()->exec(event->screenPos());
}

void BarPlotPrivate::hoverEnterEvent(QGraphicsSceneHoverEvent*) {
	if (!isSelected()) {
		m_hovered = true;
		Q_EMIT q->hovered();
		update();
	}
}

void BarPlotPrivate::hoverLeaveEvent(QGraphicsSceneHoverEvent*) {
	if (m_hovered) {
		m_hovered = false;
		Q_EMIT q->unhovered();
		update();
	}
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################
//! Save as XML
void BarPlot::save(QXmlStreamWriter* writer) const {
	Q_D(const BarPlot);

	writer->writeStartElement("barPlot");
	writeBasicAttributes(writer);
	writeCommentElement(writer);

	// general
	writer->writeStartElement("general");
	writer->writeAttribute("type", QString::number(static_cast<int>(d->type)));
	writer->writeAttribute("orientation", QString::number(static_cast<int>(d->orientation)));
	writer->writeAttribute("widthFactor", QString::number(d->widthFactor));
	writer->writeAttribute("plotRangeIndex", QString::number(m_cSystemIndex));
	writer->writeAttribute("xMin", QString::number(d->xMin));
	writer->writeAttribute("xMax", QString::number(d->xMax));
	writer->writeAttribute("yMin", QString::number(d->yMin));
	writer->writeAttribute("yMax", QString::number(d->yMax));

	if (d->xColumn)
		writer->writeAttribute("xColumn", d->xColumn->path());

	for (auto* column : d->dataColumns) {
		writer->writeStartElement("column");
		writer->writeAttribute("path", column->path());
		writer->writeEndElement();
	}
	writer->writeEndElement();

	// box filling
	for (auto* background : d->backgrounds)
		background->save(writer);

	// box border
	writer->writeStartElement("border");
	WRITE_QPEN(d->borderPen);
	writer->writeAttribute("opacity", QString::number(d->borderOpacity));
	writer->writeEndElement();

	writer->writeEndElement(); // close "BarPlot" section
}

//! Load from XML
bool BarPlot::load(XmlStreamReader* reader, bool preview) {
	Q_D(BarPlot);

	if (!readBasicAttributes(reader))
		return false;

	KLocalizedString attributeWarning = ki18n("Attribute '%1' missing or empty, default value is used");
	QXmlStreamAttributes attribs;
	QString str;
	bool firstBackgroundRead = false;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == "barPlot")
			break;

		if (!reader->isStartElement())
			continue;

		if (!preview && reader->name() == "comment") {
			if (!readCommentElement(reader))
				return false;
		} else if (!preview && reader->name() == "general") {
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
		} else if (reader->name() == "column") {
			attribs = reader->attributes();

			str = attribs.value("path").toString();
			if (!str.isEmpty())
				d->dataColumnPaths << str;
			// 			READ_COLUMN(dataColumn);
		} else if (!preview && reader->name() == "filling") {
			if(!firstBackgroundRead) {
				auto* background = d->backgrounds.at(0);
				background->load(reader, preview);
				firstBackgroundRead = true;
			} else {
				auto* background = new Background(QString());
				background->setPrefix(QLatin1String("Filling"));
				background->setEnabledAvailable(true);
				background->setHidden(true);
				addChild(background);
				connect(background, &Background::updateRequested, [=] {
					d->updatePixmap();
				});
				background->load(reader, preview);
				d->backgrounds << background;
			}
		} else if (!preview && reader->name() == "border") {
			attribs = reader->attributes();

			READ_QPEN(d->borderPen);
			READ_DOUBLE_VALUE("opacity", borderOpacity);
		} else { // unknown element
			reader->raiseWarning(i18n("unknown element '%1'", reader->name().toString()));
			if (!reader->skipToEndElement())
				return false;
		}
	}

	d->dataColumns.resize(d->dataColumnPaths.size());

	return true;
}

//##############################################################################
//#########################  Theme management ##################################
//##############################################################################
void BarPlot::loadThemeConfig(const KConfig& config) {
	KConfigGroup group;
	if (config.hasGroup(QLatin1String("Theme")))
		group = config.group("XYCurve"); // when loading from the theme config, use the same properties as for XYCurve
	else
		group = config.group("BarPlot");

	const auto* plot = static_cast<const CartesianPlot*>(parentAspect());
	int index = plot->curveChildIndex(this);
	const QColor themeColor = plot->themeColorPalette(index);

	QPen p;

	Q_D(BarPlot);
	d->m_suppressRecalc = false;

	// box border
	p.setStyle((Qt::PenStyle)group.readEntry("LineStyle", (int)Qt::SolidLine));
	p.setWidthF(group.readEntry("LineWidth", Worksheet::convertToSceneUnits(1.0, Worksheet::Unit::Point)));
	if (plot->theme() != QLatin1String("Tufte"))
		p.setColor(themeColor);
	else
		p.setColor(Qt::white); // white color for bar borders in Tufte's theme
	setBorderPen(p);
	setBorderOpacity(group.readEntry("LineOpacity", 1.0));

	// box filling
	for (int i = 0; i < d->backgrounds.count(); ++i) {
		auto* background = d->backgrounds.at(i);
		background->loadThemeConfig(group);
		background->setFirstColor(plot->themeColorPalette(i));
	}

	d->m_suppressRecalc = false;
	d->recalcShapeAndBoundingRect();
}
