/*
	File                 : WorksheetElementContainer.cpp
	Project              : LabPlot
	Description          : Worksheet element container - parent of multiple elements
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2009 Tilman Benkert <thzs@gmx.net>
	SPDX-FileCopyrightText: 2012-2024 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "backend/worksheet/WorksheetElementContainer.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/trace.h"
#include "backend/worksheet/ResizeItem.h"
#include "backend/worksheet/WorksheetElementContainerPrivate.h"
#include "backend/worksheet/plots/cartesian/Plot.h"

#include <QGraphicsScene>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QPainter>

#include <KLocalizedString>

/**
 * \class WorksheetElementContainer
 * \brief Worksheet element container - parent of multiple elements.
 *
 * This class provides the functionality for a containers of multiple
 * worksheet elements. Such a container can be a plot or group of elements.
 *
 * \ingroup worksheet
 */

WorksheetElementContainer::WorksheetElementContainer(const QString& name, AspectType type)
	: WorksheetElement(name, new WorksheetElementContainerPrivate(this), type) {
	connect(this, &WorksheetElementContainer::childAspectAdded, this, &WorksheetElementContainer::handleAspectAdded);
	connect(this, &WorksheetElementContainer::childAspectMoved, this, &WorksheetElementContainer::handleAspectMoved);
}

WorksheetElementContainer::WorksheetElementContainer(const QString& name, WorksheetElementContainerPrivate* dd, AspectType type)
	: WorksheetElement(name, dd, type) {
	connect(this, &WorksheetElementContainer::childAspectAdded, this, &WorksheetElementContainer::handleAspectAdded);
	connect(this, &WorksheetElementContainer::childAspectMoved, this, &WorksheetElementContainer::handleAspectMoved);
}

// no need to delete the d-pointer here - it inherits from QGraphicsItem
// and is deleted during the cleanup in QGraphicsScene
WorksheetElementContainer::~WorksheetElementContainer() = default;

QRectF WorksheetElementContainer::rect() const {
	Q_D(const WorksheetElementContainer);
	return d->rect;
}

STD_SWAP_METHOD_SETTER_CMD_IMPL(WorksheetElementContainer, SetVisible, bool, swapVisible)
void WorksheetElementContainer::setVisible(bool on) {
	Q_D(WorksheetElementContainer);

	// take care of proper ordering on the undo-stack,
	// when making the container and all its children visible/invisible.
	// if visible is set true, change the visibility of the container first
	if (on) {
		beginMacro(i18n("%1: set visible", name()));
		exec(new WorksheetElementContainerSetVisibleCmd(d, on, ki18n("%1: set visible")));
	} else
		beginMacro(i18n("%1: set invisible", name()));

	// change the visibility of all children
	const auto& elements = children<Plot>(AbstractAspect::ChildIndexFlag::IncludeHidden | AbstractAspect::ChildIndexFlag::Compress);
	for (auto* elem : elements) {
		auto* plot = dynamic_cast<Plot*>(elem);
		if (plot) {
			// making curves invisible triggers the recalculation of plot ranges if auto-scale is active.
			// this should be avoided by supressing the retransformation in the curves.
			plot->setSuppressRetransform(true);
			elem->setVisible(on);
			plot->setSuppressRetransform(false);
		} else if (elem)
			elem->setVisible(on);
	}

	// if visible is set false, change the visibility of the container last
	if (!on)
		exec(new WorksheetElementContainerSetVisibleCmd(d, false, ki18n("%1: set invisible")));

	endMacro();
}

bool WorksheetElementContainer::isFullyVisible() const {
	const auto& elements = children<WorksheetElement>(AbstractAspect::ChildIndexFlag::IncludeHidden | AbstractAspect::ChildIndexFlag::Compress);
	for (const auto* elem : elements) {
		if (!elem->isVisible())
			return false;
	}
	return true;
}

void WorksheetElementContainer::setPrinting(bool on) {
	Q_D(WorksheetElementContainer);
	d->m_printing = on;
}

void WorksheetElementContainer::setResizeEnabled(bool enabled) {
	if (m_resizeItem)
		m_resizeItem->setVisible(enabled);
	else {
		if (enabled) {
			m_resizeItem = new ResizeItem(this);
			m_resizeItem->setRect(rect());
		}
	}
}

void WorksheetElementContainer::retransform() {
	if (isLoading())
		return;

	PERFTRACE(QLatin1String(Q_FUNC_INFO));
	Q_D(WorksheetElementContainer);

	// when retransforming every child here, don't emit the changed signal for every child,
	// emit it only once for the whole plot to avoid multiple refreshes of the worksheet preview.
	d->suppressChanged = true;

	const auto& elements = children<WorksheetElement>(AbstractAspect::ChildIndexFlag::IncludeHidden | AbstractAspect::ChildIndexFlag::Compress);
	for (auto* child : elements)
		child->retransform();

	d->recalcShapeAndBoundingRect();

	if (m_resizeItem)
		m_resizeItem->setRect(rect());

	d->suppressChanged = false;
	Q_EMIT changed();
}

/*!
 * called if the size of the worksheet page was changed and the content has to be adjusted/resized (\c pageResize = true)
 * or if a new rectangular for the element container was set (\c pageResize = false).
 * In the second case, \c WorksheetElement::handleResize() is called for every worksheet child to adjust the content to the new size.
 * In the first case, a new rectangular for the container is calculated and set first, which on the other hand, triggers the content adjustments
 * in the container children.
 */
void WorksheetElementContainer::handleResize(double horizontalRatio, double verticalRatio, bool pageResize) {
	DEBUG(Q_FUNC_INFO);
	Q_D(const WorksheetElementContainer);
	if (pageResize) {
		QRectF rect(d->rect);
		rect.setWidth(d->rect.width() * horizontalRatio);
		rect.setHeight(d->rect.height() * verticalRatio);
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
		connect(element, &WorksheetElement::changed, this, &WorksheetElementContainer::childChanged);
		element->graphicsItem()->setParentItem(d);

		qreal zVal = 0;
		for (auto* child : children<WorksheetElement>(ChildIndexFlag::IncludeHidden))
			child->setZValue(zVal++);
	}

	if (!isLoading())
		d->recalcShapeAndBoundingRect();
}

/*!
 * called when one of the children was moved, re-adjusts the Z-values for all children.
 */
void WorksheetElementContainer::handleAspectMoved() {
	qreal zVal = 0;
	const auto& children = this->children<WorksheetElement>(ChildIndexFlag::IncludeHidden);
	for (auto* child : children)
		child->setZValue(zVal++);
}

void WorksheetElementContainer::childHovered() {
	Q_D(WorksheetElementContainer);
	if (!d->isSelected()) {
		if (isHovered())
			setHover(false);
		else
			d->update();
	}
}

void WorksheetElementContainer::childUnhovered() {
	Q_D(WorksheetElementContainer);
	if (!d->isSelected())
		setHover(true);
}

void WorksheetElementContainer::childChanged() {
	Q_D(WorksheetElementContainer);
	if (d->suppressChanged)
		return;

	Q_EMIT changed();
}

void WorksheetElementContainer::prepareGeometryChange() {
	Q_D(WorksheetElementContainer);
	d->prepareGeometryChangeRequested();
}

// ################################################################
// ################### Private implementation ##########################
// ################################################################
WorksheetElementContainerPrivate::WorksheetElementContainerPrivate(WorksheetElementContainer* owner)
	: WorksheetElementPrivate(owner)
	, q(owner) {
	setAcceptHoverEvents(true);
}

void WorksheetElementContainerPrivate::contextMenuEvent(QGraphicsSceneContextMenuEvent* event) {
	scene()->clearSelection();
	setSelected(true);
	QMenu* menu = q->createContextMenu();
	menu->exec(event->screenPos());
}

void WorksheetElementContainerPrivate::prepareGeometryChangeRequested() {
	prepareGeometryChange(); // this is not const!
	recalcShapeAndBoundingRect();
}

void WorksheetElementContainerPrivate::recalcShapeAndBoundingRect() {
	// 	if (q->isLoading())
	// 		return;

	// old logic calculating the bounding box as the box covering all children.
	// we might need this logic later once we implement something like selection of multiple plots, etc.
	// 	boundingRectangle = QRectF();
	// 	QVector<WorksheetElement*> childList = q->children<WorksheetElement>(AbstractAspect::IncludeHidden | AbstractAspect::Compress);
	// 	foreach (const WorksheetElement* elem, childList)
	// 		boundingRectangle |= elem->graphicsItem()->mapRectToParent(elem->graphicsItem()->boundingRect());
	//
	qreal penWidth = 2.;
	m_boundingRectangle = q->rect();
	// QDEBUG(Q_FUNC_INFO << ", bound rect = " << boundingRectangle)
	m_boundingRectangle = QRectF(-m_boundingRectangle.width() / 2. - penWidth / 2.,
								 -m_boundingRectangle.height() / 2. - penWidth / 2.,
								 m_boundingRectangle.width() + penWidth,
								 m_boundingRectangle.height() + penWidth);

	QPainterPath path;
	path.addRect(m_boundingRectangle);

	// make the shape somewhat thicker than the hoveredPen to make the selection/hovering box more visible
	m_shape = QPainterPath();
	m_shape.addPath(WorksheetElement::shapeFromPath(path, QPen(QBrush(), penWidth)));
}

// Inherited from QGraphicsItem
QRectF WorksheetElementContainerPrivate::boundingRect() const {
	return m_boundingRectangle;
}

// Inherited from QGraphicsItem
void WorksheetElementContainerPrivate::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget*) {
	if (!isVisible())
		return;

	if (m_hovered && !isSelected() && !m_printing) {
		painter->setPen(QPen(QApplication::palette().color(QPalette::Shadow), 2, Qt::SolidLine));
		painter->drawPath(m_shape);
	}

	if (isSelected() && !m_printing) {
		painter->setPen(QPen(QApplication::palette().color(QPalette::Highlight), 2, Qt::SolidLine));
		painter->drawPath(m_shape);
	}
}

void WorksheetElementContainerPrivate::retransform() {
}
