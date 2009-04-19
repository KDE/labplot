/***************************************************************************
    File                 : LinearAxis.cpp
    Project              : LabPlot/SciDAVis
    Description          : Linear axis for cartesian coordinate systems.
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

#include "worksheet/LinearAxis.h"
#include "worksheet/AbstractCoordinateSystem.h"
#include "worksheet/CartesianCoordinateSystem.h"
#include "lib/commandtemplates.h"
#include <QGraphicsItem>
#include <QGraphicsItemGroup>
#include <QGraphicsLineItem>

/**
 * \class LinearAxis
 * \brief Linear axis for cartesian coordinate systems.
 *
 *  
 */

// TODO: decide whether it makes sense to move some of the functionality into a class AbstractAxis

class LinearAxis::Private {
	public:
		Private(LinearAxis *owner) : q(owner) {
		}

		~Private() {
		}

		QString name() const {
			return q->name();
		}

		AxisOrientation orientation; //!< left, right, bottom, or top (usually not changed after creation)
		qreal offset; //!< offset from zero in the directin perpendicular to the axis
		qreal start; //!< start coordinate of the axis line
		qreal end; //!< end coordinate of the axis line
		qreal tickStart; //!< coordinate of the first tick (typically ==0 or ==start)
		qreal tickEnd; //!< coordinate of the last tick (typically ==end)
		int majorTickCount; //!< number of major ticks
		int minorTickCount; //!< number of minor ticks (between each two major ticks)
		qreal majorTicksLength; //!< major tick length (in page units!)
		qreal minorTicksLength; //!< minor tick length (in page units!)
		TicksDirection majorTicksDirection; //!< major ticks direction: inwards, outwards, both, or none
		TicksDirection minorTicksDirection; //!< minor ticks direction: inwards, outwards, both, or none

		mutable QGraphicsItemGroup itemGroup;
		mutable QGraphicsLineItem *axisLineItem;
		mutable QList<QGraphicsLineItem *> majorTickItems;
		mutable QList<QGraphicsLineItem *> minorTickItems;

		void retransform() const;
		void retransformTicks() const;
		void retransformTicks(const AbstractCoordinateSystem *cSystem) const;
		qreal setZValue(const qreal &z);
		bool setVisible(const bool &on);

		LinearAxis * const q;
};

LinearAxis::LinearAxis(const QString &name, const AxisOrientation &orientation)
		: AbstractWorksheetElement(name), d(new Private(this)) {
	d->orientation = orientation;
	d->offset = 0;
	d->start = 0;
	d->end = 10;
	d->tickStart = 0;
	d->tickEnd = 10;
	d->majorTickCount = 10;
	d->minorTickCount = 1;
	d->majorTicksLength = 0.5;
	d->minorTicksLength = 0.25;
	d->majorTicksDirection = ticksOut;
	d->minorTicksDirection = ticksOut;
	d->axisLineItem = new QGraphicsLineItem();
	d->itemGroup.addToGroup(d->axisLineItem);
	retransform();
}

LinearAxis::~LinearAxis() {
	delete d;
}

/* ============================ accessor documentation ================= */
/**
   \fn LinearAxis::CLASS_D_ACCESSOR_DECL(AxisOrientation, orientation, Orientation);
   \brief Get/set the axis orientation: left, right, bottom, or top (usually not changed after creation).

   The orientation has not much to do with the actual position of the axis, which
   is determined by LinearAxis::Private::offset. It only determines whether the axis
   is horizontal or vertical and which direction means in/out for the ticks.
 */
/**
   \fn LinearAxis::BASIC_D_ACCESSOR_DECL(qreal, offset, Offset);
   \brief Get/set the offset from zero in the directin perpendicular to the axis.
 */
/**
   \fn LinearAxis::BASIC_D_ACCESSOR_DECL(qreal, start, Start);
   \brief Get/set the start coordinate of the axis line.
 */
/**
   \fn LinearAxis::BASIC_D_ACCESSOR_DECL(qreal, end, End);
   \brief Get/set the end coordinate of the axis line.
 */
/**
   \fn LinearAxis::BASIC_D_ACCESSOR_DECL(qreal, tickStart, TickStart);
   \brief Get/set the coordinate of the first tick (typically ==0 or ==start).
 */
/**
   \fn LinearAxis::BASIC_D_ACCESSOR_DECL(qreal, tickEnd, TickEnd);
   \brief Get/set the coordinate of the last tick (typically ==end).
 */
/**
   \fn LinearAxis::BASIC_D_ACCESSOR_DECL(int, majorTickCount, MajorTickCount);
   \brief Get/set the number of major ticks.
 */
/**
   \fn LinearAxis::BASIC_D_ACCESSOR_DECL(int, minorTickCount, MinorTickCount);
   \brief Get/set the number of minor ticks (between each two major ticks).
 */
/**
   \fn LinearAxis::BASIC_D_ACCESSOR_DECL(qreal, majorTicksLength, MajorTicksLength);
   \brief Get/set the major tick length (in page units!).
 */
/**
   \fn LinearAxis::BASIC_D_ACCESSOR_DECL(qreal, minorTicksLength, MinorTicksLength);
   \brief Get/set the minor tick length (in page units!).
 */
/**
   \fn LinearAxis::CLASS_D_ACCESSOR_DECL(TicksDirection, majorTicksDirection, MajorTicksDirection);
   \brief Get/set the major ticks direction: inwards, outwards, both, or none.
 */
/**
   \fn LinearAxis::CLASS_D_ACCESSOR_DECL(TicksDirection, minorTicksDirection, MinorTicksDirection);
   \brief Get/set the minor ticks direction: inwards, outwards, both, or none.
 */

/* ============================ getter methods ================= */
CLASS_D_READER_IMPL(LinearAxis, LinearAxis::AxisOrientation, orientation, orientation);
BASIC_D_READER_IMPL(LinearAxis, qreal, offset, offset);
BASIC_D_READER_IMPL(LinearAxis, qreal, start, start);
BASIC_D_READER_IMPL(LinearAxis, qreal, end, end);
BASIC_D_READER_IMPL(LinearAxis, qreal, tickStart, tickStart);
BASIC_D_READER_IMPL(LinearAxis, qreal, tickEnd, tickEnd);
BASIC_D_READER_IMPL(LinearAxis, int, majorTickCount, majorTickCount);
BASIC_D_READER_IMPL(LinearAxis, int, minorTickCount, minorTickCount);
BASIC_D_READER_IMPL(LinearAxis, qreal, majorTicksLength, majorTicksLength);
BASIC_D_READER_IMPL(LinearAxis, qreal, minorTicksLength, minorTicksLength);
CLASS_D_READER_IMPL(LinearAxis, LinearAxis::TicksDirection, majorTicksDirection, majorTicksDirection);
CLASS_D_READER_IMPL(LinearAxis, LinearAxis::TicksDirection, minorTicksDirection, minorTicksDirection);

/* ============================ setter methods and undo commands ================= */

STD_SETTER_CMD_IMPL_F(LinearAxis, SetOrientation, LinearAxis::AxisOrientation, orientation, retransform);
void LinearAxis::setOrientation(const AxisOrientation &orientation) {
	if (orientation != d->orientation)
		exec(new LinearAxisSetOrientationCmd(d, orientation, tr("%1: set axis orientation")));
}

STD_SETTER_CMD_IMPL_F(LinearAxis, SetOffset, qreal, offset, retransform);
void LinearAxis::setOffset(qreal offset) {
	if (offset != d->offset)
		exec(new LinearAxisSetOffsetCmd(d, offset, tr("%1: set axis offset")));
}

STD_SETTER_CMD_IMPL_F(LinearAxis, SetStart, qreal, start, retransform);
void LinearAxis::setStart(qreal start) {
	if (start != d->start)
		exec(new LinearAxisSetStartCmd(d, start, tr("%1: set axis start")));
}

STD_SETTER_CMD_IMPL_F(LinearAxis, SetEnd, qreal, end, retransform);
void LinearAxis::setEnd(qreal end) {
	if (end != d->end)
		exec(new LinearAxisSetEndCmd(d, end, tr("%1: set axis end")));
}

STD_SETTER_CMD_IMPL_F(LinearAxis, SetTickStart, qreal, tickStart, retransformTicks);
void LinearAxis::setTickStart(qreal tickStart) {
	if (tickStart != d->tickStart)
		exec(new LinearAxisSetTickStartCmd(d, tickStart, tr("%1: set first tick")));
}

STD_SETTER_CMD_IMPL_F(LinearAxis, SetTickEnd, qreal, tickEnd, retransformTicks);
void LinearAxis::setTickEnd(qreal tickEnd) {
	if (tickEnd != d->tickEnd)
		exec(new LinearAxisSetTickEndCmd(d, tickEnd, tr("%1: set axis end")));
}

STD_SETTER_CMD_IMPL_F(LinearAxis, SetMajorTickCount, int, majorTickCount, retransformTicks);
void LinearAxis::setMajorTickCount(int majorTickCount) {
	if (majorTickCount != d->majorTickCount)
		exec(new LinearAxisSetMajorTickCountCmd(d, majorTickCount, tr("%1: set major tick count")));
}

STD_SETTER_CMD_IMPL_F(LinearAxis, SetMinorTickCount, int, minorTickCount, retransformTicks);
void LinearAxis::setMinorTickCount(int minorTickCount) {
	if (minorTickCount != d->minorTickCount)
		exec(new LinearAxisSetMinorTickCountCmd(d, minorTickCount, tr("%1: set minor tick count")));
}

STD_SETTER_CMD_IMPL_F(LinearAxis, SetMajorTicksLength, qreal, majorTicksLength, retransformTicks);
void LinearAxis::setMajorTicksLength(qreal majorTicksLength) {
	if (majorTicksLength != d->majorTicksLength)
		exec(new LinearAxisSetMajorTicksLengthCmd(d, majorTicksLength, tr("%1: set major ticks length")));
}

STD_SETTER_CMD_IMPL_F(LinearAxis, SetMinorTicksLength, qreal, minorTicksLength, retransformTicks);
void LinearAxis::setMinorTicksLength(qreal minorTicksLength) {
	if (minorTicksLength != d->minorTicksLength)
		exec(new LinearAxisSetMinorTicksLengthCmd(d, minorTicksLength, tr("%1: set minor ticks length")));
}

STD_SETTER_CMD_IMPL_F(LinearAxis, SetMajorTicksDirection, LinearAxis::TicksDirection, majorTicksDirection, retransformTicks);
void LinearAxis::setMajorTicksDirection(const TicksDirection &majorTicksDirection) {
	if (majorTicksDirection != d->majorTicksDirection)
		exec(new LinearAxisSetMajorTicksDirectionCmd(d, majorTicksDirection, tr("%1: set major ticks direction")));
}

STD_SETTER_CMD_IMPL_F(LinearAxis, SetMinorTicksDirection, LinearAxis::TicksDirection, minorTicksDirection, retransformTicks);
void LinearAxis::setMinorTicksDirection(const TicksDirection &minorTicksDirection) {
	if (minorTicksDirection != d->minorTicksDirection)
		exec(new LinearAxisSetMinorTicksDirectionCmd(d, minorTicksDirection, tr("%1: set minor ticks direction")));
}

/* ============================ other methods ================= */

QList<QGraphicsItem *> LinearAxis::graphicsItems() const {
	return QList<QGraphicsItem *>() << &(d->itemGroup);
}

qreal LinearAxis::Private::setZValue(const qreal &z) {
	qreal oldZ = itemGroup.zValue();
	itemGroup.setZValue(z);
	foreach(QGraphicsLineItem *item, majorTickItems)
		item->setZValue(z);
	foreach(QGraphicsLineItem *item, minorTickItems)
		item->setZValue(z);
	return oldZ;
}

STD_SWAP_METHOD_SETTER_CMD_IMPL(LinearAxis, SetZ, qreal, setZValue);
void LinearAxis::setZValue(qreal z) {
	if (zValue() != z)
		exec(new LinearAxisSetZCmd(d, z, tr("%1: set z value")));
}

qreal LinearAxis::zValue () const {
	return d->itemGroup.zValue();;
}

QRectF LinearAxis::boundingRect() const {
	return d->itemGroup.boundingRect();
}

bool LinearAxis::contains(const QPointF &position) const {
	// TODO
	return false;
}

bool LinearAxis::Private::setVisible(const bool &on) {
	bool oldValue = itemGroup.isVisible();
	itemGroup.setVisible(on);
	return oldValue;
}

STD_SWAP_METHOD_SETTER_CMD_IMPL(LinearAxis, SetVisible, bool, setVisible);
void LinearAxis::setVisible(bool on) {
	exec(new LinearAxisSetVisibleCmd(d, on, on ? tr("%1: set visible") : tr("%1: set invisible")));
}

bool LinearAxis::isVisible() const {
	return d->itemGroup.isVisible();
}

void LinearAxis::retransform() const {
	d->retransform();
}

void LinearAxis::Private::retransform() const {
	const AbstractCoordinateSystem *cSystem = q->coordinateSystem();

	QPointF startPoint;
	QPointF endPoint;

	if (orientation & LinearAxis::axisHorizontal) {
		startPoint.setX(start);
		startPoint.setY(offset);
		endPoint.setX(end);
		endPoint.setY(offset);
	} else { // vertical
		startPoint.setX(offset);
		startPoint.setY(start);
		endPoint.setX(offset);
		endPoint.setY(end);
	}
	if (cSystem) {
		startPoint = cSystem->mapLogicalToScene(startPoint);
		endPoint = cSystem->mapLogicalToScene(endPoint);
	} 
	axisLineItem->setLine(QLineF(startPoint, endPoint));

	retransformTicks(cSystem);
}

void LinearAxis::Private::retransformTicks() const {
	retransformTicks(q->coordinateSystem());
}

void LinearAxis::Private::retransformTicks(const AbstractCoordinateSystem *cSystem) const {
	if (noTicks == majorTicksDirection) {
		foreach(QGraphicsLineItem *item, majorTickItems)
			itemGroup.removeFromGroup(item);
		while (majorTickItems.size() > majorTickCount)
			delete majorTickItems.takeLast();
	} else {
		while (majorTickItems.size() > majorTickCount) {
			QGraphicsLineItem *item = majorTickItems.takeLast();
			itemGroup.removeFromGroup(item);
			delete item;
		}
	}

	if (noTicks == minorTicksDirection || majorTickCount <= 1) {
		foreach(QGraphicsLineItem *item, minorTickItems)
			itemGroup.removeFromGroup(item);
		while (minorTickItems.size() > minorTickCount)
			delete minorTickItems.takeLast();
	} else {
		while (minorTickItems.size() > minorTickCount) {
			QGraphicsLineItem *item = minorTickItems.takeLast();
			itemGroup.removeFromGroup(item);
			delete item;
		}
	}

	int xDirection = 1;
	int yDirection = 1;
	const CartesianCoordinateSystem *cCSystem = qobject_cast<const CartesianCoordinateSystem *>(cSystem);
	if (cCSystem) {
		xDirection = cCSystem->xDirection();
		yDirection = cCSystem->yDirection();
	}

	for (int iMajor = 0; iMajor < majorTickCount; iMajor++) {
		QPointF startPoint;
		QPointF endPoint;
		
		qreal majorTickPos = tickStart + (majorTickCount > 1 ? (tickEnd - tickStart) * (qreal)iMajor / ((qreal)(majorTickCount - 1)) : 0);
		if (noTicks != majorTicksDirection) {
			if (orientation & LinearAxis::axisHorizontal) {
				startPoint.setX(majorTickPos);
				endPoint.setX(majorTickPos);
				startPoint.setY(offset);
				endPoint.setY(offset);

				if (cSystem) {
					startPoint = cSystem->mapLogicalToScene(startPoint);
					endPoint = cSystem->mapLogicalToScene(endPoint);
				}

				if (orientation & axisNormalTicks) {
					startPoint += QPointF(0, (majorTicksDirection & ticksIn) ? yDirection * majorTicksLength : 0);
					endPoint += QPointF(0, (majorTicksDirection & ticksOut) ? -yDirection * majorTicksLength : 0);
				} else {
					startPoint += QPointF(0, (majorTicksDirection & ticksOut) ? yDirection * majorTicksLength : 0);
					endPoint += QPointF(0, (majorTicksDirection & ticksIn) ? -yDirection * majorTicksLength : 0);
				}
			} else { // vertical
				startPoint.setY(majorTickPos);
				endPoint.setY(majorTickPos);
				startPoint.setX(offset);
				endPoint.setX(offset);

				if (cSystem) {
					startPoint = cSystem->mapLogicalToScene(startPoint);
					endPoint = cSystem->mapLogicalToScene(endPoint);
				}

				if (orientation & axisNormalTicks) {
					startPoint += QPointF((majorTicksDirection & ticksIn) ? xDirection * majorTicksLength : 0, 0);
					endPoint += QPointF((majorTicksDirection & ticksOut) ? -xDirection * majorTicksLength : 0, 0);
				} else {
					startPoint += QPointF((majorTicksDirection & ticksOut) ? xDirection * majorTicksLength : 0, 0);
					endPoint += QPointF((majorTicksDirection & ticksIn) ? -xDirection * majorTicksLength : 0, 0);
				}
			}

			if (majorTickItems.size() <= iMajor) {
				QGraphicsLineItem *majorTick = new QGraphicsLineItem(QLineF(startPoint, endPoint));
				majorTick->setZValue(itemGroup.zValue());
				majorTickItems.append(majorTick);
				itemGroup.addToGroup(majorTick);
			} else
				majorTickItems.at(iMajor)->setLine(QLineF(startPoint, endPoint));
		}

		if ((noTicks != minorTicksDirection) && (majorTickCount > 1) && (minorTickCount > 0) && (iMajor < majorTickCount - 1)) {
			for (int iMinor = 0; iMinor < minorTickCount; iMinor++) {
				qreal minorTickPos = majorTickPos + (qreal)(iMinor + 1) * (tickEnd - tickStart) \
						/ (qreal)(majorTickCount - 1) / (qreal)(minorTickCount + 1);
				if (orientation & LinearAxis::axisHorizontal) {
					startPoint.setX(minorTickPos);
					endPoint.setX(minorTickPos);
					startPoint.setY(offset);
					endPoint.setY(offset);

					if (cSystem) {
						startPoint = cSystem->mapLogicalToScene(startPoint);
						endPoint = cSystem->mapLogicalToScene(endPoint);
					}

					if (orientation & axisNormalTicks) {
						startPoint += QPointF(0, (minorTicksDirection & ticksIn) ? yDirection * minorTicksLength : 0);
						endPoint += QPointF(0, (minorTicksDirection & ticksOut) ? -yDirection * minorTicksLength : 0);
					} else {
						startPoint += QPointF(0, (minorTicksDirection & ticksOut) ? yDirection * minorTicksLength : 0);
						endPoint += QPointF(0, (minorTicksDirection & ticksIn) ? -yDirection * minorTicksLength : 0);
					}
				} else { // vertical
					startPoint.setY(minorTickPos);
					endPoint.setY(minorTickPos);
					startPoint.setX(offset);
					endPoint.setX(offset);

					if (cSystem) {
						startPoint = cSystem->mapLogicalToScene(startPoint);
						endPoint = cSystem->mapLogicalToScene(endPoint);
					}

					if (orientation & axisNormalTicks) {
						startPoint += QPointF((minorTicksDirection & ticksIn) ? xDirection * minorTicksLength : 0, 0);
						endPoint += QPointF((minorTicksDirection & ticksOut) ? -xDirection * minorTicksLength : 0, 0);
					} else {
						startPoint += QPointF((minorTicksDirection & ticksOut) ? xDirection * minorTicksLength : 0, 0);
						endPoint += QPointF((minorTicksDirection & ticksIn) ? -xDirection * minorTicksLength : 0, 0);
					}
				}

				if (minorTickItems.size() <= (iMajor * minorTickCount + iMinor)) {
					QGraphicsLineItem *minorTick = new QGraphicsLineItem(QLineF(startPoint, endPoint));
					minorTick->setZValue(itemGroup.zValue());
					minorTickItems.append(minorTick);
					itemGroup.addToGroup(minorTick);
				} else
					minorTickItems.at(iMajor * minorTickCount + iMinor)->setLine(QLineF(startPoint, endPoint));
			}
		}
	}
}



