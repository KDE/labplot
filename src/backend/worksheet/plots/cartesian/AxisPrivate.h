/***************************************************************************
    File                 : AxisPrivate.h
    Project              : LabPlot
    Description          : Private members of Axis.
    --------------------------------------------------------------------
    Copyright            : (C) 2011-2015 Alexander Semke (alexander.semke@web.de)

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

class QGraphicsSceneHoverEvent;

class AxisGrid;
class CartesianPlot;
class AbstractCoordinateSystem;
class CartesianCoordinateSystem;
class TextLabel;

//TODO: align
class AxisPrivate: public QGraphicsItem {
	public:
		explicit AxisPrivate(Axis* owner);

		CartesianPlot* m_plot;
		const CartesianCoordinateSystem* m_cSystem;
		bool m_printing;
		bool m_hovered;
		bool m_suppressRecalc;

		//general
		bool autoScale;
		Axis::AxisOrientation orientation; //!< horizontal or vertical
		Axis::AxisPosition position; //!< left, right, bottom, top or custom (usually not changed after creation)
		Axis::AxisScale scale;
		float offset; //!< offset from zero in the direction perpendicular to the axis
		float start; //!< start coordinate of the axis line
		float end; //!< end coordinate of the axis line
		qreal scalingFactor;
		qreal zeroOffset;

		//line
		QList<QLineF> lines;
		QPen linePen;
		qreal lineOpacity;
		Axis::ArrowType arrowType;
		Axis::ArrowPosition arrowPosition;
		float arrowSize;

		// Title
		TextLabel* title;
		float titleOffsetX; //distance to the axis line
		float titleOffsetY; //distance to the axis line

		// Ticks
		Axis::TicksDirection majorTicksDirection; //!< major ticks direction: inwards, outwards, both, or none
		Axis::TicksType majorTicksType; //!< the way how the number of major ticks is specified  - either as a total number or an increment
		int majorTicksNumber; //!< number of major ticks
		qreal majorTicksIncrement; //!< increment (step) for the major ticks
		const AbstractColumn* majorTicksColumn; //!< column containing values for major ticks' positions
		QString majorTicksColumnPath;
		qreal majorTicksLength; //!< major tick length (in page units!)
		QPen majorTicksPen;
		qreal majorTicksOpacity;

		Axis::TicksDirection minorTicksDirection; //!< minor ticks direction: inwards, outwards, both, or none
		Axis::TicksType minorTicksType;  //!< the way how the number of minor ticks is specified  - either as a total number or an increment
		int minorTicksNumber; //!< number of minor ticks (between each two major ticks)
		qreal minorTicksIncrement; //!< increment (step) for the minor ticks
		const AbstractColumn* minorTicksColumn; //!< column containing values for minor ticks' positions
		QString minorTicksColumnPath;
		qreal minorTicksLength; //!< minor tick length (in page units!)
		QPen minorTicksPen;
		qreal minorTicksOpacity;

		// Tick Label
		Axis::LabelsFormat labelsFormat;
		int labelsPrecision;
		bool labelsAutoPrecision;
		Axis::LabelsPosition labelsPosition;
		qreal labelsRotationAngle;
		QColor labelsColor;
		QFont labelsFont;
		float labelsOffset; //!< offset, distance to the end of the tick line (in page units)
		qreal labelsOpacity;
		QString labelsPrefix;
		QString labelsSuffix;
		QList<QPointF> majorTickPoints;//!< position of the major ticks  on the axis.
		QList<QPointF> minorTickPoints;//!< position of the major ticks  on the axis.
		QList<QPointF> tickLabelPoints; //!< position of the major tick labels (left lower edge of label's bounding rect)
		QList<float> tickLabelValues; //!< major tick labels values
		QList<QString> tickLabelStrings; //!< the actual text of the major tick labels

		//Grid
		AxisGrid* gridItem;
		QPen majorGridPen;
		qreal majorGridOpacity;
		QPen minorGridPen;
		qreal minorGridOpacity;

		QPainterPath linePath;
		QPainterPath arrowPath;
		QPainterPath majorTicksPath;
		QPainterPath minorTicksPath;
		QPainterPath majorGridPath;
		QPainterPath minorGridPath;
		QRectF boundingRectangle;
		QPainterPath axisShape;

		QString name() const;
		virtual QRectF boundingRect() const;
		virtual QPainterPath shape() const;
		virtual void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget* widget = 0);

		virtual void retransform();
		void retransformLine();
		void retransformArrow();
		void addArrow(const QPointF& point, int direction);
		void retransformTicks();
		void retransformTickLabelPositions();
		void retransformTickLabelStrings();
		void retransformMinorGrid();
		void retransformMajorGrid();
		int upperLabelsPrecision(int precision);
		int lowerLabelsPrecision(int precision);
		double round(double value, int precision);
		virtual void recalcShapeAndBoundingRect();
		bool swapVisible(bool on);

		Axis* const q;

	private:
        virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent*);
		virtual void hoverEnterEvent(QGraphicsSceneHoverEvent*);
		virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent*);

	protected:
		bool transformAnchor(QPointF*);
};

#endif
