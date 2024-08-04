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
class Heatmap;

class AxisPrivate : public WorksheetElementPrivate {
public:
	explicit AxisPrivate(Axis*);

	void retransform() override;
	void retransformRange();
	void retransformLine();
	void retransformArrow();
	void retransformColorBar();
	void retransformTicks();
	void retransformTickLabelPositions();
	void retransformTickLabelStrings();
	void retransformMinorGrid();
	void retransformMajorGrid();
	void updateGrid();
	bool swapVisible(bool);
	void recalcShapeAndBoundingRect() override;
	static QString createScientificRepresentation(const QString& mantissa, const QString& exponent);

	bool isDefault{false};

	// general
	Axis::RangeType rangeType{Axis::RangeType::Auto};
	Axis::Orientation orientation{Axis::Orientation::Horizontal}; //!< horizontal or vertical
	Axis::Position position{Axis::Position::Centered}; //!< left, right, bottom, top or custom (usually not changed after creation)
	double offset{0}; //!< offset from zero in the direction perpendicular to the axis
	Range<double> range; //!< coordinate range of the axis line
	bool rangeScale{true};
	RangeT::Scale scale{RangeT::Scale::Linear}; //!< Scale if rangeScale is false
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
	Axis::ArrowType arrowType{Axis::ArrowType::NoArrow};
	Axis::ArrowPosition arrowPosition{Axis::ArrowPosition::Right};
	qreal arrowSize{Worksheet::convertToSceneUnits(10, Worksheet::Unit::Point)};

	// Title
	TextLabel* title{nullptr};
	qreal titleOffsetX{Worksheet::convertToSceneUnits(2, Worksheet::Unit::Point)}; // distance to the axis line
	qreal titleOffsetY{Worksheet::convertToSceneUnits(2, Worksheet::Unit::Point)}; // distance to the axis line

	// Ticks
	Axis::TicksDirection majorTicksDirection{Axis::ticksOut}; //!< major ticks direction: inwards, outwards, both, or none
	Axis::TicksType majorTicksType{
		Axis::TicksType::TotalNumber}; //!< the way how the number of major ticks is specified  - either as a total number or an increment
	bool majorTicksAutoNumber{true}; //!< If the number of ticks should be adjusted automatically or not
	int majorTicksNumber{6}; //!< number of major ticks
	qreal majorTicksSpacing{0.0}; //!< spacing (step) for the major ticks
	const AbstractColumn* majorTicksColumn{nullptr}; //!< column containing values for major ticks' positions
	QString majorTicksColumnPath;
	qreal majorTicksLength{Worksheet::convertToSceneUnits(6.0, Worksheet::Unit::Point)}; //!< major tick length (in page units!)
	Line* majorTicksLine{nullptr};

	Axis::TicksDirection minorTicksDirection{Axis::ticksOut}; //!< minor ticks direction: inwards, outwards, both, or none
	Axis::TicksType minorTicksType{
		Axis::TicksType::TotalNumber}; //!< the way how the number of minor ticks is specified  - either as a total number or an increment
	bool minorTicksAutoNumber{true}; //!< If the number of ticks should be adjusted automatically or not
	int minorTicksNumber{1}; //!< number of minor ticks (between each two major ticks)
	qreal minorTicksIncrement{0.0}; //!< spacing (step) for the minor ticks
	const AbstractColumn* minorTicksColumn{nullptr}; //!< column containing values for minor ticks' positions
	QString minorTicksColumnPath;
	qreal minorTicksLength{Worksheet::convertToSceneUnits(3.0, Worksheet::Unit::Point)}; //!< minor tick length (in page units!)
	Line* minorTicksLine{nullptr};

	// Tick Label
	Axis::LabelsFormat labelsFormat{Axis::LabelsFormat::Decimal};
	bool labelsFormatAuto{true};
	int labelsPrecision{1};
	bool labelsAutoPrecision{true};
	QString labelsDateTimeFormat;
	Axis::LabelsPosition labelsPosition{Axis::LabelsPosition::Out};
	Axis::LabelsTextType labelsTextType{Axis::LabelsTextType::PositionValues};
	const AbstractColumn* labelsTextColumn{nullptr};
	QString labelsTextColumnPath;
	qreal labelsRotationAngle{0};
	QColor labelsColor;
	QFont labelsFont;
	Axis::LabelsBackgroundType labelsBackgroundType{Axis::LabelsBackgroundType::Transparent};
	QColor labelsBackgroundColor;
	qreal labelsOffset{Worksheet::convertToSceneUnits(5.0, Worksheet::Unit::Point)}; //!< offset, distance to the end of the tick line (in page units)
	qreal labelsOpacity{1.0};
	QString labelsPrefix;
	QString labelsSuffix;

	bool colorBar{false};
	Heatmap* heatmap{nullptr};
	QString heatmapPath;
	double colorBarWidth{0};

	// Grid
	AxisGrid* gridItem{nullptr};
	Line* majorGridLine{nullptr};
	Line* minorGridLine{nullptr};

	Axis* const q{nullptr};

	QPainterPath linePath;
	QPainterPath majorGridPath;
	QPainterPath minorGridPath;

	QVector<QPointF> majorTickPoints; //!< position of the major ticks  on the axis.
	QVector<QPointF> minorTickPoints; //!< position of the minor ticks  on the axis.
	QVector<QPointF> tickLabelPoints; //!< position of the major tick labels (left lower edge of label's bounding rect)
	QVector<double> tickLabelValues; //!< major tick labels values
	QVector<QString> tickLabelValuesString; //!< major tick labels used when a custom text column is selected
	QVector<QString> tickLabelStrings; //!< the actual text of the major tick labels

private:
	CartesianPlot* plot() const {
		return m_plot; // convenience method
	}
	void mousePressEvent(QGraphicsSceneMouseEvent*) override;
	void mouseMoveEvent(QGraphicsSceneMouseEvent*) override;
	void mouseReleaseEvent(QGraphicsSceneMouseEvent*) override;
	void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget* widget = nullptr) override;

	void addArrow(QPointF point, int direction);
	int upperLabelsPrecision(int precision, Axis::LabelsFormat);
	int lowerLabelsPrecision(int precision, Axis::LabelsFormat);
	bool calculateTickHorizontal(Axis::TicksDirection tickDirection,
								 double ticksLength,
								 double xTickPos,
								 double yAnchorPos,
								 double centerValue,
								 int rangeDirection,
								 QPointF& anchorPointOut,
								 QPointF& startPointOut,
								 QPointF& endPointOut);
	bool calculateTickVertical(Axis::TicksDirection tickDirection,
							   double ticksLength,
							   double yTickPos,
							   double xAnchorPos,
							   double centerValue,
							   int rangeDirection,
							   QPointF& anchorPointOut,
							   QPointF& startPointOut,
							   QPointF& endPointOut);
	int determineMinorTicksNumber() const;
	static double calculateAutoParameters(int& majorTickCount, const Range<double>& r, double& spacing);
	static double calculateStartFromIncrement(double start, RangeT::Scale scale, double increment, bool* ok);
	static int calculateTicksNumberFromIncrement(double start, double end, RangeT::Scale scale, double increment);

	QPainterPath arrowPath;
	QPainterPath majorTicksPath;
	QPainterPath minorTicksPath;

	bool m_panningStarted{false};
	QPointF m_panningStart;

	friend class AxisTest;
};

#endif
