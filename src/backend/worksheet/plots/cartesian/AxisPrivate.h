/***************************************************************************
    File                 : AxisPrivate.h
    Project              : LabPlot/SciDAVis
    Description          : Private members of Axis.
    --------------------------------------------------------------------
    Copyright            : (C) 2009 Tilman Benkert (thzs*gmx.net)
    Copyright            : (C) 2011-2012 Alexander Semke (alexander.semke*web.de)
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

#ifndef AXISPRIVATE_H
#define AXISPRIVATE_H

#include <QGraphicsItem>
#include <QPen>
#include <QFont>
#include "Axis.h"

class AbstractCoordinateSystem;
class TextLabel;

class AxisPrivate: public QGraphicsItem {
	public:
		AxisPrivate(Axis *owner);

		bool autoScale;
		Axis::AxisOrientation orientation; //!< horizontal or vertical
		Axis::AxisPosition position; //!< left, right, bottom, top or custom (usually not changed after creation)
		Axis::AxisScale scale;
		float offset; //!< offset from zero in the direction perpendicular to the axis
		float start; //!< start coordinate of the axis line
		float end; //!< end coordinate of the axis line
		qreal scalingFactor;
		qreal zeroOffset;
		
		QPen linePen;
		qreal lineOpacity;

		// Title
		TextLabel *title;
		float titleOffset; //distance to the axis line

		// Ticks
		Axis::TicksDirection majorTicksDirection; //!< major ticks direction: inwards, outwards, both, or none
		Axis::TicksType majorTicksType; //!< the way how the number of major ticks is specified  - either as a total number or an increment
		int majorTicksNumber; //!< number of major ticks
		qreal majorTicksIncrement; //!< increment (step) for the major ticks
		qreal majorTicksLength; //!< major tick length (in page units!)
		QPen majorTicksPen;
		qreal majorTicksOpacity;
		
		Axis::TicksDirection minorTicksDirection; //!< minor ticks direction: inwards, outwards, both, or none
		Axis::TicksType minorTicksType;  //!< the way how the number of minor ticks is specified  - either as a total number or an increment
		int minorTicksNumber; //!< number of minor ticks (between each two major ticks)
		qreal minorTicksIncrement; //!< increment (step) for the minor ticks
		qreal minorTicksLength; //!< minor tick length (in page units!)
		QPen minorTicksPen;
		qreal minorTicksOpacity;	
		
		// Tick Label
		Axis::LabelsPosition labelsPosition;
		qreal labelsFontSize;
		qreal labelsRotationAngle;
		QColor labelsColor;
		QFont labelsFont;
		float labelsOffset; //!< offset, distance to the end of the tick line (in page units)
		qreal labelsOpacity;	
		char numericFormat; //TODO
		int displayedDigits; //TODO
		// TODO support for date/time and string labels
		QString labelsPrefix;
		QString labelsSuffix;
		QList<QPointF> tickPoints;//!< position of the major ticks  on the axis.
		QList<QPointF> tickLabelPoints; //!< position of the major tick labels (left lower edge of label's bounding rect)
		QList<QString> tickLabelStrings; //!< the actual text of the major tick labels
		
		//TODO: Grid
		
		QPainterPath linePath;
		QPainterPath majorTicksPath;
		QPainterPath minorTicksPath;
		QRectF boundingRectangle;
		QPainterPath axisShape;

		//TODO extra tick label
// 		QList<TextLabel *> labels;

		QString name() const;
		virtual QRectF boundingRect() const;
		virtual QPainterPath shape() const;
		virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget * widget = 0);

		virtual void retransform();
		virtual void retransformTicks();
		virtual void retransformTickLabels();
		virtual void retransformTicks(const AbstractCoordinateSystem *cSystem);
		virtual void recalcShapeAndBoundingRect();
		bool swapVisible(bool on);

		Axis * const q;

	private:
        void contextMenuEvent(QGraphicsSceneContextMenuEvent*);

	protected:
		bool transformAnchor(const AbstractCoordinateSystem *cSystem, QPointF *anchorPoint);
};

#endif
