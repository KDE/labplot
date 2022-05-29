/*
	File                 : ReferenceRange.cpp
	Project              : LabPlot
	Description          : Reference range on the plot
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ReferenceRange.h"
#include "ReferenceRangePrivate.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/commandtemplates.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/CartesianCoordinateSystem.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "kdefrontend/GuiTools.h"

#include <QActionGroup>
#include <QGraphicsSceneMouseEvent>
#include <QMenu>
#include <QPainter>

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>

/**
 * \class ReferenceRange
 * \brief A customizable point.
 *
 * The position can be either specified by mouse events or by providing the
 * x- and y- coordinates in parent's coordinate system
 */

ReferenceRange::ReferenceRange(CartesianPlot* plot, const QString& name)
	: WorksheetElement(name, new ReferenceRangePrivate(this), AspectType::ReferenceRange) {
	m_plot = plot;
	init();
}

// no need to delete the d-pointer here - it inherits from QGraphicsItem
// and is deleted during the cleanup in QGraphicsScene
ReferenceRange::~ReferenceRange() = default;

void ReferenceRange::init() {
	Q_D(ReferenceRange);

	KConfig config;
	KConfigGroup group = config.group("ReferenceRange");

	d->coordinateBindingEnabled = true;
	d->orientation = (Orientation)group.readEntry("Orientation", static_cast<int>(Orientation::Vertical));

	// default position - 10% of the plot width/height positioned around the center
	auto cs = plot()->coordinateSystem(coordinateSystemIndex());
	const auto x = m_plot->xRange(cs->xIndex()).center();
	const auto y = m_plot->yRange(cs->yIndex()).center();
	const auto w = m_plot->xRange(cs->xIndex()).length() * 0.1;
	const auto h = m_plot->yRange(cs->yIndex()).length() * 0.1;
	d->positionLogical = QPointF(x, y);
	d->positionLogicalStart = QPointF(x - w / 2, y - h / 2);
	d->positionLogicalEnd = QPointF(x + w / 2, y + h / 2);
	d->updatePosition(); // to update also scene coordinates

	// background
	d->backgroundType = (WorksheetElement::BackgroundType)group.readEntry("BackgroundType", static_cast<int>(BackgroundType::Color));
	d->backgroundColorStyle =
		(WorksheetElement::BackgroundColorStyle)group.readEntry("BackgroundColorStyle", static_cast<int>(BackgroundColorStyle::SingleColor));
	d->backgroundImageStyle = (WorksheetElement::BackgroundImageStyle)group.readEntry("BackgroundImageStyle", static_cast<int>(BackgroundImageStyle::Scaled));
	d->backgroundBrushStyle = (Qt::BrushStyle)group.readEntry("BackgroundBrushStyle", static_cast<int>(Qt::SolidPattern));
	d->backgroundFileName = group.readEntry("BackgroundFileName", QString());
	d->backgroundFirstColor = group.readEntry("BackgroundFirstColor", QColor(Qt::white));
	d->backgroundSecondColor = group.readEntry("BackgroundSecondColor", QColor(Qt::black));
	d->backgroundOpacity = group.readEntry("BackgroundOpacity", 1.0);

	// border
	d->borderPen = QPen(group.readEntry("BorderColor", QColor(Qt::black)),
						group.readEntry("BorderWidth", Worksheet::convertToSceneUnits(1.0, Worksheet::Unit::Point)),
						(Qt::PenStyle)group.readEntry("BorderStyle", (int)Qt::SolidLine));
	d->borderOpacity = group.readEntry("BorderOpacity", 1.0);

	connect(this, &WorksheetElement::positionLogicalChanged, this, &ReferenceRange::updateStartEndPositions);
}

/*!
	Returns an icon to be used in the project explorer.
*/
QIcon ReferenceRange::icon() const {
	return QIcon::fromTheme(QLatin1String("draw-rectangle"));
}

void ReferenceRange::initActions() {
	visibilityAction = new QAction(i18n("Visible"), this);
	visibilityAction->setCheckable(true);
	connect(visibilityAction, &QAction::triggered, this, &ReferenceRange::visibilityChangedSlot);

	// Orientation
	auto* orientationActionGroup = new QActionGroup(this);
	orientationActionGroup->setExclusive(true);
	connect(orientationActionGroup, &QActionGroup::triggered, this, &ReferenceRange::orientationChangedSlot);

	orientationHorizontalAction = new QAction(QIcon::fromTheme(QLatin1String("labplot-axis-horizontal")), i18n("Horizontal"), orientationActionGroup);
	orientationHorizontalAction->setCheckable(true);

	orientationVerticalAction = new QAction(QIcon::fromTheme(QLatin1String("labplot-axis-vertical")), i18n("Vertical"), orientationActionGroup);
	orientationVerticalAction->setCheckable(true);

	// Line
	lineStyleActionGroup = new QActionGroup(this);
	lineStyleActionGroup->setExclusive(true);
	connect(lineStyleActionGroup, &QActionGroup::triggered, this, &ReferenceRange::lineStyleChanged);

	lineColorActionGroup = new QActionGroup(this);
	lineColorActionGroup->setExclusive(true);
	connect(lineColorActionGroup, &QActionGroup::triggered, this, &ReferenceRange::lineColorChanged);
}

void ReferenceRange::initMenus() {
	this->initActions();

	// Orientation
	orientationMenu = new QMenu(i18n("Orientation"));
	orientationMenu->setIcon(QIcon::fromTheme(QLatin1String("labplot-axis-horizontal")));
	orientationMenu->addAction(orientationHorizontalAction);
	orientationMenu->addAction(orientationVerticalAction);

	// Line
	lineMenu = new QMenu(i18n("Border"));
	lineMenu->setIcon(QIcon::fromTheme(QLatin1String("draw-line")));
	lineStyleMenu = new QMenu(i18n("Style"), lineMenu);
	lineStyleMenu->setIcon(QIcon::fromTheme(QLatin1String("object-stroke-style")));
	lineMenu->setIcon(QIcon::fromTheme(QLatin1String("draw-line")));
	lineMenu->addMenu(lineStyleMenu);

	lineColorMenu = new QMenu(i18n("Color"), lineMenu);
	lineColorMenu->setIcon(QIcon::fromTheme(QLatin1String("fill-color")));
	GuiTools::fillColorMenu(lineColorMenu, lineColorActionGroup);
	lineMenu->addMenu(lineColorMenu);
}

QMenu* ReferenceRange::createContextMenu() {
	if (!orientationMenu)
		initMenus();

	QMenu* menu = WorksheetElement::createContextMenu();
	QAction* firstAction = menu->actions().at(1); // skip the first action because of the "title-action"
	visibilityAction->setChecked(isVisible());
	menu->insertAction(firstAction, visibilityAction);

	Q_D(const ReferenceRange);

	// Orientation
	if (d->orientation == Orientation::Horizontal)
		orientationHorizontalAction->setChecked(true);
	else
		orientationVerticalAction->setChecked(true);
	menu->insertMenu(firstAction, orientationMenu);

	// Border line styles
	GuiTools::updatePenStyles(lineStyleMenu, lineStyleActionGroup, d->borderPen.color());
	GuiTools::selectPenStyleAction(lineStyleActionGroup, d->borderPen.style());
	GuiTools::selectColorAction(lineColorActionGroup, d->borderPen.color());

	menu->insertMenu(firstAction, lineMenu);
	menu->insertSeparator(firstAction);

	return menu;
}

QGraphicsItem* ReferenceRange::graphicsItem() const {
	return d_ptr;
}

void ReferenceRange::retransform() {
	Q_D(ReferenceRange);
	d->retransform();
}

void ReferenceRange::handleResize(double /*horizontalRatio*/, double /*verticalRatio*/, bool /*pageResize*/) {
}

/* ============================ getter methods ================= */
BASIC_SHARED_D_READER_IMPL(ReferenceRange, ReferenceRange::Orientation, orientation, orientation)
BASIC_SHARED_D_READER_IMPL(ReferenceRange, QPointF, positionLogicalStart, positionLogicalStart)
BASIC_SHARED_D_READER_IMPL(ReferenceRange, QPointF, positionLogicalEnd, positionLogicalEnd)

BASIC_SHARED_D_READER_IMPL(ReferenceRange, WorksheetElement::BackgroundType, backgroundType, backgroundType)
BASIC_SHARED_D_READER_IMPL(ReferenceRange, WorksheetElement::BackgroundColorStyle, backgroundColorStyle, backgroundColorStyle)
BASIC_SHARED_D_READER_IMPL(ReferenceRange, WorksheetElement::BackgroundImageStyle, backgroundImageStyle, backgroundImageStyle)
BASIC_SHARED_D_READER_IMPL(ReferenceRange, Qt::BrushStyle, backgroundBrushStyle, backgroundBrushStyle)
BASIC_SHARED_D_READER_IMPL(ReferenceRange, QColor, backgroundFirstColor, backgroundFirstColor)
BASIC_SHARED_D_READER_IMPL(ReferenceRange, QColor, backgroundSecondColor, backgroundSecondColor)
BASIC_SHARED_D_READER_IMPL(ReferenceRange, QString, backgroundFileName, backgroundFileName)
BASIC_SHARED_D_READER_IMPL(ReferenceRange, qreal, backgroundOpacity, backgroundOpacity)

BASIC_SHARED_D_READER_IMPL(ReferenceRange, QPen, borderPen, borderPen)
BASIC_SHARED_D_READER_IMPL(ReferenceRange, qreal, borderOpacity, borderOpacity)

/* ============================ setter methods and undo commands ================= */
STD_SETTER_CMD_IMPL_F_S(ReferenceRange, SetOrientation, ReferenceRange::Orientation, orientation, retransform)
void ReferenceRange::setOrientation(Orientation orientation) {
	Q_D(ReferenceRange);
	if (orientation != d->orientation)
		exec(new ReferenceRangeSetOrientationCmd(d, orientation, ki18n("%1: set orientation")));
}

STD_SETTER_CMD_IMPL_F_S(ReferenceRange, SetPositionLogicalStart, QPointF, positionLogicalStart, retransform)
void ReferenceRange::setPositionLogicalStart(QPointF pos) {
	Q_D(ReferenceRange);
	if (pos != d->positionLogicalStart)
		exec(new ReferenceRangeSetPositionLogicalStartCmd(d, pos, ki18n("%1: set start logical position")));
}

STD_SETTER_CMD_IMPL_F_S(ReferenceRange, SetPositionLogicalEnd, QPointF, positionLogicalEnd, retransform)
void ReferenceRange::setPositionLogicalEnd(QPointF pos) {
	Q_D(ReferenceRange);
	if (pos != d->positionLogicalEnd)
		exec(new ReferenceRangeSetPositionLogicalEndCmd(d, pos, ki18n("%1: set end logical position")));
}

// Background
STD_SETTER_CMD_IMPL_F_S(ReferenceRange, SetBackgroundType, WorksheetElement::BackgroundType, backgroundType, update)
void ReferenceRange::setBackgroundType(WorksheetElement::BackgroundType type) {
	Q_D(ReferenceRange);
	if (type != d->backgroundType)
		exec(new ReferenceRangeSetBackgroundTypeCmd(d, type, ki18n("%1: background type changed")));
}

STD_SETTER_CMD_IMPL_F_S(ReferenceRange, SetBackgroundColorStyle, WorksheetElement::BackgroundColorStyle, backgroundColorStyle, update)
void ReferenceRange::setBackgroundColorStyle(WorksheetElement::BackgroundColorStyle style) {
	Q_D(ReferenceRange);
	if (style != d->backgroundColorStyle)
		exec(new ReferenceRangeSetBackgroundColorStyleCmd(d, style, ki18n("%1: background color style changed")));
}

STD_SETTER_CMD_IMPL_F_S(ReferenceRange, SetBackgroundImageStyle, WorksheetElement::BackgroundImageStyle, backgroundImageStyle, update)
void ReferenceRange::setBackgroundImageStyle(WorksheetElement::BackgroundImageStyle style) {
	Q_D(ReferenceRange);
	if (style != d->backgroundImageStyle)
		exec(new ReferenceRangeSetBackgroundImageStyleCmd(d, style, ki18n("%1: background image style changed")));
}

STD_SETTER_CMD_IMPL_F_S(ReferenceRange, SetBackgroundBrushStyle, Qt::BrushStyle, backgroundBrushStyle, update)
void ReferenceRange::setBackgroundBrushStyle(Qt::BrushStyle style) {
	Q_D(ReferenceRange);
	if (style != d->backgroundBrushStyle)
		exec(new ReferenceRangeSetBackgroundBrushStyleCmd(d, style, ki18n("%1: background brush style changed")));
}

STD_SETTER_CMD_IMPL_F_S(ReferenceRange, SetBackgroundFirstColor, QColor, backgroundFirstColor, update)
void ReferenceRange::setBackgroundFirstColor(const QColor& color) {
	Q_D(ReferenceRange);
	if (color != d->backgroundFirstColor)
		exec(new ReferenceRangeSetBackgroundFirstColorCmd(d, color, ki18n("%1: set background first color")));
}

STD_SETTER_CMD_IMPL_F_S(ReferenceRange, SetBackgroundSecondColor, QColor, backgroundSecondColor, update)
void ReferenceRange::setBackgroundSecondColor(const QColor& color) {
	Q_D(ReferenceRange);
	if (color != d->backgroundSecondColor)
		exec(new ReferenceRangeSetBackgroundSecondColorCmd(d, color, ki18n("%1: set background second color")));
}

STD_SETTER_CMD_IMPL_F_S(ReferenceRange, SetBackgroundFileName, QString, backgroundFileName, update)
void ReferenceRange::setBackgroundFileName(const QString& fileName) {
	Q_D(ReferenceRange);
	if (fileName != d->backgroundFileName)
		exec(new ReferenceRangeSetBackgroundFileNameCmd(d, fileName, ki18n("%1: set background image")));
}

STD_SETTER_CMD_IMPL_F_S(ReferenceRange, SetBackgroundOpacity, qreal, backgroundOpacity, update)
void ReferenceRange::setBackgroundOpacity(qreal opacity) {
	Q_D(ReferenceRange);
	if (opacity != d->backgroundOpacity)
		exec(new ReferenceRangeSetBackgroundOpacityCmd(d, opacity, ki18n("%1: set plot area opacity")));
}

// Border
STD_SETTER_CMD_IMPL_F_S(ReferenceRange, SetBorderPen, QPen, borderPen, update)
void ReferenceRange::setBorderPen(const QPen& pen) {
	Q_D(ReferenceRange);
	if (pen != d->borderPen)
		exec(new ReferenceRangeSetBorderPenCmd(d, pen, ki18n("%1: set plot area border")));
}

STD_SETTER_CMD_IMPL_F_S(ReferenceRange, SetBorderOpacity, qreal, borderOpacity, update)
void ReferenceRange::setBorderOpacity(qreal opacity) {
	Q_D(ReferenceRange);
	if (opacity != d->borderOpacity)
		exec(new ReferenceRangeSetBorderOpacityCmd(d, opacity, ki18n("%1: set plot area border opacity")));
}

//##############################################################################
//######  SLOTs for changes triggered via QActions in the context menu  ########
//##############################################################################
void ReferenceRange::orientationChangedSlot(QAction* action) {
	if (action == orientationHorizontalAction)
		this->setOrientation(Orientation::Horizontal);
	else
		this->setOrientation(Orientation::Vertical);
}

void ReferenceRange::lineStyleChanged(QAction* action) {
	Q_D(const ReferenceRange);
	QPen pen = d->borderPen;
	pen.setStyle(GuiTools::penStyleFromAction(lineStyleActionGroup, action));
	this->setBorderPen(pen);
}

void ReferenceRange::lineColorChanged(QAction* action) {
	Q_D(const ReferenceRange);
	QPen pen = d->borderPen;
	pen.setColor(GuiTools::colorFromAction(lineColorActionGroup, action));
	this->setBorderPen(pen);
}

void ReferenceRange::visibilityChangedSlot() {
	Q_D(const ReferenceRange);
	this->setVisible(!d->isVisible());
}

//##############################################################################
//####################### Private implementation ###############################
//##############################################################################
ReferenceRangePrivate::ReferenceRangePrivate(ReferenceRange* owner)
	: WorksheetElementPrivate(owner)
	, q(owner) {
	setFlag(QGraphicsItem::ItemSendsGeometryChanges);
	setFlag(QGraphicsItem::ItemIsMovable);
	setFlag(QGraphicsItem::ItemIsSelectable);
	setFlag(QGraphicsItem::ItemIsFocusable);
	setAcceptHoverEvents(true);
}

/*!
	calculates the position and the bounding box of the item/point. Called on geometry or properties changes.
 */
void ReferenceRangePrivate::retransform() {
	if (suppressRetransform || !q->cSystem || q->isLoading())
		return;

	// it should be enough to set the position limits in init() and in setOrientation() only
	// but since we don't do it during undo/redo of setOrientation(), we end up having wrong
	// limits after undo/redo. So, we set it here in retransform again.
	switch (orientation) {
	case ReferenceRange::Orientation::Horizontal:
		position.positionLimit = WorksheetElement::PositionLimit::Y;
		break;
	case ReferenceRange::Orientation::Vertical:
		position.positionLimit = WorksheetElement::PositionLimit::X;
		break;
	case ReferenceRange::Orientation::Both:
		position.positionLimit = WorksheetElement::PositionLimit::None;
		break;
	}

	auto cs = q->plot()->coordinateSystem(q->coordinateSystemIndex());
	const auto xRange{q->m_plot->xRange(cs->xIndex())};
	const auto yRange{q->m_plot->yRange(cs->yIndex())};

	// calculate the position in the scene coordinates
	if (orientation == ReferenceRange::Orientation::Vertical) {
		positionLogical = QPointF(positionLogical.x(), yRange.center());
		rect.setX(positionLogicalStart.x());
		rect.setY(yRange.start());
		rect.setWidth(positionLogicalEnd.x() - positionLogicalStart.x());
		rect.setHeight(yRange.length());
	} else {
		positionLogical = QPointF(xRange.center(), positionLogical.y());
		rect.setX(yRange.start());
		rect.setY(positionLogicalStart.y());
		rect.setWidth(xRange.length());
		rect.setHeight(positionLogicalEnd.y() - positionLogicalStart.y());
	}
	updatePosition(); // To update position.point

	prevPositionLogical = positionLogical;

// 	qDebug() << "logical rect " << rect;

	// position.point contains already the scene position, but here it will be determined,
	// if the point lies outside of the datarect or not
	// 	QVector<QPointF> listScene = q->cSystem->mapLogicalToScene(Points() << positionLogical);
	// 	QDEBUG(Q_FUNC_INFO << ", scene list = " << listScene)

	// 	rect = q->cSystem->mapLogicalToScene(rect, &m_visible);

	Lines lines;
	lines << QLineF(rect.topLeft(), rect.topRight());
	lines << QLineF(rect.topRight(), rect.bottomRight());
	lines << QLineF(rect.bottomRight(), rect.bottomLeft());
	lines << QLineF(rect.bottomLeft(), rect.topLeft());
// 	qDebug() << "logical lines " << lines;
	const auto& unclippedLines = q->cSystem->mapLogicalToScene(lines, AbstractCoordinateSystem::MappingFlag::SuppressPageClipping);
// 	qDebug() << "scene lines " << unclippedLines;

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

	rect = polygon.boundingRect();
// 	qDebug() << "scene rect " << rect;

	recalcShapeAndBoundingRect();
}

/*!
 * called when the user moves the graphics item with the mouse and the logical position of the item is changed.
 * Here we update the logical coordinates for the start and end points and notify the dock widget.
 */
// TODO: make this undo/redo-able
void ReferenceRange::updateStartEndPositions(QPointF newPosition) {
	Q_D(ReferenceRange);
	if (d->orientation == WorksheetElement::Orientation::Horizontal) {
		double delta = newPosition.y() - d->prevPositionLogical.y();
		d->positionLogicalStart.setY(d->positionLogicalStart.y() + delta);
		d->positionLogicalEnd.setY(d->positionLogicalEnd.y() + delta);
	} else {
		double delta = newPosition.x() - d->prevPositionLogical.x();
		d->positionLogicalStart.setX(d->positionLogicalStart.x() + delta);
		d->positionLogicalEnd.setX(d->positionLogicalEnd.x() + delta);
	}
	Q_EMIT positionLogicalStartChanged(d->positionLogicalStart);
	Q_EMIT positionLogicalEndChanged(d->positionLogicalEnd);
	d->retransform();
}

/*!
	Returns the outer bounds of the item as a rectangle.
 */
QRectF ReferenceRangePrivate::boundingRect() const {
	return boundingRectangle;
}

/*!
	Returns the shape of this item as a QPainterPath in local coordinates.
*/
QPainterPath ReferenceRangePrivate::shape() const {
	return rangeShape;
}

/*!
  recalculates the outer bounds and the shape of the item.
*/
void ReferenceRangePrivate::recalcShapeAndBoundingRect() {
	prepareGeometryChange();

	rangeShape = QPainterPath();
	if (m_visible) {
		QPainterPath path;
		path.addRect(rect);
		rangeShape.addPath(WorksheetElement::shapeFromPath(path, borderPen));
		boundingRectangle = rangeShape.boundingRect();
	}
}

void ReferenceRangePrivate::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget*) {
	if (!m_visible)
		return;

	// draw the area
	painter->setOpacity(backgroundOpacity);
	painter->setPen(Qt::NoPen);
	if (backgroundType == WorksheetElement::BackgroundType::Color) {
		switch (backgroundColorStyle) {
		case WorksheetElement::BackgroundColorStyle::SingleColor: {
			painter->setBrush(QBrush(backgroundFirstColor));
			break;
		}
		case WorksheetElement::BackgroundColorStyle::HorizontalLinearGradient: {
			QLinearGradient linearGrad(rect.topLeft(), rect.topRight());
			linearGrad.setColorAt(0, backgroundFirstColor);
			linearGrad.setColorAt(1, backgroundSecondColor);
			painter->setBrush(QBrush(linearGrad));
			break;
		}
		case WorksheetElement::BackgroundColorStyle::VerticalLinearGradient: {
			QLinearGradient linearGrad(rect.topLeft(), rect.bottomLeft());
			linearGrad.setColorAt(0, backgroundFirstColor);
			linearGrad.setColorAt(1, backgroundSecondColor);
			painter->setBrush(QBrush(linearGrad));
			break;
		}
		case WorksheetElement::BackgroundColorStyle::TopLeftDiagonalLinearGradient: {
			QLinearGradient linearGrad(rect.topLeft(), rect.bottomRight());
			linearGrad.setColorAt(0, backgroundFirstColor);
			linearGrad.setColorAt(1, backgroundSecondColor);
			painter->setBrush(QBrush(linearGrad));
			break;
		}
		case WorksheetElement::BackgroundColorStyle::BottomLeftDiagonalLinearGradient: {
			QLinearGradient linearGrad(rect.bottomLeft(), rect.topRight());
			linearGrad.setColorAt(0, backgroundFirstColor);
			linearGrad.setColorAt(1, backgroundSecondColor);
			painter->setBrush(QBrush(linearGrad));
			break;
		}
		case WorksheetElement::BackgroundColorStyle::RadialGradient: {
			QRadialGradient radialGrad(rect.center(), rect.width() / 2);
			radialGrad.setColorAt(0, backgroundFirstColor);
			radialGrad.setColorAt(1, backgroundSecondColor);
			painter->setBrush(QBrush(radialGrad));
			break;
		}
		}
	} else if (backgroundType == WorksheetElement::BackgroundType::Image) {
		if (!backgroundFileName.trimmed().isEmpty()) {
			QPixmap pix(backgroundFileName);
			switch (backgroundImageStyle) {
			case WorksheetElement::BackgroundImageStyle::ScaledCropped:
				pix = pix.scaled(rect.size().toSize(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
				painter->setBrush(QBrush(pix));
				painter->setBrushOrigin(pix.size().width() / 2, pix.size().height() / 2);
				break;
			case WorksheetElement::BackgroundImageStyle::Scaled:
				pix = pix.scaled(rect.size().toSize(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
				painter->setBrush(QBrush(pix));
				painter->setBrushOrigin(pix.size().width() / 2, pix.size().height() / 2);
				break;
			case WorksheetElement::BackgroundImageStyle::ScaledAspectRatio:
				pix = pix.scaled(rect.size().toSize(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
				painter->setBrush(QBrush(pix));
				painter->setBrushOrigin(pix.size().width() / 2, pix.size().height() / 2);
				break;
			case WorksheetElement::BackgroundImageStyle::Centered:
				painter->drawPixmap(QPointF(rect.center().x() - pix.size().width() / 2, rect.center().y() - pix.size().height() / 2), pix);
				break;
			case WorksheetElement::BackgroundImageStyle::Tiled:
				painter->setBrush(QBrush(pix));
				break;
			case WorksheetElement::BackgroundImageStyle::CenterTiled:
				painter->setBrush(QBrush(pix));
				painter->setBrushOrigin(pix.size().width() / 2, pix.size().height() / 2);
			}
		}
	} else if (backgroundType == WorksheetElement::BackgroundType::Pattern) {
		painter->setBrush(QBrush(backgroundFirstColor, backgroundBrushStyle));
	}

	// draw the background
	painter->drawRect(rect);

	// draw the border
	if (borderPen.style() != Qt::NoPen) {
		painter->setPen(borderPen);
		painter->setBrush(Qt::NoBrush);
		painter->setOpacity(borderOpacity);
	}
	painter->drawRect(rect);

	if (m_hovered && !isSelected() && !q->isPrinting()) {
		painter->setPen(QPen(QApplication::palette().color(QPalette::Shadow), 2, Qt::SolidLine));
		painter->drawPath(rangeShape);
	}

	if (isSelected() && !q->isPrinting()) {
		painter->setPen(QPen(QApplication::palette().color(QPalette::Highlight), 2, Qt::SolidLine));
		painter->drawPath(rangeShape);
	}
}

void ReferenceRangePrivate::contextMenuEvent(QGraphicsSceneContextMenuEvent* event) {
	q->createContextMenu()->exec(event->screenPos());
}

void ReferenceRangePrivate::hoverEnterEvent(QGraphicsSceneHoverEvent*) {
	if (!isSelected()) {
		m_hovered = true;
		Q_EMIT q->hovered();
		update();
	}
}

void ReferenceRangePrivate::hoverLeaveEvent(QGraphicsSceneHoverEvent*) {
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
void ReferenceRange::save(QXmlStreamWriter* writer) const {
	Q_D(const ReferenceRange);

	writer->writeStartElement("referenceRange");
	writeBasicAttributes(writer);
	writeCommentElement(writer);

	// position and orientation
	writer->writeStartElement("geometry");
	WorksheetElement::save(writer);
	writer->writeAttribute("logicalPosStartX", QString::number(d->positionLogicalStart.x()));
	writer->writeAttribute("logicalPosStartY", QString::number(d->positionLogicalStart.y()));
	writer->writeAttribute("logicalPosEndX", QString::number(d->positionLogicalEnd.x()));
	writer->writeAttribute("logicalPosEndY", QString::number(d->positionLogicalEnd.y()));
	writer->writeAttribute("orientation", QString::number(static_cast<int>(d->orientation)));
	writer->writeEndElement();

	// background
	writer->writeStartElement("background");
	writer->writeAttribute("colorStyle", QString::number(static_cast<int>(d->backgroundColorStyle)));
	writer->writeAttribute("imageStyle", QString::number(static_cast<int>(d->backgroundImageStyle)));
	writer->writeAttribute("brushStyle", QString::number(d->backgroundBrushStyle));
	writer->writeAttribute("firstColor_r", QString::number(d->backgroundFirstColor.red()));
	writer->writeAttribute("firstColor_g", QString::number(d->backgroundFirstColor.green()));
	writer->writeAttribute("firstColor_b", QString::number(d->backgroundFirstColor.blue()));
	writer->writeAttribute("secondColor_r", QString::number(d->backgroundSecondColor.red()));
	writer->writeAttribute("secondColor_g", QString::number(d->backgroundSecondColor.green()));
	writer->writeAttribute("secondColor_b", QString::number(d->backgroundSecondColor.blue()));
	writer->writeAttribute("fileName", d->backgroundFileName);
	writer->writeAttribute("opacity", QString::number(d->backgroundOpacity));
	writer->writeEndElement();

	// border
	writer->writeStartElement("border");
	WRITE_QPEN(d->borderPen);
	writer->writeAttribute("borderOpacity", QString::number(d->borderOpacity));
	writer->writeEndElement();

	writer->writeEndElement(); // close "ReferenceRange" section
}

//! Load from XML
bool ReferenceRange::load(XmlStreamReader* reader, bool preview) {
	Q_D(ReferenceRange);

	if (!readBasicAttributes(reader))
		return false;

	KLocalizedString attributeWarning = ki18n("Attribute '%1' missing or empty, default value is used");
	QXmlStreamAttributes attribs;
	QString str;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == "referenceRange")
			break;

		if (!reader->isStartElement())
			continue;

		if (!preview && reader->name() == "comment") {
			if (!readCommentElement(reader))
				return false;
		} else if (!preview && reader->name() == "geometry") {
			attribs = reader->attributes();
			READ_INT_VALUE("orientation", orientation, Orientation);
			WorksheetElement::load(reader, preview);

			str = attribs.value("logicalPosStartX").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("logicalPosStartX").toString());
			else
				d->positionLogicalStart.setX(str.toDouble());

			str = attribs.value("logicalPosStartY").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("logicalPosStartY").toString());
			else
				d->positionLogicalStart.setY(str.toDouble());

			str = attribs.value("logicalPosEndX").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("logicalPosEndX").toString());
			else
				d->positionLogicalEnd.setX(str.toDouble());

			str = attribs.value("logicalPosEndY").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("logicalPosEndY").toString());
			else
				d->positionLogicalEnd.setY(str.toDouble());
		} else if (!preview && reader->name() == "background") {
			attribs = reader->attributes();

			str = attribs.value("type").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("type").toString());
			else
				d->backgroundType = WorksheetElement::BackgroundType(str.toInt());

			str = attribs.value("colorStyle").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("colorStyle").toString());
			else
				d->backgroundColorStyle = WorksheetElement::BackgroundColorStyle(str.toInt());

			str = attribs.value("imageStyle").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("imageStyle").toString());
			else
				d->backgroundImageStyle = WorksheetElement::BackgroundImageStyle(str.toInt());

			str = attribs.value("brushStyle").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("brushStyle").toString());
			else
				d->backgroundBrushStyle = Qt::BrushStyle(str.toInt());

			str = attribs.value("firstColor_r").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("firstColor_r").toString());
			else
				d->backgroundFirstColor.setRed(str.toInt());

			str = attribs.value("firstColor_g").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("firstColor_g").toString());
			else
				d->backgroundFirstColor.setGreen(str.toInt());

			str = attribs.value("firstColor_b").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("firstColor_b").toString());
			else
				d->backgroundFirstColor.setBlue(str.toInt());

			str = attribs.value("secondColor_r").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("secondColor_r").toString());
			else
				d->backgroundSecondColor.setRed(str.toInt());

			str = attribs.value("secondColor_g").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("secondColor_g").toString());
			else
				d->backgroundSecondColor.setGreen(str.toInt());

			str = attribs.value("secondColor_b").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("secondColor_b").toString());
			else
				d->backgroundSecondColor.setBlue(str.toInt());

			str = attribs.value("fileName").toString();
			d->backgroundFileName = str;

			str = attribs.value("opacity").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("opacity").toString());
			else
				d->backgroundOpacity = str.toDouble();
		} else if (!preview && reader->name() == "border") {
			attribs = reader->attributes();

			READ_QPEN(d->borderPen);

			str = attribs.value("borderOpacity").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("borderOpacity").toString());
			else
				d->borderOpacity = str.toDouble();
		} else { // unknown element
			reader->raiseWarning(i18n("unknown element '%1'", reader->name().toString()));
			if (!reader->skipToEndElement())
				return false;
		}
	}
	return true;
}

//##############################################################################
//#########################  Theme management ##################################
//##############################################################################
void ReferenceRange::loadThemeConfig(const KConfig&) {
	// determine the index of the current range in the list of all range children
	// and apply the theme color for this index
	const auto* plot = dynamic_cast<const CartesianPlot*>(parentAspect());
	if (!plot)
		return;
	int index = 0;
	const auto& children = plot->children<WorksheetElement>();
	for (auto* child : children) {
		if (child == this)
			break;

		if (child->inherits(AspectType::ReferenceRange))
			++index;
	}
	const QColor themeColor = plot->themeColorPalette(index);

	// background
	setBackgroundType(WorksheetElement::BackgroundType::Color);
	setBackgroundColorStyle(WorksheetElement::BackgroundColorStyle::SingleColor);
	setBackgroundBrushStyle(Qt::SolidPattern);
	setBackgroundFirstColor(themeColor);
	setBackgroundOpacity(0.8);

	// border
	QPen p;
	p.setStyle(Qt::NoPen);
	p.setColor(themeColor);
	p.setWidthF(Worksheet::convertToSceneUnits(1.0, Worksheet::Unit::Point));
	setBorderPen(p);
	setBorderOpacity(1.0);
}
