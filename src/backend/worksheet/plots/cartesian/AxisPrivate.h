/*
	File                 : AxisPrivate.h
	Project              : LabPlot
	Description          : Private members of Axis.
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2011-2022 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2020-2021 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef AXISPRIVATE_H
#define AXISPRIVATE_H

#include "Axis.h"

#include "backend/worksheet/WorksheetElementPrivate.h"
#include <QFont>
#include <QPen>

class QGraphicsSceneHoverEvent;

class AxisGrid;
class Line;
class TextLabel;

class AxisPrivate : public WorksheetElementPrivate {
public:
	explicit AxisPrivate(Axis*);

	QRectF boundingRect() const override;
	QPainterPath shape() const override;

	void retransform() override;
	void retransformRange();
	void retransformLine();
	void retransformArrow();
	void retransformTicks();
	void retransformTickLabelPositions();
	void retransformTickLabelStrings();
	void retransformMinorGrid();
	void retransformMajorGrid();
	void updateGrid();
	bool swapVisible(bool);
	void recalcShapeAndBoundingRect() override;
	bool isHovered() const;
	static QString createScientificRepresentation(const QString& mantissa, const QString& exponent);

	bool isDefault{false};

	// general
	Axis::RangeType rangeType;
	Axis::Orientation orientation; //!< horizontal or vertical
	Axis::Position position; //!< left, right, bottom, top or custom (usually not changed after creation)
	RangeT::Scale scale;
	double offset{0}; //!< offset from zero in the direction perpendicular to the axis
	Range<double> range; //!< coordinate range of the axis line
	Axis::TicksStartType majorTicksStartType{Axis::TicksStartType::Offset};
	qreal majorTickStartOffset{0};
	qreal majorTickStartValue{0};
	qreal scalingFactor{1};
	qreal zeroOffset{0};
	bool showScaleOffset{true};
	double logicalPosition{0};

	// line
	QVector<QLineF> lines;
	Line* line{nullptr};
	Axis::ArrowType arrowType;
	Axis::ArrowPosition arrowPosition;
	qreal arrowSize;

	// Title
	TextLabel* title{nullptr};
	qreal titleOffsetX; // distance to the axis line
	qreal titleOffsetY; // distance to the axis line

	// Ticks
	Axis::TicksDirection majorTicksDirection; //!< major ticks direction: inwards, outwards, both, or none
	Axis::TicksType majorTicksType; //!< the way how the number of major ticks is specified  - either as a total number or an increment
	bool majorTicksAutoNumber{true}; //!< If the number of ticks should be adjusted automatically or not
	int majorTicksNumber; //!< number of major ticks
	qreal majorTicksSpacing; //!< spacing (step) for the major ticks
	const AbstractColumn* majorTicksColumn{nullptr}; //!< column containing values for major ticks' positions
	QString majorTicksColumnPath;
	qreal majorTicksLength; //!< major tick length (in page units!)
	Line* majorTicksLine{nullptr};

	Axis::TicksDirection minorTicksDirection; //!< minor ticks direction: inwards, outwards, both, or none
	Axis::TicksType minorTicksType; //!< the way how the number of minor ticks is specified  - either as a total number or an increment
	bool minorTicksAutoNumber{true}; //!< If the number of ticks should be adjusted automatically or not
	int minorTicksNumber; //!< number of minor ticks (between each two major ticks)
	qreal minorTicksIncrement; //!< spacing (step) for the minor ticks
	const AbstractColumn* minorTicksColumn{nullptr}; //!< column containing values for minor ticks' positions
	QString minorTicksColumnPath;
	qreal minorTicksLength; //!< minor tick length (in page units!)
	Line* minorTicksLine{nullptr};

	// Tick Label
	Axis::LabelsFormat labelsFormat;
	bool labelsFormatAuto{true};
	int labelsPrecision;
	bool labelsAutoPrecision;
	QString labelsDateTimeFormat;
	Axis::LabelsPosition labelsPosition;
	Axis::LabelsTextType labelsTextType;
	const AbstractColumn* labelsTextColumn{nullptr};
	QString labelsTextColumnPath;
	qreal labelsRotationAngle;
	QColor labelsColor;
	QFont labelsFont;
	Axis::LabelsBackgroundType labelsBackgroundType;
	QColor labelsBackgroundColor;
	qreal labelsOffset; //!< offset, distance to the end of the tick line (in page units)
	qreal labelsOpacity;
	QString labelsPrefix;
	QString labelsSuffix;

	// Grid
	AxisGrid* gridItem{nullptr};
	Line* majorGridLine{nullptr};
	Line* minorGridLine{nullptr};

	Axis* const q{nullptr};

	QPainterPath linePath;
	QPainterPath majorGridPath;
	QPainterPath minorGridPath;

	QVector<QPointF> majorTickPoints; //!< position of the major ticks  on the axis.
	QVector<QPointF> minorTickPoints; //!< position of the major ticks  on the axis.
	QVector<QPointF> tickLabelPoints; //!< position of the major tick labels (left lower edge of label's bounding rect)
	QVector<double> tickLabelValues; //!< major tick labels values
	QVector<QString> tickLabelValuesString; //!< major tick labels used when a custom text column is selected
	QVector<QString> tickLabelStrings; //!< the actual text of the major tick labels

private:
	CartesianPlot* plot() const {
		return q->m_plot; // convenience method
	}
	void contextMenuEvent(QGraphicsSceneContextMenuEvent*) override;
	void hoverEnterEvent(QGraphicsSceneHoverEvent*) override;
	void hoverLeaveEvent(QGraphicsSceneHoverEvent*) override;
	void mousePressEvent(QGraphicsSceneMouseEvent*) override;
	void mouseMoveEvent(QGraphicsSceneMouseEvent*) override;
	void mouseReleaseEvent(QGraphicsSceneMouseEvent*) override;
	void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget* widget = nullptr) override;

	void addArrow(QPointF point, int direction);
	int upperLabelsPrecision(int precision, Axis::LabelsFormat);
	int lowerLabelsPrecision(int precision, Axis::LabelsFormat);
	bool transformAnchor(QPointF*);

	QPainterPath arrowPath;
	QPainterPath majorTicksPath;
	QPainterPath minorTicksPath;
	QRectF boundingRectangle;
	QPainterPath axisShape;

	bool m_hovered{false};
	bool m_suppressRecalc{false};
	bool m_panningStarted{false};
	QPointF m_panningStart;
};

#endif
