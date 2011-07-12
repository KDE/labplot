/***************************************************************************
    File                 : Axis.cpp
    Project              : LabPlot/SciDAVis
    Description          : Axis for cartesian coordinate systems.
    --------------------------------------------------------------------
    Copyright            : (C) 2009 Tilman Benkert (thzs*gmx.net)
    Copyright            : (C) 2011 Alexander Semke (alexander.semke*web.de)
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

#include "worksheet/Axis.h"
#include "worksheet/ScalableTextLabel.h"
#include "worksheet/AxisPrivate.h"
#include "worksheet/AbstractCoordinateSystem.h"
#include "worksheet/CartesianCoordinateSystem.h"
#include "lib/commandtemplates.h"
#include <QBrush>
#include <QPen>
#include <QPainter>
#include <QLocale>
#include <QFontMetricsF>
#include <QtDebug>

#include <math.h>

/**
 * \class Axis
 * \brief Axis for cartesian coordinate systems.
 *
 *  
 */

// TODO: decide whether it makes sense to move some of the functionality into a class AbstractAxis

Axis::Axis(const QString &name, const AxisOrientation &orientation)
		: AbstractWorksheetElement(name), d_ptr(new AxisPrivate(this)) {
	d_ptr->orientation = orientation;
	init();
}

Axis::Axis(const QString &name, const AxisOrientation &orientation, AxisPrivate *dd)
		: AbstractWorksheetElement(name), d_ptr(dd) {
	d_ptr->orientation = orientation;
	init();
}

void Axis::init() {
	Q_D(Axis);

	//TODO d_ptr->setFlag(QGraphicsItem::ItemIsSelectable, true);
	
	d->scale = ScaleLinear;
	d->offset = 0;
	d->start = 0;
	d->end = 10;
	d->scalingFactor = 1.0;
	d->zeroOffset = 0;
	
	d->lineOpacity = 1.0;
	
	d->majorTicksDirection = ticksOut;
	d->majorTicksType = TicksTotalNumber;
	d->majorTicksNumber = 11;
	d->majorTicksIncrement = 1.0;
	d->majorTicksLength = 0.5;
	d->majorTicksOpacity = 1.0;
	
	d->minorTicksDirection = ticksOut;
	d->minorTicksType = TicksTotalNumber;
	d->minorTicksNumber = 1;
	d->minorTicksIncrement = 0.5;
	d->minorTicksLength = 0.25;
	d->minorTicksOpacity = 1.0;
	
	d->numericFormat = 'f';
	d->displayedDigits = 1;
	
	d->labelsPosition = LabelsOut;
	d->labelsColor = QColor(Qt::black);
	d->labelsFontSize = 2;
	d->labelsRotationAngle = 0;
	d->labelsOpacity = 1.0;
	
	retransform();
}

Axis::~Axis() {
	delete d_ptr;
}

/* ============================ accessor documentation ================= */
/**
   \fn Axis::CLASS_D_ACCESSOR_DECL(AxisOrientation, orientation, Orientation);
   \brief Get/set the axis orientation: left, right, bottom, or top (usually not changed after creation).

   The orientation has not much to do with the actual position of the axis, which
   is determined by Axis::Private::offset. It only determines whether the axis
   is horizontal or vertical and which direction means in/out for the ticks.
 */
/**
   \fn Axis::BASIC_D_ACCESSOR_DECL(qreal, offset, Offset);
   \brief Get/set the offset from zero in the directin perpendicular to the axis.
 */
/**
   \fn Axis::BASIC_D_ACCESSOR_DECL(qreal, start, Start);
   \brief Get/set the start coordinate of the axis line.
 */
/**
   \fn Axis::BASIC_D_ACCESSOR_DECL(qreal, end, End);
   \brief Get/set the end coordinate of the axis line.
 */
/**
   \fn Axis::BASIC_D_ACCESSOR_DECL(int, majorTicksNumber, MajorTicksNumber);
   \brief Get/set the number of major ticks.
 */
/**
   \fn Axis::BASIC_D_ACCESSOR_DECL(int, minorTicksNumber, MinorTicksNumber);
   \brief Get/set the number of minor ticks (between each two major ticks).
 */
/**
   \fn Axis::BASIC_D_ACCESSOR_DECL(qreal, majorTicksLength, MajorTicksLength);
   \brief Get/set the major tick length (in page units!).
 */
/**
   \fn Axis::BASIC_D_ACCESSOR_DECL(qreal, minorTicksLength, MinorTicksLength);
   \brief Get/set the minor tick length (in page units!).
 */
/**
   \fn Axis::CLASS_D_ACCESSOR_DECL(TicksDirection, majorTicksDirection, MajorTicksDirection);
   \brief Get/set the major ticks direction: inwards, outwards, both, or none.
 */
/**
   \fn Axis::CLASS_D_ACCESSOR_DECL(TicksDirection, minorTicksDirection, MinorTicksDirection);
   \brief Get/set the minor ticks direction: inwards, outwards, both, or none.
 */
/**
   \fn Axis::BASIC_D_ACCESSOR_DECL(qreal, labelsRotationAngle, LabelRotationAngle);
   \brief Get/set the rotation angle of the tick labels.
 */
/**
   \fn Axis::BASIC_D_ACCESSOR_DECL(qreal, labelsFontSize, LabelFontSize);
   \brief Get/set the font size of the tick labels.
 */
/**
   \fn Axis::CLASS_D_ACCESSOR_DECL(QColor, labelsColor, LabelColor);
   \brief Get/set the color of the tick labels.
 */
/**
   \fn Axis::CLASS_D_ACCESSOR_DECL(QFont, labelsFont, LabelFont);
   \brief Get/set the font of the tick labels (size of the QFont will be ignored).
 */
/**
   \fn Axis::CLASS_D_ACCESSOR_DECL(QPointF, labelsOffset, LabelOffset);
   \brief Get/set the position offset of the tick labels relative to the end of the tick line.
 */
/**
   \fn Axis::CLASS_D_ACCESSOR_DECL(QPen, pen, Pen);
   \brief Get/set the pen for the lines.
 */

/* ============================ getter methods ================= */
CLASS_SHARED_D_READER_IMPL(Axis, Axis::AxisOrientation, orientation, orientation);
BASIC_SHARED_D_READER_IMPL(Axis, Axis::AxisScale, scale, scale);
BASIC_SHARED_D_READER_IMPL(Axis, qreal, offset, offset);
BASIC_SHARED_D_READER_IMPL(Axis, qreal, start, start);
BASIC_SHARED_D_READER_IMPL(Axis, qreal, end, end);
BASIC_SHARED_D_READER_IMPL(Axis, qreal, scalingFactor, scalingFactor);
BASIC_SHARED_D_READER_IMPL(Axis, qreal, zeroOffset, zeroOffset);

CLASS_SHARED_D_READER_IMPL(Axis, QPen, linePen, linePen);
BASIC_SHARED_D_READER_IMPL(Axis, qreal, lineOpacity, lineOpacity);

CLASS_SHARED_D_READER_IMPL(Axis, Axis::TicksDirection, majorTicksDirection, majorTicksDirection);
CLASS_SHARED_D_READER_IMPL(Axis, Axis::TicksType, majorTicksType, majorTicksType);
BASIC_SHARED_D_READER_IMPL(Axis, int, majorTicksNumber, majorTicksNumber);
BASIC_SHARED_D_READER_IMPL(Axis, qreal, majorTicksIncrement, majorTicksIncrement);
BASIC_SHARED_D_READER_IMPL(Axis, qreal, majorTicksLength, majorTicksLength);
CLASS_SHARED_D_READER_IMPL(Axis, QPen, majorTicksPen, majorTicksPen);
BASIC_SHARED_D_READER_IMPL(Axis, qreal, majorTicksOpacity, majorTicksOpacity);

CLASS_SHARED_D_READER_IMPL(Axis, Axis::TicksDirection, minorTicksDirection, minorTicksDirection);
CLASS_SHARED_D_READER_IMPL(Axis, Axis::TicksType, minorTicksType, minorTicksType);
BASIC_SHARED_D_READER_IMPL(Axis, int, minorTicksNumber, minorTicksNumber);
BASIC_SHARED_D_READER_IMPL(Axis, qreal, minorTicksIncrement, minorTicksIncrement);
BASIC_SHARED_D_READER_IMPL(Axis, qreal, minorTicksLength, minorTicksLength);
CLASS_SHARED_D_READER_IMPL(Axis, QPen, minorTicksPen, minorTicksPen);
BASIC_SHARED_D_READER_IMPL(Axis, qreal, minorTicksOpacity, minorTicksOpacity);

CLASS_SHARED_D_READER_IMPL(Axis, Axis::LabelsPosition, labelsPosition, labelsPosition);
CLASS_SHARED_D_READER_IMPL(Axis, QPointF, labelsOffset, labelsOffset);
BASIC_SHARED_D_READER_IMPL(Axis, qreal, labelsRotationAngle, labelsRotationAngle);
BASIC_SHARED_D_READER_IMPL(Axis, qreal, labelsFontSize, labelsFontSize);
CLASS_SHARED_D_READER_IMPL(Axis, QColor, labelsColor, labelsColor);
CLASS_SHARED_D_READER_IMPL(Axis, QFont, labelsFont, labelsFont);
CLASS_SHARED_D_READER_IMPL(Axis, QString, labelsPrefix, labelsPrefix);
CLASS_SHARED_D_READER_IMPL(Axis, QString, labelsSuffix, labelsSuffix);
BASIC_SHARED_D_READER_IMPL(Axis, qreal, labelsOpacity, labelsOpacity);

/* ============================ setter methods and undo commands ================= */

STD_SWAP_METHOD_SETTER_CMD_IMPL(Axis, SetVisible, bool, swapVisible);
void Axis::setVisible(bool on) {
	Q_D(Axis);
	exec(new AxisSetVisibleCmd(d, on, on ? tr("%1: set visible") : tr("%1: set invisible")));
}

bool Axis::isVisible() const {
	Q_D(const Axis);
	return d->isVisible();
}

STD_SETTER_CMD_IMPL_F(Axis, SetOrientation, Axis::AxisOrientation, orientation, retransform);
void Axis::setOrientation( AxisOrientation orientation) {
	Q_D(Axis);
	if (orientation != d->orientation)
		exec(new AxisSetOrientationCmd(d, orientation, tr("%1: set axis orientation")));
}

STD_SETTER_CMD_IMPL_F(Axis, SetScaling, Axis::AxisScale, scale, retransform);
void Axis::setScale(AxisScale scale) {
	Q_D(Axis);
	if (scale != d->scale)
		exec(new AxisSetScalingCmd(d, scale, tr("%1: set axis scale")));
}

STD_SETTER_CMD_IMPL_F(Axis, SetOffset, qreal, offset, retransform);
void Axis::setOffset(qreal offset) {
	Q_D(Axis);
	if (offset != d->offset)
		exec(new AxisSetOffsetCmd(d, offset, tr("%1: set axis offset")));
}

STD_SETTER_CMD_IMPL_F(Axis, SetStart, qreal, start, retransform);
void Axis::setStart(qreal start) {
	Q_D(Axis);
	if (start != d->start)
		exec(new AxisSetStartCmd(d, start, tr("%1: set axis start")));
}

STD_SETTER_CMD_IMPL_F(Axis, SetEnd, qreal, end, retransform);
void Axis::setEnd(qreal end) {
	Q_D(Axis);
	if (end != d->end)
		exec(new AxisSetEndCmd(d, end, tr("%1: set axis end")));
}

//TODO undo-functions
STD_SETTER_CMD_IMPL_F(Axis, SetScalingFactor, qreal, scalingFactor, retransform);
void Axis::setScalingFactor(qreal scalingFactor) {
	Q_D(Axis);
	if (scalingFactor != d->scalingFactor)
		exec(new AxisSetScalingFactorCmd(d, scalingFactor, tr("%1: set axis scaling factor")));
}

STD_SETTER_CMD_IMPL_F(Axis, SetZeroOffset, qreal, zeroOffset, retransform);
void Axis::setZeroOffset(qreal zeroOffset) {
	Q_D(Axis);
	if (zeroOffset != d->zeroOffset)
		exec(new AxisSetZeroOffsetCmd(d, zeroOffset, tr("%1: set axis zero offset")));
}


//Line
STD_SETTER_CMD_IMPL_F(Axis, SetLinePen, QPen, linePen, recalcShapeAndBoundingRect);
void Axis::setLinePen(const QPen &pen) {
	Q_D(Axis);
	if (pen != d->linePen)
		exec(new AxisSetLinePenCmd(d, pen, tr("%1: set line style")));
}

STD_SETTER_CMD_IMPL_F(Axis, SetLineOpacity, qreal, lineOpacity, update);
void Axis::setLineOpacity(qreal opacity){
	Q_D(Axis);
	if (opacity != d->lineOpacity)
		exec(new AxisSetLineOpacityCmd(d, opacity, tr("%1: set line opacity")));
}

//Major ticks
STD_SETTER_CMD_IMPL_F(Axis, SetMajorTicksDirection, Axis::TicksDirection, majorTicksDirection, retransformTicks);
void Axis::setMajorTicksDirection(const TicksDirection &majorTicksDirection) {
	Q_D(Axis);
	if (majorTicksDirection != d->majorTicksDirection)
		exec(new AxisSetMajorTicksDirectionCmd(d, majorTicksDirection, tr("%1: set major ticks direction")));
}

STD_SETTER_CMD_IMPL_F(Axis, SetMajorTicksType, Axis::TicksType, majorTicksType, retransformTicks);
void Axis::setMajorTicksType(const TicksType &majorTicksType) {
	Q_D(Axis);
	if (majorTicksType!= d->majorTicksType)
		exec(new AxisSetMajorTicksTypeCmd(d, majorTicksType, tr("%1: set major ticks type")));
}

STD_SETTER_CMD_IMPL_F(Axis, SetMajorTicksNumber, int, majorTicksNumber, retransformTicks);
void Axis::setMajorTicksNumber(int majorTicksNumber) {
	Q_D(Axis);
	if (majorTicksNumber != d->majorTicksNumber)
		exec(new AxisSetMajorTicksNumberCmd(d, majorTicksNumber, tr("%1: set the total number of the major ticks")));
}

STD_SETTER_CMD_IMPL_F(Axis, SetMajorTicksIncrement, qreal, majorTicksIncrement, retransformTicks);
void Axis::setMajorTicksIncrement(qreal majorTicksIncrement) {
	Q_D(Axis);
	if (majorTicksIncrement != d->majorTicksIncrement)
		exec(new AxisSetMajorTicksIncrementCmd(d, majorTicksIncrement, tr("%1: set the increment for the major ticks")));
}

STD_SETTER_CMD_IMPL_F(Axis, SetMajorTicksPen, QPen, majorTicksPen, recalcShapeAndBoundingRect);
void Axis::setMajorTicksPen(const QPen &pen) {
	Q_D(Axis);
	if (pen != d->majorTicksPen)
		exec(new AxisSetMajorTicksPenCmd(d, pen, tr("%1: set major ticks style")));
}

STD_SETTER_CMD_IMPL_F(Axis, SetMajorTicksLength, qreal, majorTicksLength, retransformTicks);
void Axis::setMajorTicksLength(qreal majorTicksLength) {
	Q_D(Axis);
	if (majorTicksLength != d->majorTicksLength)
		exec(new AxisSetMajorTicksLengthCmd(d, majorTicksLength, tr("%1: set major ticks length")));
}

STD_SETTER_CMD_IMPL_F(Axis, SetMajorTicksOpacity, qreal, majorTicksOpacity, update);
void Axis::setMajorTicksOpacity(qreal opacity){
	Q_D(Axis);
	if (opacity != d->majorTicksOpacity)
		exec(new AxisSetMajorTicksOpacityCmd(d, opacity, tr("%1: set major ticks opacity")));
}


//Minor ticks
STD_SETTER_CMD_IMPL_F(Axis, SetMinorTicksDirection, Axis::TicksDirection, minorTicksDirection, retransformTicks);
void Axis::setMinorTicksDirection(const TicksDirection &minorTicksDirection) {
	Q_D(Axis);
	if (minorTicksDirection != d->minorTicksDirection)
		exec(new AxisSetMinorTicksDirectionCmd(d, minorTicksDirection, tr("%1: set minor ticks direction")));
}

STD_SETTER_CMD_IMPL_F(Axis, SetMinorTicksType, Axis::TicksType, minorTicksType, retransformTicks);
void Axis::setMinorTicksType(const TicksType &minorTicksType) {
	Q_D(Axis);
	if (minorTicksType!= d->minorTicksType)
		exec(new AxisSetMinorTicksTypeCmd(d, minorTicksType, tr("%1: set minor ticks type")));
}

STD_SETTER_CMD_IMPL_F(Axis, SetMinorTicksNumber, int, minorTicksNumber, retransformTicks);
void Axis::setMinorTicksNumber(int minorTicksNumber) {
	Q_D(Axis);
	if (minorTicksNumber != d->minorTicksNumber)
		exec(new AxisSetMinorTicksNumberCmd(d, minorTicksNumber, tr("%1: set the total number of the minor ticks")));
}

STD_SETTER_CMD_IMPL_F(Axis, SetMinorTicksIncrement, qreal, minorTicksIncrement, retransformTicks);
void Axis::setMinorTicksIncrement(qreal minorTicksIncrement) {
	Q_D(Axis);
	if (minorTicksIncrement != d->minorTicksIncrement)
		exec(new AxisSetMinorTicksIncrementCmd(d, minorTicksIncrement, tr("%1: set the increment for the minor ticks")));
}

STD_SETTER_CMD_IMPL_F(Axis, SetMinorTicksPen, QPen, minorTicksPen, recalcShapeAndBoundingRect);
void Axis::setMinorTicksPen(const QPen &pen) {
	Q_D(Axis);
	if (pen != d->minorTicksPen)
		exec(new AxisSetMinorTicksPenCmd(d, pen, tr("%1: set minor ticks style")));
}

STD_SETTER_CMD_IMPL_F(Axis, SetMinorTicksLength, qreal, minorTicksLength, retransformTicks);
void Axis::setMinorTicksLength(qreal minorTicksLength) {
	Q_D(Axis);
	if (minorTicksLength != d->minorTicksLength)
		exec(new AxisSetMinorTicksLengthCmd(d, minorTicksLength, tr("%1: set minor ticks length")));
}

STD_SETTER_CMD_IMPL_F(Axis, SetMinorTicksOpacity, qreal, minorTicksOpacity, update);
void Axis::setMinorTicksOpacity(qreal opacity){
	Q_D(Axis);
	if (opacity != d->minorTicksOpacity)
		exec(new AxisSetMinorTicksOpacityCmd(d, opacity, tr("%1: set minor ticks opacity")));
}


//Labels
STD_SETTER_CMD_IMPL_F(Axis, SetLabelsPosition, Axis::LabelsPosition, labelsPosition, retransformTicks);
void Axis::setLabelsPosition(const LabelsPosition & labelsPosition) {
	Q_D(Axis);
	if (labelsPosition != d->labelsPosition)
		exec(new AxisSetLabelsPositionCmd(d, labelsPosition, tr("%1: set labels position")));
}

STD_SWAP_METHOD_SETTER_CMD_IMPL(Axis, SetLabelsOffset, QPointF, swapLabelOffset);
void Axis::setLabelsOffset(const QPointF &newOffset) {
	Q_D(Axis);
	if (newOffset != d->labelsOffset)
		exec(new AxisSetLabelsOffsetCmd(d, newOffset, tr("%1: set label offset")));
}

STD_SETTER_CMD_IMPL_F(Axis, SetLabelsRotationAngle, qreal, labelsRotationAngle, restyleLabels);
void Axis::setLabelsRotationAngle(qreal angle) {
	Q_D(Axis);
	if (angle != d->labelsRotationAngle)
		exec(new AxisSetLabelsRotationAngleCmd(d, angle, tr("%1: set label rotation angle")));
}

STD_SETTER_CMD_IMPL_F(Axis, SetLabelsFontSize, qreal, labelsFontSize, restyleLabels);
void Axis::setLabelsFontSize(qreal size) {
	Q_D(Axis);
	if (size != d->labelsFontSize)
		exec(new AxisSetLabelsFontSizeCmd(d, size, tr("%1: set label font size")));
}

STD_SETTER_CMD_IMPL_F(Axis, SetLabelsColor, QColor, labelsColor, restyleLabels);
void Axis::setLabelsColor(const QColor &color) {
	Q_D(Axis);
	if (color != d->labelsColor)
		exec(new AxisSetLabelsColorCmd(d, color, tr("%1: set label color")));
}

STD_SETTER_CMD_IMPL_F(Axis, SetLabelsFont, QFont, labelsFont, restyleLabels);
void Axis::setLabelsFont(const QFont &font) {
	Q_D(Axis);
	if (font != d->labelsFont)
		exec(new AxisSetLabelsFontCmd(d, font, tr("%1: set label font")));
}

//TODO setPrefix/Suffix triggers retransformTricks -> redisign
STD_SETTER_CMD_IMPL_F(Axis, SetLabelsPrefix, QString, labelsPrefix, retransformTicks);
void Axis::setLabelsPrefix(const QString& prefix) {
	Q_D(Axis);
	if (prefix != d->labelsPrefix)
		exec(new AxisSetLabelsPrefixCmd(d, prefix, tr("%1: set label prefix")));
}

STD_SETTER_CMD_IMPL_F(Axis, SetLabelsSuffix, QString, labelsSuffix, retransformTicks);
void Axis::setLabelsSuffix(const QString& suffix) {
	Q_D(Axis);
	if (suffix != d->labelsSuffix)
		exec(new AxisSetLabelsSuffixCmd(d, suffix, tr("%1: set label suffix")));
}

STD_SETTER_CMD_IMPL_F(Axis, SetLabelsOpacity, qreal, labelsOpacity, update);
void Axis::setLabelsOpacity(qreal opacity){
	Q_D(Axis);
	if (opacity != d->labelsOpacity)
		exec(new AxisSetLabelsOpacityCmd(d, opacity, tr("%1: set labels opacity")));
}

/* ============================ other methods ================= */

QGraphicsItem *Axis::graphicsItem() const {
	return d_ptr;
}

bool Axis::Private::swapVisible(bool on) {
	bool oldValue = isVisible();
	setVisible(on);
	return oldValue;
}

QPointF Axis::Private::swapLabelOffset(const QPointF &newOffset)
{
	QPointF oldOffset = labelsOffset;
	labelsOffset = newOffset;
	recalcShapeAndBoundingRect();
	return oldOffset;
}


void Axis::retransform() {
	Q_D(Axis);
	d->retransform();
}


//Private implementation 

void Axis::Private::retransform() {
	const AbstractCoordinateSystem *cSystem = q->coordinateSystem();

	linePath = QPainterPath();

	QList<QLineF> lines;
	QPointF startPoint;
	QPointF endPoint;

	if (orientation == Axis::AxisHorizontal) {
		startPoint.setX(start);
		startPoint.setY(offset);
		endPoint.setX(end);
		endPoint.setY(offset);
	} else { // vertical
		startPoint.setX(offset);
		startPoint.setY(start);
		endPoint.setX(offset);
		endPoint.setY(end);
	}

	lines.append(QLineF(startPoint, endPoint));
	if (cSystem) {
		lines = cSystem->mapLogicalToScene(lines, AbstractCoordinateSystem::MarkGaps);
	} 

	foreach (QLineF line, lines) {
		linePath.moveTo(line.p1());
		linePath.lineTo(line.p2());
	}

	retransformTicks(cSystem);
}

void Axis::Private::retransformTicks() {
	retransformTicks(q->coordinateSystem());
}

//! helper function for retransformTicks(const AbstractCoordinateSystem *cSystem)
bool Axis::Private::transformAnchor(const AbstractCoordinateSystem *cSystem, QPointF *anchorPoint) {
	if (cSystem) {
		QList<QPointF> points;
		points.append(*anchorPoint);
		points = cSystem->mapLogicalToScene(points, AbstractCoordinateSystem::SuppressPageClipping);
		if (points.count() != 1) // point is not mappable or in a coordinate gap
			return false;
		else
			*anchorPoint = points.at(0);
	}
	return true;
}

void Axis::Private::retransformTicks(const AbstractCoordinateSystem *cSystem) {
	const CartesianCoordinateSystem *cCSystem = qobject_cast<const CartesianCoordinateSystem *>(cSystem);

	majorTicksPath = QPainterPath();
	minorTicksPath = QPainterPath();
	qDeleteAll(labels);
	labels.clear();

  
  if (majorTicksNumber<1 || (majorTicksDirection == Axis::noTicks && minorTicksDirection == Axis::noTicks) ){
	recalcShapeAndBoundingRect();
	return;
  }
  
  //determine the spacing for the major ticks
  double majorTicksSpacing;
  int tmpMajorTicksNumber;
  if (majorTicksType == Axis::TicksTotalNumber){
	//the total number of the major ticks is given - > determine the spacing
	  switch (scale){
		case Axis::ScaleLinear:
		  majorTicksSpacing = (end - start)/ (majorTicksNumber - 1);
		  break;
		case Axis::ScaleLog10:
		  majorTicksSpacing = (log10(end) - log10(start))/(majorTicksNumber - 1);
		  break;
		case Axis::ScaleLog2:
		  majorTicksSpacing = (log(end) - log(start))/log(2)/(majorTicksNumber - 1);
		  break;
		case Axis::ScaleLn:
		  majorTicksSpacing = (log(end) - log(start))/(majorTicksNumber - 1);
		  break;		
		case Axis::ScaleSqrt:
		  majorTicksSpacing = (sqrt(end) - sqrt(start))/(majorTicksNumber - 1);
		  break;					
		case Axis::ScaleX2:
		  majorTicksSpacing = (pow(end,2) - pow(start,2))/(majorTicksNumber - 1);
		  break;			
		default://Linear
		  majorTicksSpacing = (end - start)/(majorTicksNumber - 1);
	  }
	  tmpMajorTicksNumber = majorTicksNumber;
  }else{
	//the spacing (increment ) of the major ticks is given - > determine the number
	majorTicksSpacing = majorTicksIncrement;
	switch (scale){
	  case Axis::ScaleLinear:
		tmpMajorTicksNumber = (end-start)/majorTicksSpacing + 1;
		break;
	  case Axis::ScaleLog10:
		tmpMajorTicksNumber = (log10(end)-log10(start))/majorTicksSpacing + 1;
		break;
	  case Axis::ScaleLog2:
		tmpMajorTicksNumber = (log(end)-log(start))/log(2)/majorTicksSpacing + 1;
		break;
	  case Axis::ScaleLn:
		tmpMajorTicksNumber = (log(end)-log(start))/majorTicksSpacing + 1;
		break;		
	  case Axis::ScaleSqrt:
		tmpMajorTicksNumber = (sqrt(end)-sqrt(start))/majorTicksSpacing + 1;
		break;					
	  case Axis::ScaleX2:
		tmpMajorTicksNumber = (pow(end,2)-pow(start,2))/majorTicksSpacing + 1;
		break;			
	  default://Linear
		tmpMajorTicksNumber = (end-start)/majorTicksSpacing + 1;
	}
  }
  
  int tmpMinorTicksNumber;
	if (minorTicksType == Axis::TicksTotalNumber){
	  tmpMinorTicksNumber = minorTicksNumber;
  }else{
	  tmpMinorTicksNumber = (end - start)/ (majorTicksNumber - 1)/minorTicksIncrement - 1;
  }
  
  int xDirection = 1;
  int yDirection = 1;
  if (cCSystem) {
	  xDirection = cCSystem->xDirection();
	  yDirection = cCSystem->yDirection();
  }

  QPointF anchorPoint;
  QPointF startPoint;
  QPointF endPoint;
  QPointF textPos;
  qreal majorTickPos;
  qreal minorTickPos;
  qreal nextMajorTickPos;
	
	for (int iMajor = 0; iMajor < tmpMajorTicksNumber; iMajor++){
	  switch (scale){
		case Axis::ScaleLinear:
		  majorTickPos = start + majorTicksSpacing * (qreal)iMajor;
		  break;
		case Axis::ScaleLog10:
		  majorTickPos = pow(10, log10(start) + majorTicksSpacing * (qreal)iMajor);
		  break;
		case Axis::ScaleLog2:
		  majorTickPos = pow(2, log(start)/log(2) + majorTicksSpacing * (qreal)iMajor);
		  break;
		case Axis::ScaleLn:
		  majorTickPos = exp(log(start) + majorTicksSpacing * (qreal)iMajor);
		  break;
		case Axis::ScaleSqrt:
		  majorTickPos = pow(sqrt(start) + majorTicksSpacing * (qreal)iMajor, 2);
		  break;
		case Axis::ScaleX2:
		  majorTickPos = sqrt(sqrt(start) + majorTicksSpacing * (qreal)iMajor);
		  break;		  
		default://Linear
		  majorTickPos = start + majorTicksSpacing * (qreal)iMajor; 
	  }
  
	  if (majorTicksDirection != Axis::noTicks ){
		  if (orientation == Axis::AxisHorizontal){
			  anchorPoint.setX(majorTickPos);
			  anchorPoint.setY(offset);
			  
			  if (transformAnchor(cSystem, &anchorPoint)){
				startPoint = anchorPoint + QPointF(0, (majorTicksDirection & Axis::ticksIn)  ? yDirection * majorTicksLength  : 0);
				endPoint   = anchorPoint + QPointF(0, (majorTicksDirection & Axis::ticksOut) ? -yDirection * majorTicksLength : 0);
			}
		  }else{ // vertical
			  anchorPoint.setY(majorTickPos);
			  anchorPoint.setX(offset);

			  if (transformAnchor(cSystem, &anchorPoint)){
				startPoint = anchorPoint + QPointF((majorTicksDirection & Axis::ticksIn)  ? xDirection * majorTicksLength  : 0, 0);
				endPoint = anchorPoint + QPointF((majorTicksDirection & Axis::ticksOut) ? -xDirection * majorTicksLength : 0, 0);
			  }
		  }
		  
		  majorTicksPath.moveTo(startPoint);
		  majorTicksPath.lineTo(endPoint);
		  
		  if (labelsPosition != Axis::NoLabels){
			if (labelsPosition == Axis::LabelsOut)
			  textPos = endPoint;
			else
			  textPos = startPoint;
			
			addTextLabel( labelsPrefix + QLocale().toString(scalingFactor*majorTickPos+zeroOffset, numericFormat, displayedDigits) + labelsSuffix, textPos );
		  }	
	  }

	  //minor ticks
	  if ((Axis::noTicks != minorTicksDirection) && (tmpMajorTicksNumber > 1) && (tmpMinorTicksNumber > 0) && (iMajor < tmpMajorTicksNumber - 1)) {
		  for (int iMinor = 0; iMinor < tmpMinorTicksNumber; iMinor++) {
			switch (scale){
			  case Axis::ScaleLinear:
				minorTickPos = majorTickPos + (qreal)(iMinor + 1) * majorTicksSpacing / (qreal)(tmpMinorTicksNumber + 1);
				break;
			  case Axis::ScaleLog10:
				nextMajorTickPos = start + pow(10, majorTicksSpacing * (qreal)(iMajor + 1));
				minorTickPos = majorTickPos + (qreal)(iMinor + 1) * (nextMajorTickPos - majorTickPos) / (qreal)(tmpMinorTicksNumber + 1);
				break;
			  case Axis::ScaleLog2:
				nextMajorTickPos = start + pow(2, majorTicksSpacing * (qreal)(iMajor + 1));
				minorTickPos = majorTickPos + (qreal)(iMinor + 1) * (nextMajorTickPos - majorTickPos) / (qreal)(tmpMinorTicksNumber + 1);
				break;
			  case Axis::ScaleLn:
				nextMajorTickPos = start + exp(majorTicksSpacing * (qreal)(iMajor + 1));
				minorTickPos = majorTickPos + (qreal)(iMinor + 1) * (nextMajorTickPos - majorTickPos) / (qreal)(tmpMinorTicksNumber + 1);
				break;
			  case Axis::ScaleSqrt:
				nextMajorTickPos = start + pow(majorTicksSpacing * (qreal)(iMajor + 1), 2);
				minorTickPos = majorTickPos + (qreal)(iMinor + 1) * (nextMajorTickPos - majorTickPos) / (qreal)(tmpMinorTicksNumber + 1);
				break;
			  case Axis::ScaleX2:
				nextMajorTickPos = start + sqrt(majorTicksSpacing * (qreal)(iMajor + 1));
				minorTickPos = majorTickPos + (qreal)(iMinor + 1) * (nextMajorTickPos - majorTickPos) / (qreal)(tmpMinorTicksNumber + 1);
				break;
			  default://Linear
				minorTickPos = majorTickPos + (qreal)(iMinor + 1) * majorTicksSpacing / (qreal)(tmpMinorTicksNumber + 1);
			}
			  
			if (orientation == Axis::AxisHorizontal){
			  anchorPoint.setX(minorTickPos);
			  anchorPoint.setY(offset);

				if (transformAnchor(cSystem, &anchorPoint)){
				  startPoint = anchorPoint + QPointF(0, (minorTicksDirection & Axis::ticksIn)  ? yDirection * minorTicksLength  : 0);
				  endPoint   = anchorPoint + QPointF(0, (minorTicksDirection & Axis::ticksOut) ? -yDirection * minorTicksLength : 0);
				}
			}else{ // vertical
				anchorPoint.setY(minorTickPos);
				anchorPoint.setX(offset);

				if (transformAnchor(cSystem, &anchorPoint)){
				  startPoint = anchorPoint + QPointF((minorTicksDirection & Axis::ticksIn)  ? xDirection * minorTicksLength  : 0, 0);
				  endPoint   = anchorPoint + QPointF((minorTicksDirection & Axis::ticksOut) ? -xDirection * minorTicksLength : 0, 0);
				}
			}
			
			minorTicksPath.moveTo(startPoint);
			minorTicksPath.lineTo(endPoint);
		  }
	  }
	}

	restyleLabels(); // this calls recalcShapeAndBoundingRect()
}

void Axis::Private::recalcShapeAndBoundingRect() {
	prepareGeometryChange();

	boundingRectangle = linePath.boundingRect();
	boundingRectangle |= majorTicksPath.boundingRect();
	boundingRectangle |= minorTicksPath.boundingRect();

	boundingRectangle = boundingRectangle.normalized();

	axisShape = AbstractWorksheetElement::shapeFromPath(linePath, pen);
	axisShape.addPath(AbstractWorksheetElement::shapeFromPath(majorTicksPath, pen));
	axisShape.addPath(AbstractWorksheetElement::shapeFromPath(minorTicksPath, pen));

	foreach (ScalableTextLabel *textLabel, labels) {
		QRectF rect = textLabel->boundingRect();
		rect.translate(labelsOffset);
		boundingRectangle |= rect;
		axisShape.addRect(rect);
	}
	
	update();
}

void Axis::Private::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget * widget) {
	Q_UNUSED(option)
	Q_UNUSED(widget)

  if (!isVisible())
	return;
  
  //draw the line
  if (linePen.style() != Qt::NoPen){
	painter->setOpacity(lineOpacity);
	painter->setPen(linePen);
	painter->setBrush(Qt::NoBrush);
	painter->drawPath(linePath);
  }

  //draw the major ticks
  if (majorTicksDirection != Axis::noTicks){
	painter->setOpacity(majorTicksOpacity);
	painter->setPen(majorTicksPen);
	painter->setBrush(Qt::NoBrush);
	painter->drawPath(majorTicksPath);
  }
  
  //draw the minor ticks
  if (minorTicksDirection != Axis::noTicks){
	painter->setOpacity(minorTicksOpacity);
	painter->setPen(minorTicksPen);
	painter->setBrush(Qt::NoBrush);
	painter->drawPath(minorTicksPath);
  }
  
  if (labelsPosition != Axis::NoLabels){
	painter->setOpacity(labelsOpacity);
	painter->translate(labelsOffset);
	foreach (ScalableTextLabel *textLabel, labels)
	  textLabel->paint(painter);
	
	painter->translate(-labelsOffset);
  }
}


void AxisPrivate::addTextLabel(const QString &text, const QPointF &pos) {
  ScalableTextLabel *label = new ScalableTextLabel();
  label->setText(text);
  label->setPosition(pos);

  ScalableTextLabel::HorizontalAlignment hAlign = ScalableTextLabel::hAlignCenter;
  ScalableTextLabel::VerticalAlignment vAlign = ScalableTextLabel::vAlignCenter;

  if (orientation == Axis::AxisHorizontal){
	  if (labelsPosition == Axis::LabelsOut)
		vAlign =  ScalableTextLabel::vAlignTop;
	  else
		vAlign =  ScalableTextLabel::vAlignBottom;
  }else{
	  if (labelsPosition == Axis::LabelsOut)
		hAlign =  ScalableTextLabel::hAlignRight;
	  else
		hAlign =  ScalableTextLabel::hAlignLeft;
  }
  
  label->setAlignment(hAlign, vAlign);
  labels.append(label);
}

void AxisPrivate::restyleLabels() {
	foreach(ScalableTextLabel *label, labels) {
		label->setFontSize(labelsFontSize);
		label->setRotationAngle(labelsRotationAngle);
		label->setFont(labelsFont);
		label->setTextColor(labelsColor);
	}
	recalcShapeAndBoundingRect();
}

void Axis::handlePageResize(double horizontalRatio, double verticalRatio) {
	Q_D(Axis);

	QPen pen = d->linePen;
	pen.setWidthF(pen.widthF() * (horizontalRatio + verticalRatio) / 2.0);
	setLinePen(pen);

	if (d->orientation & Axis::AxisHorizontal) {
		setMajorTicksLength(d->majorTicksLength * verticalRatio); // ticks are perpendicular to axis line -> verticalRatio relevant
		setMinorTicksLength(d->minorTicksLength * verticalRatio);
		setLabelsFontSize(d->labelsFontSize * verticalRatio);
	} else {
		setMajorTicksLength(d->majorTicksLength * horizontalRatio);
		setMinorTicksLength(d->minorTicksLength * horizontalRatio);
		setLabelsFontSize(d->labelsFontSize * verticalRatio); // this is not perfectly correct for rotated labels 
															// when the page aspect ratio changes, but should not matter
	}
	setLabelsOffset(QPointF(d->labelsOffset.x() * horizontalRatio, d->labelsOffset.y() * verticalRatio));

	BaseClass::handlePageResize(horizontalRatio, verticalRatio);
}
