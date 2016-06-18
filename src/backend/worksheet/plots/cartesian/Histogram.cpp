/***************************************************************************
    File                 : Histogram.cpp
    Project              : LabPlot
    Description          : Histogram
    --------------------------------------------------------------------
    Copyright            : (C) 2016 Anu Mittal (anu22mittal@gmail.com)
    
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
  \class Histogram
  \brief A 2D-curve, provides an interface for editing many properties of the curve.

  \ingroup worksheet
*/

#include "Histogram.h"
#include "HistogramPrivate.h"
#include "backend/core/column/Column.h"
#include "backend/worksheet/plots/AbstractCoordinateSystem.h"
#include "backend/worksheet/plots/cartesian/CartesianCoordinateSystem.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/lib/commandtemplates.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/lib/XmlStreamReader.h"

#include <QGraphicsDropShadowEffect>
#include <QGraphicsItem>
#include <QGraphicsItemGroup>
#include <QGraphicsPathItem>
#include <QGraphicsEllipseItem>
#include <QPainterPath>
#include <QPainter>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QtDebug>
#include <QElapsedTimer>

#include <KConfigGroup>
#include <KLocale>

#include <gsl/gsl_spline.h>
#include <gsl/gsl_errno.h>
#include <math.h>
#include <vector>

Histogram::Histogram(const QString &name)
		: WorksheetElement(name), d_ptr(new HistogramPrivate(this)){
	init();
}

Histogram::Histogram(const QString &name, HistogramPrivate *dd)
		: WorksheetElement(name), d_ptr(dd){
	init();
}

Histogram::~Histogram() {
	//no need to delete the d-pointer here - it inherits from QGraphicsItem
	//and is deleted during the cleanup in QGraphicsScene
}

void Histogram::init(){
	Q_D(Histogram);

	KConfig config;
	KConfigGroup group = config.group("Histogram");

	d->xColumn = NULL;
	d->yColumn = NULL;

	d->lineType = (Histogram::LineType) group.readEntry("LineType", (int)Histogram::Line);
	d->lineSkipGaps = group.readEntry("SkipLineGaps", false);
	d->lineInterpolationPointsCount = group.readEntry("LineInterpolationPointsCount", 1);
	d->linePen.setStyle( (Qt::PenStyle) group.readEntry("LineStyle", (int)Qt::SolidLine) );
	d->linePen.setColor( group.readEntry("LineColor", QColor(Qt::black)) );
	d->linePen.setWidthF( group.readEntry("LineWidth", Worksheet::convertToSceneUnits(1.0, Worksheet::Point)) );
	d->lineOpacity = group.readEntry("LineOpacity", 1.0);

	d->dropLineType = (Histogram::DropLineType) group.readEntry("DropLineType", (int)Histogram::NoLine);
	d->dropLinePen.setStyle( (Qt::PenStyle) group.readEntry("DropLineStyle", (int)Qt::SolidLine) );
	d->dropLinePen.setColor( group.readEntry("DropLineColor", QColor(Qt::black)));
	d->dropLinePen.setWidthF( group.readEntry("DropLineWidth", Worksheet::convertToSceneUnits(1.0, Worksheet::Point)) );
	d->dropLineOpacity = group.readEntry("DropLineOpacity", 1.0);

	d->symbolsStyle = (Symbol::Style)group.readEntry("SymbolStyle", (int)Symbol::NoSymbols);
	d->symbolsSize = group.readEntry("SymbolSize", Worksheet::convertToSceneUnits(5, Worksheet::Point));
	d->symbolsRotationAngle = group.readEntry("SymbolRotation", 0.0);
	d->symbolsOpacity = group.readEntry("SymbolOpacity", 1.0);
  	d->symbolsBrush.setStyle( (Qt::BrushStyle)group.readEntry("SymbolFillingStyle", (int)Qt::NoBrush) );
  	d->symbolsBrush.setColor( group.readEntry("SymbolFillingColor", QColor(Qt::black)) );
  	d->symbolsPen.setStyle( (Qt::PenStyle)group.readEntry("SymbolBorderStyle", (int)Qt::SolidLine) );
  	d->symbolsPen.setColor( group.readEntry("SymbolBorderColor", QColor(Qt::black)) );
	d->symbolsPen.setWidthF( group.readEntry("SymbolBorderWidth", Worksheet::convertToSceneUnits(0.0, Worksheet::Point)) );

	d->valuesType = (Histogram::ValuesType) group.readEntry("ValuesType", (int)Histogram::NoValues);
	d->valuesColumn = NULL;
	d->valuesPosition = (Histogram::ValuesPosition) group.readEntry("ValuesPosition", (int)Histogram::ValuesAbove);
	d->valuesDistance = group.readEntry("ValuesDistance", Worksheet::convertToSceneUnits(5, Worksheet::Point));
	d->valuesRotationAngle = group.readEntry("ValuesRotation", 0.0);
	d->valuesOpacity = group.readEntry("ValuesOpacity", 1.0);
	d->valuesPrefix = group.readEntry("ValuesPrefix", "");
	d->valuesSuffix = group.readEntry("ValuesSuffix", "");
	d->valuesFont = group.readEntry("ValuesFont", QFont());
	d->valuesFont.setPixelSize( Worksheet::convertToSceneUnits( 8, Worksheet::Point ) );
	d->valuesColor = group.readEntry("ValuesColor", QColor(Qt::black));

	d->fillingPosition = (Histogram::FillingPosition) group.readEntry("FillingPosition", (int)Histogram::NoFilling);
	d->fillingType = (PlotArea::BackgroundType) group.readEntry("FillingType", (int)PlotArea::Color);
	d->fillingColorStyle = (PlotArea::BackgroundColorStyle) group.readEntry("FillingColorStyle", (int) PlotArea::SingleColor);
	d->fillingImageStyle = (PlotArea::BackgroundImageStyle) group.readEntry("FillingImageStyle", (int) PlotArea::Scaled);
	d->fillingBrushStyle = (Qt::BrushStyle) group.readEntry("FillingBrushStyle", (int) Qt::SolidPattern);
	d->fillingFileName = group.readEntry("FillingFileName", QString());
	d->fillingFirstColor = group.readEntry("FillingFirstColor", QColor(Qt::white));
	d->fillingSecondColor = group.readEntry("FillingSecondColor", QColor(Qt::black));
	d->fillingOpacity = group.readEntry("FillingOpacity", 1.0);

	d->xErrorType = (Histogram::ErrorType) group.readEntry("XErrorType", (int)Histogram::NoError);
	d->xErrorPlusColumn = NULL;
	d->xErrorMinusColumn = NULL;
	d->yErrorType = (Histogram::ErrorType) group.readEntry("YErrorType", (int)Histogram::NoError);
	d->yErrorPlusColumn = NULL;
	d->yErrorMinusColumn = NULL;
	d->errorBarsType = (Histogram::ErrorBarsType) group.readEntry("ErrorBarsType", (int)Histogram::ErrorBarsSimple);
	d->errorBarsCapSize = group.readEntry( "ErrorBarsCapSize", Worksheet::convertToSceneUnits(10, Worksheet::Point) );
	d->errorBarsPen.setStyle( (Qt::PenStyle)group.readEntry("ErrorBarsStyle", (int)Qt::SolidLine) );
	d->errorBarsPen.setColor( group.readEntry("ErrorBarsColor", QColor(Qt::black)) );
	d->errorBarsPen.setWidthF( group.readEntry("ErrorBarsWidth", Worksheet::convertToSceneUnits(0.0, Worksheet::Point)) );
	d->errorBarsOpacity = group.readEntry("ErrorBarsOpacity", 1.0);

	this->initActions();
}

void Histogram::initActions(){
	visibilityAction = new QAction(i18n("visible"), this);
	visibilityAction->setCheckable(true);
	connect(visibilityAction, SIGNAL(triggered()), this, SLOT(visibilityChanged()));
}

QMenu* Histogram::createContextMenu(){
	QMenu *menu = WorksheetElement::createContextMenu();
	QAction* firstAction = menu->actions().at(1); //skip the first action because of the "title-action"
	visibilityAction->setChecked(isVisible());
	menu->insertAction(firstAction, visibilityAction);
	return menu;
}

/*!
	Returns an icon to be used in the project explorer.
*/
QIcon Histogram::icon() const{
	return QIcon::fromTheme("labplot-xy-curve");
}

QGraphicsItem* Histogram::graphicsItem() const{
	return d_ptr;
}

STD_SWAP_METHOD_SETTER_CMD_IMPL(Histogram, SetVisible, bool, swapVisible)
void Histogram::setVisible(bool on){
	Q_D(Histogram);
	exec(new HistogramSetVisibleCmd(d, on, on ? i18n("%1: set visible") : i18n("%1: set invisible")));
}

bool Histogram::isVisible() const{
	Q_D(const Histogram);
	return d->isVisible();
}

void Histogram::setPrinting(bool on) {
	Q_D(Histogram);
	d->m_printing = on;
}

//##############################################################################
//##########################  getter methods  ##################################
//##############################################################################
BASIC_SHARED_D_READER_IMPL(Histogram, const AbstractColumn*, xColumn, xColumn)
BASIC_SHARED_D_READER_IMPL(Histogram, const AbstractColumn*, yColumn, yColumn)
QString& Histogram::xColumnPath() const { return d_ptr->xColumnPath; }
QString& Histogram::yColumnPath() const {	return d_ptr->yColumnPath; }

//line
BASIC_SHARED_D_READER_IMPL(Histogram, Histogram::LineType, lineType, lineType)
BASIC_SHARED_D_READER_IMPL(Histogram, bool, lineSkipGaps, lineSkipGaps)
BASIC_SHARED_D_READER_IMPL(Histogram, int, lineInterpolationPointsCount, lineInterpolationPointsCount)
CLASS_SHARED_D_READER_IMPL(Histogram, QPen, linePen, linePen)
BASIC_SHARED_D_READER_IMPL(Histogram, qreal, lineOpacity, lineOpacity)

//droplines
BASIC_SHARED_D_READER_IMPL(Histogram, Histogram::DropLineType, dropLineType, dropLineType)
CLASS_SHARED_D_READER_IMPL(Histogram, QPen, dropLinePen, dropLinePen)
BASIC_SHARED_D_READER_IMPL(Histogram, qreal, dropLineOpacity, dropLineOpacity)

//symbols
BASIC_SHARED_D_READER_IMPL(Histogram, Symbol::Style, symbolsStyle, symbolsStyle)
BASIC_SHARED_D_READER_IMPL(Histogram, qreal, symbolsOpacity, symbolsOpacity)
BASIC_SHARED_D_READER_IMPL(Histogram, qreal, symbolsRotationAngle, symbolsRotationAngle)
BASIC_SHARED_D_READER_IMPL(Histogram, qreal, symbolsSize, symbolsSize)
CLASS_SHARED_D_READER_IMPL(Histogram, QBrush, symbolsBrush, symbolsBrush)
CLASS_SHARED_D_READER_IMPL(Histogram, QPen, symbolsPen, symbolsPen)

//values
BASIC_SHARED_D_READER_IMPL(Histogram, Histogram::ValuesType, valuesType, valuesType)
BASIC_SHARED_D_READER_IMPL(Histogram, const AbstractColumn *, valuesColumn, valuesColumn)
QString& Histogram::valuesColumnPath() const { return d_ptr->valuesColumnPath; }
BASIC_SHARED_D_READER_IMPL(Histogram, Histogram::ValuesPosition, valuesPosition, valuesPosition)
BASIC_SHARED_D_READER_IMPL(Histogram, qreal, valuesDistance, valuesDistance)
BASIC_SHARED_D_READER_IMPL(Histogram, qreal, valuesRotationAngle, valuesRotationAngle)
BASIC_SHARED_D_READER_IMPL(Histogram, qreal, valuesOpacity, valuesOpacity)
CLASS_SHARED_D_READER_IMPL(Histogram, QString, valuesPrefix, valuesPrefix)
CLASS_SHARED_D_READER_IMPL(Histogram, QString, valuesSuffix, valuesSuffix)
CLASS_SHARED_D_READER_IMPL(Histogram, QColor, valuesColor, valuesColor)
CLASS_SHARED_D_READER_IMPL(Histogram, QFont, valuesFont, valuesFont)

//filling
BASIC_SHARED_D_READER_IMPL(Histogram, Histogram::FillingPosition, fillingPosition, fillingPosition)
BASIC_SHARED_D_READER_IMPL(Histogram, PlotArea::BackgroundType, fillingType, fillingType)
BASIC_SHARED_D_READER_IMPL(Histogram, PlotArea::BackgroundColorStyle, fillingColorStyle, fillingColorStyle)
BASIC_SHARED_D_READER_IMPL(Histogram, PlotArea::BackgroundImageStyle, fillingImageStyle, fillingImageStyle)
CLASS_SHARED_D_READER_IMPL(Histogram, Qt::BrushStyle, fillingBrushStyle, fillingBrushStyle)
CLASS_SHARED_D_READER_IMPL(Histogram, QColor, fillingFirstColor, fillingFirstColor)
CLASS_SHARED_D_READER_IMPL(Histogram, QColor, fillingSecondColor, fillingSecondColor)
CLASS_SHARED_D_READER_IMPL(Histogram, QString, fillingFileName, fillingFileName)
BASIC_SHARED_D_READER_IMPL(Histogram, qreal, fillingOpacity, fillingOpacity)

//error bars
BASIC_SHARED_D_READER_IMPL(Histogram, Histogram::ErrorType, xErrorType, xErrorType)
BASIC_SHARED_D_READER_IMPL(Histogram, const AbstractColumn*, xErrorPlusColumn, xErrorPlusColumn)
QString& Histogram::xErrorPlusColumnPath() const { return d_ptr->xErrorPlusColumnPath; }
BASIC_SHARED_D_READER_IMPL(Histogram, const AbstractColumn*, xErrorMinusColumn, xErrorMinusColumn)
QString& Histogram::xErrorMinusColumnPath() const { return d_ptr->xErrorMinusColumnPath; }

BASIC_SHARED_D_READER_IMPL(Histogram, Histogram::ErrorType, yErrorType, yErrorType)
BASIC_SHARED_D_READER_IMPL(Histogram, const AbstractColumn*, yErrorPlusColumn, yErrorPlusColumn)
QString& Histogram::yErrorPlusColumnPath() const { return d_ptr->yErrorPlusColumnPath; }
BASIC_SHARED_D_READER_IMPL(Histogram, const AbstractColumn*, yErrorMinusColumn, yErrorMinusColumn)
QString& Histogram::yErrorMinusColumnPath() const { return d_ptr->yErrorMinusColumnPath; }

BASIC_SHARED_D_READER_IMPL(Histogram, Histogram::ErrorBarsType, errorBarsType, errorBarsType)
BASIC_SHARED_D_READER_IMPL(Histogram, qreal, errorBarsCapSize, errorBarsCapSize)
CLASS_SHARED_D_READER_IMPL(Histogram, QPen, errorBarsPen, errorBarsPen)
BASIC_SHARED_D_READER_IMPL(Histogram, qreal, errorBarsOpacity, errorBarsOpacity)

//##############################################################################
//#################  setter methods and undo commands ##########################
//##############################################################################
STD_SETTER_CMD_IMPL_F_S(Histogram, SetXColumn, const AbstractColumn*, xColumn, retransform)
void Histogram::setXColumn(const AbstractColumn* column) {
	Q_D(Histogram);
	if (column != d->xColumn) {
		exec(new HistogramSetXColumnCmd(d, column, i18n("%1: assign x values")));

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

STD_SETTER_CMD_IMPL_F_S(Histogram, SetYColumn, const AbstractColumn*, yColumn, retransform)
void Histogram::setYColumn(const AbstractColumn* column) {
	Q_D(Histogram);
	if (column != d->yColumn) {
		exec(new HistogramSetYColumnCmd(d, column, i18n("%1: assign y values")));

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
STD_SETTER_CMD_IMPL_F_S(Histogram, SetLineType, Histogram::LineType, lineType, updateLines)
void Histogram::setLineType(LineType type) {
	Q_D(Histogram);
	if (type != d->lineType)
		exec(new HistogramSetLineTypeCmd(d, type, i18n("%1: line type changed")));
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetLineSkipGaps, bool, lineSkipGaps, updateLines)
void Histogram::setLineSkipGaps(bool skip) {
	Q_D(Histogram);
	if (skip != d->lineSkipGaps)
		exec(new HistogramSetLineSkipGapsCmd(d, skip, i18n("%1: set skip line gaps")));
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetLineInterpolationPointsCount, int, lineInterpolationPointsCount, updateLines)
void Histogram::setLineInterpolationPointsCount(int count) {
	Q_D(Histogram);
	if (count != d->lineInterpolationPointsCount)
		exec(new HistogramSetLineInterpolationPointsCountCmd(d, count, i18n("%1: set the number of interpolation points")));
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetLinePen, QPen, linePen, recalcShapeAndBoundingRect)
void Histogram::setLinePen(const QPen &pen) {
	Q_D(Histogram);
	if (pen != d->linePen)
		exec(new HistogramSetLinePenCmd(d, pen, i18n("%1: set line style")));
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetLineOpacity, qreal, lineOpacity, update);
void Histogram::setLineOpacity(qreal opacity) {
	Q_D(Histogram);
	if (opacity != d->lineOpacity)
		exec(new HistogramSetLineOpacityCmd(d, opacity, i18n("%1: set line opacity")));
}

//Drop lines
STD_SETTER_CMD_IMPL_F_S(Histogram, SetDropLineType, Histogram::DropLineType, dropLineType, updateDropLines)
void Histogram::setDropLineType(DropLineType type) {
	Q_D(Histogram);
	if (type != d->dropLineType)
		exec(new HistogramSetDropLineTypeCmd(d, type, i18n("%1: drop line type changed")));
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetDropLinePen, QPen, dropLinePen, recalcShapeAndBoundingRect)
void Histogram::setDropLinePen(const QPen &pen) {
	Q_D(Histogram);
	if (pen != d->dropLinePen)
		exec(new HistogramSetDropLinePenCmd(d, pen, i18n("%1: set drop line style")));
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetDropLineOpacity, qreal, dropLineOpacity, updatePixmap)
void Histogram::setDropLineOpacity(qreal opacity) {
	Q_D(Histogram);
	if (opacity != d->dropLineOpacity)
		exec(new HistogramSetDropLineOpacityCmd(d, opacity, i18n("%1: set drop line opacity")));
}

// Symbols-Tab
STD_SETTER_CMD_IMPL_F_S(Histogram, SetSymbolsStyle, Symbol::Style, symbolsStyle, updateSymbols)
void Histogram::setSymbolsStyle(Symbol::Style style) {
	Q_D(Histogram);
	if (style != d->symbolsStyle)
		exec(new HistogramSetSymbolsStyleCmd(d, style, i18n("%1: set symbol style")));
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetSymbolsSize, qreal, symbolsSize, updateSymbols)
void Histogram::setSymbolsSize(qreal size) {
	Q_D(Histogram);
	if (!qFuzzyCompare(1 + size, 1 + d->symbolsSize))
		exec(new HistogramSetSymbolsSizeCmd(d, size, i18n("%1: set symbol size")));
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetSymbolsRotationAngle, qreal, symbolsRotationAngle, updateSymbols)
void Histogram::setSymbolsRotationAngle(qreal angle) {
	Q_D(Histogram);
	if (!qFuzzyCompare(1 + angle, 1 + d->symbolsRotationAngle))
		exec(new HistogramSetSymbolsRotationAngleCmd(d, angle, i18n("%1: rotate symbols")));
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetSymbolsBrush, QBrush, symbolsBrush, updatePixmap)
void Histogram::setSymbolsBrush(const QBrush &brush) {
	Q_D(Histogram);
	if (brush != d->symbolsBrush)
		exec(new HistogramSetSymbolsBrushCmd(d, brush, i18n("%1: set symbol filling")));
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetSymbolsPen, QPen, symbolsPen, updateSymbols)
void Histogram::setSymbolsPen(const QPen &pen) {
	Q_D(Histogram);
	if (pen != d->symbolsPen)
		exec(new HistogramSetSymbolsPenCmd(d, pen, i18n("%1: set symbol outline style")));
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetSymbolsOpacity, qreal, symbolsOpacity, updatePixmap)
void Histogram::setSymbolsOpacity(qreal opacity) {
	Q_D(Histogram);
	if (opacity != d->symbolsOpacity)
		exec(new HistogramSetSymbolsOpacityCmd(d, opacity, i18n("%1: set symbols opacity")));
}

//Values-Tab
STD_SETTER_CMD_IMPL_F_S(Histogram, SetValuesType, Histogram::ValuesType, valuesType, updateValues)
void Histogram::setValuesType(Histogram::ValuesType type) {
	Q_D(Histogram);
	if (type != d->valuesType)
		exec(new HistogramSetValuesTypeCmd(d, type, i18n("%1: set values type")));
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetValuesColumn, const AbstractColumn*, valuesColumn, updateValues)
void Histogram::setValuesColumn(const AbstractColumn* column) {
	Q_D(Histogram);
	if (column != d->valuesColumn) {
		exec(new HistogramSetValuesColumnCmd(d, column, i18n("%1: set values column")));
		if (column) {
			connect(column, SIGNAL(dataChanged(const AbstractColumn*)), this, SLOT(updateValues()));
			connect(column->parentAspect(), SIGNAL(aspectAboutToBeRemoved(const AbstractAspect*)),
					this, SLOT(valuesColumnAboutToBeRemoved(const AbstractAspect*)));
		}
	}
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetValuesPosition, Histogram::ValuesPosition, valuesPosition, updateValues)
void Histogram::setValuesPosition(ValuesPosition position) {
	Q_D(Histogram);
	if (position != d->valuesPosition)
		exec(new HistogramSetValuesPositionCmd(d, position, i18n("%1: set values position")));
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetValuesDistance, qreal, valuesDistance, updateValues)
void Histogram::setValuesDistance(qreal distance) {
	Q_D(Histogram);
	if (distance != d->valuesDistance)
		exec(new HistogramSetValuesDistanceCmd(d, distance, i18n("%1: set values distance")));
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetValuesRotationAngle, qreal, valuesRotationAngle, updateValues)
void Histogram::setValuesRotationAngle(qreal angle) {
	Q_D(Histogram);
	if (!qFuzzyCompare(1 + angle, 1 + d->valuesRotationAngle))
		exec(new HistogramSetValuesRotationAngleCmd(d, angle, i18n("%1: rotate values")));
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetValuesOpacity, qreal, valuesOpacity, updatePixmap)
void Histogram::setValuesOpacity(qreal opacity) {
	Q_D(Histogram);
	if (opacity != d->valuesOpacity)
		exec(new HistogramSetValuesOpacityCmd(d, opacity, i18n("%1: set values opacity")));
}

//TODO: Format, Precision

STD_SETTER_CMD_IMPL_F_S(Histogram, SetValuesPrefix, QString, valuesPrefix, updateValues)
void Histogram::setValuesPrefix(const QString& prefix) {
	Q_D(Histogram);
	if (prefix!= d->valuesPrefix)
		exec(new HistogramSetValuesPrefixCmd(d, prefix, i18n("%1: set values prefix")));
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetValuesSuffix, QString, valuesSuffix, updateValues)
void Histogram::setValuesSuffix(const QString& suffix) {
	Q_D(Histogram);
	if (suffix!= d->valuesSuffix)
		exec(new HistogramSetValuesSuffixCmd(d, suffix, i18n("%1: set values suffix")));
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetValuesFont, QFont, valuesFont, updateValues)
void Histogram::setValuesFont(const QFont& font) {
	Q_D(Histogram);
	if (font!= d->valuesFont)
		exec(new HistogramSetValuesFontCmd(d, font, i18n("%1: set values font")));
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetValuesColor, QColor, valuesColor, updatePixmap)
void Histogram::setValuesColor(const QColor& color) {
	Q_D(Histogram);
	if (color != d->valuesColor)
		exec(new HistogramSetValuesColorCmd(d, color, i18n("%1: set values color")));
}

//Filling
STD_SETTER_CMD_IMPL_F_S(Histogram, SetFillingPosition, Histogram::FillingPosition, fillingPosition, updateFilling)
void Histogram::setFillingPosition(FillingPosition position) {
	Q_D(Histogram);
	if (position != d->fillingPosition)
		exec(new HistogramSetFillingPositionCmd(d, position, i18n("%1: filling position changed")));
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetFillingType, PlotArea::BackgroundType, fillingType, updatePixmap)
void Histogram::setFillingType(PlotArea::BackgroundType type) {
	Q_D(Histogram);
	if (type != d->fillingType)
		exec(new HistogramSetFillingTypeCmd(d, type, i18n("%1: filling type changed")));
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetFillingColorStyle, PlotArea::BackgroundColorStyle, fillingColorStyle, updatePixmap)
void Histogram::setFillingColorStyle(PlotArea::BackgroundColorStyle style) {
	Q_D(Histogram);
	if (style != d->fillingColorStyle)
		exec(new HistogramSetFillingColorStyleCmd(d, style, i18n("%1: filling color style changed")));
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetFillingImageStyle, PlotArea::BackgroundImageStyle, fillingImageStyle, updatePixmap)
void Histogram::setFillingImageStyle(PlotArea::BackgroundImageStyle style) {
	Q_D(Histogram);
	if (style != d->fillingImageStyle)
		exec(new HistogramSetFillingImageStyleCmd(d, style, i18n("%1: filling image style changed")));
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetFillingBrushStyle, Qt::BrushStyle, fillingBrushStyle, updatePixmap)
void Histogram::setFillingBrushStyle(Qt::BrushStyle style) {
	Q_D(Histogram);
	if (style != d->fillingBrushStyle)
		exec(new HistogramSetFillingBrushStyleCmd(d, style, i18n("%1: filling brush style changed")));
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetFillingFirstColor, QColor, fillingFirstColor, updatePixmap)
void Histogram::setFillingFirstColor(const QColor& color) {
	Q_D(Histogram);
	if (color!= d->fillingFirstColor)
		exec(new HistogramSetFillingFirstColorCmd(d, color, i18n("%1: set filling first color")));
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetFillingSecondColor, QColor, fillingSecondColor, updatePixmap)
void Histogram::setFillingSecondColor(const QColor& color) {
	Q_D(Histogram);
	if (color!= d->fillingSecondColor)
		exec(new HistogramSetFillingSecondColorCmd(d, color, i18n("%1: set filling second color")));
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetFillingFileName, QString, fillingFileName, updatePixmap)
void Histogram::setFillingFileName(const QString& fileName) {
	Q_D(Histogram);
	if (fileName!= d->fillingFileName)
		exec(new HistogramSetFillingFileNameCmd(d, fileName, i18n("%1: set filling image")));
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetFillingOpacity, qreal, fillingOpacity, updatePixmap)
void Histogram::setFillingOpacity(qreal opacity) {
	Q_D(Histogram);
	if (opacity != d->fillingOpacity)
		exec(new HistogramSetFillingOpacityCmd(d, opacity, i18n("%1: set filling opacity")));
}

//Error bars
STD_SETTER_CMD_IMPL_F_S(Histogram, SetXErrorType, Histogram::ErrorType, xErrorType, updateErrorBars)
void Histogram::setXErrorType(ErrorType type) {
	Q_D(Histogram);
	if (type != d->xErrorType)
		exec(new HistogramSetXErrorTypeCmd(d, type, i18n("%1: x-error type changed")));
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetXErrorPlusColumn, const AbstractColumn*, xErrorPlusColumn, updateErrorBars)
void Histogram::setXErrorPlusColumn(const AbstractColumn* column) {
	Q_D(Histogram);
	if (column != d->xErrorPlusColumn) {
		exec(new HistogramSetXErrorPlusColumnCmd(d, column, i18n("%1: set x-error column")));
		if (column) {
			connect(column, SIGNAL(dataChanged(const AbstractColumn*)), this, SLOT(updateErrorBars()));
			connect(column->parentAspect(), SIGNAL(aspectAboutToBeRemoved(const AbstractAspect*)),
					this, SLOT(xErrorPlusColumnAboutToBeRemoved(const AbstractAspect*)));
		}
	}
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetXErrorMinusColumn, const AbstractColumn*, xErrorMinusColumn, updateErrorBars)
void Histogram::setXErrorMinusColumn(const AbstractColumn* column) {
	Q_D(Histogram);
	if (column != d->xErrorMinusColumn) {
		exec(new HistogramSetXErrorMinusColumnCmd(d, column, i18n("%1: set x-error column")));
		if (column) {
			connect(column, SIGNAL(dataChanged(const AbstractColumn*)), this, SLOT(updateErrorBars()));
			connect(column->parentAspect(), SIGNAL(aspectAboutToBeRemoved(const AbstractAspect*)),
					this, SLOT(xErrorMinusColumnAboutToBeRemoved(const AbstractAspect*)));
		}
	}
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetYErrorType, Histogram::ErrorType, yErrorType, updateErrorBars)
void Histogram::setYErrorType(ErrorType type) {
	Q_D(Histogram);
	if (type != d->yErrorType)
		exec(new HistogramSetYErrorTypeCmd(d, type, i18n("%1: y-error type changed")));
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetYErrorPlusColumn, const AbstractColumn*, yErrorPlusColumn, updateErrorBars)
void Histogram::setYErrorPlusColumn(const AbstractColumn* column) {
	Q_D(Histogram);
	if (column != d->yErrorPlusColumn) {
		exec(new HistogramSetYErrorPlusColumnCmd(d, column, i18n("%1: set y-error column")));
		if (column) {
			connect(column, SIGNAL(dataChanged(const AbstractColumn*)), this, SLOT(updateErrorBars()));
			connect(column->parentAspect(), SIGNAL(aspectAboutToBeRemoved(const AbstractAspect*)),
					this, SLOT(yErrorPlusColumnAboutToBeRemoved(const AbstractAspect*)));
		}
	}
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetYErrorMinusColumn, const AbstractColumn*, yErrorMinusColumn, updateErrorBars)
void Histogram::setYErrorMinusColumn(const AbstractColumn* column) {
	Q_D(Histogram);
	if (column != d->yErrorMinusColumn) {
		exec(new HistogramSetYErrorMinusColumnCmd(d, column, i18n("%1: set y-error column")));
		if (column) {
			connect(column, SIGNAL(dataChanged(const AbstractColumn*)), this, SLOT(updateErrorBars()));
			connect(column->parentAspect(), SIGNAL(aspectAboutToBeRemoved(const AbstractAspect*)),
					this, SLOT(yErrorMinusColumnAboutToBeRemoved(const AbstractAspect*)));
		}
	}
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetErrorBarsCapSize, qreal, errorBarsCapSize, updateErrorBars)
void Histogram::setErrorBarsCapSize(qreal size) {
	Q_D(Histogram);
	if (size != d->errorBarsCapSize)
		exec(new HistogramSetErrorBarsCapSizeCmd(d, size, i18n("%1: set error bar cap size")));
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetErrorBarsType, Histogram::ErrorBarsType, errorBarsType, updateErrorBars)
void Histogram::setErrorBarsType(ErrorBarsType type) {
	Q_D(Histogram);
	if (type != d->errorBarsType)
		exec(new HistogramSetErrorBarsTypeCmd(d, type, i18n("%1: error bar type changed")));
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetErrorBarsPen, QPen, errorBarsPen, recalcShapeAndBoundingRect)
void Histogram::setErrorBarsPen(const QPen& pen) {
	Q_D(Histogram);
	if (pen != d->errorBarsPen)
		exec(new HistogramSetErrorBarsPenCmd(d, pen, i18n("%1: set error bar style")));
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetErrorBarsOpacity, qreal, errorBarsOpacity, updatePixmap)
void Histogram::setErrorBarsOpacity(qreal opacity) {
	Q_D(Histogram);
	if (opacity != d->errorBarsOpacity)
		exec(new HistogramSetErrorBarsOpacityCmd(d, opacity, i18n("%1: set error bar opacity")));
}

void Histogram::suppressRetransform(bool b) {
	Q_D(Histogram);
	d->m_suppressRetransform = b;
}

//##############################################################################
//#################################  SLOTS  ####################################
//##############################################################################
void Histogram::retransform() {
	d_ptr->retransform();
}

void Histogram::updateValues() {
	d_ptr->updateValues();
}

void Histogram::updateErrorBars() {
	d_ptr->updateErrorBars();
}

//TODO
void Histogram::handlePageResize(double horizontalRatio, double verticalRatio){
	Q_D(const Histogram);

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

void Histogram::xColumnAboutToBeRemoved(const AbstractAspect* aspect) {
	Q_D(Histogram);
	if (aspect == d->xColumn) {
		d->xColumn = 0;
		d->retransform();
	}
}

void Histogram::yColumnAboutToBeRemoved(const AbstractAspect* aspect) {
	Q_D(Histogram);
	if (aspect == d->yColumn) {
		d->yColumn = 0;
		d->retransform();
	}
}

void Histogram::valuesColumnAboutToBeRemoved(const AbstractAspect* aspect) {
	Q_D(Histogram);
	if (aspect == d->valuesColumn) {
		d->valuesColumn = 0;
		d->updateValues();
	}
}

void Histogram::xErrorPlusColumnAboutToBeRemoved(const AbstractAspect* aspect) {
	Q_D(Histogram);
	if (aspect == d->xErrorPlusColumn) {
		d->xErrorPlusColumn = 0;
		d->updateErrorBars();
	}
}

void Histogram::xErrorMinusColumnAboutToBeRemoved(const AbstractAspect* aspect) {
	Q_D(Histogram);
	if (aspect == d->xErrorMinusColumn) {
		d->xErrorMinusColumn = 0;
		d->updateErrorBars();
	}
}

void Histogram::yErrorPlusColumnAboutToBeRemoved(const AbstractAspect* aspect) {
	Q_D(Histogram);
	if (aspect == d->yErrorPlusColumn) {
		d->yErrorPlusColumn = 0;
		d->updateErrorBars();
	}
}

void Histogram::yErrorMinusColumnAboutToBeRemoved(const AbstractAspect* aspect) {
	Q_D(Histogram);
	if (aspect == d->yErrorMinusColumn) {
		d->yErrorMinusColumn = 0;
		d->updateErrorBars();
	}
}

//##############################################################################
//######  SLOTs for changes triggered via QActions in the context menu  ########
//##############################################################################
void Histogram::visibilityChanged() {
	Q_D(const Histogram);
	this->setVisible(!d->isVisible());
}

//##############################################################################
//######################### Private implementation #############################
//##############################################################################
HistogramPrivate::HistogramPrivate(Histogram *owner) : m_printing(false), m_hovered(false), m_suppressRecalc(false),
	m_suppressRetransform(false), m_hoverEffectImageIsDirty(false), m_selectionEffectImageIsDirty(false), q(owner) {
	setFlag(QGraphicsItem::ItemIsSelectable, true);
	setAcceptHoverEvents(true);
}

QString HistogramPrivate::name() const {
	return q->name();
}

QRectF HistogramPrivate::boundingRect() const {
	return boundingRectangle;
}

/*!
  Returns the shape of the Histogram as a QPainterPath in local coordinates
*/
QPainterPath HistogramPrivate::shape() const {
	return curveShape;
}

void HistogramPrivate::contextMenuEvent(QGraphicsSceneContextMenuEvent* event){
    q->createContextMenu()->exec(event->screenPos());
}

bool HistogramPrivate::swapVisible(bool on){
	bool oldValue = isVisible();
	setVisible(on);
	emit q->visibilityChanged(on);
	return oldValue;
}

/*!
  recalculates the position of the points to be drawn. Called when the data was changed.
  Triggers the update of lines, drop lines, symbols etc.
*/
void HistogramPrivate::retransform(){
	if (m_suppressRetransform){
		return;
	}

// 	qDebug()<<"HistogramPrivate::retransform() " << q->name();
	symbolPointsLogical.clear();
	symbolPointsScene.clear();
	connectedPointsLogical.clear();

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
			}
			symbolPointsLogical.append(tempPoint);
			connectedPointsLogical.push_back(true);
		} else {
			if (connectedPointsLogical.size())
				connectedPointsLogical[connectedPointsLogical.size()-1] = false;
		}
	}

	//calculate the scene coordinates
	const AbstractPlot* plot = dynamic_cast<const AbstractPlot*>(q->parentAspect());
	if (!plot)
		return;

	const CartesianCoordinateSystem *cSystem = dynamic_cast<const CartesianCoordinateSystem*>(plot->coordinateSystem());
	Q_ASSERT(cSystem);
	visiblePoints = std::vector<bool>(symbolPointsLogical.count(), false);
	cSystem->mapLogicalToScene(symbolPointsLogical, symbolPointsScene, visiblePoints);

	m_suppressRecalc = true;
	updateLines();
	updateDropLines();
	updateSymbols();
	updateValues();
	m_suppressRecalc = false;
	updateErrorBars();
}

/*!
  recalculates the painter path for the lines connecting the data points.
  Called each time when the type of this connection is changed.
*/
void HistogramPrivate::updateLines(){
  	linePath = QPainterPath();
	lines.clear();
	if (lineType == Histogram::NoLine){
	  updateFilling();
	  recalcShapeAndBoundingRect();
	  return;
	}

	const int count=symbolPointsLogical.count();

	//nothing to do, if no data points available
	if (count<=1){
	  	recalcShapeAndBoundingRect();
		return;
	}

	//calculate the lines connecting the data points
	QPointF tempPoint1, tempPoint2;
	QPointF curPoint, nextPoint;
	switch(lineType){
          case Histogram::NoLine:
		break;
	  case Histogram::Line:{
		for (int i=0; i<count-1; i++){
		  if (!lineSkipGaps && !connectedPointsLogical[i]) continue;
		  lines.append(QLineF(symbolPointsLogical.at(i), symbolPointsLogical.at(i+1)));
		}
		break;
	  }
	  case Histogram::StartHorizontal:{
		for (int i=0; i<count-1; i++){
		  if (!lineSkipGaps && !connectedPointsLogical[i]) continue;
		  curPoint=symbolPointsLogical.at(i);
		  nextPoint=symbolPointsLogical.at(i+1);
		  tempPoint1=QPointF(nextPoint.x(), curPoint.y());
		  lines.append(QLineF(curPoint, tempPoint1));
		  lines.append(QLineF(tempPoint1, nextPoint));
		}
		break;
	  }
	  case Histogram::StartVertical:{
		for (int i=0; i<count-1; i++){
		  if (!lineSkipGaps && !connectedPointsLogical[i]) continue;
		  curPoint=symbolPointsLogical.at(i);
		  nextPoint=symbolPointsLogical.at(i+1);
		  tempPoint1=QPointF(curPoint.x(), nextPoint.y());
		  lines.append(QLineF(curPoint, tempPoint1));
		  lines.append(QLineF(tempPoint1,nextPoint));
		}
		break;
	  }
	  case Histogram::MidpointHorizontal:{
		for (int i=0; i<count-1; i++){
		  if (!lineSkipGaps && !connectedPointsLogical[i]) continue;
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
	  case Histogram::MidpointVertical:{
		for (int i=0; i<count-1; i++){
		  if (!lineSkipGaps && !connectedPointsLogical[i]) continue;
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
	  case Histogram::Segments2:{
		int skip=0;
		for (int i=0; i<count-1; i++){
		  if (skip!=1){
			if (!lineSkipGaps && !connectedPointsLogical[i]) {skip=0; continue;}
			lines.append(QLineF(symbolPointsLogical.at(i), symbolPointsLogical.at(i+1)));
			skip++;
		  }else{
			skip=0;
		  }
		}
		break;
	  }
	  case Histogram::Segments3:{
		int skip=0;
		for (int i=0; i<count-1; i++){
		  if (skip!=2){
			if (!lineSkipGaps && !connectedPointsLogical[i]) {skip=0; continue;}
			lines.append(QLineF(symbolPointsLogical.at(i), symbolPointsLogical.at(i+1)));
			skip++;
		  }else{
			skip=0;
		  }
		}
		break;
	  }
	  case Histogram::SplineCubicNatural:
	  case Histogram::SplineCubicPeriodic:
	  case Histogram::SplineAkimaNatural:
	  case Histogram::SplineAkimaPeriodic:{
		//TODO: optimize! try to omit the copying from the column to the arrays of doubles.
		//TODO: forward the error message to the UI.
		gsl_interp_accel *acc  = gsl_interp_accel_alloc();
		gsl_spline *spline=0;

		double x[count],  y[count];
		for (int i=0; i<count; i++){
		  x[i]=symbolPointsLogical.at(i).x();
		  y[i]=symbolPointsLogical.at(i).y();
		}

		gsl_set_error_handler_off();
		if (lineType==Histogram::SplineCubicNatural){
			spline = gsl_spline_alloc(gsl_interp_cspline, count);
		}else if (lineType==Histogram::SplineCubicPeriodic){
			spline = gsl_spline_alloc(gsl_interp_cspline_periodic, count);
		}else if (lineType==Histogram::SplineAkimaNatural){
			spline = gsl_spline_alloc(gsl_interp_akima, count);
		}else if (lineType==Histogram::SplineAkimaPeriodic){
			spline = gsl_spline_alloc(gsl_interp_akima_periodic, count);
		}

		if (!spline) {
			QString msg;
			if ( (lineType==Histogram::SplineAkimaNatural || lineType==Histogram::SplineAkimaPeriodic) && count<5)
				msg=i18n("Error: Akima spline interpolation requires a minimum of 5 points.");
			else
				msg =i18n("Couldn't initialize spline function");
			qDebug()<<msg;
			recalcShapeAndBoundingRect();
			return;
		}

		int status = gsl_spline_init (spline, x, y, count);
		if (status ) {
			//TODO: check in gsl/interp.c when GSL_EINVAL is thrown
			QString gslError;
			if (status == GSL_EINVAL)
				gslError = "x values must be monotonically increasing.";
			else
				gslError = gsl_strerror (status);

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

		  for (xi=x1; xi<x2; xi += step){
			yi = gsl_spline_eval (spline, xi, acc);
			xinterp.push_back(xi);
			yinterp.push_back(yi);
		  }
		}

		for (unsigned int i=0; i<xinterp.size()-1; i++){
		  lines.append(QLineF(xinterp[i], yinterp[i], xinterp[i+1], yinterp[i+1]));
		}
		lines.append(QLineF(xinterp[xinterp.size()-1], yinterp[yinterp.size()-1], x[count-1], y[count-1]));

		gsl_spline_free (spline);
        gsl_interp_accel_free (acc);
		break;
	  }
	  default:
		break;
	}

	//map the lines to scene coordinates
	const CartesianPlot* plot = dynamic_cast<const CartesianPlot*>(q->parentAspect());
	const AbstractCoordinateSystem* cSystem = plot->coordinateSystem();
	lines = cSystem->mapLogicalToScene(lines);

	//new line path
	foreach (const QLineF& line, lines){
		linePath.moveTo(line.p1());
		linePath.lineTo(line.p2());
	}

	updateFilling();
	recalcShapeAndBoundingRect();
}

/*!
  recalculates the painter path for the drop lines.
  Called each time when the type of the drop lines is changed.
*/
void HistogramPrivate::updateDropLines(){
  	dropLinePath = QPainterPath();
	if (dropLineType == Histogram::NoDropLine){
	  recalcShapeAndBoundingRect();
	  return;
	}

	//calculate drop lines
	const CartesianPlot* plot = dynamic_cast<const CartesianPlot*>(q->parentAspect());
	QList<QLineF> lines;
	float xMin = 0;
	float yMin = 0;

	xMin = plot->xMin();
	yMin = plot->yMin();
	switch(dropLineType){
	  case Histogram::NoDropLine:
		break;
	  case Histogram::DropLineX:{
		for(int i=0; i<symbolPointsLogical.size(); ++i){
			if (!visiblePoints[i]) continue;
			const QPointF& point = symbolPointsLogical.at(i);
			lines.append(QLineF(point, QPointF(point.x(), yMin)));
		}
		break;
	  }
	  case Histogram::DropLineY:{
		for(int i=0; i<symbolPointsLogical.size(); ++i){
			if (!visiblePoints[i]) continue;
			const QPointF& point = symbolPointsLogical.at(i);
			lines.append(QLineF(point, QPointF(xMin, point.y())));
		}
		break;
	  }
	  case Histogram::DropLineXY:{
		for(int i=0; i<symbolPointsLogical.size(); ++i){
			if (!visiblePoints[i]) continue;
			const QPointF& point = symbolPointsLogical.at(i);
			lines.append(QLineF(point, QPointF(point.x(), yMin)));
			lines.append(QLineF(point, QPointF(xMin, point.y())));
		}
		break;
	  }
	  case Histogram::DropLineXZeroBaseline:{
		for(int i=0; i<symbolPointsLogical.size(); ++i){
			if (!visiblePoints[i]) continue;
			const QPointF& point = symbolPointsLogical.at(i);
			lines.append(QLineF(point, QPointF(point.x(), 0)));
		}
		break;
	  }
	  case Histogram::DropLineXMinBaseline:{
		for(int i=0; i<symbolPointsLogical.size(); ++i){
			if (!visiblePoints[i]) continue;
			const QPointF& point = symbolPointsLogical.at(i);
			lines.append( QLineF(point, QPointF(point.x(), dynamic_cast<const Column*>(yColumn)->minimum())) );
		}
		break;
	  }
	  case Histogram::DropLineXMaxBaseline:{
		for(int i=0; i<symbolPointsLogical.size(); ++i){
			if (!visiblePoints[i]) continue;
			const QPointF& point = symbolPointsLogical.at(i);
			lines.append( QLineF(point, QPointF(point.x(), dynamic_cast<const Column*>(yColumn)->maximum())) );
		}
		break;
	  }
	  default:
		break;
	}

	//map the drop lines to scene coordinates
	const AbstractCoordinateSystem* cSystem = plot->coordinateSystem();
	lines = cSystem->mapLogicalToScene(lines);

	//new painter path for the drop lines
	foreach (const QLineF& line, lines){
		dropLinePath.moveTo(line.p1());
		dropLinePath.lineTo(line.p2());
	}

	recalcShapeAndBoundingRect();
}

void HistogramPrivate::updateSymbols(){
	symbolsPath = QPainterPath();
	if (symbolsStyle != Symbol::NoSymbols){
		QPainterPath path = Symbol::pathFromStyle(symbolsStyle);

		QTransform trafo;
		trafo.scale(symbolsSize, symbolsSize);
		path = trafo.map(path);
		trafo.reset();

		if (symbolsRotationAngle != 0) {
			trafo.rotate(symbolsRotationAngle);
			path = trafo.map(path);
		}

		foreach (const QPointF& point, symbolPointsScene) {
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
void HistogramPrivate::updateValues(){
  	valuesPath = QPainterPath();
	valuesPoints.clear();
	valuesStrings.clear();

	if (valuesType == Histogram::NoValues){
		recalcShapeAndBoundingRect();
		return;
	}

	//determine the value string for all points that are currently visible in the plot
	switch (valuesType){
	  case Histogram::NoValues:
	  case Histogram::ValuesX:{
		for(int i=0; i<symbolPointsLogical.size(); ++i){
			if (!visiblePoints[i]) continue;
 			valuesStrings << valuesPrefix + QString::number(symbolPointsLogical.at(i).x()) + valuesSuffix;
		}
	  break;
	  }
	  case Histogram::ValuesY:{
		for(int i=0; i<symbolPointsLogical.size(); ++i){
			if (!visiblePoints[i]) continue;
 			valuesStrings << valuesPrefix + QString::number(symbolPointsLogical.at(i).y()) + valuesSuffix;
		}
		break;
	  }
	  case Histogram::ValuesXY:{
		for(int i=0; i<symbolPointsLogical.size(); ++i){
			if (!visiblePoints[i]) continue;
			valuesStrings << valuesPrefix + QString::number(symbolPointsLogical.at(i).x()) + ','
							+ QString::number(symbolPointsLogical.at(i).y()) + valuesSuffix;
		}
		break;
	  }
	  case Histogram::ValuesXYBracketed:{
		for(int i=0; i<symbolPointsLogical.size(); ++i){
			if (!visiblePoints[i]) continue;
			valuesStrings <<  valuesPrefix + '(' + QString::number(symbolPointsLogical.at(i).x()) + ','
							+ QString::number(symbolPointsLogical.at(i).y()) +')' + valuesSuffix;
		}
		break;
	  }
	  case Histogram::ValuesCustomColumn:{
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
	  case Histogram::ValuesAbove:{
		for (int i=0; i<valuesStrings.size(); i++){
		  w=fm.width(valuesStrings.at(i));
		  tempPoint.setX( symbolPointsScene.at(i).x() - w/2);
		  tempPoint.setY( symbolPointsScene.at(i).y() - valuesDistance );
		  valuesPoints.append(tempPoint);
		  }
		  break;
		}
	  case Histogram::ValuesUnder:{
		for (int i=0; i<valuesStrings.size(); i++){
		  w=fm.width(valuesStrings.at(i));
		  tempPoint.setX( symbolPointsScene.at(i).x() -w/2 );
		  tempPoint.setY( symbolPointsScene.at(i).y() + valuesDistance + h/2);
		  valuesPoints.append(tempPoint);
		}
		break;
	  }
	  case Histogram::ValuesLeft:{
		for (int i=0; i<valuesStrings.size(); i++){
		  w=fm.width(valuesStrings.at(i));
		  tempPoint.setX( symbolPointsScene.at(i).x() - valuesDistance - w - 1 );
		  tempPoint.setY( symbolPointsScene.at(i).y());
		  valuesPoints.append(tempPoint);
		}
		break;
	  }
	  case Histogram::ValuesRight:{
		for (int i=0; i<valuesStrings.size(); i++){
		  w=fm.width(valuesStrings.at(i));
		  tempPoint.setX( symbolPointsScene.at(i).x() + valuesDistance - 1 );
		  tempPoint.setY( symbolPointsScene.at(i).y() );
		  valuesPoints.append(tempPoint);
		}
		break;
	  }
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

void HistogramPrivate::updateFilling() {
	fillPolygons.clear();

	if (fillingPosition==Histogram::NoFilling) {
		recalcShapeAndBoundingRect();
		return;
	}

	QList<QLineF> fillLines;
	const CartesianPlot* plot = dynamic_cast<const CartesianPlot*>(q->parentAspect());
	const AbstractCoordinateSystem* cSystem = plot->coordinateSystem();

	//if there're no interpolation lines available (Histogram::NoLine selected), create line-interpolation,
	//use already available lines otherwise.
	if (lines.size()) {
		fillLines = lines;
	} else {
		for (int i=0; i<symbolPointsLogical.count()-1; i++){
		  if (!lineSkipGaps && !connectedPointsLogical[i]) continue;
		  fillLines.append(QLineF(symbolPointsLogical.at(i), symbolPointsLogical.at(i+1)));
		}
		fillLines = cSystem->mapLogicalToScene(fillLines);
	}

	//no lines available (no points), nothing to do
	if (!fillLines.size())
		return;


	//create polygon(s):
	//1. Depending on the current zoom-level, only a subset of the curve may be visible in the plot
	//and more of the filling area should be shown than the area defined by the start and end points of the currently visible points.
	//We check first whether the curve crosses the boundaries of the plot and determine new start and end points and put them to the boundaries.
	//2. Furthermore, depending on the current filling type we determine the end point (x- or y-coordinate) where all polygons are closed at the end.
	QPolygonF pol;
	QPointF start = fillLines.at(0).p1(); //starting point of the current polygon, initialize with the first visible point
	QPointF end = fillLines.at(fillLines.size()-1).p2(); //starting point of the current polygon, initialize with the last visible point
	const QPointF& first = symbolPointsLogical.at(0); //first point of the curve, may not be visible currently
	const QPointF& last = symbolPointsLogical.at(symbolPointsLogical.size()-1);//first point of the curve, may not be visible currently
	QPointF edge;
	float xEnd=0, yEnd=0;
	if (fillingPosition == Histogram::FillingAbove) {
		edge = cSystem->mapLogicalToScene(QPointF(plot->xMin(), plot->yMin()));

		//start point
		if (AbstractCoordinateSystem::essentiallyEqual(start.y(), edge.y())) {
			if (first.x() < plot->xMin())
				start = edge;
			else if (first.x() > plot->xMax())
				start = cSystem->mapLogicalToScene(QPointF(plot->xMax(), plot->yMin()));
			else
				start = cSystem->mapLogicalToScene(QPointF(first.x(), plot->yMin()));
		}

		//end point
		if (AbstractCoordinateSystem::essentiallyEqual(end.y(), edge.y())) {
			if (last.x() < plot->xMin())
				end = edge;
			else if (last.x() > plot->xMax())
				end = cSystem->mapLogicalToScene(QPointF(plot->xMax(), plot->yMin()));
			else
				end = cSystem->mapLogicalToScene(QPointF(last.x(), plot->yMin()));
		}

		//coordinate at which to close all polygons
		yEnd = cSystem->mapLogicalToScene(QPointF(plot->xMin(), plot->yMax())).y();
	} else if (fillingPosition == Histogram::FillingBelow) {
		edge = cSystem->mapLogicalToScene(QPointF(plot->xMin(), plot->yMax()));

		//start point
		if (AbstractCoordinateSystem::essentiallyEqual(start.y(), edge.y())) {
			if (first.x() < plot->xMin())
				start = edge;
			else if (first.x() > plot->xMax())
				start = cSystem->mapLogicalToScene(QPointF(plot->xMax(), plot->yMax()));
			else
				start = cSystem->mapLogicalToScene(QPointF(first.x(), plot->yMax()));
		}

		//end point
		if (AbstractCoordinateSystem::essentiallyEqual(end.y(), edge.y())) {
			if (last.x() < plot->xMin())
				end = edge;
			else if (last.x() > plot->xMax())
				end = cSystem->mapLogicalToScene(QPointF(plot->xMax(), plot->yMax()));
			else
				end = cSystem->mapLogicalToScene(QPointF(last.x(), plot->yMax()));
		}

		//coordinate at which to close all polygons
		yEnd = cSystem->mapLogicalToScene(QPointF(plot->xMin(), plot->yMin())).y();
	} else if (fillingPosition == Histogram::FillingZeroBaseline) {
		edge = cSystem->mapLogicalToScene(QPointF(plot->xMin(), plot->yMax()));

		//start point
		if (AbstractCoordinateSystem::essentiallyEqual(start.y(), edge.y())) {
			if (plot->yMax()>0) {
				if (first.x() < plot->xMin())
					start = edge;
				else if (first.x() > plot->xMax())
					start = cSystem->mapLogicalToScene(QPointF(plot->xMax(), plot->yMax()));
				else
					start = cSystem->mapLogicalToScene(QPointF(first.x(), plot->yMax()));
			} else {
				if (first.x() < plot->xMin())
					start = edge;
				else if (first.x() > plot->xMax())
					start = cSystem->mapLogicalToScene(QPointF(plot->xMax(), plot->yMin()));
				else
					start = cSystem->mapLogicalToScene(QPointF(first.x(), plot->yMin()));
			}
		}

		//end point
		if (AbstractCoordinateSystem::essentiallyEqual(end.y(), edge.y())) {
			if (plot->yMax()>0) {
				if (last.x() < plot->xMin())
					end = edge;
				else if (last.x() > plot->xMax())
					end = cSystem->mapLogicalToScene(QPointF(plot->xMax(), plot->yMax()));
				else
					end = cSystem->mapLogicalToScene(QPointF(last.x(), plot->yMax()));
			} else {
				if (last.x() < plot->xMin())
					end = edge;
				else if (last.x() > plot->xMax())
					end = cSystem->mapLogicalToScene(QPointF(plot->xMax(), plot->yMin()));
				else
					end = cSystem->mapLogicalToScene(QPointF(last.x(), plot->yMin()));
			}
		}

		yEnd = cSystem->mapLogicalToScene(QPointF(plot->xMin(), plot->yMin()>0 ? plot->yMin() : 0)).y();
	}else if (fillingPosition == Histogram::FillingLeft) {
		edge = cSystem->mapLogicalToScene(QPointF(plot->xMax(), plot->yMin()));

		//start point
		if (AbstractCoordinateSystem::essentiallyEqual(start.x(), edge.x())) {
			if (first.y() < plot->yMin())
				start = edge;
			else if (first.y() > plot->yMax())
				start = cSystem->mapLogicalToScene(QPointF(plot->xMax(), plot->yMax()));
			else
				start = cSystem->mapLogicalToScene(QPointF(plot->xMax(), first.y()));
		}

		//end point
		if (AbstractCoordinateSystem::essentiallyEqual(end.x(), edge.x())) {
			if (last.y() < plot->yMin())
				end = edge;
			else if (last.y() > plot->yMax())
				end = cSystem->mapLogicalToScene(QPointF(plot->xMax(), plot->yMax()));
			else
				end = cSystem->mapLogicalToScene(QPointF(plot->xMax(), last.y()));
		}

		//coordinate at which to close all polygons
		xEnd = cSystem->mapLogicalToScene(QPointF(plot->xMin(), plot->yMin())).x();
	} else { //FillingRight
		edge = cSystem->mapLogicalToScene(QPointF(plot->xMin(), plot->yMin()));

		//start point
		if (AbstractCoordinateSystem::essentiallyEqual(start.x(), edge.x())) {
			if (first.y() < plot->yMin())
				start = edge;
			else if (first.y() > plot->yMax())
				start = cSystem->mapLogicalToScene(QPointF(plot->xMin(), plot->yMax()));
			else
				start = cSystem->mapLogicalToScene(QPointF(plot->xMin(), first.y()));
		}

		//end point
		if (AbstractCoordinateSystem::essentiallyEqual(end.x(), edge.x())) {
			if (last.y() < plot->yMin())
				end = edge;
			else if (last.y() > plot->yMax())
				end = cSystem->mapLogicalToScene(QPointF(plot->xMin(), plot->yMax()));
			else
				end = cSystem->mapLogicalToScene(QPointF(plot->xMin(), last.y()));
		}

		//coordinate at which to close all polygons
		xEnd = cSystem->mapLogicalToScene(QPointF(plot->xMax(), plot->yMin())).x();
	}

	if (start != fillLines.at(0).p1())
		pol << start;

	QPointF p1, p2;
	for (int i=0; i<fillLines.size(); ++i) {
		const QLineF& line = fillLines.at(i);
		p1 = line.p1();
		p2 = line.p2();
		if (i!=0 && p1!=fillLines.at(i-1).p2()) {
			//the first point of the current line is not equal to the last point of the previous line
			//->check whether we have a break in between.
			bool gap = false; //TODO
			if (!gap) {
				//-> we have no break in the curve -> connect the points by a horizontal/vertical line
				pol << fillLines.at(i-1).p2() << p1;
			} else {
				//-> we have a break in the curve -> close the polygon add it to the polygon list and start a new polygon
				if (fillingPosition==Histogram::FillingAbove || fillingPosition==Histogram::FillingBelow || fillingPosition==Histogram::FillingZeroBaseline) {
					pol << QPointF(fillLines.at(i-1).p2().x(), yEnd);
					pol << QPointF(start.x(), yEnd);
				} else {
					pol << QPointF(xEnd, fillLines.at(i-1).p2().y());
					pol << QPointF(xEnd, start.y());
				}

				fillPolygons << pol;
				pol.clear();
				start = p1;
			}
		}
		pol << p1 << p2;
	}

	if (p2!=end)
		pol << end;

	//close the last polygon
	if (fillingPosition==Histogram::FillingAbove || fillingPosition==Histogram::FillingBelow || fillingPosition==Histogram::FillingZeroBaseline) {
		pol << QPointF(end.x(), yEnd);
		pol << QPointF(start.x(), yEnd);
	} else {
		pol << QPointF(xEnd, end.y());
		pol << QPointF(xEnd, start.y());
	}

	fillPolygons << pol;
	recalcShapeAndBoundingRect();
}

void HistogramPrivate::updateErrorBars(){
  	errorBarsPath = QPainterPath();
	if (xErrorType==Histogram::NoError && yErrorType==Histogram::NoError){
		recalcShapeAndBoundingRect();
		return;
	}

	QList<QLineF> lines;
	float errorPlus, errorMinus;
	const CartesianPlot* plot = dynamic_cast<const CartesianPlot*>(q->parentAspect());
	const AbstractCoordinateSystem* cSystem = plot->coordinateSystem();

	//the cap size for the errorbars is given in scene units.
	//determine first the (half of the) cap size in logical units:
	// * take the first visible point in logical units
	// * convert it to scene units
	// * add to this point an offset corresponding to the cap size in scene units
	// * convert this point back to logical units
	// * subtract from this point the original coordinates (without the new offset)
	//   to determine the cap size in logical units.
	float capSizeX = 0;
	float capSizeY = 0;
	if (errorBarsType != Histogram::ErrorBarsSimple && !symbolPointsLogical.isEmpty()) {
		//determine the index of the first visible point
		size_t i = 0;
		while( i<visiblePoints.size() && !visiblePoints[i])
			i++;

		if (i==visiblePoints.size())
			return; //no visible points -> no error bars to draw

		//cap size for x-error bars
		QPointF pointScene = cSystem->mapLogicalToScene(symbolPointsLogical.at(i));
		pointScene.setY(pointScene.y()-errorBarsCapSize);
		QPointF pointLogical = cSystem->mapSceneToLogical(pointScene);
		capSizeX = (pointLogical.y() - symbolPointsLogical.at(i).y())/2;

		//cap size for y-error bars
		pointScene = cSystem->mapLogicalToScene(symbolPointsLogical.at(i));
		pointScene.setX(pointScene.x()+errorBarsCapSize);
		pointLogical = cSystem->mapSceneToLogical(pointScene);
		capSizeY = (pointLogical.x() - symbolPointsLogical.at(i).x())/2;
	}

	for (int i=0; i<symbolPointsLogical.size(); ++i){
		if (!visiblePoints[i])
			continue;

		const QPointF& point = symbolPointsLogical.at(i);

		//error bars for x
		if (xErrorType!=Histogram::NoError) {
			//determine the values for the errors
			if (xErrorPlusColumn && xErrorPlusColumn->isValid(i) && !xErrorPlusColumn->isMasked(i))
				errorPlus = xErrorPlusColumn->valueAt(i);
			else
				errorPlus = 0;

			if (xErrorType==Histogram::SymmetricError) {
				errorMinus = errorPlus;
			} else {
				if (xErrorMinusColumn && xErrorMinusColumn->isValid(i) && !xErrorMinusColumn->isMasked(i))
					errorMinus = xErrorMinusColumn->valueAt(i);
				else
					errorMinus = 0;
			}

			//draw the error bars
			switch(errorBarsType) {
				case Histogram::ErrorBarsSimple:
					lines.append(QLineF(QPointF(point.x()-errorMinus, point.y()),
										QPointF(point.x()+errorPlus, point.y())));
					break;
				case Histogram::ErrorBarsWithEnds: {
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
		if (yErrorType!=Histogram::NoError) {
			//determine the values for the errors
			if (yErrorPlusColumn && yErrorPlusColumn->isValid(i) && !yErrorPlusColumn->isMasked(i))
				errorPlus = yErrorPlusColumn->valueAt(i);
			else
				errorPlus = 0;

			if (yErrorType==Histogram::SymmetricError) {
				errorMinus = errorPlus;
			} else {
				if (yErrorMinusColumn && yErrorMinusColumn->isValid(i) && !yErrorMinusColumn->isMasked(i) )
					errorMinus = yErrorMinusColumn->valueAt(i);
				else
					errorMinus = 0;
			}

			//draw the error bars
			switch(errorBarsType) {
				case Histogram::ErrorBarsSimple:
					lines.append(QLineF(QPointF(point.x(), point.y()-errorMinus),
										QPointF(point.x(), point.y()+errorPlus)));
					break;
				case Histogram::ErrorBarsWithEnds: {
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
	foreach (const QLineF& line, lines){
		errorBarsPath.moveTo(line.p1());
		errorBarsPath.lineTo(line.p2());
	}

	recalcShapeAndBoundingRect();
}

/*!
  recalculates the outer bounds and the shape of the curve.
*/
void HistogramPrivate::recalcShapeAndBoundingRect() {
	if (m_suppressRecalc)
		return;

	prepareGeometryChange();
	curveShape = QPainterPath();
	if (lineType != Histogram::NoLine){
		curveShape.addPath(WorksheetElement::shapeFromPath(linePath, linePen));
	}

	if (dropLineType != Histogram::NoDropLine){
		curveShape.addPath(WorksheetElement::shapeFromPath(dropLinePath, dropLinePen));
	}

	if (symbolsStyle != Symbol::NoSymbols){
		curveShape.addPath(symbolsPath);
	}

	if (valuesType != Histogram::NoValues){
		curveShape.addPath(valuesPath);
	}

	if (xErrorType != Histogram::NoError || yErrorType != Histogram::NoError){
		curveShape.addPath(WorksheetElement::shapeFromPath(errorBarsPath, errorBarsPen));
	}

	boundingRectangle = curveShape.boundingRect();

	foreach(const QPolygonF& pol, fillPolygons)
		boundingRectangle = boundingRectangle.united(pol.boundingRect());

	//TODO: when the selection is painted, line intersections are visible.
	//simplified() removes those artifacts but is horrible slow for curves with large number of points.
	//search for an alternative.
	//curveShape = curveShape.simplified();

	updatePixmap();
}

void HistogramPrivate::draw(QPainter *painter) {
	//draw filling
	if (fillingPosition != Histogram::NoFilling) {
		painter->setOpacity(fillingOpacity);
		painter->setPen(Qt::SolidLine);
		drawFilling(painter);
	}

	//draw lines
	if (lineType != Histogram::NoLine){
		painter->setOpacity(lineOpacity);
		painter->setPen(linePen);
		painter->setBrush(Qt::NoBrush);
		painter->drawPath(linePath);
	}

	//draw drop lines
	if (dropLineType != Histogram::NoDropLine){
		painter->setOpacity(dropLineOpacity);
		painter->setPen(dropLinePen);
		painter->setBrush(Qt::NoBrush);
		painter->drawPath(dropLinePath);
	}

	//draw error bars
	if ( (xErrorType != Histogram::NoError) || (yErrorType != Histogram::NoError) ){
		painter->setOpacity(errorBarsOpacity);
		painter->setPen(errorBarsPen);
		painter->setBrush(Qt::NoBrush);
		painter->drawPath(errorBarsPath);
	}

	//draw symbols
	if (symbolsStyle != Symbol::NoSymbols){
		painter->setOpacity(symbolsOpacity);
		painter->setPen(symbolsPen);
		painter->setBrush(symbolsBrush);
		drawSymbols(painter);
	}

	//draw values
	if (valuesType != Histogram::NoValues){
		painter->setOpacity(valuesOpacity);
		painter->setPen(valuesColor);
		painter->setBrush(Qt::SolidPattern);
		drawValues(painter);
	}
}

void HistogramPrivate::updatePixmap() {
// 	QTime timer;
// 	timer.start();
	QPixmap pixmap(boundingRectangle.width(), boundingRectangle.height());
	if (boundingRectangle.width()==0 || boundingRectangle.width()==0) {
		m_pixmap = pixmap;
		m_hoverEffectImageIsDirty = true;
		m_selectionEffectImageIsDirty = true;
		return;
	}
	pixmap.fill(Qt::transparent);
	QPainter painter(&pixmap);
	painter.setRenderHint(QPainter::Antialiasing, true);
	painter.translate(-boundingRectangle.topLeft());

	draw(&painter);
	painter.end();

	m_pixmap = pixmap;
	m_hoverEffectImageIsDirty = true;
	m_selectionEffectImageIsDirty = true;
// 	qDebug() << "Update the pixmap: " << timer.elapsed() << "ms";
}



//TODO: move this to a central place
QImage HistogramPrivate::blurred(const QImage& image, const QRect& rect, int radius, bool alphaOnly)
{
    int tab[] = { 14, 10, 8, 6, 5, 5, 4, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2 };
    int alpha = (radius < 1)  ? 16 : (radius > 17) ? 1 : tab[radius-1];

    QImage result = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    int r1 = rect.top();
    int r2 = rect.bottom();
    int c1 = rect.left();
    int c2 = rect.right();

    int bpl = result.bytesPerLine();
    int rgba[4];
    unsigned char* p;

    int i1 = 0;
    int i2 = 3;

    if (alphaOnly)
        i1 = i2 = (QSysInfo::ByteOrder == QSysInfo::BigEndian ? 0 : 3);

    for (int col = c1; col <= c2; col++) {
        p = result.scanLine(r1) + col * 4;
        for (int i = i1; i <= i2; i++)
            rgba[i] = p[i] << 4;

        p += bpl;
        for (int j = r1; j < r2; j++, p += bpl)
            for (int i = i1; i <= i2; i++)
                p[i] = (rgba[i] += ((p[i] << 4) - rgba[i]) * alpha / 16) >> 4;
    }

    for (int row = r1; row <= r2; row++) {
        p = result.scanLine(row) + c1 * 4;
        for (int i = i1; i <= i2; i++)
            rgba[i] = p[i] << 4;

        p += 4;
        for (int j = c1; j < c2; j++, p += 4)
            for (int i = i1; i <= i2; i++)
                p[i] = (rgba[i] += ((p[i] << 4) - rgba[i]) * alpha / 16) >> 4;
    }

    for (int col = c1; col <= c2; col++) {
        p = result.scanLine(r2) + col * 4;
        for (int i = i1; i <= i2; i++)
            rgba[i] = p[i] << 4;

        p -= bpl;
        for (int j = r1; j < r2; j++, p -= bpl)
            for (int i = i1; i <= i2; i++)
                p[i] = (rgba[i] += ((p[i] << 4) - rgba[i]) * alpha / 16) >> 4;
    }

    for (int row = r1; row <= r2; row++) {
        p = result.scanLine(row) + c2 * 4;
        for (int i = i1; i <= i2; i++)
            rgba[i] = p[i] << 4;

        p -= 4;
        for (int j = c1; j < c2; j++, p -= 4)
            for (int i = i1; i <= i2; i++)
                p[i] = (rgba[i] += ((p[i] << 4) - rgba[i]) * alpha / 16) >> 4;
    }

    return result;
}

/*!
  Reimplementation of QGraphicsItem::paint(). This function does the actual painting of the curve.
  \sa QGraphicsItem::paint().
*/
void HistogramPrivate::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget){
// 	qDebug()<<"HistogramPrivate::paint, " + q->name();
	Q_UNUSED(option);
	Q_UNUSED(widget);
	if (!isVisible())
		return;

// 	QTime timer;
// 	timer.start();
	painter->setPen(Qt::NoPen);
	painter->setBrush(Qt::NoBrush);
	painter->setRenderHint(QPainter::SmoothPixmapTransform, true);

// TODO: draw directly
	draw(painter);
// or use pixmap for double buffering
// 	painter->drawPixmap(boundingRectangle.topLeft(), m_pixmap);
// 	qDebug() << "Paint the pixmap: " << timer.elapsed() << "ms";

	if (m_hovered && !isSelected() && !m_printing){
// 		timer.start();
		if (m_hoverEffectImageIsDirty) {
			QPixmap pix = m_pixmap;
			pix.fill(q->hoveredPen.color());
			pix.setAlphaChannel(m_pixmap.alphaChannel());
			m_hoverEffectImage =blurred(pix.toImage(), m_pixmap.rect(), 5,false);
			m_hoverEffectImageIsDirty = false;
		}

		painter->setOpacity(q->hoveredOpacity*2);
		painter->drawImage(boundingRectangle.topLeft(), m_hoverEffectImage, m_pixmap.rect());
// 		qDebug() << "Paint hovering effect: " << timer.elapsed() << "ms";
		return;
	}

	if (isSelected() && !m_printing){
// 		timer.start();
		if (m_selectionEffectImageIsDirty) {
			QPixmap pix = m_pixmap;
			pix.fill(q->selectedPen.color());
			pix.setAlphaChannel(m_pixmap.alphaChannel());
			m_selectionEffectImage = blurred(pix.toImage(), m_pixmap.rect(), 5,false);
			m_selectionEffectImageIsDirty = false;
		}

		painter->setOpacity(q->selectedOpacity*2);
		painter->drawImage(boundingRectangle.topLeft(), m_selectionEffectImage, m_pixmap.rect());
// 		qDebug() << "Paint selection effect: " << timer.elapsed() << "ms";
		return;
	}
}

/*!
	Drawing of symbolsPath is very slow, so we draw every symbol in the loop
	which us much faster (factor 10)
*/
void HistogramPrivate::drawSymbols(QPainter* painter) {
	QPainterPath path = Symbol::pathFromStyle(symbolsStyle);

	QTransform trafo;
	trafo.scale(symbolsSize, symbolsSize);
	path = trafo.map(path);
	trafo.reset();
	if (symbolsRotationAngle != 0) {
		trafo.rotate(symbolsRotationAngle);
		path = trafo.map(path);
	}
	foreach (const QPointF& point, symbolPointsScene) {
		trafo.reset();
		trafo.translate(point.x(), point.y());
		painter->drawPath(trafo.map(path));
	}
}

void HistogramPrivate::drawValues(QPainter* painter) {
	QTransform trafo;
	QPainterPath path;
	for (int i=0; i<valuesPoints.size(); i++){
		path = QPainterPath();
		path.addText( QPoint(0,0), valuesFont, valuesStrings.at(i) );

		trafo.reset();
		trafo.translate( valuesPoints.at(i).x(), valuesPoints.at(i).y() );
		if (valuesRotationAngle!=0)
			trafo.rotate( -valuesRotationAngle );

		painter->drawPath(trafo.map(path));
	}
}

void HistogramPrivate::drawFilling(QPainter* painter) {
	foreach(const QPolygonF& pol, fillPolygons) {
		QRectF rect = pol.boundingRect();
		if (fillingType == PlotArea::Color){
			switch (fillingColorStyle){
				case PlotArea::SingleColor:{
					painter->setBrush(QBrush(fillingFirstColor));
					break;
				}
				case PlotArea::HorizontalLinearGradient:{
					QLinearGradient linearGrad(rect.topLeft(), rect.topRight());
					linearGrad.setColorAt(0, fillingFirstColor);
					linearGrad.setColorAt(1, fillingSecondColor);
					painter->setBrush(QBrush(linearGrad));
					break;
				}
				case PlotArea::VerticalLinearGradient:{
					QLinearGradient linearGrad(rect.topLeft(), rect.bottomLeft());
					linearGrad.setColorAt(0, fillingFirstColor);
					linearGrad.setColorAt(1, fillingSecondColor);
					painter->setBrush(QBrush(linearGrad));
					break;
				}
				case PlotArea::TopLeftDiagonalLinearGradient:{
					QLinearGradient linearGrad(rect.topLeft(), rect.bottomRight());
					linearGrad.setColorAt(0, fillingFirstColor);
					linearGrad.setColorAt(1, fillingSecondColor);
					painter->setBrush(QBrush(linearGrad));
					break;
				}
				case PlotArea::BottomLeftDiagonalLinearGradient:{
					QLinearGradient linearGrad(rect.bottomLeft(), rect.topRight());
					linearGrad.setColorAt(0, fillingFirstColor);
					linearGrad.setColorAt(1, fillingSecondColor);
					painter->setBrush(QBrush(linearGrad));
					break;
				}
				case PlotArea::RadialGradient:{
					QRadialGradient radialGrad(rect.center(), rect.width()/2);
					radialGrad.setColorAt(0, fillingFirstColor);
					radialGrad.setColorAt(1, fillingSecondColor);
					painter->setBrush(QBrush(radialGrad));
					break;
				}
			}
		}else if (fillingType == PlotArea::Image){
			if ( !fillingFileName.trimmed().isEmpty() ) {
				QPixmap pix(fillingFileName);
				switch (fillingImageStyle){
					case PlotArea::ScaledCropped:
						pix = pix.scaled(rect.size().toSize(),Qt::KeepAspectRatioByExpanding,Qt::SmoothTransformation);
						painter->setBrush(QBrush(pix));
						painter->setBrushOrigin(pix.size().width()/2,pix.size().height()/2);
						break;
					case PlotArea::Scaled:
						pix = pix.scaled(rect.size().toSize(),Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
						painter->setBrush(QBrush(pix));
						painter->setBrushOrigin(pix.size().width()/2,pix.size().height()/2);
						break;
					case PlotArea::ScaledAspectRatio:
						pix = pix.scaled(rect.size().toSize(),Qt::KeepAspectRatio,Qt::SmoothTransformation);
						painter->setBrush(QBrush(pix));
						painter->setBrushOrigin(pix.size().width()/2,pix.size().height()/2);
						break;
					case PlotArea::Centered:{
						QPixmap backpix(rect.size().toSize());
						backpix.fill();
						QPainter p(&backpix);
						p.drawPixmap(QPointF(0,0),pix);
						p.end();
						painter->setBrush(QBrush(backpix));
						painter->setBrushOrigin(-pix.size().width()/2,-pix.size().height()/2);
						break;
					}
					case PlotArea::Tiled:
						painter->setBrush(QBrush(pix));
						break;
					case PlotArea::CenterTiled:
						painter->setBrush(QBrush(pix));
						painter->setBrushOrigin(pix.size().width()/2,pix.size().height()/2);
				}
			}
		} else if (fillingType == PlotArea::Pattern){
			painter->setBrush(QBrush(fillingFirstColor,fillingBrushStyle));
		}

		painter->drawPolygon(pol);
	}
}

void HistogramPrivate::hoverEnterEvent(QGraphicsSceneHoverEvent*) {
	const CartesianPlot* plot = dynamic_cast<const CartesianPlot*>(q->parentAspect());
	if (plot->mouseMode() == CartesianPlot::SelectionMode && !isSelected()) {
		m_hovered = true;
		q->hovered();
		update();
	}
}

void HistogramPrivate::hoverLeaveEvent(QGraphicsSceneHoverEvent*) {
	const CartesianPlot* plot = dynamic_cast<const CartesianPlot*>(q->parentAspect());
	if (plot->mouseMode() == CartesianPlot::SelectionMode && m_hovered) {
		m_hovered = false;
		q->unhovered();
		update();
	}
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################
//! Save as XML
void Histogram::save(QXmlStreamWriter* writer) const{
	Q_D(const Histogram);

    writer->writeStartElement( "Histogram" );
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
	writer->writeAttribute( "skipGaps", QString::number(d->lineSkipGaps) );
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
	writer->writeAttribute( "symbolsStyle", QString::number(d->symbolsStyle) );
	writer->writeAttribute( "opacity", QString::number(d->symbolsOpacity) );
	writer->writeAttribute( "rotation", QString::number(d->symbolsRotationAngle) );
	writer->writeAttribute( "size", QString::number(d->symbolsSize) );
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

	//Filling
	writer->writeStartElement( "filling" );
	writer->writeAttribute( "position", QString::number(d->fillingPosition) );
    writer->writeAttribute( "type", QString::number(d->fillingType) );
    writer->writeAttribute( "colorStyle", QString::number(d->fillingColorStyle) );
    writer->writeAttribute( "imageStyle", QString::number(d->fillingImageStyle) );
    writer->writeAttribute( "brushStyle", QString::number(d->fillingBrushStyle) );
    writer->writeAttribute( "firstColor_r", QString::number(d->fillingFirstColor.red()) );
    writer->writeAttribute( "firstColor_g", QString::number(d->fillingFirstColor.green()) );
    writer->writeAttribute( "firstColor_b", QString::number(d->fillingFirstColor.blue()) );
    writer->writeAttribute( "secondColor_r", QString::number(d->fillingSecondColor.red()) );
    writer->writeAttribute( "secondColor_g", QString::number(d->fillingSecondColor.green()) );
    writer->writeAttribute( "secondColor_b", QString::number(d->fillingSecondColor.blue()) );
    writer->writeAttribute( "fileName", d->fillingFileName );
	writer->writeAttribute( "opacity", QString::number(d->fillingOpacity) );
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

	writer->writeEndElement(); //close "Histogram" section
}

//! Load from XML
bool Histogram::load(XmlStreamReader* reader){
	Q_D(Histogram);

    if(!reader->isStartElement() || reader->name() != "Histogram"){
        reader->raiseError(i18n("no xy-curve element found"));
        return false;
    }

    if (!readBasicAttributes(reader))
        return false;

    QString attributeWarning = i18n("Attribute '%1' missing or empty, default value is used");
    QXmlStreamAttributes attribs;
    QString str;

    while (!reader->atEnd()){
        reader->readNext();
        if (reader->isEndElement() && reader->name() == "Histogram")
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
                d->lineType = (Histogram::LineType)str.toInt();

			str = attribs.value("skipGaps").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'skipGps'"));
            else
                d->lineSkipGaps = str.toInt();

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
                d->dropLineType = (Histogram::DropLineType)str.toInt();

			READ_QPEN(d->dropLinePen);

			str = attribs.value("opacity").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'opacity'"));
            else
                d->dropLineOpacity = str.toDouble();
		}else if (reader->name() == "symbols"){
			attribs = reader->attributes();

			str = attribs.value("symbolsStyle").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'symbolsStyle'"));
            else
                d->symbolsStyle = (Symbol::Style)str.toInt();

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

			READ_QBRUSH(d->symbolsBrush);
			READ_QPEN(d->symbolsPen);
		}else if (reader->name() == "values"){
			attribs = reader->attributes();

			str = attribs.value("type").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'type'"));
            else
                d->valuesType = (Histogram::ValuesType)str.toInt();

			READ_COLUMN(valuesColumn);

			str = attribs.value("position").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'position'"));
            else
                d->valuesPosition = (Histogram::ValuesPosition)str.toInt();

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
                d->valuesOpacity = str.toDouble();

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
		}else if (reader->name() == "filling"){
            attribs = reader->attributes();

			str = attribs.value("position").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("position"));
            else
                d->fillingPosition = Histogram::FillingPosition(str.toInt());

            str = attribs.value("type").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("type"));
            else
                d->fillingType = PlotArea::BackgroundType(str.toInt());

            str = attribs.value("colorStyle").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("colorStyle"));
            else
                d->fillingColorStyle = PlotArea::BackgroundColorStyle(str.toInt());

            str = attribs.value("imageStyle").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("imageStyle"));
            else
                d->fillingImageStyle = PlotArea::BackgroundImageStyle(str.toInt());

            str = attribs.value("brushStyle").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("brushStyle"));
            else
                d->fillingBrushStyle = Qt::BrushStyle(str.toInt());

            str = attribs.value("firstColor_r").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("firstColor_r"));
            else
                d->fillingFirstColor.setRed(str.toInt());

            str = attribs.value("firstColor_g").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("firstColor_g"));
            else
                d->fillingFirstColor.setGreen(str.toInt());

            str = attribs.value("firstColor_b").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("firstColor_b"));
            else
                d->fillingFirstColor.setBlue(str.toInt());

            str = attribs.value("secondColor_r").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("secondColor_r"));
            else
                d->fillingSecondColor.setRed(str.toInt());

            str = attribs.value("secondColor_g").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("secondColor_g"));
            else
                d->fillingSecondColor.setGreen(str.toInt());

            str = attribs.value("secondColor_b").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("secondColor_b"));
            else
                d->fillingSecondColor.setBlue(str.toInt());

            str = attribs.value("fileName").toString();
            d->fillingFileName = str;

            str = attribs.value("opacity").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("opacity"));
            else
                d->fillingOpacity = str.toDouble();
		}else if (reader->name() == "errorBars"){
			attribs = reader->attributes();

			str = attribs.value("xErrorType").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'xErrorType'"));
            else
                d->xErrorType = (Histogram::ErrorType)str.toInt();

			READ_COLUMN(xErrorPlusColumn);
			READ_COLUMN(xErrorMinusColumn);

			str = attribs.value("yErrorType").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'yErrorType'"));
            else
                d->yErrorType = (Histogram::ErrorType)str.toInt();

			READ_COLUMN(yErrorPlusColumn);
			READ_COLUMN(yErrorMinusColumn);

			str = attribs.value("type").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'type'"));
            else
                d->errorBarsType = (Histogram::ErrorBarsType)str.toInt();

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
