/***************************************************************************
    File                 : LineSymbolCurvePrivate.h
    Project              : LabPlot/SciDAVis
    Description          : Private members of LineSymbolCurve
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

#ifndef LINESYMBOLCURVEPRIVATE_H
#define LINESYMBOLCURVEPRIVATE_H

#include "worksheet/AbstractCurveSymbol.h"

class LineSymbolCurvePrivate: public QGraphicsItem {
	public:
		LineSymbolCurvePrivate(LineSymbolCurve *owner);
		~LineSymbolCurvePrivate();

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
		QPainterPath curveShape;
		QList<QPointF> symbolPoints;

		QBrush symbolsBrush;
		QPen symbolsPen;
		QPen linePen;

		virtual void retransform();
		void updateVisibility();
		bool swapVisible(bool on);

		virtual QRectF boundingRect() const { return boundingRectangle; }
		QPainterPath shape() const { return curveShape; }
    	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget * widget = 0);

		LineSymbolCurve * const q;

		// TODO: make other attributes adjustable
		// add pens and brush
};


#endif


