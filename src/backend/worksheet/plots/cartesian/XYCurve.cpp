/***************************************************************************
    File                 : XYCurve.cpp
    Project              : LabPlot
    Description          : A xy-curve
    --------------------------------------------------------------------
    Copyright            : (C) 2010-2021 Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2013-2021 Stefan Gerlach (stefan.gerlach@uni.kn)

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
#include "backend/worksheet/plots/cartesian/Symbol.h"
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
#include <KLocalizedString>

extern "C" {
#include <gsl/gsl_math.h>
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

void XYCurve::init() {
	Q_D(XYCurve);

	KConfig config;
	KConfigGroup group = config.group("XYCurve");

	d->lineType = (LineType) group.readEntry("LineType", static_cast<int>(LineType::Line));
	d->lineIncreasingXOnly = group.readEntry("LineIncreasingXOnly", false);
	d->lineSkipGaps = group.readEntry("SkipLineGaps", false);
	d->lineInterpolationPointsCount = group.readEntry("LineInterpolationPointsCount", 1);
	d->linePen.setStyle( (Qt::PenStyle) group.readEntry("LineStyle", (int)Qt::SolidLine) );
	d->linePen.setColor( group.readEntry("LineColor", QColor(Qt::black)) );
	d->linePen.setWidthF( group.readEntry("LineWidth", Worksheet::convertToSceneUnits(1.0, Worksheet::Unit::Point)) );
	d->lineOpacity = group.readEntry("LineOpacity", 1.0);

	d->dropLineType = (DropLineType) group.readEntry("DropLineType", static_cast<int>(LineType::NoLine));
	d->dropLinePen.setStyle( (Qt::PenStyle) group.readEntry("DropLineStyle", (int)Qt::SolidLine) );
	d->dropLinePen.setColor( group.readEntry("DropLineColor", QColor(Qt::black)));
	d->dropLinePen.setWidthF( group.readEntry("DropLineWidth", Worksheet::convertToSceneUnits(1.0, Worksheet::Unit::Point)) );
	d->dropLineOpacity = group.readEntry("DropLineOpacity", 1.0);

	//initialize the symbol
	d->symbol = new Symbol(QString());
	addChild(d->symbol);
	d->symbol->setHidden(true);
	connect(d->symbol, &Symbol::updateRequested, [=]{d->updateSymbols();});
	connect(d->symbol, &Symbol::updatePixmapRequested, [=]{d->updatePixmap();});
	d->symbol->init(group);

	d->valuesType = (ValuesType) group.readEntry("ValuesType", static_cast<int>(ValuesType::NoValues));
	d->valuesPosition = (ValuesPosition) group.readEntry("ValuesPosition", static_cast<int>(ValuesPosition::Above));
	d->valuesDistance = group.readEntry("ValuesDistance", Worksheet::convertToSceneUnits(5, Worksheet::Unit::Point));
	d->valuesRotationAngle = group.readEntry("ValuesRotation", 0.0);
	d->valuesOpacity = group.readEntry("ValuesOpacity", 1.0);
	d->valuesNumericFormat = group.readEntry("ValuesNumericFormat", "f").at(0).toLatin1();
	d->valuesPrecision = group.readEntry("ValuesNumericFormat", 2);
	d->valuesDateTimeFormat = group.readEntry("ValuesDateTimeFormat", "yyyy-MM-dd");
	d->valuesPrefix = group.readEntry("ValuesPrefix", "");
	d->valuesSuffix = group.readEntry("ValuesSuffix", "");
	d->valuesFont = group.readEntry("ValuesFont", QFont());
	d->valuesFont.setPixelSize( Worksheet::convertToSceneUnits(8, Worksheet::Unit::Point) );
	d->valuesColor = group.readEntry("ValuesColor", QColor(Qt::black));

	d->fillingPosition = (FillingPosition) group.readEntry("FillingPosition", static_cast<int>(FillingPosition::NoFilling));
	d->fillingType = (PlotArea::BackgroundType) group.readEntry("FillingType", static_cast<int>(PlotArea::BackgroundType::Color));
	d->fillingColorStyle = (PlotArea::BackgroundColorStyle) group.readEntry("FillingColorStyle", static_cast<int>(PlotArea::BackgroundColorStyle::SingleColor));
	d->fillingImageStyle = (PlotArea::BackgroundImageStyle) group.readEntry("FillingImageStyle", static_cast<int>(PlotArea::BackgroundImageStyle::Scaled));
	d->fillingBrushStyle = (Qt::BrushStyle) group.readEntry("FillingBrushStyle", static_cast<int>(Qt::SolidPattern));
	d->fillingFileName = group.readEntry("FillingFileName", QString());
	d->fillingFirstColor = group.readEntry("FillingFirstColor", QColor(Qt::white));
	d->fillingSecondColor = group.readEntry("FillingSecondColor", QColor(Qt::black));
	d->fillingOpacity = group.readEntry("FillingOpacity", 1.0);

	d->xErrorType = (ErrorType) group.readEntry("XErrorType", static_cast<int>(ErrorType::NoError));
	d->yErrorType = (ErrorType) group.readEntry("YErrorType", static_cast<int>(ErrorType::NoError));
	d->errorBarsType = (ErrorBarsType) group.readEntry("ErrorBarsType", static_cast<int>(ErrorBarsType::Simple));
	d->errorBarsCapSize = group.readEntry( "ErrorBarsCapSize", Worksheet::convertToSceneUnits(10, Worksheet::Unit::Point) );
	d->errorBarsPen.setStyle( (Qt::PenStyle)group.readEntry("ErrorBarsStyle", (int)Qt::SolidLine) );
	d->errorBarsPen.setColor( group.readEntry("ErrorBarsColor", QColor(Qt::black)) );
	d->errorBarsPen.setWidthF( group.readEntry("ErrorBarsWidth", Worksheet::convertToSceneUnits(1.0, Worksheet::Unit::Point)) );
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
//	auto* plot = static_cast<CartesianPlot*>(parentAspect());
	menu->insertMenu(visibilityAction, m_plot->analysisMenu());
	menu->insertSeparator(visibilityAction);
	menu->insertSeparator(firstAction);

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

/*!
 * \brief XYCurve::activateCurve
 * Checks if the mousepos distance to the curve is less than @p maxDist
 * \p mouseScenePos
 * \p maxDist Maximum distance the point lies away from the curve
 * \return Returns true if the distance is smaller than maxDist.
 */
bool XYCurve::activateCurve(QPointF mouseScenePos, double maxDist) {
	Q_D(XYCurve);
	return d->activateCurve(mouseScenePos, maxDist);
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

//##############################################################################
//##########################  getter methods  ##################################
//##############################################################################

//data source
BASIC_SHARED_D_READER_IMPL(XYCurve, const AbstractColumn*, xColumn, xColumn)
BASIC_SHARED_D_READER_IMPL(XYCurve, const AbstractColumn*, yColumn, yColumn)
BASIC_SHARED_D_READER_IMPL(XYCurve, QString, xColumnPath, xColumnPath)
BASIC_SHARED_D_READER_IMPL(XYCurve, QString, yColumnPath, yColumnPath)

//line
BASIC_SHARED_D_READER_IMPL(XYCurve, XYCurve::LineType, lineType, lineType)
BASIC_SHARED_D_READER_IMPL(XYCurve, bool, lineSkipGaps, lineSkipGaps)
BASIC_SHARED_D_READER_IMPL(XYCurve, bool, lineIncreasingXOnly, lineIncreasingXOnly)
BASIC_SHARED_D_READER_IMPL(XYCurve, int, lineInterpolationPointsCount, lineInterpolationPointsCount)
BASIC_SHARED_D_READER_IMPL(XYCurve, QPen, linePen, linePen)
BASIC_SHARED_D_READER_IMPL(XYCurve, qreal, lineOpacity, lineOpacity)

//droplines
BASIC_SHARED_D_READER_IMPL(XYCurve, XYCurve::DropLineType, dropLineType, dropLineType)
BASIC_SHARED_D_READER_IMPL(XYCurve, QPen, dropLinePen, dropLinePen)
BASIC_SHARED_D_READER_IMPL(XYCurve, qreal, dropLineOpacity, dropLineOpacity)

//symbols
Symbol* XYCurve::symbol() const {
	Q_D(const XYCurve);
	return d->symbol;
}

//values
BASIC_SHARED_D_READER_IMPL(XYCurve, XYCurve::ValuesType, valuesType, valuesType)
BASIC_SHARED_D_READER_IMPL(XYCurve, const AbstractColumn *, valuesColumn, valuesColumn)
BASIC_SHARED_D_READER_IMPL(XYCurve, QString, valuesColumnPath, valuesColumnPath)

BASIC_SHARED_D_READER_IMPL(XYCurve, XYCurve::ValuesPosition, valuesPosition, valuesPosition)
BASIC_SHARED_D_READER_IMPL(XYCurve, qreal, valuesDistance, valuesDistance)
BASIC_SHARED_D_READER_IMPL(XYCurve, qreal, valuesRotationAngle, valuesRotationAngle)
BASIC_SHARED_D_READER_IMPL(XYCurve, qreal, valuesOpacity, valuesOpacity)
BASIC_SHARED_D_READER_IMPL(XYCurve, char, valuesNumericFormat, valuesNumericFormat)
BASIC_SHARED_D_READER_IMPL(XYCurve, int, valuesPrecision, valuesPrecision)
BASIC_SHARED_D_READER_IMPL(XYCurve, QString, valuesDateTimeFormat, valuesDateTimeFormat)
BASIC_SHARED_D_READER_IMPL(XYCurve, QString, valuesPrefix, valuesPrefix)
BASIC_SHARED_D_READER_IMPL(XYCurve, QString, valuesSuffix, valuesSuffix)
BASIC_SHARED_D_READER_IMPL(XYCurve, QColor, valuesColor, valuesColor)
BASIC_SHARED_D_READER_IMPL(XYCurve, QFont, valuesFont, valuesFont)

//filling
BASIC_SHARED_D_READER_IMPL(XYCurve, XYCurve::FillingPosition, fillingPosition, fillingPosition)
BASIC_SHARED_D_READER_IMPL(XYCurve, PlotArea::BackgroundType, fillingType, fillingType)
BASIC_SHARED_D_READER_IMPL(XYCurve, PlotArea::BackgroundColorStyle, fillingColorStyle, fillingColorStyle)
BASIC_SHARED_D_READER_IMPL(XYCurve, PlotArea::BackgroundImageStyle, fillingImageStyle, fillingImageStyle)
BASIC_SHARED_D_READER_IMPL(XYCurve, Qt::BrushStyle, fillingBrushStyle, fillingBrushStyle)
BASIC_SHARED_D_READER_IMPL(XYCurve, QColor, fillingFirstColor, fillingFirstColor)
BASIC_SHARED_D_READER_IMPL(XYCurve, QColor, fillingSecondColor, fillingSecondColor)
BASIC_SHARED_D_READER_IMPL(XYCurve, QString, fillingFileName, fillingFileName)
BASIC_SHARED_D_READER_IMPL(XYCurve, qreal, fillingOpacity, fillingOpacity)

//error bars
BASIC_SHARED_D_READER_IMPL(XYCurve, XYCurve::ErrorType, xErrorType, xErrorType)
BASIC_SHARED_D_READER_IMPL(XYCurve, const AbstractColumn*, xErrorPlusColumn, xErrorPlusColumn)
BASIC_SHARED_D_READER_IMPL(XYCurve, const AbstractColumn*, xErrorMinusColumn, xErrorMinusColumn)
BASIC_SHARED_D_READER_IMPL(XYCurve, XYCurve::ErrorType, yErrorType, yErrorType)
BASIC_SHARED_D_READER_IMPL(XYCurve, const AbstractColumn*, yErrorPlusColumn, yErrorPlusColumn)
BASIC_SHARED_D_READER_IMPL(XYCurve, const AbstractColumn*, yErrorMinusColumn, yErrorMinusColumn)

BASIC_SHARED_D_READER_IMPL(XYCurve, QString, xErrorPlusColumnPath, xErrorPlusColumnPath)
BASIC_SHARED_D_READER_IMPL(XYCurve, QString, xErrorMinusColumnPath, xErrorMinusColumnPath)
BASIC_SHARED_D_READER_IMPL(XYCurve, QString, yErrorPlusColumnPath, yErrorPlusColumnPath)
BASIC_SHARED_D_READER_IMPL(XYCurve, QString, yErrorMinusColumnPath, yErrorMinusColumnPath)

BASIC_SHARED_D_READER_IMPL(XYCurve, XYCurve::ErrorBarsType, errorBarsType, errorBarsType)
BASIC_SHARED_D_READER_IMPL(XYCurve, qreal, errorBarsCapSize, errorBarsCapSize)
BASIC_SHARED_D_READER_IMPL(XYCurve, QPen, errorBarsPen, errorBarsPen)
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
			connect(column, &AbstractColumn::dataChanged, this, &XYCurve::updateErrorBars);
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

	d->symbol->setSize(d->symbol->size() * horizontalRatio);

	QPen pen = d->symbol->pen();
	pen.setWidthF(pen.widthF() * (horizontalRatio + verticalRatio) / 2.);
	d->symbol->setPen(pen);

	pen = d->linePen;
	pen.setWidthF(pen.widthF() * (horizontalRatio + verticalRatio) / 2.);
	setLinePen(pen);

	//setValuesDistance(d->distance*);
	QFont font = d->valuesFont;
	font.setPointSizeF(font.pointSizeF()*horizontalRatio);
	setValuesFont(font);
}

// Finds index where x is located and returns the value "index" after the found value
/*!
 * Find nearest x value from a value xpos and his y value
 * @param xpos position for which the next index xpos should be found
 * @param offset Offset from the index where xpos is. Positive is after the found index, negative is before the found index
 * @param x x value at the found index
 * @param y y value at the found index
 * @param valueFound True when value found, otherwise false
 */
int XYCurve::getNextValue(double xpos, int offset, double& x, double& y, bool& valueFound) const {

	valueFound = false;
	AbstractColumn::Properties properties = xColumn()->properties();
	if (properties == AbstractColumn::Properties::MonotonicDecreasing)
		offset *=-1;

	int index = xColumn()->indexForValue(xpos);
	if (index < 0)
		return -1;
	if (offset > 0 && index+offset < xColumn()->rowCount())
		index += offset;
	else if (offset > 0)
		index = xColumn()->rowCount() -1;
	else if ((offset < 0 && index+offset > 0))
		index += offset;
	else
		index = 0;


	AbstractColumn::ColumnMode xMode = xColumn()->columnMode();

	if (xMode == AbstractColumn::ColumnMode::Numeric ||
			xMode == AbstractColumn::ColumnMode::Integer)
		x = xColumn()->valueAt(index);
	else if (xMode == AbstractColumn::ColumnMode::DateTime ||
			 xMode == AbstractColumn::ColumnMode::Day ||
			 xMode == AbstractColumn::ColumnMode::Month)
		x = xColumn()->dateTimeAt(index).toMSecsSinceEpoch();
	else
		return index;


	AbstractColumn::ColumnMode yMode = yColumn()->columnMode();

	if (yMode == AbstractColumn::ColumnMode::Numeric ||
			yMode == AbstractColumn::ColumnMode::Integer)
		y = yColumn()->valueAt(index);
	else if (yMode == AbstractColumn::ColumnMode::DateTime ||
			 yMode == AbstractColumn::ColumnMode::Day ||
			 yMode == AbstractColumn::ColumnMode::Month)
		y = yColumn()->dateTimeAt(index).toMSecsSinceEpoch();
	else
		return index;

	valueFound = true;
	return index;
}

void XYCurve::xColumnAboutToBeRemoved(const AbstractAspect* aspect) {
	Q_D(XYCurve);
	if (aspect == d->xColumn) {
		disconnect(aspect, nullptr, this, nullptr);
		d->xColumn = nullptr;
		d->retransform();
	}
}

void XYCurve::yColumnAboutToBeRemoved(const AbstractAspect* aspect) {
	Q_D(XYCurve);
	if (aspect == d->yColumn) {
		disconnect(aspect, nullptr, this, nullptr);
		d->yColumn = nullptr;
		d->retransform();
	}
}

void XYCurve::valuesColumnAboutToBeRemoved(const AbstractAspect* aspect) {
	Q_D(XYCurve);
	if (aspect == d->valuesColumn) {
		disconnect(aspect, nullptr, this, nullptr);
		d->valuesColumn = nullptr;
		d->updateValues();
	}
}

void XYCurve::xErrorPlusColumnAboutToBeRemoved(const AbstractAspect* aspect) {
	Q_D(XYCurve);
	if (aspect == d->xErrorPlusColumn) {
		disconnect(aspect, nullptr, this, nullptr);
		d->xErrorPlusColumn = nullptr;
		d->updateErrorBars();
	}
}

void XYCurve::xErrorMinusColumnAboutToBeRemoved(const AbstractAspect* aspect) {
	Q_D(XYCurve);
	if (aspect == d->xErrorMinusColumn) {
		disconnect(aspect, nullptr, this, nullptr);
		d->xErrorMinusColumn = nullptr;
		d->updateErrorBars();
	}
}

void XYCurve::yErrorPlusColumnAboutToBeRemoved(const AbstractAspect* aspect) {
	Q_D(XYCurve);
	if (aspect == d->yErrorPlusColumn) {
		disconnect(aspect, nullptr, this, nullptr);
		d->yErrorPlusColumn = nullptr;
		d->updateErrorBars();
	}
}

void XYCurve::yErrorMinusColumnAboutToBeRemoved(const AbstractAspect* aspect) {
	Q_D(XYCurve);
	if (aspect == d->yErrorMinusColumn) {
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
	setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
	setData(0, static_cast<int>(WorksheetElement::WorksheetElementName::XYCurve));
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

	//When making a graphics item invisible, it gets deselected in the scene.
	//In this case we don't want to deselect the item in the project explorer.
	//We need to supress the deselection in the view.
	auto* worksheet = static_cast<Worksheet*>(q->parent(AspectType::Worksheet));
	worksheet->suppressSelectionChangedEvent(true);
	setVisible(on);
	worksheet->suppressSelectionChangedEvent(false);

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

	DEBUG("\n" << Q_FUNC_INFO << ", name = " << STDSTRING(name()) << ", m_suppressRetransform = " << m_suppressRetransform);
	if (m_suppressRetransform || !plot())
		return;

	{
#ifdef PERFTRACE_CURVES
		PERFTRACE(name().toLatin1() + ", XYCurvePrivate::retransform()");
#endif

	m_scenePoints.clear();

	if (!xColumn || !yColumn) {
		DEBUG(Q_FUNC_INFO << ", xColumn or yColumn not available");
		linePath = QPainterPath();
		dropLinePath = QPainterPath();
		symbolsPath = QPainterPath();
		valuesPath = QPainterPath();
		errorBarsPath = QPainterPath();
		curveShape = QPainterPath();
		m_lines.clear();
		m_valuePoints.clear();
		m_valueStrings.clear();
		m_fillPolygons.clear();
		recalcShapeAndBoundingRect();
		return;
	}

		if (!plot()->isPanningActive())
			WAIT_CURSOR;

	//calculate the scene coordinates
	// This condition cannot be used, because m_logicalPoints is also used in updateErrorBars(), updateDropLines() and in updateFilling()
	// TODO: check updateErrorBars() and updateDropLines() and if they aren't available don't calculate this part
	//if (symbolsStyle != Symbol::Style::NoSymbols || valuesType != XYCurve::NoValues ) {
	{
#ifdef PERFTRACE_CURVES
	PERFTRACE(name().toLatin1() + ", XYCurvePrivate::retransform(), map logical points to scene coordinates");
#endif

	const int numberOfPoints = m_logicalPoints.size();
	DEBUG(Q_FUNC_INFO << ", number of logical points = " << numberOfPoints)
	if (numberOfPoints > 0) {
		const auto dataRect{ plot()->dataRect() };
		// this is the old method considering DPI
		DEBUG(Q_FUNC_INFO << ", plot->dataRect() width/height = " << dataRect.width() << '/'  << dataRect.height());
		//const double widthDatarectInch = Worksheet::convertFromSceneUnits(plot->dataRect().width(), Worksheet::Unit::Inch);
		//const double heightDatarectInch = Worksheet::convertFromSceneUnits(plot->dataRect().height(), Worksheet::Unit::Inch);
		//DEBUG(Q_FUNC_INFO << ", widthDatarectInch/heightDatarectInch = " << widthDatarectInch << '/' << heightDatarectInch)
		DEBUG(Q_FUNC_INFO << ", logical DPI X/Y = " << QApplication::desktop()->logicalDpiX() << '/' << QApplication::desktop()->logicalDpiY())
		DEBUG(Q_FUNC_INFO << ", physical DPI X/Y = " << QApplication::desktop()->physicalDpiX() << '/' << QApplication::desktop()->physicalDpiY())
		//const int numberOfPixelX = ceil(widthDatarectInch * QApplication::desktop()->physicalDpiX());
		//const int numberOfPixelY = ceil(heightDatarectInch * QApplication::desktop()->physicalDpiY());

		// new method
		const int numberOfPixelX = dataRect.width();
		const int numberOfPixelY = dataRect.height();

		if (numberOfPixelX <= 0 || numberOfPixelY <= 0) {
			DEBUG(Q_FUNC_INFO << ", number of pixel X <= 0 or number of pixel Y <= 0!")
			RESET_CURSOR;
			return;
		}

		DEBUG(Q_FUNC_INFO << ", numberOfPixelX/numberOfPixelY = " << numberOfPixelX << '/' << numberOfPixelY)

		const auto columnProperties = xColumn->properties();
		int startIndex, endIndex;
		if (columnProperties == AbstractColumn::Properties::MonotonicDecreasing ||
			columnProperties == AbstractColumn::Properties::MonotonicIncreasing) {
			DEBUG(Q_FUNC_INFO << ", column monotonic")
			double xMin = q->cSystem->mapSceneToLogical(dataRect.topLeft()).x();
			double xMax = q->cSystem->mapSceneToLogical(dataRect.bottomRight()).x();
			DEBUG(Q_FUNC_INFO << ", xMin/xMax = " << xMin << '/' << xMax)

			startIndex = Column::indexForValue(xMin, m_logicalPoints, columnProperties);
			endIndex = Column::indexForValue(xMax, m_logicalPoints, columnProperties);

			if (startIndex > endIndex && endIndex >= 0)
				std::swap(startIndex, endIndex);

			if (startIndex < 0)
				startIndex = 0;
			if (endIndex < 0)
				endIndex = numberOfPoints - 1;

		} else {
			DEBUG(Q_FUNC_INFO << ", column not monotonic")
			startIndex = 0;
			endIndex = numberOfPoints - 1;
		}
		//} // (symbolsStyle != Symbol::NoSymbols || valuesType != XYCurve::NoValues )

		m_pointVisible.clear();
		m_pointVisible.resize(numberOfPoints);
		q->cSystem->mapLogicalToScene(startIndex, endIndex, m_logicalPoints, m_scenePoints, m_pointVisible);
	}
	}
	//} // (symbolsStyle != Symbol::Style::NoSymbols || valuesType != XYCurve::NoValues )

		RESET_CURSOR;
	}

	m_suppressRecalc = true;
	updateLines();
	updateDropLines();
	updateSymbols();
	updateValues();
	m_suppressRecalc = false;
	updateErrorBars();
}

/*!
 * called if the x- or y-data was changed.
 * copies the valid data points from the x- and y-columns into the internal container
 */
void XYCurvePrivate::recalcLogicalPoints() {
	PERFTRACE(name().toLatin1() + ", XYCurvePrivate::recalcLogicalPoints()");

	m_pointVisible.clear();
	m_logicalPoints.clear();
	connectedPointsLogical.clear();
	validPointsIndicesLogical.clear();

	if (!xColumn || !yColumn)
		return;

	auto xColMode = xColumn->columnMode();
	auto yColMode = yColumn->columnMode();
	const int rows = xColumn->rowCount();
	m_logicalPoints.reserve(rows);

	//take only valid and non masked points
	for (int row{0}; row < rows; row++) {
		if ( xColumn->isValid(row) && yColumn->isValid(row)
				&& (!xColumn->isMasked(row)) && (!yColumn->isMasked(row)) ) {
			QPointF tempPoint;

			switch (xColMode) {
			case AbstractColumn::ColumnMode::Numeric:
			case AbstractColumn::ColumnMode::Integer:
			case AbstractColumn::ColumnMode::BigInt:
				tempPoint.setX(xColumn->valueAt(row));
				break;
			case AbstractColumn::ColumnMode::DateTime:
				tempPoint.setX(xColumn->dateTimeAt(row).toMSecsSinceEpoch());
				break;
			case AbstractColumn::ColumnMode::Text:
			case AbstractColumn::ColumnMode::Month:
			case AbstractColumn::ColumnMode::Day:
				break;
			}

			switch (yColMode) {
			case AbstractColumn::ColumnMode::Numeric:
			case AbstractColumn::ColumnMode::Integer:
			case AbstractColumn::ColumnMode::BigInt:
				tempPoint.setY(yColumn->valueAt(row));
				break;
			case AbstractColumn::ColumnMode::DateTime:
				tempPoint.setY(yColumn->dateTimeAt(row).toMSecsSinceEpoch());
				break;
			case AbstractColumn::ColumnMode::Text:
			case AbstractColumn::ColumnMode::Month:
			case AbstractColumn::ColumnMode::Day:
				break;
			}

			m_logicalPoints.append(tempPoint);
			//TODO: append, resize-reserve
			connectedPointsLogical.push_back(true);
			validPointsIndicesLogical.push_back(row);
		} else {
			if (!connectedPointsLogical.empty())
				connectedPointsLogical[connectedPointsLogical.size() - 1] = false;
		}
	}

	m_pointVisible.resize(m_logicalPoints.size());
}

/*!
 * Adds a line, which connects two points, but only if they don't lie on the same xAxis pixel.
 * If they lie on the same x pixel, draw a vertical line between the minimum and maximum y value. So all points are included
 * This function is only valid for linear x Axis scale!
 * @param p0 first point
 * @param p1 second point
 * @param overlap if at the previous call was an overlap between the previous two points
 * @param minLogicalDiffX logical difference between two pixels
 * @param pixelDiff x pixel distance between two points
 */
void XYCurvePrivate::addLinearLine(QPointF p0, QPointF p1, QPointF& lastPoint, double minLogicalDiffX, qint64& pixelDiff) {
	pixelDiff = qRound64(p1.x() / minLogicalDiffX) - qRound64(p0.x() / minLogicalDiffX);
	//QDEBUG(Q_FUNC_INFO << ", " << p0 << " -> " << p1  << "p0.x*minLogicalDiffX =" << p0.x()*minLogicalDiffX << ", p1.x*minLogicalDiffX =" << p1.x()*minLogicalDiffX << ", pixelDiff =" << pixelDiff);

	addUniqueLine(p0, p1, lastPoint, pixelDiff);
}

/*!
 * Adds a line, which connects two points, but only if they don't lie on the same xAxis pixel.
 * If they lie on the same x pixel, draw a vertical line between the minimum and maximum y value. So all points are included
 * This function can be used for all axis scalings (linear, log, sqrt, ...). For the linear case use the function above, because it's optimized for the linear case
 * @param p0 first point
 * @param p1 second point
 * @param lastPoint remember last point in case of overlap
 * @param pixelDiff x pixel distance between two points
 * @param pixelCount pixel count
 */
void XYCurvePrivate::addLine(QPointF p0, QPointF p1, QPointF& lastPoint, qint64& pixelDiff, int numberOfPixelX) {
	//DEBUG(Q_FUNC_INFO << ", coordinate system index: " << cSystem->xIndex())

	const auto xIndex{ q->cSystem->xIndex() };
	if (plot()->xRangeScale(xIndex) == RangeT::Scale::Linear) {
		double minLogicalDiffX = plot()->xRangeCSystem(q->coordinateSystemIndex()).size()/numberOfPixelX;
		//DEBUG("	plot->xMax() - plot->xMin() = " << plot->xMax() - plot->xMin())
		//DEBUG("	plot->dataRect().width() = " << plot->dataRect().width())
		//DEBUG("	-> minLogicalDiffX = " << minLogicalDiffX)
		addLinearLine(p0, p1, lastPoint, minLogicalDiffX, pixelDiff);
	} else {
		// for nonlinear scaling the pixel distance must be calculated for every point pair
		bool visible;
		QPointF p0Scene = q->cSystem->mapLogicalToScene(p0, visible, CartesianCoordinateSystem::MappingFlag::SuppressPageClipping);
		QPointF p1Scene = q->cSystem->mapLogicalToScene(p1, visible, CartesianCoordinateSystem::MappingFlag::SuppressPageClipping);

		// if the point is not valid, don't create a line
		if (visible)
			return;

		// using only the difference between the points is not sufficient, because p0 is updated always
		// if new line is added or not
		const auto dataRect{ plot()->dataRect() };
		qint64 p0Pixel = qRound64((p0Scene.x() - dataRect.x()) / (double)dataRect.width() * numberOfPixelX);
		qint64 p1Pixel = qRound64((p1Scene.x() - dataRect.x()) / (double)dataRect.width() * numberOfPixelX);
		//DEBUG(Q_FUNC_INFO << ", p0Pixel/p1Pixel = " << p0Pixel << ' ' << p1Pixel)
		pixelDiff = p1Pixel - p0Pixel;
		addUniqueLine(p0, p1, lastPoint, pixelDiff);
	}
}

/*!
 * \brief XYCurvePrivate::addUniqueLine
 * This function is called from the other two addLine() functions to avoid duplication
 * @param p0 first point
 * @param p1 second point
 * @param lastPoint remember last point in case of overlap
 * @param pixelDiff x pixel distance between two points
 */
void XYCurvePrivate::addUniqueLine(QPointF p0, QPointF p1, QPointF& lastPoint, qint64& pixelDiff) {
	//QDEBUG(Q_FUNC_INFO << " :" << p0 << " ->" << p1 << ", lastPoint =" << lastPoint << ", pixelDiff =" << pixelDiff)
	if (pixelDiff == 0) {
		//QDEBUG("	pixelDiff == 0!")
		if (std::isnan(lastPoint.x()))	// save last point
			lastPoint = p0;
	} else {	// pixelDiff > 0
		//QDEBUG("	pixelDiff =" << pixelDiff << ", last point : " << lastPoint)
		if (!std::isnan(lastPoint.x())) { // when previously lastPoint, draw a line
			//QDEBUG("	REDUCED LINE from " << lastPoint << " to " << p0)
			//TODO: only when line in scene
			//if ((p0.x() >= plot->xMin() && p0.x() <= plot->xMax()) || (p1.x() >= plot->xMin() && p1.x() <= plot->xMax()))
			// || (p0.x() < plot->xMin() && p1.x() > plot->xMax()) || (p0.x() > plot->xMax() && p1.x() < plot->xMin())
			// same for y
			m_lines.append(QLineF(lastPoint, p0));

			lastPoint.setX(qQNaN());
		}

		//QDEBUG("	LINE " << p0 << ' ' << p1)
		//TODO only when line in scene (s.a.)
		m_lines.append(QLineF(p0, p1));
	}
}

/*!
  recalculates the painter path for the lines connecting the data points.
  Called each time when the type of this connection is changed.
TODO: At the moment also the points which are outside of the scene are added. This algorithm can be improved by omitting lines
  lines not visible in plot
*/
void XYCurvePrivate::updateLines() {
#ifdef PERFTRACE_CURVES
	PERFTRACE(name().toLatin1() + ", XYCurvePrivate::updateLines()");
#endif
	linePath = QPainterPath();
	m_lines.clear();
	if (lineType == XYCurve::LineType::NoLine) {
		DEBUG(Q_FUNC_INFO << ", nothing to do, since line type is XYCurve::LineType::NoLine");
		updateFilling();
		recalcShapeAndBoundingRect();
		return;
	}

	int numberOfPoints{m_logicalPoints.size()};
	if (numberOfPoints <= 1) {
		DEBUG(Q_FUNC_INFO << ", nothing to do, since not enough data points available");
		recalcShapeAndBoundingRect();
		return;
	}

	const QRectF pageRect = plot()->dataRect();
	// old method using DPI
	//const double widthDatarectInch = Worksheet::convertFromSceneUnits(plot->dataRect().width(), Worksheet::Unit::Inch);
	//float heightDatarectInch = Worksheet::convertFromSceneUnits(plot->dataRect().height(), Worksheet::Unit::Inch);	// unsed
	//const int countPixelX = ceil(widthDatarectInch * QApplication::desktop()->physicalDpiX());
	//int countPixelY = ceil(heightDatarectInch*QApplication::desktop()->physicalDpiY());	// unused
	// new method
	const int numberOfPixelX = pageRect.width();

	// only valid for linear scale
	//double minLogicalDiffX = 1/((plot->xMax()-plot->xMin())/countPixelX);	// unused
	//double minLogicalDiffY = 1/((plot->yMax()-plot->yMin())/countPixelY); // unused

	//calculate the lines connecting the data points
	{
#ifdef PERFTRACE_CURVES
		PERFTRACE(name().toLatin1() + ", XYCurvePrivate::updateLines(), calculate the lines connecting the data points");
#endif

	// find index for xMin and xMax to not loop through all values
	int startIndex, endIndex;
	auto columnProperties = q->xColumn()->properties();
	if (columnProperties == AbstractColumn::Properties::MonotonicDecreasing ||
		columnProperties == AbstractColumn::Properties::MonotonicIncreasing) {
		DEBUG(Q_FUNC_INFO << ", monotonic")
		const double xMin = q->cSystem->mapSceneToLogical(pageRect.topLeft()).x();
		const double xMax = q->cSystem->mapSceneToLogical(pageRect.bottomRight()).x();

		startIndex = Column::indexForValue(xMin, m_logicalPoints, columnProperties);
		endIndex = Column::indexForValue(xMax, m_logicalPoints, columnProperties);

		if (startIndex > endIndex)
			std::swap(startIndex, endIndex);

		startIndex--; // use one value before
		endIndex++;
		if (startIndex < 0)
			startIndex = 0;
		if (endIndex < 0 || endIndex >= numberOfPoints)
			endIndex = numberOfPoints - 1;

		numberOfPoints = endIndex - startIndex + 1;
	} else {
		DEBUG(Q_FUNC_INFO << ", non monotonic")
		startIndex = 0;
		endIndex = numberOfPoints - 1;
	}
	DEBUG(Q_FUNC_INFO << ", start/endIndex = " << startIndex << '/' << endIndex)

	QPointF tempPoint1, tempPoint2; // used as temporaryPoints to interpolate datapoints if set
	if (columnProperties == AbstractColumn::Properties::Constant) {
		DEBUG(Q_FUNC_INFO << ", CONSTANT column")
		const auto xRange{ plot()->xRange_(q->cSystem->xIndex()) };
		const auto yRange{ plot()->yRange_(q->cSystem->yIndex()) };
		tempPoint1 = QPointF(xRange.start(), yRange.start());
		tempPoint2 = QPointF(xRange.start(), yRange.end());
		m_lines.append(QLineF(tempPoint1, tempPoint2));
	} else {
		QPointF lastPoint{qQNaN(), qQNaN()};	// last x value
		qint64 pixelDiff;
		QPointF p0, p1;

		switch (lineType) {
		case XYCurve::LineType::NoLine:
			break;
		case XYCurve::LineType::Line: {
			for (int i{startIndex}; i < endIndex; i++) {
				if (!lineSkipGaps && !connectedPointsLogical.at(i))
					continue;
				p0 = m_logicalPoints.at(i);
				p1 = m_logicalPoints.at(i+1);
				if (lineIncreasingXOnly && (p1.x() < p0.x())) // skip points
					continue;
				addLine(p0, p1, lastPoint, pixelDiff, numberOfPixelX);
			}

			if (!std::isnan(lastPoint.x()))	// last line
				m_lines.append(QLineF(lastPoint, p1));

			break;
		}
		case XYCurve::LineType::StartHorizontal: {
			for (int i{startIndex}; i < endIndex; i++) {
				if (!lineSkipGaps && !connectedPointsLogical[i])
					continue;
				p0 = m_logicalPoints.at(i);
				p1 = m_logicalPoints.at(i+1);
				if (lineIncreasingXOnly && (p1.x() < p0.x()))
					continue;

				tempPoint1 = QPointF(p1.x(), p0.y());
				addLine(p0, tempPoint1, lastPoint, pixelDiff, numberOfPixelX);
				addLine(tempPoint1, p1, lastPoint, pixelDiff, numberOfPixelX);
			}
			if (!std::isnan(lastPoint.x()))	// last line
				m_lines.append(QLineF(lastPoint, p1));

			break;
		}
		case XYCurve::LineType::StartVertical: {
			for (int i{startIndex}; i < endIndex; i++) {
				if (!lineSkipGaps && !connectedPointsLogical.at(i))
					continue;
				p0 = m_logicalPoints.at(i);
				p1 = m_logicalPoints.at(i+1);
				if (lineIncreasingXOnly && (p1.x() < p0.x()))
					continue;
				tempPoint1 = QPointF(p0.x(), p1.y());
				addLine(p0, tempPoint1, lastPoint, pixelDiff, numberOfPixelX);
				addLine(tempPoint1, p1, lastPoint, pixelDiff, numberOfPixelX);
			}
			if (!std::isnan(lastPoint.x()))	// last line
				m_lines.append(QLineF(lastPoint, p1));

			break;
		}
		case XYCurve::LineType::MidpointHorizontal: {
			for (int i{startIndex}; i < endIndex; i++) {
				if (!lineSkipGaps && !connectedPointsLogical[i])
					continue;

				p0 = m_logicalPoints.at(i);
				p1 = m_logicalPoints.at(i+1);
				if (lineIncreasingXOnly && (p1.x() < p0.x()))
					continue;
				tempPoint1 = QPointF(p0.x() + (p1.x()-p0.x())/2., p0.y());
				tempPoint2 = QPointF(p0.x() + (p1.x()-p0.x())/2., p1.y());
				addLine(p0, tempPoint1, lastPoint, pixelDiff, numberOfPixelX);
				addLine(tempPoint1, tempPoint2, lastPoint, pixelDiff, numberOfPixelX);
				addLine(tempPoint2, p1, lastPoint, pixelDiff, numberOfPixelX);
			}
			if (!std::isnan(lastPoint.x()))	// last line
				m_lines.append(QLineF(lastPoint, p1));

			break;
		}
		case XYCurve::LineType::MidpointVertical: {
			for (int i{startIndex}; i < endIndex; i++) {
				if (!lineSkipGaps && !connectedPointsLogical[i])
					continue;

				p0 = m_logicalPoints.at(i);
				p1 = m_logicalPoints.at(i+1);
				if (lineIncreasingXOnly && (p1.x() < p0.x()))
					continue;
				tempPoint1 = QPointF(p0.x(), p0.y() + (p1.y()-p0.y())/2.);
				tempPoint2 = QPointF(p1.x(), p0.y() + (p1.y()-p0.y())/2.);
				addLine(p0, tempPoint1, lastPoint, pixelDiff, numberOfPixelX);
				addLine(tempPoint1, tempPoint2, lastPoint, pixelDiff, numberOfPixelX);
				addLine(tempPoint2, p1, lastPoint, pixelDiff, numberOfPixelX);
			}
			if (!std::isnan(lastPoint.x()))	// last line
				m_lines.append(QLineF(lastPoint, p1));

			break;
		}
		case XYCurve::LineType::Segments2: {
			int skip{0};
			for (int i{startIndex}; i < endIndex; i++) {
				p0 = m_logicalPoints.at(i);
				p1 = m_logicalPoints.at(i+1);
				if (skip != 1) {
					if ( (!lineSkipGaps && !connectedPointsLogical[i])
						|| (lineIncreasingXOnly && (p1.x() < p0.x())) ) {
						skip = 0;
						continue;
					}
					addLine(p0, p1, lastPoint, pixelDiff, numberOfPixelX);
					skip++;
				} else {
					skip = 0;
					if (!std::isnan(lastPoint.x())) {
						lastPoint.setX(qQNaN());
						m_lines.append(QLineF(lastPoint, p1));
					}
				}
			}
			if (!std::isnan(lastPoint.x()))	// last line
				m_lines.append(QLineF(m_logicalPoints.at(endIndex - 1), m_logicalPoints.at(endIndex)));

			break;
		}
		case XYCurve::LineType::Segments3: {
			int skip{0};
			for (int i{startIndex}; i < endIndex; i++) {
				if (skip != 2) {
					p0 = m_logicalPoints.at(i);
					p1 = m_logicalPoints.at(i+1);
					if ( (!lineSkipGaps && !connectedPointsLogical[i])
						|| (lineIncreasingXOnly && (p1.x() < p0.x())) ) {
						skip = 0;
						continue;
					}
					addLine(p0, p1, lastPoint, pixelDiff, numberOfPixelX);
					skip++;
				} else {
					skip = 0;
					if (!std::isnan(lastPoint.x())) {
						lastPoint.setX(qQNaN());
						m_lines.append(QLineF(lastPoint, p1));
					}
					if (!std::isnan(lastPoint.x()))	// last line
						m_lines.append(QLineF(m_logicalPoints[endIndex-1], m_logicalPoints[endIndex]));

					break;
				}
			}
			if (!std::isnan(lastPoint.x()))	// last line
				m_lines.append(QLineF(m_logicalPoints.at(endIndex - 1), m_logicalPoints.at(endIndex)));

			break;
		}
		case XYCurve::LineType::SplineCubicNatural:
		case XYCurve::LineType::SplineCubicPeriodic:
		case XYCurve::LineType::SplineAkimaNatural:
		case XYCurve::LineType::SplineAkimaPeriodic: {
			std::unique_ptr<double[]> x(new double[numberOfPoints]());
			std::unique_ptr<double[]> y(new double[numberOfPoints]());
			for (int i{0}; i < numberOfPoints; i++) { // TODO: interpolating only between the visible points?
				x[i] = m_logicalPoints.at(i+startIndex).x();
				y[i] = m_logicalPoints.at(i+startIndex).y();
			}

			gsl_interp_accel *acc = gsl_interp_accel_alloc();
			gsl_spline *spline{nullptr};
			gsl_set_error_handler_off();
			switch (lineType) {
			case XYCurve::LineType::SplineCubicNatural:
				spline = gsl_spline_alloc(gsl_interp_cspline, numberOfPoints);
				break;
			case XYCurve::LineType::SplineCubicPeriodic:
				spline = gsl_spline_alloc(gsl_interp_cspline_periodic, numberOfPoints);
				break;
			case XYCurve::LineType::SplineAkimaNatural:
				spline = gsl_spline_alloc(gsl_interp_akima, numberOfPoints);
				break;
			case XYCurve::LineType::SplineAkimaPeriodic:
				spline = gsl_spline_alloc(gsl_interp_akima_periodic, numberOfPoints);
				break;
			case XYCurve::LineType::NoLine:
			case XYCurve::LineType::Line:
			case XYCurve::LineType::StartHorizontal:
			case XYCurve::LineType::StartVertical:
			case XYCurve::LineType::MidpointHorizontal:
			case XYCurve::LineType::MidpointVertical:
			case XYCurve::LineType::Segments2:
			case XYCurve::LineType::Segments3:
				break;
			}

			if (!spline) {
				QString msg;
				if ( (lineType == XYCurve::LineType::SplineAkimaNatural || lineType == XYCurve::LineType::SplineAkimaPeriodic) && numberOfPoints < 5)
					msg = i18n("Error: Akima spline interpolation requires a minimum of 5 points.");
				else
					msg = i18n("Error: Could not initialize the spline function.");
				emit q->info(msg);

				recalcShapeAndBoundingRect();
				gsl_interp_accel_free(acc);
				return;
			}

			int status = gsl_spline_init(spline, x.get(), y.get(), numberOfPoints);
			if (status != 0) {
				//TODO: check in gsl/interp.c when GSL_EINVAL is thrown
				QString gslError;
				if (status == GSL_EINVAL)
					gslError = i18n("x values must be monotonically increasing.");
				else
					gslError = gslErrorToString(status);
				emit q->info( i18n("Error: %1", gslError) );

				recalcShapeAndBoundingRect();
				gsl_spline_free(spline);
				gsl_interp_accel_free(acc);
				return;
			}

			//create interpolating points
			//TODO: QVector
			std::vector<double> xinterp, yinterp;
			for (int i{0}; i < numberOfPoints - 1; i++) {
				const double x1 = x[i];
				const double x2 = x[i+1];
				const double step = std::abs(x2 - x1)/(lineInterpolationPointsCount + 1);

				for (int j{0}; j < (lineInterpolationPointsCount + 1); j++) {
					const double xi = x1 + j*step;
					const double yi = gsl_spline_eval(spline, xi, acc);
					xinterp.push_back(xi);
					yinterp.push_back(yi);
				}
			}

			if (!xinterp.empty()) {
				for (unsigned int i{0}; i < xinterp.size() - 1; i++) {
					p0 = QPointF(xinterp[i], yinterp[i]);
					p1 = QPointF(xinterp[i + 1], yinterp[i + 1]);
					addLine(p0, p1, lastPoint, pixelDiff, numberOfPixelX);
				}

				addLine(QPointF(xinterp[xinterp.size() - 1], yinterp[yinterp.size() - 1]), QPointF(x[numberOfPoints - 1], y[numberOfPoints - 1]),
						lastPoint, pixelDiff, numberOfPixelX);

				// add last line
				if (!std::isnan(lastPoint.x()))
					m_lines.append(QLineF(QPointF(xinterp[xinterp.size() - 1], yinterp[yinterp.size() - 1]),
								QPointF(x[numberOfPoints - 1], y[numberOfPoints - 1])));
			}

			gsl_spline_free(spline);
			gsl_interp_accel_free(acc);
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
		m_lines = q->cSystem->mapLogicalToScene(m_lines);
	}

	{
#ifdef PERFTRACE_CURVES
		PERFTRACE(name().toLatin1() + ", XYCurvePrivate::updateLines(), calculate new line path");
#endif
		//new line path
		for (const auto& line : qAsConst(m_lines)) {
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
	if (dropLineType == XYCurve::DropLineType::NoDropLine) {
		recalcShapeAndBoundingRect();
		return;
	}

	//calculate drop lines
	QVector<QLineF> dlines;
	const double xMin = plot()->xRange_(q->cSystem->xIndex()).start();
	const double yMin = plot()->yRange_(q->cSystem->xIndex()).start();

	//don't skip the invisible points, we still need to calculate
	//the drop lines falling into the plot region
	switch (dropLineType) {
	case XYCurve::DropLineType::NoDropLine:
		break;
	case XYCurve::DropLineType::X:
		for (const auto& point: qAsConst(m_logicalPoints)) {
			dlines.append(QLineF(point, QPointF(point.x(), yMin)));
		}
		break;
	case XYCurve::DropLineType::Y:
		for (const auto& point: qAsConst(m_logicalPoints)) {
			dlines.append(QLineF(point, QPointF(xMin, point.y())));
		}
		break;
	case XYCurve::DropLineType::XY:
		for (const auto& point: qAsConst(m_logicalPoints)) {
			dlines.append(QLineF(point, QPointF(point.x(), yMin)));
			dlines.append(QLineF(point, QPointF(xMin, point.y())));
		}
		break;
	case XYCurve::DropLineType::XZeroBaseline:
		for (const auto& point: qAsConst(m_logicalPoints)) {
			dlines.append(QLineF(point, QPointF(point.x(), 0)));
		}
		break;
	case XYCurve::DropLineType::XMinBaseline:
		for (const auto& point: qAsConst(m_logicalPoints)) {
			dlines.append(QLineF(point, QPointF(point.x(), yColumn->minimum())));
		}
		break;
	case XYCurve::DropLineType::XMaxBaseline:
		for (const auto& point: qAsConst(m_logicalPoints)) {
			dlines.append(QLineF(point, QPointF(point.x(), yColumn->maximum())));
		}
		break;
	}

	//map the drop lines to scene coordinates
	dlines = q->cSystem->mapLogicalToScene(dlines);

	//new painter path for the drop lines
	for (const auto& line : qAsConst(dlines)) {
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
	if (symbol->style() != Symbol::Style::NoSymbols) {
		QPainterPath path = Symbol::pathFromStyle(symbol->style());

		QTransform trafo;
		trafo.scale(symbol->size(), symbol->size());
		path = trafo.map(path);
		trafo.reset();

		if (symbol->rotationAngle() != 0) {
			trafo.rotate(symbol->rotationAngle());
			path = trafo.map(path);
		}

		for (const auto& point : qAsConst(m_scenePoints)) {
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
	m_valuePoints.clear();
	m_valueStrings.clear();

	const int numberOfPoints = m_logicalPoints.size();
	if (valuesType == XYCurve::ValuesType::NoValues || numberOfPoints == 0) {
		recalcShapeAndBoundingRect();
		return;
	}
	m_valuePoints.reserve(numberOfPoints);
	m_valueStrings.reserve(numberOfPoints);

	//determine the value string for all points that are currently visible in the plot
	int i{0};
	SET_NUMBER_LOCALE
	switch (valuesType) {
	case XYCurve::ValuesType::NoValues:
	case XYCurve::ValuesType::X: {
		auto xRangeFormat{ plot()->xRange_(q->cSystem->xIndex()).format() };
		int precision = valuesPrecision;
		if (xColumn->columnMode() == AbstractColumn::ColumnMode::Integer || xColumn->columnMode() == AbstractColumn::ColumnMode::BigInt)
			precision = 0;
		for (const auto& point : qAsConst(m_logicalPoints)) {
			if (!m_pointVisible.at(i++)) continue;
			QString value;
			if (xRangeFormat == RangeT::Format::Numeric)
				value = numberLocale.toString(point.x(), valuesNumericFormat, precision);
			else
				value = QDateTime::fromMSecsSinceEpoch(point.x()).toString(valuesDateTimeFormat);
			m_valueStrings << valuesPrefix + value + valuesSuffix;
		}
		break;
	}
	case XYCurve::ValuesType::Y: {
		auto rangeFormat{ plot()->yRange_(q->cSystem->yIndex()).format() };
		int precision = valuesPrecision;
		if (yColumn->columnMode() == AbstractColumn::ColumnMode::Integer || yColumn->columnMode() == AbstractColumn::ColumnMode::BigInt)
			precision = 0;
		for (const auto& point : qAsConst(m_logicalPoints)) {
			if (!m_pointVisible.at(i++)) continue;
			QString value;
			if (rangeFormat == RangeT::Format::Numeric)
				value = numberLocale.toString(point.y(), valuesNumericFormat, precision);
			else
				value = QDateTime::fromMSecsSinceEpoch(point.y()).toString(valuesDateTimeFormat);
			m_valueStrings << valuesPrefix + value + valuesSuffix;
		}
		break;
	}
	case XYCurve::ValuesType::XY:
	case XYCurve::ValuesType::XYBracketed: {
		auto xRangeFormat{ plot()->xRange_(q->cSystem->xIndex()).format() };
		auto yRangeFormat{ plot()->yRange_(q->cSystem->yIndex()).format() };

		int xPrecision = valuesPrecision;
		if (xColumn->columnMode() == AbstractColumn::ColumnMode::Integer || xColumn->columnMode() == AbstractColumn::ColumnMode::BigInt)
			xPrecision = 0;

		int yPrecision = valuesPrecision;
		if (yColumn->columnMode() == AbstractColumn::ColumnMode::Integer || yColumn->columnMode() == AbstractColumn::ColumnMode::BigInt)
			yPrecision = 0;

		for (const auto& point : qAsConst(m_logicalPoints)) {
			if (!m_pointVisible.at(i++)) continue;
			QString value;
			if (valuesType == XYCurve::ValuesType::XYBracketed)
				value = '(';
			if (xRangeFormat == RangeT::Format::Numeric)
				value += numberLocale.toString(point.x(), valuesNumericFormat, xPrecision);
			else
				value += QDateTime::fromMSecsSinceEpoch(point.x()).toString(valuesDateTimeFormat);

			if (yRangeFormat == RangeT::Format::Numeric)
				value += ',' + numberLocale.toString(point.y(), valuesNumericFormat, yPrecision);
			else
				value += ',' + QDateTime::fromMSecsSinceEpoch(point.y()).toString(valuesDateTimeFormat);

			if (valuesType == XYCurve::ValuesType::XYBracketed)
				value += ')';

			m_valueStrings << valuesPrefix + value + valuesSuffix;
		}
		break;
	}
	case XYCurve::ValuesType::CustomColumn: {
		if (!valuesColumn) {
			recalcShapeAndBoundingRect();
			return;
		}

		const int endRow{qMin(numberOfPoints, valuesColumn->rowCount())};
		auto xColMode{valuesColumn->columnMode()};
		for (int i = 0; i < endRow; ++i) {
			if (!m_pointVisible[i]) continue;

			if ( !valuesColumn->isValid(i) || valuesColumn->isMasked(i) )
				continue;

			switch (xColMode) {
			case AbstractColumn::ColumnMode::Numeric:
				m_valueStrings << valuesPrefix + numberLocale.toString(valuesColumn->valueAt(i), valuesNumericFormat, valuesPrecision) + valuesSuffix;
				break;
			case AbstractColumn::ColumnMode::Integer:
			case AbstractColumn::ColumnMode::BigInt:
				m_valueStrings << valuesPrefix + numberLocale.toString(valuesColumn->valueAt(i)) + valuesSuffix;
				break;
			case AbstractColumn::ColumnMode::Text:
				m_valueStrings << valuesPrefix + valuesColumn->textAt(i) + valuesSuffix;
				break;
			case AbstractColumn::ColumnMode::DateTime:
			case AbstractColumn::ColumnMode::Month:
			case AbstractColumn::ColumnMode::Day:
				m_valueStrings << valuesPrefix + valuesColumn->dateTimeAt(i).toString(valuesDateTimeFormat) + valuesSuffix;
				break;
			}
		}
	}
	}
	m_valueStrings.squeeze();

	//Calculate the coordinates where to paint the value strings.
	//The coordinates depend on the actual size of the string.
	QPointF tempPoint;
	QFontMetrics fm(valuesFont);
	const int h{fm.ascent()};

	i = 0;
	for (const auto& string : qAsConst(m_valueStrings)) {
		const int w{fm.boundingRect(string).width()};
		const double x{m_scenePoints.at(i).x()};
		const double y{m_scenePoints.at(i).y()};
		i++;

		switch (valuesPosition) {
		case XYCurve::ValuesPosition::Above:
			tempPoint = QPointF(x - w/2., y - valuesDistance);
			break;
		case XYCurve::ValuesPosition::Under:
			tempPoint = QPointF(x - w/2., y + valuesDistance + h/2.);
			break;
		case XYCurve::ValuesPosition::Left:
			tempPoint = QPointF(x - valuesDistance - w - 1., y);
			break;
		case XYCurve::ValuesPosition::Right:
			tempPoint = QPointF(x + valuesDistance - 1., y);
			break;
		}
		m_valuePoints.append(tempPoint);
	}
	m_valuePoints.squeeze();

	QTransform trafo;
	QPainterPath path;
	i = 0;
	for (const auto& point : qAsConst(m_valuePoints)) {
		path = QPainterPath();
		path.addText(QPoint(0, 0), valuesFont, m_valueStrings.at(i++));

		trafo.reset();
		trafo.translate(point.x(), point.y());
		if (valuesRotationAngle != 0)
			trafo.rotate(-valuesRotationAngle);

		valuesPath.addPath(trafo.map(path));
	}

	recalcShapeAndBoundingRect();
}

void XYCurvePrivate::updateFilling() {
	if (m_suppressRetransform)
		return;

	m_fillPolygons.clear();

	//don't try to calculate the filling polygons if
	// - no filling was enabled
	// - the number of visible points on the scene is too high
	// - no scene points available, everything outside of the plot region or no scene points calculated yet
	if (fillingPosition == XYCurve::FillingPosition::NoFilling || m_scenePoints.size() > 1000 || m_scenePoints.isEmpty()) {
		recalcShapeAndBoundingRect();
		return;
	}

	QVector<QLineF> fillLines;

	//if there're no interpolation lines available (XYCurve::NoLine selected), create line-interpolation,
	//use already available lines otherwise.
	if (!m_lines.isEmpty())
		fillLines = m_lines;
	else {
		for (int i = 0; i < m_logicalPoints.size() - 1; i++) {
			if (!lineSkipGaps && !connectedPointsLogical[i]) continue;
			fillLines.append(QLineF(m_logicalPoints.at(i), m_logicalPoints.at(i+1)));
		}

		//no lines available (no points), nothing to do
		if (fillLines.isEmpty())
			return;

		fillLines = q->cSystem->mapLogicalToScene(fillLines);

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
	const QPointF& first = m_logicalPoints.at(0); //first point of the curve, may not be visible currently
	const QPointF& last = m_logicalPoints.at(m_logicalPoints.size()-1);//last point of the curve, may not be visible currently
	QPointF edge;
	double xEnd{0.}, yEnd{0.};
	const auto xRange{ plot()->xRange_(q->cSystem->xIndex()) };
	const auto yRange{ plot()->yRange_(q->cSystem->yIndex()) };
	const double xMin{ xRange.start() }, xMax{ xRange.end() };
	const double yMin{ yRange.start() }, yMax{ yRange.end() };
	bool visible;
	if (fillingPosition == XYCurve::FillingPosition::Above) {
		edge = q->cSystem->mapLogicalToScene(QPointF(xMin, yMin), visible);

		//start point
		if (nsl_math_essentially_equal(start.y(), edge.y())) {
			if (first.x() < xMin)
				start = edge;
			else if (first.x() > xMax)
				start = q->cSystem->mapLogicalToScene(QPointF(xMax, yMin), visible);
			else
				start = q->cSystem->mapLogicalToScene(QPointF(first.x(), yMin), visible);
		}

		//end point
		if (nsl_math_essentially_equal(end.y(), edge.y())) {
			if (last.x() < xMin)
				end = edge;
			else if (last.x() > xMax)
				end = q->cSystem->mapLogicalToScene(QPointF(xMax, yMin), visible);
			else
				end = q->cSystem->mapLogicalToScene(QPointF(last.x(), yMin), visible);
		}

		//coordinate at which to close all polygons
		yEnd = q->cSystem->mapLogicalToScene(QPointF(xMin, yMax), visible).y();
	} else if (fillingPosition == XYCurve::FillingPosition::Below) {
		edge = q->cSystem->mapLogicalToScene(QPointF(xMin, yMax), visible);

		//start point
		if (nsl_math_essentially_equal(start.y(), edge.y())) {
			if (first.x() < xMin)
				start = edge;
			else if (first.x() > xMax)
				start = q->cSystem->mapLogicalToScene(QPointF(xMax, yMax), visible);
			else
				start = q->cSystem->mapLogicalToScene(QPointF(first.x(), yMax), visible);
		}

		//end point
		if (nsl_math_essentially_equal(end.y(), edge.y())) {
			if (last.x() < xMin)
				end = edge;
			else if (last.x() > xMax)
				end = q->cSystem->mapLogicalToScene(QPointF(xMax, yMax), visible);
			else
				end = q->cSystem->mapLogicalToScene(QPointF(last.x(), yMax), visible);
		}

		//coordinate at which to close all polygons
		yEnd = q->cSystem->mapLogicalToScene(QPointF(xMin, yMin), visible).y();
	} else if (fillingPosition == XYCurve::FillingPosition::ZeroBaseline) {
		edge = q->cSystem->mapLogicalToScene(QPointF(xMin, yMax), visible);

		//start point
		if (nsl_math_essentially_equal(start.y(), edge.y())) {
			if (yMax > 0) {
				if (first.x() < xMin)
					start = edge;
				else if (first.x() > xRange.end())
					start = q->cSystem->mapLogicalToScene(QPointF(xMax, yMax), visible);
				else
					start = q->cSystem->mapLogicalToScene(QPointF(first.x(), yMax), visible);
			} else {
				if (first.x() < xMin)
					start = edge;
				else if (first.x() > xMax)
					start = q->cSystem->mapLogicalToScene(QPointF(xMax, yMin), visible);
				else
					start = q->cSystem->mapLogicalToScene(QPointF(first.x(), yMin), visible);
			}
		}

		//end point
		if (nsl_math_essentially_equal(end.y(), edge.y())) {
			if (yMax > 0) {
				if (last.x() < xMin)
					end = edge;
				else if (last.x() > xMax)
					end = q->cSystem->mapLogicalToScene(QPointF(xMax, yMax), visible);
				else
					end = q->cSystem->mapLogicalToScene(QPointF(last.x(), yMax), visible);
			} else {
				if (last.x() < xMin)
					end = edge;
				else if (last.x() > xMax)
					end = q->cSystem->mapLogicalToScene(QPointF(xMax, yMin), visible);
				else
					end = q->cSystem->mapLogicalToScene(QPointF(last.x(), yMin), visible);
			}
		}

		yEnd = q->cSystem->mapLogicalToScene(QPointF(xMin, yMin > 0 ? yMin : 0), visible).y();
	} else if (fillingPosition == XYCurve::FillingPosition::Left) {
		edge = q->cSystem->mapLogicalToScene(QPointF(xMax, yMin), visible);

		//start point
		if (nsl_math_essentially_equal(start.x(), edge.x())) {
			if (first.y() < yMin)
				start = edge;
			else if (first.y() > yMax)
				start = q->cSystem->mapLogicalToScene(QPointF(xMax, yMax), visible);
			else
				start = q->cSystem->mapLogicalToScene(QPointF(xMax, first.y()), visible);
		}

		//end point
		if (nsl_math_essentially_equal(end.x(), edge.x())) {
			if (last.y() < yMin)
				end = edge;
			else if (last.y() > yMax)
				end = q->cSystem->mapLogicalToScene(QPointF(xMax, yMax), visible);
			else
				end = q->cSystem->mapLogicalToScene(QPointF(xMax, last.y()), visible);
		}

		//coordinate at which to close all polygons
		xEnd = q->cSystem->mapLogicalToScene(QPointF(xMin, yMin), visible).x();
	} else { //FillingRight
		edge = q->cSystem->mapLogicalToScene(QPointF(xMin, yMin), visible);

		//start point
		if (nsl_math_essentially_equal(start.x(), edge.x())) {
			if (first.y() < yMin)
				start = edge;
			else if (first.y() > yMax)
				start = q->cSystem->mapLogicalToScene(QPointF(xMin, yMax), visible);
			else
				start = q->cSystem->mapLogicalToScene(QPointF(xMin, first.y()), visible);
		}

		//end point
		if (nsl_math_essentially_equal(end.x(), edge.x())) {
			if (last.y() < yMin)
				end = edge;
			else if (last.y() > yMax)
				end = q->cSystem->mapLogicalToScene(QPointF(xMin, yMax), visible);
			else
				end = q->cSystem->mapLogicalToScene(QPointF(xMin, last.y()), visible);
		}

		//coordinate at which to close all polygons
		xEnd = q->cSystem->mapLogicalToScene(QPointF(xMax, yMin), visible).x();
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
				if (fillingPosition == XYCurve::FillingPosition::Above || fillingPosition == XYCurve::FillingPosition::Below || fillingPosition == XYCurve::FillingPosition::ZeroBaseline) {
					pol << QPointF(fillLines.at(i-1).p2().x(), yEnd);
					pol << QPointF(start.x(), yEnd);
				} else {
					pol << QPointF(xEnd, fillLines.at(i-1).p2().y());
					pol << QPointF(xEnd, start.y());
				}

				m_fillPolygons << pol;
				pol.clear();
				start = p1;
			}
		}
		pol << p1 << p2;
	}

	if (p2 != end)
		pol << end;

	//close the last polygon
	if (fillingPosition == XYCurve::FillingPosition::Above || fillingPosition == XYCurve::FillingPosition::Below || fillingPosition == XYCurve::FillingPosition::ZeroBaseline) {
		pol << QPointF(end.x(), yEnd);
		pol << QPointF(start.x(), yEnd);
	} else {
		pol << QPointF(xEnd, end.y());
		pol << QPointF(xEnd, start.y());
	}

	m_fillPolygons << pol;
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
		return qQNaN();
	}

	auto yColumnMode = yColumn()->columnMode();
	const int index = xColumn()->indexForValue(x);
	if (index < 0) {
		valueFound = false;
		return qQNaN();
	}

	valueFound = true;
	if (yColumnMode == AbstractColumn::ColumnMode::Numeric || yColumnMode == AbstractColumn::ColumnMode::Integer ||
            yColumnMode == AbstractColumn::ColumnMode::BigInt)
		return yColumn()->valueAt(index);
	else {
		valueFound = false;
		return qQNaN();
	}
}

/*!
 * @param x :value for which y should be found
 * @param valueFound: returns true if y value found, otherwise false
 * @param x_new: exact x value where y value is
 * @return y value from x value
 */
double XYCurve::y(double x, double &x_new, bool &valueFound) const {
	AbstractColumn::ColumnMode yColumnMode = yColumn()->columnMode();
	int index = xColumn()->indexForValue(x);
	if (index < 0) {
		valueFound = false;
		return qQNaN();
	}

	AbstractColumn::ColumnMode xColumnMode = xColumn()->columnMode();
	if (xColumnMode == AbstractColumn::ColumnMode::Numeric ||
			xColumnMode == AbstractColumn::ColumnMode::Integer)
		x_new = xColumn()->valueAt(index);
	else if(xColumnMode == AbstractColumn::ColumnMode::DateTime ||
			xColumnMode == AbstractColumn::ColumnMode::Day ||
			xColumnMode == AbstractColumn::ColumnMode::Month)
		x_new = xColumn()->dateTimeAt(index).toMSecsSinceEpoch();
	else {
		// any other type implemented
		valueFound = false;
		return qQNaN();
	}


	valueFound = true;
	if (yColumnMode == AbstractColumn::ColumnMode::Numeric ||
			yColumnMode == AbstractColumn::ColumnMode::Integer)
		return yColumn()->valueAt(index);
	else {
		valueFound = false;
		return qQNaN();
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

	auto yColumnMode = yColumn()->columnMode();
	const int index = xColumn()->indexForValue(x);
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

bool XYCurve::minMaxX(const Range<int>& indexRange, Range<double>& xRange, bool includeErrorBars) const {
	DEBUG(Q_FUNC_INFO << ", x column max = " << xColumn()->maximum())
	return minMax(xColumn(), yColumn(), xErrorType(), xErrorPlusColumn(), xErrorMinusColumn(), indexRange, xRange, includeErrorBars);
}

bool XYCurve::minMaxY(const Range<int>& indexRange, Range<double>& yRange, bool includeErrorBars) const {
	return minMax(yColumn(), xColumn(), yErrorType(), yErrorPlusColumn(), yErrorMinusColumn(), indexRange, yRange, includeErrorBars);
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
bool XYCurve::minMax(const AbstractColumn* column1, const AbstractColumn* column2, const ErrorType errorType, const AbstractColumn* errorPlusColumn, const AbstractColumn* errorMinusColumn, const Range<int>& indexRange, Range<double>& range, bool includeErrorBars) const {
	// when property is increasing or decreasing there is a benefit in finding minimum and maximum
	// for property == AbstractColumn::Properties::No it must be iterated over all values so it does not matter if this function or the below one is used
	// if the property of the second column is not AbstractColumn::Properties::No means, that all values are valid and not masked
	DEBUG(Q_FUNC_INFO << std::endl << ", x column 1 min/max = " << column1->minimum() << "/" << column1->maximum())
	if ((!includeErrorBars || errorType == ErrorType::NoError) && column1->properties() != AbstractColumn::Properties::No && column2 && column2->properties() != AbstractColumn::Properties::No) {
		DEBUG( Q_FUNC_INFO << std::endl << ", column 1 min/max = " << column1->minimum(indexRange.start(), indexRange.end()) << "/"
		      << column1->maximum(indexRange.start(), indexRange.end()) )
		//TODO: Range
		range.setRange( column1->minimum(indexRange.start(), indexRange.end()), column1->maximum(indexRange.start(), indexRange.end()) );
		return true;
	}

	if (column1->rowCount() == 0)
		return false;

	range.setRange(qInf(), -qInf());
	DEBUG(Q_FUNC_INFO << ", calculate range from " << indexRange.start() << " to " << indexRange.end())

	for (int i = indexRange.start(); i < indexRange.end(); ++i) {
		//DEBUG(Q_FUNC_INFO << ", i = " << (int)i)
		if (!column1->isValid(i) || column1->isMasked(i) || (column2 && (!column2->isValid(i) || column2->isMasked(i))))
			continue;

		if ( (errorPlusColumn && i >= errorPlusColumn->rowCount())
				|| (errorMinusColumn && i >= errorMinusColumn->rowCount()) )
			continue;

		double value;
		if (column1->columnMode() == AbstractColumn::ColumnMode::Numeric || column1->columnMode() == AbstractColumn::ColumnMode::Integer ||
				column1->columnMode() == AbstractColumn::ColumnMode::BigInt)
			value = column1->valueAt(i);
		else if (column1->columnMode() == AbstractColumn::ColumnMode::DateTime ||
				 column1->columnMode() == AbstractColumn::ColumnMode::Month ||
				 column1->columnMode() == AbstractColumn::ColumnMode::Day)
			value = column1->dateTimeAt(i).toMSecsSinceEpoch();
		else
			return false;

		if (errorType == ErrorType::NoError) {
			if (value < range.start())
				range.start() = value;

			if (value > range.end())
				range.end() = value;
		} else {
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

			if (errorType == ErrorType::Symmetric)
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

			if (value - errorMinus < range.start())
				range.start() = value - errorMinus;

			if (value + errorPlus > range.end())
				range.end() = value + errorPlus;
		}
		//DEBUG(Q_FUNC_INFO << ", range = " << range.toStdString())
	}
	return true;
}

bool XYCurvePrivate::activateCurve(QPointF mouseScenePos, double maxDist) {
	if (!isVisible())
		return false;

	int rowCount{0};
	if (lineType != XYCurve::LineType::NoLine)
		rowCount = m_lines.count();
	else if (symbol->style() != Symbol::Style::NoSymbols)
		rowCount = m_scenePoints.size();
	else
		return false;

	if (rowCount == 0)
		return false;

	if (maxDist < 0)
		maxDist = (linePen.width() < 10) ? 10. : linePen.width();

	auto properties{q->xColumn()->properties()};
	if (properties == AbstractColumn::Properties::No) {
		// assumption: points exist if no line. otherwise previously returned false
		if (lineType == XYCurve::LineType::NoLine) {
			QPointF curvePosPrevScene = m_scenePoints.at(0);
			QPointF curvePosScene = curvePosPrevScene;
			for (int row = 0; row < rowCount; row ++) {
				if (gsl_hypot(mouseScenePos.x() - curvePosScene.x(), mouseScenePos.y() - curvePosScene.y()) <= maxDist)
					return true;

				curvePosPrevScene = curvePosScene;
				curvePosScene = m_scenePoints.at(row);
			}
		} else {
			for (int row = 0; row < rowCount; row++) {
				QLineF line = m_lines.at(row);
				if (pointLiesNearLine(line.p1(), line.p2(), mouseScenePos, maxDist))
					return true;
			}
		}

	} else if (properties == AbstractColumn::Properties::MonotonicIncreasing ||
			   properties == AbstractColumn::Properties::MonotonicDecreasing) {

		bool increase{true};
		if (properties == AbstractColumn::Properties::MonotonicDecreasing)
			increase = false;

		double x{mouseScenePos.x() - maxDist};
		int index{0};

		QPointF curvePosScene;
		QPointF curvePosPrevScene;

		if (lineType == XYCurve::LineType::NoLine) {
			curvePosScene  = m_scenePoints.at(index);
			curvePosPrevScene = curvePosScene;
			index = Column::indexForValue(x, m_scenePoints, static_cast<AbstractColumn::Properties>(properties));
		} else
			index = Column::indexForValue(x, m_lines, static_cast<AbstractColumn::Properties>(properties));

		if (index >= 1)
			index --; // use one before so it is secured that I'm before point.x()
		else if (index == -1)
			return false;

		const double xMax{mouseScenePos.x() + maxDist};
		bool stop{false};
		while (true) {
			// assumption: points exist if no line. otherwise previously returned false
			if (lineType == XYCurve::LineType::NoLine) {// check points only if no line otherwise check only the lines
				if (curvePosScene.x() > xMax)
					stop = true; // one more time if bigger
				if (gsl_hypot(mouseScenePos.x()- curvePosScene.x(), mouseScenePos.y()-curvePosScene.y()) <= maxDist)
					return true;
			} else {
				if (m_lines.at(index).p1().x() > xMax)
					stop = true; // one more time if bigger

				QLineF line = m_lines.at(index);
				if (pointLiesNearLine(line.p1(), line.p2(), mouseScenePos, maxDist))
					return true;
			}

			if (stop || (index >= rowCount - 1 && increase) || (index <= 0 && !increase))
				break;

			if (increase)
				index++;
			else
				index--;

			if (lineType == XYCurve::LineType::NoLine) {
				curvePosPrevScene = curvePosScene;
				curvePosScene = m_scenePoints.at(index);
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
bool XYCurvePrivate::pointLiesNearLine(const QPointF p1, const QPointF p2, const QPointF pos, const double maxDist) const {
	const double dx12{p2.x() - p1.x()};
	const double dy12{p2.y() - p1.y()};
	const double vecLength{gsl_hypot(dx12, dy12)};

	const double dx1m{pos.x() - p1.x()};
	const double dy1m{pos.y() - p1.y()};
	if (vecLength == 0) {
		if (gsl_hypot(dx1m, dy1m) <= maxDist)
			return true;
		return false;
	}
	QPointF unitvec(dx12/vecLength, dy12/vecLength);

	const double dist_segm{std::abs(dx1m*unitvec.y() - dy1m*unitvec.x())};
	const double scalarProduct{dx1m*unitvec.x() + dy1m*unitvec.y()};

	if (scalarProduct > 0) {
		if (scalarProduct < vecLength && dist_segm < maxDist)
			return true;
	}
	return false;
}

// TODO: curvePosScene.x() >= mouseScenePos.x() &&
// curvePosPrevScene.x() < mouseScenePos.x()
// should not be here
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
			for (int i = 0; i < q->lineInterpolationPointsCount() + 1; i++) {
				QLineF line = m_lines.at(index*(q->lineInterpolationPointsCount()+1)+i);
				QPointF p1{line.p1()}; //cSystem->mapLogicalToScene(line.p1());
				QPointF p2{line.p2()}; //cSystem->mapLogicalToScene(line.p2());
				if (pointLiesNearLine(p1, p2, mouseScenePos, maxDist))
					return true;
			}
		} else {
			// point is not in the near of the point, but it can be in the near of the connection line of two points
			if (pointLiesNearLine(curvePosPrevScene, curvePosScene, mouseScenePos, maxDist))
				return true;
		}
	}
	return false;
}

void XYCurvePrivate::updateErrorBars() {
	errorBarsPath = QPainterPath();
	if (xErrorType == XYCurve::ErrorType::NoError && yErrorType == XYCurve::ErrorType::NoError) {
		recalcShapeAndBoundingRect();
		return;
	}

	QVector<QLineF> elines;
	QVector<QPointF> pointsErrorBarAnchorX;
	QVector<QPointF> pointsErrorBarAnchorY;

	for (int i = 0; i < m_logicalPoints.size(); ++i) {
		if (!m_pointVisible.at(i))
			continue;

		const QPointF& point{m_logicalPoints.at(i)};
		const int index{validPointsIndicesLogical.at(i)};
		double errorPlus, errorMinus;

		//error bars for x
		if (xErrorType != XYCurve::ErrorType::NoError) {
			//determine the values for the errors
			if (xErrorPlusColumn && xErrorPlusColumn->isValid(index) && !xErrorPlusColumn->isMasked(index))
				errorPlus = xErrorPlusColumn->valueAt(index);
			else
				errorPlus = 0;

			if (xErrorType == XYCurve::ErrorType::Symmetric)
				errorMinus = errorPlus;
			else {
				if (xErrorMinusColumn && xErrorMinusColumn->isValid(index) && !xErrorMinusColumn->isMasked(index))
					errorMinus = xErrorMinusColumn->valueAt(index);
				else
					errorMinus = 0;
			}

			//draw the error bars
			if (errorMinus != 0 || errorPlus != 0)
				elines.append(QLineF(QPointF(point.x()-errorMinus, point.y()),
									QPointF(point.x()+errorPlus, point.y())));

			//determine the end points of the errors bars in logical coordinates to draw later the cap
			if (errorBarsType == XYCurve::ErrorBarsType::WithEnds) {
				if (errorMinus != 0)
					pointsErrorBarAnchorX << QPointF(point.x() - errorMinus, point.y());
				if (errorPlus != 0)
					pointsErrorBarAnchorX << QPointF(point.x() + errorPlus, point.y());
			}
		}

		//error bars for y
		if (yErrorType != XYCurve::ErrorType::NoError) {
			//determine the values for the errors
			if (yErrorPlusColumn && yErrorPlusColumn->isValid(index) && !yErrorPlusColumn->isMasked(index))
				errorPlus = yErrorPlusColumn->valueAt(index);
			else
				errorPlus = 0;

			if (yErrorType == XYCurve::ErrorType::Symmetric)
				errorMinus = errorPlus;
			else {
				if (yErrorMinusColumn && yErrorMinusColumn->isValid(index) && !yErrorMinusColumn->isMasked(index) )
					errorMinus = yErrorMinusColumn->valueAt(index);
				else
					errorMinus = 0;
			}

			//draw the error bars
			if (errorMinus != 0 || errorPlus != 0)
				elines.append(QLineF(QPointF(point.x(), point.y() + errorPlus),
									QPointF(point.x(), point.y() - errorMinus)));

			//determine the end points of the errors bars in logical coordinates to draw later the cap
			if (errorBarsType == XYCurve::ErrorBarsType::WithEnds) {
				if (errorMinus != 0)
					pointsErrorBarAnchorY << QPointF(point.x(), point.y() + errorPlus);
				if (errorPlus != 0)
					pointsErrorBarAnchorY << QPointF(point.x(), point.y() - errorMinus);
			}
		}
	}

	//map the error bars to scene coordinates
	elines = q->cSystem->mapLogicalToScene(elines);

	//new painter path for the error bars
	for (const auto& line : qAsConst(elines)) {
		errorBarsPath.moveTo(line.p1());
		errorBarsPath.lineTo(line.p2());
	}

	//add caps for x error bars
	if (!pointsErrorBarAnchorX.isEmpty()) {
		pointsErrorBarAnchorX = q->cSystem->mapLogicalToScene(pointsErrorBarAnchorX);
		for (const auto& point : qAsConst(pointsErrorBarAnchorX)) {
			errorBarsPath.moveTo(QPointF(point.x(), point.y() - errorBarsCapSize/2.));
			errorBarsPath.lineTo(QPointF(point.x(), point.y() + errorBarsCapSize/2.));
		}
	}

	//add caps for y error bars
	if (!pointsErrorBarAnchorY.isEmpty()) {
		pointsErrorBarAnchorY = q->cSystem->mapLogicalToScene(pointsErrorBarAnchorY);
		for (const auto& point : qAsConst(pointsErrorBarAnchorY)) {
			errorBarsPath.moveTo(QPointF(point.x() - errorBarsCapSize/2., point.y()));
			errorBarsPath.lineTo(QPointF(point.x() + errorBarsCapSize/2., point.y()));
		}
	}

	recalcShapeAndBoundingRect();
}

/*!
  recalculates the outer bounds and the shape of the curve.
*/
void XYCurvePrivate::recalcShapeAndBoundingRect() {
	DEBUG(Q_FUNC_INFO << ", m_suppressRecalc = " << m_suppressRecalc);
	if (m_suppressRecalc)
		return;

#ifdef PERFTRACE_CURVES
	PERFTRACE(name().toLatin1() + ", XYCurvePrivate::recalcShapeAndBoundingRect()");
#endif

	prepareGeometryChange();
	curveShape = QPainterPath();
	if (lineType != XYCurve::LineType::NoLine)
		curveShape.addPath(WorksheetElement::shapeFromPath(linePath, linePen));

	if (dropLineType != XYCurve::DropLineType::NoDropLine)
		curveShape.addPath(WorksheetElement::shapeFromPath(dropLinePath, dropLinePen));

	if (symbol->style() != Symbol::Style::NoSymbols)
		curveShape.addPath(symbolsPath);

	if (valuesType != XYCurve::ValuesType::NoValues)
		curveShape.addPath(valuesPath);

	if (xErrorType != XYCurve::ErrorType::NoError || yErrorType != XYCurve::ErrorType::NoError)
		curveShape.addPath(WorksheetElement::shapeFromPath(errorBarsPath, errorBarsPen));

	boundingRectangle = curveShape.boundingRect();

	for (const auto& pol : qAsConst(m_fillPolygons))
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
	if (fillingPosition != XYCurve::FillingPosition::NoFilling) {
		painter->setOpacity(fillingOpacity);
		painter->setPen(Qt::SolidLine);
		drawFilling(painter);
	}

	//draw lines
	if (lineType != XYCurve::LineType::NoLine) {
		painter->setOpacity(lineOpacity);
		painter->setPen(linePen);
		painter->setBrush(Qt::NoBrush);
		painter->drawPath(linePath);
	}

	//draw drop lines
	if (dropLineType != XYCurve::DropLineType::NoDropLine) {
		painter->setOpacity(dropLineOpacity);
		painter->setPen(dropLinePen);
		painter->setBrush(Qt::NoBrush);
		painter->drawPath(dropLinePath);
	}

	//draw error bars
	if ( (xErrorType != XYCurve::ErrorType::NoError) || (yErrorType != XYCurve::ErrorType::NoError) ) {
		painter->setOpacity(errorBarsOpacity);
		painter->setPen(errorBarsPen);
		painter->setBrush(Qt::NoBrush);
		painter->drawPath(errorBarsPath);
	}

	//draw symbols
	if (symbol->style() != Symbol::Style::NoSymbols) {
		painter->setOpacity(symbol->opacity());
		painter->setPen(symbol->pen());
		painter->setBrush(symbol->brush());
		drawSymbols(painter);
	}

	//draw values
	if (valuesType != XYCurve::ValuesType::NoValues) {
		painter->setOpacity(valuesOpacity);
		painter->setPen(QPen(valuesColor));
		painter->setFont(valuesFont);
		drawValues(painter);
	}
}

void XYCurvePrivate::updatePixmap() {
	DEBUG(Q_FUNC_INFO << ", m_suppressRecalc = " << m_suppressRecalc);
	if (m_suppressRecalc)
		return;

	WAIT_CURSOR;

	m_hoverEffectImageIsDirty = true;
	m_selectionEffectImageIsDirty = true;
	if (boundingRectangle.width() == 0 || boundingRectangle.height() == 0) {
		DEBUG(Q_FUNC_INFO << ", boundingRectangle.width() or boundingRectangle.height() == 0");
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

QVariant XYCurvePrivate::itemChange(GraphicsItemChange change, const QVariant & value) {

	// signalize, that the curve was selected. Will be used to create a new InfoElement (Marker)
	if (change == QGraphicsItem::ItemSelectedChange)
		if (value.toBool() && q->cSystem)
			emit q->selected(q->cSystem->mapSceneToLogical(mousePos).x());
	return QGraphicsItem::itemChange(change,value);
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

	if ( !q->isPrinting() && KSharedConfig::openConfig()->group("Settings_Worksheet").readEntry<bool>("DoubleBuffering", true) )
		painter->drawPixmap(boundingRectangle.topLeft(), m_pixmap); //draw the cached pixmap (fast)
	else
		draw(painter); //draw directly again (slow)


	if (m_hovered && !isSelected() && !q->isPrinting()) {
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

	if (isSelected() && !q->isPrinting()) {
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
	QPainterPath path = Symbol::pathFromStyle(symbol->style());

	QTransform trafo;
	trafo.scale(symbol->size(), symbol->size());

	if (symbol->rotationAngle() != 0)
		trafo.rotate(-symbol->rotationAngle());

	path = trafo.map(path);

	for (const auto& point : qAsConst(m_scenePoints)) {
		trafo.reset();
		trafo.translate(point.x(), point.y());
		painter->drawPath(trafo.map(path));
	}
}

void XYCurvePrivate::drawValues(QPainter* painter) {
	int i = 0;
	for (const auto& point : qAsConst(m_valuePoints)) {
		painter->translate(point);
		if (valuesRotationAngle != 0)
			painter->rotate(-valuesRotationAngle);

		painter->drawText(QPoint(0, 0), m_valueStrings.at(i++));

		if (valuesRotationAngle != 0)
			painter->rotate(valuesRotationAngle);
		painter->translate(-point);
	}
}

void XYCurvePrivate::drawFilling(QPainter* painter) {
	for (const auto& pol : qAsConst(m_fillPolygons)) {
		QRectF rect = pol.boundingRect();
		if (fillingType == PlotArea::BackgroundType::Color) {
			switch (fillingColorStyle) {
			case PlotArea::BackgroundColorStyle::SingleColor: {
					painter->setBrush(QBrush(fillingFirstColor));
					break;
				}
			case PlotArea::BackgroundColorStyle::HorizontalLinearGradient: {
					QLinearGradient linearGrad(rect.topLeft(), rect.topRight());
					linearGrad.setColorAt(0, fillingFirstColor);
					linearGrad.setColorAt(1, fillingSecondColor);
					painter->setBrush(QBrush(linearGrad));
					break;
				}
			case PlotArea::BackgroundColorStyle::VerticalLinearGradient: {
					QLinearGradient linearGrad(rect.topLeft(), rect.bottomLeft());
					linearGrad.setColorAt(0, fillingFirstColor);
					linearGrad.setColorAt(1, fillingSecondColor);
					painter->setBrush(QBrush(linearGrad));
					break;
				}
			case PlotArea::BackgroundColorStyle::TopLeftDiagonalLinearGradient: {
					QLinearGradient linearGrad(rect.topLeft(), rect.bottomRight());
					linearGrad.setColorAt(0, fillingFirstColor);
					linearGrad.setColorAt(1, fillingSecondColor);
					painter->setBrush(QBrush(linearGrad));
					break;
				}
			case PlotArea::BackgroundColorStyle::BottomLeftDiagonalLinearGradient: {
					QLinearGradient linearGrad(rect.bottomLeft(), rect.topRight());
					linearGrad.setColorAt(0, fillingFirstColor);
					linearGrad.setColorAt(1, fillingSecondColor);
					painter->setBrush(QBrush(linearGrad));
					break;
				}
			case PlotArea::BackgroundColorStyle::RadialGradient: {
					QRadialGradient radialGrad(rect.center(), rect.width()/2);
					radialGrad.setColorAt(0, fillingFirstColor);
					radialGrad.setColorAt(1, fillingSecondColor);
					painter->setBrush(QBrush(radialGrad));
					break;
				}
			}
		} else if (fillingType == PlotArea::BackgroundType::Image) {
			if ( !fillingFileName.trimmed().isEmpty() ) {
				QPixmap pix(fillingFileName);
				switch (fillingImageStyle) {
				case PlotArea::BackgroundImageStyle::ScaledCropped:
					pix = pix.scaled(rect.size().toSize(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
					painter->setBrush(QBrush(pix));
					painter->setBrushOrigin(pix.size().width()/2, pix.size().height()/2);
					break;
				case PlotArea::BackgroundImageStyle::Scaled:
					pix = pix.scaled(rect.size().toSize(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
					painter->setBrush(QBrush(pix));
					painter->setBrushOrigin(pix.size().width()/2, pix.size().height()/2);
					break;
				case PlotArea::BackgroundImageStyle::ScaledAspectRatio:
					pix = pix.scaled(rect.size().toSize(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
					painter->setBrush(QBrush(pix));
					painter->setBrushOrigin(pix.size().width()/2, pix.size().height()/2);
					break;
				case PlotArea::BackgroundImageStyle::Centered: {
						QPixmap backpix(rect.size().toSize());
						backpix.fill();
						QPainter p(&backpix);
						p.drawPixmap(QPointF(0, 0), pix);
						p.end();
						painter->setBrush(QBrush(backpix));
						painter->setBrushOrigin(-pix.size().width()/2, -pix.size().height()/2);
						break;
					}
				case PlotArea::BackgroundImageStyle::Tiled:
					painter->setBrush(QBrush(pix));
					break;
				case PlotArea::BackgroundImageStyle::CenterTiled:
					painter->setBrush(QBrush(pix));
					painter->setBrushOrigin(pix.size().width()/2, pix.size().height()/2);
				}
			}
		} else if (fillingType == PlotArea::BackgroundType::Pattern)
			painter->setBrush(QBrush(fillingFirstColor, fillingBrushStyle));

		painter->drawPolygon(pol);
	}
}

void XYCurvePrivate::suppressRetransform(bool on) {
	m_suppressRetransform = on;
	m_suppressRecalc = on;
}

/*!
 * checks if the mousePress event was done near the histogram shape
 * and selects the graphics item if it is the case.
 * \p event
 */
void XYCurvePrivate::mousePressEvent(QGraphicsSceneMouseEvent* event) {
	if (plot()->mouseMode() != CartesianPlot::MouseMode::Selection) {
		event->ignore();
		return QGraphicsItem::mousePressEvent(event);
	}
	mousePos = event->pos();

	if(q->activateCurve(event->pos())) {
		setSelected(true);
		return;
	}

	event->ignore();
	setSelected(false);
	QGraphicsItem::mousePressEvent(event);
}

/*!
 * Is called in CartesianPlot::hoverMoveEvent where it is determined which curve to hover.
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
	writer->writeAttribute("xColumn", d->xColumnPath);
	writer->writeAttribute("yColumn", d->yColumnPath);
	writer->writeAttribute( "plotRangeIndex", QString::number(m_cSystemIndex) );
	writer->writeAttribute( "visible", QString::number(d->isVisible()) );
	writer->writeEndElement();

	//Line
	writer->writeStartElement( "lines" );
	writer->writeAttribute( "type", QString::number(static_cast<int>(d->lineType)) );
	writer->writeAttribute( "skipGaps", QString::number(d->lineSkipGaps) );
	writer->writeAttribute( "increasingXOnly", QString::number(d->lineIncreasingXOnly) );
	writer->writeAttribute( "interpolationPointsCount", QString::number(d->lineInterpolationPointsCount) );
	WRITE_QPEN(d->linePen);
	writer->writeAttribute( "opacity", QString::number(d->lineOpacity) );
	writer->writeEndElement();

	//Drop lines
	writer->writeStartElement( "dropLines" );
	writer->writeAttribute( "type", QString::number(static_cast<int>(d->dropLineType)) );
	WRITE_QPEN(d->dropLinePen);
	writer->writeAttribute( "opacity", QString::number(d->dropLineOpacity) );
	writer->writeEndElement();

	//Symbols
	d->symbol->save(writer);

	//Values
	writer->writeStartElement( "values" );
	writer->writeAttribute( "type", QString::number(static_cast<int>(d->valuesType)) );
	writer->writeAttribute("valuesColumn", d->valuesColumnPath);
	writer->writeAttribute( "position", QString::number(static_cast<int>(d->valuesPosition)) );
	writer->writeAttribute( "distance", QString::number(d->valuesDistance) );
	writer->writeAttribute( "rotation", QString::number(d->valuesRotationAngle) );
	writer->writeAttribute( "opacity", QString::number(d->valuesOpacity) );
	writer->writeAttribute("numericFormat", QString(d->valuesNumericFormat));
	writer->writeAttribute("dateTimeFormat", d->valuesDateTimeFormat);
	writer->writeAttribute( "precision", QString::number(d->valuesPrecision) );
	writer->writeAttribute( "prefix", d->valuesPrefix );
	writer->writeAttribute( "suffix", d->valuesSuffix );
	WRITE_QCOLOR(d->valuesColor);
	WRITE_QFONT(d->valuesFont);
	writer->writeEndElement();

	//Filling
	writer->writeStartElement( "filling" );
	writer->writeAttribute( "position", QString::number(static_cast<int>(d->fillingPosition)) );
	writer->writeAttribute( "type", QString::number(static_cast<int>(d->fillingType)) );
	writer->writeAttribute( "colorStyle", QString::number(static_cast<int>(d->fillingColorStyle)) );
	writer->writeAttribute( "imageStyle", QString::number(static_cast<int>(d->fillingImageStyle)) );
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
	writer->writeAttribute( "xErrorType", QString::number(static_cast<int>(d->xErrorType)) );
	writer->writeAttribute("xErrorPlusColumn", d->xErrorPlusColumnPath);
	writer->writeAttribute("xErrorMinusColumn", d->xErrorMinusColumnPath);
	writer->writeAttribute( "yErrorType", QString::number(static_cast<int>(d->yErrorType)) );
	writer->writeAttribute("yErrorPlusColumn", d->yErrorPlusColumnPath);
	writer->writeAttribute("yErrorMinusColumn", d->yErrorMinusColumnPath);
	writer->writeAttribute( "type", QString::number(static_cast<int>(d->errorBarsType)) );
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
			READ_INT_VALUE_DIRECT("plotRangeIndex", m_cSystemIndex, int);
		} else if (!preview && reader->name() == "lines") {
			attribs = reader->attributes();

			READ_INT_VALUE("type", lineType, LineType);
			READ_INT_VALUE("skipGaps", lineSkipGaps, bool);
			READ_INT_VALUE("increasingXOnly", lineIncreasingXOnly, bool);
			READ_INT_VALUE("interpolationPointsCount", lineInterpolationPointsCount, int);
			READ_QPEN(d->linePen);
			READ_DOUBLE_VALUE("opacity", lineOpacity);
		} else if (!preview && reader->name() == "dropLines") {
			attribs = reader->attributes();

			READ_INT_VALUE("type", dropLineType, DropLineType);
			READ_QPEN(d->dropLinePen);
			READ_DOUBLE_VALUE("opacity", dropLineOpacity);

		} else if (!preview && reader->name() == "symbols") {
			d->symbol->load(reader, preview);
		} else if (!preview && reader->name() == "values") {
			attribs = reader->attributes();

			READ_INT_VALUE("type", valuesType, ValuesType);
			READ_COLUMN(valuesColumn);

			READ_INT_VALUE("position", valuesPosition, ValuesPosition);
			READ_DOUBLE_VALUE("distance", valuesDistance);
			READ_DOUBLE_VALUE("rotation", valuesRotationAngle);
			READ_DOUBLE_VALUE("opacity", valuesOpacity);

			str = attribs.value("numericFormat").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("numericFormat").toString());
			else
				d->valuesNumericFormat = *(str.toLatin1().data());

			READ_STRING_VALUE("dateTimeFormat", valuesDateTimeFormat);
			READ_INT_VALUE("precision", valuesPrecision, int);

			//don't produce any warning if no prefix or suffix is set (empty string is allowed here in xml)
			d->valuesPrefix = attribs.value("prefix").toString();
			d->valuesSuffix = attribs.value("suffix").toString();

			READ_QCOLOR(d->valuesColor);
			READ_QFONT(d->valuesFont);
		} else if (!preview && reader->name() == "filling") {
			attribs = reader->attributes();

			READ_INT_VALUE("position", fillingPosition, FillingPosition);
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

			READ_INT_VALUE("xErrorType", xErrorType, ErrorType);
			READ_COLUMN(xErrorPlusColumn);
			READ_COLUMN(xErrorMinusColumn);

			READ_INT_VALUE("yErrorType", yErrorType, ErrorType);
			READ_COLUMN(yErrorPlusColumn);
			READ_COLUMN(yErrorMinusColumn);

			READ_INT_VALUE("type", errorBarsType, ErrorBarsType);
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

	const auto* plot = dynamic_cast<const CartesianPlot*>(parentAspect());
	int index = plot->curveChildIndex(this);
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
	p.setWidthF(group.readEntry("LineWidth", Worksheet::convertToSceneUnits(1.0, Worksheet::Unit::Point)));
	p.setColor(themeColor);
	this->setLinePen(p);
	this->setLineOpacity(group.readEntry("LineOpacity", 1.0));

	//Drop line
	p.setStyle((Qt::PenStyle)group.readEntry("DropLineStyle", (int)Qt::SolidLine));
	p.setWidthF(group.readEntry("DropLineWidth", Worksheet::convertToSceneUnits(1.0, Worksheet::Unit::Point)));
	p.setColor(themeColor);
	this->setDropLinePen(p);
	this->setDropLineOpacity(group.readEntry("DropLineOpacity", 1.0));

	//Symbol
	d->symbol->loadThemeConfig(group, themeColor);

	//Values
	this->setValuesOpacity(group.readEntry("ValuesOpacity", 1.0));
	this->setValuesColor(group.readEntry("ValuesColor", themeColor));

	//Filling
	this->setFillingBrushStyle((Qt::BrushStyle)group.readEntry("FillingBrushStyle", (int)Qt::SolidPattern));
	this->setFillingColorStyle((PlotArea::BackgroundColorStyle)group.readEntry("FillingColorStyle", static_cast<int>(PlotArea::BackgroundColorStyle::SingleColor)));
	this->setFillingOpacity(group.readEntry("FillingOpacity", 1.0));
	this->setFillingPosition((FillingPosition)group.readEntry("FillingPosition", static_cast<int>(FillingPosition::NoFilling)));
	this->setFillingFirstColor(themeColor);
	this->setFillingSecondColor(group.readEntry("FillingSecondColor", QColor(Qt::black)));
	this->setFillingType((PlotArea::BackgroundType)group.readEntry("FillingType", static_cast<int>(PlotArea::BackgroundType::Color)));

	//Error Bars
	p.setStyle((Qt::PenStyle)group.readEntry("ErrorBarsStyle", (int)Qt::SolidLine));
	p.setWidthF(group.readEntry("ErrorBarsWidth", Worksheet::convertToSceneUnits(1.0, Worksheet::Unit::Point)));
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
	Q_D(const XYCurve);
	d->symbol->saveThemeConfig(group);

	//Values
	group.writeEntry("ValuesOpacity", this->valuesOpacity());
	group.writeEntry("ValuesColor", (QColor) this->valuesColor());
	group.writeEntry("ValuesFont", this->valuesFont());

	int index = parentAspect()->indexOfChild<XYCurve>(this);
	if (index < 5) {
		KConfigGroup themeGroup = config.group("Theme");
		for (int i = index; i < 5; i++) {
			QString s = "ThemePaletteColor" + QString::number(i+1);
			themeGroup.writeEntry(s, (QColor)this->linePen().color());
		}
	}
}
