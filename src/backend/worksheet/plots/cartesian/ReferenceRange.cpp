/*
	File                 : ReferenceRange.cpp
	Project              : LabPlot
	Description          : Reference range on the plot
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022-2023 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ReferenceRange.h"
#include "ReferenceRangePrivate.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/commandtemplates.h"
#include "backend/worksheet/Background.h"
#include "backend/worksheet/Line.h"
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
	KConfigGroup group = config.group(QStringLiteral("ReferenceRange"));

	d->coordinateBindingEnabled = true;
	d->orientation = (Orientation)group.readEntry(QStringLiteral("Orientation"), static_cast<int>(Orientation::Vertical));
	switch (d->orientation) {
	case WorksheetElement::Orientation::Horizontal:
		d->position.positionLimit = WorksheetElement::PositionLimit::Y;
		break;
	case WorksheetElement::Orientation::Vertical:
		d->position.positionLimit = WorksheetElement::PositionLimit::X;
		break;
	case WorksheetElement::Orientation::Both:
		d->position.positionLimit = WorksheetElement::PositionLimit::None;
		break;
	}

	// default position - 10% of the plot width/height positioned around the center
	auto cs = plot()->coordinateSystem(coordinateSystemIndex());
	const auto x = m_plot->range(Dimension::X, cs->index(Dimension::X)).center();
	const auto y = m_plot->range(Dimension::Y, cs->index(Dimension::Y)).center();
	const auto w = m_plot->range(Dimension::X, cs->index(Dimension::X)).length() * 0.1;
	const auto h = m_plot->range(Dimension::Y, cs->index(Dimension::Y)).length() * 0.1;
	d->positionLogical = QPointF(x, y);
	d->positionLogicalStart = QPointF(x - w / 2, y - h / 2);
	d->positionLogicalEnd = QPointF(x + w / 2, y + h / 2);
	d->updatePosition(); // to update also scene coordinates

	// background
	d->background = new Background(QString());
	d->background->setEnabledAvailable(true);
	addChild(d->background);
	d->background->setHidden(true);
	d->background->init(group);
	connect(d->background, &Background::updateRequested, [=] {
		d->update();
	});

	// border
	d->line = new Line(QString());
	d->line->setHidden(true);
	addChild(d->line);
	d->line->init(group);
	connect(d->line, &Line::updatePixmapRequested, [=] {
		d->update();
	});
	connect(d->line, &Line::updateRequested, [=] {
		d->recalcShapeAndBoundingRect();
	});

	connect(this, &WorksheetElement::positionLogicalChanged, this, &ReferenceRange::updateStartEndPositions);
}

/*!
	Returns an icon to be used in the project explorer.
*/
QIcon ReferenceRange::icon() const {
	return QIcon::fromTheme(QStringLiteral("draw-rectangle"));
}

void ReferenceRange::initActions() {
	visibilityAction = new QAction(i18n("Visible"), this);
	visibilityAction->setCheckable(true);
	connect(visibilityAction, &QAction::triggered, this, &ReferenceRange::visibilityChangedSlot);

	// Orientation
	auto* orientationActionGroup = new QActionGroup(this);
	orientationActionGroup->setExclusive(true);
	connect(orientationActionGroup, &QActionGroup::triggered, this, &ReferenceRange::orientationChangedSlot);

	orientationHorizontalAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-axis-horizontal")), i18n("Horizontal"), orientationActionGroup);
	orientationHorizontalAction->setCheckable(true);

	orientationVerticalAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-axis-vertical")), i18n("Vertical"), orientationActionGroup);
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
	orientationMenu->setIcon(QIcon::fromTheme(QStringLiteral("labplot-axis-horizontal")));
	orientationMenu->addAction(orientationHorizontalAction);
	orientationMenu->addAction(orientationVerticalAction);

	// Line
	lineMenu = new QMenu(i18n("Border Line"));
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
	const auto& pen = d->line->pen();
	GuiTools::updatePenStyles(lineStyleMenu, lineStyleActionGroup, pen.color());
	GuiTools::selectPenStyleAction(lineStyleActionGroup, pen.style());
	GuiTools::selectColorAction(lineColorActionGroup, pen.color());

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

Line* ReferenceRange::line() const {
	Q_D(const ReferenceRange);
	return d->line;
}

Background* ReferenceRange::background() const {
	Q_D(const ReferenceRange);
	return d->background;
}

/* ============================ setter methods and undo commands ================= */
STD_SETTER_CMD_IMPL_F_S(ReferenceRange, SetOrientation, ReferenceRange::Orientation, orientation, updateOrientation)
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
	d->line->setStyle(GuiTools::penStyleFromAction(lineStyleActionGroup, action));
}

void ReferenceRange::lineColorChanged(QAction* action) {
	Q_D(const ReferenceRange);
	d->line->setColor(GuiTools::colorFromAction(lineColorActionGroup, action));
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

    //updatePosition(); // To update position.point

	// calculate rect in logical coordinates
	auto cs = q->plot()->coordinateSystem(q->coordinateSystemIndex());
	if (orientation == ReferenceRange::Orientation::Vertical) {
		const auto yRange{q->m_plot->range(Dimension::Y, cs->index(Dimension::Y))};
		const auto p1 = QPointF(positionLogicalStart.x(), yRange.start());
		const auto p2 = QPointF(positionLogicalEnd.x(), yRange.end());
		const auto pointsScene = cs->mapLogicalToScene({p1, p2}, AbstractCoordinateSystem::MappingFlag::SuppressPageClipping);
		const auto newPos = QPointF((pointsScene.at(0).x() + pointsScene.at(1).x())/2, (pointsScene.at(0).y() + pointsScene.at(1).y())/2);
		const auto diffX = qAbs(pointsScene.at(0).x() - pointsScene.at(1).x());
		const auto diffY = qAbs(pointsScene.at(0).y() - pointsScene.at(1).y());
		rect.setX(-diffX/2);
		rect.setY(-diffY/2);
		rect.setWidth(diffX);
		rect.setHeight(diffY);
		recalcShapeAndBoundingRect();
        positionLogical = cs->mapSceneToLogical(newPos);
        updatePosition();
	} else {
		const auto xRange{q->m_plot->range(Dimension::X, cs->index(Dimension::X))};
		rect.setX(xRange.start());
		rect.setY(positionLogicalStart.y());
		rect.setWidth(xRange.length());
		rect.setHeight(positionLogicalEnd.y() - positionLogicalStart.y());
	}

	// calculate rect in scene coordinates
	// TODO: taken from BoxPlotPrivate::updateFillingRect(), maybe a more simpler version is possible here
	Lines lines;
	lines << QLineF(rect.topLeft(), rect.topRight());
	lines << QLineF(rect.topRight(), rect.bottomRight());
	lines << QLineF(rect.bottomRight(), rect.bottomLeft());
	lines << QLineF(rect.bottomLeft(), rect.topLeft());
	// const auto& unclippedLines = q->cSystem->mapLogicalToScene(lines, AbstractCoordinateSystem::MappingFlag::SuppressPageClipping);
	const auto& unclippedLines = lines;

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

	recalcShapeAndBoundingRect();
}

void ReferenceRangePrivate::updateOrientation() {
	switch (orientation) {
	case WorksheetElement::Orientation::Horizontal:
		position.positionLimit = WorksheetElement::PositionLimit::Y;
		break;
	case WorksheetElement::Orientation::Vertical:
		position.positionLimit = WorksheetElement::PositionLimit::X;
		break;
	case WorksheetElement::Orientation::Both:
		position.positionLimit = WorksheetElement::PositionLimit::None;
		break;
	}
	retransform();
}

/*!
 * called when the user moves the graphics item with the mouse and the scene position of the item is changed.
 * Here we update the logical coordinates for the start and end points based on the new valud for the logical
 * position \c newPosition of the item's center and notify the dock widget.
 */
// TODO: make this undo/redo-able
void ReferenceRange::updateStartEndPositions(QPointF newPosition) {
	Q_D(ReferenceRange);
	if (d->orientation == WorksheetElement::Orientation::Horizontal) {
		const double width = (d->positionLogicalEnd.y() - d->positionLogicalStart.y()) / 2;
		d->positionLogicalStart.setY(newPosition.y() + width); // y-axis is reversed, change the sign here
		d->positionLogicalEnd.setY(newPosition.y() - width);
	} else {
		const double width = (d->positionLogicalEnd.x() - d->positionLogicalStart.x()) / 2;
		d->positionLogicalStart.setX(newPosition.x() - width);
		d->positionLogicalEnd.setX(newPosition.x() + width);
	}
	Q_EMIT positionLogicalStartChanged(d->positionLogicalStart);
	Q_EMIT positionLogicalEndChanged(d->positionLogicalEnd);
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
		rangeShape.addPath(WorksheetElement::shapeFromPath(path, line->pen()));
		boundingRectangle = rangeShape.boundingRect();
	}
}

void ReferenceRangePrivate::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget*) {
	if (!m_visible)
		return;

	// draw filling
	if (background->enabled()) {
		painter->setOpacity(background->opacity());
		painter->setPen(Qt::NoPen);
		drawFilling(painter);
	}

	// draw the background
	painter->drawRect(rect);

	// draw the border
	if (line->style() != Qt::NoPen) {
		painter->setPen(line->pen());
		painter->setBrush(Qt::NoBrush);
		painter->setOpacity(line->opacity());
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

void ReferenceRangePrivate::drawFilling(QPainter* painter) const {
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
			case Background::ImageStyle::Centered:
				painter->drawPixmap(QPointF(rect.center().x() - pix.size().width() / 2, rect.center().y() - pix.size().height() / 2), pix);
				break;
			case Background::ImageStyle::Tiled:
				painter->setBrush(QBrush(pix));
				break;
			case Background::ImageStyle::CenterTiled:
				painter->setBrush(QBrush(pix));
				painter->setBrushOrigin(pix.size().width() / 2, pix.size().height() / 2);
			}
		}
	} else if (background->type() == Background::Type::Pattern) {
		painter->setBrush(QBrush(background->firstColor(), background->brushStyle()));
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

	writer->writeStartElement(QStringLiteral("referenceRange"));
	writeBasicAttributes(writer);
	writeCommentElement(writer);

	// position and orientation
	writer->writeStartElement(QStringLiteral("geometry"));
	WorksheetElement::save(writer);
	writer->writeAttribute(QStringLiteral("logicalPosStartX"), QString::number(d->positionLogicalStart.x()));
	writer->writeAttribute(QStringLiteral("logicalPosStartY"), QString::number(d->positionLogicalStart.y()));
	writer->writeAttribute(QStringLiteral("logicalPosEndX"), QString::number(d->positionLogicalEnd.x()));
	writer->writeAttribute(QStringLiteral("logicalPosEndY"), QString::number(d->positionLogicalEnd.y()));
	writer->writeAttribute(QStringLiteral("orientation"), QString::number(static_cast<int>(d->orientation)));
	writer->writeEndElement();

	d->background->save(writer);
	d->line->save(writer);

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
		if (reader->isEndElement() && reader->name() == QStringLiteral("referenceRange"))
			break;

		if (!reader->isStartElement())
			continue;

		if (!preview && reader->name() == QStringLiteral("comment")) {
			if (!readCommentElement(reader))
				return false;
		} else if (!preview && reader->name() == QStringLiteral("geometry")) {
			attribs = reader->attributes();
			READ_INT_VALUE("orientation", orientation, Orientation);
			WorksheetElement::load(reader, preview);

			str = attribs.value(QStringLiteral("logicalPosStartX")).toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs(QStringLiteral("logicalPosStartX")).toString());
			else
				d->positionLogicalStart.setX(str.toDouble());

			str = attribs.value(QStringLiteral("logicalPosStartY")).toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs(QStringLiteral("logicalPosStartY")).toString());
			else
				d->positionLogicalStart.setY(str.toDouble());

			str = attribs.value(QStringLiteral("logicalPosEndX")).toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs(QStringLiteral("logicalPosEndX")).toString());
			else
				d->positionLogicalEnd.setX(str.toDouble());

			str = attribs.value(QStringLiteral("logicalPosEndY")).toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs(QStringLiteral("logicalPosEndY")).toString());
			else
				d->positionLogicalEnd.setY(str.toDouble());
		} else if (!preview && reader->name() == QStringLiteral("background"))
			d->background->load(reader, preview);
		else if (!preview && reader->name() == QStringLiteral("line"))
			d->line->load(reader, preview);
		else { // unknown element
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
void ReferenceRange::loadThemeConfig(const KConfig& config) {
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

	const auto& themeColor = plot->themeColorPalette(index);

	KConfigGroup group;
	if (config.hasGroup(QStringLiteral("Theme")))
		group = config.group(QStringLiteral("Axis")); // when loading from the theme config, use the same properties as for Axis
	else
		group = config.group(QStringLiteral("ReferenceRange"));

	Q_D(ReferenceRange);
	d->line->loadThemeConfig(group);
	d->background->loadThemeConfig(group, themeColor);
}
