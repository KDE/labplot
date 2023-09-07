/*
	File                 : XYCurve.cpp
	Project              : LabPlot
	Description          : A xy-curve
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2010-2023 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2013-2021 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

/*!
  \class XYCurve
  \brief A 2D-curve, provides an interface for editing many properties of the curve.

  \ingroup worksheet
*/

#include "XYCurve.h"
#include "XYCurvePrivate.h"
#include "backend/core/AbstractColumn.h"
#include "backend/core/Project.h"
#include "backend/core/Settings.h"
#include "backend/core/column/Column.h"
#include "backend/gsl/errors.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/macrosCurve.h"
#include "backend/lib/trace.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/worksheet/Background.h"
#include "backend/worksheet/Line.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/CartesianCoordinateSystem.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/worksheet/plots/cartesian/Symbol.h"
#include "tools/ImageTools.h"

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>

#include <QDesktopWidget>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QPainter>

#include <gsl/gsl_errno.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_spline.h>

using Dimension = CartesianCoordinateSystem::Dimension;

CURVE_COLUMN_CONNECT(XYCurve, X, x, recalcLogicalPoints)
CURVE_COLUMN_CONNECT(XYCurve, Y, y, recalcLogicalPoints)
CURVE_COLUMN_CONNECT(XYCurve, XErrorPlus, xErrorPlus, recalcLogicalPoints)
CURVE_COLUMN_CONNECT(XYCurve, XErrorMinus, xErrorMinus, recalcLogicalPoints)
CURVE_COLUMN_CONNECT(XYCurve, YErrorPlus, yErrorPlus, recalcLogicalPoints)
CURVE_COLUMN_CONNECT(XYCurve, YErrorMinus, yErrorMinus, recalcLogicalPoints)
CURVE_COLUMN_CONNECT(XYCurve, Values, values, recalcLogicalPoints)

XYCurve::XYCurve(const QString& name, AspectType type)
	: Plot(name, new XYCurvePrivate(this), type) {
	init();
}

XYCurve::XYCurve(const QString& name, XYCurvePrivate* dd, AspectType type)
	: Plot(name, dd, type) {
	init();
}

// no need to delete the d-pointer here - it inherits from QGraphicsItem
// and is deleted during the cleanup in QGraphicsScene
XYCurve::~XYCurve() = default;

void XYCurve::init() {
	Q_D(XYCurve);

	KConfig config;
	KConfigGroup group = config.group("XYCurve");

	d->legendVisible = group.readEntry("LegendVisible", true);

	d->lineType = (LineType)group.readEntry("LineType", static_cast<int>(LineType::Line));
	d->lineIncreasingXOnly = group.readEntry("LineIncreasingXOnly", false);
	d->lineSkipGaps = group.readEntry("SkipLineGaps", false);
	d->lineInterpolationPointsCount = group.readEntry("LineInterpolationPointsCount", 1);

	d->line = new Line(QString());
	d->line->setCreateXmlElement(false);
	d->line->setHidden(true);
	addChild(d->line);
	d->line->init(group);
	connect(d->line, &Line::updatePixmapRequested, [=] {
		d->updatePixmap();
		Q_EMIT updateLegendRequested();
	});
	connect(d->line, &Line::updateRequested, [=] {
		d->recalcShapeAndBoundingRect();
		Q_EMIT updateLegendRequested();
	});

	d->dropLine = new Line(QString());
	d->dropLine->setPrefix(QLatin1String("DropLine"));
	d->dropLine->setHidden(true);
	addChild(d->dropLine);
	d->dropLine->init(group);
	connect(d->dropLine, &Line::dropLineTypeChanged, [=] {
		d->updateDropLines();
	});
	connect(d->dropLine, &Line::updatePixmapRequested, [=] {
		d->updatePixmap();
	});
	connect(d->dropLine, &Line::updateRequested, [=] {
		d->recalcShapeAndBoundingRect();
	});

	// initialize the symbol
	d->symbol = new Symbol(QString());
	addChild(d->symbol);
	d->symbol->setHidden(true);
	d->symbol->init(group);
	connect(d->symbol, &Symbol::updateRequested, [=] {
		d->updateSymbols();
		Q_EMIT updateLegendRequested();
	});
	connect(d->symbol, &Symbol::updatePixmapRequested, [=] {
		d->updatePixmap();
		Q_EMIT updateLegendRequested();
	});

	// values
	d->valuesType = (ValuesType)group.readEntry("ValuesType", static_cast<int>(ValuesType::NoValues));
	d->valuesPosition = (ValuesPosition)group.readEntry("ValuesPosition", static_cast<int>(ValuesPosition::Above));
	d->valuesDistance = group.readEntry("ValuesDistance", Worksheet::convertToSceneUnits(5, Worksheet::Unit::Point));
	d->valuesRotationAngle = group.readEntry("ValuesRotation", 0.0);
	d->valuesOpacity = group.readEntry("ValuesOpacity", 1.0);
	d->valuesNumericFormat = group.readEntry("ValuesNumericFormat", "f").at(0).toLatin1();
	d->valuesPrecision = group.readEntry("ValuesNumericFormat", 2);
	d->valuesDateTimeFormat = group.readEntry("ValuesDateTimeFormat", "yyyy-MM-dd");
	d->valuesPrefix = group.readEntry("ValuesPrefix", "");
	d->valuesSuffix = group.readEntry("ValuesSuffix", "");
	d->valuesFont = group.readEntry("ValuesFont", QFont());
	d->valuesFont.setPixelSize(Worksheet::convertToSceneUnits(8, Worksheet::Unit::Point));
	d->valuesColor = group.readEntry("ValuesColor", QColor(Qt::black));

	// Background/Filling
	d->background = new Background(QString());
	d->background->setPrefix(QLatin1String("Filling"));
	d->background->setPositionAvailable(true);
	addChild(d->background);
	d->background->setHidden(true);
	d->background->init(group);
	connect(d->background, &Background::updateRequested, [=] {
		d->updatePixmap();
	});
	connect(d->background, &Background::updatePositionRequested, [=] {
		d->updateFilling();
	});

	// error bars
	d->xErrorType = (ErrorType)group.readEntry("XErrorType", static_cast<int>(ErrorType::NoError));
	d->yErrorType = (ErrorType)group.readEntry("YErrorType", static_cast<int>(ErrorType::NoError));
	d->errorBarsLine = new Line(QString());
	d->errorBarsLine->setPrefix(QLatin1String("ErrorBars"));
	d->errorBarsLine->setCreateXmlElement(false); // errorBars element is created in XYCurve::save()
	d->errorBarsLine->setErrorBarsTypeAvailable(true);
	d->errorBarsLine->setHidden(true);
	addChild(d->errorBarsLine);
	d->errorBarsLine->init(group);
	connect(d->errorBarsLine, &Line::errorBarsTypeChanged, [=] {
		d->updateErrorBars();
	});
	connect(d->errorBarsLine, &Line::errorBarsCapSizeChanged, [=] {
		d->updateErrorBars();
	});
	connect(d->errorBarsLine, &Line::updatePixmapRequested, [=] {
		d->updatePixmap();
	});
	connect(d->errorBarsLine, &Line::updateRequested, [=] {
		d->recalcShapeAndBoundingRect();
	});

	// marginal plots (rug, histogram, boxplot)
	d->rugEnabled = group.readEntry("RugEnabled", false);
	d->rugOrientation = (WorksheetElement::Orientation)group.readEntry("RugOrientation", (int)WorksheetElement::Orientation::Both);
	d->rugLength = group.readEntry("RugLength", Worksheet::convertToSceneUnits(5, Worksheet::Unit::Point));
	d->rugWidth = group.readEntry("RugWidth", 0.0);
	d->rugOffset = group.readEntry("RugOffset", 0.0);
}

void XYCurve::initActions() {
	visibilityAction = new QAction(QIcon::fromTheme(QStringLiteral("view-visible")), i18n("Visible"), this);
	visibilityAction->setCheckable(true);
	connect(visibilityAction, SIGNAL(triggered(bool)), this, SLOT(changeVisibility()));

	navigateToAction = new QAction(QIcon::fromTheme(QStringLiteral("go-next-view")), QString(), this);
	connect(navigateToAction, SIGNAL(triggered(bool)), this, SLOT(navigateTo()));

	m_menusInitialized = true;
}

QMenu* XYCurve::createContextMenu() {
	if (!m_menusInitialized)
		initActions();

	QMenu* menu = WorksheetElement::createContextMenu();
	QAction* firstAction = menu->actions().at(1); // skip the first action because of the "title-action"
	visibilityAction->setChecked(isVisible());
	menu->insertAction(firstAction, visibilityAction);

	//"data analysis" menu
	//	auto* plot = static_cast<CartesianPlot*>(parentAspect());
	menu->insertMenu(visibilityAction, m_plot->analysisMenu());
	menu->insertSeparator(visibilityAction);
	menu->insertSeparator(firstAction);

	//"Navigate to spreadsheet"-action, show only if x- or y-columns have data from a spreadsheet
	AbstractAspect* parentSpreadsheet = nullptr;
	if (xColumn() && dynamic_cast<Spreadsheet*>(xColumn()->parentAspect()))
		parentSpreadsheet = xColumn()->parentAspect();
	else if (yColumn() && dynamic_cast<Spreadsheet*>(yColumn()->parentAspect()))
		parentSpreadsheet = yColumn()->parentAspect();

	if (parentSpreadsheet) {
		navigateToAction->setText(i18n("Navigate to \"%1\"", parentSpreadsheet->name()));
		navigateToAction->setData(parentSpreadsheet->path());
		menu->insertAction(visibilityAction, navigateToAction);
		menu->insertSeparator(visibilityAction);
	}

	// if the context menu is called on an item that is not selected yet, select it
	if (!graphicsItem()->isSelected())
		graphicsItem()->setSelected(true);

	return menu;
}

/*!
	Returns an icon to be used in the project explorer.
*/
QIcon XYCurve::icon() const {
	return QIcon::fromTheme(QStringLiteral("labplot-xy-curve"));
}

QGraphicsItem* XYCurve::graphicsItem() const {
	return d_ptr;
}

/*!
 * \brief XYCurve::activatePlot
 * Checks if the mousepos distance to the curve is less than @p maxDist
 * \p mouseScenePos
 * \p maxDist Maximum distance the point lies away from the curve
 * \return Returns true if the distance is smaller than maxDist.
 */
bool XYCurve::activatePlot(QPointF mouseScenePos, double maxDist) {
	Q_D(XYCurve);
	return d->activatePlot(mouseScenePos, maxDist);
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

// ##############################################################################
// ##########################  getter methods  ##################################
// ##############################################################################
//  general
BASIC_SHARED_D_READER_IMPL(XYCurve, bool, legendVisible, legendVisible)

// data source
const AbstractColumn* XYCurve::column(const Dimension dim) const {
	switch (dim) {
	case Dimension::X:
		return xColumn();
	case Dimension::Y:
		return yColumn();
	}
	return nullptr;
}
BASIC_SHARED_D_READER_IMPL(XYCurve, const AbstractColumn*, xColumn, xColumn)
BASIC_SHARED_D_READER_IMPL(XYCurve, const AbstractColumn*, yColumn, yColumn)
BASIC_SHARED_D_READER_IMPL(XYCurve, QString, xColumnPath, xColumnPath)
BASIC_SHARED_D_READER_IMPL(XYCurve, QString, yColumnPath, yColumnPath)

// line
BASIC_SHARED_D_READER_IMPL(XYCurve, XYCurve::LineType, lineType, lineType)
BASIC_SHARED_D_READER_IMPL(XYCurve, bool, lineSkipGaps, lineSkipGaps)
BASIC_SHARED_D_READER_IMPL(XYCurve, bool, lineIncreasingXOnly, lineIncreasingXOnly)
BASIC_SHARED_D_READER_IMPL(XYCurve, int, lineInterpolationPointsCount, lineInterpolationPointsCount)

Line* XYCurve::line() const {
	Q_D(const XYCurve);
	return d->line;
}

Line* XYCurve::dropLine() const {
	Q_D(const XYCurve);
	return d->dropLine;
}

// symbols
Symbol* XYCurve::symbol() const {
	Q_D(const XYCurve);
	return d->symbol;
}

// values
BASIC_SHARED_D_READER_IMPL(XYCurve, XYCurve::ValuesType, valuesType, valuesType)
BASIC_SHARED_D_READER_IMPL(XYCurve, const AbstractColumn*, valuesColumn, valuesColumn)
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

// Filling
Background* XYCurve::background() const {
	Q_D(const XYCurve);
	return d->background;
}

// error bars
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

Line* XYCurve::errorBarsLine() const {
	Q_D(const XYCurve);
	return d->errorBarsLine;
}

// margin plots
BASIC_SHARED_D_READER_IMPL(XYCurve, bool, rugEnabled, rugEnabled)
BASIC_SHARED_D_READER_IMPL(XYCurve, WorksheetElement::Orientation, rugOrientation, rugOrientation)
BASIC_SHARED_D_READER_IMPL(XYCurve, double, rugLength, rugLength)
BASIC_SHARED_D_READER_IMPL(XYCurve, double, rugWidth, rugWidth)
BASIC_SHARED_D_READER_IMPL(XYCurve, double, rugOffset, rugOffset)

/*!
 * return \c true if the data in the source columns (x, y) used in the analysis curves, \c false otherwise
 */
bool XYCurve::isSourceDataChangedSinceLastRecalc() const {
	Q_D(const XYCurve);
	return d->sourceDataChangedSinceLastRecalc;
}

double XYCurve::minimum(const Dimension) const {
	// TODO
	return NAN;
}

double XYCurve::maximum(const Dimension) const {
	// TODO
	return NAN;
}

bool XYCurve::hasData() const {
	Q_D(const XYCurve);
	return (d->xColumn != nullptr || d->yColumn != nullptr);
}

// ##############################################################################
// #################  setter methods and undo commands ##########################
// ##############################################################################

// 1) add XYCurveSetXColumnCmd as friend class to XYCurve
// 2) add XYCURVE_COLUMN_CONNECT(x) as private method to XYCurve
// 3) define all missing slots
CURVE_COLUMN_SETTER_CMD_IMPL_F_S(XYCurve, X, x, recalcLogicalPoints)
void XYCurve::setXColumn(const AbstractColumn* column) {
	Q_D(XYCurve);
	if (column != d->xColumn)
		exec(new XYCurveSetXColumnCmd(d, column, ki18n("%1: x-data source changed")));
}

CURVE_COLUMN_SETTER_CMD_IMPL_F_S(XYCurve, Y, y, recalcLogicalPoints)
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

STD_SETTER_CMD_IMPL_S(XYCurve, SetLegendVisible, bool, legendVisible)
void XYCurve::setLegendVisible(bool visible) {
	Q_D(XYCurve);
	if (visible != d->legendVisible)
		exec(new XYCurveSetLegendVisibleCmd(d, visible, ki18n("%1: legend visibility changed")));
}

// Line
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

// Values-Tab
STD_SETTER_CMD_IMPL_F_S(XYCurve, SetValuesType, XYCurve::ValuesType, valuesType, updateValues)
void XYCurve::setValuesType(XYCurve::ValuesType type) {
	Q_D(XYCurve);
	if (type != d->valuesType)
		exec(new XYCurveSetValuesTypeCmd(d, type, ki18n("%1: set values type")));
}

CURVE_COLUMN_SETTER_CMD_IMPL_F_S(XYCurve, Values, values, updateValues)
void XYCurve::setValuesColumn(const AbstractColumn* column) {
	Q_D(XYCurve);
	if (column != d->valuesColumn) {
		exec(new XYCurveSetValuesColumnCmd(d, column, ki18n("%1: set values column")));

		// no need to recalculate the points on value labels changes
		disconnect(column, &AbstractColumn::dataChanged, this, &XYCurve::recalcLogicalPoints);

		if (column)
			connect(column, &AbstractColumn::dataChanged, this, &XYCurve::updateValues);
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

// Error bars
STD_SETTER_CMD_IMPL_F_S(XYCurve, SetXErrorType, XYCurve::ErrorType, xErrorType, updateErrorBars)
void XYCurve::setXErrorType(ErrorType type) {
	Q_D(XYCurve);
	if (type != d->xErrorType)
		exec(new XYCurveSetXErrorTypeCmd(d, type, ki18n("%1: x-error type changed")));
}

CURVE_COLUMN_SETTER_CMD_IMPL_F_S(XYCurve, XErrorPlus, xErrorPlus, updateErrorBars)
void XYCurve::setXErrorPlusColumn(const AbstractColumn* column) {
	Q_D(XYCurve);
	if (column != d->xErrorPlusColumn) {
		exec(new XYCurveSetXErrorPlusColumnCmd(d, column, ki18n("%1: set x-error column")));
		if (column) {
			connect(column, &AbstractColumn::dataChanged, this, &XYCurve::updateErrorBars);
			// in the macro we connect to recalcLogicalPoints which is not needed for error columns
			disconnect(column, &AbstractColumn::dataChanged, this, &XYCurve::recalcLogicalPoints);
		}
	}
}

void XYCurve::setXErrorPlusColumnPath(const QString& path) {
	Q_D(XYCurve);
	d->xErrorPlusColumnPath = path;
}

CURVE_COLUMN_SETTER_CMD_IMPL_F_S(XYCurve, XErrorMinus, xErrorMinus, updateErrorBars)
void XYCurve::setXErrorMinusColumn(const AbstractColumn* column) {
	Q_D(XYCurve);
	if (column != d->xErrorMinusColumn) {
		exec(new XYCurveSetXErrorMinusColumnCmd(d, column, ki18n("%1: set x-error column")));
		if (column) {
			connect(column, &AbstractColumn::dataChanged, this, &XYCurve::updateErrorBars);
			// in the macro we connect to recalcLogicalPoints which is not needed for error columns
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

CURVE_COLUMN_SETTER_CMD_IMPL_F_S(XYCurve, YErrorPlus, yErrorPlus, updateErrorBars)
void XYCurve::setYErrorPlusColumn(const AbstractColumn* column) {
	Q_D(XYCurve);
	if (column != d->yErrorPlusColumn) {
		exec(new XYCurveSetYErrorPlusColumnCmd(d, column, ki18n("%1: set y-error column")));
		if (column) {
			connect(column, &AbstractColumn::dataChanged, this, &XYCurve::updateErrorBars);
			// in the macro we connect to recalcLogicalPoints which is not needed for error columns
			disconnect(column, &AbstractColumn::dataChanged, this, &XYCurve::recalcLogicalPoints);
		}
	}
}

void XYCurve::setYErrorPlusColumnPath(const QString& path) {
	Q_D(XYCurve);
	d->yErrorPlusColumnPath = path;
}

CURVE_COLUMN_SETTER_CMD_IMPL_F_S(XYCurve, YErrorMinus, yErrorMinus, updateErrorBars)
void XYCurve::setYErrorMinusColumn(const AbstractColumn* column) {
	Q_D(XYCurve);
	if (column != d->yErrorMinusColumn) {
		exec(new XYCurveSetYErrorMinusColumnCmd(d, column, ki18n("%1: set y-error column")));
		if (column) {
			connect(column, &AbstractColumn::dataChanged, this, &XYCurve::updateErrorBars);
			// in the macro we connect to recalcLogicalPoints which is not needed for error columns
			disconnect(column, &AbstractColumn::dataChanged, this, &XYCurve::recalcLogicalPoints);
		}
	}
}

void XYCurve::setYErrorMinusColumnPath(const QString& path) {
	Q_D(XYCurve);
	d->yErrorMinusColumnPath = path;
}

// margin plots
STD_SETTER_CMD_IMPL_F_S(XYCurve, SetRugEnabled, bool, rugEnabled, updateRug)
void XYCurve::setRugEnabled(bool enabled) {
	Q_D(XYCurve);
	if (enabled != d->rugEnabled)
		exec(new XYCurveSetRugEnabledCmd(d, enabled, ki18n("%1: change rug enabled")));
}

STD_SETTER_CMD_IMPL_F_S(XYCurve, SetRugOrientation, WorksheetElement::Orientation, rugOrientation, updateRug)
void XYCurve::setRugOrientation(WorksheetElement::Orientation orientation) {
	Q_D(XYCurve);
	if (orientation != d->rugOrientation)
		exec(new XYCurveSetRugOrientationCmd(d, orientation, ki18n("%1: set rug orientation")));
}

STD_SETTER_CMD_IMPL_F_S(XYCurve, SetRugWidth, double, rugWidth, updatePixmap)
void XYCurve::setRugWidth(double width) {
	Q_D(XYCurve);
	if (width != d->rugWidth)
		exec(new XYCurveSetRugWidthCmd(d, width, ki18n("%1: change rug width")));
}

STD_SETTER_CMD_IMPL_F_S(XYCurve, SetRugLength, double, rugLength, updateRug)
void XYCurve::setRugLength(double length) {
	Q_D(XYCurve);
	if (length != d->rugLength)
		exec(new XYCurveSetRugLengthCmd(d, length, ki18n("%1: change rug length")));
}

STD_SETTER_CMD_IMPL_F_S(XYCurve, SetRugOffset, double, rugOffset, updateRug)
void XYCurve::setRugOffset(double offset) {
	Q_D(XYCurve);
	if (offset != d->rugOffset)
		exec(new XYCurveSetRugOffsetCmd(d, offset, ki18n("%1: change rug offset")));
}

// ##############################################################################
// #################################  SLOTS  ####################################
// ##############################################################################
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

// TODO
void XYCurve::handleResize(double horizontalRatio, double verticalRatio, bool /*pageResize*/) {
	Q_D(const XYCurve);

	d->symbol->setSize(d->symbol->size() * horizontalRatio);

	QPen pen = d->symbol->pen();
	pen.setWidthF(pen.widthF() * (horizontalRatio + verticalRatio) / 2.);
	d->symbol->setPen(pen);

	double width = d->line->width() * (horizontalRatio + verticalRatio) / 2.;
	d->line->setWidth(width);

	// setValuesDistance(d->distance*);
	QFont font = d->valuesFont;
	font.setPointSizeF(font.pointSizeF() * horizontalRatio);
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
		offset *= -1;

	int index = xColumn()->indexForValue(xpos);
	if (index < 0)
		return -1;
	if (offset > 0 && index + offset < xColumn()->rowCount())
		index += offset;
	else if (offset > 0)
		index = xColumn()->rowCount() - 1;
	else if ((offset < 0 && index + offset > 0))
		index += offset;
	else
		index = 0;

	AbstractColumn::ColumnMode xMode = xColumn()->columnMode();

	if (xMode == AbstractColumn::ColumnMode::Double || xMode == AbstractColumn::ColumnMode::Integer)
		x = xColumn()->valueAt(index);
	else if (xMode == AbstractColumn::ColumnMode::DateTime || xMode == AbstractColumn::ColumnMode::Day || xMode == AbstractColumn::ColumnMode::Month)
		x = xColumn()->dateTimeAt(index).toMSecsSinceEpoch();
	else
		return index;

	AbstractColumn::ColumnMode yMode = yColumn()->columnMode();

	if (yMode == AbstractColumn::ColumnMode::Double || yMode == AbstractColumn::ColumnMode::Integer)
		y = yColumn()->valueAt(index);
	else if (yMode == AbstractColumn::ColumnMode::DateTime || yMode == AbstractColumn::ColumnMode::Day || yMode == AbstractColumn::ColumnMode::Month)
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
		d->m_logicalPoints.clear();
		d->retransform();
	}
}

void XYCurve::yColumnAboutToBeRemoved(const AbstractAspect* aspect) {
	Q_D(XYCurve);
	if (aspect == d->yColumn) {
		disconnect(aspect, nullptr, this, nullptr);
		d->yColumn = nullptr;
		d->m_logicalPoints.clear();
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

// TODO: where are these two functions used?
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

// ##############################################################################
// ######  SLOTs for changes triggered via QActions in the context menu  ########
// ##############################################################################

void XYCurve::navigateTo() {
	project()->navigateTo(navigateToAction->data().toString());
}

// ##############################################################################
// ######################### Private implementation #############################
// ##############################################################################
XYCurvePrivate::XYCurvePrivate(XYCurve* owner)
	: PlotPrivate(owner)
	, q(owner) {
	setFlag(QGraphicsItem::ItemIsSelectable, true);
	setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
	setAcceptHoverEvents(false);
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
	if (q->activatePlot(event->pos())) {
		q->createContextMenu()->exec(event->screenPos());
		return;
	}
	QGraphicsItem::contextMenuEvent(event);
}

void XYCurvePrivate::calculateScenePoints() {
	if (!q->plot() || !m_scenePointsDirty || !xColumn)
		return;
#ifdef PERFTRACE_CURVES
	PERFTRACE(QLatin1String(Q_FUNC_INFO) + QStringLiteral(", curve ") + name());
#endif

	m_scenePoints.clear();

	// calculate the scene coordinates
	//  This condition cannot be used, because m_logicalPoints is also used in updateErrorBars(), updateDropLines() and in updateFilling()
	//  TODO: check updateErrorBars() and updateDropLines() and if they aren't available don't calculate this part
	// if (symbolsStyle != Symbol::Style::NoSymbols || valuesType != XYCurve::NoValues ) {
	{
#ifdef PERFTRACE_CURVES
		PERFTRACE(QLatin1String(Q_FUNC_INFO) + QStringLiteral(", curve ") + name() + QStringLiteral(", map logical points to scene coordinates"));
#endif

		const int numberOfPoints = m_logicalPoints.size();
		DEBUG(Q_FUNC_INFO << ", number of logical points = " << numberOfPoints)
		// for (auto p : m_logicalPoints)
		//	QDEBUG(Q_FUNC_INFO << ", logical points: " << QString::number(p.x(), 'g', 12) << " = " << QDateTime::fromMSecsSinceEpoch(p.x(), Qt::UTC))

		if (numberOfPoints > 0) {
			const auto dataRect{plot()->dataRect()};
			// this is the old method considering DPI
			DEBUG(Q_FUNC_INFO << ", plot->dataRect() width/height = " << dataRect.width() << '/' << dataRect.height());
			DEBUG(Q_FUNC_INFO << ", logical DPI X/Y = " << QApplication::desktop()->logicalDpiX() << '/' << QApplication::desktop()->logicalDpiY())
			DEBUG(Q_FUNC_INFO << ", physical DPI X/Y = " << QApplication::desktop()->physicalDpiX() << '/' << QApplication::desktop()->physicalDpiY())

			// new method
			const int numberOfPixelX = dataRect.width();
			const int numberOfPixelY = dataRect.height();

			if (numberOfPixelX <= 0 || numberOfPixelY <= 0) {
				DEBUG(Q_FUNC_INFO << ", number of pixel X <= 0 or number of pixel Y <= 0!")
				return;
			}
			DEBUG("	numberOfPixelX/numberOfPixelY = " << numberOfPixelX << '/' << numberOfPixelY)

			// eliminate multiple scene points (size (numberOfPixelX + 1) * (numberOfPixelY + 1)) TODO: why "+1"
			QVector<QVector<bool>> scenePointsUsed(numberOfPixelX + 1);
			for (auto& col : scenePointsUsed)
				col.resize(numberOfPixelY + 1);

			const auto columnProperties = xColumn->properties();
			int startIndex, endIndex;
			if (columnProperties == AbstractColumn::Properties::MonotonicDecreasing || columnProperties == AbstractColumn::Properties::MonotonicIncreasing) {
				DEBUG(Q_FUNC_INFO << ", column monotonic")
				if (!q->cSystem->isValid())
					return;
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

			m_pointVisible.resize(numberOfPoints);
			q->cSystem->mapLogicalToScene(startIndex, endIndex, m_logicalPoints, m_scenePoints, m_pointVisible);
			// for (auto p : m_logicalPoints)
			//	QDEBUG(Q_FUNC_INFO << ", logical points: " << QString::number(p.x(), 'g', 12) << " = " << QDateTime::fromMSecsSinceEpoch(p.x(), Qt::UTC))
		}
	}
	//} // (symbolsStyle != Symbol::Style::NoSymbols || valuesType != XYCurve::NoValues )
	m_scenePointsDirty = false;
}

/*!
  called when the size of the plot or its data ranges (manual changes, zooming, etc.) were changed.
  recalculates the position of the scene points to be drawn.
  triggers the update of lines, drop lines, symbols etc.
*/
void XYCurvePrivate::retransform() {
	const bool suppressed = !isVisible() || q->isLoading() || suppressRetransform || !plot();
	DEBUG("\n" << Q_FUNC_INFO << ", name = " << STDSTRING(name()) << ", suppressRetransform = " << suppressRetransform);
	trackRetransformCalled(suppressed);
	if (suppressed)
		return;

	m_scenePointsDirty = true;
	m_scenePoints.clear(); // free memory

	DEBUG(Q_FUNC_INFO << ", x/y column = " << xColumn << "/" << yColumn);
	// Q_ASSERT(xColumn != nullptr);
	if (!xColumn || !yColumn) {
		DEBUG(Q_FUNC_INFO << ", WARNING: xColumn or yColumn not available");
		linePath = QPainterPath();
		dropLinePath = QPainterPath();
		symbolsPath = QPainterPath();
		valuesPath = QPainterPath();
		errorBarsPath = QPainterPath();
		rugPath = QPainterPath();
		curveShape = QPainterPath();
		m_lines.clear();
		m_valuePoints.clear();
		m_valueStrings.clear();
		m_fillPolygons.clear();
		recalcShapeAndBoundingRect();
		return;
	}

	m_suppressRecalc = true;
	updateLines();
	updateDropLines();
	updateSymbols();
	updateRug();
	updateValues();
	m_suppressRecalc = false;
	updateErrorBars();
}

/*!
 * called if the x- or y-data was changed.
 * copies the valid data points from the x- and y-columns into the internal container
 */
void XYCurvePrivate::recalcLogicalPoints() {
	PERFTRACE(QLatin1String(Q_FUNC_INFO) + QStringLiteral(", curve ") + name());

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

	// take only valid and non masked points
	for (int row = 0; row < rows; row++) {
		if (xColumn->isValid(row) && yColumn->isValid(row) && (!xColumn->isMasked(row)) && (!yColumn->isMasked(row))) {
			QPointF tempPoint;

			switch (xColMode) {
			case AbstractColumn::ColumnMode::Double:
				tempPoint.setX(xColumn->doubleAt(row));
				break;
			case AbstractColumn::ColumnMode::Integer:
				tempPoint.setX(xColumn->integerAt(row));
				break;
			case AbstractColumn::ColumnMode::BigInt:
				tempPoint.setX(xColumn->bigIntAt(row));
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
			case AbstractColumn::ColumnMode::Double:
				tempPoint.setY(yColumn->doubleAt(row));
				break;
			case AbstractColumn::ColumnMode::Integer:
				tempPoint.setY(yColumn->integerAt(row));
				break;
			case AbstractColumn::ColumnMode::BigInt:
				tempPoint.setY(yColumn->bigIntAt(row));
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
			// TODO: append, resize-reserve
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
 * This function can be used for all axis scalings (linear, log, sqrt, ...). For the linear case use the function above, because it's optimized for the linear
 * case
 * @param p next point
 * @param x
 * @param minY
 * @param maxY
 * @param lastPoint remember last point in case of overlap
 * @param pixelDiff x pixel distance between two points
 * @param pixelCount pixel count
 */
void XYCurvePrivate::addLine(QPointF p,
							 double& x,
							 double& minY,
							 double& maxY,
							 QPointF& lastPoint,
							 int& pixelDiff,
							 int numberOfPixelX,
							 double minDiffX,
							 RangeT::Scale scale,
							 bool& prevPixelDiffZero) {
	if (scale == RangeT::Scale::Linear) {
		pixelDiff = (std::round(p.x() / minDiffX) - x) != 0; // only relevant if greater zero or not
		addUniqueLine(p, minY, maxY, lastPoint, pixelDiff, m_lines, prevPixelDiffZero);
		if (pixelDiff > 0) // set x to next pixel
			x = std::round(p.x() / minDiffX);
	} else {
		// for nonlinear scaling the pixel distance must be calculated for every point
		static const double preCalc = (double)plot()->dataRect().width() / numberOfPixelX;
		bool visible;
		QPointF pScene = q->cSystem->mapLogicalToScene(p, visible, CartesianCoordinateSystem::MappingFlag::SuppressPageClipping);

		// if the point is not valid, don't create a line
		if (!visible)
			return;

		// using only the difference between the points is not sufficient, because
		// p0 is updated always independent if new line added or not
		const int p1Pixel = std::round((pScene.x() - plot()->dataRect().x()) / preCalc);
		pixelDiff = p1Pixel - x;

		addUniqueLine(p, minY, maxY, lastPoint, pixelDiff, m_lines, prevPixelDiffZero);

		if (pixelDiff > 0) // set x to next pixel
			x = std::round((pScene.x() - plot()->dataRect().x()) / preCalc);
	}
}

/*!
 * \brief XYCurvePrivate::addUniqueLine
 * This function is called from the other two addLine() functions to avoid duplication
 * @param p next point
 * @param minY
 * @param maxY
 * @param lastPoint remember last point in case of overlap
 * @param pixelDiff pixel distance in x between two points
 */
void XYCurvePrivate::addUniqueLine(QPointF p, double& minY, double& maxY, QPointF& lastPoint, int& pixelDiff, QVector<QLineF>& lines, bool& prevPixelDiffZero) {
	if (pixelDiff == 0) {
		maxY = std::max(p.y(), maxY);
		minY = std::min(p.y(), minY);
		prevPixelDiffZero = true;
	} else {
		if (prevPixelDiffZero) {
			// If previously more than one point lied on the same pixel
			// a vertical line will be drawn which connects all points
			// on this x value. So we don't have to draw a lot of lines,
			// but only one

			// Not needed, because the line would be that small that it will never be visible
			// if (m_lines.count() > 0) {
			//	auto p_temp = m_lines.last().p2();
			//	m_lines.append(QLineF(p_temp.x(), p_temp.y(), x, p_temp.y()));
			// }
			if (maxY != minY)
				lines.append(QLineF(lastPoint.x(), minY, lastPoint.x(), maxY));
			lines.append(QLineF(lastPoint, p));
		} else if (!std::isnan(lastPoint.x()) && !std::isnan(lastPoint.y()))
			lines.append(QLineF(lastPoint, p));
		prevPixelDiffZero = false;
		minY = p.y();
		maxY = p.y();
		// TODO: needed?
		//			if (p1.y() >= minY && p1.y() <= maxY && pixelDiff == 1)
		//				return;
	}
	lastPoint = p;
}

/*!
  recalculates the painter path for the lines connecting the data points.
  Called each time when the type of this connection is changed.
TODO: At the moment also the points which are outside of the scene are added. This algorithm can be improved by omitting all
  lines where both points are outside of the scene
*/
void XYCurvePrivate::updateLines() {
#ifdef PERFTRACE_CURVES
	PERFTRACE(QLatin1String(Q_FUNC_INFO) + QStringLiteral(", curve ") + name());
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
	// const double widthDatarectInch = Worksheet::convertFromSceneUnits(plot()->dataRect().width(), Worksheet::Unit::Inch);
	// float heightDatarectInch = Worksheet::convertFromSceneUnits(plot()->dataRect().height(), Worksheet::Unit::Inch);
	// const int numberOfPixelX = ceil(widthDatarectInch * QApplication::desktop()->physicalDpiX());
	const int numberOfPixelX = pageRect.width();

	// calculate the lines connecting the data points
	{
#ifdef PERFTRACE_CURVES
		PERFTRACE(QLatin1String(Q_FUNC_INFO) + QStringLiteral(", curve ") + name() + QStringLiteral(", calculate the lines connecting the data points"));
#endif

		// find index for xMin and xMax to not loop through all values
		int startIndex, endIndex;
		auto columnProperties = q->xColumn()->properties();
		if (columnProperties == AbstractColumn::Properties::MonotonicDecreasing || columnProperties == AbstractColumn::Properties::MonotonicIncreasing) {
			DEBUG(Q_FUNC_INFO << ", monotonic")
			if (!q->cSystem->isValid())
				return;
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
			auto cs = plot()->coordinateSystem(q->coordinateSystemIndex());
			const auto xRange{plot()->range(Dimension::X, cs->index(Dimension::X))};
			const auto yRange{plot()->range(Dimension::Y, cs->index(Dimension::Y))};
			tempPoint1 = QPointF(xRange.start(), yRange.start());
			tempPoint2 = QPointF(xRange.start(), yRange.end());
			m_lines.append(QLineF(tempPoint1, tempPoint2));
		} else {
			QPointF lastPoint{NAN, NAN}; // last x value
			int pixelDiff = 0;
			bool prevPixelDiffZero = false;
			double minY{INFINITY}, maxY{-INFINITY};
			QPointF p0, p1;
			const auto xIndex{q->cSystem->index(Dimension::X)};
			const auto xRange{plot()->range(Dimension::X, xIndex)};
			double minDiffX;
			const RangeT::Scale scale = plot()->xRangeScale(xIndex);

			// setting initial point
			double xPos = NAN;
			if (scale == RangeT::Scale::Linear)
				minDiffX = (xRange.end() - xRange.start()) / numberOfPixelX;
			else {
				// For nonlinear x achses, the linear scene coordinates are used
				minDiffX = plot()->dataRect().width() / numberOfPixelX;
			}

			// determine first point
			for (int i{startIndex}; i < endIndex; i++) {
				if (!lineSkipGaps && !connectedPointsLogical.at(i))
					continue;

				p0 = m_logicalPoints.at(i);
				startIndex = i + 1;

				if (lineType != XYCurve::LineType::SplineCubicNatural && lineType != XYCurve::LineType::SplineCubicPeriodic
					&& lineType != XYCurve::LineType::SplineAkimaNatural && lineType != XYCurve::LineType::SplineAkimaPeriodic) {
					if (scale == RangeT::Scale::Linear) {
						xPos = std::round(p0.x() / minDiffX);
						lastPoint = p0;
						minY = p0.y();
						maxY = p0.y();
					} else {
						bool visible;
						QPointF pScene = q->cSystem->mapLogicalToScene(p0, visible, CartesianCoordinateSystem::MappingFlag::SuppressPageClipping);
						if (!visible)
							continue;
						xPos = std::round((pScene.x() - plot()->dataRect().x()) / ((double)plot()->dataRect().width() * numberOfPixelX));
						lastPoint = p0;
					}
				}
				break;
			}

			switch (lineType) {
			case XYCurve::LineType::NoLine:
				break;
			case XYCurve::LineType::Line: {
#ifdef PERFTRACE_CURVES
				PERFTRACE(name() + QLatin1String(Q_FUNC_INFO) + QStringLiteral(", find relevant lines"));
#endif
				for (int i{startIndex}; i <= endIndex; i++) {
					p1 = m_logicalPoints.at(i);
					if (!lineSkipGaps && (i > startIndex && !connectedPointsLogical.at(i - 1))) {
						if (pixelDiff == 0)
							m_lines.append(QLineF(QPointF(p0.x(), minY), QPointF(p0.x(), maxY)));
						prevPixelDiffZero = false;
						p0 = p1;
						lastPoint = p1;
						continue;
					}

					if (lineIncreasingXOnly && (p1.x() < p0.x())) // skip points
						continue;
					addLine(p1, xPos, minY, maxY, lastPoint, pixelDiff, numberOfPixelX, minDiffX, scale, prevPixelDiffZero);
					p0 = p1;
				}

				if (pixelDiff == 0)
					m_lines.append(QLineF(QPointF(p0.x(), minY), QPointF(p0.x(), maxY)));

				break;
			}
			case XYCurve::LineType::StartHorizontal: {
				for (int i{startIndex}; i <= endIndex; i++) {
					p1 = m_logicalPoints.at(i);
					if (!lineSkipGaps && (i > startIndex && !connectedPointsLogical.at(i - 1))) {
						// if (pixelDiff == 0) // not needed, because last line will be always a vertical
						m_lines.append(QLineF(QPointF(p0.x(), minY), QPointF(p0.x(), maxY)));
						prevPixelDiffZero = false;
						p0 = p1;
						lastPoint = p1;
						continue;
					}
					if (lineIncreasingXOnly && (p1.x() < p0.x()))
						continue;

					tempPoint1 = QPointF(p1.x(), p0.y()); // horizontal line
					addLine(tempPoint1, xPos, minY, maxY, lastPoint, pixelDiff, numberOfPixelX, minDiffX, scale, prevPixelDiffZero);
					addLine(p1, xPos, minY, maxY, lastPoint, pixelDiff, numberOfPixelX, minDiffX, scale, prevPixelDiffZero);
					p0 = p1;
				}
				// last line is a vertical line so it must be drawn manually (pixelDiff is 0)
				m_lines.append(QLineF(QPointF(p0.x(), minY), QPointF(p0.x(), maxY)));
				break;
			}
			case XYCurve::LineType::StartVertical: {
				for (int i{startIndex}; i <= endIndex; i++) {
					p1 = m_logicalPoints.at(i);
					if (!lineSkipGaps && (i > startIndex && !connectedPointsLogical.at(i - 1))) {
						if (pixelDiff == 0)
							m_lines.append(QLineF(QPointF(p0.x(), minY), QPointF(p0.x(), maxY)));
						prevPixelDiffZero = false;
						p0 = p1;
						lastPoint = p1;
						continue;
					}
					if (lineIncreasingXOnly && (p1.x() < p0.x()))
						continue;
					tempPoint1 = QPointF(p0.x(), p1.y());
					addLine(tempPoint1, xPos, minY, maxY, lastPoint, pixelDiff, numberOfPixelX, minDiffX, scale, prevPixelDiffZero);
					addLine(p1, xPos, minY, maxY, lastPoint, pixelDiff, numberOfPixelX, minDiffX, scale, prevPixelDiffZero);
					p0 = p1;
				}

				if (pixelDiff == 0)
					m_lines.append(QLineF(QPointF(p0.x(), minY), QPointF(p0.x(), maxY)));

				break;
			}
			case XYCurve::LineType::MidpointHorizontal: {
				for (int i{startIndex}; i <= endIndex; i++) {
					p1 = m_logicalPoints.at(i);
					if (!lineSkipGaps && (i > startIndex && !connectedPointsLogical.at(i - 1))) {
						if (pixelDiff == 0)
							m_lines.append(QLineF(QPointF(p0.x(), minY), QPointF(p0.x(), maxY)));
						prevPixelDiffZero = false;
						p0 = p1;
						lastPoint = p1;
						continue;
					}
					if (lineIncreasingXOnly && (p1.x() < p0.x()))
						continue;
					tempPoint1 = QPointF(p0.x() + (p1.x() - p0.x()) / 2., p0.y()); // horizontal until mid at p0.y level
					tempPoint2 = QPointF(p0.x() + (p1.x() - p0.x()) / 2., p1.y()); // vertical line
					addLine(tempPoint1, xPos, minY, maxY, lastPoint, pixelDiff, numberOfPixelX, minDiffX, scale, prevPixelDiffZero);
					addLine(tempPoint2, xPos, minY, maxY, lastPoint, pixelDiff, numberOfPixelX, minDiffX, scale, prevPixelDiffZero);
					addLine(p1, xPos, minY, maxY, lastPoint, pixelDiff, numberOfPixelX, minDiffX, scale, prevPixelDiffZero);
					p0 = p1;
				}

				if (pixelDiff == 0)
					m_lines.append(QLineF(QPointF(p0.x(), minY), QPointF(p0.x(), maxY)));

				break;
			}
			case XYCurve::LineType::MidpointVertical: {
				for (int i{startIndex}; i <= endIndex; i++) {
					p1 = m_logicalPoints.at(i);
					if (!lineSkipGaps && (i > startIndex && !connectedPointsLogical.at(i - 1))) {
						// if (pixelDiff == 0) // last line will be always a vertical line
						m_lines.append(QLineF(QPointF(p0.x(), minY), QPointF(p0.x(), maxY)));
						prevPixelDiffZero = false;
						p0 = p1;
						lastPoint = p1;
						continue;
					}
					if (lineIncreasingXOnly && (p1.x() < p0.x()))
						continue;
					tempPoint1 = QPointF(p0.x(), p0.y() + (p1.y() - p0.y()) / 2.);
					tempPoint2 = QPointF(p1.x(), p0.y() + (p1.y() - p0.y()) / 2.);
					addLine(tempPoint1, xPos, minY, maxY, lastPoint, pixelDiff, numberOfPixelX, minDiffX, scale, prevPixelDiffZero);
					addLine(tempPoint2, xPos, minY, maxY, lastPoint, pixelDiff, numberOfPixelX, minDiffX, scale, prevPixelDiffZero);
					addLine(p1, xPos, minY, maxY, lastPoint, pixelDiff, numberOfPixelX, minDiffX, scale, prevPixelDiffZero);
					p0 = p1;
				}
				// last line is a vertical line so it must be drawn manually (pixelDiff is 0)
				m_lines.append(QLineF(QPointF(p0.x(), minY), QPointF(p0.x(), maxY)));
				break;
			}
			case XYCurve::LineType::Segments2:
			case XYCurve::LineType::Segments3: {
				int skip{0};
				int skip_index = 1;
				if (lineType == XYCurve::LineType::Segments2)
					skip_index = 1;
				else if (lineType == XYCurve::LineType::Segments3)
					skip_index = 2;
				else
					DEBUG("WARNING: unhandled case!!!!");

				for (int i{startIndex}; i <= endIndex; i++) {
					p1 = m_logicalPoints.at(i);
					if (skip < skip_index) {
						if (!lineSkipGaps && (i > startIndex && !connectedPointsLogical.at(i - 1))) {
							if (pixelDiff == 0)
								m_lines.append(QLineF(QPointF(p0.x(), minY), QPointF(p0.x(), maxY)));
							prevPixelDiffZero = false;
							p0 = p1;
							lastPoint = p1;
							skip = 0;
							continue;
						}

						if (lineIncreasingXOnly && (p1.x() < p0.x())) {
							skip = 0;
							continue;
						}
						addLine(p1, xPos, minY, maxY, lastPoint, pixelDiff, numberOfPixelX, minDiffX, scale, prevPixelDiffZero);
						skip++;
					} else {
						skip = 0;
						if (!lineSkipGaps && (i > startIndex && !connectedPointsLogical.at(i - 1))) {
							if (pixelDiff == 0)
								m_lines.append(QLineF(QPointF(p0.x(), minY), QPointF(p0.x(), maxY)));
							prevPixelDiffZero = false;
							skip += 2; // if one point is missing, 2 lines are skipped
						}
						lastPoint = p1;
					}
					p0 = p1;
				}

				if (pixelDiff == 0 && skip != skip_index)
					m_lines.append(QLineF(QPointF(p0.x(), minY), QPointF(p0.x(), maxY)));

				break;
			}
			case XYCurve::LineType::SplineCubicNatural:
			case XYCurve::LineType::SplineCubicPeriodic:
			case XYCurve::LineType::SplineAkimaNatural:
			case XYCurve::LineType::SplineAkimaPeriodic: {
				std::unique_ptr<double[]> x(new double[numberOfPoints]());
				std::unique_ptr<double[]> y(new double[numberOfPoints]());
				int validPoints = 1; // p0 is valid
				x[0] = p0.x();
				y[0] = p0.y();
				for (int i{startIndex}; i <= endIndex; i++) {
					p1 = m_logicalPoints.at(i);
					if (!lineSkipGaps && (i > startIndex && !connectedPointsLogical.at(i - 1))) {
						if (pixelDiff == 0)
							m_lines.append(QLineF(QPointF(p0.x(), minY), QPointF(p0.x(), maxY)));
						p0 = p1;
						lastPoint = p1;
						continue;
					}
					if (lineIncreasingXOnly && (p1.x() < p0.x()))
						continue;
					validPoints++;
					x[i] = p1.x();
					y[i] = p1.y();
				}
				numberOfPoints = validPoints;

				gsl_interp_accel* acc = gsl_interp_accel_alloc();
				gsl_spline* spline{nullptr};
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
					if ((lineType == XYCurve::LineType::SplineAkimaNatural || lineType == XYCurve::LineType::SplineAkimaPeriodic) && numberOfPoints < 5)
						msg = i18n("Error: Akima spline interpolation requires a minimum of 5 points.");
					else
						msg = i18n("Error: Could not initialize the spline function.");
					Q_EMIT q->info(msg);

					recalcShapeAndBoundingRect();
					gsl_interp_accel_free(acc);
					return;
				}

				int status = gsl_spline_init(spline, x.get(), y.get(), numberOfPoints);
				if (status != 0) {
					// TODO: check in gsl/interp.c when GSL_EINVAL is thrown
					QString gslError;
					if (status == GSL_EINVAL)
						gslError = i18n("x values must be monotonically increasing.");
					else
						gslError = gslErrorToString(status);
					Q_EMIT q->info(i18n("Error: %1", gslError));

					recalcShapeAndBoundingRect();
					gsl_spline_free(spline);
					gsl_interp_accel_free(acc);
					return;
				}

				// create interpolating points
				// TODO: QVector
				std::vector<double> xinterp, yinterp;
				for (int i{0}; i < numberOfPoints - 1; i++) {
					const double x1 = x[i];
					const double x2 = x[i + 1];
					const double step = std::abs(x2 - x1) / (lineInterpolationPointsCount + 1);

					for (int j{0}; j < (lineInterpolationPointsCount + 1); j++) {
						const double xi = x1 + j * step;
						const double yi = gsl_spline_eval(spline, xi, acc);
						xinterp.push_back(xi);
						yinterp.push_back(yi);
					}
				}

				if (!xinterp.empty()) {
					for (unsigned int i{0}; i < xinterp.size() - 1; i++) {
						p0 = QPointF(xinterp[i], yinterp[i]);
						addLine(p0, xPos, minY, maxY, lastPoint, pixelDiff, numberOfPixelX, minDiffX, scale, prevPixelDiffZero);
					}

					addLine(QPointF(x[numberOfPoints - 1], y[numberOfPoints - 1]),
							xPos,
							minY,
							maxY,
							lastPoint,
							pixelDiff,
							numberOfPixelX,
							minDiffX,
							scale,
							prevPixelDiffZero);
				}

				gsl_spline_free(spline);
				gsl_interp_accel_free(acc);
				break;
			}
			}
		}
	}

	// map the lines to scene coordinates
	{
#ifdef PERFTRACE_CURVES
		PERFTRACE(QLatin1String(Q_FUNC_INFO) + QStringLiteral(", curve ") + name() + QStringLiteral(", map lines to scene coordinates"));
#endif
		emit q->linesUpdated(q, m_lines);
		m_lines = q->cSystem->mapLogicalToScene(m_lines);
	}

	{
#ifdef PERFTRACE_CURVES
		PERFTRACE(QLatin1String(Q_FUNC_INFO) + QStringLiteral(", curve ") + name() + QStringLiteral(", calculate new line path"));
#endif
		// new line path
		if (!m_lines.isEmpty()) {
			linePath.moveTo(m_lines.constFirst().p1());
			QPointF prevP2;
			for (const auto& line : qAsConst(m_lines)) {
				if (prevP2 != line.p1())
					linePath.moveTo(line.p1());
				linePath.lineTo(line.p2());
				prevP2 = line.p2();
			}
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
	if (dropLine->dropLineType() == XYCurve::DropLineType::NoDropLine) {
		recalcShapeAndBoundingRect();
		return;
	}

	// calculate the scene points to get the values for m_pointsVisible
	calculateScenePoints();

	// calculate drop lines
	QVector<QLineF> dlines;
	auto cs = plot()->coordinateSystem(q->coordinateSystemIndex());
	const double xMin = plot()->range(Dimension::X, cs->index(Dimension::X)).start();
	const double yMin = plot()->range(Dimension::Y, cs->index(Dimension::Y)).start();

	int i = 0;
	switch (dropLine->dropLineType()) {
	case XYCurve::DropLineType::NoDropLine:
		break;
	case XYCurve::DropLineType::X:
		for (const auto& point : qAsConst(m_logicalPoints)) {
			if (!m_pointVisible.at(i++))
				continue;
			dlines.append(QLineF(point, QPointF(point.x(), yMin)));
		}
		break;
	case XYCurve::DropLineType::Y:
		for (const auto& point : qAsConst(m_logicalPoints)) {
			if (!m_pointVisible.at(i++))
				continue;
			dlines.append(QLineF(point, QPointF(xMin, point.y())));
		}
		break;
	case XYCurve::DropLineType::XY:
		for (const auto& point : qAsConst(m_logicalPoints)) {
			if (!m_pointVisible.at(i++))
				continue;
			dlines.append(QLineF(point, QPointF(point.x(), yMin)));
			dlines.append(QLineF(point, QPointF(xMin, point.y())));
		}
		break;
	case XYCurve::DropLineType::XZeroBaseline:
		for (const auto& point : qAsConst(m_logicalPoints)) {
			if (!m_pointVisible.at(i++))
				continue;
			dlines.append(QLineF(point, QPointF(point.x(), 0)));
		}
		break;
	case XYCurve::DropLineType::XMinBaseline:
		for (const auto& point : qAsConst(m_logicalPoints)) {
			if (!m_pointVisible.at(i++))
				continue;
			dlines.append(QLineF(point, QPointF(point.x(), yColumn->minimum())));
		}
		break;
	case XYCurve::DropLineType::XMaxBaseline:
		for (const auto& point : qAsConst(m_logicalPoints)) {
			if (!m_pointVisible.at(i++))
				continue;
			dlines.append(QLineF(point, QPointF(point.x(), yColumn->maximum())));
		}
		break;
	}

	// map the drop lines to scene coordinates
	dlines = q->cSystem->mapLogicalToScene(dlines);

	// new painter path for the drop lines
	for (const auto& line : qAsConst(dlines)) {
		dropLinePath.moveTo(line.p1());
		dropLinePath.lineTo(line.p2());
	}

	recalcShapeAndBoundingRect();
}

void XYCurvePrivate::updateSymbols() {
#ifdef PERFTRACE_CURVES
	PERFTRACE(QLatin1String(Q_FUNC_INFO) + QStringLiteral(", curve ") + name());
#endif
	symbolsPath = QPainterPath();
	if (symbol->style() != Symbol::Style::NoSymbols) {
		QPainterPath path = Symbol::stylePath(symbol->style());

		QTransform trafo;
		trafo.scale(symbol->size(), symbol->size());
		path = trafo.map(path);
		trafo.reset();

		if (symbol->rotationAngle() != 0.) {
			trafo.rotate(symbol->rotationAngle());
			path = trafo.map(path);
		}
		calculateScenePoints();
		for (const auto& point : qAsConst(m_scenePoints)) {
			trafo.reset();
			trafo.translate(point.x(), point.y());
			symbolsPath.addPath(trafo.map(path));
		}
	}

	recalcShapeAndBoundingRect();
}

void XYCurvePrivate::updateRug() {
	rugPath = QPainterPath();

	if (!rugEnabled || !plot()) {
		recalcShapeAndBoundingRect();
		return;
	}

	QVector<QPointF> points;
	auto cs = plot()->coordinateSystem(q->coordinateSystemIndex());
	const double xMin = plot()->range(Dimension::X, cs->index(Dimension::X)).start();
	const double yMin = plot()->range(Dimension::Y, cs->index(Dimension::Y)).start();

	// vertical rug
	if (rugOrientation == WorksheetElement::Orientation::Vertical || rugOrientation == WorksheetElement::Orientation::Both) {
		for (const auto& point : qAsConst(m_logicalPoints))
			points << QPointF(xMin, point.y());

		// map the points to scene coordinates
		points = q->cSystem->mapLogicalToScene(points);

		// path for the vertical rug lines
		for (const auto& point : qAsConst(points)) {
			rugPath.moveTo(point.x() + rugOffset, point.y());
			rugPath.lineTo(point.x() + rugOffset + rugLength, point.y());
		}
	}

	// horizontal rug
	if (rugOrientation == WorksheetElement::Orientation::Horizontal || rugOrientation == WorksheetElement::Orientation::Both) {
		points.clear();
		for (const auto& point : qAsConst(m_logicalPoints))
			points << QPointF(point.x(), yMin);

		// map the points to scene coordinates
		points = q->cSystem->mapLogicalToScene(points);

		// path for the horizontal rug lines
		for (const auto& point : qAsConst(points)) {
			rugPath.moveTo(point.x(), point.y() - rugOffset);
			rugPath.lineTo(point.x(), point.y() - rugOffset - rugLength);
		}
	}

	recalcShapeAndBoundingRect();
}

/*!
  recreates the value strings to be shown and recalculates their draw position.
*/
void XYCurvePrivate::updateValues() {
#ifdef PERFTRACE_CURVES
	PERFTRACE(QLatin1String(Q_FUNC_INFO) + QLatin1String(", curve ") + name());
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

	calculateScenePoints();

	// determine the value string for all points that are currently visible in the plot
	int i{0};
	auto cs = plot()->coordinateSystem(q->coordinateSystemIndex());
	const auto numberLocale = QLocale();
	switch (valuesType) {
	case XYCurve::ValuesType::NoValues:
	case XYCurve::ValuesType::X: {
		auto xRangeFormat{plot()->range(Dimension::X, cs->index(Dimension::X)).format()};
		int precision = valuesPrecision;
		if (xColumn->columnMode() == AbstractColumn::ColumnMode::Integer || xColumn->columnMode() == AbstractColumn::ColumnMode::BigInt)
			precision = 0;
		for (const auto& point : qAsConst(m_logicalPoints)) {
			if (!m_pointVisible.at(i++))
				continue;
			QString value;
			if (xRangeFormat == RangeT::Format::Numeric)
				value = numberLocale.toString(point.x(), valuesNumericFormat, precision);
			else
				value = QDateTime::fromMSecsSinceEpoch(point.x(), Qt::UTC).toString(valuesDateTimeFormat);
			m_valueStrings << valuesPrefix + value + valuesSuffix;
		}
		break;
	}
	case XYCurve::ValuesType::Y: {
		auto rangeFormat{plot()->range(Dimension::Y, cs->index(Dimension::Y)).format()};
		int precision = valuesPrecision;
		if (yColumn->columnMode() == AbstractColumn::ColumnMode::Integer || yColumn->columnMode() == AbstractColumn::ColumnMode::BigInt)
			precision = 0;
		for (const auto& point : qAsConst(m_logicalPoints)) {
			if (!m_pointVisible.at(i++))
				continue;
			QString value;
			if (rangeFormat == RangeT::Format::Numeric)
				value = numberLocale.toString(point.y(), valuesNumericFormat, precision);
			else
				value = QDateTime::fromMSecsSinceEpoch(point.y(), Qt::UTC).toString(valuesDateTimeFormat);
			m_valueStrings << valuesPrefix + value + valuesSuffix;
		}
		break;
	}
	case XYCurve::ValuesType::XY:
	case XYCurve::ValuesType::XYBracketed: {
		auto xRangeFormat{plot()->range(Dimension::X, cs->index(Dimension::X)).format()};
		auto yRangeFormat{plot()->range(Dimension::Y, cs->index(Dimension::Y)).format()};

		int xPrecision = valuesPrecision;
		if (xColumn->columnMode() == AbstractColumn::ColumnMode::Integer || xColumn->columnMode() == AbstractColumn::ColumnMode::BigInt)
			xPrecision = 0;

		int yPrecision = valuesPrecision;
		if (yColumn->columnMode() == AbstractColumn::ColumnMode::Integer || yColumn->columnMode() == AbstractColumn::ColumnMode::BigInt)
			yPrecision = 0;

		for (const auto& point : qAsConst(m_logicalPoints)) {
			if (!m_pointVisible.at(i++))
				continue;
			QString value;
			if (valuesType == XYCurve::ValuesType::XYBracketed)
				value = QLatin1Char('(');
			if (xRangeFormat == RangeT::Format::Numeric)
				value += numberLocale.toString(point.x(), valuesNumericFormat, xPrecision);
			else
				value += QDateTime::fromMSecsSinceEpoch(point.x(), Qt::UTC).toString(valuesDateTimeFormat);

			if (yRangeFormat == RangeT::Format::Numeric)
				value += QLatin1Char(',') + numberLocale.toString(point.y(), valuesNumericFormat, yPrecision);
			else
				value += QLatin1Char(',') + QDateTime::fromMSecsSinceEpoch(point.y(), Qt::UTC).toString(valuesDateTimeFormat);

			if (valuesType == XYCurve::ValuesType::XYBracketed)
				value += QLatin1Char(')');

			m_valueStrings << valuesPrefix + value + valuesSuffix;
		}
		break;
	}
	case XYCurve::ValuesType::CustomColumn: {
		if (!valuesColumn) {
			recalcShapeAndBoundingRect();
			return;
		}

		const int endRow{std::min(std::min(xColumn->rowCount(), yColumn->rowCount()), valuesColumn->rowCount())};
		auto xColMode{xColumn->columnMode()};
		auto vColMode{valuesColumn->columnMode()};

		// need to check x range
		auto cs = plot()->coordinateSystem(q->coordinateSystemIndex());
		auto xRange = plot()->range(Dimension::X, cs->index(Dimension::X));

		size_t index = 0; // index of valid points (logicalPoints)
		for (int i = 0; i < endRow; ++i) {
			// ignore value labels for invalid data points
			// otherwise the assignment to the data points get lost
			if (!xColumn->isValid(i) || xColumn->isMasked(i) || !yColumn->isValid(i) || yColumn->isMasked(i) || !m_pointVisible.at(index++))
				continue;

			if (!valuesColumn->isValid(i) || valuesColumn->isMasked(i)) {
				m_valueStrings << QString();
				continue;
			}

			// check if inside x range
			switch (xColMode) {
			case AbstractColumn::ColumnMode::Double:
			case AbstractColumn::ColumnMode::Integer:
			case AbstractColumn::ColumnMode::BigInt:
				if (!xRange.contains(xColumn->valueAt(i)))
					continue;
				break;
			case AbstractColumn::ColumnMode::DateTime:
			case AbstractColumn::ColumnMode::Month:
			case AbstractColumn::ColumnMode::Day:
				if (xColumn->dateTimeAt(i) < QDateTime::fromMSecsSinceEpoch(xRange.start(), Qt::UTC)
					|| xColumn->dateTimeAt(i) > QDateTime::fromMSecsSinceEpoch(xRange.end(), Qt::UTC))
					continue;
				break;
			case AbstractColumn::ColumnMode::Text:
				break;
			}

			switch (vColMode) {
			case AbstractColumn::ColumnMode::Double:
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

	// Calculate the coordinates where to paint the value strings.
	// The coordinates depend on the actual size of the string.
	QPointF tempPoint;
	QFontMetrics fm(valuesFont);
	const int h{fm.ascent()};

	i = 0;
	for (const auto& string : qAsConst(m_valueStrings)) {
		// catch case with more label strings than scene points (should not happen even with custom column)
		if (i >= m_scenePoints.size())
			break;
		const int w{fm.boundingRect(string).width()};
		const double x{m_scenePoints.at(i).x()};
		const double y{m_scenePoints.at(i).y()};
		i++;

		switch (valuesPosition) {
		case XYCurve::ValuesPosition::Above:
			tempPoint = QPointF(x - w / 2., y - valuesDistance);
			break;
		case XYCurve::ValuesPosition::Under:
			tempPoint = QPointF(x - w / 2., y + valuesDistance + h / 2.);
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
	if (suppressRetransform)
		return;

	m_fillPolygons.clear();

	// don't try to calculate the filling polygons if
	//  - no filling was enabled
	//  - the number of visible points on the scene is too high
	//  - no scene points available, everything outside of the plot region or no scene points calculated yet
	auto fillingPosition = background->position();
	if (fillingPosition == Background::Position::No) {
		recalcShapeAndBoundingRect();
		return;
	}
	calculateScenePoints(); // TODO: find other way
	if (m_scenePoints.size() > 1000 || m_scenePoints.isEmpty()) {
		recalcShapeAndBoundingRect();
		return;
	}

	QVector<QLineF> fillLines;

	// if there're no interpolation lines available (XYCurve::NoLine selected), create line-interpolation,
	// use already available lines otherwise.
	if (!m_lines.isEmpty())
		fillLines = m_lines;
	else {
		for (int i = 0; i < m_logicalPoints.size() - 1; i++) {
			if (!lineSkipGaps && !connectedPointsLogical[i])
				continue;
			fillLines.append(QLineF(m_logicalPoints.at(i), m_logicalPoints.at(i + 1)));
		}

		// no lines available (no points), nothing to do
		if (fillLines.isEmpty())
			return;

		fillLines = q->cSystem->mapLogicalToScene(fillLines);

		// no lines available (no points) after mapping, nothing to do
		if (fillLines.isEmpty())
			return;
	}

	// create polygon(s):
	// 1. Depending on the current zoom-level, only a subset of the curve may be visible in the plot
	// and more of the filling area should be shown than the area defined by the start and end points of the currently visible points.
	// We check first whether the curve crosses the boundaries of the plot and determine new start and end points and put them to the boundaries.
	// 2. Furthermore, depending on the current filling type we determine the end point (x- or y-coordinate) where all polygons are closed at the end.
	QPolygonF pol;
	QPointF start = fillLines.at(0).p1(); // starting point of the current polygon, initialize with the first visible point
	QPointF end = fillLines.at(fillLines.size() - 1).p2(); // end point of the current polygon, initialize with the last visible point
	const QPointF& first = m_logicalPoints.at(0); // first point of the curve, may not be visible currently
	const QPointF& last = m_logicalPoints.at(m_logicalPoints.size() - 1); // last point of the curve, may not be visible currently
	QPointF edge;
	double xEnd{0.}, yEnd{0.};
	auto cs = plot()->coordinateSystem(q->coordinateSystemIndex());
	const auto xRange{plot()->range(Dimension::X, cs->index(Dimension::X))};
	const auto yRange{plot()->range(Dimension::Y, cs->index(Dimension::Y))};
	const double xMin{xRange.start()}, xMax{xRange.end()};
	const double yMin{yRange.start()}, yMax{yRange.end()};
	bool visible;
	if (fillingPosition == Background::Position::Above) {
		edge = q->cSystem->mapLogicalToScene(QPointF(xMin, yMin), visible);

		// start point
		if (nsl_math_essentially_equal(start.y(), edge.y())) {
			if (first.x() < xMin)
				start = edge;
			else if (first.x() > xMax)
				start = q->cSystem->mapLogicalToScene(QPointF(xMax, yMin), visible);
			else
				start = q->cSystem->mapLogicalToScene(QPointF(first.x(), yMin), visible);
		}

		// end point
		if (nsl_math_essentially_equal(end.y(), edge.y())) {
			if (last.x() < xMin)
				end = edge;
			else if (last.x() > xMax)
				end = q->cSystem->mapLogicalToScene(QPointF(xMax, yMin), visible);
			else
				end = q->cSystem->mapLogicalToScene(QPointF(last.x(), yMin), visible);
		}

		// coordinate at which to close all polygons
		yEnd = q->cSystem->mapLogicalToScene(QPointF(xMin, yMax), visible).y();
	} else if (fillingPosition == Background::Position::Below) {
		edge = q->cSystem->mapLogicalToScene(QPointF(xMin, yMax), visible);

		// start point
		if (nsl_math_essentially_equal(start.y(), edge.y())) {
			if (first.x() < xMin)
				start = edge;
			else if (first.x() > xMax)
				start = q->cSystem->mapLogicalToScene(QPointF(xMax, yMax), visible);
			else
				start = q->cSystem->mapLogicalToScene(QPointF(first.x(), yMax), visible);
		}

		// end point
		if (nsl_math_essentially_equal(end.y(), edge.y())) {
			if (last.x() < xMin)
				end = edge;
			else if (last.x() > xMax)
				end = q->cSystem->mapLogicalToScene(QPointF(xMax, yMax), visible);
			else
				end = q->cSystem->mapLogicalToScene(QPointF(last.x(), yMax), visible);
		}

		// coordinate at which to close all polygons
		yEnd = q->cSystem->mapLogicalToScene(QPointF(xMin, yMin), visible).y();
	} else if (fillingPosition == Background::Position::ZeroBaseline) {
		edge = q->cSystem->mapLogicalToScene(QPointF(xMin, yMax), visible);

		// start point
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

		// end point
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
	} else if (fillingPosition == Background::Position::Left) {
		edge = q->cSystem->mapLogicalToScene(QPointF(xMax, yMin), visible);

		// start point
		if (nsl_math_essentially_equal(start.x(), edge.x())) {
			if (first.y() < yMin)
				start = edge;
			else if (first.y() > yMax)
				start = q->cSystem->mapLogicalToScene(QPointF(xMax, yMax), visible);
			else
				start = q->cSystem->mapLogicalToScene(QPointF(xMax, first.y()), visible);
		}

		// end point
		if (nsl_math_essentially_equal(end.x(), edge.x())) {
			if (last.y() < yMin)
				end = edge;
			else if (last.y() > yMax)
				end = q->cSystem->mapLogicalToScene(QPointF(xMax, yMax), visible);
			else
				end = q->cSystem->mapLogicalToScene(QPointF(xMax, last.y()), visible);
		}

		// coordinate at which to close all polygons
		xEnd = q->cSystem->mapLogicalToScene(QPointF(xMin, yMin), visible).x();
	} else { // FillingRight
		edge = q->cSystem->mapLogicalToScene(QPointF(xMin, yMin), visible);

		// start point
		if (nsl_math_essentially_equal(start.x(), edge.x())) {
			if (first.y() < yMin)
				start = edge;
			else if (first.y() > yMax)
				start = q->cSystem->mapLogicalToScene(QPointF(xMin, yMax), visible);
			else
				start = q->cSystem->mapLogicalToScene(QPointF(xMin, first.y()), visible);
		}

		// end point
		if (nsl_math_essentially_equal(end.x(), edge.x())) {
			if (last.y() < yMin)
				end = edge;
			else if (last.y() > yMax)
				end = q->cSystem->mapLogicalToScene(QPointF(xMin, yMax), visible);
			else
				end = q->cSystem->mapLogicalToScene(QPointF(xMin, last.y()), visible);
		}

		// coordinate at which to close all polygons
		xEnd = q->cSystem->mapLogicalToScene(QPointF(xMax, yMin), visible).x();
	}

	if (start != fillLines.at(0).p1())
		pol << start;

	QPointF p1, p2;
	for (int i = 0; i < fillLines.size(); ++i) {
		const QLineF& line = fillLines.at(i);
		p1 = line.p1();
		p2 = line.p2();
		if (i != 0 && p1 != fillLines.at(i - 1).p2()) {
			// the first point of the current line is not equal to the last point of the previous line
			//->check whether we have a break in between.
			const bool gap = false; // TODO
			if (!gap) {
				//-> we have no break in the curve -> connect the points by a horizontal/vertical line
				pol << fillLines.at(i - 1).p2() << p1;
			} else {
				//-> we have a break in the curve -> close the polygon, add it to the polygon list and start a new polygon
				if (fillingPosition == Background::Position::Above || fillingPosition == Background::Position::Below
					|| fillingPosition == Background::Position::ZeroBaseline) {
					pol << QPointF(fillLines.at(i - 1).p2().x(), yEnd);
					pol << QPointF(start.x(), yEnd);
				} else {
					pol << QPointF(xEnd, fillLines.at(i - 1).p2().y());
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

	// close the last polygon
	if (fillingPosition == Background::Position::Above || fillingPosition == Background::Position::Below
		|| fillingPosition == Background::Position::ZeroBaseline) {
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
double XYCurve::y(double x, bool& valueFound) const {
	if (!yColumn() || !xColumn()) {
		valueFound = false;
		return NAN;
	}

	const int index = xColumn()->indexForValue(x);
	if (index < 0) {
		valueFound = false;
		return NAN;
	}

	valueFound = true;
	if (yColumn()->isNumeric())
		return yColumn()->valueAt(index);
	else {
		valueFound = false;
		return NAN;
	}
}

/*!
 * @param x :value for which y should be found
 * @param valueFound: returns true if y value found, otherwise false
 * @param x_new: exact x value where y value is
 * @return y value from x value
 */
double XYCurve::y(double x, double& x_new, bool& valueFound) const {
	int index = xColumn()->indexForValue(x);
	if (index < 0) {
		valueFound = false;
		return NAN;
	}

	AbstractColumn::ColumnMode xColumnMode = xColumn()->columnMode();
	if (xColumn()->isNumeric())
		x_new = xColumn()->valueAt(index);
	else if (xColumnMode == AbstractColumn::ColumnMode::DateTime || xColumnMode == AbstractColumn::ColumnMode::Day
			 || xColumnMode == AbstractColumn::ColumnMode::Month)
		x_new = xColumn()->dateTimeAt(index).toMSecsSinceEpoch();
	else {
		// any other type implemented
		valueFound = false;
		return NAN;
	}

	valueFound = true;
	if (yColumn()->isNumeric())
		return yColumn()->valueAt(index);
	else {
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
QDateTime XYCurve::yDateTime(double x, bool& valueFound) const {
	if (!yColumn() || !xColumn()) {
		valueFound = false;
		return {};
	}

	auto yColumnMode = yColumn()->columnMode();
	const int index = xColumn()->indexForValue(x);
	if (index < 0) {
		valueFound = false;
		return {};
	}

	valueFound = true;
	if (yColumnMode == AbstractColumn::ColumnMode::Day || yColumnMode == AbstractColumn::ColumnMode::Month
		|| yColumnMode == AbstractColumn::ColumnMode::DateTime)
		return yColumn()->dateTimeAt(index);

	valueFound = false;
	return {};
}

bool XYCurve::minMax(const Dimension dim, const Range<int>& indexRange, Range<double>& r, bool includeErrorBars) const {
	switch (dim) {
	case Dimension::X:
		return minMax(xColumn(), yColumn(), xErrorType(), xErrorPlusColumn(), xErrorMinusColumn(), indexRange, r, includeErrorBars);
	case Dimension::Y:
		return minMax(yColumn(), xColumn(), yErrorType(), yErrorPlusColumn(), yErrorMinusColumn(), indexRange, r, includeErrorBars);
	}
	return false;
}

/*!
 * Calculates the minimum \p min and maximum \p max of a curve with optionally respecting the error bars
 * This function does not check if the values are out of range
 * \p indexMax is not included
 * \p column1
 * \p column2
 * \p errorType
 * \p errorPlusColumn
 * \p errorMinusColumn
 * \p indexRange [min, max]
 * \p min
 * \p max
 * \p includeErrorBars If true respect the error bars in the min/max calculation
 */
bool XYCurve::minMax(const AbstractColumn* column1,
					 const AbstractColumn* column2,
					 const ErrorType errorType,
					 const AbstractColumn* errorPlusColumn,
					 const AbstractColumn* errorMinusColumn,
					 const Range<int>& indexRange,
					 Range<double>& range,
					 bool includeErrorBars) const {
#ifdef PERFTRACE_AUTOSCALE
	PERFTRACE(name() + QLatin1String(Q_FUNC_INFO));
#endif
	// when property is increasing or decreasing there is a benefit in finding minimum and maximum
	// for property == AbstractColumn::Properties::No it must be iterated over all values so it does not matter if this function or the below one is used
	// if the property of the second column is not AbstractColumn::Properties::No means, that all values are valid and not masked
	// DEBUG(Q_FUNC_INFO << "\n, column 1 min/max = " << column1->minimum() << "/" << column1->maximum())
	if ((!includeErrorBars || errorType == ErrorType::NoError) && column1->properties() != AbstractColumn::Properties::No && column2
		&& column2->properties() != AbstractColumn::Properties::No) {
		auto min = column1->minimum(indexRange.start(), indexRange.end());
		auto max = column1->maximum(indexRange.start(), indexRange.end());
		DEBUG(Q_FUNC_INFO << "\n, column 1 min/max in index range = " << min << "/" << max)
		// TODO: Range
		range.setRange(min, max);
		return true;
	}

	if (column1->rowCount() == 0)
		return false;

	range.setRange(INFINITY, -INFINITY);
	// DEBUG(Q_FUNC_INFO << ", calculate range for index range " << indexRange.start() << " .. " << indexRange.end())

	for (int i = indexRange.start(); i <= indexRange.end(); ++i) {
		if (!column1->isValid(i) || column1->isMasked(i) || (column2 && (!column2->isValid(i) || column2->isMasked(i))))
			continue;

		if ((errorPlusColumn && i >= errorPlusColumn->rowCount()) || (errorMinusColumn && i >= errorMinusColumn->rowCount()))
			continue;

		double value;
		if (column1->isNumeric())
			value = column1->valueAt(i);
		else if (column1->columnMode() == AbstractColumn::ColumnMode::DateTime || column1->columnMode() == AbstractColumn::ColumnMode::Month
				 || column1->columnMode() == AbstractColumn::ColumnMode::Day)
			value = column1->dateTimeAt(i).toMSecsSinceEpoch();
		else
			return false;

		if (errorType == ErrorType::NoError) {
			if (value < range.start())
				range.start() = value;

			if (value > range.end())
				range.end() = value;
		} else {
			// determine the values for the errors
			double errorPlus, errorMinus;
			if (errorPlusColumn && errorPlusColumn->isValid(i) && !errorPlusColumn->isMasked(i))
				if (errorPlusColumn->isNumeric())
					errorPlus = errorPlusColumn->valueAt(i);
				else if (errorPlusColumn->columnMode() == AbstractColumn::ColumnMode::DateTime
						 || errorPlusColumn->columnMode() == AbstractColumn::ColumnMode::Month
						 || errorPlusColumn->columnMode() == AbstractColumn::ColumnMode::Day)
					errorPlus = errorPlusColumn->dateTimeAt(i).toMSecsSinceEpoch();
				else
					return false;
			else
				errorPlus = 0;

			if (errorType == ErrorType::Symmetric)
				errorMinus = errorPlus;
			else {
				if (errorMinusColumn && errorMinusColumn->isValid(i) && !errorMinusColumn->isMasked(i))
					if (errorMinusColumn->isNumeric())
						errorMinus = errorMinusColumn->valueAt(i);
					else if (errorMinusColumn->columnMode() == AbstractColumn::ColumnMode::DateTime
							 || errorMinusColumn->columnMode() == AbstractColumn::ColumnMode::Month
							 || errorMinusColumn->columnMode() == AbstractColumn::ColumnMode::Day)
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
		// DEBUG(Q_FUNC_INFO << ", range = " << range.toStdString())
	}
	return true;
}

bool XYCurvePrivate::activatePlot(QPointF mouseScenePos, double maxDist) {
	if (!isVisible())
		return false;

	int rowCount{0};
	if (lineType != XYCurve::LineType::NoLine)
		rowCount = m_lines.count();
	else if (symbol->style() != Symbol::Style::NoSymbols) {
		calculateScenePoints();
		rowCount = m_scenePoints.size();
	} else
		return false;

	if (rowCount == 0)
		return false;

	if (maxDist < 0)
		maxDist = (line->pen().width() < 10) ? 10. : line->pen().width();

	const double maxDistSquare = gsl_pow_2(maxDist);

	auto properties = q->xColumn()->properties();
	if (properties == AbstractColumn::Properties::No || properties == AbstractColumn::Properties::NonMonotonic) {
		// assumption: points exist if no line. otherwise previously returned false
		if (lineType == XYCurve::LineType::NoLine) {
			calculateScenePoints();
			QPointF curvePosPrevScene = m_scenePoints.at(0);
			QPointF curvePosScene = curvePosPrevScene;
			for (int row = 0; row < rowCount; row++) {
				if (gsl_pow_2(mouseScenePos.x() - curvePosScene.x()) + gsl_pow_2(mouseScenePos.y() - curvePosScene.y()) <= maxDistSquare)
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

	} else if (properties == AbstractColumn::Properties::MonotonicIncreasing || properties == AbstractColumn::Properties::MonotonicDecreasing) {
		bool increase{true};
		if (properties == AbstractColumn::Properties::MonotonicDecreasing)
			increase = false;

		double x{mouseScenePos.x() - maxDist};
		int index{0};

		QPointF curvePosScene;
		QPointF curvePosPrevScene;

		if (lineType == XYCurve::LineType::NoLine) {
			calculateScenePoints();
			curvePosScene = m_scenePoints.at(index);
			curvePosPrevScene = curvePosScene;
			index = Column::indexForValue(x, m_scenePoints, static_cast<AbstractColumn::Properties>(properties));
		} else
			index = Column::indexForValue(x, m_lines, static_cast<AbstractColumn::Properties>(properties));

		if (index >= 1)
			index--; // use one before so it is secured that I'm before point.x()
		else if (index == -1)
			return false;

		const double xMax{mouseScenePos.x() + maxDist};
		bool stop{false};
		while (true) {
			// assumption: points exist if no line. otherwise previously returned false
			if (lineType == XYCurve::LineType::NoLine) { // check points only if no line otherwise check only the lines
				if (curvePosScene.x() > xMax)
					stop = true; // one more time if bigger
				if (gsl_hypot(mouseScenePos.x() - curvePosScene.x(), mouseScenePos.y() - curvePosScene.y()) <= maxDist)
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
				calculateScenePoints();
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
	if (vecLength == 0.) {
		if (gsl_hypot(dx1m, dy1m) <= maxDist)
			return true;
		return false;
	}
	QPointF unitvec(dx12 / vecLength, dy12 / vecLength);

	const double dist_segm{std::abs(dx1m * unitvec.y() - dy1m * unitvec.x())};
	const double scalarProduct{dx1m * unitvec.x() + dy1m * unitvec.y()};

	if (scalarProduct > 0) {
		if (scalarProduct < vecLength && dist_segm < maxDist)
			return true;
	}
	return false;
}

// TODO: curvePosScene.x() >= mouseScenePos.x() &&
// curvePosPrevScene.x() < mouseScenePos.x()
// should not be here
bool XYCurvePrivate::pointLiesNearCurve(const QPointF mouseScenePos,
										const QPointF curvePosPrevScene,
										const QPointF curvePosScene,
										const int index,
										const double maxDist) const {
	if (q->lineType() != XYCurve::LineType::NoLine && curvePosScene.x() >= mouseScenePos.x() && curvePosPrevScene.x() < mouseScenePos.x()) {
		if (q->lineType() == XYCurve::LineType::Line) {
			// point is not in the near of the point, but it can be in the near of the connection line of two points
			if (pointLiesNearLine(curvePosPrevScene, curvePosScene, mouseScenePos, maxDist))
				return true;
		} else if (q->lineType() == XYCurve::LineType::StartHorizontal) {
			QPointF tempPoint = curvePosPrevScene;
			tempPoint.setX(curvePosScene.x());
			if (pointLiesNearLine(curvePosPrevScene, tempPoint, mouseScenePos, maxDist))
				return true;
			if (pointLiesNearLine(tempPoint, curvePosScene, mouseScenePos, maxDist))
				return true;
		} else if (q->lineType() == XYCurve::LineType::StartVertical) {
			QPointF tempPoint = curvePosPrevScene;
			tempPoint.setY(curvePosScene.y());
			if (pointLiesNearLine(curvePosPrevScene, tempPoint, mouseScenePos, maxDist))
				return true;
			if (pointLiesNearLine(tempPoint, curvePosScene, mouseScenePos, maxDist))
				return true;
		} else if (q->lineType() == XYCurve::LineType::MidpointHorizontal) {
			QPointF tempPoint = curvePosPrevScene;
			tempPoint.setX(curvePosPrevScene.x() + (curvePosScene.x() - curvePosPrevScene.x()) / 2);
			if (pointLiesNearLine(curvePosPrevScene, tempPoint, mouseScenePos, maxDist))
				return true;
			QPointF tempPoint2(tempPoint.x(), curvePosScene.y());
			if (pointLiesNearLine(tempPoint, tempPoint2, mouseScenePos, maxDist))
				return true;

			if (pointLiesNearLine(tempPoint2, curvePosScene, mouseScenePos, maxDist))
				return true;
		} else if (q->lineType() == XYCurve::LineType::MidpointVertical) {
			QPointF tempPoint = curvePosPrevScene;
			tempPoint.setY(curvePosPrevScene.y() + (curvePosScene.y() - curvePosPrevScene.y()) / 2);
			if (pointLiesNearLine(curvePosPrevScene, tempPoint, mouseScenePos, maxDist))
				return true;
			QPointF tempPoint2(tempPoint.y(), curvePosScene.x());
			if (pointLiesNearLine(tempPoint, tempPoint2, mouseScenePos, maxDist))
				return true;

			if (pointLiesNearLine(tempPoint2, curvePosScene, mouseScenePos, maxDist))
				return true;
		} else if (q->lineType() == XYCurve::LineType::SplineAkimaNatural || q->lineType() == XYCurve::LineType::SplineCubicNatural
				   || q->lineType() == XYCurve::LineType::SplineAkimaPeriodic || q->lineType() == XYCurve::LineType::SplineCubicPeriodic) {
			for (int i = 0; i < q->lineInterpolationPointsCount() + 1; i++) {
				QLineF line = m_lines.at(index * (q->lineInterpolationPointsCount() + 1) + i);
				QPointF p1{line.p1()}; // cSystem->mapLogicalToScene(line.p1());
				QPointF p2{line.p2()}; // cSystem->mapLogicalToScene(line.p2());
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
	const auto errorBarsType = errorBarsLine->errorBarsType();

	calculateScenePoints();

	for (int i = 0; i < m_logicalPoints.size(); ++i) {
		if (!m_pointVisible.at(i))
			continue;

		const QPointF& point{m_logicalPoints.at(i)};
		const int index{validPointsIndicesLogical.at(i)};
		double errorPlus, errorMinus;

		// error bars for x
		if (xErrorType != XYCurve::ErrorType::NoError) {
			// determine the values for the errors
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

			// draw the error bars
			if (errorMinus != 0. || errorPlus != 0.)
				elines.append(QLineF(QPointF(point.x() - errorMinus, point.y()), QPointF(point.x() + errorPlus, point.y())));

			// determine the end points of the errors bars in logical coordinates to draw later the cap
			if (errorBarsType == XYCurve::ErrorBarsType::WithEnds) {
				if (errorMinus != 0.)
					pointsErrorBarAnchorX << QPointF(point.x() - errorMinus, point.y());
				if (errorPlus != 0.)
					pointsErrorBarAnchorX << QPointF(point.x() + errorPlus, point.y());
			}
		}

		// error bars for y
		if (yErrorType != XYCurve::ErrorType::NoError) {
			// determine the values for the errors
			if (yErrorPlusColumn && yErrorPlusColumn->isValid(index) && !yErrorPlusColumn->isMasked(index))
				errorPlus = yErrorPlusColumn->valueAt(index);
			else
				errorPlus = 0;

			if (yErrorType == XYCurve::ErrorType::Symmetric)
				errorMinus = errorPlus;
			else {
				if (yErrorMinusColumn && yErrorMinusColumn->isValid(index) && !yErrorMinusColumn->isMasked(index))
					errorMinus = yErrorMinusColumn->valueAt(index);
				else
					errorMinus = 0;
			}

			// draw the error bars
			if (errorMinus != 0. || errorPlus != 0.)
				elines.append(QLineF(QPointF(point.x(), point.y() + errorPlus), QPointF(point.x(), point.y() - errorMinus)));

			// determine the end points of the errors bars in logical coordinates to draw later the cap
			if (errorBarsType == XYCurve::ErrorBarsType::WithEnds) {
				if (errorMinus != 0.)
					pointsErrorBarAnchorY << QPointF(point.x(), point.y() + errorPlus);
				if (errorPlus != 0.)
					pointsErrorBarAnchorY << QPointF(point.x(), point.y() - errorMinus);
			}
		}
	}

	// map the error bars to scene coordinates
	elines = q->cSystem->mapLogicalToScene(elines);

	// new painter path for the error bars
	for (const auto& line : qAsConst(elines)) {
		errorBarsPath.moveTo(line.p1());
		errorBarsPath.lineTo(line.p2());
	}

	// add caps for x error bars
	const auto errorBarsCapSize = errorBarsLine->errorBarsCapSize();
	if (!pointsErrorBarAnchorX.isEmpty()) {
		pointsErrorBarAnchorX = q->cSystem->mapLogicalToScene(pointsErrorBarAnchorX);
		for (const auto& point : qAsConst(pointsErrorBarAnchorX)) {
			errorBarsPath.moveTo(QPointF(point.x(), point.y() - errorBarsCapSize / 2.));
			errorBarsPath.lineTo(QPointF(point.x(), point.y() + errorBarsCapSize / 2.));
		}
	}

	// add caps for y error bars
	if (!pointsErrorBarAnchorY.isEmpty()) {
		pointsErrorBarAnchorY = q->cSystem->mapLogicalToScene(pointsErrorBarAnchorY);
		for (const auto& point : qAsConst(pointsErrorBarAnchorY)) {
			errorBarsPath.moveTo(QPointF(point.x() - errorBarsCapSize / 2., point.y()));
			errorBarsPath.lineTo(QPointF(point.x() + errorBarsCapSize / 2., point.y()));
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
	PERFTRACE(QLatin1String(Q_FUNC_INFO) + QLatin1String(", curve ") + name());
#endif

	prepareGeometryChange();
	curveShape = QPainterPath();
	if (lineType != XYCurve::LineType::NoLine)
		curveShape.addPath(WorksheetElement::shapeFromPath(linePath, line->pen()));

	if (dropLine->dropLineType() != XYCurve::DropLineType::NoDropLine)
		curveShape.addPath(WorksheetElement::shapeFromPath(dropLinePath, dropLine->pen()));

	if (symbol->style() != Symbol::Style::NoSymbols)
		curveShape.addPath(symbolsPath);

	curveShape.addPath(rugPath);

	if (valuesType != XYCurve::ValuesType::NoValues)
		curveShape.addPath(valuesPath);

	if (xErrorType != XYCurve::ErrorType::NoError || yErrorType != XYCurve::ErrorType::NoError)
		curveShape.addPath(WorksheetElement::shapeFromPath(errorBarsPath, errorBarsLine->pen()));

	boundingRectangle = curveShape.boundingRect();

	for (const auto& pol : qAsConst(m_fillPolygons))
		boundingRectangle = boundingRectangle.united(pol.boundingRect());

	// TODO: when the selection is painted, line intersections are visible.
	// simplified() removes those artifacts but is horrible slow for curves with large number of points.
	// search for an alternative.
	// curveShape = curveShape.simplified();

	updatePixmap();
}

void XYCurvePrivate::draw(QPainter* painter) {
#ifdef PERFTRACE_CURVES
	PERFTRACE(QLatin1String(Q_FUNC_INFO) + QLatin1String(", curve ") + name());
#endif

	// draw filling
	if (background->position() != Background::Position::No) {
		painter->setOpacity(background->opacity());
		painter->setPen(Qt::SolidLine);
		for (const auto& polygon : qAsConst(m_fillPolygons))
			drawFillingPollygon(polygon, painter, background);
	}

	// draw lines
	if (lineType != XYCurve::LineType::NoLine) {
		painter->setOpacity(line->opacity());
		painter->setPen(line->pen());
		painter->setBrush(Qt::NoBrush);
		if (line->pen().style() == Qt::SolidLine && !q->isPrinting()) {
			// Much fast than drawPath but has problems
			// with different styles
			// When exporting to svg or pdf, this creates for every line
			// it's own path in the saved file which is not desired. We
			// would like to have one complete path for a curve not many paths
			for (auto& line : m_lines)
				painter->drawLine(line);
		} else {
			painter->drawPath(linePath);
		}
	}

	// draw drop lines
	if (dropLine->dropLineType() != XYCurve::DropLineType::NoDropLine) {
		painter->setOpacity(dropLine->opacity());
		painter->setPen(dropLine->pen());
		painter->setBrush(Qt::NoBrush);
		painter->drawPath(dropLinePath);
	}

	// draw error bars
	if ((xErrorType != XYCurve::ErrorType::NoError) || (yErrorType != XYCurve::ErrorType::NoError)) {
		painter->setOpacity(errorBarsLine->opacity());
		painter->setPen(errorBarsLine->pen());
		painter->setBrush(Qt::NoBrush);
		painter->drawPath(errorBarsPath);
	}

	// draw symbols
	if (symbol->style() != Symbol::Style::NoSymbols) {
		calculateScenePoints();
		symbol->draw(painter, m_scenePoints);
	}

	// draw values
	if (valuesType != XYCurve::ValuesType::NoValues) {
		painter->setOpacity(valuesOpacity);
		painter->setPen(QPen(valuesColor));
		painter->setFont(valuesFont);
		drawValues(painter);
	}

	// draw rug
	if (rugEnabled) {
		QPen pen;
		pen.setColor(symbol->brush().color());
		pen.setWidthF(rugWidth);
		painter->setPen(pen);
		painter->setOpacity(symbol->opacity());
		painter->drawPath(rugPath);
	}
}

void XYCurvePrivate::updatePixmap() {
	DEBUG(Q_FUNC_INFO << ", m_suppressRecalc = " << m_suppressRecalc);
	if (m_suppressRecalc)
		return;

	m_hoverEffectImageIsDirty = true;
	m_selectionEffectImageIsDirty = true;
	if (boundingRectangle.width() == 0 || boundingRectangle.height() == 0) {
		DEBUG(Q_FUNC_INFO << ", boundingRectangle.width() or boundingRectangle.height() == 0");
		m_pixmap = QPixmap();
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
}

QVariant XYCurvePrivate::itemChange(GraphicsItemChange change, const QVariant& value) {
	// signalize, that the curve was selected. Will be used to create a new InfoElement (Marker)
	if (change == QGraphicsItem::ItemSelectedChange) {
		if (value.toBool() && q->cSystem && q->cSystem->isValid()) {
			Q_EMIT q->selected(q->cSystem->mapSceneToLogical(mousePos).x());
		}
	}
	return QGraphicsItem::itemChange(change, value);
}

/*!
  Reimplementation of QGraphicsItem::paint(). This function does the actual painting of the curve.
  \sa QGraphicsItem::paint().
*/
void XYCurvePrivate::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget*) {
	if (!isVisible())
		return;

	painter->setPen(Qt::NoPen);
	painter->setBrush(Qt::NoBrush);
	painter->setRenderHint(QPainter::SmoothPixmapTransform, true);

	if (!q->isPrinting() && Settings::group(QStringLiteral("Settings_Worksheet")).readEntry<bool>("DoubleBuffering", true))
		painter->drawPixmap(boundingRectangle.topLeft(), m_pixmap); // draw the cached pixmap (fast)
	else
		draw(painter); // draw directly again (slow)

	if (m_hovered && !isSelected() && !q->isPrinting()) {
		if (m_hoverEffectImageIsDirty) {
			QPixmap pix = m_pixmap;
			QPainter p(&pix);
			p.setCompositionMode(QPainter::CompositionMode_SourceIn); // source (shadow) pixels merged with the alpha channel of the destination (m_pixmap)
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

void XYCurvePrivate::drawValues(QPainter* painter) {
	// QDEBUG(Q_FUNC_INFO << ", value strings = " << m_valueStrings)
	int i = 0;
	for (const auto& point : qAsConst(m_valuePoints)) {
		painter->translate(point);
		if (valuesRotationAngle != 0.)
			painter->rotate(-valuesRotationAngle);

		painter->drawText(QPoint(0, 0), m_valueStrings.at(i++));

		if (valuesRotationAngle != 0.)
			painter->rotate(valuesRotationAngle);
		painter->translate(-point);
	}
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

	if (q->activatePlot(event->pos())) {
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
	if (on == m_hovered)
		return; // don't update if state not changed

	m_hovered = on;
	on ? Q_EMIT q->hovered() : emit q->unhovered();
	update();
}

// ##############################################################################
// ##################  Serialization/Deserialization  ###########################
// ##############################################################################
//! Save as XML
void XYCurve::save(QXmlStreamWriter* writer) const {
	Q_D(const XYCurve);

	writer->writeStartElement(QStringLiteral("xyCurve"));
	writeBasicAttributes(writer);
	writeCommentElement(writer);

	// general
	writer->writeStartElement(QStringLiteral("general"));

	// if the data columns are valid, write their current paths.
	// if not, write the last used paths so the columns can be restored later
	// when the columns with the same path are added again to the project
	if (d->xColumn)
		writer->writeAttribute(QStringLiteral("xColumn"), d->xColumn->path());
	else
		writer->writeAttribute(QStringLiteral("xColumn"), d->xColumnPath);

	if (d->yColumn)
		writer->writeAttribute(QStringLiteral("yColumn"), d->yColumn->path());
	else
		writer->writeAttribute(QStringLiteral("yColumn"), d->yColumnPath);

	writer->writeAttribute(QStringLiteral("plotRangeIndex"), QString::number(m_cSystemIndex));
	writer->writeAttribute(QStringLiteral("legendVisible"), QString::number(d->legendVisible));
	writer->writeAttribute(QStringLiteral("visible"), QString::number(d->isVisible()));
	writer->writeEndElement();

	// Line
	writer->writeStartElement(QStringLiteral("lines"));
	writer->writeAttribute(QStringLiteral("type"), QString::number(static_cast<int>(d->lineType)));
	writer->writeAttribute(QStringLiteral("skipGaps"), QString::number(d->lineSkipGaps));
	writer->writeAttribute(QStringLiteral("increasingXOnly"), QString::number(d->lineIncreasingXOnly));
	writer->writeAttribute(QStringLiteral("interpolationPointsCount"), QString::number(d->lineInterpolationPointsCount));
	d->line->save(writer);
	writer->writeEndElement();

	// Drop lines
	d->dropLine->save(writer);

	// Symbols
	d->symbol->save(writer);

	// Values
	writer->writeStartElement(QStringLiteral("values"));
	writer->writeAttribute(QStringLiteral("type"), QString::number(static_cast<int>(d->valuesType)));
	writer->writeAttribute(QStringLiteral("valuesColumn"), d->valuesColumnPath);
	writer->writeAttribute(QStringLiteral("position"), QString::number(static_cast<int>(d->valuesPosition)));
	writer->writeAttribute(QStringLiteral("distance"), QString::number(d->valuesDistance));
	writer->writeAttribute(QStringLiteral("rotation"), QString::number(d->valuesRotationAngle));
	writer->writeAttribute(QStringLiteral("opacity"), QString::number(d->valuesOpacity));
	writer->writeAttribute(QStringLiteral("numericFormat"), QChar::fromLatin1(d->valuesNumericFormat));
	writer->writeAttribute(QStringLiteral("dateTimeFormat"), d->valuesDateTimeFormat);
	writer->writeAttribute(QStringLiteral("precision"), QString::number(d->valuesPrecision));
	writer->writeAttribute(QStringLiteral("prefix"), d->valuesPrefix);
	writer->writeAttribute(QStringLiteral("suffix"), d->valuesSuffix);
	WRITE_QCOLOR(d->valuesColor);
	WRITE_QFONT(d->valuesFont);
	writer->writeEndElement();

	// Filling
	d->background->save(writer);

	// Error bars
	writer->writeStartElement(QStringLiteral("errorBars"));
	writer->writeAttribute(QStringLiteral("xErrorType"), QString::number(static_cast<int>(d->xErrorType)));
	writer->writeAttribute(QStringLiteral("xErrorPlusColumn"), d->xErrorPlusColumnPath);
	writer->writeAttribute(QStringLiteral("xErrorMinusColumn"), d->xErrorMinusColumnPath);
	writer->writeAttribute(QStringLiteral("yErrorType"), QString::number(static_cast<int>(d->yErrorType)));
	writer->writeAttribute(QStringLiteral("yErrorPlusColumn"), d->yErrorPlusColumnPath);
	writer->writeAttribute(QStringLiteral("yErrorMinusColumn"), d->yErrorMinusColumnPath);
	d->errorBarsLine->save(writer);
	writer->writeEndElement();

	// margin plots
	writer->writeStartElement(QStringLiteral("margins"));
	writer->writeAttribute(QStringLiteral("rugEnabled"), QString::number(d->rugEnabled));
	writer->writeAttribute(QStringLiteral("rugOrientation"), QString::number(static_cast<int>(d->rugOrientation)));
	writer->writeAttribute(QStringLiteral("rugLength"), QString::number(d->rugLength));
	writer->writeAttribute(QStringLiteral("rugWidth"), QString::number(d->rugWidth));
	writer->writeAttribute(QStringLiteral("rugOffset"), QString::number(d->rugOffset));
	writer->writeEndElement();

	writer->writeEndElement(); // close "xyCurve" section
}

//! Load from XML
bool XYCurve::load(XmlStreamReader* reader, bool preview) {
	Q_D(XYCurve);

	if (!readBasicAttributes(reader))
		return false;

	QXmlStreamAttributes attribs;
	QString str;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == QLatin1String("xyCurve"))
			break;

		if (!reader->isStartElement())
			continue;

		if (reader->name() == QLatin1String("comment")) {
			if (!readCommentElement(reader))
				return false;
		} else if (reader->name() == QLatin1String("general")) {
			attribs = reader->attributes();
			READ_COLUMN(xColumn);
			READ_COLUMN(yColumn);
			READ_INT_VALUE("legendVisible", legendVisible, bool);

			str = attribs.value(QStringLiteral("visible")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("visible"));
			else
				d->setVisible(str.toInt());
			READ_INT_VALUE_DIRECT("plotRangeIndex", m_cSystemIndex, int);
		} else if (!preview && reader->name() == QLatin1String("lines")) {
			attribs = reader->attributes();

			READ_INT_VALUE("type", lineType, LineType);
			READ_INT_VALUE("skipGaps", lineSkipGaps, bool);
			READ_INT_VALUE("increasingXOnly", lineIncreasingXOnly, bool);
			READ_INT_VALUE("interpolationPointsCount", lineInterpolationPointsCount, int);
			d->line->load(reader, preview);
		} else if (!preview && reader->name() == QLatin1String("dropLines")) {
			d->dropLine->load(reader, preview);
		} else if (!preview && reader->name() == QLatin1String("symbols")) {
			d->symbol->load(reader, preview);
		} else if (!preview && reader->name() == QLatin1String("values")) {
			attribs = reader->attributes();

			READ_INT_VALUE("type", valuesType, ValuesType);
			READ_COLUMN(valuesColumn);

			READ_INT_VALUE("position", valuesPosition, ValuesPosition);
			READ_DOUBLE_VALUE("distance", valuesDistance);
			READ_DOUBLE_VALUE("rotation", valuesRotationAngle);
			READ_DOUBLE_VALUE("opacity", valuesOpacity);

			str = attribs.value(QStringLiteral("numericFormat")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("numericFormat"));
			else
				d->valuesNumericFormat = *(str.toLatin1().data());

			READ_STRING_VALUE("dateTimeFormat", valuesDateTimeFormat);
			READ_INT_VALUE("precision", valuesPrecision, int);

			// don't produce any warning if no prefix or suffix is set (empty string is allowed here in xml)
			d->valuesPrefix = attribs.value(QStringLiteral("prefix")).toString();
			d->valuesSuffix = attribs.value(QStringLiteral("suffix")).toString();

			READ_QCOLOR(d->valuesColor);
			READ_QFONT(d->valuesFont);
		} else if (!preview && reader->name() == QLatin1String("filling"))
			d->background->load(reader, preview);
		else if (!preview && reader->name() == QLatin1String("errorBars")) {
			attribs = reader->attributes();

			READ_INT_VALUE("xErrorType", xErrorType, ErrorType);
			READ_COLUMN(xErrorPlusColumn);
			READ_COLUMN(xErrorMinusColumn);

			READ_INT_VALUE("yErrorType", yErrorType, ErrorType);
			READ_COLUMN(yErrorPlusColumn);
			READ_COLUMN(yErrorMinusColumn);

			d->errorBarsLine->load(reader, preview);
		} else if (!preview && reader->name() == QLatin1String("margins")) {
			attribs = reader->attributes();

			READ_INT_VALUE("rugEnabled", rugEnabled, bool);
			READ_INT_VALUE("rugOrientation", rugOrientation, Orientation);
			READ_DOUBLE_VALUE("rugLength", rugLength);
			READ_DOUBLE_VALUE("rugWidth", rugWidth);
			READ_DOUBLE_VALUE("rugOffset", rugOffset);
		} else { // unknown element
			reader->raiseUnknownElementWarning();
			if (!reader->skipToEndElement())
				return false;
		}
	}

	return true;
}

// ##############################################################################
// #########################  Theme management ##################################
// ##############################################################################
void XYCurve::loadThemeConfig(const KConfig& config) {
	KConfigGroup group = config.group("XYCurve");

	const auto* plot = dynamic_cast<const CartesianPlot*>(parentAspect());
	if (!plot)
		return;
	const int index = plot->curveChildIndex(this);
	const QColor themeColor = plot->themeColorPalette(index);

	Q_D(XYCurve);
	d->m_suppressRecalc = true;

	d->line->loadThemeConfig(group, themeColor);
	d->dropLine->loadThemeConfig(group, themeColor);
	d->symbol->loadThemeConfig(group, themeColor);
	d->background->loadThemeConfig(group);
	d->errorBarsLine->loadThemeConfig(group, themeColor);

	// Values
	this->setValuesOpacity(group.readEntry("ValuesOpacity", 1.0));
	this->setValuesColor(group.readEntry("ValuesColor", themeColor));

	// margins
	if (plot->theme() == QLatin1String("Tufte")) {
		if (d->xColumn && d->xColumn->rowCount() < 100) {
			setRugEnabled(true);
			setRugOrientation(WorksheetElement::Orientation::Both);
		}
	} else
		setRugEnabled(false);

	d->m_suppressRecalc = false;
	d->recalcShapeAndBoundingRect();
}

void XYCurve::saveThemeConfig(const KConfig& config) {
	KConfigGroup group = config.group("XYCurve");
	Q_D(const XYCurve);

	d->line->saveThemeConfig(group);
	d->dropLine->saveThemeConfig(group);
	d->background->saveThemeConfig(group);
	d->symbol->saveThemeConfig(group);
	d->errorBarsLine->saveThemeConfig(group);

	// Values
	group.writeEntry("ValuesOpacity", this->valuesOpacity());
	group.writeEntry("ValuesColor", (QColor)this->valuesColor());
	group.writeEntry("ValuesFont", this->valuesFont());

	const int index = parentAspect()->indexOfChild<XYCurve>(this);
	if (index < 5) {
		KConfigGroup themeGroup = config.group("Theme");
		for (int i = index; i < 5; i++) {
			QString s = QStringLiteral("ThemePaletteColor") + QString::number(i + 1);
			themeGroup.writeEntry(s, (QColor)d->line->pen().color());
		}
	}
}
