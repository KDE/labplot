/***************************************************************************
    File                 : Axis.cpp
    Project              : LabPlot
    Description          : Axis for cartesian coordinate systems.
    --------------------------------------------------------------------
    Copyright            : (C) 2011-2018 Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2013-2020 Stefan Gerlach  (stefan.gerlach@uni.kn)

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/

#include "AxisPrivate.h"
#include "CartesianCoordinateSystem.h"
#include "CartesianPlot.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/TextLabel.h"
#include "backend/core/AbstractColumn.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/macros.h"
// #include "backend/lib/trace.h"
#include "kdefrontend/GuiTools.h"

#include <KConfig>
#include <KLocalizedString>

#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QPainter>
#include <QTextDocument>

extern "C" {
#include <gsl/gsl_math.h>
#include "backend/nsl/nsl_math.h"
}

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
		gridShape.addPath(WorksheetElement::shapeFromPath(axis->majorGridPath, axis->majorGridPen));
		gridShape.addPath(WorksheetElement::shapeFromPath(axis->minorGridPath, axis->minorGridPen));
		QRectF boundingRectangle = gridShape.boundingRect();
		return boundingRectangle;
	}

	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override {
		Q_UNUSED(option)
		Q_UNUSED(widget)

		if (!axis->isVisible()) return;
		if (axis->linePath.isEmpty()) return;

		//draw major grid
		if (axis->majorGridPen.style() != Qt::NoPen) {
			painter->setOpacity(axis->majorGridOpacity);
			painter->setPen(axis->majorGridPen);
			painter->setBrush(Qt::NoBrush);
			painter->drawPath(axis->majorGridPath);
		}

		//draw minor grid
		if (axis->minorGridPen.style() != Qt::NoPen) {
			painter->setOpacity(axis->minorGridOpacity);
			painter->setPen(axis->minorGridPen);
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
Axis::Axis(const QString& name, Orientation orientation)
		: WorksheetElement(name, AspectType::Axis), d_ptr(new AxisPrivate(this)) {
	d_ptr->orientation = orientation;
	init();
}

Axis::Axis(const QString& name, Orientation orientation, AxisPrivate* dd)
		: WorksheetElement(name, AspectType::Axis), d_ptr(dd) {
	d_ptr->orientation = orientation;
	init();
}

void Axis::finalizeAdd() {
	Q_D(Axis);
	d->plot = dynamic_cast<CartesianPlot*>(parentAspect());
	Q_ASSERT(d->plot);
	//TODO
	d->cSystem = dynamic_cast<const CartesianCoordinateSystem*>(d->plot->coordinateSystem(0));
}

void Axis::init() {
	Q_D(Axis);

	KConfig config;
	KConfigGroup group = config.group("Axis");

	d->autoScale = true;
	d->position = Axis::Position::Custom;
	d->offset = group.readEntry("PositionOffset", 0);
	d->scale = (Scale) group.readEntry("Scale", static_cast<int>(Scale::Linear));
	d->autoScale = group.readEntry("AutoScale", true);
	d->range = Range<double>(group.readEntry("Start", 0), group.readEntry("End", 10));
	d->zeroOffset = group.readEntry("ZeroOffset", 0);
	d->scalingFactor = group.readEntry("ScalingFactor", 1.0);

	d->linePen.setStyle( (Qt::PenStyle) group.readEntry("LineStyle", (int) Qt::SolidLine) );
	d->linePen.setWidthF( group.readEntry("LineWidth", Worksheet::convertToSceneUnits( 1.0, Worksheet::Unit::Point) ) );
	d->lineOpacity = group.readEntry("LineOpacity", 1.0);
	d->arrowType = (Axis::ArrowType) group.readEntry("ArrowType", static_cast<int>(ArrowType::NoArrow));
	d->arrowPosition = (Axis::ArrowPosition) group.readEntry("ArrowPosition", static_cast<int>(ArrowPosition::Right));
	d->arrowSize = group.readEntry("ArrowSize", Worksheet::convertToSceneUnits(10, Worksheet::Unit::Point));

	// axis title
 	d->title = new TextLabel(this->name(), TextLabel::Type::AxisTitle);
	connect(d->title, &TextLabel::changed, this, &Axis::labelChanged);
	addChild(d->title);
	d->title->setHidden(true);
	d->title->graphicsItem()->setParentItem(d);
	d->title->graphicsItem()->setFlag(QGraphicsItem::ItemIsMovable, false);
	d->title->graphicsItem()->setFlag(QGraphicsItem::ItemIsFocusable, false);
	d->title->graphicsItem()->setAcceptHoverEvents(false);
	d->title->setText(this->name());
	if (d->orientation == Orientation::Vertical)
		d->title->setRotationAngle(90);
	d->titleOffsetX = Worksheet::convertToSceneUnits(2, Worksheet::Unit::Point); //distance to the axis tick labels
	d->titleOffsetY = Worksheet::convertToSceneUnits(2, Worksheet::Unit::Point); //distance to the axis tick labels

	d->majorTicksDirection = (Axis::TicksDirection) group.readEntry("MajorTicksDirection", (int) Axis::ticksOut);
	d->majorTicksType = (TicksType) group.readEntry("MajorTicksType", static_cast<int>(TicksType::TotalNumber));
	d->majorTicksNumber = group.readEntry("MajorTicksNumber", 11);
	d->majorTicksSpacing = group.readEntry("MajorTicksIncrement", 0.0); // set to 0, so axisdock determines the value to not have to many labels the first time switched to Spacing

	d->majorTicksPen.setStyle((Qt::PenStyle) group.readEntry("MajorTicksLineStyle", (int)Qt::SolidLine) );
	d->majorTicksPen.setColor( group.readEntry("MajorTicksColor", QColor(Qt::black) ) );
	d->majorTicksPen.setWidthF( group.readEntry("MajorTicksWidth", Worksheet::convertToSceneUnits(1.0, Worksheet::Unit::Point) ) );
	d->majorTicksLength = group.readEntry("MajorTicksLength", Worksheet::convertToSceneUnits(6.0, Worksheet::Unit::Point));
	d->majorTicksOpacity = group.readEntry("MajorTicksOpacity", 1.0);

	d->minorTicksDirection = (TicksDirection) group.readEntry("MinorTicksDirection", (int) Axis::ticksOut);
	d->minorTicksType = (TicksType) group.readEntry("MinorTicksType", static_cast<int>(TicksType::TotalNumber));
	d->minorTicksNumber = group.readEntry("MinorTicksNumber", 1);
	d->minorTicksIncrement = group.readEntry("MinorTicksIncrement", 0.0);	// see MajorTicksIncrement
	d->minorTicksPen.setStyle((Qt::PenStyle) group.readEntry("MinorTicksLineStyle", (int)Qt::SolidLine) );
	d->minorTicksPen.setColor( group.readEntry("MinorTicksColor", QColor(Qt::black) ) );
	d->minorTicksPen.setWidthF( group.readEntry("MinorTicksWidth", Worksheet::convertToSceneUnits(1.0, Worksheet::Unit::Point) ) );
	d->minorTicksLength = group.readEntry("MinorTicksLength", Worksheet::convertToSceneUnits(3.0, Worksheet::Unit::Point));
	d->minorTicksOpacity = group.readEntry("MinorTicksOpacity", 1.0);

	//Labels
	d->labelsFormat = (LabelsFormat) group.readEntry("LabelsFormat", static_cast<int>(LabelsFormat::Decimal));
	d->labelsAutoPrecision = group.readEntry("LabelsAutoPrecision", true);
	d->labelsPrecision = group.readEntry("LabelsPrecision", 1);
	d->labelsDateTimeFormat = group.readEntry("LabelsDateTimeFormat", "yyyy-MM-dd hh:mm:ss");
	d->labelsPosition = (LabelsPosition) group.readEntry("LabelsPosition", (int) LabelsPosition::Out);
	d->labelsOffset = group.readEntry("LabelsOffset",  Worksheet::convertToSceneUnits( 5.0, Worksheet::Unit::Point ));
	d->labelsRotationAngle = group.readEntry("LabelsRotation", 0);
	d->labelsFont = group.readEntry("LabelsFont", QFont());
	d->labelsFont.setPixelSize( Worksheet::convertToSceneUnits( 10.0, Worksheet::Unit::Point ) );
	d->labelsColor = group.readEntry("LabelsFontColor", QColor(Qt::black));
	d->labelsBackgroundType = (LabelsBackgroundType) group.readEntry("LabelsBackgroundType", static_cast<int>(LabelsBackgroundType::Transparent));
	d->labelsBackgroundColor = group.readEntry("LabelsBackgroundColor", QColor(Qt::white));
	d->labelsPrefix =  group.readEntry("LabelsPrefix", "");
	d->labelsSuffix =  group.readEntry("LabelsSuffix", "");
	d->labelsOpacity = group.readEntry("LabelsOpacity", 1.0);

	//major grid
	d->majorGridPen.setStyle( (Qt::PenStyle) group.readEntry("MajorGridStyle", (int) Qt::SolidLine) );
	d->majorGridPen.setColor(group.readEntry("MajorGridColor", QColor(Qt::gray)) );
	d->majorGridPen.setWidthF( group.readEntry("MajorGridWidth", Worksheet::convertToSceneUnits( 1.0, Worksheet::Unit::Point ) ) );
	d->majorGridOpacity = group.readEntry("MajorGridOpacity", 1.0);

	//minor grid
	d->minorGridPen.setStyle( (Qt::PenStyle) group.readEntry("MinorGridStyle", (int) Qt::DotLine) );
	d->minorGridPen.setColor(group.readEntry("MinorGridColor", QColor(Qt::gray)) );
	d->minorGridPen.setWidthF( group.readEntry("MinorGridWidth", Worksheet::convertToSceneUnits( 1.0, Worksheet::Unit::Point ) ) );
	d->minorGridOpacity = group.readEntry("MinorGridOpacity", 1.0);
}

/*!
 * For the most frequently edited properties, create Actions and ActionGroups for the context menu.
 * For some ActionGroups the actual actions are created in \c GuiTool,
 */
void Axis::initActions() {
	visibilityAction = new QAction(QIcon::fromTheme("view-visible"), i18n("Visible"), this);
	visibilityAction->setCheckable(true);
	connect(visibilityAction, &QAction::triggered, this, &Axis::visibilityChangedSlot);

	//Orientation
	orientationActionGroup = new QActionGroup(this);
	orientationActionGroup->setExclusive(true);
	connect(orientationActionGroup, &QActionGroup::triggered, this, &Axis::orientationChangedSlot);

	orientationHorizontalAction = new QAction(QIcon::fromTheme("labplot-axis-horizontal"), i18n("Horizontal"), orientationActionGroup);
	orientationHorizontalAction->setCheckable(true);

	orientationVerticalAction = new QAction(QIcon::fromTheme("labplot-axis-vertical"), i18n("Vertical"), orientationActionGroup);
	orientationVerticalAction->setCheckable(true);

	//Line
	lineStyleActionGroup = new QActionGroup(this);
	lineStyleActionGroup->setExclusive(true);
	connect(lineStyleActionGroup, &QActionGroup::triggered, this, &Axis::lineStyleChanged);

	lineColorActionGroup = new QActionGroup(this);
	lineColorActionGroup->setExclusive(true);
	connect(lineColorActionGroup, &QActionGroup::triggered, this, &Axis::lineColorChanged);

	//Ticks
	//TODO
}

void Axis::initMenus() {
	this->initActions();

	//Orientation
	orientationMenu = new QMenu(i18n("Orientation"));
	orientationMenu->setIcon(QIcon::fromTheme("labplot-axis-horizontal"));
	orientationMenu->addAction(orientationHorizontalAction);
	orientationMenu->addAction(orientationVerticalAction);

	//Line
	lineMenu = new QMenu(i18n("Line"));
	lineMenu->setIcon(QIcon::fromTheme("draw-line"));
	lineStyleMenu = new QMenu(i18n("Style"), lineMenu);
	lineStyleMenu->setIcon(QIcon::fromTheme("object-stroke-style"));
	lineMenu->setIcon(QIcon::fromTheme("draw-line"));
	lineMenu->addMenu( lineStyleMenu );

	lineColorMenu = new QMenu(i18n("Color"), lineMenu);
	lineColorMenu->setIcon(QIcon::fromTheme("fill-color"));
	GuiTools::fillColorMenu( lineColorMenu, lineColorActionGroup );
	lineMenu->addMenu( lineColorMenu );
}

QMenu* Axis::createContextMenu() {
	if (!orientationMenu)
		initMenus();

	Q_D(const Axis);
	QMenu* menu = WorksheetElement::createContextMenu();
	QAction* firstAction = menu->actions().at(1); //skip the first action because of the "title-action"

	visibilityAction->setChecked(isVisible());
	menu->insertAction(firstAction, visibilityAction);

	//Orientation
	if (d->orientation == Orientation::Horizontal)
		orientationHorizontalAction->setChecked(true);
	else
		orientationVerticalAction->setChecked(true);

	menu->insertMenu(firstAction, orientationMenu);

	//Line styles
	GuiTools::updatePenStyles( lineStyleMenu, lineStyleActionGroup, d->linePen.color() );
	GuiTools::selectPenStyleAction(lineStyleActionGroup, d->linePen.style() );

	GuiTools::selectColorAction(lineColorActionGroup, d->linePen.color() );

	menu->insertMenu(firstAction, lineMenu);
	menu->insertSeparator(firstAction);

	return menu;
}

/*!
	Returns an icon to be used in the project explorer.
*/
QIcon Axis::icon() const{
	Q_D(const Axis);
	QIcon icon;
	if (d->orientation == Orientation::Horizontal)
		icon = QIcon::fromTheme("labplot-axis-horizontal");
	else
		icon = QIcon::fromTheme("labplot-axis-vertical");

	return icon;
}

Axis::~Axis() {
	if (orientationMenu) {
		delete orientationMenu;
		delete lineMenu;
	}

	//no need to delete d->title, since it was added with addChild in init();

	//no need to delete the d-pointer here - it inherits from QGraphicsItem
	//and is deleted during the cleanup in QGraphicsScene
}

QGraphicsItem *Axis::graphicsItem() const {
	return d_ptr;
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
	Q_UNUSED(pageResize);

	double ratio = 0;
	if (horizontalRatio > 1.0 || verticalRatio > 1.0)
		ratio = qMax(horizontalRatio, verticalRatio);
	else
		ratio = qMin(horizontalRatio, verticalRatio);

	QPen pen = d->linePen;
	pen.setWidthF(pen.widthF() * ratio);
	d->linePen = pen;

	d->majorTicksLength *= ratio; // ticks are perpendicular to axis line -> verticalRatio relevant
	d->minorTicksLength *= ratio;
	d->labelsFont.setPixelSize( d->labelsFont.pixelSize() * ratio ); //TODO: take into account rotated labels
	d->labelsOffset *= ratio;
	d->title->handleResize(horizontalRatio, verticalRatio, pageResize);
}

/* ============================ getter methods ================= */
BASIC_SHARED_D_READER_IMPL(Axis, bool, autoScale, autoScale)
BASIC_SHARED_D_READER_IMPL(Axis, Axis::Orientation, orientation, orientation)
BASIC_SHARED_D_READER_IMPL(Axis, Axis::Position, position, position)
BASIC_SHARED_D_READER_IMPL(Axis, Axis::Scale, scale, scale)
BASIC_SHARED_D_READER_IMPL(Axis, double, offset, offset)
BASIC_SHARED_D_READER_IMPL(Axis, Range<double>, range, range)
BASIC_SHARED_D_READER_IMPL(Axis, qreal, scalingFactor, scalingFactor)
BASIC_SHARED_D_READER_IMPL(Axis, qreal, zeroOffset, zeroOffset)

BASIC_SHARED_D_READER_IMPL(Axis, TextLabel*, title, title)
BASIC_SHARED_D_READER_IMPL(Axis, qreal, titleOffsetX, titleOffsetX)
BASIC_SHARED_D_READER_IMPL(Axis, qreal, titleOffsetY, titleOffsetY)

CLASS_SHARED_D_READER_IMPL(Axis, QPen, linePen, linePen)
BASIC_SHARED_D_READER_IMPL(Axis, qreal, lineOpacity, lineOpacity)
BASIC_SHARED_D_READER_IMPL(Axis, Axis::ArrowType, arrowType, arrowType)
BASIC_SHARED_D_READER_IMPL(Axis, Axis::ArrowPosition, arrowPosition, arrowPosition)
BASIC_SHARED_D_READER_IMPL(Axis, qreal, arrowSize, arrowSize)

BASIC_SHARED_D_READER_IMPL(Axis, Axis::TicksDirection, majorTicksDirection, majorTicksDirection)
BASIC_SHARED_D_READER_IMPL(Axis, Axis::TicksType, majorTicksType, majorTicksType)
BASIC_SHARED_D_READER_IMPL(Axis, int, majorTicksNumber, majorTicksNumber)
BASIC_SHARED_D_READER_IMPL(Axis, qreal, majorTicksSpacing, majorTicksSpacing)
BASIC_SHARED_D_READER_IMPL(Axis, const AbstractColumn*, majorTicksColumn, majorTicksColumn)
QString& Axis::majorTicksColumnPath() const { return d_ptr->majorTicksColumnPath; }
BASIC_SHARED_D_READER_IMPL(Axis, qreal, majorTicksLength, majorTicksLength)
CLASS_SHARED_D_READER_IMPL(Axis, QPen, majorTicksPen, majorTicksPen)
BASIC_SHARED_D_READER_IMPL(Axis, qreal, majorTicksOpacity, majorTicksOpacity)

BASIC_SHARED_D_READER_IMPL(Axis, Axis::TicksDirection, minorTicksDirection, minorTicksDirection)
BASIC_SHARED_D_READER_IMPL(Axis, Axis::TicksType, minorTicksType, minorTicksType)
BASIC_SHARED_D_READER_IMPL(Axis, int, minorTicksNumber, minorTicksNumber)
BASIC_SHARED_D_READER_IMPL(Axis, qreal, minorTicksSpacing, minorTicksIncrement)
BASIC_SHARED_D_READER_IMPL(Axis, const AbstractColumn*, minorTicksColumn, minorTicksColumn)
QString& Axis::minorTicksColumnPath() const { return d_ptr->minorTicksColumnPath; }
BASIC_SHARED_D_READER_IMPL(Axis, qreal, minorTicksLength, minorTicksLength)
CLASS_SHARED_D_READER_IMPL(Axis, QPen, minorTicksPen, minorTicksPen)
BASIC_SHARED_D_READER_IMPL(Axis, qreal, minorTicksOpacity, minorTicksOpacity)

BASIC_SHARED_D_READER_IMPL(Axis, Axis::LabelsFormat, labelsFormat, labelsFormat);
BASIC_SHARED_D_READER_IMPL(Axis, bool, labelsAutoPrecision, labelsAutoPrecision);
BASIC_SHARED_D_READER_IMPL(Axis, int, labelsPrecision, labelsPrecision);
BASIC_SHARED_D_READER_IMPL(Axis, QString, labelsDateTimeFormat, labelsDateTimeFormat);
BASIC_SHARED_D_READER_IMPL(Axis, Axis::LabelsPosition, labelsPosition, labelsPosition);
BASIC_SHARED_D_READER_IMPL(Axis, qreal, labelsOffset, labelsOffset);
BASIC_SHARED_D_READER_IMPL(Axis, qreal, labelsRotationAngle, labelsRotationAngle);
CLASS_SHARED_D_READER_IMPL(Axis, QColor, labelsColor, labelsColor);
CLASS_SHARED_D_READER_IMPL(Axis, QFont, labelsFont, labelsFont);
BASIC_SHARED_D_READER_IMPL(Axis, Axis::LabelsBackgroundType, labelsBackgroundType, labelsBackgroundType);
CLASS_SHARED_D_READER_IMPL(Axis, QColor, labelsBackgroundColor, labelsBackgroundColor);
CLASS_SHARED_D_READER_IMPL(Axis, QString, labelsPrefix, labelsPrefix);
CLASS_SHARED_D_READER_IMPL(Axis, QString, labelsSuffix, labelsSuffix);
BASIC_SHARED_D_READER_IMPL(Axis, qreal, labelsOpacity, labelsOpacity);

CLASS_SHARED_D_READER_IMPL(Axis, QPen, majorGridPen, majorGridPen)
BASIC_SHARED_D_READER_IMPL(Axis, qreal, majorGridOpacity, majorGridOpacity)
CLASS_SHARED_D_READER_IMPL(Axis, QPen, minorGridPen, minorGridPen)
BASIC_SHARED_D_READER_IMPL(Axis, qreal, minorGridOpacity, minorGridOpacity)

/* ============================ setter methods and undo commands ================= */
STD_SETTER_CMD_IMPL_F_S(Axis, SetAutoScale, bool, autoScale, retransform);
void Axis::setAutoScale(bool autoScale) {
	Q_D(Axis);
	if (autoScale != d->autoScale) {
		exec(new AxisSetAutoScaleCmd(d, autoScale, ki18n("%1: set axis auto scaling")));

		if (autoScale) {
			auto* plot = qobject_cast<CartesianPlot*>(parentAspect());
			if (!plot)
				return;

			if (d->orientation == Axis::Orientation::Horizontal)
				d->range = plot->xRange(d->cSystem->xIndex());
			else
				d->range =  plot->yRange();

			retransform();
			emit rangeChanged(d->range);
		}
	}
}

STD_SWAP_METHOD_SETTER_CMD_IMPL(Axis, SetVisible, bool, swapVisible);
void Axis::setVisible(bool on) {
	Q_D(Axis);
	exec(new AxisSetVisibleCmd(d, on, on ? ki18n("%1: set visible") : ki18n("%1: set invisible")));
}

bool Axis::isVisible() const {
	Q_D(const Axis);
	return d->isVisible();
}

void Axis::setDefault(bool value) {
	Q_D(Axis);
	d->isDefault = value;
}

bool Axis::isDefault() const {
	Q_D(const Axis);
	return d->isDefault;
}

void Axis::setPrinting(bool on) {
	Q_D(Axis);
	d->setPrinting(on);
}

bool Axis::isHovered() const {
	Q_D(const Axis);
	return d->isHovered();
}

STD_SETTER_CMD_IMPL_F_S(Axis, SetOrientation, Axis::Orientation, orientation, retransform);
void Axis::setOrientation(Orientation orientation) {
	Q_D(Axis);
	if (orientation != d->orientation)
		exec(new AxisSetOrientationCmd(d, orientation, ki18n("%1: set axis orientation")));
}

STD_SETTER_CMD_IMPL_F_S(Axis, SetPosition, Axis::Position, position, retransform);
void Axis::setPosition(Position position) {
	Q_D(Axis);
	if (position != d->position)
		exec(new AxisSetPositionCmd(d, position, ki18n("%1: set axis position")));
}

STD_SETTER_CMD_IMPL_F_S(Axis, SetScaling, Axis::Scale, scale, retransformTicks);
void Axis::setScale(Scale scale) {
	Q_D(Axis);
	if (scale != d->scale)
		exec(new AxisSetScalingCmd(d, scale, ki18n("%1: set axis scale")));
}

STD_SETTER_CMD_IMPL_F(Axis, SetOffset, double, offset, retransform);
void Axis::setOffset(double offset, bool undo) {
	Q_D(Axis);
	if (offset != d->offset) {
		if (undo) {
			exec(new AxisSetOffsetCmd(d, offset, ki18n("%1: set axis offset")));
		} else {
			d->offset = offset;
			//don't need to call retransform() afterward
			//since the only usage of this call is in CartesianPlot, where retransform is called for all children anyway.
		}
		emit positionChanged(offset);
	}
}

STD_SETTER_CMD_IMPL_F_S(Axis, SetRange, Range<double>, range, retransform);
void Axis::setRange(Range<double> range) {
	Q_D(Axis);
	if (range != d->range)
		exec(new AxisSetRangeCmd(d, range, ki18n("%1: set axis range")));
}
void Axis::setStart(double min) {
	Q_D(Axis);
	Range<double> range{min, d->range.end()};
	setRange(range);
}
void Axis::setEnd(double max) {
	Q_D(Axis);
	Range<double> range{d->range.start(), max};
	setRange(range);
}
void Axis::setRange(double min, double max) {
	Range<double> range{min, max};
	setRange(range);
}

STD_SETTER_CMD_IMPL_F_S(Axis, SetZeroOffset, qreal, zeroOffset, retransform);
void Axis::setZeroOffset(qreal zeroOffset) {
	Q_D(Axis);
	if (zeroOffset != d->zeroOffset)
		exec(new AxisSetZeroOffsetCmd(d, zeroOffset, ki18n("%1: set axis zero offset")));
}

STD_SETTER_CMD_IMPL_F_S(Axis, SetScalingFactor, qreal, scalingFactor, retransform);
void Axis::setScalingFactor(qreal scalingFactor) {
	Q_D(Axis);
	if (scalingFactor != d->scalingFactor)
		exec(new AxisSetScalingFactorCmd(d, scalingFactor, ki18n("%1: set axis scaling factor")));
}

//Title
STD_SETTER_CMD_IMPL_F_S(Axis, SetTitleOffsetX, qreal, titleOffsetX, retransform);
void Axis::setTitleOffsetX(qreal offset) {
	Q_D(Axis);
	if (offset != d->titleOffsetX)
		exec(new AxisSetTitleOffsetXCmd(d, offset, ki18n("%1: set title offset")));
}

STD_SETTER_CMD_IMPL_F_S(Axis, SetTitleOffsetY, qreal, titleOffsetY, retransform);
void Axis::setTitleOffsetY(qreal offset) {
	Q_D(Axis);
	if (offset != d->titleOffsetY)
		exec(new AxisSetTitleOffsetYCmd(d, offset, ki18n("%1: set title offset")));
}

//Line
STD_SETTER_CMD_IMPL_F_S(Axis, SetLinePen, QPen, linePen, recalcShapeAndBoundingRect);
void Axis::setLinePen(const QPen &pen) {
	Q_D(Axis);
	if (pen != d->linePen)
		exec(new AxisSetLinePenCmd(d, pen, ki18n("%1: set line style")));
}

STD_SETTER_CMD_IMPL_F_S(Axis, SetLineOpacity, qreal, lineOpacity, update);
void Axis::setLineOpacity(qreal opacity) {
	Q_D(Axis);
	if (opacity != d->lineOpacity)
		exec(new AxisSetLineOpacityCmd(d, opacity, ki18n("%1: set line opacity")));
}

STD_SETTER_CMD_IMPL_F_S(Axis, SetArrowType, Axis::ArrowType, arrowType, retransformArrow);
void Axis::setArrowType(ArrowType type) {
	Q_D(Axis);
	if (type != d->arrowType)
		exec(new AxisSetArrowTypeCmd(d, type, ki18n("%1: set arrow type")));
}

STD_SETTER_CMD_IMPL_F_S(Axis, SetArrowPosition, Axis::ArrowPosition, arrowPosition, retransformArrow);
void Axis::setArrowPosition(ArrowPosition position) {
	Q_D(Axis);
	if (position != d->arrowPosition)
		exec(new AxisSetArrowPositionCmd(d, position, ki18n("%1: set arrow position")));
}

STD_SETTER_CMD_IMPL_F_S(Axis, SetArrowSize, qreal, arrowSize, retransformArrow);
void Axis::setArrowSize(qreal arrowSize) {
	Q_D(Axis);
	if (arrowSize != d->arrowSize)
		exec(new AxisSetArrowSizeCmd(d, arrowSize, ki18n("%1: set arrow size")));
}

//Major ticks
STD_SETTER_CMD_IMPL_F_S(Axis, SetMajorTicksDirection, Axis::TicksDirection, majorTicksDirection, retransformTicks);
void Axis::setMajorTicksDirection(TicksDirection majorTicksDirection) {
	Q_D(Axis);
	if (majorTicksDirection != d->majorTicksDirection)
		exec(new AxisSetMajorTicksDirectionCmd(d, majorTicksDirection, ki18n("%1: set major ticks direction")));
}

STD_SETTER_CMD_IMPL_F_S(Axis, SetMajorTicksType, Axis::TicksType, majorTicksType, retransformTicks);
void Axis::setMajorTicksType(TicksType majorTicksType) {
	Q_D(Axis);
	if (majorTicksType!= d->majorTicksType)
		exec(new AxisSetMajorTicksTypeCmd(d, majorTicksType, ki18n("%1: set major ticks type")));
}
STD_SETTER_CMD_IMPL_F_S(Axis, SetMajorTicksNumber, int, majorTicksNumber, retransformTicks);
void Axis::setMajorTicksNumber(int majorTicksNumber) {
	Q_D(Axis);
	if (majorTicksNumber != d->majorTicksNumber)
		exec(new AxisSetMajorTicksNumberCmd(d, majorTicksNumber, ki18n("%1: set the total number of the major ticks")));
}

STD_SETTER_CMD_IMPL_F_S(Axis, SetMajorTicksSpacing, qreal, majorTicksSpacing, retransformTicks);
void Axis::setMajorTicksSpacing(qreal majorTicksSpacing) {
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
			connect(column->parentAspect(), &AbstractAspect::aspectAboutToBeRemoved,
					this, &Axis::majorTicksColumnAboutToBeRemoved);
			//TODO: add disconnect in the undo-function
		}
	}
}

STD_SETTER_CMD_IMPL_F_S(Axis, SetMajorTicksPen, QPen, majorTicksPen, recalcShapeAndBoundingRect);
void Axis::setMajorTicksPen(const QPen& pen) {
	Q_D(Axis);
	if (pen != d->majorTicksPen)
		exec(new AxisSetMajorTicksPenCmd(d, pen, ki18n("%1: set major ticks style")));
}

STD_SETTER_CMD_IMPL_F_S(Axis, SetMajorTicksLength, qreal, majorTicksLength, retransformTicks);
void Axis::setMajorTicksLength(qreal majorTicksLength) {
	Q_D(Axis);
	if (majorTicksLength != d->majorTicksLength)
		exec(new AxisSetMajorTicksLengthCmd(d, majorTicksLength, ki18n("%1: set major ticks length")));
}

STD_SETTER_CMD_IMPL_F_S(Axis, SetMajorTicksOpacity, qreal, majorTicksOpacity, update);
void Axis::setMajorTicksOpacity(qreal opacity) {
	Q_D(Axis);
	if (opacity != d->majorTicksOpacity)
		exec(new AxisSetMajorTicksOpacityCmd(d, opacity, ki18n("%1: set major ticks opacity")));
}

//Minor ticks
STD_SETTER_CMD_IMPL_F_S(Axis, SetMinorTicksDirection, Axis::TicksDirection, minorTicksDirection, retransformTicks);
void Axis::setMinorTicksDirection(TicksDirection minorTicksDirection) {
	Q_D(Axis);
	if (minorTicksDirection != d->minorTicksDirection)
		exec(new AxisSetMinorTicksDirectionCmd(d, minorTicksDirection, ki18n("%1: set minor ticks direction")));
}

STD_SETTER_CMD_IMPL_F_S(Axis, SetMinorTicksType, Axis::TicksType, minorTicksType, retransformTicks);
void Axis::setMinorTicksType(TicksType minorTicksType) {
	Q_D(Axis);
	if (minorTicksType!= d->minorTicksType)
		exec(new AxisSetMinorTicksTypeCmd(d, minorTicksType, ki18n("%1: set minor ticks type")));
}

STD_SETTER_CMD_IMPL_F_S(Axis, SetMinorTicksNumber, int, minorTicksNumber, retransformTicks);
void Axis::setMinorTicksNumber(int minorTicksNumber) {
	Q_D(Axis);
	if (minorTicksNumber != d->minorTicksNumber)
		exec(new AxisSetMinorTicksNumberCmd(d, minorTicksNumber, ki18n("%1: set the total number of the minor ticks")));
}

STD_SETTER_CMD_IMPL_F_S(Axis, SetMinorTicksSpacing, qreal, minorTicksIncrement, retransformTicks);
void Axis::setMinorTicksSpacing(qreal minorTicksSpacing) {
	Q_D(Axis);
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
			connect(column->parentAspect(), &AbstractAspect::aspectAboutToBeRemoved,
					this, &Axis::minorTicksColumnAboutToBeRemoved);
			//TODO: add disconnect in the undo-function
		}
	}
}

STD_SETTER_CMD_IMPL_F_S(Axis, SetMinorTicksPen, QPen, minorTicksPen, recalcShapeAndBoundingRect);
void Axis::setMinorTicksPen(const QPen& pen) {
	Q_D(Axis);
	if (pen != d->minorTicksPen)
		exec(new AxisSetMinorTicksPenCmd(d, pen, ki18n("%1: set minor ticks style")));
}

STD_SETTER_CMD_IMPL_F_S(Axis, SetMinorTicksLength, qreal, minorTicksLength, retransformTicks);
void Axis::setMinorTicksLength(qreal minorTicksLength) {
	Q_D(Axis);
	if (minorTicksLength != d->minorTicksLength)
		exec(new AxisSetMinorTicksLengthCmd(d, minorTicksLength, ki18n("%1: set minor ticks length")));
}

STD_SETTER_CMD_IMPL_F_S(Axis, SetMinorTicksOpacity, qreal, minorTicksOpacity, update);
void Axis::setMinorTicksOpacity(qreal opacity) {
	Q_D(Axis);
	if (opacity != d->minorTicksOpacity)
		exec(new AxisSetMinorTicksOpacityCmd(d, opacity, ki18n("%1: set minor ticks opacity")));
}

//Labels
STD_SETTER_CMD_IMPL_F_S(Axis, SetLabelsFormat, Axis::LabelsFormat, labelsFormat, retransformTicks);
void Axis::setLabelsFormat(LabelsFormat labelsFormat) {
	Q_D(Axis);
	if (labelsFormat != d->labelsFormat) {

		//TODO: this part is not undo/redo-aware
		if (labelsFormat == LabelsFormat::Decimal)
			d->labelsFormatDecimalOverruled = true;
		else
			d->labelsFormatDecimalOverruled = false;

		exec(new AxisSetLabelsFormatCmd(d, labelsFormat, ki18n("%1: set labels format")));
	}
}

STD_SETTER_CMD_IMPL_F_S(Axis, SetLabelsAutoPrecision, bool, labelsAutoPrecision, retransformTickLabelStrings);
void Axis::setLabelsAutoPrecision(bool labelsAutoPrecision) {
	Q_D(Axis);
	if (labelsAutoPrecision != d->labelsAutoPrecision)
		exec(new AxisSetLabelsAutoPrecisionCmd(d, labelsAutoPrecision, ki18n("%1: set labels precision")));
}

STD_SETTER_CMD_IMPL_F_S(Axis, SetLabelsPrecision, int, labelsPrecision, retransformTickLabelStrings);
void Axis::setLabelsPrecision(int labelsPrecision) {
	Q_D(Axis);
	if (labelsPrecision != d->labelsPrecision)
		exec(new AxisSetLabelsPrecisionCmd(d, labelsPrecision, ki18n("%1: set labels precision")));
}

STD_SETTER_CMD_IMPL_F_S(Axis, SetLabelsDateTimeFormat, QString, labelsDateTimeFormat, retransformTickLabelStrings);
void Axis::setLabelsDateTimeFormat(const QString& format) {
	Q_D(Axis);
	if (format != d->labelsDateTimeFormat)
		exec(new AxisSetLabelsDateTimeFormatCmd(d, format, ki18n("%1: set labels datetime format")));
}

STD_SETTER_CMD_IMPL_F_S(Axis, SetLabelsPosition, Axis::LabelsPosition, labelsPosition, retransformTickLabelPositions);
void Axis::setLabelsPosition(LabelsPosition labelsPosition) {
	Q_D(Axis);
	if (labelsPosition != d->labelsPosition)
		exec(new AxisSetLabelsPositionCmd(d, labelsPosition, ki18n("%1: set labels position")));
}

STD_SETTER_CMD_IMPL_F_S(Axis, SetLabelsOffset, double, labelsOffset, retransformTickLabelPositions);
void Axis::setLabelsOffset(double offset) {
	Q_D(Axis);
	if (offset != d->labelsOffset)
		exec(new AxisSetLabelsOffsetCmd(d, offset, ki18n("%1: set label offset")));
}

STD_SETTER_CMD_IMPL_F_S(Axis, SetLabelsRotationAngle, qreal, labelsRotationAngle, retransformTickLabelPositions);
void Axis::setLabelsRotationAngle(qreal angle) {
	Q_D(Axis);
	if (angle != d->labelsRotationAngle)
		exec(new AxisSetLabelsRotationAngleCmd(d, angle, ki18n("%1: set label rotation angle")));
}

STD_SETTER_CMD_IMPL_F_S(Axis, SetLabelsColor, QColor, labelsColor, update);
void Axis::setLabelsColor(const QColor& color) {
	Q_D(Axis);
	if (color != d->labelsColor)
		exec(new AxisSetLabelsColorCmd(d, color, ki18n("%1: set label color")));
}

STD_SETTER_CMD_IMPL_F_S(Axis, SetLabelsFont, QFont, labelsFont, retransformTickLabelStrings);
void Axis::setLabelsFont(const QFont& font) {
	Q_D(Axis);
	if (font != d->labelsFont)
		exec(new AxisSetLabelsFontCmd(d, font, ki18n("%1: set label font")));
}

STD_SETTER_CMD_IMPL_F_S(Axis, SetLabelsBackgroundType, Axis::LabelsBackgroundType, labelsBackgroundType, update);
void Axis::setLabelsBackgroundType(LabelsBackgroundType type) {
	Q_D(Axis);
	if (type != d->labelsBackgroundType)
		exec(new AxisSetLabelsBackgroundTypeCmd(d, type, ki18n("%1: set labels background type")));
}

STD_SETTER_CMD_IMPL_F_S(Axis, SetLabelsBackgroundColor, QColor, labelsBackgroundColor, update);
void Axis::setLabelsBackgroundColor(const QColor& color) {
	Q_D(Axis);
	if (color != d->labelsBackgroundColor)
		exec(new AxisSetLabelsBackgroundColorCmd(d, color, ki18n("%1: set label background color")));
}

STD_SETTER_CMD_IMPL_F_S(Axis, SetLabelsPrefix, QString, labelsPrefix, retransformTickLabelStrings);
void Axis::setLabelsPrefix(const QString& prefix) {
	Q_D(Axis);
	if (prefix != d->labelsPrefix)
		exec(new AxisSetLabelsPrefixCmd(d, prefix, ki18n("%1: set label prefix")));
}

STD_SETTER_CMD_IMPL_F_S(Axis, SetLabelsSuffix, QString, labelsSuffix, retransformTickLabelStrings);
void Axis::setLabelsSuffix(const QString& suffix) {
	Q_D(Axis);
	if (suffix != d->labelsSuffix)
		exec(new AxisSetLabelsSuffixCmd(d, suffix, ki18n("%1: set label suffix")));
}

STD_SETTER_CMD_IMPL_F_S(Axis, SetLabelsOpacity, qreal, labelsOpacity, update);
void Axis::setLabelsOpacity(qreal opacity) {
	Q_D(Axis);
	if (opacity != d->labelsOpacity)
		exec(new AxisSetLabelsOpacityCmd(d, opacity, ki18n("%1: set labels opacity")));
}

//Major grid
STD_SETTER_CMD_IMPL_F_S(Axis, SetMajorGridPen, QPen, majorGridPen, retransformMajorGrid);
void Axis::setMajorGridPen(const QPen& pen) {
	Q_D(Axis);
	if (pen != d->majorGridPen)
		exec(new AxisSetMajorGridPenCmd(d, pen, ki18n("%1: set major grid style")));
}

STD_SETTER_CMD_IMPL_F_S(Axis, SetMajorGridOpacity, qreal, majorGridOpacity, updateGrid);
void Axis::setMajorGridOpacity(qreal opacity) {
	Q_D(Axis);
	if (opacity != d->majorGridOpacity)
		exec(new AxisSetMajorGridOpacityCmd(d, opacity, ki18n("%1: set major grid opacity")));
}

//Minor grid
STD_SETTER_CMD_IMPL_F_S(Axis, SetMinorGridPen, QPen, minorGridPen, retransformMinorGrid);
void Axis::setMinorGridPen(const QPen& pen) {
	Q_D(Axis);
	if (pen != d->minorGridPen)
		exec(new AxisSetMinorGridPenCmd(d, pen, ki18n("%1: set minor grid style")));
}

STD_SETTER_CMD_IMPL_F_S(Axis, SetMinorGridOpacity, qreal, minorGridOpacity, updateGrid);
void Axis::setMinorGridOpacity(qreal opacity) {
	Q_D(Axis);
	if (opacity != d->minorGridOpacity)
		exec(new AxisSetMinorGridOpacityCmd(d, opacity, ki18n("%1: set minor grid opacity")));
}

//##############################################################################
//####################################  SLOTs   ################################
//##############################################################################
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

//##############################################################################
//######  SLOTs for changes triggered via QActions in the context menu  ########
//##############################################################################
void Axis::orientationChangedSlot(QAction* action) {
	if (action == orientationHorizontalAction)
		this->setOrientation(Axis::Orientation::Horizontal);
	else
		this->setOrientation(Axis::Orientation::Vertical);
}

void Axis::lineStyleChanged(QAction* action) {
	Q_D(const Axis);
	QPen pen = d->linePen;
	pen.setStyle(GuiTools::penStyleFromAction(lineStyleActionGroup, action));
	this->setLinePen(pen);
}

void Axis::lineColorChanged(QAction* action) {
	Q_D(const Axis);
	QPen pen = d->linePen;
	pen.setColor(GuiTools::colorFromAction(lineColorActionGroup, action));
	this->setLinePen(pen);
}

void Axis::visibilityChangedSlot() {
	Q_D(const Axis);
	this->setVisible(!d->isVisible());
}

//#####################################################################
//################### Private implementation ##########################
//#####################################################################
AxisPrivate::AxisPrivate(Axis* owner) : gridItem(new AxisGrid(this)), q(owner) {
	setFlag(QGraphicsItem::ItemIsSelectable, true);
	setFlag(QGraphicsItem::ItemIsFocusable, true);
	setAcceptHoverEvents(true);
}

QString AxisPrivate::name() const{
	return q->name();
}

bool AxisPrivate::swapVisible(bool on) {
	bool oldValue = isVisible();

	//When making a graphics item invisible, it gets deselected in the scene.
	//In this case we don't want to deselect the item in the project explorer.
	//We need to supress the deselection in the view.
	auto* worksheet = static_cast<Worksheet*>(q->parent(AspectType::Worksheet));
	worksheet->suppressSelectionChangedEvent(true);
	setVisible(on);
	gridItem->setVisible(on);
	worksheet->suppressSelectionChangedEvent(false);

	emit q->visibilityChanged(on);
	return oldValue;
}

QRectF AxisPrivate::boundingRect() const{
	return boundingRectangle;
}

/*!
  Returns the shape of the XYCurve as a QPainterPath in local coordinates
*/
QPainterPath AxisPrivate::shape() const{
	return axisShape;
}

/*!
	recalculates the position of the axis on the worksheet
 */
void AxisPrivate::retransform() {
	DEBUG(Q_FUNC_INFO)
	if (suppressRetransform || !plot)
		return;

// 	PERFTRACE(name().toLatin1() + ", AxisPrivate::retransform()");
	m_suppressRecalc = true;
	retransformLine();
	m_suppressRecalc = false;
	recalcShapeAndBoundingRect();
}

void AxisPrivate::retransformLine() {
	DEBUG(Q_FUNC_INFO << ", x range index = " << cSystem->xIndex())
	if (suppressRetransform)
		return;

	linePath = QPainterPath();
	lines.clear();

	QPointF startPoint;
	QPointF endPoint;

	if (orientation == Axis::Orientation::Horizontal) {
		if (position == Axis::Position::Top)
			offset = plot->yRange().end();
		else if (position == Axis::Position::Bottom)
			offset = plot->yRange().start();
		else if (position == Axis::Position::Centered)
			offset = plot->yRange().center();

		startPoint = QPointF(range.start(), offset);
		endPoint = QPointF(range.end(), offset);
	} else { // vertical
		if (position == Axis::Position::Left)
			offset = plot->xRange(cSystem->xIndex()).start();
		else if (position == Axis::Position::Right)
			offset = plot->xRange(cSystem->xIndex()).end();
		else if (position == Axis::Position::Centered)
			offset = plot->xRange(cSystem->xIndex()).center();

		startPoint = QPointF(offset, range.start());
		endPoint = QPointF(offset, range.end());
	}

	lines.append(QLineF(startPoint, endPoint));
	lines = cSystem->mapLogicalToScene(lines, AbstractCoordinateSystem::MappingFlag::MarkGaps);
	for (const auto& line : lines) {
		linePath.moveTo(line.p1());
		linePath.lineTo(line.p2());
	}

	if (linePath.isEmpty()) {
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
		const QPointF& endPoint = lines.at(lines.size()-1).p2();
		this->addArrow(endPoint, 1);
	}

	if (arrowPosition == Axis::ArrowPosition::Left || arrowPosition == Axis::ArrowPosition::Both) {
		const QPointF& endPoint = lines.at(0).p1();
		this->addArrow(endPoint, -1);
	}

	recalcShapeAndBoundingRect();
}

void AxisPrivate::addArrow(QPointF startPoint, int direction) {
	static const double cos_phi = cos(M_PI/6.);

	if (orientation == Axis::Orientation::Horizontal) {
		QPointF endPoint = QPointF(startPoint.x() + direction*arrowSize, startPoint.y());
		arrowPath.moveTo(startPoint);
		arrowPath.lineTo(endPoint);

		switch (arrowType) {
			case Axis::ArrowType::NoArrow:
				break;
			case Axis::ArrowType::SimpleSmall:
				arrowPath.moveTo(endPoint);
				arrowPath.lineTo(QPointF(endPoint.x()-direction*arrowSize/4, endPoint.y()-arrowSize/4*cos_phi));
				arrowPath.moveTo(endPoint);
				arrowPath.lineTo(QPointF(endPoint.x()-direction*arrowSize/4, endPoint.y()+arrowSize/4*cos_phi));
				break;
			case Axis::ArrowType::SimpleBig:
				arrowPath.moveTo(endPoint);
				arrowPath.lineTo(QPointF(endPoint.x()-direction*arrowSize/2, endPoint.y()-arrowSize/2*cos_phi));
				arrowPath.moveTo(endPoint);
				arrowPath.lineTo(QPointF(endPoint.x()-direction*arrowSize/2, endPoint.y()+arrowSize/2*cos_phi));
				break;
			case Axis::ArrowType::FilledSmall:
				arrowPath.lineTo(QPointF(endPoint.x()-direction*arrowSize/4, endPoint.y()-arrowSize/4*cos_phi));
				arrowPath.lineTo(QPointF(endPoint.x()-direction*arrowSize/4, endPoint.y()+arrowSize/4*cos_phi));
				arrowPath.lineTo(endPoint);
				break;
			case Axis::ArrowType::FilledBig:
				arrowPath.lineTo(QPointF(endPoint.x()-direction*arrowSize/2, endPoint.y()-arrowSize/2*cos_phi));
				arrowPath.lineTo(QPointF(endPoint.x()-direction*arrowSize/2, endPoint.y()+arrowSize/2*cos_phi));
				arrowPath.lineTo(endPoint);
				break;
			case Axis::ArrowType::SemiFilledSmall:
				arrowPath.lineTo(QPointF(endPoint.x()-direction*arrowSize/4, endPoint.y()-arrowSize/4*cos_phi));
				arrowPath.lineTo(QPointF(endPoint.x()-direction*arrowSize/8, endPoint.y()));
				arrowPath.lineTo(QPointF(endPoint.x()-direction*arrowSize/4, endPoint.y()+arrowSize/4*cos_phi));
				arrowPath.lineTo(endPoint);
				break;
			case Axis::ArrowType::SemiFilledBig:
				arrowPath.lineTo(QPointF(endPoint.x()-direction*arrowSize/2, endPoint.y()-arrowSize/2*cos_phi));
				arrowPath.lineTo(QPointF(endPoint.x()-direction*arrowSize/4, endPoint.y()));
				arrowPath.lineTo(QPointF(endPoint.x()-direction*arrowSize/2, endPoint.y()+arrowSize/2*cos_phi));
				arrowPath.lineTo(endPoint);
				break;
		}
	} else { //vertical orientation
		QPointF endPoint = QPointF(startPoint.x(), startPoint.y()-direction*arrowSize);
		arrowPath.moveTo(startPoint);
		arrowPath.lineTo(endPoint);

		switch (arrowType) {
			case Axis::ArrowType::NoArrow:
				break;
			case Axis::ArrowType::SimpleSmall:
				arrowPath.moveTo(endPoint);
				arrowPath.lineTo(QPointF(endPoint.x()-arrowSize/4*cos_phi, endPoint.y()+direction*arrowSize/4));
				arrowPath.moveTo(endPoint);
				arrowPath.lineTo(QPointF(endPoint.x()+arrowSize/4*cos_phi, endPoint.y()+direction*arrowSize/4));
				break;
			case Axis::ArrowType::SimpleBig:
				arrowPath.moveTo(endPoint);
				arrowPath.lineTo(QPointF(endPoint.x()-arrowSize/2*cos_phi, endPoint.y()+direction*arrowSize/2));
				arrowPath.moveTo(endPoint);
				arrowPath.lineTo(QPointF(endPoint.x()+arrowSize/2*cos_phi, endPoint.y()+direction*arrowSize/2));
				break;
			case Axis::ArrowType::FilledSmall:
				arrowPath.lineTo(QPointF(endPoint.x()-arrowSize/4*cos_phi, endPoint.y()+direction*arrowSize/4));
				arrowPath.lineTo(QPointF(endPoint.x()+arrowSize/4*cos_phi, endPoint.y()+direction*arrowSize/4));
				arrowPath.lineTo(endPoint);
				break;
			case Axis::ArrowType::FilledBig:
				arrowPath.lineTo(QPointF(endPoint.x()-arrowSize/2*cos_phi, endPoint.y()+direction*arrowSize/2));
				arrowPath.lineTo(QPointF(endPoint.x()+arrowSize/2*cos_phi, endPoint.y()+direction*arrowSize/2));
				arrowPath.lineTo(endPoint);
				break;
			case Axis::ArrowType::SemiFilledSmall:
				arrowPath.lineTo(QPointF(endPoint.x()-arrowSize/4*cos_phi, endPoint.y()+direction*arrowSize/4));
				arrowPath.lineTo(QPointF(endPoint.x(), endPoint.y()+direction*arrowSize/8));
				arrowPath.lineTo(QPointF(endPoint.x()+arrowSize/4*cos_phi, endPoint.y()+direction*arrowSize/4));
				arrowPath.lineTo(endPoint);
				break;
			case Axis::ArrowType::SemiFilledBig:
				arrowPath.lineTo(QPointF(endPoint.x()-arrowSize/2*cos_phi, endPoint.y()+direction*arrowSize/2));
				arrowPath.lineTo(QPointF(endPoint.x(), endPoint.y()+direction*arrowSize/4));
				arrowPath.lineTo(QPointF(endPoint.x()+arrowSize/2*cos_phi, endPoint.y()+direction*arrowSize/2));
				arrowPath.lineTo(endPoint);
				break;
		}
	}
}

//! helper function for retransformTicks()
bool AxisPrivate::transformAnchor(QPointF* anchorPoint) {
	QVector<QPointF> points;
	points.append(*anchorPoint);
	points = cSystem->mapLogicalToScene(points);

	if (points.count() != 1) { // point is not mappable or in a coordinate gap
		return false;
	} else {
		*anchorPoint = points.at(0);
		return true;
	}
}

/*!
	recalculates the position of the axis ticks.
 */
void AxisPrivate::retransformTicks() {
	if (suppressRetransform)
		return;

	majorTicksPath = QPainterPath();
	minorTicksPath = QPainterPath();
	majorTickPoints.clear();
	minorTickPoints.clear();
	tickLabelValues.clear();

	if ( majorTicksNumber < 1 || (majorTicksDirection == Axis::noTicks && minorTicksDirection == Axis::noTicks) ) {
		retransformTickLabelPositions(); //this calls recalcShapeAndBoundingRect()
		return;
	}

	//determine the increment for the major ticks
	double majorTicksIncrement = 0;
	int tmpMajorTicksNumber = 0;
	double start{range.start()}, end{range.end()};
	//TODO: check that start and end are > 0 for log and >=0 for sqrt, etc.
	if (majorTicksType == Axis::TicksType::TotalNumber) {
		//the total number of major ticks is given - > determine the increment
		tmpMajorTicksNumber = majorTicksNumber;
		switch (scale) {
			case Axis::Scale::Linear:
				majorTicksIncrement = range.size()/(majorTicksNumber-1);
				break;
			case Axis::Scale::Log10:
				majorTicksIncrement = (log10(end) - log10(start))/(majorTicksNumber-1);
				break;
			case Axis::Scale::Log2:
				majorTicksIncrement = (log(end) - log(start))/log(2)/(majorTicksNumber-1);
				break;
			case Axis::Scale::Ln:
				majorTicksIncrement = (log(end) - log(start))/(majorTicksNumber-1);
				break;
			case Axis::Scale::Sqrt:
				majorTicksIncrement = (sqrt(end) - sqrt(start))/(majorTicksNumber-1);
				break;
			case Axis::Scale::X2:
				majorTicksIncrement = (end*end - start*start)/(majorTicksNumber-1);
		}
	} else if (majorTicksType == Axis::TicksType::Spacing) {
		//the increment of the major ticks is given -> determine the number
		majorTicksIncrement = majorTicksSpacing * GSL_SIGN(end-start);
		switch (scale) {
			case Axis::Scale::Linear:
				tmpMajorTicksNumber = qRound(range.size()/majorTicksIncrement + 1);
				break;
			case Axis::Scale::Log10:
				tmpMajorTicksNumber = qRound((log10(end)-log10(start))/majorTicksIncrement + 1);
				break;
			case Axis::Scale::Log2:
				tmpMajorTicksNumber = qRound((log(end)-log(start))/log(2)/majorTicksIncrement + 1);
				break;
			case Axis::Scale::Ln:
				tmpMajorTicksNumber = qRound((log(end)-log(start))/majorTicksIncrement + 1);
				break;
			case Axis::Scale::Sqrt:
				tmpMajorTicksNumber = qRound((sqrt(end)-sqrt(start))/majorTicksIncrement + 1);
				break;
			case Axis::Scale::X2:
				tmpMajorTicksNumber = qRound((end*end - start*start)/majorTicksIncrement + 1);
		}
	} else { //custom column was provided
		if (majorTicksColumn) {
			tmpMajorTicksNumber = majorTicksColumn->rowCount();
		} else {
			retransformTickLabelPositions(); //this calls recalcShapeAndBoundingRect()
			return;
		}
	}

	int tmpMinorTicksNumber;
	if (minorTicksType == Axis::TicksType::TotalNumber)
		tmpMinorTicksNumber = minorTicksNumber;
	else if (minorTicksType == Axis::TicksType::Spacing)
		tmpMinorTicksNumber = fabs(end - start)/ (majorTicksNumber - 1)/minorTicksIncrement - 1;
	else
		(minorTicksColumn) ? tmpMinorTicksNumber = minorTicksColumn->rowCount() : tmpMinorTicksNumber = 0;

	QPointF anchorPoint;
	QPointF startPoint;
	QPointF endPoint;
	qreal majorTickPos = 0.0;
	qreal minorTickPos;
	qreal nextMajorTickPos = 0.0;
	const int xDirection = cSystem->xDirection();
	const int yDirection = cSystem->yDirection();
	const double middleX = plot->xRange(cSystem->xIndex()).center();
	const double middleY = plot->yRange().center();
	bool valid;

	//DEBUG("tmpMajorTicksNumber = " << tmpMajorTicksNumber)
	for (int iMajor = 0; iMajor < tmpMajorTicksNumber; iMajor++) {
		//DEBUG("major tick " << iMajor)
		//calculate major tick's position
		if (majorTicksType != Axis::TicksType::CustomColumn) {
			switch (scale) {
				case Axis::Scale::Linear:
					majorTickPos = start + majorTicksIncrement * iMajor;
					nextMajorTickPos = majorTickPos + majorTicksIncrement;
					break;
				case Axis::Scale::Log10:
					majorTickPos = start * pow(10, majorTicksIncrement*iMajor);
					nextMajorTickPos = majorTickPos * pow(10, majorTicksIncrement);
					break;
				case Axis::Scale::Log2:
					majorTickPos = start * pow(2, majorTicksIncrement*iMajor);
					nextMajorTickPos = majorTickPos * pow(2, majorTicksIncrement);
					break;
				case Axis::Scale::Ln:
					majorTickPos = start * exp(majorTicksIncrement*iMajor);
					nextMajorTickPos = majorTickPos * exp(majorTicksIncrement);
					break;
				case Axis::Scale::Sqrt:
					majorTickPos = pow(sqrt(start) + majorTicksIncrement*iMajor, 2);
					nextMajorTickPos = pow(sqrt(start) + majorTicksIncrement*(iMajor+1), 2);
					break;
				case Axis::Scale::X2:
					majorTickPos = sqrt(start*start + majorTicksIncrement*iMajor);
					nextMajorTickPos = sqrt(start*start + majorTicksIncrement*(iMajor+1));
					break;
			}
		} else {	// custom column
			if (!majorTicksColumn->isValid(iMajor) || majorTicksColumn->isMasked(iMajor))
				continue;
			majorTickPos = majorTicksColumn->valueAt(iMajor);
			// set next major tick pos for minor ticks
			if (iMajor < tmpMajorTicksNumber - 1) {
				if (majorTicksColumn->isValid(iMajor+1) && !majorTicksColumn->isMasked(iMajor+1))
					nextMajorTickPos = majorTicksColumn->valueAt(iMajor+1);
			} else	// last major tick
				tmpMinorTicksNumber = 0;
		}

		//calculate start and end points for major tick's line
		if (majorTicksDirection != Axis::noTicks) {
			if (orientation == Axis::Orientation::Horizontal) {
				anchorPoint.setX(majorTickPos);
				anchorPoint.setY(offset);
				valid = transformAnchor(&anchorPoint);
				if (valid) {
					if (offset < middleY) {
						startPoint = anchorPoint + QPointF(0, (majorTicksDirection & Axis::ticksIn)  ? yDirection * majorTicksLength  : 0);
						endPoint   = anchorPoint + QPointF(0, (majorTicksDirection & Axis::ticksOut) ? -yDirection * majorTicksLength : 0);
					} else {
						startPoint = anchorPoint + QPointF(0, (majorTicksDirection & Axis::ticksOut)  ? yDirection * majorTicksLength  : 0);
						endPoint   = anchorPoint + QPointF(0, (majorTicksDirection & Axis::ticksIn) ? -yDirection * majorTicksLength : 0);
					}
				}
			} else { // vertical
				anchorPoint.setY(majorTickPos);
				anchorPoint.setX(offset);
				valid = transformAnchor(&anchorPoint);

				if (valid) {
					if (offset < middleX) {
						startPoint = anchorPoint + QPointF((majorTicksDirection & Axis::ticksIn)  ? xDirection * majorTicksLength  : 0, 0);
						endPoint = anchorPoint + QPointF((majorTicksDirection & Axis::ticksOut) ? -xDirection * majorTicksLength : 0, 0);
					} else {
						startPoint = anchorPoint + QPointF((majorTicksDirection & Axis::ticksOut) ? xDirection * majorTicksLength : 0, 0);
						endPoint = anchorPoint + QPointF((majorTicksDirection & Axis::ticksIn)  ? -xDirection *  majorTicksLength  : 0, 0);
					}
				}
			}

			double value = scalingFactor * majorTickPos + zeroOffset;

			//if custom column is used, we can have duplicated values in it and we need only unique values
			if (majorTicksType == Axis::TicksType::CustomColumn && tickLabelValues.indexOf(value) != -1)
				valid = false;

			//add major tick's line to the painter path
			if (valid) {
				majorTicksPath.moveTo(startPoint);
				majorTicksPath.lineTo(endPoint);
				majorTickPoints << anchorPoint;
				tickLabelValues << value;
			}
		}

		//minor ticks
		//DEBUG("	tmpMinorTicksNumber = " << tmpMinorTicksNumber)
		if (Axis::noTicks != minorTicksDirection && tmpMajorTicksNumber > 1 && tmpMinorTicksNumber > 0 && iMajor < tmpMajorTicksNumber - 1 && nextMajorTickPos != majorTickPos) {
			//minor ticks are placed at equidistant positions independent of the selected scaling for the major ticks positions
			double minorTicksIncrement = (nextMajorTickPos - majorTickPos)/(tmpMinorTicksNumber + 1);
			//DEBUG("	nextMajorTickPos = " << nextMajorTickPos)
			//DEBUG("	majorTickPos = " << majorTickPos)
			//DEBUG("	minorTicksIncrement = " << minorTicksIncrement)

			for (int iMinor = 0; iMinor < tmpMinorTicksNumber; iMinor++) {
				//calculate minor tick's position
				if (minorTicksType != Axis::TicksType::CustomColumn) {
					minorTickPos = majorTickPos + (iMinor + 1) * minorTicksIncrement;
				} else {
					if (!minorTicksColumn->isValid(iMinor) || minorTicksColumn->isMasked(iMinor))
						continue;
					minorTickPos = minorTicksColumn->valueAt(iMinor);

					//in the case a custom column is used for the minor ticks, we draw them _once_ for the whole range of the axis.
					//execute the minor ticks loop only once.
					if (iMajor > 0)
						break;
				}
				//DEBUG("		minorTickPos = " << minorTickPos)

				//calculate start and end points for minor tick's line
				if (orientation == Axis::Orientation::Horizontal) {
					anchorPoint.setX(minorTickPos);
					anchorPoint.setY(offset);
					valid = transformAnchor(&anchorPoint);

					if (valid) {
						if (offset < middleY) {
							startPoint = anchorPoint + QPointF(0, (minorTicksDirection & Axis::ticksIn)  ? yDirection * minorTicksLength  : 0);
							endPoint   = anchorPoint + QPointF(0, (minorTicksDirection & Axis::ticksOut) ? -yDirection * minorTicksLength : 0);
						} else {
							startPoint = anchorPoint + QPointF(0, (minorTicksDirection & Axis::ticksOut)  ? yDirection * minorTicksLength  : 0);
							endPoint   = anchorPoint + QPointF(0, (minorTicksDirection & Axis::ticksIn) ? -yDirection * minorTicksLength : 0);
						}
					}
				} else { // vertical
					anchorPoint.setY(minorTickPos);
					anchorPoint.setX(offset);
					valid = transformAnchor(&anchorPoint);

					if (valid) {
						if (offset < middleX) {
							startPoint = anchorPoint + QPointF((minorTicksDirection & Axis::ticksIn)  ? xDirection * minorTicksLength  : 0, 0);
							endPoint   = anchorPoint + QPointF((minorTicksDirection & Axis::ticksOut) ? -xDirection * minorTicksLength : 0, 0);
						} else {
							startPoint = anchorPoint + QPointF((minorTicksDirection & Axis::ticksOut)  ? xDirection * minorTicksLength  : 0, 0);
							endPoint   = anchorPoint + QPointF((minorTicksDirection & Axis::ticksIn) ? -xDirection * minorTicksLength : 0, 0);
						}
					}
				}

				//add minor tick's line to the painter path
				if (valid) {
					minorTicksPath.moveTo(startPoint);
					minorTicksPath.lineTo(endPoint);
					minorTickPoints << anchorPoint;
				}
			}
		}
	}

	//tick positions where changed -> update the position of the tick labels and grid lines
	retransformTickLabelStrings();
	retransformMajorGrid();
	retransformMinorGrid();
}

/*!
	creates the tick label strings starting with the most optimal
	(=the smallest possible number of float digits) precision for the floats
*/
void AxisPrivate::retransformTickLabelStrings() {
	DEBUG(Q_FUNC_INFO <<", labels precision = " << labelsPrecision)
	if (suppressRetransform)
		return;

	if (labelsAutoPrecision) {
		//check, whether we need to increase the current precision
		int newPrecision = upperLabelsPrecision(labelsPrecision, labelsFormat);
		if (newPrecision != labelsPrecision) {
			labelsPrecision = newPrecision;
			emit q->labelsPrecisionChanged(labelsPrecision);
		} else {
			//check, whether we can reduce the current precision
			newPrecision = lowerLabelsPrecision(labelsPrecision, labelsFormat);
			if (newPrecision != labelsPrecision) {
				labelsPrecision = newPrecision;
				emit q->labelsPrecisionChanged(labelsPrecision);
			}
		}
		DEBUG(Q_FUNC_INFO << ", auto labels precision = " << labelsPrecision)
	}

	//automatically switch from 'decimal' to 'scientific' format for big and small numbers
	//and back to decimal when the numbers get smaller after the auto-switch again
	if (labelsFormat == Axis::LabelsFormat::Decimal && !labelsFormatDecimalOverruled) {
		for (auto value : tickLabelValues) {
			if ( qAbs(value) > 1.e4 || (qAbs(value) > 0 && qAbs(value) < 1e-4) ) {
				labelsFormat = Axis::LabelsFormat::Scientific;
				emit q->labelsFormatChanged(labelsFormat);
				labelsFormatAutoChanged = true;
				break;
			}
		}
	} else if (labelsFormatAutoChanged ) {
		//check whether we still have big or small numbers
		bool changeBack = true;
		for (auto value : tickLabelValues) {
			if ( qAbs(value) > 1.e4 || (qAbs(value) > 0 && qAbs(value) < 1.e-4) ) {
				changeBack = false;
				break;
			}
		}

		if (changeBack) {
			labelsFormatAutoChanged = false;
			labelsFormat = Axis::LabelsFormat::Decimal;
			emit q->labelsFormatChanged(labelsFormat);
		}
	}

	tickLabelStrings.clear();
	QString str;
	SET_NUMBER_LOCALE
	if ( (orientation == Axis::Orientation::Horizontal && plot->xRangeFormat() == CartesianPlot::RangeFormat::Numeric)
		|| (orientation == Axis::Orientation::Vertical && plot->yRangeFormat() == CartesianPlot::RangeFormat::Numeric) ) {
		if (labelsFormat == Axis::LabelsFormat::Decimal) {
			QString nullStr = numberLocale.toString(0., 'f', labelsPrecision);
			for (const auto value : tickLabelValues) {
				str = numberLocale.toString(value, 'f', labelsPrecision);
				if (str == "-" + nullStr) str = nullStr;
				str = labelsPrefix + str + labelsSuffix;
				tickLabelStrings << str;
			}
		} else if (labelsFormat == Axis::LabelsFormat::ScientificE) {
			QString nullStr = numberLocale.toString(0., 'e', labelsPrecision);
			for (const auto value : tickLabelValues) {
				if (value == 0)	// just show "0"
					str = numberLocale.toString(value, 'f', 0);
				else
					str = numberLocale.toString(value, 'e', labelsPrecision);
				if (str == "-" + nullStr) str = nullStr;
				str = labelsPrefix + str + labelsSuffix;
				tickLabelStrings << str;
			}
		} else if (labelsFormat == Axis::LabelsFormat::Powers10) {
			for (const auto value : tickLabelValues) {
				if (value == 0)	// just show "0"
					str = numberLocale.toString(value, 'f', 0);
				else  {
					str = "10<sup>" + numberLocale.toString(log10(qAbs(value)), 'f', labelsPrecision) + "</sup>";
					if (value < 0)
						str.prepend("-");
				}
				str = labelsPrefix + str + labelsSuffix;
				tickLabelStrings << str;
			}
		} else if (labelsFormat == Axis::LabelsFormat::Powers2) {
			for (const auto value : tickLabelValues) {
				if (value == 0)	// just show "0"
					str = numberLocale.toString(value, 'f', 0);
				else  {
					str = "2<span style=\"vertical-align:super\">" + numberLocale.toString(log2(qAbs(value)), 'f', labelsPrecision) + "</span>";
					if (value < 0)
						str.prepend("-");
				}
				str = labelsPrefix + str + labelsSuffix;
				tickLabelStrings << str;
			}
		} else if (labelsFormat == Axis::LabelsFormat::PowersE) {
			for (const auto value : tickLabelValues) {
				if (value == 0)	// just show "0"
					str = numberLocale.toString(value, 'f', 0);
				else {
					str = "e<span style=\"vertical-align:super\">" + numberLocale.toString(log(qAbs(value)), 'f', labelsPrecision) + "</span>";
					if (value < 0)
						str.prepend("-");
				}
				str = labelsPrefix + str + labelsSuffix;
				tickLabelStrings << str;
			}
		} else if (labelsFormat == Axis::LabelsFormat::MultipliesPi) {
			for (const auto value : tickLabelValues) {
				if (value == 0)	// just show "0"
					str = numberLocale.toString(value, 'f', 0);
				else
					str = "<span>" + numberLocale.toString(value / M_PI, 'f', labelsPrecision) + "</span>" + QChar(0x03C0);
				str = labelsPrefix + str + labelsSuffix;
				tickLabelStrings << str;
			}
		} else if (labelsFormat == Axis::LabelsFormat::Scientific) {
			for (const auto value : tickLabelValues) {
				if (value == 0)	// just show "0"
					str = numberLocale.toString(value, 'f', 0);
				else if (qAbs(value) < 10. && qAbs(value) > .1)	// use normal notation for values near 1
					str = numberLocale.toString(value, 'f', labelsPrecision);
				else {
					str = numberLocale.toString(value, 'e', labelsPrecision);
					str.remove("+");	// remove '+' in exponent
					str.replace( QRegExp("e(.*)"), "×10<sup>\\1</sup>" );	// e(-)NN -> ×10<sup>(-)NN</sup>
				}
				str = labelsPrefix + str + labelsSuffix;
				tickLabelStrings << str;
			}
		}
	} else {
		for (const auto value : tickLabelValues) {
			QDateTime dateTime;
			dateTime.setMSecsSinceEpoch(value);
			str = dateTime.toString(labelsDateTimeFormat);
			str = labelsPrefix + str + labelsSuffix;
			tickLabelStrings << str;
		}
	}

	//recalculate the position of the tick labels
	retransformTickLabelPositions();
}

/*!
	returns the smallest upper limit for the precision
	where no duplicates for the tick label values occur.
 */
int AxisPrivate::upperLabelsPrecision(const int precision, const Axis::LabelsFormat format) {
	DEBUG(Q_FUNC_INFO << ", precision = " << precision);

	// avoid problems with zero range axis
	if (tickLabelValues.isEmpty() || qFuzzyCompare(tickLabelValues.constFirst(), tickLabelValues.constLast())) {
		DEBUG(Q_FUNC_INFO << ", zero range axis detected. ticklabel values: ")
		QDEBUG(Q_FUNC_INFO << tickLabelValues)
		return 0;
	}

	//round values to the current precision and look for duplicates.
	//if there are duplicates, increase the precision.
	QVector<double> tempValues;
	switch (format) {
	case Axis::LabelsFormat::Decimal:
	case Axis::LabelsFormat::MultipliesPi:
		for (const auto value : tickLabelValues)
			tempValues.append( nsl_math_round_places(value, precision) );
		break;
	case Axis::LabelsFormat::ScientificE:
	case Axis::LabelsFormat::Scientific:
		for (const auto value : tickLabelValues)
			tempValues.append( nsl_math_round_precision(value, precision) );
		break;
	case Axis::LabelsFormat::Powers10:
		for (const auto value : tickLabelValues)
			tempValues.append( nsl_math_round_places(log10(qAbs(value)), precision) );
		break;
	case Axis::LabelsFormat::Powers2:
		for (const auto value : tickLabelValues)
			tempValues.append( nsl_math_round_places(log2(qAbs(value)), precision) );
		break;
	case Axis::LabelsFormat::PowersE:
		for (const auto value : tickLabelValues)
			tempValues.append( nsl_math_round_places(log(qAbs(value)), precision) );
	}

	for (int i = 0; i < tempValues.size(); ++i) {
		for (int j = 0; j < tempValues.size(); ++j) {
			if (i == j)
				continue;

			// if duplicate for the current precision found, increase the precision and check again
			if (tempValues.at(i) == tempValues.at(j))
				return upperLabelsPrecision(precision + 1, format);
		}
	}

	//no duplicates for the current precision found: return the current value
	DEBUG(Q_FUNC_INFO << ", upper precision = " << precision);
	return precision;
}

/*!
	returns highest lower limit for the precision
	where no duplicates for the tick label values occur.
*/
int AxisPrivate::lowerLabelsPrecision(const int precision, const Axis::LabelsFormat format) {
	DEBUG(Q_FUNC_INFO << ", precision = " << precision);
	//round value to the current precision and look for duplicates.
	//if there are duplicates, decrease the precision.
	QVector<double> tempValues;
	switch (format) {
	case Axis::LabelsFormat::Decimal:
	case Axis::LabelsFormat::MultipliesPi:
		for (auto value : tickLabelValues)
			tempValues.append( nsl_math_round_places(value, precision-1) );
		break;
	case Axis::LabelsFormat::ScientificE:
	case Axis::LabelsFormat::Scientific:
		for (auto value : tickLabelValues)
			tempValues.append( nsl_math_round_precision(value, precision-1) );
		break;
	case Axis::LabelsFormat::Powers10:
		for (auto value : tickLabelValues)
			tempValues.append( nsl_math_round_places(log10(qAbs(value)), precision-1) );
		break;
	case Axis::LabelsFormat::Powers2:
		for (auto value : tickLabelValues)
			tempValues.append( nsl_math_round_places(log2(qAbs(value)), precision-1) );
		break;
	case Axis::LabelsFormat::PowersE:
		for (auto value : tickLabelValues)
			tempValues.append( nsl_math_round_places(log(qAbs(value)), precision-1) );
	}


	//check whether we have duplicates with reduced precision
	//-> current precision cannot be reduced, return the current value
	for (int i = 0; i < tempValues.size(); ++i) {
		for (int j = 0; j < tempValues.size(); ++j) {
			if (i == j) continue;
			if (tempValues.at(i) == tempValues.at(j))
				return precision;
		}
	}

	if (precision == 0) {
		bool hasDoubles = false;
		for (auto value : tickLabelValues) {
			if (floor(value) != value) {
				hasDoubles = true;
				break;
			}
		}

		//if we have double values we don't want to show them as integers, keep at least one digit.
		if (hasDoubles)
			return 1;
		else
			return 0;
	} else {
		//no duplicates found, reduce further, and check again
		return lowerLabelsPrecision(precision - 1, format);
	}
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
	double width = 0;
	double height = fm.ascent();
	QPointF pos;
	const double middleX = plot->xRange(cSystem->xIndex()).center();
	const double middleY = plot->yRange().center();
	const int xDirection = cSystem->xDirection();
	const int yDirection = cSystem->yDirection();

	QPointF startPoint, endPoint, anchorPoint;

	QTextDocument td;
	td.setDefaultFont(labelsFont);
	// TODO: M_PI/180. factor
	const double cosine = cos(labelsRotationAngle * M_PI / 180.); // calculate only one time
	const double sine = sin(labelsRotationAngle * M_PI / 180.); // calculate only one time
	for ( int i = 0; i < majorTickPoints.size(); i++ ) {
		if ((orientation == Axis::Orientation::Horizontal && plot->xRangeFormat() == CartesianPlot::RangeFormat::Numeric) ||
				(orientation == Axis::Orientation::Vertical && plot->yRangeFormat() == CartesianPlot::RangeFormat::Numeric)) {
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
		anchorPoint = majorTickPoints.at(i);

		//center align all labels with respect to the end point of the tick line
		if (orientation == Axis::Orientation::Horizontal) {
			if (offset < middleY) {
				startPoint = anchorPoint + QPointF(0, (majorTicksDirection & Axis::ticksIn)  ? yDirection * majorTicksLength  : 0);
				endPoint   = anchorPoint + QPointF(0, (majorTicksDirection & Axis::ticksOut) ? -yDirection * majorTicksLength : 0);
			} else {
				startPoint = anchorPoint + QPointF(0, (majorTicksDirection & Axis::ticksOut)  ? yDirection * majorTicksLength  : 0);
				endPoint   = anchorPoint + QPointF(0, (majorTicksDirection & Axis::ticksIn) ? -yDirection * majorTicksLength : 0);
			}

			// for rotated labels (angle is not zero), align label's corner at the position of the tick
			if (fabs(fabs(labelsRotationAngle) - 180.) < 1.e-2) { // +-180°
				if (labelsPosition == Axis::LabelsPosition::Out) {
					pos.setX(endPoint.x() + width/2);
					pos.setY(endPoint.y() + labelsOffset );
				} else {
					pos.setX(startPoint.x() + width/2);
					pos.setY(startPoint.y() - height - labelsOffset);
				}
			} else if (labelsRotationAngle <= -0.01) { // [-0.01°, -180°)
				if (labelsPosition == Axis::LabelsPosition::Out) {
					pos.setX(endPoint.x() + sine * height/2);
					pos.setY(endPoint.y() + labelsOffset + cosine * height/2);
				} else {
					pos.setX(startPoint.x() + sine * height/2 - diffx);
					pos.setY(startPoint.y() - labelsOffset + cosine * height/2 + diffy);
				}
			} else if (labelsRotationAngle >= 0.01) { // [0.01°, 180°)
				if (labelsPosition == Axis::LabelsPosition::Out) {
					pos.setX(endPoint.x() - diffx + sine * height/2);
					pos.setY(endPoint.y() + labelsOffset + diffy + cosine * height/2);
				} else {
					pos.setX(startPoint.x() + sine * height/2);
					pos.setY(startPoint.y() - labelsOffset + cosine * height/2);
				}
			} else {	// 0°
				if (labelsPosition == Axis::LabelsPosition::Out) {
					pos.setX(endPoint.x() - width/2);
					pos.setY(endPoint.y() + height + labelsOffset);
				} else {
					pos.setX(startPoint.x() - width/2);
					pos.setY(startPoint.y() - labelsOffset);
				}
			}
		// ---------------------- vertical -------------------------
		} else {
			if (offset < middleX) {
				startPoint = anchorPoint + QPointF((majorTicksDirection & Axis::ticksIn)  ? xDirection * majorTicksLength  : 0, 0);
				endPoint = anchorPoint + QPointF((majorTicksDirection & Axis::ticksOut) ? -xDirection * majorTicksLength : 0, 0);
			} else {
				startPoint = anchorPoint + QPointF((majorTicksDirection & Axis::ticksOut) ? xDirection * majorTicksLength : 0, 0);
				endPoint = anchorPoint + QPointF((majorTicksDirection & Axis::ticksIn)  ? -xDirection *  majorTicksLength  : 0, 0);
			}

			if (fabs(labelsRotationAngle - 90.) < 1.e-2) { // +90°
				if (labelsPosition == Axis::LabelsPosition::Out) {
					pos.setX(endPoint.x() - labelsOffset);
					pos.setY(endPoint.y() + width/2 );
				} else {
					pos.setX(startPoint.x() + labelsOffset);
					pos.setY(startPoint.y() + width/2);
				}
			} else if (fabs(labelsRotationAngle + 90.) < 1.e-2) { // -90°
				if (labelsPosition == Axis::LabelsPosition::Out) {
					pos.setX(endPoint.x() - labelsOffset - height);
					pos.setY(endPoint.y() - width/2);
				} else {
					pos.setX(startPoint.x() + labelsOffset);
					pos.setY(startPoint.y() - width/2);
				}
			} else if (fabs(fabs(labelsRotationAngle) - 180.) < 1.e-2) { // +-180°
				if (labelsPosition == Axis::LabelsPosition::Out) {
					pos.setX(endPoint.x() - labelsOffset);
					pos.setY(endPoint.y() - height/2);
				} else {
					pos.setX(startPoint.x() + labelsOffset + width);
					pos.setY(startPoint.y() - height/2);
				}
			} else if (fabs(labelsRotationAngle) >= 0.01 && fabs(labelsRotationAngle) <= 89.99) { // [0.01°, 90°)
				if (labelsPosition == Axis::LabelsPosition::Out) {
					// left
					pos.setX(endPoint.x() - labelsOffset - diffx + sine * height/2);
					pos.setY(endPoint.y() + cosine * height/2 + diffy);
				} else {
					pos.setX(startPoint.x() + labelsOffset + sine * height/2);
					pos.setY(startPoint.y() + cosine * height/2);
				}
			} else if (fabs(labelsRotationAngle) >= 90.01 && fabs(labelsRotationAngle) <= 179.99) { // [90.01, 180)
				if (labelsPosition == Axis::LabelsPosition::Out) {
					// left
					pos.setX(endPoint.x() - labelsOffset + sine * height/2);
					pos.setY(endPoint.y() + cosine * height/2);
				} else {
					pos.setX(startPoint.x() + labelsOffset - diffx + sine * height/2);
					pos.setY(startPoint.y() + diffy + cosine * height/2);
				}
			} else { // 0°
				if (labelsPosition == Axis::LabelsPosition::Out) {
					pos.setX(endPoint.x() - width - labelsOffset);
					pos.setY(endPoint.y() + height/2);
				} else {
					pos.setX(startPoint.x() + labelsOffset);
					pos.setY(startPoint.y() + height/2);
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
	if (majorGridPen.style() == Qt::NoPen || majorTickPoints.size() == 0) {
		recalcShapeAndBoundingRect();
		return;
	}

	//major tick points are already in scene coordinates, convert them back to logical...
	//TODO: mapping should work without SuppressPageClipping-flag, check float comparisons in the map-function.
	//Currently, grid lines disappear sometimes without this flag
	QVector<QPointF> logicalMajorTickPoints = cSystem->mapSceneToLogical(majorTickPoints, AbstractCoordinateSystem::MappingFlag::SuppressPageClipping);

	if (logicalMajorTickPoints.isEmpty())
		return;

	//TODO:
	//when iterating over all grid lines, skip the first and the last points for auto scaled axes,
	//since we don't want to paint any grid lines at the plot boundaries
	bool skipLowestTick, skipUpperTick;
	if (orientation == Axis::Orientation::Horizontal) { //horizontal axis
		skipLowestTick = qFuzzyCompare(logicalMajorTickPoints.at(0).x(), plot->xRange(cSystem->xIndex()).start());
		skipUpperTick = qFuzzyCompare(logicalMajorTickPoints.at(logicalMajorTickPoints.size()-1).x(), plot->xRange(cSystem->xIndex()).end());
	} else {
		skipLowestTick = qFuzzyCompare(logicalMajorTickPoints.at(0).y(), plot->yRange().start());
		skipUpperTick = qFuzzyCompare(logicalMajorTickPoints.at(logicalMajorTickPoints.size()-1).y(), plot->yRange().end());
	}

	int start, end;	// TODO: hides Axis::start, Axis::end!
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
	if (orientation == Axis::Orientation::Horizontal) { //horizontal axis
		const Range<double> yRange{plot->yRange()};

		for (int i = start; i < end; ++i) {
			const QPointF& point = logicalMajorTickPoints.at(i);
			lines.append( QLineF(point.x(), yRange.start(), point.x(), yRange.end()) );
		}
	} else { //vertical axis
		const Range<double> xRange{plot->xRange(cSystem->xIndex())};

		//skip the first and the last points, since we don't want to paint any grid lines at the plot boundaries
		for (int i = start; i < end; ++i) {
			const QPointF& point = logicalMajorTickPoints.at(i);
			lines.append( QLineF(xRange.start(), point.y(), xRange.end(), point.y()) );
		}
	}

	lines = cSystem->mapLogicalToScene(lines, AbstractCoordinateSystem::MappingFlag::SuppressPageClipping);
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
	if (minorGridPen.style() == Qt::NoPen) {
		recalcShapeAndBoundingRect();
		return;
	}

	//minor tick points are already in scene coordinates, convert them back to logical...
	//TODO: mapping should work without SuppressPageClipping-flag, check float comparisons in the map-function.
	//Currently, grid lines disappear sometimes without this flag
	QVector<QPointF> logicalMinorTickPoints = cSystem->mapSceneToLogical(minorTickPoints, AbstractCoordinateSystem::MappingFlag::SuppressPageClipping);

	QVector<QLineF> lines;
	if (orientation == Axis::Orientation::Horizontal) { //horizontal axis
		const Range<double> yRange{plot->yRange()};

		for (const auto point : logicalMinorTickPoints)
			lines.append( QLineF(point.x(), yRange.start(), point.x(), yRange.end()) );
	} else { //vertical axis
		const Range<double> xRange{plot->xRange(cSystem->xIndex())};

		for (const auto point: logicalMinorTickPoints)
			lines.append( QLineF(xRange.start(), point.y(), xRange.end(), point.y()) );
	}

	lines = cSystem->mapLogicalToScene(lines, AbstractCoordinateSystem::MappingFlag::SuppressPageClipping);
	for (const auto& line : lines) {
		minorGridPath.moveTo(line.p1());
		minorGridPath.lineTo(line.p2());
	}

	recalcShapeAndBoundingRect();
}

/*!
 * called when the opacity of the grid was changes, update the grid graphics item
 */
//TODO: this function is only needed for loaded projects where update() doesn't seem to be enough
//and we have to call gridItem->update() explicitly.
//This is not required for newly created plots/axes. Why is this difference?
void AxisPrivate::updateGrid() {
	gridItem->update();
}

void AxisPrivate::recalcShapeAndBoundingRect() {
	if (m_suppressRecalc)
		return;

	prepareGeometryChange();

	if (linePath.isEmpty()) {
		axisShape = QPainterPath();
		boundingRectangle = QRectF();
		title->setPositionInvalid(true);
		if (plot) plot->prepareGeometryChange();
		return;
	} else {
		title->setPositionInvalid(false);
	}

	axisShape = WorksheetElement::shapeFromPath(linePath, linePen);
	axisShape.addPath(WorksheetElement::shapeFromPath(arrowPath, linePen));
	axisShape.addPath(WorksheetElement::shapeFromPath(majorTicksPath, majorTicksPen));
	axisShape.addPath(WorksheetElement::shapeFromPath(minorTicksPath, minorTicksPen));

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
			trafo.translate( tickLabelPoints.at(i).x(), tickLabelPoints.at(i).y() );

			trafo.rotate(-labelsRotationAngle);
			tempPath = trafo.map(tempPath);

			tickLabelsPath.addPath(WorksheetElement::shapeFromPath(tempPath, linePen));
		}
		axisShape.addPath(WorksheetElement::shapeFromPath(tickLabelsPath, QPen()));
	}

	//add title label, if available
	if ( title->isVisible() && !title->text().text.isEmpty() ) {
		const QRectF& titleRect = title->graphicsItem()->boundingRect();
		if (titleRect.width() != 0.0 &&  titleRect.height() != 0.0) {
			//determine the new position of the title label:
			//we calculate the new position here and not in retransform(),
			//since it depends on the size and position of the tick labels, tickLabelsPath, available here.
			QRectF rect = linePath.boundingRect();
			qreal offsetX = titleOffsetX; //the distance to the axis line
			qreal offsetY = titleOffsetY; //the distance to the axis line
			if (orientation == Axis::Orientation::Horizontal) {
				offsetY -= titleRect.height()/2;
				if (labelsPosition == Axis::LabelsPosition::Out)
					offsetY -= labelsOffset + tickLabelsPath.boundingRect().height();
				title->setPosition( QPointF( (rect.topLeft().x() + rect.topRight().x())/2 + titleOffsetX, rect.bottomLeft().y() - offsetY ) );
			} else {
				offsetX -= titleRect.width()/2;
				if (labelsPosition == Axis::LabelsPosition::Out)
					offsetX -= labelsOffset+ tickLabelsPath.boundingRect().width();
				title->setPosition( QPointF( rect.topLeft().x() + offsetX, (rect.topLeft().y() + rect.bottomLeft().y())/2 - titleOffsetY) );
			}
			axisShape.addPath(WorksheetElement::shapeFromPath(title->graphicsItem()->mapToParent(title->graphicsItem()->shape()), linePen));
		}
	}

	boundingRectangle = axisShape.boundingRect();

	//if the axis goes beyond the current bounding box of the plot (too high offset is used, too long labels etc.)
	//request a prepareGeometryChange() for the plot in order to properly keep track of geometry changes
	if (plot)
		plot->prepareGeometryChange();
}

/*!
	paints the content of the axis. Reimplemented from \c QGraphicsItem.
	\sa QGraphicsItem::paint()
 */
void AxisPrivate::paint(QPainter *painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
	Q_UNUSED(option)
	Q_UNUSED(widget)

	if (!isVisible())
		return;

	if (linePath.isEmpty())
		return;

	//draw the line
	if (linePen.style() != Qt::NoPen) {
		painter->setOpacity(lineOpacity);
		painter->setPen(linePen);
		painter->setBrush(Qt::SolidPattern);
		painter->drawPath(linePath);

		//draw the arrow
		if (arrowType != Axis::ArrowType::NoArrow)
			painter->drawPath(arrowPath);
	}

	//draw the major ticks
	if (majorTicksDirection != Axis::noTicks) {
		painter->setOpacity(majorTicksOpacity);
		painter->setPen(majorTicksPen);
		painter->setBrush(Qt::NoBrush);
		painter->drawPath(majorTicksPath);
	}

	//draw the minor ticks
	if (minorTicksDirection != Axis::noTicks) {
		painter->setOpacity(minorTicksOpacity);
		painter->setPen(minorTicksPen);
		painter->setBrush(Qt::NoBrush);
		painter->drawPath(minorTicksPath);
	}

	// draw tick labels
	if (labelsPosition != Axis::LabelsPosition::NoLabels) {
		painter->setOpacity(labelsOpacity);
		painter->setPen(QPen(labelsColor));
		painter->setFont(labelsFont);
		QTextDocument doc;
		doc.setDefaultFont(labelsFont);
		QFontMetrics fm(labelsFont);
		if ((orientation == Axis::Orientation::Horizontal && plot->xRangeFormat() == CartesianPlot::RangeFormat::Numeric) ||
				(orientation == Axis::Orientation::Vertical && plot->yRangeFormat() == CartesianPlot::RangeFormat::Numeric)) {
			//QDEBUG(Q_FUNC_INFO << ", axis tick label strings: " << tickLabelStrings)
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
					QString style("p {color: %1;}");
					doc.setDefaultStyleSheet(style.arg(labelsColor.name()));
					doc.setHtml("<p>" + tickLabelStrings.at(i) + "</p>");
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
	}

	if (m_hovered && !isSelected() && !m_printing) {
		painter->setPen(QPen(QApplication::palette().color(QPalette::Shadow), 2, Qt::SolidLine));
		painter->drawPath(axisShape);
	}

	if (isSelected() && !m_printing) {
		painter->setPen(QPen(QApplication::palette().color(QPalette::Highlight), 2, Qt::SolidLine));
		painter->drawPath(axisShape);
	}
}

void AxisPrivate::contextMenuEvent(QGraphicsSceneContextMenuEvent* event) {
	q->createContextMenu()->exec(event->screenPos());
}

void AxisPrivate::hoverEnterEvent(QGraphicsSceneHoverEvent*) {
	if (!isSelected()) {
		m_hovered = true;
		emit q->hovered();
		update(axisShape.boundingRect());
	}
}

void AxisPrivate::hoverLeaveEvent(QGraphicsSceneHoverEvent*) {
	if (m_hovered) {
		m_hovered = false;
		emit q->unhovered();
		update(axisShape.boundingRect());
	}
}

void AxisPrivate::mousePressEvent(QGraphicsSceneMouseEvent* event) {
	auto* plot = static_cast<CartesianPlot*>(q->parentAspect());
	if (!plot->isLocked()) {
		m_panningStarted = true;
		m_panningStart = event->pos();
	} else
		QGraphicsItem::mousePressEvent(event);
}

void AxisPrivate::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
	if (m_panningStarted) {
		if (orientation == WorksheetElement::Orientation::Horizontal) {
			setCursor(Qt::SizeHorCursor);
			const int deltaXScene = (m_panningStart.x() - event->pos().x());
			if (abs(deltaXScene) < 5)
				return;

			auto* plot = static_cast<CartesianPlot*>(q->parentAspect());
			if (deltaXScene > 0)
				plot->shiftRightX();
			else
				plot->shiftLeftX();
		} else {
			setCursor(Qt::SizeVerCursor);
			const int deltaYScene = (m_panningStart.y() - event->pos().y());
			if (abs(deltaYScene) < 5)
				return;

			auto* plot = static_cast<CartesianPlot*>(q->parentAspect());
			if (deltaYScene > 0)
				plot->shiftUpY();
			else
				plot->shiftDownY();
		}

		m_panningStart = event->pos();
	}
}


void AxisPrivate::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
	setCursor(Qt::ArrowCursor);
	m_panningStarted = false;
	QGraphicsItem::mouseReleaseEvent(event);
}

void AxisPrivate::setPrinting(bool on) {
	m_printing = on;
}

bool AxisPrivate::isHovered() const {
	return m_hovered;
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################
//! Save as XML
void Axis::save(QXmlStreamWriter* writer) const {
	Q_D(const Axis);

	writer->writeStartElement("axis");
	writeBasicAttributes(writer);
	writeCommentElement(writer);

	//general
	writer->writeStartElement( "general" );
	writer->writeAttribute( "autoScale", QString::number(d->autoScale) );
	writer->writeAttribute( "orientation", QString::number(static_cast<int>(d->orientation)) );
	writer->writeAttribute( "position", QString::number(static_cast<int>(d->position)) );
	writer->writeAttribute( "scale", QString::number(static_cast<int>(d->scale)) );
	writer->writeAttribute( "offset", QString::number(d->offset) );
	writer->writeAttribute( "start", QString::number(d->range.start()) );
	writer->writeAttribute( "end", QString::number(d->range.end()) );
	writer->writeAttribute( "scalingFactor", QString::number(d->scalingFactor) );
	writer->writeAttribute( "zeroOffset", QString::number(d->zeroOffset) );
	writer->writeAttribute( "titleOffsetX", QString::number(d->titleOffsetX) );
	writer->writeAttribute( "titleOffsetY", QString::number(d->titleOffsetY) );
	writer->writeAttribute( "visible", QString::number(d->isVisible()) );
	writer->writeEndElement();

	//label
	d->title->save(writer);

	//line
	writer->writeStartElement( "line" );
	WRITE_QPEN(d->linePen);
	writer->writeAttribute( "opacity", QString::number(d->lineOpacity) );
	writer->writeAttribute( "arrowType", QString::number(static_cast<int>(d->arrowType)) );
	writer->writeAttribute( "arrowPosition", QString::number(static_cast<int>(d->arrowPosition)) );
	writer->writeAttribute( "arrowSize", QString::number(d->arrowSize) );
	writer->writeEndElement();

	//major ticks
	writer->writeStartElement( "majorTicks" );
	writer->writeAttribute( "direction", QString::number(d->majorTicksDirection) );
	writer->writeAttribute( "type", QString::number(static_cast<int>(d->majorTicksType)) );
	writer->writeAttribute( "number", QString::number(d->majorTicksNumber) );
	writer->writeAttribute( "increment", QString::number(d->majorTicksSpacing) );
	WRITE_COLUMN(d->majorTicksColumn, majorTicksColumn);
	writer->writeAttribute( "length", QString::number(d->majorTicksLength) );
	WRITE_QPEN(d->majorTicksPen);
	writer->writeAttribute( "opacity", QString::number(d->majorTicksOpacity) );
	writer->writeEndElement();

	//minor ticks
	writer->writeStartElement( "minorTicks" );
	writer->writeAttribute( "direction", QString::number(d->minorTicksDirection) );
	writer->writeAttribute( "type", QString::number(static_cast<int>(d->minorTicksType)) );
	writer->writeAttribute( "number", QString::number(d->minorTicksNumber) );
	writer->writeAttribute( "increment", QString::number(d->minorTicksIncrement) );
	WRITE_COLUMN(d->minorTicksColumn, minorTicksColumn);
	writer->writeAttribute( "length", QString::number(d->minorTicksLength) );
	WRITE_QPEN(d->minorTicksPen);
	writer->writeAttribute( "opacity", QString::number(d->minorTicksOpacity) );
	writer->writeEndElement();

	//extra ticks

	//labels
	writer->writeStartElement( "labels" );
	writer->writeAttribute( "position", QString::number(static_cast<int>(d->labelsPosition)) );
	writer->writeAttribute( "offset", QString::number(d->labelsOffset) );
	writer->writeAttribute( "rotation", QString::number(d->labelsRotationAngle) );
	writer->writeAttribute( "format", QString::number(static_cast<int>(d->labelsFormat)) );
	writer->writeAttribute( "precision", QString::number(d->labelsPrecision) );
	writer->writeAttribute( "autoPrecision", QString::number(d->labelsAutoPrecision) );
	writer->writeAttribute( "dateTimeFormat", d->labelsDateTimeFormat );
	WRITE_QCOLOR(d->labelsColor);
	WRITE_QFONT(d->labelsFont);
	writer->writeAttribute( "prefix", d->labelsPrefix );
	writer->writeAttribute( "suffix", d->labelsSuffix );
	writer->writeAttribute( "opacity", QString::number(d->labelsOpacity) );
	writer->writeAttribute( "backgroundType", QString::number(static_cast<int>(d->labelsBackgroundType)) );
	writer->writeAttribute( "backgroundColor_r", QString::number(d->labelsBackgroundColor.red()) );
	writer->writeAttribute( "backgroundColor_g", QString::number(d->labelsBackgroundColor.green()) );
	writer->writeAttribute( "backgroundColor_b", QString::number(d->labelsBackgroundColor.blue()) );
	writer->writeEndElement();

	//grid
	writer->writeStartElement( "majorGrid" );
	WRITE_QPEN(d->majorGridPen);
	writer->writeAttribute( "opacity", QString::number(d->majorGridOpacity) );
	writer->writeEndElement();

	writer->writeStartElement( "minorGrid" );
	WRITE_QPEN(d->minorGridPen);
	writer->writeAttribute( "opacity", QString::number(d->minorGridOpacity) );
	writer->writeEndElement();

	writer->writeEndElement(); // close "axis" section
}

//! Load from XML
bool Axis::load(XmlStreamReader* reader, bool preview) {
	Q_D(Axis);

	if (!readBasicAttributes(reader))
		return false;

	KLocalizedString attributeWarning = ki18n("Attribute '%1' missing or empty, default value is used");
	QXmlStreamAttributes attribs;
	QString str;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == "axis")
			break;

		if (!reader->isStartElement())
			continue;

		if (!preview && reader->name() == "comment") {
			if (!readCommentElement(reader)) return false;
		} else if (!preview && reader->name() == "general") {
			attribs = reader->attributes();

			READ_INT_VALUE("autoScale", autoScale, bool);
			READ_INT_VALUE("orientation", orientation, Orientation);
			READ_INT_VALUE("position", position, Axis::Position);
			READ_INT_VALUE("scale", scale, Axis::Scale);
			READ_DOUBLE_VALUE("offset", offset);
			READ_DOUBLE_VALUE("start", range.start());
			READ_DOUBLE_VALUE("end", range.end());
			READ_DOUBLE_VALUE("scalingFactor", scalingFactor);
			READ_DOUBLE_VALUE("zeroOffset", zeroOffset);
			READ_DOUBLE_VALUE("titleOffsetX", titleOffsetX);
			READ_DOUBLE_VALUE("titleOffsetY", titleOffsetY);

			str = attribs.value("visible").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("visible").toString());
			else
				d->setVisible(str.toInt());
		} else if (reader->name() == "textLabel") {
			d->title->load(reader, preview);
		} else if (!preview && reader->name() == "line") {
			attribs = reader->attributes();

			READ_QPEN(d->linePen);
			READ_DOUBLE_VALUE("opacity", lineOpacity);
			READ_INT_VALUE("arrowType", arrowType, Axis::ArrowType);
			READ_INT_VALUE("arrowPosition", arrowPosition, Axis::ArrowPosition);
			READ_DOUBLE_VALUE("arrowSize", arrowSize);
		} else if (!preview && reader->name() == "majorTicks") {
			attribs = reader->attributes();

			READ_INT_VALUE("direction", majorTicksDirection, Axis::TicksDirection);
			READ_INT_VALUE("type", majorTicksType, Axis::TicksType);
			READ_INT_VALUE("number", majorTicksNumber, int);
			READ_DOUBLE_VALUE("increment", majorTicksSpacing);
			READ_COLUMN(majorTicksColumn);
			READ_DOUBLE_VALUE("length", majorTicksLength);
			READ_QPEN(d->majorTicksPen);
			READ_DOUBLE_VALUE("opacity", majorTicksOpacity);
		} else if (!preview && reader->name() == "minorTicks") {
			attribs = reader->attributes();

			READ_INT_VALUE("direction", minorTicksDirection, Axis::TicksDirection);
			READ_INT_VALUE("type", minorTicksType, Axis::TicksType);
			READ_INT_VALUE("number", minorTicksNumber, int);
			READ_DOUBLE_VALUE("increment", minorTicksIncrement);
			READ_COLUMN(minorTicksColumn);
			READ_DOUBLE_VALUE("length", minorTicksLength);
			READ_QPEN(d->minorTicksPen);
			READ_DOUBLE_VALUE("opacity", minorTicksOpacity);
		} else if (!preview && reader->name() == "labels") {
			attribs = reader->attributes();

			READ_INT_VALUE("position", labelsPosition, Axis::LabelsPosition);
			READ_DOUBLE_VALUE("offset", labelsOffset);
			READ_DOUBLE_VALUE("rotation", labelsRotationAngle);
			READ_INT_VALUE("format", labelsFormat, Axis::LabelsFormat);
			READ_INT_VALUE("precision", labelsPrecision, int);
			READ_INT_VALUE("autoPrecision", labelsAutoPrecision, bool);
			d->labelsDateTimeFormat = attribs.value("dateTimeFormat").toString();
			READ_QCOLOR(d->labelsColor);
			READ_QFONT(d->labelsFont);

			//don't produce any warning if no prefix or suffix is set (empty string is allowed here in xml)
			d->labelsPrefix = attribs.value("prefix").toString();
			d->labelsSuffix = attribs.value("suffix").toString();

			READ_DOUBLE_VALUE("opacity", labelsOpacity);

			READ_INT_VALUE("backgroundType", labelsBackgroundType, Axis::LabelsBackgroundType);
			str = attribs.value("backgroundColor_r").toString();
			if(!str.isEmpty())
				d->labelsBackgroundColor.setRed(str.toInt());

			str = attribs.value("backgroundColor_g").toString();
			if(!str.isEmpty())
				d->labelsBackgroundColor.setGreen(str.toInt());

			str = attribs.value("backgroundColor_b").toString();
			if(!str.isEmpty())
				d->labelsBackgroundColor.setBlue(str.toInt());
		} else if (!preview && reader->name() == "majorGrid") {
			attribs = reader->attributes();

			READ_QPEN(d->majorGridPen);
			READ_DOUBLE_VALUE("opacity", majorGridOpacity);
		} else if (!preview && reader->name() == "minorGrid") {
			attribs = reader->attributes();

			READ_QPEN(d->minorGridPen);
			READ_DOUBLE_VALUE("opacity", minorGridOpacity);
		} else { // unknown element
			reader->raiseWarning(i18n("unknown element '%1'", reader->name().toString()));
			if (!reader->skipToEndElement()) return false;
		}
	}

	return true;
}

//##############################################################################
//#########################  Theme management ##################################
//##############################################################################
void Axis::loadThemeConfig(const KConfig& config) {
	const KConfigGroup& group = config.group("Axis");

	//we don't want to show the major and minor grid lines for non-first horizontal/vertical axes
	//determine the index of the axis among other axes having the same orientation
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

	QPen p;

	// Tick label
	this->setLabelsColor(group.readEntry("LabelsFontColor", QColor(Qt::black)));
	this->setLabelsOpacity(group.readEntry("LabelsOpacity", 1.0));

	//use plot area color for the background color of the labels
	const KConfigGroup& groupPlot = config.group("CartesianPlot");
	this->setLabelsBackgroundColor(groupPlot.readEntry("BackgroundFirstColor", QColor(Qt::white)));

	//Line
	this->setLineOpacity(group.readEntry("LineOpacity", 1.0));
	p.setStyle((Qt::PenStyle)group.readEntry("LineStyle", (int)Qt::SolidLine));
	p.setColor(group.readEntry("LineColor", QColor(Qt::black)));
	p.setWidthF(group.readEntry("LineWidth", Worksheet::convertToSceneUnits(1.0, Worksheet::Unit::Point)));
	this->setLinePen(p);

	//Major grid
	if (firstAxis) {
		p.setStyle((Qt::PenStyle)group.readEntry("MajorGridStyle", (int)Qt::SolidLine));
		p.setColor(group.readEntry("MajorGridColor", QColor(Qt::gray)));
		p.setWidthF(group.readEntry("MajorGridWidth", Worksheet::convertToSceneUnits(1.0, Worksheet::Unit::Point)));
	} else
		p.setStyle(Qt::NoPen);
	this->setMajorGridPen(p);
	this->setMajorGridOpacity(group.readEntry("MajorGridOpacity", 1.0));

	//Major ticks
	p.setStyle((Qt::PenStyle)group.readEntry("MajorTicksLineStyle", (int)Qt::SolidLine));
	p.setColor(group.readEntry("MajorTicksColor", QColor(Qt::black)));
	p.setWidthF(group.readEntry("MajorTicksWidth", Worksheet::convertToSceneUnits(1.0, Worksheet::Unit::Point)));
	this->setMajorTicksPen(p);
	this->setMajorTicksOpacity(group.readEntry("MajorTicksOpacity", 1.0));

	//Minor grid
	if (firstAxis) {
		p.setStyle((Qt::PenStyle)group.readEntry("MinorGridStyle", (int)Qt::DotLine));
		p.setColor(group.readEntry("MinorGridColor", QColor(Qt::gray)));
		p.setWidthF(group.readEntry("MinorGridWidth", Worksheet::convertToSceneUnits(1.0, Worksheet::Unit::Point)));
	} else
		p.setStyle(Qt::NoPen);
	this->setMinorGridOpacity(group.readEntry("MinorGridOpacity", 1.0));
	this->setMinorGridPen(p);

	//Minor ticks
	p.setStyle((Qt::PenStyle)group.readEntry("MinorTicksLineStyle", (int)Qt::SolidLine));
	p.setColor(group.readEntry("MinorTicksColor", QColor(Qt::black)));
	p.setWidthF(group.readEntry("MinorTicksWidth", Worksheet::convertToSceneUnits(1.0, Worksheet::Unit::Point)));
	this->setMinorTicksPen(p);
	this->setMinorTicksOpacity(group.readEntry("MinorTicksOpacity", 1.0));

	//load the theme for the title label
	Q_D(Axis);
	d->title->loadThemeConfig(config);
}

void Axis::saveThemeConfig(const KConfig& config) {
	KConfigGroup group = config.group("Axis");

	// Tick label
	group.writeEntry("LabelsFontColor", this->labelsColor());
	group.writeEntry("LabelsOpacity", this->labelsOpacity());
	group.writeEntry("LabelsBackgroundColor", this->labelsBackgroundColor());

	//Line
	group.writeEntry("LineOpacity", this->lineOpacity());
	group.writeEntry("LineColor", this->linePen().color());
	group.writeEntry("LineStyle", (int) this->linePen().style());
	group.writeEntry("LineWidth", this->linePen().widthF());

	//Major ticks
	group.writeEntry("MajorGridOpacity", this->majorGridOpacity());
	group.writeEntry("MajorGridColor", this->majorGridPen().color());
	group.writeEntry("MajorGridStyle", (int) this->majorGridPen().style());
	group.writeEntry("MajorGridWidth", this->majorGridPen().widthF());
	group.writeEntry("MajorTicksColor", this->majorTicksPen().color());
	group.writeEntry("MajorTicksLineStyle", (int) this->majorTicksPen().style());
	group.writeEntry("MajorTicksWidth", this->majorTicksPen().widthF());
	group.writeEntry("MajorTicksOpacity", this->majorTicksOpacity());
	group.writeEntry("MajorTicksType", (int)this->majorTicksType());

	//Minor ticks
	group.writeEntry("MinorGridOpacity", this->minorGridOpacity());
	group.writeEntry("MinorGridColor", this->minorGridPen().color());
	group.writeEntry("MinorGridStyle", (int) this->minorGridPen().style());
	group.writeEntry("MinorGridWidth", this->minorGridPen().widthF());
	group.writeEntry("MinorTicksColor", this->minorTicksPen().color());
	group.writeEntry("MinorTicksLineStyle", (int) this->minorTicksPen().style());
	group.writeEntry("MinorTicksWidth", this->minorTicksPen().widthF());
	group.writeEntry("MinorTicksOpacity", this->minorTicksOpacity());
	group.writeEntry("MinorTicksType", (int)this->minorTicksType());

	//same the theme config for the title label
	Q_D(Axis);
	d->title->saveThemeConfig(config);
}
