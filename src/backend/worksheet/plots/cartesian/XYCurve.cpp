/***************************************************************************
    File                 : XYCurve.cpp
    Project              : LabPlot/AbstractColumn
    Description          : A 2D-curve.
    --------------------------------------------------------------------
    Copyright            : (C) 2009 Tilman Benkert (thzs*gmx.net)
    Copyright            : (C) 2010-2013 Alexander Semke (alexander.semke*web.de)
    Copyright            : (C) 2013 Stefan Gerlach (stefan.gerlach*uni.kn)
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
 
  \ingroup worksheet
*/

#include "XYCurve.h"
#include "XYCurvePrivate.h"
#include "backend/worksheet/plots/AbstractCoordinateSystem.h"
#include "backend/worksheet/plots/cartesian/CartesianCoordinateSystem.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/lib/commandtemplates.h"
#include "backend/core/plugin/PluginManager.h"
#include "backend/worksheet/interfaces.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/lib/XmlStreamReader.h"

#include <QGraphicsItem>
#include <QGraphicsItemGroup>
#include <QGraphicsPathItem>
#include <QGraphicsEllipseItem>
#include <QPainterPath>
#include <QPainter>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QtDebug>

#ifndef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
#include <KIcon>
#endif

#ifdef HAVE_GSL
#include <gsl/gsl_spline.h>
#include <gsl/gsl_errno.h>
#endif
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

XYCurve::~XYCurve() {
	//no need to delete the d-pointer here - it inherits from QGraphicsItem
	//and is deleted during the cleanup in QGraphicsScene
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
	d->symbolsRotationAngle = 0;
	d->symbolsSize = Worksheet::convertToSceneUnits( 5, Worksheet::Point  );
	d->symbolsPrototype = NULL;
	d->swapSymbolsTypeId("diamond");

	d->valuesType = XYCurve::NoValues;
	d->valuesColumn = NULL;
	d->valuesPosition = XYCurve::ValuesAbove;
	d->valuesDistance =  Worksheet::convertToSceneUnits( 5, Worksheet::Point );
	d->valuesRotationAngle = 0;
	d->valuesOpacity = 1.0;
	d->valuesFont.setPixelSize( Worksheet::convertToSceneUnits( 6, Worksheet::Point ) );

	d->xErrorType = XYCurve::NoError;
	d->xErrorPlusColumn = NULL;
	d->xErrorMinusColumn = NULL;
	d->yErrorType = XYCurve::NoError;
	d->errorBarsCapSize = Worksheet::convertToSceneUnits(10, Worksheet::Point);
	d->yErrorPlusColumn = NULL;
	d->yErrorMinusColumn = NULL;
	d->errorBarsType = XYCurve::ErrorBarsSimple;
	d->errorBarsOpacity = 1.0;

	graphicsItem()->setFlag(QGraphicsItem::ItemIsSelectable, true);

	this->initActions();
}

void XYCurve::initActions(){
	visibilityAction = new QAction(tr("visible"), this);
	visibilityAction->setCheckable(true);
	connect(visibilityAction, SIGNAL(triggered()), this, SLOT(visibilityChanged()));
}

QMenu* XYCurve::createContextMenu(){
// 	Q_D(const XYCurve);
	QMenu *menu = AbstractWorksheetElement::createContextMenu();

#ifdef ACTIVATE_SCIDAVIS_SPECIFIC_CODE	
	QAction* firstAction = menu->actions().first();
#else
	QAction* firstAction = menu->actions().at(1); //skip the first action because of the "title-action"
#endif

	visibilityAction->setChecked(isVisible());
	menu->insertAction(firstAction, visibilityAction);

	return menu;
}

/*!
	Returns an icon to be used in the project explorer.
*/
QIcon XYCurve::icon() const{
	QIcon icon;
#ifndef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
		icon = KIcon("xy-curve");
#endif
	return icon;
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
  return ( QStringList()<<tr("above")<<tr("below")<<tr("left")<<tr("right") );
}

QGraphicsItem *XYCurve::graphicsItem() const{
	return d_ptr;
}


STD_SWAP_METHOD_SETTER_CMD_IMPL(XYCurve, SetVisible, bool, swapVisible)
void XYCurve::setVisible(bool on){
	Q_D(XYCurve);
	exec(new XYCurveSetVisibleCmd(d, on, on ? tr("%1: set visible") : tr("%1: set invisible")));
}

bool XYCurve::isVisible() const{
	Q_D(const XYCurve);
	return d->isVisible();
}

//##############################################################################
//##########################  getter methods  ##################################
//##############################################################################
BASIC_SHARED_D_READER_IMPL(XYCurve, const AbstractColumn *, xColumn, xColumn)
BASIC_SHARED_D_READER_IMPL(XYCurve, const AbstractColumn *, yColumn, yColumn)
QString& XYCurve::xColumnName() const { return d_ptr->xColumnName; }
QString& XYCurve::yColumnName() const {	return d_ptr->yColumnName; }
QString& XYCurve::xColumnParentName() const { return d_ptr->xColumnParentName;}
QString& XYCurve::yColumnParentName() const { return d_ptr->yColumnParentName; }

//line
BASIC_SHARED_D_READER_IMPL(XYCurve, XYCurve::LineType, lineType, lineType)
BASIC_SHARED_D_READER_IMPL(XYCurve, int, lineInterpolationPointsCount, lineInterpolationPointsCount)
CLASS_SHARED_D_READER_IMPL(XYCurve, QPen, linePen, linePen)
BASIC_SHARED_D_READER_IMPL(XYCurve, qreal, lineOpacity, lineOpacity)

//droplines
BASIC_SHARED_D_READER_IMPL(XYCurve, XYCurve::DropLineType, dropLineType, dropLineType)
CLASS_SHARED_D_READER_IMPL(XYCurve, QPen, dropLinePen, dropLinePen)
BASIC_SHARED_D_READER_IMPL(XYCurve, qreal, dropLineOpacity, dropLineOpacity)

//symbols
BASIC_SHARED_D_READER_IMPL(XYCurve, qreal, symbolsOpacity, symbolsOpacity)
BASIC_SHARED_D_READER_IMPL(XYCurve, qreal, symbolsRotationAngle, symbolsRotationAngle)
BASIC_SHARED_D_READER_IMPL(XYCurve, qreal, symbolsSize, symbolsSize)
CLASS_SHARED_D_READER_IMPL(XYCurve, QString, symbolsTypeId, symbolsTypeId)
CLASS_SHARED_D_READER_IMPL(XYCurve, QBrush, symbolsBrush, symbolsBrush)
CLASS_SHARED_D_READER_IMPL(XYCurve, QPen, symbolsPen, symbolsPen)

//values
BASIC_SHARED_D_READER_IMPL(XYCurve, XYCurve::ValuesType, valuesType, valuesType)
BASIC_SHARED_D_READER_IMPL(XYCurve, const AbstractColumn *, valuesColumn, valuesColumn)
QString& XYCurve::valuesColumnName() const { return d_ptr->valuesColumnName; }
QString& XYCurve::valuesColumnParentName() const { return d_ptr->valuesColumnParentName;}
BASIC_SHARED_D_READER_IMPL(XYCurve, XYCurve::ValuesPosition, valuesPosition, valuesPosition)
BASIC_SHARED_D_READER_IMPL(XYCurve, qreal, valuesDistance, valuesDistance)
BASIC_SHARED_D_READER_IMPL(XYCurve, qreal, valuesRotationAngle, valuesRotationAngle)
BASIC_SHARED_D_READER_IMPL(XYCurve, qreal, valuesOpacity, valuesOpacity)
CLASS_SHARED_D_READER_IMPL(XYCurve, QString, valuesPrefix, valuesPrefix)
CLASS_SHARED_D_READER_IMPL(XYCurve, QString, valuesSuffix, valuesSuffix)
CLASS_SHARED_D_READER_IMPL(XYCurve, QColor, valuesColor, valuesColor)
CLASS_SHARED_D_READER_IMPL(XYCurve, QFont, valuesFont, valuesFont)

//error bars
BASIC_SHARED_D_READER_IMPL(XYCurve, XYCurve::ErrorType, xErrorType, xErrorType)
BASIC_SHARED_D_READER_IMPL(XYCurve, const AbstractColumn *, xErrorPlusColumn, xErrorPlusColumn)
QString& XYCurve::xErrorPlusColumnName() const { return d_ptr->xErrorPlusColumnName; }
QString& XYCurve::xErrorPlusColumnParentName() const { return d_ptr->xErrorPlusColumnParentName; }
BASIC_SHARED_D_READER_IMPL(XYCurve, const AbstractColumn *, xErrorMinusColumn, xErrorMinusColumn)
QString& XYCurve::xErrorMinusColumnName() const { return d_ptr->xErrorMinusColumnName; }
QString& XYCurve::xErrorMinusColumnParentName() const { return d_ptr->xErrorMinusColumnParentName; }

BASIC_SHARED_D_READER_IMPL(XYCurve, XYCurve::ErrorType, yErrorType, yErrorType)
BASIC_SHARED_D_READER_IMPL(XYCurve, const AbstractColumn *, yErrorPlusColumn, yErrorPlusColumn)
QString& XYCurve::yErrorPlusColumnName() const { return d_ptr->yErrorPlusColumnName; }
QString& XYCurve::yErrorPlusColumnParentName() const { return d_ptr->yErrorPlusColumnParentName; }
BASIC_SHARED_D_READER_IMPL(XYCurve, const AbstractColumn *, yErrorMinusColumn, yErrorMinusColumn)
QString& XYCurve::yErrorMinusColumnName() const { return d_ptr->yErrorMinusColumnName; }
QString& XYCurve::yErrorMinusColumnParentName() const { return d_ptr->yErrorMinusColumnParentName; }

BASIC_SHARED_D_READER_IMPL(XYCurve, XYCurve::ErrorBarsType, errorBarsType, errorBarsType)
BASIC_SHARED_D_READER_IMPL(XYCurve, qreal, errorBarsCapSize, errorBarsCapSize)
CLASS_SHARED_D_READER_IMPL(XYCurve, QPen, errorBarsPen, errorBarsPen)
BASIC_SHARED_D_READER_IMPL(XYCurve, qreal, errorBarsOpacity, errorBarsOpacity)

//##############################################################################
//#################  setter methods and undo commands ##########################
//##############################################################################
STD_SETTER_CMD_IMPL_F_S(XYCurve, SetXColumn, const AbstractColumn*, xColumn, retransform)
void XYCurve::setXColumn(const AbstractColumn* column) {
	Q_D(XYCurve);
	if (column != d->xColumn) {
		exec(new XYCurveSetXColumnCmd(d, column, tr("%1: assign x values")));

		//emit xDataChanged() in order to notify the plot about the changes
		emit xDataChanged();
		if (column) {
			connect(column, SIGNAL(dataChanged(const AbstractColumn*)), this, SIGNAL(xDataChanged()));

			//update the curve itself on changes
			connect(column, SIGNAL(dataChanged(const AbstractColumn*)), this, SLOT(retransform()));
			connect(column->parentAspect(), SIGNAL(aspectAboutToBeRemoved(const AbstractAspect*)),
					this, SLOT(xColumnAboutToBeRemoved(const AbstractAspect*)));
			//TODO: add disconnect in the undo-function
		}
	}
}

STD_SETTER_CMD_IMPL_F_S(XYCurve, SetYColumn, const AbstractColumn*, yColumn, retransform)
void XYCurve::setYColumn(const AbstractColumn* column) {
	Q_D(XYCurve);
	if (column != d->yColumn) {
		exec(new XYCurveSetYColumnCmd(d, column, tr("%1: assign y values")));

		//emit yDataChanged() in order to notify the plot about the changes
		emit yDataChanged();
		if (column) {
			connect(column, SIGNAL(dataChanged(const AbstractColumn*)), this, SIGNAL(yDataChanged()));

			//update the curve itself on changes
			connect(column, SIGNAL(dataChanged(const AbstractColumn*)), this, SLOT(retransform()));
			connect(column->parentAspect(), SIGNAL(aspectAboutToBeRemoved(const AbstractAspect*)), 
					this, SLOT(yColumnAboutToBeRemoved(const AbstractAspect*)));
			//TODO: add disconnect in the undo-function
		}
	}
}

//Line
STD_SETTER_CMD_IMPL_F_S(XYCurve, SetLineType, XYCurve::LineType, lineType, updateLines)
void XYCurve::setLineType(LineType type) {
	Q_D(XYCurve);
	if (type != d->lineType)
		exec(new XYCurveSetLineTypeCmd(d, type, tr("%1: line type changed")));
}

STD_SETTER_CMD_IMPL_F_S(XYCurve, SetLineInterpolationPointsCount, int, lineInterpolationPointsCount, updateLines)
void XYCurve::setLineInterpolationPointsCount(int count) {
	Q_D(XYCurve);
	if (count != d->lineInterpolationPointsCount)
		exec(new XYCurveSetLineInterpolationPointsCountCmd(d, count, tr("%1: set the number of interpolation points")));
}

STD_SETTER_CMD_IMPL_F_S(XYCurve, SetLinePen, QPen, linePen, recalcShapeAndBoundingRect)
void XYCurve::setLinePen(const QPen &pen) {
	Q_D(XYCurve);
	if (pen != d->linePen)
		exec(new XYCurveSetLinePenCmd(d, pen, tr("%1: set line style")));
}

STD_SETTER_CMD_IMPL_F_S(XYCurve, SetLineOpacity, qreal, lineOpacity, update);
void XYCurve::setLineOpacity(qreal opacity) {
	Q_D(XYCurve);
	if (opacity != d->lineOpacity)
		exec(new XYCurveSetLineOpacityCmd(d, opacity, tr("%1: set line opacity")));
}

//Drop lines
STD_SETTER_CMD_IMPL_F_S(XYCurve, SetDropLineType, XYCurve::DropLineType, dropLineType, updateDropLines)
void XYCurve::setDropLineType(DropLineType type) {
	Q_D(XYCurve);
	if (type != d->dropLineType)
		exec(new XYCurveSetDropLineTypeCmd(d, type, tr("%1: drop line type changed")));
}

STD_SETTER_CMD_IMPL_F_S(XYCurve, SetDropLinePen, QPen, dropLinePen, recalcShapeAndBoundingRect)
void XYCurve::setDropLinePen(const QPen &pen) {
	Q_D(XYCurve);
	if (pen != d->dropLinePen)
		exec(new XYCurveSetDropLinePenCmd(d, pen, tr("%1: set drop line style")));
}

STD_SETTER_CMD_IMPL_F_S(XYCurve, SetDropLineOpacity, qreal, dropLineOpacity, update)
void XYCurve::setDropLineOpacity(qreal opacity) {
	Q_D(XYCurve);
	if (opacity != d->dropLineOpacity)
		exec(new XYCurveSetDropLineOpacityCmd(d, opacity, tr("%1: set drop line opacity")));
}

// Symbols-Tab
STD_SWAP_METHOD_SETTER_CMD_IMPL(XYCurve, SetSymbolsTypeId, QString, swapSymbolsTypeId)
void XYCurve::setSymbolsTypeId(const QString &id) {
	Q_D(XYCurve);
	if (id != d->symbolsTypeId)
		exec(new XYCurveSetSymbolsTypeIdCmd(d, id, tr("%1: set symbol type")));
}

STD_SETTER_CMD_IMPL_F_S(XYCurve, SetSymbolsSize, qreal, symbolsSize, updateSymbols)
void XYCurve::setSymbolsSize(qreal size) {
	Q_D(XYCurve);
	if (!qFuzzyCompare(1 + size, 1 + d->symbolsSize))
		exec(new XYCurveSetSymbolsSizeCmd(d, size, tr("%1: set symbol size")));
}

STD_SETTER_CMD_IMPL_F_S(XYCurve, SetSymbolsRotationAngle, qreal, symbolsRotationAngle, updateSymbols)
void XYCurve::setSymbolsRotationAngle(qreal angle) {
	Q_D(XYCurve);
	if (!qFuzzyCompare(1 + angle, 1 + d->symbolsRotationAngle))
		exec(new XYCurveSetSymbolsRotationAngleCmd(d, angle, tr("%1: rotate symbols")));
}

STD_SETTER_CMD_IMPL_F_S(XYCurve, SetSymbolsBrush, QBrush, symbolsBrush, update)
void XYCurve::setSymbolsBrush(const QBrush &brush) {
	Q_D(XYCurve);
	if (brush != d->symbolsBrush)
		exec(new XYCurveSetSymbolsBrushCmd(d, brush, tr("%1: set symbol filling")));
}

STD_SETTER_CMD_IMPL_F_S(XYCurve, SetSymbolsPen, QPen, symbolsPen, updateSymbols)
void XYCurve::setSymbolsPen(const QPen &pen) {
	Q_D(XYCurve);
	if (pen != d->symbolsPen)
		exec(new XYCurveSetSymbolsPenCmd(d, pen, tr("%1: set symbol outline style")));
}

STD_SETTER_CMD_IMPL_F_S(XYCurve, SetSymbolsOpacity, qreal, symbolsOpacity, update)
void XYCurve::setSymbolsOpacity(qreal opacity) {
	Q_D(XYCurve);
	if (opacity != d->symbolsOpacity)
		exec(new XYCurveSetSymbolsOpacityCmd(d, opacity, tr("%1: set symbols opacity")));
}

//Values-Tab
STD_SETTER_CMD_IMPL_F_S(XYCurve, SetValuesType, XYCurve::ValuesType, valuesType, updateValues)
void XYCurve::setValuesType(XYCurve::ValuesType type) {
	Q_D(XYCurve);
	if (type != d->valuesType)
		exec(new XYCurveSetValuesTypeCmd(d, type, tr("%1: set values type")));
}

STD_SETTER_CMD_IMPL_F_S(XYCurve, SetValuesColumn, const AbstractColumn*, valuesColumn, updateValues)
void XYCurve::setValuesColumn(const AbstractColumn* column) {
	Q_D(XYCurve);
	if (column != d->valuesColumn) {
		exec(new XYCurveSetValuesColumnCmd(d, column, tr("%1: set values column")));
		if (column) {
			connect(column, SIGNAL(dataChanged(const AbstractColumn*)), this, SLOT(updateValues()));
			connect(column->parentAspect(), SIGNAL(aspectAboutToBeRemoved(const AbstractAspect*)),
					this, SLOT(valuesColumnAboutToBeRemoved(const AbstractAspect*)));
		}
	}
}

STD_SETTER_CMD_IMPL_F_S(XYCurve, SetValuesPosition, XYCurve::ValuesPosition, valuesPosition, updateValues)
void XYCurve::setValuesPosition(ValuesPosition position) {
	Q_D(XYCurve);
	if (position != d->valuesPosition)
		exec(new XYCurveSetValuesPositionCmd(d, position, tr("%1: set values position")));
}

STD_SETTER_CMD_IMPL_F_S(XYCurve, SetValuesDistance, qreal, valuesDistance, updateValues)
void XYCurve::setValuesDistance(qreal distance) {
	Q_D(XYCurve);
	if (distance != d->valuesDistance)
		exec(new XYCurveSetValuesDistanceCmd(d, distance, tr("%1: set values distance")));
}

STD_SETTER_CMD_IMPL_F_S(XYCurve, SetValuesRotationAngle, qreal, valuesRotationAngle, updateValues)
void XYCurve::setValuesRotationAngle(qreal angle) {
	Q_D(XYCurve);
	if (!qFuzzyCompare(1 + angle, 1 + d->valuesRotationAngle))
		exec(new XYCurveSetValuesRotationAngleCmd(d, angle, tr("%1: rotate values")));
}

STD_SETTER_CMD_IMPL_F_S(XYCurve, SetValuesOpacity, qreal, valuesOpacity, update)
void XYCurve::setValuesOpacity(qreal opacity) {
	Q_D(XYCurve);
	if (opacity != d->valuesOpacity)
		exec(new XYCurveSetValuesOpacityCmd(d, opacity, tr("%1: set values opacity")));
}

//TODO: Format, Precision

STD_SETTER_CMD_IMPL_F_S(XYCurve, SetValuesPrefix, QString, valuesPrefix, updateValues)
void XYCurve::setValuesPrefix(const QString& prefix) {
	Q_D(XYCurve);
	if (prefix!= d->valuesPrefix)
		exec(new XYCurveSetValuesPrefixCmd(d, prefix, tr("%1: set values prefix")));
}

STD_SETTER_CMD_IMPL_F_S(XYCurve, SetValuesSuffix, QString, valuesSuffix, updateValues)
void XYCurve::setValuesSuffix(const QString& suffix) {
	Q_D(XYCurve);
	if (suffix!= d->valuesSuffix)
		exec(new XYCurveSetValuesSuffixCmd(d, suffix, tr("%1: set values suffix")));
}

STD_SETTER_CMD_IMPL_F_S(XYCurve, SetValuesFont, QFont, valuesFont, updateValues)
void XYCurve::setValuesFont(const QFont& font) {
	Q_D(XYCurve);
	if (font!= d->valuesFont)
		exec(new XYCurveSetValuesFontCmd(d, font, tr("%1: set values font")));
}

STD_SETTER_CMD_IMPL_F_S(XYCurve, SetValuesColor, QColor, valuesColor, update)
void XYCurve::setValuesColor(const QColor& color) {
	Q_D(XYCurve);
	if (color != d->valuesColor)
		exec(new XYCurveSetValuesColorCmd(d, color, tr("%1: set values color")));
}

//Error bars
STD_SETTER_CMD_IMPL_F_S(XYCurve, SetXErrorType, XYCurve::ErrorType, xErrorType, updateErrorBars)
void XYCurve::setXErrorType(ErrorType type) {
	Q_D(XYCurve);
	if (type != d->xErrorType)
		exec(new XYCurveSetXErrorTypeCmd(d, type, tr("%1: x-error type changed")));
}

STD_SETTER_CMD_IMPL_F_S(XYCurve, SetXErrorPlusColumn, const AbstractColumn*, xErrorPlusColumn, updateErrorBars)
void XYCurve::setXErrorPlusColumn(const AbstractColumn* column) {
	Q_D(XYCurve);
	if (column != d->xErrorPlusColumn) {
		exec(new XYCurveSetXErrorPlusColumnCmd(d, column, tr("%1: set x-error column")));
		if (column) {
			connect(column, SIGNAL(dataChanged(const AbstractColumn*)), this, SLOT(updateErrorBars()));
			connect(column->parentAspect(), SIGNAL(aspectAboutToBeRemoved(const AbstractAspect*)),
					this, SLOT(xErrorPlusColumnAboutToBeRemoved(const AbstractAspect*)));
		}
	}		
}

STD_SETTER_CMD_IMPL_F_S(XYCurve, SetXErrorMinusColumn, const AbstractColumn*, xErrorMinusColumn, updateErrorBars)
void XYCurve::setXErrorMinusColumn(const AbstractColumn* column) {
	Q_D(XYCurve);
	if (column != d->xErrorMinusColumn) {
		exec(new XYCurveSetXErrorMinusColumnCmd(d, column, tr("%1: set x-error column")));
		if (column) {
			connect(column, SIGNAL(dataChanged(const AbstractColumn*)), this, SLOT(updateErrorBars()));
			connect(column->parentAspect(), SIGNAL(aspectAboutToBeRemoved(const AbstractAspect*)),
					this, SLOT(xErrorMinusColumnAboutToBeRemoved(const AbstractAspect*)));
		}
	}
}

STD_SETTER_CMD_IMPL_F_S(XYCurve, SetYErrorType, XYCurve::ErrorType, yErrorType, updateErrorBars)
void XYCurve::setYErrorType(ErrorType type) {
	Q_D(XYCurve);
	if (type != d->yErrorType)
		exec(new XYCurveSetYErrorTypeCmd(d, type, tr("%1: y-error type changed")));
}

STD_SETTER_CMD_IMPL_F_S(XYCurve, SetYErrorPlusColumn, const AbstractColumn*, yErrorPlusColumn, updateErrorBars)
void XYCurve::setYErrorPlusColumn(const AbstractColumn* column) {
	Q_D(XYCurve);
	if (column != d->xErrorPlusColumn) {
		exec(new XYCurveSetYErrorPlusColumnCmd(d, column, tr("%1: set y-error column")));
		if (column) {
			connect(column, SIGNAL(dataChanged(const AbstractColumn*)), this, SLOT(updateErrorBars()));
			connect(column->parentAspect(), SIGNAL(aspectAboutToBeRemoved(const AbstractAspect*)),
					this, SLOT(yErrorPlusColumnAboutToBeRemoved(const AbstractAspect*)));
		}
	}
}

STD_SETTER_CMD_IMPL_F_S(XYCurve, SetYErrorMinusColumn, const AbstractColumn*, yErrorMinusColumn, updateErrorBars)
void XYCurve::setYErrorMinusColumn(const AbstractColumn* column) {
	Q_D(XYCurve);
	if (column != d->xErrorMinusColumn) {
		exec(new XYCurveSetYErrorMinusColumnCmd(d, column, tr("%1: set y-error column")));
		if (column) {
			connect(column, SIGNAL(dataChanged(const AbstractColumn*)), this, SLOT(updateErrorBars()));
			connect(column->parentAspect(), SIGNAL(aspectAboutToBeRemoved(const AbstractAspect*)),
					this, SLOT(yErrorMinusColumnAboutToBeRemoved(const AbstractAspect*)));
		}
	}
}

STD_SETTER_CMD_IMPL_F_S(XYCurve, SetErrorBarsCapSize, qreal, errorBarsCapSize, updateErrorBars)
void XYCurve::setErrorBarsCapSize(qreal size) {
	Q_D(XYCurve);
	if (size != d->errorBarsCapSize)
		exec(new XYCurveSetErrorBarsCapSizeCmd(d, size, tr("%1: set error bar cap size")));
}

STD_SETTER_CMD_IMPL_F_S(XYCurve, SetErrorBarsType, XYCurve::ErrorBarsType, errorBarsType, updateErrorBars)
void XYCurve::setErrorBarsType(ErrorBarsType type) {
	Q_D(XYCurve);
	if (type != d->errorBarsType)
		exec(new XYCurveSetErrorBarsTypeCmd(d, type, tr("%1: error bar type changed")));
}

STD_SETTER_CMD_IMPL_F_S(XYCurve, SetErrorBarsPen, QPen, errorBarsPen, recalcShapeAndBoundingRect)
void XYCurve::setErrorBarsPen(const QPen& pen) {
	Q_D(XYCurve);
	if (pen != d->errorBarsPen)
		exec(new XYCurveSetErrorBarsPenCmd(d, pen, tr("%1: set error bar style")));
}

STD_SETTER_CMD_IMPL_F_S(XYCurve, SetErrorBarsOpacity, qreal, errorBarsOpacity, update)
void XYCurve::setErrorBarsOpacity(qreal opacity) {
	Q_D(XYCurve);
	if (opacity != d->errorBarsOpacity)
		exec(new XYCurveSetErrorBarsOpacityCmd(d, opacity, tr("%1: set error bar opacity")));
}

//##############################################################################
//#################################  SLOTS  ####################################
//##############################################################################
void XYCurve::retransform() {
	d_ptr->retransform();
}

void XYCurve::updateValues() {
	d_ptr->updateValues();
}

void XYCurve::updateErrorBars() {
	d_ptr->updateErrorBars();
}

//TODO
void XYCurve::handlePageResize(double horizontalRatio, double verticalRatio){
	Q_D(const XYCurve);
	
	setSymbolsSize(d->symbolsSize * horizontalRatio);
	
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

void XYCurve::xColumnAboutToBeRemoved(const AbstractAspect* aspect) {
	Q_D(XYCurve);
	if (aspect == d->xColumn) {
		d->xColumn = 0;
		d->retransform();
	}
}

void XYCurve::yColumnAboutToBeRemoved(const AbstractAspect* aspect) {
	Q_D(XYCurve);
	if (aspect == d->yColumn) {
		d->yColumn = 0;
		d->retransform();
	}
}

void XYCurve::valuesColumnAboutToBeRemoved(const AbstractAspect* aspect) {
	Q_D(XYCurve);
	if (aspect == d->valuesColumn) {
		d->valuesColumn = 0;
		d->updateValues();
	}
}

void XYCurve::xErrorPlusColumnAboutToBeRemoved(const AbstractAspect* aspect) {
	Q_D(XYCurve);
	if (aspect == d->xErrorPlusColumn) {
		d->xErrorPlusColumn = 0;
		d->updateErrorBars();
	}
}

void XYCurve::xErrorMinusColumnAboutToBeRemoved(const AbstractAspect* aspect) {
	Q_D(XYCurve);
	if (aspect == d->xErrorMinusColumn) {
		d->xErrorMinusColumn = 0;
		d->updateErrorBars();
	}
}

void XYCurve::yErrorPlusColumnAboutToBeRemoved(const AbstractAspect* aspect) {
	Q_D(XYCurve);
	if (aspect == d->yErrorPlusColumn) {
		d->yErrorPlusColumn = 0;
		d->updateErrorBars();
	}
}

void XYCurve::yErrorMinusColumnAboutToBeRemoved(const AbstractAspect* aspect) {
	Q_D(XYCurve);
	if (aspect == d->yErrorMinusColumn) {
		d->yErrorMinusColumn = 0;
		d->updateErrorBars();
	}
}

//##############################################################################
//######  SLOTs for changes triggered via QActions in the context menu  ########
//##############################################################################
void XYCurve::visibilityChanged(){
	Q_D(const XYCurve);
	this->setVisible(!d->isVisible());
}

//##############################################################################
//######################### Private implementation #############################
//##############################################################################
XYCurvePrivate::XYCurvePrivate(XYCurve *owner) : symbolsFactory(0), q(owner) {
	foreach(QObject *plugin, PluginManager::plugins()) {
		CurveSymbolFactory* factory = qobject_cast<CurveSymbolFactory*>(plugin);
		if (factory)
			symbolsFactory = factory;
	}
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

void XYCurvePrivate::contextMenuEvent(QGraphicsSceneContextMenuEvent* event){
    q->createContextMenu()->exec(event->screenPos());
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
	symbolPointsScene.clear();

	if ( (NULL == xColumn) || (NULL == yColumn) ){
		linePath = QPainterPath();
		dropLinePath = QPainterPath();
		symbolsPath = QPainterPath();
		valuesPath = QPainterPath();
		errorBarsPath = QPainterPath();
		recalcShapeAndBoundingRect();
		return;
	}
	
	int startRow = 0;
	int endRow = xColumn->rowCount() - 1;
	QPointF tempPoint;

	AbstractColumn::ColumnMode xColMode = xColumn->columnMode();
	AbstractColumn::ColumnMode yColMode = yColumn->columnMode();

	//take over only valid and non masked points.
	for (int row = startRow; row <= endRow; row++ ){
		if ( xColumn->isValid(row) && yColumn->isValid(row) 
			&& (!xColumn->isMasked(row)) && (!yColumn->isMasked(row)) ) {

			switch(xColMode) {
				case AbstractColumn::Numeric:
					tempPoint.setX(xColumn->valueAt(row));
					break;
				case AbstractColumn::Text:
					//TODO
				case AbstractColumn::DateTime:
				case AbstractColumn::Month:
				case AbstractColumn::Day:
					//TODO
					break;
				default:
					break;
			}

			switch(yColMode) {
				case AbstractColumn::Numeric:
					tempPoint.setY(yColumn->valueAt(row));
					break;
				case AbstractColumn::Text:
					//TODO
				case AbstractColumn::DateTime:
				case AbstractColumn::Month:
				case AbstractColumn::Day:
					//TODO
					break;
				default:
					break;
			}
			symbolPointsLogical.append(tempPoint);
		}
	}

	//calculate the scene coordinates
	const AbstractPlot* plot = dynamic_cast<const AbstractPlot*>(q->parentAspect());
	Q_ASSERT(plot);
	const CartesianCoordinateSystem *cSystem = dynamic_cast<const CartesianCoordinateSystem*>(plot->coordinateSystem());
	Q_ASSERT(cSystem);
	visiblePoints = std::vector<bool>(symbolPointsLogical.count(), false);
	cSystem->mapLogicalToScene(symbolPointsLogical, symbolPointsScene, visiblePoints);

	updateLines();
	updateDropLines();
	updateSymbols();
	updateValues();
	updateErrorBars();
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
	
	//nothing to do, if no data points available
	if (count<=1){
	  	recalcShapeAndBoundingRect();
		return;
	}
	
	//calculate the lines connecting the data points
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
#ifdef HAVE_GSL
		//TODO: optimize! try to ommit the copying from the column to the arrays of doubles.
		gsl_interp_accel *acc  = gsl_interp_accel_alloc();
		gsl_spline *spline=0;
		  
		double x[count],  y[count];
		for (int i=0; i<count; i++){
		  x[i]=symbolPointsLogical.at(i).x();
		  y[i]=symbolPointsLogical.at(i).y();
		}
		
		if (lineType==XYCurve::SplineCubicNatural){
			spline = gsl_spline_alloc(gsl_interp_cspline, count);
		}else if (lineType==XYCurve::SplineCubicPeriodic){
			spline = gsl_spline_alloc(gsl_interp_cspline_periodic, count);
		}else if (lineType==XYCurve::SplineAkimaNatural){
			spline = gsl_spline_alloc(gsl_interp_akima, count);
		}else if (lineType==XYCurve::SplineAkimaPeriodic){
			spline = gsl_spline_alloc(gsl_interp_akima_periodic, count);
		}

		if (!spline) {
			//TODO:akima splines don't work at the moment
			qDebug()<<"Couldn't initalize spline function";
			recalcShapeAndBoundingRect();
			return;
		}

		gsl_set_error_handler_off();
		int status = gsl_spline_init (spline, x, y, count);
		if (status ) {
			//TODO: check in gsl/interp.c when GSL_EINVAL is thrown
			QString gslError;
			if (status == GSL_EINVAL)
				gslError = "x values must be monotonically increasing.";
			else
				gslError = gsl_strerror (status);

			//TODO: forward the error message to the UI.
			qDebug() << "Error in spline calculation. " << gslError;

			recalcShapeAndBoundingRect();
			return;
		}
		
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
#endif
	  }
	  default:
		break;
	}

	//map the lines to scene coordinates
	const CartesianPlot* plot = dynamic_cast<const CartesianPlot*>(q->parentAspect());
	Q_ASSERT(plot);
	const AbstractCoordinateSystem* cSystem = plot->coordinateSystem();
	Q_ASSERT(cSystem);
	lines = cSystem->mapLogicalToScene(lines);
	
	//new line path
	foreach (QLineF line, lines){
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
	
	//calculate drop lines
	QList<QLineF> lines;
	float xMin = 0;
	float yMin = 0;
	const CartesianPlot* plot = dynamic_cast<const CartesianPlot*>(q->parentAspect());
	Q_ASSERT(plot);
	
	xMin = plot->xMin();
	yMin = plot->yMin();
	switch(dropLineType){
	  case XYCurve::DropLineX:{
		for(int i=0; i<symbolPointsLogical.size(); ++i){
			if (!visiblePoints[i]) continue;
			const QPointF& point = symbolPointsLogical.at(i);
			lines.append(QLineF(point, QPointF(point.x(), yMin)));
		}
		break;
	  }
	  case XYCurve::DropLineY:{
		for(int i=0; i<symbolPointsLogical.size(); ++i){
			if (!visiblePoints[i]) continue;
			const QPointF& point = symbolPointsLogical.at(i);
			lines.append(QLineF(point, QPointF(xMin, point.y())));
		}
		break;
	  }
	  case XYCurve::DropLineXY:{
		for(int i=0; i<symbolPointsLogical.size(); ++i){
			if (!visiblePoints[i]) continue;
			const QPointF& point = symbolPointsLogical.at(i);
			lines.append(QLineF(point, QPointF(point.x(), yMin)));
			lines.append(QLineF(point, QPointF(xMin, point.y())));
		}
		break;
	  }
	  default:
		break;
	}
	
	//map the drop lines to scene coordinates
	const AbstractCoordinateSystem* cSystem = plot->coordinateSystem();
	Q_ASSERT(cSystem);
	lines = cSystem->mapLogicalToScene(lines);

	//new painter path for the drop lines
	foreach (QLineF line, lines){
		dropLinePath.moveTo(line.p1());
		dropLinePath.lineTo(line.p2());
	}

	recalcShapeAndBoundingRect();
}

void XYCurvePrivate::updateSymbols(){
	symbolsPath = QPainterPath();

	if (symbolsPrototype->id() != "none"){
		QPainterPath path = symbolsPrototype->shape();
		QTransform trafo;
		if (symbolsRotationAngle != 0) {
			trafo.rotate(symbolsRotationAngle);
			path = trafo.map(path);
		}

		foreach (QPointF point, symbolPointsScene) {
			trafo.reset();
			trafo.translate(point.x(), point.y());
			symbolsPath.addPath(trafo.map(path));
		}
	}

	recalcShapeAndBoundingRect();
}

/*!
  recreates the value strings to be shown and recalculates their draw position.
*/
void XYCurvePrivate::updateValues(){
  	valuesPath = QPainterPath();
	valuesPoints.clear();
	valuesStrings.clear();
	
	if (valuesType== XYCurve::NoValues){
		recalcShapeAndBoundingRect();
		return;
	}

	//determine the value string for all points that are currently visible in the plot
	switch (valuesType){
	  case XYCurve::NoValues:
	  case XYCurve::ValuesX:{
		for(int i=0; i<symbolPointsLogical.size(); ++i){
			if (!visiblePoints[i]) continue;
 			valuesStrings << valuesPrefix + QString::number(symbolPointsLogical.at(i).x()) + valuesSuffix;
		}
	  break;
	  }
	  case XYCurve::ValuesY:{
		for(int i=0; i<symbolPointsLogical.size(); ++i){
			if (!visiblePoints[i]) continue;
 			valuesStrings << valuesPrefix + QString::number(symbolPointsLogical.at(i).y()) + valuesSuffix;
		}
		break;
	  }
	  case XYCurve::ValuesXY:{
		for(int i=0; i<symbolPointsLogical.size(); ++i){
			if (!visiblePoints[i]) continue;
			valuesStrings << valuesPrefix + QString::number(symbolPointsLogical.at(i).x()) + ","
							+ QString::number(symbolPointsLogical.at(i).y()) + valuesSuffix;
		}
		break;
	  }
	  case XYCurve::ValuesXYBracketed:{
		for(int i=0; i<symbolPointsLogical.size(); ++i){
			if (!visiblePoints[i]) continue;
			valuesStrings <<  valuesPrefix + "(" + QString::number(symbolPointsLogical.at(i).x()) + ","
							+ QString::number(symbolPointsLogical.at(i).y()) +")" + valuesSuffix;
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

		AbstractColumn::ColumnMode xColMode = valuesColumn->columnMode();
		for (int i=0; i<endRow; ++i){
			if (!visiblePoints[i]) continue;

			if ( !valuesColumn->isValid(i) || valuesColumn->isMasked(i) )
				continue;

			switch (xColMode){
				case AbstractColumn::Numeric:
					valuesStrings << valuesPrefix + QString::number(valuesColumn->valueAt(i)) + valuesSuffix;
					break;
				case AbstractColumn::Text:
					valuesStrings << valuesPrefix + valuesColumn->textAt(i) + valuesSuffix;
				case AbstractColumn::DateTime:
				case AbstractColumn::Month:
				case AbstractColumn::Day:
					//TODO
					break;
				default:
					break;
			}
		}
	  }
	}

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
		  tempPoint.setX( symbolPointsScene.at(i).x() - w/2);
		  tempPoint.setY( symbolPointsScene.at(i).y() - valuesDistance );
		  valuesPoints.append(tempPoint);
		  }
		  break;
		}
	  case XYCurve::ValuesUnder:{
		for (int i=0; i<valuesStrings.size(); i++){
		  w=fm.width(valuesStrings.at(i));
		  tempPoint.setX( symbolPointsScene.at(i).x() -w/2 );
		  tempPoint.setY( symbolPointsScene.at(i).y() + valuesDistance + h/2);
		  valuesPoints.append(tempPoint);
		}
		break;
	  }
	  case XYCurve::ValuesLeft:{
		for (int i=0; i<valuesStrings.size(); i++){
		  w=fm.width(valuesStrings.at(i));
		  tempPoint.setX( symbolPointsScene.at(i).x() - valuesDistance - w - 1 );
		  tempPoint.setY( symbolPointsScene.at(i).y());
		  valuesPoints.append(tempPoint);
		}
		break;
	  }
	  case XYCurve::ValuesRight:{
		for (int i=0; i<valuesStrings.size(); i++){
		  w=fm.width(valuesStrings.at(i));
		  tempPoint.setX( symbolPointsScene.at(i).x() + valuesDistance - 1 );
		  tempPoint.setY( symbolPointsScene.at(i).y() );
		  valuesPoints.append(tempPoint);
		}
		break;
	  }
	  default:
	  break;
	}

	QTransform trafo;
	QPainterPath path;
	for (int i=0; i<valuesPoints.size(); i++){
		path = QPainterPath();
		path.addText( QPoint(0,0), valuesFont, valuesStrings.at(i) );

		trafo.reset();
		trafo.translate( valuesPoints.at(i).x(), valuesPoints.at(i).y() );
		if (valuesRotationAngle!=0)
			trafo.rotate( -valuesRotationAngle );

		valuesPath.addPath(trafo.map(path));
	}

	recalcShapeAndBoundingRect();
}

void XYCurvePrivate::updateErrorBars(){
  	errorBarsPath = QPainterPath();
	if (xErrorType==XYCurve::NoError && yErrorType==XYCurve::NoError){
		recalcShapeAndBoundingRect();
		return;
	}

	QList<QLineF> lines;
	float errorPlus, errorMinus;
	
	const CartesianPlot* plot = dynamic_cast<const CartesianPlot*>(q->parentAspect());
	Q_ASSERT(plot);	
	const AbstractCoordinateSystem* cSystem = plot->coordinateSystem();
	Q_ASSERT(cSystem);

	//the cap size for the errorbars is given in scene units.
	//determine first the (half of the) cap size in logical units:
	// * take the first visible point in logical units
	// * convert it to scene units
	// * add to this point an offset corresponding to the cap size in scene units
	// * convert this point back to logical units
	// * substract from this point the original coordinates (without the new offset)
	//   to determine the cap size in logical units.
	float capSizeX = 0;
	float capSizeY = 0;
	if (errorBarsType != XYCurve::ErrorBarsSimple && !symbolPointsLogical.isEmpty()) {
		//determine the index of the first visible point
		size_t i = 0;
		while( !visiblePoints[i] && i<visiblePoints.size())
			i++;

		if (i==visiblePoints.size())
			return; //no visible points -> no error bars to draw

		//cap size for x-error bars
		QPointF pointScene = cSystem->mapLogicalToScene(symbolPointsLogical.at(i));
		pointScene.setY(pointScene.y()-errorBarsCapSize);
		QPointF pointLogical = cSystem->mapSceneToLogical(pointScene, AbstractCoordinateSystem::SuppressPageClipping);
		capSizeX = (pointLogical.y() - symbolPointsLogical.at(i).y())/2;
		
		//cap size for y-error bars
		pointScene = cSystem->mapLogicalToScene(symbolPointsLogical.at(i));
		pointScene.setX(pointScene.x()+errorBarsCapSize);
		pointLogical = cSystem->mapSceneToLogical(pointScene, AbstractCoordinateSystem::SuppressPageClipping);
		capSizeY = (pointLogical.x() - symbolPointsLogical.at(i).x())/2;		
	}

	for (int i=0; i<symbolPointsLogical.size(); ++i){
		if (!visiblePoints[i])
			continue;

		const QPointF& point = symbolPointsLogical.at(i);

		//error bars for x
		if (xErrorType!=XYCurve::NoError) {
			//determine the values for the errors
			if (xErrorPlusColumn && xErrorPlusColumn->isValid(i) && !xErrorPlusColumn->isMasked(i))
				errorPlus = xErrorPlusColumn->valueAt(i);
			else
				errorPlus = 0;

			if (xErrorType==XYCurve::SymmetricError) {
				errorMinus = errorPlus;
			} else {
				if (xErrorMinusColumn && xErrorMinusColumn->isValid(i) && !xErrorMinusColumn->isMasked(i))
					errorMinus = xErrorMinusColumn->valueAt(i);
				else
					errorMinus = 0;
			}

			//draw the error bars
			switch(errorBarsType) {
				case XYCurve::ErrorBarsSimple:
					lines.append(QLineF(QPointF(point.x()-errorMinus, point.y()),
										QPointF(point.x()+errorPlus, point.y())));
					break;
				case XYCurve::ErrorBarsWithEnds: {
					lines.append(QLineF(QPointF(point.x()-errorMinus, point.y()),
										QPointF(point.x()+errorPlus, point.y())));
					if (errorMinus!=0) {
						lines.append(QLineF(QPointF(point.x()-errorMinus, point.y()-capSizeX),
											QPointF(point.x()-errorMinus, point.y()+capSizeX)));
					}
					if (errorPlus!=0) {
						lines.append(QLineF(QPointF(point.x()+errorPlus, point.y()-capSizeX),
											QPointF(point.x()+errorPlus, point.y()+capSizeX)));
					}
					break;
				}
			}
		}

		//error bars for y
		if (yErrorType!=XYCurve::NoError) {
			//determine the values for the errors
			if (yErrorPlusColumn && yErrorPlusColumn->isValid(i) && !yErrorPlusColumn->isMasked(i))
				errorPlus = yErrorPlusColumn->valueAt(i);
			else
				errorPlus = 0;

			if (yErrorType==XYCurve::SymmetricError) {
				errorMinus = errorPlus;
			} else {
				if (yErrorMinusColumn && yErrorMinusColumn->isValid(i) && !yErrorMinusColumn->isMasked(i) )
					errorMinus = yErrorMinusColumn->valueAt(i);
				else
					errorMinus = 0;
			}

			//draw the error bars
			switch(errorBarsType) {
				case XYCurve::ErrorBarsSimple:
					lines.append(QLineF(QPointF(point.x(), point.y()-errorMinus),
										QPointF(point.x(), point.y()+errorPlus)));
					break;
				case XYCurve::ErrorBarsWithEnds: {
					lines.append(QLineF(QPointF(point.x(), point.y()-errorMinus),
										QPointF(point.x(), point.y()+errorPlus)));				
					if (errorMinus!=0) {
						lines.append(QLineF(QPointF(point.x()-capSizeY, point.y()-errorMinus),
											QPointF(point.x()+capSizeY, point.y()-errorMinus)));
					}
					if (errorPlus!=0) {
						lines.append(QLineF(QPointF(point.x()-capSizeY, point.y()+errorPlus),
											QPointF(point.x()+capSizeY, point.y()+errorPlus)));
					}
					break;
				}
			}			
		}
	}

	//map the error bars to scene coordinates
	lines = cSystem->mapLogicalToScene(lines);
	
	//new painter path for the drop lines
	foreach (QLineF line, lines){
		errorBarsPath.moveTo(line.p1());
		errorBarsPath.lineTo(line.p2());
	}
	
	recalcShapeAndBoundingRect();
}

/*!
  recalculates the outer bounds and the shape of the curve.
*/
void XYCurvePrivate::recalcShapeAndBoundingRect() {
	prepareGeometryChange();
	boundingRectangle = QRectF();
	curveShape = QPainterPath();
	if (lineType != XYCurve::NoLine){
		curveShape.addPath(AbstractWorksheetElement::shapeFromPath(linePath, linePen));
		boundingRectangle = boundingRectangle.united(linePath.boundingRect());
	}
	
	if (dropLineType != XYCurve::NoDropLine){
		curveShape.addPath(AbstractWorksheetElement::shapeFromPath(dropLinePath, dropLinePen));
		boundingRectangle = boundingRectangle.united(dropLinePath.boundingRect());
	}

	if (symbolsPrototype->id() != "none"){
		curveShape.addPath(AbstractWorksheetElement::shapeFromPath(symbolsPath, symbolsPen));
		boundingRectangle = boundingRectangle.united(symbolsPath.boundingRect());
	}

	if (valuesType != XYCurve::NoValues){
		curveShape.addPath(AbstractWorksheetElement::shapeFromPath(valuesPath, QPen()));
		boundingRectangle = boundingRectangle.united(valuesPath.boundingRect());
	}

	if (xErrorType!=XYCurve::NoError || yErrorType!=XYCurve::NoError){
		curveShape.addPath(AbstractWorksheetElement::shapeFromPath(errorBarsPath, errorBarsPen));
		boundingRectangle = boundingRectangle.united(errorBarsPath.boundingRect());
	}

	//TODO: when the selection is painted, line intersections are visible.
	//simplified() removes those artifacts but is horrible slow for curves with large number of points.
	//search for an alternative.
	//curveShape = curveShape.simplified();
}

QString XYCurvePrivate::swapSymbolsTypeId(const QString &id) {
	QString oldId = symbolsTypeId;
	if (!symbolsFactory)
		return oldId;

	if (id != symbolsTypeId) {
		symbolsTypeId = id;
		delete symbolsPrototype;
		symbolsPrototype = NULL;
		const AbstractCurveSymbol* prototype = symbolsFactory->prototype(symbolsTypeId);
		if (prototype)
			symbolsPrototype = prototype->clone();
		if (!symbolsPrototype)
			return oldId;

		emit q->symbolsTypeIdChanged(id);
	}

	symbolsPrototype->setSize(symbolsSize);
	symbolsPrototype->setAspectRatio(1);
	symbolsPrototype->setBrush(symbolsBrush);
	symbolsPrototype->setPen(symbolsPen);
	symbolsPrototype->setRotationAngle(symbolsRotationAngle);
	updateSymbols();
	return oldId;
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

  //draw error bars
  if ( (xErrorType != XYCurve::NoError) || (yErrorType != XYCurve::NoError) ){
	painter->setOpacity(errorBarsOpacity);
	painter->setPen(errorBarsPen);
	painter->setBrush(Qt::NoBrush);
	painter->drawPath(errorBarsPath);
  }

	//draw symbols
	if (symbolsPrototype->id() != "none"){
		painter->setOpacity(symbolsOpacity);
		painter->setPen(symbolsPen);
		painter->setBrush(symbolsBrush);
		painter->drawPath(symbolsPath);
	}

	//draw values
	if (valuesType != XYCurve::NoValues){
		painter->setOpacity(valuesOpacity);
		painter->setPen(valuesColor);
		painter->setBrush(Qt::NoBrush);
		painter->setBrush(Qt::SolidPattern);
		painter->drawPath(valuesPath);
	}

  painter->setOpacity(opacity);

  if (isSelected()){
	painter->setPen(QPen(Qt::blue, 0, Qt::SolidLine));
	painter->drawPath(shape());
  }
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################
//! Save as XML
void XYCurve::save(QXmlStreamWriter* writer) const{
	Q_D(const XYCurve);

    writer->writeStartElement( "xyCurve" );
    writeBasicAttributes( writer );
    writeCommentElement( writer );

	//general
	writer->writeStartElement( "general" );
	WRITE_COLUMN(d->xColumn, xColumn);
	WRITE_COLUMN(d->yColumn, yColumn);
	writer->writeAttribute( "visible", QString::number(d->isVisible()) );
	writer->writeEndElement();

	//Line
    writer->writeStartElement( "lines" );
	writer->writeAttribute( "type", QString::number(d->lineType) );
	writer->writeAttribute( "interpolationPointsCount", QString::number(d->lineInterpolationPointsCount) );
	WRITE_QPEN(d->linePen);
	writer->writeAttribute( "opacity", QString::number(d->lineOpacity) );
	writer->writeEndElement();
	
	//Drop lines
	writer->writeStartElement( "dropLines" );
	writer->writeAttribute( "type", QString::number(d->dropLineType) );
	WRITE_QPEN(d->dropLinePen);
	writer->writeAttribute( "opacity", QString::number(d->dropLineOpacity) );
	writer->writeEndElement();
	
	//Symbols
	writer->writeStartElement( "symbols" );
	writer->writeAttribute( "opacity", QString::number(d->symbolsOpacity) );
	writer->writeAttribute( "rotation", QString::number(d->symbolsRotationAngle) );
	writer->writeAttribute( "size", QString::number(d->symbolsSize) );
	writer->writeAttribute( "id", d->symbolsTypeId );
	WRITE_QBRUSH(d->symbolsBrush);
	WRITE_QPEN(d->symbolsPen);
	writer->writeEndElement();
	
	//Values
	writer->writeStartElement( "values" );
	writer->writeAttribute( "type", QString::number(d->valuesType) );
	WRITE_COLUMN(d->valuesColumn, valuesColumn);
	writer->writeAttribute( "position", QString::number(d->valuesPosition) );
	writer->writeAttribute( "distance", QString::number(d->valuesDistance) );
	writer->writeAttribute( "rotation", QString::number(d->valuesRotationAngle) );
	writer->writeAttribute( "opacity", QString::number(d->valuesOpacity) );
	//TODO values format and precision
	writer->writeAttribute( "prefix", d->valuesPrefix );
	writer->writeAttribute( "suffix", d->valuesSuffix );
	WRITE_QCOLOR(d->valuesColor);
	WRITE_QFONT(d->valuesFont);
	writer->writeEndElement();

	//Error bars
	writer->writeStartElement( "errorBars" );
	writer->writeAttribute( "xErrorType", QString::number(d->xErrorType) );
	WRITE_COLUMN(d->xErrorPlusColumn, xErrorPlusColumn);
	WRITE_COLUMN(d->xErrorMinusColumn, xErrorMinusColumn);
	writer->writeAttribute( "yErrorType", QString::number(d->yErrorType) );
	WRITE_COLUMN(d->yErrorPlusColumn, yErrorPlusColumn);
	WRITE_COLUMN(d->yErrorMinusColumn, yErrorMinusColumn);
	writer->writeAttribute( "type", QString::number(d->errorBarsType) );
	writer->writeAttribute( "capSize", QString::number(d->errorBarsCapSize) );
	WRITE_QPEN(d->errorBarsPen);
	writer->writeAttribute( "opacity", QString::number(d->errorBarsOpacity) );
	writer->writeEndElement();

	writer->writeEndElement(); //close "xyCurve" section
}

//! Load from XML
bool XYCurve::load(XmlStreamReader* reader){
	Q_D(XYCurve);

    if(!reader->isStartElement() || reader->name() != "xyCurve"){
        reader->raiseError(tr("no xy-curve element found"));
        return false;
    }

    if (!readBasicAttributes(reader))
        return false;

    QString attributeWarning = tr("Attribute '%1' missing or empty, default value is used");
    QXmlStreamAttributes attribs;
    QString str;

    while (!reader->atEnd()){
        reader->readNext();
        if (reader->isEndElement() && reader->name() == "xyCurve")
            break;

        if (!reader->isStartElement())
            continue;

        if (reader->name() == "comment"){
            if (!readCommentElement(reader)) return false;
		}else if (reader->name() == "general"){
			attribs = reader->attributes();

			READ_COLUMN(xColumn);
			READ_COLUMN(yColumn);
			
			str = attribs.value("visible").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'visible'"));
            else
                d->setVisible(str.toInt());
		}else if (reader->name() == "lines"){	
			attribs = reader->attributes();
	
			str = attribs.value("type").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'type'"));
            else
                d->lineType = (XYCurve::LineType)str.toInt();

			str = attribs.value("interpolationPointsCount").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'interpolationPointsCount'"));
            else
                d->lineInterpolationPointsCount = str.toInt();

			READ_QPEN(d->linePen);
			
			str = attribs.value("opacity").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'opacity'"));
            else
                d->lineOpacity = str.toDouble();
		}else if (reader->name() == "dropLines"){	
			attribs = reader->attributes();
	
			str = attribs.value("type").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'type'"));
            else
                d->dropLineType = (XYCurve::DropLineType)str.toInt();

			READ_QPEN(d->dropLinePen);
			
			str = attribs.value("opacity").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'opacity'"));
            else
                d->dropLineOpacity = str.toDouble();	
		}else if (reader->name() == "symbols"){	
			attribs = reader->attributes();

			str = attribs.value("opacity").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'opacity'"));
            else
                d->symbolsOpacity = str.toDouble();
			
			str = attribs.value("rotation").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'rotation'"));
            else
                d->symbolsRotationAngle = str.toDouble();

			str = attribs.value("size").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'size'"));
            else
                d->symbolsSize = str.toDouble();

			str = attribs.value("id").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'id'"));
            else
                d->swapSymbolsTypeId(str);

			READ_QBRUSH(d->symbolsBrush);
			READ_QPEN(d->symbolsPen);
		}else if (reader->name() == "values"){
			attribs = reader->attributes();
			
			str = attribs.value("type").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'type'"));
            else
                d->valuesType = (XYCurve::ValuesType)str.toInt();

			READ_COLUMN(valuesColumn);
			
			str = attribs.value("position").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'position'"));
            else
                d->valuesPosition = (XYCurve::ValuesPosition)str.toInt();

			str = attribs.value("distance").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'distance'"));
            else
                d->valuesDistance = str.toDouble();

			str = attribs.value("rotation").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'rotation'"));
            else
                d->valuesRotationAngle = str.toDouble();

			str = attribs.value("opacity").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'opacity'"));
            else
                d->valuesOpacity = str.toInt();

			//don't produce any warning if no prefix or suffix is set (empty string is allowd here in xml)
			d->valuesPrefix = attribs.value("prefix").toString();
			d->valuesSuffix = attribs.value("suffix").toString();

			READ_QCOLOR(d->valuesColor);
			READ_QFONT(d->valuesFont);
			
			str = attribs.value("opacity").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'opacity'"));
            else
                d->valuesOpacity = str.toDouble();
		}else if (reader->name() == "errorBars"){
			attribs = reader->attributes();

			str = attribs.value("xErrorType").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'xErrorType'"));
            else
                d->xErrorType = (XYCurve::ErrorType)str.toInt();

			READ_COLUMN(xErrorPlusColumn);
			READ_COLUMN(xErrorMinusColumn);

			str = attribs.value("yErrorType").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'yErrorType'"));
            else
                d->yErrorType = (XYCurve::ErrorType)str.toInt();

			READ_COLUMN(yErrorPlusColumn);
			READ_COLUMN(yErrorMinusColumn);

			str = attribs.value("type").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'type'"));
            else
                d->errorBarsType = (XYCurve::ErrorBarsType)str.toInt();

			str = attribs.value("capSize").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'capSize'"));
            else
                d->errorBarsCapSize = str.toDouble();

			READ_QPEN(d->errorBarsPen);
			
			str = attribs.value("opacity").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'opacity'"));
            else
                d->errorBarsOpacity = str.toDouble();
		}
	}

	return true;
}
