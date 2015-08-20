
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

#include "CustomItem.h"
#include "Worksheet.h"
#include "CustomItemPrivate.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/core/PlotCurve.h"

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
 * \brief A customizable error-bar for custom-item.
 */

ErrorBarItem::ErrorBarItem(CustomItem *parent, const ErrorBarType& type) : QGraphicsRectItem(parent->graphicsItem(), 0),
     barLineItem(new QGraphicsLineItem(parent->graphicsItem(), 0)),
     m_parentItem(parent),
     m_type(type) {

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
 * \class Custom-Item
 * \brief A customizable symbol supports error-bars.
 *
 * The custom-item is aligned relative to the specified position.
 * The position can be either specified by mouse events or by providing the
 * x- and y- coordinates in parent's coordinate system, or by specifying one
 * of the predefined position flags (\ca HorizontalPosition, \ca VerticalPosition).
 */

CustomItem::CustomItem(const QString& name):WorksheetElement(name),
    d_ptr(new CustomItemPrivate(this)) {

    init();
}

CustomItem::CustomItem(const QString& name, CustomItemPrivate *dd):WorksheetElement(name), d_ptr(dd) {

    init();
}

CustomItem::~CustomItem() {
    //no need to delete the d-pointer here - it inherits from QGraphicsItem
    //and is deleted during the cleanup in QGraphicsScene
}

void CustomItem::init() {
    Q_D(CustomItem);

    KConfig config;
    KConfigGroup group;
    group = config.group("CustomItem");
    d->position.horizontalPosition = (HorizontalPosition) group.readEntry("PositionX", (int)CustomItem::hPositionCustom);
    d->position.verticalPosition = (VerticalPosition) group.readEntry("PositionY", (int) CustomItem::vPositionCustom);
    d->position.point.setX( group.readEntry("PositionXValue", Worksheet::convertToSceneUnits(1, Worksheet::Centimeter)) );
    d->position.point.setY( group.readEntry("PositionYValue", Worksheet::convertToSceneUnits(1, Worksheet::Centimeter)) );
    d->scaleFactor = Worksheet::convertToSceneUnits(1, Worksheet::Point);
    d->itemsStyle = (CustomItem::ItemsStyle)group.readEntry("ItemStyle", (int)CustomItem::Cross);
    d->itemsSize = group.readEntry("ItemSize", Worksheet::convertToSceneUnits(2, Worksheet::Point));
    d->itemsRotationAngle = group.readEntry("ItemRotation", 0.0);
    d->itemsOpacity = group.readEntry("ItemOpacity", 1.0);
    d->itemsBrush.setStyle( (Qt::BrushStyle)group.readEntry("ItemFillingStyle", (int)Qt::NoBrush) );
    d->itemsBrush.setColor( group.readEntry("ItemFillingColor", QColor(Qt::black)) );
    d->itemsPen.setStyle( (Qt::PenStyle)group.readEntry("ItemBorderStyle", (int)Qt::SolidLine) );
    d->itemsPen.setColor( group.readEntry("ItemBorderColor", QColor(Qt::red)) );
    d->itemsPen.setWidthF( group.readEntry("ItemBorderWidth", Worksheet::convertToSceneUnits(1, Worksheet::Point)) );
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

void CustomItem::initActions() {
    Q_D(CustomItem);
    visibilityAction = new QAction(i18n("visible"), this);
    visibilityAction->setCheckable(true);
    connect(visibilityAction, SIGNAL(triggered()), this, SLOT(visibilityChanged()));
}

void CustomItem::initErrorBar(const Image::Errors& errors) {
    m_errorBarItemList.clear();
    if (errors.x != Image::NoError) {
        setXSymmetricError(errors.x == Image::SymmetricError);

        ErrorBarItem* plusDeltaXItem = new ErrorBarItem(this, ErrorBarItem::PlusDeltaX);
        plusDeltaXItem->setPosition(plusDeltaXPos());
        connect(this, SIGNAL(plusDeltaXPosChanged(QPointF)), plusDeltaXItem, SLOT(setPosition(QPointF)));

        ErrorBarItem* minusDeltaXItem = new ErrorBarItem(this, ErrorBarItem::MinusDeltaX);
        minusDeltaXItem->setPosition(minusDeltaXPos());
        connect(this, SIGNAL(minusDeltaXPosChanged(QPointF)), minusDeltaXItem, SLOT(setPosition(QPointF)));

        m_errorBarItemList<<plusDeltaXItem<<minusDeltaXItem;
    }

    if (errors.y != Image::NoError) {
        setYSymmetricError(errors.y == Image::SymmetricError);

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
QIcon CustomItem::icon() const{
    return  KIcon("draw-cross");
}

QMenu* CustomItem::createContextMenu(){
    QMenu *menu = WorksheetElement::createContextMenu();

#ifdef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
    QAction* firstAction = menu->actions().first();
#else
    QAction* firstAction = menu->actions().at(1); //skip the first action because of the "title-action"
#endif

    visibilityAction->setChecked(isVisible());
    menu->insertAction(firstAction, visibilityAction);

    return menu;
}

QGraphicsItem* CustomItem::graphicsItem() const {
    return d_ptr;
}

void CustomItem::setParentGraphicsItem(QGraphicsItem* item) {
    Q_D(CustomItem);
    d->setParentItem(item);
    d->updatePosition();
}

void CustomItem::retransform(){
    Q_D(CustomItem);
    d->retransform();
}

void CustomItem::handlePageResize(double horizontalRatio, double verticalRatio) {
    Q_UNUSED(horizontalRatio);
    Q_UNUSED(verticalRatio);

    Q_D(CustomItem);
    d->scaleFactor = Worksheet::convertToSceneUnits(1, Worksheet::Point);
}

/* ============================ getter methods ================= */
//item
CLASS_SHARED_D_READER_IMPL(CustomItem, CustomItem::PositionWrapper, position, position)
BASIC_SHARED_D_READER_IMPL(CustomItem, CustomItem::ItemsStyle, itemsStyle, itemsStyle)
BASIC_SHARED_D_READER_IMPL(CustomItem, qreal, itemsOpacity, itemsOpacity)
BASIC_SHARED_D_READER_IMPL(CustomItem, qreal, itemsRotationAngle, itemsRotationAngle)
BASIC_SHARED_D_READER_IMPL(CustomItem, qreal, itemsSize, itemsSize)
CLASS_SHARED_D_READER_IMPL(CustomItem, QBrush, itemsBrush, itemsBrush)
CLASS_SHARED_D_READER_IMPL(CustomItem, QPen, itemsPen, itemsPen)

//error-bar
BASIC_SHARED_D_READER_IMPL(CustomItem, qreal, errorBarSize, errorBarSize)
CLASS_SHARED_D_READER_IMPL(CustomItem, QBrush, errorBarBrush, errorBarBrush)
CLASS_SHARED_D_READER_IMPL(CustomItem, QPen, errorBarPen, errorBarPen)
CLASS_SHARED_D_READER_IMPL(CustomItem, QPointF, plusDeltaXPos, plusDeltaXPos)
CLASS_SHARED_D_READER_IMPL(CustomItem, QPointF, minusDeltaXPos, minusDeltaXPos)
CLASS_SHARED_D_READER_IMPL(CustomItem, QPointF, plusDeltaYPos, plusDeltaYPos)
CLASS_SHARED_D_READER_IMPL(CustomItem, QPointF, minusDeltaYPos, minusDeltaYPos)
BASIC_SHARED_D_READER_IMPL(CustomItem, bool, xSymmetricError, xSymmetricError)
BASIC_SHARED_D_READER_IMPL(CustomItem, bool, ySymmetricError, ySymmetricError)

/* ============================ setter methods and undo commands ================= */
STD_SETTER_CMD_IMPL_F_S(CustomItem, SetItemsStyle, CustomItem::ItemsStyle, itemsStyle, retransform)
void CustomItem::setItemsStyle(CustomItem::ItemsStyle style) {
    Q_D(CustomItem);
    if (style != d->itemsStyle)
        exec(new CustomItemSetItemsStyleCmd(d, style, i18n("%1: set item style")));
}

STD_SETTER_CMD_IMPL_F_S(CustomItem, SetItemsSize, qreal, itemsSize, retransform)
void CustomItem::setItemsSize(qreal size) {
    Q_D(CustomItem);
    if (!qFuzzyCompare(1 + size, 1 + d->itemsSize))
        exec(new CustomItemSetItemsSizeCmd(d, size, i18n("%1: set item size")));
}

STD_SETTER_CMD_IMPL_F_S(CustomItem, SetItemsRotationAngle, qreal, itemsRotationAngle, recalcShapeAndBoundingRect)
void CustomItem::setItemsRotationAngle(qreal angle) {
    Q_D(CustomItem);
    if (!qFuzzyCompare(1 + angle, 1 + d->itemsRotationAngle))
        exec(new CustomItemSetItemsRotationAngleCmd(d, angle, i18n("%1: rotate items")));
}

STD_SETTER_CMD_IMPL_F_S(CustomItem, SetItemsBrush, QBrush, itemsBrush, retransform)
void CustomItem::setItemsBrush(const QBrush &brush) {
    Q_D(CustomItem);
    if (brush != d->itemsBrush)
        exec(new CustomItemSetItemsBrushCmd(d, brush, i18n("%1: set item filling")));
}

STD_SETTER_CMD_IMPL_F_S(CustomItem, SetItemsPen, QPen, itemsPen, retransform)
void CustomItem::setItemsPen(const QPen &pen) {
    Q_D(CustomItem);
    if (pen != d->itemsPen)
        exec(new CustomItemSetItemsPenCmd(d, pen, i18n("%1: set outline style")));
}

STD_SETTER_CMD_IMPL_F_S(CustomItem, SetItemsOpacity, qreal, itemsOpacity, retransform)
void CustomItem::setItemsOpacity(qreal opacity) {
    Q_D(CustomItem);
    if (opacity != d->itemsOpacity)
        exec(new CustomItemSetItemsOpacityCmd(d, opacity, i18n("%1: set items opacity")));
}

STD_SETTER_CMD_IMPL_F_S(CustomItem, SetPosition, CustomItem::PositionWrapper, position, retransform)
void CustomItem::setPosition(const PositionWrapper& pos) {
    Q_D(CustomItem);
    if (pos.point!=d->position.point || pos.horizontalPosition!=d->position.horizontalPosition
            || pos.verticalPosition!=d->position.verticalPosition)
        exec(new CustomItemSetPositionCmd(d, pos, i18n("%1: set position")));
}

STD_SETTER_CMD_IMPL_F_S(CustomItem, SetErrorBarSize, qreal, errorBarSize, retransformErrorBar)
void CustomItem::setErrorBarSize(qreal size) {
    Q_D(CustomItem);
    if (size != d->errorBarSize)
        exec(new CustomItemSetErrorBarSizeCmd(d, size, i18n("%1: set error bar size")));
}

STD_SETTER_CMD_IMPL_F_S(CustomItem, SetErrorBarBrush, QBrush, errorBarBrush, retransformErrorBar)
void CustomItem::setErrorBarBrush(const QBrush &brush) {
    Q_D(CustomItem);
    if (brush != d->errorBarBrush)
        exec(new CustomItemSetErrorBarBrushCmd(d, brush, i18n("%1: set error bar filling")));
}

STD_SETTER_CMD_IMPL_F_S(CustomItem, SetErrorBarPen, QPen, errorBarPen, retransformErrorBar)
void CustomItem::setErrorBarPen(const QPen &pen) {
    Q_D(CustomItem);
    if (pen != d->errorBarPen)
        exec(new CustomItemSetErrorBarPenCmd(d, pen, i18n("%1: set error bar outline style")));
}

STD_SETTER_CMD_IMPL_F_S(CustomItem, SetPlusDeltaXPos, QPointF, plusDeltaXPos, updateData)
void CustomItem::setPlusDeltaXPos(const QPointF& pos) {
    Q_D(CustomItem);
    if ( pos != d->plusDeltaXPos ) {
        if (d->xSymmetricError) {
            beginMacro(i18n("%1: set +delta X position", name()));
            exec(new CustomItemSetPlusDeltaXPosCmd(d, pos, i18n("%1: set +delta X position")));
            setMinusDeltaXPos(QPointF(-qAbs(pos.x()), pos.y()));
            endMacro();
        } else {
            exec(new CustomItemSetPlusDeltaXPosCmd(d, pos, i18n("%1: set +delta X position")));
        }
    }
}

STD_SETTER_CMD_IMPL_F_S(CustomItem, SetMinusDeltaXPos, QPointF, minusDeltaXPos, updateData)
void CustomItem::setMinusDeltaXPos(const QPointF& pos) {
    Q_D(CustomItem);
    if ( pos != d->minusDeltaXPos ) {
        if (d->xSymmetricError) {
            beginMacro(i18n("%1: set -delta X position", name()));
            exec(new CustomItemSetMinusDeltaXPosCmd(d, pos, i18n("%1: set -delta X position")));
            setPlusDeltaXPos(QPointF(qAbs(pos.x()), pos.y()));
            endMacro();
        } else {
            exec(new CustomItemSetMinusDeltaXPosCmd(d, pos, i18n("%1: set -delta X position")));
        }
    }
}

STD_SETTER_CMD_IMPL_F_S(CustomItem, SetPlusDeltaYPos, QPointF, plusDeltaYPos, updateData)
void CustomItem::setPlusDeltaYPos(const QPointF& pos) {
    Q_D(CustomItem);
    if ( pos != d->plusDeltaYPos ) {
        if (d->ySymmetricError) {
            beginMacro(i18n("%1: set +delta Y position", name()));
            exec(new CustomItemSetPlusDeltaYPosCmd(d, pos, i18n("%1: set +delta Y position")));
            setMinusDeltaYPos(QPointF(pos.x(), qAbs(pos.y())));
            endMacro();
        } else {
            exec(new CustomItemSetPlusDeltaYPosCmd(d, pos, i18n("%1: set +delta Y position")));
        }
    }
}

STD_SETTER_CMD_IMPL_F_S(CustomItem, SetMinusDeltaYPos, QPointF, minusDeltaYPos, updateData)
void CustomItem::setMinusDeltaYPos(const QPointF& pos) {
    Q_D(CustomItem);
    if ( pos != d->minusDeltaYPos ) {
        if (d->ySymmetricError) {
            beginMacro(i18n("%1: set -delta Y position", name()));
            exec(new CustomItemSetMinusDeltaYPosCmd(d, pos, i18n("%1: set -delta Y position")));
            setPlusDeltaYPos(QPointF(pos.x(), -qAbs(pos.y())));
            endMacro();
        } else {
            exec(new CustomItemSetMinusDeltaYPosCmd(d, pos, i18n("%1: set -delta Y position")));
        }
    }
}

void CustomItem::setXSymmetricError(const bool value) {
    Q_D(CustomItem);
    d->xSymmetricError = value;
}

void CustomItem::setYSymmetricError(const bool value) {
    Q_D(CustomItem);
    d->ySymmetricError = value;
}

QPainterPath CustomItem::itemsPathFromStyle(CustomItem::ItemsStyle style) {
    QPainterPath path;
    QPolygonF polygon;
    if (style == CustomItem::Circle) {
        path.addEllipse(QPoint(0,0), 0.5, 0.5);
    } else if (style == CustomItem::Square) {
        path.addRect(QRectF(- 0.5, -0.5, 1.0, 1.0));
    } else if (style == CustomItem::EquilateralTriangle) {
        polygon<<QPointF(-0.5, 0.5)<<QPointF(0, -0.5)<<QPointF(0.5, 0.5)<<QPointF(-0.5, 0.5);
        path.addPolygon(polygon);
    } else if (style == CustomItem::RightTriangle) {
        polygon<<QPointF(-0.5, -0.5)<<QPointF(0.5, 0.5)<<QPointF(-0.5, 0.5)<<QPointF(-0.5, -0.5);
        path.addPolygon(polygon);
    } else if (style == CustomItem::Bar) {
        path.addRect(QRectF(- 0.5, -0.2, 1.0, 0.4));
    } else if (style == CustomItem::PeakedBar) {
        polygon<<QPointF(-0.5, 0)<<QPointF(-0.3, -0.2)<<QPointF(0.3, -0.2)<<QPointF(0.5, 0)
                <<QPointF(0.3, 0.2)<<QPointF(-0.3, 0.2)<<QPointF(-0.5, 0);
        path.addPolygon(polygon);
    } else if (style == CustomItem::SkewedBar) {
        polygon<<QPointF(-0.5, 0.2)<<QPointF(-0.2, -0.2)<<QPointF(0.5, -0.2)<<QPointF(0.2, 0.2)<<QPointF(-0.5, 0.2);
        path.addPolygon(polygon);
    } else if (style == CustomItem::Diamond) {
        polygon<<QPointF(-0.5, 0)<<QPointF(0, -0.5)<<QPointF(0.5, 0)<<QPointF(0, 0.5)<<QPointF(-0.5, 0);
        path.addPolygon(polygon);
    } else if (style == CustomItem::Lozenge) {
        polygon<<QPointF(-0.25, 0)<<QPointF(0, -0.5)<<QPointF(0.25, 0)<<QPointF(0, 0.5)<<QPointF(-0.25, 0);
        path.addPolygon(polygon);
    } else if (style == CustomItem::Tie) {
        polygon<<QPointF(-0.5, -0.5)<<QPointF(0.5, -0.5)<<QPointF(-0.5, 0.5)<<QPointF(0.5, 0.5)<<QPointF(-0.5, -0.5);
        path.addPolygon(polygon);
    } else if (style == CustomItem::TinyTie) {
        polygon<<QPointF(-0.2, -0.5)<<QPointF(0.2, -0.5)<<QPointF(-0.2, 0.5)<<QPointF(0.2, 0.5)<<QPointF(-0.2, -0.5);
        path.addPolygon(polygon);
    } else if (style == CustomItem::Plus) {
        polygon<<QPointF(-0.2, -0.5)<<QPointF(0.2, -0.5)<<QPointF(0.2, -0.2)<<QPointF(0.5, -0.2)<<QPointF(0.5, 0.2)
                <<QPointF(0.2, 0.2)<<QPointF(0.2, 0.5)<<QPointF(-0.2, 0.5)<<QPointF(-0.2, 0.2)<<QPointF(-0.5, 0.2)
                <<QPointF(-0.5, -0.2)<<QPointF(-0.2, -0.2)<<QPointF(-0.2, -0.5);
        path.addPolygon(polygon);
    } else if (style == CustomItem::Boomerang) {
        polygon<<QPointF(-0.5, 0.5)<<QPointF(0, -0.5)<<QPointF(0.5, 0.5)<<QPointF(0, 0)<<QPointF(-0.5, 0.5);
        path.addPolygon(polygon);
    } else if (style == CustomItem::SmallBoomerang) {
        polygon<<QPointF(-0.3, 0.5)<<QPointF(0, -0.5)<<QPointF(0.3, 0.5)<<QPointF(0, 0)<<QPointF(-0.3, 0.5);
        path.addPolygon(polygon);
    } else if (style == CustomItem::Star4) {
        polygon<<QPointF(-0.5, 0)<<QPointF(-0.1, -0.1)<<QPointF(0, -0.5)<<QPointF(0.1, -0.1)<<QPointF(0.5, 0)
                <<QPointF(0.1, 0.1)<<QPointF(0, 0.5)<<QPointF(-0.1, 0.1)<<QPointF(-0.5, 0);
        path.addPolygon(polygon);
    } else if (style == CustomItem::Star5) {
        polygon<<QPointF(-0.5, 0)<<QPointF(-0.1, -0.1)<<QPointF(0, -0.5)<<QPointF(0.1, -0.1)<<QPointF(0.5, 0)
                <<QPointF(0.1, 0.1)<<QPointF(0.5, 0.5)<<QPointF(0, 0.2)<<QPointF(-0.5, 0.5)
                <<QPointF(-0.1, 0.1)<<QPointF(-0.5, 0);
        path.addPolygon(polygon);
    } else if (style == CustomItem::Line) {
        path = QPainterPath(QPointF(0, -0.5));
        path.lineTo(0, 0.5);
    } else if (style == CustomItem::Cross) {
        path = QPainterPath(QPointF(0, -0.5));
        path.lineTo(0, 0.5);
        path.moveTo(-0.5, 0);
        path.lineTo(0.5, 0);
    }

    return path;
}

QString CustomItem::itemsNameFromStyle(CustomItem::ItemsStyle style) {
    QString name;
    if (style == CustomItem::Circle)
        name = i18n("circle");
    else if (style == CustomItem::Square)
        name = i18n("square");
    else if (style == CustomItem::EquilateralTriangle)
        name = i18n("equilateral triangle");
    else if (style == CustomItem::RightTriangle)
        name = i18n("right triangle");
    else if (style == CustomItem::Bar)
        name = i18n("bar");
    else if (style == CustomItem::PeakedBar)
        name = i18n("peaked bar");
    else if (style == CustomItem::SkewedBar)
        name = i18n("skewed bar");
    else if (style == CustomItem::Diamond)
        name = i18n("diamond");
    else if (style == CustomItem::Lozenge)
        name = i18n("lozenge");
    else if (style == CustomItem::Tie)
        name = i18n("tie");
    else if (style == CustomItem::TinyTie)
        name = i18n("tiny tie");
    else if (style == CustomItem::Plus)
        name = i18n("plus");
    else if (style == CustomItem::Boomerang)
        name = i18n("boomerang");
    else if (style == CustomItem::SmallBoomerang)
        name = i18n("small boomerang");
    else if (style == CustomItem::Star4)
        name = i18n("star4");
    else if (style == CustomItem::Star5)
        name = i18n("star5");
    else if (style == CustomItem::Line)
        name = i18n("line");
    else if (style == CustomItem::Cross)
        name = i18n("cross");

    return name;
}

/*!
    sets the position without undo/redo-stuff
*/
void CustomItem::setPosition(const QPointF& point) {
    Q_D(CustomItem);
    if (point != d->position.point){
        d->position.point = point;
        retransform();
    }
}

STD_SWAP_METHOD_SETTER_CMD_IMPL_F(CustomItem, SetVisible, bool, swapVisible, retransform);
void CustomItem::setVisible(bool on) {
    Q_D(CustomItem);
    exec(new CustomItemSetVisibleCmd(d, on, on ? i18n("%1: set visible") : i18n("%1: set invisible")));
}

bool CustomItem::isVisible() const {
    Q_D(const CustomItem);
    return d->isVisible();
}

void CustomItem::setPrinting(bool on) {
    Q_D(CustomItem);
    d->m_printing = on;
}

void CustomItem::suppressHoverEvents(bool on) {
    Q_D(CustomItem);
    d->m_suppressHoverEvents = on;
}
//##############################################################################
//######  SLOTs for changes triggered via QActions in the context menu  ########
//##############################################################################
void CustomItem::visibilityChanged() {
    Q_D(const CustomItem);
    this->setVisible(!d->isVisible());
}

//##############################################################################
//####################### Private implementation ###############################
//##############################################################################
CustomItemPrivate::CustomItemPrivate(CustomItem *owner)
        : suppressItemChangeEvent(false),
          suppressRetransform(false),
          m_printing(false),
          m_hovered(false),
          m_suppressHoverEvents(true),
          q(owner){

    setFlag(QGraphicsItem::ItemSendsGeometryChanges);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setAcceptHoverEvents(true);
}

QString CustomItemPrivate::name() const{
    return q->name();
}

/*!
    calculates the position and the bounding box of the item. Called on geometry or properties changes.
 */
void CustomItemPrivate::retransform(){
    if (suppressRetransform)
        return;

    if (position.horizontalPosition != CustomItem::hPositionCustom
        || position.verticalPosition != CustomItem::vPositionCustom)
        updatePosition();

    float x = position.point.x();
    float y = position.point.y();
    QPointF itemPos;
    itemPos.setX( x );
    itemPos.setY( y );

    suppressItemChangeEvent=true;
    setPos(itemPos);
    suppressItemChangeEvent=false;
    QPainterPath path = CustomItem::itemsPathFromStyle(itemsStyle);
    boundingRectangle = path.boundingRect();
    recalcShapeAndBoundingRect();

    emit(q->changed());
}

/*!
    calculates the position of the item, when the position relative to the parent was specified (left, right, etc.)
*/
void CustomItemPrivate::updatePosition(){
    //determine the parent item
    QRectF parentRect;
    QGraphicsItem* parent = parentItem();
    if (parent){
        parentRect = parent->boundingRect();
    } else {
        if (!scene())
            return;

        parentRect = scene()->sceneRect();
    }

    if (position.horizontalPosition != CustomItem::hPositionCustom){
        if (position.horizontalPosition == CustomItem::hPositionLeft)
            position.point.setX( parentRect.x() );
        else if (position.horizontalPosition == CustomItem::hPositionCenter)
            position.point.setX( parentRect.x() + parentRect.width()/2 );
        else if (position.horizontalPosition == CustomItem::hPositionRight)
            position.point.setX( parentRect.x() + parentRect.width() );
    }

    if (position.verticalPosition != CustomItem::vPositionCustom){
        if (position.verticalPosition == CustomItem::vPositionTop)
            position.point.setY( parentRect.y() );
        else if (position.verticalPosition == CustomItem::vPositionCenter)
            position.point.setY( parentRect.y() + parentRect.height()/2 );
        else if (position.verticalPosition == CustomItem::vPositionBottom)
            position.point.setY( parentRect.y() + parentRect.height() );
    }

    emit q->positionChanged(position);
    updateData();
}

/*!
  update color and size of all error-bar.
*/
void CustomItemPrivate::retransformErrorBar() {
    foreach (ErrorBarItem* item, q->m_errorBarItemList) {
        if (item) {
            item->setBrush(errorBarBrush);
            item->setPen(errorBarPen);
            item->setRectSize(errorBarSize);
        }
    }
}

/*!
  update datasheet on any change in position of custom-item or it's error-bar.
*/
void CustomItemPrivate::updateData() {
    PlotCurve* curve = dynamic_cast<PlotCurve*>(q->parentAspect());
    curve->updateData(q);
}

bool CustomItemPrivate::swapVisible(bool on){
    bool oldValue = isVisible();
    setVisible(on);
    emit q->changed();
    emit q->visibleChanged(on);
    return oldValue;
}

/*!
    Returns the outer bounds of the item as a rectangle.
 */
QRectF CustomItemPrivate::boundingRect() const{
    return transformedBoundingRectangle;
}

/*!
    Returns the shape of this item as a QPainterPath in local coordinates.
*/
QPainterPath CustomItemPrivate::shape() const{
    return itemShape;
}

/*!
  recalculates the outer bounds and the shape of the item.
*/
void CustomItemPrivate::recalcShapeAndBoundingRect(){
    prepareGeometryChange();

    QMatrix matrix;
    matrix.scale(itemsSize, itemsSize);
    matrix.rotate(-itemsRotationAngle);
    matrix.scale(scaleFactor,scaleFactor);
    transformedBoundingRectangle = matrix.mapRect(boundingRectangle);
    itemShape = QPainterPath();
    itemShape.addRect(transformedBoundingRectangle);
}

void CustomItemPrivate::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget * widget){
    Q_UNUSED(option)
    Q_UNUSED(widget)

    QPainterPath path = CustomItem::itemsPathFromStyle(itemsStyle);
    QTransform trafo;
    trafo.scale(itemsSize, itemsSize);
    trafo.scale(scaleFactor, scaleFactor);
    path = trafo.map(path);
    trafo.reset();
    if (itemsRotationAngle != 0) {
        trafo.rotate(-itemsRotationAngle);
        path = trafo.map(path);
    }
    painter->save();
    painter->setPen(itemsPen);
    painter->setBrush(itemsBrush);
    painter->setOpacity(itemsOpacity);
    painter->drawPath(path);
    painter->restore();

    if (!m_suppressHoverEvents) {
        if (m_hovered && !isSelected() && !m_printing){
            painter->setPen(q->hoveredPen);
            painter->setOpacity(q->hoveredOpacity);
            painter->drawPath(itemShape);
        }
    }

    if (isSelected() && !m_printing){
        painter->setPen(q->selectedPen);
        painter->setOpacity(q->selectedOpacity);
        painter->drawPath(itemShape);
    }
}

QVariant CustomItemPrivate::itemChange(GraphicsItemChange change, const QVariant &value){
    if (suppressItemChangeEvent)
        return value;

    if (change == QGraphicsItem::ItemPositionChange) {
        CustomItem::PositionWrapper tempPosition;
        tempPosition.point = value.toPointF();
        tempPosition.horizontalPosition = CustomItem::hPositionCustom;
        tempPosition.verticalPosition = CustomItem::vPositionCustom;

        //emit the signals in order to notify the UI.
        //we don't set the position related member variables during the mouse movements.
        //this is done on mouse release events only.
        emit q->positionChanged(tempPosition);
    }
    return QGraphicsItem::itemChange(change, value);
}

void CustomItemPrivate::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
    QPointF point = pos();
    if (abs(point.x()-position.point.x())>20 && qAbs(point.y()-position.point.y())>20 ) {
        //position was changed -> set the position related member variables
        suppressRetransform = true;
        CustomItem::PositionWrapper tempPosition;
        tempPosition.point = point;
        tempPosition.horizontalPosition = CustomItem::hPositionCustom;
        tempPosition.verticalPosition = CustomItem::vPositionCustom;
        q->setPosition(tempPosition);
        suppressRetransform = false;
    }

    QGraphicsItem::mouseReleaseEvent(event);
}

void CustomItemPrivate::contextMenuEvent(QGraphicsSceneContextMenuEvent* event){
    q->createContextMenu()->exec(event->screenPos());
}

void CustomItemPrivate::hoverEnterEvent(QGraphicsSceneHoverEvent*) {
    if (!isSelected()) {
        m_hovered = true;
        q->hovered();
        update();
    }
}

void CustomItemPrivate::hoverLeaveEvent(QGraphicsSceneHoverEvent*) {
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
void CustomItem::save(QXmlStreamWriter* writer) const{
    Q_D(const CustomItem);

    writer->writeStartElement( "customItem" );
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
    writer->writeAttribute( "itemsStyle", QString::number(d->itemsStyle) );
    writer->writeAttribute( "opacity", QString::number(d->itemsOpacity) );
    writer->writeAttribute( "rotation", QString::number(d->itemsRotationAngle) );
    writer->writeAttribute( "size", QString::number(d->itemsSize) );
    WRITE_QBRUSH(d->itemsBrush);
    WRITE_QPEN(d->itemsPen);
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

    writer->writeEndElement(); // close "CustomItem" section
}

//! Load from XML
bool CustomItem::load(XmlStreamReader* reader){
    Q_D(CustomItem);

   if(!reader->isStartElement() || reader->name() != "customItem"){
        reader->raiseError(i18n("no custom-item element found"));
        return false;
    }

    if (!readBasicAttributes(reader))
        return false;

    QString attributeWarning = i18n("Attribute '%1' missing or empty, default value is used");
    QXmlStreamAttributes attribs;
    QString str;

    while (!reader->atEnd()){
        reader->readNext();
        if (reader->isEndElement() && reader->name() == "customItem")
            break;

        if (!reader->isStartElement())
            continue;

        if (reader->name() == "comment"){
            if (!readCommentElement(reader)) return false;
        }else if (reader->name() == "geometry"){
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
                d->position.horizontalPosition = (CustomItem::HorizontalPosition)str.toInt();

            str = attribs.value("verticalPosition").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'verticalPosition'"));
            else
                d->position.verticalPosition = (CustomItem::VerticalPosition)str.toInt();

            str = attribs.value("visible").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'visible'"));
            else
                d->setVisible(str.toInt());
        } else if (reader->name() == "errorBar"){
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

        } else if (reader->name() == "properties"){
            attribs = reader->attributes();

            str = attribs.value("itemsStyle").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'itemsStyle'"));
            else
                d->itemsStyle = (CustomItem::ItemsStyle)str.toInt();

            str = attribs.value("opacity").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'opacity'"));
            else
                d->itemsOpacity = str.toDouble();

            str = attribs.value("rotation").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'rotation'"));
            else
                d->itemsRotationAngle = str.toDouble();

            str = attribs.value("size").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'size'"));
            else
                d->itemsSize = str.toDouble();

            READ_QBRUSH(d->itemsBrush);
            READ_QPEN(d->itemsPen);
        } else { // unknown element
            reader->raiseWarning(i18n("unknown element '%1'", reader->name().toString()));
            if (!reader->skipToEndElement()) return false;
        }
    }

    retransform();
    return true;
}
