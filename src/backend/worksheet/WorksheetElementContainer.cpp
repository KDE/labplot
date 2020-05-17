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
#include "backend/worksheet/plots/cartesian/Axis.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/macros.h"
#include "backend/lib/trace.h"

#include <QGraphicsScene>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QPainter>

#include <KLocalizedString>

/**
 * \class WorksheetElementContainer
 * \ingroup worksheet
 * \brief Worksheet element container - parent of multiple elements
 * This class provides the functionality for a containers of multiple
 * worksheet elements. Such a container can be a plot or group of elements.
 */

WorksheetElementContainer::WorksheetElementContainer(const QString& name, AspectType type)
	: WorksheetElement(name, type), d_ptr(new WorksheetElementContainerPrivate(this)) {

	connect(this, &WorksheetElementContainer::aspectAdded, this, &WorksheetElementContainer::handleAspectAdded);
}

WorksheetElementContainer::WorksheetElementContainer(const QString& name, WorksheetElementContainerPrivate* dd, AspectType type)
	: WorksheetElement(name, type), d_ptr(dd) {

	connect(this, &WorksheetElementContainer::aspectAdded, this, &WorksheetElementContainer::handleAspectAdded);
}

//no need to delete the d-pointer here - it inherits from QGraphicsItem
//and is deleted during the cleanup in QGraphicsScene
WorksheetElementContainer::~WorksheetElementContainer() = default;

QGraphicsItem* WorksheetElementContainer::graphicsItem() const {
	return const_cast<QGraphicsItem*>(static_cast<const QGraphicsItem*>(d_ptr));
}

QRectF WorksheetElementContainer::rect() const {
	Q_D(const WorksheetElementContainer);
	return d->rect;
}

STD_SWAP_METHOD_SETTER_CMD_IMPL(WorksheetElementContainer, SetVisible, bool, swapVisible)
void WorksheetElementContainer::setVisible(bool on) {
	Q_D(WorksheetElementContainer);

	//take care of proper ordering on the undo-stack,
	//when making the container and all its children visible/invisible.
	//if visible is set true, change the visibility of the container first
	if (on) {
		beginMacro( i18n("%1: set visible", name()) );
		exec( new WorksheetElementContainerSetVisibleCmd(d, on, ki18n("%1: set visible")) );
	} else {
		beginMacro( i18n("%1: set invisible", name()) );
	}

	//change the visibility of all children
	QVector<WorksheetElement*> childList = children<WorksheetElement>(AbstractAspect::ChildIndexFlag::IncludeHidden | AbstractAspect::ChildIndexFlag::Compress);
	for (auto* elem : childList)
		elem->setVisible(on);

	//if visible is set false, change the visibility of the container last
	if (!on)
		exec(new WorksheetElementContainerSetVisibleCmd(d, false, ki18n("%1: set invisible")));

	endMacro();
}

bool WorksheetElementContainer::isVisible() const {
	Q_D(const WorksheetElementContainer);
	return d->isVisible();
}

bool WorksheetElementContainer::isFullyVisible() const {
	QVector<WorksheetElement*> childList = children<WorksheetElement>(AbstractAspect::ChildIndexFlag::IncludeHidden | AbstractAspect::ChildIndexFlag::Compress);
	for (const auto* elem : childList) {
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
// 	if (isLoading())
// 		return;

	PERFTRACE("WorksheetElementContainer::retransform()");
	Q_D(WorksheetElementContainer);

	QVector<WorksheetElement*> childList = children<WorksheetElement>(AbstractAspect::ChildIndexFlag::IncludeHidden | AbstractAspect::ChildIndexFlag::Compress);
	for (auto* child : childList)
		child->retransform();

	d->recalcShapeAndBoundingRect();
}

/*!
 * called if the size of the worksheet page was changed and the content has to be adjusted/resized (\c pageResize = true)
 * or if a new rectangular for the element container was set (\c pageResize = false).
 * In the second case, \c WorksheetElement::handleResize() is called for every worksheet child to adjuste the content to the new size.
 * In the first case, a new rectangular for the container is calculated and set first, which on the other hand, triggers the content adjustments
 * in the container children.
 */
void WorksheetElementContainer::handleResize(double horizontalRatio, double verticalRatio, bool pageResize) {
	DEBUG("WorksheetElementContainer::handleResize()");
	Q_D(const WorksheetElementContainer);
	if (pageResize) {
		QRectF rect(d->rect);
		rect.setWidth(d->rect.width()*horizontalRatio);
		rect.setHeight(d->rect.height()*verticalRatio);
		setRect(rect);
	} else {
// 		for (auto* elem : children<WorksheetElement>(IncludeHidden))
// 			elem->handleResize(horizontalRatio, verticalRatio);
	}
}

void WorksheetElementContainer::handleAspectAdded(const AbstractAspect* aspect) {
	Q_D(WorksheetElementContainer);

	const auto* element = qobject_cast<const WorksheetElement*>(aspect);
	if (element && (aspect->parentAspect() == this)) {
		connect(element, &WorksheetElement::hovered, this, &WorksheetElementContainer::childHovered);
		connect(element, &WorksheetElement::unhovered, this, &WorksheetElementContainer::childUnhovered);
		element->graphicsItem()->setParentItem(d);

		qreal zVal = 0;
		for (auto* child : children<WorksheetElement>(ChildIndexFlag::IncludeHidden))
			child->setZValue(zVal++);
	}

	if (!isLoading())
		d->recalcShapeAndBoundingRect();
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
WorksheetElementContainerPrivate::WorksheetElementContainerPrivate(WorksheetElementContainer *owner) : q(owner) {
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

bool WorksheetElementContainerPrivate::swapVisible(bool on) {
	bool oldValue = isVisible();
	setVisible(on);
	emit q->visibleChanged(on);
	return oldValue;
}

void WorksheetElementContainerPrivate::prepareGeometryChangeRequested() {
	prepareGeometryChange();
	recalcShapeAndBoundingRect();
}

void WorksheetElementContainerPrivate::recalcShapeAndBoundingRect() {
// 	if (q->isLoading())
// 		return;

	//old logic calculating the bounding box as as the box covering all children.
	//we might need this logic later once we implement something like selection of multiple plots, etc.
// 	boundingRectangle = QRectF();
// 	QVector<WorksheetElement*> childList = q->children<WorksheetElement>(AbstractAspect::IncludeHidden | AbstractAspect::Compress);
// 	foreach (const WorksheetElement* elem, childList)
// 		boundingRectangle |= elem->graphicsItem()->mapRectToParent(elem->graphicsItem()->boundingRect());
//
	float penWidth = 2.;
	boundingRectangle = q->rect();
	boundingRectangle = QRectF(-boundingRectangle.width()/2 - penWidth / 2, -boundingRectangle.height()/2 - penWidth / 2,
				  boundingRectangle.width() + penWidth, boundingRectangle.height() + penWidth);

	QPainterPath path;
	path.addRect(boundingRectangle);

	//make the shape somewhat thicker then the hoveredPen to make the selection/hovering box more visible
	containerShape = QPainterPath();
	containerShape.addPath(WorksheetElement::shapeFromPath(path, QPen(QBrush(), penWidth)));
}

// Inherited from QGraphicsItem
QRectF WorksheetElementContainerPrivate::boundingRect() const {
	return boundingRectangle;
}

// Inherited from QGraphicsItem
void WorksheetElementContainerPrivate::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
	Q_UNUSED(option)
	Q_UNUSED(widget)

	if (!isVisible())
		return;

	if (m_hovered && !isSelected() && !m_printing) {
		painter->setPen(QPen(QApplication::palette().color(QPalette::Shadow), 2, Qt::SolidLine));
		painter->drawPath(containerShape);
	}

	if (isSelected() && !m_printing) {
		painter->setPen(QPen(QApplication::palette().color(QPalette::Highlight), 2, Qt::SolidLine));
		painter->drawPath(containerShape);
	}
}
