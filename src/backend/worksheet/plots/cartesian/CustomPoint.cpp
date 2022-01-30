/*
	File                 : CustomPoint.cpp
	Project              : LabPlot
	Description          : Custom user-defined point on the plot
	--------------------------------------------------------------------
    SPDX-FileCopyrightText: 2015 Ankit Wagadre <wagadre.ankit@gmail.com>
    SPDX-FileCopyrightText: 2015-2021 Alexander Semke <alexander.semke@web.de>
    SPDX-FileCopyrightText: 2020 Martin Marmsoler <martin.marmsoler@gmail.com>
    SPDX-FileCopyrightText: 2021 Stefan Gerlach <stefan.gerlach@uni.kn>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "CustomPoint.h"
#include "CustomPointPrivate.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/PlotArea.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/worksheet/plots/cartesian/CartesianCoordinateSystem.h"
#include "backend/worksheet/plots/cartesian/Symbol.h"
#include "backend/core/Project.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/XmlStreamReader.h"

#include <QPainter>
#include <QMenu>
#include <QGraphicsSceneMouseEvent>

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>

/**
 * \class CustomPoint
 * \brief A customizable point.
 *
 * The position can be either specified by mouse events or by providing the
 * x- and y- coordinates in parent's coordinate system
 */

CustomPoint::CustomPoint(CartesianPlot* plot, const QString& name)
	: WorksheetElement(name, new CustomPointPrivate(this), AspectType::CustomPoint) {

	m_plot = plot;
	DEBUG(Q_FUNC_INFO << ", cSystem index = " << m_cSystemIndex)
	DEBUG(Q_FUNC_INFO << ", plot cSystem count = " << m_plot->coordinateSystemCount())
	cSystem = dynamic_cast<const CartesianCoordinateSystem*>(m_plot->coordinateSystem(m_cSystemIndex));

	init();
}

//no need to delete the d-pointer here - it inherits from QGraphicsItem
//and is deleted during the cleanup in QGraphicsScene
CustomPoint::~CustomPoint() = default;

void CustomPoint::init() {
	Q_D(CustomPoint);

	// default position
	auto cs = plot()->coordinateSystem(coordinateSystemIndex());
	const auto x = m_plot->xRange(cs->xIndex()).center();
	const auto y = m_plot->yRange(cs->yIndex()).center();
	d->positionLogical = QPointF(x, y);
	d->updatePosition(); // To update also scene coordinates

	//initialize the symbol
	d->symbol = new Symbol(QString());
	addChild(d->symbol);
	d->symbol->setHidden(true);
	connect(d->symbol, &Symbol::updateRequested, [=]{d->recalcShapeAndBoundingRect();});
	connect(d->symbol, &Symbol::updatePixmapRequested, [=]{d->update();});
	KConfig config;
	d->symbol->init(config.group("CustomPoint"));

	initActions();
}

void CustomPoint::initActions() {
	visibilityAction = new QAction(i18n("Visible"), this);
	visibilityAction->setCheckable(true);
	connect(visibilityAction, &QAction::triggered, this, &CustomPoint::changeVisibility);
}

/*!
    Returns an icon to be used in the project explorer.
*/
QIcon CustomPoint::icon() const {
	return QIcon::fromTheme("draw-cross");
}

QMenu* CustomPoint::createContextMenu() {
	//no context menu if the custom point is a child of an InfoElement,
	//everything is controlled by the parent
	if (parentAspect()->type() == AspectType::InfoElement)
		return nullptr;

	QMenu* menu = WorksheetElement::createContextMenu();
	QAction* firstAction = menu->actions().at(1); //skip the first action because of the "title-action"
	visibilityAction->setChecked(isVisible());
	menu->insertAction(firstAction, visibilityAction);
	menu->insertSeparator(firstAction);

	return menu;
}

QGraphicsItem* CustomPoint::graphicsItem() const {
	return d_ptr;
}

void CustomPoint::retransform() {
	DEBUG(Q_FUNC_INFO)
	Q_D(CustomPoint);
	d->retransform();
}

void CustomPoint::handleResize(double /*horizontalRatio*/, double /*verticalRatio*/, bool /*pageResize*/) {
}

Symbol* CustomPoint::symbol() const {
	Q_D(const CustomPoint);
	return d->symbol;
}

void CustomPoint::setParentGraphicsItem(QGraphicsItem* item) {
	Q_D(CustomPoint);
	d->setParentItem(item);
}

//##############################################################################
//####################### Private implementation ###############################
//##############################################################################
CustomPointPrivate::CustomPointPrivate(CustomPoint* owner) : WorksheetElementPrivate(owner), q(owner) {
	setFlag(QGraphicsItem::ItemSendsGeometryChanges);
	setFlag(QGraphicsItem::ItemIsMovable);
	setFlag(QGraphicsItem::ItemIsSelectable);
	setFlag(QGraphicsItem::ItemIsFocusable);
	setAcceptHoverEvents(true);
}

const CartesianPlot* CustomPointPrivate::plot() {
	return q->m_plot;
}

/*!
    calculates the position and the bounding box of the item/point. Called on geometry or properties changes.
 */
void CustomPointPrivate::retransform() {
	DEBUG(Q_FUNC_INFO)
	if (suppressRetransform || q->isLoading())
		return;

	updatePosition();
	recalcShapeAndBoundingRect();
}

/*!
    Returns the shape of this item as a QPainterPath in local coordinates.
*/
QPainterPath CustomPointPrivate::shape() const {
	return pointShape;
}

/*!
  recalculates the outer bounds and the shape of the item.
*/
void CustomPointPrivate::recalcShapeAndBoundingRect() {
	prepareGeometryChange();

	pointShape = QPainterPath();
	if (m_visible && symbol->style() != Symbol::Style::NoSymbols) {
		QPainterPath path = Symbol::stylePath(symbol->style());

		QTransform trafo;
		trafo.scale(symbol->size(), symbol->size());
		path = trafo.map(path);
		trafo.reset();

		if (symbol->rotationAngle() != 0.) {
			trafo.rotate(symbol->rotationAngle());
			path = trafo.map(path);
		}

		pointShape.addPath(WorksheetElement::shapeFromPath(trafo.map(path), symbol->pen()));
		boundingRectangle = pointShape.boundingRect();
	}
}

void CustomPointPrivate::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget*) {
	if (!m_visible)
		return;

	if (symbol->style() != Symbol::Style::NoSymbols) {
		painter->setOpacity(symbol->opacity());
		painter->setPen(symbol->pen());
		painter->setBrush(symbol->brush());
		painter->drawPath(pointShape);
	}

	if (m_hovered && !isSelected() && !q->isPrinting()) {
		painter->setPen(QPen(QApplication::palette().color(QPalette::Shadow), 2, Qt::SolidLine));
		painter->drawPath(pointShape);
	}

	if (isSelected() && !q->isPrinting()) {
		painter->setPen(QPen(QApplication::palette().color(QPalette::Highlight), 2, Qt::SolidLine));
		painter->drawPath(pointShape);
	}
}

void CustomPointPrivate::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
	//don't move when the parent is a InfoElement, because there
	//the custompoint position changes by the mouse are not allowed
	if (q->parentAspect()->type() == AspectType::InfoElement)
		return;

	WorksheetElementPrivate::mouseReleaseEvent(event);
}

void CustomPointPrivate::contextMenuEvent(QGraphicsSceneContextMenuEvent* event) {
	q->createContextMenu()->exec(event->screenPos());
}

void CustomPointPrivate::hoverEnterEvent(QGraphicsSceneHoverEvent*) {
	if (!isSelected()) {
		m_hovered = true;
		Q_EMIT q->hovered();
		update();
	}
}

void CustomPointPrivate::hoverLeaveEvent(QGraphicsSceneHoverEvent*) {
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
void CustomPoint::save(QXmlStreamWriter* writer) const {
	Q_D(const CustomPoint);

	writer->writeStartElement("customPoint");
	writeBasicAttributes(writer);
	writeCommentElement(writer);

	//geometry
	writer->writeStartElement("geometry");
	WorksheetElement::save(writer);
	writer->writeEndElement();

	d->symbol->save(writer);

	writer->writeEndElement(); // close "CustomPoint" section
}

//! Load from XML
bool CustomPoint::load(XmlStreamReader* reader, bool preview) {
	Q_D(CustomPoint);

	if (!readBasicAttributes(reader))
		return false;

	KLocalizedString attributeWarning = ki18n("Attribute '%1' missing or empty, default value is used");
	QXmlStreamAttributes attribs;
	QString str;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == "customPoint")
			break;

		if (!reader->isStartElement())
			continue;

		if (!preview && reader->name() == "comment") {
			if (!readCommentElement(reader)) return false;
		} else if (!preview && reader->name() == "geometry") {
			WorksheetElement::load(reader, preview);
			if (project()->xmlVersion() < 6) {
				// Before version 6 the position in the file was always a logical position
				d->positionLogical = d->position.point;
				d->position.point = QPointF(0, 0);
				d->coordinateBindingEnabled = true;
			}
		} else if (!preview && reader->name() == "symbol") {
			d->symbol->load(reader, preview);
		} else { // unknown element
			reader->raiseWarning(i18n("unknown element '%1'", reader->name().toString()));
			if (!reader->skipToEndElement()) return false;
		}
	}
	return true;
}
