/***************************************************************************
    File                 : XYCurve.cpp
    Project              : LabPlot
    Description          : A xy-curve
    --------------------------------------------------------------------
    Copyright            : (C) 2010-2020 Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2013-2020 Stefan Gerlach (stefan.gerlach@uni.kn)

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
  \brief A 2D-curve, provides an interface for editing many properties of the curve.

  \ingroup worksheet
*/

#include "XYCurve.h"
#include "XYCurvePrivate.h"
#include "backend/core/column/Column.h"
#include "backend/worksheet/plots/cartesian/CartesianCoordinateSystem.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/lib/commandtemplates.h"
#include "backend/core/Project.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/macros.h"
#include "backend/lib/trace.h"
#include "backend/gsl/errors.h"
#include "tools/ImageTools.h"

#include <QPainter>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QDesktopWidget>

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

extern "C" {
#include <gsl/gsl_spline.h>
#include <gsl/gsl_errno.h>
}

XYCurve::XYCurve(const QString &name, AspectType type)
	: WorksheetElement(name, type), d_ptr(new XYCurvePrivate(this)) {

	init();
}

XYCurve::XYCurve(const QString& name, XYCurvePrivate* dd, AspectType type)
	: WorksheetElement(name, type), d_ptr(dd) {

	init();
}

//no need to delete the d-pointer here - it inherits from QGraphicsItem
//and is deleted during the cleanup in QGraphicsScene
XYCurve::~XYCurve() = default;

void XYCurve::finalizeAdd() {
	Q_D(XYCurve);
	d->plot = static_cast<const CartesianPlot*>(parentAspect());
	d->cSystem = static_cast<const CartesianCoordinateSystem*>(d->plot->coordinateSystem());
}

void XYCurve::init() {
	Q_D(XYCurve);

	KConfig config;
	KConfigGroup group = config.group("XYCurve");

	d->lineType = (XYCurve::LineType) group.readEntry("LineType", (int)XYCurve::Line);
	d->lineIncreasingXOnly = group.readEntry("LineIncreasingXOnly", false);
	d->lineSkipGaps = group.readEntry("SkipLineGaps", false);
	d->lineInterpolationPointsCount = group.readEntry("LineInterpolationPointsCount", 1);
	d->linePen.setStyle( (Qt::PenStyle) group.readEntry("LineStyle", (int)Qt::SolidLine) );
	d->linePen.setColor( group.readEntry("LineColor", QColor(Qt::black)) );
	d->linePen.setWidthF( group.readEntry("LineWidth", Worksheet::convertToSceneUnits(1.0, Worksheet::Point)) );
	d->lineOpacity = group.readEntry("LineOpacity", 1.0);

	d->dropLineType = (XYCurve::DropLineType) group.readEntry("DropLineType", (int)XYCurve::NoLine);
	d->dropLinePen.setStyle( (Qt::PenStyle) group.readEntry("DropLineStyle", (int)Qt::SolidLine) );
	d->dropLinePen.setColor( group.readEntry("DropLineColor", QColor(Qt::black)));
	d->dropLinePen.setWidthF( group.readEntry("DropLineWidth", Worksheet::convertToSceneUnits(1.0, Worksheet::Point)) );
	d->dropLineOpacity = group.readEntry("DropLineOpacity", 1.0);

	d->symbolsStyle = (Symbol::Style)group.readEntry("SymbolStyle", (int)Symbol::NoSymbols);
	d->symbolsSize = group.readEntry("SymbolSize", Worksheet::convertToSceneUnits(5, Worksheet::Point));
	d->symbolsRotationAngle = group.readEntry("SymbolRotation", 0.0);
	d->symbolsOpacity = group.readEntry("SymbolOpacity", 1.0);
	d->symbolsBrush.setStyle( (Qt::BrushStyle)group.readEntry("SymbolFillingStyle", (int)Qt::SolidPattern) );
	d->symbolsBrush.setColor( group.readEntry("SymbolFillingColor", QColor(Qt::black)) );
	d->symbolsPen.setStyle( (Qt::PenStyle)group.readEntry("SymbolBorderStyle", (int)Qt::SolidLine) );
	d->symbolsPen.setColor( group.readEntry("SymbolBorderColor", QColor(Qt::black)) );
	d->symbolsPen.setWidthF( group.readEntry("SymbolBorderWidth", Worksheet::convertToSceneUnits(0.0, Worksheet::Point)) );

	d->valuesType = (XYCurve::ValuesType) group.readEntry("ValuesType", (int)XYCurve::NoValues);
	d->valuesPosition = (XYCurve::ValuesPosition) group.readEntry("ValuesPosition", (int)XYCurve::ValuesAbove);
	d->valuesDistance = group.readEntry("ValuesDistance", Worksheet::convertToSceneUnits(5, Worksheet::Point));
	d->valuesRotationAngle = group.readEntry("ValuesRotation", 0.0);
	d->valuesOpacity = group.readEntry("ValuesOpacity", 1.0);
	d->valuesNumericFormat = group.readEntry("ValuesNumericFormat", "f").at(0).toLatin1();
	d->valuesPrecision = group.readEntry("ValuesNumericFormat", 2);
	d->valuesDateTimeFormat = group.readEntry("ValuesDateTimeFormat", "yyyy-MM-dd");
	d->valuesPrefix = group.readEntry("ValuesPrefix", "");
	d->valuesSuffix = group.readEntry("ValuesSuffix", "");
	d->valuesFont = group.readEntry("ValuesFont", QFont());
	d->valuesFont.setPixelSize( Worksheet::convertToSceneUnits( 8, Worksheet::Point ) );
	d->valuesColor = group.readEntry("ValuesColor", QColor(Qt::black));

	d->fillingPosition = (XYCurve::FillingPosition) group.readEntry("FillingPosition", (int)XYCurve::NoFilling);
	d->fillingType = (PlotArea::BackgroundType) group.readEntry("FillingType", (int)PlotArea::Color);
	d->fillingColorStyle = (PlotArea::BackgroundColorStyle) group.readEntry("FillingColorStyle", (int) PlotArea::SingleColor);
	d->fillingImageStyle = (PlotArea::BackgroundImageStyle) group.readEntry("FillingImageStyle", (int) PlotArea::Scaled);
	d->fillingBrushStyle = (Qt::BrushStyle) group.readEntry("FillingBrushStyle", (int) Qt::SolidPattern);
	d->fillingFileName = group.readEntry("FillingFileName", QString());
	d->fillingFirstColor = group.readEntry("FillingFirstColor", QColor(Qt::white));
	d->fillingSecondColor = group.readEntry("FillingSecondColor", QColor(Qt::black));
	d->fillingOpacity = group.readEntry("FillingOpacity", 1.0);

	d->xErrorType = (XYCurve::ErrorType) group.readEntry("XErrorType", (int)XYCurve::NoError);
	d->yErrorType = (XYCurve::ErrorType) group.readEntry("YErrorType", (int)XYCurve::NoError);
	d->errorBarsType = (XYCurve::ErrorBarsType) group.readEntry("ErrorBarsType", (int)XYCurve::ErrorBarsSimple);
	d->errorBarsCapSize = group.readEntry( "ErrorBarsCapSize", Worksheet::convertToSceneUnits(10, Worksheet::Point) );
	d->errorBarsPen.setStyle( (Qt::PenStyle)group.readEntry("ErrorBarsStyle", (int)Qt::SolidLine) );
	d->errorBarsPen.setColor( group.readEntry("ErrorBarsColor", QColor(Qt::black)) );
	d->errorBarsPen.setWidthF( group.readEntry("ErrorBarsWidth", Worksheet::convertToSceneUnits(1.0, Worksheet::Point)) );
	d->errorBarsOpacity = group.readEntry("ErrorBarsOpacity", 1.0);
}

void XYCurve::initActions() {
	visibilityAction = new QAction(QIcon::fromTheme("view-visible"), i18n("Visible"), this);
	visibilityAction->setCheckable(true);
	connect(visibilityAction, SIGNAL(triggered(bool)), this, SLOT(visibilityChanged()));

	navigateToAction = new QAction(QIcon::fromTheme("go-next-view"), QString(), this);
	connect(navigateToAction, SIGNAL(triggered(bool)), this, SLOT(navigateTo()));

	m_menusInitialized = true;
}

QMenu* XYCurve::createContextMenu() {
	if (!m_menusInitialized)
		initActions();

	QMenu* menu = WorksheetElement::createContextMenu();
	QAction* firstAction = menu->actions().at(1); //skip the first action because of the "title-action"
	visibilityAction->setChecked(isVisible());
	menu->insertAction(firstAction, visibilityAction);

	//"data analysis" menu
	auto* plot = static_cast<CartesianPlot*>(parentAspect());
	menu->insertMenu(visibilityAction, plot->analysisMenu());
	menu->insertSeparator(visibilityAction);

	//"Navigate to spreadsheet"-action, show only if x- or y-columns have data from a spreadsheet
	AbstractAspect* parentSpreadsheet = nullptr;
	if (xColumn() && dynamic_cast<Spreadsheet*>(xColumn()->parentAspect()) )
		parentSpreadsheet = xColumn()->parentAspect();
	else if (yColumn() && dynamic_cast<Spreadsheet*>(yColumn()->parentAspect()) )
		parentSpreadsheet = yColumn()->parentAspect();

	if (parentSpreadsheet) {
		navigateToAction->setText(i18n("Navigate to \"%1\"", parentSpreadsheet->name()));
		navigateToAction->setData(parentSpreadsheet->path());
		menu->insertAction(visibilityAction, navigateToAction);
		menu->insertSeparator(visibilityAction);
	}

	//if the context menu is called on an item that is not selected yet, select it
	if (!graphicsItem()->isSelected())
		graphicsItem()->setSelected(true);

	return menu;
}

/*!
	Returns an icon to be used in the project explorer.
*/
QIcon XYCurve::icon() const {
	return QIcon::fromTheme("labplot-xy-curve");
}

QGraphicsItem* XYCurve::graphicsItem() const {
	return d_ptr;
}

STD_SWAP_METHOD_SETTER_CMD_IMPL(XYCurve, SetVisible, bool, swapVisible)
void XYCurve::setVisible(bool on) {
	Q_D(XYCurve);
	exec(new XYCurveSetVisibleCmd(d, on, on ? ki18n("%1: set visible") : ki18n("%1: set invisible")));
}

bool XYCurve::isVisible() const {
	Q_D(const XYCurve);
	return d->isVisible();
}

void XYCurve::setPrinting(bool on) {
	Q_D(XYCurve);
	d->setPrinting(on);
}

//##############################################################################
//##########################  getter methods  ##################################
//##############################################################################

//data source
BASIC_SHARED_D_READER_IMPL(XYCurve, const AbstractColumn*, xColumn, xColumn)
BASIC_SHARED_D_READER_IMPL(XYCurve, const AbstractColumn*, yColumn, yColumn)
CLASS_SHARED_D_READER_IMPL(XYCurve, QString, xColumnPath, xColumnPath)
CLASS_SHARED_D_READER_IMPL(XYCurve, QString, yColumnPath, yColumnPath)

//line
BASIC_SHARED_D_READER_IMPL(XYCurve, XYCurve::LineType, lineType, lineType)
BASIC_SHARED_D_READER_IMPL(XYCurve, bool, lineSkipGaps, lineSkipGaps)
BASIC_SHARED_D_READER_IMPL(XYCurve, bool, lineIncreasingXOnly, lineIncreasingXOnly)
BASIC_SHARED_D_READER_IMPL(XYCurve, int, lineInterpolationPointsCount, lineInterpolationPointsCount)
CLASS_SHARED_D_READER_IMPL(XYCurve, QPen, linePen, linePen)
BASIC_SHARED_D_READER_IMPL(XYCurve, qreal, lineOpacity, lineOpacity)

//droplines
BASIC_SHARED_D_READER_IMPL(XYCurve, XYCurve::DropLineType, dropLineType, dropLineType)
CLASS_SHARED_D_READER_IMPL(XYCurve, QPen, dropLinePen, dropLinePen)
BASIC_SHARED_D_READER_IMPL(XYCurve, qreal, dropLineOpacity, dropLineOpacity)

//symbols
BASIC_SHARED_D_READER_IMPL(XYCurve, Symbol::Style, symbolsStyle, symbolsStyle)
BASIC_SHARED_D_READER_IMPL(XYCurve, qreal, symbolsOpacity, symbolsOpacity)
BASIC_SHARED_D_READER_IMPL(XYCurve, qreal, symbolsRotationAngle, symbolsRotationAngle)
BASIC_SHARED_D_READER_IMPL(XYCurve, qreal, symbolsSize, symbolsSize)
CLASS_SHARED_D_READER_IMPL(XYCurve, QBrush, symbolsBrush, symbolsBrush)
CLASS_SHARED_D_READER_IMPL(XYCurve, QPen, symbolsPen, symbolsPen)

//values
BASIC_SHARED_D_READER_IMPL(XYCurve, XYCurve::ValuesType, valuesType, valuesType)
BASIC_SHARED_D_READER_IMPL(XYCurve, const AbstractColumn *, valuesColumn, valuesColumn)
CLASS_SHARED_D_READER_IMPL(XYCurve, QString, valuesColumnPath, valuesColumnPath)

BASIC_SHARED_D_READER_IMPL(XYCurve, XYCurve::ValuesPosition, valuesPosition, valuesPosition)
BASIC_SHARED_D_READER_IMPL(XYCurve, qreal, valuesDistance, valuesDistance)
BASIC_SHARED_D_READER_IMPL(XYCurve, qreal, valuesRotationAngle, valuesRotationAngle)
BASIC_SHARED_D_READER_IMPL(XYCurve, qreal, valuesOpacity, valuesOpacity)
CLASS_SHARED_D_READER_IMPL(XYCurve, char, valuesNumericFormat, valuesNumericFormat)
BASIC_SHARED_D_READER_IMPL(XYCurve, int, valuesPrecision, valuesPrecision)
CLASS_SHARED_D_READER_IMPL(XYCurve, QString, valuesDateTimeFormat, valuesDateTimeFormat)
CLASS_SHARED_D_READER_IMPL(XYCurve, QString, valuesPrefix, valuesPrefix)
CLASS_SHARED_D_READER_IMPL(XYCurve, QString, valuesSuffix, valuesSuffix)
CLASS_SHARED_D_READER_IMPL(XYCurve, QColor, valuesColor, valuesColor)
CLASS_SHARED_D_READER_IMPL(XYCurve, QFont, valuesFont, valuesFont)

//filling
BASIC_SHARED_D_READER_IMPL(XYCurve, XYCurve::FillingPosition, fillingPosition, fillingPosition)
BASIC_SHARED_D_READER_IMPL(XYCurve, PlotArea::BackgroundType, fillingType, fillingType)
BASIC_SHARED_D_READER_IMPL(XYCurve, PlotArea::BackgroundColorStyle, fillingColorStyle, fillingColorStyle)
BASIC_SHARED_D_READER_IMPL(XYCurve, PlotArea::BackgroundImageStyle, fillingImageStyle, fillingImageStyle)
CLASS_SHARED_D_READER_IMPL(XYCurve, Qt::BrushStyle, fillingBrushStyle, fillingBrushStyle)
CLASS_SHARED_D_READER_IMPL(XYCurve, QColor, fillingFirstColor, fillingFirstColor)
CLASS_SHARED_D_READER_IMPL(XYCurve, QColor, fillingSecondColor, fillingSecondColor)
CLASS_SHARED_D_READER_IMPL(XYCurve, QString, fillingFileName, fillingFileName)
BASIC_SHARED_D_READER_IMPL(XYCurve, qreal, fillingOpacity, fillingOpacity)

//error bars
BASIC_SHARED_D_READER_IMPL(XYCurve, XYCurve::ErrorType, xErrorType, xErrorType)
BASIC_SHARED_D_READER_IMPL(XYCurve, const AbstractColumn*, xErrorPlusColumn, xErrorPlusColumn)
BASIC_SHARED_D_READER_IMPL(XYCurve, const AbstractColumn*, xErrorMinusColumn, xErrorMinusColumn)
BASIC_SHARED_D_READER_IMPL(XYCurve, XYCurve::ErrorType, yErrorType, yErrorType)
BASIC_SHARED_D_READER_IMPL(XYCurve, const AbstractColumn*, yErrorPlusColumn, yErrorPlusColumn)
BASIC_SHARED_D_READER_IMPL(XYCurve, const AbstractColumn*, yErrorMinusColumn, yErrorMinusColumn)

CLASS_SHARED_D_READER_IMPL(XYCurve, QString, xErrorPlusColumnPath, xErrorPlusColumnPath)
CLASS_SHARED_D_READER_IMPL(XYCurve, QString, xErrorMinusColumnPath, xErrorMinusColumnPath)
CLASS_SHARED_D_READER_IMPL(XYCurve, QString, yErrorPlusColumnPath, yErrorPlusColumnPath)
CLASS_SHARED_D_READER_IMPL(XYCurve, QString, yErrorMinusColumnPath, yErrorMinusColumnPath)

BASIC_SHARED_D_READER_IMPL(XYCurve, XYCurve::ErrorBarsType, errorBarsType, errorBarsType)
BASIC_SHARED_D_READER_IMPL(XYCurve, qreal, errorBarsCapSize, errorBarsCapSize)
CLASS_SHARED_D_READER_IMPL(XYCurve, QPen, errorBarsPen, errorBarsPen)
BASIC_SHARED_D_READER_IMPL(XYCurve, qreal, errorBarsOpacity, errorBarsOpacity)

/*!
 * return \c true if the data in the source columns (x, y) used in the analysis curves, \c false otherwise
 */
bool XYCurve::isSourceDataChangedSinceLastRecalc() const {
	Q_D(const XYCurve);
	return d->sourceDataChangedSinceLastRecalc;
}

//##############################################################################
//#################  setter methods and undo commands ##########################
//##############################################################################

// 1) add XYCurveSetXColumnCmd as friend class to XYCurve
// 2) add XYCURVE_COLUMN_CONNECT(x) as private method to XYCurve
// 3) define all missing slots
XYCURVE_COLUMN_SETTER_CMD_IMPL_F_S(X, x, recalcLogicalPoints)
void XYCurve::setXColumn(const AbstractColumn* column) {
	Q_D(XYCurve);
	if (column != d->xColumn)
		exec(new XYCurveSetXColumnCmd(d, column, ki18n("%1: x-data source changed")));
}

XYCURVE_COLUMN_SETTER_CMD_IMPL_F_S(Y, y, recalcLogicalPoints)
void XYCurve::setYColumn(const AbstractColumn* column) {
	Q_D(XYCurve);
	if (column != d->yColumn)
		exec(new XYCurveSetYColumnCmd(d, column, ki18n("%1: y-data source changed")));
}

void XYCurve::setXColumnPath(const QString& path) {
	Q_D(XYCurve);
	d->xColumnPath = path;
}

void XYCurve::setYColumnPath(const QString& path) {
	Q_D(XYCurve);
	d->yColumnPath = path;
}

//Line
STD_SETTER_CMD_IMPL_F_S(XYCurve, SetLineType, XYCurve::LineType, lineType, updateLines)
void XYCurve::setLineType(LineType type) {
	Q_D(XYCurve);
	if (type != d->lineType)
		exec(new XYCurveSetLineTypeCmd(d, type, ki18n("%1: line type changed")));
}

STD_SETTER_CMD_IMPL_F_S(XYCurve, SetLineSkipGaps, bool, lineSkipGaps, updateLines)
void XYCurve::setLineSkipGaps(bool skip) {
	Q_D(XYCurve);
	if (skip != d->lineSkipGaps)
		exec(new XYCurveSetLineSkipGapsCmd(d, skip, ki18n("%1: set skip line gaps")));
}

STD_SETTER_CMD_IMPL_F_S(XYCurve, SetLineIncreasingXOnly, bool, lineIncreasingXOnly, updateLines)
void XYCurve::setLineIncreasingXOnly(bool incr) {
	Q_D(XYCurve);
	if (incr != d->lineIncreasingXOnly)
		exec(new XYCurveSetLineIncreasingXOnlyCmd(d, incr, ki18n("%1: set increasing X")));
}

STD_SETTER_CMD_IMPL_F_S(XYCurve, SetLineInterpolationPointsCount, int, lineInterpolationPointsCount, updateLines)
void XYCurve::setLineInterpolationPointsCount(int count) {
	Q_D(XYCurve);
	if (count != d->lineInterpolationPointsCount)
		exec(new XYCurveSetLineInterpolationPointsCountCmd(d, count, ki18n("%1: set the number of interpolation points")));
}

STD_SETTER_CMD_IMPL_F_S(XYCurve, SetLinePen, QPen, linePen, recalcShapeAndBoundingRect)
void XYCurve::setLinePen(const QPen &pen) {
	Q_D(XYCurve);
	if (pen != d->linePen)
		exec(new XYCurveSetLinePenCmd(d, pen, ki18n("%1: set line style")));
}

STD_SETTER_CMD_IMPL_F_S(XYCurve, SetLineOpacity, qreal, lineOpacity, updatePixmap);
void XYCurve::setLineOpacity(qreal opacity) {
	Q_D(XYCurve);
	if (opacity != d->lineOpacity)
		exec(new XYCurveSetLineOpacityCmd(d, opacity, ki18n("%1: set line opacity")));
}

//Drop lines
STD_SETTER_CMD_IMPL_F_S(XYCurve, SetDropLineType, XYCurve::DropLineType, dropLineType, updateDropLines)
void XYCurve::setDropLineType(DropLineType type) {
	Q_D(XYCurve);
	if (type != d->dropLineType)
		exec(new XYCurveSetDropLineTypeCmd(d, type, ki18n("%1: drop line type changed")));
}

STD_SETTER_CMD_IMPL_F_S(XYCurve, SetDropLinePen, QPen, dropLinePen, recalcShapeAndBoundingRect)
void XYCurve::setDropLinePen(const QPen &pen) {
	Q_D(XYCurve);
	if (pen != d->dropLinePen)
		exec(new XYCurveSetDropLinePenCmd(d, pen, ki18n("%1: set drop line style")));
}

STD_SETTER_CMD_IMPL_F_S(XYCurve, SetDropLineOpacity, qreal, dropLineOpacity, updatePixmap)
void XYCurve::setDropLineOpacity(qreal opacity) {
	Q_D(XYCurve);
	if (opacity != d->dropLineOpacity)
		exec(new XYCurveSetDropLineOpacityCmd(d, opacity, ki18n("%1: set drop line opacity")));
}

// Symbols-Tab
STD_SETTER_CMD_IMPL_F_S(XYCurve, SetSymbolsStyle, Symbol::Style, symbolsStyle, retransform)
void XYCurve::setSymbolsStyle(Symbol::Style style) {
	Q_D(XYCurve);
	if (style != d->symbolsStyle)
		exec(new XYCurveSetSymbolsStyleCmd(d, style, ki18n("%1: set symbol style")));
}

STD_SETTER_CMD_IMPL_F_S(XYCurve, SetSymbolsSize, qreal, symbolsSize, updateSymbols)
void XYCurve::setSymbolsSize(qreal size) {
	Q_D(XYCurve);
	if (!qFuzzyCompare(1 + size, 1 + d->symbolsSize))
		exec(new XYCurveSetSymbolsSizeCmd(d, size, ki18n("%1: set symbol size")));
}

STD_SETTER_CMD_IMPL_F_S(XYCurve, SetSymbolsRotationAngle, qreal, symbolsRotationAngle, updateSymbols)
void XYCurve::setSymbolsRotationAngle(qreal angle) {
	Q_D(XYCurve);
	if (!qFuzzyCompare(1 + angle, 1 + d->symbolsRotationAngle))
		exec(new XYCurveSetSymbolsRotationAngleCmd(d, angle, ki18n("%1: rotate symbols")));
}

STD_SETTER_CMD_IMPL_F_S(XYCurve, SetSymbolsBrush, QBrush, symbolsBrush, updatePixmap)
void XYCurve::setSymbolsBrush(const QBrush &brush) {
	Q_D(XYCurve);
	if (brush != d->symbolsBrush)
		exec(new XYCurveSetSymbolsBrushCmd(d, brush, ki18n("%1: set symbol filling")));
}

STD_SETTER_CMD_IMPL_F_S(XYCurve, SetSymbolsPen, QPen, symbolsPen, updateSymbols)
void XYCurve::setSymbolsPen(const QPen &pen) {
	Q_D(XYCurve);
	if (pen != d->symbolsPen)
		exec(new XYCurveSetSymbolsPenCmd(d, pen, ki18n("%1: set symbol outline style")));
}

STD_SETTER_CMD_IMPL_F_S(XYCurve, SetSymbolsOpacity, qreal, symbolsOpacity, updatePixmap)
void XYCurve::setSymbolsOpacity(qreal opacity) {
	Q_D(XYCurve);
	if (opacity != d->symbolsOpacity)
		exec(new XYCurveSetSymbolsOpacityCmd(d, opacity, ki18n("%1: set symbols opacity")));
}

//Values-Tab
STD_SETTER_CMD_IMPL_F_S(XYCurve, SetValuesType, XYCurve::ValuesType, valuesType, updateValues)
void XYCurve::setValuesType(XYCurve::ValuesType type) {
	Q_D(XYCurve);
	if (type != d->valuesType)
		exec(new XYCurveSetValuesTypeCmd(d, type, ki18n("%1: set values type")));
}

XYCURVE_COLUMN_SETTER_CMD_IMPL_F_S(Values, values, updateValues)
void XYCurve::setValuesColumn(const AbstractColumn* column) {
	Q_D(XYCurve);
	if (column != d->valuesColumn) {
		exec(new XYCurveSetValuesColumnCmd(d, column, ki18n("%1: set values column")));
		if (column)
			connect(column, SIGNAL(dataChanged(const AbstractColumn*)), this, SLOT(updateValues()));
	}
}

void XYCurve::setValuesColumnPath(const QString& path) {
	Q_D(XYCurve);
	d->valuesColumnPath = path;
}

STD_SETTER_CMD_IMPL_F_S(XYCurve, SetValuesPosition, XYCurve::ValuesPosition, valuesPosition, updateValues)
void XYCurve::setValuesPosition(ValuesPosition position) {
	Q_D(XYCurve);
	if (position != d->valuesPosition)
		exec(new XYCurveSetValuesPositionCmd(d, position, ki18n("%1: set values position")));
}

STD_SETTER_CMD_IMPL_F_S(XYCurve, SetValuesDistance, qreal, valuesDistance, updateValues)
void XYCurve::setValuesDistance(qreal distance) {
	Q_D(XYCurve);
	if (distance != d->valuesDistance)
		exec(new XYCurveSetValuesDistanceCmd(d, distance, ki18n("%1: set values distance")));
}

STD_SETTER_CMD_IMPL_F_S(XYCurve, SetValuesRotationAngle, qreal, valuesRotationAngle, updateValues)
void XYCurve::setValuesRotationAngle(qreal angle) {
	Q_D(XYCurve);
	if (!qFuzzyCompare(1 + angle, 1 + d->valuesRotationAngle))
		exec(new XYCurveSetValuesRotationAngleCmd(d, angle, ki18n("%1: rotate values")));
}

STD_SETTER_CMD_IMPL_F_S(XYCurve, SetValuesOpacity, qreal, valuesOpacity, updatePixmap)
void XYCurve::setValuesOpacity(qreal opacity) {
	Q_D(XYCurve);
	if (opacity != d->valuesOpacity)
		exec(new XYCurveSetValuesOpacityCmd(d, opacity, ki18n("%1: set values opacity")));
}

STD_SETTER_CMD_IMPL_F_S(XYCurve, SetValuesNumericFormat, char, valuesNumericFormat, updateValues)
void XYCurve::setValuesNumericFormat(char format) {
	Q_D(XYCurve);
	if (format != d->valuesNumericFormat)
		exec(new XYCurveSetValuesNumericFormatCmd(d, format, ki18n("%1: set values numeric format")));
}

STD_SETTER_CMD_IMPL_F_S(XYCurve, SetValuesPrecision, int, valuesPrecision, updateValues)
void XYCurve::setValuesPrecision(int precision) {
	Q_D(XYCurve);
	if (precision != d->valuesPrecision)
		exec(new XYCurveSetValuesPrecisionCmd(d, precision, ki18n("%1: set values precision")));
}

STD_SETTER_CMD_IMPL_F_S(XYCurve, SetValuesDateTimeFormat, QString, valuesDateTimeFormat, updateValues)
void XYCurve::setValuesDateTimeFormat(const QString& format) {
	Q_D(XYCurve);
	if (format != d->valuesDateTimeFormat)
		exec(new XYCurveSetValuesDateTimeFormatCmd(d, format, ki18n("%1: set values datetime format")));
}

STD_SETTER_CMD_IMPL_F_S(XYCurve, SetValuesPrefix, QString, valuesPrefix, updateValues)
void XYCurve::setValuesPrefix(const QString& prefix) {
	Q_D(XYCurve);
	if (prefix != d->valuesPrefix)
		exec(new XYCurveSetValuesPrefixCmd(d, prefix, ki18n("%1: set values prefix")));
}

STD_SETTER_CMD_IMPL_F_S(XYCurve, SetValuesSuffix, QString, valuesSuffix, updateValues)
void XYCurve::setValuesSuffix(const QString& suffix) {
	Q_D(XYCurve);
	if (suffix != d->valuesSuffix)
		exec(new XYCurveSetValuesSuffixCmd(d, suffix, ki18n("%1: set values suffix")));
}

STD_SETTER_CMD_IMPL_F_S(XYCurve, SetValuesFont, QFont, valuesFont, updateValues)
void XYCurve::setValuesFont(const QFont& font) {
	Q_D(XYCurve);
	if (font != d->valuesFont)
		exec(new XYCurveSetValuesFontCmd(d, font, ki18n("%1: set values font")));
}

STD_SETTER_CMD_IMPL_F_S(XYCurve, SetValuesColor, QColor, valuesColor, updatePixmap)
void XYCurve::setValuesColor(const QColor& color) {
	Q_D(XYCurve);
	if (color != d->valuesColor)
		exec(new XYCurveSetValuesColorCmd(d, color, ki18n("%1: set values color")));
}

//Filling
STD_SETTER_CMD_IMPL_F_S(XYCurve, SetFillingPosition, XYCurve::FillingPosition, fillingPosition, updateFilling)
void XYCurve::setFillingPosition(FillingPosition position) {
	Q_D(XYCurve);
	if (position != d->fillingPosition)
		exec(new XYCurveSetFillingPositionCmd(d, position, ki18n("%1: filling position changed")));
}

STD_SETTER_CMD_IMPL_F_S(XYCurve, SetFillingType, PlotArea::BackgroundType, fillingType, updatePixmap)
void XYCurve::setFillingType(PlotArea::BackgroundType type) {
	Q_D(XYCurve);
	if (type != d->fillingType)
		exec(new XYCurveSetFillingTypeCmd(d, type, ki18n("%1: filling type changed")));
}

STD_SETTER_CMD_IMPL_F_S(XYCurve, SetFillingColorStyle, PlotArea::BackgroundColorStyle, fillingColorStyle, updatePixmap)
void XYCurve::setFillingColorStyle(PlotArea::BackgroundColorStyle style) {
	Q_D(XYCurve);
	if (style != d->fillingColorStyle)
		exec(new XYCurveSetFillingColorStyleCmd(d, style, ki18n("%1: filling color style changed")));
}

STD_SETTER_CMD_IMPL_F_S(XYCurve, SetFillingImageStyle, PlotArea::BackgroundImageStyle, fillingImageStyle, updatePixmap)
void XYCurve::setFillingImageStyle(PlotArea::BackgroundImageStyle style) {
	Q_D(XYCurve);
	if (style != d->fillingImageStyle)
		exec(new XYCurveSetFillingImageStyleCmd(d, style, ki18n("%1: filling image style changed")));
}

STD_SETTER_CMD_IMPL_F_S(XYCurve, SetFillingBrushStyle, Qt::BrushStyle, fillingBrushStyle, updatePixmap)
void XYCurve::setFillingBrushStyle(Qt::BrushStyle style) {
	Q_D(XYCurve);
	if (style != d->fillingBrushStyle)
		exec(new XYCurveSetFillingBrushStyleCmd(d, style, ki18n("%1: filling brush style changed")));
}

STD_SETTER_CMD_IMPL_F_S(XYCurve, SetFillingFirstColor, QColor, fillingFirstColor, updatePixmap)
void XYCurve::setFillingFirstColor(const QColor& color) {
	Q_D(XYCurve);
	if (color != d->fillingFirstColor)
		exec(new XYCurveSetFillingFirstColorCmd(d, color, ki18n("%1: set filling first color")));
}

STD_SETTER_CMD_IMPL_F_S(XYCurve, SetFillingSecondColor, QColor, fillingSecondColor, updatePixmap)
void XYCurve::setFillingSecondColor(const QColor& color) {
	Q_D(XYCurve);
	if (color != d->fillingSecondColor)
		exec(new XYCurveSetFillingSecondColorCmd(d, color, ki18n("%1: set filling second color")));
}

STD_SETTER_CMD_IMPL_F_S(XYCurve, SetFillingFileName, QString, fillingFileName, updatePixmap)
void XYCurve::setFillingFileName(const QString& fileName) {
	Q_D(XYCurve);
	if (fileName != d->fillingFileName)
		exec(new XYCurveSetFillingFileNameCmd(d, fileName, ki18n("%1: set filling image")));
}

STD_SETTER_CMD_IMPL_F_S(XYCurve, SetFillingOpacity, qreal, fillingOpacity, updatePixmap)
void XYCurve::setFillingOpacity(qreal opacity) {
	Q_D(XYCurve);
	if (opacity != d->fillingOpacity)
		exec(new XYCurveSetFillingOpacityCmd(d, opacity, ki18n("%1: set filling opacity")));
}

//Error bars
STD_SETTER_CMD_IMPL_F_S(XYCurve, SetXErrorType, XYCurve::ErrorType, xErrorType, updateErrorBars)
void XYCurve::setXErrorType(ErrorType type) {
	Q_D(XYCurve);
	if (type != d->xErrorType)
		exec(new XYCurveSetXErrorTypeCmd(d, type, ki18n("%1: x-error type changed")));
}

XYCURVE_COLUMN_SETTER_CMD_IMPL_F_S(XErrorPlus, xErrorPlus, updateErrorBars)
void XYCurve::setXErrorPlusColumn(const AbstractColumn* column) {
	Q_D(XYCurve);
	if (column != d->xErrorPlusColumn) {
		exec(new XYCurveSetXErrorPlusColumnCmd(d, column, ki18n("%1: set x-error column")));
		if (column) {
			connect(column, &AbstractColumn::dataChanged, this, &XYCurve::updateErrorBars);
			//in the macro we connect to recalcLogicalPoints which is not needed for error columns
			disconnect(column, &AbstractColumn::dataChanged, this, &XYCurve::recalcLogicalPoints);
		}
	}
}

void XYCurve::setXErrorPlusColumnPath(const QString& path) {
	Q_D(XYCurve);
	d->xErrorPlusColumnPath = path;
}

XYCURVE_COLUMN_SETTER_CMD_IMPL_F_S(XErrorMinus, xErrorMinus, updateErrorBars)
void XYCurve::setXErrorMinusColumn(const AbstractColumn* column) {
	Q_D(XYCurve);
	if (column != d->xErrorMinusColumn) {
		exec(new XYCurveSetXErrorMinusColumnCmd(d, column, ki18n("%1: set x-error column")));
		if (column) {
			connect(column, &AbstractColumn::dataChanged, this, &XYCurve::updateErrorBars);
			//in the macro we connect to recalcLogicalPoints which is not needed for error columns
			disconnect(column, &AbstractColumn::dataChanged, this, &XYCurve::recalcLogicalPoints);
		}
	}
}

void XYCurve::setXErrorMinusColumnPath(const QString& path) {
	Q_D(XYCurve);
	d->xErrorMinusColumnPath = path;
}

STD_SETTER_CMD_IMPL_F_S(XYCurve, SetYErrorType, XYCurve::ErrorType, yErrorType, updateErrorBars)
void XYCurve::setYErrorType(ErrorType type) {
	Q_D(XYCurve);
	if (type != d->yErrorType)
		exec(new XYCurveSetYErrorTypeCmd(d, type, ki18n("%1: y-error type changed")));
}

XYCURVE_COLUMN_SETTER_CMD_IMPL_F_S(YErrorPlus, yErrorPlus, updateErrorBars)
void XYCurve::setYErrorPlusColumn(const AbstractColumn* column) {
	Q_D(XYCurve);
	if (column != d->yErrorPlusColumn) {
		exec(new XYCurveSetYErrorPlusColumnCmd(d, column, ki18n("%1: set y-error column")));
		if (column) {
			connect(column, SIGNAL(dataChanged(const AbstractColumn*)), this, SLOT(updateErrorBars()));
			//in the macro we connect to recalcLogicalPoints which is not needed for error columns
			disconnect(column, &AbstractColumn::dataChanged, this, &XYCurve::recalcLogicalPoints);
		}
	}
}

void XYCurve::setYErrorPlusColumnPath(const QString& path) {
	Q_D(XYCurve);
	d->yErrorPlusColumnPath = path;
}

XYCURVE_COLUMN_SETTER_CMD_IMPL_F_S(YErrorMinus, yErrorMinus, updateErrorBars)
void XYCurve::setYErrorMinusColumn(const AbstractColumn* column) {
	Q_D(XYCurve);
	if (column != d->yErrorMinusColumn) {
		exec(new XYCurveSetYErrorMinusColumnCmd(d, column, ki18n("%1: set y-error column")));
		if (column) {
			connect(column, &AbstractColumn::dataChanged, this, &XYCurve::updateErrorBars);
			//in the macro we connect to recalcLogicalPoints which is not needed for error columns
			disconnect(column, &AbstractColumn::dataChanged, this, &XYCurve::recalcLogicalPoints);
		}
	}
}

void XYCurve::setYErrorMinusColumnPath(const QString& path) {
	Q_D(XYCurve);
	d->yErrorMinusColumnPath = path;
}

STD_SETTER_CMD_IMPL_F_S(XYCurve, SetErrorBarsCapSize, qreal, errorBarsCapSize, updateErrorBars)
void XYCurve::setErrorBarsCapSize(qreal size) {
	Q_D(XYCurve);
	if (size != d->errorBarsCapSize)
		exec(new XYCurveSetErrorBarsCapSizeCmd(d, size, ki18n("%1: set error bar cap size")));
}

STD_SETTER_CMD_IMPL_F_S(XYCurve, SetErrorBarsType, XYCurve::ErrorBarsType, errorBarsType, updateErrorBars)
void XYCurve::setErrorBarsType(ErrorBarsType type) {
	Q_D(XYCurve);
	if (type != d->errorBarsType)
		exec(new XYCurveSetErrorBarsTypeCmd(d, type, ki18n("%1: error bar type changed")));
}

STD_SETTER_CMD_IMPL_F_S(XYCurve, SetErrorBarsPen, QPen, errorBarsPen, recalcShapeAndBoundingRect)
void XYCurve::setErrorBarsPen(const QPen& pen) {
	Q_D(XYCurve);
	if (pen != d->errorBarsPen)
		exec(new XYCurveSetErrorBarsPenCmd(d, pen, ki18n("%1: set error bar style")));
}

STD_SETTER_CMD_IMPL_F_S(XYCurve, SetErrorBarsOpacity, qreal, errorBarsOpacity, updatePixmap)
void XYCurve::setErrorBarsOpacity(qreal opacity) {
	Q_D(XYCurve);
	if (opacity != d->errorBarsOpacity)
		exec(new XYCurveSetErrorBarsOpacityCmd(d, opacity, ki18n("%1: set error bar opacity")));
}

void XYCurve::suppressRetransform(bool b) {
	Q_D(XYCurve);
	d->suppressRetransform(b);
}

//##############################################################################
//#################################  SLOTS  ####################################
//##############################################################################
void XYCurve::retransform() {
	Q_D(XYCurve);
	d->retransform();
}

void XYCurve::recalcLogicalPoints() {
	Q_D(XYCurve);
	d->recalcLogicalPoints();
}

void XYCurve::updateValues() {
	Q_D(XYCurve);
	d->updateValues();
}

void XYCurve::updateErrorBars() {
	Q_D(XYCurve);
	d->updateErrorBars();
}

//TODO
void XYCurve::handleResize(double horizontalRatio, double verticalRatio, bool pageResize) {
	Q_UNUSED(pageResize);
	Q_D(const XYCurve);

	setSymbolsSize(d->symbolsSize * horizontalRatio);

	QPen pen = d->symbolsPen;
	pen.setWidthF(pen.widthF() * (horizontalRatio + verticalRatio) / 2.0);
	setSymbolsPen(pen);

	pen = d->linePen;
	pen.setWidthF(pen.widthF() * (horizontalRatio + verticalRatio) / 2.0);
	setLinePen(pen);

	//setValuesDistance(d->distance*);
	QFont font = d->valuesFont;
	font.setPointSizeF(font.pointSizeF()*horizontalRatio);
	setValuesFont(font);
}

/*!
 * returns \c true if the aspect being removed \c removedAspect is equal to \c column
 * or to one of its parents. returns \c false otherwise.
 */
bool XYCurve::columnRemoved(const AbstractColumn* column, const AbstractAspect* removedAspect) const {
	// TODO: BAD HACK.
	// In macrosXYCurve.h every parent of the column is connected to the function aspectAboutToBeRemoved().
	// When a column is removed, the function aspectAboutToBeRemoved is called and the column pointer is set to nullptr.
	// However, when a child of the parent is removed, the parent calls the aspectAboutToBeRemoved() again, but
	// the column was already disconnected.
	// Better solution would be to emit aspectAboutToBeRemoved() for every column when their parents are removed.
	// At the moment this signal is only emitted when the column is deleted directly and not when its parent is deleted.
	// Once this is done, the connection of all parents to the aspectAboutToBeRemoved() signal can be removed.
	if (!column)
		return false;

	bool removed = (removedAspect == column);
	if (!removed) {
		auto* parent = column->parentAspect();
		while (parent) {
			if (parent == removedAspect) {
				removed = true;
				break;
			}
			parent = parent->parentAspect();
		}
	}
	return removed;
}

void XYCurve::xColumnAboutToBeRemoved(const AbstractAspect* aspect) {
	Q_D(XYCurve);
	if (columnRemoved(d->xColumn, aspect)) {
		disconnect(aspect, nullptr, this, nullptr);
		d->xColumn = nullptr;
		d->retransform();
	}
}

void XYCurve::yColumnAboutToBeRemoved(const AbstractAspect* aspect) {
	Q_D(XYCurve);
	if (columnRemoved(d->yColumn, aspect)) {
		disconnect(aspect, nullptr, this, nullptr);
		d->yColumn = nullptr;
		d->retransform();
	}
}

void XYCurve::valuesColumnAboutToBeRemoved(const AbstractAspect* aspect) {
	Q_D(XYCurve);
	if (columnRemoved(d->valuesColumn, aspect)) {
		disconnect(aspect, nullptr, this, nullptr);
		d->valuesColumn = nullptr;
		d->updateValues();
	}
}

void XYCurve::xErrorPlusColumnAboutToBeRemoved(const AbstractAspect* aspect) {
	Q_D(XYCurve);
	if (columnRemoved(d->xErrorPlusColumn, aspect)) {
		disconnect(aspect, nullptr, this, nullptr);
		d->xErrorPlusColumn = nullptr;
		d->updateErrorBars();
	}
}

void XYCurve::xErrorMinusColumnAboutToBeRemoved(const AbstractAspect* aspect) {
	Q_D(XYCurve);
	if (columnRemoved(d->xErrorMinusColumn, aspect)) {
		disconnect(aspect, nullptr, this, nullptr);
		d->xErrorMinusColumn = nullptr;
		d->updateErrorBars();
	}
}

void XYCurve::yErrorPlusColumnAboutToBeRemoved(const AbstractAspect* aspect) {
	Q_D(XYCurve);
	if (columnRemoved(d->yErrorPlusColumn, aspect)) {
		disconnect(aspect, nullptr, this, nullptr);
		d->yErrorPlusColumn = nullptr;
		d->updateErrorBars();
	}
}

void XYCurve::yErrorMinusColumnAboutToBeRemoved(const AbstractAspect* aspect) {
	Q_D(XYCurve);
	if (columnRemoved(d->yErrorMinusColumn, aspect)) {
		disconnect(aspect, nullptr, this, nullptr);
		d->yErrorMinusColumn = nullptr;
		d->updateErrorBars();
	}
}

void XYCurve::xColumnNameChanged() {
	Q_D(XYCurve);
	setXColumnPath(d->xColumn->path());
}

void XYCurve::yColumnNameChanged() {
	Q_D(XYCurve);
	setYColumnPath(d->yColumn->path());
}

void XYCurve::xErrorPlusColumnNameChanged() {
	Q_D(XYCurve);
	setXErrorPlusColumnPath(d->xErrorPlusColumn->path());
}

void XYCurve::xErrorMinusColumnNameChanged() {
	Q_D(XYCurve);
	setXErrorMinusColumnPath(d->xErrorMinusColumn->path());
}

void XYCurve::yErrorPlusColumnNameChanged() {
	Q_D(XYCurve);
	setYErrorPlusColumnPath(d->yErrorPlusColumn->path());
}

void XYCurve::yErrorMinusColumnNameChanged() {
	Q_D(XYCurve);
	setYErrorMinusColumnPath(d->yErrorMinusColumn->path());
}

void XYCurve::valuesColumnNameChanged() {
	Q_D(XYCurve);
	setValuesColumnPath(d->valuesColumn->path());
}

//##############################################################################
//######  SLOTs for changes triggered via QActions in the context menu  ########
//##############################################################################
void XYCurve::visibilityChanged() {
	Q_D(const XYCurve);
	this->setVisible(!d->isVisible());
}

void XYCurve::navigateTo() {
	project()->navigateTo(navigateToAction->data().toString());
}

//##############################################################################
//######################### Private implementation #############################
//##############################################################################
XYCurvePrivate::XYCurvePrivate(XYCurve *owner) : q(owner) {
	setFlag(QGraphicsItem::ItemIsSelectable, true);
	setAcceptHoverEvents(false);
}

QString XYCurvePrivate::name() const {
	return q->name();
}

QRectF XYCurvePrivate::boundingRect() const {
	return boundingRectangle;
}

/*!
  Returns the shape of the XYCurve as a QPainterPath in local coordinates
*/
QPainterPath XYCurvePrivate::shape() const {
	return curveShape;
}

void XYCurvePrivate::contextMenuEvent(QGraphicsSceneContextMenuEvent* event) {
	if (q->activateCurve(event->pos())) {
		q->createContextMenu()->exec(event->screenPos());
		return;
	}
	QGraphicsItem::contextMenuEvent(event);
}

bool XYCurvePrivate::swapVisible(bool on) {
	bool oldValue = isVisible();
	setVisible(on);
	emit q->visibilityChanged(on);
	retransform();
	return oldValue;
}

/*!
  called when the size of the plot or its data ranges (manual changes, zooming, etc.) were changed.
  recalculates the position of the scene points to be drawn.
  triggers the update of lines, drop lines, symbols etc.
*/
void XYCurvePrivate::retransform() {

	if (!isVisible())
		return;

	DEBUG("\nXYCurvePrivate::retransform() name = " << STDSTRING(name()) << ", m_suppressRetransform = " << m_suppressRetransform);
	DEBUG("	plot = " << plot);
	if (m_suppressRetransform || !plot)
		return;

	{
#ifdef PERFTRACE_CURVES
	PERFTRACE(name().toLatin1() + ", XYCurvePrivate::retransform()");
#endif

	symbolPointsScene.clear();

	if ( (nullptr == xColumn) || (nullptr == yColumn) ) {
		DEBUG("	xColumn or yColumn == NULL");
		linePath = QPainterPath();
		dropLinePath = QPainterPath();
		symbolsPath = QPainterPath();
		valuesPath = QPainterPath();
		errorBarsPath = QPainterPath();
		curveShape = QPainterPath();
		lines.clear();
		valuesPoints.clear();
		valuesStrings.clear();
		fillPolygons.clear();
		recalcShapeAndBoundingRect();
		return;
	}

	if (!plot->isPanningActive())
		WAIT_CURSOR;

	//calculate the scene coordinates
	// This condition cannot be used, because symbolPointsLogical is also used in updateErrorBars(), updateDropLines() and in updateFilling()
	// TODO: check updateErrorBars() and updateDropLines() and if they aren't available don't calculate this part
	//if (symbolsStyle != Symbol::NoSymbols || valuesType != XYCurve::NoValues ) {
	{
	#ifdef PERFTRACE_CURVES
			PERFTRACE(name().toLatin1() + ", XYCurvePrivate::retransform(), map logical points to scene coordinates");
	#endif

	if (!symbolPointsLogical.isEmpty()) {
		float widthDatarectInch = Worksheet::convertFromSceneUnits(plot->dataRect().width(), Worksheet::Inch);
		float heightDatarectInch = Worksheet::convertFromSceneUnits(plot->dataRect().height(), Worksheet::Inch);
		int countPixelX = ceil(widthDatarectInch*QApplication::desktop()->physicalDpiX());
		int countPixelY = ceil(heightDatarectInch*QApplication::desktop()->physicalDpiY());

		if (countPixelX <= 0 || countPixelY <= 0) {
			RESET_CURSOR;
			return;
		}

		double minLogicalDiffX = 1./(plot->dataRect().width()/countPixelX);
		double minLogicalDiffY = 1./(plot->dataRect().height()/countPixelY);
		QVector<QVector<bool>> scenePointsUsed;
		// size of the datarect in pixels
		scenePointsUsed.resize(countPixelX + 1);
		for (int i = 0; i < countPixelX + 1; i++)
			scenePointsUsed[i].resize(countPixelY + 1);

		auto columnProperties = xColumn->properties();
		int startIndex;
		int endIndex;
		if (columnProperties == AbstractColumn::Properties::MonotonicDecreasing ||
			columnProperties == AbstractColumn::Properties::MonotonicIncreasing) {
			double xMin = cSystem->mapSceneToLogical(plot->dataRect().topLeft()).x();
			double xMax = cSystem->mapSceneToLogical(plot->dataRect().bottomRight()).x();
			startIndex = Column::indexForValue(xMin, symbolPointsLogical, static_cast<AbstractColumn::Properties>(columnProperties));
			endIndex = Column::indexForValue(xMax, symbolPointsLogical, static_cast<AbstractColumn::Properties>(columnProperties));

			if (startIndex > endIndex && startIndex >= 0 && endIndex >= 0)
				std::swap(startIndex, endIndex);

			if (startIndex < 0)
				startIndex = 0;
			if (endIndex < 0)
				endIndex = symbolPointsLogical.size()-1;

		} else {
			startIndex = 0;
			endIndex = symbolPointsLogical.size()-1;
		}

		visiblePoints = std::vector<bool>(symbolPointsLogical.count(), false);
		cSystem->mapLogicalToScene(startIndex, endIndex, symbolPointsLogical,
								   symbolPointsScene, visiblePoints, scenePointsUsed,
								   minLogicalDiffX, minLogicalDiffY);
	}
	}
	//} // (symbolsStyle != Symbol::NoSymbols || valuesType != XYCurve::NoValues )

	m_suppressRecalc = true;
	updateLines();
	updateDropLines();
	updateSymbols();
	updateValues();
	m_suppressRecalc = false;
	updateErrorBars();

	RESET_CURSOR;
	}
}

/*!
 * called if the x- or y-data was changed.
 * copies the valid data points from the x- and y-columns into the internal container
 */
void XYCurvePrivate::recalcLogicalPoints() {
	DEBUG("XYCurvePrivate::recalcLogicalPoints()");
	PERFTRACE(name().toLatin1() + ", XYCurvePrivate::recalcLogicalPoints()");

	symbolPointsLogical.clear();
	connectedPointsLogical.clear();
	validPointsIndicesLogical.clear();
	visiblePoints.clear();

	if (!xColumn || !yColumn)
		return;

	AbstractColumn::ColumnMode xColMode = xColumn->columnMode();
	AbstractColumn::ColumnMode yColMode = yColumn->columnMode();
	QPointF tempPoint;

	//take over only valid and non masked points.
	for (int row = 0; row < xColumn->rowCount(); row++) {
		if ( xColumn->isValid(row) && yColumn->isValid(row)
				&& (!xColumn->isMasked(row)) && (!yColumn->isMasked(row)) ) {
			switch (xColMode) {
			case AbstractColumn::Numeric:
			case AbstractColumn::Integer:
			case AbstractColumn::BigInt:
				tempPoint.setX(xColumn->valueAt(row));
				break;
			case AbstractColumn::Text:
				break;
			case AbstractColumn::DateTime:
				tempPoint.setX(xColumn->dateTimeAt(row).toMSecsSinceEpoch());
				break;
			case AbstractColumn::Month:
			case AbstractColumn::Day:
				break;
			}

			switch (yColMode) {
			case AbstractColumn::Numeric:
			case AbstractColumn::Integer:
			case AbstractColumn::BigInt:
				tempPoint.setY(yColumn->valueAt(row));
				break;
			case AbstractColumn::Text:
				break;
			case AbstractColumn::DateTime:
				tempPoint.setY(yColumn->dateTimeAt(row).toMSecsSinceEpoch());
				break;
			case AbstractColumn::Month:
			case AbstractColumn::Day:
				break;
			}
			symbolPointsLogical.append(tempPoint);
			connectedPointsLogical.push_back(true);
			validPointsIndicesLogical.push_back(row);
		} else {
			if (!connectedPointsLogical.empty())
				connectedPointsLogical[connectedPointsLogical.size()-1] = false;
		}
	}

	visiblePoints = std::vector<bool>(symbolPointsLogical.count(), false);
}

/*!
 * Adds a line, which connects two points, but only if the don't lie on the same xAxis pixel.
 * If they lie on the same x pixel, draw a vertical line between the minimum and maximum y value. So all points are included
 * This function is only valid for linear x Axis scale!
 * @param p0 first point
 * @param p1 second point
 * @param minY
 * @param maxY
 * @param overlap if at the previous call was an overlap between the previous two points
 * @param minLogicalDiffX logical difference between two pixels
 * @param pixelDiff x pixel distance between two points
 */
void XYCurvePrivate::addLine(QPointF p0, QPointF p1, double& minY, double& maxY, bool& overlap, double minLogicalDiffX, int& pixelDiff) {
	pixelDiff = (int)(p1.x() * minLogicalDiffX) - (int)(p0.x() * minLogicalDiffX);

	addLine(p0, p1, minY, maxY, overlap, pixelDiff);
}

/*!
 * Adds a line, which connects two points, but only if they don't lie on the same xAxis pixel.
 * If they lie on the same x pixel, draw a vertical line between the minimum and maximum y value. So all points are included
 * This function can be used for all axis scalings (linear, log, sqrt, ...). For the linear case use the function above, because it's optimized for the linear case
 * @param p0 first point
 * @param p1 second point
 * @param minY
 * @param maxY
 * @param overlap if at the previous call was an overlap between the previous two points
 * @param pixelDiff x pixel distance between two points
 * @param pixelCount pixel count
 */
void XYCurvePrivate::addLine(QPointF p0, QPointF p1, double& minY, double& maxY, bool& overlap, int& pixelDiff, int pixelCount) {

	if (plot->xScale() == CartesianPlot::Scale::ScaleLinear) { // implemented for completeness only
		double minLogicalDiffX = 1./((plot->xMax() - plot->xMin())/pixelCount);
		addLine(p0, p1, minY, maxY, overlap, minLogicalDiffX, pixelDiff);
	} else {
		// for nonlinear scaling the pixel distance must be calculated for every point pair
		QPointF p0Scene = cSystem->mapLogicalToScene(p0, CartesianCoordinateSystem::MappingFlag::SuppressPageClipping);
		QPointF p1Scene = cSystem->mapLogicalToScene(p1, CartesianCoordinateSystem::MappingFlag::SuppressPageClipping);

		// if the point is not valid, don't create a line
		//if (std::isnan(p0Scene.x()) || std::isnan(p0Scene.y()))
		if ((p0Scene.x() == 0 && p0Scene.y() == 0) || (p1Scene.x() == 0 && p1Scene.y() == 0)) { // not possible to create line
			DEBUG("Not possible to create a line between : " << p0Scene.x() << ' ' << p0Scene.y() << ", "<< p1Scene.x() << ' ' << p1Scene.y())
			return;
		}

		// using only the difference between the points is not sufficient, because p0 is updated always
		// independent if new line added or not
		int p0Pixel = (int)((p0Scene.x() - plot->dataRect().x()) / plot->dataRect().width() * pixelCount);
		int p1Pixel = (int)((p1Scene.x() - plot->dataRect().x()) / plot->dataRect().width() * pixelCount);
		pixelDiff = p1Pixel - p0Pixel;
		addLine(p0, p1, minY, maxY, overlap, pixelDiff);
	}
}

/*!
 * \brief XYCurvePrivate::addLine
 * This function is called from the other two addLine() functions to avoid duplication
 * @param p0 first point
 * @param p1 second point
 * @param minY
 * @param maxY
 * @param overlap if at the previous call was an overlap between the previous two points
 * @param pixelDiff x pixel distance between two points
 */
void XYCurvePrivate::addLine(QPointF p0, QPointF p1, double& minY, double& maxY, bool& overlap, int& pixelDiff) {
	//QDEBUG("XYCurvePrivate::addLine():" << p0 << ' ' << p1 << ' ' << minY << ' ' << maxY << ' ' << overlap << ' ' << pixelDiff)
	if (pixelDiff == 0) {
		if (overlap) { // second and so the x axis pixels are the same
		  if (p1.y() > maxY)
			maxY = p1.y();

		  if (p1.y() < minY)
			minY = p1.y();

		} else { // first time pixel are same
			if (p0.y() < p1.y()) {
				minY = p0.y();
				maxY = p1.y();
			} else {
				maxY = p0.y();
				minY = p1.y();
			}
			overlap = true;
		}
	} else {
		if (overlap) { // when previously overlap was true, draw the previous line
			overlap = false;

			// last point from previous pixel must be evaluated
			if (p0.y() > maxY)
			  maxY = p0.y();

			if (p0.y() < minY)
			  minY = p0.y();


			if (true) { //p1.x() >= plot->xMin() && p1.x() <= plot->xMax()) { // x inside scene
				if (minY == maxY) {
					lines.append(QLineF(p0, p1)); // line from previous point to actual point
				} else if (p0.y() == minY) { // draw vertical line
					lines.append(QLineF(p0.x(), maxY, p0.x(), minY));
					if (p1.y() >= minY && p1.y() <= maxY && pixelDiff == 1)
						return;

					lines.append(QLineF(p0, p1));
				} else if (p0.y() == maxY) { // draw vertical line
					lines.append(QLineF(p0.x(), maxY, p0.x(), minY));
					if (p1.y() >= minY && p1.y() <= maxY && pixelDiff == 1)
						return;

					// draw line, only if there is a pixelDiff = 1 otherwise no line needed, because when drawing a new vertical line, this line is already included
					lines.append(QLineF(p0, p1));
				} else { // last point nor min nor max
					lines.append(QLineF(p0.x(), maxY, p0.x(), minY));
					if (p1.y() >= minY && p1.y() <= maxY && pixelDiff == 1)
						return;

					lines.append(QLineF(p0, p1));
				}
			} else // x in scene
				DEBUG("addLine: not in scene");
		} else // no overlap
			lines.append(QLineF(p0, p1));
	}
}

/*!
  recalculates the painter path for the lines connecting the data points.
  Called each time when the type of this connection is changed.
  At the moment also the points which are outside of the scene are added. This algorithm can be improved by letting away all
  lines where both points are outside of the scene
*/
void XYCurvePrivate::updateLines() {
#ifdef PERFTRACE_CURVES
	PERFTRACE(name().toLatin1() + ", XYCurvePrivate::updateLines()");
#endif
	linePath = QPainterPath();
	lines.clear();
	if (lineType == XYCurve::NoLine) {
		DEBUG("	nothing to do, since line type is XYCurve::NoLine");
		updateFilling();
		recalcShapeAndBoundingRect();
		return;
	}

	unsigned int count = (unsigned int)symbolPointsLogical.count();
	if (count <= 1) {
		DEBUG("	nothing to do, since no data points available");
		recalcShapeAndBoundingRect();
		return;
	}

	float widthDatarectInch = Worksheet::convertFromSceneUnits(plot->dataRect().width(), Worksheet::Inch);
	//float heightDatarectInch = Worksheet::convertFromSceneUnits(plot->dataRect().height(), Worksheet::Inch);	// unsed
	int countPixelX = ceil(widthDatarectInch*QApplication::desktop()->physicalDpiX());
	//int countPixelY = ceil(heightDatarectInch*QApplication::desktop()->physicalDpiY());	// unused

	// only valid for linear scale
	//double minLogicalDiffX = 1/((plot->xMax()-plot->xMin())/countPixelX);	// unused
	//double minLogicalDiffY = 1/((plot->yMax()-plot->yMin())/countPixelY); // unused

	//calculate the lines connecting the data points
	{
#ifdef PERFTRACE_CURVES
	PERFTRACE(name().toLatin1() + ", XYCurvePrivate::updateLines(), calculate the lines connecting the data points");
#endif
	QPointF tempPoint1, tempPoint2; // used as temporaryPoints to interpolate datapoints if the corresponding setting is set
	int startIndex, endIndex;

	// find index for xMin and xMax to not loop throug all values
	AbstractColumn::Properties columnProperties = q->xColumn()->properties();
	if (columnProperties == AbstractColumn::Properties::MonotonicDecreasing ||
		columnProperties == AbstractColumn::Properties::MonotonicIncreasing) {
		double xMin = cSystem->mapSceneToLogical(plot->dataRect().topLeft()).x();
		double xMax = cSystem->mapSceneToLogical(plot->dataRect().bottomRight()).x();
		startIndex= Column::indexForValue(xMin, symbolPointsLogical, columnProperties);
		endIndex = Column::indexForValue(xMax, symbolPointsLogical, columnProperties);

		if (startIndex > endIndex)
			std::swap(startIndex, endIndex);

		startIndex--; // use one value before
		endIndex ++;
		if (startIndex < 0)
			startIndex = 0;
		if(endIndex < 0 || endIndex >= static_cast<int>(count))
			endIndex = static_cast<int>(count)-1;

		count = static_cast<unsigned int>(endIndex - startIndex +1);
	}else {
		startIndex = 0;
		endIndex = static_cast<int>(count)-1;
	}

	if (columnProperties == AbstractColumn::Properties::Constant) {
		tempPoint1 = QPointF(plot->xMin(), plot->yMin());
		tempPoint2 = QPointF(plot->xMin(), plot->yMax());
		lines.append(QLineF(tempPoint1, tempPoint2));
	} else {
		bool overlap = false;
		double maxY, minY; // are initialized in add line()
		int pixelDiff;
		QPointF p0;
		QPointF p1;

		switch (lineType) {
		case XYCurve::NoLine:
			break;
		case XYCurve::Line: {
			for (int i = startIndex; i < endIndex; i++) {
				if (!lineSkipGaps && !connectedPointsLogical[i])
					continue;
				p0 = symbolPointsLogical[i];
				p1 = symbolPointsLogical[i+1];
				if (lineIncreasingXOnly && (p1.x() < p0.x())) // when option set skip points
					continue;
				addLine(p0, p1, minY, maxY, overlap, pixelDiff, countPixelX);
			}
			// add last line
			if (overlap)
				lines.append(QLineF(QPointF(p1.x(), minY), QPointF(p1.x(), maxY)));

			break;
		}
		case XYCurve::StartHorizontal: {
			for (int i = startIndex; i < endIndex; i++) {
				if (!lineSkipGaps && !connectedPointsLogical[i])
					continue;
				p0 = symbolPointsLogical[i];
				p1 = symbolPointsLogical[i+1];
				if (lineIncreasingXOnly && (p1.x() < p0.x()))
					continue;

				tempPoint1 = QPointF(p1.x(), p0.y());
				addLine(p0, tempPoint1, minY, maxY, overlap, pixelDiff, countPixelX);
				addLine(tempPoint1, p1, minY, maxY, overlap, pixelDiff, countPixelX);
			}
			// add last line
			if (overlap)
				lines.append(QLineF(QPointF(p1.x(), minY), QPointF(p1.x(), maxY)));

			break;
		}
		case XYCurve::StartVertical: {
			for (int i = startIndex; i < endIndex; i++) {
				if (!lineSkipGaps && !connectedPointsLogical[i])
					continue;
				p0 = symbolPointsLogical[i];
				p1 = symbolPointsLogical[i+1];
				if (lineIncreasingXOnly && (p1.x() < p0.x()))
					continue;
				tempPoint1 = QPointF(p0.x(), p1.y());
				addLine(p0, tempPoint1, minY, maxY, overlap, pixelDiff, countPixelX);
				addLine(tempPoint1, p1, minY, maxY, overlap, pixelDiff, countPixelX);
			}
			// add last line
			if (overlap)
				lines.append(QLineF(QPointF(p1.x(), minY), QPointF(p1.x(), maxY)));

			break;
		}
		case XYCurve::MidpointHorizontal: {
			for (int i = startIndex; i < endIndex; i++) {
				if (!lineSkipGaps && !connectedPointsLogical[i])
					continue;

				p0 = symbolPointsLogical[i];
				p1 = symbolPointsLogical[i+1];
				if (lineIncreasingXOnly && (p1.x() < p0.x()))
					continue;
				tempPoint1 = QPointF(p0.x() + (p1.x()-p0.x())/2, p0.y());
				tempPoint2 = QPointF(p0.x() + (p1.x()-p0.x())/2, p1.y());
				addLine(p0, tempPoint1, minY, maxY, overlap, pixelDiff, countPixelX);
				addLine(tempPoint1, tempPoint2, minY, maxY, overlap, pixelDiff, countPixelX);
				addLine(tempPoint2, p1, minY, maxY, overlap, pixelDiff, countPixelX);
			}
			// add last line
			if (overlap)
				lines.append(QLineF(QPointF(p1.x(), minY), QPointF(p1.x(), maxY)));

			break;
		}
		case XYCurve::MidpointVertical: {
			for (int i = startIndex; i < endIndex; i++) {
				if (!lineSkipGaps && !connectedPointsLogical[i])
					continue;

				p0 = symbolPointsLogical[i];
				p1 = symbolPointsLogical[i+1];
				if (lineIncreasingXOnly && (p1.x() < p0.x()))
					continue;
				tempPoint1 = QPointF(p0.x(), p0.y() + (p1.y()-p0.y())/2);
				tempPoint2 = QPointF(p1.x(), p0.y() + (p1.y()-p0.y())/2);
				addLine(p0, tempPoint1, minY, maxY, overlap, pixelDiff, countPixelX);
				addLine(tempPoint1, tempPoint2, minY, maxY, overlap, pixelDiff, countPixelX);
				addLine(tempPoint2, p1, minY, maxY, overlap, pixelDiff, countPixelX);
			}
			// add last line
			if (overlap)
				lines.append(QLineF(QPointF(p1.x(), minY), QPointF(p1.x(), maxY)));

			break;
		}
		case XYCurve::Segments2: {
			int skip = 0;
			for (int i = startIndex; i < endIndex; i++) {
				p0 = symbolPointsLogical[i];
				p1 = symbolPointsLogical[i+1];
				if (skip != 1) {
					if ( (!lineSkipGaps && !connectedPointsLogical[i])
						|| (lineIncreasingXOnly && (p1.x() < p0.x())) ) {
						skip = 0;
						continue;
					}
					addLine(p0, p1, minY, maxY, overlap, pixelDiff, countPixelX);
					skip++;
				} else {
					skip = 0;
					if (overlap) {
						overlap = false;
						lines.append(QLineF(QPointF(p0.x(), minY), QPointF(p0.x(), maxY)));
					}
				}
			}
			// add last line
			if (overlap)
				lines.append(QLineF(symbolPointsLogical[endIndex-1], symbolPointsLogical[endIndex]));

			break;
		}
		case XYCurve::Segments3: {
			int skip = 0;
			for (int i = startIndex; i < endIndex; i++) {
				if (skip != 2) {
					p0 = symbolPointsLogical[i];
					p1 = symbolPointsLogical[i+1];
					if ( (!lineSkipGaps && !connectedPointsLogical[i])
						|| (lineIncreasingXOnly && (p1.x() < p0.x())) ) {
						skip = 0;
						continue;
					}
					addLine(p0, p1, minY, maxY, overlap, pixelDiff, countPixelX);
					skip++;
				} else {
					skip = 0;
					if (overlap) {
						overlap = false;
						lines.append(QLineF(QPointF(p0.x(), minY), QPointF(p0.x(), maxY)));
					}
				}
			}
			// add last line
			if (overlap)
				lines.append(QLineF(symbolPointsLogical[endIndex-1], symbolPointsLogical[endIndex]));

			break;
		}
		case XYCurve::SplineCubicNatural:
		case XYCurve::SplineCubicPeriodic:
		case XYCurve::SplineAkimaNatural:
		case XYCurve::SplineAkimaPeriodic: {
			gsl_interp_accel *acc = gsl_interp_accel_alloc();
			gsl_spline *spline = nullptr;

			double* x = new double[count];
			double* y = new double[count];
			for (unsigned int i = 0; i < count; i++) { // TODO: interpolating only between the visible points?
				x[i] = symbolPointsLogical[i+startIndex].x();
				y[i] = symbolPointsLogical[i+startIndex].y();
			}

			gsl_set_error_handler_off();
			if (lineType == XYCurve::SplineCubicNatural)
				spline = gsl_spline_alloc(gsl_interp_cspline, count);
			else if (lineType == XYCurve::SplineCubicPeriodic)
				spline = gsl_spline_alloc(gsl_interp_cspline_periodic, count);
			else if (lineType == XYCurve::SplineAkimaNatural)
				spline = gsl_spline_alloc(gsl_interp_akima, count);
			else if (lineType == XYCurve::SplineAkimaPeriodic)
				spline = gsl_spline_alloc(gsl_interp_akima_periodic, count);

			if (!spline) {
				QString msg;
				if ( (lineType == XYCurve::SplineAkimaNatural || lineType == XYCurve::SplineAkimaPeriodic) && count < 5)
					msg = i18n("Error: Akima spline interpolation requires a minimum of 5 points.");
				else
					msg = i18n("Error: Could not initialize the spline function.");
				emit q->info(msg);

				recalcShapeAndBoundingRect();
				delete[] x;
				delete[] y;
				gsl_interp_accel_free (acc);
				return;
			}

			int status = gsl_spline_init (spline, x, y, count);
			if (status) {
				//TODO: check in gsl/interp.c when GSL_EINVAL is thrown
				QString gslError;
				if (status == GSL_EINVAL)
					gslError = i18n("x values must be monotonically increasing.");
				else
					gslError = gslErrorToString(status);
				emit q->info( i18n("Error: %1", gslError) );

				recalcShapeAndBoundingRect();
				delete[] x;
				delete[] y;
				gsl_spline_free (spline);
				gsl_interp_accel_free (acc);
				return;
			}

			//create interpolating points
			std::vector<double> xinterp, yinterp;
			for (unsigned int i = 0; i < count - 1; i++) {
				const double x1 = x[i];
				const double x2 = x[i+1];
				const double step = fabs(x2 - x1)/(lineInterpolationPointsCount + 1);

				for (int i = 0; i < (lineInterpolationPointsCount + 1); i++) {
					double xi = x1+i*step;
					double yi = gsl_spline_eval(spline, xi, acc);
					xinterp.push_back(xi);
					yinterp.push_back(yi);
				}
			}

			if (!xinterp.empty()) {

				for (unsigned int i = 0; i < xinterp.size() - 1; i++) {
					p0 = QPointF(xinterp[i], yinterp[i]);
					p1 = QPointF(xinterp[i+1], yinterp[i+1]);
					addLine(p0, p1, minY, maxY, overlap, pixelDiff, countPixelX);
				}

				addLine(QPointF(xinterp[xinterp.size()-1], yinterp[yinterp.size()-1]), QPointF(x[count-1], y[count-1]), minY, maxY, overlap, pixelDiff, countPixelX);

				// add last line
				if (overlap)
					lines.append(QLineF(QPointF(xinterp[xinterp.size()-1], yinterp[yinterp.size()-1]), QPointF(x[count-1], y[count-1])));
			}

			delete[] x;
			delete[] y;
			gsl_spline_free (spline);
			gsl_interp_accel_free (acc);
			break;
			}
		}
	}
	}

	//map the lines to scene coordinates
	{
#ifdef PERFTRACE_CURVES
		PERFTRACE(name().toLatin1() + ", XYCurvePrivate::updateLines(), map lines to scene coordinates");
#endif
		lines = cSystem->mapLogicalToScene(lines);
	}

	{
#ifdef PERFTRACE_CURVES
	PERFTRACE(name().toLatin1() + ", XYCurvePrivate::updateLines(), calculate new line path");
#endif
	//new line path
	for (const auto& line : lines) {
		linePath.moveTo(line.p1());
		linePath.lineTo(line.p2());
	}
	}

	updateFilling();
	recalcShapeAndBoundingRect();
}

/*!
  recalculates the painter path for the drop lines.
  Called each time when the type of the drop lines is changed.
*/
void XYCurvePrivate::updateDropLines() {
	dropLinePath = QPainterPath();
	if (dropLineType == XYCurve::NoDropLine) {
		recalcShapeAndBoundingRect();
		return;
	}

	//calculate drop lines
	QVector<QLineF> lines;
	float xMin = 0;
	float yMin = 0;

	xMin = plot->xMin();
	yMin = plot->yMin();
	switch (dropLineType) {
	case XYCurve::NoDropLine:
		break;
	case XYCurve::DropLineX:
		for (int i = 0; i < symbolPointsLogical.size(); ++i) {
			if (!visiblePoints[i]) continue;
			const QPointF& point = symbolPointsLogical.at(i);
			lines.append(QLineF(point, QPointF(point.x(), yMin)));
		}
		break;
	case XYCurve::DropLineY:
		for (int i = 0; i < symbolPointsLogical.size(); ++i) {
			if (!visiblePoints[i]) continue;
			const QPointF& point = symbolPointsLogical.at(i);
			lines.append(QLineF(point, QPointF(xMin, point.y())));
		}
		break;
	case XYCurve::DropLineXY:
		for (int i = 0; i < symbolPointsLogical.size(); ++i) {
			if (!visiblePoints[i]) continue;
			const QPointF& point = symbolPointsLogical.at(i);
			lines.append(QLineF(point, QPointF(point.x(), yMin)));
			lines.append(QLineF(point, QPointF(xMin, point.y())));
		}
		break;
	case XYCurve::DropLineXZeroBaseline:
		for (int i = 0; i < symbolPointsLogical.size(); ++i) {
			if (!visiblePoints[i]) continue;
			const QPointF& point = symbolPointsLogical.at(i);
			lines.append(QLineF(point, QPointF(point.x(), 0)));
		}
		break;
	case XYCurve::DropLineXMinBaseline:
		for (int i = 0; i < symbolPointsLogical.size(); ++i) {
			if (!visiblePoints[i]) continue;
			const QPointF& point = symbolPointsLogical.at(i);
			lines.append( QLineF(point, QPointF(point.x(), yColumn->minimum())) );
		}
		break;
	case XYCurve::DropLineXMaxBaseline:
		for (int i = 0; i < symbolPointsLogical.size(); ++i) {
			if (!visiblePoints[i]) continue;
			const QPointF& point = symbolPointsLogical.at(i);
			lines.append( QLineF(point, QPointF(point.x(), yColumn->maximum())) );
		}
		break;
	}

	//map the drop lines to scene coordinates
	lines = cSystem->mapLogicalToScene(lines);

	//new painter path for the drop lines
	for (const auto& line : lines) {
		dropLinePath.moveTo(line.p1());
		dropLinePath.lineTo(line.p2());
	}

	recalcShapeAndBoundingRect();
}

void XYCurvePrivate::updateSymbols() {
#ifdef PERFTRACE_CURVES
	PERFTRACE(name().toLatin1() + ", XYCurvePrivate::updateSymbols()");
#endif
	symbolsPath = QPainterPath();
	if (symbolsStyle != Symbol::NoSymbols) {
		QPainterPath path = Symbol::pathFromStyle(symbolsStyle);

		QTransform trafo;
		trafo.scale(symbolsSize, symbolsSize);
		path = trafo.map(path);
		trafo.reset();

		if (symbolsRotationAngle != 0) {
			trafo.rotate(symbolsRotationAngle);
			path = trafo.map(path);
		}

		for (const auto& point : symbolPointsScene) {
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
void XYCurvePrivate::updateValues() {
#ifdef PERFTRACE_CURVES
	PERFTRACE(name().toLatin1() + ", XYCurvePrivate::updateValues()");
#endif
	valuesPath = QPainterPath();
	valuesPoints.clear();
	valuesStrings.clear();

	if (valuesType == XYCurve::NoValues || symbolPointsLogical.isEmpty()) {
		recalcShapeAndBoundingRect();
		return;
	}

	//determine the value string for all points that are currently visible in the plot
	switch (valuesType) {
	case XYCurve::NoValues:
	case XYCurve::ValuesX: {
		CartesianPlot::RangeFormat rangeFormat = plot->xRangeFormat();
		int precision = valuesPrecision;
		if (xColumn->columnMode() == AbstractColumn::Integer || xColumn->columnMode() == AbstractColumn::BigInt)
			precision = 0;
		for (int i = 0; i < symbolPointsLogical.size(); ++i) {
			if (!visiblePoints[i]) continue;
			QString value;
			if (rangeFormat == CartesianPlot::Numeric)
				value = QString::number(symbolPointsLogical[i].x(), valuesNumericFormat, precision);
			else
				value = QDateTime::fromMSecsSinceEpoch(symbolPointsLogical[i].x()).toString(valuesDateTimeFormat);
			valuesStrings << valuesPrefix + value + valuesSuffix;
		}
		break;
	}
	case XYCurve::ValuesY: {
		CartesianPlot::RangeFormat rangeFormat = plot->yRangeFormat();
		int precision = valuesPrecision;
		if (yColumn->columnMode() == AbstractColumn::Integer || yColumn->columnMode() == AbstractColumn::BigInt)
			precision = 0;
		for (int i = 0; i < symbolPointsLogical.size(); ++i) {
			if (!visiblePoints[i]) continue;
			QString value;
			if (rangeFormat == CartesianPlot::Numeric)
				value = QString::number(symbolPointsLogical[i].y(), valuesNumericFormat, precision);
			else
				value = QDateTime::fromMSecsSinceEpoch(symbolPointsLogical[i].y()).toString(valuesDateTimeFormat);
			valuesStrings << valuesPrefix + value + valuesSuffix;
		}
		break;
	}
	case XYCurve::ValuesXY:
	case XYCurve::ValuesXYBracketed: {
		CartesianPlot::RangeFormat xRangeFormat = plot->xRangeFormat();
		CartesianPlot::RangeFormat yRangeFormat = plot->yRangeFormat();

		int xPrecision = valuesPrecision;
		if (xColumn->columnMode() == AbstractColumn::Integer || xColumn->columnMode() == AbstractColumn::BigInt)
			xPrecision = 0;

		int yPrecision = valuesPrecision;
		if (yColumn->columnMode() == AbstractColumn::Integer || yColumn->columnMode() == AbstractColumn::BigInt)
			yPrecision = 0;

		for (int i = 0; i < symbolPointsLogical.size(); ++i) {
			if (!visiblePoints[i]) continue;
			QString value;
			if (valuesType == XYCurve::ValuesXYBracketed)
				value = '(';
			if (xRangeFormat == CartesianPlot::Numeric)
				value += QString::number(symbolPointsLogical[i].x(), valuesNumericFormat, xPrecision);
			else
				value += QDateTime::fromMSecsSinceEpoch(symbolPointsLogical[i].x()).toString(valuesDateTimeFormat);

			if (yRangeFormat == CartesianPlot::Numeric)
				value += ',' + QString::number(symbolPointsLogical[i].y(), valuesNumericFormat, yPrecision);
			else
				value += ',' + QDateTime::fromMSecsSinceEpoch(symbolPointsLogical[i].y()).toString(valuesDateTimeFormat);

			if (valuesType == XYCurve::ValuesXYBracketed)
				value += ')';

			valuesStrings << valuesPrefix + value + valuesSuffix;
		}
		break;
	}
	case XYCurve::ValuesCustomColumn: {
		if (!valuesColumn) {
			recalcShapeAndBoundingRect();
			return;
		}

		int endRow;
		if (symbolPointsLogical.size()>valuesColumn->rowCount())
			endRow =  valuesColumn->rowCount();
		else
			endRow = symbolPointsLogical.size();

		AbstractColumn::ColumnMode xColMode = valuesColumn->columnMode();
		for (int i = 0; i < endRow; ++i) {
			if (!visiblePoints[i]) continue;

			if ( !valuesColumn->isValid(i) || valuesColumn->isMasked(i) )
				continue;

			switch (xColMode) {
			case AbstractColumn::Numeric:
			case AbstractColumn::Integer:
			case AbstractColumn::BigInt:
				valuesStrings << valuesPrefix + QString::number(valuesColumn->valueAt(i)) + valuesSuffix;
				break;
			case AbstractColumn::Text:
				valuesStrings << valuesPrefix + valuesColumn->textAt(i) + valuesSuffix;
				break;
			case AbstractColumn::DateTime:
			case AbstractColumn::Month:
			case AbstractColumn::Day:
				valuesStrings << valuesPrefix + valuesColumn->dateTimeAt(i).toString(valuesDateTimeFormat) + valuesSuffix;
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
	qreal h = fm.ascent();

	for (int i = 0; i < valuesStrings.size(); i++) {
		w = fm.boundingRect(valuesStrings.at(i)).width();
		switch (valuesPosition) {
		case XYCurve::ValuesAbove:
			tempPoint.setX( symbolPointsScene.at(i).x() - w/2);
			tempPoint.setY( symbolPointsScene.at(i).y() - valuesDistance );
			break;
		case XYCurve::ValuesUnder:
			tempPoint.setX( symbolPointsScene.at(i).x() -w/2 );
			tempPoint.setY( symbolPointsScene.at(i).y() + valuesDistance + h/2);
			break;
		case XYCurve::ValuesLeft:
			tempPoint.setX( symbolPointsScene.at(i).x() - valuesDistance - w - 1 );
			tempPoint.setY( symbolPointsScene.at(i).y());
			break;
		case XYCurve::ValuesRight:
			tempPoint.setX( symbolPointsScene.at(i).x() + valuesDistance - 1 );
			tempPoint.setY( symbolPointsScene.at(i).y() );
			break;
		}
		valuesPoints.append(tempPoint);
	}

	QTransform trafo;
	QPainterPath path;
	for (int i = 0; i < valuesPoints.size(); i++) {
		path = QPainterPath();
		path.addText( QPoint(0,0), valuesFont, valuesStrings.at(i) );

		trafo.reset();
		trafo.translate( valuesPoints.at(i).x(), valuesPoints.at(i).y() );
		if (valuesRotationAngle != 0)
			trafo.rotate( -valuesRotationAngle );

		valuesPath.addPath(trafo.map(path));
	}

	recalcShapeAndBoundingRect();
}

void XYCurvePrivate::updateFilling() {
	if (m_suppressRetransform)
		return;

	fillPolygons.clear();

	//don't try to calculate the filling polygons if
	// - no filling was enabled
	// - the nubmer of visible points on the scene is too high
	// - no scene points available, everything outside of the plot region or no scene points calculated yet
	if (fillingPosition == XYCurve::NoFilling || symbolPointsScene.size() > 1000 || symbolPointsScene.isEmpty()) {
		recalcShapeAndBoundingRect();
		return;
	}

	QVector<QLineF> fillLines;

	//if there're no interpolation lines available (XYCurve::NoLine selected), create line-interpolation,
	//use already available lines otherwise.
	if (!lines.isEmpty())
		fillLines = lines;
	else {
		for (int i = 0; i < symbolPointsLogical.count() - 1; i++) {
			if (!lineSkipGaps && !connectedPointsLogical[i]) continue;
			fillLines.append(QLineF(symbolPointsLogical.at(i), symbolPointsLogical.at(i+1)));
		}

		//no lines available (no points), nothing to do
		if (fillLines.isEmpty())
			return;

		fillLines = cSystem->mapLogicalToScene(fillLines);

		//no lines available (no points) after mapping, nothing to do
		if (fillLines.isEmpty())
			return;
	}

	//create polygon(s):
	//1. Depending on the current zoom-level, only a subset of the curve may be visible in the plot
	//and more of the filling area should be shown than the area defined by the start and end points of the currently visible points.
	//We check first whether the curve crosses the boundaries of the plot and determine new start and end points and put them to the boundaries.
	//2. Furthermore, depending on the current filling type we determine the end point (x- or y-coordinate) where all polygons are closed at the end.
	QPolygonF pol;
	QPointF start = fillLines.at(0).p1(); //starting point of the current polygon, initialize with the first visible point
	QPointF end = fillLines.at(fillLines.size()-1).p2(); //end point of the current polygon, initialize with the last visible point
	const QPointF& first = symbolPointsLogical.at(0); //first point of the curve, may not be visible currently
	const QPointF& last = symbolPointsLogical.at(symbolPointsLogical.size()-1);//last point of the curve, may not be visible currently
	QPointF edge;
	float xEnd = 0, yEnd = 0;
	if (fillingPosition == XYCurve::FillingAbove) {
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
	} else if (fillingPosition == XYCurve::FillingBelow) {
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
	} else if (fillingPosition == XYCurve::FillingZeroBaseline) {
		edge = cSystem->mapLogicalToScene(QPointF(plot->xMin(), plot->yMax()));

		//start point
		if (AbstractCoordinateSystem::essentiallyEqual(start.y(), edge.y())) {
			if (plot->yMax() > 0) {
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
			if (plot->yMax() > 0) {
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
	} else if (fillingPosition == XYCurve::FillingLeft) {
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
	for (int i = 0; i < fillLines.size(); ++i) {
		const QLineF& line = fillLines.at(i);
		p1 = line.p1();
		p2 = line.p2();
		if (i != 0 && p1 != fillLines.at(i-1).p2()) {
			//the first point of the current line is not equal to the last point of the previous line
			//->check whether we have a break in between.
			const bool gap = false; //TODO
			if (!gap) {
				//-> we have no break in the curve -> connect the points by a horizontal/vertical line
				pol << fillLines.at(i-1).p2() << p1;
			} else {
				//-> we have a break in the curve -> close the polygon, add it to the polygon list and start a new polygon
				if (fillingPosition == XYCurve::FillingAbove || fillingPosition == XYCurve::FillingBelow || fillingPosition == XYCurve::FillingZeroBaseline) {
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

	if (p2 != end)
		pol << end;

	//close the last polygon
	if (fillingPosition == XYCurve::FillingAbove || fillingPosition == XYCurve::FillingBelow || fillingPosition == XYCurve::FillingZeroBaseline) {
		pol << QPointF(end.x(), yEnd);
		pol << QPointF(start.x(), yEnd);
	} else {
		pol << QPointF(xEnd, end.y());
		pol << QPointF(xEnd, start.y());
	}

	fillPolygons << pol;
	recalcShapeAndBoundingRect();
}

 /*!
 * Find y value which corresponds to a @p x . @p valueFound indicates, if value was found.
 * When monotonic increasing or decreasing a different algorithm will be used, which needs less steps (mean) (log_2(rowCount)) to find the value.
 * @param x
 * @param valueFound
 * @return
 */
double XYCurve::y(double x, bool &valueFound) const {
	if (!yColumn() || !xColumn()) {
		valueFound = false;
		return NAN;
	}

	AbstractColumn::ColumnMode yColumnMode = yColumn()->columnMode();
	int index = xColumn()->indexForValue(x);
	if (index < 0) {
		valueFound = false;
		return NAN;
	}

	valueFound = true;
	if (yColumnMode == AbstractColumn::ColumnMode::Numeric || yColumnMode == AbstractColumn::ColumnMode::Integer ||
			yColumnMode == AbstractColumn::ColumnMode::BigInt) {
		return yColumn()->valueAt(index);
	} else {
		valueFound = false;
		return NAN;
	}
}

/*!
* Find y DateTime which corresponds to a @p x . @p valueFound indicates, if value was found.
* When monotonic increasing or decreasing a different algorithm will be used, which needs less steps (mean) (log_2(rowCount)) to find the value.
* @param x
* @param valueFound
* @return Return found value
*/
QDateTime XYCurve::yDateTime(double x, bool &valueFound) const {
	if (!yColumn() || !xColumn()) {
		valueFound = false;
		return QDateTime();
	}
   AbstractColumn::ColumnMode yColumnMode = yColumn()->columnMode();
   int index = xColumn()->indexForValue(x);
   if (index < 0) {
	   valueFound = false;
	   return QDateTime();
   }

   valueFound = true;
   if (yColumnMode == AbstractColumn::ColumnMode::Day ||
	   yColumnMode == AbstractColumn::ColumnMode::Month ||
	   yColumnMode == AbstractColumn::ColumnMode::DateTime)
	   return yColumn()->dateTimeAt(index);

   valueFound = false;
   return QDateTime();
}

bool XYCurve::minMaxY(int indexMin, int indexMax, double& yMin, double& yMax, bool includeErrorBars) const {
	return minMax(yColumn(), xColumn(), yErrorType(), yErrorPlusColumn(), yErrorMinusColumn(), indexMin, indexMax, yMin, yMax, includeErrorBars);
}

bool XYCurve::minMaxX(int indexMin, int indexMax, double& xMin, double& xMax, bool includeErrorBars) const {
	return minMax(xColumn(), yColumn(), xErrorType(), xErrorPlusColumn(), xErrorMinusColumn(), indexMin, indexMax, xMin, xMax, includeErrorBars);
}

/*!
 * Calculates the minimum \p min and maximum \p max of a curve with optionally respecting the error bars
 * This function does not check if the values are out of range
 * \p indexMax is not included
 * \p column
 * \p errorType
 * \p errorPlusColumn
 * \p errorMinusColumn
 * \p indexMin
 * \p indexMax
 * \p min
 * \p max
 * \ includeErrorBars If true respect the error bars in the min/max calculation
 */
bool XYCurve::minMax(const AbstractColumn* column1, const AbstractColumn* column2, const ErrorType errorType, const AbstractColumn* errorPlusColumn, const AbstractColumn* errorMinusColumn, int indexMin, int indexMax, double& min, double& max, bool includeErrorBars) const {
	// when property is increasing or decreasing there is a benefit in finding minimum and maximum
	// for property == AbstractColumn::Properties::No it must be iterated over all values so it does not matter if this function or the below one is used
	// if the property of the second column is not AbstractColumn::Properties::No means, that all values are valid and not masked
	if ((!includeErrorBars || errorType == XYCurve::NoError) && column1->properties() != AbstractColumn::Properties::No && column2 && column2->properties() != AbstractColumn::Properties::No) {
		min = column1->minimum(indexMin, indexMax);
		max = column1->maximum(indexMin, indexMax);
		return true;
	}

	if (column1->rowCount() == 0)
		return false;

	min = INFINITY;
	max = -INFINITY;

	for (int i = indexMin; i < indexMax; ++i) {
		if (!column1->isValid(i) || column1->isMasked(i) || (column2 && (!column2->isValid(i) || column2->isMasked(i))))
			continue;

		if ( (errorPlusColumn && i >= errorPlusColumn->rowCount())
			|| (errorMinusColumn && i >= errorMinusColumn->rowCount()) )
			continue;

		//determine the values for the errors
		double errorPlus, errorMinus;
		if (errorPlusColumn && errorPlusColumn->isValid(i) && !errorPlusColumn->isMasked(i))
			if (errorPlusColumn->columnMode() == AbstractColumn::ColumnMode::Numeric ||
					errorPlusColumn->columnMode() == AbstractColumn::ColumnMode::Integer ||
					errorPlusColumn->columnMode() == AbstractColumn::ColumnMode::BigInt)
				errorPlus = errorPlusColumn->valueAt(i);
			else if (errorPlusColumn->columnMode() == AbstractColumn::ColumnMode::DateTime ||
					 errorPlusColumn->columnMode() == AbstractColumn::ColumnMode::Month ||
					 errorPlusColumn->columnMode() == AbstractColumn::ColumnMode::Day)
				errorPlus = errorPlusColumn->dateTimeAt(i).toMSecsSinceEpoch();
			else
				return false;
		else
			errorPlus = 0;

		if (errorType == XYCurve::SymmetricError)
			errorMinus = errorPlus;
		else {
			if (errorMinusColumn && errorMinusColumn->isValid(i) && !errorMinusColumn->isMasked(i))
				if (errorMinusColumn->columnMode() == AbstractColumn::ColumnMode::Numeric ||
					errorMinusColumn->columnMode() == AbstractColumn::ColumnMode::Integer ||
					errorMinusColumn->columnMode() == AbstractColumn::ColumnMode::BigInt)
					errorMinus = errorMinusColumn->valueAt(i);
				else if (errorMinusColumn->columnMode() == AbstractColumn::ColumnMode::DateTime ||
						 errorMinusColumn->columnMode() == AbstractColumn::ColumnMode::Month ||
						 errorMinusColumn->columnMode() == AbstractColumn::ColumnMode::Day)
					errorMinus = errorMinusColumn->dateTimeAt(i).toMSecsSinceEpoch();
				else
					return false;
			else
				errorMinus = 0;
		}

		double value;
		if (column1->columnMode() == AbstractColumn::ColumnMode::Numeric || column1->columnMode() == AbstractColumn::ColumnMode::Integer ||
				column1->columnMode() == AbstractColumn::ColumnMode::BigInt)
			value = column1->valueAt(i);
		else if (column1->columnMode() == AbstractColumn::ColumnMode::DateTime ||
				 column1->columnMode() == AbstractColumn::ColumnMode::Month ||
				 column1->columnMode() == AbstractColumn::ColumnMode::Day) {
			value = column1->dateTimeAt(i).toMSecsSinceEpoch();
		} else
			return false;

		if (value - errorMinus < min)
			min = value - errorMinus;

		if (value + errorPlus > max)
			max = value + errorPlus;
	}
	return true;
}

/*!
 * \brief XYCurve::activateCurve
 * Checks if the mousepos distance to the curve is less than @p pow(maxDist,2)
 * \p mouseScenePos
 * \p maxDist Maximum distance the point lies away from the curve
 * \return Returns true if the distance is smaller than pow(maxDist,2).
 */
bool XYCurve::activateCurve(QPointF mouseScenePos, double maxDist) {
	Q_D(XYCurve);
	return d->activateCurve(mouseScenePos, maxDist);
}

bool XYCurvePrivate::activateCurve(QPointF mouseScenePos, double maxDist) {
	if (!isVisible())
		return false;

	int rowCount = 0;
	if (lineType != XYCurve::LineType::NoLine)
		rowCount = lines.count();
	else if (symbolsStyle != Symbol::Style::NoSymbols)
		rowCount = symbolPointsScene.count();
	else
		return false;

	if (rowCount == 0)
		return false;

	if (maxDist < 0)
		maxDist = (linePen.width() < 10) ? 10 : linePen.width();

	double maxDistSquare = maxDist * maxDist;

	auto properties = q->xColumn()->properties();
	if (properties == AbstractColumn::Properties::No) {
		// assumption: points exist if no line. otherwise previously returned false
		if (lineType == XYCurve::NoLine) {
			QPointF curvePosPrevScene = symbolPointsScene[0];
			QPointF curvePosScene = curvePosPrevScene;
			for (int row =0; row < rowCount; row ++) {
				if (pow(mouseScenePos.x() - curvePosScene.x(), 2) + pow(mouseScenePos.y() - curvePosScene.y(), 2) <= maxDistSquare)
					return true;

				curvePosPrevScene = curvePosScene;
				curvePosScene = symbolPointsScene[row];
			}
		} else {
			for (int row = 0; row < rowCount; row++) {
				QLineF line = lines[row];
				if (pointLiesNearLine(line.p1(), line.p2(), mouseScenePos, maxDist))
					return true;
			}
		}

	} else if (properties == AbstractColumn::Properties::MonotonicIncreasing ||
			properties == AbstractColumn::Properties::MonotonicDecreasing) {

		bool increase = true;
		if (properties == AbstractColumn::Properties::MonotonicDecreasing)
			increase = false;

		double x = mouseScenePos.x()-maxDist;
		int index = 0;

		QPointF curvePosScene;
		QPointF curvePosPrevScene;

		if (lineType == XYCurve::NoLine) {
			curvePosScene  = symbolPointsScene[index];
			curvePosPrevScene = curvePosScene;
			index = Column::indexForValue(x, symbolPointsScene, static_cast<AbstractColumn::Properties>(properties));
		} else
			index = Column::indexForValue(x, lines, static_cast<AbstractColumn::Properties>(properties));

		if (index >= 1)
			index --; // use one before so it is secured that I'm before point.x()
		else if (index == -1)
			return false;

		double xMaxSquare = mouseScenePos.x() + maxDist;
		bool stop = false;
		while (true) {
			// assumption: points exist if no line. otherwise previously returned false
			if (lineType == XYCurve::NoLine) {// check points only if no line otherwise check only the lines
				if (curvePosScene.x() > xMaxSquare)
					stop = true; // one more time if bigger
				if (pow(mouseScenePos.x()- curvePosScene.x(),2)+pow(mouseScenePos.y()-curvePosScene.y(),2) <= maxDistSquare)
					return true;
			} else {
				if (lines[index].p1().x() > xMaxSquare)
					stop = true; // one more time if bigger

				QLineF line = lines[index];
				if (pointLiesNearLine(line.p1(), line.p2(), mouseScenePos, maxDist))
					return true;
			}

			if (stop || (index >= rowCount-1 && increase) || (index <=0 && !increase))
				break;

			if (increase)
				index++;
			else
				index--;

			if (lineType == XYCurve::NoLine) {
				curvePosPrevScene = curvePosScene;
				curvePosScene = symbolPointsScene[index];
			}
		}
	}

	return false;
}

/*!
 * \brief XYCurve::pointLiesNearLine
 * Calculates if a point \p pos lies near than maxDist to the line created by the points \p p1 and \p p2
 * https://stackoverflow.com/questions/11604680/point-laying-near-line
 * \p p1 first point of the line
 * \p p2 second point of the line
 * \p pos Position to check
 * \p maxDist Maximal distance away from the curve, which is valid
 * \return Return true if point lies next to the line
 */
bool XYCurvePrivate::pointLiesNearLine(const QPointF p1, const QPointF p2, const QPointF pos, const double maxDist) const{
	double dx12 = p2.x() - p1.x();
	double dy12 = p2.y() - p1.y();
	double vecLenght = sqrt(pow(dx12,2) + pow(dy12,2));

	if (vecLenght == 0) {
		if (pow(p1.x() - pos.x(), 2) + pow(p1.y()-pos.y(), 2) <= pow(maxDist, 2))
			return true;
		 return false;
	}
	QPointF unitvec(dx12/vecLenght,dy12/vecLenght);

	double dx1m = pos.x() - p1.x();
	double dy1m = pos.y() - p1.y();

	double dist_segm = qAbs(dx1m*unitvec.y() - dy1m*unitvec.x());
	double scalarProduct = dx1m*unitvec.x() + dy1m*unitvec.y();

	if (scalarProduct > 0) {
		if (scalarProduct < vecLenght && dist_segm < maxDist)
			return true;
	}
	return false;
}

// TODO: curvePosScene.x() >= mouseScenePos.x() &&
// curvePosPrevScene.x() < mouseScenePos.x()
// dürfte eigentlich nicht drin sein
bool XYCurvePrivate::pointLiesNearCurve(const QPointF mouseScenePos, const QPointF curvePosPrevScene, const QPointF curvePosScene, const int index, const double maxDist) const {
	if (q->lineType() != XYCurve::LineType::NoLine &&
			curvePosScene.x() >= mouseScenePos.x() &&
			curvePosPrevScene.x() < mouseScenePos.x()) {

		if (q->lineType() == XYCurve::LineType::Line) {
			// point is not in the near of the point, but it can be in the near of the connection line of two points
			if (pointLiesNearLine(curvePosPrevScene,curvePosScene, mouseScenePos, maxDist))
				return true;
		} else if (q->lineType() == XYCurve::LineType::StartHorizontal) {
			QPointF tempPoint = curvePosPrevScene;
			tempPoint.setX(curvePosScene.x());
			if (pointLiesNearLine(curvePosPrevScene,tempPoint, mouseScenePos, maxDist))
				return true;
			if (pointLiesNearLine(tempPoint,curvePosScene, mouseScenePos, maxDist))
				return true;
		} else if (q->lineType() == XYCurve::LineType::StartVertical) {
			QPointF tempPoint = curvePosPrevScene;
			tempPoint.setY(curvePosScene.y());
			if (pointLiesNearLine(curvePosPrevScene,tempPoint, mouseScenePos, maxDist))
				return true;
			if (pointLiesNearLine(tempPoint,curvePosScene, mouseScenePos, maxDist))
				return true;
		} else if (q->lineType() == XYCurve::LineType::MidpointHorizontal) {
			QPointF tempPoint = curvePosPrevScene;
			tempPoint.setX(curvePosPrevScene.x()+(curvePosScene.x()-curvePosPrevScene.x())/2);
			if (pointLiesNearLine(curvePosPrevScene,tempPoint, mouseScenePos, maxDist))
				return true;
			QPointF tempPoint2(tempPoint.x(), curvePosScene.y());
			if (pointLiesNearLine(tempPoint,tempPoint2, mouseScenePos, maxDist))
				return true;

			if (pointLiesNearLine(tempPoint2,curvePosScene, mouseScenePos, maxDist))
				return true;
		} else if (q->lineType() == XYCurve::LineType::MidpointVertical) {
			QPointF tempPoint = curvePosPrevScene;
			tempPoint.setY(curvePosPrevScene.y()+(curvePosScene.y()-curvePosPrevScene.y())/2);
			if (pointLiesNearLine(curvePosPrevScene,tempPoint, mouseScenePos, maxDist))
				return true;
			QPointF tempPoint2(tempPoint.y(), curvePosScene.x());
			if (pointLiesNearLine(tempPoint,tempPoint2, mouseScenePos, maxDist))
				return true;

			if (pointLiesNearLine(tempPoint2,curvePosScene, mouseScenePos, maxDist))
				return true;
		} else if (q->lineType() == XYCurve::LineType::SplineAkimaNatural ||
				   q->lineType() == XYCurve::LineType::SplineCubicNatural ||
				   q->lineType() == XYCurve::LineType::SplineAkimaPeriodic ||
				   q->lineType() == XYCurve::LineType::SplineCubicPeriodic) {
			for (int i=0; i < q->lineInterpolationPointsCount()+1; i++) {
				QLineF line = lines[index*(q->lineInterpolationPointsCount()+1)+i];
				QPointF p1 = line.p1(); //cSystem->mapLogicalToScene(line.p1());
				QPointF p2 = line.p2(); //cSystem->mapLogicalToScene(line.p2());
				if (pointLiesNearLine(p1, p2, mouseScenePos, maxDist))
					return true;
			}
		} else {
			// point is not in the near of the point, but it can be in the near of the connection line of two points
			if (pointLiesNearLine(curvePosPrevScene,curvePosScene, mouseScenePos, maxDist))
				return true;
		}
	}
	return false;
}

/*!
 * \brief XYCurve::setHover
 * Will be called in CartesianPlot::hoverMoveEvent()
 * See d->setHover(on) for more documentation
 * \p on
 */
void XYCurve::setHover(bool on) {
	Q_D(XYCurve);
	d->setHover(on);
}

void XYCurvePrivate::updateErrorBars() {
	errorBarsPath = QPainterPath();
	if (xErrorType == XYCurve::NoError && yErrorType == XYCurve::NoError) {
		recalcShapeAndBoundingRect();
		return;
	}

	QVector<QLineF> lines;
	QVector<QPointF> pointsErrorBarAnchorX;
	QVector<QPointF> pointsErrorBarAnchorY;
	float errorPlus, errorMinus;

	for (int i = 0; i < symbolPointsLogical.size(); ++i) {
		if (!visiblePoints[i])
			continue;

		const QPointF& point = symbolPointsLogical.at(i);
		int index = validPointsIndicesLogical.at(i);

		//error bars for x
		if (xErrorType != XYCurve::NoError) {
			//determine the values for the errors
			if (xErrorPlusColumn && xErrorPlusColumn->isValid(index) && !xErrorPlusColumn->isMasked(index))
				errorPlus = xErrorPlusColumn->valueAt(index);
			else
				errorPlus = 0;

			if (xErrorType == XYCurve::SymmetricError)
				errorMinus = errorPlus;
			else {
				if (xErrorMinusColumn && xErrorMinusColumn->isValid(index) && !xErrorMinusColumn->isMasked(index))
					errorMinus = xErrorMinusColumn->valueAt(index);
				else
					errorMinus = 0;
			}

			//draw the error bars
			if (errorMinus != 0 || errorPlus !=0)
				lines.append(QLineF(QPointF(point.x()-errorMinus, point.y()),
									QPointF(point.x()+errorPlus, point.y())));

			//determine the end points of the errors bars in logical coordinates to draw later the cap
			if (errorBarsType == XYCurve::ErrorBarsWithEnds) {
				pointsErrorBarAnchorX << QPointF(point.x() - errorMinus, point.y());
				pointsErrorBarAnchorX << QPointF(point.x() + errorPlus, point.y());
			}
		}

		//error bars for y
		if (yErrorType != XYCurve::NoError) {
			//determine the values for the errors
			if (yErrorPlusColumn && yErrorPlusColumn->isValid(index) && !yErrorPlusColumn->isMasked(index))
				errorPlus = yErrorPlusColumn->valueAt(index);
			else
				errorPlus = 0;

			if (yErrorType == XYCurve::SymmetricError)
				errorMinus = errorPlus;
			else {
				if (yErrorMinusColumn && yErrorMinusColumn->isValid(index) && !yErrorMinusColumn->isMasked(index) )
					errorMinus = yErrorMinusColumn->valueAt(index);
				else
					errorMinus = 0;
			}

			//draw the error bars
			if (errorMinus != 0 || errorPlus !=0)
				lines.append(QLineF(QPointF(point.x(), point.y() + errorMinus),
									QPointF(point.x(), point.y() - errorPlus)));

			//determine the end points of the errors bars in logical coordinates to draw later the cap
			if (errorBarsType == XYCurve::ErrorBarsWithEnds) {
				pointsErrorBarAnchorY << QPointF(point.x(), point.y() + errorMinus);
				pointsErrorBarAnchorY << QPointF(point.x(), point.y() - errorPlus);
			}
		}
	}

	//map the error bars to scene coordinates
	lines = cSystem->mapLogicalToScene(lines);

	//new painter path for the error bars
	for (const auto& line : lines) {
		errorBarsPath.moveTo(line.p1());
		errorBarsPath.lineTo(line.p2());
	}

	//add caps for x error bars
	if (!pointsErrorBarAnchorX.isEmpty()) {
		pointsErrorBarAnchorX = cSystem->mapLogicalToScene(pointsErrorBarAnchorX);
		for (const auto& point : pointsErrorBarAnchorX) {
			errorBarsPath.moveTo(QPointF(point.x(), point.y() - errorBarsCapSize/2));
			errorBarsPath.lineTo(QPointF(point.x(), point.y() + errorBarsCapSize/2));
		}
	}

	//add caps for y error bars
	if (!pointsErrorBarAnchorY.isEmpty()) {
		pointsErrorBarAnchorY = cSystem->mapLogicalToScene(pointsErrorBarAnchorY);
		for (const auto& point : pointsErrorBarAnchorY) {
			errorBarsPath.moveTo(QPointF(point.x() - errorBarsCapSize/2, point.y()));
			errorBarsPath.lineTo(QPointF(point.x() + errorBarsCapSize/2, point.y()));
		}
	}

	recalcShapeAndBoundingRect();
}

/*!
  recalculates the outer bounds and the shape of the curve.
*/
void XYCurvePrivate::recalcShapeAndBoundingRect() {
	DEBUG("XYCurvePrivate::recalcShapeAndBoundingRect() m_suppressRecalc = " << m_suppressRecalc);
	if (m_suppressRecalc)
		return;

#ifdef PERFTRACE_CURVES
	PERFTRACE(name().toLatin1() + ", XYCurvePrivate::recalcShapeAndBoundingRect()");
#endif

	prepareGeometryChange();
	curveShape = QPainterPath();
	if (lineType != XYCurve::NoLine)
		curveShape.addPath(WorksheetElement::shapeFromPath(linePath, linePen));

	if (dropLineType != XYCurve::NoDropLine)
		curveShape.addPath(WorksheetElement::shapeFromPath(dropLinePath, dropLinePen));

	if (symbolsStyle != Symbol::NoSymbols)
		curveShape.addPath(symbolsPath);

	if (valuesType != XYCurve::NoValues)
		curveShape.addPath(valuesPath);

	if (xErrorType != XYCurve::NoError || yErrorType != XYCurve::NoError)
		curveShape.addPath(WorksheetElement::shapeFromPath(errorBarsPath, errorBarsPen));

	boundingRectangle = curveShape.boundingRect();

	for (const auto& pol : fillPolygons)
		boundingRectangle = boundingRectangle.united(pol.boundingRect());

	//TODO: when the selection is painted, line intersections are visible.
	//simplified() removes those artifacts but is horrible slow for curves with large number of points.
	//search for an alternative.
	//curveShape = curveShape.simplified();

	updatePixmap();
}

void XYCurvePrivate::draw(QPainter* painter) {
#ifdef PERFTRACE_CURVES
	PERFTRACE(name().toLatin1() + ", XYCurvePrivate::draw()");
#endif

	//draw filling
	if (fillingPosition != XYCurve::NoFilling) {
		painter->setOpacity(fillingOpacity);
		painter->setPen(Qt::SolidLine);
		drawFilling(painter);
	}

	//draw lines
	if (lineType != XYCurve::NoLine) {
		painter->setOpacity(lineOpacity);
		painter->setPen(linePen);
		painter->setBrush(Qt::NoBrush);
		painter->drawPath(linePath);
	}

	//draw drop lines
	if (dropLineType != XYCurve::NoDropLine) {
		painter->setOpacity(dropLineOpacity);
		painter->setPen(dropLinePen);
		painter->setBrush(Qt::NoBrush);
		painter->drawPath(dropLinePath);
	}

	//draw error bars
	if ( (xErrorType != XYCurve::NoError) || (yErrorType != XYCurve::NoError) ) {
		painter->setOpacity(errorBarsOpacity);
		painter->setPen(errorBarsPen);
		painter->setBrush(Qt::NoBrush);
		painter->drawPath(errorBarsPath);
	}

	//draw symbols
	if (symbolsStyle != Symbol::NoSymbols) {
		painter->setOpacity(symbolsOpacity);
		painter->setPen(symbolsPen);
		painter->setBrush(symbolsBrush);
		drawSymbols(painter);
	}

	//draw values
	if (valuesType != XYCurve::NoValues) {
		painter->setOpacity(valuesOpacity);
		//don't use any painter pen, since this will force QPainter to render the text outline which is expensive
		painter->setPen(Qt::NoPen);
		painter->setBrush(valuesColor);
		drawValues(painter);
	}
}

void XYCurvePrivate::updatePixmap() {
	DEBUG("XYCurvePrivate::updatePixmap() m_suppressRecalc = " << m_suppressRecalc);
	if (m_suppressRecalc)
		return;

	WAIT_CURSOR;

	m_hoverEffectImageIsDirty = true;
	m_selectionEffectImageIsDirty = true;
	if (boundingRectangle.width() == 0 || boundingRectangle.height() == 0) {
		DEBUG("	boundingRectangle.width() or boundingRectangle.height() == 0");
		m_pixmap = QPixmap();
		RESET_CURSOR;
		return;
	}
	QPixmap pixmap(ceil(boundingRectangle.width()), ceil(boundingRectangle.height()));
	pixmap.fill(Qt::transparent);
	QPainter painter(&pixmap);
	painter.setRenderHint(QPainter::Antialiasing, true);
	painter.translate(-boundingRectangle.topLeft());

	draw(&painter);
	painter.end();
	m_pixmap = pixmap;

	update();
	RESET_CURSOR;
}

/*!
  Reimplementation of QGraphicsItem::paint(). This function does the actual painting of the curve.
  \sa QGraphicsItem::paint().
*/
void XYCurvePrivate::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
	Q_UNUSED(option);
	Q_UNUSED(widget);
	if (!isVisible())
		return;

	painter->setPen(Qt::NoPen);
	painter->setBrush(Qt::NoBrush);
	painter->setRenderHint(QPainter::SmoothPixmapTransform, true);

	if ( KSharedConfig::openConfig()->group("Settings_Worksheet").readEntry<bool>("DoubleBuffering", true) )
		painter->drawPixmap(boundingRectangle.topLeft(), m_pixmap); //draw the cached pixmap (fast)
	else
		draw(painter); //draw directly again (slow)


	if (m_hovered && !isSelected() && !m_printing) {
		if (m_hoverEffectImageIsDirty) {
			QPixmap pix = m_pixmap;
			QPainter p(&pix);
			p.setCompositionMode(QPainter::CompositionMode_SourceIn);	// source (shadow) pixels merged with the alpha channel of the destination (m_pixmap)
			p.fillRect(pix.rect(), QApplication::palette().color(QPalette::Shadow));
			p.end();

			m_hoverEffectImage = ImageTools::blurred(pix.toImage(), m_pixmap.rect(), 5);
			m_hoverEffectImageIsDirty = false;
		}

		painter->drawImage(boundingRectangle.topLeft(), m_hoverEffectImage, m_pixmap.rect());
		return;
	}

	if (isSelected() && !m_printing) {
		if (m_selectionEffectImageIsDirty) {
			QPixmap pix = m_pixmap;
			QPainter p(&pix);
			p.setCompositionMode(QPainter::CompositionMode_SourceIn);
			p.fillRect(pix.rect(), QApplication::palette().color(QPalette::Highlight));
			p.end();

			m_selectionEffectImage = ImageTools::blurred(pix.toImage(), m_pixmap.rect(), 5);
			m_selectionEffectImageIsDirty = false;
		}

		painter->drawImage(boundingRectangle.topLeft(), m_selectionEffectImage, m_pixmap.rect());
	}
}

/*!
	Drawing of symbolsPath is very slow, so we draw every symbol in the loop which is much faster (factor 10)
*/
void XYCurvePrivate::drawSymbols(QPainter* painter) {
	QPainterPath path = Symbol::pathFromStyle(symbolsStyle);

	QTransform trafo;
	trafo.scale(symbolsSize, symbolsSize);
	path = trafo.map(path);
	trafo.reset();
	if (symbolsRotationAngle != 0) {
		trafo.rotate(-symbolsRotationAngle);
		path = trafo.map(path);
	}
	for (const auto& point : symbolPointsScene) {
		trafo.reset();
		trafo.translate(point.x(), point.y());
		painter->drawPath(trafo.map(path));
	}
}

void XYCurvePrivate::drawValues(QPainter* painter) {
	QTransform trafo;
	QPainterPath path;
	for (int i = 0; i < valuesPoints.size(); i++) {
		path = QPainterPath();
		path.addText( QPoint(0,0), valuesFont, valuesStrings.at(i) );

		trafo.reset();
		trafo.translate( valuesPoints.at(i).x(), valuesPoints.at(i).y() );
		if (valuesRotationAngle != 0)
			trafo.rotate( -valuesRotationAngle );

		painter->drawPath(trafo.map(path));
	}
}

void XYCurvePrivate::drawFilling(QPainter* painter) {
	for (const auto& pol : fillPolygons) {
		QRectF rect = pol.boundingRect();
		if (fillingType == PlotArea::Color) {
			switch (fillingColorStyle) {
			case PlotArea::SingleColor: {
					painter->setBrush(QBrush(fillingFirstColor));
					break;
				}
			case PlotArea::HorizontalLinearGradient: {
					QLinearGradient linearGrad(rect.topLeft(), rect.topRight());
					linearGrad.setColorAt(0, fillingFirstColor);
					linearGrad.setColorAt(1, fillingSecondColor);
					painter->setBrush(QBrush(linearGrad));
					break;
				}
			case PlotArea::VerticalLinearGradient: {
					QLinearGradient linearGrad(rect.topLeft(), rect.bottomLeft());
					linearGrad.setColorAt(0, fillingFirstColor);
					linearGrad.setColorAt(1, fillingSecondColor);
					painter->setBrush(QBrush(linearGrad));
					break;
				}
			case PlotArea::TopLeftDiagonalLinearGradient: {
					QLinearGradient linearGrad(rect.topLeft(), rect.bottomRight());
					linearGrad.setColorAt(0, fillingFirstColor);
					linearGrad.setColorAt(1, fillingSecondColor);
					painter->setBrush(QBrush(linearGrad));
					break;
				}
			case PlotArea::BottomLeftDiagonalLinearGradient: {
					QLinearGradient linearGrad(rect.bottomLeft(), rect.topRight());
					linearGrad.setColorAt(0, fillingFirstColor);
					linearGrad.setColorAt(1, fillingSecondColor);
					painter->setBrush(QBrush(linearGrad));
					break;
				}
			case PlotArea::RadialGradient: {
					QRadialGradient radialGrad(rect.center(), rect.width()/2);
					radialGrad.setColorAt(0, fillingFirstColor);
					radialGrad.setColorAt(1, fillingSecondColor);
					painter->setBrush(QBrush(radialGrad));
					break;
				}
			}
		} else if (fillingType == PlotArea::Image) {
			if ( !fillingFileName.trimmed().isEmpty() ) {
				QPixmap pix(fillingFileName);
				switch (fillingImageStyle) {
				case PlotArea::ScaledCropped:
					pix = pix.scaled(rect.size().toSize(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
					painter->setBrush(QBrush(pix));
					painter->setBrushOrigin(pix.size().width()/2, pix.size().height()/2);
					break;
				case PlotArea::Scaled:
					pix = pix.scaled(rect.size().toSize(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
					painter->setBrush(QBrush(pix));
					painter->setBrushOrigin(pix.size().width()/2, pix.size().height()/2);
					break;
				case PlotArea::ScaledAspectRatio:
					pix = pix.scaled(rect.size().toSize(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
					painter->setBrush(QBrush(pix));
					painter->setBrushOrigin(pix.size().width()/2, pix.size().height()/2);
					break;
				case PlotArea::Centered: {
						QPixmap backpix(rect.size().toSize());
						backpix.fill();
						QPainter p(&backpix);
						p.drawPixmap(QPointF(0, 0), pix);
						p.end();
						painter->setBrush(QBrush(backpix));
						painter->setBrushOrigin(-pix.size().width()/2, -pix.size().height()/2);
						break;
					}
				case PlotArea::Tiled:
					painter->setBrush(QBrush(pix));
					break;
				case PlotArea::CenterTiled:
					painter->setBrush(QBrush(pix));
					painter->setBrushOrigin(pix.size().width()/2, pix.size().height()/2);
				}
			}
		} else if (fillingType == PlotArea::Pattern)
			painter->setBrush(QBrush(fillingFirstColor, fillingBrushStyle));

		painter->drawPolygon(pol);
	}
}

void XYCurvePrivate::setPrinting(bool on) {
	m_printing = on;
}

void XYCurvePrivate::suppressRetransform(bool on) {
	m_suppressRetransform = on;
	m_suppressRecalc = on;
}

/*!
 * \brief XYCurvePrivate::mousePressEvent
 * checks with activateCurve, if the mousePress was in the near
 * of the curve. If it was, the curve will be selected
 * \p event
 */
void XYCurvePrivate::mousePressEvent(QGraphicsSceneMouseEvent* event) {
	if (plot->mouseMode() != CartesianPlot::MouseMode::SelectionMode) {
		event->ignore();
		return QGraphicsItem::mousePressEvent(event);
	}

	if(q->activateCurve(event->pos())){
		setSelected(true);
		return;
	}

	event->ignore();
	setSelected(false);
	QGraphicsItem::mousePressEvent(event);
}

/*!
 * \brief XYCurvePrivate::setHover
 * Will be called from CartesianPlot::hoverMoveEvent which
 * determines, which curve is hovered
 * \p on
 */
void XYCurvePrivate::setHover(bool on) {
	if(on == m_hovered)
		return; // don't update if state not changed

	m_hovered = on;
	on ? emit q->hovered() : emit q->unhovered();
	update();
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################
//! Save as XML
void XYCurve::save(QXmlStreamWriter* writer) const {
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
	writer->writeAttribute( "skipGaps", QString::number(d->lineSkipGaps) );
	writer->writeAttribute( "increasingXOnly", QString::number(d->lineIncreasingXOnly) );
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

	writer->writeEndElement(); //close "xyCurve" section
}

//! Load from XML
bool XYCurve::load(XmlStreamReader* reader, bool preview) {
	Q_D(XYCurve);

	if (!readBasicAttributes(reader))
		return false;

	KLocalizedString attributeWarning = ki18n("Attribute '%1' missing or empty, default value is used");
	QXmlStreamAttributes attribs;
	QString str;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == "xyCurve")
			break;

		if (!reader->isStartElement())
			continue;

		if (reader->name() == "comment") {
			if (!readCommentElement(reader)) return false;
		} else if (reader->name() == "general") {
			attribs = reader->attributes();
			READ_COLUMN(xColumn);
			READ_COLUMN(yColumn);

			str = attribs.value("visible").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("visible").toString());
			else
				d->setVisible(str.toInt());
		} else if (!preview && reader->name() == "lines") {
			attribs = reader->attributes();

			READ_INT_VALUE("type", lineType, XYCurve::LineType);
			READ_INT_VALUE("skipGaps", lineSkipGaps, bool);
			READ_INT_VALUE("increasingXOnly", lineIncreasingXOnly, bool);
			READ_INT_VALUE("interpolationPointsCount", lineInterpolationPointsCount, int);
			READ_QPEN(d->linePen);
			READ_DOUBLE_VALUE("opacity", lineOpacity);
		} else if (!preview && reader->name() == "dropLines") {
			attribs = reader->attributes();

			READ_INT_VALUE("type", dropLineType, XYCurve::DropLineType);
			READ_QPEN(d->dropLinePen);
			READ_DOUBLE_VALUE("opacity", dropLineOpacity);

		} else if (!preview && reader->name() == "symbols") {
			attribs = reader->attributes();

			READ_INT_VALUE("symbolsStyle", symbolsStyle, Symbol::Style);
			READ_DOUBLE_VALUE("opacity", symbolsOpacity);
			READ_DOUBLE_VALUE("rotation", symbolsRotationAngle);
			READ_DOUBLE_VALUE("size", symbolsSize);

			READ_QBRUSH(d->symbolsBrush);
			READ_QPEN(d->symbolsPen);
		} else if (!preview && reader->name() == "values") {
			attribs = reader->attributes();

			READ_INT_VALUE("type", valuesType, XYCurve::ValuesType);
			READ_COLUMN(valuesColumn);

			READ_INT_VALUE("position", valuesPosition, XYCurve::ValuesPosition);
			READ_DOUBLE_VALUE("distance", valuesDistance);
			READ_DOUBLE_VALUE("rotation", valuesRotationAngle);
			READ_DOUBLE_VALUE("opacity", valuesOpacity);

			//don't produce any warning if no prefix or suffix is set (empty string is allowed here in xml)
			d->valuesPrefix = attribs.value("prefix").toString();
			d->valuesSuffix = attribs.value("suffix").toString();

			READ_QCOLOR(d->valuesColor);
			READ_QFONT(d->valuesFont);
		} else if (!preview && reader->name() == "filling") {
			attribs = reader->attributes();

			READ_INT_VALUE("position", fillingPosition, XYCurve::FillingPosition);
			READ_INT_VALUE("type", fillingType, PlotArea::BackgroundType);
			READ_INT_VALUE("colorStyle", fillingColorStyle, PlotArea::BackgroundColorStyle);
			READ_INT_VALUE("imageStyle", fillingImageStyle, PlotArea::BackgroundImageStyle );
			READ_INT_VALUE("brushStyle", fillingBrushStyle, Qt::BrushStyle);

			str = attribs.value("firstColor_r").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("firstColor_r").toString());
			else
				d->fillingFirstColor.setRed(str.toInt());

			str = attribs.value("firstColor_g").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("firstColor_g").toString());
			else
				d->fillingFirstColor.setGreen(str.toInt());

			str = attribs.value("firstColor_b").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("firstColor_b").toString());
			else
				d->fillingFirstColor.setBlue(str.toInt());

			str = attribs.value("secondColor_r").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("secondColor_r").toString());
			else
				d->fillingSecondColor.setRed(str.toInt());

			str = attribs.value("secondColor_g").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("secondColor_g").toString());
			else
				d->fillingSecondColor.setGreen(str.toInt());

			str = attribs.value("secondColor_b").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("secondColor_b").toString());
			else
				d->fillingSecondColor.setBlue(str.toInt());

			READ_STRING_VALUE("fileName", fillingFileName);
			READ_DOUBLE_VALUE("opacity", fillingOpacity);
		} else if (!preview && reader->name() == "errorBars") {
			attribs = reader->attributes();

			READ_INT_VALUE("xErrorType", xErrorType, XYCurve::ErrorType);
			READ_COLUMN(xErrorPlusColumn);
			READ_COLUMN(xErrorMinusColumn);

			READ_INT_VALUE("yErrorType", yErrorType, XYCurve::ErrorType);
			READ_COLUMN(yErrorPlusColumn);
			READ_COLUMN(yErrorMinusColumn);

			READ_INT_VALUE("type", errorBarsType, XYCurve::ErrorBarsType);
			READ_DOUBLE_VALUE("capSize", errorBarsCapSize);

			READ_QPEN(d->errorBarsPen);

			READ_DOUBLE_VALUE("opacity", errorBarsOpacity);
		}
	}

	return true;
}

//##############################################################################
//#########################  Theme management ##################################
//##############################################################################
void XYCurve::loadThemeConfig(const KConfig& config) {
	KConfigGroup group = config.group("XYCurve");

	int index = parentAspect()->indexOfChild<XYCurve>(this);
	const auto* plot = dynamic_cast<const CartesianPlot*>(parentAspect());
	QColor themeColor;
	if (index<plot->themeColorPalette().size())
		themeColor = plot->themeColorPalette().at(index);
	else {
		if (plot->themeColorPalette().size())
			themeColor = plot->themeColorPalette().last();
	}

	QPen p;

	Q_D(XYCurve);
	d->m_suppressRecalc = true;

	//Line
	p.setStyle((Qt::PenStyle)group.readEntry("LineStyle", (int)Qt::SolidLine));
	p.setWidthF(group.readEntry("LineWidth", Worksheet::convertToSceneUnits(1.0, Worksheet::Point)));
	p.setColor(themeColor);
	this->setLinePen(p);
	this->setLineOpacity(group.readEntry("LineOpacity", 1.0));

	//Drop line
	p.setStyle((Qt::PenStyle)group.readEntry("DropLineStyle", (int)Qt::SolidLine));
	p.setWidthF(group.readEntry("DropLineWidth", Worksheet::convertToSceneUnits(1.0, Worksheet::Point)));
	p.setColor(themeColor);
	this->setDropLinePen(p);
	this->setDropLineOpacity(group.readEntry("DropLineOpacity", 1.0));

	//Symbol
	this->setSymbolsOpacity(group.readEntry("SymbolOpacity", 1.0));
	QBrush brush;
	brush.setStyle((Qt::BrushStyle)group.readEntry("SymbolFillingStyle", (int)Qt::SolidPattern));
	brush.setColor(themeColor);
	this->setSymbolsBrush(brush);
	p.setStyle((Qt::PenStyle)group.readEntry("SymbolBorderStyle", (int)Qt::SolidLine));
	p.setColor(themeColor);
	p.setWidthF(group.readEntry("SymbolBorderWidth", Worksheet::convertToSceneUnits(0.0, Worksheet::Point)));
	this->setSymbolsPen(p);

	//Values
	this->setValuesOpacity(group.readEntry("ValuesOpacity", 1.0));
	this->setValuesColor(group.readEntry("ValuesColor", QColor(Qt::black)));

	//Filling
	this->setFillingBrushStyle((Qt::BrushStyle)group.readEntry("FillingBrushStyle", (int)Qt::SolidPattern));
	this->setFillingColorStyle((PlotArea::BackgroundColorStyle)group.readEntry("FillingColorStyle", (int)PlotArea::SingleColor));
	this->setFillingOpacity(group.readEntry("FillingOpacity", 1.0));
	this->setFillingPosition((XYCurve::FillingPosition)group.readEntry("FillingPosition", (int)XYCurve::NoFilling));
	this->setFillingFirstColor(themeColor);
	this->setFillingSecondColor(group.readEntry("FillingSecondColor", QColor(Qt::black)));
	this->setFillingType((PlotArea::BackgroundType)group.readEntry("FillingType", (int)PlotArea::Color));

	//Error Bars
	p.setStyle((Qt::PenStyle)group.readEntry("ErrorBarsStyle", (int)Qt::SolidLine));
	p.setWidthF(group.readEntry("ErrorBarsWidth", Worksheet::convertToSceneUnits(1.0, Worksheet::Point)));
	p.setColor(themeColor);
	this->setErrorBarsPen(p);
	this->setErrorBarsOpacity(group.readEntry("ErrorBarsOpacity", 1.0));

	d->m_suppressRecalc = false;
	d->recalcShapeAndBoundingRect();
}

void XYCurve::saveThemeConfig(const KConfig& config) {
	KConfigGroup group = config.group("XYCurve");

	//Drop line
	group.writeEntry("DropLineColor",(QColor) this->dropLinePen().color());
	group.writeEntry("DropLineStyle",(int) this->dropLinePen().style());
	group.writeEntry("DropLineWidth", this->dropLinePen().widthF());
	group.writeEntry("DropLineOpacity",this->dropLineOpacity());

	//Error Bars
	group.writeEntry("ErrorBarsCapSize",this->errorBarsCapSize());
	group.writeEntry("ErrorBarsOpacity",this->errorBarsOpacity());
	group.writeEntry("ErrorBarsColor",(QColor) this->errorBarsPen().color());
	group.writeEntry("ErrorBarsStyle",(int) this->errorBarsPen().style());
	group.writeEntry("ErrorBarsWidth", this->errorBarsPen().widthF());

	//Filling
	group.writeEntry("FillingBrushStyle",(int) this->fillingBrushStyle());
	group.writeEntry("FillingColorStyle",(int) this->fillingColorStyle());
	group.writeEntry("FillingOpacity", this->fillingOpacity());
	group.writeEntry("FillingPosition",(int) this->fillingPosition());
	group.writeEntry("FillingSecondColor",(QColor) this->fillingSecondColor());
	group.writeEntry("FillingType",(int) this->fillingType());

	//Line
	group.writeEntry("LineOpacity", this->lineOpacity());
	group.writeEntry("LineStyle",(int) this->linePen().style());
	group.writeEntry("LineWidth", this->linePen().widthF());

	//Symbol
	group.writeEntry("SymbolOpacity", this->symbolsOpacity());

	//Values
	group.writeEntry("ValuesOpacity", this->valuesOpacity());
	group.writeEntry("ValuesColor", (QColor) this->valuesColor());
	group.writeEntry("ValuesFont", this->valuesFont());

	int index = parentAspect()->indexOfChild<XYCurve>(this);
	if (index < 5) {
		KConfigGroup themeGroup = config.group("Theme");
		for (int i = index; i<5; i++) {
			QString s = "ThemePaletteColor" + QString::number(i+1);
			themeGroup.writeEntry(s,(QColor) this->linePen().color());
		}
	}
}
