/***************************************************************************
    File                 : WorksheetElementContainer.cpp
    Project              : LabPlot/SciDAVis
    Description          : Generic WorksheetElement container.
    --------------------------------------------------------------------
    Copyright            : (C) 2009 Tilman Benkert (thzs*gmx.net)
    Copyright            : (C) 2012 by Alexander Semke (alexander.semke*web.de)
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

#include "backend/worksheet/WorksheetElementContainer.h"
#include "backend/worksheet/WorksheetElementContainerPrivate.h"
#include <QtDebug>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QPainter>

/**
 * \class WorksheetElementContainer
 * \brief Generic AbstractWorksheetElement container.
 * \ingroup worksheet
 * This class combines functionality of worksheet element containers
 * such as groups, plots etc.
 *
 */

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

WorksheetElementContainer::~WorksheetElementContainer(){
	delete d_ptr;
}

QGraphicsItem *WorksheetElementContainer::graphicsItem() const {
	return const_cast<QGraphicsItem *>(static_cast<const QGraphicsItem *>(d_ptr));
}

QRectF WorksheetElementContainer::rect() const{
	Q_D(const WorksheetElementContainer);
	return d->rect;
}

void WorksheetElementContainer::setVisible(bool on){
	Q_D(WorksheetElementContainer);
	
	//change the visibility of the containter itself
	d->setVisible(on);
	
	//change also the visibility of all children
	QList<AbstractWorksheetElement *> childList = children<AbstractWorksheetElement>(AbstractAspect::IncludeHidden | AbstractAspect::Compress);
	foreach(AbstractWorksheetElement *elem, childList){
		elem->setVisible(on);
	}
}

bool WorksheetElementContainer::isVisible() const {
	Q_D(const WorksheetElementContainer);
	return d->isVisible();
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
	Q_D(WorksheetElementContainer);
	QRectF rect(d->rect);
	rect.setWidth(d->rect.width()*horizontalRatio);
	rect.setHeight(d->rect.height()*verticalRatio);
	setRect(rect);
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

//################################################################
//################### Private implementation ##########################
//################################################################
WorksheetElementContainerPrivate::WorksheetElementContainerPrivate(WorksheetElementContainer *owner)
	: q(owner) {
	this->setAcceptHoverEvents(true);
}

void WorksheetElementContainerPrivate::contextMenuEvent(QGraphicsSceneContextMenuEvent *event) {
	scene()->clearSelection();
	setSelected(true);
	QMenu* menu = q->createContextMenu();
	menu->exec(event->screenPos());
}
void WorksheetElementContainerPrivate::hoverLeaveEvent (QGraphicsSceneHoverEvent*) {
	m_hovered = false;
	update();
}

void WorksheetElementContainerPrivate::hoverEnterEvent ( QGraphicsSceneHoverEvent * ) {
	m_hovered = true;
	update();
}

// Inherited from QGraphicsItem
QRectF WorksheetElementContainerPrivate::boundingRect() const {
	QRectF rect;
	QList<AbstractWorksheetElement *> childList = q->children<AbstractWorksheetElement>(AbstractAspect::IncludeHidden | AbstractAspect::Compress);
	foreach(const AbstractWorksheetElement *elem, childList)
		rect |= elem->graphicsItem()->mapRectToParent( elem->graphicsItem()->boundingRect() );
	return rect;
}
 
// Inherited from QGraphicsItem
void WorksheetElementContainerPrivate::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
	Q_UNUSED(option)
	Q_UNUSED(widget)
	
	if (!isVisible())
		return;

	if (m_hovered && !isSelected()){
		QPainterPath path = shape();
		painter->setPen(QPen(QColor(128,179,255), 10, Qt::SolidLine));
		painter->drawPath(path);
	}
	
	//TODO remove this later
	if (isSelected()){
		QPainterPath path = shape();  
		painter->setPen(QPen(Qt::blue, 10, Qt::SolidLine));
		painter->drawPath(path);
  }
}
