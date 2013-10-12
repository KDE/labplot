/***************************************************************************
    File                 : AxisPrivate.h
    Project              : LabPlot/SciDAVis
    Description          : Private members of Axis.
    --------------------------------------------------------------------
    Copyright            : (C) 2009 Tilman Benkert (thzs*gmx.net)
    Copyright            : (C) 2011-2013 Alexander Semke (alexander.semke*web.de)
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
#include "Axis.h"

class CartesianPlot;
class AbstractCoordinateSystem;
class CartesianCoordinateSystem;
class TextLabel;

class AxisPrivate: public QGraphicsItem {
	public:
		AxisPrivate(Axis* owner);

		const CartesianPlot* m_plot;
		const CartesianCoordinateSystem* m_cSystem;

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
		QPen linePen;
		qreal lineOpacity;

		// Title
		TextLabel* title;
		float titleOffset; //distance to the axis line

		// Ticks
		Axis::TicksDirection majorTicksDirection; //!< major ticks direction: inwards, outwards, both, or none
		Axis::TicksType majorTicksType; //!< the way how the number of major ticks is specified  - either as a total number or an increment
		int majorTicksNumber; //!< number of major ticks
		qreal majorTicksIncrement; //!< increment (step) for the major ticks
		const AbstractColumn* majorTicksColumn; //!< column containing values for major ticks' positions
		QString majorTicksColumnName;
		QString majorTicksColumnParentName;
		qreal majorTicksLength; //!< major tick length (in page units!)
		QPen majorTicksPen;
		qreal majorTicksOpacity;

		Axis::TicksDirection minorTicksDirection; //!< minor ticks direction: inwards, outwards, both, or none
		Axis::TicksType minorTicksType;  //!< the way how the number of minor ticks is specified  - either as a total number or an increment
		int minorTicksNumber; //!< number of minor ticks (between each two major ticks)
		qreal minorTicksIncrement; //!< increment (step) for the minor ticks
		const AbstractColumn* minorTicksColumn; //!< column containing values for minor ticks' positions
		QString minorTicksColumnName;
		QString minorTicksColumnParentName;
		qreal minorTicksLength; //!< minor tick length (in page units!)
		QPen minorTicksPen;
		qreal minorTicksOpacity;

		// Tick Label
		Axis::LabelsFormat labelsFormat;
		int labelsPrecision;
		bool labelsAutoPrecision;
		Axis::LabelsPosition labelsPosition;
		qreal labelsFontSize;
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
		QPen majorGridPen;
		qreal majorGridOpacity;
		QPen minorGridPen;
		qreal minorGridOpacity;

		QPainterPath linePath;
		QPainterPath majorTicksPath;
		QPainterPath minorTicksPath;
		QPainterPath majorGridPath;
		QPainterPath minorGridPath;
		QRectF boundingRectangle;
		QPainterPath axisShape;
		QPainterPath axisShapeWithoutGrids;

		QString name() const;
		virtual QRectF boundingRect() const;
		virtual QPainterPath shape() const;
		virtual void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget* widget = 0);

		virtual void retransform();
		void retransformLine();
		void retransformTicks();
		void retransformTickLabels();
		void retransformTickLabelStrings();
		void retransformMinorGrid();
		void retransformMajorGrid();
		int upperLabelsPrecision(int precision);
		int lowerLabelsPrecision(int precision);
		float round(float value, int precision);
		virtual void recalcShapeAndBoundingRect();
		bool swapVisible(bool on);

		Axis* const q;

	private:
        void contextMenuEvent(QGraphicsSceneContextMenuEvent*);

	protected:
		bool transformAnchor(QPointF*);
};

#endif
