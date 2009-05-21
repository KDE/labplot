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

/**
 * \class WorksheetElementContainer
 * \brief Generic AbstractWorksheetElement container.
 *
 * This class combines functionality of worksheet element containers
 * such as groups, plots, coordinate systems, etc.
 *
 */

WorksheetElementContainerPrivate::WorksheetElementContainerPrivate(WorksheetElementContainer *owner)
	: q(owner) {
}

WorksheetElementContainerPrivate::~WorksheetElementContainerPrivate() {
}

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

void WorksheetElementContainer::setZValue(qreal z) {
	QList<AbstractWorksheetElement *> childList = children<AbstractWorksheetElement>(AbstractAspect::IncludeHidden | AbstractAspect::Compress);
	foreach(AbstractWorksheetElement *elem, childList)
		elem->setZValue(z);
}

qreal WorksheetElementContainer::zValue () const {
	return zValueMin();
}

/**
 * \brief Set the maximum and minimum Z value of the child elements keeping their drawing order.
 */
void WorksheetElementContainer::setZValueRange(qreal minZ, qreal maxZ) {
	if (maxZ < minZ)
		qSwap(minZ, maxZ);
	qreal oldMinZ = zValueMin();
	qreal oldMaxZ = zValueMax();
	qreal oldZDiff = oldMaxZ - oldMinZ;
	qreal zDiff = maxZ - minZ;
	if (0.0 == oldZDiff) // avoid division by zero
		setZValue(minZ);

	QList<AbstractWorksheetElement *> childList = children<AbstractWorksheetElement>(AbstractAspect::IncludeHidden | AbstractAspect::Compress);
	foreach(AbstractWorksheetElement *elem, childList) {
		WorksheetElementContainer *container = qobject_cast<WorksheetElementContainer *>(elem);
		if (container) {
			qreal containerMinZ = minZ + (container->zValueMin() - oldMinZ) / oldZDiff * zDiff;
			qreal containerMaxZ = minZ + (container->zValueMax() - oldMinZ) / oldZDiff * zDiff;
			container->setZValueRange(containerMinZ, containerMaxZ);
		} else {
			elem->setZValue(minZ + (elem->zValue() - oldMinZ) / oldZDiff * zDiff);
		}
	}
}

/**
 * \brief Return the minmum Z value of all child element Z values.
 */
qreal WorksheetElementContainer::zValueMin() const {
	qreal min = 0.0;
	QList<AbstractWorksheetElement *> childList = children<AbstractWorksheetElement>(AbstractAspect::IncludeHidden | AbstractAspect::Compress);
	if (!childList.isEmpty()) {
		min = childList.at(0)->zValue();
		foreach(const AbstractWorksheetElement *elem, childList) {
			if (elem->zValue() < min) 
				min = elem->zValue();
		}
	}
	return min;
}

/**
 * \brief Return the maximum Z value of all child element Z values.
 */
qreal WorksheetElementContainer::zValueMax() const {
	qreal max = 0.0;
	QList<AbstractWorksheetElement *> childList = children<AbstractWorksheetElement>(AbstractAspect::IncludeHidden | AbstractAspect::Compress);
	if (!childList.isEmpty()) {
		max = childList.at(0)->zValue();
		foreach(const AbstractWorksheetElement *elem, childList) {
			if (elem->zValue() > max) 
				max = elem->zValue();
		}
	}
	return max;
}

QRectF WorksheetElementContainerPrivate::boundingRect() const {
	QRectF rect;
	QList<AbstractWorksheetElement *> childList = q->children<AbstractWorksheetElement>(AbstractAspect::IncludeHidden | AbstractAspect::Compress);
	foreach(const AbstractWorksheetElement *elem, childList)
		rect |= elem->graphicsItem()->boundingRect();
	return rect;
}
    
void WorksheetElementContainerPrivate::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
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

void WorksheetElementContainer::handleAspectAdded(const AbstractAspect *aspect) {
	Q_D(WorksheetElementContainer);

	const AbstractWorksheetElement *elem = qobject_cast<const AbstractWorksheetElement*>(aspect);
	if (elem && (elem->parentAspect() == this)) {
		QGraphicsItem *item = elem->graphicsItem();
		if (item)
			item->setParentItem(d);			
	}
}

void WorksheetElementContainer::handleAspectAboutToBeRemoved(const AbstractAspect *aspect) {
	const AbstractWorksheetElement *elem = qobject_cast<const AbstractWorksheetElement*>(aspect);
	if (elem && (elem->parentAspect() == this)) {
		QGraphicsItem *item = elem->graphicsItem();
		if (item)
			item->setParentItem(NULL);			
	}
}


