/*
	File                 : DatapickerPoint.cpp
	Project              : LabPlot
	Description          : Graphic Item for coordinate points of Datapicker
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2015 Ankit Wagadre <wagadre.ankit@gmail.com>
	SPDX-FileCopyrightText: 2015-2024 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "DatapickerPoint.h"
#include "DatapickerPointPrivate.h"
#include "backend/datapicker/DatapickerCurve.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/commandtemplates.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/Symbol.h"

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QMenu>
#include <QPainter>

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>

/**
 * \class ErrorBarItem
 * \brief A customizable error-bar for DatapickerPoint.
 */

ErrorBarItem::ErrorBarItem(DatapickerPoint* parent, ErrorBarType type)
	: QGraphicsRectItem(parent->graphicsItem())
	, barLineItem(new QGraphicsLineItem(parent->graphicsItem()))
	, m_type(type)
	, m_parentItem(parent) {
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
	QTransform matrix;
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
	if (m_type == ErrorBarType::PlusDeltaX || m_type == ErrorBarType::MinusDeltaX)
		setCursor(Qt::SizeHorCursor);
	else
		setCursor(Qt::SizeVerCursor);
}

QVariant ErrorBarItem::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value) {
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
	: AbstractAspect(name, AspectType::DatapickerPoint)
	, d_ptr(new DatapickerPointPrivate(this)) {
	init();
}

DatapickerPoint::DatapickerPoint(const QString& name, DatapickerPointPrivate* dd)
	: AbstractAspect(name, AspectType::DatapickerPoint)
	, d_ptr(dd) {
	init();
}

// no need to delete the d-pointer here - it inherits from QGraphicsItem
// and is deleted during the cleanup in QGraphicsScene
DatapickerPoint::~DatapickerPoint() = default;

void DatapickerPoint::finalizeAdd() {
	// call retransform _after_ the parent (image or curve) was set and we can determin the properties of the parent (symbol, etc.)
	// TODO: do we need a full retransform here or is calling d->updateProperties() enough?
	retransform();
}

void DatapickerPoint::init() {
	Q_D(DatapickerPoint);

	KConfig config;
	KConfigGroup group = config.group(QStringLiteral("DatapickerPoint"));
	d->position.setX(group.readEntry(QStringLiteral("PositionXValue"), Worksheet::convertToSceneUnits(1, Worksheet::Unit::Centimeter)));
	d->position.setY(group.readEntry(QStringLiteral("PositionYValue"), Worksheet::convertToSceneUnits(1, Worksheet::Unit::Centimeter)));
	d->plusDeltaXPos = group.readEntry(QStringLiteral("PlusDeltaXPos"), QPointF(30, 0));
	d->minusDeltaXPos = group.readEntry(QStringLiteral("MinusDeltaXPos"), QPointF(-30, 0));
	d->plusDeltaYPos = group.readEntry(QStringLiteral("PlusDeltaYPos"), QPointF(0, -30));
	d->minusDeltaYPos = group.readEntry(QStringLiteral("MinusDeltaYPos"), QPointF(0, 30));
}

void DatapickerPoint::initErrorBar(DatapickerCurve::Errors errors) {
	if (m_errorBarItemList.isEmpty() && errors.x == DatapickerCurve::ErrorType::NoError && errors.y == DatapickerCurve::ErrorType::NoError)
		return; // no need to update
	m_errorBarItemList.clear();
	if (errors.x != DatapickerCurve::ErrorType::NoError) {
		auto* plusDeltaXItem = new ErrorBarItem(this, ErrorBarItem::ErrorBarType::PlusDeltaX);
		plusDeltaXItem->setPosition(plusDeltaXPos());
		connect(this, &DatapickerPoint::plusDeltaXPosChanged, plusDeltaXItem, &ErrorBarItem::setPosition);

		auto* minusDeltaXItem = new ErrorBarItem(this, ErrorBarItem::ErrorBarType::MinusDeltaX);
		minusDeltaXItem->setPosition(minusDeltaXPos());
		connect(this, &DatapickerPoint::minusDeltaXPosChanged, minusDeltaXItem, &ErrorBarItem::setPosition);

		m_errorBarItemList << plusDeltaXItem << minusDeltaXItem;
	}

	if (errors.y != DatapickerCurve::ErrorType::NoError) {
		auto* plusDeltaYItem = new ErrorBarItem(this, ErrorBarItem::ErrorBarType::PlusDeltaY);
		plusDeltaYItem->setPosition(plusDeltaYPos());
		connect(this, &DatapickerPoint::plusDeltaYPosChanged, plusDeltaYItem, &ErrorBarItem::setPosition);

		auto* minusDeltaYItem = new ErrorBarItem(this, ErrorBarItem::ErrorBarType::MinusDeltaY);
		minusDeltaYItem->setPosition(minusDeltaYPos());
		connect(this, &DatapickerPoint::minusDeltaYPosChanged, minusDeltaYItem, &ErrorBarItem::setPosition);

		m_errorBarItemList << plusDeltaYItem << minusDeltaYItem;
	}

	retransform();
}

/*!
	Returns an icon to be used in the project explorer.
*/
QIcon DatapickerPoint::icon() const {
	return QIcon::fromTheme(QStringLiteral("draw-cross"));
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
// point
BASIC_SHARED_D_READER_IMPL(DatapickerPoint, QPointF, position, position)
// error-bar
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
			setMinusDeltaXPos(QPointF(-std::abs(pos.x()), pos.y()));
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
			setPlusDeltaXPos(QPointF(std::abs(pos.x()), pos.y()));
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
			setMinusDeltaYPos(QPointF(pos.x(), std::abs(pos.y())));
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
			setPlusDeltaYPos(QPointF(pos.x(), -std::abs(pos.y())));
		} else
			exec(new DatapickerPointSetMinusDeltaYPosCmd(d, pos, ki18n("%1: set -delta_Y position")));
		endMacro();
	}
}

void DatapickerPoint::setPrinting(bool on) {
	Q_D(DatapickerPoint);
	d->m_printing = on;
}

void DatapickerPoint::setIsReferencePoint(bool value) {
	Q_D(DatapickerPoint);
	d->isReferencePoint = value;
}

bool DatapickerPoint::isReferencePoint() const {
	Q_D(const DatapickerPoint);
	return d->isReferencePoint;
}

// ##############################################################################
// ####################### Private implementation ###############################
// ##############################################################################
DatapickerPointPrivate::DatapickerPointPrivate(DatapickerPoint* owner)
	: q(owner) {
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
	if (q->isLoading())
		return;

	setPos(position);
	updatePoint();
	updateProperties();
	recalcShapeAndBoundingRect();
	retransformErrorBar();
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

void DatapickerPointPrivate::updateProperties() {
	auto* curve = dynamic_cast<DatapickerCurve*>(q->parentAspect());
	auto* image = dynamic_cast<DatapickerImage*>(q->parentAspect());
	if (image) {
		symbol = image->symbol();
		setVisible(image->pointVisibility());
	} else if (curve) {
		symbol = curve->symbol();
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
	return m_boundingRectangle;
}

/*!
	Returns the shape of this item as a QPainterPath in local coordinates.
*/
QPainterPath DatapickerPointPrivate::shape() const {
	return m_shape;
}

/*!
  recalculates the outer bounds and the shape of the item.
*/
void DatapickerPointPrivate::recalcShapeAndBoundingRect() {
	prepareGeometryChange();

	QPainterPath path = Symbol::stylePath(symbol->style());
	QTransform trafo;
	trafo.scale(symbol->size(), symbol->size());
	path = trafo.map(path);
	trafo.reset();

	if (symbol->rotationAngle() != 0.) {
		trafo.rotate(symbol->rotationAngle());
		path = trafo.map(path);
	}

	m_shape.addPath(WorksheetElement::shapeFromPath(trafo.map(path), symbol->pen()));
	m_boundingRectangle = m_shape.boundingRect();
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

QVariant DatapickerPointPrivate::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value) {
	if (change == QGraphicsItem::GraphicsItemChange::ItemSelectedHasChanged && value.toBool())
		Q_EMIT q->pointSelected(q);
	else if (change == QGraphicsItem::GraphicsItemChange::ItemPositionChange)
		Q_EMIT q->positionChanged(value.toPointF());
	return QGraphicsItem::itemChange(change, value);
}

void DatapickerPointPrivate::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget*) {
	symbol->draw(painter, QPointF(0., 0.));

	if (isSelected() && !m_printing) {
		// TODO: move the initialization of QPen to a parent class later so we don't
		// need to create it in every paint() call.
		painter->setPen(QPen(QApplication::palette().color(QPalette::Highlight), 1, Qt::SolidLine));
		painter->drawPath(m_shape);
	}
}

void DatapickerPointPrivate::contextMenuEvent(QGraphicsSceneContextMenuEvent* event) {
	q->createContextMenu()->exec(event->screenPos());
}

// ##############################################################################
// ##################  Serialization/Deserialization  ###########################
// ##############################################################################
//! Save as XML
void DatapickerPoint::save(QXmlStreamWriter* writer) const {
	Q_D(const DatapickerPoint);

	writer->writeStartElement(QStringLiteral("datapickerPoint"));
	writeBasicAttributes(writer);

	// geometry
	writer->writeStartElement(QStringLiteral("geometry"));
	writer->writeAttribute(QStringLiteral("x"), QString::number(d->position.x()));
	writer->writeAttribute(QStringLiteral("y"), QString::number(d->position.y()));
	writer->writeEndElement();

	auto* curve = dynamic_cast<DatapickerCurve*>(parentAspect());
	if (curve && (curve->curveErrorTypes().x != DatapickerCurve::ErrorType::NoError || curve->curveErrorTypes().y != DatapickerCurve::ErrorType::NoError)) {
		writer->writeStartElement(QStringLiteral("errorBar"));
		writer->writeAttribute(QStringLiteral("plusDeltaXPos_x"), QString::number(d->plusDeltaXPos.x()));
		writer->writeAttribute(QStringLiteral("plusDeltaXPos_y"), QString::number(d->plusDeltaXPos.y()));
		writer->writeAttribute(QStringLiteral("minusDeltaXPos_x"), QString::number(d->minusDeltaXPos.x()));
		writer->writeAttribute(QStringLiteral("minusDeltaXPos_y"), QString::number(d->minusDeltaXPos.y()));
		writer->writeAttribute(QStringLiteral("plusDeltaYPos_x"), QString::number(d->plusDeltaYPos.x()));
		writer->writeAttribute(QStringLiteral("plusDeltaYPos_y"), QString::number(d->plusDeltaYPos.y()));
		writer->writeAttribute(QStringLiteral("minusDeltaYPos_x"), QString::number(d->minusDeltaYPos.x()));
		writer->writeAttribute(QStringLiteral("minusDeltaYPos_y"), QString::number(d->minusDeltaYPos.y()));
		writer->writeEndElement();
	}

	writer->writeEndElement(); // close "DatapickerPoint" section
}

//! Load from XML
bool DatapickerPoint::load(XmlStreamReader* reader, bool preview) {
	Q_D(DatapickerPoint);

	if (!readBasicAttributes(reader))
		return false;

	QXmlStreamAttributes attribs;
	QString str;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == QStringLiteral("datapickerPoint"))
			break;

		if (!reader->isStartElement())
			continue;

		if (!preview && reader->name() == QStringLiteral("geometry")) {
			attribs = reader->attributes();

			str = attribs.value(QStringLiteral("x")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("x"));
			else
				d->position.setX(str.toDouble());

			str = attribs.value(QStringLiteral("y")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("y"));
			else
				d->position.setY(str.toDouble());
		} else if (!preview && reader->name() == QStringLiteral("errorBar")) {
			attribs = reader->attributes();

			str = attribs.value(QStringLiteral("plusDeltaXPos_x")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("plusDeltaXPos_x"));
			else
				d->plusDeltaXPos.setX(str.toDouble());

			str = attribs.value(QStringLiteral("plusDeltaXPos_y")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("plusDeltaXPos_y"));
			else
				d->plusDeltaXPos.setY(str.toDouble());

			str = attribs.value(QStringLiteral("minusDeltaXPos_x")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("minusDeltaXPos_x"));
			else
				d->minusDeltaXPos.setX(str.toDouble());

			str = attribs.value(QStringLiteral("minusDeltaXPos_y")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("minusDeltaXPos_y"));
			else
				d->minusDeltaXPos.setY(str.toDouble());

			str = attribs.value(QStringLiteral("plusDeltaYPos_x")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("plusDeltaYPos_x"));
			else
				d->plusDeltaYPos.setX(str.toDouble());

			str = attribs.value(QStringLiteral("plusDeltaYPos_y")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("plusDeltaYPos_y"));
			else
				d->plusDeltaYPos.setY(str.toDouble());

			str = attribs.value(QStringLiteral("minusDeltaYPos_x")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("minusDeltaYPos_x"));
			else
				d->minusDeltaYPos.setX(str.toDouble());

			str = attribs.value(QStringLiteral("minusDeltaYPos_y")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("minusDeltaYPos_y"));
			else
				d->minusDeltaYPos.setY(str.toDouble());
		} else { // unknown element
			reader->raiseUnknownElementWarning();
			if (!reader->skipToEndElement())
				return false;
		}
	}

	return true;
}
