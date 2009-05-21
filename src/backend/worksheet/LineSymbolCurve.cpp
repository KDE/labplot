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

/**
 * \class LineSymbolCurve
 * \brief Linear axis for cartesian coordinate systems.
 *
 *  
 */

class LineSymbolCurve::Private: public QGraphicsItem {
	public:
		Private(LineSymbolCurve *owner);
		~Private();

		QString name() const {
			return q->name();
		}

		bool lineVisible; //!< show/hide line
		bool symbolsVisible; //! show/hide symbols
		qreal symbolRotationAngle;
		qreal symbolSize;
		qreal symbolAspectRatio;
		QString symbolTypeId;
		const AbstractColumn *xColumn; //!< Pointer to X column
		const AbstractColumn *yColumn; //!< Pointer to Y column
	
		QPainterPath linePath;
		AbstractCurveSymbol *symbolPrototype;
		QRectF boundingRectangle;
		QList<QPointF> symbolPoints;

		void retransform();
		void retransformSymbols(const AbstractCoordinateSystem *cSystem);
		void updateVisibility();
		qreal swapZValue(qreal z);
		bool swapVisible(bool on);

		virtual QRectF boundingRect() const { return boundingRectangle; }
    	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget * widget = 0);

		LineSymbolCurve * const q;

		// TODO: make other attributes adjustable
		// add pens and brush
};

LineSymbolCurve::Private::Private(LineSymbolCurve *owner): q(owner) {
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

	symbolSize = 2.5;
	symbolPrototype->setSize(symbolSize);
	symbolPrototype->setAspectRatio(symbolAspectRatio);
	symbolPrototype->setBrush(QBrush(Qt::red));

	retransform();
}

LineSymbolCurve::Private::~Private() {
}

LineSymbolCurve::LineSymbolCurve(const QString &name)
		: AbstractWorksheetElement(name), d(new Private(this)) {
}

LineSymbolCurve::~LineSymbolCurve() {
	delete d;
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

QGraphicsItem *LineSymbolCurve::graphicsItem() const {
	return d;
}

qreal LineSymbolCurve::Private::swapZValue(qreal z) {
	qreal oldZ = zValue();
	setZValue(z);
	return oldZ;
}

STD_SWAP_METHOD_SETTER_CMD_IMPL(LineSymbolCurve, SetZ, qreal, swapZValue);
void LineSymbolCurve::setZValue(qreal z) {
	if (zValue() != z)
		exec(new LineSymbolCurveSetZCmd(d, z, tr("%1: set z value")));
}

qreal LineSymbolCurve::zValue () const {
	return d->zValue();
}

bool LineSymbolCurve::Private::swapVisible(bool on) {
	bool oldValue = isVisible();
	setVisible(on);
	return oldValue;
}

STD_SWAP_METHOD_SETTER_CMD_IMPL(LineSymbolCurve, SetVisible, bool, swapVisible);
void LineSymbolCurve::setVisible(bool on) {
	exec(new LineSymbolCurveSetVisibleCmd(d, on, on ? tr("%1: set visible") : tr("%1: set invisible")));
}

bool LineSymbolCurve::isVisible() const {
	return d->isVisible();
}

void LineSymbolCurve::retransform() {
	d->retransform();
}

void LineSymbolCurve::Private::retransform() {
	const AbstractCoordinateSystem *cSystem = q->coordinateSystem();

	linePath = QPainterPath();
	boundingRectangle = QRect();

	if ( (NULL == xColumn) || (NULL == yColumn) )
		return;
	
	// TODO: add start row/end row attributes

	int startRow = 0;
	int endRow = xColumn->rowCount() - 1;

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
				linePath.moveTo(tempPoint);
			else
				linePath.lineTo(tempPoint);

			count++;
		}
	}

	boundingRectangle = linePath.boundingRect();
	retransformSymbols(cSystem);
}

void LineSymbolCurve::Private::retransformSymbols(const AbstractCoordinateSystem *cSystem) {
	symbolPoints.clear();

	int startRow = 0;
	int endRow = xColumn->rowCount() - 1;
	QPointF tempPoint;

	QRectF prototypeBoundingRect = symbolPrototype->boundingRect();

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
			
			symbolPoints.append(tempPoint);

			prototypeBoundingRect.moveCenter(tempPoint); 
			boundingRectangle |= prototypeBoundingRect;
		}
	}
}


void LineSymbolCurve::Private::updateVisibility() {
	// TODO
}


void LineSymbolCurve::Private::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget * widget)
{
	// TODO: set pen and brush
	// symbols first/last option
	
	if (lineVisible)
		painter->drawPath(linePath);

	if (symbolsVisible)
	{
		foreach(QPointF point, symbolPoints) {
			painter->translate(point);
			symbolPrototype->paint(painter, option, widget);
			painter->translate(-point);
		}
	}
}

