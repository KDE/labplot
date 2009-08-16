/***************************************************************************
    File                 : LogAxis.cpp
    Project              : LabPlot/SciDAVis
    Description          : Logarithmic axis for cartesian coordinate systems.
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

#include "worksheet/LogAxis.h"
#include "worksheet/LogAxisPrivate.h"
#include "worksheet/AbstractCoordinateSystem.h"
#include "worksheet/CartesianCoordinateSystem.h"
#include "lib/commandtemplates.h"
#include <QGraphicsItem>
#include <QGraphicsItemGroup>
#include <QGraphicsLineItem>
#include <QBrush>
#include <QPen>
#include <QPainter>
#include <cmath>
#include <QtDebug>
#include <QLocale>

/**
 * \class LogAxis
 * \brief Logarithmic axis for cartesian coordinate systems.
 *
 *  
 */

LogAxis::LogAxis(const QString &name, const AxisOrientation &orientation, double base)
		: LinearAxis(name, orientation, new LogAxisPrivate(this)) {
	init(base);
}
		
LogAxis::LogAxis(const QString &name, const AxisOrientation &orientation, double base, LogAxisPrivate *dd)
		: LinearAxis(name, orientation, dd) {
	init(base);
}

void LogAxis::init(double base) {
	Q_D(LogAxis);
	d->base = base;
	d->start = 1;
	d->end = 10;
	d->tickStart = 1;
	d->tickEnd = 10;
	d->majorTickCount = 2;
	d->minorTickCount = 1;

	retransform();
}

LogAxis::~LogAxis() {
}

void LogAxis::Private::retransformTicks(const AbstractCoordinateSystem *cSystem) {
	const CartesianCoordinateSystem *cCSystem = qobject_cast<const CartesianCoordinateSystem *>(cSystem);

	majorTicksPath = QPainterPath();
	minorTicksPath = QPainterPath();
	axisShape = QPainterPath();
	boundingRectangle = QRect();
	qDeleteAll(labels);
	labels.clear();

	int xDirection = 1;
	int yDirection = 1;
	if (cCSystem) {
		xDirection = cCSystem->xDirection();
		yDirection = cCSystem->yDirection();
	}

	const double majorTickSpacing = majorTickCount > 1 ? (log(tickEnd) - log(tickStart))/log(base) / ((qreal)(majorTickCount - 1)) : 0;
	for (int iMajor = 0; iMajor < majorTickCount; iMajor++) {
		QPointF anchorPoint;
		QPointF startPoint;
		QPointF endPoint;
		QPointF textPos;
		
		qreal majorTickPos = pow(base, log(tickStart)/log(base) + majorTickSpacing * (qreal)iMajor);
		if (LinearAxis::noTicks != majorTicksDirection) {
			if (orientation & LinearAxis::axisHorizontal) {
				anchorPoint.setX(majorTickPos);
				anchorPoint.setY(offset);

				if (transformAnchor(cSystem, &anchorPoint)) {
					if (orientation & LinearAxis::axisNormalTicks) {
						startPoint = anchorPoint + QPointF(0, (majorTicksDirection & LinearAxis::ticksIn)  ? yDirection * majorTicksLength  : 0);
						endPoint   = anchorPoint + QPointF(0, (majorTicksDirection & LinearAxis::ticksOut) ? -yDirection * majorTicksLength : 0);
						textPos = endPoint;
					} else {
						startPoint = anchorPoint + QPointF(0, (majorTicksDirection & LinearAxis::ticksOut) ? yDirection * majorTicksLength  : 0);
						endPoint   = anchorPoint + QPointF(0, (majorTicksDirection & LinearAxis::ticksIn)  ? -yDirection * majorTicksLength : 0);
						textPos = startPoint;
					}
					majorTicksPath.moveTo(startPoint);
					majorTicksPath.lineTo(endPoint);
					addTextLabel(QLocale().toString(majorTickPos, numericFormat, displayedDigits), textPos);
				}
			} else { // vertical
				anchorPoint.setY(majorTickPos);
				anchorPoint.setX(offset);

				if (transformAnchor(cSystem, &anchorPoint)) {
					if (orientation & LinearAxis::axisNormalTicks) {
						startPoint = anchorPoint + QPointF((majorTicksDirection & LinearAxis::ticksIn)  ? xDirection * majorTicksLength  : 0, 0);
						endPoint   = anchorPoint + QPointF((majorTicksDirection & LinearAxis::ticksOut) ? -xDirection * majorTicksLength : 0, 0);
						textPos = endPoint;
					} else {
						startPoint = anchorPoint + QPointF((majorTicksDirection & LinearAxis::ticksOut) ? xDirection * majorTicksLength  : 0, 0);
						endPoint   = anchorPoint + QPointF((majorTicksDirection & LinearAxis::ticksIn)  ? -xDirection * majorTicksLength : 0, 0);
						textPos = startPoint;
					}
					majorTicksPath.moveTo(startPoint);
					majorTicksPath.lineTo(endPoint);
					addTextLabel(QLocale().toString(majorTickPos, numericFormat, displayedDigits), textPos);
				}
			}
		}

		if ((LinearAxis::noTicks != minorTicksDirection) && (majorTickCount > 1) && (minorTickCount > 0) && (iMajor < majorTickCount - 1)) {
			for (int iMinor = 0; iMinor < minorTickCount; iMinor++) {
				qreal nextMajorTickPos = tickStart + pow(base, majorTickSpacing * (qreal)(iMajor + 1));
				qreal minorTickPos = majorTickPos + (qreal)(iMinor + 1) * (nextMajorTickPos - majorTickPos) / (qreal)(minorTickCount + 1);
				if (orientation & LinearAxis::axisHorizontal) {
					anchorPoint.setX(minorTickPos);
					anchorPoint.setY(offset);

					if (transformAnchor(cSystem, &anchorPoint)) {
						if (orientation & LinearAxis::axisNormalTicks) {
							startPoint = anchorPoint + QPointF(0, (minorTicksDirection & LinearAxis::ticksIn)  ? yDirection * minorTicksLength  : 0);
							endPoint   = anchorPoint + QPointF(0, (minorTicksDirection & LinearAxis::ticksOut) ? -yDirection * minorTicksLength : 0);
						} else {
							startPoint = anchorPoint + QPointF(0, (minorTicksDirection & LinearAxis::ticksOut) ? yDirection * minorTicksLength  : 0);
							endPoint   = anchorPoint + QPointF(0, (minorTicksDirection & LinearAxis::ticksIn)  ? -yDirection * minorTicksLength : 0);
						}
						minorTicksPath.moveTo(startPoint);
						minorTicksPath.lineTo(endPoint);
					}
				} else { // vertical
					anchorPoint.setY(minorTickPos);
					anchorPoint.setX(offset);

					if (transformAnchor(cSystem, &anchorPoint)) {
						if (orientation & LinearAxis::axisNormalTicks) {
							startPoint = anchorPoint + QPointF((minorTicksDirection & LinearAxis::ticksIn)  ? xDirection * minorTicksLength  : 0, 0);
							endPoint   = anchorPoint + QPointF((minorTicksDirection & LinearAxis::ticksOut) ? -xDirection * minorTicksLength : 0, 0);
						} else {
							startPoint = anchorPoint + QPointF((minorTicksDirection & LinearAxis::ticksOut) ? xDirection * minorTicksLength  : 0, 0);
							endPoint   = anchorPoint + QPointF((minorTicksDirection & LinearAxis::ticksIn)  ? -xDirection * minorTicksLength : 0, 0);
						}
						minorTicksPath.moveTo(startPoint);
						minorTicksPath.lineTo(endPoint);
					}
				}
			}
		}
	}

	restyleLabels(); // this calls recalcShapeAndBoundingRect()
}

