/*
	File                 : ReferenceLine.cpp
	Project              : LabPlot
	Description          : Custom user-defined point on the plot
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2020-2022 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2021 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ReferenceLine.h"
#include "ReferenceLinePrivate.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/commandtemplates.h"
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

using Dimension = CartesianCoordinateSystem::Dimension;

/**
 * \class ReferenceLine
 * \brief A customizable point.
 *
 * The position can be either specified by mouse events or by providing the
 * x- and y- coordinates in parent's coordinate system
 */

ReferenceLine::ReferenceLine(CartesianPlot* plot, const QString& name)
	: WorksheetElement(name, new ReferenceLinePrivate(this), AspectType::ReferenceLine) {
	m_plot = plot;
	init();
}

// no need to delete the d-pointer here - it inherits from QGraphicsItem
// and is deleted during the cleanup in QGraphicsScene
ReferenceLine::~ReferenceLine() = default;

void ReferenceLine::init() {
	Q_D(ReferenceLine);

	KConfig config;
	KConfigGroup group = config.group("ReferenceLine");

	d->coordinateBindingEnabled = true;
	d->orientation = (Orientation)group.readEntry("Orientation", static_cast<int>(Orientation::Vertical));
	if (d->orientation == Orientation::Horizontal)
		d->position.positionLimit = PositionLimit::Y;
	else if (d->orientation == Orientation::Vertical)
		d->position.positionLimit = PositionLimit::X;
	else
		d->position.positionLimit = PositionLimit::None;

	// default position
	auto cs = plot()->coordinateSystem(coordinateSystemIndex());
	const auto x = m_plot->range(Dimension::X, cs->index(Dimension::X)).center();
	const auto y = m_plot->range(Dimension::Y, cs->index(Dimension::Y)).center();
	DEBUG(Q_FUNC_INFO << ", x/y pos = " << x << " / " << y)
	d->positionLogical = QPointF(x, y);
	d->updatePosition(); // To update also scene coordinates

	// line
	d->line = new Line(QString());
	d->line->setHidden(true);
	addChild(d->line);
	connect(d->line, &Line::updatePixmapRequested, [=] {
		d->update();
	});
	connect(d->line, &Line::updateRequested, [=] {
		d->recalcShapeAndBoundingRect();
	});
}

/*!
	Returns an icon to be used in the project explorer.
*/
QIcon ReferenceLine::icon() const {
	return QIcon::fromTheme(QLatin1String("draw-line"));
}

void ReferenceLine::initActions() {
	visibilityAction = new QAction(i18n("Visible"), this);
	visibilityAction->setCheckable(true);
	connect(visibilityAction, &QAction::triggered, this, &ReferenceLine::visibilityChangedSlot);

	// Orientation
	auto* orientationActionGroup = new QActionGroup(this);
	orientationActionGroup->setExclusive(true);
	connect(orientationActionGroup, &QActionGroup::triggered, this, &ReferenceLine::orientationChangedSlot);

	orientationHorizontalAction = new QAction(QIcon::fromTheme(QLatin1String("labplot-axis-horizontal")), i18n("Horizontal"), orientationActionGroup);
	orientationHorizontalAction->setCheckable(true);

	orientationVerticalAction = new QAction(QIcon::fromTheme(QLatin1String("labplot-axis-vertical")), i18n("Vertical"), orientationActionGroup);
	orientationVerticalAction->setCheckable(true);

	// Line
	lineStyleActionGroup = new QActionGroup(this);
	lineStyleActionGroup->setExclusive(true);
	connect(lineStyleActionGroup, &QActionGroup::triggered, this, &ReferenceLine::lineStyleChanged);

	lineColorActionGroup = new QActionGroup(this);
	lineColorActionGroup->setExclusive(true);
	connect(lineColorActionGroup, &QActionGroup::triggered, this, &ReferenceLine::lineColorChanged);
}

void ReferenceLine::initMenus() {
	this->initActions();

	// Orientation
	orientationMenu = new QMenu(i18n("Orientation"));
	orientationMenu->setIcon(QIcon::fromTheme(QLatin1String("labplot-axis-horizontal")));
	orientationMenu->addAction(orientationHorizontalAction);
	orientationMenu->addAction(orientationVerticalAction);

	// Line
	lineMenu = new QMenu(i18n("Line"));
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

QMenu* ReferenceLine::createContextMenu() {
	if (!orientationMenu)
		initMenus();

	QMenu* menu = WorksheetElement::createContextMenu();
	QAction* firstAction = menu->actions().at(1); // skip the first action because of the "title-action"
	visibilityAction->setChecked(isVisible());
	menu->insertAction(firstAction, visibilityAction);

	Q_D(const ReferenceLine);

	// Orientation
	if (d->orientation == Orientation::Horizontal)
		orientationHorizontalAction->setChecked(true);
	else
		orientationVerticalAction->setChecked(true);
	menu->insertMenu(firstAction, orientationMenu);

	// Line styles
	const auto& pen = d->line->pen();
	GuiTools::updatePenStyles(lineStyleMenu, lineStyleActionGroup, pen.color());
	GuiTools::selectPenStyleAction(lineStyleActionGroup, pen.style());
	GuiTools::selectColorAction(lineColorActionGroup, pen.color());

	menu->insertMenu(firstAction, lineMenu);
	menu->insertSeparator(firstAction);

	return menu;
}

QGraphicsItem* ReferenceLine::graphicsItem() const {
	return d_ptr;
}

void ReferenceLine::retransform() {
	Q_D(ReferenceLine);
	d->retransform();
}

void ReferenceLine::handleResize(double /*horizontalRatio*/, double /*verticalRatio*/, bool /*pageResize*/) {
}

/* ============================ getter methods ================= */
BASIC_SHARED_D_READER_IMPL(ReferenceLine, ReferenceLine::Orientation, orientation, orientation)

Line* ReferenceLine::line() const {
	Q_D(const ReferenceLine);
	return d->line;
}

/* ============================ setter methods and undo commands ================= */
STD_SETTER_CMD_IMPL_F_S(ReferenceLine, SetOrientation, ReferenceLine::Orientation, orientation, retransform)
void ReferenceLine::setOrientation(Orientation orientation) {
	Q_D(ReferenceLine);
	if (orientation != d->orientation) {
		exec(new ReferenceLineSetOrientationCmd(d, orientation, ki18n("%1: set orientation")));
		switch (orientation) {
		case ReferenceLine::Orientation::Horizontal:
			d->position.positionLimit = PositionLimit::Y;
			break;
		case ReferenceLine::Orientation::Vertical:
			d->position.positionLimit = PositionLimit::X;
			break;
		case ReferenceLine::Orientation::Both:
			d->position.positionLimit = PositionLimit::None;
			break;
		}
	}
}

//##############################################################################
//######  SLOTs for changes triggered via QActions in the context menu  ########
//##############################################################################
void ReferenceLine::orientationChangedSlot(QAction* action) {
	if (action == orientationHorizontalAction)
		this->setOrientation(Orientation::Horizontal);
	else
		this->setOrientation(Orientation::Vertical);
}

void ReferenceLine::lineStyleChanged(QAction* action) {
	Q_D(const ReferenceLine);
	QPen pen = d->line->pen();
	pen.setStyle(GuiTools::penStyleFromAction(lineStyleActionGroup, action));
	d->line->setPen(pen);
}

void ReferenceLine::lineColorChanged(QAction* action) {
	Q_D(const ReferenceLine);
	QPen pen = d->line->pen();
	pen.setColor(GuiTools::colorFromAction(lineColorActionGroup, action));
	d->line->setPen(pen);
}

void ReferenceLine::visibilityChangedSlot() {
	Q_D(const ReferenceLine);
	this->setVisible(!d->isVisible());
}

//##############################################################################
//####################### Private implementation ###############################
//##############################################################################
ReferenceLinePrivate::ReferenceLinePrivate(ReferenceLine* owner)
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
void ReferenceLinePrivate::retransform() {
	if (suppressRetransform || !q->cSystem || q->isLoading())
		return;

	auto cs = q->plot()->coordinateSystem(q->coordinateSystemIndex());
	const auto xRange{q->m_plot->range(Dimension::X, cs->index(Dimension::X))};
	const auto yRange{q->m_plot->range(Dimension::Y, cs->index(Dimension::Y))};

	// calculate the position in the scene coordinates
	if (orientation == ReferenceLine::Orientation::Vertical)
		positionLogical = QPointF(positionLogical.x(), yRange.center());
	else
		positionLogical = QPointF(xRange.center(), positionLogical.y());
	updatePosition(); // To update position.point

	// position.point contains already the scene position, but here it will be determined,
	// if the point lies outside of the datarect or not
	QVector<QPointF> listScene = q->cSystem->mapLogicalToScene(Points() << positionLogical);
	QDEBUG(Q_FUNC_INFO << ", scene list = " << listScene)

	if (!listScene.isEmpty()) {
		m_visible = true;

		// determine the length of the line to be drawn
		QVector<QPointF> pointsLogical;
		if (orientation == ReferenceLine::Orientation::Vertical)
			pointsLogical << QPointF(positionLogical.x(), yRange.start()) << QPointF(positionLogical.x(), yRange.end());
		else
			pointsLogical << QPointF(xRange.start(), positionLogical.y()) << QPointF(xRange.end(), positionLogical.y());

		QVector<QPointF> pointsScene = q->cSystem->mapLogicalToScene(pointsLogical);

		if (pointsScene.size() > 1) {
			if (orientation == ReferenceLine::Orientation::Vertical)
				length = pointsScene.at(0).y() - pointsScene.at(1).y();
			else
				length = pointsScene.at(0).x() - pointsScene.at(1).x();
		}
	} else
		m_visible = false;
	QDEBUG(Q_FUNC_INFO << ", scene list after = " << listScene)

	recalcShapeAndBoundingRect();
}

/*!
	Returns the outer bounds of the item as a rectangle.
 */
QRectF ReferenceLinePrivate::boundingRect() const {
	return boundingRectangle;
}

/*!
	Returns the shape of this item as a QPainterPath in local coordinates.
*/
QPainterPath ReferenceLinePrivate::shape() const {
	return lineShape;
}

/*!
  recalculates the outer bounds and the shape of the item.
*/
void ReferenceLinePrivate::recalcShapeAndBoundingRect() {
	prepareGeometryChange();

	lineShape = QPainterPath();
	if (m_visible) {
		QPainterPath path;
		if (orientation == ReferenceLine::Orientation::Horizontal) {
			path.moveTo(-length / 2, 0);
			path.lineTo(length / 2, 0);
		} else {
			path.moveTo(0, length / 2);
			path.lineTo(0, -length / 2);
		}
		lineShape.addPath(WorksheetElement::shapeFromPath(path, line->pen()));
		boundingRectangle = lineShape.boundingRect();
	}
}

void ReferenceLinePrivate::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget*) {
	if (!m_visible)
		return;

	painter->setOpacity(line->opacity());
	painter->setPen(line->pen());
	if (orientation == ReferenceLine::Orientation::Horizontal)
		painter->drawLine(-length / 2, 0, length / 2, 0);
	else
		painter->drawLine(0, length / 2, 0, -length / 2);

	if (m_hovered && !isSelected() && !q->isPrinting()) {
		painter->setPen(QPen(QApplication::palette().color(QPalette::Shadow), 2, Qt::SolidLine));
		painter->drawPath(lineShape);
	}

	if (isSelected() && !q->isPrinting()) {
		painter->setPen(QPen(QApplication::palette().color(QPalette::Highlight), 2, Qt::SolidLine));
		painter->drawPath(lineShape);
	}
}

void ReferenceLinePrivate::contextMenuEvent(QGraphicsSceneContextMenuEvent* event) {
	q->createContextMenu()->exec(event->screenPos());
}

void ReferenceLinePrivate::hoverEnterEvent(QGraphicsSceneHoverEvent*) {
	if (!isSelected()) {
		m_hovered = true;
		Q_EMIT q->hovered();
		update();
	}
}

void ReferenceLinePrivate::hoverLeaveEvent(QGraphicsSceneHoverEvent*) {
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
void ReferenceLine::save(QXmlStreamWriter* writer) const {
	Q_D(const ReferenceLine);

	writer->writeStartElement(QStringLiteral("referenceLine"));
	writeBasicAttributes(writer);
	writeCommentElement(writer);

	writer->writeStartElement(QStringLiteral("geometry"));
	WorksheetElement::save(writer);
	writer->writeAttribute(QStringLiteral("orientation"), QString::number(static_cast<int>(d->orientation)));
	writer->writeEndElement();

	d->line->save(writer);

	writer->writeEndElement(); // close "ReferenceLine" section
}

//! Load from XML
bool ReferenceLine::load(XmlStreamReader* reader, bool preview) {
	Q_D(ReferenceLine);

	if (!readBasicAttributes(reader))
		return false;

	KLocalizedString attributeWarning = ki18n("Attribute '%1' missing or empty, default value is used");
	QXmlStreamAttributes attribs;
	QString str;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == QLatin1String("referenceLine"))
			break;

		if (!reader->isStartElement())
			continue;

		if (!preview && reader->name() == QLatin1String("comment")) {
			if (!readCommentElement(reader))
				return false;
		} else if (!preview && reader->name() == QLatin1String("general")) {
			// old logic for the position for xml version < 6
			Q_D(ReferenceLine);
			attribs = reader->attributes();
			auto str = attribs.value(QStringLiteral("position")).toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs(QStringLiteral("position")).toString());
			else {
				d->positionLogical.setX(str.toDouble());
				d->positionLogical.setY(str.toDouble());
			}
			d->coordinateBindingEnabled = true;

			READ_INT_VALUE("orientation", orientation, Orientation);
			READ_INT_VALUE_DIRECT("plotRangeIndex", m_cSystemIndex, int);

			str = attribs.value(QStringLiteral("visible")).toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs(QStringLiteral("visible")).toString());
			else
				d->setVisible(str.toInt());
		} else if (!preview && reader->name() == QLatin1String("geometry")) {
			attribs = reader->attributes();
			// new logic for the position for xmlVersion >= 6
			READ_INT_VALUE("orientation", orientation, Orientation);
			WorksheetElement::load(reader, preview);
		} else if (!preview && reader->name() == QLatin1String("line")) {
			d->line->load(reader, preview);
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
void ReferenceLine::loadThemeConfig(const KConfig& config) {
	Q_D(ReferenceLine);

	// for the properties of the line read the properties of the axis line
	const KConfigGroup& group = config.group("Axis");
	const auto& themeColor = group.readEntry("LineColor", QColor(Qt::black));
	d->line->loadThemeConfig(group, themeColor);
}
