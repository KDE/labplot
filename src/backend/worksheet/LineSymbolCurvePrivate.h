/***************************************************************************
    File                 : LineSymbolCurvePrivate.h
    Project              : LabPlot/SciDAVis
    Description          : Private members of LineSymbolCurve
    --------------------------------------------------------------------
    Copyright            : (C) 2009 Tilman Benkert (thzs*gmx.net)
    Copyright            : (C) 2010 Alexander Semke (alexander.semke*web.de)
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
#include "worksheet/LineSymbolCurve.h"

class LineSymbolCurvePrivate: public QGraphicsItem {
	public:
		LineSymbolCurvePrivate(LineSymbolCurve *owner);
		~LineSymbolCurvePrivate();

		QString name() const;
		virtual QRectF boundingRect() const;
		QPainterPath shape() const;
		
		virtual void retransform();
		bool swapVisible(bool on);
		QString swapSymbolTypeId(const QString &id);
		virtual void recalcShapeAndBoundingRect();
		void updateSymbolPrototype();

		virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget * widget = 0);
		
		const AbstractColumn *xColumn;
		const AbstractColumn *yColumn;
		
		LineSymbolCurve::LineType lineType;
		QPen linePen;
// 		bool lineVisible;
		qreal lineOpacity;
		
// 		bool symbolsVisible;
		QBrush symbolsBrush;
		QPen symbolsPen;
		qreal symbolsOpacity;
		qreal symbolRotationAngle;
		qreal symbolSize;
		qreal symbolAspectRatio;
		QString symbolTypeId;
	
		QPainterPath linePath;
		AbstractCurveSymbol *symbolPrototype;
		QRectF boundingRectangle;
		QPainterPath curveShape;
		QList<QPointF> symbolPoints;

		LineSymbolCurve * const q;
};

#endif
