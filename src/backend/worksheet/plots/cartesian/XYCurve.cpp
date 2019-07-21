/***************************************************************************
    File                 : XYCurve.cpp
    Project              : LabPlot
    Description          : A xy-curve
    --------------------------------------------------------------------
    Copyright            : (C) 2010-2018 Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2013 Stefan Gerlach (stefan.gerlach@uni.kn)

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
	d->plot = dynamic_cast<const CartesianPlot*>(parentAspect());
	Q_ASSERT(d->plot);
	d->cSystem = dynamic_cast<const CartesianCoordinateSystem*>(d->plot->coordinateSystem());
}

void XYCurve::init() {
	Q_D(XYCurve);

	KConfig config;
	KConfigGroup group = config.group("XYCurve");

	d->xColumn = nullptr;
	d->yColumn = nullptr;

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
	d->valuesColumn = nullptr;
	d->valuesPosition = (XYCurve::ValuesPosition) group.readEntry("ValuesPosition", (int)XYCurve::ValuesAbove);
	d->valuesDistance = group.readEntry("ValuesDistance", Worksheet::convertToSceneUnits(5, Worksheet::Point));
	d->valuesRotationAngle = group.readEntry("ValuesRotation", 0.0);
	d->valuesOpacity = group.readEntry("ValuesOpacity", 1.0);
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
	d->xErrorPlusColumn = nullptr;
	d->xErrorMinusColumn = nullptr;
	d->yErrorType = (XYCurve::ErrorType) group.readEntry("YErrorType", (int)XYCurve::NoError);
	d->yErrorPlusColumn = nullptr;
	d->yErrorMinusColumn = nullptr;
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
	auto* plot = dynamic_cast<CartesianPlot*>(parentAspect());
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
const QString& XYCurve::valuesColumnPath() const {
	return d_ptr->valuesColumnPath;
}
BASIC_SHARED_D_READER_IMPL(XYCurve, XYCurve::ValuesPosition, valuesPosition, valuesPosition)
BASIC_SHARED_D_READER_IMPL(XYCurve, qreal, valuesDistance, valuesDistance)
BASIC_SHARED_D_READER_IMPL(XYCurve, qreal, valuesRotationAngle, valuesRotationAngle)
BASIC_SHARED_D_READER_IMPL(XYCurve, qreal, valuesOpacity, valuesOpacity)
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
const QString& XYCurve::xErrorPlusColumnPath() const {
	return d_ptr->xErrorPlusColumnPath;
}
BASIC_SHARED_D_READER_IMPL(XYCurve, const AbstractColumn*, xErrorMinusColumn, xErrorMinusColumn)
const QString& XYCurve::xErrorMinusColumnPath() const {
	return d_ptr->xErrorMinusColumnPath;
}

BASIC_SHARED_D_READER_IMPL(XYCurve, XYCurve::ErrorType, yErrorType, yErrorType)
BASIC_SHARED_D_READER_IMPL(XYCurve, const AbstractColumn*, yErrorPlusColumn, yErrorPlusColumn)
const QString& XYCurve::yErrorPlusColumnPath() const {
	return d_ptr->yErrorPlusColumnPath;
}
BASIC_SHARED_D_READER_IMPL(XYCurve, const AbstractColumn*, yErrorMinusColumn, yErrorMinusColumn)
const QString& XYCurve::yErrorMinusColumnPath() const {
	return d_ptr->yErrorMinusColumnPath;
}

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
STD_SETTER_CMD_IMPL_F_S(XYCurve, SetXColumn, const AbstractColumn*, xColumn, recalcLogicalPoints)
void XYCurve::setXColumn(const AbstractColumn* column) {
	Q_D(XYCurve);
	if (column != d->xColumn) {
		setXColumnPath(column->path());
		exec(new XYCurveSetXColumnCmd(d, column, ki18n("%1: x-data source changed")));

		//emit xDataChanged() in order to notify the plot about the changes
		emit xDataChanged();
		if (column) {
			//update the curve itself on changes
			connect(column, &AbstractColumn::dataChanged, this, [=](){ d->recalcLogicalPoints(); });
			connect(column->parentAspect(), &AbstractAspect::aspectAboutToBeRemoved,
					this, &XYCurve::xColumnAboutToBeRemoved);
			connect(column, &AbstractAspect::aspectDescriptionChanged, this, &XYCurve::xColumnNameChanged);
			//after the curve was updated, emit the signal to update the plot ranges
			connect(column, SIGNAL(dataChanged(const AbstractColumn*)), this, SIGNAL(xDataChanged()));

			//TODO: add disconnect in the undo-function
		}
	}
}

STD_SETTER_CMD_IMPL_F_S(XYCurve, SetYColumn, const AbstractColumn*, yColumn, recalcLogicalPoints)
void XYCurve::setYColumn(const AbstractColumn* column) {
	Q_D(XYCurve);
	if (column != d->yColumn) {
		setYColumnPath(column->path());
		// disconnect old column
		disconnect(d->yColumn, &AbstractAspect::aspectDescriptionChanged, this, &XYCurve::yColumnNameChanged);
		exec(new XYCurveSetYColumnCmd(d, column, ki18n("%1: y-data source changed")));

		//emit yDataChanged() in order to notify the plot about the changes
		emit yDataChanged();
		if (column) {
			//update the curve itself on changes
			connect(column, &AbstractColumn::dataChanged, this, [=](){ d->recalcLogicalPoints(); });
			connect(column->parentAspect(), &AbstractAspect::aspectAboutToBeRemoved,
					this, &XYCurve::yColumnAboutToBeRemoved);
			connect(column, &AbstractAspect::aspectDescriptionChanged, this, &XYCurve::yColumnNameChanged);

			//after the curve was updated, emit the signal to update the plot ranges
			connect(column, SIGNAL(dataChanged(const AbstractColumn*)), this, SIGNAL(yDataChanged()));

			//TODO: add disconnect in the undo-function
		}
	}
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

STD_SETTER_CMD_IMPL_F_S(XYCurve, SetValuesColumn, const AbstractColumn*, valuesColumn, updateValues)
void XYCurve::setValuesColumn(const AbstractColumn* column) {
	Q_D(XYCurve);
	if (column != d->valuesColumn) {
		exec(new XYCurveSetValuesColumnCmd(d, column, ki18n("%1: set values column")));
		if (column) {
			connect(column, SIGNAL(dataChanged(const AbstractColumn*)), this, SLOT(updateValues()));
			connect(column->parentAspect(), &AbstractAspect::aspectAboutToBeRemoved,
					this, &XYCurve::aspectAboutToBeRemoved);
		}
	}
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

//TODO: Format, Precision

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

STD_SETTER_CMD_IMPL_F_S(XYCurve, SetXErrorPlusColumn, const AbstractColumn*, xErrorPlusColumn, updateErrorBars)
void XYCurve::setXErrorPlusColumn(const AbstractColumn* column) {
	Q_D(XYCurve);
	if (column != d->xErrorPlusColumn) {
		exec(new XYCurveSetXErrorPlusColumnCmd(d, column, ki18n("%1: set x-error column")));
		if (column) {
			connect(column, SIGNAL(dataChanged(const AbstractColumn*)), this, SLOT(updateErrorBars()));
			connect(column->parentAspect(), &AbstractAspect::aspectAboutToBeRemoved,
					this, &XYCurve::xErrorPlusColumnAboutToBeRemoved);
		}
	}
}

STD_SETTER_CMD_IMPL_F_S(XYCurve, SetXErrorMinusColumn, const AbstractColumn*, xErrorMinusColumn, updateErrorBars)
void XYCurve::setXErrorMinusColumn(const AbstractColumn* column) {
	Q_D(XYCurve);
	if (column != d->xErrorMinusColumn) {
		exec(new XYCurveSetXErrorMinusColumnCmd(d, column, ki18n("%1: set x-error column")));
		if (column) {
			connect(column, SIGNAL(dataChanged(const AbstractColumn*)), this, SLOT(updateErrorBars()));
			connect(column->parentAspect(), &AbstractAspect::aspectAboutToBeRemoved,
					this, &XYCurve::xErrorMinusColumnAboutToBeRemoved);
		}
	}
}

STD_SETTER_CMD_IMPL_F_S(XYCurve, SetYErrorType, XYCurve::ErrorType, yErrorType, updateErrorBars)
void XYCurve::setYErrorType(ErrorType type) {
	Q_D(XYCurve);
	if (type != d->yErrorType)
		exec(new XYCurveSetYErrorTypeCmd(d, type, ki18n("%1: y-error type changed")));
}

STD_SETTER_CMD_IMPL_F_S(XYCurve, SetYErrorPlusColumn, const AbstractColumn*, yErrorPlusColumn, updateErrorBars)
void XYCurve::setYErrorPlusColumn(const AbstractColumn* column) {
	Q_D(XYCurve);
	if (column != d->yErrorPlusColumn) {
		exec(new XYCurveSetYErrorPlusColumnCmd(d, column, ki18n("%1: set y-error column")));
		if (column) {
			connect(column, SIGNAL(dataChanged(const AbstractColumn*)), this, SLOT(updateErrorBars()));
			connect(column->parentAspect(), &AbstractAspect::aspectAboutToBeRemoved,
					this, &XYCurve::yErrorPlusColumnAboutToBeRemoved);
		}
	}
}

STD_SETTER_CMD_IMPL_F_S(XYCurve, SetYErrorMinusColumn, const AbstractColumn*, yErrorMinusColumn, updateErrorBars)
void XYCurve::setYErrorMinusColumn(const AbstractColumn* column) {
	Q_D(XYCurve);
	if (column != d->yErrorMinusColumn) {
		exec(new XYCurveSetYErrorMinusColumnCmd(d, column, ki18n("%1: set y-error column")));
		if (column) {
			connect(column, SIGNAL(dataChanged(const AbstractColumn*)), this, SLOT(updateErrorBars()));
			connect(column->parentAspect(), &AbstractAspect::aspectAboutToBeRemoved,
					this, &XYCurve::yErrorMinusColumnAboutToBeRemoved);
		}
	}
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

void XYCurve::xColumnAboutToBeRemoved(const AbstractAspect* aspect) {
	Q_D(XYCurve);
	if (aspect == d->xColumn) {
		d->xColumn = nullptr;
		d->retransform();
	}
}

void XYCurve::yColumnAboutToBeRemoved(const AbstractAspect* aspect) {
	Q_D(XYCurve);
	if (aspect == d->yColumn) {
		d->yColumn = nullptr;
		d->retransform();
	}
}

void XYCurve::valuesColumnAboutToBeRemoved(const AbstractAspect* aspect) {
	Q_D(XYCurve);
	if (aspect == d->valuesColumn) {
		d->valuesColumn = nullptr;
		d->updateValues();
	}
}

void XYCurve::xErrorPlusColumnAboutToBeRemoved(const AbstractAspect* aspect) {
	Q_D(XYCurve);
	if (aspect == d->xErrorPlusColumn) {
		d->xErrorPlusColumn = nullptr;
		d->updateErrorBars();
	}
}

void XYCurve::xErrorMinusColumnAboutToBeRemoved(const AbstractAspect* aspect) {
	Q_D(XYCurve);
	if (aspect == d->xErrorMinusColumn) {
		d->xErrorMinusColumn = nullptr;
		d->updateErrorBars();
	}
}

void XYCurve::yErrorPlusColumnAboutToBeRemoved(const AbstractAspect* aspect) {
	Q_D(XYCurve);
	if (aspect == d->yErrorPlusColumn) {
		d->yErrorPlusColumn = nullptr;
		d->updateErrorBars();
	}
}

void XYCurve::yErrorMinusColumnAboutToBeRemoved(const AbstractAspect* aspect) {
	Q_D(XYCurve);
	if (aspect == d->yErrorMinusColumn) {
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

	DEBUG("\nXYCurvePrivate::retransform() name = " << name().toStdString() << ", m_suppressRetransform = " << m_suppressRetransform);
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

		if (countPixelX <=0 || countPixelY <=0) {
			RESET_CURSOR;
			return;
		}

		double minLogicalDiffX = 1/(plot->dataRect().width()/countPixelX);
		double minLogicalDiffY = 1/(plot->dataRect().height()/countPixelY);
		QVector<QVector<bool>> scenePointsUsed;
		// size of the datarect in pixels
		scenePointsUsed.resize(countPixelX+1);
		for (int i=0; i< countPixelX+1; i++)
			scenePointsUsed[i].resize(countPixelY+1);

		int columnProperties = xColumn->properties();
		int startIndex;
		int endIndex;
		if (columnProperties == AbstractColumn::Properties::MonotonicDecreasing ||
			columnProperties == AbstractColumn::Properties::MonotonicIncreasing) {
			double xMin = cSystem->mapSceneToLogical(plot->dataRect().topLeft()).x();
			double xMax = cSystem->mapSceneToLogical(plot->dataRect().bottomRight()).x();
			startIndex = q->indexForX(xMin, symbolPointsLogical, static_cast<AbstractColumn::Properties>(columnProperties));
			endIndex = q->indexForX(xMax, symbolPointsLogical, static_cast<AbstractColumn::Properties>(columnProperties));

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
 * Adds a line, which connects two points, but only if the don't lie on the same xAxis pixel.
 * If they lie on the same x pixel, draw a vertical line between the minimum and maximum y value. So all points are included
 * This function can be used for all axis scalings (log, sqrt, linear, ...). For the linear case use the above function, because it's optimized for the linear case
 * @param p0 first point
 * @param p1 second point
 * @param minY
 * @param maxY
 * @param overlap if at the previous call was an overlap between the previous two points
 * @param minLogicalDiffX logical difference between two pixels
 * @param pixelDiff x pixel distance between two points
 */
void XYCurvePrivate::addLine(QPointF p0, QPointF p1, double& minY, double& maxY, bool& overlap, int& pixelDiff, int pixelCount) {

	if (plot->xScale() == CartesianPlot::Scale::ScaleLinear) { // implemented for completeness only
		double minLogicalDiffX = 1/((plot->xMax()-plot->xMin())/pixelCount);
		addLine(p0, p1, minY, maxY, overlap, minLogicalDiffX, pixelDiff);
	} else {
		// for nonlinear scaling the pixel distance must be calculated for every point pair
		QPointF p0Scene = cSystem->mapLogicalToScene(p0, CartesianCoordinateSystem::MappingFlag::SuppressPageClipping);
		QPointF p1Scene = cSystem->mapLogicalToScene(p1, CartesianCoordinateSystem::MappingFlag::SuppressPageClipping);

		// if the point is not valid, don't create a line
		//if (std::isnan(p0Scene.x()) || std::isnan(p0Scene.y()))
		if ((p0Scene.x() == 0 && p0Scene.y() == 0) || (p1Scene.x() == 0 && p1Scene.y() == 0)) // no possibility to create line
			return;

		// using only the difference between the points is not sufficient, because p0 is updated always
		// indipendent if new line added or not
		int p0Pixel = (int)((p0Scene.x() - plot->dataRect().x()) / plot->dataRect().width() * pixelCount);
		int p1Pixel = (int)((p1Scene.x() - plot->dataRect().x()) / plot->dataRect().width() * pixelCount);
		pixelDiff = p1Pixel - p0Pixel;
		addLine(p0, p1, minY, maxY, overlap, pixelDiff);
	}
}

/*!
 * \brief XYCurvePrivate::addLine
 * This function is part of the other two addLine() functions to not have two times the same code
 * @param p0 first point
 * @param p1 second point
 * @param minY
 * @param maxY
 * @param overlap if at the previous call was an overlap between the previous two points
 * @param pixelDiff x pixel distance between two points
 */
void XYCurvePrivate::addLine(QPointF p0, QPointF p1, double& minY, double& maxY, bool& overlap, int& pixelDiff) {
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


			if (1) { //p1.x() >= plot->xMin() && p1.x() <= plot->xMax()) { // x inside scene
				if (minY == maxY) {
					lines.append(QLineF(p0, p1)); // line from previous point to actual point
				} else if (p0.y() == minY) { // draw vertical line
					lines.append(QLineF(p0.x(),maxY, p0.x(), minY));
					if (p1.y() >= minY && p1.y() <= maxY && pixelDiff == 1)
						return;

					lines.append(QLineF(p0,p1));
				} else if (p0.y() == maxY) { // draw vertical line
					lines.append(QLineF(p0.x(),maxY, p0.x(), minY));
					if (p1.y() >= minY && p1.y() <= maxY && pixelDiff == 1)
						return;

					// draw line, only if there is a pixelDiff = 1 otherwise no line needed, because when drawing a new vertical line, this line is already included
					lines.append(QLineF(p0,p1));
				} else { // last point nor min nor max
					lines.append(QLineF(p0.x(),maxY, p0.x(), minY));
					if (p1.y() >= minY && p1.y() <= maxY && pixelDiff == 1)
						return;

					lines.append(QLineF(p0,p1));
				}
			} else// x in scene
				DEBUG("addLine: not in scene");
		} else// no overlap
			lines.append(QLineF(p0,p1));
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
	bool overlap = false;
	double maxY, minY; // are initialized in add line()
	int pixelDiff;
	QPointF p0;
	QPointF p1;

	// find index for xMin and xMax to not loop throug all values
	AbstractColumn::Properties columnProperties = q->xColumn()->properties();
	if (columnProperties == AbstractColumn::Properties::MonotonicDecreasing ||
		columnProperties == AbstractColumn::Properties::MonotonicIncreasing) {
		double xMin = cSystem->mapSceneToLogical(plot->dataRect().topLeft()).x();
		double xMax = cSystem->mapSceneToLogical(plot->dataRect().bottomRight()).x();
		startIndex= q->indexForX(xMin, symbolPointsLogical, columnProperties);
		endIndex = q->indexForX(xMax, symbolPointsLogical, columnProperties);

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
			if (overlap) {
				overlap = false;
				lines.append(QLineF(QPointF(p1.x(), minY), QPointF(p1.x(), maxY)));
			}
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
			if (overlap) {
				overlap = false;
				lines.append(QLineF(QPointF(p1.x(), minY), QPointF(p1.x(), maxY)));
			}
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
			if (overlap) {
				overlap = false;
				lines.append(QLineF(QPointF(p1.x(), minY), QPointF(p1.x(), maxY)));
			}
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
			if (overlap) {
				overlap = false;
				lines.append(QLineF(QPointF(p1.x(), minY), QPointF(p1.x(), maxY)));
			}
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
			if (overlap) {
				overlap = false;
				lines.append(QLineF(QPointF(p1.x(), minY), QPointF(p1.x(), maxY)));
			}
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
			if (overlap) {
				overlap = false;
				lines.append(QLineF(symbolPointsLogical[endIndex-1], symbolPointsLogical[endIndex]));
			}
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
			if (overlap) {
				overlap = false;
				lines.append(QLineF(symbolPointsLogical[endIndex-1], symbolPointsLogical[endIndex]));
			}
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
				double xi, yi;
				const double step = fabs(x2 - x1)/(lineInterpolationPointsCount + 1);

				for (int i=0; i < (lineInterpolationPointsCount + 1); i++) {
					xi = x1+i*step;
					yi = gsl_spline_eval(spline, xi, acc);
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
				if (overlap) {
					overlap = false;
					lines.append(QLineF(QPointF(xinterp[xinterp.size()-1], yinterp[yinterp.size()-1]), QPointF(x[count-1], y[count-1])));
				}
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
			lines.append( QLineF(point, QPointF(point.x(), dynamic_cast<const Column*>(yColumn)->minimum())) );
		}
		break;
	case XYCurve::DropLineXMaxBaseline:
		for (int i = 0; i < symbolPointsLogical.size(); ++i) {
			if (!visiblePoints[i]) continue;
			const QPointF& point = symbolPointsLogical.at(i);
			lines.append( QLineF(point, QPointF(point.x(), dynamic_cast<const Column*>(yColumn)->maximum())) );
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

	if (valuesType == XYCurve::NoValues) {
		recalcShapeAndBoundingRect();
		return;
	}

	//determine the value string for all points that are currently visible in the plot
	switch (valuesType) {
	case XYCurve::NoValues:
	case XYCurve::ValuesX: {
			for (int i = 0; i < symbolPointsScene.size(); ++i) {
				if (!visiblePoints[i]) continue;
				valuesStrings << valuesPrefix + QString::number(cSystem->mapSceneToLogical(symbolPointsScene[i]).x()) + valuesSuffix;
			}
			break;
		}
	case XYCurve::ValuesY: {
			for (int i = 0; i < symbolPointsScene.size(); ++i) {
				if (!visiblePoints[i]) continue;
				valuesStrings << valuesPrefix + QString::number(cSystem->mapSceneToLogical(symbolPointsScene[i]).y()) + valuesSuffix;
			}
			break;
		}
	case XYCurve::ValuesXY: {
			for (int i = 0; i < symbolPointsScene.size(); ++i) {
				if (!visiblePoints[i]) continue;
				QPointF logicalValue = cSystem->mapSceneToLogical(symbolPointsScene[i]);
				valuesStrings << valuesPrefix + QString::number(logicalValue.x()) + ','
							  + QString::number(logicalValue.y()) + valuesSuffix;
			}
			break;
		}
	case XYCurve::ValuesXYBracketed: {
			for (int i = 0; i < symbolPointsScene.size(); ++i) {
				if (!visiblePoints[i]) continue;
				QPointF logicalValue = cSystem->mapSceneToLogical(symbolPointsScene[i]);
				valuesStrings <<  valuesPrefix + '(' + QString::number(logicalValue.x()) + ','
							  + QString::number(logicalValue.y()) +')' + valuesSuffix;
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
	qreal h = fm.ascent();

	for (int i = 0; i < valuesStrings.size(); i++) {
		w = fm.width(valuesStrings.at(i));
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
	if (fillingPosition == XYCurve::NoFilling || symbolPointsScene.size()>1000 || symbolPointsScene.isEmpty()) {
		recalcShapeAndBoundingRect();
		return;
	}

	QVector<QLineF> fillLines;

	//if there're no interpolation lines available (XYCurve::NoLine selected), create line-interpolation,
	//use already available lines otherwise.
	if (!lines.isEmpty())
		fillLines = lines;
	else {
		for (int i = 0; i < symbolPointsLogical.count()-1; i++) {
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
 * calculates log2(x)+1 for an integer value.
 * Used in y(double x) to calculate the maximum steps
 * source: https://stackoverflow.com/questions/11376288/fast-computing-of-log2-for-64-bit-integers
 * source: http://graphics.stanford.edu/~seander/bithacks.html#IntegerLogLookup
 * @param value
 * @return returns calculated value
 */
// TODO: testing if it is faster than calculating log2.
int XYCurve::calculateMaxSteps (unsigned int value) {
	const signed char LogTable256[256] = {
		-1,0,1,1,2,2,2,2,3,3,3,3,3,3,3,3,
		4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
		5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
		5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
		6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
		6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
		6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
		6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
		7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
		7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
		7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
		7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
		7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
		7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
		7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
		7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7
	};

	unsigned int r;     // r will be lg(v)
	unsigned int t, tt; // temporaries
	if ((tt = value >> 16))
		r = (t = tt >> 8) ? 24 + LogTable256[t] : 16 + LogTable256[tt];
	else
		r = (t = value >> 8) ? 8 + LogTable256[t] : LogTable256[value];

	return r+1;
}

 /*!
 * Find y value which corresponds to a @p x . @p valueFound indicates, if value was found.
 * When monotonic increasing or decreasing a different algorithm will be used, which needs less steps (mean) (log_2(rowCount)) to find the value.
 * @param x
 * @param valueFound
 * @return
 */
double XYCurve::y(double x, bool &valueFound) const {

    AbstractColumn::ColumnMode yColumnMode = yColumn()->columnMode();
	int index = indexForX(x);
	if (index < 0) {
		valueFound = false;
		return NAN;
	}

	valueFound = true;
	if (yColumnMode == AbstractColumn::ColumnMode::Numeric ||
		yColumnMode == AbstractColumn::ColumnMode::Integer) {
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

   AbstractColumn::ColumnMode yColumnMode = yColumn()->columnMode();
   int index = indexForX(x);
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

/*!
* Find index which corresponds to a @p x .
* When monotonic increasing or decreasing a different algorithm will be used, which needs less steps (mean) (log_2(rowCount)) to find the value.
* @param x
* @return -1 if index not found, otherwise the index
*/
int XYCurve::indexForX(double x) const {
	int rowCount = xColumn()->rowCount();

	double prevValue = 0;
	qint64 prevValueDateTime = 0;
	AbstractColumn::ColumnMode xColumnMode = xColumn()->columnMode();
	int properties = xColumn()->properties();
	if (properties == AbstractColumn::Properties::MonotonicIncreasing ||
			properties == AbstractColumn::Properties::MonotonicDecreasing) {
		// bisects the index every time, so it is possible to find the value in log_2(rowCount) steps
		bool increase = (properties != AbstractColumn::Properties::MonotonicDecreasing);

		int lowerIndex = 0;
		int higherIndex = rowCount - 1;

		unsigned int maxSteps = calculateMaxSteps(static_cast<unsigned int>(rowCount))+1;

		if ((xColumnMode == AbstractColumn::ColumnMode::Numeric ||
			 xColumnMode == AbstractColumn::ColumnMode::Integer)) {
			for (unsigned int i = 0; i < maxSteps; i++) { // so no log_2(rowCount) needed
				int index = lowerIndex + round(static_cast<double>(higherIndex - lowerIndex)/2);
				double value = xColumn()->valueAt(index);

				if (higherIndex - lowerIndex < 2) {
					if (abs(xColumn()->valueAt(lowerIndex) - x) < abs(xColumn()->valueAt(higherIndex) - x))
						index = lowerIndex;
					else
						index = higherIndex;

					return index;
				}

				if (value >= x && increase)
					higherIndex = index;
				else if (value >= x && !increase)
					lowerIndex = index;
				else if (value < x && increase)
					lowerIndex = index;
				else if (value < x && !increase)
					higherIndex = index;

			}
		} else if ((xColumnMode == AbstractColumn::ColumnMode::DateTime ||
					xColumnMode == AbstractColumn::ColumnMode::Month ||
					xColumnMode == AbstractColumn::ColumnMode::Day)) {
			qint64 xInt64 = static_cast<qint64>(x);
			for (unsigned int i = 0; i < maxSteps; i++) { // so no log_2(rowCount) needed
				int index = lowerIndex + round(static_cast<double>(higherIndex - lowerIndex)/2);
				qint64 value = xColumn()->dateTimeAt(index).toMSecsSinceEpoch();

				if (higherIndex - lowerIndex < 2) {
					if (abs(xColumn()->dateTimeAt(lowerIndex).toMSecsSinceEpoch() - xInt64) < abs(xColumn()->dateTimeAt(higherIndex).toMSecsSinceEpoch() - xInt64))
						index = lowerIndex;
					else
						index = higherIndex;

					return index;
				}

				if (value > xInt64 && increase)
					higherIndex = index;
				else if (value > xInt64 && !increase)
					lowerIndex = index;
				else if (value < xInt64 && increase)
					lowerIndex = index;
				else if (value < xInt64 && !increase)
					higherIndex = index;

			}
		}

	} else if (properties == AbstractColumn::Properties::Constant) {
		if (rowCount > 0)
			return 0;
		else
			return -1;
	} else {
		// naiv way
		if ((xColumnMode == AbstractColumn::ColumnMode::Numeric ||
			 xColumnMode == AbstractColumn::ColumnMode::Integer)) {
			for (int row = 0; row < rowCount; row++) {
				if (xColumn()->isValid(row)) {
					if (row == 0)
						prevValue = xColumn()->valueAt(row);

					double value = xColumn()->valueAt(row);
					if (abs(value - x) <= abs(prevValue - x)) { // <= prevents also that row - 1 become < 0
						if (row < rowCount - 1)
							prevValue = value;
						else {
							return row;
						}
					}else{
						return row-1;
					}
				}
			}
		} else if ((xColumnMode == AbstractColumn::ColumnMode::DateTime ||
					xColumnMode == AbstractColumn::ColumnMode::Month ||
					xColumnMode == AbstractColumn::ColumnMode::Day)) {
			qint64 xInt64 = static_cast<qint64>(x);
			for (int row = 0; row < rowCount; row++) {
				if (xColumn()->isValid(row)) {
					if (row == 0)
						prevValueDateTime = xColumn()->dateTimeAt(row).toMSecsSinceEpoch();

					qint64 value = xColumn()->dateTimeAt(row).toMSecsSinceEpoch();
					if (abs(value - xInt64) <= abs(prevValueDateTime - xInt64)) { // "<=" prevents also that row - 1 become < 0
						if (row < rowCount - 1)
							prevValueDateTime = value;
						else
							return row;
					} else {
						return row - 1;
					}
				}
			}

		}
	}
	return -1;
}

/*!
* Find index which corresponds to a @p x . In a vector of values
* When monotonic increasing or decreasing a different algorithm will be used, which needs less steps (mean) (log_2(rowCount)) to find the value.
* @param x
* @return -1 if index not found, otherwise the index
*/
int XYCurve::indexForX(double x, QVector<double>& column, AbstractColumn::Properties properties) const {
	int rowCount = column.count();
	if (rowCount == 0)
		return -1;

	double prevValue = 0;
	//qint64 prevValueDateTime = 0;
	if (properties == AbstractColumn::Properties::MonotonicIncreasing ||
			properties == AbstractColumn::Properties::MonotonicDecreasing) {
		// bisects the index every time, so it is possible to find the value in log_2(rowCount) steps
		bool increase = true;
		if(properties == AbstractColumn::Properties::MonotonicDecreasing)
			increase = false;

		int lowerIndex = 0;
		int higherIndex = rowCount-1;

		unsigned int maxSteps = calculateMaxSteps(static_cast<unsigned int>(rowCount))+1;

		for (unsigned int i = 0; i < maxSteps; i++) { // so no log_2(rowCount) needed
			int index = lowerIndex + round(static_cast<double>(higherIndex - lowerIndex)/2);
			double value = column[index];

			if (higherIndex - lowerIndex < 2) {
				if (abs(column[lowerIndex] - x) < abs(column[higherIndex] - x))
					index = lowerIndex;
				else
					index = higherIndex;

				return index;
			}

			if (value >= x && increase)
				higherIndex = index;
			else if (value >= x && !increase)
				lowerIndex = index;
			else if (value < x && increase)
				lowerIndex = index;
			else if (value < x && !increase)
				higherIndex = index;

		}

	} else if (properties == AbstractColumn::Properties::Constant) {
		return 0;
	} else {
		// AbstractColumn::Properties::No
		// naiv way
		prevValue = column[0];
		for (int row = 0; row < rowCount; row++) {

			double value = column[row];
			if (abs(value - x) <= abs(prevValue - x)) { // "<=" prevents also that row - 1 become < 0
				if (row < rowCount - 1)
					prevValue = value;
				else
					return row;
			} else {
				return row - 1;
			}
		}
	}
	return -1;
}

/*!
* Find index which corresponds to a @p x . In a vector of values
* When monotonic increasing or decreasing a different algorithm will be used, which needs less steps (mean) (log_2(rowCount)) to find the value.
* @param x
* @return -1 if index not found, otherwise the index
*/
int XYCurve::indexForX(const double x, const QVector<QPointF>& points, AbstractColumn::Properties properties) const {
	int rowCount = points.count();

	if (rowCount == 0)
		return -1;

	double prevValue = 0;
	//qint64 prevValueDateTime = 0;
	if (properties == AbstractColumn::Properties::MonotonicIncreasing ||
			properties == AbstractColumn::Properties::MonotonicDecreasing) {
		// bisects the index every time, so it is possible to find the value in log_2(rowCount) steps
		bool increase = true;
		if(properties == AbstractColumn::Properties::MonotonicDecreasing)
			increase = false;

		int lowerIndex = 0;
		int higherIndex = rowCount - 1;

		unsigned int maxSteps = calculateMaxSteps(static_cast<unsigned int>(rowCount))+1;

		for (unsigned int i = 0; i < maxSteps; i++) { // so no log_2(rowCount) needed
			int index = lowerIndex + round(static_cast<double>(higherIndex - lowerIndex)/2);
			double value = points[index].x();

			if (higherIndex - lowerIndex < 2) {
				if (abs(points[lowerIndex].x() - x) < abs(points[higherIndex].x() - x))
					index = lowerIndex;
				else
					index = higherIndex;

				return index;
			}

			if (value >= x && increase)
				higherIndex = index;
			else if (value >= x && !increase)
				lowerIndex = index;
			else if (value < x && increase)
				lowerIndex = index;
			else if (value < x && !increase)
				higherIndex = index;

		}

	} else if (properties == AbstractColumn::Properties::Constant) {
		return 0;
	} else {
		// AbstractColumn::Properties::No
		// naiv way
		prevValue = points[0].x();
		for (int row = 0; row < rowCount; row++) {

			double value = points[row].x();
			if (abs(value - x) <= abs(prevValue - x)) { // "<=" prevents also that row - 1 become < 0
				if (row < rowCount - 1)
					prevValue = value;
				else
					return row;
			} else {
				return row - 1;
			}
		}
	}
	return -1;
}

/*!
* Find index which corresponds to a @p x . In a vector of values
* When monotonic increasing or decreasing a different algorithm will be used, which needs less steps (mean) (log_2(rowCount)) to find the value.
* @param x
* @return -1 if index not found, otherwise the index
*/
int XYCurve::indexForX(double x, QVector<QLineF>& lines, AbstractColumn::Properties properties) const {
	int rowCount = lines.count();
	if (rowCount == 0)
		return -1;
	// use only p1 to find index
	double prevValue = 0;
	//qint64 prevValueDateTime = 0;
	if (properties == AbstractColumn::Properties::MonotonicIncreasing ||
			properties == AbstractColumn::Properties::MonotonicDecreasing) {
		// bisects the index every time, so it is possible to find the value in log_2(rowCount) steps
		bool increase = true;
		if(properties == AbstractColumn::Properties::MonotonicDecreasing)
			increase = false;

		int lowerIndex = 0;
		int higherIndex = rowCount-1;

		unsigned int maxSteps = calculateMaxSteps(static_cast<unsigned int>(rowCount))+1;

		for (unsigned int i = 0; i < maxSteps; i++) { // so no log_2(rowCount) needed
			int index = lowerIndex + round(static_cast<double>(higherIndex - lowerIndex)/2);
			double value = lines[index].p1().x();

			if (higherIndex - lowerIndex < 2) {
				if (abs(lines[lowerIndex].p1().x() - x) < abs(lines[higherIndex].p1().x() - x))
					index = lowerIndex;
				else
					index = higherIndex;

				return index;
			}

			if (value >= x && increase)
				higherIndex = index;
			else if (value >= x && !increase)
				lowerIndex = index;
			else if (value < x && increase)
				lowerIndex = index;
			else if (value < x && !increase)
				higherIndex = index;

		}

	} else if (properties == AbstractColumn::Properties::Constant) {
		return 0;
	} else {
		// AbstractColumn::Properties::No
		// naiv way
		prevValue = lines[0].p1().x();
		for (int row = 0; row < rowCount; row++) {
			double value = lines[row].p1().x();
			if (abs(value - x) <= abs(prevValue - x)) { // "<=" prevents also that row - 1 become < 0
				if (row < rowCount - 1)
					prevValue = value;
				else
					return row;
			} else {
				return row - 1;
			}
		}
	}
	return -1;
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
		maxDist = linePen.width() < 10 ? 10: linePen.width();

	double maxDistSquare = pow(maxDist,2);

	int properties = q->xColumn()->properties();
	if (properties == AbstractColumn::Properties::No) {
		// assumption: points exist if no line. otherwise previously returned false
		if (lineType == XYCurve::NoLine) {
			QPointF curvePosPrevScene = symbolPointsScene[0];
			QPointF curvePosScene = curvePosPrevScene;
			for (int row =0; row < rowCount; row ++) {
				if (pow(mouseScenePos.x() - curvePosScene.x(),2) + pow(mouseScenePos.y() - curvePosScene.y(),2) <= maxDistSquare)
					return true;

				curvePosPrevScene = curvePosScene;
				curvePosScene = symbolPointsScene[row];
			}
		} else {
			for (int row=0; row < rowCount; row++) {
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
			index = q->indexForX(x, symbolPointsScene, static_cast<AbstractColumn::Properties>(properties));
		} else
			index = q->indexForX(x, lines, static_cast<AbstractColumn::Properties>(properties));

		if (index >= 1)
			index --; // use one before so it is secured that I'm before point.x()

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

	double dist_segm = abs(dx1m*unitvec.y() - dy1m*unitvec.x());
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
	float errorPlus, errorMinus;

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
	if (errorBarsType != XYCurve::ErrorBarsSimple && !symbolPointsLogical.isEmpty()) {
		//determine the index of the first visible point
		size_t i = 0;
		while (i<visiblePoints.size() && !visiblePoints[i])
			i++;

		if (i == visiblePoints.size())
			return; //no visible points -> no error bars to draw

		//cap size for x-error bars
		QPointF pointScene = cSystem->mapLogicalToScene(symbolPointsLogical.at((int)i));
		pointScene.setY(pointScene.y()-errorBarsCapSize);
		QPointF pointLogical = cSystem->mapSceneToLogical(pointScene);
		capSizeX = (pointLogical.y() - symbolPointsLogical.at((int)i).y())/2;

		//cap size for y-error bars
		pointScene = cSystem->mapLogicalToScene(symbolPointsLogical.at((int)i));
		pointScene.setX(pointScene.x()+errorBarsCapSize);
		pointLogical = cSystem->mapSceneToLogical(pointScene);
		capSizeY = (pointLogical.x() - symbolPointsLogical.at((int)i).x())/2;
	}

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
			switch (errorBarsType) {
			case XYCurve::ErrorBarsSimple:
				lines.append(QLineF(QPointF(point.x()-errorMinus, point.y()),
				                    QPointF(point.x()+errorPlus, point.y())));
				break;
			case XYCurve::ErrorBarsWithEnds:
				lines.append(QLineF(QPointF(point.x()-errorMinus, point.y()),
				                    QPointF(point.x()+errorPlus, point.y())));
				if (errorMinus != 0) {
					lines.append(QLineF(QPointF(point.x()-errorMinus, point.y()-capSizeX),
					                    QPointF(point.x()-errorMinus, point.y()+capSizeX)));
				}
				if (errorPlus != 0) {
					lines.append(QLineF(QPointF(point.x()+errorPlus, point.y()-capSizeX),
					                    QPointF(point.x()+errorPlus, point.y()+capSizeX)));
				}
				break;
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
			switch (errorBarsType) {
			case XYCurve::ErrorBarsSimple:
				lines.append(QLineF(QPointF(point.x(), point.y()-errorMinus),
				                    QPointF(point.x(), point.y()+errorPlus)));
				break;
			case XYCurve::ErrorBarsWithEnds:
				lines.append(QLineF(QPointF(point.x(), point.y()-errorMinus),
				                    QPointF(point.x(), point.y()+errorPlus)));
				if (errorMinus != 0)
					lines.append(QLineF(QPointF(point.x()-capSizeY, point.y()-errorMinus),
					                    QPointF(point.x()+capSizeY, point.y()-errorMinus)));
				if (errorPlus != 0)
					lines.append(QLineF(QPointF(point.x()-capSizeY, point.y()+errorPlus),
					                    QPointF(point.x()+capSizeY, point.y()+errorPlus)));
				break;
			}
		}
	}

	//map the error bars to scene coordinates
	lines = cSystem->mapLogicalToScene(lines);

	//new painter path for the drop lines
	for (const auto& line : lines) {
		errorBarsPath.moveTo(line.p1());
		errorBarsPath.lineTo(line.p2());
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
	p.setStyle((Qt::PenStyle)group.readEntry("LineStyle", (int)this->linePen().style()));
	p.setWidthF(group.readEntry("LineWidth", this->linePen().widthF()));
	p.setColor(themeColor);
	this->setLinePen(p);
	this->setLineOpacity(group.readEntry("LineOpacity", this->lineOpacity()));

	//Drop line
	p.setStyle((Qt::PenStyle)group.readEntry("DropLineStyle",(int) this->dropLinePen().style()));
	p.setWidthF(group.readEntry("DropLineWidth", this->dropLinePen().widthF()));
	p.setColor(themeColor);
	this->setDropLinePen(p);
	this->setDropLineOpacity(group.readEntry("DropLineOpacity", this->dropLineOpacity()));

	//Symbol
	this->setSymbolsOpacity(group.readEntry("SymbolOpacity", this->symbolsOpacity()));
	QBrush brush = symbolsBrush();
	brush.setColor(themeColor);
	this->setSymbolsBrush(brush);
	p = symbolsPen();
	p.setColor(themeColor);
	this->setSymbolsPen(p);

	//Values
	this->setValuesOpacity(group.readEntry("ValuesOpacity", this->valuesOpacity()));
	this->setValuesColor(group.readEntry("ValuesColor", this->valuesColor()));

	//Filling
	this->setFillingBrushStyle((Qt::BrushStyle)group.readEntry("FillingBrushStyle",(int) this->fillingBrushStyle()));
	this->setFillingColorStyle((PlotArea::BackgroundColorStyle)group.readEntry("FillingColorStyle",(int) this->fillingColorStyle()));
	this->setFillingOpacity(group.readEntry("FillingOpacity", this->fillingOpacity()));
	this->setFillingPosition((XYCurve::FillingPosition)group.readEntry("FillingPosition",(int) this->fillingPosition()));
	this->setFillingSecondColor(group.readEntry("FillingSecondColor",(QColor) this->fillingSecondColor()));
	this->setFillingFirstColor(themeColor);
	this->setFillingType((PlotArea::BackgroundType)group.readEntry("FillingType",(int) this->fillingType()));

	//Error Bars
	p.setStyle((Qt::PenStyle)group.readEntry("ErrorBarsStyle",(int) this->errorBarsPen().style()));
	p.setWidthF(group.readEntry("ErrorBarsWidth", this->errorBarsPen().widthF()));
	p.setColor(themeColor);
	this->setErrorBarsPen(p);
	this->setErrorBarsOpacity(group.readEntry("ErrorBarsOpacity",this->errorBarsOpacity()));

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
