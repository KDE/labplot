/***************************************************************************
    File                 : DropLineCurve.cpp
    Project              : LabPlot/SciDAVis
    Description          : A curve drawn as drop lines and/or symbols
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

/**
 * \class DropLineCurve
 * \brief A curve drawn as drop lines and/or symbols
 *
 * 
 */

#include "worksheet/DropLineCurve.h"
#include "worksheet/DropLineCurvePrivate.h"
#include "worksheet/AbstractCoordinateSystem.h"
#include "worksheet/CartesianCoordinateSystem.h"
#include "lib/commandtemplates.h"
#include "core/plugin/PluginManager.h"
#include "worksheet/symbols/EllipseCurveSymbol.h"
#include "worksheet/interfaces.h"

#include <QGraphicsItem>
#include <QGraphicsItemGroup>
#include <QGraphicsPathItem>
#include <QGraphicsEllipseItem>
#include <QPainterPath>
#include <QPainter>
#include <QtDebug>

DropLineCurvePrivate::DropLineCurvePrivate(DropLineCurve *owner)
	: LineSymbolCurvePrivate(owner) {

}

DropLineCurvePrivate::~DropLineCurvePrivate() {
}

DropLineCurve::DropLineCurve(const QString &name)
	: LineSymbolCurve(name, new DropLineCurvePrivate(this)) {
	Q_D(DropLineCurve);

	d->retransform();
}

DropLineCurve::DropLineCurve(const QString &name, DropLineCurvePrivate *dd)
		: LineSymbolCurve(name, dd) {
}

DropLineCurve::~DropLineCurve() {
}

void DropLineCurvePrivate::retransform() {
	const AbstractCoordinateSystem *cSystem = q->coordinateSystem();

	prepareGeometryChange();
	linePath = QPainterPath();
	curveShape = QPainterPath();
	boundingRectangle = QRect();
	symbolPoints.clear();

	if ( (NULL == xColumn) || (NULL == yColumn) )
		return;
	
	// TODO: add start row/end row attributes

	int startRow = 0;
	int endRow = xColumn->rowCount() - 1;

	QList<QLineF> lines;

	SciDAVis::ColumnMode xColMode = xColumn->columnMode();
	SciDAVis::ColumnMode yColMode = yColumn->columnMode();

	for (int row = startRow; row <= endRow; row++ ) {

		if ( xColumn->isValid(row) && yColumn->isValid(row) 
			&& (!xColumn->isMasked(row)) && (!yColumn->isMasked(row)) ) {
			QPointF tempPoint;

			switch(xColMode) {
				case SciDAVis::Numeric:
					tempPoint.setX(xColumn->valueAt(row));
					break;
				case SciDAVis::Text:
					//TODO
				case SciDAVis::DateTime:
				case SciDAVis::Month:
				case SciDAVis::Day:
					//TODO
					break;
				default:
					break;
			}

			switch(yColMode) {
				case SciDAVis::Numeric:
					tempPoint.setY(yColumn->valueAt(row));
					break;
				case SciDAVis::Text:
					//TODO
				case SciDAVis::DateTime:
				case SciDAVis::Month:
				case SciDAVis::Day:
					//TODO
					break;
				default:
					break;
			}
			symbolPoints.append(tempPoint);
		}
	}

	if (symbolPoints.count() > 0) {
		 QListIterator<QPointF> iter(symbolPoints);
		 while (iter.hasNext()) {
		 	QPointF point = iter.next();
			lines.append(QLineF(point, QPointF(point.x(), 0)));
		}
	}

	if (cSystem) {
		symbolPoints = cSystem->mapLogicalToScene(symbolPoints);
		lines = cSystem->mapLogicalToScene(lines);
	}

	foreach (QLineF line, lines) {
		linePath.moveTo(line.p1());
		linePath.lineTo(line.p2());
	}
	boundingRectangle = linePath.boundingRect();

	QRectF prototypeBoundingRect = symbolPrototype->boundingRect();
	QPainterPath symbolsPath;
	foreach (QPointF point, symbolPoints) {
		prototypeBoundingRect.moveCenter(point); 
		boundingRectangle |= prototypeBoundingRect;
		symbolsPath.addEllipse(prototypeBoundingRect);
	}

	boundingRectangle = boundingRectangle.normalized();

	curveShape = AbstractWorksheetElement::shapeFromPath(linePath, linePen);
	curveShape.addPath(AbstractWorksheetElement::shapeFromPath(symbolsPath, symbolsPen));
}

