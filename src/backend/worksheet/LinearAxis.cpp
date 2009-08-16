/***************************************************************************
    File                 : LinearAxis.cpp
    Project              : LabPlot/SciDAVis
    Description          : Linear axis for cartesian coordinate systems.
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

#include "worksheet/LinearAxis.h"
#include "worksheet/ScalableTextLabel.h"
#include "worksheet/LinearAxisPrivate.h"
#include "worksheet/AbstractCoordinateSystem.h"
#include "worksheet/CartesianCoordinateSystem.h"
#include "lib/commandtemplates.h"
#include <QBrush>
#include <QPen>
#include <QPainter>
#include <QLocale>
#include <QFontMetricsF>
#include <QtDebug>

/**
 * \class LinearAxis
 * \brief Linear axis for cartesian coordinate systems.
 *
 *  
 */

// TODO: decide whether it makes sense to move some of the functionality into a class AbstractAxis

LinearAxis::LinearAxis(const QString &name, const AxisOrientation &orientation)
		: AbstractWorksheetElement(name), d_ptr(new LinearAxisPrivate(this)) {
	d_ptr->orientation = orientation;
	init();
}

LinearAxis::LinearAxis(const QString &name, const AxisOrientation &orientation, LinearAxisPrivate *dd)
		: AbstractWorksheetElement(name), d_ptr(dd) {
	d_ptr->orientation = orientation;
	init();
}

void LinearAxis::init() {
	Q_D(LinearAxis);

	d->offset = 0;
	d->start = 0;
	d->end = 10;
	d->tickStart = 0;
	d->tickEnd = 10;
	d->majorTickCount = 11;
	d->minorTickCount = 1;
	d->majorTicksLength = 0.5;
	d->minorTicksLength = 0.25;
	d->majorTicksDirection = ticksOut;
	d->minorTicksDirection = ticksOut;
	d->numericFormat = 'f';
	d->displayedDigits = 1;
	d->labelColor = QColor(Qt::black);
	d->labelFontSize = 2;
	d->labelRotationAngle = 0;
	retransform();
}

LinearAxis::~LinearAxis() {
	delete d_ptr;
}

/* ============================ accessor documentation ================= */
/**
   \fn LinearAxis::CLASS_D_ACCESSOR_DECL(AxisOrientation, orientation, Orientation);
   \brief Get/set the axis orientation: left, right, bottom, or top (usually not changed after creation).

   The orientation has not much to do with the actual position of the axis, which
   is determined by LinearAxis::Private::offset. It only determines whether the axis
   is horizontal or vertical and which direction means in/out for the ticks.
 */
/**
   \fn LinearAxis::BASIC_D_ACCESSOR_DECL(qreal, offset, Offset);
   \brief Get/set the offset from zero in the directin perpendicular to the axis.
 */
/**
   \fn LinearAxis::BASIC_D_ACCESSOR_DECL(qreal, start, Start);
   \brief Get/set the start coordinate of the axis line.
 */
/**
   \fn LinearAxis::BASIC_D_ACCESSOR_DECL(qreal, end, End);
   \brief Get/set the end coordinate of the axis line.
 */
/**
   \fn LinearAxis::BASIC_D_ACCESSOR_DECL(qreal, tickStart, TickStart);
   \brief Get/set the coordinate of the first tick (typically ==0 or ==start).
 */
/**
   \fn LinearAxis::BASIC_D_ACCESSOR_DECL(qreal, tickEnd, TickEnd);
   \brief Get/set the coordinate of the last tick (typically ==end).
 */
/**
   \fn LinearAxis::BASIC_D_ACCESSOR_DECL(int, majorTickCount, MajorTickCount);
   \brief Get/set the number of major ticks.
 */
/**
   \fn LinearAxis::BASIC_D_ACCESSOR_DECL(int, minorTickCount, MinorTickCount);
   \brief Get/set the number of minor ticks (between each two major ticks).
 */
/**
   \fn LinearAxis::BASIC_D_ACCESSOR_DECL(qreal, majorTicksLength, MajorTicksLength);
   \brief Get/set the major tick length (in page units!).
 */
/**
   \fn LinearAxis::BASIC_D_ACCESSOR_DECL(qreal, minorTicksLength, MinorTicksLength);
   \brief Get/set the minor tick length (in page units!).
 */
/**
   \fn LinearAxis::CLASS_D_ACCESSOR_DECL(TicksDirection, majorTicksDirection, MajorTicksDirection);
   \brief Get/set the major ticks direction: inwards, outwards, both, or none.
 */
/**
   \fn LinearAxis::CLASS_D_ACCESSOR_DECL(TicksDirection, minorTicksDirection, MinorTicksDirection);
   \brief Get/set the minor ticks direction: inwards, outwards, both, or none.
 */
/**
   \fn LinearAxis::BASIC_D_ACCESSOR_DECL(qreal, labelRotationAngle, LabelRotationAngle);
   \brief Get/set the rotation angle of the tick labels.
 */
/**
   \fn LinearAxis::BASIC_D_ACCESSOR_DECL(qreal, labelFontSize, LabelFontSize);
   \brief Get/set the font size of the tick labels.
 */
/**
   \fn LinearAxis::CLASS_D_ACCESSOR_DECL(QColor, labelColor, LabelColor);
   \brief Get/set the color of the tick labels.
 */
/**
   \fn LinearAxis::CLASS_D_ACCESSOR_DECL(QFont, labelFont, LabelFont);
   \brief Get/set the font of the tick labels (size of the QFont will be ignored).
 */
/**
   \fn LinearAxis::CLASS_D_ACCESSOR_DECL(QPointF, labelOffset, LabelOffset);
   \brief Get/set the position offset of the tick labels relative to the end of the tick line.
 */
/**
   \fn LinearAxis::CLASS_D_ACCESSOR_DECL(QPen, pen, Pen);
   \brief Get/set the pen for the lines.
 */

/* ============================ getter methods ================= */
CLASS_SHARED_D_READER_IMPL(LinearAxis, LinearAxis::AxisOrientation, orientation, orientation);
BASIC_SHARED_D_READER_IMPL(LinearAxis, qreal, offset, offset);
BASIC_SHARED_D_READER_IMPL(LinearAxis, qreal, start, start);
BASIC_SHARED_D_READER_IMPL(LinearAxis, qreal, end, end);
BASIC_SHARED_D_READER_IMPL(LinearAxis, qreal, tickStart, tickStart);
BASIC_SHARED_D_READER_IMPL(LinearAxis, qreal, tickEnd, tickEnd);
BASIC_SHARED_D_READER_IMPL(LinearAxis, int, majorTickCount, majorTickCount);
BASIC_SHARED_D_READER_IMPL(LinearAxis, int, minorTickCount, minorTickCount);
BASIC_SHARED_D_READER_IMPL(LinearAxis, qreal, majorTicksLength, majorTicksLength);
BASIC_SHARED_D_READER_IMPL(LinearAxis, qreal, minorTicksLength, minorTicksLength);
CLASS_SHARED_D_READER_IMPL(LinearAxis, LinearAxis::TicksDirection, majorTicksDirection, majorTicksDirection);
CLASS_SHARED_D_READER_IMPL(LinearAxis, LinearAxis::TicksDirection, minorTicksDirection, minorTicksDirection);
BASIC_SHARED_D_READER_IMPL(LinearAxis, qreal, labelRotationAngle, labelRotationAngle);
BASIC_SHARED_D_READER_IMPL(LinearAxis, qreal, labelFontSize, labelFontSize);
CLASS_SHARED_D_READER_IMPL(LinearAxis, QColor, labelColor, labelColor);
CLASS_SHARED_D_READER_IMPL(LinearAxis, QFont, labelFont, labelFont);
CLASS_SHARED_D_READER_IMPL(LinearAxis, QPointF, labelOffset, labelOffset);
CLASS_SHARED_D_READER_IMPL(LinearAxis, QPen, pen, pen);

/* ============================ setter methods and undo commands ================= */

STD_SETTER_CMD_IMPL_F(LinearAxis, SetOrientation, LinearAxis::AxisOrientation, orientation, retransform);
void LinearAxis::setOrientation(const AxisOrientation &orientation) {
	Q_D(LinearAxis);
	if (orientation != d->orientation)
		exec(new LinearAxisSetOrientationCmd(d, orientation, tr("%1: set axis orientation")));
}

STD_SETTER_CMD_IMPL_F(LinearAxis, SetOffset, qreal, offset, retransform);
void LinearAxis::setOffset(qreal offset) {
	Q_D(LinearAxis);
	if (offset != d->offset)
		exec(new LinearAxisSetOffsetCmd(d, offset, tr("%1: set axis offset")));
}

STD_SETTER_CMD_IMPL_F(LinearAxis, SetStart, qreal, start, retransform);
void LinearAxis::setStart(qreal start) {
	Q_D(LinearAxis);
	if (start != d->start)
		exec(new LinearAxisSetStartCmd(d, start, tr("%1: set axis start")));
}

STD_SETTER_CMD_IMPL_F(LinearAxis, SetEnd, qreal, end, retransform);
void LinearAxis::setEnd(qreal end) {
	Q_D(LinearAxis);
	if (end != d->end)
		exec(new LinearAxisSetEndCmd(d, end, tr("%1: set axis end")));
}

STD_SETTER_CMD_IMPL_F(LinearAxis, SetTickStart, qreal, tickStart, retransformTicks);
void LinearAxis::setTickStart(qreal tickStart) {
	Q_D(LinearAxis);
	if (tickStart != d->tickStart)
		exec(new LinearAxisSetTickStartCmd(d, tickStart, tr("%1: set first tick")));
}

STD_SETTER_CMD_IMPL_F(LinearAxis, SetTickEnd, qreal, tickEnd, retransformTicks);
void LinearAxis::setTickEnd(qreal tickEnd) {
	Q_D(LinearAxis);
	if (tickEnd != d->tickEnd)
		exec(new LinearAxisSetTickEndCmd(d, tickEnd, tr("%1: set axis end")));
}

STD_SETTER_CMD_IMPL_F(LinearAxis, SetMajorTickCount, int, majorTickCount, retransformTicks);
void LinearAxis::setMajorTickCount(int majorTickCount) {
	Q_D(LinearAxis);
	if (majorTickCount != d->majorTickCount)
		exec(new LinearAxisSetMajorTickCountCmd(d, majorTickCount, tr("%1: set major tick count")));
}

STD_SETTER_CMD_IMPL_F(LinearAxis, SetMinorTickCount, int, minorTickCount, retransformTicks);
void LinearAxis::setMinorTickCount(int minorTickCount) {
	Q_D(LinearAxis);
	if (minorTickCount != d->minorTickCount)
		exec(new LinearAxisSetMinorTickCountCmd(d, minorTickCount, tr("%1: set minor tick count")));
}

STD_SETTER_CMD_IMPL_F(LinearAxis, SetMajorTicksLength, qreal, majorTicksLength, retransformTicks);
void LinearAxis::setMajorTicksLength(qreal majorTicksLength) {
	Q_D(LinearAxis);
	if (majorTicksLength != d->majorTicksLength)
		exec(new LinearAxisSetMajorTicksLengthCmd(d, majorTicksLength, tr("%1: set major ticks length")));
}

STD_SETTER_CMD_IMPL_F(LinearAxis, SetMinorTicksLength, qreal, minorTicksLength, retransformTicks);
void LinearAxis::setMinorTicksLength(qreal minorTicksLength) {
	Q_D(LinearAxis);
	if (minorTicksLength != d->minorTicksLength)
		exec(new LinearAxisSetMinorTicksLengthCmd(d, minorTicksLength, tr("%1: set minor ticks length")));
}

STD_SETTER_CMD_IMPL_F(LinearAxis, SetMajorTicksDirection, LinearAxis::TicksDirection, majorTicksDirection, retransformTicks);
void LinearAxis::setMajorTicksDirection(const TicksDirection &majorTicksDirection) {
	Q_D(LinearAxis);
	if (majorTicksDirection != d->majorTicksDirection)
		exec(new LinearAxisSetMajorTicksDirectionCmd(d, majorTicksDirection, tr("%1: set major ticks direction")));
}

STD_SETTER_CMD_IMPL_F(LinearAxis, SetMinorTicksDirection, LinearAxis::TicksDirection, minorTicksDirection, retransformTicks);
void LinearAxis::setMinorTicksDirection(const TicksDirection &minorTicksDirection) {
	Q_D(LinearAxis);
	if (minorTicksDirection != d->minorTicksDirection)
		exec(new LinearAxisSetMinorTicksDirectionCmd(d, minorTicksDirection, tr("%1: set minor ticks direction")));
}

STD_SETTER_CMD_IMPL_F(LinearAxis, SetLabelRotationAngle, qreal, labelRotationAngle, restyleLabels);
void LinearAxis::setLabelRotationAngle(qreal angle) {
	Q_D(LinearAxis);
	if (angle != d->labelRotationAngle)
		exec(new LinearAxisSetLabelRotationAngleCmd(d, angle, tr("%1: set label rotation angle")));
}

STD_SETTER_CMD_IMPL_F(LinearAxis, SetLabelFontSize, qreal, labelFontSize, restyleLabels);
void LinearAxis::setLabelFontSize(qreal size) {
	Q_D(LinearAxis);
	if (size != d->labelFontSize)
		exec(new LinearAxisSetLabelFontSizeCmd(d, size, tr("%1: set label font size")));
}

STD_SETTER_CMD_IMPL_F(LinearAxis, SetLabelColor, QColor, labelColor, restyleLabels);
void LinearAxis::setLabelColor(const QColor &color) {
	Q_D(LinearAxis);
	if (color != d->labelColor)
		exec(new LinearAxisSetLabelColorCmd(d, color, tr("%1: set label color")));
}

STD_SETTER_CMD_IMPL_F(LinearAxis, SetLabelFont, QFont, labelFont, restyleLabels);
void LinearAxis::setLabelFont(const QFont &font) {
	Q_D(LinearAxis);
	if (font != d->labelFont)
		exec(new LinearAxisSetLabelFontCmd(d, font, tr("%1: set label font")));
}

STD_SWAP_METHOD_SETTER_CMD_IMPL(LinearAxis, SetLabelOffset, QPointF, swapLabelOffset);
void LinearAxis::setLabelOffset(const QPointF &newOffset) {
	Q_D(LinearAxis);
	if (newOffset != d->labelOffset)
		exec(new LinearAxisSetLabelOffsetCmd(d, newOffset, tr("%1: set label offset")));
}

STD_SWAP_METHOD_SETTER_CMD_IMPL(LinearAxis, SetVisible, bool, swapVisible);
void LinearAxis::setVisible(bool on) {
	Q_D(LinearAxis);
	exec(new LinearAxisSetVisibleCmd(d, on, on ? tr("%1: set visible") : tr("%1: set invisible")));
}

STD_SETTER_CMD_IMPL_F(LinearAxis, SetPen, QPen, pen, recalcShapeAndBoundingRect);
void LinearAxis::setPen(const QPen &pen) {
	Q_D(LinearAxis);
	if (pen != d->pen)
		exec(new LinearAxisSetPenCmd(d, pen, tr("%1: set line style")));
}

bool LinearAxis::isVisible() const {
	Q_D(const LinearAxis);
	return d->isVisible();
}


/* ============================ other methods ================= */

QGraphicsItem *LinearAxis::graphicsItem() const {
	return d_ptr;
}

bool LinearAxis::Private::swapVisible(bool on) {
	bool oldValue = isVisible();
	setVisible(on);
	return oldValue;
}

QPointF LinearAxis::Private::swapLabelOffset(const QPointF &newOffset)
{
	QPointF oldOffset = labelOffset;
	labelOffset = newOffset;
	recalcShapeAndBoundingRect();
	return oldOffset;
}


void LinearAxis::retransform() {
	Q_D(LinearAxis);
	d->retransform();
}

void LinearAxis::Private::retransform() {
	const AbstractCoordinateSystem *cSystem = q->coordinateSystem();

	linePath = QPainterPath();

	QList<QLineF> lines;
	QPointF startPoint;
	QPointF endPoint;

	if (orientation & LinearAxis::axisHorizontal) {
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

void LinearAxis::Private::retransformTicks() {
	retransformTicks(q->coordinateSystem());
}

//! helper function for retransformTicks(const AbstractCoordinateSystem *cSystem)
bool LinearAxis::Private::transformAnchor(const AbstractCoordinateSystem *cSystem, QPointF *anchorPoint) {
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

// TODO: make this more generic to share more code with LogAxis
void LinearAxis::Private::retransformTicks(const AbstractCoordinateSystem *cSystem) {
	const CartesianCoordinateSystem *cCSystem = qobject_cast<const CartesianCoordinateSystem *>(cSystem);

	majorTicksPath = QPainterPath();
	minorTicksPath = QPainterPath();
	axisShape = QPainterPath();
	boundingRectangle = QRect();
	qDeleteAll(labels);
	labels.clear();

	int xDirection = 1;
	int yDirection = 1;
	if (cCSystem) {
		xDirection = cCSystem->xDirection();
		yDirection = cCSystem->yDirection();
	}
				
	const double majorTickSpacing = majorTickCount > 1 ? (tickEnd - tickStart) / ((qreal)(majorTickCount - 1)) : 0;
	for (int iMajor = 0; iMajor < majorTickCount; iMajor++) {
		QPointF anchorPoint;
		QPointF startPoint;
		QPointF endPoint;
		QPointF textPos;
		
		qreal majorTickPos = tickStart + majorTickSpacing * (qreal)iMajor;
		if (LinearAxis::noTicks != majorTicksDirection) {
			if (orientation & LinearAxis::axisHorizontal) {
				anchorPoint.setX(majorTickPos);
				anchorPoint.setY(offset);

				if (transformAnchor(cSystem, &anchorPoint)) {
					if (orientation & LinearAxis::axisNormalTicks) {
						startPoint = anchorPoint + QPointF(0, (majorTicksDirection & LinearAxis::ticksIn)  ? yDirection * majorTicksLength  : 0);
						endPoint   = anchorPoint + QPointF(0, (majorTicksDirection & LinearAxis::ticksOut) ? -yDirection * majorTicksLength : 0);
						textPos = endPoint;
					} else {
						startPoint = anchorPoint + QPointF(0, (majorTicksDirection & LinearAxis::ticksOut) ? yDirection * majorTicksLength  : 0);
						endPoint   = anchorPoint + QPointF(0, (majorTicksDirection & LinearAxis::ticksIn)  ? -yDirection * majorTicksLength : 0);
						textPos = startPoint;
					}
					majorTicksPath.moveTo(startPoint);
					majorTicksPath.lineTo(endPoint);
					addTextLabel(QLocale().toString(majorTickPos, numericFormat, displayedDigits), textPos);
				}
			} else { // vertical
				anchorPoint.setY(majorTickPos);
				anchorPoint.setX(offset);

				if (transformAnchor(cSystem, &anchorPoint)) {
					if (orientation & LinearAxis::axisNormalTicks) {
						startPoint = anchorPoint + QPointF((majorTicksDirection & LinearAxis::ticksIn)  ? xDirection * majorTicksLength  : 0, 0);
						endPoint   = anchorPoint + QPointF((majorTicksDirection & LinearAxis::ticksOut) ? -xDirection * majorTicksLength : 0, 0);
						textPos = endPoint;
					} else {
						startPoint = anchorPoint + QPointF((majorTicksDirection & LinearAxis::ticksOut) ? xDirection * majorTicksLength  : 0, 0);
						endPoint   = anchorPoint + QPointF((majorTicksDirection & LinearAxis::ticksIn)  ? -xDirection * majorTicksLength : 0, 0);
						textPos = startPoint;
					}
					majorTicksPath.moveTo(startPoint);
					majorTicksPath.lineTo(endPoint);
					addTextLabel(QLocale().toString(majorTickPos, numericFormat, displayedDigits), textPos);
				}
			}
		}

		if ((LinearAxis::noTicks != minorTicksDirection) && (majorTickCount > 1) && (minorTickCount > 0) && (iMajor < majorTickCount - 1)) {
			for (int iMinor = 0; iMinor < minorTickCount; iMinor++) {
				qreal minorTickPos = majorTickPos + (qreal)(iMinor + 1) * majorTickSpacing / (qreal)(minorTickCount + 1);
				if (orientation & LinearAxis::axisHorizontal) {
					anchorPoint.setX(minorTickPos);
					anchorPoint.setY(offset);

					if (transformAnchor(cSystem, &anchorPoint)) {
						if (orientation & LinearAxis::axisNormalTicks) {
							startPoint = anchorPoint + QPointF(0, (minorTicksDirection & LinearAxis::ticksIn)  ? yDirection * minorTicksLength  : 0);
							endPoint   = anchorPoint + QPointF(0, (minorTicksDirection & LinearAxis::ticksOut) ? -yDirection * minorTicksLength : 0);
						} else {
							startPoint = anchorPoint + QPointF(0, (minorTicksDirection & LinearAxis::ticksOut) ? yDirection * minorTicksLength  : 0);
							endPoint   = anchorPoint + QPointF(0, (minorTicksDirection & LinearAxis::ticksIn)  ? -yDirection * minorTicksLength : 0);
						}
						minorTicksPath.moveTo(startPoint);
						minorTicksPath.lineTo(endPoint);
					}
				} else { // vertical
					anchorPoint.setY(minorTickPos);
					anchorPoint.setX(offset);

					if (transformAnchor(cSystem, &anchorPoint)) {
						if (orientation & LinearAxis::axisNormalTicks) {
							startPoint = anchorPoint + QPointF((minorTicksDirection & LinearAxis::ticksIn)  ? xDirection * minorTicksLength  : 0, 0);
							endPoint   = anchorPoint + QPointF((minorTicksDirection & LinearAxis::ticksOut) ? -xDirection * minorTicksLength : 0, 0);
						} else {
							startPoint = anchorPoint + QPointF((minorTicksDirection & LinearAxis::ticksOut) ? xDirection * minorTicksLength  : 0, 0);
							endPoint   = anchorPoint + QPointF((minorTicksDirection & LinearAxis::ticksIn)  ? -xDirection * minorTicksLength : 0, 0);
						}
						minorTicksPath.moveTo(startPoint);
						minorTicksPath.lineTo(endPoint);
					}
				}
			}
		}
	}

	restyleLabels(); // this calls recalcShapeAndBoundingRect()
}

void LinearAxis::Private::recalcShapeAndBoundingRect() {
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
		rect.translate(labelOffset);
		boundingRectangle |= rect;
		axisShape.addRect(rect);
	}
	
	update();
}

void LinearAxis::Private::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget * widget) {
	Q_UNUSED(option)
	Q_UNUSED(widget)

	painter->setPen(pen);
	
	painter->drawPath(linePath);
	painter->drawPath(minorTicksPath);
	painter->drawPath(majorTicksPath);

	painter->translate(labelOffset);

	foreach (ScalableTextLabel *textLabel, labels)
		textLabel->paint(painter);

	painter->translate(-labelOffset);
}


void LinearAxisPrivate::addTextLabel(const QString &text, const QPointF &pos) {
	ScalableTextLabel *label = new ScalableTextLabel();
	label->setText(text);
	label->setPosition(pos);

	ScalableTextLabel::HorizontalAlignment hAlign = ScalableTextLabel::hAlignCenter;
	ScalableTextLabel::VerticalAlignment vAlign = ScalableTextLabel::vAlignCenter;
	if (orientation & LinearAxis::axisHorizontal) {
		if (qFuzzyCompare(1, 1 + labelRotationAngle)) {
			if (orientation & LinearAxis::axisNormalTicks)
				vAlign = ScalableTextLabel::vAlignTop;
			else
				vAlign = ScalableTextLabel::vAlignBottom;
		} else {
			if (orientation & LinearAxis::axisNormalTicks) {
				if (labelRotationAngle < 0)
					hAlign = ScalableTextLabel::hAlignRight;
				else
					hAlign = ScalableTextLabel::hAlignLeft;
			} else {
				if (labelRotationAngle < 0)
					hAlign = ScalableTextLabel::hAlignLeft;
				else
					hAlign = ScalableTextLabel::hAlignRight;
			}
		}
	} else {
		if (orientation & LinearAxis::axisNormalTicks)
			hAlign = ScalableTextLabel::hAlignRight;
		else
			hAlign = ScalableTextLabel::hAlignLeft;
	}
	label->setAlignment(hAlign, vAlign);

	labels.append(label);
}

void LinearAxisPrivate::restyleLabels() {
	foreach(ScalableTextLabel *label, labels) {
		label->setFontSize(labelFontSize);
		label->setRotationAngle(labelRotationAngle);
		label->setFont(labelFont);
		label->setTextColor(labelColor);
	}
	recalcShapeAndBoundingRect();
}

void LinearAxis::handlePageResize(double horizontalRatio, double verticalRatio) {
	Q_D(LinearAxis);

	QPen pen = d->pen;
	pen.setWidthF(pen.widthF() * (horizontalRatio + verticalRatio) / 2.0);
	setPen(pen);

	if (d->orientation & LinearAxis::axisHorizontal) {
		setMajorTicksLength(d->majorTicksLength * verticalRatio); // ticks are perpendicular to axis line -> verticalRatio relevant
		setMinorTicksLength(d->minorTicksLength * verticalRatio);
		setLabelFontSize(d->labelFontSize * verticalRatio);
	} else {
		setMajorTicksLength(d->majorTicksLength * horizontalRatio);
		setMinorTicksLength(d->minorTicksLength * horizontalRatio);
		setLabelFontSize(d->labelFontSize * verticalRatio); // this is not perfectly correct for rotated labels 
															// when the page aspect ratio changes, but should not matter
	}
	setLabelOffset(QPointF(d->labelOffset.x() * horizontalRatio, d->labelOffset.y() * verticalRatio));

	BaseClass::handlePageResize(horizontalRatio, verticalRatio);
}

