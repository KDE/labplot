/***************************************************************************
    File                 : LinearAxisPrivate.h
    Project              : LabPlot/SciDAVis
    Description          : Private members of LinearAxis.
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

#ifndef LINEARAXISPRIVATE_H
#define LINEARAXISPRIVATE_H

#include <QGraphicsItem>
#include <QPen>
#include <QFont>
#include "worksheet/ScalableTextLabel.h"

class LinearAxisPrivate: public QGraphicsItem {
	public:
		LinearAxisPrivate(LinearAxis *owner) : q(owner) {
		}

		virtual ~LinearAxisPrivate() {
		}

		QString name() const {
			return q->name();
		}

		LinearAxis::AxisOrientation orientation; //!< left, right, bottom, or top (usually not changed after creation)
		qreal offset; //!< offset from zero in the directin perpendicular to the axis
		qreal start; //!< start coordinate of the axis line
		qreal end; //!< end coordinate of the axis line
		qreal tickStart; //!< coordinate of the first tick (typically ==0 or ==start)
		qreal tickEnd; //!< coordinate of the last tick (typically ==end)
		int majorTickCount; //!< number of major ticks
		int minorTickCount; //!< number of minor ticks (between each two major ticks)
		qreal majorTicksLength; //!< major tick length (in page units!)
		qreal minorTicksLength; //!< minor tick length (in page units!)
		LinearAxis::TicksDirection majorTicksDirection; //!< major ticks direction: inwards, outwards, both, or none
		LinearAxis::TicksDirection minorTicksDirection; //!< minor ticks direction: inwards, outwards, both, or none
		qreal labelFontSize;
		qreal labelRotationAngle;

		QPainterPath linePath;
		QPainterPath majorTicksPath;
		QPainterPath minorTicksPath;
		QList<ScalableTextLabel *> labels;
		QRectF boundingRectangle;
		QPainterPath axisShape;

		QPen pen;
		QColor labelColor;
		QFont labelFont;
		QPointF labelOffset;

		char numericFormat;
		int displayedDigits;
		// TODO support for date/time and string labels

		virtual QRectF boundingRect() const { return boundingRectangle; }
		virtual QPainterPath shape() const { return axisShape; }
    	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget * widget = 0);

		virtual void retransform();
		virtual void retransformTicks();
		virtual void restyleLabels();
		virtual void retransformTicks(const AbstractCoordinateSystem *cSystem);
		virtual void recalcShapeAndBoundingRect();
		bool swapVisible(bool on);
		QPointF swapLabelOffset(const QPointF &newOffset);
		virtual void addTextLabel(const QString &text, const QPointF &pos);

		LinearAxis * const q;

	protected:
		bool transformAnchor(const AbstractCoordinateSystem *cSystem, QPointF *anchorPoint);
};

#endif


