/***************************************************************************
    File                 : BoxPlot.cpp
    Project              : LabPlot
    Description          : Box Plot
    --------------------------------------------------------------------
    Copyright            : (C) 2021 Alexander Semke (alexander.semke@web.de)
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

#include "BoxPlot.h"
#include "BoxPlotPrivate.h"
#include "backend/core/AbstractColumn.h"
#include "backend/core/column/Column.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/worksheet/plots/cartesian/CartesianCoordinateSystem.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/XmlStreamReader.h"
#include "tools/ImageTools.h"
#include "backend/lib/trace.h"

#include <QActionGroup>
#include <QPainter>
#include <QMenu>
#include <QGraphicsSceneMouseEvent>

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>

/**
 * \class BoxPlot
 * \brief Box Plot
 */

BoxPlot::BoxPlot(const QString& name) : WorksheetElement(name, AspectType::BoxPlot),
	d_ptr(new BoxPlotPrivate(this)) {

	init();
}

BoxPlot::BoxPlot(const QString& name, BoxPlotPrivate* dd)
	: WorksheetElement(name, AspectType::BoxPlot), d_ptr(dd) {

	init();
}

//no need to delete the d-pointer here - it inherits from QGraphicsItem
//and is deleted during the cleanup in QGraphicsScene
BoxPlot::~BoxPlot() = default;

void BoxPlot::init() {
	Q_D(BoxPlot);

	KConfig config;
	KConfigGroup group = config.group("BoxPlot");

	//general
	d->whiskersType = (BoxPlot::WhiskersType) group.readEntry("WhiskersType", (int)BoxPlot::IQR);
	d->orientation = (BoxPlot::Orientation) group.readEntry("Orientation", (int)BoxPlot::Orientation::Vertical);
	d->variableWidth = group.readEntry("VariableWidth", false);

	//box filling
	d->fillingEnabled = group.readEntry("FillingEnabled", false);
	d->fillingType = (PlotArea::BackgroundType) group.readEntry("FillingType", static_cast<int>(PlotArea::BackgroundType::Color));
	d->fillingColorStyle = (PlotArea::BackgroundColorStyle) group.readEntry("FillingColorStyle", static_cast<int>(PlotArea::BackgroundColorStyle::SingleColor));
	d->fillingImageStyle = (PlotArea::BackgroundImageStyle) group.readEntry("FillingImageStyle", static_cast<int>(PlotArea::BackgroundImageStyle::Scaled));
	d->fillingBrushStyle = (Qt::BrushStyle) group.readEntry("FillingBrushStyle", static_cast<int>(Qt::SolidPattern));
	d->fillingFileName = group.readEntry("FillingFileName", QString());
	d->fillingFirstColor = group.readEntry("FillingFirstColor", QColor(Qt::white));
	d->fillingSecondColor = group.readEntry("FillingSecondColor", QColor(Qt::black));
	d->fillingOpacity = group.readEntry("FillingOpacity", 1.0);

	//median line
	d->medianLinePen = QPen(group.readEntry("MedianLineColor", QColor(Qt::black)),
	                    group.readEntry("MedianLineWidth", Worksheet::convertToSceneUnits(1.0, Worksheet::Unit::Point)),
	                    (Qt::PenStyle) group.readEntry("BorderStyle", (int)Qt::SolidLine));
	d->medianLineOpacity = group.readEntry("BorderOpacity", 1.0);

	//box border
	d->borderPen = QPen(group.readEntry("BorderColor", QColor(Qt::black)),
	                    group.readEntry("BorderWidth", Worksheet::convertToSceneUnits(1.0, Worksheet::Unit::Point)),
	                    (Qt::PenStyle) group.readEntry("BorderStyle", (int)Qt::SolidLine));
	d->borderOpacity = group.readEntry("BorderOpacity", 1.0);

	//markers
	d->symbolOutliersStyle = (Symbol::Style)group.readEntry("SymbolOutliersStyle", static_cast<int>(Symbol::Style::Circle));
	d->symbolMeanStyle = (Symbol::Style)group.readEntry("SymbolMeanStyle", static_cast<int>(Symbol::Style::Cross));
	d->symbolsSize = group.readEntry("SymbolSize", Worksheet::convertToSceneUnits(5, Worksheet::Unit::Point));
	d->symbolsRotationAngle = group.readEntry("SymbolRotation", 0.0);
	d->symbolsOpacity = group.readEntry("SymbolOpacity", 1.0);
	d->symbolsBrush.setStyle( (Qt::BrushStyle)group.readEntry("SymbolFillingStyle", (int)Qt::SolidPattern) );
	d->symbolsBrush.setColor( group.readEntry("SymbolFillingColor", QColor(Qt::black)) );
	d->symbolsPen.setStyle( (Qt::PenStyle)group.readEntry("SymbolBorderStyle", (int)Qt::SolidLine) );
	d->symbolsPen.setColor( group.readEntry("SymbolBorderColor", QColor(Qt::black)) );
	d->symbolsPen.setWidthF( group.readEntry("SymbolBorderWidth", Worksheet::convertToSceneUnits(1.0, Worksheet::Unit::Point)) );

	//whiskers
	d->whiskersPen = QPen(group.readEntry("WhiskersColor", QColor(Qt::black)),
	                    group.readEntry("WhiskersWidth", Worksheet::convertToSceneUnits(1.0, Worksheet::Unit::Point)),
	                    (Qt::PenStyle) group.readEntry("WhiskersStyle", (int)Qt::SolidLine));
	d->whiskersOpacity = group.readEntry("WhiskersOpacity", 1.0);
	d->whiskersCapSize = group.readEntry("WhiskersCapSize", Worksheet::convertToSceneUnits(5.0, Worksheet::Unit::Point));
}

/*!
    Returns an icon to be used in the project explorer.
*/
QIcon BoxPlot::icon() const {
	return  QIcon::fromTheme(QLatin1String("draw-line"));
}

void BoxPlot::initActions() {
	visibilityAction = new QAction(i18n("Visible"), this);
	visibilityAction->setCheckable(true);
	connect(visibilityAction, &QAction::triggered, this, &BoxPlot::visibilityChangedSlot);

	//Orientation
	auto* orientationActionGroup = new QActionGroup(this);
	orientationActionGroup->setExclusive(true);
	connect(orientationActionGroup, &QActionGroup::triggered, this, &BoxPlot::orientationChangedSlot);

	orientationHorizontalAction = new QAction(QIcon::fromTheme(QLatin1String("labplot-axis-horizontal")), i18n("Horizontal"), orientationActionGroup);
	orientationHorizontalAction->setCheckable(true);

	orientationVerticalAction = new QAction(QIcon::fromTheme(QLatin1String("labplot-axis-vertical")), i18n("Vertical"), orientationActionGroup);
	orientationVerticalAction->setCheckable(true);
}

void BoxPlot::initMenus() {
	this->initActions();

	//Orientation
	orientationMenu = new QMenu(i18n("Orientation"));
	orientationMenu->setIcon(QIcon::fromTheme(QLatin1String("labplot-axis-horizontal")));
	orientationMenu->addAction(orientationHorizontalAction);
	orientationMenu->addAction(orientationVerticalAction);
}

QMenu* BoxPlot::createContextMenu() {
	if (!orientationMenu)
		initMenus();

	QMenu* menu = WorksheetElement::createContextMenu();
	QAction* firstAction = menu->actions().at(1); //skip the first action because of the "title-action"

	//Orientation
// 	if (d->orientation == Orientation::Horizontal)
// 		orientationHorizontalAction->setChecked(true);
// 	else
// 		orientationVerticalAction->setChecked(true);
	menu->insertMenu(firstAction, orientationMenu);

	//Visibility
	visibilityAction->setChecked(isVisible());
	menu->insertAction(firstAction, visibilityAction);

	return menu;
}

QGraphicsItem* BoxPlot::graphicsItem() const {
	return d_ptr;
}

void BoxPlot::retransform() {
	Q_D(BoxPlot);
	d->retransform();
}

void BoxPlot::recalc() {
	Q_D(BoxPlot);
	d->recalc();
}

void BoxPlot::handleResize(double horizontalRatio, double verticalRatio, bool pageResize) {
	Q_UNUSED(horizontalRatio)
	Q_UNUSED(verticalRatio)
	Q_UNUSED(pageResize)
}

/* ============================ getter methods ================= */
//general
BASIC_SHARED_D_READER_IMPL(BoxPlot, QVector<const AbstractColumn*>, dataColumns, dataColumns)
BASIC_SHARED_D_READER_IMPL(BoxPlot, BoxPlot::Orientation, orientation, orientation)
BASIC_SHARED_D_READER_IMPL(BoxPlot, BoxPlot::WhiskersType, whiskersType, whiskersType)
BASIC_SHARED_D_READER_IMPL(BoxPlot, bool, variableWidth, variableWidth)

//box
//filling
BASIC_SHARED_D_READER_IMPL(BoxPlot, bool, fillingEnabled, fillingEnabled)
BASIC_SHARED_D_READER_IMPL(BoxPlot, PlotArea::BackgroundType, fillingType, fillingType)
BASIC_SHARED_D_READER_IMPL(BoxPlot, PlotArea::BackgroundColorStyle, fillingColorStyle, fillingColorStyle)
BASIC_SHARED_D_READER_IMPL(BoxPlot, PlotArea::BackgroundImageStyle, fillingImageStyle, fillingImageStyle)
BASIC_SHARED_D_READER_IMPL(BoxPlot, Qt::BrushStyle, fillingBrushStyle, fillingBrushStyle)
BASIC_SHARED_D_READER_IMPL(BoxPlot, QColor, fillingFirstColor, fillingFirstColor)
BASIC_SHARED_D_READER_IMPL(BoxPlot, QColor, fillingSecondColor, fillingSecondColor)
BASIC_SHARED_D_READER_IMPL(BoxPlot, QString, fillingFileName, fillingFileName)
BASIC_SHARED_D_READER_IMPL(BoxPlot, qreal, fillingOpacity, fillingOpacity)

//border
BASIC_SHARED_D_READER_IMPL(BoxPlot, QPen, borderPen, borderPen)
BASIC_SHARED_D_READER_IMPL(BoxPlot, qreal, borderOpacity, borderOpacity)

//median line
BASIC_SHARED_D_READER_IMPL(BoxPlot, QPen, medianLinePen, medianLinePen)
BASIC_SHARED_D_READER_IMPL(BoxPlot, qreal, medianLineOpacity, medianLineOpacity)

//markers
BASIC_SHARED_D_READER_IMPL(BoxPlot, Symbol::Style, symbolOutliersStyle, symbolOutliersStyle)
BASIC_SHARED_D_READER_IMPL(BoxPlot, Symbol::Style, symbolMeanStyle, symbolMeanStyle)
BASIC_SHARED_D_READER_IMPL(BoxPlot, qreal, symbolsOpacity, symbolsOpacity)
BASIC_SHARED_D_READER_IMPL(BoxPlot, qreal, symbolsRotationAngle, symbolsRotationAngle)
BASIC_SHARED_D_READER_IMPL(BoxPlot, qreal, symbolsSize, symbolsSize)
BASIC_SHARED_D_READER_IMPL(BoxPlot, QBrush, symbolsBrush, symbolsBrush)
BASIC_SHARED_D_READER_IMPL(BoxPlot, QPen, symbolsPen, symbolsPen)

//whiskers
BASIC_SHARED_D_READER_IMPL(BoxPlot, QPen, whiskersPen, whiskersPen)
BASIC_SHARED_D_READER_IMPL(BoxPlot, qreal, whiskersOpacity, whiskersOpacity)
BASIC_SHARED_D_READER_IMPL(BoxPlot, double, whiskersCapSize, whiskersCapSize)

QVector<QString>& BoxPlot::dataColumnPaths() const {
	return d_ptr->dataColumnPaths;
}

double BoxPlot::xMinimum() const {
	return d_ptr->xMinimum();
}

double BoxPlot::xMaximum() const {
	return d_ptr->xMaximum();
}

double BoxPlot::yMinimum() const {
	return d_ptr->yMinimum();
}

double BoxPlot::yMaximum() const {
	return d_ptr->yMaximum();
}

/* ============================ setter methods and undo commands ================= */

//General
STD_SETTER_CMD_IMPL_F_S(BoxPlot, SetDataColumns, QVector<const AbstractColumn*>, dataColumns, recalc)
void BoxPlot::setDataColumns(const QVector<const AbstractColumn*> columns) {
	Q_D(BoxPlot);
	if (columns != d->dataColumns) {
		exec(new BoxPlotSetDataColumnsCmd(d, columns, ki18n("%1: set data columns")));

		for (auto * column : columns) {
			if (!column)
				continue;

			connect(column, &AbstractColumn::dataChanged, this, &BoxPlot::dataChanged);

			//update the curve itself on changes
			connect(column, &AbstractColumn::dataChanged, this, &BoxPlot::recalc);
			connect(column->parentAspect(), &AbstractAspect::aspectAboutToBeRemoved,
					this, &BoxPlot::dataColumnAboutToBeRemoved);
			//TODO: add disconnect in the undo-function
		}
	}
}

STD_SETTER_CMD_IMPL_F_S(BoxPlot, SetOrientation, BoxPlot::Orientation, orientation, recalc)
void BoxPlot::setOrientation(BoxPlot::Orientation orientation) {
	Q_D(BoxPlot);
	if (orientation != d->orientation)
		exec(new BoxPlotSetOrientationCmd(d, orientation, ki18n("%1: set orientation")));
}

STD_SETTER_CMD_IMPL_F_S(BoxPlot, SetVariableWidth, bool, variableWidth, recalc)
void BoxPlot::setVariableWidth(bool variableWidth) {
	Q_D(BoxPlot);
	if (variableWidth != d->variableWidth)
		exec(new BoxPlotSetVariableWidthCmd(d, variableWidth, ki18n("%1: variable width changed")));
}

STD_SWAP_METHOD_SETTER_CMD_IMPL_F(BoxPlot, SetVisible, bool, swapVisible, update);
void BoxPlot::setVisible(bool on) {
	Q_D(BoxPlot);
	exec(new BoxPlotSetVisibleCmd(d, on, on ? ki18n("%1: set visible") : ki18n("%1: set invisible")));
}

bool BoxPlot::isVisible() const {
	Q_D(const BoxPlot);
	return d->isVisible();
}

//Filling
STD_SETTER_CMD_IMPL_F_S(BoxPlot, SetFillingEnabled, bool, fillingEnabled, updatePixmap)
void BoxPlot::setFillingEnabled(bool enabled) {
	Q_D(BoxPlot);
	if (enabled != d->fillingEnabled)
		exec(new BoxPlotSetFillingEnabledCmd(d, enabled, ki18n("%1: filling changed")));
}

STD_SETTER_CMD_IMPL_F_S(BoxPlot, SetFillingType, PlotArea::BackgroundType, fillingType, updatePixmap)
void BoxPlot::setFillingType(PlotArea::BackgroundType type) {
	Q_D(BoxPlot);
	if (type != d->fillingType)
		exec(new BoxPlotSetFillingTypeCmd(d, type, ki18n("%1: filling type changed")));
}

STD_SETTER_CMD_IMPL_F_S(BoxPlot, SetFillingColorStyle, PlotArea::BackgroundColorStyle, fillingColorStyle, updatePixmap)
void BoxPlot::setFillingColorStyle(PlotArea::BackgroundColorStyle style) {
	Q_D(BoxPlot);
	if (style != d->fillingColorStyle)
		exec(new BoxPlotSetFillingColorStyleCmd(d, style, ki18n("%1: filling color style changed")));
}

STD_SETTER_CMD_IMPL_F_S(BoxPlot, SetFillingImageStyle, PlotArea::BackgroundImageStyle, fillingImageStyle, updatePixmap)
void BoxPlot::setFillingImageStyle(PlotArea::BackgroundImageStyle style) {
	Q_D(BoxPlot);
	if (style != d->fillingImageStyle)
		exec(new BoxPlotSetFillingImageStyleCmd(d, style, ki18n("%1: filling image style changed")));
}

STD_SETTER_CMD_IMPL_F_S(BoxPlot, SetFillingBrushStyle, Qt::BrushStyle, fillingBrushStyle, updatePixmap)
void BoxPlot::setFillingBrushStyle(Qt::BrushStyle style) {
	Q_D(BoxPlot);
	if (style != d->fillingBrushStyle)
		exec(new BoxPlotSetFillingBrushStyleCmd(d, style, ki18n("%1: filling brush style changed")));
}

STD_SETTER_CMD_IMPL_F_S(BoxPlot, SetFillingFirstColor, QColor, fillingFirstColor, updatePixmap)
void BoxPlot::setFillingFirstColor(const QColor& color) {
	Q_D(BoxPlot);
	if (color!= d->fillingFirstColor)
		exec(new BoxPlotSetFillingFirstColorCmd(d, color, ki18n("%1: set filling first color")));
}

STD_SETTER_CMD_IMPL_F_S(BoxPlot, SetFillingSecondColor, QColor, fillingSecondColor, updatePixmap)
void BoxPlot::setFillingSecondColor(const QColor& color) {
	Q_D(BoxPlot);
	if (color!= d->fillingSecondColor)
		exec(new BoxPlotSetFillingSecondColorCmd(d, color, ki18n("%1: set filling second color")));
}

STD_SETTER_CMD_IMPL_F_S(BoxPlot, SetFillingFileName, QString, fillingFileName, updatePixmap)
void BoxPlot::setFillingFileName(const QString& fileName) {
	Q_D(BoxPlot);
	if (fileName!= d->fillingFileName)
		exec(new BoxPlotSetFillingFileNameCmd(d, fileName, ki18n("%1: set filling image")));
}

STD_SETTER_CMD_IMPL_F_S(BoxPlot, SetFillingOpacity, qreal, fillingOpacity, updatePixmap)
void BoxPlot::setFillingOpacity(qreal opacity) {
	Q_D(BoxPlot);
	if (opacity != d->fillingOpacity)
		exec(new BoxPlotSetFillingOpacityCmd(d, opacity, ki18n("%1: set filling opacity")));
}

//box border
STD_SETTER_CMD_IMPL_F_S(BoxPlot, SetBorderPen, QPen, borderPen, recalcShapeAndBoundingRect)
void BoxPlot::setBorderPen(const QPen &pen) {
	Q_D(BoxPlot);
	if (pen != d->borderPen)
		exec(new BoxPlotSetBorderPenCmd(d, pen, ki18n("%1: set border pen")));
}

STD_SETTER_CMD_IMPL_F_S(BoxPlot, SetBorderOpacity, qreal, borderOpacity, updatePixmap)
void BoxPlot::setBorderOpacity(qreal opacity) {
	Q_D(BoxPlot);
	if (opacity != d->borderOpacity)
		exec(new BoxPlotSetBorderOpacityCmd(d, opacity, ki18n("%1: set border opacity")));
}

//median line
STD_SETTER_CMD_IMPL_F_S(BoxPlot, SetMedianLinePen, QPen, medianLinePen, updatePixmap)
void BoxPlot::setMedianLinePen(const QPen& pen) {
	Q_D(BoxPlot);
	if (pen != d->medianLinePen)
		exec(new BoxPlotSetMedianLinePenCmd(d, pen, ki18n("%1: set median line pen")));
}

STD_SETTER_CMD_IMPL_F_S(BoxPlot, SetMedianLineOpacity, qreal, medianLineOpacity, updatePixmap)
void BoxPlot::setMedianLineOpacity(qreal opacity) {
	Q_D(BoxPlot);
	if (opacity != d->medianLineOpacity)
		exec(new BoxPlotSetMedianLineOpacityCmd(d, opacity, ki18n("%1: set median line opacity")));
}

//markers
STD_SETTER_CMD_IMPL_F_S(BoxPlot, SetSymbolOutliersStyle, Symbol::Style, symbolOutliersStyle, recalcShapeAndBoundingRect)
void BoxPlot::setSymbolOutliersStyle(Symbol::Style style) {
	Q_D(BoxPlot);
	if (style != d->symbolOutliersStyle)
		exec(new BoxPlotSetSymbolOutliersStyleCmd(d, style, ki18n("%1: set outliers symbol style")));
}

STD_SETTER_CMD_IMPL_F_S(BoxPlot, SetSymbolMeanStyle, Symbol::Style, symbolMeanStyle, recalcShapeAndBoundingRect)
void BoxPlot::setSymbolMeanStyle(Symbol::Style style) {
	Q_D(BoxPlot);
	if (style != d->symbolMeanStyle)
		exec(new BoxPlotSetSymbolMeanStyleCmd(d, style, ki18n("%1: set mean symbol style")));
}

STD_SETTER_CMD_IMPL_F_S(BoxPlot, SetSymbolsSize, qreal, symbolsSize, recalcShapeAndBoundingRect)
void BoxPlot::setSymbolsSize(qreal size) {
	Q_D(BoxPlot);
	if (!qFuzzyCompare(1 + size, 1 + d->symbolsSize))
		exec(new BoxPlotSetSymbolsSizeCmd(d, size, ki18n("%1: set symbol size")));
}

STD_SETTER_CMD_IMPL_F_S(BoxPlot, SetSymbolsRotationAngle, qreal, symbolsRotationAngle, recalcShapeAndBoundingRect)
void BoxPlot::setSymbolsRotationAngle(qreal angle) {
	Q_D(BoxPlot);
	if (!qFuzzyCompare(1 + angle, 1 + d->symbolsRotationAngle))
		exec(new BoxPlotSetSymbolsRotationAngleCmd(d, angle, ki18n("%1: rotate symbols")));
}

STD_SETTER_CMD_IMPL_F_S(BoxPlot, SetSymbolsBrush, QBrush, symbolsBrush, updatePixmap)
void BoxPlot::setSymbolsBrush(const QBrush& brush) {
	Q_D(BoxPlot);
	if (brush != d->symbolsBrush)
		exec(new BoxPlotSetSymbolsBrushCmd(d, brush, ki18n("%1: set symbol filling")));
}

STD_SETTER_CMD_IMPL_F_S(BoxPlot, SetSymbolsPen, QPen, symbolsPen, recalcShapeAndBoundingRect)
void BoxPlot::setSymbolsPen(const QPen& pen) {
	Q_D(BoxPlot);
	if (pen != d->symbolsPen)
		exec(new BoxPlotSetSymbolsPenCmd(d, pen, ki18n("%1: set symbol outline style")));
}

STD_SETTER_CMD_IMPL_F_S(BoxPlot, SetSymbolsOpacity, qreal, symbolsOpacity, updatePixmap)
void BoxPlot::setSymbolsOpacity(qreal opacity) {
	Q_D(BoxPlot);
	if (opacity != d->symbolsOpacity)
		exec(new BoxPlotSetSymbolsOpacityCmd(d, opacity, ki18n("%1: set symbols opacity")));
}

//whiskers
STD_SETTER_CMD_IMPL_F_S(BoxPlot, SetWhiskersType, BoxPlot::WhiskersType, whiskersType, recalc)
void BoxPlot::setWhiskersType(BoxPlot::WhiskersType type) {
	Q_D(BoxPlot);
	if (type != d->whiskersType)
		exec(new BoxPlotSetWhiskersTypeCmd(d, type, ki18n("%1: set whiskers type")));
}

STD_SETTER_CMD_IMPL_F_S(BoxPlot, SetWhiskersPen, QPen, whiskersPen, recalcShapeAndBoundingRect)
void BoxPlot::setWhiskersPen(const QPen& pen) {
	Q_D(BoxPlot);
	if (pen != d->whiskersPen)
		exec(new BoxPlotSetWhiskersPenCmd(d, pen, ki18n("%1: set whiskers pen")));
}

STD_SETTER_CMD_IMPL_F_S(BoxPlot, SetWhiskersOpacity, qreal, whiskersOpacity, updatePixmap)
void BoxPlot::setWhiskersOpacity(qreal opacity) {
	Q_D(BoxPlot);
	if (opacity != d->whiskersOpacity)
		exec(new BoxPlotSetWhiskersOpacityCmd(d, opacity, ki18n("%1: set whiskers opacity")));
}

STD_SETTER_CMD_IMPL_F_S(BoxPlot, SetWhiskersCapSize, double, whiskersCapSize, recalc)
void BoxPlot::setWhiskersCapSize(double size) {
	Q_D(BoxPlot);
	if (size != d->whiskersCapSize)
		exec(new BoxPlotSetWhiskersCapSizeCmd(d, size, ki18n("%1: set whiskers cap size")));
}

//##############################################################################
//#################################  SLOTS  ####################################
//##############################################################################

void BoxPlot::dataColumnAboutToBeRemoved(const AbstractAspect* aspect) {
	Q_D(BoxPlot);
	for (int i = 0; i < d->dataColumns.size(); ++i) {
		if (aspect == d->dataColumns.at(i)) {
			d->dataColumns[i] = nullptr;
			d->retransform();
			break;
		}
	}
}

//##############################################################################
//######  SLOTs for changes triggered via QActions in the context menu  ########
//##############################################################################
void BoxPlot::orientationChangedSlot(QAction* action) {
	Q_UNUSED(action)
	//TODO
}

void BoxPlot::visibilityChangedSlot() {
	Q_D(const BoxPlot);
	this->setVisible(!d->isVisible());
}

//##############################################################################
//####################### Private implementation ###############################
//##############################################################################
BoxPlotPrivate::BoxPlotPrivate(BoxPlot* owner) : q(owner) {
	setFlag(QGraphicsItem::ItemIsSelectable);
	setAcceptHoverEvents(true);
}

QString BoxPlotPrivate::name() const {
	return q->name();
}

/*!
  called when the size of the plot or its data ranges (manual changes, zooming, etc.) were changed.
  recalculates the position of the scene points to be drawn.
  triggers the update of lines, drop lines, symbols etc.
*/
void BoxPlotPrivate::retransform() {
	if (m_suppressRetransform || !isVisible())
		return;

	PERFTRACE(name().toLatin1() + ", BoxPlotPrivate::retransform()");

	const int count = dataColumns.size();
	for (int i = 0; i < count; ++i) {
		m_boxRect[i].clear();
		m_medianLine[i] = QLineF();
		m_whiskersPath[i] = QPainterPath();
		m_outliersCount[i] = 0;
		m_outliersSymbolPoints[i].clear();
	}

	if (count) {
		if (orientation == BoxPlot::Orientation::Vertical) {
			for (int i = 0; i < count; ++i) {
				if (dataColumns.at(i))
					verticalBoxPlot(i);
			}
		} else {
			for (int i = 0; i < count; ++i) {
				if (dataColumns.at(i))
					horizontalBoxPlot(i);
			}
		}
	}

	recalcShapeAndBoundingRect();
}

void BoxPlotPrivate::recalc() {
	PERFTRACE(name().toLatin1() + ", BoxPlotPrivate::recalc()");

	//resize the internal containers
	const int count = dataColumns.size();
	m_boxRect.resize(count);
	m_xMinBox.resize(count);
	m_xMaxBox.resize(count);
	m_yMinBox.resize(count);
	m_yMaxBox.resize(count);
	m_median.resize(count);
	m_medianLine.resize(count);
	m_whiskersPath.resize(count);
	m_whiskerMin.resize(count);
	m_whiskerMax.resize(count);
	m_outliersSymbolPointsLogical.resize(count);
	m_outliersSymbolPoints.resize(count);
	m_outliersCount.resize(count);
	m_mean.resize(count);
	m_meanSymbolPoint.resize(count);
	m_meanSymbolPointVisible.resize(count);

	m_yMin = INFINITY;
	m_yMax = -INFINITY;

	//calculate the new min and max values of the box plot
	//for the current sizes of the box and of the whiskers
	if (orientation == BoxPlot::Orientation::Vertical) {
		m_xMin = 0.0;
		m_xMax = count;
		m_yMin = INFINITY;
		m_yMax = -INFINITY;
	} else { //horizontal
		m_xMin = INFINITY;
		m_xMax = -INFINITY;
		m_yMin = 0.0;
		m_yMax = count;
	}

	if (variableWidth) {
		m_widthScaleFactor = -qInf();
		for (const auto* col : dataColumns) {
			auto* column = static_cast<const Column*>(col);
			if (column->statistics().size > m_widthScaleFactor)
				m_widthScaleFactor = column->statistics().size;
		}
		m_widthScaleFactor = std::sqrt(m_widthScaleFactor);
	}

	for (int i = 0; i < count; ++i)
		recalc(i);

	//the size of the boxplot changed because of the actual
	//data changes or because of new boxplot settings.
	//emit dataChanged() in order to recalculate everything
	//in the parent plot with the new size/shape of the boxplot
	emit q->dataChanged();
}

void BoxPlotPrivate::setOutlierPoint(QPointF& point, double pos, double value) {
	if (orientation == BoxPlot::Orientation::Vertical) {
		point.setX(pos);
		point.setY(value);

		if (value > m_yMax)
			m_yMax = value;
		else if (value < m_yMin)
			m_yMin = value;
	} else {
		point.setX(value);
		point.setY(pos);

		if (value > m_xMax)
			m_xMax = value;
		else if (value < m_xMin)
			m_xMin = value;
	}
}

void BoxPlotPrivate::recalc(int index) {
	PERFTRACE(name().toLatin1() + ", BoxPlotPrivate::recalc(index)");
	auto* column = static_cast<const Column*>(dataColumns.at(index));
	if (!column)
		return;

	const auto& statistics = column->statistics();
	double width = 0.5;
	if (variableWidth && m_widthScaleFactor != 0)
		width *= std::sqrt(statistics.size)/m_widthScaleFactor;

	const double x = index + 0.5;

	//box
	if (orientation == BoxPlot::Orientation::Vertical) {
		m_xMinBox[index] = x - width/2;
		m_xMaxBox[index] = x + width/2;
		m_yMinBox[index] = statistics.firstQuartile;
		m_yMaxBox[index] = statistics.thirdQuartile;
	} else {
		m_xMinBox[index] = statistics.firstQuartile;
		m_xMaxBox[index] = statistics.thirdQuartile;
		m_yMinBox[index] = x - width/2;
		m_yMaxBox[index] = x + width/2;
	}

	//mean and median
	m_median[index] = statistics.median;
	m_mean[index] = statistics.arithmeticMean;

	//whiskers
	switch (whiskersType) {
	case BoxPlot::WhiskersType::MinMax: {
		m_whiskerMax[index] = statistics.maximum;
		m_whiskerMin[index] = statistics.minimum;
		break;
	}
	case BoxPlot::WhiskersType::IQR: {
		if (orientation == BoxPlot::Orientation::Vertical) {
			m_whiskerMax[index] = m_yMaxBox.at(index) + 1.5*statistics.iqr;
			m_whiskerMin[index] = m_yMinBox.at(index) - 1.5*statistics.iqr;
		} else {
			m_whiskerMax[index] = m_xMaxBox.at(index) + 1.5*statistics.iqr;
			m_whiskerMin[index] = m_xMinBox.at(index) - 1.5*statistics.iqr;
		}
		break;
	}
	case BoxPlot::WhiskersType::STDDEV: {
		if (orientation == BoxPlot::Orientation::Vertical) {
			m_whiskerMax[index] = m_yMaxBox.at(index) + statistics.standardDeviation;
			m_whiskerMin[index] = m_yMinBox.at(index) - statistics.standardDeviation;
		} else {
			m_whiskerMax[index] = m_xMaxBox.at(index) + statistics.standardDeviation;
			m_whiskerMin[index] = m_xMinBox.at(index) - statistics.standardDeviation;
		}
		break;
	}
	}

	//outliers symbols
	m_outliersCount[index] = 0;
	m_outliersSymbolPointsLogical[index].clear();
	if (orientation == BoxPlot::Orientation::Vertical) {
		if (m_whiskerMax[index] > m_yMax)
			m_yMax = m_whiskerMax[index];
		if (m_whiskerMin[index] < m_yMin)
			m_yMin = m_whiskerMin[index];
	} else {
		if (m_whiskerMax[index] > m_xMax)
			m_xMax = m_whiskerMax[index];
		if (m_whiskerMin[index] < m_xMin)
			m_xMin = m_whiskerMin[index];
	}

	if (whiskersType != BoxPlot::WhiskersType::MinMax) {
		double whiskerMax = - qInf(); //upper adjacent value
		double whiskerMin = qInf(); //lower adjacent value
		switch (column->columnMode()) {
		case AbstractColumn::ColumnMode::Numeric:
		case AbstractColumn::ColumnMode::Integer:
		case AbstractColumn::ColumnMode::BigInt:
			for (int row = 0; row < column->rowCount(); ++row) {
				if ( column->isValid(row) && !column->isMasked(row) ) {
					const double value = column->valueAt(row);
					if (value > m_whiskerMax.at(index) || value < m_whiskerMin.at(index)) {
						QPointF point;
						setOutlierPoint(point, x, value);
						++m_outliersCount[index];
						if (m_outliersSymbolPointsLogical[index].indexOf(point) == -1)
							m_outliersSymbolPointsLogical[index] << point;
					} else if (whiskersType == BoxPlot::WhiskersType::IQR) {
						//determine the upper/lower adjucent values
						if (value > whiskerMax)
							whiskerMax = value;
						else if (value < whiskerMin)
							whiskerMin = value;
					}
				}
			}
			break;
		case AbstractColumn::ColumnMode::DateTime:
			for (int row = 0; row < column->rowCount(); ++row) {
				if ( column->isValid(row) && !column->isMasked(row) ){
					const double value = column->dateTimeAt(row).toMSecsSinceEpoch();
					if (value > m_whiskerMax.at(index) || value < m_whiskerMin.at(index)) {
						QPointF point;
						setOutlierPoint(point, x, value);
						++m_outliersCount[index];
						if (m_outliersSymbolPointsLogical[index].indexOf(point) == -1)
							m_outliersSymbolPointsLogical[index] << point;
					} else if (whiskersType == BoxPlot::WhiskersType::IQR) {
						//determine the upper/lower adjucent values
						if (value > whiskerMax)
							whiskerMax = value;
						else if (value < whiskerMin)
							whiskerMin = value;
					}
				}
			}
			break;
		case AbstractColumn::ColumnMode::Text:
		case AbstractColumn::ColumnMode::Month:
		case AbstractColumn::ColumnMode::Day:
			break;
		}

		//set the whisker ends at the upper and lower adjacent values
		if (whiskersType == BoxPlot::WhiskersType::IQR) {
			m_whiskerMax[index] = whiskerMax;
			m_whiskerMin[index] = whiskerMin;
		}
	}
}

void BoxPlotPrivate::verticalBoxPlot(int index) {
	PERFTRACE(name().toLatin1() + ", BoxPlotPrivate::verticalBoxPlot()");

	QVector<QLineF> lines;
	const double x = index + 0.5;

	//box
	//first line starting at the top left corner of the box
	lines << QLineF(m_xMinBox.at(index), m_yMaxBox.at(index), m_xMaxBox.at(index), m_yMaxBox.at(index));
	lines << QLineF(m_xMaxBox.at(index), m_yMaxBox.at(index), m_xMaxBox.at(index), m_yMinBox.at(index));
	lines << QLineF(m_xMaxBox.at(index), m_yMinBox.at(index), m_xMinBox.at(index), m_yMinBox.at(index));
	lines << QLineF(m_xMinBox.at(index), m_yMinBox.at(index), m_xMinBox.at(index), m_yMaxBox.at(index));
	m_boxRect[index] = q->cSystem->mapLogicalToScene(lines);

	//median line
	lines.clear();
	lines << QLineF(m_xMinBox.at(index), m_median.at(index), m_xMaxBox.at(index), m_median.at(index));
	lines = q->cSystem->mapLogicalToScene(lines);
	if (!lines.isEmpty())
		m_medianLine[index] = lines.first();

	//whisker lines
	lines.clear();
	lines << QLineF(x, m_yMaxBox.at(index), x, m_whiskerMax.at(index)); //upper whisker
	lines << QLineF(x, m_yMinBox.at(index), x, m_whiskerMin.at(index)); //lower whisker
	lines = q->cSystem->mapLogicalToScene(lines);
	for (const auto& line : qAsConst(lines)) {
		m_whiskersPath[index].moveTo(line.p1());
		m_whiskersPath[index].lineTo(line.p2());
	}

	//whisker caps
	if (!m_whiskersPath[index].isEmpty()) {
		QPointF maxPoint = q->cSystem->mapLogicalToScene(QPointF(x, m_whiskerMax.at(index)));
		QPointF minPoint = q->cSystem->mapLogicalToScene(QPointF(x, m_whiskerMin.at(index)));
		m_whiskersPath[index].moveTo(QPointF(maxPoint.x() - whiskersCapSize/2., maxPoint.y()));
		m_whiskersPath[index].lineTo(QPointF(maxPoint.x() + whiskersCapSize/2., maxPoint.y()));

		m_whiskersPath[index].moveTo(QPointF(minPoint.x() - whiskersCapSize/2., minPoint.y()));
		m_whiskersPath[index].lineTo(QPointF(minPoint.x() + whiskersCapSize/2., minPoint.y()));
	}

	//outliers symbols
	mapOutliersToScene(index);

	//mean symbol
	QVector<QPointF> points = {QPointF(x, m_mean.at(index))};
	points = q->cSystem->mapLogicalToScene(points);
	if (points.count() == 1) {
		m_meanSymbolPoint[index] = points.at(0);
		m_meanSymbolPointVisible[index] = true;
	} else
		m_meanSymbolPointVisible[index] = false;
}

void BoxPlotPrivate::horizontalBoxPlot(int index) {
	PERFTRACE(name().toLatin1() + ", BoxPlotPrivate::horizontalBoxPlot()");

	QVector<QLineF> lines;
	const double y = index + 0.5;

	//box
	//first line starting at the top left corner of the box
	lines << QLineF(m_xMinBox.at(index), m_yMaxBox.at(index), m_xMaxBox.at(index), m_yMaxBox.at(index));
	lines << QLineF(m_xMaxBox.at(index), m_yMaxBox.at(index), m_xMaxBox.at(index), m_yMinBox.at(index));
	lines << QLineF(m_xMaxBox.at(index), m_yMinBox.at(index), m_xMinBox.at(index), m_yMinBox.at(index));
	lines << QLineF(m_xMinBox.at(index), m_yMinBox.at(index), m_xMinBox.at(index), m_yMaxBox.at(index));
	m_boxRect[index] = q->cSystem->mapLogicalToScene(lines);

	//median line
	lines << QLineF(m_median.at(index), m_yMinBox.at(index), m_median.at(index), m_yMaxBox.at(index));
	lines = q->cSystem->mapLogicalToScene(lines);
	if (!lines.isEmpty())
		m_medianLine[index] = lines.first();

	//whisker lines
	lines.clear();
	lines << QLineF(m_xMaxBox.at(index), y, m_whiskerMax.at(index), y); //upper whisker
	lines << QLineF(m_xMinBox.at(index), y, m_whiskerMin.at(index), y); //lower whisker
	lines = q->cSystem->mapLogicalToScene(lines);
	for (const auto& line : qAsConst(lines)) {
		m_whiskersPath[index].moveTo(line.p1());
		m_whiskersPath[index].lineTo(line.p2());
	}

	//whisker caps
	if (!m_whiskersPath[index].isEmpty()) {
		QPointF maxPoint = q->cSystem->mapLogicalToScene(QPointF(m_whiskerMax.at(index), y));
		QPointF minPoint = q->cSystem->mapLogicalToScene(QPointF(m_whiskerMin.at(index), y));
		m_whiskersPath[index].moveTo(QPointF(maxPoint.x(), maxPoint.y() - whiskersCapSize/2));
		m_whiskersPath[index].lineTo(QPointF(maxPoint.x(), maxPoint.y() + whiskersCapSize/2));

		m_whiskersPath[index].moveTo(QPointF(minPoint.x(), minPoint.y() - whiskersCapSize/2));
		m_whiskersPath[index].lineTo(QPointF(minPoint.x(), minPoint.y() + whiskersCapSize/2));
	}

	//outliers symbols
	mapOutliersToScene(index);

	//mean symbol
	QVector<QPointF> points = {QPointF(m_mean.at(index), y)};
	points = q->cSystem->mapLogicalToScene(points);
	if (points.count() == 1) {
		m_meanSymbolPoint[index] = points.at(0);
		m_meanSymbolPointVisible[index] = true;
	} else
		m_meanSymbolPointVisible[index] = false;
}

/*!
 * map the outlier points from logical to scene coordinates,
 * avoid drawing overlapping points, logic similar to
 * //XYCurvePrivate::retransform()
 */
void BoxPlotPrivate::mapOutliersToScene(int index) {
	const int numberOfPoints = m_outliersSymbolPointsLogical[index].size();
	if (numberOfPoints > 0) {
		const auto dataRect{ q->m_plot->dataRect() };
		const int numberOfPixelX = dataRect.width();
		const int numberOfPixelY = dataRect.height();

		if (numberOfPixelX <= 0 || numberOfPixelY <= 0)
			return;

		const int startIndex = 0;
		const int endIndex = m_outliersSymbolPointsLogical[index].size() - 1;
		const double minLogicalDiffX = dataRect.width()/numberOfPixelX;
		const double minLogicalDiffY = dataRect.height()/numberOfPixelY;

		// eliminate multiple scene points (size (numberOfPixelX + 1) * (numberOfPixelY + 1))
		QVector<QVector<bool>> scenePointsUsed(numberOfPixelX + 1);
		for (auto& col: scenePointsUsed)
			col.resize(numberOfPixelY + 1);

		QVector<bool> m_pointVisible;
		m_pointVisible.resize(numberOfPoints);
		q->cSystem->mapLogicalToScene(startIndex, endIndex, m_outliersSymbolPointsLogical[index], m_outliersSymbolPoints[index],
			m_pointVisible, scenePointsUsed, minLogicalDiffX, minLogicalDiffY);
	}
}

bool BoxPlotPrivate::swapVisible(bool on) {
	bool oldValue = isVisible();

	//When making a graphics item invisible, it gets deselected in the scene.
	//In this case we don't want to deselect the item in the project explorer.
	//We need to supress the deselection in the view.
	auto* worksheet = static_cast<Worksheet*>(q->parent(AspectType::Worksheet));
	worksheet->suppressSelectionChangedEvent(true);
	setVisible(on);
	worksheet->suppressSelectionChangedEvent(false);

// 	emit q->changed();
	emit q->visibilityChanged(on);
	return oldValue;
}

/*!
    Returns the outer bounds of the item as a rectangle.
 */
QRectF BoxPlotPrivate::boundingRect() const {
	return m_boundingRectangle;
}

/*!
    Returns the shape of this item as a QPainterPath in local coordinates.
*/
QPainterPath BoxPlotPrivate::shape() const {
	return m_boxPlotShape;
}

/*!
  recalculates the outer bounds and the shape of the item.
*/
void BoxPlotPrivate::recalcShapeAndBoundingRect() {
	prepareGeometryChange();
	m_boxPlotShape = QPainterPath();

	for (int i = 0; i < dataColumns.size(); ++i) {
		if (!dataColumns.at(i)
			|| static_cast<const Column*>(dataColumns.at(i))->statistics().size == 0)
			continue;

		QPainterPath boxPath;
		for (const auto& line : qAsConst(m_boxRect.at(i))) {
			boxPath.moveTo(line.p1());
			boxPath.lineTo(line.p2());
		}
		m_boxPlotShape.addPath(WorksheetElement::shapeFromPath(boxPath, borderPen));

		m_boxPlotShape.addPath(WorksheetElement::shapeFromPath(m_whiskersPath.at(i), whiskersPen));

		if (symbolOutliersStyle != Symbol::Style::NoSymbols) {
			QPainterPath symbolsPath = QPainterPath();
			QPainterPath path = Symbol::pathFromStyle(symbolOutliersStyle);

			QTransform trafo;
			trafo.scale(symbolsSize, symbolsSize);
			path = trafo.map(path);
			trafo.reset();

			if (symbolsRotationAngle != 0) {
				trafo.rotate(symbolsRotationAngle);
				path = trafo.map(path);
			}

			for (const auto& point : qAsConst(m_outliersSymbolPoints.at(i))) {
				trafo.reset();
				trafo.translate(point.x(), point.y());
				symbolsPath.addPath(trafo.map(path));
			}

			m_boxPlotShape.addPath(symbolsPath);
		}
	}

	m_boundingRectangle = m_boxPlotShape.boundingRect();
	updatePixmap();
}

double BoxPlotPrivate::xMinimum() {
	return m_xMin;
}

double BoxPlotPrivate::xMaximum() {
	return m_xMax;
}

double BoxPlotPrivate::yMinimum() {
	return m_yMin;
}

double BoxPlotPrivate::yMaximum() {
	return m_yMax;
}

void BoxPlotPrivate::updatePixmap() {
	PERFTRACE(name().toLatin1() + ", BoxPlotPrivate::updatePixmap()");
	QPixmap pixmap(m_boundingRectangle.width(), m_boundingRectangle.height());
	if (m_boundingRectangle.width() == 0 || m_boundingRectangle.height() == 0) {
		m_pixmap = pixmap;
		m_hoverEffectImageIsDirty = true;
		m_selectionEffectImageIsDirty = true;
		return;
	}
	pixmap.fill(Qt::transparent);
	QPainter painter(&pixmap);
	painter.setRenderHint(QPainter::Antialiasing, true);
	painter.translate(-m_boundingRectangle.topLeft());

	draw(&painter);
	painter.end();

	m_pixmap = pixmap;
	m_hoverEffectImageIsDirty = true;
	m_selectionEffectImageIsDirty = true;
	update();
}

void BoxPlotPrivate::draw(QPainter* painter) {
	PERFTRACE(name().toLatin1() + ", BoxPlotPrivate::draw()");

	for (int i = 0; i < dataColumns.size(); ++i) {
		//no need to draw anything if the column doesn't have any valid values
		if (static_cast<const Column*>(dataColumns.at(i))->statistics().size == 0)
			continue;

		if (!m_boxRect.at(i).isEmpty()) {
			//draw the box filling
			if (fillingEnabled) {
				painter->setOpacity(fillingOpacity);
				painter->setPen(Qt::SolidLine);
				drawFilling(painter, i);
			}

			//draw the border
			if (borderPen.style() != Qt::NoPen) {
				painter->setPen(borderPen);
				painter->setBrush(Qt::NoBrush);
				painter->setOpacity(borderOpacity);
				for (const auto& line : m_boxRect.at(i))
					painter->drawLine(line);
			}

			//draw the median line
			if (medianLinePen.style() != Qt::NoPen) {
				painter->setPen(medianLinePen);
				painter->setBrush(Qt::NoBrush);
				painter->setOpacity(medianLineOpacity);
				painter->drawLine(m_medianLine.at(i));
			}
		}

		//draw the whiskers
		if (whiskersPen.style() != Qt::NoPen && !m_whiskersPath.at(i).isEmpty()) {
			painter->setPen(whiskersPen);
			painter->setBrush(Qt::NoBrush);
			painter->setOpacity(whiskersOpacity);
			painter->drawPath(m_whiskersPath.at(i));
		}

		//draw the symbols for the outliers and for the mean
		if (symbolOutliersStyle != Symbol::Style::NoSymbols || symbolMeanStyle != Symbol::Style::NoSymbols) {
			painter->setOpacity(symbolsOpacity);
			painter->setPen(symbolsPen);
			painter->setBrush(symbolsBrush);
			drawSymbols(painter, i);
		}
	}
}

void BoxPlotPrivate::drawSymbols(QPainter* painter, int index) {

	//outliers
	if (symbolOutliersStyle != Symbol::Style::NoSymbols) {
		QPainterPath path = Symbol::pathFromStyle(symbolOutliersStyle);
		QTransform trafo;
		trafo.scale(symbolsSize, symbolsSize);
		if (symbolsRotationAngle != 0)
			trafo.rotate(-symbolsRotationAngle);

		path = trafo.map(path);

		for (const auto& point : qAsConst(m_outliersSymbolPoints.at(index))) {
			trafo.reset();
			trafo.translate(point.x(), point.y());
			painter->drawPath(trafo.map(path));
		}
	}

	//mean
	if (symbolMeanStyle != Symbol::Style::NoSymbols && m_meanSymbolPointVisible.at(index)) {
		QTransform trafo;
		trafo.scale(symbolsSize, symbolsSize);
		QPainterPath path = Symbol::pathFromStyle(symbolMeanStyle);
		if (symbolsRotationAngle != 0)
			trafo.rotate(-symbolsRotationAngle);

		path = trafo.map(path);

		trafo.reset();
		trafo.translate(m_meanSymbolPoint.at(index).x(), m_meanSymbolPoint.at(index).y());
		painter->drawPath(trafo.map(path));
	}
}

void BoxPlotPrivate::drawFilling(QPainter* painter, int index) {
	PERFTRACE(name().toLatin1() + ", BoxPlotPrivate::drawFilling()");

	if (m_boxRect.at(index).isEmpty())
		return;

	QPainterPath boxPath;
	for (const auto& line : qAsConst(m_boxRect.at(index))) {
		boxPath.moveTo(line.p1());
		boxPath.lineTo(line.p2());
	}
	const QRectF& rect = boxPath.boundingRect();

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
				pix = pix.scaled(rect.size().toSize(),Qt::KeepAspectRatioByExpanding,Qt::SmoothTransformation);
				painter->setBrush(QBrush(pix));
				painter->setBrushOrigin(pix.size().width()/2,pix.size().height()/2);
				break;
			case PlotArea::BackgroundImageStyle::Scaled:
				pix = pix.scaled(rect.size().toSize(),Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
				painter->setBrush(QBrush(pix));
				painter->setBrushOrigin(pix.size().width()/2,pix.size().height()/2);
				break;
			case PlotArea::BackgroundImageStyle::ScaledAspectRatio:
				pix = pix.scaled(rect.size().toSize(),Qt::KeepAspectRatio,Qt::SmoothTransformation);
				painter->setBrush(QBrush(pix));
				painter->setBrushOrigin(pix.size().width()/2,pix.size().height()/2);
				break;
			case PlotArea::BackgroundImageStyle::Centered: {
				QPixmap backpix(rect.size().toSize());
				backpix.fill();
				QPainter p(&backpix);
				p.drawPixmap(QPointF(0,0),pix);
				p.end();
				painter->setBrush(QBrush(backpix));
				painter->setBrushOrigin(-pix.size().width()/2,-pix.size().height()/2);
				break;
			}
			case PlotArea::BackgroundImageStyle::Tiled:
				painter->setBrush(QBrush(pix));
				break;
			case PlotArea::BackgroundImageStyle::CenterTiled:
				painter->setBrush(QBrush(pix));
				painter->setBrushOrigin(pix.size().width()/2,pix.size().height()/2);
			}
		}
	} else if (fillingType == PlotArea::BackgroundType::Pattern)
		painter->setBrush(QBrush(fillingFirstColor,fillingBrushStyle));

	painter->drawRect(rect);
}

void BoxPlotPrivate::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
	Q_UNUSED(option)
	Q_UNUSED(widget)

	if (!isVisible())
		return;

	painter->setPen(Qt::NoPen);
	painter->setBrush(Qt::NoBrush);
	painter->setRenderHint(QPainter::SmoothPixmapTransform, true);

	if ( KSharedConfig::openConfig()->group("Settings_Worksheet").readEntry<bool>("DoubleBuffering", true) )
		painter->drawPixmap(m_boundingRectangle.topLeft(), m_pixmap); //draw the cached pixmap (fast)
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

		painter->drawImage(m_boundingRectangle.topLeft(), m_hoverEffectImage, m_pixmap.rect());
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

		painter->drawImage(m_boundingRectangle.topLeft(), m_selectionEffectImage, m_pixmap.rect());
		return;
	}
}

void BoxPlotPrivate::contextMenuEvent(QGraphicsSceneContextMenuEvent* event) {
	q->createContextMenu()->exec(event->screenPos());
}

void BoxPlotPrivate::hoverEnterEvent(QGraphicsSceneHoverEvent*) {
	if (!isSelected()) {
		m_hovered = true;
		emit q->hovered();
		update();
	}
}

void BoxPlotPrivate::hoverLeaveEvent(QGraphicsSceneHoverEvent*) {
	if (m_hovered) {
		m_hovered = false;
		emit q->unhovered();
		update();
	}
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################
//! Save as XML
void BoxPlot::save(QXmlStreamWriter* writer) const {
	Q_D(const BoxPlot);

	writer->writeStartElement("boxPlot");
	writeBasicAttributes(writer);
	writeCommentElement(writer);

	//general
	writer->writeStartElement("general");
	for (auto* column : d->dataColumns) {
		writer->writeStartElement("column");
		writer->writeAttribute("path", column->path());
		writer->writeEndElement();
	}
	writer->writeAttribute("orientation", QString::number(static_cast<int>(d->orientation)));
	writer->writeAttribute("variableWidth", QString::number(d->variableWidth));
	writer->writeAttribute( "plotRangeIndex", QString::number(m_cSystemIndex) );
	writer->writeEndElement();

	//box filling
	writer->writeStartElement("filling");
	writer->writeAttribute( "enabled", QString::number(d->fillingEnabled) );
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

	//box border
	writer->writeStartElement("border");
	WRITE_QPEN(d->borderPen);
	writer->writeAttribute("opacity", QString::number(d->borderOpacity) );
	writer->writeEndElement();

	//median line
	writer->writeStartElement("medianLine");
	WRITE_QPEN(d->medianLinePen);
	writer->writeAttribute("opacity", QString::number(d->medianLineOpacity));
	writer->writeEndElement();

	//symbols for the outliers and for the mean
	writer->writeStartElement("symbols");
	writer->writeAttribute("symbolOutliersStyle", QString::number(static_cast<int>(d->symbolOutliersStyle)));
	writer->writeAttribute("symbolMeanStyle", QString::number(static_cast<int>(d->symbolMeanStyle)) );
	writer->writeAttribute("opacity", QString::number(d->symbolsOpacity));
	writer->writeAttribute("rotation", QString::number(d->symbolsRotationAngle));
	writer->writeAttribute("size", QString::number(d->symbolsSize));
	WRITE_QBRUSH(d->symbolsBrush);
	WRITE_QPEN(d->symbolsPen);
	writer->writeEndElement();

	//whiskers
	writer->writeStartElement("whiskers");
	writer->writeAttribute("type", QString::number(d->whiskersType));
	WRITE_QPEN(d->whiskersPen);
	writer->writeAttribute("opacity", QString::number(d->whiskersOpacity));
	writer->writeAttribute("capSize", QString::number(d->whiskersCapSize));
	writer->writeEndElement();

	writer->writeEndElement(); // close "BoxPlot" section
}

//! Load from XML
bool BoxPlot::load(XmlStreamReader* reader, bool preview) {
	Q_D(BoxPlot);

	if (!readBasicAttributes(reader))
		return false;

	KLocalizedString attributeWarning = ki18n("Attribute '%1' missing or empty, default value is used");
	QXmlStreamAttributes attribs;
	QString str;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == "boxPlot")
			break;

		if (!reader->isStartElement())
			continue;

		if (!preview && reader->name() == "comment") {
			if (!readCommentElement(reader)) return false;
		} else if (!preview && reader->name() == "general") {
			attribs = reader->attributes();

			READ_INT_VALUE("orientation", orientation, BoxPlot::Orientation);
			READ_INT_VALUE("variableWidth", variableWidth, bool);
			READ_INT_VALUE_DIRECT("plotRangeIndex", m_cSystemIndex, int);
		} else if (reader->name() == "column") {
			attribs = reader->attributes();

			str = attribs.value("path").toString();
			if (!str.isEmpty())
				d->dataColumnPaths << str;
// 			READ_COLUMN(dataColumn);
		} else if (!preview && reader->name() == "filling") {
			attribs = reader->attributes();

			READ_INT_VALUE("enabled", fillingEnabled, bool);
			READ_INT_VALUE("type", fillingType, PlotArea::BackgroundType);
			READ_INT_VALUE("colorStyle", fillingColorStyle, PlotArea::BackgroundColorStyle);
			READ_INT_VALUE("imageStyle", fillingImageStyle, PlotArea::BackgroundImageStyle);
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

			d->fillingFileName = attribs.value("fileName").toString();
			READ_DOUBLE_VALUE("opacity", fillingOpacity);
		} else if (!preview && reader->name() == "border") {
			attribs = reader->attributes();

			READ_QPEN(d->borderPen);
			READ_DOUBLE_VALUE("opacity", borderOpacity);
		} else if (!preview && reader->name() == "medianLine") {
			attribs = reader->attributes();

			READ_QPEN(d->medianLinePen);
			READ_DOUBLE_VALUE("opacity", medianLineOpacity);
		} else if (!preview && reader->name() == "symbols") {
			attribs = reader->attributes();

			READ_INT_VALUE("symbolOutliersStyle", symbolOutliersStyle, Symbol::Style);
			READ_INT_VALUE("symbolMeanStyle", symbolMeanStyle, Symbol::Style);
			READ_DOUBLE_VALUE("opacity", symbolsOpacity);
			READ_DOUBLE_VALUE("rotation", symbolsRotationAngle);
			READ_DOUBLE_VALUE("size", symbolsSize);

			READ_QBRUSH(d->symbolsBrush);
			READ_QPEN(d->symbolsPen);
		} else if (!preview && reader->name() == "whiskers") {
			attribs = reader->attributes();

			READ_INT_VALUE("type", whiskersType, BoxPlot::WhiskersType);
			READ_QPEN(d->whiskersPen);
			READ_DOUBLE_VALUE("opacity", whiskersOpacity);
			READ_DOUBLE_VALUE("capSize", whiskersCapSize);
		} else { // unknown element
			reader->raiseWarning(i18n("unknown element '%1'", reader->name().toString()));
			if (!reader->skipToEndElement()) return false;
		}
	}

	d->dataColumns.resize(d->dataColumnPaths.size());

	return true;
}

//##############################################################################
//#########################  Theme management ##################################
//##############################################################################
void BoxPlot::loadThemeConfig(const KConfig& config) {
	KConfigGroup group;
	if (config.hasGroup(QLatin1String("Theme")))
		group = config.group("XYCurve"); //when loading from the theme config, use the same properties as for XYCurve
	else
		group = config.group("BoxPlot");

	const auto* plot = static_cast<const CartesianPlot*>(parentAspect());
	int index = plot->curveChildIndex(this);
	QColor themeColor;
	if (index<plot->themeColorPalette().size())
		themeColor = plot->themeColorPalette().at(index);
	else {
		if (plot->themeColorPalette().size())
			themeColor = plot->themeColorPalette().last();
	}

	QPen p;

	Q_D(BoxPlot);
	d->m_suppressRecalc = false;

	//box border
	p.setStyle((Qt::PenStyle) group.readEntry("LineStyle", (int)Qt::SolidLine));
	p.setWidthF(group.readEntry("LineWidth", Worksheet::convertToSceneUnits(1.0, Worksheet::Unit::Point)));
	p.setColor(themeColor);
	setBorderPen(p);
	setBorderOpacity(group.readEntry("LineOpacity", 1.0));

	//box filling
	setFillingBrushStyle((Qt::BrushStyle)group.readEntry("FillingBrushStyle", (int)Qt::SolidPattern));
	setFillingColorStyle((PlotArea::BackgroundColorStyle)group.readEntry("FillingColorStyle", static_cast<int>(PlotArea::BackgroundColorStyle::SingleColor)));
	setFillingOpacity(group.readEntry("FillingOpacity", 1.0));
	setFillingFirstColor(themeColor);
	setFillingSecondColor(group.readEntry("FillingSecondColor", QColor(Qt::black)));
	setFillingType((PlotArea::BackgroundType)group.readEntry("FillingType", static_cast<int>(PlotArea::BackgroundType::Color)));

	//median line
	setMedianLinePen(p);
	setMedianLineOpacity(group.readEntry("LineOpacity", 1.0));

	//whiskers
	setWhiskersPen(p);
	setWhiskersOpacity(group.readEntry("LineOpacity", 1.0));

	//symbols for the mean and for the outliers
	this->setSymbolsOpacity(group.readEntry("SymbolOpacity", 1.0));
	QBrush brush;
	brush.setStyle((Qt::BrushStyle)group.readEntry("SymbolFillingStyle", (int)Qt::SolidPattern));
	brush.setColor(themeColor);
	this->setSymbolsBrush(brush);
	p.setStyle((Qt::PenStyle)group.readEntry("SymbolBorderStyle", (int)Qt::SolidLine));
	p.setColor(themeColor);
	p.setWidthF(group.readEntry("SymbolBorderWidth", Worksheet::convertToSceneUnits(1.0, Worksheet::Unit::Point)));
	this->setSymbolsPen(p);

	d->m_suppressRecalc = false;
	d->recalcShapeAndBoundingRect();
}
