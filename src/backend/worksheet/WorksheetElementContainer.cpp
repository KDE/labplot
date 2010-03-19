/***************************************************************************
    File                 : WorksheetElementContainer.cpp
    Project              : LabPlot/SciDAVis
    Description          : Generic WorksheetElement container.
    --------------------------------------------------------------------
    Copyright            : (C) 2009 Tilman Benkert (thzs*gmx.net)
                           (replace * with @ in the email addresses) 
                           
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

#include "worksheet/WorksheetElementContainer.h"
#include "worksheet/WorksheetElementContainerPrivate.h"
#include <QtDebug>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QMenu>

/**
 * \class WorksheetElementContainer
 * \brief Generic AbstractWorksheetElement container.
 * \ingroup datasources
 * This class combines functionality of worksheet element containers
 * such as groups, plots, coordinate systems, etc.
 *
 */

WorksheetElementContainerPrivate::WorksheetElementContainerPrivate(WorksheetElementContainer *owner)
	: q(owner) {
}

WorksheetElementContainerPrivate::~WorksheetElementContainerPrivate() {
}

 void WorksheetElementContainerPrivate::contextMenuEvent(QGraphicsSceneContextMenuEvent *event){
     scene()->clearSelection();
     setSelected(true);
     QMenu* menu = q->createContextMenu();
     menu->exec(event->screenPos());
 }
 
 
//  QVariant WorksheetElementContainerPrivate::itemChange(GraphicsItemChange change, const QVariant &value){
//    /*
//      if (change == ItemPositionChange && scene()) {
//          // value is the new position.
//          QPointF newPos = value.toPointF();
//          QRectF rect = scene()->sceneRect();
//          if (!rect.contains(newPos)) {
//              // Keep the item inside the scene rect.
//              newPos.setX(qMin(rect.right(), qMax(newPos.x(), rect.left())));
//              newPos.setY(qMin(rect.bottom(), qMax(newPos.y(), rect.top())));
//              return newPos;
//          }
//      }
//      */
//     if (change == QGraphicsItem::ItemSelectedHasChanged)
//         emit selectedChange(this);
// //     return value;
// 	
//      return QGraphicsItem::itemChange(change, value);
//  }
 
WorksheetElementContainer::WorksheetElementContainer(const QString &name) 
	: AbstractWorksheetElement(name), d_ptr(new WorksheetElementContainerPrivate(this)) {
	  
	connect(this, SIGNAL(aspectAdded(const AbstractAspect*)),
		this, SLOT(handleAspectAdded(const AbstractAspect*)));
	connect(this, SIGNAL(aspectAboutToBeRemoved(const AbstractAspect*)),
		this, SLOT(handleAspectAboutToBeRemoved(const AbstractAspect*)));
}

WorksheetElementContainer::WorksheetElementContainer(const QString &name, WorksheetElementContainerPrivate *dd)
    : AbstractWorksheetElement(name), d_ptr(dd) {
      
	connect(this, SIGNAL(aspectAdded(const AbstractAspect*)),
		this, SLOT(handleAspectAdded(const AbstractAspect*)));
	connect(this, SIGNAL(aspectAboutToBeRemoved(const AbstractAspect*)),
		this, SLOT(handleAspectAboutToBeRemoved(const AbstractAspect*)));
}

WorksheetElementContainer::~WorksheetElementContainer() {
	delete d_ptr;
}

QGraphicsItem *WorksheetElementContainer::graphicsItem() const {
	return const_cast<QGraphicsItem *>(static_cast<const QGraphicsItem *>(d_ptr));
}

QRectF WorksheetElementContainerPrivate::boundingRect() const {
	QRectF rect;
	QList<AbstractWorksheetElement *> childList = q->children<AbstractWorksheetElement>(AbstractAspect::IncludeHidden | AbstractAspect::Compress);
	foreach(const AbstractWorksheetElement *elem, childList)
		rect |= elem->graphicsItem()->boundingRect();
	return rect;
}
    
void WorksheetElementContainerPrivate::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
//   QGraphicsItem::paint(painter,option, widget);
	Q_UNUSED(painter)
	Q_UNUSED(option)
	Q_UNUSED(widget)
}

void WorksheetElementContainer::setVisible(bool on) {
	QList<AbstractWorksheetElement *> childList = children<AbstractWorksheetElement>(AbstractAspect::IncludeHidden | AbstractAspect::Compress);
	foreach(AbstractWorksheetElement *elem, childList)
		elem->setVisible(on);
}

bool WorksheetElementContainer::isVisible() const {
	QList<AbstractWorksheetElement *> childList = children<AbstractWorksheetElement>(AbstractAspect::IncludeHidden | AbstractAspect::Compress);
	foreach(const AbstractWorksheetElement *elem, childList) {
		if (elem->isVisible()) 
			return true;
	}
	return false;
}

bool WorksheetElementContainer::isFullyVisible() const {
	QList<AbstractWorksheetElement *> childList = children<AbstractWorksheetElement>(AbstractAspect::IncludeHidden | AbstractAspect::Compress);
	foreach(const AbstractWorksheetElement *elem, childList) {
		if (!elem->isVisible()) 
			return false;
	}
	return true;
}

void WorksheetElementContainer::retransform() {
	QList<AbstractWorksheetElement *> childList = children<AbstractWorksheetElement>(AbstractAspect::IncludeHidden | AbstractAspect::Compress);
	foreach(AbstractWorksheetElement *elem, childList)
		elem->retransform();
}

void WorksheetElementContainer::handlePageResize(double horizontalRatio, double verticalRatio) {
	QList<AbstractWorksheetElement *> childList = children<AbstractWorksheetElement>(AbstractAspect::IncludeHidden | AbstractAspect::Compress);
	foreach(AbstractWorksheetElement *elem, childList)
		elem->handlePageResize(horizontalRatio, verticalRatio);
}

void WorksheetElementContainer::handleAspectAdded(const AbstractAspect *aspect) {
	Q_D(WorksheetElementContainer);

	const AbstractWorksheetElement *addedElement = qobject_cast<const AbstractWorksheetElement*>(aspect);
	if (addedElement && (aspect->parentAspect() == this)) {
		QGraphicsItem *item = addedElement->graphicsItem();
		Q_ASSERT(item != NULL);
		item->setParentItem(d);			

		qreal zVal = 0;
		QList<AbstractWorksheetElement *> childElements = children<AbstractWorksheetElement>(IncludeHidden);
		foreach(AbstractWorksheetElement *elem, childElements) {
			elem->graphicsItem()->setZValue(zVal++);
		}
	}
}

void WorksheetElementContainer::handleAspectAboutToBeRemoved(const AbstractAspect *aspect) {
	const AbstractWorksheetElement *removedElement = qobject_cast<const AbstractWorksheetElement*>(aspect);
	if (removedElement && (aspect->parentAspect() == this)) {
		QGraphicsItem *item = removedElement->graphicsItem();
		Q_ASSERT(item != NULL);
		item->setParentItem(NULL);			
	}
}
