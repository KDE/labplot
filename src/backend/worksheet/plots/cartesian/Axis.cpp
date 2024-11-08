/*
	File                 : Axis.cpp
	Project              : LabPlot
	Description          : Axis for cartesian coordinate systems.
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2011-2018 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2013-2021 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "AxisPrivate.h"
#include "backend/core/AbstractColumn.h"
#include "backend/core/Project.h"
#include "backend/core/Time.h"
#include "backend/core/column/Column.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/macros.h"
#include "backend/lib/trace.h"
#include "backend/worksheet/Line.h"
#include "backend/worksheet/TextLabel.h"
#include "backend/worksheet/Worksheet.h"
#include "frontend/GuiTools.h"

#include "backend/nsl/nsl_math.h"
#include "backend/nsl/nsl_sf_basic.h"
#include <gsl/gsl_math.h>

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>

#include <QActionGroup>
#include <QGraphicsSceneMouseEvent>
#include <QMenu>
#include <QPainter>
#include <QTextDocument>
#include <QtMath>

using Dimension = CartesianCoordinateSystem::Dimension;

namespace {
constexpr int maxNumberMajorTicks = 100;
constexpr int _maxNumberMajorTicksCustomColumn = 21; // Use one more because one will be subtracted below
constexpr int hoverSelectionEffectPenWidth = 2;
} // Anonymous namespace

/**
 * \class AxisGrid
 * \brief Helper class to get the axis grid drawn with the z-Value=0.
 *
 * The painting of the grid lines is separated from the painting of the axis itself.
 * This allows to use a different z-values for the grid lines (z=0, drawn below all other objects )
 * and for the axis (z=FLT_MAX, drawn on top of all other objects)
 *
 *  \ingroup worksheet
 */
class AxisGrid : public QGraphicsItem {
public:
	explicit AxisGrid(AxisPrivate* a) {
		axis = a;
		setFlag(QGraphicsItem::ItemIsSelectable, false);
		setFlag(QGraphicsItem::ItemIsFocusable, false);
		setAcceptHoverEvents(false);
	}

	QRectF boundingRect() const override {
		QPainterPath gridShape;
		gridShape.addPath(WorksheetElement::shapeFromPath(axis->majorGridPath, axis->majorGridLine->pen()));
		gridShape.addPath(WorksheetElement::shapeFromPath(axis->minorGridPath, axis->minorGridLine->pen()));
		QRectF boundingRectangle = gridShape.boundingRect();
		return boundingRectangle;
	}

	void paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget*) override {
		if (!axis->isVisible() || axis->linePath.isEmpty())
			return;

		// draw major grid
		if (axis->majorGridLine->pen().style() != Qt::NoPen) {
			painter->setOpacity(axis->majorGridLine->opacity());
			painter->setPen(axis->majorGridLine->pen());
			painter->setBrush(Qt::NoBrush);
			painter->drawPath(axis->majorGridPath);
		}

		// draw minor grid
		if (axis->minorGridLine->pen().style() != Qt::NoPen) {
			painter->setOpacity(axis->minorGridLine->opacity());
			painter->setPen(axis->minorGridLine->pen());
			painter->setBrush(Qt::NoBrush);
			painter->drawPath(axis->minorGridPath);
		}
	}

private:
	AxisPrivate* axis;
};

/**
 * \class Axis
 * \brief Axis for cartesian coordinate systems.
 *
 *  \ingroup worksheet
 */
Axis::Axis(const QString& name, Orientation orientation, bool loading)
	: WorksheetElement(name, new AxisPrivate(this), AspectType::Axis) {
	init(orientation, loading);
}

Axis::Axis(const QString& name, Orientation orientation, AxisPrivate* dd)
	: WorksheetElement(name, dd, AspectType::Axis) {
	init(orientation);
}

void Axis::init(Orientation orientation, bool loading) {
	Q_D(Axis);

	// line
	d->line = new Line(QStringLiteral("line"));
	d->line->setHidden(true);
	d->line->setCreateXmlElement(false); // line properties are written out together with arrow properties in Axis::save()
	addChild(d->line);
	connect(d->line, &Line::updatePixmapRequested, [=] {
		d->update();
		Q_EMIT changed();
	});
	connect(d->line, &Line::updateRequested, [=] {
		d->recalcShapeAndBoundingRect();
	});

	// axis title
	d->title = new TextLabel(this->name(), TextLabel::Type::AxisTitle);
	d->title->setText(this->name());
	connect(d->title, &TextLabel::changed, this, &Axis::labelChanged);
	addChild(d->title);
	d->title->setHidden(true);
	d->title->graphicsItem()->setParentItem(d);
	d->title->graphicsItem()->setFlag(QGraphicsItem::ItemIsMovable, false);
	d->title->graphicsItem()->setFlag(QGraphicsItem::ItemIsFocusable, false);
	d->title->graphicsItem()->setAcceptHoverEvents(false);

	// major ticks line
	d->majorTicksLine = new Line(QStringLiteral("majorTicksLine"));
	d->majorTicksLine->setHidden(true);
	d->majorTicksLine->setPrefix(QStringLiteral("MajorTicks"));
	d->majorTicksLine->setCreateXmlElement(false);
	addChild(d->majorTicksLine);
	connect(d->majorTicksLine, &Line::updatePixmapRequested, [=] {
		d->update();
		Q_EMIT changed();
	});
	connect(d->majorTicksLine, &Line::updateRequested, [=] {
		d->recalcShapeAndBoundingRect();
	});

	// minor ticks line
	d->minorTicksLine = new Line(QStringLiteral("minorTicksLine"));
	d->minorTicksLine->setHidden(true);
	d->minorTicksLine->setPrefix(QStringLiteral("MinorTicks"));
	d->minorTicksLine->setCreateXmlElement(false);
	addChild(d->minorTicksLine);
	connect(d->minorTicksLine, &Line::updatePixmapRequested, [=] {
		d->update();
		Q_EMIT changed();
	});
	connect(d->minorTicksLine, &Line::updateRequested, [=] {
		d->recalcShapeAndBoundingRect();
	});

	// major grid line
	d->majorGridLine = new Line(QStringLiteral("majorGridLine"));
	d->majorGridLine->setPrefix(QStringLiteral("MajorGrid"));
	d->majorGridLine->setHidden(true);
	addChild(d->majorGridLine);
	connect(d->majorGridLine, &Line::updatePixmapRequested, [=] {
		d->updateGrid();
		Q_EMIT changed();
	});
	connect(d->majorGridLine, &Line::updateRequested, [=] {
		d->retransformMajorGrid();
	});

	// minor grid line
	d->minorGridLine = new Line(QStringLiteral("minorGridLine"));
	d->minorGridLine->setPrefix(QStringLiteral("MinorGrid"));
	d->minorGridLine->setHidden(true);
	addChild(d->minorGridLine);
	connect(d->minorGridLine, &Line::updatePixmapRequested, [=] {
		d->updateGrid();
		Q_EMIT changed();
	});
	connect(d->minorGridLine, &Line::updateRequested, [=] {
		d->retransformMinorGrid();
	});

	connect(this, &WorksheetElement::coordinateSystemIndexChanged, [this]() {
		Q_D(Axis);
		d->retransformRange();
	});

	if (loading)
		return;

	// init the properties
	KConfig config;
	KConfigGroup group = config.group(QStringLiteral("Axis"));

	d->orientation = orientation;
	d->rangeType = (Axis::RangeType)group.readEntry(QStringLiteral("RangeType"), static_cast<int>(RangeType::Auto));
	d->position = Axis::Position::Centered;
	d->offset = group.readEntry(QStringLiteral("PositionOffset"), 0);
	d->range = Range<double>(group.readEntry(QStringLiteral("Start"), 0.), group.readEntry("End", 10.)); // not auto ticked if already set to 1 here!
	d->range.scale() = (RangeT::Scale)group.readEntry(QStringLiteral("Scale"), static_cast<int>(RangeT::Scale::Linear));
	d->majorTicksStartType =
		static_cast<Axis::TicksStartType>(group.readEntry(QStringLiteral("MajorTicksStartType"), static_cast<bool>(Axis::TicksStartType::Offset)));
	d->majorTickStartOffset = group.readEntry(QStringLiteral("MajorTickStartOffset"), 0.0);
	d->majorTickStartValue = group.readEntry(QStringLiteral("MajorTickStartValue"), 0.0);
	d->scalingFactor = group.readEntry(QStringLiteral("ScalingFactor"), 1.0);
	d->zeroOffset = group.readEntry(QStringLiteral("ZeroOffset"), 0);
	d->showScaleOffset = group.readEntry(QStringLiteral("ShowScaleOffset"), true);

	d->line->init(group);
	d->arrowType = (Axis::ArrowType)group.readEntry(QStringLiteral("ArrowType"), static_cast<int>(ArrowType::NoArrow));
	d->arrowPosition = (Axis::ArrowPosition)group.readEntry(QStringLiteral("ArrowPosition"), static_cast<int>(ArrowPosition::Right));
	d->arrowSize = group.readEntry(QStringLiteral("ArrowSize"), Worksheet::convertToSceneUnits(10, Worksheet::Unit::Point));

	if (d->orientation == Orientation::Vertical) {
		d->title->setRotationAngle(90);
		d->titleOffsetX = 0; // distance to the axis tick labels
		d->titleOffsetY = 0; // centering the title
	} else {
		d->titleOffsetX = 0; // centering the title
		d->titleOffsetY = 0; // distance to the axis tick labels
	}

	d->majorTicksLine->init(group);
	d->majorTicksDirection = (Axis::TicksDirection)group.readEntry(QStringLiteral("MajorTicksDirection"), (int)Axis::ticksOut);
	d->majorTicksType = (TicksType)group.readEntry(QStringLiteral("MajorTicksType"), static_cast<int>(TicksType::TotalNumber));
	d->majorTicksNumber = group.readEntry(QStringLiteral("MajorTicksNumber"), 11);
	d->majorTicksSpacing = group.readEntry(QStringLiteral("MajorTicksIncrement"),
										   0.0); // set to 0, so axisdock determines the value to not have to many labels the first time switched to Spacing
	d->majorTicksLength = group.readEntry(QStringLiteral("MajorTicksLength"), Worksheet::convertToSceneUnits(6.0, Worksheet::Unit::Point));

	d->minorTicksLine->init(group);
	d->minorTicksDirection = (TicksDirection)group.readEntry(QStringLiteral("MinorTicksDirection"), (int)Axis::ticksOut);
	d->minorTicksType = (TicksType)group.readEntry(QStringLiteral("MinorTicksType"), static_cast<int>(TicksType::TotalNumber));
	d->minorTicksNumber = group.readEntry(QStringLiteral("MinorTicksNumber"), 1);
	d->minorTicksIncrement = group.readEntry(QStringLiteral("MinorTicksIncrement"), 0.0); // see MajorTicksIncrement
	d->minorTicksLength = group.readEntry(QStringLiteral("MinorTicksLength"), Worksheet::convertToSceneUnits(3.0, Worksheet::Unit::Point));

	// Labels
	d->labelsFormat = (LabelsFormat)group.readEntry(QStringLiteral("LabelsFormat"), static_cast<int>(LabelsFormat::Decimal));
	d->labelsAutoPrecision = group.readEntry(QStringLiteral("LabelsAutoPrecision"), true);
	d->labelsPrecision = group.readEntry(QStringLiteral("LabelsPrecision"), 1);
	d->labelsDateTimeFormat = group.readEntry(QStringLiteral("LabelsDateTimeFormat"), QStringLiteral("yyyy-MM-dd hh:mm:ss"));
	d->labelsPosition = (LabelsPosition)group.readEntry(QStringLiteral("LabelsPosition"), (int)LabelsPosition::Out);
	d->labelsOffset = group.readEntry(QStringLiteral("LabelsOffset"), Worksheet::convertToSceneUnits(5.0, Worksheet::Unit::Point));
	d->labelsRotationAngle = group.readEntry(QStringLiteral("LabelsRotation"), 0);
	d->labelsTextType = (LabelsTextType)group.readEntry(QStringLiteral("LabelsTextType"), static_cast<int>(LabelsTextType::PositionValues));
	d->labelsFont = group.readEntry(QStringLiteral("LabelsFont"), QFont());
	d->labelsFont.setPointSizeF(Worksheet::convertToSceneUnits(10.0, Worksheet::Unit::Point));
	d->labelsColor = group.readEntry(QStringLiteral("LabelsFontColor"), QColor(Qt::black));
	d->labelsBackgroundType =
		(LabelsBackgroundType)group.readEntry(QStringLiteral("LabelsBackgroundType"), static_cast<int>(LabelsBackgroundType::Transparent));
	d->labelsBackgroundColor = group.readEntry(QStringLiteral("LabelsBackgroundColor"), QColor(Qt::white));
	d->labelsPrefix = group.readEntry(QStringLiteral("LabelsPrefix"), QStringLiteral(""));
	d->labelsSuffix = group.readEntry(QStringLiteral("LabelsSuffix"), QStringLiteral(""));
	d->labelsOpacity = group.readEntry(QStringLiteral("LabelsOpacity"), 1.0);

	// grid lines
	d->majorGridLine->init(group);
	d->minorGridLine->init(group);
}

/*!
 * For the most frequently edited properties, create Actions and ActionGroups for the context menu.
 * For some ActionGroups the actual actions are created in \c GuiTool,
 */
void Axis::initActions() {
	// Orientation
	orientationActionGroup = new QActionGroup(this);
	orientationActionGroup->setExclusive(true);
	connect(orientationActionGroup, &QActionGroup::triggered, this, &Axis::orientationChangedSlot);

	orientationHorizontalAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-axis-horizontal")), i18n("Horizontal"), orientationActionGroup);
	orientationHorizontalAction->setCheckable(true);

	orientationVerticalAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-axis-vertical")), i18n("Vertical"), orientationActionGroup);
	orientationVerticalAction->setCheckable(true);

	// Line
	lineStyleActionGroup = new QActionGroup(this);
	lineStyleActionGroup->setExclusive(true);
	connect(lineStyleActionGroup, &QActionGroup::triggered, this, &Axis::lineStyleChanged);

	lineColorActionGroup = new QActionGroup(this);
	lineColorActionGroup->setExclusive(true);
	connect(lineColorActionGroup, &QActionGroup::triggered, this, &Axis::lineColorChanged);

	// Ticks
	// TODO
}

void Axis::initMenus() {
	this->initActions();

	// Orientation
	orientationMenu = new QMenu(i18n("Orientation"));
	orientationMenu->setIcon(QIcon::fromTheme(QStringLiteral("labplot-axis-horizontal")));
	orientationMenu->addAction(orientationHorizontalAction);
	orientationMenu->addAction(orientationVerticalAction);

	// Line
	lineMenu = new QMenu(i18n("Line"));
	lineMenu->setIcon(QIcon::fromTheme(QStringLiteral("draw-line")));
	lineStyleMenu = new QMenu(i18n("Style"), lineMenu);
	lineStyleMenu->setIcon(QIcon::fromTheme(QStringLiteral("object-stroke-style")));
	lineMenu->setIcon(QIcon::fromTheme(QStringLiteral("draw-line")));
	lineMenu->addMenu(lineStyleMenu);

	lineColorMenu = new QMenu(i18n("Color"), lineMenu);
	lineColorMenu->setIcon(QIcon::fromTheme(QStringLiteral("fill-color")));
	GuiTools::fillColorMenu(lineColorMenu, lineColorActionGroup);
	lineMenu->addMenu(lineColorMenu);
}

QMenu* Axis::createContextMenu() {
	if (!orientationMenu)
		initMenus();

	Q_D(const Axis);
	QMenu* menu = WorksheetElement::createContextMenu();
	QAction* visibilityAction = this->visibilityAction();

	// Orientation
	if (d->orientation == Orientation::Horizontal)
		orientationHorizontalAction->setChecked(true);
	else
		orientationVerticalAction->setChecked(true);

	menu->insertMenu(visibilityAction, orientationMenu);

	// Line styles
	GuiTools::updatePenStyles(lineStyleMenu, lineStyleActionGroup, d->line->pen().color());
	GuiTools::selectPenStyleAction(lineStyleActionGroup, d->line->pen().style());

	GuiTools::selectColorAction(lineColorActionGroup, d->line->pen().color());

	menu->insertMenu(visibilityAction, lineMenu);
	menu->insertSeparator(visibilityAction);

	return menu;
}

/*!
	Returns an icon to be used in the project explorer.
*/
QIcon Axis::icon() const {
	Q_D(const Axis);
	QIcon icon;
	if (d->orientation == Orientation::Horizontal)
		icon = QIcon::fromTheme(QStringLiteral("labplot-axis-horizontal"));
	else
		icon = QIcon::fromTheme(QStringLiteral("labplot-axis-vertical"));

	return icon;
}

Axis::~Axis() {
	if (orientationMenu) {
		delete orientationMenu;
		delete lineMenu;
	}

	// no need to delete d->title, since it was added with addChild in init();

	// no need to delete the d-pointer here - it inherits from QGraphicsItem
	// and is deleted during the cleanup in QGraphicsScene
}

/*!
 * overrides the implementation in WorksheetElement and sets the z-value to the maximal possible,
 * axes are drawn on top of all other object in the plot.
 */
void Axis::setZValue(qreal) {
	Q_D(Axis);
	d->setZValue(std::numeric_limits<double>::max());
	d->gridItem->setParentItem(d->parentItem());
	d->gridItem->setZValue(0);
}

void Axis::retransform() {
	Q_D(Axis);
	d->retransform();
}

void Axis::retransformTickLabelStrings() {
	Q_D(Axis);
	d->retransformTickLabelStrings();
}

void Axis::setSuppressRetransform(bool value) {
	Q_D(Axis);
	d->suppressRetransform = value;
}

void Axis::handleResize(double horizontalRatio, double verticalRatio, bool pageResize) {
	Q_D(Axis);

	double ratio = 0;
	if (horizontalRatio > 1.0 || verticalRatio > 1.0)
		ratio = std::max(horizontalRatio, verticalRatio);
	else
		ratio = std::min(horizontalRatio, verticalRatio);

	double width = d->line->width() * ratio;
	d->line->setWidth(width);

	d->majorTicksLength *= ratio; // ticks are perpendicular to axis line -> verticalRatio relevant
	d->minorTicksLength *= ratio;
	d->labelsFont.setPointSizeF(d->labelsFont.pointSizeF() * ratio); // TODO: take into account rotated labels
	d->labelsOffset *= ratio;
	d->title->handleResize(horizontalRatio, verticalRatio, pageResize);
}

/* ============================ getter methods ================= */
BASIC_SHARED_D_READER_IMPL(Axis, Axis::RangeType, rangeType, rangeType)
BASIC_SHARED_D_READER_IMPL(Axis, Axis::Orientation, orientation, orientation)
BASIC_SHARED_D_READER_IMPL(Axis, Axis::Position, position, position)
BASIC_SHARED_D_READER_IMPL(Axis, double, offset, offset)
BASIC_SHARED_D_READER_IMPL(Axis, Range<double>, range, range)
BASIC_SHARED_D_READER_IMPL(Axis, bool, rangeScale, rangeScale)
RangeT::Scale Axis::scale() const {
	Q_D(const Axis);
	if (d->rangeScale)
		return d->range.scale();
	return d->scale;
}
BASIC_SHARED_D_READER_IMPL(Axis, Axis::TicksStartType, majorTicksStartType, majorTicksStartType)
BASIC_SHARED_D_READER_IMPL(Axis, qreal, majorTickStartOffset, majorTickStartOffset)
BASIC_SHARED_D_READER_IMPL(Axis, qreal, majorTickStartValue, majorTickStartValue)
BASIC_SHARED_D_READER_IMPL(Axis, qreal, scalingFactor, scalingFactor)
BASIC_SHARED_D_READER_IMPL(Axis, qreal, zeroOffset, zeroOffset)
BASIC_SHARED_D_READER_IMPL(Axis, bool, showScaleOffset, showScaleOffset)
BASIC_SHARED_D_READER_IMPL(Axis, double, logicalPosition, logicalPosition)

// title
BASIC_SHARED_D_READER_IMPL(Axis, TextLabel*, title, title)
BASIC_SHARED_D_READER_IMPL(Axis, qreal, titleOffsetX, titleOffsetX)
BASIC_SHARED_D_READER_IMPL(Axis, qreal, titleOffsetY, titleOffsetY)

// line
Line* Axis::line() const {
	Q_D(const Axis);
	return d->line;
}

BASIC_SHARED_D_READER_IMPL(Axis, Axis::ArrowType, arrowType, arrowType)
BASIC_SHARED_D_READER_IMPL(Axis, Axis::ArrowPosition, arrowPosition, arrowPosition)
BASIC_SHARED_D_READER_IMPL(Axis, qreal, arrowSize, arrowSize)

BASIC_SHARED_D_READER_IMPL(Axis, Axis::TicksDirection, majorTicksDirection, majorTicksDirection)
BASIC_SHARED_D_READER_IMPL(Axis, Axis::TicksType, majorTicksType, majorTicksType)
BASIC_SHARED_D_READER_IMPL(Axis, bool, majorTicksAutoNumber, majorTicksAutoNumber)
BASIC_SHARED_D_READER_IMPL(Axis, int, majorTicksNumber, majorTicksNumber)
BASIC_SHARED_D_READER_IMPL(Axis, qreal, majorTicksSpacing, majorTicksSpacing)
BASIC_SHARED_D_READER_IMPL(Axis, const AbstractColumn*, majorTicksColumn, majorTicksColumn)
QString& Axis::majorTicksColumnPath() const {
	D(Axis);
	return d->majorTicksColumnPath;
}
BASIC_SHARED_D_READER_IMPL(Axis, qreal, majorTicksLength, majorTicksLength)

Line* Axis::majorTicksLine() const {
	Q_D(const Axis);
	return d->majorTicksLine;
}

BASIC_SHARED_D_READER_IMPL(Axis, Axis::TicksDirection, minorTicksDirection, minorTicksDirection)
BASIC_SHARED_D_READER_IMPL(Axis, Axis::TicksType, minorTicksType, minorTicksType)
BASIC_SHARED_D_READER_IMPL(Axis, bool, minorTicksAutoNumber, minorTicksAutoNumber)
BASIC_SHARED_D_READER_IMPL(Axis, int, minorTicksNumber, minorTicksNumber)
BASIC_SHARED_D_READER_IMPL(Axis, qreal, minorTicksSpacing, minorTicksIncrement)
BASIC_SHARED_D_READER_IMPL(Axis, const AbstractColumn*, minorTicksColumn, minorTicksColumn)
QString& Axis::minorTicksColumnPath() const {
	D(Axis);
	return d->minorTicksColumnPath;
}
BASIC_SHARED_D_READER_IMPL(Axis, qreal, minorTicksLength, minorTicksLength)

Line* Axis::minorTicksLine() const {
	Q_D(const Axis);
	return d->minorTicksLine;
}

BASIC_SHARED_D_READER_IMPL(Axis, Axis::LabelsFormat, labelsFormat, labelsFormat)
BASIC_SHARED_D_READER_IMPL(Axis, bool, labelsFormatAuto, labelsFormatAuto)
BASIC_SHARED_D_READER_IMPL(Axis, bool, labelsAutoPrecision, labelsAutoPrecision)
BASIC_SHARED_D_READER_IMPL(Axis, int, labelsPrecision, labelsPrecision)
BASIC_SHARED_D_READER_IMPL(Axis, QString, labelsDateTimeFormat, labelsDateTimeFormat)
BASIC_SHARED_D_READER_IMPL(Axis, Axis::LabelsPosition, labelsPosition, labelsPosition)
BASIC_SHARED_D_READER_IMPL(Axis, qreal, labelsOffset, labelsOffset)
BASIC_SHARED_D_READER_IMPL(Axis, qreal, labelsRotationAngle, labelsRotationAngle)
BASIC_SHARED_D_READER_IMPL(Axis, Axis::LabelsTextType, labelsTextType, labelsTextType)
BASIC_SHARED_D_READER_IMPL(Axis, const AbstractColumn*, labelsTextColumn, labelsTextColumn)
QString& Axis::labelsTextColumnPath() const {
	D(Axis);
	return d->labelsTextColumnPath;
}
QVector<double> Axis::tickLabelValues() const {
	D(Axis);
	return d->tickLabelValues;
}
QVector<QString> Axis::tickLabelStrings() const {
	D(Axis);
	return d->tickLabelStrings;
}
BASIC_SHARED_D_READER_IMPL(Axis, QColor, labelsColor, labelsColor)
BASIC_SHARED_D_READER_IMPL(Axis, QFont, labelsFont, labelsFont)
BASIC_SHARED_D_READER_IMPL(Axis, Axis::LabelsBackgroundType, labelsBackgroundType, labelsBackgroundType)
BASIC_SHARED_D_READER_IMPL(Axis, QColor, labelsBackgroundColor, labelsBackgroundColor)
BASIC_SHARED_D_READER_IMPL(Axis, QString, labelsPrefix, labelsPrefix)
BASIC_SHARED_D_READER_IMPL(Axis, QString, labelsSuffix, labelsSuffix)
BASIC_SHARED_D_READER_IMPL(Axis, qreal, labelsOpacity, labelsOpacity)

int Axis::maxNumberMajorTicksCustomColumn() {
	return _maxNumberMajorTicksCustomColumn;
}

// grid
Line* Axis::majorGridLine() const {
	Q_D(const Axis);
	return d->majorGridLine;
}

Line* Axis::minorGridLine() const {
	Q_D(const Axis);
	return d->minorGridLine;
}

/* ============================ setter methods and undo commands ================= */
STD_SETTER_CMD_IMPL_F(Axis, SetRangeType, Axis::RangeType, rangeType, retransformRange)
void Axis::setRangeType(const Axis::RangeType rangeType) {
	Q_D(Axis);
	if (rangeType != d->rangeType)
		exec(new AxisSetRangeTypeCmd(d, rangeType, ki18n("%1: set axis range type")));
}

void Axis::setDefault(bool value) {
	Q_D(Axis);
	d->isDefault = value;
}

bool Axis::isDefault() const {
	Q_D(const Axis);
	return d->isDefault;
}

bool Axis::isNumeric() const {
	Q_D(const Axis);
	const int xIndex{cSystem->index(Dimension::X)}, yIndex{cSystem->index(Dimension::Y)};
	bool numeric = ((d->orientation == Axis::Orientation::Horizontal && d->m_plot->xRangeFormat(xIndex) == RangeT::Format::Numeric)
					|| (d->orientation == Axis::Orientation::Vertical && d->m_plot->yRangeFormat(yIndex) == RangeT::Format::Numeric));
	return numeric;
}

STD_SETTER_CMD_IMPL_F_S(Axis, SetOrientation, Axis::Orientation, orientation, retransform)
void Axis::setOrientation(Orientation orientation) {
	Q_D(Axis);
	if (orientation != d->orientation)
		exec(new AxisSetOrientationCmd(d, orientation, ki18n("%1: set axis orientation")));
}

STD_SETTER_CMD_IMPL_F_S(Axis, SetPosition, Axis::Position, position, retransform)
void Axis::setPosition(Position position) {
	Q_D(Axis);
	if (position != d->position)
		exec(new AxisSetPositionCmd(d, position, ki18n("%1: set axis position")));
}

STD_SETTER_CMD_IMPL_F(Axis, SetOffset, double, offset, retransform)
void Axis::setOffset(double offset, bool undo) {
	Q_D(Axis);
	if (offset != d->offset) {
		if (undo) {
			exec(new AxisSetOffsetCmd(d, offset, ki18n("%1: set axis offset")));
		} else {
			d->offset = offset;
			// don't need to call retransform() afterward
			// since the only usage of this call is in CartesianPlot, where retransform is called for all children anyway.
		}
		Q_EMIT positionChanged(offset);
	}
}

STD_SETTER_CMD_IMPL_F_S(Axis, SetRange, Range<double>, range, retransform)
void Axis::setRange(Range<double> range) {
	DEBUG(Q_FUNC_INFO << ", range = " << range.toStdString())
	Q_D(Axis);
	if (range != d->range) {
		exec(new AxisSetRangeCmd(d, range, ki18n("%1: set axis range")));
		// auto set tick count when changing range (only changed here)
		if (d->majorTicksAutoNumber)
			setMajorTicksNumber(d->range.autoTickCount(), true);
	}
}
void Axis::setStart(double min) {
	Q_D(Axis);
	Range<double> range = d->range;
	const auto scale = range.scale();
	if (!((RangeT::isLogScale(scale) && min <= 0) || (scale == RangeT::Scale::Sqrt && min < 0))) {
		range.setStart(min);
		setRange(range);
	}
	Q_EMIT startChanged(range.start()); // Feedback
}
void Axis::setEnd(double max) {
	Q_D(Axis);
	Range<double> range = d->range;
	const auto scale = range.scale();
	if (!((RangeT::isLogScale(scale) && max <= 0) || (scale == RangeT::Scale::Sqrt && max < 0))) {
		range.setEnd(max);
		setRange(range);
	}
	Q_EMIT endChanged(range.end()); // Feedback
}
void Axis::setRange(double min, double max) {
	Q_D(Axis);
	Range<double> range = d->range;
	range.setStart(min);
	QDEBUG("scale = " << range.scale())
	range.setEnd(max);
	setRange(range);
}

STD_SETTER_CMD_IMPL_F_S(Axis, SetRangeScale, bool, rangeScale, retransformTicks)
void Axis::setRangeScale(bool rangeScale) {
	Q_D(Axis);
	if (rangeScale != d->rangeScale)
		exec(new AxisSetRangeScaleCmd(d, rangeScale, ki18n("%1: set range scale")));
}

STD_SETTER_CMD_IMPL_F_S(Axis, SetScale, RangeT::Scale, scale, retransformTicks)
void Axis::setScale(RangeT::Scale scale) {
	Q_D(Axis);
	if (scale != d->scale)
		exec(new AxisSetScaleCmd(d, scale, ki18n("%1: set scale")));
}

STD_SETTER_CMD_IMPL_F_S(Axis, SetMajorTicksStartType, Axis::TicksStartType, majorTicksStartType, retransform)
void Axis::setMajorTicksStartType(Axis::TicksStartType type) {
	Q_D(Axis);
	if (type != d->majorTicksStartType)
		exec(new AxisSetMajorTicksStartTypeCmd(d, type, ki18n("%1: set major tick start type")));
}

STD_SETTER_CMD_IMPL_F_S(Axis, SetMajorTickStartOffset, qreal, majorTickStartOffset, retransform)
void Axis::setMajorTickStartOffset(qreal offset) {
	Q_D(Axis);
	if (offset != d->majorTickStartOffset)
		exec(new AxisSetMajorTickStartOffsetCmd(d, offset, ki18n("%1: set major tick start offset")));
}

STD_SETTER_CMD_IMPL_F_S(Axis, SetMajorTickStartValue, qreal, majorTickStartValue, retransform)
void Axis::setMajorTickStartValue(qreal value) {
	Q_D(Axis);
	// TODO: check if value is invalid
	if (value != d->majorTickStartValue)
		exec(new AxisSetMajorTickStartValueCmd(d, value, ki18n("%1: set major tick start value")));
}

STD_SETTER_CMD_IMPL_F_S(Axis, SetScalingFactor, qreal, scalingFactor, retransform)
void Axis::setScalingFactor(qreal scalingFactor) {
	Q_D(Axis);
	// TODO: check negative values and log-scales?
	if (scalingFactor == 0) {
		Q_EMIT scalingFactorChanged(d->scalingFactor); // return current scalingfactor as feedback for the spinbox
		return;
	}
	if (scalingFactor != d->scalingFactor)
		exec(new AxisSetScalingFactorCmd(d, scalingFactor, ki18n("%1: set axis scaling factor")));
}

STD_SETTER_CMD_IMPL_F_S(Axis, SetZeroOffset, qreal, zeroOffset, retransform)
void Axis::setZeroOffset(qreal zeroOffset) {
	Q_D(Axis);

	// TODO: check for negative values and log scales?
	if (zeroOffset != d->zeroOffset)
		exec(new AxisSetZeroOffsetCmd(d, zeroOffset, ki18n("%1: set axis zero offset")));
}
STD_SETTER_CMD_IMPL_F_S(Axis, ShowScaleOffset, bool, showScaleOffset, retransform)
void Axis::setShowScaleOffset(bool b) {
	Q_D(Axis);
	if (b != d->showScaleOffset)
		exec(new AxisShowScaleOffsetCmd(d, b, ki18n("%1: show scale and offset")));
}

STD_SETTER_CMD_IMPL_F_S(Axis, SetLogicalPosition, double, logicalPosition, retransform)
void Axis::setLogicalPosition(double pos) {
	Q_D(Axis);
	if (pos != d->logicalPosition)
		exec(new AxisSetLogicalPositionCmd(d, pos, ki18n("%1: set axis logical position")));
}

// Title
STD_SETTER_CMD_IMPL_F_S(Axis, SetTitleOffsetX, qreal, titleOffsetX, retransform)
void Axis::setTitleOffsetX(qreal offset) {
	Q_D(Axis);
	if (offset != d->titleOffsetX)
		exec(new AxisSetTitleOffsetXCmd(d, offset, ki18n("%1: set title offset")));
}

STD_SETTER_CMD_IMPL_F_S(Axis, SetTitleOffsetY, qreal, titleOffsetY, retransform)
void Axis::setTitleOffsetY(qreal offset) {
	Q_D(Axis);
	if (offset != d->titleOffsetY)
		exec(new AxisSetTitleOffsetYCmd(d, offset, ki18n("%1: set title offset")));
}

// Line
STD_SETTER_CMD_IMPL_F_S(Axis, SetArrowType, Axis::ArrowType, arrowType, retransformArrow)
void Axis::setArrowType(ArrowType type) {
	Q_D(Axis);
	if (type != d->arrowType)
		exec(new AxisSetArrowTypeCmd(d, type, ki18n("%1: set arrow type")));
}

STD_SETTER_CMD_IMPL_F_S(Axis, SetArrowPosition, Axis::ArrowPosition, arrowPosition, retransformArrow)
void Axis::setArrowPosition(ArrowPosition position) {
	Q_D(Axis);
	if (position != d->arrowPosition)
		exec(new AxisSetArrowPositionCmd(d, position, ki18n("%1: set arrow position")));
}

STD_SETTER_CMD_IMPL_F_S(Axis, SetArrowSize, qreal, arrowSize, retransformArrow)
void Axis::setArrowSize(qreal arrowSize) {
	Q_D(Axis);
	if (arrowSize != d->arrowSize)
		exec(new AxisSetArrowSizeCmd(d, arrowSize, ki18n("%1: set arrow size")));
}

// Major ticks
STD_SETTER_CMD_IMPL_F_S(Axis, SetMajorTicksDirection, Axis::TicksDirection, majorTicksDirection, retransformTicks)
void Axis::setMajorTicksDirection(TicksDirection majorTicksDirection) {
	Q_D(Axis);
	if (majorTicksDirection != d->majorTicksDirection)
		exec(new AxisSetMajorTicksDirectionCmd(d, majorTicksDirection, ki18n("%1: set major ticks direction")));
}

STD_SETTER_CMD_IMPL_F_S(Axis, SetMajorTicksType, Axis::TicksType, majorTicksType, retransformTicks)
void Axis::setMajorTicksType(TicksType majorTicksType) {
	Q_D(Axis);
	if (majorTicksType != d->majorTicksType)
		exec(new AxisSetMajorTicksTypeCmd(d, majorTicksType, ki18n("%1: set major ticks type")));
}

STD_SETTER_CMD_IMPL_S(Axis, SetMajorTicksNumberNoFinalize, int, majorTicksNumber) // no retransformTicks called
STD_SETTER_CMD_IMPL_F_S(Axis, SetMajorTicksAutoNumber, bool, majorTicksAutoNumber, retransformTicks)
void Axis::setMajorTicksAutoNumber(bool automatic) {
	Q_D(Axis);
	if (automatic != d->majorTicksAutoNumber) {
		auto* parent = new AxisSetMajorTicksAutoNumberCmd(d, automatic, ki18n("%1: enable/disable major automatic tick numbers"));
		if (automatic && d->range.autoTickCount() != d->majorTicksNumber)
			new AxisSetMajorTicksNumberNoFinalizeCmd(d, d->range.autoTickCount(), ki18n("%1: set the total number of the major ticks"), parent);
		exec(parent);
	}
}

STD_SETTER_CMD_IMPL_S(Axis, SetMajorTicksAutoNumberNoFinalize, bool, majorTicksAutoNumber) // no retransformTicks called
STD_SETTER_CMD_IMPL_F_S(Axis, SetMajorTicksNumber, int, majorTicksNumber, retransformTicks)
void Axis::setMajorTicksNumber(int number, bool automatic) {
	DEBUG(Q_FUNC_INFO << ", number = " << number)
	Q_D(Axis);
	if (number > maxNumberMajorTicks) {
		// Notifiy the user that the number was invalid
		Q_EMIT majorTicksNumberChanged(maxNumberMajorTicks);
		return;
	} else if (number != d->majorTicksNumber) {
		auto* parent = new AxisSetMajorTicksNumberCmd(d, number, ki18n("%1: set the total number of the major ticks"));
		if (!automatic)
			new AxisSetMajorTicksAutoNumberNoFinalizeCmd(d, false, ki18n("%1: disable major automatic tick numbers"), parent);
		exec(parent);
	}
}

STD_SETTER_CMD_IMPL_F_S(Axis, SetMajorTicksSpacing, qreal, majorTicksSpacing, retransformTicks)
void Axis::setMajorTicksSpacing(qreal majorTicksSpacing) {
	double range = this->range().length();
	DEBUG(Q_FUNC_INFO << ", major spacing = " << majorTicksSpacing << ", range = " << range)
	// fix spacing if incorrect (not set or > 100 ticks)
	if (majorTicksSpacing == 0. || range / majorTicksSpacing > maxNumberMajorTicks) {
		if (majorTicksSpacing == 0.)
			majorTicksSpacing = range / (majorTicksNumber() - 1);

		if (range / majorTicksSpacing > maxNumberMajorTicks)
			majorTicksSpacing = range / maxNumberMajorTicks;

		Q_EMIT majorTicksSpacingChanged(majorTicksSpacing);
		return;
	}

	Q_D(Axis);
	if (majorTicksSpacing != d->majorTicksSpacing)
		exec(new AxisSetMajorTicksSpacingCmd(d, majorTicksSpacing, ki18n("%1: set the spacing of the major ticks")));
}

STD_SETTER_CMD_IMPL_F_S(Axis, SetMajorTicksColumn, const AbstractColumn*, majorTicksColumn, retransformTicks)
void Axis::setMajorTicksColumn(const AbstractColumn* column) {
	Q_D(Axis);
	if (column != d->majorTicksColumn) {
		exec(new AxisSetMajorTicksColumnCmd(d, column, ki18n("%1: assign major ticks' values")));

		if (column) {
			connect(column, &AbstractColumn::dataChanged, this, &Axis::retransformTicks);
			connect(column->parentAspect(), &AbstractAspect::childAspectAboutToBeRemoved, this, &Axis::majorTicksColumnAboutToBeRemoved);
			// TODO: add disconnect in the undo-function
		}
	}
}

STD_SETTER_CMD_IMPL_F_S(Axis, SetMajorTicksLength, qreal, majorTicksLength, retransformTicks)
void Axis::setMajorTicksLength(qreal majorTicksLength) {
	Q_D(Axis);
	if (majorTicksLength != d->majorTicksLength)
		exec(new AxisSetMajorTicksLengthCmd(d, majorTicksLength, ki18n("%1: set major ticks length")));
}

// Minor ticks
STD_SETTER_CMD_IMPL_F_S(Axis, SetMinorTicksDirection, Axis::TicksDirection, minorTicksDirection, retransformTicks)
void Axis::setMinorTicksDirection(TicksDirection minorTicksDirection) {
	Q_D(Axis);
	if (minorTicksDirection != d->minorTicksDirection)
		exec(new AxisSetMinorTicksDirectionCmd(d, minorTicksDirection, ki18n("%1: set minor ticks direction")));
}

STD_SETTER_CMD_IMPL_F_S(Axis, SetMinorTicksType, Axis::TicksType, minorTicksType, retransformTicks)
void Axis::setMinorTicksType(TicksType minorTicksType) {
	Q_D(Axis);
	if (minorTicksType != d->minorTicksType)
		exec(new AxisSetMinorTicksTypeCmd(d, minorTicksType, ki18n("%1: set minor ticks type")));
}

STD_SETTER_CMD_IMPL_S(Axis, SetMinorTicksNumberNoFinalize, int, minorTicksNumber)
STD_SETTER_CMD_IMPL_F_S(Axis, SetMinorTicksAutoNumber, bool, minorTicksAutoNumber, retransformTicks)
void Axis::setMinorTicksAutoNumber(bool automatic) {
	Q_D(Axis);
	if (automatic != d->minorTicksAutoNumber) {
		auto* parent = new AxisSetMinorTicksAutoNumberCmd(d, automatic, ki18n("%1: enable/disable minor automatic tick numbers"));
		// TODO: for automatic it is always 1. Is that ok?
		if (automatic && 1 != d->minorTicksNumber)
			new AxisSetMinorTicksNumberNoFinalizeCmd(d, 1, ki18n("%1: set the total number of the minor ticks"), parent);
		exec(parent);
	}
}

STD_SETTER_CMD_IMPL_S(Axis, SetMinorTicksAutoNumberNoFinalize, bool, minorTicksAutoNumber) // no retransformTicks called
STD_SETTER_CMD_IMPL_F_S(Axis, SetMinorTicksNumber, int, minorTicksNumber, retransformTicks)
void Axis::setMinorTicksNumber(int minorTicksNumber) {
	DEBUG(Q_FUNC_INFO << ", number = " << minorTicksNumber)
	Q_D(Axis);
	if (minorTicksNumber != d->minorTicksNumber) {
		auto* parent = new AxisSetMinorTicksNumberCmd(d, minorTicksNumber, ki18n("%1: set the total number of the minor ticks"));
		new AxisSetMinorTicksAutoNumberNoFinalizeCmd(d, false, ki18n("%1: disable major automatic tick numbers"), parent);
		exec(parent);
	}
}

STD_SETTER_CMD_IMPL_F_S(Axis, SetMinorTicksSpacing, qreal, minorTicksIncrement, retransformTicks)
void Axis::setMinorTicksSpacing(qreal minorTicksSpacing) {
	Q_D(Axis);
	double range = this->range().length();
	int numberTicks = 0;

	int majorTicks = majorTicksNumber();
	if (minorTicksSpacing > 0.)
		numberTicks = range / (majorTicks - 1) / minorTicksSpacing - 1; // recalc

	// set if unset or > 100.
	if (minorTicksSpacing == 0. || numberTicks > 100) {
		if (minorTicksSpacing == 0.)
			minorTicksSpacing = range / (majorTicks - 1) / (minorTicksNumber() + 1);

		numberTicks = range / (majorTicks - 1) / minorTicksSpacing - 1; // recalculate number of ticks

		if (numberTicks > 100) // maximum 100 minor ticks
			minorTicksSpacing = range / (majorTicks - 1) / (100 + 1);

		Q_EMIT minorTicksIncrementChanged(minorTicksSpacing);
		return;
	}

	if (minorTicksSpacing != d->minorTicksIncrement)
		exec(new AxisSetMinorTicksSpacingCmd(d, minorTicksSpacing, ki18n("%1: set the spacing of the minor ticks")));
}

STD_SETTER_CMD_IMPL_F_S(Axis, SetMinorTicksColumn, const AbstractColumn*, minorTicksColumn, retransformTicks)
void Axis::setMinorTicksColumn(const AbstractColumn* column) {
	Q_D(Axis);
	if (column != d->minorTicksColumn) {
		exec(new AxisSetMinorTicksColumnCmd(d, column, ki18n("%1: assign minor ticks' values")));

		if (column) {
			connect(column, &AbstractColumn::dataChanged, this, &Axis::retransformTicks);
			connect(column->parentAspect(), &AbstractAspect::childAspectAboutToBeRemoved, this, &Axis::minorTicksColumnAboutToBeRemoved);
			// TODO: add disconnect in the undo-function
		}
	}
}

STD_SETTER_CMD_IMPL_F_S(Axis, SetMinorTicksLength, qreal, minorTicksLength, retransformTicks)
void Axis::setMinorTicksLength(qreal minorTicksLength) {
	Q_D(Axis);
	if (minorTicksLength != d->minorTicksLength)
		exec(new AxisSetMinorTicksLengthCmd(d, minorTicksLength, ki18n("%1: set minor ticks length")));
}

// Labels
STD_SETTER_CMD_IMPL_F_S(Axis, SetLabelsFormat, Axis::LabelsFormat, labelsFormat, retransformTicks)
void Axis::setLabelsFormat(LabelsFormat labelsFormat) {
	DEBUG(Q_FUNC_INFO << ", format = " << ENUM_TO_STRING(Axis, LabelsFormat, labelsFormat))
	Q_D(Axis);
	if (labelsFormat != d->labelsFormat)
		exec(new AxisSetLabelsFormatCmd(d, labelsFormat, ki18n("%1: set labels format")));
}

STD_SETTER_CMD_IMPL_F_S(Axis, SetLabelsFormatAuto, bool, labelsFormatAuto, retransformTicks)
void Axis::setLabelsFormatAuto(bool automatic) {
	Q_D(Axis);
	if (automatic != d->labelsFormatAuto)
		exec(new AxisSetLabelsFormatAutoCmd(d, automatic, ki18n("%1: set labels format automatic")));
}

STD_SETTER_CMD_IMPL_F_S(Axis, SetLabelsAutoPrecision, bool, labelsAutoPrecision, retransformTickLabelStrings)
void Axis::setLabelsAutoPrecision(bool labelsAutoPrecision) {
	Q_D(Axis);
	if (labelsAutoPrecision != d->labelsAutoPrecision)
		exec(new AxisSetLabelsAutoPrecisionCmd(d, labelsAutoPrecision, ki18n("%1: set labels precision")));
}

STD_SETTER_CMD_IMPL_F_S(Axis, SetLabelsPrecision, int, labelsPrecision, retransformTickLabelStrings)
void Axis::setLabelsPrecision(int labelsPrecision) {
	Q_D(Axis);
	if (labelsPrecision != d->labelsPrecision)
		exec(new AxisSetLabelsPrecisionCmd(d, labelsPrecision, ki18n("%1: set labels precision")));
}

STD_SETTER_CMD_IMPL_F_S(Axis, SetLabelsDateTimeFormat, QString, labelsDateTimeFormat, retransformTickLabelStrings)
void Axis::setLabelsDateTimeFormat(const QString& format) {
	Q_D(Axis);
	if (format != d->labelsDateTimeFormat)
		exec(new AxisSetLabelsDateTimeFormatCmd(d, format, ki18n("%1: set labels datetime format")));
}

STD_SETTER_CMD_IMPL_F_S(Axis, SetLabelsPosition, Axis::LabelsPosition, labelsPosition, retransformTickLabelPositions)
void Axis::setLabelsPosition(LabelsPosition labelsPosition) {
	Q_D(Axis);
	if (labelsPosition != d->labelsPosition)
		exec(new AxisSetLabelsPositionCmd(d, labelsPosition, ki18n("%1: set labels position")));
}

STD_SETTER_CMD_IMPL_F_S(Axis, SetLabelsOffset, double, labelsOffset, retransformTickLabelPositions)
void Axis::setLabelsOffset(double offset) {
	Q_D(Axis);
	if (offset != d->labelsOffset)
		exec(new AxisSetLabelsOffsetCmd(d, offset, ki18n("%1: set label offset")));
}

STD_SETTER_CMD_IMPL_F_S(Axis, SetLabelsRotationAngle, qreal, labelsRotationAngle, retransformTickLabelPositions)
void Axis::setLabelsRotationAngle(qreal angle) {
	Q_D(Axis);
	if (angle != d->labelsRotationAngle)
		exec(new AxisSetLabelsRotationAngleCmd(d, angle, ki18n("%1: set label rotation angle")));
}

STD_SETTER_CMD_IMPL_F_S(Axis, SetLabelsTextType, Axis::LabelsTextType, labelsTextType, retransformTicks)
void Axis::setLabelsTextType(LabelsTextType type) {
	Q_D(Axis);
	if (type != d->labelsTextType)
		exec(new AxisSetLabelsTextTypeCmd(d, type, ki18n("%1: set labels text type")));
}

STD_SETTER_CMD_IMPL_F_S(Axis, SetLabelsTextColumn, const AbstractColumn*, labelsTextColumn, retransformTicks)
void Axis::setLabelsTextColumn(const AbstractColumn* column) {
	Q_D(Axis);
	if (column != d->labelsTextColumn) {
		exec(new AxisSetLabelsTextColumnCmd(d, column, ki18n("%1: set labels text column")));

		if (column) {
			connect(column, &AbstractColumn::dataChanged, this, &Axis::retransformTicks);
			connect(column->parentAspect(), &AbstractAspect::childAspectAboutToBeRemoved, this, &Axis::retransformTicks);
			// TODO: add disconnect in the undo-function
		}
	}
}

STD_SETTER_CMD_IMPL_F_S(Axis, SetLabelsColor, QColor, labelsColor, update)
void Axis::setLabelsColor(const QColor& color) {
	Q_D(Axis);
	if (color != d->labelsColor)
		exec(new AxisSetLabelsColorCmd(d, color, ki18n("%1: set label color")));
}

STD_SETTER_CMD_IMPL_F_S(Axis, SetLabelsFont, QFont, labelsFont, retransformTickLabelStrings)
void Axis::setLabelsFont(const QFont& font) {
	Q_D(Axis);
	if (font != d->labelsFont)
		exec(new AxisSetLabelsFontCmd(d, font, ki18n("%1: set label font")));
}

STD_SETTER_CMD_IMPL_F_S(Axis, SetLabelsBackgroundType, Axis::LabelsBackgroundType, labelsBackgroundType, update)
void Axis::setLabelsBackgroundType(LabelsBackgroundType type) {
	Q_D(Axis);
	if (type != d->labelsBackgroundType)
		exec(new AxisSetLabelsBackgroundTypeCmd(d, type, ki18n("%1: set labels background type")));
}

STD_SETTER_CMD_IMPL_F_S(Axis, SetLabelsBackgroundColor, QColor, labelsBackgroundColor, update)
void Axis::setLabelsBackgroundColor(const QColor& color) {
	Q_D(Axis);
	if (color != d->labelsBackgroundColor)
		exec(new AxisSetLabelsBackgroundColorCmd(d, color, ki18n("%1: set label background color")));
}

STD_SETTER_CMD_IMPL_F_S(Axis, SetLabelsPrefix, QString, labelsPrefix, retransformTickLabelStrings)
void Axis::setLabelsPrefix(const QString& prefix) {
	Q_D(Axis);
	if (prefix != d->labelsPrefix)
		exec(new AxisSetLabelsPrefixCmd(d, prefix, ki18n("%1: set label prefix")));
}

STD_SETTER_CMD_IMPL_F_S(Axis, SetLabelsSuffix, QString, labelsSuffix, retransformTickLabelStrings)
void Axis::setLabelsSuffix(const QString& suffix) {
	Q_D(Axis);
	if (suffix != d->labelsSuffix)
		exec(new AxisSetLabelsSuffixCmd(d, suffix, ki18n("%1: set label suffix")));
}

STD_SETTER_CMD_IMPL_F_S(Axis, SetLabelsOpacity, qreal, labelsOpacity, update)
void Axis::setLabelsOpacity(qreal opacity) {
	Q_D(Axis);
	if (opacity != d->labelsOpacity)
		exec(new AxisSetLabelsOpacityCmd(d, opacity, ki18n("%1: set labels opacity")));
}

// ##############################################################################
// ####################################  SLOTs   ################################
// ##############################################################################
void Axis::labelChanged() {
	Q_D(Axis);
	d->recalcShapeAndBoundingRect();
}

void Axis::retransformTicks() {
	Q_D(Axis);
	d->retransformTicks();
}

void Axis::majorTicksColumnAboutToBeRemoved(const AbstractAspect* aspect) {
	Q_D(Axis);
	if (aspect == d->majorTicksColumn) {
		d->majorTicksColumn = nullptr;
		d->retransformTicks();
	}
}

void Axis::minorTicksColumnAboutToBeRemoved(const AbstractAspect* aspect) {
	Q_D(Axis);
	if (aspect == d->minorTicksColumn) {
		d->minorTicksColumn = nullptr;
		d->retransformTicks();
	}
}

// ##############################################################################
// ######  SLOTs for changes triggered via QActions in the context menu  ########
// ##############################################################################
void Axis::orientationChangedSlot(QAction* action) {
	if (action == orientationHorizontalAction)
		this->setOrientation(Axis::Orientation::Horizontal);
	else
		this->setOrientation(Axis::Orientation::Vertical);
}

void Axis::lineStyleChanged(QAction* action) {
	Q_D(const Axis);
	d->line->setStyle(GuiTools::penStyleFromAction(lineStyleActionGroup, action));
}

void Axis::lineColorChanged(QAction* action) {
	Q_D(const Axis);
	d->line->setColor(GuiTools::colorFromAction(lineColorActionGroup, action));
}

// #####################################################################
// ################### Private implementation ##########################
// #####################################################################
AxisPrivate::AxisPrivate(Axis* owner)
	: WorksheetElementPrivate(owner)
	, gridItem(new AxisGrid(this))
	, q(owner) {
	setFlag(QGraphicsItem::ItemIsSelectable, true);
	setFlag(QGraphicsItem::ItemIsFocusable, true);
	setFlag(QGraphicsItem::ItemIsMovable, false);
	setAcceptHoverEvents(true);
}

bool AxisPrivate::swapVisible(bool on) {
	bool oldValue = isVisible();

	// When making a graphics item invisible, it gets deselected in the scene.
	// In this case we don't want to deselect the item in the project explorer.
	// We need to supress the deselection in the view.
	auto* worksheet = static_cast<Worksheet*>(q->parent(AspectType::Worksheet));
	if (worksheet) {
		worksheet->suppressSelectionChangedEvent(true);
		setVisible(on);
		gridItem->setVisible(on);
		worksheet->suppressSelectionChangedEvent(false);
	} else
		setVisible(on);

	Q_EMIT q->changed();
	Q_EMIT q->visibleChanged(on);
	return oldValue;
}

/*!
	recalculates the position of the axis on the worksheet
 */
void AxisPrivate::retransform() {
	DEBUG(Q_FUNC_INFO)
	const bool suppress = suppressRetransform || !plot() || q->isLoading();
	trackRetransformCalled(suppress);
	if (suppress)
		return;

	// 	PERFTRACE(name().toLatin1() + ", AxisPrivate::retransform()");
	suppressRecalc = true;
	retransformLine();
	suppressRecalc = false;
	recalcShapeAndBoundingRect();
}

void AxisPrivate::retransformRange() {
	switch (rangeType) { // also if not changing (like on plot range changes)
	case Axis::RangeType::Auto: {
		if (orientation == Axis::Orientation::Horizontal)
			range = m_plot->range(Dimension::X, q->cSystem->index(Dimension::X));
		else
			range = m_plot->range(Dimension::Y, q->cSystem->index(Dimension::Y));

		DEBUG(Q_FUNC_INFO << ", new auto range = " << range.toStdString())
		break;
	}
	case Axis::RangeType::AutoData:
		if (orientation == Axis::Orientation::Horizontal)
			range = m_plot->dataRange(Dimension::X, q->cSystem->index(Dimension::X));
		else
			range = m_plot->dataRange(Dimension::Y, q->cSystem->index(Dimension::Y));

		DEBUG(Q_FUNC_INFO << ", new auto data range = " << range.toStdString())
		break;
	case Axis::RangeType::Custom:
		return;
	}

	retransform();
	Q_EMIT q->rangeChanged(range);
}

void AxisPrivate::retransformLine() {
	DEBUG(Q_FUNC_INFO << ", \"" << STDSTRING(title->name()) << "\", coordinate system " << q->m_cSystemIndex + 1)
	DEBUG(Q_FUNC_INFO << ", x range is x range " << q->cSystem->index(Dimension::X) + 1)
	DEBUG(Q_FUNC_INFO << ", y range is y range " << q->cSystem->index(Dimension::Y) + 1)
	//	DEBUG(Q_FUNC_INFO << ", x range index check = " << dynamic_cast<const
	// CartesianCoordinateSystem*>(plot()->coordinateSystem(q->m_cSystemIndex))->index(Dimension::X)
	//)
	DEBUG(Q_FUNC_INFO << ", axis range = " << range.toStdString() << " scale = " << ENUM_TO_STRING(RangeT, Scale, q->scale()))

	if (suppressRetransform)
		return;

	linePath = QPainterPath();
	lines.clear();

	QPointF startPoint, endPoint;
	if (orientation == Axis::Orientation::Horizontal) {
		if (position == Axis::Position::Logical) {
			startPoint = QPointF(range.start(), logicalPosition);
			endPoint = QPointF(range.end(), logicalPosition);
			lines.append(QLineF(startPoint, endPoint));
			// QDEBUG(Q_FUNC_INFO << ", Logical LINE = " << lines)
			lines = q->cSystem->mapLogicalToScene(lines, AbstractCoordinateSystem::MappingFlag::MarkGaps);
		} else {
			WorksheetElement::PositionWrapper wrapper;
			if (position == Axis::Position::Top)
				wrapper.verticalPosition = WorksheetElement::VerticalPosition::Top;
			else if (position == Axis::Position::Centered)
				wrapper.verticalPosition = WorksheetElement::VerticalPosition::Center;
			else // (position == Axis::Position::Bottom) // default
				wrapper.verticalPosition = WorksheetElement::VerticalPosition::Bottom;

			wrapper.point = QPointF(offset, offset);
			const auto pos = q->relativePosToParentPos(wrapper);

			Lines ranges{QLineF(QPointF(range.start(), 1.), QPointF(range.end(), 1.))};
			// y=1 may be outside clip range: suppress clipping. value must be > 0 for log scales
			const auto sceneRange = q->cSystem->mapLogicalToScene(ranges, CartesianCoordinateSystem::MappingFlag::SuppressPageClipping);
			// QDEBUG(Q_FUNC_INFO << ", scene range = " << sceneRange)

			if (sceneRange.size() > 0) {
				// std::max/std::min: stay inside rect()
				QRectF rect = m_plot->dataRect();
				startPoint = QPointF(std::max(sceneRange.at(0).x1(), rect.x()), pos.y());
				endPoint = QPointF(std::min(sceneRange.at(0).x2(), rect.x() + rect.width()), pos.y());

				lines.append(QLineF(startPoint, endPoint));
				// QDEBUG(Q_FUNC_INFO << ", Non Logical LINE =" << lines)
			}
		}
	} else { // vertical
		if (position == Axis::Position::Logical) {
			startPoint = QPointF(logicalPosition, range.start());
			endPoint = QPointF(logicalPosition, range.end());
			lines.append(QLineF(startPoint, endPoint));
			// QDEBUG(Q_FUNC_INFO << ", LOGICAL LINES = " << lines)
			lines = q->cSystem->mapLogicalToScene(lines, AbstractCoordinateSystem::MappingFlag::MarkGaps);
		} else {
			WorksheetElement::PositionWrapper wrapper;
			if (position == Axis::Position::Left)
				wrapper.horizontalPosition = WorksheetElement::HorizontalPosition::Left;
			else if (position == Axis::Position::Centered)
				wrapper.horizontalPosition = WorksheetElement::HorizontalPosition::Center;
			else // (position == Axis::Position::Right) // default
				wrapper.horizontalPosition = WorksheetElement::HorizontalPosition::Right;

			wrapper.point = QPointF(offset, offset);
			const auto pos = q->relativePosToParentPos(wrapper);

			Lines ranges{QLineF(QPointF(1., range.start()), QPointF(1., range.end()))};
			// x=1 may be outside clip range: suppress clipping. value must be > 0 for log scales
			const auto sceneRange = q->cSystem->mapLogicalToScene(ranges, CartesianCoordinateSystem::MappingFlag::SuppressPageClipping);
			// QDEBUG(Q_FUNC_INFO << ", scene range = " << sceneRange)
			if (sceneRange.size() > 0) {
				// std::max/std::min: stay inside rect()
				QRectF rect = m_plot->dataRect();
				startPoint = QPointF(pos.x(), std::min(sceneRange.at(0).y1(), rect.y() + rect.height()));
				endPoint = QPointF(pos.x(), std::max(sceneRange.at(0).y2(), rect.y()));

				lines.append(QLineF(startPoint, endPoint));
				// QDEBUG(Q_FUNC_INFO << ", Non Logical LINE = " << lines)
			}
		}
	}

	for (const auto& line : qAsConst(lines)) {
		linePath.moveTo(line.p1());
		linePath.lineTo(line.p2());
	}

	if (linePath.isEmpty()) {
		DEBUG(Q_FUNC_INFO << ", WARNING: line path is empty")
		recalcShapeAndBoundingRect();
		return;
	} else {
		retransformArrow();
		retransformTicks();
	}
}

void AxisPrivate::retransformArrow() {
	if (suppressRetransform)
		return;

	arrowPath = QPainterPath();
	if (arrowType == Axis::ArrowType::NoArrow || lines.isEmpty()) {
		recalcShapeAndBoundingRect();
		return;
	}

	if (arrowPosition == Axis::ArrowPosition::Right || arrowPosition == Axis::ArrowPosition::Both) {
		const QPointF& endPoint = lines.at(lines.size() - 1).p2();
		this->addArrow(endPoint, 1);
	}

	if (arrowPosition == Axis::ArrowPosition::Left || arrowPosition == Axis::ArrowPosition::Both) {
		const QPointF& endPoint = lines.at(0).p1();
		this->addArrow(endPoint, -1);
	}

	recalcShapeAndBoundingRect();
}

void AxisPrivate::addArrow(QPointF startPoint, int direction) {
	static const double cos_phi = cos(M_PI / 6.);

	if (orientation == Axis::Orientation::Horizontal) {
		QPointF endPoint = QPointF(startPoint.x() + direction * arrowSize, startPoint.y());
		arrowPath.moveTo(startPoint);
		arrowPath.lineTo(endPoint);

		switch (arrowType) {
		case Axis::ArrowType::NoArrow:
			break;
		case Axis::ArrowType::SimpleSmall:
			arrowPath.moveTo(endPoint);
			arrowPath.lineTo(QPointF(endPoint.x() - direction * arrowSize / 4, endPoint.y() - arrowSize / 4 * cos_phi));
			arrowPath.moveTo(endPoint);
			arrowPath.lineTo(QPointF(endPoint.x() - direction * arrowSize / 4, endPoint.y() + arrowSize / 4 * cos_phi));
			break;
		case Axis::ArrowType::SimpleBig:
			arrowPath.moveTo(endPoint);
			arrowPath.lineTo(QPointF(endPoint.x() - direction * arrowSize / 2, endPoint.y() - arrowSize / 2 * cos_phi));
			arrowPath.moveTo(endPoint);
			arrowPath.lineTo(QPointF(endPoint.x() - direction * arrowSize / 2, endPoint.y() + arrowSize / 2 * cos_phi));
			break;
		case Axis::ArrowType::FilledSmall:
			arrowPath.lineTo(QPointF(endPoint.x() - direction * arrowSize / 4, endPoint.y() - arrowSize / 4 * cos_phi));
			arrowPath.lineTo(QPointF(endPoint.x() - direction * arrowSize / 4, endPoint.y() + arrowSize / 4 * cos_phi));
			arrowPath.lineTo(endPoint);
			break;
		case Axis::ArrowType::FilledBig:
			arrowPath.lineTo(QPointF(endPoint.x() - direction * arrowSize / 2, endPoint.y() - arrowSize / 2 * cos_phi));
			arrowPath.lineTo(QPointF(endPoint.x() - direction * arrowSize / 2, endPoint.y() + arrowSize / 2 * cos_phi));
			arrowPath.lineTo(endPoint);
			break;
		case Axis::ArrowType::SemiFilledSmall:
			arrowPath.lineTo(QPointF(endPoint.x() - direction * arrowSize / 4, endPoint.y() - arrowSize / 4 * cos_phi));
			arrowPath.lineTo(QPointF(endPoint.x() - direction * arrowSize / 8, endPoint.y()));
			arrowPath.lineTo(QPointF(endPoint.x() - direction * arrowSize / 4, endPoint.y() + arrowSize / 4 * cos_phi));
			arrowPath.lineTo(endPoint);
			break;
		case Axis::ArrowType::SemiFilledBig:
			arrowPath.lineTo(QPointF(endPoint.x() - direction * arrowSize / 2, endPoint.y() - arrowSize / 2 * cos_phi));
			arrowPath.lineTo(QPointF(endPoint.x() - direction * arrowSize / 4, endPoint.y()));
			arrowPath.lineTo(QPointF(endPoint.x() - direction * arrowSize / 2, endPoint.y() + arrowSize / 2 * cos_phi));
			arrowPath.lineTo(endPoint);
			break;
		}
	} else { // vertical orientation
		QPointF endPoint = QPointF(startPoint.x(), startPoint.y() - direction * arrowSize);
		arrowPath.moveTo(startPoint);
		arrowPath.lineTo(endPoint);

		switch (arrowType) {
		case Axis::ArrowType::NoArrow:
			break;
		case Axis::ArrowType::SimpleSmall:
			arrowPath.moveTo(endPoint);
			arrowPath.lineTo(QPointF(endPoint.x() - arrowSize / 4 * cos_phi, endPoint.y() + direction * arrowSize / 4));
			arrowPath.moveTo(endPoint);
			arrowPath.lineTo(QPointF(endPoint.x() + arrowSize / 4 * cos_phi, endPoint.y() + direction * arrowSize / 4));
			break;
		case Axis::ArrowType::SimpleBig:
			arrowPath.moveTo(endPoint);
			arrowPath.lineTo(QPointF(endPoint.x() - arrowSize / 2 * cos_phi, endPoint.y() + direction * arrowSize / 2));
			arrowPath.moveTo(endPoint);
			arrowPath.lineTo(QPointF(endPoint.x() + arrowSize / 2 * cos_phi, endPoint.y() + direction * arrowSize / 2));
			break;
		case Axis::ArrowType::FilledSmall:
			arrowPath.lineTo(QPointF(endPoint.x() - arrowSize / 4 * cos_phi, endPoint.y() + direction * arrowSize / 4));
			arrowPath.lineTo(QPointF(endPoint.x() + arrowSize / 4 * cos_phi, endPoint.y() + direction * arrowSize / 4));
			arrowPath.lineTo(endPoint);
			break;
		case Axis::ArrowType::FilledBig:
			arrowPath.lineTo(QPointF(endPoint.x() - arrowSize / 2 * cos_phi, endPoint.y() + direction * arrowSize / 2));
			arrowPath.lineTo(QPointF(endPoint.x() + arrowSize / 2 * cos_phi, endPoint.y() + direction * arrowSize / 2));
			arrowPath.lineTo(endPoint);
			break;
		case Axis::ArrowType::SemiFilledSmall:
			arrowPath.lineTo(QPointF(endPoint.x() - arrowSize / 4 * cos_phi, endPoint.y() + direction * arrowSize / 4));
			arrowPath.lineTo(QPointF(endPoint.x(), endPoint.y() + direction * arrowSize / 8));
			arrowPath.lineTo(QPointF(endPoint.x() + arrowSize / 4 * cos_phi, endPoint.y() + direction * arrowSize / 4));
			arrowPath.lineTo(endPoint);
			break;
		case Axis::ArrowType::SemiFilledBig:
			arrowPath.lineTo(QPointF(endPoint.x() - arrowSize / 2 * cos_phi, endPoint.y() + direction * arrowSize / 2));
			arrowPath.lineTo(QPointF(endPoint.x(), endPoint.y() + direction * arrowSize / 4));
			arrowPath.lineTo(QPointF(endPoint.x() + arrowSize / 2 * cos_phi, endPoint.y() + direction * arrowSize / 2));
			arrowPath.lineTo(endPoint);
			break;
		}
	}
}

//! helper function for retransformTicks()
/*!
 * \brief AxisPrivate::transformAnchor
 * Transform a position in logical coordinates into scene corrdinates
 * \param anchorPoint point which should be converted. Contains the result of the conversion
 * if the transformation was valid
 * \return true if transformation was successful else false
 * Successful means, that the point is inside the coordinate system
 */
bool AxisPrivate::transformAnchor(QPointF& anchorPoint) {
	QVector<QPointF> points;
	points.append(anchorPoint);
	points = q->cSystem->mapLogicalToScene(points);

	if (points.count() != 1) { // point is not mappable or in a coordinate gap
		return false;
	} else {
		anchorPoint = points.at(0);
		return true;
	}
}

bool AxisPrivate::calculateTickHorizontal(Axis::TicksDirection tickDirection,
										  double ticksLength,
										  double tickStartPos,
										  double dummyOtherDirPos,
										  double otherDirAnchorPoint,
										  double centerValue,
										  int rangeDirection,
										  QPointF& anchorPointOut,
										  QPointF& startPointOut,
										  QPointF& endPointOut) {
	bool valid = false;
	anchorPointOut.setX(tickStartPos);
	anchorPointOut.setY(dummyOtherDirPos); // set dummy logical point, but it must be within the datarect, otherwise valid will be always false
	valid = transformAnchor(anchorPointOut);
	anchorPointOut.setY(otherDirAnchorPoint);
	if (valid) {
		// for yDirection == -1 start is above end
		if (anchorPointOut.y() >= centerValue) { // below
			startPointOut = anchorPointOut + QPointF(0, (tickDirection & Axis::ticksIn) ? rangeDirection * ticksLength : 0);
			endPointOut = anchorPointOut + QPointF(0, (tickDirection & Axis::ticksOut) ? -rangeDirection * ticksLength : 0);
		} else { // above
			startPointOut = anchorPointOut + QPointF(0, (tickDirection & Axis::ticksOut) ? rangeDirection * ticksLength : 0);
			endPointOut = anchorPointOut + QPointF(0, (tickDirection & Axis::ticksIn) ? -rangeDirection * ticksLength : 0);
		}
	}
	return valid;
}

bool AxisPrivate::calculateTickVertical(Axis::TicksDirection tickDirection,
										double ticksLength,
										double tickStartPos,
										double dummyOtherDirPos,
										double otherDirAnchorPoint,
										double centerValue,
										int rangeDirection,
										QPointF& anchorPointOut,
										QPointF& startPointOut,
										QPointF& endPointOut) {
	bool valid = false;
	anchorPointOut.setY(tickStartPos);
	anchorPointOut.setX(dummyOtherDirPos); // set dummy logical point, but it must be within the datarect, otherwise valid will be always false
	valid = transformAnchor(anchorPointOut);
	anchorPointOut.setX(otherDirAnchorPoint);
	if (valid) {
		// for xDirection == 1 start is right of end
		if (anchorPointOut.x() < centerValue) { // left
			startPointOut = anchorPointOut + QPointF((tickDirection & Axis::ticksIn) ? rangeDirection * ticksLength : 0, 0);
			endPointOut = anchorPointOut + QPointF((tickDirection & Axis::ticksOut) ? -rangeDirection * ticksLength : 0, 0);
		} else { // right
			startPointOut = anchorPointOut + QPointF((tickDirection & Axis::ticksOut) ? rangeDirection * ticksLength : 0, 0);
			endPointOut = anchorPointOut + QPointF((tickDirection & Axis::ticksIn) ? -rangeDirection * ticksLength : 0, 0);
		}
	}
	return valid;
}

int AxisPrivate::determineMinorTicksNumber() const {
	int tmpMinorTicksNumber = 0;
	switch (minorTicksType) {
	case Axis::TicksType::TotalNumber:
		tmpMinorTicksNumber = minorTicksNumber;
		break;
	case Axis::TicksType::Spacing:
		tmpMinorTicksNumber = range.length() / minorTicksIncrement - 1;
		if (majorTicksNumber > 1)
			tmpMinorTicksNumber /= majorTicksNumber - 1;
		break;
	case Axis::TicksType::CustomColumn: // Fall through
	case Axis::TicksType::CustomValues:
		(minorTicksColumn) ? tmpMinorTicksNumber = minorTicksColumn->rowCount() : tmpMinorTicksNumber = 0;
		break;
	case Axis::TicksType::ColumnLabels:
		break; // not supported
	}
	return tmpMinorTicksNumber;
}

/*!
	recalculates the position of the axis ticks.
 */
void AxisPrivate::retransformTicks() {
#if PERFTRACE_AXIS
	PERFTRACE(QLatin1String(Q_FUNC_INFO) + QStringLiteral(", axis ") + name());
#endif
	// DEBUG(Q_FUNC_INFO << ' ' << STDSTRING(title->name()))
	if (suppressRetransform)
		return;

	majorTicksPath = QPainterPath();
	minorTicksPath = QPainterPath();
	majorTickPoints.clear();
	minorTickPoints.clear();
	tickLabelValues.clear();
	tickLabelValuesString.clear();

	if (!q->cSystem) {
		DEBUG(Q_FUNC_INFO << ", WARNING: axis has no coordinate system!")
		return;
	}

	// determine the increment for the major ticks
	double majorTicksIncrement = 0;
	int tmpMajorTicksNumber = 0;
	double start{range.start()}, end{range.end()};
	if (majorTicksStartType == Axis::TicksStartType::Absolute) {
		start = majorTickStartValue;
	} else if (majorTicksStartType == Axis::TicksStartType::Offset) {
		if (q->isNumeric())
			start += majorTickStartOffset;
		else {
			auto startDt = QDateTime::fromMSecsSinceEpoch(start, Qt::UTC);
			startDt.setTimeSpec(Qt::TimeSpec::UTC);
			const auto& dt = DateTime::dateTime(majorTickStartOffset);
			startDt = startDt.addYears(dt.year);
			startDt = startDt.addMonths(dt.month);
			startDt = startDt.addDays(dt.day);
			startDt = startDt.addMSecs(DateTime::milliseconds(dt.hour, dt.minute, dt.second, dt.millisecond));
			start = startDt.toMSecsSinceEpoch();
		}
	}

	// if type is tick number and tick number is auto: recalculate in case scale has changed
	if (majorTicksType == Axis::TicksType::TotalNumber && majorTicksAutoNumber) {
		auto r = range;
		r.setStart(start);
		r.setEnd(end);
		majorTicksNumber = r.autoTickCount();
	}

	if (majorTicksNumber < 1 || (majorTicksDirection == Axis::noTicks && minorTicksDirection == Axis::noTicks)) {
		retransformTickLabelPositions(); // this calls recalcShapeAndBoundingRect()
		return;
	}

	if (majorTicksType == Axis::TicksType::CustomColumn || majorTicksType == Axis::TicksType::CustomValues) {
		if (majorTicksColumn) {
			if (majorTicksAutoNumber) {
				tmpMajorTicksNumber = qMin(_maxNumberMajorTicksCustomColumn, majorTicksColumn->rowCount(start, end));
				majorTicksNumber = tmpMajorTicksNumber;
				Q_EMIT q->majorTicksNumberChanged(tmpMajorTicksNumber);
			} else
				tmpMajorTicksNumber = majorTicksNumber;
			// Do the calculation of the new start/end after recalculating majorTicksNumber, otherwise it could happen that the
			// ticks are really near to each other
			if (start < end) {
				start = qMax(start, majorTicksColumn->minimum());
				end = qMin(end, majorTicksColumn->maximum());
			} else {
				end = qMax(end, majorTicksColumn->minimum());
				start = qMax(start, majorTicksColumn->maximum());
			}
		} else {
			retransformTickLabelPositions(); // this calls recalcShapeAndBoundingRect()
			return;
		}
	} else if (majorTicksType == Axis::TicksType::ColumnLabels) {
		const Column* c = dynamic_cast<const Column*>(majorTicksColumn);
		if (c && c->valueLabelsInitialized()) {
			if (majorTicksAutoNumber) {
				tmpMajorTicksNumber = qMin(_maxNumberMajorTicksCustomColumn, c->valueLabelsCount(start, end));
				majorTicksNumber = tmpMajorTicksNumber;
				Q_EMIT q->majorTicksNumberChanged(tmpMajorTicksNumber);
			} else
				tmpMajorTicksNumber = c->valueLabelsCount(start, end);
			if (start < end) {
				start = qMax(start, c->valueLabelsMinimum());
				end = qMin(end, c->valueLabelsMaximum());
			} else {
				end = qMax(end, c->valueLabelsMinimum());
				start = qMax(start, c->valueLabelsMaximum());
			}
		} else {
			retransformTickLabelPositions(); // this calls recalcShapeAndBoundingRect()
			return;
		}
	}

	QDEBUG(Q_FUNC_INFO << ", ticks type = " << majorTicksType)
	switch (majorTicksType) {
	case Axis::TicksType::TotalNumber: // total number of major ticks is given - > determine the increment
		tmpMajorTicksNumber = majorTicksNumber;
		// fall through
	case Axis::TicksType::ColumnLabels: // fall through
	case Axis::TicksType::CustomColumn: // fall through
	case Axis::TicksType::CustomValues:
		switch (q->scale()) {
		case RangeT::Scale::Linear:
		case RangeT::Scale::Inverse:
			majorTicksIncrement = end - start;
			break;
		case RangeT::Scale::Log10:
			if (start != 0. && end / start > 0.)
				majorTicksIncrement = log10(end / start);
			break;
		case RangeT::Scale::Log2:
			if (start != 0. && end / start > 0.)
				majorTicksIncrement = log2(end / start);
			break;
		case RangeT::Scale::Ln:
			if (start != 0. && end / start > 0.)
				majorTicksIncrement = log(end / start);
			break;
		case RangeT::Scale::Sqrt:
			if (start >= 0. && end >= 0.)
				majorTicksIncrement = std::sqrt(end) - std::sqrt(start);
			break;
		case RangeT::Scale::Square:
			majorTicksIncrement = end * end - start * start;
			break;
		}
		if (tmpMajorTicksNumber > 1)
			majorTicksIncrement /= tmpMajorTicksNumber - 1;
		DEBUG(Q_FUNC_INFO << ", major ticks by number. increment = " << majorTicksIncrement << " number = " << majorTicksNumber)
		break;
	case Axis::TicksType::Spacing:
		// the increment of the major ticks is given -> determine the number
		// TODO: majorTicksSpacing == 0?
		majorTicksIncrement = majorTicksSpacing * GSL_SIGN(end - start);
		if (q->isNumeric() || (!q->isNumeric() && q->scale() != RangeT::Scale::Linear)) {
			switch (q->scale()) {
			case RangeT::Scale::Linear:
			case RangeT::Scale::Inverse:
				tmpMajorTicksNumber = std::round((end - start) / majorTicksIncrement + 1);
				break;
			case RangeT::Scale::Log10:
				if (start != 0. && end / start > 0.)
					tmpMajorTicksNumber = std::round(log10(end / start) / majorTicksIncrement + 1);
				break;
			case RangeT::Scale::Log2:
				if (start != 0. && end / start > 0.)
					tmpMajorTicksNumber = std::round(log2(end / start) / majorTicksIncrement + 1);
				break;
			case RangeT::Scale::Ln:
				if (start != 0. && end / start > 0.)
					tmpMajorTicksNumber = std::round(std::log(end / start) / majorTicksIncrement + 1);
				break;
			case RangeT::Scale::Sqrt:
				if (start >= 0. && end >= 0.)
					tmpMajorTicksNumber = std::round((std::sqrt(end) - std::sqrt(start)) / majorTicksIncrement + 1);
				break;
			case RangeT::Scale::Square:
				tmpMajorTicksNumber = std::round((end * end - start * start) / majorTicksIncrement + 1);
				break;
			}
		} else {
			// Datetime with linear spacing: Calculation will be done directly where the majorTickPos will be calculated
		}
		break;
	}

	// minor ticks
	int tmpMinorTicksNumber = determineMinorTicksNumber();

	//	const int xIndex{ q->cSystem->index(Dimension::X) }, yIndex{ q->cSystem->index(Dimension::Y) };
	DEBUG(Q_FUNC_INFO << ", coordinate system " << q->m_cSystemIndex + 1)
	auto cs = plot()->coordinateSystem(q->coordinateSystemIndex());
	const int xRangeDirection = plot()->range(Dimension::X, cs->index(Dimension::X)).direction();
	const int yRangeDirection = plot()->range(Dimension::Y, cs->index(Dimension::Y)).direction();
	const int xDirection = q->cSystem->direction(Dimension::X) * xRangeDirection;
	const int yDirection = q->cSystem->direction(Dimension::Y) * yRangeDirection;

	// calculate the position of the center point in scene coordinates,
	// will be used later to differentiate between "in" and "out" depending
	// on the position relative to the center.
	const double middleX = plot()->range(Dimension::X, cs->index(Dimension::X)).center();
	const double middleY = plot()->range(Dimension::Y, cs->index(Dimension::Y)).center();
	QPointF center(middleX, middleY);
	bool valid = true;
	center = q->cSystem->mapLogicalToScene(center, valid);

	const bool dateTimeSpacing = !q->isNumeric() && q->scale() == RangeT::Scale::Linear && majorTicksType == Axis::TicksType::Spacing;
	DateTime::DateTime dt;
	QDateTime majorTickPosDateTime;
	QDateTime nextMajorTickPosDateTime;
	qreal nextMajorTickPos = 0.0;
	if (dateTimeSpacing) {
		dt = DateTime::dateTime(majorTicksSpacing);
		majorTickPosDateTime = QDateTime::fromMSecsSinceEpoch(start, Qt::UTC);
	}
	const auto dtValid = majorTickPosDateTime.isValid();

	for (int iMajor = 0; iMajor < tmpMajorTicksNumber || (dateTimeSpacing && dtValid); iMajor++) {
		// DEBUG(Q_FUNC_INFO << ", major tick " << iMajor)
		qreal majorTickPos = 0.0;
		// calculate major tick's position
		if (!dateTimeSpacing) {
			// DEBUG(Q_FUNC_INFO << ", start = " << start << ", incr = " << majorTicksIncrement << ", i = " << iMajor)
			switch (q->scale()) {
			case RangeT::Scale::Linear:
			case RangeT::Scale::Inverse:
				majorTickPos = start + majorTicksIncrement * iMajor;
				if (std::abs(majorTickPos) < 1.e-15 * majorTicksIncrement) // avoid rounding errors when close to zero
					majorTickPos = 0;
				nextMajorTickPos = majorTickPos + majorTicksIncrement;
				break;
			case RangeT::Scale::Log10:
				majorTickPos = start * std::pow(10, majorTicksIncrement * iMajor);
				nextMajorTickPos = majorTickPos * std::pow(10, majorTicksIncrement);
				break;
			case RangeT::Scale::Log2:
				majorTickPos = start * std::exp2(majorTicksIncrement * iMajor);
				nextMajorTickPos = majorTickPos * exp2(majorTicksIncrement);
				break;
			case RangeT::Scale::Ln:
				majorTickPos = start * std::exp(majorTicksIncrement * iMajor);
				nextMajorTickPos = majorTickPos * exp(majorTicksIncrement);
				break;
			case RangeT::Scale::Sqrt:
				majorTickPos = std::pow(std::sqrt(start) + majorTicksIncrement * iMajor, 2);
				nextMajorTickPos = std::pow(std::sqrt(start) + majorTicksIncrement * (iMajor + 1), 2);
				break;
			case RangeT::Scale::Square:
				majorTickPos = std::sqrt(start * start + majorTicksIncrement * iMajor);
				nextMajorTickPos = std::sqrt(start * start + majorTicksIncrement * (iMajor + 1));
				break;
			}
			// DEBUG(majorTickPos << " " << nextMajorTickPos)
		} else {
			// Datetime Linear
			if (iMajor == 0)
				majorTickPos = start;
			else {
				majorTickPosDateTime = nextMajorTickPosDateTime;
				majorTickPos = nextMajorTickPos;
			}

			nextMajorTickPosDateTime = majorTickPosDateTime;
			nextMajorTickPosDateTime = nextMajorTickPosDateTime.addYears(dt.year);
			nextMajorTickPosDateTime = nextMajorTickPosDateTime.addMonths(dt.month);
			nextMajorTickPosDateTime = nextMajorTickPosDateTime.addDays(dt.day);
			nextMajorTickPosDateTime = nextMajorTickPosDateTime.addMSecs(DateTime::milliseconds(dt.hour, dt.minute, dt.second, dt.millisecond));
			nextMajorTickPos = nextMajorTickPosDateTime.toMSecsSinceEpoch();
		}

		// finish here when out of range
		if (iMajor > maxNumberMajorTicks)
			break;
		if ((majorTicksIncrement > 0 && majorTickPos > end) || (majorTicksIncrement < 0 && majorTickPos < end))
			break;

		int columnIndex = iMajor; // iMajor used if for the labels a custom column is used.
		if ((majorTicksType == Axis::TicksType::CustomColumn || majorTicksType == Axis::TicksType::CustomValues)
			&& (majorTicksColumn->rowCount() >= _maxNumberMajorTicksCustomColumn)) {
			// Do not use all values of the column, but just a portion of it
			columnIndex = majorTicksColumn->indexForValue(majorTickPos);
			Q_ASSERT(columnIndex >= 0);
			majorTickPos = majorTicksColumn->valueAt(columnIndex);

			const auto columnIndexNextMajor = majorTicksColumn->indexForValue(nextMajorTickPos);
			Q_ASSERT(columnIndexNextMajor >= 0);
			nextMajorTickPos = majorTicksColumn->valueAt(columnIndexNextMajor);
			if (majorTickPos == nextMajorTickPos && iMajor + 1 < tmpMajorTicksNumber)
				continue; // No need to draw majorTicksPos, because NextMajorTicksPos will completely overlap. Only for the last one
		} else if ((majorTicksType == Axis::TicksType::CustomColumn || majorTicksType == Axis::TicksType::CustomValues)) {
			majorTickPos = majorTicksColumn->valueAt(columnIndex);
			if (majorTicksColumn->rowCount() > columnIndex + 1)
				nextMajorTickPos = majorTicksColumn->valueAt(columnIndex + 1);
			else
				nextMajorTickPos = majorTickPos;
		} else if (majorTicksType == Axis::TicksType::ColumnLabels) {
			const auto* c = static_cast<const Column*>(majorTicksColumn);
			Q_ASSERT(tmpMajorTicksNumber > 0);
			Q_ASSERT(c);
			columnIndex = c->valueLabelsIndexForValue(majorTickPos);
			Q_ASSERT(columnIndex >= 0);
			majorTickPos = c->valueLabelsValueAt(columnIndex);
		}

		qreal otherDirAnchorPoint = 0.0;
		if (!lines.isEmpty()) {
			if (orientation == Axis::Orientation::Vertical)
				otherDirAnchorPoint = lines.first().p1().x();
			else
				otherDirAnchorPoint = lines.first().p1().y();
		}

		QPointF anchorPoint, startPoint, endPoint;
		// calculate start and end points for major tick's line
		if (majorTicksDirection != Axis::noTicks) {
			if (orientation == Axis::Orientation::Horizontal) {
				auto startY = q->plot()->range(Dimension::Y, cs->index(Dimension::Y)).start();
				valid = calculateTickHorizontal(majorTicksDirection,
												majorTicksLength,
												majorTickPos,
												startY,
												otherDirAnchorPoint,
												center.y(),
												yDirection,
												anchorPoint,
												startPoint,
												endPoint);
			} else { // vertical
				auto startX = q->plot()->range(Dimension::X, cs->index(Dimension::X)).start();
				valid = calculateTickVertical(majorTicksDirection,
											  majorTicksLength,
											  majorTickPos,
											  startX,
											  otherDirAnchorPoint,
											  center.x(),
											  xDirection,
											  anchorPoint,
											  startPoint,
											  endPoint);
			}

			const qreal value = scalingFactor * majorTickPos + zeroOffset;
			// DEBUG(Q_FUNC_INFO << ", value = " << value << " " << scalingFactor << " " << majorTickPos << " " << zeroOffset)

			// if custom column is used, we can have duplicated values in it and we need only unique values
			if ((majorTicksType == Axis::TicksType::CustomColumn || majorTicksType == Axis::TicksType::ColumnLabels) && tickLabelValues.indexOf(value) != -1)
				valid = false;

			// add major tick's line to the painter path
			if (valid) {
				if (majorTicksLine->pen().style() != Qt::NoPen) {
					majorTicksPath.moveTo(startPoint);
					majorTicksPath.lineTo(endPoint);
				}
				majorTickPoints << anchorPoint;
				if (majorTicksType == Axis::TicksType::ColumnLabels) {
					const Column* c = dynamic_cast<const Column*>(majorTicksColumn);
					// majorTicksType == Axis::TicksType::ColumnLabels
					if (c && c->valueLabelsInitialized()) {
						if (columnIndex < c->valueLabelsCount())
							tickLabelValuesString << c->valueLabelAt(columnIndex);
					}
				} else {
					switch (labelsTextType) {
					case Axis::LabelsTextType::PositionValues:
						tickLabelValues << value;
						break;
					case Axis::LabelsTextType::CustomValues: {
						if (labelsTextColumn && columnIndex < labelsTextColumn->rowCount()) {
							switch (labelsTextColumn->columnMode()) {
							case AbstractColumn::ColumnMode::Double:
							case AbstractColumn::ColumnMode::Integer:
							case AbstractColumn::ColumnMode::BigInt:
								tickLabelValues << labelsTextColumn->valueAt(columnIndex);
								break;
							case AbstractColumn::ColumnMode::DateTime:
							case AbstractColumn::ColumnMode::Month:
							case AbstractColumn::ColumnMode::Day:
								tickLabelValues << labelsTextColumn->dateTimeAt(columnIndex).toMSecsSinceEpoch();
								break;
							case AbstractColumn::ColumnMode::Text:
								tickLabelValuesString << labelsTextColumn->textAt(columnIndex);
								break;
							}
						}
					}
					}
				}
			}
		}

		// minor ticks
		// DEBUG("	tmpMinorTicksNumber = " << tmpMinorTicksNumber)
		if (Axis::noTicks != minorTicksDirection && tmpMinorTicksNumber > 0
			&& ((tmpMajorTicksNumber > 1 && iMajor < tmpMajorTicksNumber - 1) || (dateTimeSpacing && dtValid)) && nextMajorTickPos != majorTickPos) {
			// minor ticks are placed at equidistant positions independent of the selected scaling for the major ticks positions
			double minorTicksIncrement = (nextMajorTickPos - majorTickPos) / (tmpMinorTicksNumber + 1);
			// DEBUG("	nextMajorTickPos = " << nextMajorTickPos)
			// DEBUG("	majorTickPos = " << majorTickPos)
			// DEBUG("	minorTicksIncrement = " << minorTicksIncrement)

			qreal minorTickPos;
			for (int iMinor = 0; iMinor < tmpMinorTicksNumber; iMinor++) {
				// calculate minor tick's position
				if (minorTicksType != Axis::TicksType::CustomColumn) {
					minorTickPos = majorTickPos + (iMinor + 1) * minorTicksIncrement;
				} else {
					if (!minorTicksColumn->isValid(iMinor) || minorTicksColumn->isMasked(iMinor))
						continue;
					minorTickPos = minorTicksColumn->valueAt(iMinor);

					// in the case a custom column is used for the minor ticks, we draw them _once_ for the whole range of the axis.
					// execute the minor ticks loop only once.
					if (iMajor > 0)
						break;
				}
				// DEBUG("		minorTickPos = " << minorTickPos)

				// calculate start and end points for minor tick's line (same as major ticks)
				if (orientation == Axis::Orientation::Horizontal) {
					auto startY = q->plot()->range(Dimension::Y, cs->index(Dimension::Y)).start();
					valid = calculateTickHorizontal(minorTicksDirection,
													minorTicksLength,
													minorTickPos,
													startY,
													otherDirAnchorPoint,
													center.y(),
													yDirection,
													anchorPoint,
													startPoint,
													endPoint);
				} else { // vertical
					auto startX = q->plot()->range(Dimension::X, cs->index(Dimension::X)).start();
					valid = calculateTickVertical(minorTicksDirection,
												  minorTicksLength,
												  minorTickPos,
												  startX,
												  otherDirAnchorPoint,
												  center.x(),
												  xDirection,
												  anchorPoint,
												  startPoint,
												  endPoint);
				}

				// add minor tick's line to the painter path
				if (valid) {
					if (minorTicksLine->pen().style() != Qt::NoPen) {
						minorTicksPath.moveTo(startPoint);
						minorTicksPath.lineTo(endPoint);
					}
					minorTickPoints << anchorPoint;
				}
			}
		}
	}
	//	QDEBUG(Q_FUNC_INFO << tickLabelValues)

	// tick positions where changed -> update the position of the tick labels and grid lines
	retransformTickLabelStrings();
	retransformMajorGrid();
	retransformMinorGrid();
}

/*!
	creates the tick label strings starting with the optimal
	(=the smallest possible number of digits) precision for the floats
*/
void AxisPrivate::retransformTickLabelStrings() {
	DEBUG(Q_FUNC_INFO << ' ' << STDSTRING(title->name()) << ", labels precision = " << labelsPrecision << ", labels auto precision = " << labelsAutoPrecision)
	if (suppressRetransform)
		return;
	QDEBUG(Q_FUNC_INFO << ", values = " << tickLabelValues)

	const auto cs = plot()->coordinateSystem(q->coordinateSystemIndex());

	// automatically switch from 'decimal' to 'scientific' format for large and small numbers
	// and back to decimal when the numbers get smaller after the auto-switch
	DEBUG(Q_FUNC_INFO << ", format = " << ENUM_TO_STRING(Axis, LabelsFormat, labelsFormat))
	if (labelsFormatAuto) {
		bool largeValue = false;
		for (auto value : tickLabelValues) {
			// switch to Scientific for large and small values if at least one is
			if (std::abs(value) > 1.e4 || (std::abs(value) > 1.e-16 && std::abs(value) < 1e-4)) {
				largeValue = true;
				break;
			}
		}
		if (largeValue)
			labelsFormat = Axis::LabelsFormat::Scientific;
		else
			labelsFormat = Axis::LabelsFormat::Decimal;
		Q_EMIT q->labelsFormatChanged(labelsFormat);
	}

	// determine labels precision
	if (labelsAutoPrecision) {
		// do we need to increase the current precision?
		int newPrecision = upperLabelsPrecision(labelsPrecision, labelsFormat);
		if (newPrecision != labelsPrecision) {
			labelsPrecision = newPrecision;
			Q_EMIT q->labelsPrecisionChanged(labelsPrecision);
		} else {
			// can we reduce the current precision?
			newPrecision = lowerLabelsPrecision(labelsPrecision, labelsFormat);
			if (newPrecision != labelsPrecision) {
				labelsPrecision = newPrecision;
				Q_EMIT q->labelsPrecisionChanged(labelsPrecision);
			}
		}
		DEBUG(Q_FUNC_INFO << ", auto labels precision = " << labelsPrecision)
	}

	// category of format
	bool numeric = false, datetime = false, text = false;
	if (majorTicksType == Axis::TicksType::ColumnLabels)
		text = true;
	else if (labelsTextType == Axis::LabelsTextType::PositionValues) {
		auto xRangeFormat{plot()->range(Dimension::X, cs->index(Dimension::X)).format()};
		auto yRangeFormat{plot()->range(Dimension::Y, cs->index(Dimension::Y)).format()};
		numeric = ((orientation == Axis::Orientation::Horizontal && xRangeFormat == RangeT::Format::Numeric)
				   || (orientation == Axis::Orientation::Vertical && yRangeFormat == RangeT::Format::Numeric));

		if (!numeric)
			datetime = true;
	} else {
		if (labelsTextColumn) {
			switch (labelsTextColumn->columnMode()) {
			case AbstractColumn::ColumnMode::Double:
			case AbstractColumn::ColumnMode::Integer:
			case AbstractColumn::ColumnMode::BigInt:
				numeric = true;
				break;
			case AbstractColumn::ColumnMode::DateTime:
			case AbstractColumn::ColumnMode::Month:
			case AbstractColumn::ColumnMode::Day:
				datetime = true;
				break;
			case AbstractColumn::ColumnMode::Text:
				text = true;
			}
		}
	}

	tickLabelStrings.clear();
	QString str;
	const auto numberLocale = QLocale();
	if (numeric) {
		switch (labelsFormat) {
		case Axis::LabelsFormat::Decimal: {
			QString nullStr = numberLocale.toString(0., 'f', labelsPrecision);
			for (const auto value : qAsConst(tickLabelValues)) {
				// toString() does not round: use NSL function
				if (RangeT::isLogScale(q->scale())) { // don't use same precision for all label on log scales
					const int precision = labelsAutoPrecision ? std::max(labelsPrecision, nsl_math_decimal_places(value) + 1) : labelsPrecision;
					str = numberLocale.toString(value, 'f', precision);
				} else
					str = numberLocale.toString(nsl_math_round_places(value, labelsPrecision), 'f', labelsPrecision);
				if (str == QLatin1String("-") + nullStr)
					str = nullStr;
				str = labelsPrefix + str + labelsSuffix;
				tickLabelStrings << str;
			}
			break;
		}
		case Axis::LabelsFormat::ScientificE: {
			QString nullStr = numberLocale.toString(0., 'e', labelsPrecision);
			for (const auto value : qAsConst(tickLabelValues)) {
				if (value == 0) // just show "0"
					str = numberLocale.toString(value, 'f', 0);
				else {
					int e;
					const double frac = nsl_math_frexp10(value, &e);
					// DEBUG(Q_FUNC_INFO << ", rounded frac * pow (10, e) = " << nsl_math_round_places(frac, labelsPrecision) * pow(10, e))
					str = numberLocale.toString(nsl_math_round_places(frac, labelsPrecision) * gsl_pow_int(10., e), 'e', labelsPrecision);
				}
				if (str == QLatin1String("-") + nullStr)
					str = nullStr; // avoid "-O"
				str = labelsPrefix + str + labelsSuffix;
				tickLabelStrings << str;
			}
			break;
		}
		case Axis::LabelsFormat::Powers10: {
			for (const auto value : qAsConst(tickLabelValues)) {
				if (value == 0) // just show "0"
					str = numberLocale.toString(value, 'f', 0);
				else {
					str = QStringLiteral("10<sup>")
						+ numberLocale.toString(nsl_math_round_places(log10(std::abs(value)), labelsPrecision), 'f', labelsPrecision)
						+ QStringLiteral("</sup>");
					if (value < 0)
						str.prepend(QLatin1Char('-'));
				}
				str = labelsPrefix + str + labelsSuffix;
				tickLabelStrings << str;
			}
			break;
		}
		case Axis::LabelsFormat::Powers2: {
			for (const auto value : qAsConst(tickLabelValues)) {
				if (value == 0) // just show "0"
					str = numberLocale.toString(value, 'f', 0);
				else {
					str = QStringLiteral("2<span style=\"vertical-align:super\">")
						+ numberLocale.toString(nsl_math_round_places(log2(std::abs(value)), labelsPrecision), 'f', labelsPrecision)
						+ QStringLiteral("</spanlabelsPrecision)>");
					if (value < 0)
						str.prepend(QLatin1Char('-'));
				}
				str = labelsPrefix + str + labelsSuffix;
				tickLabelStrings << str;
			}
			break;
		}
		case Axis::LabelsFormat::PowersE: {
			for (const auto value : qAsConst(tickLabelValues)) {
				if (value == 0) // just show "0"
					str = numberLocale.toString(value, 'f', 0);
				else {
					str = QStringLiteral("e<span style=\"vertical-align:super\">")
						+ numberLocale.toString(nsl_math_round_places(log(std::abs(value)), labelsPrecision), 'f', labelsPrecision) + QStringLiteral("</span>");
					if (value < 0)
						str.prepend(QLatin1Char('-'));
				}
				str = labelsPrefix + str + labelsSuffix;
				tickLabelStrings << str;
			}
			break;
		}
		case Axis::LabelsFormat::MultipliesPi: {
			for (const auto value : qAsConst(tickLabelValues)) {
				if (value == 0) // just show "0"
					str = numberLocale.toString(value, 'f', 0);
				else if (nsl_math_approximately_equal_eps(value, M_PI, 1.e-3))
					str = QChar(0x03C0);
				else
					str = QStringLiteral("<span>") + numberLocale.toString(nsl_math_round_places(value / M_PI, labelsPrecision), 'f', labelsPrecision)
						+ QStringLiteral("</span>") + QChar(0x03C0);
				str = labelsPrefix + str + labelsSuffix;
				tickLabelStrings << str;
			}
			break;
		}
		case Axis::LabelsFormat::Scientific: {
			for (const auto value : qAsConst(tickLabelValues)) {
				// DEBUG(Q_FUNC_INFO << ", value = " << value << ", precision = " << labelsPrecision)
				if (value == 0) // just show "0"
					str = numberLocale.toString(value, 'f', 0);
				else {
					int e;
					const double frac = nsl_math_frexp10(value, &e);
					if (std::abs(value) < 100. && std::abs(value) > .01) // use normal notation for values near 1, precision reduced by exponent but >= 0
						str = numberLocale.toString(nsl_math_round_places(frac, labelsPrecision) * gsl_pow_int(10., e), 'f', std::max(labelsPrecision - e, 0));
					else {
						// DEBUG(Q_FUNC_INFO << ", nsl rounded = " << nsl_math_round_places(frac, labelsPrecision))
						//  only round fraction
						str = numberLocale.toString(nsl_math_round_places(frac, labelsPrecision), 'f', labelsPrecision);
						str = createScientificRepresentation(str, numberLocale.toString(e));
					}
				}
				str = labelsPrefix + str + labelsSuffix;
				tickLabelStrings << str;
			}
		}

			DEBUG(Q_FUNC_INFO << ", tick label = " << STDSTRING(str))
		}
	} else if (datetime) {
		for (const auto value : qAsConst(tickLabelValues)) {
			QDateTime dateTime;
			dateTime.setTimeSpec(Qt::UTC);
			dateTime.setMSecsSinceEpoch(value);
			str = dateTime.toString(labelsDateTimeFormat);
			str = labelsPrefix + str + labelsSuffix;
			tickLabelStrings << str;
		}
	} else if (text) {
		for (auto& t : tickLabelValuesString) {
			str = labelsPrefix + t + labelsSuffix;
			tickLabelStrings << str;
		}
	}

	QDEBUG(Q_FUNC_INFO << ", strings = " << tickLabelStrings)

	// recalculate the position of the tick labels
	retransformTickLabelPositions();
}

/*!
	returns the smallest upper limit for the precision
	where no duplicates for the tick label values occur.
 */
int AxisPrivate::upperLabelsPrecision(const int precision, const Axis::LabelsFormat format) {
	DEBUG(Q_FUNC_INFO << ", precision = " << precision << ", format = " << ENUM_TO_STRING(Axis, LabelsFormat, format));

	// catch out of limit values
	if (precision > 6)
		return 6;

	// avoid problems with zero range axis
	if (tickLabelValues.isEmpty() || qFuzzyCompare(tickLabelValues.constFirst(), tickLabelValues.constLast())) {
		DEBUG(Q_FUNC_INFO << ", zero range axis detected.")
		return 0;
	}

	// round values to the current precision and look for duplicates.
	// if there are duplicates, increase the precision.
	QVector<double> tempValues;
	tempValues.reserve(tickLabelValues.size());

	switch (format) {
	case Axis::LabelsFormat::Decimal:
		for (const auto value : tickLabelValues)
			tempValues.append(nsl_math_round_places(value, precision));
		break;
	case Axis::LabelsFormat::MultipliesPi:
		for (const auto value : tickLabelValues)
			tempValues.append(nsl_math_round_places(value / M_PI, precision));
		break;
	case Axis::LabelsFormat::ScientificE:
	case Axis::LabelsFormat::Scientific:
		for (const auto value : tickLabelValues) {
			int e;
			const double frac = nsl_math_frexp10(value, &e);
			// DEBUG(Q_FUNC_INFO << ", frac = " << frac << ", exp = " << e)
			tempValues.append(nsl_math_round_precision(frac, precision) * gsl_pow_int(10., e));
		}
		break;
	case Axis::LabelsFormat::Powers10:
		for (const auto value : tickLabelValues) {
			if (value == 0)
				tempValues.append(log10(DBL_MIN));
			else {
				// DEBUG(Q_FUNC_INFO << ", rounded value = " << nsl_math_round_places(log10(std::abs(value)), precision))
				tempValues.append(nsl_math_round_places(log10(std::abs(value)), precision));
			}
		}
		break;
	case Axis::LabelsFormat::Powers2:
		for (const auto value : tickLabelValues) {
			if (value == 0)
				tempValues.append(log2(DBL_MIN));
			else
				tempValues.append(nsl_math_round_places(log2(std::abs(value)), precision));
		}
		break;
	case Axis::LabelsFormat::PowersE:
		for (const auto value : tickLabelValues) {
			if (value == 0)
				tempValues.append(log(DBL_MIN));
			else
				tempValues.append(nsl_math_round_places(log(std::abs(value)), precision));
		}
	}
	// QDEBUG(Q_FUNC_INFO << ", rounded values: " << tempValues)

	const double scaling = std::abs(tickLabelValues.last() - tickLabelValues.first());
	DEBUG(Q_FUNC_INFO << ", scaling = " << scaling)
	for (int i = 0; i < tempValues.size(); ++i) {
		// check if rounded value differs too much
		double relDiff = 0;
		// DEBUG(Q_FUNC_INFO << ", round value = " << tempValues.at(i) << ", tick label =  " << tickLabelValues.at(i))
		switch (format) {
		case Axis::LabelsFormat::Decimal:
		case Axis::LabelsFormat::Scientific:
		case Axis::LabelsFormat::ScientificE:
			relDiff = std::abs(tempValues.at(i) - tickLabelValues.at(i)) / scaling;
			break;
		case Axis::LabelsFormat::MultipliesPi:
			relDiff = std::abs(M_PI * tempValues.at(i) - tickLabelValues.at(i)) / scaling;
			break;
		case Axis::LabelsFormat::Powers10:
			relDiff = std::abs(nsl_sf_exp10(tempValues.at(i)) - tickLabelValues.at(i)) / scaling;
			break;
		case Axis::LabelsFormat::Powers2:
			relDiff = std::abs(exp2(tempValues.at(i)) - tickLabelValues.at(i)) / scaling;
			break;
		case Axis::LabelsFormat::PowersE:
			relDiff = std::abs(exp(tempValues.at(i)) - tickLabelValues.at(i)) / scaling;
		}
		// DEBUG(Q_FUNC_INFO << ", rel. diff = " << relDiff)
		for (int j = 0; j < tempValues.size(); ++j) {
			if (i == j)
				continue;

			// if duplicate for the current precision found or differs too much, increase the precision and check again
			// DEBUG(Q_FUNC_INFO << ", compare " << tempValues.at(i) << " with " << tempValues.at(j))
			if (tempValues.at(i) == tempValues.at(j) || relDiff > 0.01) { // > 1%
				// DEBUG(Q_FUNC_INFO << ", duplicates found : " << tempValues.at(i))
				return upperLabelsPrecision(precision + 1, format);
			}
		}
	}

	// no duplicates for the current precision found: return the current value
	DEBUG(Q_FUNC_INFO << ", upper precision = " << precision);
	return precision;
}

/*!
	returns highest lower limit for the precision
	where no duplicates for the tick label values occur.
*/
int AxisPrivate::lowerLabelsPrecision(const int precision, const Axis::LabelsFormat format) {
	DEBUG(Q_FUNC_INFO << ", precision = " << precision << ", format = " << ENUM_TO_STRING(Axis, LabelsFormat, format));
	// round value to the current precision and look for duplicates.
	// if there are duplicates, decrease the precision.

	// no tick labels, no precision
	if (tickLabelValues.size() == 0)
		return 0;

	QVector<double> tempValues;
	tempValues.reserve(tickLabelValues.size());

	switch (format) {
	case Axis::LabelsFormat::Decimal:
		for (auto value : tickLabelValues)
			tempValues.append(nsl_math_round_places(value, precision));
		break;
	case Axis::LabelsFormat::MultipliesPi:
		for (auto value : tickLabelValues)
			tempValues.append(nsl_math_round_places(value / M_PI, precision));
		break;
	case Axis::LabelsFormat::ScientificE:
	case Axis::LabelsFormat::Scientific:
		for (auto value : tickLabelValues) {
			int e;
			const double frac = nsl_math_frexp10(value, &e);
			// DEBUG(Q_FUNC_INFO << ", frac = " << frac << ", exp = " << e)
			// DEBUG(Q_FUNC_INFO << ", rounded frac = " << nsl_math_round_precision(frac, precision))
			tempValues.append(nsl_math_round_precision(frac, precision) * gsl_pow_int(10., e));
		}
		break;
	case Axis::LabelsFormat::Powers10:
		for (auto value : tickLabelValues) {
			if (value == 0)
				tempValues.append(log10(DBL_MIN));
			else
				tempValues.append(nsl_math_round_places(log10(std::abs(value)), precision));
		}
		break;
	case Axis::LabelsFormat::Powers2:
		for (auto value : tickLabelValues) {
			if (value == 0)
				tempValues.append(log2(DBL_MIN));
			else
				tempValues.append(nsl_math_round_places(log2(std::abs(value)), precision));
		}
		break;
	case Axis::LabelsFormat::PowersE:
		for (auto value : tickLabelValues) {
			if (value == 0)
				tempValues.append(log(DBL_MIN));
			else
				tempValues.append(nsl_math_round_places(log(std::abs(value)), precision));
		}
	}
	// QDEBUG(Q_FUNC_INFO << ", rounded values = " << tempValues)

	// check whether we have duplicates with reduced precision
	//-> current precision cannot be reduced, return the previous value
	const double scale = std::abs(tickLabelValues.last() - tickLabelValues.first());
	// DEBUG(Q_FUNC_INFO << ", scale = " << scale)
	for (int i = 0; i < tempValues.size(); ++i) {
		// return if rounded value differs too much
		double relDiff = 0;
		switch (format) {
		case Axis::LabelsFormat::Decimal:
		case Axis::LabelsFormat::Scientific:
		case Axis::LabelsFormat::ScientificE:
			relDiff = std::abs(tempValues.at(i) - tickLabelValues.at(i)) / scale;
			break;
		case Axis::LabelsFormat::MultipliesPi:
			relDiff = std::abs(M_PI * tempValues.at(i) - tickLabelValues.at(i)) / scale;
			break;
		case Axis::LabelsFormat::Powers10:
			relDiff = std::abs(nsl_sf_exp10(tempValues.at(i)) - tickLabelValues.at(i)) / scale;
			break;
		case Axis::LabelsFormat::Powers2:
			relDiff = std::abs(exp2(tempValues.at(i)) - tickLabelValues.at(i)) / scale;
			break;
		case Axis::LabelsFormat::PowersE:
			relDiff = std::abs(exp(tempValues.at(i)) - tickLabelValues.at(i)) / scale;
		}
		// DEBUG(Q_FUNC_INFO << ", rel. diff = " << relDiff)

		if (relDiff > 0.01) // > 1 %
			return precision + 1;
		for (int j = 0; j < tempValues.size(); ++j) {
			if (i == j)
				continue;
			if (tempValues.at(i) == tempValues.at(j))
				return precision + 1;
		}
	}

	// no duplicates found, reduce further, and check again
	if (precision > 0)
		return lowerLabelsPrecision(precision - 1, format);

	return 0;
}

/*!
	recalculates the position of the tick labels.
	Called when the geometry related properties (position, offset, font size, suffix, prefix) of the labels are changed.
 */
void AxisPrivate::retransformTickLabelPositions() {
	tickLabelPoints.clear();
	if (majorTicksDirection == Axis::noTicks || labelsPosition == Axis::LabelsPosition::NoLabels) {
		recalcShapeAndBoundingRect();
		return;
	}

	QFontMetrics fm(labelsFont);
	double width = 0, height = fm.ascent();
	QPointF pos;

	//	const int xIndex{ q->cSystem->index(Dimension::X) }, yIndex{ q->cSystem->index(Dimension::Y) };
	DEBUG(Q_FUNC_INFO << ' ' << STDSTRING(title->name()) << ", coordinate system index = " << q->m_cSystemIndex)
	//	DEBUG(Q_FUNC_INFO << ", x range " << xIndex+1)
	//	DEBUG(Q_FUNC_INFO << ", y range " << yIndex+1)
	auto cs = plot()->coordinateSystem(q->coordinateSystemIndex());
	const double middleX = plot()->range(Dimension::X, cs->index(Dimension::X)).center();
	const double middleY = plot()->range(Dimension::Y, cs->index(Dimension::Y)).center();
	QPointF center(middleX, middleY);
	//	const int xDirection = q->cSystem->direction(Dimension::X);
	//	const int yDirection = q->cSystem->direction(Dimension::Y);

	//	QPointF startPoint, endPoint, anchorPoint;

	QTextDocument td;
	td.setDefaultFont(labelsFont);
	const double cosine = std::cos(qDegreesToRadians(labelsRotationAngle)); // calculate only once
	const double sine = std::sin(qDegreesToRadians(labelsRotationAngle)); // calculate only once

	int size = std::min(majorTickPoints.size(), tickLabelStrings.size());
	auto xRangeFormat{plot()->range(Dimension::X, cs->index(Dimension::X)).format()};
	auto yRangeFormat{plot()->range(Dimension::Y, cs->index(Dimension::Y)).format()};
	for (int i = 0; i < size; i++) {
		if ((orientation == Axis::Orientation::Horizontal && xRangeFormat == RangeT::Format::Numeric)
			|| (orientation == Axis::Orientation::Vertical && yRangeFormat == RangeT::Format::Numeric)) {
			if (labelsFormat == Axis::LabelsFormat::Decimal || labelsFormat == Axis::LabelsFormat::ScientificE) {
				width = fm.boundingRect(tickLabelStrings.at(i)).width();
			} else {
				td.setHtml(tickLabelStrings.at(i));
				width = td.size().width();
				height = td.size().height();
			}
		} else { // Datetime
			width = fm.boundingRect(tickLabelStrings.at(i)).width();
		}

		const double diffx = cosine * width;
		const double diffy = sine * width;
		QPointF anchorPoint = majorTickPoints.at(i);

		// center align all labels with respect to the end point of the tick line
		const int xRangeDirection = plot()->range(Dimension::X, cs->index(Dimension::X)).direction();
		const int yRangeDirection = plot()->range(Dimension::Y, cs->index(Dimension::Y)).direction();
		//		DEBUG(Q_FUNC_INFO << ", x/y range direction = " << xRangeDirection << "/" << yRangeDirection)
		const int xDirection = q->cSystem->direction(Dimension::X) * xRangeDirection;
		const int yDirection = q->cSystem->direction(Dimension::Y) * yRangeDirection;
		//		DEBUG(Q_FUNC_INFO << ", x/y direction = " << xDirection << "/" << yDirection)
		QPointF startPoint, endPoint;
		if (orientation == Axis::Orientation::Horizontal) {
			if (anchorPoint.y() >= center.y()) { // below
				startPoint = anchorPoint + QPointF(0, (majorTicksDirection & Axis::ticksIn) ? yDirection * majorTicksLength : 0);
				endPoint = anchorPoint + QPointF(0, (majorTicksDirection & Axis::ticksOut) ? -yDirection * majorTicksLength : 0);
			} else { // above
				startPoint = anchorPoint + QPointF(0, (majorTicksDirection & Axis::ticksOut) ? yDirection * majorTicksLength : 0);
				endPoint = anchorPoint + QPointF(0, (majorTicksDirection & Axis::ticksIn) ? -yDirection * majorTicksLength : 0);
			}

			// for rotated labels (angle is not zero), align label's corner at the position of the tick
			if (std::abs(std::abs(labelsRotationAngle) - 180.) < 1.e-2) { // +-180°
				if (labelsPosition == Axis::LabelsPosition::Out) {
					pos.setX(endPoint.x() + width / 2);
					pos.setY(endPoint.y() + labelsOffset);
				} else {
					pos.setX(startPoint.x() + width / 2);
					pos.setY(startPoint.y() - height - labelsOffset);
				}
			} else if (labelsRotationAngle <= -0.01) { // [-0.01°, -180°)
				if (labelsPosition == Axis::LabelsPosition::Out) {
					pos.setX(endPoint.x() + sine * height / 2);
					pos.setY(endPoint.y() + labelsOffset + cosine * height / 2);
				} else {
					pos.setX(startPoint.x() + sine * height / 2 - diffx);
					pos.setY(startPoint.y() - labelsOffset + cosine * height / 2 + diffy);
				}
			} else if (labelsRotationAngle >= 0.01) { // [0.01°, 180°)
				if (labelsPosition == Axis::LabelsPosition::Out) {
					pos.setX(endPoint.x() - diffx + sine * height / 2);
					pos.setY(endPoint.y() + labelsOffset + diffy + cosine * height / 2);
				} else {
					pos.setX(startPoint.x() + sine * height / 2);
					pos.setY(startPoint.y() - labelsOffset + cosine * height / 2);
				}
			} else { // 0°
				if (labelsPosition == Axis::LabelsPosition::Out) {
					pos.setX(endPoint.x() - width / 2);
					pos.setY(endPoint.y() + height + labelsOffset);
				} else {
					pos.setX(startPoint.x() - width / 2);
					pos.setY(startPoint.y() - labelsOffset);
				}
			}
		} else { // ---------------------- vertical -------------------------
			if (anchorPoint.x() < center.x()) {
				startPoint = anchorPoint + QPointF((majorTicksDirection & Axis::ticksIn) ? xDirection * majorTicksLength : 0, 0);
				endPoint = anchorPoint + QPointF((majorTicksDirection & Axis::ticksOut) ? -xDirection * majorTicksLength : 0, 0);
			} else {
				startPoint = anchorPoint + QPointF((majorTicksDirection & Axis::ticksOut) ? xDirection * majorTicksLength : 0, 0);
				endPoint = anchorPoint + QPointF((majorTicksDirection & Axis::ticksIn) ? -xDirection * majorTicksLength : 0, 0);
			}

			if (std::abs(labelsRotationAngle - 90.) < 1.e-2) { // +90°
				if (labelsPosition == Axis::LabelsPosition::Out) {
					pos.setX(endPoint.x() - labelsOffset);
					pos.setY(endPoint.y() + width / 2);
				} else {
					pos.setX(startPoint.x() + labelsOffset);
					pos.setY(startPoint.y() + width / 2);
				}
			} else if (std::abs(labelsRotationAngle + 90.) < 1.e-2) { // -90°
				if (labelsPosition == Axis::LabelsPosition::Out) {
					pos.setX(endPoint.x() - labelsOffset - height);
					pos.setY(endPoint.y() - width / 2);
				} else {
					pos.setX(startPoint.x() + labelsOffset);
					pos.setY(startPoint.y() - width / 2);
				}
			} else if (std::abs(std::abs(labelsRotationAngle) - 180.) < 1.e-2) { // +-180°
				if (labelsPosition == Axis::LabelsPosition::Out) {
					pos.setX(endPoint.x() - labelsOffset);
					pos.setY(endPoint.y() - height / 2);
				} else {
					pos.setX(startPoint.x() + labelsOffset + width);
					pos.setY(startPoint.y() - height / 2);
				}
			} else if (std::abs(labelsRotationAngle) >= 0.01 && std::abs(labelsRotationAngle) <= 89.99) { // [0.01°, 90°)
				if (labelsPosition == Axis::LabelsPosition::Out) {
					// left
					pos.setX(endPoint.x() - labelsOffset - diffx + sine * height / 2);
					pos.setY(endPoint.y() + cosine * height / 2 + diffy);
				} else {
					pos.setX(startPoint.x() + labelsOffset + sine * height / 2);
					pos.setY(startPoint.y() + cosine * height / 2);
				}
			} else if (std::abs(labelsRotationAngle) >= 90.01 && std::abs(labelsRotationAngle) <= 179.99) { // [90.01, 180)
				if (labelsPosition == Axis::LabelsPosition::Out) {
					// left
					pos.setX(endPoint.x() - labelsOffset + sine * height / 2);
					pos.setY(endPoint.y() + cosine * height / 2);
				} else {
					pos.setX(startPoint.x() + labelsOffset - diffx + sine * height / 2);
					pos.setY(startPoint.y() + diffy + cosine * height / 2);
				}
			} else { // 0°
				if (labelsPosition == Axis::LabelsPosition::Out) {
					pos.setX(endPoint.x() - width - labelsOffset);
					pos.setY(endPoint.y() + height / 2);
				} else {
					pos.setX(startPoint.x() + labelsOffset);
					pos.setY(startPoint.y() + height / 2);
				}
			}
		}
		tickLabelPoints << pos;
	}

	recalcShapeAndBoundingRect();
}

void AxisPrivate::retransformMajorGrid() {
	if (suppressRetransform)
		return;

	majorGridPath = QPainterPath();
	if (majorGridLine->pen().style() == Qt::NoPen || majorTickPoints.size() == 0) {
		recalcShapeAndBoundingRect();
		return;
	}

	// major tick points are already in scene coordinates, convert them back to logical...
	// TODO: mapping should work without SuppressPageClipping-flag, check float comparisons in the map-function.
	// Currently, grid lines disappear sometimes without this flag
	QVector<QPointF> logicalMajorTickPoints = q->cSystem->mapSceneToLogical(majorTickPoints, AbstractCoordinateSystem::MappingFlag::SuppressPageClipping);
	// for (auto p : logicalMajorTickPoints)
	//	QDEBUG(Q_FUNC_INFO << ", logical major tick: " << QString::number(p.x(), 'g', 12) << " = " << QDateTime::fromMSecsSinceEpoch(p.x(), Qt::UTC))

	if (logicalMajorTickPoints.isEmpty())
		return;

	DEBUG(Q_FUNC_INFO << ' ' << STDSTRING(title->name()) << ", coordinate system " << q->m_cSystemIndex + 1)
	DEBUG(Q_FUNC_INFO << ", x range " << q->cSystem->index(Dimension::X) + 1)
	DEBUG(Q_FUNC_INFO << ", y range " << q->cSystem->index(Dimension::Y) + 1)
	auto cs = plot()->coordinateSystem(q->coordinateSystemIndex());
	const auto& xRange = plot()->range(Dimension::X, cs->index(Dimension::X));
	const auto& yRange = plot()->range(Dimension::Y, cs->index(Dimension::Y));

	// TODO:
	// when iterating over all grid lines, skip the first and the last points for auto scaled axes,
	// since we don't want to paint any grid lines at the plot boundaries
	bool skipLowestTick, skipUpperTick;
	if (orientation == Axis::Orientation::Horizontal) { // horizontal axis
		skipLowestTick = qFuzzyCompare(logicalMajorTickPoints.at(0).x(), xRange.start());
		skipUpperTick = qFuzzyCompare(logicalMajorTickPoints.at(logicalMajorTickPoints.size() - 1).x(), xRange.end());
	} else {
		skipLowestTick = qFuzzyCompare(logicalMajorTickPoints.at(0).y(), yRange.start());
		skipUpperTick = qFuzzyCompare(logicalMajorTickPoints.at(logicalMajorTickPoints.size() - 1).y(), yRange.end());
	}

	int start, end; // TODO: hides Axis::start, Axis::end!
	if (skipLowestTick) {
		if (logicalMajorTickPoints.size() > 1)
			start = 1;
		else
			start = 0;
	} else {
		start = 0;
	}

	if (skipUpperTick) {
		if (logicalMajorTickPoints.size() > 1)
			end = logicalMajorTickPoints.size() - 1;
		else
			end = 0;

	} else {
		end = logicalMajorTickPoints.size();
	}

	QVector<QLineF> lines;
	if (orientation == Axis::Orientation::Horizontal) { // horizontal axis
		for (int i = start; i < end; ++i) {
			const QPointF& point = logicalMajorTickPoints.at(i);
			lines.append(QLineF(point.x(), yRange.start(), point.x(), yRange.end()));
		}
	} else { // vertical axis
		// skip the first and the last points, since we don't want to paint any grid lines at the plot boundaries
		for (int i = start; i < end; ++i) {
			const QPointF& point = logicalMajorTickPoints.at(i);
			lines.append(QLineF(xRange.start(), point.y(), xRange.end(), point.y()));
		}
	}

	lines = q->cSystem->mapLogicalToScene(lines, AbstractCoordinateSystem::MappingFlag::SuppressPageClipping);
	for (const auto& line : lines) {
		majorGridPath.moveTo(line.p1());
		majorGridPath.lineTo(line.p2());
	}

	recalcShapeAndBoundingRect();
}

void AxisPrivate::retransformMinorGrid() {
	if (suppressRetransform)
		return;

	minorGridPath = QPainterPath();
	if (minorGridLine->pen().style() == Qt::NoPen) {
		recalcShapeAndBoundingRect();
		return;
	}

	// minor tick points are already in scene coordinates, convert them back to logical...
	// TODO: mapping should work without SuppressPageClipping-flag, check float comparisons in the map-function.
	// Currently, grid lines disappear sometimes without this flag
	QVector<QPointF> logicalMinorTickPoints = q->cSystem->mapSceneToLogical(minorTickPoints, AbstractCoordinateSystem::MappingFlag::SuppressPageClipping);

	DEBUG(Q_FUNC_INFO << ' ' << STDSTRING(title->name()) << ", coordinate system " << q->m_cSystemIndex + 1)
	DEBUG(Q_FUNC_INFO << ", x range " << q->cSystem->index(Dimension::X) + 1)
	DEBUG(Q_FUNC_INFO << ", y range " << q->cSystem->index(Dimension::Y) + 1)

	QVector<QLineF> lines;
	auto cs = plot()->coordinateSystem(q->coordinateSystemIndex());
	if (orientation == Axis::Orientation::Horizontal) { // horizontal axis
		const Range<double> yRange{plot()->range(Dimension::Y, cs->index(Dimension::Y))};

		for (const auto& point : logicalMinorTickPoints)
			lines.append(QLineF(point.x(), yRange.start(), point.x(), yRange.end()));
	} else { // vertical axis
		const Range<double> xRange{plot()->range(Dimension::X, cs->index(Dimension::X))};

		for (const auto& point : logicalMinorTickPoints)
			lines.append(QLineF(xRange.start(), point.y(), xRange.end(), point.y()));
	}

	lines = q->cSystem->mapLogicalToScene(lines, AbstractCoordinateSystem::MappingFlag::SuppressPageClipping);
	for (const auto& line : lines) {
		minorGridPath.moveTo(line.p1());
		minorGridPath.lineTo(line.p2());
	}

	recalcShapeAndBoundingRect();
}

/*!
 * called when the opacity of the grid was changes, update the grid graphics item
 */
// TODO: this function is only needed for loaded projects where update() doesn't seem to be enough
// and we have to call gridItem->update() explicitly.
// This is not required for newly created plots/axes. Why is this difference?
void AxisPrivate::updateGrid() {
	gridItem->update();
}

void AxisPrivate::recalcShapeAndBoundingRect() {
	if (suppressRecalc)
		return;

	prepareGeometryChange();

	QPainterPath tmpPath; // temp path used to calculate the bounding box for all elements that the axis consists of

	if (linePath.isEmpty()) {
		m_boundingRectangle = QRectF();
		title->setPositionInvalid(true);
		if (plot())
			plot()->prepareGeometryChange();
		return;
	} else {
		title->setPositionInvalid(false);
	}

	const auto& linePen = line->pen();
	tmpPath = WorksheetElement::shapeFromPath(linePath, linePen);
	tmpPath.addPath(WorksheetElement::shapeFromPath(arrowPath, linePen));
	tmpPath.addPath(WorksheetElement::shapeFromPath(majorTicksPath, majorTicksLine->pen()));
	tmpPath.addPath(WorksheetElement::shapeFromPath(minorTicksPath, minorTicksLine->pen()));

	QPainterPath tickLabelsPath = QPainterPath();
	if (labelsPosition != Axis::LabelsPosition::NoLabels) {
		QTransform trafo;
		QPainterPath tempPath;
		QFontMetrics fm(labelsFont);
		QTextDocument td;
		td.setDefaultFont(labelsFont);
		for (int i = 0; i < tickLabelPoints.size(); i++) {
			tempPath = QPainterPath();
			if (labelsFormat == Axis::LabelsFormat::Decimal || labelsFormat == Axis::LabelsFormat::ScientificE) {
				tempPath.addRect(fm.boundingRect(tickLabelStrings.at(i)));
			} else {
				td.setHtml(tickLabelStrings.at(i));
				tempPath.addRect(QRectF(0, -td.size().height(), td.size().width(), td.size().height()));
			}

			trafo.reset();
			trafo.translate(tickLabelPoints.at(i).x(), tickLabelPoints.at(i).y());

			trafo.rotate(-labelsRotationAngle);
			tempPath = trafo.map(tempPath);

			tickLabelsPath.addPath(WorksheetElement::shapeFromPath(tempPath, linePen));
		}
		tmpPath.addPath(WorksheetElement::shapeFromPath(tickLabelsPath, QPen()));
	}

	const auto margin = (double)hoverSelectionEffectPenWidth / 2;
	const auto axisRect = tmpPath.boundingRect().marginsRemoved(QMarginsF(margin, margin, margin, margin));
	tmpPath.addRect(axisRect); // add rect instead of the actual path for ticks - this is done for performance reasons,the calculation for many ticks and long
							   // tick texts can be very expensive

	// add title label, if available
	QTextDocument doc; // text may be Html, so check if plain text is empty
	doc.setHtml(title->text().text);
	// QDEBUG(Q_FUNC_INFO << ", title text plain: " << doc.toPlainText())
	QPainterPath titlePath;
	QPolygonF polygon;
	if (title->isVisible() && !doc.toPlainText().isEmpty()) {
		const QRectF& titleRect = title->graphicsItem()->boundingRect();
		if (titleRect.size() != QSizeF(0, 0)) {
			// determine the new position of the title label:
			// we calculate the new position here and not in retransform(),
			// since it depends on the size and position of the tick labels, tickLabelsPath, available here.
			QRectF rect = linePath.boundingRect();
			qreal offsetX = titleOffsetX, offsetY = titleOffsetY; // the distances to the axis line
			if (orientation == Axis::Orientation::Horizontal) {
				offsetY -= titleRect.height() * title->scale() / 2.;
				if (labelsPosition == Axis::LabelsPosition::Out)
					offsetY -= labelsOffset + tickLabelsPath.boundingRect().height();
				title->setPosition(QPointF((rect.topLeft().x() + rect.topRight().x()) / 2. + titleOffsetX, rect.bottomLeft().y() - offsetY));
			} else {
				offsetX -= titleRect.height() * title->scale() / 2.;
				if (labelsPosition == Axis::LabelsPosition::Out)
					offsetX -= labelsOffset + tickLabelsPath.boundingRect().width();
				title->setPosition(QPointF(rect.topLeft().x() + offsetX, (rect.topLeft().y() + rect.bottomLeft().y()) / 2. - titleOffsetY));
			}
			titlePath = WorksheetElement::shapeFromPath(title->graphicsItem()->mapToParent(title->graphicsItem()->shape()), linePen);
			const auto& axisTopLeft = axisRect.topLeft();
			const auto& axisTopRight = axisRect.topRight();
			const auto& axisBottomLeft = axisRect.bottomLeft();
			const auto& axisBottomRight = axisRect.bottomRight();
			const auto& titleTopLeft = titlePath.boundingRect().topLeft();
			const auto& titleTopRight = titlePath.boundingRect().topRight();
			const auto& titleBottomLeft = titlePath.boundingRect().bottomLeft();
			const auto& titleBottomRight = titlePath.boundingRect().bottomRight();

			QVector<QPointF> vertices;

			if (titlePath.intersects(tmpPath)) {
				// Draw cross shaped bounded rect
				if (Axis::Orientation::Horizontal == orientation) {
					if (axisTopLeft.y() < titleTopLeft.y()) {
						vertices << axisTopLeft << axisBottomLeft << QPointF(titleTopLeft.x(), axisBottomLeft.y()) << titleBottomLeft << titleBottomRight
								 << QPointF(titleTopRight.x(), axisBottomRight.y()) << axisBottomRight << axisTopRight << axisTopLeft;
					} else if (axisBottomLeft.y() > titleBottomLeft.y()) {
						vertices << axisTopLeft << QPointF(titleBottomLeft.x(), axisTopLeft.y()) << titleTopLeft << titleTopRight
								 << QPointF(titleBottomRight.x(), axisTopRight.y()) << axisTopRight << axisBottomRight << axisBottomLeft << axisTopLeft;
					} else {
						vertices << titleTopLeft << QPointF(titleTopLeft.x(), axisTopLeft.y()) << axisTopLeft << axisBottomLeft
								 << QPointF(titleBottomLeft.x(), axisBottomLeft.y()) << titleBottomLeft << titleBottomRight
								 << QPointF(titleBottomRight.x(), axisBottomRight.y()) << axisBottomRight << axisTopRight
								 << QPointF(titleTopRight.x(), axisTopRight.y()) << titleTopRight << titleTopLeft;
					}
				} else {
					if (axisTopRight.x() > titleTopRight.x()) {
						vertices << axisTopLeft << QPointF(axisTopLeft.x(), titleTopRight.y()) << titleTopLeft << titleBottomLeft
								 << QPointF(axisBottomLeft.x(), titleBottomRight.y()) << axisBottomLeft << axisBottomRight << axisTopRight << axisTopLeft;
					} else if (axisTopLeft.x() < titleTopLeft.x()) {
						vertices << axisTopLeft << axisTopRight << QPointF(axisTopRight.x(), titleTopRight.y()) << titleTopRight << titleBottomRight
								 << QPointF(axisBottomRight.x(), titleBottomLeft.y()) << axisBottomRight << axisBottomLeft << axisTopLeft;
					} else {
						vertices << axisTopLeft << QPointF(axisTopLeft.x(), titleTopLeft.y()) << titleTopLeft << titleBottomLeft
								 << QPointF(axisTopLeft.x(), titleBottomLeft.y()) << axisBottomLeft << axisBottomRight
								 << QPointF(axisTopRight.x(), titleBottomRight.y()) << titleBottomRight << titleTopRight
								 << QPointF(axisTopRight.x(), titleTopRight.y()) << axisTopRight << axisTopLeft;
					}
				}
			} else {
				// Draw T-shaped bounded rect
				if (Axis::Orientation::Horizontal == orientation) {
					if (axisTopLeft.y() < titleTopLeft.y() || axisBottomLeft.y() < titleBottomLeft.y())
						vertices << axisTopLeft << axisBottomLeft << QPointF(titleTopLeft.x(), axisBottomLeft.y()) << titleBottomLeft << titleBottomRight
								 << QPointF(titleTopRight.x(), axisBottomRight.y()) << axisBottomRight << axisTopRight << axisTopLeft;
					else
						vertices << axisTopLeft << QPointF(titleBottomLeft.x(), axisTopLeft.y()) << titleTopLeft << titleTopRight
								 << QPointF(titleBottomRight.x(), axisTopRight.y()) << axisTopRight << axisBottomRight << axisBottomLeft << axisTopLeft;
				} else {
					if (axisTopLeft.x() > titleTopLeft.x() || axisBottomLeft.x() > titleBottomLeft.x())
						vertices << axisTopLeft << QPointF(axisTopLeft.x(), titleTopRight.y()) << titleTopLeft << titleBottomLeft
								 << QPointF(axisBottomLeft.x(), titleBottomRight.y()) << axisBottomLeft << axisBottomRight << axisTopRight << axisTopLeft;
					else
						vertices << axisTopLeft << axisTopRight << QPointF(axisTopRight.x(), titleTopRight.y()) << titleTopRight << titleBottomRight
								 << QPointF(axisBottomRight.x(), titleBottomLeft.y()) << axisBottomRight << axisBottomLeft << axisTopLeft;
				}
			}
			for (auto vertex : vertices)
				polygon.append(vertex);
			tmpPath.addPolygon(polygon);
		}
	}
	m_boundingRectangle = tmpPath.boundingRect();
	m_shape = QPainterPath();
	if (!polygon.isEmpty())
		m_shape.addPolygon(polygon);
	else
		m_shape.addRect(m_boundingRectangle);
	// if the axis goes beyond the current bounding box of the plot (too high offset is used, too long labels etc.)
	// request a prepareGeometryChange() for the plot in order to properly keep track of geometry changes
	if (plot())
		plot()->prepareGeometryChange();

	Q_EMIT q->changed();
}

/*!
	paints the content of the axis. Reimplemented from \c QGraphicsItem.
	\sa QGraphicsItem::paint()
 */
void AxisPrivate::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget*) {
	if (!isVisible() || linePath.isEmpty())
		return;

	// draw the line
	if (line->pen().style() != Qt::NoPen) {
		painter->setOpacity(line->opacity());
		painter->setPen(line->pen());
		painter->drawPath(linePath);

		// DUMP_PAINTER_PATH(linePath);

		// draw the arrow
		if (arrowType != Axis::ArrowType::NoArrow) {
			painter->setBrush(QBrush(line->color(), Qt::SolidPattern));
			painter->drawPath(arrowPath);
		}
	}

	// draw the major ticks
	if (majorTicksDirection != Axis::noTicks) {
		painter->setOpacity(majorTicksLine->opacity());
		painter->setPen(majorTicksLine->pen());
		painter->setBrush(Qt::NoBrush);
		painter->drawPath(majorTicksPath);
	}

	// draw the minor ticks
	if (minorTicksDirection != Axis::noTicks) {
		painter->setOpacity(minorTicksLine->opacity());
		painter->setPen(minorTicksLine->pen());
		painter->setBrush(Qt::NoBrush);
		painter->drawPath(minorTicksPath);
	}

	// draw tick labels
	if (labelsPosition != Axis::LabelsPosition::NoLabels) {
		auto cs = plot()->coordinateSystem(q->coordinateSystemIndex());
		painter->setOpacity(labelsOpacity);
		painter->setPen(QPen(labelsColor));
		painter->setFont(labelsFont);
		QTextDocument doc;
		doc.setDefaultFont(labelsFont);
		QFontMetrics fm(labelsFont);
		auto xRangeFormat{plot()->range(Dimension::X, cs->index(Dimension::X)).format()};
		auto yRangeFormat{plot()->range(Dimension::Y, cs->index(Dimension::Y)).format()};
		if ((orientation == Axis::Orientation::Horizontal && xRangeFormat == RangeT::Format::Numeric)
			|| (orientation == Axis::Orientation::Vertical && yRangeFormat == RangeT::Format::Numeric)) {
			// QDEBUG(Q_FUNC_INFO << ", axis tick label strings: " << tickLabelStrings)
			for (int i = 0; i < tickLabelPoints.size(); i++) {
				painter->translate(tickLabelPoints.at(i));
				painter->save();
				painter->rotate(-labelsRotationAngle);

				if (labelsFormat == Axis::LabelsFormat::Decimal || labelsFormat == Axis::LabelsFormat::ScientificE) {
					if (labelsBackgroundType != Axis::LabelsBackgroundType::Transparent) {
						const QRect& rect = fm.boundingRect(tickLabelStrings.at(i));
						painter->fillRect(rect, labelsBackgroundColor);
					}
					painter->drawText(QPoint(0, 0), tickLabelStrings.at(i));
				} else {
					const QString style(QStringLiteral("p {color: %1;}"));
					doc.setDefaultStyleSheet(style.arg(labelsColor.name()));
					doc.setHtml(QStringLiteral("<p>") + tickLabelStrings.at(i) + QStringLiteral("</p>"));
					QSizeF size = doc.size();
					int height = size.height();
					if (labelsBackgroundType != Axis::LabelsBackgroundType::Transparent) {
						int width = size.width();
						painter->fillRect(0, -height, width, height, labelsBackgroundColor);
					}
					painter->translate(0, -height);
					doc.drawContents(painter);
				}
				painter->restore();
				painter->translate(-tickLabelPoints.at(i));
			}
		} else { // datetime
			for (int i = 0; i < tickLabelPoints.size(); i++) {
				painter->translate(tickLabelPoints.at(i));
				painter->save();
				painter->rotate(-labelsRotationAngle);
				if (labelsBackgroundType != Axis::LabelsBackgroundType::Transparent) {
					const QRect& rect = fm.boundingRect(tickLabelStrings.at(i));
					painter->fillRect(rect, labelsBackgroundColor);
				}
				painter->drawText(QPoint(0, 0), tickLabelStrings.at(i));
				painter->restore();
				painter->translate(-tickLabelPoints.at(i));
			}
		}

		// scale + offset label
		if (showScaleOffset && tickLabelPoints.size() > 0) {
			QString text;
			const auto numberLocale = QLocale();
			if (scalingFactor != 1)
				text += UTF8_QSTRING("×") + numberLocale.toString(1. / scalingFactor);
			if (zeroOffset != 0) {
				if (zeroOffset < 0)
					text += QLatin1String("+");
				text += numberLocale.toString(-zeroOffset);
			}

			// used to determinde direction (up/down, left/right)
			auto cs = plot()->coordinateSystem(q->coordinateSystemIndex());
			const qreal middleX = plot()->range(Dimension::X, cs->index(Dimension::X)).center();
			const qreal middleY = plot()->range(Dimension::Y, cs->index(Dimension::Y)).center();
			QPointF center(middleX, middleY);
			bool valid = true;
			center = q->cSystem->mapLogicalToScene(center, valid);

			QPointF lastTickPoint = tickLabelPoints.at(tickLabelPoints.size() - 1);
			QPointF labelPosition;
			QFontMetrics fm(labelsFont);
			if (orientation == Axis::Orientation::Horizontal) {
				if (center.y() < lastTickPoint.y())
					labelPosition = QPointF(-fm.boundingRect(text).width(), 40);
				else
					labelPosition = QPointF(-fm.boundingRect(text).width(), -40);
			} else {
				if (center.x() < lastTickPoint.x())
					labelPosition = QPointF(40, 40);
				else
					labelPosition = QPointF(-fm.boundingRect(text).width() - 10, 40);
			}
			const QPointF offsetLabelPoint = lastTickPoint + labelPosition;
			painter->translate(offsetLabelPoint);
			// TODO: own format, rotation, etc.
			//	painter->save();
			painter->drawText(QPoint(0, 0), text);
			//	painter->restore();
			painter->translate(-offsetLabelPoint);
		}
	}

	// shape and label
	if (m_hovered && !isSelected() && !q->isPrinting()) {
		painter->setPen(QPen(QApplication::palette().color(QPalette::Shadow), hoverSelectionEffectPenWidth, Qt::SolidLine));
		painter->drawPath(m_shape);
	}

	if (isSelected() && !q->isPrinting()) {
		painter->setPen(QPen(QApplication::palette().color(QPalette::Highlight), hoverSelectionEffectPenWidth, Qt::SolidLine));
		painter->drawPath(m_shape);
	}

#if DEBUG_AXIS_BOUNDING_RECT
	painter->setPen(QColor(Qt::GlobalColor::blue));
	painter->drawRect(boundingRect());
#endif
}

void AxisPrivate::mousePressEvent(QGraphicsSceneMouseEvent* event) {
	auto* plot = static_cast<CartesianPlot*>(q->parentAspect());
	if (plot->isInteractive()) {
		m_panningStarted = true;
		m_panningStart = event->pos();
	} else
		QGraphicsItem::mousePressEvent(event);
}

void AxisPrivate::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
	if (m_panningStarted) {
		Dimension dim = Dimension::X;
		int delta = 0;
		auto cs = plot()->coordinateSystem(q->coordinateSystemIndex());
		if (orientation == WorksheetElement::Orientation::Horizontal) {
			setCursor(Qt::SizeHorCursor);
			delta = (m_panningStart.x() - event->pos().x());
			if (std::abs(delta) < 5.)
				return;
			dim = Dimension::X;
		} else {
			setCursor(Qt::SizeVerCursor);
			delta = (m_panningStart.y() - event->pos().y());
			if (std::abs(delta) < 5.)
				return;
			dim = Dimension::Y;
		}

		Q_EMIT q->shiftSignal(delta, dim, cs->index(dim));

		m_panningStart = event->pos();
	}
}

void AxisPrivate::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
	setCursor(Qt::ArrowCursor);
	m_panningStarted = false;
	QGraphicsItem::mouseReleaseEvent(event);
}

QString AxisPrivate::createScientificRepresentation(const QString& mantissa, const QString& exponent) {
	return mantissa + QStringLiteral("×10<sup>") + exponent + QStringLiteral("</sup>");
}

// ##############################################################################
// ##################  Serialization/Deserialization  ###########################
// ##############################################################################
//! Save as XML
void Axis::save(QXmlStreamWriter* writer) const {
	Q_D(const Axis);

	writer->writeStartElement(QStringLiteral("axis"));
	writeBasicAttributes(writer);
	writeCommentElement(writer);

	// general
	writer->writeStartElement(QStringLiteral("general"));
	writer->writeAttribute(QStringLiteral("rangeType"), QString::number(static_cast<int>(d->rangeType)));
	writer->writeAttribute(QStringLiteral("orientation"), QString::number(static_cast<int>(d->orientation)));
	writer->writeAttribute(QStringLiteral("position"), QString::number(static_cast<int>(d->position)));
	writer->writeAttribute(QStringLiteral("scale"), QString::number(static_cast<int>(d->scale)));
	writer->writeAttribute(QStringLiteral("rangeScale"), QString::number(static_cast<int>(d->rangeScale)));
	writer->writeAttribute(QStringLiteral("offset"), QString::number(d->offset));
	writer->writeAttribute(QStringLiteral("logicalPosition"), QString::number(d->logicalPosition));
	writer->writeAttribute(QStringLiteral("scaleRange"), QString::number(static_cast<int>(d->range.scale())));
	writer->writeAttribute(QStringLiteral("start"), QString::number(d->range.start(), 'g', 12));
	writer->writeAttribute(QStringLiteral("end"), QString::number(d->range.end(), 'g', 12));
	writer->writeAttribute(QStringLiteral("majorTicksStartType"), QString::number(static_cast<int>(d->majorTicksStartType)));
	writer->writeAttribute(QStringLiteral("majorTickStartOffset"), QString::number(d->majorTickStartOffset));
	writer->writeAttribute(QStringLiteral("majorTickStartValue"), QString::number(d->majorTickStartValue));
	writer->writeAttribute(QStringLiteral("scalingFactor"), QString::number(d->scalingFactor));
	writer->writeAttribute(QStringLiteral("zeroOffset"), QString::number(d->zeroOffset));
	writer->writeAttribute(QStringLiteral("showScaleOffset"), QString::number(d->showScaleOffset));
	writer->writeAttribute(QStringLiteral("titleOffsetX"), QString::number(d->titleOffsetX));
	writer->writeAttribute(QStringLiteral("titleOffsetY"), QString::number(d->titleOffsetY));
	writer->writeAttribute(QStringLiteral("plotRangeIndex"), QString::number(m_cSystemIndex));
	writer->writeAttribute(QStringLiteral("visible"), QString::number(d->isVisible()));
	writer->writeEndElement();

	// label
	d->title->save(writer);

	// line
	writer->writeStartElement(QStringLiteral("line"));
	d->line->save(writer);
	writer->writeAttribute(QStringLiteral("arrowType"), QString::number(static_cast<int>(d->arrowType)));
	writer->writeAttribute(QStringLiteral("arrowPosition"), QString::number(static_cast<int>(d->arrowPosition)));
	writer->writeAttribute(QStringLiteral("arrowSize"), QString::number(d->arrowSize));
	writer->writeEndElement();

	// major ticks
	writer->writeStartElement(QStringLiteral("majorTicks"));
	writer->writeAttribute(QStringLiteral("direction"), QString::number(d->majorTicksDirection));
	writer->writeAttribute(QStringLiteral("type"), QString::number(static_cast<int>(d->majorTicksType)));
	writer->writeAttribute(QStringLiteral("numberAuto"), QString::number(d->majorTicksAutoNumber));
	writer->writeAttribute(QStringLiteral("number"), QString::number(d->majorTicksNumber));
	writer->writeAttribute(QStringLiteral("increment"), QString::number(d->majorTicksSpacing));
	WRITE_COLUMN(d->majorTicksColumn, majorTicksColumn);
	writer->writeAttribute(QStringLiteral("length"), QString::number(d->majorTicksLength));
	d->majorTicksLine->save(writer);
	writer->writeEndElement();

	// minor ticks
	writer->writeStartElement(QStringLiteral("minorTicks"));
	writer->writeAttribute(QStringLiteral("direction"), QString::number(d->minorTicksDirection));
	writer->writeAttribute(QStringLiteral("type"), QString::number(static_cast<int>(d->minorTicksType)));
	writer->writeAttribute(QStringLiteral("numberAuto"), QString::number(d->minorTicksAutoNumber));
	writer->writeAttribute(QStringLiteral("number"), QString::number(d->minorTicksNumber));
	writer->writeAttribute(QStringLiteral("increment"), QString::number(d->minorTicksIncrement));
	WRITE_COLUMN(d->minorTicksColumn, minorTicksColumn);
	writer->writeAttribute(QStringLiteral("length"), QString::number(d->minorTicksLength));
	d->minorTicksLine->save(writer);
	writer->writeEndElement();

	// extra ticks

	// labels
	writer->writeStartElement(QStringLiteral("labels"));
	writer->writeAttribute(QStringLiteral("position"), QString::number(static_cast<int>(d->labelsPosition)));
	writer->writeAttribute(QStringLiteral("offset"), QString::number(d->labelsOffset));
	writer->writeAttribute(QStringLiteral("rotation"), QString::number(d->labelsRotationAngle));
	writer->writeAttribute(QStringLiteral("textType"), QString::number(static_cast<int>(d->labelsTextType)));
	WRITE_COLUMN(d->labelsTextColumn, labelsTextColumn);
	writer->writeAttribute(QStringLiteral("format"), QString::number(static_cast<int>(d->labelsFormat)));
	writer->writeAttribute(QStringLiteral("formatAuto"), QString::number(static_cast<int>(d->labelsFormatAuto)));
	writer->writeAttribute(QStringLiteral("precision"), QString::number(d->labelsPrecision));
	writer->writeAttribute(QStringLiteral("autoPrecision"), QString::number(d->labelsAutoPrecision));
	writer->writeAttribute(QStringLiteral("dateTimeFormat"), d->labelsDateTimeFormat);
	WRITE_QCOLOR(d->labelsColor);
	WRITE_QFONT(d->labelsFont);
	writer->writeAttribute(QStringLiteral("prefix"), d->labelsPrefix);
	writer->writeAttribute(QStringLiteral("suffix"), d->labelsSuffix);
	writer->writeAttribute(QStringLiteral("opacity"), QString::number(d->labelsOpacity));
	writer->writeAttribute(QStringLiteral("backgroundType"), QString::number(static_cast<int>(d->labelsBackgroundType)));
	writer->writeAttribute(QStringLiteral("backgroundColor_r"), QString::number(d->labelsBackgroundColor.red()));
	writer->writeAttribute(QStringLiteral("backgroundColor_g"), QString::number(d->labelsBackgroundColor.green()));
	writer->writeAttribute(QStringLiteral("backgroundColor_b"), QString::number(d->labelsBackgroundColor.blue()));
	writer->writeEndElement();

	// grid
	d->majorGridLine->save(writer);
	d->minorGridLine->save(writer);

	writer->writeEndElement(); // close "axis" section
}

//! Load from XML
bool Axis::load(XmlStreamReader* reader, bool preview) {
	Q_D(Axis);

	if (!readBasicAttributes(reader))
		return false;

	QXmlStreamAttributes attribs;
	QString str;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == QLatin1String("axis"))
			break;

		if (!reader->isStartElement())
			continue;

		if (!preview && reader->name() == QLatin1String("comment")) {
			if (!readCommentElement(reader))
				return false;
		} else if (!preview && reader->name() == QLatin1String("general")) {
			attribs = reader->attributes();
			if (Project::xmlVersion() < 5) {
				bool autoScale = attribs.value(QStringLiteral("autoScale")).toInt();
				if (autoScale)
					d->rangeType = Axis::RangeType::Auto;
				else
					d->rangeType = Axis::RangeType::Custom;
			} else
				READ_INT_VALUE("rangeType", rangeType, Axis::RangeType);

			READ_INT_VALUE("orientation", orientation, Orientation);
			READ_INT_VALUE("position", position, Axis::Position);
			// scale
			str = attribs.value(QStringLiteral("scale")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("scale"));
			else
				d->scale = static_cast<RangeT::Scale>(str.toInt());

			str = attribs.value(QStringLiteral("rangeScale")).toString();
			if (str.isEmpty())
				d->rangeScale = false; // backward compatibility
			else
				d->rangeScale = static_cast<bool>(str.toInt());

			str = attribs.value(QStringLiteral("scaleRange")).toString();
			if (str.isEmpty())
				d->range.scale() = d->scale; // backward compatibility
			else
				d->rangeScale = static_cast<bool>(str.toInt());

			READ_DOUBLE_VALUE("offset", offset);
			READ_DOUBLE_VALUE("logicalPosition", logicalPosition);
			READ_DOUBLE_VALUE("start", range.start());
			READ_DOUBLE_VALUE("end", range.end());
			READ_INT_VALUE("majorTicksStartType", majorTicksStartType, TicksStartType);
			READ_DOUBLE_VALUE("majorTickStartOffset", majorTickStartOffset);
			READ_DOUBLE_VALUE("majorTickStartValue", majorTickStartValue);
			READ_DOUBLE_VALUE("scalingFactor", scalingFactor);
			READ_DOUBLE_VALUE("zeroOffset", zeroOffset);
			READ_INT_VALUE("showScaleOffset", showScaleOffset, bool);
			READ_DOUBLE_VALUE("titleOffsetX", titleOffsetX);
			READ_DOUBLE_VALUE("titleOffsetY", titleOffsetY);
			READ_INT_VALUE_DIRECT("plotRangeIndex", m_cSystemIndex, int);

			if (Project::xmlVersion() < 2) {
				// earlier, offset was only used when the enum  value Custom was used.
				// after the positioning rework, it is possible to specify the offset for
				// all other positions like Left, Right, etc.
				// Also, Custom was renamed to Logical and d->logicalPosition is used now.
				// Adjust the values from older projects.
				if (d->position == Axis::Position::Logical)
					d->logicalPosition = d->offset;
				else
					d->offset = 0.0;
			}

			str = attribs.value(QStringLiteral("visible")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("visible"));
			else
				d->setVisible(str.toInt());
		} else if (reader->name() == QLatin1String("textLabel")) {
			d->title->load(reader, preview);
		} else if (!preview && reader->name() == QLatin1String("line")) {
			attribs = reader->attributes();
			d->line->load(reader, preview);
			READ_INT_VALUE("arrowType", arrowType, Axis::ArrowType);
			READ_INT_VALUE("arrowPosition", arrowPosition, Axis::ArrowPosition);
			READ_DOUBLE_VALUE("arrowSize", arrowSize);
		} else if (!preview && reader->name() == QLatin1String("majorTicks")) {
			attribs = reader->attributes();

			READ_INT_VALUE("direction", majorTicksDirection, Axis::TicksDirection);
			READ_INT_VALUE("type", majorTicksType, Axis::TicksType);
			READ_INT_VALUE("numberAuto", majorTicksAutoNumber, bool);
			READ_INT_VALUE("number", majorTicksNumber, int);
			READ_DOUBLE_VALUE("increment", majorTicksSpacing);
			READ_COLUMN(majorTicksColumn);
			READ_DOUBLE_VALUE("length", majorTicksLength);
			d->majorTicksLine->load(reader, preview);
		} else if (!preview && reader->name() == QLatin1String("minorTicks")) {
			attribs = reader->attributes();

			READ_INT_VALUE("direction", minorTicksDirection, Axis::TicksDirection);
			READ_INT_VALUE("type", minorTicksType, Axis::TicksType);
			READ_INT_VALUE("numberAuto", minorTicksAutoNumber, bool);
			READ_INT_VALUE("number", minorTicksNumber, int);
			READ_DOUBLE_VALUE("increment", minorTicksIncrement);
			READ_COLUMN(minorTicksColumn);
			READ_DOUBLE_VALUE("length", minorTicksLength);
			d->minorTicksLine->load(reader, preview);
		} else if (!preview && reader->name() == QLatin1String("labels")) {
			attribs = reader->attributes();

			READ_INT_VALUE("position", labelsPosition, Axis::LabelsPosition);
			READ_DOUBLE_VALUE("offset", labelsOffset);
			READ_DOUBLE_VALUE("rotation", labelsRotationAngle);
			READ_INT_VALUE("textType", labelsTextType, Axis::LabelsTextType);
			READ_COLUMN(labelsTextColumn);
			READ_INT_VALUE("format", labelsFormat, Axis::LabelsFormat);
			READ_INT_VALUE("formatAuto", labelsFormatAuto, bool);
			READ_INT_VALUE("precision", labelsPrecision, int);
			READ_INT_VALUE("autoPrecision", labelsAutoPrecision, bool);
			d->labelsDateTimeFormat = attribs.value(QStringLiteral("dateTimeFormat")).toString();
			READ_QCOLOR(d->labelsColor);
			READ_QFONT(d->labelsFont);

			// don't produce any warning if no prefix or suffix is set (empty string is allowed here in xml)
			d->labelsPrefix = attribs.value(QStringLiteral("prefix")).toString();
			d->labelsSuffix = attribs.value(QStringLiteral("suffix")).toString();

			READ_DOUBLE_VALUE("opacity", labelsOpacity);

			READ_INT_VALUE("backgroundType", labelsBackgroundType, Axis::LabelsBackgroundType);
			str = attribs.value(QStringLiteral("backgroundColor_r")).toString();
			if (!str.isEmpty())
				d->labelsBackgroundColor.setRed(str.toInt());

			str = attribs.value(QStringLiteral("backgroundColor_g")).toString();
			if (!str.isEmpty())
				d->labelsBackgroundColor.setGreen(str.toInt());

			str = attribs.value(QStringLiteral("backgroundColor_b")).toString();
			if (!str.isEmpty())
				d->labelsBackgroundColor.setBlue(str.toInt());
		} else if (!preview && reader->name() == QLatin1String("majorGrid")) {
			d->majorGridLine->load(reader, preview);
		} else if (!preview && reader->name() == QLatin1String("minorGrid")) {
			d->minorGridLine->load(reader, preview);
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
void Axis::loadThemeConfig(const KConfig& config) {
	Q_D(Axis);
	const KConfigGroup& group = config.group(QStringLiteral("Axis"));

	// we don't want to show the major and minor grid lines for non-first horizontal/vertical axes
	// determine the index of the axis among other axes having the same orientation
	bool firstAxis = true;
	for (const auto* axis : parentAspect()->children<Axis>()) {
		if (orientation() == axis->orientation()) {
			if (axis == this) {
				break;
			} else {
				firstAxis = false;
				break;
			}
		}
	}

	// Tick label
	this->setLabelsColor(group.readEntry(QStringLiteral("LabelsFontColor"), QColor(Qt::black)));
	this->setLabelsOpacity(group.readEntry(QStringLiteral("LabelsOpacity"), 1.0));

	// use plot area color for the background color of the labels
	const KConfigGroup& groupPlot = config.group(QStringLiteral("CartesianPlot"));
	this->setLabelsBackgroundColor(groupPlot.readEntry(QStringLiteral("BackgroundFirstColor"), QColor(Qt::white)));

	// Line
	d->line->setColor(group.readEntry(QStringLiteral("LineColor"), QColor(Qt::black)));
	d->line->setWidth(group.readEntry(QStringLiteral("LineWidth"), Worksheet::convertToSceneUnits(1.0, Worksheet::Unit::Point)));
	d->line->setOpacity(group.readEntry(QStringLiteral("LineOpacity"), 1.0));

	const auto* plot = static_cast<const CartesianPlot*>(parentAspect());
	if (firstAxis && plot->theme() == QLatin1String("Tufte")) {
		setRangeType(RangeType::AutoData);
		d->line->setStyle(Qt::SolidLine);
	} else {
		// switch back to "Auto" range type when "AutoData" was selected (either because of Tufte or manually selected),
		// don't do anything if "Custom" is selected
		if (rangeType() == RangeType::AutoData)
			setRangeType(RangeType::Auto);

		d->line->setStyle((Qt::PenStyle)group.readEntry(QStringLiteral("LineStyle"), (int)Qt::SolidLine));
	}

	// Title
	if (plot->theme() == QLatin1String("Sparkline"))
		d->title->setText(QString());

	// Major grid
	if (firstAxis)
		d->majorGridLine->setStyle((Qt::PenStyle)group.readEntry(QStringLiteral("MajorGridStyle"), (int)Qt::SolidLine));
	else
		d->majorGridLine->setStyle(Qt::NoPen);

	d->majorGridLine->setColor(group.readEntry(QStringLiteral("MajorGridColor"), QColor(Qt::gray)));
	d->majorGridLine->setWidth(group.readEntry(QStringLiteral("MajorGridWidth"), Worksheet::convertToSceneUnits(1.0, Worksheet::Unit::Point)));
	d->majorGridLine->setOpacity(group.readEntry(QStringLiteral("MajorGridOpacity"), 1.0));

	// Major ticks
	this->setMajorTicksDirection((Axis::TicksDirection)group.readEntry(QStringLiteral("MajorTicksDirection"), (int)Axis::ticksIn));
	this->setMajorTicksLength(group.readEntry(QStringLiteral("MajorTicksLength"), Worksheet::convertToSceneUnits(6.0, Worksheet::Unit::Point)));
	d->majorTicksLine->loadThemeConfig(group);

	// Minor grid
	if (firstAxis)
		d->minorGridLine->setStyle((Qt::PenStyle)group.readEntry(QStringLiteral("MinorGridStyle"), (int)Qt::DotLine));
	else
		d->minorGridLine->setStyle(Qt::NoPen);

	d->minorGridLine->setColor(group.readEntry(QStringLiteral("MinorGridColor"), QColor(Qt::gray)));
	d->minorGridLine->setWidth(group.readEntry(QStringLiteral("MinorGridWidth"), Worksheet::convertToSceneUnits(1.0, Worksheet::Unit::Point)));
	d->minorGridLine->setOpacity(group.readEntry(QStringLiteral("MinorGridOpacity"), 1.0));

	// Minor ticks
	this->setMinorTicksDirection((Axis::TicksDirection)group.readEntry(QStringLiteral("MinorTicksDirection"), (int)Axis::ticksIn));
	this->setMinorTicksLength(group.readEntry(QStringLiteral("MinorTicksLength"), Worksheet::convertToSceneUnits(3.0, Worksheet::Unit::Point)));
	d->minorTicksLine->loadThemeConfig(group);

	// load the theme for the title label
	d->title->loadThemeConfig(config);
}

void Axis::saveThemeConfig(const KConfig& config) {
	Q_D(Axis);
	KConfigGroup group = config.group(QStringLiteral("Axis"));

	// Tick label
	group.writeEntry(QStringLiteral("LabelsFontColor"), this->labelsColor());
	group.writeEntry(QStringLiteral("LabelsOpacity"), this->labelsOpacity());
	group.writeEntry(QStringLiteral("LabelsBackgroundColor"), this->labelsBackgroundColor());

	// Line
	d->line->saveThemeConfig(group);

	// Major ticks
	group.writeEntry(QStringLiteral("MajorTicksType"), (int)this->majorTicksType());
	group.writeEntry(QStringLiteral("MajorTicksLength"), d->majorTicksLength);
	d->majorTicksLine->saveThemeConfig(group);

	// Minor ticks
	group.writeEntry(QStringLiteral("MinorTicksType"), (int)this->minorTicksType());
	group.writeEntry(QStringLiteral("MinorTicksLength"), d->majorTicksLength);
	d->minorTicksLine->saveThemeConfig(group);

	// grid
	d->majorGridLine->saveThemeConfig(group);
	d->minorGridLine->saveThemeConfig(group);

	// title labe
	d->title->saveThemeConfig(config);
}
