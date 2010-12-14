/***************************************************************************
    File                 : LineSymbolCurve.cpp
    Project              : LabPlot/SciDAVis
    Description          : A curve drawn as line and/or symbols
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

/**
 * \class LineSymbolCurve
 * \brief A curve drawn as line and/or symbols
 *
 * 
 */

#include "worksheet/LineSymbolCurve.h"
#include "worksheet/LineSymbolCurvePrivate.h"
#include "worksheet/AbstractCoordinateSystem.h"
#include "worksheet/CartesianCoordinateSystem.h"
#include "lib/commandtemplates.h"
#include "core/plugin/PluginManager.h"
#include "worksheet/symbols/EllipseCurveSymbol.h"
#include "worksheet/interfaces.h"

#include <QGraphicsItem>
#include <QGraphicsItemGroup>
#include <QGraphicsPathItem>
#include <QGraphicsEllipseItem>
#include <QPainterPath>
#include <QPainter>
#include <QtDebug>

#include <gsl/gsl_spline.h>
#include <math.h>
#include <vector>

LineSymbolCurvePrivate::LineSymbolCurvePrivate(LineSymbolCurve *owner): q(owner){
  	xColumn = NULL;
	yColumn = NULL;
	valuesColumn = NULL;
	
	lineType = LineSymbolCurve::NoLine;
	lineInterpolationPointsCount = 1;
	lineOpacity = 1.0;
	dropLineType = LineSymbolCurve::NoDropLine;
	dropLineOpacity = 1.0;
	
	symbolsOpacity = 1.0;
	symbolRotationAngle = 0;
	symbolSize = 1;
	symbolAspectRatio = 1;
	symbolPrototype = NULL;
	swapSymbolTypeId("diamond");

	valuesType = LineSymbolCurve::NoValues;
	valuesPosition = LineSymbolCurve::ValuesAbove;
	valuesDistance = 1.0;
	valuesOpacity = 1.0;
	
 	// TODO: remove this temporary code later
	symbolSize = 1;
	symbolPrototype->setBrush(QBrush(Qt::red));
}

LineSymbolCurvePrivate::~LineSymbolCurvePrivate() {
}

QString LineSymbolCurvePrivate::name() const{
  return q->name();
}

QRectF LineSymbolCurvePrivate::boundingRect() const{
  return boundingRectangle;
}

/*!
  Returns the shape of the LineSymbolCurve as a QPainterPath in local coordinates
*/
QPainterPath LineSymbolCurvePrivate::shape() const{
  return curveShape;
}
		


LineSymbolCurve::LineSymbolCurve(const QString &name)
		: AbstractWorksheetElement(name), d_ptr(new LineSymbolCurvePrivate(this)) {

	d_ptr->retransform();
}

LineSymbolCurve::LineSymbolCurve(const QString &name, LineSymbolCurvePrivate *dd)
		: AbstractWorksheetElement(name), d_ptr(dd) {
}

LineSymbolCurve::~LineSymbolCurve() {
	delete d_ptr;
}


/* ============================ accessor documentation ================= */
                                                                           /**
  \fn   LineSymbolCurve::POINTER_D_ACCESSOR_DECL(const AbstractColumn, xColumn, XColumn);
  \brief Set/get the pointer to the X column.
*/
/**
  \fn   LineSymbolCurve::POINTER_D_ACCESSOR_DECL(const AbstractColumn, yColumn, YColumn);
  \brief Set/get the pointer to the Y column.
*/

/**
  \fn   LineSymbolCurve::CLASS_D_ACCESSOR_DECL(LineSymbolCurve::LineType, lineType , LineType);
  \brief Set/get the line type.
*/
/**
  \fn   LineSymbolCurve::BASIC_D_ACCESSOR_DECL(bool, lineVisible, LineVisible);
  \brief Set/get whether the line is visible/invisible.
*/
/**
  \fn   LineSymbolCurve::CLASS_D_ACCESSOR_DECL(QPen, linePen, LinePen);
  \brief Get/set the line pen.
*/


/**
  \fn   LineSymbolCurve::BASIC_D_ACCESSOR_DECL(bool, symbolsVisible, SymbolsVisible);
  \brief Set/get whether the symbols are visible/invisible.
*/
/**
  \fn   LineSymbolCurve::BASIC_D_ACCESSOR_DECL(bool, symbolsOpacity, SymbolsOpacity);
  \brief Set/get the opacity of the symbols. The opacity ranges from 0.0 to 1.0, where 0.0 is fully transparent and 1.0 is fully opaque.
*/
/**
  \fn   LineSymbolCurve::BASIC_D_ACCESSOR_DECL(qreal, symbolRotationAngle, SymbolRotationAngle);
  \brief Set/get the rotation angle of the symbols.
*/
/**
  \fn   LineSymbolCurve::BASIC_D_ACCESSOR_DECL(qreal, symbolSize, SymbolSize);
  \brief Set/get the (horizontal) size of the symbols.
*/
/**
  \fn   LineSymbolCurve::BASIC_D_ACCESSOR_DECL(qreal, symbolAspectRatio, SymbolAspectRatio);
  \brief Set/get the ratio between the width and height of the symbols.
*/
/**
  \fn   LineSymbolCurve::CLASS_D_ACCESSOR_DECL(QString, symbolTypeId, SymbolTypeId);
  \brief Set/get the symbol type.
*/
/**
  \fn   LineSymbolCurve::CLASS_D_ACCESSOR_DECL(QBrush, symbolsBrush, SymbolsBrush);
  \brief Get/set the symbol filling brush.
*/
/**
  \fn   LineSymbolCurve::CLASS_D_ACCESSOR_DECL(QPen, symbolsPen, SymbolsPen);
  \brief Get/set the symbol outline pen.
*/


/* ============================ getter methods ================= */
BASIC_SHARED_D_READER_IMPL(LineSymbolCurve, const AbstractColumn *, xColumn, xColumn);
BASIC_SHARED_D_READER_IMPL(LineSymbolCurve, const AbstractColumn *, yColumn, yColumn);

BASIC_SHARED_D_READER_IMPL(LineSymbolCurve, LineSymbolCurve::LineType, lineType, lineType);
BASIC_SHARED_D_READER_IMPL(LineSymbolCurve, int, lineInterpolationPointsCount, lineInterpolationPointsCount);
CLASS_SHARED_D_READER_IMPL(LineSymbolCurve, QPen, linePen, linePen);
BASIC_SHARED_D_READER_IMPL(LineSymbolCurve, qreal, lineOpacity, lineOpacity);

BASIC_SHARED_D_READER_IMPL(LineSymbolCurve, LineSymbolCurve::DropLineType, dropLineType, dropLineType);
CLASS_SHARED_D_READER_IMPL(LineSymbolCurve, QPen, dropLinePen, dropLinePen);
BASIC_SHARED_D_READER_IMPL(LineSymbolCurve, qreal, dropLineOpacity, dropLineOpacity);

BASIC_SHARED_D_READER_IMPL(LineSymbolCurve, qreal, symbolsOpacity, symbolsOpacity);
BASIC_SHARED_D_READER_IMPL(LineSymbolCurve, qreal, symbolRotationAngle, symbolRotationAngle);
BASIC_SHARED_D_READER_IMPL(LineSymbolCurve, qreal, symbolSize, symbolSize);
BASIC_SHARED_D_READER_IMPL(LineSymbolCurve, qreal, symbolAspectRatio, symbolAspectRatio);
CLASS_SHARED_D_READER_IMPL(LineSymbolCurve, QString, symbolTypeId, symbolTypeId);
CLASS_SHARED_D_READER_IMPL(LineSymbolCurve, QBrush, symbolsBrush, symbolsBrush);
CLASS_SHARED_D_READER_IMPL(LineSymbolCurve, QPen, symbolsPen, symbolsPen);

BASIC_SHARED_D_READER_IMPL(LineSymbolCurve, LineSymbolCurve::ValuesPosition, valuesPosition, valuesPosition);
BASIC_SHARED_D_READER_IMPL(LineSymbolCurve, qreal, valuesDistance, valuesDistance);
BASIC_SHARED_D_READER_IMPL(LineSymbolCurve, qreal, valuesRotationAngle, valuesRotationAngle);
BASIC_SHARED_D_READER_IMPL(LineSymbolCurve, qreal, valuesOpacity, valuesOpacity);
CLASS_SHARED_D_READER_IMPL(LineSymbolCurve, QString, valuesPrefix, valuesPrefix);
CLASS_SHARED_D_READER_IMPL(LineSymbolCurve, QString, valuesSuffix, valuesSuffix);
CLASS_SHARED_D_READER_IMPL(LineSymbolCurve, QPen, valuesPen, valuesPen);
CLASS_SHARED_D_READER_IMPL(LineSymbolCurve, QFont, valuesFont, valuesFont);

/* ============================ setter methods and undo commands ================= */

//Line 

STD_SETTER_CMD_IMPL_F(LineSymbolCurve, SetXColumn, const AbstractColumn *, xColumn, retransform);
void LineSymbolCurve::setXColumn(const AbstractColumn *xColumn) {
	Q_D(LineSymbolCurve);
	if (xColumn != d->xColumn)
		exec(new LineSymbolCurveSetXColumnCmd(d, xColumn, tr("%1: assign x values")));
}

STD_SETTER_CMD_IMPL_F(LineSymbolCurve, SetYColumn, const AbstractColumn *, yColumn, retransform);
void LineSymbolCurve::setYColumn(const AbstractColumn *yColumn) {
	Q_D(LineSymbolCurve);
	if (yColumn != d->yColumn)
		exec(new LineSymbolCurveSetYColumnCmd(d, yColumn, tr("%1: assign y values")));
}

//Line
STD_SETTER_CMD_IMPL_F(LineSymbolCurve, SetLineType, LineSymbolCurve::LineType, lineType, updateLines);
void LineSymbolCurve::setLineType(LineType type) {
	Q_D(LineSymbolCurve);
	if (type != d->lineType)
		exec(new LineSymbolCurveSetLineTypeCmd(d, type, tr("%1: line type changed")));
}

STD_SETTER_CMD_IMPL_F(LineSymbolCurve, SetLineInterpolationPointsCount, int, lineInterpolationPointsCount, updateLines);
void LineSymbolCurve::setLineInterpolationPointsCount(int count) {
	Q_D(LineSymbolCurve);
	if (count != d->lineInterpolationPointsCount)
		exec(new LineSymbolCurveSetLineInterpolationPointsCountCmd(d, count, tr("%1: set interpolation points")));
}

STD_SETTER_CMD_IMPL_F(LineSymbolCurve, SetLinePen, QPen, linePen, recalcShapeAndBoundingRect);
void LineSymbolCurve::setLinePen(const QPen &pen) {
	Q_D(LineSymbolCurve);
	if (pen != d->linePen)
		exec(new LineSymbolCurveSetLinePenCmd(d, pen, tr("%1: set line style")));
}

STD_SETTER_CMD_IMPL_F(LineSymbolCurve, SetLineOpacity, qreal, lineOpacity, update);
void LineSymbolCurve::setLineOpacity(qreal opacity) {
	Q_D(LineSymbolCurve);
	if (opacity != d->lineOpacity)
		exec(new LineSymbolCurveSetLineOpacityCmd(d, opacity, tr("%1: set line opacity")));
}

//Drop lines
STD_SETTER_CMD_IMPL_F(LineSymbolCurve, SetDropLineType, LineSymbolCurve::DropLineType, dropLineType, updateDropLines);
void LineSymbolCurve::setDropLineType(DropLineType type) {
	Q_D(LineSymbolCurve);
	if (type != d->dropLineType)
		exec(new LineSymbolCurveSetDropLineTypeCmd(d, type, tr("%1: drop line type changed")));
}

STD_SETTER_CMD_IMPL_F(LineSymbolCurve, SetDropLinePen, QPen, dropLinePen, recalcShapeAndBoundingRect);
void LineSymbolCurve::setDropLinePen(const QPen &pen) {
	Q_D(LineSymbolCurve);
	if (pen != d->dropLinePen)
		exec(new LineSymbolCurveSetDropLinePenCmd(d, pen, tr("%1: set drop line style")));
}

STD_SETTER_CMD_IMPL_F(LineSymbolCurve, SetDropLineOpacity, qreal, dropLineOpacity, update);
void LineSymbolCurve::setDropLineOpacity(qreal opacity) {
	Q_D(LineSymbolCurve);
	if (opacity != d->dropLineOpacity)
		exec(new LineSymbolCurveSetDropLineOpacityCmd(d, opacity, tr("%1: set drop line opacity")));
}

// Symbols 
STD_SETTER_CMD_IMPL_F(LineSymbolCurve, SetSymbolsOpacity, qreal, symbolsOpacity, update);
void LineSymbolCurve::setSymbolsOpacity(qreal opacity) {
	Q_D(LineSymbolCurve);
	if (opacity != d->symbolsOpacity)
		exec(new LineSymbolCurveSetSymbolsOpacityCmd(d, opacity, tr("%1: set symbols opacity")));
}


STD_SETTER_CMD_IMPL_F(LineSymbolCurve, SetSymbolRotationAngle, qreal, symbolRotationAngle,updateSymbol);
void LineSymbolCurve::setSymbolRotationAngle(qreal angle) {
	Q_D(LineSymbolCurve);
	if (!qFuzzyCompare(1 + angle, 1 + d->symbolRotationAngle))
		exec(new LineSymbolCurveSetSymbolRotationAngleCmd(d, angle, tr("%1: rotate symbols")));
}

STD_SETTER_CMD_IMPL_F(LineSymbolCurve, SetSymbolSize, qreal, symbolSize, updateSymbol);
void LineSymbolCurve::setSymbolSize(qreal size) {
	Q_D(LineSymbolCurve);
	if (!qFuzzyCompare(1 + size, 1 + d->symbolSize))
		exec(new LineSymbolCurveSetSymbolSizeCmd(d, size, tr("%1: set symbol size")));
}

//TODO ???
STD_SETTER_CMD_IMPL_F(LineSymbolCurve, SetSymbolAspectRatio, qreal, symbolAspectRatio, updateSymbol);
void LineSymbolCurve::setSymbolAspectRatio(qreal ratio) {
	Q_D(LineSymbolCurve);
	if (!qFuzzyCompare(1 + ratio, 1 + d->symbolAspectRatio))
		exec(new LineSymbolCurveSetSymbolAspectRatioCmd(d, ratio, tr("%1: set symbol aspect ratio")));
}

STD_SWAP_METHOD_SETTER_CMD_IMPL(LineSymbolCurve, SetSymbolTypeId, QString, swapSymbolTypeId);
void LineSymbolCurve::setSymbolTypeId(const QString &id) {
	Q_D(LineSymbolCurve);
	if (id != d->symbolTypeId)
		exec(new LineSymbolCurveSetSymbolTypeIdCmd(d, id, tr("%1: set symbol type")));
}

STD_SETTER_CMD_IMPL_F(LineSymbolCurve, SetSymbolsBrush, QBrush, symbolsBrush, update);
void LineSymbolCurve::setSymbolsBrush(const QBrush &brush) {
	Q_D(LineSymbolCurve);
	if (brush != d->symbolsBrush)
		exec(new LineSymbolCurveSetSymbolsBrushCmd(d, brush, tr("%1: set symbol filling")));
}

STD_SETTER_CMD_IMPL_F(LineSymbolCurve, SetSymbolsPen, QPen, symbolsPen, recalcShapeAndBoundingRect);
void LineSymbolCurve::setSymbolsPen(const QPen &pen) {
	Q_D(LineSymbolCurve);
	if (pen != d->symbolsPen)
		exec(new LineSymbolCurveSetSymbolsPenCmd(d, pen, tr("%1: set symbol outline style")));
}

//Values
STD_SETTER_CMD_IMPL_F(LineSymbolCurve, SetValuesType, LineSymbolCurve::ValuesType, valuesType, updateValues);
void LineSymbolCurve::setValuesType(ValuesType type) {
	Q_D(LineSymbolCurve);
	if (type != d->valuesType)
		exec(new LineSymbolCurveSetValuesTypeCmd(d, type, tr("%1: set values type")));
}

STD_SETTER_CMD_IMPL_F(LineSymbolCurve, SetValuesPosition, LineSymbolCurve::ValuesPosition, valuesPosition, updateValues);
void LineSymbolCurve::setValuesPosition(ValuesPosition position) {
	Q_D(LineSymbolCurve);
	if (position != d->valuesPosition)
		exec(new LineSymbolCurveSetValuesPositionCmd(d, position, tr("%1: set values position")));
}

STD_SETTER_CMD_IMPL_F(LineSymbolCurve, SetValuesDistance, qreal, valuesDistance, updateValues);
void LineSymbolCurve::setValuesDistance(qreal distance) {
	Q_D(LineSymbolCurve);
	if (distance != d->valuesDistance)
		exec(new LineSymbolCurveSetValuesDistanceCmd(d, distance, tr("%1: set values distance")));
}

STD_SETTER_CMD_IMPL_F(LineSymbolCurve, SetValuesOpacity, qreal, valuesOpacity, update);
void LineSymbolCurve::setValuesOpacity(qreal opacity) {
	Q_D(LineSymbolCurve);
	if (opacity != d->valuesOpacity)
		exec(new LineSymbolCurveSetValuesOpacityCmd(d, opacity, tr("%1: set values opacity")));
}

STD_SETTER_CMD_IMPL_F(LineSymbolCurve, SetValuesRotationAngle, qreal, valuesRotationAngle,update);
void LineSymbolCurve::setValuesRotationAngle(qreal angle) {
	Q_D(LineSymbolCurve);
	if (!qFuzzyCompare(1 + angle, 1 + d->valuesRotationAngle))
		exec(new LineSymbolCurveSetValuesRotationAngleCmd(d, angle, tr("%1: rotate valuess")));
}

STD_SETTER_CMD_IMPL_F(LineSymbolCurve, SetValuesPrefix, QString, valuesPrefix, updateValues);
void LineSymbolCurve::setValuesPrefix(const QString& prefix) {
	Q_D(LineSymbolCurve);
	if (prefix!= d->valuesPrefix)
		exec(new LineSymbolCurveSetValuesPrefixCmd(d, prefix, tr("%1: set values prefix")));
}

STD_SETTER_CMD_IMPL_F(LineSymbolCurve, SetValuesSuffix, QString, valuesSuffix, updateValues);
void LineSymbolCurve::setValuesSuffix(const QString& suffix) {
	Q_D(LineSymbolCurve);
	if (suffix!= d->valuesSuffix)
		exec(new LineSymbolCurveSetValuesSuffixCmd(d, suffix, tr("%1: set values suffix")));
}

STD_SETTER_CMD_IMPL_F(LineSymbolCurve, SetValuesFont, QFont, valuesFont, updateValues);
void LineSymbolCurve::setValuesFont(const QFont& font) {
	Q_D(LineSymbolCurve);
	if (font!= d->valuesFont)
		exec(new LineSymbolCurveSetValuesFontCmd(d, font, tr("%1: set values font")));
}

STD_SETTER_CMD_IMPL_F(LineSymbolCurve, SetValuesPen, QPen, valuesPen, recalcShapeAndBoundingRect);
void LineSymbolCurve::setValuesPen(const QPen &pen) {
	Q_D(LineSymbolCurve);
	if (pen != d->valuesPen)
		exec(new LineSymbolCurveSetValuesPenCmd(d, pen, tr("%1: set values style")));
}

/* ============================ other methods ================= */
QStringList LineSymbolCurve::lineTypeStrings(){
  //TODO add i18n-Version
  return ( QStringList()<<tr("none")<<tr("line")
								<<tr("horiz. start")<<tr("vert. start")
								<<tr("horiz. midpoint")<<tr("vert. midpoint")
								<<tr("2-segments")<<tr("3-segments") 
								<<tr("cubic spline (natural)")
								<<tr("cubic spline (periodic)")
								<<tr("Akima-spline (natural)")
								<<tr("Akima-spline (periodic)") );
}

QStringList LineSymbolCurve::dropLineTypeStrings(){
  //TODO add i18n-Version
  return ( QStringList()<<tr("none")
			<<tr("drop lines, X")<<tr("drop lines, Y")<<tr("drop lines, XY") );
}

QStringList LineSymbolCurve::valuesTypeStrings(){
  //TODO add i18n-Version
  return ( QStringList()<<tr("no values")<<"x"<<"y"<<"x, y"<<"(x, y)"<<tr("custom column") );
}

QStringList LineSymbolCurve::valuesPositionStrings(){
  //TODO add i18n-Version
  return ( QStringList()<<tr("above")<<tr("under")<<tr("left")<<tr("right") );
}

QGraphicsItem *LineSymbolCurve::graphicsItem() const {
	return d_ptr;
}

bool LineSymbolCurve::Private::swapVisible(bool on) {
	bool oldValue = isVisible();
	setVisible(on);
	return oldValue;
}

STD_SWAP_METHOD_SETTER_CMD_IMPL(LineSymbolCurve, SetVisible, bool, swapVisible);
void LineSymbolCurve::setVisible(bool on) {
	Q_D(LineSymbolCurve);
	exec(new LineSymbolCurveSetVisibleCmd(d, on, on ? tr("%1: set visible") : tr("%1: set invisible")));
}

bool LineSymbolCurve::isVisible() const {
	Q_D(const LineSymbolCurve);
	return d->isVisible();
}

void LineSymbolCurve::retransform() {
	d_ptr->retransform();
}

void LineSymbolCurve::handlePageResize(double horizontalRatio, double verticalRatio) {
  qDebug()<<"hadlePageResize";
  //TODO
/*	Q_D(const LineSymbolCurve);
	
	setSymbolSize(d->symbolSize * horizontalRatio);
	setSymbolAspectRatio(d->symbolAspectRatio * horizontalRatio / verticalRatio);
	
	QPen pen = d->symbolsPen;
	pen.setWidthF(pen.widthF() * (horizontalRatio + verticalRatio) / 2.0);
	setSymbolsPen(pen);
	
	pen = d->linePen;
	pen.setWidthF(pen.widthF() * (horizontalRatio + verticalRatio) / 2.0);
	setLinePen(pen);

	//setValuesDistance(d->distance*);
	QFont font=d->valuesFont;
	font.setPointSizeF(font.pointSizeF()*horizontalRatio);
	setValuesFont(font);
	
	BaseClass::handlePageResize(horizontalRatio, verticalRatio);*/
}

//############ private-implementation #################
void LineSymbolCurve::Private::retransform() {
	symbolPointsLogical.clear();

	if ( (NULL == xColumn) || (NULL == yColumn) )
		return;
	
	// TODO: add start row/end row attributes

	int startRow = 0;
	int endRow = xColumn->rowCount() - 1;
	QPointF tempPoint;

	SciDAVis::ColumnMode xColMode = xColumn->columnMode();
	SciDAVis::ColumnMode yColMode = yColumn->columnMode();

	for (int row = startRow; row <= endRow; row++ ){
		if ( xColumn->isValid(row) && yColumn->isValid(row) 
			&& (!xColumn->isMasked(row)) && (!yColumn->isMasked(row)) ) {

			switch(xColMode) {
				case SciDAVis::Numeric:
					tempPoint.setX(xColumn->valueAt(row));
					break;
				case SciDAVis::Text:
					//TODO
				case SciDAVis::DateTime:
				case SciDAVis::Month:
				case SciDAVis::Day:
					//TODO
					break;
				default:
					break;
			}

			switch(yColMode) {
				case SciDAVis::Numeric:
					tempPoint.setY(yColumn->valueAt(row));
					break;
				case SciDAVis::Text:
					//TODO
				case SciDAVis::DateTime:
				case SciDAVis::Month:
				case SciDAVis::Day:
					//TODO
					break;
				default:
					break;
			}
			symbolPointsLogical.append(tempPoint);
		}
	}


	const AbstractCoordinateSystem *cSystem = q->coordinateSystem();
	if (cSystem)
	  symbolPoints = cSystem->mapLogicalToScene(symbolPointsLogical);
	
  updateLines();
  updateDropLines();
  updateValues();
  
  recalcShapeAndBoundingRect();
}

/*!
  recalculates the painter path for the lines connecting the data points.
  Called each time when the type of this connection is changed.
*/
void LineSymbolCurve::Private::updateLines(){
  	linePath = QPainterPath();
	if (lineType == LineSymbolCurve::NoLine){
	  recalcShapeAndBoundingRect();
	  return;
	}
	
	int count=symbolPointsLogical.count();
	if (count<=1){
	  	recalcShapeAndBoundingRect();
		return;
	}
	
	QList<QLineF> lines;
	QPointF tempPoint1, tempPoint2;
	QPointF curPoint, nextPoint;
	switch(lineType){
	  case LineSymbolCurve::Line:{
		for (int i=0; i<count-1; i++){
		  lines.append(QLineF(symbolPointsLogical.at(i), symbolPointsLogical.at(i+1)));
		}
		break;
	  }
	  case LineSymbolCurve::StartHorizontal:{
		for (int i=0; i<count-1; i++){
		  curPoint=symbolPointsLogical.at(i);
		  nextPoint=symbolPointsLogical.at(i+1);
		  tempPoint1=QPointF(nextPoint.x(), curPoint.y());
		  lines.append(QLineF(curPoint, tempPoint1));
		  lines.append(QLineF(tempPoint1, nextPoint));
		}
		break;
	  }
	  case LineSymbolCurve::StartVertical:{
		for (int i=0; i<count-1; i++){
		  curPoint=symbolPointsLogical.at(i);
		  nextPoint=symbolPointsLogical.at(i+1);
		  tempPoint1=QPointF(curPoint.x(), nextPoint.y());
		  lines.append(QLineF(curPoint, tempPoint1));
		  lines.append(QLineF(tempPoint1,nextPoint));
		}
		break;
	  }
	  case LineSymbolCurve::MidpointHorizontal:{
		for (int i=0; i<count-1; i++){
		  curPoint=symbolPointsLogical.at(i);
		  nextPoint=symbolPointsLogical.at(i+1);
		  tempPoint1=QPointF(curPoint.x() + (nextPoint.x()-curPoint.x())/2, curPoint.y());
		  tempPoint2=QPointF(curPoint.x() + (nextPoint.x()-curPoint.x())/2, nextPoint.y());
		  lines.append(QLineF(curPoint, tempPoint1));
		  lines.append(QLineF(tempPoint1, tempPoint2));
		  lines.append(QLineF(tempPoint2, nextPoint));
		}
		break;
	  }
	  case LineSymbolCurve::MidpointVertical:{
		for (int i=0; i<count-1; i++){
		  curPoint=symbolPointsLogical.at(i);
		  nextPoint=symbolPointsLogical.at(i+1);		  
		  tempPoint1=QPointF(curPoint.x(), curPoint.y() + (nextPoint.y()-curPoint.y())/2);
		  tempPoint2=QPointF(nextPoint.x(), curPoint.y() + (nextPoint.y()-curPoint.y())/2);
		  lines.append(QLineF(curPoint, tempPoint1));
		  lines.append(QLineF(tempPoint1, tempPoint2));
		  lines.append(QLineF(tempPoint2, nextPoint));
		}
		break;
	  }	  
	  case LineSymbolCurve::Segments2:{
		int skip=0;
		for (int i=0; i<count-1; i++){
		  if (skip!=1){
			lines.append(QLineF(symbolPointsLogical.at(i), symbolPointsLogical.at(i+1)));
			skip++;
		  }else{
			skip=0;
		  }
		}
		break;
	  }
	  case LineSymbolCurve::Segments3:{
		int skip=0;
		for (int i=0; i<count-1; i++){
		  if (skip!=2){
			lines.append(QLineF(symbolPointsLogical.at(i), symbolPointsLogical.at(i+1)));
			skip++;
		  }else{
			skip=0;
		  }
		}
		break;
	  }
	  case LineSymbolCurve::SplineCubicNatural:
	  case LineSymbolCurve::SplineCubicPeriodic:
	  case LineSymbolCurve::SplineAkimaNatural:
	  case LineSymbolCurve::SplineAkimaPeriodic:{
		//TODO: optimize! try to ommit the copying from the column to the arrays of doubles.
		gsl_interp_accel *acc  = gsl_interp_accel_alloc();
		gsl_spline *spline;
		  
		double x[count],  y[count];
		for (int i=0; i<count; i++){
		  x[i]=symbolPointsLogical.at(i).x();
		  y[i]=symbolPointsLogical.at(i).y();
		}
		
		if (lineType==LineSymbolCurve::SplineCubicNatural){
		  spline  = gsl_spline_alloc (gsl_interp_cspline, count);
		}else if (lineType==LineSymbolCurve::SplineCubicPeriodic){
			spline  = gsl_spline_alloc (gsl_interp_cspline_periodic, count);
		}else if (lineType==LineSymbolCurve::SplineAkimaNatural){
		  spline  = gsl_spline_alloc (gsl_interp_akima, count);
		}else if (lineType==LineSymbolCurve::SplineAkimaPeriodic){
			spline  = gsl_spline_alloc (gsl_interp_akima_periodic, count);
		}
		
		gsl_spline_init (spline, x, y, count);
		
		//create interpolating points
		std::vector<double> xinterp, yinterp;
		double step;
		double xi, yi, x1, x2;
		for (int i=0; i<count-1; i++){
		   x1 = x[i];
		   x2 = x[i+1];
		   step=fabs(x2-x1)/(lineInterpolationPointsCount+1);
		   
		  for (xi=x1; xi<=x2; xi += step){
			yi = gsl_spline_eval (spline, xi, acc);
			xinterp.push_back(xi);
			yinterp.push_back(yi);
		  }
		}
		
		for (int i=0; i<xinterp.size()-1; i++){
		  lines.append(QLineF(xinterp[i], yinterp[i], xinterp[i+1], yinterp[i+1]));
		}
		
		gsl_spline_free (spline);
        gsl_interp_accel_free (acc);
		break;
	  }
	  default:
		break;
	}
	
	const AbstractCoordinateSystem *cSystem = q->coordinateSystem();
	if (cSystem)
	  lines = cSystem->mapLogicalToScene(lines);
	
	foreach (QLineF line, lines) {
		linePath.moveTo(line.p1());
		linePath.lineTo(line.p2());
	}
	
	recalcShapeAndBoundingRect();
}

/*!
  recalculates the painter path for the drop lines.
  Called each time when the type of the drop lines is changed.
*/
void LineSymbolCurve::Private::updateDropLines(){
  	dropLinePath = QPainterPath();
	if (dropLineType == LineSymbolCurve::NoDropLine){
	  recalcShapeAndBoundingRect();
	  return;
	}
	
	QList<QLineF> lines;
	QPointF tempPoint;
	int count=symbolPointsLogical.count();
	switch(dropLineType){
	  case LineSymbolCurve::DropLineX:{
		for (int i=0; i<count; i++){
		  lines.append(QLineF(symbolPointsLogical.at(i), QPointF(symbolPointsLogical.at(i).x(), 0)));
		}
		break;
	  }
	  case LineSymbolCurve::DropLineY:{
		for (int i=0; i<count; i++){
		  lines.append(QLineF(symbolPointsLogical.at(i), QPointF(0, symbolPointsLogical.at(i).y())));
		}
		break;
	  }
	  case LineSymbolCurve::DropLineXY:{
		for (int i=0; i<count; i++){
		  lines.append(QLineF(symbolPointsLogical.at(i), QPointF(symbolPointsLogical.at(i).x(), 0)));
		  lines.append(QLineF(symbolPointsLogical.at(i), QPointF(0, symbolPointsLogical.at(i).y())));
		}
		break;
	  }
	  default:
		break;
	}
	
	const AbstractCoordinateSystem *cSystem = q->coordinateSystem();
	if (cSystem)
	  lines = cSystem->mapLogicalToScene(lines);
	
	foreach (QLineF line, lines) {
		dropLinePath.moveTo(line.p1());
		dropLinePath.lineTo(line.p2());
	}
	
	recalcShapeAndBoundingRect();
}

void LineSymbolCurve::Private::updateValues(){
  	valuesPath = QPainterPath();
	valuesPoints.clear();
	valuesStrings.clear();
	
	if (valuesType== LineSymbolCurve::NoValues){
	  recalcShapeAndBoundingRect();
	  return;
	}
	
	//determine the value string for all points
	switch (valuesType){
	  case LineSymbolCurve::ValuesX:{
		foreach(QPointF point, symbolPointsLogical){
 			valuesStrings << valuesPrefix + QString().setNum(point.x()) + valuesSuffix;
		}
	  break;
	  }
	  case LineSymbolCurve::ValuesY:{
		foreach(QPointF point, symbolPointsLogical){
 			valuesStrings << valuesPrefix + QString().setNum(point.y()) + valuesSuffix;
		}
		break;
	  }
	  case LineSymbolCurve::ValuesXY:{
		foreach(QPointF point, symbolPointsLogical){
			valuesStrings << valuesPrefix + QString().setNum(point.x()) + "," + QString().setNum(point.y()) + valuesSuffix;
		}
		break;
	  }
	  case LineSymbolCurve::ValuesXYBracketed:{
		foreach(QPointF point, symbolPointsLogical){
			valuesStrings <<  valuesPrefix + "(" + QString().setNum(point.x()) + "," + QString().setNum(point.y()) +")" + valuesSuffix;
		}
		break;
	  }
	  case LineSymbolCurve::ValuesCustomColumn:{
		//TODO
// 		if (!valuesColumn){
// 		  recalcShapeAndBoundingRect();
// 		  return;
// 		}
		recalcShapeAndBoundingRect();
		return;
	  }
	}
	qDebug()<<valuesStrings;
		
	//determine the value strings for points only that are visible on the coordinate system
// 	const AbstractCoordinateSystem *cSystem = q->coordinateSystem();
// 	if (cSystem)
	  //TODO: implement this function
// 	  valuesStrings = cSystem->mapLogicalToScene(symbolPointsLogical, valuesStrings);	
// 
	
	//calculate the coordinates where to paint the value strings	
	QPointF tempPoint;
	QFontMetrics fm(valuesFont);
	qreal w;
	qreal h=fm.ascent();
	switch(valuesPosition){
	  case LineSymbolCurve::ValuesAbove:{
		for (int i=0; i<symbolPoints.size(); i++){
		  w=fm.width(valuesStrings.at(i));
		  tempPoint.setX( symbolPoints.at(i).x() - w/2);
		  tempPoint.setY( symbolPoints.at(i).y() - valuesDistance );
		  valuesPoints.append(tempPoint);
		  }
		  break;
		}
	  case LineSymbolCurve::ValuesUnder:{
		for (int i=0; i<symbolPoints.size(); i++){
		  w=fm.width(valuesStrings.at(i));
		  tempPoint.setX( symbolPoints.at(i).x() -w/2 );
		  tempPoint.setY( symbolPoints.at(i).y() + valuesDistance + h/2);
		  valuesPoints.append(tempPoint);
		}
		break;
	  }
	  case LineSymbolCurve::ValuesLeft:{
		for (int i=0; i<symbolPoints.size(); i++){
		  w=fm.width(valuesStrings.at(i));
		  tempPoint.setX( symbolPoints.at(i).x() - valuesDistance - w - 1 );
		  tempPoint.setY( symbolPoints.at(i).y());
		  valuesPoints.append(tempPoint);
		}
		break;
	  }
	  case LineSymbolCurve::ValuesRight:{
		for (int i=0; i<symbolPoints.size(); i++){
		  w=fm.width(valuesStrings.at(i));
		  tempPoint.setX( symbolPoints.at(i).x() + valuesDistance - 1 );
		  tempPoint.setY( symbolPoints.at(i).y() );
		  valuesPoints.append(tempPoint);
		}
		break;
	  }
	  default:
	  break;
	}
	
	recalcShapeAndBoundingRect();
}

void LineSymbolCurve::Private::recalcShapeAndBoundingRect() {
	prepareGeometryChange();
	boundingRectangle = QRectF();
	boundingRectangle = boundingRectangle.normalized();
	curveShape = QPainterPath();
	
	if (lineType != LineSymbolCurve::NoLine){
		curveShape.addPath(AbstractWorksheetElement::shapeFromPath(linePath, linePen));
		boundingRectangle = boundingRectangle.united(linePath.boundingRect());
	}
	
	if (dropLineType != LineSymbolCurve::NoDropLine){
		curveShape.addPath(AbstractWorksheetElement::shapeFromPath(dropLinePath, dropLinePen));
		boundingRectangle = boundingRectangle.united(dropLinePath.boundingRect());
	}
	
	if (symbolPrototype->id() !=" none"){
	  QPainterPath symbolsPath;
	  QRectF prototypeBoundingRect = symbolPrototype->boundingRect();
	  foreach (QPointF point, symbolPoints) {
		  prototypeBoundingRect.moveCenter(point); 
		  boundingRectangle |= prototypeBoundingRect;
		  symbolsPath.addEllipse(prototypeBoundingRect);
	  }
	  curveShape.addPath(AbstractWorksheetElement::shapeFromPath(symbolsPath, symbolsPen));
	}

	if (valuesPosition != LineSymbolCurve::NoValues){
	  	for (int i=0; i<symbolPoints.size(); i++){
		  valuesPath.addText( valuesPoints.at(i), valuesFont, valuesStrings.at(i) );
		}
		curveShape.addPath(AbstractWorksheetElement::shapeFromPath(valuesPath, valuesPen));
		boundingRectangle = boundingRectangle.united(valuesPath.boundingRect());
	}
	
	update();
}

QString LineSymbolCurve::Private::swapSymbolTypeId(const QString &id) {
  
	QString oldId = symbolTypeId;
	if (id != symbolTypeId)
	{
		symbolTypeId = id;
		delete symbolPrototype;
		symbolPrototype = NULL;
		if (symbolTypeId != "ellipse") {
			foreach(QObject *plugin, PluginManager::plugins()) {
				CurveSymbolFactory *factory = qobject_cast<CurveSymbolFactory *>(plugin);
				if (factory) {
					const AbstractCurveSymbol *prototype = factory->prototype(symbolTypeId);
					if (prototype)
					{
						symbolPrototype = prototype->clone();
						break;
					}
				}
			}
		}
	}
	if (!symbolPrototype) // safety fallback
		symbolPrototype = EllipseCurveSymbol::staticPrototype()->clone();

	symbolPrototype->setSize(symbolSize);
	symbolPrototype->setAspectRatio(symbolAspectRatio);
	symbolPrototype->setBrush(symbolsBrush);
	symbolPrototype->setPen(symbolsPen);
	symbolPrototype->setRotationAngle(symbolRotationAngle);
	recalcShapeAndBoundingRect();
	return oldId;
	
}

void LineSymbolCurve::Private::updateSymbol(){
	swapSymbolTypeId(symbolTypeId);
}

void LineSymbolCurve::Private::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget * widget){
  if (!isVisible())
	return;
  
  //draw lines
  qreal opacity=painter->opacity();
  if (lineType != LineSymbolCurve::NoLine){
	painter->setOpacity(lineOpacity);
	painter->setPen(linePen);
	painter->setBrush(Qt::NoBrush);
	painter->drawPath(linePath);
  }

  //draw drop lines
  if (dropLineType != LineSymbolCurve::NoDropLine){
	painter->setOpacity(dropLineOpacity);
	painter->setPen(dropLinePen);
	painter->setBrush(Qt::NoBrush);
	painter->drawPath(dropLinePath);
  }
  
  //draw symbols
  if (symbolPrototype->id()!="none"){
	  painter->setOpacity(symbolsOpacity);
// 	  painter->setPen(symbolsPen);
// 	  painter->setBrush(symbolsBrush);
// 	  painter->rotate(symbolRotationAngle);
	  foreach(QPointF point, symbolPoints){
		  painter->translate(point);
		  symbolPrototype->paint(painter, option, widget);
		  painter->translate(-point);
	  }
  }
  
  //draw values
  if (valuesType != LineSymbolCurve::NoValues){
	painter->setOpacity(valuesOpacity);
 	painter->setPen(valuesPen);	
	painter->setBrush(Qt::NoBrush);
// 	painter->rotate(valuesRotationAngle);
	painter->setFont(valuesFont);
	for (int i=0; i<symbolPoints.size(); i++){
// 	painter->save();
// 	  painter->rotate(valuesRotationAngle);
	  painter->drawText(valuesPoints.at(i), valuesStrings.at(i) );
// 	  painter->restore();
	}
  }
// 	painter->rotate(-valuesRotationAngle);
  
  painter->setOpacity(opacity);
}
