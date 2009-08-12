/***************************************************************************
    File                 : LineSymbolCurve.cpp
    Project              : LabPlot/SciDAVis
    Description          : A curve drawn as line and/or symbols
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
 * \class LineSymbolCurve
 * \brief A curve drawn as line and/or symbols
 *
 * 
 */

#include "worksheet/LineSymbolCurve.h"
#include "worksheet/LineSymbolCurvePrivate.h"
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

LineSymbolCurvePrivate::LineSymbolCurvePrivate(LineSymbolCurve *owner): q(owner) {
	lineVisible = true;
	symbolsVisible = true;
	symbolTypeId = "diamond";
	xColumn = NULL;
	yColumn = NULL;
	symbolRotationAngle = 0;
	symbolSize = 1;
	symbolAspectRatio = 1;

	symbolPrototype = NULL;
	if (symbolTypeId != "ellipse") {
		foreach(QObject *plugin, PluginManager::plugins()) {
			CurveSymbolFactory *factory = qobject_cast<CurveSymbolFactory *>(plugin);
			if (factory) {
				const AbstractCurveSymbol *prototype = factory->prototype(symbolTypeId);
				if (prototype)
				{
					symbolPrototype = prototype->clone();
					break;
				}
			}
		}
	}
	if (!symbolPrototype) // safety fallback
		symbolPrototype = EllipseCurveSymbol::staticPrototype()->clone();

	// TODO: remove this temporary code later
	symbolSize = 2.5;
	symbolPrototype->setBrush(QBrush(Qt::red));

	symbolPrototype->setSize(symbolSize);
	symbolPrototype->setAspectRatio(symbolAspectRatio);
//TODO	symbolPrototype->setBrush(symbolsBrush);
//TODO	symbolPrototype->setPen(symbolsPen);
}

LineSymbolCurvePrivate::~LineSymbolCurvePrivate() {
}

LineSymbolCurve::LineSymbolCurve(const QString &name)
		: AbstractWorksheetElement(name), d_ptr(new LineSymbolCurvePrivate(this)) {

	d_ptr->retransform();
}

LineSymbolCurve::LineSymbolCurve(const QString &name, LineSymbolCurvePrivate *dd)
		: AbstractWorksheetElement(name), d_ptr(dd) {
}

LineSymbolCurve::~LineSymbolCurve() {
	delete d_ptr;
}

/* ============================ accessor documentation ================= */
/**
  \fn   LineSymbolCurve::BASIC_D_ACCESSOR_DECL(bool, lineVisible, LineVisible);
  \brief Set/get whether the line is visible/invisible.
*/
/**
  \fn   LineSymbolCurve::BASIC_D_ACCESSOR_DECL(bool, symbolsVisible, SymbolsVisible);
  \brief Set/get whether the symbols are visible/invisible.
*/
/**
  \fn   LineSymbolCurve::POINTER_D_ACCESSOR_DECL(const AbstractColumn, xColumn, XColumn);
  \brief Set/get the pointer to the X column.
*/
/**
  \fn   LineSymbolCurve::POINTER_D_ACCESSOR_DECL(const AbstractColumn, yColumn, YColumn);
  \brief Set/get the pointer to the Y column.
*/

/* ============================ getter methods ================= */
BASIC_SHARED_D_READER_IMPL(LineSymbolCurve, bool, lineVisible, lineVisible);
BASIC_SHARED_D_READER_IMPL(LineSymbolCurve, bool, symbolsVisible, symbolsVisible);
BASIC_SHARED_D_READER_IMPL(LineSymbolCurve, const AbstractColumn *, xColumn, xColumn);
BASIC_SHARED_D_READER_IMPL(LineSymbolCurve, const AbstractColumn *, yColumn, yColumn);

/* ============================ setter methods and undo commands ================= */

STD_SETTER_CMD_IMPL_F(LineSymbolCurve, SetLineVisible, bool, lineVisible, updateVisibility);
void LineSymbolCurve::setLineVisible(bool visible) {
	Q_D(LineSymbolCurve);
	if (visible != d->lineVisible)
		exec(new LineSymbolCurveSetLineVisibleCmd(d, visible, visible ? tr("%1: set line visible") : tr("%1: set line invisible")));
}

STD_SETTER_CMD_IMPL_F(LineSymbolCurve, SetSymbolsVisible, bool, symbolsVisible, updateVisibility);
void LineSymbolCurve::setSymbolsVisible(bool visible) {
	Q_D(LineSymbolCurve);
	if (visible != d->symbolsVisible)
		exec(new LineSymbolCurveSetSymbolsVisibleCmd(d, visible, visible ? tr("%1: set symbols visible") : tr("%1: set symbols invisible")));
}

STD_SETTER_CMD_IMPL_F(LineSymbolCurve, SetXColumn, const AbstractColumn *, xColumn, retransform);
void LineSymbolCurve::setXColumn(const AbstractColumn *xColumn) {
	Q_D(LineSymbolCurve);
	if (xColumn != d->xColumn)
		exec(new LineSymbolCurveSetXColumnCmd(d, xColumn, tr("%1: assign x values")));
}

STD_SETTER_CMD_IMPL_F(LineSymbolCurve, SetYColumn, const AbstractColumn *, yColumn, retransform);
void LineSymbolCurve::setYColumn(const AbstractColumn *yColumn) {
	Q_D(LineSymbolCurve);
	if (yColumn != d->yColumn)
		exec(new LineSymbolCurveSetYColumnCmd(d, yColumn, tr("%1: assign y values")));
}

/* ============================ other methods ================= */

QGraphicsItem *LineSymbolCurve::graphicsItem() const {
	return d_ptr;
}

bool LineSymbolCurve::Private::swapVisible(bool on) {
	bool oldValue = isVisible();
	setVisible(on);
	return oldValue;
}

STD_SWAP_METHOD_SETTER_CMD_IMPL(LineSymbolCurve, SetVisible, bool, swapVisible);
void LineSymbolCurve::setVisible(bool on) {
	Q_D(LineSymbolCurve);
	exec(new LineSymbolCurveSetVisibleCmd(d, on, on ? tr("%1: set visible") : tr("%1: set invisible")));
}

bool LineSymbolCurve::isVisible() const {
	Q_D(const LineSymbolCurve);
	return d->isVisible();
}

void LineSymbolCurve::retransform() {
	d_ptr->retransform();
}

void LineSymbolCurve::Private::retransform() {
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

	if (symbolPoints.count() > 1) {
		 QListIterator<QPointF> p1Iter(symbolPoints);
		 QListIterator<QPointF> p2Iter(symbolPoints);
		 p2Iter.next();
		 while (p2Iter.hasNext())
			lines.append(QLineF(p1Iter.next(), p2Iter.next()));
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

void LineSymbolCurve::Private::updateVisibility() {
	// TODO
}

void LineSymbolCurve::Private::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget * widget)
{
	// symbols first/last option
		
	if (lineVisible) {
		painter->setPen(linePen);
		painter->setBrush(Qt::NoBrush);
		painter->drawPath(linePath);
	}

	if (symbolsVisible)
	{
		symbolPrototype->setPen(symbolsPen);
		symbolPrototype->setBrush(symbolsBrush);
		foreach(QPointF point, symbolPoints) {
			painter->translate(point);
			symbolPrototype->paint(painter, option, widget);
			painter->translate(-point);
		}
	}
}

