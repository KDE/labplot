/***************************************************************************
	File                 : CustomPoint.cpp
	Project              : LabPlot
	Description          : Custom user-defined point on the plot
	--------------------------------------------------------------------
	Copyright            : (C) 2015 Ankit Wagadre (wagadre.ankit@gmail.com)
	Copyright            : (C) 2015-2020 Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2020 Martin Marmsoler (martin.marmsoler@gmail.com)
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

#include "CustomPoint.h"
#include "CustomPointPrivate.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/worksheet/plots/cartesian/CartesianCoordinateSystem.h"
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

CustomPoint::CustomPoint(const CartesianPlot* plot, const QString& name)
	: WorksheetElement(name, AspectType::CustomPoint), d_ptr(new CustomPointPrivate(this, plot)) {

	init();
}

CustomPoint::CustomPoint(const QString& name, CustomPointPrivate* dd)
	: WorksheetElement(name, AspectType::CustomPoint), d_ptr(dd) {

	init();
}

//no need to delete the d-pointer here - it inherits from QGraphicsItem
//and is deleted during the cleanup in QGraphicsScene
CustomPoint::~CustomPoint() = default;

void CustomPoint::init() {
	Q_D(CustomPoint);

	KConfig config;
	KConfigGroup group;
	group = config.group("CustomPoint");
	d->position.setX( group.readEntry("PositionXValue", d->plot->xRange().center()) );
	d->position.setY( group.readEntry("PositionYValue", d->plot->yRange().center()) );

	d->symbolStyle = (Symbol::Style)group.readEntry("SymbolStyle", (int)Symbol::Style::Circle);
	d->symbolSize = group.readEntry("SymbolSize", Worksheet::convertToSceneUnits(5, Worksheet::Unit::Point));
	d->symbolRotationAngle = group.readEntry("SymbolRotation", 0.0);
	d->symbolOpacity = group.readEntry("SymbolOpacity", 1.0);
	d->symbolBrush.setStyle( (Qt::BrushStyle)group.readEntry("SymbolFillingStyle", (int)Qt::SolidPattern) );
	d->symbolBrush.setColor( group.readEntry("SymbolFillingColor", QColor(Qt::red)) );
	d->symbolPen.setStyle( (Qt::PenStyle)group.readEntry("SymbolBorderStyle", (int)Qt::SolidLine) );
	d->symbolPen.setColor( group.readEntry("SymbolBorderColor", QColor(Qt::black)) );
	d->symbolPen.setWidthF( group.readEntry("SymbolBorderWidth", Worksheet::convertToSceneUnits(0.0, Worksheet::Unit::Point)) );

	this->initActions();
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
	return  QIcon::fromTheme("draw-cross");
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

	return menu;
}

QGraphicsItem* CustomPoint::graphicsItem() const {
	return d_ptr;
}

void CustomPoint::retransform() {
	Q_D(CustomPoint);
	d->retransform();
}

void CustomPoint::handleResize(double horizontalRatio, double verticalRatio, bool pageResize) {
	Q_UNUSED(horizontalRatio);
	Q_UNUSED(verticalRatio);
	Q_UNUSED(pageResize);
}

/* ============================ getter methods ================= */
CLASS_SHARED_D_READER_IMPL(CustomPoint, QPointF, position, position)

//symbols
BASIC_SHARED_D_READER_IMPL(CustomPoint, Symbol::Style, symbolStyle, symbolStyle)
BASIC_SHARED_D_READER_IMPL(CustomPoint, qreal, symbolOpacity, symbolOpacity)
BASIC_SHARED_D_READER_IMPL(CustomPoint, qreal, symbolRotationAngle, symbolRotationAngle)
BASIC_SHARED_D_READER_IMPL(CustomPoint, qreal, symbolSize, symbolSize)
CLASS_SHARED_D_READER_IMPL(CustomPoint, QBrush, symbolBrush, symbolBrush)
CLASS_SHARED_D_READER_IMPL(CustomPoint, QPen, symbolPen, symbolPen)

/* ============================ setter methods and undo commands ================= */
STD_SETTER_CMD_IMPL_F_S(CustomPoint, SetPosition, QPointF, position, retransform)
void CustomPoint::setPosition(QPointF position) {
	Q_D(CustomPoint);
	if (position != d->position)
		exec(new CustomPointSetPositionCmd(d, position, ki18n("%1: set position")));
}

//Symbol
STD_SETTER_CMD_IMPL_F_S(CustomPoint, SetSymbolStyle, Symbol::Style, symbolStyle, recalcShapeAndBoundingRect)
void CustomPoint::setSymbolStyle(Symbol::Style style) {
	Q_D(CustomPoint);
	if (style != d->symbolStyle)
		exec(new CustomPointSetSymbolStyleCmd(d, style, ki18n("%1: set symbol style")));
}

STD_SETTER_CMD_IMPL_F_S(CustomPoint, SetSymbolSize, qreal, symbolSize, recalcShapeAndBoundingRect)
void CustomPoint::setSymbolSize(qreal size) {
	Q_D(CustomPoint);
	if (!qFuzzyCompare(1 + size, 1 + d->symbolSize))
		exec(new CustomPointSetSymbolSizeCmd(d, size, ki18n("%1: set symbol size")));
}

STD_SETTER_CMD_IMPL_F_S(CustomPoint, SetSymbolRotationAngle, qreal, symbolRotationAngle, recalcShapeAndBoundingRect)
void CustomPoint::setSymbolRotationAngle(qreal angle) {
	Q_D(CustomPoint);
	if (!qFuzzyCompare(1 + angle, 1 + d->symbolRotationAngle))
		exec(new CustomPointSetSymbolRotationAngleCmd(d, angle, ki18n("%1: rotate symbols")));
}

STD_SETTER_CMD_IMPL_F_S(CustomPoint, SetSymbolBrush, QBrush, symbolBrush, update)
void CustomPoint::setSymbolBrush(const QBrush &brush) {
	Q_D(CustomPoint);
	if (brush != d->symbolBrush)
		exec(new CustomPointSetSymbolBrushCmd(d, brush, ki18n("%1: set symbol filling")));
}

STD_SETTER_CMD_IMPL_F_S(CustomPoint, SetSymbolPen, QPen, symbolPen, recalcShapeAndBoundingRect)
void CustomPoint::setSymbolPen(const QPen &pen) {
	Q_D(CustomPoint);
	if (pen != d->symbolPen)
		exec(new CustomPointSetSymbolPenCmd(d, pen, ki18n("%1: set symbol outline style")));
}

STD_SETTER_CMD_IMPL_F_S(CustomPoint, SetSymbolOpacity, qreal, symbolOpacity, update)
void CustomPoint::setSymbolOpacity(qreal opacity) {
	Q_D(CustomPoint);
	if (opacity != d->symbolOpacity)
		exec(new CustomPointSetSymbolOpacityCmd(d, opacity, ki18n("%1: set symbol opacity")));
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

void CustomPoint::setPrinting(bool on) {
	Q_D(CustomPoint);
	d->m_printing = on;
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
CustomPointPrivate::CustomPointPrivate(CustomPoint* owner, const CartesianPlot* p) : plot(p), q(owner) {
	setFlag(QGraphicsItem::ItemSendsGeometryChanges);
	setFlag(QGraphicsItem::ItemIsMovable);
	setFlag(QGraphicsItem::ItemIsSelectable);
	setAcceptHoverEvents(true);
}

QString CustomPointPrivate::name() const {
	return q->name();
}

/*!
    calculates the position and the bounding box of the item/point. Called on geometry or properties changes.
 */
void CustomPointPrivate::retransform() {
	if (suppressRetransform)
		return;

	if (!parentItem())
		return;

	//calculate the point in the scene coordinates
	//TODO
	const auto* cSystem = dynamic_cast<const CartesianCoordinateSystem*>(plot->coordinateSystem(0));
	QVector<QPointF> listScene = cSystem->mapLogicalToScene(QVector<QPointF>{position});
	if (!listScene.isEmpty()) {
		m_visible = true;
		positionScene = listScene.at(0);
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
	if (m_visible && symbolStyle != Symbol::Style::NoSymbols) {
		QPainterPath path = Symbol::pathFromStyle(symbolStyle);

		QTransform trafo;
		trafo.scale(symbolSize, symbolSize);
		path = trafo.map(path);
		trafo.reset();

		if (symbolRotationAngle != 0) {
			trafo.rotate(symbolRotationAngle);
			path = trafo.map(path);
		}

		pointShape.addPath(WorksheetElement::shapeFromPath(trafo.map(path), symbolPen));
		transformedBoundingRectangle = pointShape.boundingRect();
	}
}

void CustomPointPrivate::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
	Q_UNUSED(option)
	Q_UNUSED(widget)

	if (!m_visible)
		return;

	if (symbolStyle != Symbol::Style::NoSymbols) {
		painter->setOpacity(symbolOpacity);
		painter->setPen(symbolPen);
		painter->setBrush(symbolBrush);
		painter->drawPath(pointShape);
	}

	if (m_hovered && !isSelected() && !m_printing) {
		painter->setPen(QPen(QApplication::palette().color(QPalette::Shadow), 2, Qt::SolidLine));
		painter->drawPath(pointShape);
	}

	if (isSelected() && !m_printing) {
		painter->setPen(QPen(QApplication::palette().color(QPalette::Highlight), 2, Qt::SolidLine));
		painter->drawPath(pointShape);
	}
}

QVariant CustomPointPrivate::itemChange(GraphicsItemChange change, const QVariant &value) {
	if (suppressItemChangeEvent)
		return value;

	if (change == QGraphicsItem::ItemPositionChange) {
		//emit the signals in order to notify the UI.
		//TODO
		const auto* cSystem = dynamic_cast<const CartesianCoordinateSystem*>(plot->coordinateSystem(0));
		QPointF scenePos = mapParentToPlotArea(value.toPointF());
		QPointF logicalPos = cSystem->mapSceneToLogical(scenePos); // map parent to scene
		//q->setPosition(logicalPos);

		// not needed, because positionChanged trigger the widget, which sets the position
		//position = logicalPos; // don't use setPosition, because this will call retransform and then again this function
		emit q->positionChanged(logicalPos);
	}

	return QGraphicsItem::itemChange(change, value);
}

void CustomPointPrivate::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
	if (q->parentAspect()->type() == AspectType::InfoElement)
		return; // don't move when the parent is a InfoElement, because there nou custompoint position change by mouse is not allowed

	//position was changed -> set the position member variables
	suppressRetransform = true;
	//TODO
	const auto* cSystem = dynamic_cast<const CartesianCoordinateSystem*>(plot->coordinateSystem(0));
	q->setPosition(cSystem->mapSceneToLogical(pos()));
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
	AbstractAspect* parent = q->parent(AspectType::CartesianPlot);

	if (parent) {
		CartesianPlot* plot = static_cast<CartesianPlot*>(parent);
		// first mapping to item coordinates and from there back to parent
		// WorksheetinfoElement: parentItem()->parentItem() == plot->graphicsItem()
		// plot->graphicsItem().pos() == plot->plotArea()->graphicsItem().pos()
		return mapToParent(mapFromItem(plot->plotArea()->graphicsItem(), point));
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
	AbstractAspect* parent = q->parent(AspectType::CartesianPlot);
	if (parent) {
		CartesianPlot* plot = static_cast<CartesianPlot*>(parent);
		// mapping from parent to item coordinates and them to plot area
		return mapToItem(plot->plotArea()->graphicsItem(), mapFromParent(point));
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
	writer->writeAttribute( "visible", QString::number(d->isVisible()) );
	writer->writeEndElement();

	//Symbols
	writer->writeStartElement("symbol");
	writer->writeAttribute( "symbolStyle", QString::number(static_cast<int>(d->symbolStyle)) );
	writer->writeAttribute( "opacity", QString::number(d->symbolOpacity) );
	writer->writeAttribute( "rotation", QString::number(d->symbolRotationAngle) );
	writer->writeAttribute( "size", QString::number(d->symbolSize) );
	WRITE_QBRUSH(d->symbolBrush);
	WRITE_QPEN(d->symbolPen);
	writer->writeEndElement();

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

			str = attribs.value("visible").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("visible").toString());
			else
				d->setVisible(str.toInt());
		} else if (!preview && reader->name() == "symbol") {
			attribs = reader->attributes();

			READ_INT_VALUE("symbolStyle", symbolStyle, Symbol::Style);
			READ_DOUBLE_VALUE("opacity", symbolOpacity);
			READ_DOUBLE_VALUE("rotation", symbolRotationAngle);
			READ_DOUBLE_VALUE("size", symbolSize);
			READ_QBRUSH(d->symbolBrush);
			READ_QPEN(d->symbolPen);
		} else { // unknown element
			reader->raiseWarning(i18n("unknown element '%1'", reader->name().toString()));
			if (!reader->skipToEndElement()) return false;
		}
	}

	retransform();
	return true;
}
