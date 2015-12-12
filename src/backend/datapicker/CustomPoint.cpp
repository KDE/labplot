/***************************************************************************
    File                 : CustomPoint.cpp
    Project              : LabPlot
    Description          : Graphic Item for coordinate points of Datapicker
    --------------------------------------------------------------------
    Copyright            : (C) 2015 by Ankit Wagadre (wagadre.ankit@gmail.com)
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
#include "backend/worksheet/Worksheet.h"
#include "CustomPointPrivate.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/datapicker/DatapickerCurve.h"

#include <QPainter>
#include <QGraphicsScene>
#include <QMenu>
#include <QGraphicsSceneMouseEvent>

#include <KIcon>
#include <KConfig>
#include <KConfigGroup>
#include <KLocale>

/**
 * \class ErrorBarItem
 * \brief A customizable error-bar for CustomPoint.
 */

ErrorBarItem::ErrorBarItem(CustomPoint *parent, const ErrorBarType& type) : QGraphicsRectItem(parent->graphicsItem(), 0),
	barLineItem(new QGraphicsLineItem(parent->graphicsItem(), 0)),
	m_type(type),
	m_parentItem(parent) {

	setFlag(QGraphicsItem::ItemIsMovable);
	setFlag(QGraphicsItem::ItemIsSelectable);
	setFlag(QGraphicsItem::ItemSendsGeometryChanges);
	initRect();
}

void ErrorBarItem::initRect() {
	QRectF xBarRect(-0.15, -0.5, 0.3, 1);
	QRectF yBarRect(-0.5, -0.15, 1, 0.3);

	if (m_type == PlusDeltaX || m_type == MinusDeltaX)
		m_rect = xBarRect;
	else
		m_rect = yBarRect;

	setRectSize(m_parentItem->errorBarSize());
	setPen(m_parentItem->errorBarPen());
	setBrush(m_parentItem->errorBarBrush());
}

void ErrorBarItem::setPosition(const QPointF& position) {
	setPos(position);
	barLineItem->setLine(0, 0, position.x(), position.y());
}

void ErrorBarItem::setRectSize(const qreal size) {
	QMatrix matrix;
	matrix.scale(size, size);
	setRect(matrix.mapRect(m_rect));
}

void ErrorBarItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
	if (m_type == PlusDeltaX)
		m_parentItem->setPlusDeltaXPos(pos());
	else if (m_type == MinusDeltaX)
		m_parentItem->setMinusDeltaXPos(pos());
	else if (m_type == PlusDeltaY)
		m_parentItem->setPlusDeltaYPos(pos());
	else if (m_type == MinusDeltaY)
		m_parentItem->setMinusDeltaYPos(pos());

	QGraphicsItem::mouseReleaseEvent(event);
}

QVariant ErrorBarItem::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value) {
	if (change == QGraphicsItem::ItemPositionChange) {
		QPointF newPos = value.toPointF();
		barLineItem->setLine(0, 0, newPos.x(), newPos.y());
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
 * of the predefined position flags (\ca HorizontalPosition, \ca VerticalPosition).
 */

CustomPoint::CustomPoint(const QString& name):WorksheetElement(name),
    d_ptr(new CustomPointPrivate(this)) {

	init();
}

CustomPoint::CustomPoint(const QString& name, CustomPointPrivate *dd):WorksheetElement(name), d_ptr(dd) {

	init();
}

CustomPoint::~CustomPoint() {
	//no need to delete the d-pointer here - it inherits from QGraphicsItem
	//and is deleted during the cleanup in QGraphicsScene
}

void CustomPoint::init() {
    Q_D(CustomPoint);

	KConfig config;
	KConfigGroup group;
    group = config.group("CustomPoint");
    d->position.horizontalPosition = (HorizontalPosition) group.readEntry("PositionX", (int)CustomPoint::hPositionCustom);
    d->position.verticalPosition = (VerticalPosition) group.readEntry("PositionY", (int) CustomPoint::vPositionCustom);
	d->position.point.setX( group.readEntry("PositionXValue", Worksheet::convertToSceneUnits(1, Worksheet::Centimeter)) );
	d->position.point.setY( group.readEntry("PositionYValue", Worksheet::convertToSceneUnits(1, Worksheet::Centimeter)) );
	d->scaleFactor = Worksheet::convertToSceneUnits(1, Worksheet::Point);
    d->pointStyle = (CustomPoint::PointsStyle)group.readEntry("PointStyle", (int)CustomPoint::Cross);
	d->size = group.readEntry("Size", Worksheet::convertToSceneUnits(2, Worksheet::Point));
	d->rotationAngle = group.readEntry("Rotation", 0.0);
	d->opacity = group.readEntry("Opacity", 1.0);
	d->brush.setStyle( (Qt::BrushStyle)group.readEntry("FillingStyle", (int)Qt::NoBrush) );
	d->brush.setColor( group.readEntry("FillingColor", QColor(Qt::black)) );
	d->pen.setStyle( (Qt::PenStyle)group.readEntry("BorderStyle", (int)Qt::SolidLine) );
	d->pen.setColor( group.readEntry("BorderColor", QColor(Qt::red)) );
	d->pen.setWidthF( group.readEntry("BorderWidth", Worksheet::convertToSceneUnits(1, Worksheet::Point)) );
	d->errorBarSize = group.readEntry("ErrorBarSize", Worksheet::convertToSceneUnits(8, Worksheet::Point));
	d->errorBarBrush.setStyle( (Qt::BrushStyle)group.readEntry("ErrorBarFillingStyle", (int)Qt::NoBrush) );
	d->errorBarBrush.setColor( group.readEntry("ErrorBarFillingColor", QColor(Qt::black)) );
	d->errorBarPen.setStyle( (Qt::PenStyle)group.readEntry("ErrorBarBorderStyle", (int)Qt::SolidLine) );
	d->errorBarPen.setColor( group.readEntry("ErrorBarBorderColor", QColor(Qt::black)) );
	d->errorBarPen.setWidthF( group.readEntry("ErrorBarBorderWidth", Worksheet::convertToSceneUnits(1, Worksheet::Point)) );
	d->plusDeltaXPos = group.readEntry("PlusDeltaXPos", QPointF(30, 0));
	d->minusDeltaXPos = group.readEntry("MinusDeltaXPos", QPointF(-30, 0));
	d->plusDeltaYPos = group.readEntry("PlusDeltaYPos", QPointF(0, -30));
	d->minusDeltaYPos = group.readEntry("MinusDeltaYPos", QPointF(0, 30));
	d->xSymmetricError = group.readEntry("XSymmetricError", false);
	d->ySymmetricError = group.readEntry("YSymmetricError", false);
	this->initActions();
}

void CustomPoint::initActions() {
    Q_D(CustomPoint);
	visibilityAction = new QAction(i18n("visible"), this);
	visibilityAction->setCheckable(true);
	connect(visibilityAction, SIGNAL(triggered()), this, SLOT(visibilityChanged()));
}

void CustomPoint::initErrorBar(const DatapickerCurve::Errors& errors) {
	m_errorBarItemList.clear();
	if (errors.x != DatapickerCurve::NoError) {
		setXSymmetricError(errors.x == DatapickerCurve::SymmetricError);

		ErrorBarItem* plusDeltaXItem = new ErrorBarItem(this, ErrorBarItem::PlusDeltaX);
		plusDeltaXItem->setPosition(plusDeltaXPos());
		connect(this, SIGNAL(plusDeltaXPosChanged(QPointF)), plusDeltaXItem, SLOT(setPosition(QPointF)));

		ErrorBarItem* minusDeltaXItem = new ErrorBarItem(this, ErrorBarItem::MinusDeltaX);
		minusDeltaXItem->setPosition(minusDeltaXPos());
		connect(this, SIGNAL(minusDeltaXPosChanged(QPointF)), minusDeltaXItem, SLOT(setPosition(QPointF)));

		m_errorBarItemList<<plusDeltaXItem<<minusDeltaXItem;
	}

	if (errors.y != DatapickerCurve::NoError) {
		setYSymmetricError(errors.y == DatapickerCurve::SymmetricError);

		ErrorBarItem* plusDeltaYItem = new ErrorBarItem(this, ErrorBarItem::PlusDeltaY);
		plusDeltaYItem->setPosition(plusDeltaYPos());
		connect(this, SIGNAL(plusDeltaYPosChanged(QPointF)), plusDeltaYItem, SLOT(setPosition(QPointF)));

		ErrorBarItem* minusDeltaYItem = new ErrorBarItem(this, ErrorBarItem::MinusDeltaY);
		minusDeltaYItem->setPosition(minusDeltaYPos());
		connect(this, SIGNAL(minusDeltaYPosChanged(QPointF)), minusDeltaYItem, SLOT(setPosition(QPointF)));

		m_errorBarItemList<<plusDeltaYItem<<minusDeltaYItem;
	}
}

/*!
    Returns an icon to be used in the project explorer.
*/
QIcon CustomPoint::icon() const {
	return  KIcon("draw-cross");
}

QMenu* CustomPoint::createContextMenu() {
	QMenu* menu = WorksheetElement::createContextMenu();
	QAction* firstAction = menu->actions().at(1); //skip the first action because of the "title-action"
	visibilityAction->setChecked(isVisible());
	menu->insertAction(firstAction, visibilityAction);

	return menu;
}

QGraphicsItem* CustomPoint::graphicsItem() const {
	return d_ptr;
}

void CustomPoint::setParentGraphicsItem(QGraphicsItem* item) {
    Q_D(CustomPoint);
	d->setParentItem(item);
	d->updatePosition();
}

void CustomPoint::retransform() {
    Q_D(CustomPoint);
	d->retransform();
}

void CustomPoint::handlePageResize(double horizontalRatio, double verticalRatio) {
	Q_UNUSED(horizontalRatio);
	Q_UNUSED(verticalRatio);

    Q_D(CustomPoint);
	d->scaleFactor = Worksheet::convertToSceneUnits(1, Worksheet::Point);
}

/* ============================ getter methods ================= */
//point
CLASS_SHARED_D_READER_IMPL(CustomPoint, CustomPoint::PositionWrapper, position, position)
BASIC_SHARED_D_READER_IMPL(CustomPoint, CustomPoint::PointsStyle, pointStyle, pointStyle)
BASIC_SHARED_D_READER_IMPL(CustomPoint, qreal, opacity, opacity)
BASIC_SHARED_D_READER_IMPL(CustomPoint, qreal, rotationAngle, rotationAngle)
BASIC_SHARED_D_READER_IMPL(CustomPoint, qreal, size, size)
CLASS_SHARED_D_READER_IMPL(CustomPoint, QBrush, brush, brush)
CLASS_SHARED_D_READER_IMPL(CustomPoint, QPen, pen, pen)

//error-bar
BASIC_SHARED_D_READER_IMPL(CustomPoint, qreal, errorBarSize, errorBarSize)
CLASS_SHARED_D_READER_IMPL(CustomPoint, QBrush, errorBarBrush, errorBarBrush)
CLASS_SHARED_D_READER_IMPL(CustomPoint, QPen, errorBarPen, errorBarPen)
CLASS_SHARED_D_READER_IMPL(CustomPoint, QPointF, plusDeltaXPos, plusDeltaXPos)
CLASS_SHARED_D_READER_IMPL(CustomPoint, QPointF, minusDeltaXPos, minusDeltaXPos)
CLASS_SHARED_D_READER_IMPL(CustomPoint, QPointF, plusDeltaYPos, plusDeltaYPos)
CLASS_SHARED_D_READER_IMPL(CustomPoint, QPointF, minusDeltaYPos, minusDeltaYPos)
BASIC_SHARED_D_READER_IMPL(CustomPoint, bool, xSymmetricError, xSymmetricError)
BASIC_SHARED_D_READER_IMPL(CustomPoint, bool, ySymmetricError, ySymmetricError)

/* ============================ setter methods and undo commands ================= */
STD_SETTER_CMD_IMPL_F_S(CustomPoint, SetPointStyle, CustomPoint::PointsStyle, pointStyle, retransform)
void CustomPoint::setPointStyle(CustomPoint::PointsStyle newStyle) {
    Q_D(CustomPoint);
	if (newStyle != d->pointStyle)
        exec(new CustomPointSetPointStyleCmd(d, newStyle, i18n("%1: set point's style")));
}

STD_SETTER_CMD_IMPL_F_S(CustomPoint, SetSize, qreal, size, retransform)
void CustomPoint::setSize(qreal value) {
    Q_D(CustomPoint);
	if (!qFuzzyCompare(1 + value, 1 + d->size))
        exec(new CustomPointSetSizeCmd(d, value, i18n("%1: set point's size")));
}

STD_SETTER_CMD_IMPL_F_S(CustomPoint, SetRotationAngle, qreal, rotationAngle, recalcShapeAndBoundingRect)
void CustomPoint::setRotationAngle(qreal angle) {
    Q_D(CustomPoint);
	if (!qFuzzyCompare(1 + angle, 1 + d->rotationAngle))
        exec(new CustomPointSetRotationAngleCmd(d, angle, i18n("%1: rotate point")));
}

STD_SETTER_CMD_IMPL_F_S(CustomPoint, SetBrush, QBrush, brush, retransform)
void CustomPoint::setBrush(const QBrush& newBrush) {
    Q_D(CustomPoint);
	if (newBrush != d->brush)
        exec(new CustomPointSetBrushCmd(d, newBrush, i18n("%1: set point's filling")));
}

STD_SETTER_CMD_IMPL_F_S(CustomPoint, SetPen, QPen, pen, retransform)
void CustomPoint::setPen(const QPen &newPen) {
    Q_D(CustomPoint);
	if (newPen != d->pen)
        exec(new CustomPointSetPenCmd(d, newPen, i18n("%1: set outline style")));
}

STD_SETTER_CMD_IMPL_F_S(CustomPoint, SetOpacity, qreal, opacity, retransform)
void CustomPoint::setOpacity(qreal newOpacity) {
    Q_D(CustomPoint);
	if (newOpacity != d->opacity)
        exec(new CustomPointSetOpacityCmd(d, newOpacity, i18n("%1: set point's opacity")));
}

STD_SETTER_CMD_IMPL_F_S(CustomPoint, SetPosition, CustomPoint::PositionWrapper, position, retransform)
void CustomPoint::setPosition(const PositionWrapper& pos) {
    Q_D(CustomPoint);
	if (pos.point!=d->position.point || pos.horizontalPosition!=d->position.horizontalPosition
	        || pos.verticalPosition!=d->position.verticalPosition)
        exec(new CustomPointSetPositionCmd(d, pos, i18n("%1: set position")));
}

STD_SETTER_CMD_IMPL_F_S(CustomPoint, SetErrorBarSize, qreal, errorBarSize, retransformErrorBar)
void CustomPoint::setErrorBarSize(qreal size) {
    Q_D(CustomPoint);
	if (size != d->errorBarSize)
        exec(new CustomPointSetErrorBarSizeCmd(d, size, i18n("%1: set error bar size")));
}

STD_SETTER_CMD_IMPL_F_S(CustomPoint, SetErrorBarBrush, QBrush, errorBarBrush, retransformErrorBar)
void CustomPoint::setErrorBarBrush(const QBrush &brush) {
    Q_D(CustomPoint);
	if (brush != d->errorBarBrush)
        exec(new CustomPointSetErrorBarBrushCmd(d, brush, i18n("%1: set error bar filling")));
}

STD_SETTER_CMD_IMPL_F_S(CustomPoint, SetErrorBarPen, QPen, errorBarPen, retransformErrorBar)
void CustomPoint::setErrorBarPen(const QPen &pen) {
    Q_D(CustomPoint);
	if (pen != d->errorBarPen)
        exec(new CustomPointSetErrorBarPenCmd(d, pen, i18n("%1: set error bar outline style")));
}

STD_SETTER_CMD_IMPL_F_S(CustomPoint, SetPlusDeltaXPos, QPointF, plusDeltaXPos, updateData)
void CustomPoint::setPlusDeltaXPos(const QPointF& pos) {
    Q_D(CustomPoint);
	if ( pos != d->plusDeltaXPos ) {
		beginMacro(i18n("%1: set +delta_X position", name()));
		if (d->xSymmetricError) {
            exec(new CustomPointSetPlusDeltaXPosCmd(d, pos, i18n("%1: set +delta X position")));
			setMinusDeltaXPos(QPointF(-qAbs(pos.x()), pos.y()));
		} else {
            exec(new CustomPointSetPlusDeltaXPosCmd(d, pos, i18n("%1: set +delta X position")));
		}
		endMacro();
	}
}

STD_SETTER_CMD_IMPL_F_S(CustomPoint, SetMinusDeltaXPos, QPointF, minusDeltaXPos, updateData)
void CustomPoint::setMinusDeltaXPos(const QPointF& pos) {
    Q_D(CustomPoint);
	if ( pos != d->minusDeltaXPos ) {
		beginMacro(i18n("%1: set -delta_X position", name()));
		if (d->xSymmetricError) {
            exec(new CustomPointSetMinusDeltaXPosCmd(d, pos, i18n("%1: set -delta X position")));
			setPlusDeltaXPos(QPointF(qAbs(pos.x()), pos.y()));
		} else {
            exec(new CustomPointSetMinusDeltaXPosCmd(d, pos, i18n("%1: set -delta X position")));
		}
		endMacro();
	}
}

STD_SETTER_CMD_IMPL_F_S(CustomPoint, SetPlusDeltaYPos, QPointF, plusDeltaYPos, updateData)
void CustomPoint::setPlusDeltaYPos(const QPointF& pos) {
    Q_D(CustomPoint);
	if ( pos != d->plusDeltaYPos ) {
		beginMacro(i18n("%1: set +delta Y position", name()));
		if (d->ySymmetricError) {
            exec(new CustomPointSetPlusDeltaYPosCmd(d, pos, i18n("%1: set +delta Y position")));
			setMinusDeltaYPos(QPointF(pos.x(), qAbs(pos.y())));
		} else {
            exec(new CustomPointSetPlusDeltaYPosCmd(d, pos, i18n("%1: set +delta Y position")));
		}
		endMacro();
	}
}

STD_SETTER_CMD_IMPL_F_S(CustomPoint, SetMinusDeltaYPos, QPointF, minusDeltaYPos, updateData)
void CustomPoint::setMinusDeltaYPos(const QPointF& pos) {
    Q_D(CustomPoint);
	if ( pos != d->minusDeltaYPos ) {
		beginMacro(i18n("%1: set -delta Y position", name()));
		if (d->ySymmetricError) {
            exec(new CustomPointSetMinusDeltaYPosCmd(d, pos, i18n("%1: set -delta Y position")));
			setPlusDeltaYPos(QPointF(pos.x(), -qAbs(pos.y())));
		} else {
            exec(new CustomPointSetMinusDeltaYPosCmd(d, pos, i18n("%1: set -delta Y position")));
		}
		endMacro();
	}
}

void CustomPoint::setXSymmetricError(const bool on) {
    Q_D(CustomPoint);
	d->xSymmetricError = on;
}

void CustomPoint::setYSymmetricError(const bool on) {
    Q_D(CustomPoint);
	d->ySymmetricError = on;
}

QPainterPath CustomPoint::pointPathFromStyle(CustomPoint::PointsStyle style) {
	QPainterPath path;
	QPolygonF polygon;
    if (style == CustomPoint::Circle) {
		path.addEllipse(QPoint(0,0), 0.5, 0.5);
    } else if (style == CustomPoint::Square) {
		path.addRect(QRectF(- 0.5, -0.5, 1.0, 1.0));
    } else if (style == CustomPoint::EquilateralTriangle) {
		polygon<<QPointF(-0.5, 0.5)<<QPointF(0, -0.5)<<QPointF(0.5, 0.5)<<QPointF(-0.5, 0.5);
		path.addPolygon(polygon);
    } else if (style == CustomPoint::RightTriangle) {
		polygon<<QPointF(-0.5, -0.5)<<QPointF(0.5, 0.5)<<QPointF(-0.5, 0.5)<<QPointF(-0.5, -0.5);
		path.addPolygon(polygon);
    } else if (style == CustomPoint::Bar) {
		path.addRect(QRectF(- 0.5, -0.2, 1.0, 0.4));
    } else if (style == CustomPoint::PeakedBar) {
		polygon<<QPointF(-0.5, 0)<<QPointF(-0.3, -0.2)<<QPointF(0.3, -0.2)<<QPointF(0.5, 0)
		       <<QPointF(0.3, 0.2)<<QPointF(-0.3, 0.2)<<QPointF(-0.5, 0);
		path.addPolygon(polygon);
    } else if (style == CustomPoint::SkewedBar) {
		polygon<<QPointF(-0.5, 0.2)<<QPointF(-0.2, -0.2)<<QPointF(0.5, -0.2)<<QPointF(0.2, 0.2)<<QPointF(-0.5, 0.2);
		path.addPolygon(polygon);
    } else if (style == CustomPoint::Diamond) {
		polygon<<QPointF(-0.5, 0)<<QPointF(0, -0.5)<<QPointF(0.5, 0)<<QPointF(0, 0.5)<<QPointF(-0.5, 0);
		path.addPolygon(polygon);
    } else if (style == CustomPoint::Lozenge) {
		polygon<<QPointF(-0.25, 0)<<QPointF(0, -0.5)<<QPointF(0.25, 0)<<QPointF(0, 0.5)<<QPointF(-0.25, 0);
		path.addPolygon(polygon);
    } else if (style == CustomPoint::Tie) {
		polygon<<QPointF(-0.5, -0.5)<<QPointF(0.5, -0.5)<<QPointF(-0.5, 0.5)<<QPointF(0.5, 0.5)<<QPointF(-0.5, -0.5);
		path.addPolygon(polygon);
    } else if (style == CustomPoint::TinyTie) {
		polygon<<QPointF(-0.2, -0.5)<<QPointF(0.2, -0.5)<<QPointF(-0.2, 0.5)<<QPointF(0.2, 0.5)<<QPointF(-0.2, -0.5);
		path.addPolygon(polygon);
    } else if (style == CustomPoint::Plus) {
		polygon<<QPointF(-0.2, -0.5)<<QPointF(0.2, -0.5)<<QPointF(0.2, -0.2)<<QPointF(0.5, -0.2)<<QPointF(0.5, 0.2)
		       <<QPointF(0.2, 0.2)<<QPointF(0.2, 0.5)<<QPointF(-0.2, 0.5)<<QPointF(-0.2, 0.2)<<QPointF(-0.5, 0.2)
		       <<QPointF(-0.5, -0.2)<<QPointF(-0.2, -0.2)<<QPointF(-0.2, -0.5);
		path.addPolygon(polygon);
    } else if (style == CustomPoint::Boomerang) {
		polygon<<QPointF(-0.5, 0.5)<<QPointF(0, -0.5)<<QPointF(0.5, 0.5)<<QPointF(0, 0)<<QPointF(-0.5, 0.5);
		path.addPolygon(polygon);
    } else if (style == CustomPoint::SmallBoomerang) {
		polygon<<QPointF(-0.3, 0.5)<<QPointF(0, -0.5)<<QPointF(0.3, 0.5)<<QPointF(0, 0)<<QPointF(-0.3, 0.5);
		path.addPolygon(polygon);
    } else if (style == CustomPoint::Star4) {
		polygon<<QPointF(-0.5, 0)<<QPointF(-0.1, -0.1)<<QPointF(0, -0.5)<<QPointF(0.1, -0.1)<<QPointF(0.5, 0)
		       <<QPointF(0.1, 0.1)<<QPointF(0, 0.5)<<QPointF(-0.1, 0.1)<<QPointF(-0.5, 0);
		path.addPolygon(polygon);
    } else if (style == CustomPoint::Star5) {
		polygon<<QPointF(-0.5, 0)<<QPointF(-0.1, -0.1)<<QPointF(0, -0.5)<<QPointF(0.1, -0.1)<<QPointF(0.5, 0)
		       <<QPointF(0.1, 0.1)<<QPointF(0.5, 0.5)<<QPointF(0, 0.2)<<QPointF(-0.5, 0.5)
		       <<QPointF(-0.1, 0.1)<<QPointF(-0.5, 0);
		path.addPolygon(polygon);
    } else if (style == CustomPoint::Line) {
		path = QPainterPath(QPointF(0, -0.5));
		path.lineTo(0, 0.5);
    } else if (style == CustomPoint::Cross) {
		path = QPainterPath(QPointF(0, -0.5));
		path.lineTo(0, 0.5);
		path.moveTo(-0.5, 0);
		path.lineTo(0.5, 0);
	}

	return path;
}

QString CustomPoint::pointNameFromStyle(CustomPoint::PointsStyle style) {
	QString name;
    if (style == CustomPoint::Circle)
		name = i18n("circle");
    else if (style == CustomPoint::Square)
		name = i18n("square");
    else if (style == CustomPoint::EquilateralTriangle)
		name = i18n("equilateral triangle");
    else if (style == CustomPoint::RightTriangle)
		name = i18n("right triangle");
    else if (style == CustomPoint::Bar)
		name = i18n("bar");
    else if (style == CustomPoint::PeakedBar)
		name = i18n("peaked bar");
    else if (style == CustomPoint::SkewedBar)
		name = i18n("skewed bar");
    else if (style == CustomPoint::Diamond)
		name = i18n("diamond");
    else if (style == CustomPoint::Lozenge)
		name = i18n("lozenge");
    else if (style == CustomPoint::Tie)
		name = i18n("tie");
    else if (style == CustomPoint::TinyTie)
		name = i18n("tiny tie");
    else if (style == CustomPoint::Plus)
		name = i18n("plus");
    else if (style == CustomPoint::Boomerang)
		name = i18n("boomerang");
    else if (style == CustomPoint::SmallBoomerang)
		name = i18n("small boomerang");
    else if (style == CustomPoint::Star4)
		name = i18n("star4");
    else if (style == CustomPoint::Star5)
		name = i18n("star5");
    else if (style == CustomPoint::Line)
		name = i18n("line");
    else if (style == CustomPoint::Cross)
		name = i18n("cross");

	return name;
}

/*!
    sets the position without undo/redo-stuff
*/
void CustomPoint::setPosition(const QPointF& point) {
    Q_D(CustomPoint);
	if (point != d->position.point) {
		d->position.point = point;
		d->retransform();
	}
}

STD_SWAP_METHOD_SETTER_CMD_IMPL_F(CustomPoint, SetVisible, bool, swapVisible, retransform);
void CustomPoint::setVisible(bool on) {
    Q_D(CustomPoint);
    exec(new CustomPointSetVisibleCmd(d, on, on ? i18n("%1: set visible") : i18n("%1: set invisible")));
}

bool CustomPoint::isVisible() const {
    Q_D(const CustomPoint);
	return d->isVisible();
}

void CustomPoint::setPrinting(bool on) {
    Q_D(CustomPoint);
	d->m_printing = on;
}

void CustomPoint::suppressHoverEvents(bool on) {
    Q_D(CustomPoint);
	d->m_suppressHoverEvents = on;
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
CustomPointPrivate::CustomPointPrivate(CustomPoint *owner)
	: suppressItemChangeEvent(false),
	  suppressRetransform(false),
	  m_printing(false),
	  m_hovered(false),
	  m_suppressHoverEvents(true),
	  q(owner) {

	setFlag(QGraphicsItem::ItemSendsGeometryChanges);
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

    if (position.horizontalPosition != CustomPoint::hPositionCustom
            || position.verticalPosition != CustomPoint::vPositionCustom)
		updatePosition();

	float x = position.point.x();
	float y = position.point.y();
	QPointF itemPos;
	itemPos.setX( x );
	itemPos.setY( y );

	suppressItemChangeEvent=true;
	setPos(itemPos);
	suppressItemChangeEvent=false;
    QPainterPath path = CustomPoint::pointPathFromStyle(pointStyle);
	boundingRectangle = path.boundingRect();
	recalcShapeAndBoundingRect();
	updateData();

	emit(q->changed());
}

/*!
    calculates the position of the item, when the position relative to the parent was specified (left, right, etc.)
*/
void CustomPointPrivate::updatePosition() {
	//determine the parent item
	QRectF parentRect;
	QGraphicsItem* parent = parentItem();
	if (parent) {
		parentRect = parent->boundingRect();
	} else {
		if (!scene())
			return;

		parentRect = scene()->sceneRect();
	}

    if (position.horizontalPosition != CustomPoint::hPositionCustom) {
        if (position.horizontalPosition == CustomPoint::hPositionLeft)
			position.point.setX( parentRect.x() );
        else if (position.horizontalPosition == CustomPoint::hPositionCenter)
			position.point.setX( parentRect.x() + parentRect.width()/2 );
        else if (position.horizontalPosition == CustomPoint::hPositionRight)
			position.point.setX( parentRect.x() + parentRect.width() );
	}

    if (position.verticalPosition != CustomPoint::vPositionCustom) {
        if (position.verticalPosition == CustomPoint::vPositionTop)
			position.point.setY( parentRect.y() );
        else if (position.verticalPosition == CustomPoint::vPositionCenter)
			position.point.setY( parentRect.y() + parentRect.height()/2 );
        else if (position.verticalPosition == CustomPoint::vPositionBottom)
			position.point.setY( parentRect.y() + parentRect.height() );
	}

	emit q->positionChanged(position);
}

/*!
  update color and size of all error-bar.
*/
void CustomPointPrivate::retransformErrorBar() {
	foreach (ErrorBarItem* item, q->m_errorBarItemList) {
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
void CustomPointPrivate::updateData() {
	DatapickerCurve* curve = dynamic_cast<DatapickerCurve*>(q->parentAspect());
	if (curve)
		curve->updateData(q);
}

bool CustomPointPrivate::swapVisible(bool on) {
	bool oldValue = isVisible();
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
	return itemShape;
}

/*!
  recalculates the outer bounds and the shape of the item.
*/
void CustomPointPrivate::recalcShapeAndBoundingRect() {
	prepareGeometryChange();

	QMatrix matrix;
	matrix.scale(size, size);
	matrix.rotate(-rotationAngle);
	matrix.scale(scaleFactor,scaleFactor);
	transformedBoundingRectangle = matrix.mapRect(boundingRectangle);
	itemShape = QPainterPath();
	itemShape.addRect(transformedBoundingRectangle);
}

void CustomPointPrivate::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget * widget) {
	Q_UNUSED(option)
	Q_UNUSED(widget)

    QPainterPath path = CustomPoint::pointPathFromStyle(pointStyle);
	QTransform trafo;
	trafo.scale(size, size);
	trafo.scale(scaleFactor, scaleFactor);
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

	if (!m_suppressHoverEvents) {
		if (m_hovered && !isSelected() && !m_printing) {
			painter->setPen(q->hoveredPen);
			painter->setOpacity(q->hoveredOpacity);
			painter->drawPath(itemShape);
		}
	}

	if (isSelected() && !m_printing) {
		painter->setPen(q->selectedPen);
		painter->setOpacity(q->selectedOpacity);
		painter->drawPath(itemShape);
	}
}

QVariant CustomPointPrivate::itemChange(GraphicsItemChange change, const QVariant &value) {
	if (suppressItemChangeEvent)
		return value;

	if (change == QGraphicsItem::ItemPositionChange) {
        CustomPoint::PositionWrapper tempPosition;
		tempPosition.point = value.toPointF();
        tempPosition.horizontalPosition = CustomPoint::hPositionCustom;
        tempPosition.verticalPosition = CustomPoint::vPositionCustom;

		//emit the signals in order to notify the UI.
		//we don't set the position related member variables during the mouse movements.
		//this is done on mouse release events only.
		emit q->positionChanged(tempPosition);
	}
	return QGraphicsItem::itemChange(change, value);
}

void CustomPointPrivate::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
	QPointF point = pos();
	if (abs(point.x()-position.point.x())>20 && qAbs(point.y()-position.point.y())>20 ) {
		//position was changed -> set the position related member variables
		suppressRetransform = true;
        CustomPoint::PositionWrapper tempPosition;
		tempPosition.point = point;
        tempPosition.horizontalPosition = CustomPoint::hPositionCustom;
        tempPosition.verticalPosition = CustomPoint::vPositionCustom;
		q->setPosition(tempPosition);
		suppressRetransform = false;
	}

	QGraphicsItem::mouseReleaseEvent(event);
}

void CustomPointPrivate::contextMenuEvent(QGraphicsSceneContextMenuEvent* event) {
	q->createContextMenu()->exec(event->screenPos());
}

void CustomPointPrivate::hoverEnterEvent(QGraphicsSceneHoverEvent*) {
	if (!isSelected()) {
		m_hovered = true;
		q->hovered();
		update();
	}
}

void CustomPointPrivate::hoverLeaveEvent(QGraphicsSceneHoverEvent*) {
	if (m_hovered) {
		m_hovered = false;
		q->unhovered();
		update();
	}
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################
//! Save as XML
void CustomPoint::save(QXmlStreamWriter* writer) const {
    Q_D(const CustomPoint);

    writer->writeStartElement( "customPoint" );
	writeBasicAttributes(writer);
	writeCommentElement(writer);

	//geometry
	writer->writeStartElement( "geometry" );
	writer->writeAttribute( "x", QString::number(d->position.point.x()) );
	writer->writeAttribute( "y", QString::number(d->position.point.y()) );
	writer->writeAttribute( "horizontalPosition", QString::number(d->position.horizontalPosition) );
	writer->writeAttribute( "verticalPosition", QString::number(d->position.verticalPosition) );
	writer->writeAttribute( "visible", QString::number(d->isVisible()) );
	writer->writeEndElement();

	writer->writeStartElement( "properties" );
	writer->writeAttribute( "pointStyle", QString::number(d->pointStyle) );
	writer->writeAttribute( "opacity", QString::number(d->opacity) );
	writer->writeAttribute( "rotation", QString::number(d->rotationAngle) );
	writer->writeAttribute( "size", QString::number(d->size) );
	WRITE_QBRUSH(d->brush);
	WRITE_QPEN(d->pen);
	writer->writeEndElement();

	writer->writeStartElement( "errorBar" );
	writer->writeAttribute( "errorBarSize", QString::number(d->errorBarSize) );
	WRITE_QBRUSH(d->errorBarBrush);
	WRITE_QPEN(d->errorBarPen);
	writer->writeAttribute( "plusDeltaXPos_x", QString::number(d->plusDeltaXPos.x()) );
	writer->writeAttribute( "plusDeltaXPos_y", QString::number(d->plusDeltaXPos.y()) );
	writer->writeAttribute( "minusDeltaXPos_x", QString::number(d->minusDeltaXPos.x()) );
	writer->writeAttribute( "minusDeltaXPos_y", QString::number(d->minusDeltaXPos.y()) );
	writer->writeAttribute( "plusDeltaYPos_x", QString::number(d->plusDeltaYPos.x()) );
	writer->writeAttribute( "plusDeltaYPos_y", QString::number(d->plusDeltaYPos.y()) );
	writer->writeAttribute( "minusDeltaYPos_x", QString::number(d->minusDeltaYPos.x()) );
	writer->writeAttribute( "minusDeltaYPos_y", QString::number(d->minusDeltaYPos.y()) );
	writer->writeAttribute( "xSymmetricError", QString::number(d->xSymmetricError) );
	writer->writeAttribute( "ySymmetricError", QString::number(d->ySymmetricError) );
	writer->writeEndElement();

    writer->writeEndElement(); // close "CustomPoint" section
}

//! Load from XML
bool CustomPoint::load(XmlStreamReader* reader) {
    Q_D(CustomPoint);

    if(!reader->isStartElement() || reader->name() != "customPoint") {
		reader->raiseError(i18n("no datapicker-Point element found"));
		return false;
	}

	if (!readBasicAttributes(reader))
		return false;

	QString attributeWarning = i18n("Attribute '%1' missing or empty, default value is used");
	QXmlStreamAttributes attribs;
	QString str;

	while (!reader->atEnd()) {
		reader->readNext();
        if (reader->isEndElement() && reader->name() == "customPoint")
			break;

		if (!reader->isStartElement())
			continue;

		if (reader->name() == "comment") {
			if (!readCommentElement(reader)) return false;
		} else if (reader->name() == "geometry") {
			attribs = reader->attributes();

			str = attribs.value("x").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'x'"));
			else
				d->position.point.setX(str.toDouble());

			str = attribs.value("y").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'y'"));
			else
				d->position.point.setY(str.toDouble());

			str = attribs.value("horizontalPosition").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'horizontalPosition'"));
			else
                d->position.horizontalPosition = (CustomPoint::HorizontalPosition)str.toInt();

			str = attribs.value("verticalPosition").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'verticalPosition'"));
			else
                d->position.verticalPosition = (CustomPoint::VerticalPosition)str.toInt();

			str = attribs.value("visible").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'visible'"));
			else
				d->setVisible(str.toInt());
		} else if (reader->name() == "errorBar") {
			attribs = reader->attributes();

			READ_QBRUSH(d->errorBarBrush);
			READ_QPEN(d->errorBarPen);

			str = attribs.value("plusDeltaXPos_x").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'plusDeltaXPos_x'"));
			else
				d->plusDeltaXPos.setX(str.toDouble());

			str = attribs.value("plusDeltaXPos_y").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'plusDeltaXPos_y'"));
			else
				d->plusDeltaXPos.setY(str.toDouble());

			str = attribs.value("minusDeltaXPos_x").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'minusDeltaXPos_x'"));
			else
				d->minusDeltaXPos.setX(str.toDouble());

			str = attribs.value("minusDeltaXPos_y").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'minusDeltaXPos_y'"));
			else
				d->minusDeltaXPos.setY(str.toDouble());

			str = attribs.value("plusDeltaYPos_x").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'plusDeltaYPos_x'"));
			else
				d->plusDeltaYPos.setX(str.toDouble());

			str = attribs.value("plusDeltaYPos_y").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'plusDeltaYPos_y'"));
			else
				d->plusDeltaYPos.setY(str.toDouble());

			str = attribs.value("minusDeltaYPos_x").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'minusDeltaYPos_x'"));
			else
				d->minusDeltaYPos.setX(str.toDouble());

			str = attribs.value("minusDeltaYPos_y").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'minusDeltaYPos_y'"));
			else
				d->minusDeltaYPos.setY(str.toDouble());

			str = attribs.value("xSymmetricError").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'xSymmetricError'"));
			else
				d->xSymmetricError = str.toInt();

			str = attribs.value("ySymmetricError").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'ySymmetricError'"));
			else
				d->ySymmetricError = str.toInt();

			str = attribs.value("errorBarSize").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'errorBarSize'"));
			else
				d->errorBarSize = str.toDouble();

		} else if (reader->name() == "properties") {
			attribs = reader->attributes();

			str = attribs.value("pointStyle").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'pointStyle'"));
			else
                d->pointStyle = (CustomPoint::PointsStyle)str.toInt();

			str = attribs.value("opacity").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'opacity'"));
			else
				d->opacity = str.toDouble();

			str = attribs.value("rotation").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'rotation'"));
			else
				d->rotationAngle = str.toDouble();

			str = attribs.value("size").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'size'"));
			else
				d->size = str.toDouble();

			READ_QBRUSH(d->brush);
			READ_QPEN(d->pen);
		} else { // unknown element
			reader->raiseWarning(i18n("unknown element '%1'", reader->name().toString()));
			if (!reader->skipToEndElement()) return false;
		}
	}

	retransform();
	return true;
}
