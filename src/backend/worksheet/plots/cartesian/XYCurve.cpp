/***************************************************************************
    File                 : XYCurve.cpp
    Project              : LabPlot/SciDAVis
    Description          : A 2D-curve.
    --------------------------------------------------------------------
    Copyright            : (C) 2009 Tilman Benkert (thzs*gmx.net)
    Copyright            : (C) 2010-2012 Alexander Semke (alexander.semke*web.de)
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

/*!
  \class XYCurve
  \brief A 2D-curve, provides an interface for editing many properties of the the curve.
 
  \ingroup kdefrontend
*/

#include "XYCurve.h"
#include "XYCurvePrivate.h"
#include "../AbstractCoordinateSystem.h"
#include "CartesianCoordinateSystem.h"
#include "../AbstractPlot.h"
#include "../../../lib/commandtemplates.h"
#include "./../../../core/plugin/PluginManager.h"
#include "../../symbols/EllipseCurveSymbol.h"
#include "../../interfaces.h"
#include "../../Worksheet.h"

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

XYCurve::XYCurve(const QString &name)
		: AbstractWorksheetElement(name), d_ptr(new XYCurvePrivate(this)){
	init();
}

XYCurve::XYCurve(const QString &name, XYCurvePrivate *dd)
		: AbstractWorksheetElement(name), d_ptr(dd){
	init();
}

void XYCurve::init(){
	Q_D(XYCurve);
	
  	d->xColumn = NULL;
	d->yColumn = NULL;
	
	d->lineType = XYCurve::NoLine;
	d->lineInterpolationPointsCount = 1;
	d->lineOpacity = 1.0;
	d->dropLineType = XYCurve::NoDropLine;
	d->dropLineOpacity = 1.0;
	
	d->symbolsOpacity = 1.0;
	d->symbolRotationAngle = 0;
	d->symbolSize = Worksheet::convertToSceneUnits( 5, Worksheet::Point  );
	d->symbolAspectRatio = 1;
	d->symbolPrototype = NULL;
	d->swapSymbolTypeId("diamond");

	d->valuesType = XYCurve::NoValues;
	d->valuesColumn = NULL;	
	d->valuesPosition = XYCurve::ValuesAbove;
	d->valuesDistance =  Worksheet::convertToSceneUnits( 5, Worksheet::Point );
	d->valuesRotationAngle = 0;
	d->valuesOpacity = 1.0;
	d->valuesFont.setPointSizeF( Worksheet::convertToSceneUnits( 10, Worksheet::Point ) );
	
 	// TODO: remove this temporary code later
	d->symbolPrototype->setBrush(QBrush(Qt::red));
	
	graphicsItem()->setFlag(QGraphicsItem::ItemIsSelectable, true);
	retransform();
}

XYCurve::~XYCurve() {
	delete d_ptr;
}

QStringList XYCurve::lineTypeStrings(){
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

QStringList XYCurve::dropLineTypeStrings(){
  //TODO add i18n-Version
  return ( QStringList()<<tr("none")
			<<tr("drop lines, X")<<tr("drop lines, Y")<<tr("drop lines, XY") );
}

QStringList XYCurve::valuesTypeStrings(){
  //TODO add i18n-Version
  return ( QStringList()<<tr("no values")<<"x"<<"y"<<"x, y"<<"(x, y)"<<tr("custom column") );
}

QStringList XYCurve::valuesPositionStrings(){
  //TODO add i18n-Version
  return ( QStringList()<<tr("above")<<tr("under")<<tr("left")<<tr("right") );
}

QGraphicsItem *XYCurve::graphicsItem() const{
	return d_ptr;
}


STD_SWAP_METHOD_SETTER_CMD_IMPL(XYCurve, SetVisible, bool, swapVisible);
void XYCurve::setVisible(bool on){
	Q_D(XYCurve);
	exec(new XYCurveSetVisibleCmd(d, on, on ? tr("%1: set visible") : tr("%1: set invisible")));
}

bool XYCurve::isVisible() const{
	Q_D(const XYCurve);
	return d->isVisible();
}

void XYCurve::retransform(){
	d_ptr->retransform();
}

void XYCurve::handlePageResize(double horizontalRatio, double verticalRatio){
  //TODO
	Q_D(const XYCurve);
	
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
	
	retransform();
}



/* ============================ accessor documentation ================= */
//TODO provide a proper documentation for all functions implemented with the help of the macros

/**
  \fn   XYCurve::POINTER_D_ACCESSOR_DECL(const AbstractColumn, xColumn, XColumn);
  \brief Set/get the pointer to the X column.
*/
/**
  \fn   XYCurve::POINTER_D_ACCESSOR_DECL(const AbstractColumn, yColumn, YColumn);
  \brief Set/get the pointer to the Y column.
*/

//line
/**
  \fn   XYCurve::CLASS_D_ACCESSOR_DECL(XYCurve::LineType, lineType , LineType);
  \brief Set/get the line type.
*/
/**
  \fn   XYCurve::BASIC_D_ACCESSOR_DECL(bool, lineVisible, LineVisible);
  \brief Set/get whether the line is visible/invisible.
*/
/**
  \fn   XYCurve::CLASS_D_ACCESSOR_DECL(QPen, linePen, LinePen);
  \brief Get/set the line pen.
*/

//symbols
/**
  \fn   XYCurve::BASIC_D_ACCESSOR_DECL(bool, symbolsVisible, SymbolsVisible);
  \brief Set/get whether the symbols are visible/invisible.
*/
/**
  \fn   XYCurve::BASIC_D_ACCESSOR_DECL(bool, symbolsOpacity, SymbolsOpacity);
  \brief Set/get the opacity of the symbols. The opacity ranges from 0.0 to 1.0, where 0.0 is fully transparent and 1.0 is fully opaque.
*/
/**
  \fn   XYCurve::BASIC_D_ACCESSOR_DECL(qreal, symbolRotationAngle, SymbolRotationAngle);
  \brief Set/get the rotation angle of the symbols.
*/
/**
  \fn   XYCurve::BASIC_D_ACCESSOR_DECL(qreal, symbolSize, SymbolSize);
  \brief Set/get the (horizontal) size of the symbols.
*/
/**
  \fn   XYCurve::BASIC_D_ACCESSOR_DECL(qreal, symbolAspectRatio, SymbolAspectRatio);
  \brief Set/get the ratio between the width and height of the symbols.
*/
/**
  \fn   XYCurve::CLASS_D_ACCESSOR_DECL(QString, symbolTypeId, SymbolTypeId);
  \brief Set/get the symbol type.
*/
/**
  \fn   XYCurve::CLASS_D_ACCESSOR_DECL(QBrush, symbolsBrush, SymbolsBrush);
  \brief Get/set the symbol filling brush.
*/
/**
  \fn   XYCurve::CLASS_D_ACCESSOR_DECL(QPen, symbolsPen, SymbolsPen);
  \brief Get/set the symbol outline pen.
*/

//values

/* ============================ getter methods ================= */
BASIC_SHARED_D_READER_IMPL(XYCurve, const AbstractColumn *, xColumn, xColumn);
BASIC_SHARED_D_READER_IMPL(XYCurve, const AbstractColumn *, yColumn, yColumn);

//line
BASIC_SHARED_D_READER_IMPL(XYCurve, XYCurve::LineType, lineType, lineType);
BASIC_SHARED_D_READER_IMPL(XYCurve, int, lineInterpolationPointsCount, lineInterpolationPointsCount);
CLASS_SHARED_D_READER_IMPL(XYCurve, QPen, linePen, linePen);
BASIC_SHARED_D_READER_IMPL(XYCurve, qreal, lineOpacity, lineOpacity);

//droplines
BASIC_SHARED_D_READER_IMPL(XYCurve, XYCurve::DropLineType, dropLineType, dropLineType);
CLASS_SHARED_D_READER_IMPL(XYCurve, QPen, dropLinePen, dropLinePen);
BASIC_SHARED_D_READER_IMPL(XYCurve, qreal, dropLineOpacity, dropLineOpacity);

//symbols
BASIC_SHARED_D_READER_IMPL(XYCurve, qreal, symbolsOpacity, symbolsOpacity);
BASIC_SHARED_D_READER_IMPL(XYCurve, qreal, symbolRotationAngle, symbolRotationAngle);
BASIC_SHARED_D_READER_IMPL(XYCurve, qreal, symbolSize, symbolSize);
BASIC_SHARED_D_READER_IMPL(XYCurve, qreal, symbolAspectRatio, symbolAspectRatio);
CLASS_SHARED_D_READER_IMPL(XYCurve, QString, symbolTypeId, symbolTypeId);
CLASS_SHARED_D_READER_IMPL(XYCurve, QBrush, symbolsBrush, symbolsBrush);
CLASS_SHARED_D_READER_IMPL(XYCurve, QPen, symbolsPen, symbolsPen);

//values
BASIC_SHARED_D_READER_IMPL(XYCurve, XYCurve::ValuesType, valuesType, valuesType);
BASIC_SHARED_D_READER_IMPL(XYCurve, const AbstractColumn *, valuesColumn, valuesColumn);
BASIC_SHARED_D_READER_IMPL(XYCurve, XYCurve::ValuesPosition, valuesPosition, valuesPosition);
BASIC_SHARED_D_READER_IMPL(XYCurve, qreal, valuesDistance, valuesDistance);
BASIC_SHARED_D_READER_IMPL(XYCurve, qreal, valuesRotationAngle, valuesRotationAngle);
BASIC_SHARED_D_READER_IMPL(XYCurve, qreal, valuesOpacity, valuesOpacity);
CLASS_SHARED_D_READER_IMPL(XYCurve, QString, valuesPrefix, valuesPrefix);
CLASS_SHARED_D_READER_IMPL(XYCurve, QString, valuesSuffix, valuesSuffix);
CLASS_SHARED_D_READER_IMPL(XYCurve, QPen, valuesPen, valuesPen);
CLASS_SHARED_D_READER_IMPL(XYCurve, QFont, valuesFont, valuesFont);

/* ============================ setter methods and undo commands ================= */

STD_SETTER_CMD_IMPL_F(XYCurve, SetXColumn, const AbstractColumn *, xColumn, retransform);
void XYCurve::setXColumn(const AbstractColumn *xColumn) {
	Q_D(XYCurve);
	if (xColumn != d->xColumn)
		exec(new XYCurveSetXColumnCmd(d, xColumn, tr("%1: assign x values")));
}

STD_SETTER_CMD_IMPL_F(XYCurve, SetYColumn, const AbstractColumn *, yColumn, retransform);
void XYCurve::setYColumn(const AbstractColumn *yColumn) {
	Q_D(XYCurve);
	if (yColumn != d->yColumn)
		exec(new XYCurveSetYColumnCmd(d, yColumn, tr("%1: assign y values")));
}

//Line
STD_SETTER_CMD_IMPL_F(XYCurve, SetLineType, XYCurve::LineType, lineType, updateLines);
void XYCurve::setLineType(LineType type) {
	Q_D(XYCurve);
	if (type != d->lineType)
		exec(new XYCurveSetLineTypeCmd(d, type, tr("%1: line type changed")));
}

STD_SETTER_CMD_IMPL_F(XYCurve, SetLineInterpolationPointsCount, int, lineInterpolationPointsCount, updateLines);
void XYCurve::setLineInterpolationPointsCount(int count) {
	Q_D(XYCurve);
	if (count != d->lineInterpolationPointsCount)
		exec(new XYCurveSetLineInterpolationPointsCountCmd(d, count, tr("%1: set the number of interpolation points")));
}

STD_SETTER_CMD_IMPL_F(XYCurve, SetLinePen, QPen, linePen, recalcShapeAndBoundingRect);
void XYCurve::setLinePen(const QPen &pen) {
	Q_D(XYCurve);
	if (pen != d->linePen)
		exec(new XYCurveSetLinePenCmd(d, pen, tr("%1: set line style")));
}

STD_SETTER_CMD_IMPL_F(XYCurve, SetLineOpacity, qreal, lineOpacity, update);
void XYCurve::setLineOpacity(qreal opacity) {
	Q_D(XYCurve);
	if (opacity != d->lineOpacity)
		exec(new XYCurveSetLineOpacityCmd(d, opacity, tr("%1: set line opacity")));
}

//Drop lines
STD_SETTER_CMD_IMPL_F(XYCurve, SetDropLineType, XYCurve::DropLineType, dropLineType, updateDropLines);
void XYCurve::setDropLineType(DropLineType type) {
	Q_D(XYCurve);
	if (type != d->dropLineType)
		exec(new XYCurveSetDropLineTypeCmd(d, type, tr("%1: drop line type changed")));
}

STD_SETTER_CMD_IMPL_F(XYCurve, SetDropLinePen, QPen, dropLinePen, recalcShapeAndBoundingRect);
void XYCurve::setDropLinePen(const QPen &pen) {
	Q_D(XYCurve);
	if (pen != d->dropLinePen)
		exec(new XYCurveSetDropLinePenCmd(d, pen, tr("%1: set drop line style")));
}

STD_SETTER_CMD_IMPL_F(XYCurve, SetDropLineOpacity, qreal, dropLineOpacity, update);
void XYCurve::setDropLineOpacity(qreal opacity) {
	Q_D(XYCurve);
	if (opacity != d->dropLineOpacity)
		exec(new XYCurveSetDropLineOpacityCmd(d, opacity, tr("%1: set drop line opacity")));
}

// Symbols 
STD_SETTER_CMD_IMPL_F(XYCurve, SetSymbolsOpacity, qreal, symbolsOpacity, update);
void XYCurve::setSymbolsOpacity(qreal opacity) {
	Q_D(XYCurve);
	if (opacity != d->symbolsOpacity)
		exec(new XYCurveSetSymbolsOpacityCmd(d, opacity, tr("%1: set symbols opacity")));
}


STD_SETTER_CMD_IMPL_F(XYCurve, SetSymbolRotationAngle, qreal, symbolRotationAngle,updateSymbol);
void XYCurve::setSymbolRotationAngle(qreal angle) {
	Q_D(XYCurve);
	if (!qFuzzyCompare(1 + angle, 1 + d->symbolRotationAngle))
		exec(new XYCurveSetSymbolRotationAngleCmd(d, angle, tr("%1: rotate symbols")));
}

STD_SETTER_CMD_IMPL_F(XYCurve, SetSymbolSize, qreal, symbolSize, updateSymbol);
void XYCurve::setSymbolSize(qreal size) {
	Q_D(XYCurve);
	if (!qFuzzyCompare(1 + size, 1 + d->symbolSize))
		exec(new XYCurveSetSymbolSizeCmd(d, size, tr("%1: set symbol size")));
}

//TODO ???
STD_SETTER_CMD_IMPL_F(XYCurve, SetSymbolAspectRatio, qreal, symbolAspectRatio, updateSymbol);
void XYCurve::setSymbolAspectRatio(qreal ratio) {
	Q_D(XYCurve);
	if (!qFuzzyCompare(1 + ratio, 1 + d->symbolAspectRatio))
		exec(new XYCurveSetSymbolAspectRatioCmd(d, ratio, tr("%1: set symbol aspect ratio")));
}

STD_SWAP_METHOD_SETTER_CMD_IMPL(XYCurve, SetSymbolTypeId, QString, swapSymbolTypeId);
void XYCurve::setSymbolTypeId(const QString &id) {
	Q_D(XYCurve);
	if (id != d->symbolTypeId)
		exec(new XYCurveSetSymbolTypeIdCmd(d, id, tr("%1: set symbol type")));
}

STD_SETTER_CMD_IMPL_F(XYCurve, SetSymbolsBrush, QBrush, symbolsBrush, update);
void XYCurve::setSymbolsBrush(const QBrush &brush) {
	Q_D(XYCurve);
	if (brush != d->symbolsBrush)
		exec(new XYCurveSetSymbolsBrushCmd(d, brush, tr("%1: set symbol filling")));
}

STD_SETTER_CMD_IMPL_F(XYCurve, SetSymbolsPen, QPen, symbolsPen, recalcShapeAndBoundingRect);
void XYCurve::setSymbolsPen(const QPen &pen) {
	Q_D(XYCurve);
	if (pen != d->symbolsPen)
		exec(new XYCurveSetSymbolsPenCmd(d, pen, tr("%1: set symbol outline style")));
}

//Values
STD_SETTER_CMD_IMPL_F(XYCurve, SetValuesType, XYCurve::ValuesType, valuesType, updateValues);
void XYCurve::setValuesType(XYCurve::ValuesType type) {
	Q_D(XYCurve);
	if (type != d->valuesType)
		exec(new XYCurveSetValuesTypeCmd(d, type, tr("%1: set values type")));
}

STD_SETTER_CMD_IMPL_F(XYCurve, SetValuesColumn, const AbstractColumn *, valuesColumn, updateValues	);
void XYCurve::setValuesColumn(const AbstractColumn *valuesColumn) {
	Q_D(XYCurve);
	if (valuesColumn != d->valuesColumn)
		exec(new XYCurveSetValuesColumnCmd(d, valuesColumn, tr("%1: set values column")));
}

STD_SETTER_CMD_IMPL_F(XYCurve, SetValuesPosition, XYCurve::ValuesPosition, valuesPosition, updateValues);
void XYCurve::setValuesPosition(ValuesPosition position) {
	Q_D(XYCurve);
	if (position != d->valuesPosition)
		exec(new XYCurveSetValuesPositionCmd(d, position, tr("%1: set values position")));
}

STD_SETTER_CMD_IMPL_F(XYCurve, SetValuesDistance, qreal, valuesDistance, updateValues);
void XYCurve::setValuesDistance(qreal distance) {
	Q_D(XYCurve);
	if (distance != d->valuesDistance)
		exec(new XYCurveSetValuesDistanceCmd(d, distance, tr("%1: set values distance")));
}

STD_SETTER_CMD_IMPL_F(XYCurve, SetValuesOpacity, qreal, valuesOpacity, update);
void XYCurve::setValuesOpacity(qreal opacity) {
	Q_D(XYCurve);
	if (opacity != d->valuesOpacity)
		exec(new XYCurveSetValuesOpacityCmd(d, opacity, tr("%1: set values opacity")));
}

STD_SETTER_CMD_IMPL_F(XYCurve, SetValuesRotationAngle, qreal, valuesRotationAngle,update);
void XYCurve::setValuesRotationAngle(qreal angle) {
	Q_D(XYCurve);
	if (!qFuzzyCompare(1 + angle, 1 + d->valuesRotationAngle))
		exec(new XYCurveSetValuesRotationAngleCmd(d, angle, tr("%1: rotate values")));
}

STD_SETTER_CMD_IMPL_F(XYCurve, SetValuesPrefix, QString, valuesPrefix, updateValues);
void XYCurve::setValuesPrefix(const QString& prefix) {
	Q_D(XYCurve);
	if (prefix!= d->valuesPrefix)
		exec(new XYCurveSetValuesPrefixCmd(d, prefix, tr("%1: set values prefix")));
}

STD_SETTER_CMD_IMPL_F(XYCurve, SetValuesSuffix, QString, valuesSuffix, updateValues);
void XYCurve::setValuesSuffix(const QString& suffix) {
	Q_D(XYCurve);
	if (suffix!= d->valuesSuffix)
		exec(new XYCurveSetValuesSuffixCmd(d, suffix, tr("%1: set values suffix")));
}

STD_SETTER_CMD_IMPL_F(XYCurve, SetValuesFont, QFont, valuesFont, updateValues);
void XYCurve::setValuesFont(const QFont& font) {
	Q_D(XYCurve);
	if (font!= d->valuesFont)
		exec(new XYCurveSetValuesFontCmd(d, font, tr("%1: set values font")));
}

STD_SETTER_CMD_IMPL_F(XYCurve, SetValuesPen, QPen, valuesPen, recalcShapeAndBoundingRect);
void XYCurve::setValuesPen(const QPen &pen) {
	Q_D(XYCurve);
	if (pen != d->valuesPen)
		exec(new XYCurveSetValuesPenCmd(d, pen, tr("%1: set values style")));
}


//################################################################
//################### Private implementation ##########################
//################################################################
XYCurvePrivate::XYCurvePrivate(XYCurve *owner): q(owner){
}

XYCurvePrivate::~XYCurvePrivate() {
}

QString XYCurvePrivate::name() const{
  return q->name();
}

QRectF XYCurvePrivate::boundingRect() const{
  return boundingRectangle;
}

/*!
  Returns the shape of the XYCurve as a QPainterPath in local coordinates
*/
QPainterPath XYCurvePrivate::shape() const{
  return curveShape;
}

bool XYCurvePrivate::swapVisible(bool on){
	bool oldValue = isVisible();
	setVisible(on);
	return oldValue;
}

/*!
  recalculates the position of the points to be drawn. Called when the data was changed.
  Triggers the update of lines, drop lines, symbols etc.
*/
void XYCurvePrivate::retransform(){
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


	AbstractPlot *plot = qobject_cast<AbstractPlot*>(q->parentAspect());
	const AbstractCoordinateSystem *cSystem = plot->coordinateSystem();
	if (cSystem)
		symbolPoints = cSystem->mapLogicalToScene(symbolPointsLogical);
	
  updateLines();
  updateDropLines();
  updateValues();
}

/*!
  recalculates the painter path for the lines connecting the data points.
  Called each time when the type of this connection is changed.
*/
void XYCurvePrivate::updateLines(){
  	linePath = QPainterPath();
	if (lineType == XYCurve::NoLine){
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
	  case XYCurve::Line:{
		for (int i=0; i<count-1; i++){
		  lines.append(QLineF(symbolPointsLogical.at(i), symbolPointsLogical.at(i+1)));
		}
		break;
	  }
	  case XYCurve::StartHorizontal:{
		for (int i=0; i<count-1; i++){
		  curPoint=symbolPointsLogical.at(i);
		  nextPoint=symbolPointsLogical.at(i+1);
		  tempPoint1=QPointF(nextPoint.x(), curPoint.y());
		  lines.append(QLineF(curPoint, tempPoint1));
		  lines.append(QLineF(tempPoint1, nextPoint));
		}
		break;
	  }
	  case XYCurve::StartVertical:{
		for (int i=0; i<count-1; i++){
		  curPoint=symbolPointsLogical.at(i);
		  nextPoint=symbolPointsLogical.at(i+1);
		  tempPoint1=QPointF(curPoint.x(), nextPoint.y());
		  lines.append(QLineF(curPoint, tempPoint1));
		  lines.append(QLineF(tempPoint1,nextPoint));
		}
		break;
	  }
	  case XYCurve::MidpointHorizontal:{
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
	  case XYCurve::MidpointVertical:{
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
	  case XYCurve::Segments2:{
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
	  case XYCurve::Segments3:{
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
	  case XYCurve::SplineCubicNatural:
	  case XYCurve::SplineCubicPeriodic:
	  case XYCurve::SplineAkimaNatural:
	  case XYCurve::SplineAkimaPeriodic:{
		//TODO: optimize! try to ommit the copying from the column to the arrays of doubles.
		gsl_interp_accel *acc  = gsl_interp_accel_alloc();
		gsl_spline *spline=0;
		  
		double x[count],  y[count];
		for (int i=0; i<count; i++){
		  x[i]=symbolPointsLogical.at(i).x();
		  y[i]=symbolPointsLogical.at(i).y();
		}
		
		if (lineType==XYCurve::SplineCubicNatural){
		  spline  = gsl_spline_alloc (gsl_interp_cspline, count);
		}else if (lineType==XYCurve::SplineCubicPeriodic){
			spline  = gsl_spline_alloc (gsl_interp_cspline_periodic, count);
		}else if (lineType==XYCurve::SplineAkimaNatural){
		  spline  = gsl_spline_alloc (gsl_interp_akima, count);
		}else if (lineType==XYCurve::SplineAkimaPeriodic){
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
		
		for (unsigned int i=0; i<xinterp.size()-1; i++){
		  lines.append(QLineF(xinterp[i], yinterp[i], xinterp[i+1], yinterp[i+1]));
		}
		
		gsl_spline_free (spline);
        gsl_interp_accel_free (acc);
		break;
	  }
	  default:
		break;
	}
	
	AbstractPlot *plot = qobject_cast<AbstractPlot*>(q->parentAspect());
	const AbstractCoordinateSystem *cSystem = plot->coordinateSystem();
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
void XYCurvePrivate::updateDropLines(){
  	dropLinePath = QPainterPath();
	if (dropLineType == XYCurve::NoDropLine){
	  recalcShapeAndBoundingRect();
	  return;
	}
	
	QList<QLineF> lines;
	QPointF tempPoint;
	int count=symbolPointsLogical.count();
	switch(dropLineType){
	  case XYCurve::DropLineX:{
		for (int i=0; i<count; i++){
		  lines.append(QLineF(symbolPointsLogical.at(i), QPointF(symbolPointsLogical.at(i).x(), 0)));
		}
		break;
	  }
	  case XYCurve::DropLineY:{
		for (int i=0; i<count; i++){
		  lines.append(QLineF(symbolPointsLogical.at(i), QPointF(0, symbolPointsLogical.at(i).y())));
		}
		break;
	  }
	  case XYCurve::DropLineXY:{
		for (int i=0; i<count; i++){
		  lines.append(QLineF(symbolPointsLogical.at(i), QPointF(symbolPointsLogical.at(i).x(), 0)));
		  lines.append(QLineF(symbolPointsLogical.at(i), QPointF(0, symbolPointsLogical.at(i).y())));
		}
		break;
	  }
	  default:
		break;
	}
	
	AbstractPlot *plot = qobject_cast<AbstractPlot*>(q->parentAspect());
	const AbstractCoordinateSystem *cSystem = plot->coordinateSystem();
	if (cSystem)
	  lines = cSystem->mapLogicalToScene(lines);
	
	foreach (QLineF line, lines) {
		dropLinePath.moveTo(line.p1());
		dropLinePath.lineTo(line.p2());
	}
	
	recalcShapeAndBoundingRect();
}

/*!
  recreates the value strings to be shown and recalculates their draw position.
*/
void XYCurvePrivate::updateValues(){
//   return;
  	valuesPath = QPainterPath();
	valuesPoints.clear();
	valuesStrings.clear();
	
	if (valuesType== XYCurve::NoValues){
	  recalcShapeAndBoundingRect();
	  return;
	}
	
	//determine the value string for all points
	switch (valuesType){
	  case XYCurve::NoValues:
	  case XYCurve::ValuesX:{
		foreach(QPointF point, symbolPointsLogical){
 			valuesStrings << valuesPrefix + QString().setNum(point.x()) + valuesSuffix;
		}
	  break;
	  }
	  case XYCurve::ValuesY:{
		foreach(QPointF point, symbolPointsLogical){
 			valuesStrings << valuesPrefix + QString().setNum(point.y()) + valuesSuffix;
		}
		break;
	  }
	  case XYCurve::ValuesXY:{
		foreach(QPointF point, symbolPointsLogical){
			valuesStrings << valuesPrefix + QString().setNum(point.x()) + "," + QString().setNum(point.y()) + valuesSuffix;
		}
		break;
	  }
	  case XYCurve::ValuesXYBracketed:{
		foreach(QPointF point, symbolPointsLogical){
			valuesStrings <<  valuesPrefix + "(" + QString().setNum(point.x()) + "," + QString().setNum(point.y()) +")" + valuesSuffix;
		}
		break;
	  }
	  case XYCurve::ValuesCustomColumn:{
		if (!valuesColumn){
		  recalcShapeAndBoundingRect();
		  return;
		}

		int endRow;
		if (symbolPointsLogical.size()>valuesColumn->rowCount())
		  endRow =  valuesColumn->rowCount();
		else
		  endRow = symbolPointsLogical.size();

		SciDAVis::ColumnMode xColMode = valuesColumn->columnMode();
		for (int row=0; row<endRow; row++){
		  if ( !valuesColumn->isValid(row) || valuesColumn->isMasked(row) )
			continue;

		  switch (xColMode){
				case SciDAVis::Numeric:
				  valuesStrings << valuesPrefix + QString().setNum(valuesColumn->valueAt(row)) + valuesSuffix;
					break;
				case SciDAVis::Text:
					valuesStrings << valuesPrefix + valuesColumn->textAt(row) + valuesSuffix;
				case SciDAVis::DateTime:
				case SciDAVis::Month:
				case SciDAVis::Day:
					//TODO
					break;
				default:
					break;
			}
		}

	  }
	}
	//determine the value strings for points only that are visible on the coordinate system
// 	const AbstractCoordinateSystem *cSystem = q->coordinateSystem();
// 	if (cSystem)
	  //TODO: implement this function
// 	  valuesStrings = cSystem->mapLogicalToScene(symbolPointsLogical, valuesStrings);	
// 
	
	//Calculate the coordinates where to paint the value strings.
	//The coordinates depend on the actual size of the string.
	QPointF tempPoint;
	QFontMetrics fm(valuesFont);
	qreal w;
	qreal h=fm.ascent();
	switch(valuesPosition){
	  case XYCurve::ValuesAbove:{
		for (int i=0; i<valuesStrings.size(); i++){
		  w=fm.width(valuesStrings.at(i));
		  tempPoint.setX( symbolPoints.at(i).x() - w/2);
		  tempPoint.setY( symbolPoints.at(i).y() - valuesDistance );
		  valuesPoints.append(tempPoint);
		  }
		  break;
		}
	  case XYCurve::ValuesUnder:{
		for (int i=0; i<valuesStrings.size(); i++){
		  w=fm.width(valuesStrings.at(i));
		  tempPoint.setX( symbolPoints.at(i).x() -w/2 );
		  tempPoint.setY( symbolPoints.at(i).y() + valuesDistance + h/2);
		  valuesPoints.append(tempPoint);
		}
		break;
	  }
	  case XYCurve::ValuesLeft:{
		for (int i=0; i<valuesStrings.size(); i++){
		  w=fm.width(valuesStrings.at(i));
		  tempPoint.setX( symbolPoints.at(i).x() - valuesDistance - w - 1 );
		  tempPoint.setY( symbolPoints.at(i).y());
		  valuesPoints.append(tempPoint);
		}
		break;
	  }
	  case XYCurve::ValuesRight:{
		for (int i=0; i<valuesStrings.size(); i++){
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

/*!
  recalculates the outer bounds and the shape of the curve.
*/
void XYCurvePrivate::recalcShapeAndBoundingRect() {
	prepareGeometryChange();
	boundingRectangle = QRectF();
	boundingRectangle = boundingRectangle.normalized();
	curveShape = QPainterPath();
	
	if (lineType != XYCurve::NoLine){
		curveShape.addPath(AbstractWorksheetElement::shapeFromPath(linePath, linePen));
		boundingRectangle = boundingRectangle.united(linePath.boundingRect());
	}
	
	if (dropLineType != XYCurve::NoDropLine){
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

	if (valuesType != XYCurve::NoValues){
	  	for (int i=0; i<valuesPoints.size(); i++){
		  valuesPath.addText( valuesPoints.at(i), valuesFont, valuesStrings.at(i) );
		}
		curveShape.addPath(AbstractWorksheetElement::shapeFromPath(valuesPath, valuesPen));
		boundingRectangle = boundingRectangle.united(valuesPath.boundingRect());
	}
	
	update();
}

QString XYCurvePrivate::swapSymbolTypeId(const QString &id) {
	QString oldId = symbolTypeId;
	if (id != symbolTypeId)	{
		symbolTypeId = id;
		delete symbolPrototype;
		symbolPrototype = NULL;
		if (symbolTypeId != "ellipse") {
			foreach(QObject *plugin, PluginManager::plugins()) {
				CurveSymbolFactory *factory = qobject_cast<CurveSymbolFactory *>(plugin);
				if (factory) {
					const AbstractCurveSymbol *prototype = factory->prototype(symbolTypeId);
					if (prototype)					{
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

void XYCurve::Private::updateSymbol(){
	swapSymbolTypeId(symbolTypeId);
}

/*!
  Reimplementation of QGraphicsItem::paint(). This function does the actual painting of the curve.
  \sa QGraphicsItem::paint().
*/
void XYCurvePrivate::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget * widget){
  if (!isVisible())
	return;
  
  qreal opacity=painter->opacity();
  
  //draw lines
  if (lineType != XYCurve::NoLine){
	painter->setOpacity(lineOpacity);
	painter->setPen(linePen);
	painter->setBrush(Qt::NoBrush);
	painter->drawPath(linePath);
  }

  //draw drop lines
  if (dropLineType != XYCurve::NoDropLine){
	painter->setOpacity(dropLineOpacity);
	painter->setPen(dropLinePen);
	painter->setBrush(Qt::NoBrush);
	painter->drawPath(dropLinePath);
  }
  
  //draw symbols
  if (symbolPrototype->id()!="none"){
	  painter->setOpacity(symbolsOpacity);
	  //TODO move the symbol properties from the Symbol-Class to XYCurve?
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
  if (valuesType != XYCurve::NoValues){
	painter->setOpacity(valuesOpacity);
 	painter->setPen(valuesPen);	
	painter->setBrush(Qt::NoBrush);
	painter->setFont(valuesFont);
	for (int i=0; i<valuesPoints.size(); i++){
	  painter->translate(valuesPoints.at(i));
	  painter->save();
	  painter->rotate(-valuesRotationAngle);
 	  painter->drawText(QPoint(0,0), valuesStrings.at(i) );
 	  painter->restore();
	  painter->translate(-valuesPoints.at(i));
	}
  }
  
  painter->setOpacity(opacity);
  
  if (isSelected()){
	QPainterPath path = shape();  
	painter->setPen(QPen(Qt::blue, 0, Qt::DashLine));
	painter->drawPath(path);
  }
}
