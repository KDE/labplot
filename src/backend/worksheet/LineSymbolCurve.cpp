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

#include "worksheet/LineSymbolCurve.h"

/**
 * \class LineSymbolCurve
 * \brief A curve drawn as line and/or symbols
 *
 * 
 */


#include "worksheet/LineSymbolCurve.h"
#include "worksheet/AbstractCoordinateSystem.h"
#include "worksheet/CartesianCoordinateSystem.h"
#include "lib/commandtemplates.h"
#include <QGraphicsItem>
#include <QGraphicsItemGroup>
#include <QGraphicsPathItem>
#include <QGraphicsEllipseItem>
#include <QPainterPath>

/**
 * \class LineSymbolCurve
 * \brief Linear axis for cartesian coordinate systems.
 *
 *  
 */

class LineSymbolCurve::Private {
	public:
		Private(LineSymbolCurve *owner) : q(owner) {
		}

		~Private() {
		}

		QString name() const {
			return q->name();
		}

		bool lineVisible; //!< show/hide line
		bool symbolsVisible; //! show/hide symbols
		const AbstractColumn *xColumn; //!< Pointer to X column
		const AbstractColumn *yColumn; //!< Pointer to Y column
		QString symbolTypeId;
	
		mutable QGraphicsItemGroup itemGroup;
		mutable QGraphicsPathItem *lineItem;
		mutable QList<QGraphicsItem *> symbolItems;

		void retransform() const;
		void retransformSymbols(const AbstractCoordinateSystem *cSystem) const;
		void updateVisibility() const;
		qreal setZValue(qreal z);
		bool setVisible(bool on);

		LineSymbolCurve * const q;
};

LineSymbolCurve::LineSymbolCurve(const QString &name)
		: AbstractWorksheetElement(name), d(new Private(this)) {
	d->lineVisible = true;
	d->symbolsVisible = true;
	d->symbolTypeId = "circle";
	d->xColumn = NULL;
	d->yColumn = NULL;
	d->lineItem = new QGraphicsPathItem();
	d->itemGroup.addToGroup(d->lineItem);
	retransform();
}

LineSymbolCurve::~LineSymbolCurve() {
	delete d;
}

/* ============================ accessor documentation ================= */
/**
  \fn   LineSymbolCurve::BASIC_D_ACCESSOR_DECL(bool, lineVisible, LineVisible);
  \brief Set the line visible/invisible.
*/
/**
  \fn   LineSymbolCurve::BASIC_D_ACCESSOR_DECL(bool, symbolsVisible, SymbolsVisible);
  \brief Set the symbols visible/invisible.
*/
/**
  \fn   LineSymbolCurve::POINTER_D_ACCESSOR_DECL(const AbstractColumn, xColumn, XColumn);
  \brief Set the pointer to the X column.
*/
/**
  \fn   LineSymbolCurve::POINTER_D_ACCESSOR_DECL(const AbstractColumn, yColumn, YColumn);
  \brief Set the pointer to the Y column.
*/

/* ============================ getter methods ================= */
BASIC_D_READER_IMPL(LineSymbolCurve, bool, lineVisible, lineVisible);
BASIC_D_READER_IMPL(LineSymbolCurve, bool, symbolsVisible, symbolsVisible);
BASIC_D_READER_IMPL(LineSymbolCurve, const AbstractColumn *, xColumn, xColumn);
BASIC_D_READER_IMPL(LineSymbolCurve, const AbstractColumn *, yColumn, yColumn);

/* ============================ setter methods and undo commands ================= */

STD_SETTER_CMD_IMPL_F(LineSymbolCurve, SetLineVisible, bool, lineVisible, updateVisibility);
void LineSymbolCurve::setLineVisible(bool visible) {
	if (visible != d->lineVisible)
		exec(new LineSymbolCurveSetLineVisibleCmd(d, visible, visible ? tr("%1: set line visible") : tr("%1: set line invisible")));
}

STD_SETTER_CMD_IMPL_F(LineSymbolCurve, SetSymbolsVisible, bool, symbolsVisible, updateVisibility);
void LineSymbolCurve::setSymbolsVisible(bool visible) {
	if (visible != d->symbolsVisible)
		exec(new LineSymbolCurveSetSymbolsVisibleCmd(d, visible, visible ? tr("%1: set symbols visible") : tr("%1: set symbols invisible")));
}

STD_SETTER_CMD_IMPL_F(LineSymbolCurve, SetXColumn, const AbstractColumn *, xColumn, retransform);
void LineSymbolCurve::setXColumn(const AbstractColumn *xColumn) {
	if (xColumn != d->xColumn)
		exec(new LineSymbolCurveSetXColumnCmd(d, xColumn, tr("%1: assign x values")));
}

STD_SETTER_CMD_IMPL_F(LineSymbolCurve, SetYColumn, const AbstractColumn *, yColumn, retransform);
void LineSymbolCurve::setYColumn(const AbstractColumn *yColumn) {
	if (yColumn != d->yColumn)
		exec(new LineSymbolCurveSetYColumnCmd(d, yColumn, tr("%1: assign y values")));
}

/* ============================ other methods ================= */

QList<QGraphicsItem *> LineSymbolCurve::graphicsItems() const {
	return QList<QGraphicsItem *>() << &(d->itemGroup);
}

qreal LineSymbolCurve::Private::setZValue(qreal z) {
	qreal oldZ = itemGroup.zValue();
	itemGroup.setZValue(z);
	lineItem->setZValue(z);
	foreach(QGraphicsItem *item, symbolItems)
		item->setZValue(z);
	return oldZ;
}

STD_SWAP_METHOD_SETTER_CMD_IMPL(LineSymbolCurve, SetZ, qreal, setZValue);
void LineSymbolCurve::setZValue(qreal z) {
	if (zValue() != z)
		exec(new LineSymbolCurveSetZCmd(d, z, tr("%1: set z value")));
}

qreal LineSymbolCurve::zValue () const {
	return d->itemGroup.zValue();;
}

QRectF LineSymbolCurve::boundingRect() const {
	return d->itemGroup.boundingRect();
}

bool LineSymbolCurve::contains(const QPointF &position) const {
	// TODO
	return false;
}

bool LineSymbolCurve::Private::setVisible(bool on) {
	bool oldValue = itemGroup.isVisible();
	itemGroup.setVisible(on);
	return oldValue;
}

STD_SWAP_METHOD_SETTER_CMD_IMPL(LineSymbolCurve, SetVisible, bool, setVisible);
void LineSymbolCurve::setVisible(bool on) {
	exec(new LineSymbolCurveSetVisibleCmd(d, on, on ? tr("%1: set visible") : tr("%1: set invisible")));
}

bool LineSymbolCurve::isVisible() const {
	return d->itemGroup.isVisible();
}

void LineSymbolCurve::retransform() const {
	d->retransform();
}

void LineSymbolCurve::Private::retransform() const {
	const AbstractCoordinateSystem *cSystem = q->coordinateSystem();

	itemGroup.removeFromGroup(lineItem);
	foreach(QGraphicsItem *item, symbolItems)
			itemGroup.removeFromGroup(item);

	if ( (NULL == xColumn) || (NULL == yColumn) )
		return;
	
	// TODO: symbol stuff

	// TODO: add start row/end row attributes

	int startRow = 0;
	int endRow = xColumn->rowCount() - 1;

	QPainterPath path;
	int count = 0;
	QPointF tempPoint;

	SciDAVis::ColumnMode xColMode = xColumn->columnMode();
	SciDAVis::ColumnMode yColMode = yColumn->columnMode();

	for (int row = startRow; row <= endRow; row++ ) {

		if ( xColumn->isValid(row) && yColumn->isValid(row) 
			&& (!xColumn->isMasked(row)) && (!yColumn->isMasked(row)) ) {

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

			if (cSystem) {
				tempPoint = cSystem->mapLogicalToScene(tempPoint);
			}
			if (count == 0)
				path.moveTo(tempPoint);
			else
				path.lineTo(tempPoint);

			count++;
		}
	}
	lineItem->setPath(path);

	itemGroup.addToGroup(lineItem);
	retransformSymbols(cSystem);

}

void LineSymbolCurve::Private::retransformSymbols(const AbstractCoordinateSystem *cSystem) const {
	// TODO
}


void LineSymbolCurve::Private::updateVisibility() const {
	// TODO
}



