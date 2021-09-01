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
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/worksheet/plots/cartesian/CartesianCoordinateSystem.h"
#include "backend/worksheet/plots/cartesian/Symbol.h"
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
	: WorksheetElement(name, AspectType::CustomPoint), d_ptr(new CustomPointPrivate(this)) {

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
	d->position.setX(m_plot->xRange(cs->xIndex()).center());
	d->position.setY(m_plot->yRange(cs->yIndex()).center());

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
	connect(visibilityAction, &QAction::triggered, this, &CustomPoint::visibilityChanged);
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

void CustomPoint::handleResize(double horizontalRatio, double verticalRatio, bool pageResize) {
	Q_UNUSED(horizontalRatio)
	Q_UNUSED(verticalRatio)
	Q_UNUSED(pageResize)
}

/* ============================ getter methods ================= */
BASIC_SHARED_D_READER_IMPL(CustomPoint, QPointF, position, position)

Symbol* CustomPoint::symbol() const {
	Q_D(const CustomPoint);
	return d->symbol;
}

/* ============================ setter methods and undo commands ================= */
STD_SETTER_CMD_IMPL_F_S(CustomPoint, SetPosition, QPointF, position, retransform)
void CustomPoint::setPosition(QPointF position) {
	Q_D(CustomPoint);
	if (position != d->position)
		exec(new CustomPointSetPositionCmd(d, position, ki18n("%1: set position")));
}

STD_SWAP_METHOD_SETTER_CMD_IMPL_F(CustomPoint, SetVisible, bool, swapVisible, update);
void CustomPoint::setVisible(bool on) {
	Q_D(CustomPoint);
	exec(new CustomPointSetVisibleCmd(d, on, on ? ki18n("%1: set visible") : ki18n("%1: set invisible")));
}

bool CustomPoint::isVisible() const {
	Q_D(const CustomPoint);
	return d->isVisible();
}

void CustomPoint::setParentGraphicsItem(QGraphicsItem* item) {
	Q_D(CustomPoint);
	d->setParentItem(item);
	//d->updatePosition();
}

//##############################################################################
//######  SLOTs for changes triggered via QActions in the context menu  ########
//##############################################################################
void CustomPoint::visibilityChanged() {
	Q_D(const CustomPoint);
	this->setVisible(!d->isVisible());
}

//##############################################################################
//####################### Private implementation ###############################
//##############################################################################
CustomPointPrivate::CustomPointPrivate(CustomPoint* owner) : q(owner) {
	setFlag(QGraphicsItem::ItemSendsGeometryChanges);
	setFlag(QGraphicsItem::ItemIsMovable);
	setFlag(QGraphicsItem::ItemIsSelectable);
	setAcceptHoverEvents(true);
}

QString CustomPointPrivate::name() const {
	return q->name();
}

const CartesianPlot* CustomPointPrivate::plot() {
	return q->m_plot;
}

/*!
    calculates the position and the bounding box of the item/point. Called on geometry or properties changes.
 */
void CustomPointPrivate::retransform() {
	DEBUG(Q_FUNC_INFO)
	if (suppressRetransform || !parentItem())
		return;

	//calculate the point in the scene coordinates
	const auto& listScene = q->cSystem->mapLogicalToScene(QVector<QPointF>{position});
	if (!listScene.isEmpty()) {
		m_visible = true;
		positionScene = listScene.constFirst();
		QPointF inParentCoords = mapPlotAreaToParent(positionScene);
		suppressItemChangeEvent = true;
		setPos(inParentCoords);
		suppressItemChangeEvent = false;
	} else
		m_visible = false;

	recalcShapeAndBoundingRect();
}

bool CustomPointPrivate::swapVisible(bool on) {
	bool oldValue = isVisible();

	//When making a graphics item invisible, it gets deselected in the scene.
	//In this case we don't want to deselect the item in the project explorer.
	//We need to supress the deselection in the view.
	auto* worksheet = static_cast<Worksheet*>(q->parent(AspectType::Worksheet));
	if (worksheet) {
		worksheet->suppressSelectionChangedEvent(true);
		setVisible(on);
		worksheet->suppressSelectionChangedEvent(false);
	} else
		setVisible(on);

	emit q->changed();
	emit q->visibleChanged(on);
	return oldValue;
}

/*!
    Returns the outer bounds of the item as a rectangle.
 */
QRectF CustomPointPrivate::boundingRect() const {
	return transformedBoundingRectangle;
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
		QPainterPath path = Symbol::pathFromStyle(symbol->style());

		QTransform trafo;
		trafo.scale(symbol->size(), symbol->size());
		path = trafo.map(path);
		trafo.reset();

		if (symbol->rotationAngle() != 0.) {
			trafo.rotate(symbol->rotationAngle());
			path = trafo.map(path);
		}

		pointShape.addPath(WorksheetElement::shapeFromPath(trafo.map(path), symbol->pen()));
		transformedBoundingRectangle = pointShape.boundingRect();
	}
}

void CustomPointPrivate::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
	Q_UNUSED(option)
	Q_UNUSED(widget)

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

QVariant CustomPointPrivate::itemChange(GraphicsItemChange change, const QVariant &value) {
	if (suppressItemChangeEvent)
		return value;

	if (change == QGraphicsItem::ItemPositionChange) {
		//emit the signals in order to notify the UI.
		QPointF scenePos = mapParentToPlotArea(value.toPointF());
		QPointF logicalPos = q->cSystem->mapSceneToLogical(scenePos); // map parent to scene
		//q->setPosition(logicalPos);

		// not needed, because positionChanged trigger the widget, which sets the position
		//position = logicalPos; // don't use setPosition, because this will call retransform and then again this function
		emit q->positionChanged(logicalPos);
	}

	return QGraphicsItem::itemChange(change, value);
}

void CustomPointPrivate::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
	//don't move when the parent is a InfoElement, because there
	//the custompoint position changes by the mouse are not allowed
	if (q->parentAspect()->type() == AspectType::InfoElement)
		return;

	//position was changed -> set the position member variables
	suppressRetransform = true;
	q->setPosition(q->cSystem->mapSceneToLogical(pos()));
	suppressRetransform = false;

	QGraphicsItem::mouseReleaseEvent(event);
}

void CustomPointPrivate::contextMenuEvent(QGraphicsSceneContextMenuEvent* event) {
	q->createContextMenu()->exec(event->screenPos());
}

void CustomPointPrivate::hoverEnterEvent(QGraphicsSceneHoverEvent*) {
	if (!isSelected()) {
		m_hovered = true;
		emit q->hovered();
		update();
	}
}

void CustomPointPrivate::hoverLeaveEvent(QGraphicsSceneHoverEvent*) {
	if (m_hovered) {
		m_hovered = false;
		emit q->unhovered();
		update();
	}
}

/*!
 * \brief CustomPointPrivate::mapPlotAreaToParent
 * Mapping a point from the PlotArea (CartesianPlot::plotArea) coordinates to the parent
 * coordinates of this item
 * Needed because in some cases the parent is not the PlotArea, but a child of it (Marker/InfoElement)
 * IMPORTANT: function is also used in Textlabel, so when changing anything, change it also there
 * \param point point in plotArea coordinates
 * \return point in parent coordinates
 */
QPointF CustomPointPrivate::mapPlotAreaToParent(QPointF point) {
	if (plot()) {
		// first mapping to item coordinates and from there back to parent
		// WorksheetinfoElement: parentItem()->parentItem() == plot->graphicsItem()
		// plot->graphicsItem().pos() == plot->plotArea()->graphicsItem().pos()
		auto* plotArea = const_cast<CartesianPlot*>(plot())->plotArea();
		return mapToParent(mapFromItem(plotArea->graphicsItem(), point));
	}
	return QPointF(0, 0);
}

/*!
 * \brief CustomPointPrivate::mapParentToPlotArea
 * Mapping a point from parent coordinates to plotArea coordinates
 * Needed because in some cases the parent is not the PlotArea, but a child of it (Marker/InfoElement)
 * IMPORTANT: function is also used in Textlabel, so when changing anything, change it also there
 * \param point point in parent coordinates
 * \return point in PlotArea coordinates
 */
QPointF CustomPointPrivate::mapParentToPlotArea(QPointF point) {
	if (plot()) {
		// mapping from parent to item coordinates and them to plot area
		auto* plotArea = const_cast<CartesianPlot*>(plot())->plotArea();
		return mapToItem(plotArea->graphicsItem(), mapFromParent(point));
	}
	return QPointF(0, 0);
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
	writer->writeAttribute( "x", QString::number(d->position.x()) );
	writer->writeAttribute( "y", QString::number(d->position.y()) );
	writer->writeAttribute( "plotRangeIndex", QString::number(m_cSystemIndex) );
	writer->writeAttribute( "visible", QString::number(d->isVisible()) );
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
			attribs = reader->attributes();

			str = attribs.value("x").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("x").toString());
			else
				d->position.setX(str.toDouble());

			str = attribs.value("y").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("y").toString());
			else
				d->position.setY(str.toDouble());

			READ_INT_VALUE_DIRECT("plotRangeIndex", m_cSystemIndex, int);

			str = attribs.value("visible").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("visible").toString());
			else
				d->setVisible(str.toInt());
		} else if (!preview && reader->name() == "symbol") {
			d->symbol->load(reader, preview);
		} else { // unknown element
			reader->raiseWarning(i18n("unknown element '%1'", reader->name().toString()));
			if (!reader->skipToEndElement()) return false;
		}
	}

	retransform();
	return true;
}
