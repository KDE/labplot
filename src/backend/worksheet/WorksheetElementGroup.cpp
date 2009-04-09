/***************************************************************************
    File                 : WorksheetElementGroup.cpp
    Project              : LabPlot/SciDAVis
    Description          : Generic AbstractWorksheetElement container
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

#include "worksheet/WorksheetElementGroup.h"
#include <QtGlobal>

/**
 * \class WorksheetElementGroup
 * \brief Generic AbstractWorksheetElement container.
 *
 * The role of this class is similar to object groups in a vector drawing program. 
 *
 */

WorksheetElementGroup::WorksheetElementGroup(const QString &name) 
	: AbstractWorksheetElement(name) {
// TODO
}

WorksheetElementGroup::~WorksheetElementGroup() {
// TODO
}

QList<QGraphicsItem *> WorksheetElementGroup::graphicsItems() const {
	QList<QGraphicsItem *> itemList;
	QList<AbstractWorksheetElement *> childList = children<AbstractWorksheetElement>(AbstractAspect::IncludeHidden | AbstractAspect::Compress);
	foreach(const AbstractWorksheetElement *elem, childList)
		itemList << elem->graphicsItems();
	return itemList;
}

void WorksheetElementGroup::setZValue(qreal z) {
	QList<AbstractWorksheetElement *> childList = children<AbstractWorksheetElement>(AbstractAspect::IncludeHidden | AbstractAspect::Compress);
	foreach(AbstractWorksheetElement *elem, childList)
		elem->setZValue(z);
}

qreal WorksheetElementGroup::zValue () const {
	return zValueMin();
}

/**
 * \brief Set the maximum and minimum Z value of the child elements keeping their drawing order.
 */
void WorksheetElementGroup::setZValueRange(qreal minZ, qreal maxZ) {
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
		WorksheetElementGroup *container = qobject_cast<WorksheetElementGroup *>(elem);
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
qreal WorksheetElementGroup::zValueMin() const {
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
qreal WorksheetElementGroup::zValueMax() const {
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

QRectF WorksheetElementGroup::boundingRect() const {
	// TODO
	return QRect(0.0, 0.0, 0.0, 0.0);
}

bool WorksheetElementGroup::contains(const QPointF &position) const {
}

void WorksheetElementGroup::setVisible(bool on) {
	QList<AbstractWorksheetElement *> childList = children<AbstractWorksheetElement>(AbstractAspect::IncludeHidden | AbstractAspect::Compress);
	foreach(AbstractWorksheetElement *elem, childList)
		elem->setVisible(on);
}

bool WorksheetElementGroup::isVisible() const {
	QList<AbstractWorksheetElement *> childList = children<AbstractWorksheetElement>(AbstractAspect::IncludeHidden | AbstractAspect::Compress);
	foreach(const AbstractWorksheetElement *elem, childList) {
		if (elem->isVisible()) 
			return true;
	}
	return false;
}

bool WorksheetElementGroup::isFullVisible() const {
	QList<AbstractWorksheetElement *> childList = children<AbstractWorksheetElement>(AbstractAspect::IncludeHidden | AbstractAspect::Compress);
	foreach(const AbstractWorksheetElement *elem, childList) {
		if (!elem->isVisible()) 
			return false;
	}
	return true;
}


void WorksheetElementGroup::transform(const AbstractCoordinateSystem &system) {
	QList<AbstractWorksheetElement *> childList = children<AbstractWorksheetElement>(AbstractAspect::IncludeHidden | AbstractAspect::Compress);
	foreach(AbstractWorksheetElement *elem, childList)
		elem->transform(system);
}




