
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

#include <QPainter>
#include <QGraphicsScene>
#include <QMenu>
#include <QGraphicsSceneMouseEvent>

#include <KIcon>
#include <KConfig>
#include <KConfigGroup>
#include <KLocale>

CustomItem::CustomItem(const QString& name):WorksheetElement(name),
    d_ptr(new CustomItemPrivate(this)) {
    init();
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
    d->itemErrorBar.minusDeltaX = group.readEntry("MinusDeltaX", 0);
    d->itemErrorBar.plusDeltaX = group.readEntry("PlusDeltaX", 0);
    d->itemErrorBar.minusDeltaY = group.readEntry("MinusDeltaY", 0);
    d->itemErrorBar.plusDeltaY = group.readEntry("PlusDeltaY", 0);
    this->initActions();
}

void CustomItem::initActions() {
    visibilityAction = new QAction(i18n("visible"), this);
    visibilityAction->setCheckable(true);
    connect(visibilityAction, SIGNAL(triggered()), this, SLOT(visibilityChanged()));
}

CustomItem::~CustomItem() {
    //no need to delete the d-pointer here - it inherits from QGraphicsItem
    //and is deleted during the cleanup in QGraphicsScene
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

/*!
    Returns an icon to be used in the project explorer.
*/
QIcon CustomItem::icon() const{
    return  KIcon("");
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

/* ============================ getter methods ================= */
CLASS_SHARED_D_READER_IMPL(CustomItem, CustomItem::PositionWrapper, position, position)
CLASS_SHARED_D_READER_IMPL(CustomItem, CustomItem::ErrorBar, itemErrorBar, itemErrorBar)
BASIC_SHARED_D_READER_IMPL(CustomItem, CustomItem::ItemsStyle, itemsStyle, itemsStyle)
BASIC_SHARED_D_READER_IMPL(CustomItem, qreal, itemsOpacity, itemsOpacity)
BASIC_SHARED_D_READER_IMPL(CustomItem, qreal, itemsRotationAngle, itemsRotationAngle)
BASIC_SHARED_D_READER_IMPL(CustomItem, qreal, itemsSize, itemsSize)
CLASS_SHARED_D_READER_IMPL(CustomItem, QBrush, itemsBrush, itemsBrush)
CLASS_SHARED_D_READER_IMPL(CustomItem, QPen, itemsPen, itemsPen)

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
    if (pos.point!=d->position.point || pos.horizontalPosition!=d->position.horizontalPosition || pos.verticalPosition!=d->position.verticalPosition)
        exec(new CustomItemSetPositionCmd(d, pos, i18n("%1: set position")));
}

STD_SETTER_CMD_IMPL_F_S(CustomItem, SetItemErrorBar, CustomItem::ErrorBar, itemErrorBar, retransform)
void CustomItem::setItemErrorBar(const ErrorBar& error) {
    Q_D(CustomItem);
    if (memcmp(&error, &d->itemErrorBar, sizeof(error)) != 0)
        exec(new CustomItemSetItemErrorBarCmd(d, error, i18n("%1: set error")));
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

QPainterPath CustomItem::errorBarsPath() {
    QPainterPath path;
    QPolygonF polygon;
    if (itemErrorBar().minusDeltaX || itemErrorBar().plusDeltaX) {
        polygon<<QPointF(0, 0)<<QPointF(-itemErrorBar().minusDeltaX, 0)<<QPointF(-itemErrorBar().minusDeltaX, 5)
              <<QPointF(-itemErrorBar().minusDeltaX, -5)<<QPointF(-itemErrorBar().minusDeltaX, 0)
             <<QPointF(itemErrorBar().plusDeltaX, 0)<<QPointF(itemErrorBar().plusDeltaX, 5)
            <<QPointF(itemErrorBar().plusDeltaX, -5)<<QPointF(itemErrorBar().plusDeltaX, 0);
        path.addPolygon(polygon);
    }

    if (itemErrorBar().minusDeltaY || itemErrorBar().plusDeltaY) {
        polygon<<QPointF(0, 0)<<QPointF(0, -itemErrorBar().plusDeltaY)<<QPointF(5, -itemErrorBar().plusDeltaY)
              <<QPointF(-5, -itemErrorBar().plusDeltaY)<<QPointF(0, -itemErrorBar().plusDeltaY)
             <<QPointF(0, itemErrorBar().minusDeltaY)<<QPointF(5, itemErrorBar().minusDeltaY)
            <<QPointF(-5, itemErrorBar().minusDeltaY)<<QPointF(0, itemErrorBar().minusDeltaY);
        path.addPolygon(polygon);
    }

    return path;
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

/*!
 * position is set to invalid if the parent item is not drawn on the scene
 * (e.g. axis is not drawn because it's outside plot ranges -> don't draw axis' title label)
 */
void CustomItem::setPositionInvalid(bool invalid) {
    Q_D(CustomItem);
    if (invalid != d->positionInvalid){
        d->positionInvalid = invalid;
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
void CustomItem::visibilityChanged(){
    Q_D(const CustomItem);
    this->setVisible(!d->isVisible());
}

//##############################################################################
//####################### Private implementation ###############################
//##############################################################################
CustomItemPrivate::CustomItemPrivate(CustomItem *owner)
        : positionInvalid(false),
          suppressItemChangeEvent(false),
          suppressRetransform(false),
          m_printing(false),
          m_hovered(false),
          m_suppressHoverEvents(false),
          q(owner){
    setFlag(QGraphicsItem::ItemSendsGeometryChanges);
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
    QPainterPath errorBar = q->errorBarsPath();
    path.addPath(errorBar);
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

    if (positionInvalid)
        return;
    QPainterPath path = CustomItem::itemsPathFromStyle(itemsStyle);
    QPainterPath errorBar = q->errorBarsPath();
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
    painter->drawPath(errorBar);
    painter->drawPath(path);
    painter->restore();

    if (m_suppressHoverEvents) {
        if (m_hovered && !isSelected() && !m_printing){
            painter->setPen(q->hoveredPen);
            painter->setOpacity(q->hoveredOpacity);
            painter->drawPath(itemShape);
        }

        if (isSelected() && !m_printing){
            painter->setPen(q->selectedPen);
            painter->setOpacity(q->selectedOpacity);
            painter->drawPath(itemShape);
        }
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

    writer->writeEndElement(); // close "CustomItem" section
}

//! Load from XML
bool CustomItem::load(XmlStreamReader* reader){
    Q_D(CustomItem);

   if(!reader->isStartElement() || reader->name() != "customItem"){
        reader->raiseError(i18n("no CustomItem element found"));
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
        }else if (reader->name() == "properties"){
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
        }else{ // unknown element
            reader->raiseWarning(i18n("unknown element '%1'", reader->name().toString()));
            if (!reader->skipToEndElement()) return false;
        }
    }
    retransform();

    return true;
}

