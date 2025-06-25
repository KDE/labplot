/*
	File                 : ReferenceRange.cpp
	Project              : LabPlot
	Description          : Reference range on the plot
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022-2024 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ReferenceRange.h"
#include "ReferenceRangePrivate.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/commandtemplates.h"
#include "backend/worksheet/Background.h"
#include "backend/worksheet/Line.h"
#include "frontend/GuiTools.h"

#include <QActionGroup>
#include <QGraphicsSceneMouseEvent>
#include <QMenu>
#include <QPainter>

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>

/**
 * \class ReferenceRange
 * \brief This class implements a rectangular that can be placed at a custom reference position on the plot
 * to highlight certain range of the visualized data.
 *
 *
 * The custom position can be either specified by moving the line with the mouse or by manually providing
 * the start and end values for x or y for the vertical or horizontal orientations, respectively.
 * The coordinates are provided relatively to plot's coordinate system.
 */
ReferenceRange::ReferenceRange(CartesianPlot* plot, const QString& name, bool loading)
	: WorksheetElement(name, new ReferenceRangePrivate(this), AspectType::ReferenceRange) {
	Q_D(ReferenceRange);
	d->m_plot = plot;
	init(loading);
}

// no need to delete the d-pointer here - it inherits from QGraphicsItem
// and is deleted during the cleanup in QGraphicsScene
ReferenceRange::~ReferenceRange() = default;

void ReferenceRange::init(bool loading) {
	Q_D(ReferenceRange);

	// create the background
	d->background = new Background(QString());
	d->background->setEnabledAvailable(true);
	addChild(d->background);
	d->background->setHidden(true);
	connect(d->background, &Background::updateRequested, [=] {
		d->update();
		Q_EMIT changed();
	});

	// create the border line
	d->line = new Line(QString());
	d->line->setHidden(true);
	addChild(d->line);
	connect(d->line, &Line::updatePixmapRequested, [=] {
		d->update();
		Q_EMIT changed();
	});
	connect(d->line, &Line::updateRequested, [=] {
		d->recalcShapeAndBoundingRect();
	});

	// init the properties
	if (!loading) {
		KConfig config;
		KConfigGroup group = config.group(QStringLiteral("ReferenceRange"));

		d->orientation = (Orientation)group.readEntry(QStringLiteral("Orientation"), static_cast<int>(Orientation::Vertical));
		d->updatePositionLimit(); // set the position limit after the orientation was set
		d->background->init(group);
		d->line->init(group);

		if (plot()) {
			m_cSystemIndex = plot()->defaultCoordinateSystemIndex();
			cSystem = plot()->coordinateSystem(m_cSystemIndex);
			d->coordinateBindingEnabled = true;
			// default position - 10% of the plot width/height positioned around the center
			auto cs = plot()->coordinateSystem(coordinateSystemIndex());
			const auto x = d->m_plot->range(Dimension::X, cs->index(Dimension::X)).center();
			const auto y = d->m_plot->range(Dimension::Y, cs->index(Dimension::Y)).center();
			const auto w = d->m_plot->range(Dimension::X, cs->index(Dimension::X)).length() * 0.1;
			const auto h = d->m_plot->range(Dimension::Y, cs->index(Dimension::Y)).length() * 0.1;
			d->positionLogical = QPointF(x, y);
			d->positionLogicalStart = QPointF(x - w / 2, y - h / 2);
			d->positionLogicalEnd = QPointF(x + w / 2, y + h / 2);
		} else
			d->position.point = QPointF(0, 0); // center of parent
		d->updatePosition(); // to update also scene coordinates
	}

	connect(this, &WorksheetElement::objectPositionChanged, this, &ReferenceRange::updateStartEndPositions);
	retransform(); // TODO: why is this required here?!?
}

/*!
	Returns an icon to be used in the project explorer.
*/
QIcon ReferenceRange::icon() const {
	return QIcon::fromTheme(QStringLiteral("draw-rectangle"));
}

void ReferenceRange::initActions() {
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
	QAction* visibilityAction = this->visibilityAction();

	Q_D(const ReferenceRange);

	// Orientation
	if (d->orientation == Orientation::Horizontal)
		orientationHorizontalAction->setChecked(true);
	else
		orientationVerticalAction->setChecked(true);
	menu->insertMenu(visibilityAction, orientationMenu);

	// Border line styles
	const auto& pen = d->line->pen();
	GuiTools::updatePenStyles(lineStyleMenu, lineStyleActionGroup, pen.color());
	GuiTools::selectPenStyleAction(lineStyleActionGroup, pen.style());
	GuiTools::selectColorAction(lineColorActionGroup, pen.color());

	menu->insertMenu(visibilityAction, lineMenu);
	menu->insertSeparator(visibilityAction);

	return menu;
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

// ##############################################################################
// ######  SLOTs for changes triggered via QActions in the context menu  ########
// ##############################################################################
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

// ##############################################################################
// ####################### Private implementation ###############################
// ##############################################################################
ReferenceRangePrivate::ReferenceRangePrivate(ReferenceRange* owner)
	: WorksheetElementPrivate(owner)
	, q(owner) {
	setFlag(QGraphicsItem::ItemSendsGeometryChanges);
	setFlag(QGraphicsItem::ItemIsMovable);
	setFlag(QGraphicsItem::ItemIsSelectable);
	setFlag(QGraphicsItem::ItemIsFocusable);
	setAcceptHoverEvents(true);
}

QPointF ReferenceRangePrivate::recalculateRect() {
	auto cs = q->plot()->coordinateSystem(q->coordinateSystemIndex());
	if (!cs->isValid())
		return QPointF();

	// calculate rect in logical coordinates
	QPointF p1, p2;
	switch (orientation) {
	case ReferenceRange::Orientation::Vertical: {
		const auto& yRange = m_plot->range(Dimension::Y, cs->index(Dimension::Y));
		p1 = QPointF(positionLogicalStart.x(), yRange.start());
		p2 = QPointF(positionLogicalEnd.x(), yRange.end());
		break;
	}
	case ReferenceRange::Orientation::Horizontal: {
		const auto& xRange = m_plot->range(Dimension::X, cs->index(Dimension::X));
		p1 = QPointF(xRange.start(), positionLogicalStart.y());
		p2 = QPointF(xRange.end(), positionLogicalEnd.y());
		break;
	}
	case ReferenceRange::Orientation::Both: {
		p1 = QPointF(positionLogicalStart.x(), positionLogicalStart.y());
		p2 = QPointF(positionLogicalEnd.x(), positionLogicalEnd.y());
		break;
	}
	}
	const auto pointsSceneUnclipped = cs->mapLogicalToScene({p1, p2}, AbstractCoordinateSystem::MappingFlag::SuppressPageClipping);
	// New position without considering clipping!
	const auto newPosScene =
		QPointF((pointsSceneUnclipped.at(0).x() + pointsSceneUnclipped.at(1).x()) / 2, (pointsSceneUnclipped.at(0).y() + pointsSceneUnclipped.at(1).y()) / 2);

	auto point0Clipped = pointsSceneUnclipped.at(0);
	auto point1Clipped = pointsSceneUnclipped.at(1);

	const auto diffXUnclipped = qAbs(point0Clipped.x() - point1Clipped.x());
	const auto diffYUnclipped = qAbs(point0Clipped.y() - point1Clipped.y());

	// Clipping
	const QRectF& dataRect = static_cast<const CartesianPlot*>(q->plot())->dataRect();
	if (point0Clipped.x() < point1Clipped.x()) {
		if (point0Clipped.x() < dataRect.left()) {
			m_leftClipped = true;
			point0Clipped.setX(dataRect.left());
		} else
			m_leftClipped = false;

		if (point1Clipped.x() > dataRect.right()) {
			m_rightClipped = true;
			point1Clipped.setX(dataRect.right());
		} else
			m_rightClipped = false;
		const auto diffX = point1Clipped.x() - point0Clipped.x();
		rect.setX(-diffXUnclipped / 2 + point0Clipped.x()
				  - pointsSceneUnclipped.at(0).x()); // -diffXUnclipped/2 is the value it would be shifted if no clipping happens
		if (diffX >= 0)
			rect.setWidth(diffX);
		else
			rect.setWidth(0);
	} else {
		if (point1Clipped.x() < dataRect.left()) {
			m_leftClipped = true;
			point1Clipped.setX(dataRect.left());
		} else
			m_leftClipped = false;

		if (point0Clipped.x() > dataRect.right()) {
			m_rightClipped = true;
			point0Clipped.setX(dataRect.right());
		} else
			m_rightClipped = false;
		const auto diffX = point0Clipped.x() - point1Clipped.x();
		rect.setX(-diffXUnclipped / 2 + point1Clipped.x() - pointsSceneUnclipped.at(1).x());
		if (diffX >= 0)
			rect.setWidth(diffX);
		else
			rect.setWidth(0);
	}

	if (point0Clipped.y() < point1Clipped.y()) {
		if (point0Clipped.y() < dataRect.top()) {
			m_topClipped = true;
			point0Clipped.setY(dataRect.top());
		} else
			m_topClipped = false;

		if (point1Clipped.y() > dataRect.bottom()) {
			m_bottomClipped = true;
			point1Clipped.setY(dataRect.bottom());
		} else
			m_bottomClipped = false;
		const auto diff = point1Clipped.y() - point0Clipped.y();
		rect.setY(-diffYUnclipped / 2 + point0Clipped.y() - pointsSceneUnclipped.at(0).y());
		if (diff >= 0)
			rect.setHeight(diff);
		else
			rect.setHeight(0);
	} else {
		if (point1Clipped.y() < dataRect.top()) {
			m_topClipped = true;
			point1Clipped.setY(dataRect.top());
		} else
			m_topClipped = false;

		if (point0Clipped.y() > dataRect.bottom()) {
			m_bottomClipped = true;
			point0Clipped.setY(dataRect.bottom());
		} else
			m_bottomClipped = false;
		const auto diff = point0Clipped.y() - point1Clipped.y();
		rect.setY(-diffYUnclipped / 2 + point1Clipped.y() - pointsSceneUnclipped.at(1).y());
		if (diff >= 0)
			rect.setHeight(diff);
		else
			rect.setHeight(0);
	}

	recalcShapeAndBoundingRect();
	return newPosScene;
}

/*!
	calculates the position and the bounding box of the item/point. Called on geometry or properties changes.
 */
void ReferenceRangePrivate::retransform() {
	if (suppressRetransform || !q->cSystem || q->isLoading())
		return;

	const QPointF newPosScene = recalculateRect();

	auto cs = q->plot()->coordinateSystem(q->coordinateSystemIndex());
	positionLogical = cs->mapSceneToLogical(newPosScene, CartesianCoordinateSystem::MappingFlag::SuppressPageClipping);
	updatePosition();
}

void ReferenceRangePrivate::updateOrientation() {
	updatePositionLimit();
	retransform();
}

void ReferenceRangePrivate::updatePositionLimit() {
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
}

/*!
 * called when the user moves the graphics item with the mouse and the scene position of the item is changed.
 * Here we update the logical coordinates for the start and end points based on the new valud for the logical
 * position \c newPosition of the item's center and notify the dock widget.
 */
// TODO: make this undo/redo-able
void ReferenceRange::updateStartEndPositions() {
	Q_D(ReferenceRange);
	if (d->orientation == WorksheetElement::Orientation::Horizontal) {
		const double width = (d->positionLogicalEnd.y() - d->positionLogicalStart.y()) / 2;
		d->positionLogicalStart.setY(d->positionLogical.y() - width);
		d->positionLogicalEnd.setY(d->positionLogical.y() + width);
	} else {
		const double width = (d->positionLogicalEnd.x() - d->positionLogicalStart.x()) / 2;
		d->positionLogicalStart.setX(d->positionLogical.x() - width);
		d->positionLogicalEnd.setX(d->positionLogical.x() + width);
	}

	// Update boundingrect
	d->recalculateRect();

	Q_EMIT positionLogicalStartChanged(d->positionLogicalStart);
	Q_EMIT positionLogicalEndChanged(d->positionLogicalEnd);
}

/*!
  recalculates the outer bounds and the shape of the item.
*/
void ReferenceRangePrivate::recalcShapeAndBoundingRect() {
	prepareGeometryChange();

	m_shape = QPainterPath();
	if (m_visible) {
		QPainterPath path;

		if (!m_topClipped && !m_rightClipped && !m_bottomClipped && !m_leftClipped) {
			path.addRect(rect);
		} else {
			if (!m_topClipped) {
				path.moveTo(rect.topLeft());
				path.lineTo(rect.topRight());
			}
			if (!m_rightClipped) {
				if (m_topClipped)
					path.moveTo(rect.topRight());
				path.lineTo(rect.bottomRight());
			}
			if (!m_bottomClipped) {
				if (m_rightClipped)
					path.moveTo(rect.bottomRight());
				path.lineTo(rect.bottomLeft());
			}
			if (!m_leftClipped) {
				if (m_bottomClipped)
					path.moveTo(rect.bottomLeft());
				path.lineTo(rect.topLeft());
			}
		}

		m_shape.addPath(WorksheetElement::shapeFromPath(path, line->pen()));
		m_boundingRectangle = m_shape.boundingRect();
	}

	Q_EMIT q->changed();
}

void ReferenceRangePrivate::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget*) {
	if (!m_visible)
		return;

	if (rect.width() == 0 || rect.height() == 0)
		return;

	// draw the background filling
	if (background->enabled())
		background->draw(painter, QPolygonF(rect));

	// draw the border
	if (line->style() != Qt::NoPen) {
		painter->setPen(line->pen());
		painter->setBrush(Qt::NoBrush);
		painter->setOpacity(line->opacity());
	}

	painter->drawPath(m_shape);

	if (m_hovered && !isSelected() && !q->isPrinting()) {
		painter->setPen(QPen(QApplication::palette().color(QPalette::Shadow), 2, Qt::SolidLine));
		painter->drawPath(m_shape);
	}

	if (isSelected() && !q->isPrinting()) {
		painter->setPen(QPen(QApplication::palette().color(QPalette::Highlight), 2, Qt::SolidLine));
		painter->drawPath(m_shape);
	}
}

// ##############################################################################
// ##################  Serialization/Deserialization  ###########################
// ##############################################################################
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
			d->updatePositionLimit(); // set the position limit after the orientation was set
			WorksheetElement::load(reader, preview);

			str = attribs.value(QStringLiteral("logicalPosStartX")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("logicalPosStartX"));
			else
				d->positionLogicalStart.setX(str.toDouble());

			str = attribs.value(QStringLiteral("logicalPosStartY")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("logicalPosStartY"));
			else
				d->positionLogicalStart.setY(str.toDouble());

			str = attribs.value(QStringLiteral("logicalPosEndX")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("logicalPosEndX"));
			else
				d->positionLogicalEnd.setX(str.toDouble());

			str = attribs.value(QStringLiteral("logicalPosEndY")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("logicalPosEndY"));
			else
				d->positionLogicalEnd.setY(str.toDouble());
		} else if (!preview && reader->name() == QStringLiteral("background"))
			d->background->load(reader, preview);
		else if (!preview && reader->name() == QStringLiteral("line"))
			d->line->load(reader, preview);
		else { // unknown element
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
