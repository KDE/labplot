/*
    File                 : DatapickerPoint.cpp
    Project              : LabPlot
    Description          : Graphic Item for coordinate points of Datapicker
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2015 Ankit Wagadre (wagadre.ankit@gmail.com)
	SPDX-FileCopyrightText: 2015-2021 Alexander Semke (alexander.semke@web.de)
*/
/***************************************************************************
 *                                                                         *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/

#include "DatapickerPoint.h"
#include "DatapickerPointPrivate.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/datapicker/DatapickerCurve.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/Symbol.h"

#include <QPainter>
#include <QGraphicsScene>
#include <QMenu>
#include <QGraphicsSceneMouseEvent>

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>

/**
 * \class ErrorBarItem
 * \brief A customizable error-bar for DatapickerPoint.
 */

ErrorBarItem::ErrorBarItem(DatapickerPoint* parent, ErrorBarType type) :
	QGraphicsRectItem(parent->graphicsItem()),
	barLineItem(new QGraphicsLineItem(parent->graphicsItem())),
	m_type(type),
	m_parentItem(parent) {
	setFlag(QGraphicsItem::ItemIsMovable);
	setFlag(QGraphicsItem::ItemIsSelectable);
	setFlag(QGraphicsItem::ItemSendsGeometryChanges);
	initRect();
	setAcceptHoverEvents(true);
}

void ErrorBarItem::initRect() {
	QRectF xBarRect(-0.15, -0.5, 0.3, 1);
	QRectF yBarRect(-0.5, -0.15, 1, 0.3);

	if (m_type == ErrorBarType::PlusDeltaX || m_type == ErrorBarType::MinusDeltaX)
		m_rect = xBarRect;
	else
		m_rect = yBarRect;
}

void ErrorBarItem::setPosition(QPointF position) {
	setPos(position);
	barLineItem->setLine(0, 0, position.x(), position.y());
}

void ErrorBarItem::setRectSize(qreal size) {
	QMatrix matrix;
	matrix.scale(size, size);
	setRect(matrix.mapRect(m_rect));
}

void ErrorBarItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
	if (m_type == ErrorBarType::PlusDeltaX)
		m_parentItem->setPlusDeltaXPos(pos());
	else if (m_type == ErrorBarType::MinusDeltaX)
		m_parentItem->setMinusDeltaXPos(pos());
	else if (m_type == ErrorBarType::PlusDeltaY)
		m_parentItem->setPlusDeltaYPos(pos());
	else if (m_type == ErrorBarType::MinusDeltaY)
		m_parentItem->setMinusDeltaYPos(pos());

	QGraphicsItem::mouseReleaseEvent(event);
}

void ErrorBarItem::hoverEnterEvent(QGraphicsSceneHoverEvent*) {
	if (m_type == ErrorBarType::PlusDeltaX ||m_type == ErrorBarType::MinusDeltaX)
		setCursor(Qt::SizeHorCursor);
	else
		setCursor(Qt::SizeVerCursor);
}

QVariant ErrorBarItem::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value) {
	if (change == QGraphicsItem::ItemPositionChange) {
		QPointF newPos = value.toPointF();
		if (m_type == ErrorBarType::PlusDeltaX || m_type == ErrorBarType::MinusDeltaX) {
			newPos.setY(0);
			barLineItem->setLine(0, 0, newPos.x(), 0);
		} else {
			newPos.setX(0);
			barLineItem->setLine(0, 0, 0, newPos.y());
		}
		return QGraphicsRectItem::itemChange(change, newPos);
	}

	return QGraphicsRectItem::itemChange(change, value);
}

/**
 * \class Datapicker-Point
 * \brief A customizable symbol supports error-bars.
 *
 * The datapicker-Point is aligned relative to the specified position.
 * The position can be either specified by mouse events or by providing the
 * x- and y- coordinates in parent's coordinate system, or by specifying one
 * of the predefined position flags (\c HorizontalPosition, \c VerticalPosition).
 */

DatapickerPoint::DatapickerPoint(const QString& name)
	: AbstractAspect(name, AspectType::DatapickerPoint), d_ptr(new DatapickerPointPrivate(this)) {

	init();
}

DatapickerPoint::DatapickerPoint(const QString& name, DatapickerPointPrivate *dd)
	: AbstractAspect(name, AspectType::DatapickerPoint), d_ptr(dd) {

	init();
}

//no need to delete the d-pointer here - it inherits from QGraphicsItem
//and is deleted during the cleanup in QGraphicsScene
DatapickerPoint::~DatapickerPoint() = default;

void DatapickerPoint::init() {
	Q_D(DatapickerPoint);

	KConfig config;
	KConfigGroup group;
	group = config.group("DatapickerPoint");
	d->position.setX( group.readEntry("PositionXValue", Worksheet::convertToSceneUnits(1, Worksheet::Unit::Centimeter)) );
	d->position.setY( group.readEntry("PositionYValue", Worksheet::convertToSceneUnits(1, Worksheet::Unit::Centimeter)) );
	d->plusDeltaXPos = group.readEntry("PlusDeltaXPos", QPointF(30, 0));
	d->minusDeltaXPos = group.readEntry("MinusDeltaXPos", QPointF(-30, 0));
	d->plusDeltaYPos = group.readEntry("PlusDeltaYPos", QPointF(0, -30));
	d->minusDeltaYPos = group.readEntry("MinusDeltaYPos", QPointF(0, 30));
}

void DatapickerPoint::initErrorBar(DatapickerCurve::Errors errors) {
	m_errorBarItemList.clear();
	if (errors.x != DatapickerCurve::ErrorType::NoError) {
		auto* plusDeltaXItem = new ErrorBarItem(this, ErrorBarItem::ErrorBarType::PlusDeltaX);
		plusDeltaXItem->setPosition(plusDeltaXPos());
		connect(this, &DatapickerPoint::plusDeltaXPosChanged, plusDeltaXItem, &ErrorBarItem::setPosition);

		auto* minusDeltaXItem = new ErrorBarItem(this, ErrorBarItem::ErrorBarType::MinusDeltaX);
		minusDeltaXItem->setPosition(minusDeltaXPos());
		connect(this, &DatapickerPoint::minusDeltaXPosChanged, minusDeltaXItem, &ErrorBarItem::setPosition);

		m_errorBarItemList<<plusDeltaXItem<<minusDeltaXItem;
	}

	if (errors.y != DatapickerCurve::ErrorType::NoError) {
		auto* plusDeltaYItem = new ErrorBarItem(this, ErrorBarItem::ErrorBarType::PlusDeltaY);
		plusDeltaYItem->setPosition(plusDeltaYPos());
		connect(this, &DatapickerPoint::plusDeltaYPosChanged, plusDeltaYItem, &ErrorBarItem::setPosition);

		auto* minusDeltaYItem = new ErrorBarItem(this, ErrorBarItem::ErrorBarType::MinusDeltaY);
		minusDeltaYItem->setPosition(minusDeltaYPos());
		connect(this, &DatapickerPoint::minusDeltaYPosChanged, minusDeltaYItem, &ErrorBarItem::setPosition);

		m_errorBarItemList<<plusDeltaYItem<<minusDeltaYItem;
	}

	retransform();
}

/*!
    Returns an icon to be used in the project explorer.
*/
QIcon DatapickerPoint::icon() const {
	return QIcon::fromTheme("draw-cross");
}

QMenu* DatapickerPoint::createContextMenu() {
	QMenu* menu = AbstractAspect::createContextMenu();
	return menu;
}

QGraphicsItem* DatapickerPoint::graphicsItem() const {
	return d_ptr;
}

void DatapickerPoint::setParentGraphicsItem(QGraphicsItem* item) {
	Q_D(DatapickerPoint);
	d->setParentItem(item);
}

void DatapickerPoint::retransform() {
	Q_D(DatapickerPoint);
	d->retransform();
}

/* ============================ getter methods ================= */
//point
BASIC_SHARED_D_READER_IMPL(DatapickerPoint, QPointF, position, position)
//error-bar
BASIC_SHARED_D_READER_IMPL(DatapickerPoint, QPointF, plusDeltaXPos, plusDeltaXPos)
BASIC_SHARED_D_READER_IMPL(DatapickerPoint, QPointF, minusDeltaXPos, minusDeltaXPos)
BASIC_SHARED_D_READER_IMPL(DatapickerPoint, QPointF, plusDeltaYPos, plusDeltaYPos)
BASIC_SHARED_D_READER_IMPL(DatapickerPoint, QPointF, minusDeltaYPos, minusDeltaYPos)

/* ============================ setter methods and undo commands ================= */
STD_SETTER_CMD_IMPL_F_S(DatapickerPoint, SetPosition, QPointF, position, retransform)
void DatapickerPoint::setPosition(QPointF pos) {
	Q_D(DatapickerPoint);
	if (pos != d->position)
		exec(new DatapickerPointSetPositionCmd(d, pos, ki18n("%1: set position")));
}

STD_SETTER_CMD_IMPL_F_S(DatapickerPoint, SetPlusDeltaXPos, QPointF, plusDeltaXPos, updatePoint)
void DatapickerPoint::setPlusDeltaXPos(QPointF pos) {
	Q_D(DatapickerPoint);
	if (pos != d->plusDeltaXPos) {
		auto* curve = dynamic_cast<DatapickerCurve*>(parentAspect());
		if (!curve)
			return;

		beginMacro(i18n("%1: set +delta_X position", name()));
		if (curve->curveErrorTypes().x == DatapickerCurve::ErrorType::SymmetricError) {
			exec(new DatapickerPointSetPlusDeltaXPosCmd(d, pos, ki18n("%1: set +delta X position")));
			setMinusDeltaXPos(QPointF(-qAbs(pos.x()), pos.y()));
		} else
			exec(new DatapickerPointSetPlusDeltaXPosCmd(d, pos, ki18n("%1: set +delta X position")));
		endMacro();
	}
}

STD_SETTER_CMD_IMPL_F_S(DatapickerPoint, SetMinusDeltaXPos, QPointF, minusDeltaXPos, updatePoint)
void DatapickerPoint::setMinusDeltaXPos(QPointF pos) {
	Q_D(DatapickerPoint);
	if (pos != d->minusDeltaXPos) {
		auto* curve = dynamic_cast<DatapickerCurve*>(parentAspect());
		if (!curve)
			return;

		beginMacro(i18n("%1: set -delta_X position", name()));
		if (curve->curveErrorTypes().x == DatapickerCurve::ErrorType::SymmetricError) {
			exec(new DatapickerPointSetMinusDeltaXPosCmd(d, pos, ki18n("%1: set -delta_X position")));
			setPlusDeltaXPos(QPointF(qAbs(pos.x()), pos.y()));
		} else
			exec(new DatapickerPointSetMinusDeltaXPosCmd(d, pos, ki18n("%1: set -delta_X position")));
		endMacro();
	}
}

STD_SETTER_CMD_IMPL_F_S(DatapickerPoint, SetPlusDeltaYPos, QPointF, plusDeltaYPos, updatePoint)
void DatapickerPoint::setPlusDeltaYPos(QPointF pos) {
	Q_D(DatapickerPoint);
	if (pos != d->plusDeltaYPos) {
		auto* curve = dynamic_cast<DatapickerCurve*>(parentAspect());
		if (!curve)
			return;

		beginMacro(i18n("%1: set +delta_Y position", name()));
		if (curve->curveErrorTypes().y == DatapickerCurve::ErrorType::SymmetricError) {
			exec(new DatapickerPointSetPlusDeltaYPosCmd(d, pos, ki18n("%1: set +delta_Y position")));
			setMinusDeltaYPos(QPointF(pos.x(), qAbs(pos.y())));
		} else
			exec(new DatapickerPointSetPlusDeltaYPosCmd(d, pos, ki18n("%1: set +delta_Y position")));
		endMacro();
	}
}

STD_SETTER_CMD_IMPL_F_S(DatapickerPoint, SetMinusDeltaYPos, QPointF, minusDeltaYPos, updatePoint)
void DatapickerPoint::setMinusDeltaYPos(QPointF pos) {
	Q_D(DatapickerPoint);
	if (pos != d->minusDeltaYPos) {
		auto* curve = dynamic_cast<DatapickerCurve*>(parentAspect());
		if (!curve)
			return;

		beginMacro(i18n("%1: set -delta_Y position", name()));
		if (curve->curveErrorTypes().y == DatapickerCurve::ErrorType::SymmetricError) {
			exec(new DatapickerPointSetMinusDeltaYPosCmd(d, pos, ki18n("%1: set -delta_Y position")));
			setPlusDeltaYPos(QPointF(pos.x(), -qAbs(pos.y())));
		} else
			exec(new DatapickerPointSetMinusDeltaYPosCmd(d, pos, ki18n("%1: set -delta_Y position")));
		endMacro();
	}
}

void DatapickerPoint::setPrinting(bool on) {
	Q_D(DatapickerPoint);
	d->m_printing = on;
}

//##############################################################################
//####################### Private implementation ###############################
//##############################################################################
DatapickerPointPrivate::DatapickerPointPrivate(DatapickerPoint* owner) : q(owner) {
	setFlag(QGraphicsItem::ItemIsMovable);
	setFlag(QGraphicsItem::ItemSendsGeometryChanges);
	setFlag(QGraphicsItem::ItemIsSelectable);
	setAcceptHoverEvents(true);
}

QString DatapickerPointPrivate::name() const {
	return q->name();
}

/*!
    calculates the position and the bounding box of the item/point. Called on geometry or properties changes.
 */
void DatapickerPointPrivate::retransform() {
	updatePropeties();
	setPos(position);
	QPainterPath path = Symbol::pathFromStyle(pointStyle);
	boundingRectangle = path.boundingRect();
	recalcShapeAndBoundingRect();
	retransformErrorBar();
	updatePoint();
}

/*!
  update color and size of all error-bar.
*/
void DatapickerPointPrivate::retransformErrorBar() {
	for (auto* item : q->m_errorBarItemList) {
		if (item) {
			item->setBrush(errorBarBrush);
			item->setPen(errorBarPen);
			item->setRectSize(errorBarSize);
		}
	}
}

/*!
  update datasheet on any change in position of Datapicker-Point or it's error-bar.
*/
void DatapickerPointPrivate::updatePoint() {
	auto* curve = dynamic_cast<DatapickerCurve*>(q->parentAspect());
	if (curve)
		curve->updatePoint(q);
}

void DatapickerPointPrivate::updatePropeties() {
	auto* curve = dynamic_cast<DatapickerCurve*>(q->parentAspect());
	auto* image = dynamic_cast<DatapickerImage*>(q->parentAspect());
	if (image) {
		rotationAngle = image->symbol()->rotationAngle();
		pointStyle = image->symbol()->style();
		brush = image->symbol()->brush();
		pen = image->symbol()->pen();
		opacity = image->symbol()->opacity();
		size = image->symbol()->size();
		setVisible(image->pointVisibility());
	} else if (curve) {
		rotationAngle = curve->symbol()->rotationAngle();
		pointStyle = curve->symbol()->style();
		brush = curve->symbol()->brush();
		pen = curve->symbol()->pen();
		opacity = curve->symbol()->opacity();
		size = curve->symbol()->size();
		errorBarBrush = curve->pointErrorBarBrush();
		errorBarPen = curve->pointErrorBarPen();
		errorBarSize = curve->pointErrorBarSize();
		setVisible(curve->pointVisibility());
	}
}

/*!
    Returns the outer bounds of the item as a rectangle.
 */
QRectF DatapickerPointPrivate::boundingRect() const {
	return transformedBoundingRectangle;
}

/*!
    Returns the shape of this item as a QPainterPath in local coordinates.
*/
QPainterPath DatapickerPointPrivate::shape() const {
	return itemShape;
}

/*!
  recalculates the outer bounds and the shape of the item.
*/
void DatapickerPointPrivate::recalcShapeAndBoundingRect() {
	prepareGeometryChange();

	QMatrix matrix;
	matrix.scale(size, size);
	matrix.rotate(-rotationAngle);
	transformedBoundingRectangle = matrix.mapRect(boundingRectangle);
	itemShape = QPainterPath();
	itemShape.addRect(transformedBoundingRectangle);
	itemShape = WorksheetElement::shapeFromPath(itemShape, pen);
}

void DatapickerPointPrivate::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
	q->setPosition(pos());
	QGraphicsItem::mouseReleaseEvent(event);
}

void DatapickerPointPrivate::hoverEnterEvent(QGraphicsSceneHoverEvent*) {
	setCursor(Qt::ArrowCursor);
}

void DatapickerPointPrivate::hoverLeaveEvent(QGraphicsSceneHoverEvent*) {
	setCursor(Qt::CrossCursor);
}

void DatapickerPointPrivate::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
	Q_UNUSED(option)
	Q_UNUSED(widget)

	QPainterPath path = Symbol::pathFromStyle(pointStyle);
	QTransform trafo;
	trafo.scale(size, size);
	path = trafo.map(path);
	trafo.reset();
	if (rotationAngle != 0) {
		trafo.rotate(-rotationAngle);
		path = trafo.map(path);
	}
	painter->save();
	painter->setPen(pen);
	painter->setBrush(brush);
	painter->setOpacity(opacity);
	painter->drawPath(path);
	painter->restore();

	if (isSelected() && !m_printing) {
		//TODO: move the initialization of QPen to a parent class later so we don't
		//need to create it in every paint() call.
		painter->setPen(QPen(QApplication::palette().color(QPalette::Highlight), 1, Qt::SolidLine));
		painter->drawPath(itemShape);
	}
}

void DatapickerPointPrivate::contextMenuEvent(QGraphicsSceneContextMenuEvent* event) {
	q->createContextMenu()->exec(event->screenPos());
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################
//! Save as XML
void DatapickerPoint::save(QXmlStreamWriter* writer) const {
	Q_D(const DatapickerPoint);

	writer->writeStartElement( "datapickerPoint" );
	writeBasicAttributes(writer);

	//geometry
	writer->writeStartElement( "geometry" );
	writer->writeAttribute( "x", QString::number(d->position.x()) );
	writer->writeAttribute( "y", QString::number(d->position.y()) );
	writer->writeEndElement();

	auto* curve = dynamic_cast<DatapickerCurve*>(parentAspect());
	if (curve && (curve->curveErrorTypes().x != DatapickerCurve::ErrorType::NoError
		|| curve->curveErrorTypes().y != DatapickerCurve::ErrorType::NoError)) {

		writer->writeStartElement( "errorBar" );
		writer->writeAttribute( "plusDeltaXPos_x", QString::number(d->plusDeltaXPos.x()) );
		writer->writeAttribute( "plusDeltaXPos_y", QString::number(d->plusDeltaXPos.y()) );
		writer->writeAttribute( "minusDeltaXPos_x", QString::number(d->minusDeltaXPos.x()) );
		writer->writeAttribute( "minusDeltaXPos_y", QString::number(d->minusDeltaXPos.y()) );
		writer->writeAttribute( "plusDeltaYPos_x", QString::number(d->plusDeltaYPos.x()) );
		writer->writeAttribute( "plusDeltaYPos_y", QString::number(d->plusDeltaYPos.y()) );
		writer->writeAttribute( "minusDeltaYPos_x", QString::number(d->minusDeltaYPos.x()) );
		writer->writeAttribute( "minusDeltaYPos_y", QString::number(d->minusDeltaYPos.y()) );
		writer->writeEndElement();
	}

	writer->writeEndElement(); // close "DatapickerPoint" section
}

//! Load from XML
bool DatapickerPoint::load(XmlStreamReader* reader, bool preview) {
	Q_D(DatapickerPoint);

	if (!readBasicAttributes(reader))
		return false;

	KLocalizedString attributeWarning = ki18n("Attribute '%1' missing or empty, default value is used");
	QXmlStreamAttributes attribs;
	QString str;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == "datapickerPoint")
			break;

		if (!reader->isStartElement())
			continue;

		if (!preview && reader->name() == "geometry") {
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
		} else if (!preview && reader->name() == "errorBar") {
			attribs = reader->attributes();

			str = attribs.value("plusDeltaXPos_x").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("plusDeltaXPos_x").toString());
			else
				d->plusDeltaXPos.setX(str.toDouble());

			str = attribs.value("plusDeltaXPos_y").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("plusDeltaXPos_y").toString());
			else
				d->plusDeltaXPos.setY(str.toDouble());

			str = attribs.value("minusDeltaXPos_x").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("minusDeltaXPos_x").toString());
			else
				d->minusDeltaXPos.setX(str.toDouble());

			str = attribs.value("minusDeltaXPos_y").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("minusDeltaXPos_y").toString());
			else
				d->minusDeltaXPos.setY(str.toDouble());

			str = attribs.value("plusDeltaYPos_x").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("plusDeltaYPos_x").toString());
			else
				d->plusDeltaYPos.setX(str.toDouble());

			str = attribs.value("plusDeltaYPos_y").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("plusDeltaYPos_y").toString());
			else
				d->plusDeltaYPos.setY(str.toDouble());

			str = attribs.value("minusDeltaYPos_x").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("minusDeltaYPos_x").toString());
			else
				d->minusDeltaYPos.setX(str.toDouble());

			str = attribs.value("minusDeltaYPos_y").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("minusDeltaYPos_y").toString());
			else
				d->minusDeltaYPos.setY(str.toDouble());
		} else { // unknown element
			reader->raiseWarning(i18n("unknown element '%1'", reader->name().toString()));
			if (!reader->skipToEndElement()) return false;
		}
	}

	retransform();
	return true;
}
