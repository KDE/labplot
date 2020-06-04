/***************************************************************************
    File                 : AxisPrivate.h
    Project              : LabPlot
    Description          : Private members of Axis.
    --------------------------------------------------------------------
    Copyright            : (C) 2011-2018 Alexander Semke (alexander.semke@web.de)

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
class CartesianCoordinateSystem;
class TextLabel;

class AxisPrivate: public QGraphicsItem {
public:
	explicit AxisPrivate(Axis*);

	QRectF boundingRect() const override;
	QPainterPath shape() const override;

	QString name() const;
	void retransform();
	void retransformLine();
	void retransformArrow();
	void retransformTicks();
	void retransformTickLabelPositions();
	void retransformTickLabelStrings();
	void retransformMinorGrid();
	void retransformMajorGrid();
	void updateGrid();
	bool swapVisible(bool);
	void recalcShapeAndBoundingRect();
	void setPrinting(bool);

	bool isDefault{false};

	//general
	bool autoScale;
	Axis::Orientation orientation; //!< horizontal or vertical
	Axis::Position position; //!< left, right, bottom, top or custom (usually not changed after creation)
	Axis::Scale scale;
	double offset; //!< offset from zero in the direction perpendicular to the axis
	double start; //!< start coordinate of the axis line
	double end; //!< end coordinate of the axis line
	qreal scalingFactor;
	qreal zeroOffset;

	//line
	QVector<QLineF> lines;
	QPen linePen;
	qreal lineOpacity;
	Axis::ArrowType arrowType;
	Axis::ArrowPosition arrowPosition;
	qreal arrowSize;

	// Title
	TextLabel* title;
	qreal titleOffsetX; //distance to the axis line
	qreal titleOffsetY; //distance to the axis line

	// Ticks
	Axis::TicksDirection majorTicksDirection; //!< major ticks direction: inwards, outwards, both, or none
	Axis::TicksType majorTicksType; //!< the way how the number of major ticks is specified  - either as a total number or an increment
	int majorTicksNumber; //!< number of major ticks
	qreal majorTicksSpacing; //!< spacing (step) for the major ticks
	const AbstractColumn* majorTicksColumn{nullptr}; //!< column containing values for major ticks' positions
	QString majorTicksColumnPath;
	qreal majorTicksLength; //!< major tick length (in page units!)
	QPen majorTicksPen;
	qreal majorTicksOpacity;

	Axis::TicksDirection minorTicksDirection; //!< minor ticks direction: inwards, outwards, both, or none
	Axis::TicksType minorTicksType;  //!< the way how the number of minor ticks is specified  - either as a total number or an increment
	int minorTicksNumber; //!< number of minor ticks (between each two major ticks)
	qreal minorTicksIncrement; //!< spacing (step) for the minor ticks
	const AbstractColumn* minorTicksColumn{nullptr}; //!< column containing values for minor ticks' positions
	QString minorTicksColumnPath;
	qreal minorTicksLength; //!< minor tick length (in page units!)
	QPen minorTicksPen;
	qreal minorTicksOpacity;

	// Tick Label
	Axis::LabelsFormat labelsFormat;
	int labelsPrecision;
	bool labelsAutoPrecision;
	QString labelsDateTimeFormat;
	Axis::LabelsPosition labelsPosition;
	qreal labelsRotationAngle;
	QColor labelsColor;
	QFont labelsFont;
	qreal labelsOffset; //!< offset, distance to the end of the tick line (in page units)
	qreal labelsOpacity;
	QString labelsPrefix;
	QString labelsSuffix;

	//Grid
	AxisGrid* gridItem;
	QPen majorGridPen;
	qreal majorGridOpacity;
	QPen minorGridPen;
	qreal minorGridOpacity;

	Axis* const q;

	QPainterPath linePath;
	QPainterPath majorGridPath;
	QPainterPath minorGridPath;
	bool suppressRetransform{false};
	bool labelsFormatDecimalOverruled{false};
	bool labelsFormatAutoChanged{false};

	CartesianPlot* plot{nullptr};
	const CartesianCoordinateSystem* cSystem{nullptr};

private:
	void contextMenuEvent(QGraphicsSceneContextMenuEvent*) override;
	void hoverEnterEvent(QGraphicsSceneHoverEvent*) override;
	void hoverLeaveEvent(QGraphicsSceneHoverEvent*) override;
	void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget* widget = nullptr) override;

	void addArrow(QPointF point, int direction);
	int upperLabelsPrecision(int);
	int lowerLabelsPrecision(int);
	bool transformAnchor(QPointF*);

	QPainterPath arrowPath;
	QPainterPath majorTicksPath;
	QPainterPath minorTicksPath;
	QRectF boundingRectangle;
	QPainterPath axisShape;

	QVector<QPointF> majorTickPoints;//!< position of the major ticks  on the axis.
	QVector<QPointF> minorTickPoints;//!< position of the major ticks  on the axis.
	QVector<QPointF> tickLabelPoints; //!< position of the major tick labels (left lower edge of label's bounding rect)
	QVector<double> tickLabelValues; //!< major tick labels values
	QVector<QString> tickLabelStrings; //!< the actual text of the major tick labels

	bool m_hovered{false};
	bool m_suppressRecalc{false};
	bool m_printing{false};
};

#endif
