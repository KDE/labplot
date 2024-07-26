/*
	File                 : LollipopPlot.cpp
	Project              : LabPlot
	Description          : Lollipop Plot
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "LollipopPlot.h"
#include "LollipopPlotPrivate.h"
#include "backend/core/Settings.h"
#include "backend/core/column/Column.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/trace.h"
#include "backend/worksheet/Line.h"
#include "backend/worksheet/plots/cartesian/CartesianCoordinateSystem.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/worksheet/plots/cartesian/Symbol.h"
#include "backend/worksheet/plots/cartesian/Value.h"
#include "kdefrontend/GuiTools.h"
#include "tools/ImageTools.h"

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>

#include <QActionGroup>
#include <QGraphicsSceneMouseEvent>
#include <QMenu>
#include <QPainter>

/**
 * \class LollipopPlot
 * \brief Lollipop Plot
 */

LollipopPlot::LollipopPlot(const QString& name)
	: Plot(name, new LollipopPlotPrivate(this), AspectType::LollipopPlot) {
	init();
}

LollipopPlot::LollipopPlot(const QString& name, LollipopPlotPrivate* dd)
	: Plot(name, dd, AspectType::LollipopPlot) {
	init();
}

// no need to delete the d-pointer here - it inherits from QGraphicsItem
// and is deleted during the cleanup in QGraphicsScene
LollipopPlot::~LollipopPlot() = default;

void LollipopPlot::init() {
	Q_D(LollipopPlot);

	KConfig config;
	const auto& group = config.group(QStringLiteral("LollipopPlot"));

	// general
	d->orientation = (LollipopPlot::Orientation)group.readEntry(QStringLiteral("Orientation"), (int)LollipopPlot::Orientation::Vertical);

	// initial line, symbol and value objects that will be available even if not data column was set yet
	d->addLine(group);
	d->addSymbol(group);
	d->addValue(group);
}

/*!
	Returns an icon to be used in the project explorer.
*/
QIcon LollipopPlot::icon() const {
	return LollipopPlot::staticIcon();
}

QIcon LollipopPlot::staticIcon() {
	QPainter pa;
	pa.setRenderHint(QPainter::Antialiasing);
	int iconSize = 20;
	QPixmap pm(iconSize, iconSize);

	QPen pen(Qt::SolidLine);
	pen.setColor(GuiTools::isDarkMode() ? Qt::white : Qt::black);
	pen.setWidthF(0.0);

	pm.fill(Qt::transparent);
	pa.begin(&pm);
	pa.setPen(pen);
	pa.setBrush(pen.color());
	pa.drawLine(10, 6, 10, 14);
	pa.drawEllipse(8, 4, 4, 4);
	pa.end();

	return {pm};
}

void LollipopPlot::initActions() {
	// Orientation
	auto* orientationActionGroup = new QActionGroup(this);
	orientationActionGroup->setExclusive(true);
	connect(orientationActionGroup, &QActionGroup::triggered, this, &LollipopPlot::orientationChangedSlot);

	orientationHorizontalAction = new QAction(QIcon::fromTheme(QStringLiteral("transform-move-horizontal")), i18n("Horizontal"), orientationActionGroup);
	orientationHorizontalAction->setCheckable(true);

	orientationVerticalAction = new QAction(QIcon::fromTheme(QStringLiteral("transform-move-vertical")), i18n("Vertical"), orientationActionGroup);
	orientationVerticalAction->setCheckable(true);
}

void LollipopPlot::initMenus() {
	this->initActions();

	// Orientation
	orientationMenu = new QMenu(i18n("Orientation"));
	orientationMenu->setIcon(QIcon::fromTheme(QStringLiteral("draw-cross")));
	orientationMenu->addAction(orientationHorizontalAction);
	orientationMenu->addAction(orientationVerticalAction);
}

QMenu* LollipopPlot::createContextMenu() {
	if (!orientationMenu)
		initMenus();

	QMenu* menu = WorksheetElement::createContextMenu();
	QAction* visibilityAction = this->visibilityAction();

	// Orientation
	Q_D(const LollipopPlot);
	if (d->orientation == Orientation::Horizontal)
		orientationHorizontalAction->setChecked(true);
	else
		orientationVerticalAction->setChecked(true);
	menu->insertMenu(visibilityAction, orientationMenu);
	menu->insertSeparator(visibilityAction);

	return menu;
}

void LollipopPlot::retransform() {
	Q_D(LollipopPlot);
	d->retransform();
}

void LollipopPlot::recalc() {
	Q_D(LollipopPlot);
	d->recalc();
}

void LollipopPlot::handleResize(double /*horizontalRatio*/, double /*verticalRatio*/, bool /*pageResize*/) {
}

/* ============================ getter methods ================= */
// general
BASIC_SHARED_D_READER_IMPL(LollipopPlot, QVector<const AbstractColumn*>, dataColumns, dataColumns)
BASIC_SHARED_D_READER_IMPL(LollipopPlot, LollipopPlot::Orientation, orientation, orientation)
BASIC_SHARED_D_READER_IMPL(LollipopPlot, const AbstractColumn*, xColumn, xColumn)

QString& LollipopPlot::xColumnPath() const {
	D(LollipopPlot);
	return d->xColumnPath;
}

Line* LollipopPlot::lineAt(int index) const {
	Q_D(const LollipopPlot);
	if (index < d->lines.size())
		return d->lines.at(index);
	else
		return nullptr;
}

Symbol* LollipopPlot::symbolAt(int index) const {
	Q_D(const LollipopPlot);
	if (index < d->symbols.size())
		return d->symbols.at(index);
	else
		return nullptr;
}

QVector<QString>& LollipopPlot::dataColumnPaths() const {
	D(LollipopPlot);
	return d->dataColumnPaths;
}

double LollipopPlot::minimum(const Dimension dim) const {
	Q_D(const LollipopPlot);
	switch (dim) {
	case Dimension::X:
		return d->xMin;
	case Dimension::Y:
		return d->yMin;
	}
	return NAN;
}

double LollipopPlot::maximum(const Dimension dim) const {
	Q_D(const LollipopPlot);
	switch (dim) {
	case Dimension::X:
		return d->xMax;
	case Dimension::Y:
		return d->yMax;
	}
	return NAN;
}

bool LollipopPlot::hasData() const {
	Q_D(const LollipopPlot);
	return !d->dataColumns.isEmpty();
}

bool LollipopPlot::usingColumn(const Column* column) const {
	Q_D(const LollipopPlot);

	if (d->xColumn == column)
		return true;

	for (auto* c : d->dataColumns) {
		if (c == column)
			return true;
	}

	return false;
}

void LollipopPlot::handleAspectUpdated(const QString& aspectPath, const AbstractAspect* aspect) {
	Q_D(const LollipopPlot);
	const auto column = dynamic_cast<const AbstractColumn*>(aspect);
	if (!column)
		return;
	const auto dataColumnPaths = d->dataColumnPaths;
	auto dataColumns = d->dataColumns;
	bool changed = false;

	for (int i = 0; i < dataColumnPaths.count(); ++i) {
		const auto& path = dataColumnPaths.at(i);

		if (path == aspectPath) {
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

QColor LollipopPlot::color() const {
	Q_D(const LollipopPlot);
	if (d->lines.size() > 0 && d->lines.at(0)->style() != Qt::PenStyle::NoPen)
		return d->lines.at(0)->pen().color();
	else if (d->symbols.size() > 0 && d->symbols.at(0)->style() != Symbol::Style::NoSymbols)
		return d->symbols.at(0)->pen().color();
	return QColor();
}

// values
Value* LollipopPlot::value() const {
	Q_D(const LollipopPlot);
	return d->value;
}

/* ============================ setter methods and undo commands ================= */

// General
STD_SETTER_CMD_IMPL_F_S(LollipopPlot, SetXColumn, const AbstractColumn*, xColumn, recalc)
void LollipopPlot::setXColumn(const AbstractColumn* column) {
	Q_D(LollipopPlot);
	if (column != d->xColumn) {
		exec(new LollipopPlotSetXColumnCmd(d, column, ki18n("%1: set x column")));

		if (column) {
			// update the curve itself on changes
			connect(column, &AbstractColumn::dataChanged, this, &LollipopPlot::recalc);
			if (column->parentAspect())
				connect(column->parentAspect(), &AbstractAspect::childAspectAboutToBeRemoved, this, &LollipopPlot::dataColumnAboutToBeRemoved);

			connect(column, &AbstractColumn::dataChanged, this, &LollipopPlot::dataChanged);
			// TODO: add disconnect in the undo-function
		}
	}
}

STD_SETTER_CMD_IMPL_F_S(LollipopPlot, SetDataColumns, QVector<const AbstractColumn*>, dataColumns, recalc)
void LollipopPlot::setDataColumns(const QVector<const AbstractColumn*> columns) {
	Q_D(LollipopPlot);
	if (columns != d->dataColumns) {
		exec(new LollipopPlotSetDataColumnsCmd(d, columns, ki18n("%1: set data columns")));

		for (auto* column : columns) {
			if (!column)
				continue;

			// update the curve itself on changes
			connect(column, &AbstractColumn::dataChanged, this, &LollipopPlot::recalc);
			if (column->parentAspect())
				connect(column->parentAspect(), &AbstractAspect::childAspectAboutToBeRemoved, this, &LollipopPlot::dataColumnAboutToBeRemoved);
			// TODO: add disconnect in the undo-function

			connect(column, &AbstractColumn::dataChanged, this, &LollipopPlot::dataChanged);
			connect(column, &AbstractAspect::aspectDescriptionChanged, this, &Plot::appearanceChanged);
		}
	}
}

STD_SETTER_CMD_IMPL_F_S(LollipopPlot, SetOrientation, LollipopPlot::Orientation, orientation, recalc)
void LollipopPlot::setOrientation(LollipopPlot::Orientation orientation) {
	Q_D(LollipopPlot);
	if (orientation != d->orientation)
		exec(new LollipopPlotSetOrientationCmd(d, orientation, ki18n("%1: set orientation")));
}

// ##############################################################################
// #################################  SLOTS  ####################################
// ##############################################################################

void LollipopPlot::dataColumnAboutToBeRemoved(const AbstractAspect* aspect) {
	Q_D(LollipopPlot);
	for (int i = 0; i < d->dataColumns.size(); ++i) {
		if (aspect == d->dataColumns.at(i)) {
			d->dataColumns[i] = nullptr;
			d->retransform();
			Q_EMIT dataChanged();
			Q_EMIT changed();
			break;
		}
	}
}

// ##############################################################################
// ######  SLOTs for changes triggered via QActions in the context menu  ########
// ##############################################################################
void LollipopPlot::orientationChangedSlot(QAction* action) {
	if (action == orientationHorizontalAction)
		this->setOrientation(Axis::Orientation::Horizontal);
	else
		this->setOrientation(Axis::Orientation::Vertical);
}

// ##############################################################################
// ####################### Private implementation ###############################
// ##############################################################################
LollipopPlotPrivate::LollipopPlotPrivate(LollipopPlot* owner)
	: PlotPrivate(owner)
	, q(owner) {
	setFlag(QGraphicsItem::ItemIsSelectable);
	setAcceptHoverEvents(false);
}

Line* LollipopPlotPrivate::addLine(const KConfigGroup& group) {
	auto* line = new Line(QString());
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

	lines << line;

	return line;
}

Symbol* LollipopPlotPrivate::addSymbol(const KConfigGroup& group) {
	auto* symbol = new Symbol(QString());
	symbol->setHidden(true);
	q->addChild(symbol);

	if (!q->isLoading())
		symbol->init(group);

	q->connect(symbol, &Symbol::updateRequested, [=] {
		updatePixmap();
		Q_EMIT q->appearanceChanged();
	});

	symbols << symbol;

	return symbol;
}

void LollipopPlotPrivate::addValue(const KConfigGroup& group) {
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

/*!
  called when the size of the plot or its data ranges (manual changes, zooming, etc.) were changed.
  recalculates the position of the scene points to be drawn.
  triggers the update of lines, drop lines, symbols etc.
*/
void LollipopPlotPrivate::retransform() {
	if (suppressRetransform || !isVisible() || q->isLoading())
		return;

	PERFTRACE(name() + QLatin1String(Q_FUNC_INFO));

	const int count = dataColumns.size();
	if (!count || m_barLines.size() != count) {
		// no columns or recalc() was not called yet, nothing to do
		recalcShapeAndBoundingRect();
		return;
	}

	m_valuesPointsLogical.clear();

	if (count) {
		if (orientation == LollipopPlot::Orientation::Vertical) {
			for (int i = 0; i < count; ++i) {
				if (dataColumns.at(i))
					verticalPlot(i);
			}
		} else {
			for (int i = 0; i < count; ++i) {
				if (dataColumns.at(i))
					horizontalPlot(i);
			}
		}
	}

	updateValues(); // this also calls recalcShapeAndBoundingRect()
}

/*!
 * called when the data columns or their values were changed
 * calculates the min and max values for x and y and calls dataChanged()
 * to trigger the retransform in the parent plot
 */
void LollipopPlotPrivate::recalc() {
	PERFTRACE(name() + QLatin1String(Q_FUNC_INFO));

	const int newSize = dataColumns.size();
	// resize the internal containers
	m_barLines.clear();
	m_barLines.resize(newSize);
	m_symbolPoints.clear();
	m_symbolPoints.resize(newSize);

	const double xMinOld = xMin;
	const double xMaxOld = xMax;
	const double yMinOld = yMin;
	const double yMaxOld = yMax;

	// bar properties
	int diff = newSize - lines.size();
	if (diff > 0) {
		// one more bar needs to be added
		KConfig config;
		KConfigGroup group = config.group(QLatin1String("LollipopPlot"));
		const auto* plot = static_cast<const CartesianPlot*>(q->parentAspect());

		for (int i = 0; i < diff; ++i) {
			auto* line = addLine(group);
			auto* symbol = addSymbol(group);

			if (plot) {
				const auto& themeColor = plot->themeColorPalette(lines.count() - 1);
				line->setColor(themeColor);
				symbol->setColor(themeColor);
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
		m_symbolPoints[columnIndex].resize(size);
		if (size > barGroupsCount)
			barGroupsCount = size;

		++columnIndex;
	}

	// if an x-column was provided and it has less values than the count determined
	// above, we limit the number of bars to the number of values in the x-column
	if (xColumn) {
		int size = static_cast<const Column*>(xColumn)->statistics().size;
		if (size < barGroupsCount)
			barGroupsCount = size;
	}

	// determine min and max values for x- and y-ranges.
	// the first group is placed between 0 and 1, the second one between 1 and 2, etc.
	if (orientation == LollipopPlot::Orientation::Vertical) {
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
		for (auto* column : dataColumns) {
			double max = column->maximum();
			if (max > yMax)
				yMax = max;

			double min = column->minimum();
			if (min < yMin)
				yMin = min;
		}

		// if there are no negative values, we plot
		// in the positive y-direction only and we start at y=0
		if (yMin > 0)
			yMin = 0;
	} else { // horizontal
		// min/max for x
		xMin = 0;
		xMax = -INFINITY;
		for (auto* column : dataColumns) {
			double max = column->maximum();
			if (max > xMax)
				xMax = max;

			double min = column->minimum();
			if (min < xMin)
				xMin = min;
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
	// data changes or because of new boxplot settings, emit dataChanged()
	// in order to recalculate the data ranges in the parent plot area
	// and to retransform all its children.
	// Just call retransform() to update the plot only if the ranges didn't change.
	if (xMin != xMinOld || xMax != xMaxOld || yMin != yMinOld || yMax != yMaxOld)
		Q_EMIT q->dataChanged();
	else
		retransform();
}

void LollipopPlotPrivate::verticalPlot(int columnIndex) {
	PERFTRACE(name() + QLatin1String(Q_FUNC_INFO));

	const auto* column = static_cast<const Column*>(dataColumns.at(columnIndex));
	QVector<QLineF> barLines; // lines for all bars for one colum in scene coordinates
	QVector<QPointF> symbolPoints;

	const double barGap = m_groupWidth * 0.1; // gap between two bars within a group
	const int barCount = dataColumns.size(); // number of bars within a group
	const double width = (m_groupWidth - 2 * m_groupGap - (barCount - 1) * barGap) / barCount; // bar width

	int valueIndex = 0;
	for (int i = 0; i < column->rowCount(); ++i) {
		if (!column->isValid(i) || column->isMasked(i))
			continue;

		const double value = column->valueAt(i);
		double x;

		if (xColumn)
			x = xColumn->valueAt(i);
		else
			x = m_groupGap
				+ valueIndex * m_groupWidth; // translate to the beginning of the group - 1st group is placed between 0 and 1, 2nd between 1 and 2, etc.

		x += (width + barGap) * columnIndex; // translate to the beginning of the bar within the current group

		symbolPoints << QPointF(x + width / 2, value);
		m_valuesPointsLogical << QPointF(x + width / 2, value);
		barLines << QLineF(x + width / 2, 0, x + width / 2, value);
		++valueIndex;
	}

	m_barLines[columnIndex] = q->cSystem->mapLogicalToScene(barLines);
	m_symbolPoints[columnIndex] = q->cSystem->mapLogicalToScene(symbolPoints);
}

void LollipopPlotPrivate::horizontalPlot(int columnIndex) {
	PERFTRACE(name() + QLatin1String(Q_FUNC_INFO));

	const auto* column = static_cast<const Column*>(dataColumns.at(columnIndex));
	QVector<QLineF> barLines; // lines for all bars for one colum in scene coordinates
	QVector<QPointF> symbolPoints;

	const double barGap = m_groupWidth * 0.1; // gap between two bars within a group
	const int barCount = dataColumns.size(); // number of bars within a group
	const double width = (m_groupWidth - 2 * m_groupGap - (barCount - 1) * barGap) / barCount; // bar width

	int valueIndex = 0;
	for (int i = 0; i < column->rowCount(); ++i) {
		if (!column->isValid(i) || column->isMasked(i))
			continue;

		const double value = column->valueAt(i);
		double y;
		if (xColumn)
			y = xColumn->valueAt(i);
		else
			y = m_groupGap + valueIndex * m_groupWidth; // translate to the beginning of the group

		y += (width + barGap) * columnIndex; // translate to the beginning of the bar within the current group

		symbolPoints << QPointF(value, y - width / 2);
		m_valuesPointsLogical << QPointF(value, y - width / 2);
		barLines << QLineF(0, y - width / 2, value, y - width / 2);
		++valueIndex;
	}

	m_barLines[columnIndex] = q->cSystem->mapLogicalToScene(barLines);
	m_symbolPoints[columnIndex] = q->cSystem->mapLogicalToScene(symbolPoints);
}

void LollipopPlotPrivate::updateValues() {
	m_valuesPath = QPainterPath();
	m_valuesPoints.clear();
	m_valuesStrings.clear();

	if (value->type() == Value::NoValues) {
		recalcShapeAndBoundingRect();
		return;
	}

	// determine the value string for all points that are currently visible in the plot
	auto visiblePoints = std::vector<bool>(m_valuesPointsLogical.count(), false);
	Points pointsScene;
	q->cSystem->mapLogicalToScene(m_valuesPointsLogical, pointsScene, visiblePoints);
	const auto& prefix = value->prefix();
	const auto& suffix = value->suffix();
	if (value->type() == Value::BinEntries) {
		for (int i = 0; i < m_valuesPointsLogical.count(); ++i) {
			if (!visiblePoints[i])
				continue;

			auto& point = m_valuesPointsLogical.at(i);
			if (orientation == LollipopPlot::Orientation::Vertical)
				m_valuesStrings << prefix + QString::number(point.y()) + suffix;
			else
				m_valuesStrings << prefix + QString::number(point.x()) + suffix;
		}
	} else if (value->type() == Value::CustomColumn) {
		const auto* valuesColumn = value->column();
		if (!valuesColumn) {
			recalcShapeAndBoundingRect();
			return;
		}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
		const int endRow = std::min(m_valuesPointsLogical.size(), static_cast<qsizetype>(valuesColumn->rowCount()));
#else
		const int endRow = std::min(m_valuesPointsLogical.size(), valuesColumn->rowCount());
#endif
		const auto xColMode = valuesColumn->columnMode();
		for (int i = 0; i < endRow; ++i) {
			if (!valuesColumn->isValid(i) || valuesColumn->isMasked(i))
				continue;

			switch (xColMode) {
			case AbstractColumn::ColumnMode::Double:
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
			if (orientation == LollipopPlot::Orientation::Vertical)
				m_valuesPoints << QPointF(pointsScene.at(i).x() - w / 2, pointsScene.at(i).y() - offset);
			else
				m_valuesPoints << QPointF(pointsScene.at(i).x() + offset, pointsScene.at(i).y() - w / 2);
		}
		break;
	case Value::Under:
		for (int i = 0; i < m_valuesStrings.size(); i++) {
			w = fm.boundingRect(m_valuesStrings.at(i)).width();
			if (orientation == LollipopPlot::Orientation::Vertical)
				m_valuesPoints << QPointF(pointsScene.at(i).x() - w / 2, pointsScene.at(i).y() + offset + h / 2);
			else
				m_valuesPoints << QPointF(pointsScene.at(i).x() - offset - h / 2, pointsScene.at(i).y() - w / 2);
		}
		break;
	case Value::Left:
		for (int i = 0; i < m_valuesStrings.size(); i++) {
			w = fm.boundingRect(m_valuesStrings.at(i)).width();
			if (orientation == LollipopPlot::Orientation::Vertical)
				m_valuesPoints << QPointF(pointsScene.at(i).x() - offset - w, pointsScene.at(i).y());
			else
				m_valuesPoints << QPointF(pointsScene.at(i).x(), pointsScene.at(i).y() - offset - w);
		}
		break;
	case Value::Right:
		for (int i = 0; i < m_valuesStrings.size(); i++) {
			w = fm.boundingRect(m_valuesStrings.at(i)).width();
			if (orientation == LollipopPlot::Orientation::Vertical)
				m_valuesPoints << QPointF(pointsScene.at(i).x() + offset, pointsScene.at(i).y());
			else
				m_valuesPoints << QPointF(pointsScene.at(i).x(), pointsScene.at(i).y() + offset);
		}
		break;
	case Value::Center:
		QVector<qreal> listBarWidth;
		for (const auto& columnBarLines : m_barLines) // loop over the different data columns
			for (const auto& line : columnBarLines)
				listBarWidth.append(line.length());

		for (int i = 0; i < m_valuesStrings.size(); i++) {
			w = fm.boundingRect(m_valuesStrings.at(i)).width();
			const auto& point = pointsScene.at(i);
			if (orientation == LollipopPlot::Orientation::Vertical)
				m_valuesPoints << QPointF(point.x() - w / 2,
										  point.y() + listBarWidth.at(i) / 2 + offset - Worksheet::convertToSceneUnits(1, Worksheet::Unit::Point));
			else
				m_valuesPoints << QPointF(point.x() - listBarWidth.at(i) / 2 - offset + h / 2 - w / 2, point.y() + h / 2);
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

/*!
  recalculates the outer bounds and the shape of the item.
*/
void LollipopPlotPrivate::recalcShapeAndBoundingRect() {
	prepareGeometryChange();
	m_shape = QPainterPath();

	// lines
	int index = 0;
	for (const auto& columnBarLines : m_barLines) { // loop over the different data columns
		for (const auto& line : columnBarLines) { // loop over the bars for every data column
			QPainterPath barPath;
			barPath.moveTo(line.p1());
			barPath.lineTo(line.p2());

			if (index < lines.count()) { // TODO
				const auto& borderPen = lines.at(index)->pen();
				m_shape.addPath(WorksheetElement::shapeFromPath(barPath, borderPen));
			}
		}
		++index;
	}

	// symbols
	auto symbolsPath = QPainterPath();
	index = 0;
	for (const auto& symbolPoints : m_symbolPoints) { // loop over the different data columns
		if (index > symbols.count() - 1)
			continue;

		const auto* symbol = symbols.at(index);
		if (symbol->style() != Symbol::Style::NoSymbols) {
			auto path = WorksheetElement::shapeFromPath(Symbol::stylePath(symbol->style()), symbol->pen());

			QTransform trafo;
			trafo.scale(symbol->size(), symbol->size());
			path = trafo.map(path);
			trafo.reset();

			if (symbol->rotationAngle() != 0.) {
				trafo.rotate(symbol->rotationAngle());
				path = trafo.map(path);
			}

			for (const auto& point : symbolPoints) { // loop over the points for every data column
				trafo.reset();
				trafo.translate(point.x(), point.y());
				symbolsPath.addPath(trafo.map(path));
			}
		}
		++index;
	}

	m_shape.addPath(symbolsPath);

	if (value->type() != Value::NoValues)
		m_shape.addPath(m_valuesPath);

	m_boundingRectangle = m_shape.boundingRect();
	updatePixmap();
}

void LollipopPlotPrivate::updatePixmap() {
	PERFTRACE(name() + QLatin1String(Q_FUNC_INFO));
	m_pixmap = QPixmap(m_boundingRectangle.width(), m_boundingRectangle.height());
	if (m_boundingRectangle.width() == 0. || m_boundingRectangle.height() == 0.) {
		m_hoverEffectImageIsDirty = true;
		m_selectionEffectImageIsDirty = true;
		return;
	}
	m_pixmap.fill(Qt::transparent);
	QPainter painter(&m_pixmap);
	painter.setRenderHint(QPainter::Antialiasing, true);
	painter.translate(-m_boundingRectangle.topLeft());

	draw(&painter);
	painter.end();

	m_hoverEffectImageIsDirty = true;
	m_selectionEffectImageIsDirty = true;
	Q_EMIT q->changed();
	update();
}

void LollipopPlotPrivate::draw(QPainter* painter) {
	PERFTRACE(name() + QLatin1String(Q_FUNC_INFO));

	int columnIndex = 0;
	for (const auto& columnBarLines : m_barLines) { // loop over the different data columns
		// draw the lines
		if (columnIndex < lines.size()) { // TODO: remove this check later
			const auto& borderPen = lines.at(columnIndex)->pen();
			const double borderOpacity = lines.at(columnIndex)->opacity();
			for (const auto& line : columnBarLines) { // loop over the bars for every data column
				if (borderPen.style() != Qt::NoPen) {
					painter->setPen(borderPen);
					painter->setBrush(Qt::NoBrush);
					painter->setOpacity(borderOpacity);
					painter->drawLine(line);
				}
			}
		}

		// draw symbols
		if (columnIndex < symbols.size())
			symbols.at(columnIndex)->draw(painter, m_symbolPoints.at(columnIndex));

		++columnIndex;
	}

	// draw values
	value->draw(painter, m_valuesPoints, m_valuesStrings);
}

void LollipopPlotPrivate::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget*) {
	if (!isVisible())
		return;

	painter->setPen(Qt::NoPen);
	painter->setBrush(Qt::NoBrush);
	painter->setRenderHint(QPainter::SmoothPixmapTransform, true);

	if (!q->isPrinting() && Settings::group(QStringLiteral("Settings_Worksheet")).readEntry<bool>("DoubleBuffering", true))
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
void LollipopPlot::save(QXmlStreamWriter* writer) const {
	Q_D(const LollipopPlot);

	writer->writeStartElement(QStringLiteral("lollipopPlot"));
	writeBasicAttributes(writer);
	writeCommentElement(writer);

	// general
	writer->writeStartElement(QStringLiteral("general"));
	writer->writeAttribute(QStringLiteral("orientation"), QString::number(static_cast<int>(d->orientation)));
	writer->writeAttribute(QStringLiteral("plotRangeIndex"), QString::number(m_cSystemIndex));
	writer->writeAttribute(QStringLiteral("xMin"), QString::number(d->xMin));
	writer->writeAttribute(QStringLiteral("xMax"), QString::number(d->xMax));
	writer->writeAttribute(QStringLiteral("yMin"), QString::number(d->yMin));
	writer->writeAttribute(QStringLiteral("yMax"), QString::number(d->yMax));
	writer->writeAttribute(QStringLiteral("legendVisible"), QString::number(d->legendVisible));
	writer->writeAttribute(QStringLiteral("visible"), QString::number(d->isVisible()));

	if (d->xColumn)
		writer->writeAttribute(QStringLiteral("xColumn"), d->xColumn->path());

	for (auto* column : d->dataColumns) {
		writer->writeStartElement(QStringLiteral("column"));
		writer->writeAttribute(QStringLiteral("path"), column->path());
		writer->writeEndElement();
	}
	writer->writeEndElement();

	// lines
	for (auto* line : d->lines)
		line->save(writer);

	// symbols
	for (auto* symbol : d->symbols)
		symbol->save(writer);

	// Values
	d->value->save(writer);

	writer->writeEndElement(); // close "LollipopPlot" section
}

//! Load from XML
bool LollipopPlot::load(XmlStreamReader* reader, bool preview) {
	Q_D(LollipopPlot);

	if (!readBasicAttributes(reader))
		return false;

	QXmlStreamAttributes attribs;
	QString str;
	bool firstLineRead = false;
	bool firstSymbolRead = false;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == QLatin1String("lollipopPlot"))
			break;

		if (!reader->isStartElement())
			continue;

		if (!preview && reader->name() == QLatin1String("comment")) {
			if (!readCommentElement(reader))
				return false;
		} else if (!preview && reader->name() == QLatin1String("general")) {
			attribs = reader->attributes();

			READ_INT_VALUE("orientation", orientation, LollipopPlot::Orientation);
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
		} else if (!preview && reader->name() == QLatin1String("line")) {
			if (!firstLineRead) {
				auto* line = d->lines.at(0);
				line->load(reader, preview);
				firstLineRead = true;
			} else {
				auto* line = d->addLine(KConfigGroup());
				line->load(reader, preview);
			}
		} else if (!preview && reader->name() == QLatin1String("symbol")) {
			if (!firstSymbolRead) {
				auto* symbol = d->symbols.at(0);
				symbol->load(reader, preview);
				firstSymbolRead = true;
			} else {
				auto* symbol = d->addSymbol(KConfigGroup());
				symbol->load(reader, preview);
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
void LollipopPlot::loadThemeConfig(const KConfig& config) {
	KConfigGroup group;
	if (config.hasGroup(QStringLiteral("Theme")))
		group = config.group(QStringLiteral("XYCurve")); // when loading from the theme config, use the same properties as for XYCurve
	else
		group = config.group(QStringLiteral("LollipopPlot"));

	const auto* plot = static_cast<const CartesianPlot*>(parentAspect());
	int index = plot->curveChildIndex(this);
	const QColor themeColor = plot->themeColorPalette(index);

	Q_D(LollipopPlot);
	d->suppressRecalc = true;

	for (int i = 0; i < d->dataColumns.count(); ++i) {
		const auto& color = plot->themeColorPalette(i);

		// lines
		auto* line = d->lines.at(i);
		line->loadThemeConfig(group, color);

		// symbols
		auto* symbol = d->symbols.at(i);
		symbol->loadThemeConfig(group, color);
	}

	// values
	d->value->loadThemeConfig(group, themeColor);

	d->suppressRecalc = false;
	d->recalcShapeAndBoundingRect();
}
