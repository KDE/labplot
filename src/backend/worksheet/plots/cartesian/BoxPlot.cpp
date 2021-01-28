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
	d->whiskersType = (BoxPlot::WhiskersType) group.readEntry("WhiskersType", (int)BoxPlot::MinMax);

	//box filling
	d->fillingEnabled = group.readEntry("FillingEnabled", true);
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
	                    group.readEntry("MEdianLineWidth", Worksheet::convertToSceneUnits(1.0, Worksheet::Unit::Point)),
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
	d->symbolsPen.setWidthF( group.readEntry("SymbolBorderWidth", Worksheet::convertToSceneUnits(0.0, Worksheet::Unit::Point)) );

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
BASIC_SHARED_D_READER_IMPL(BoxPlot, const AbstractColumn*, dataColumn, dataColumn)
BASIC_SHARED_D_READER_IMPL(BoxPlot, BoxPlot::WhiskersType, whiskersType, whiskersType)

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
CLASS_SHARED_D_READER_IMPL(BoxPlot, QBrush, symbolsBrush, symbolsBrush)
CLASS_SHARED_D_READER_IMPL(BoxPlot, QPen, symbolsPen, symbolsPen)

//whiskers
BASIC_SHARED_D_READER_IMPL(BoxPlot, QPen, whiskersPen, whiskersPen)
BASIC_SHARED_D_READER_IMPL(BoxPlot, qreal, whiskersOpacity, whiskersOpacity)
BASIC_SHARED_D_READER_IMPL(BoxPlot, double, whiskersCapSize, whiskersCapSize)

QString& BoxPlot::dataColumnPath() const {
	return d_ptr->dataColumnPath;
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
STD_SETTER_CMD_IMPL_F_S(BoxPlot, SetDataColumn, const AbstractColumn*, dataColumn, recalc)
void BoxPlot::setDataColumn(const AbstractColumn* column) {
	Q_D(BoxPlot);
	if (column != d->dataColumn) {
		exec(new BoxPlotSetDataColumnCmd(d, column, ki18n("%1: set data column")));

		if (column) {
			connect(column, &AbstractColumn::dataChanged, this, &BoxPlot::dataChanged);

			//update the curve itself on changes
			connect(column, &AbstractColumn::dataChanged, this, &BoxPlot::recalc);
			connect(column->parentAspect(), &AbstractAspect::aspectAboutToBeRemoved,
					this, &BoxPlot::dataColumnAboutToBeRemoved);
			//TODO: add disconnect in the undo-function
		}
	}
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
STD_SETTER_CMD_IMPL_F_S(BoxPlot, SetBorderPen, QPen, borderPen, updatePixmap)
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
void BoxPlot::setMedianLinePen(const QPen &pen) {
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
STD_SETTER_CMD_IMPL_F_S(BoxPlot, SetSymbolOutliersStyle, Symbol::Style, symbolOutliersStyle, retransform)
void BoxPlot::setSymbolOutliersStyle(Symbol::Style style) {
	Q_D(BoxPlot);
	if (style != d->symbolOutliersStyle)
		exec(new BoxPlotSetSymbolOutliersStyleCmd(d, style, ki18n("%1: set outliers symbol style")));
}

STD_SETTER_CMD_IMPL_F_S(BoxPlot, SetSymbolMeanStyle, Symbol::Style, symbolMeanStyle, retransform)
void BoxPlot::setSymbolMeanStyle(Symbol::Style style) {
	Q_D(BoxPlot);
	if (style != d->symbolMeanStyle)
		exec(new BoxPlotSetSymbolMeanStyleCmd(d, style, ki18n("%1: set mean symbol style")));
}

STD_SETTER_CMD_IMPL_F_S(BoxPlot, SetSymbolsSize, qreal, symbolsSize, updatePixmap)
void BoxPlot::setSymbolsSize(qreal size) {
	Q_D(BoxPlot);
	if (!qFuzzyCompare(1 + size, 1 + d->symbolsSize))
		exec(new BoxPlotSetSymbolsSizeCmd(d, size, ki18n("%1: set symbol size")));
}

STD_SETTER_CMD_IMPL_F_S(BoxPlot, SetSymbolsRotationAngle, qreal, symbolsRotationAngle, updatePixmap)
void BoxPlot::setSymbolsRotationAngle(qreal angle) {
	Q_D(BoxPlot);
	if (!qFuzzyCompare(1 + angle, 1 + d->symbolsRotationAngle))
		exec(new BoxPlotSetSymbolsRotationAngleCmd(d, angle, ki18n("%1: rotate symbols")));
}

STD_SETTER_CMD_IMPL_F_S(BoxPlot, SetSymbolsBrush, QBrush, symbolsBrush, updatePixmap)
void BoxPlot::setSymbolsBrush(const QBrush &brush) {
	Q_D(BoxPlot);
	if (brush != d->symbolsBrush)
		exec(new BoxPlotSetSymbolsBrushCmd(d, brush, ki18n("%1: set symbol filling")));
}

STD_SETTER_CMD_IMPL_F_S(BoxPlot, SetSymbolsPen, QPen, symbolsPen, updatePixmap)
void BoxPlot::setSymbolsPen(const QPen &pen) {
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

STD_SETTER_CMD_IMPL_F_S(BoxPlot, SetWhiskersPen, QPen, whiskersPen, updatePixmap)
void BoxPlot::setWhiskersPen(const QPen &pen) {
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
	if (aspect == d->dataColumn) {
		d->dataColumn = nullptr;
		d->retransform();
	}
}

//##############################################################################
//######  SLOTs for changes triggered via QActions in the context menu  ########
//##############################################################################
void BoxPlot::orientationChangedSlot(QAction* action) {

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

	if (!dataColumn) {
		boxRect = QRectF();
		recalcShapeAndBoundingRect();
		return;
	}

	recalc();
	recalcShapeAndBoundingRect();
}

void BoxPlotPrivate::recalc() {
	PERFTRACE(name().toLatin1() + ", BoxPlotPrivate::recalc()");
	boxRect = QRectF();
	whiskersPath = QPainterPath();

	if (!dataColumn)
		return;

	if (orientation == BoxPlot::Orientation::Vertical)
		verticalBoxPlot();
	else
		horizontalBoxPlot();

	//the size of the boxplot changed because of the actual
	//data changes or because of new boxplot settings.
	//emit dataChanged() in order to recalculate everything
	//in the parent plot with the new size/shape of the boxplot
	emit q->dataChanged();
}

void BoxPlotPrivate::verticalBoxPlot() {
	const auto* plot = static_cast<const CartesianPlot*>(q->parentAspect());
	const auto* cSystem = static_cast<const CartesianCoordinateSystem*>(plot->defaultCoordinateSystem());
	const auto& statistics = static_cast<const Column*>(dataColumn)->statistics();
	QVector<QLineF> lines;

	const double x = 1.0;
	const double width = 0.5;

	//calculate the size and the position of the box
	double xMin = x - width/2;
	double xMax = x + width/2;
	double yMin = statistics.firstQuartile;
	double yMax = statistics.thirdQuartile;
	QPointF topLeft = QPointF(xMin, yMax);
	QPointF bottomRight = QPointF(xMax, yMin);

	//map the box to scene coordinates
	topLeft = cSystem->mapLogicalToScene(topLeft);
	bottomRight = cSystem->mapLogicalToScene(bottomRight);
	boxRect = QRectF(topLeft, bottomRight);

	//median line
	medianLine = QLineF();
	lines << QLineF(xMin, statistics.median, xMax, statistics.median);
	lines = cSystem->mapLogicalToScene(lines);
	if (!lines.isEmpty())
		medianLine = lines.first();

	//calculate the size and the position for the whiskers
	double whiskerMin = 0.;
	double whiskerMax = 0.;
	switch (whiskersType) {
	case BoxPlot::WhiskersType::MinMax: {
		whiskerMax = statistics.maximum;
		whiskerMin = statistics.minimum;
		break;
	}
	case BoxPlot::WhiskersType::IQR: {
		whiskerMax = yMax + 1.5*statistics.iqr;
		whiskerMin = yMin - 1.5*statistics.iqr;
		break;
	}
	case BoxPlot::WhiskersType::STDDEV: {
		whiskerMax = yMax + statistics.standardDeviation;
		whiskerMin = yMin - statistics.standardDeviation;
		break;
	}
	}

	lines.clear();
	lines << QLineF(x, yMax, x, whiskerMax); //upper whisker
	lines << QLineF(x, yMin, x, whiskerMin); //lower whisker
	lines = cSystem->mapLogicalToScene(lines);
	whiskersPath = QPainterPath();
	for (const auto& line : qAsConst(lines)) {
		whiskersPath.moveTo(line.p1());
		whiskersPath.lineTo(line.p2());
	}

	//add caps
	QPointF maxPoint = cSystem->mapLogicalToScene(QPointF(x, whiskerMax));
	QPointF minPoint = cSystem->mapLogicalToScene(QPointF(x, whiskerMin));
	whiskersPath.moveTo(QPointF(maxPoint.x() - whiskersCapSize/2., maxPoint.y()));
	whiskersPath.lineTo(QPointF(maxPoint.x() + whiskersCapSize/2., maxPoint.y()));

	whiskersPath.moveTo(QPointF(minPoint.x() - whiskersCapSize/2., minPoint.y()));
	whiskersPath.lineTo(QPointF(minPoint.x() + whiskersCapSize/2., minPoint.y()));

	//outliers symbols
	m_outliersSymbolPoints.clear();
	switch (dataColumn->columnMode()) {
	case AbstractColumn::ColumnMode::Numeric:
	case AbstractColumn::ColumnMode::Integer:
	case AbstractColumn::ColumnMode::BigInt:
		for (int row = 0; row < dataColumn->rowCount(); ++row) {
			if ( dataColumn->isValid(row) && !dataColumn->isMasked(row) ) {
				const double value = dataColumn->valueAt(row);
				if (value > whiskerMax || value < whiskerMin)
					m_outliersSymbolPoints << QPointF(x, value);
			}
		}
		break;
	case AbstractColumn::ColumnMode::DateTime:
		for (int row = 0; row < dataColumn->rowCount(); ++row) {
			if ( dataColumn->isValid(row) && !dataColumn->isMasked(row) ){
				const double value = dataColumn->dateTimeAt(row).toMSecsSinceEpoch();
				if (value > whiskerMax || value < whiskerMin)
					m_outliersSymbolPoints << QPointF(x, value);
			}
	}
		break;
	case AbstractColumn::ColumnMode::Text:
	case AbstractColumn::ColumnMode::Month:
	case AbstractColumn::ColumnMode::Day:
		break;
	}

	if (!m_outliersSymbolPoints.isEmpty())
		m_outliersSymbolPoints = cSystem->mapLogicalToScene(m_outliersSymbolPoints);

	//mean symbol
	m_meanSymbolPoint = cSystem->mapLogicalToScene(QPointF(x, statistics.median));

	//calculate the new min and max values of the box plot
	//for the current sizes of the box and of the whiskers
	m_xMin = xMin - 0.25;
	m_xMax = xMax + 0.25;
	m_yMin = whiskerMin;
	m_yMax = whiskerMax;
}

void BoxPlotPrivate::horizontalBoxPlot() {

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

// 	m_boundingRectangle = boxRect.united(whiskersPath.boundingRect());
	QPainterPath boxPath;
	boxPath.addRect(boxRect);
	m_boxPlotShape.addPath(WorksheetElement::shapeFromPath(boxPath, whiskersPen));
	m_boxPlotShape.addPath(WorksheetElement::shapeFromPath(whiskersPath, whiskersPen));

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

		for (const auto& point : qAsConst(m_outliersSymbolPoints)) {
			trafo.reset();
			trafo.translate(point.x(), point.y());
			symbolsPath.addPath(trafo.map(path));
		}

		m_boxPlotShape.addPath(symbolsPath);
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
}


void BoxPlotPrivate::draw(QPainter* painter) {
	PERFTRACE(name().toLatin1() + ", BoxPlotPrivate::draw()");

	//draw the box
	drawBox(painter);

	//draw the median line
	if (medianLinePen.style() != Qt::NoPen) {
		painter->setPen(medianLinePen);
		painter->setBrush(Qt::NoBrush);
		painter->setOpacity(medianLineOpacity);
		painter->drawLine(medianLine);
	}

	//draw the whiskers
	if (whiskersPen.style() != Qt::NoPen) {
		painter->setPen(whiskersPen);
		painter->setBrush(Qt::NoBrush);
		painter->setOpacity(whiskersOpacity);
		painter->drawPath(whiskersPath);
	}

	//draw the symbols for the outliers and for the mean
	if (symbolOutliersStyle != Symbol::Style::NoSymbols || symbolMeanStyle != Symbol::Style::NoSymbols) {
		painter->setOpacity(symbolsOpacity);
		painter->setPen(symbolsPen);
		painter->setBrush(symbolsBrush);
		drawSymbols(painter);
	}
}

void BoxPlotPrivate::drawSymbols(QPainter* painter) {
	//outliers
	QPainterPath path = Symbol::pathFromStyle(symbolOutliersStyle);

	QTransform trafo;
	trafo.scale(symbolsSize, symbolsSize);
	path = trafo.map(path);
	trafo.reset();
	if (symbolsRotationAngle != 0) {
		trafo.rotate(-symbolsRotationAngle);
		path = trafo.map(path);
	}
	for (const auto& point : qAsConst(m_outliersSymbolPoints)) {
		trafo.reset();
		trafo.translate(point.x(), point.y());
		painter->drawPath(trafo.map(path));
	}

	//mean
	path = Symbol::pathFromStyle(symbolMeanStyle);
	trafo.reset();
	trafo.translate(m_meanSymbolPoint.x(), m_meanSymbolPoint.y());
	painter->drawPath(trafo.map(path));
}

void BoxPlotPrivate::drawBox(QPainter* painter) {
	painter->setOpacity(fillingOpacity);
// 	painter->setPen(Qt::NoPen);
	const QRectF& rect = boxRect;
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

	//draw the border
	if (borderPen.style() != Qt::NoPen) {
		painter->setPen(borderPen);
		painter->setBrush(Qt::NoBrush);
		painter->setOpacity(borderOpacity);
		painter->drawRect(boxRect);
	}
}

void BoxPlotPrivate::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
	Q_UNUSED(option)
	Q_UNUSED(widget)

	if (!m_visible)
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
	WRITE_COLUMN(d->dataColumn, dataColumn);
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
	writer->writeAttribute("symbolsStyle", QString::number(static_cast<int>(d->symbolOutliersStyle)));
	writer->writeAttribute("symbolsStyle", QString::number(static_cast<int>(d->symbolMeanStyle)) );
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

			READ_COLUMN(dataColumn);
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

			READ_INT_VALUE("symbolOutlierssStyle", symbolOutliersStyle, Symbol::Style);
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

	if (!preview)
		retransform();

	return true;
}

//##############################################################################
//#########################  Theme management ##################################
//##############################################################################
void BoxPlot::loadThemeConfig(const KConfig& config) {
	KConfigGroup group;
	if (config.hasGroup(QLatin1String("Theme")))
		group = config.group("BoxPlot"); //when loading from the theme config, use the same properties as for BoxPlot
	else
		group = config.group("BoxPlot");


	int index = parentAspect()->indexOfChild<Histogram>(this);
	const auto* plot = static_cast<const CartesianPlot*>(parentAspect());
	QColor themeColor;
	if (index<plot->themeColorPalette().size())
		themeColor = plot->themeColorPalette().at(index);
	else {
		if (plot->themeColorPalette().size())
			themeColor = plot->themeColorPalette().last();
	}

	QPen p;

	Q_D(BoxPlot);

	//box border
	p.setStyle((Qt::PenStyle) group.readEntry("LineStyle", (int)Qt::SolidLine));
	p.setWidthF(group.readEntry("LineWidth", Worksheet::convertToSceneUnits(1.0, Worksheet::Unit::Point)));
	p.setWidthF(group.readEntry("LineWidth", borderPen().widthF()));
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

	d->recalcShapeAndBoundingRect();
}
