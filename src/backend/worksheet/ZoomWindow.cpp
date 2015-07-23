
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

#include "ZoomWindow.h"
#include "ZoomWindowPrivate.h"

#include <QPainter>
#include <QMenu>
#include <QGraphicsSceneMouseEvent>

#include <KIcon>

ZoomWindow::ZoomWindow(const QString& name):WorksheetElement(name),
    d_ptr(new ZoomWindowPrivate(this)), scaleFactor(1) {
}

ZoomWindow::~ZoomWindow() {
    //no need to delete the d-pointer here - it inherits from QGraphicsItem
    //and is deleted during the cleanup in QGraphicsScene
}

QGraphicsItem* ZoomWindow::graphicsItem() const {
    return d_ptr;
}

void ZoomWindow::setParentGraphicsItem(QGraphicsItem* item) {
    Q_D(ZoomWindow);
    d->setParentItem(item);
}

void ZoomWindow::updatePixmap(const WId& winId, const QPoint& point) {
    Q_D(ZoomWindow);
    int size = 100/scaleFactor;
    m_pixmap = QPixmap::grabWindow(winId, point.x() - size/2, point.y() - size/2, size, size);
    m_pixmap = m_pixmap.scaled(200, 200, Qt::IgnoreAspectRatio);
    d->setPixmap(m_pixmap);
    retransform();
}

void ZoomWindow::retransform(){
    Q_D(ZoomWindow);
    d->retransform();
}

/*!
    Returns an icon to be used in the project explorer.
*/
QIcon ZoomWindow::icon() const{
    return  KIcon("");
}

QMenu* ZoomWindow::createContextMenu(){
    QMenu *menu = WorksheetElement::createContextMenu();
    menu->actions().removeFirst();
    return menu;
}

/*!
    sets the position without undo/redo-stuff
*/
void ZoomWindow::setPosition(const QPointF& point) {
    Q_D(ZoomWindow);
    d->setPos(point);
    emit changed();
}

void ZoomWindow::setScaleFactor(const int value) {
    scaleFactor = value;
}

void ZoomWindow::setVisible(bool on) {
    Q_D(ZoomWindow);
    if (on != isVisible()) {
        d->setVisible(on);
        emit changed();
        emit visibleChanged(on);
    }
}

bool ZoomWindow::isVisible() const {
    Q_D(const ZoomWindow);
    return d->isVisible();
}

void ZoomWindow::setPrinting(bool on) {
    Q_D(ZoomWindow);
    d->m_printing = on;
}

//##############################################################################
//####################### Private implementation ###############################
//##############################################################################
ZoomWindowPrivate::ZoomWindowPrivate(ZoomWindow *owner)
    : m_printing(false),
      m_hovered(false),
      q(owner) {
    setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setAcceptHoverEvents(true);
}

QString ZoomWindowPrivate::name() const{
    return q->name();
}

/*!
    calculates the position and the bounding box of the item. Called on geometry or properties changes.
 */
void ZoomWindowPrivate::retransform() {
    boundingRectangle = q->m_pixmap.rect();
    windowShape = QPainterPath();
    windowShape.addRect(boundingRectangle);
    emit(q->changed());
}

/*!
    Returns the outer bounds of the item as a rectangle.
 */
QRectF ZoomWindowPrivate::boundingRect() const{
    return boundingRectangle;
}

/*!
    Returns the shape of this item as a QPainterPath in local coordinates.
*/
QPainterPath ZoomWindowPrivate::shape() const{
    return windowShape;
}

void ZoomWindowPrivate::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget * widget) {
    QGraphicsPixmapItem::paint(painter,option,widget);

    if (m_hovered && !isSelected() && !m_printing) {
        painter->setPen(q->hoveredPen);
        painter->setOpacity(q->hoveredOpacity);
        painter->drawPath(windowShape);
    }

    if (isSelected() && !m_printing) {
        painter->setPen(q->selectedPen);
        painter->setOpacity(q->selectedOpacity);
        painter->drawPath(windowShape);
    }
}


void ZoomWindowPrivate::contextMenuEvent(QGraphicsSceneContextMenuEvent* event) {
    q->createContextMenu()->exec(event->screenPos());
}

void ZoomWindowPrivate::hoverEnterEvent(QGraphicsSceneHoverEvent*) {
    if (!isSelected()) {
        m_hovered = true;
        q->hovered();
        update();
    }
}

void ZoomWindowPrivate::hoverLeaveEvent(QGraphicsSceneHoverEvent*) {
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
void ZoomWindow::save(QXmlStreamWriter* writer) const{
    Q_UNUSED(writer)
}

//! Load from XML
bool ZoomWindow::load(XmlStreamReader* reader){
    Q_UNUSED(reader)
    return true;
}
