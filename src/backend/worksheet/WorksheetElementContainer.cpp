/***************************************************************************
    File                 : WorksheetElementContainer.cpp
    Project              : LabPlot
    Description          : Worksheet element container - parent of multiple elements
    --------------------------------------------------------------------
    Copyright            : (C) 2009 Tilman Benkert (thzs@gmx.net)
    Copyright            : (C) 2012-2015 by Alexander Semke (alexander.semke@web.de)
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
#include "backend/lib/commandtemplates.h"
#include "backend/lib/macros.h"

#include <QtDebug>
#include <QGraphicsScene>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QPainter>

#include <KLocale>

/**
 * \class WorksheetElementContainer
 * \brief Worksheet element container - parent of multiple elements
 * \ingroup worksheet
 * This class provides the functionality for a containers of multiple
 * worksheet elements. Such a container can be a plot or group of elements.
 *
 */

WorksheetElementContainer::WorksheetElementContainer(const QString& name)
	: WorksheetElement(name), d_ptr(new WorksheetElementContainerPrivate(this)) {

	connect(this, SIGNAL(aspectAdded(const AbstractAspect*)),
		this, SLOT(handleAspectAdded(const AbstractAspect*)));
}

WorksheetElementContainer::WorksheetElementContainer(const QString& name, WorksheetElementContainerPrivate* dd)
    : WorksheetElement(name), d_ptr(dd) {

	connect(this, SIGNAL(aspectAdded(const AbstractAspect*)),
		this, SLOT(handleAspectAdded(const AbstractAspect*)));
}

WorksheetElementContainer::~WorksheetElementContainer(){
	//no need to delete the d-pointer here - it inherits from QGraphicsItem
	//and is deleted during the cleanup in QGraphicsScene
}

QGraphicsItem* WorksheetElementContainer::graphicsItem() const {
	return const_cast<QGraphicsItem*>(static_cast<const QGraphicsItem*>(d_ptr));
}

QRectF WorksheetElementContainer::rect() const{
	Q_D(const WorksheetElementContainer);
	return d->rect;
}

STD_SWAP_METHOD_SETTER_CMD_IMPL(WorksheetElementContainer, SetVisible, bool, swapVisible)
void WorksheetElementContainer::setVisible(bool on){
	Q_D(WorksheetElementContainer);

	beginMacro(on ? i18n("%1: set visible", name()) : i18n("%1: set invisible", name()));

	//take care of proper ordering on the undo-stack,
	//when making the container and all its children visible/invisible.
	//if visible is set true, change the visibility of the container first
	if (on)
    	exec(new WorksheetElementContainerSetVisibleCmd(d, on, on ? i18n("%1: set visible") : i18n("%1: set invisible")));

	//change the visibility of all children
	QList<WorksheetElement *> childList = children<WorksheetElement>(AbstractAspect::IncludeHidden | AbstractAspect::Compress);
	foreach(WorksheetElement *elem, childList){
		elem->setVisible(on);
	}

	//if visible is set false, change the visibility of the container last
	if (!on)
    	exec(new WorksheetElementContainerSetVisibleCmd(d, on, on ? i18n("%1: set visible") : i18n("%1: set invisible")));

	endMacro();
}

bool WorksheetElementContainer::isVisible() const {
	Q_D(const WorksheetElementContainer);
	return d->isVisible();
}

bool WorksheetElementContainer::isFullyVisible() const {
	QList<WorksheetElement*> childList = children<WorksheetElement>(AbstractAspect::IncludeHidden | AbstractAspect::Compress);
	foreach(const WorksheetElement* elem, childList) {
		if (!elem->isVisible())
			return false;
	}
	return true;
}

void WorksheetElementContainer::setPrinting(bool on) {
	Q_D(WorksheetElementContainer);
	d->m_printing = on;
}

void WorksheetElementContainer::retransform() {
	QList<WorksheetElement*> childList = children<WorksheetElement>(AbstractAspect::IncludeHidden | AbstractAspect::Compress);
	foreach(WorksheetElement* elem, childList)
		elem->retransform();
}

void WorksheetElementContainer::handlePageResize(double horizontalRatio, double verticalRatio) {
// 	qDebug()<<"WorksheetElementContainer::handlePageResize";
	Q_D(WorksheetElementContainer);
	QRectF rect(d->rect);
	rect.setWidth(d->rect.width()*horizontalRatio);
	rect.setHeight(d->rect.height()*verticalRatio);
	setRect(rect);
}

void WorksheetElementContainer::handleAspectAdded(const AbstractAspect* aspect) {
	Q_D(WorksheetElementContainer);

	const WorksheetElement* element = qobject_cast<const WorksheetElement*>(aspect);
	if (element && (aspect->parentAspect() == this)) {
		connect(element, SIGNAL(hovered()), this, SLOT(childHovered()));
		connect(element, SIGNAL(unhovered()), this, SLOT(childUnhovered()));
		QGraphicsItem *item = element->graphicsItem();
		Q_ASSERT(item != NULL);
		item->setParentItem(d);

		qreal zVal = 0;
		QList<WorksheetElement*> childElements = children<WorksheetElement>(IncludeHidden);
		foreach(WorksheetElement *elem, childElements) {
			elem->graphicsItem()->setZValue(zVal++);
		}
	}
}

void WorksheetElementContainer::childHovered() {
	Q_D(WorksheetElementContainer);
	if (!d->isSelected()) {
		if (d->m_hovered)
			d->m_hovered = false;
		d->update();
	}
}

void WorksheetElementContainer::childUnhovered() {
	Q_D(WorksheetElementContainer);
	if (!d->isSelected()) {
		d->m_hovered = true;
		d->update();
	}
}

void WorksheetElementContainer::prepareGeometryChange() {
	Q_D(WorksheetElementContainer);
	d->prepareGeometryChangeRequested();
}

//################################################################
//################### Private implementation ##########################
//################################################################
WorksheetElementContainerPrivate::WorksheetElementContainerPrivate(WorksheetElementContainer *owner)
	: q(owner), m_hovered(false), m_printing(false) {
	setAcceptHoverEvents(true);
}

QString WorksheetElementContainerPrivate::name() const {
	return q->name();
}

void WorksheetElementContainerPrivate::contextMenuEvent(QGraphicsSceneContextMenuEvent* event) {
	scene()->clearSelection();
	setSelected(true);
	QMenu* menu = q->createContextMenu();
	menu->exec(event->screenPos());
}

void WorksheetElementContainerPrivate::hoverEnterEvent(QGraphicsSceneHoverEvent*) {
	if (!isSelected()) {
		m_hovered = true;
		update();
	}
}

void WorksheetElementContainerPrivate::hoverLeaveEvent(QGraphicsSceneHoverEvent*) {
	if (m_hovered) {
		m_hovered = false;
		update();
	}
}

bool WorksheetElementContainerPrivate::swapVisible(bool on){
	bool oldValue = isVisible();
	setVisible(on);
	emit q->visibleChanged(on);
	return oldValue;
}

void WorksheetElementContainerPrivate::prepareGeometryChangeRequested() {
	prepareGeometryChange();
}

// Inherited from QGraphicsItem
QRectF WorksheetElementContainerPrivate::boundingRect() const {
	QRectF rect;
	QList<WorksheetElement *> childList = q->children<WorksheetElement>(AbstractAspect::IncludeHidden | AbstractAspect::Compress);
	foreach(const WorksheetElement *elem, childList)
		rect |= elem->graphicsItem()->mapRectToParent( elem->graphicsItem()->boundingRect() );
	return rect;
}

// Inherited from QGraphicsItem
void WorksheetElementContainerPrivate::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
// 	qDebug()<<"WorksheetElementContainerPrivate::paint";
	Q_UNUSED(option)
	Q_UNUSED(widget)

	if (!isVisible())
		return;

	if (m_hovered && !isSelected() && !m_printing){
		painter->setPen(q->hoveredPen);
		painter->setOpacity(q->hoveredOpacity);
		painter->drawRect(boundingRect());
	}

	if (isSelected() && !m_printing){
		painter->setPen(q->selectedPen);
		painter->setOpacity(q->selectedOpacity);
		painter->drawRect(boundingRect());
  }
}
