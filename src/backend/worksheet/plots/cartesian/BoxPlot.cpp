/*
    File                 : BoxPlot.cpp
    Project              : LabPlot
    Description          : Box Plot
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2021 Alexander Semke <alexander.semke@web.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "BoxPlot.h"
#include "BoxPlotPrivate.h"
#include "backend/core/AbstractColumn.h"
#include "backend/core/column/Column.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/worksheet/plots/cartesian/CartesianCoordinateSystem.h"
#include "backend/worksheet/plots/cartesian/Symbol.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/XmlStreamReader.h"
#include "tools/ImageTools.h"
#include "backend/lib/trace.h"

#include <QApplication>
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

BoxPlot::BoxPlot(const QString& name) : WorksheetElement(name, new BoxPlotPrivate(this), AspectType::BoxPlot)
{
	init();
}

BoxPlot::BoxPlot(const QString& name, BoxPlotPrivate* dd)
	: WorksheetElement(name, dd, AspectType::BoxPlot){

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
	d->ordering = (BoxPlot::Ordering) group.readEntry("Ordering", (int)BoxPlot::Ordering::None);
	d->whiskersType = (BoxPlot::WhiskersType) group.readEntry("WhiskersType", (int)BoxPlot::WhiskersType::IQR);
	d->whiskersRangeParameter= group.readEntry("WhiskersIQRParameter", 1.5);
	d->orientation = (BoxPlot::Orientation) group.readEntry("Orientation", (int)BoxPlot::Orientation::Vertical);
	d->variableWidth = group.readEntry("VariableWidth", false);
	d->widthFactor = group.readEntry("WidthFactor", 1.0);
	d->notchesEnabled = group.readEntry("NotchesEnabled", false);

	//box filling
	d->fillingEnabled = group.readEntry("FillingEnabled", true);
	d->fillingType = (WorksheetElement::BackgroundType) group.readEntry("FillingType", static_cast<int>(WorksheetElement::BackgroundType::Color));
	d->fillingColorStyle = (WorksheetElement::BackgroundColorStyle) group.readEntry("FillingColorStyle", static_cast<int>(WorksheetElement::BackgroundColorStyle::SingleColor));
	d->fillingImageStyle = (WorksheetElement::BackgroundImageStyle) group.readEntry("FillingImageStyle", static_cast<int>(WorksheetElement::BackgroundImageStyle::Scaled));
	d->fillingBrushStyle = (Qt::BrushStyle) group.readEntry("FillingBrushStyle", static_cast<int>(Qt::SolidPattern));
	d->fillingFileName = group.readEntry("FillingFileName", QString());
	d->fillingFirstColor = group.readEntry("FillingFirstColor", QColor(Qt::white));
	d->fillingSecondColor = group.readEntry("FillingSecondColor", QColor(Qt::black));
	d->fillingOpacity = group.readEntry("FillingOpacity", 0.5);

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
	d->symbolMean = new Symbol("symbolMean");
	addChild(d->symbolMean);
	d->symbolMean->setHidden(true);
	d->symbolMean->init(group);
	d->symbolMean->setStyle(Symbol::Style::Square);
	connect(d->symbolMean, &Symbol::updateRequested, [=]{d->recalcShapeAndBoundingRect();});
	connect(d->symbolMean, &Symbol::updatePixmapRequested, [=]{d->updatePixmap();});

	d->symbolMedian = new Symbol("symbolMedian");
	addChild(d->symbolMedian);
	d->symbolMedian->setHidden(true);
	d->symbolMedian->init(group);
	d->symbolMedian->setStyle(Symbol::Style::NoSymbols);
	connect(d->symbolMedian, &Symbol::updateRequested, [=]{d->recalcShapeAndBoundingRect();});
	connect(d->symbolMedian, &Symbol::updatePixmapRequested, [=]{d->updatePixmap();});

	d->symbolOutlier = new Symbol("symbolOutlier");
	addChild(d->symbolOutlier);
	d->symbolOutlier->setHidden(true);
	d->symbolOutlier->init(group);
	connect(d->symbolOutlier, &Symbol::updateRequested, [=]{d->recalcShapeAndBoundingRect();});
	connect(d->symbolOutlier, &Symbol::updatePixmapRequested, [=]{d->updatePixmap();});

	d->symbolFarOut = new Symbol("symbolFarOut");
	addChild(d->symbolFarOut);
	d->symbolFarOut->setHidden(true);
	d->symbolFarOut->init(group);
	d->symbolFarOut->setStyle(Symbol::Style::Plus);
	connect(d->symbolFarOut, &Symbol::updateRequested, [=]{d->recalcShapeAndBoundingRect();});
	connect(d->symbolFarOut, &Symbol::updatePixmapRequested, [=]{d->updatePixmap();});

	d->symbolData = new Symbol("symbolData");
	addChild(d->symbolData);
	d->symbolData->setHidden(true);
	d->symbolData->init(group);
	d->symbolData->setStyle(Symbol::Style::NoSymbols);
	d->symbolData->setOpacity(0.5);
	connect(d->symbolData, &Symbol::updateRequested, [=]{d->recalcShapeAndBoundingRect();});
	connect(d->symbolData, &Symbol::updatePixmapRequested, [=]{d->updatePixmap();});

	d->jitteringEnabled = group.readEntry("JitteringEnabled", true);

	//whiskers
	d->whiskersPen = QPen(group.readEntry("WhiskersColor", QColor(Qt::black)),
	                    group.readEntry("WhiskersWidth", Worksheet::convertToSceneUnits(1.0, Worksheet::Unit::Point)),
	                    (Qt::PenStyle) group.readEntry("WhiskersStyle", (int)Qt::SolidLine));
	d->whiskersOpacity = group.readEntry("WhiskersOpacity", 1.0);
	d->whiskersCapSize = group.readEntry("WhiskersCapSize", Worksheet::convertToSceneUnits(5.0, Worksheet::Unit::Point));
	d->whiskersCapPen = QPen(group.readEntry("WhiskersCapColor", QColor(Qt::black)),
	                    group.readEntry("WhiskersCapWidth", Worksheet::convertToSceneUnits(1.0, Worksheet::Unit::Point)),
	                    (Qt::PenStyle) group.readEntry("WhiskersCapStyle", (int)Qt::SolidLine));
	d->whiskersCapOpacity = group.readEntry("WhiskersCapOpacity", 1.0);
}

/*!
    Returns an icon to be used in the project explorer.
*/
QIcon BoxPlot::icon() const {
// 	return QIcon::fromTheme(QLatin1String("draw-line"));
	return BoxPlot::staticIcon();
}

QIcon BoxPlot::staticIcon() {
	QPainter pa;
	pa.setRenderHint(QPainter::Antialiasing);
	int iconSize = 20;
	QPixmap pm(iconSize, iconSize);

	QPen pen(Qt::SolidLine);
	const QColor& color = (QApplication::palette().color(QPalette::Base).lightness() < 128) ? Qt::white : Qt::black;
	pen.setColor(color);
	pen.setWidthF(0.0);

	pm.fill(Qt::transparent);
	pa.begin(&pm);
	pa.setPen(pen);
	pa.drawRect(6, 6, 8, 8); //box
	pa.drawLine(10, 6, 10, 0); //upper whisker
	pa.drawLine(10, 14, 10, 20); //lower whisker
	pa.end();

	return QIcon(pm);
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

void BoxPlot::handleResize(double /*horizontalRatio*/, double /*verticalRatio*/, bool /*pageResize*/) {
}

bool BoxPlot::activateCurve(QPointF mouseScenePos, double maxDist) {
	Q_D(BoxPlot);
	return d->activateCurve(mouseScenePos, maxDist);
}

void BoxPlot::setHover(bool on) {
	Q_D(BoxPlot);
	d->setHover(on);
}

/* ============================ getter methods ================= */
//general
BASIC_SHARED_D_READER_IMPL(BoxPlot, QVector<const AbstractColumn*>, dataColumns, dataColumns)
BASIC_SHARED_D_READER_IMPL(BoxPlot, BoxPlot::Ordering, ordering, ordering)
BASIC_SHARED_D_READER_IMPL(BoxPlot, BoxPlot::Orientation, orientation, orientation)
BASIC_SHARED_D_READER_IMPL(BoxPlot, bool, variableWidth, variableWidth)
BASIC_SHARED_D_READER_IMPL(BoxPlot, double, widthFactor, widthFactor)
BASIC_SHARED_D_READER_IMPL(BoxPlot, bool, notchesEnabled, notchesEnabled)

//box
//filling
BASIC_SHARED_D_READER_IMPL(BoxPlot, bool, fillingEnabled, fillingEnabled)
BASIC_SHARED_D_READER_IMPL(BoxPlot, WorksheetElement::BackgroundType, fillingType, fillingType)
BASIC_SHARED_D_READER_IMPL(BoxPlot, WorksheetElement::BackgroundColorStyle, fillingColorStyle, fillingColorStyle)
BASIC_SHARED_D_READER_IMPL(BoxPlot, WorksheetElement::BackgroundImageStyle, fillingImageStyle, fillingImageStyle)
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
Symbol* BoxPlot::symbolMean() const {
	Q_D(const BoxPlot);
	return d->symbolMean;
}

Symbol* BoxPlot::symbolMedian() const {
	Q_D(const BoxPlot);
	return d->symbolMedian;
}

Symbol* BoxPlot::symbolOutlier() const {
	Q_D(const BoxPlot);
	return d->symbolOutlier;
}

Symbol* BoxPlot::symbolFarOut() const {
	Q_D(const BoxPlot);
	return d->symbolFarOut;
}

BASIC_SHARED_D_READER_IMPL(BoxPlot, bool, jitteringEnabled, jitteringEnabled)

Symbol* BoxPlot::symbolData() const {
	Q_D(const BoxPlot);
	return d->symbolData;
}

//whiskers
BASIC_SHARED_D_READER_IMPL(BoxPlot, BoxPlot::WhiskersType, whiskersType, whiskersType)
BASIC_SHARED_D_READER_IMPL(BoxPlot, double, whiskersRangeParameter, whiskersRangeParameter)
BASIC_SHARED_D_READER_IMPL(BoxPlot, QPen, whiskersPen, whiskersPen)
BASIC_SHARED_D_READER_IMPL(BoxPlot, qreal, whiskersOpacity, whiskersOpacity)
BASIC_SHARED_D_READER_IMPL(BoxPlot, double, whiskersCapSize, whiskersCapSize)
BASIC_SHARED_D_READER_IMPL(BoxPlot, QPen, whiskersCapPen, whiskersCapPen)
BASIC_SHARED_D_READER_IMPL(BoxPlot, qreal, whiskersCapOpacity, whiskersCapOpacity)

QVector<QString>& BoxPlot::dataColumnPaths() const {
	D(BoxPlot);
	return d->dataColumnPaths;
}

double BoxPlot::xMinimum() const {
	D(BoxPlot);
	return d->xMin;
}

double BoxPlot::xMaximum() const {
	D(BoxPlot);
	return d->xMax;
}

double BoxPlot::yMinimum() const {
	D(BoxPlot);
	return d->yMin;
}

double BoxPlot::yMaximum() const {
	D(BoxPlot);
	return d->yMax;
}

/* ============================ setter methods and undo commands ================= */

//General
STD_SETTER_CMD_IMPL_F_S(BoxPlot, SetDataColumns, QVector<const AbstractColumn*>, dataColumns, recalc)
void BoxPlot::setDataColumns(const QVector<const AbstractColumn*> columns) {
	Q_D(BoxPlot);
	if (columns != d->dataColumns) {
		exec(new BoxPlotSetDataColumnsCmd(d, columns, ki18n("%1: set data columns")));

		for (auto* column : columns) {
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

STD_SETTER_CMD_IMPL_F_S(BoxPlot, SetOrdering, BoxPlot::Ordering, ordering, recalc)
void BoxPlot::setOrdering(BoxPlot::Ordering ordering) {
	Q_D(BoxPlot);
	if (ordering != d->ordering)
		exec(new BoxPlotSetOrderingCmd(d, ordering, ki18n("%1: set ordering")));
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

STD_SETTER_CMD_IMPL_F_S(BoxPlot, SetWidthFactor, double, widthFactor, recalc)
void BoxPlot::setWidthFactor(double widthFactor) {
	Q_D(BoxPlot);
	if (widthFactor != d->widthFactor)
		exec(new BoxPlotSetWidthFactorCmd(d, widthFactor, ki18n("%1: width factor changed")));
}

STD_SETTER_CMD_IMPL_F_S(BoxPlot, SetNotchesEnabled, bool, notchesEnabled, recalc)
void BoxPlot::setNotchesEnabled(bool notchesEnabled) {
	Q_D(BoxPlot);
	if (notchesEnabled != d->notchesEnabled)
		exec(new BoxPlotSetNotchesEnabledCmd(d, notchesEnabled, ki18n("%1: changed notches")));
}

//Filling
STD_SETTER_CMD_IMPL_F_S(BoxPlot, SetFillingEnabled, bool, fillingEnabled, updatePixmap)
void BoxPlot::setFillingEnabled(bool enabled) {
	Q_D(BoxPlot);
	if (enabled != d->fillingEnabled)
		exec(new BoxPlotSetFillingEnabledCmd(d, enabled, ki18n("%1: filling changed")));
}

STD_SETTER_CMD_IMPL_F_S(BoxPlot, SetFillingType, WorksheetElement::BackgroundType, fillingType, updatePixmap)
void BoxPlot::setFillingType(WorksheetElement::BackgroundType type) {
	Q_D(BoxPlot);
	if (type != d->fillingType)
		exec(new BoxPlotSetFillingTypeCmd(d, type, ki18n("%1: filling type changed")));
}

STD_SETTER_CMD_IMPL_F_S(BoxPlot, SetFillingColorStyle, WorksheetElement::BackgroundColorStyle, fillingColorStyle, updatePixmap)
void BoxPlot::setFillingColorStyle(WorksheetElement::BackgroundColorStyle style) {
	Q_D(BoxPlot);
	if (style != d->fillingColorStyle)
		exec(new BoxPlotSetFillingColorStyleCmd(d, style, ki18n("%1: filling color style changed")));
}

STD_SETTER_CMD_IMPL_F_S(BoxPlot, SetFillingImageStyle, WorksheetElement::BackgroundImageStyle, fillingImageStyle, updatePixmap)
void BoxPlot::setFillingImageStyle(WorksheetElement::BackgroundImageStyle style) {
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

//whiskers
STD_SETTER_CMD_IMPL_F_S(BoxPlot, SetWhiskersType, BoxPlot::WhiskersType, whiskersType, recalc)
void BoxPlot::setWhiskersType(BoxPlot::WhiskersType type) {
	Q_D(BoxPlot);
	if (type != d->whiskersType)
		exec(new BoxPlotSetWhiskersTypeCmd(d, type, ki18n("%1: set whiskers type")));
}

STD_SETTER_CMD_IMPL_F_S(BoxPlot, SetWhiskersRangeParameter, double, whiskersRangeParameter, recalc)
void BoxPlot::setWhiskersRangeParameter(double k) {
	Q_D(BoxPlot);
	if (k != d->whiskersRangeParameter)
		exec(new BoxPlotSetWhiskersRangeParameterCmd(d, k, ki18n("%1: set whiskers range parameter")));
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

STD_SETTER_CMD_IMPL_F_S(BoxPlot, SetWhiskersCapPen, QPen, whiskersCapPen, recalcShapeAndBoundingRect)
void BoxPlot::setWhiskersCapPen(const QPen& pen) {
	Q_D(BoxPlot);
	if (pen != d->whiskersCapPen)
		exec(new BoxPlotSetWhiskersCapPenCmd(d, pen, ki18n("%1: set whiskers cap pen")));
}

STD_SETTER_CMD_IMPL_F_S(BoxPlot, SetWhiskersCapOpacity, qreal, whiskersCapOpacity, updatePixmap)
void BoxPlot::setWhiskersCapOpacity(qreal opacity) {
	Q_D(BoxPlot);
	if (opacity != d->whiskersCapOpacity)
		exec(new BoxPlotSetWhiskersCapOpacityCmd(d, opacity, ki18n("%1: set whiskers cap opacity")));
}

//Symbols
STD_SETTER_CMD_IMPL_F_S(BoxPlot, SetJitteringEnabled, bool, jitteringEnabled, recalc)
void BoxPlot::setJitteringEnabled(bool enabled) {
	Q_D(BoxPlot);
	if (enabled != d->jitteringEnabled)
		exec(new BoxPlotSetJitteringEnabledCmd(d, enabled, ki18n("%1: jitterring changed")));
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
void BoxPlot::orientationChangedSlot(QAction*) {
	//TODO
}

void BoxPlot::visibilityChangedSlot() {
	Q_D(const BoxPlot);
	this->setVisible(!d->isVisible());
}

//##############################################################################
//####################### Private implementation ###############################
//##############################################################################
BoxPlotPrivate::BoxPlotPrivate(BoxPlot* owner) : WorksheetElementPrivate(owner), q(owner) {
	setFlag(QGraphicsItem::ItemIsSelectable);
	setAcceptHoverEvents(false);
}

bool BoxPlotPrivate::activateCurve(QPointF mouseScenePos, double /*maxDist*/) {
	if (!isVisible())
		return false;

	return shape().contains(mouseScenePos);
}

void BoxPlotPrivate::setHover(bool on) {
	if(on == m_hovered)
		return; // don't update if state not changed

	m_hovered = on;
	on ? Q_EMIT q->hovered() : emit q->unhovered();
	update();
}

/*!
  called when the size of the plot or its data ranges (manual changes, zooming, etc.) were changed.
  recalculates the position of the scene points to be drawn.
  triggers the update of lines, drop lines, symbols etc.
*/
void BoxPlotPrivate::retransform() {
	if (m_suppressRetransform || !isVisible() || q->isLoading())
		return;

	PERFTRACE(name() + Q_FUNC_INFO);

	const int count = dataColumns.size();
	if (!count || m_boxRect.size() != count) {
		//no columns or relacl() was not called yet, nothing to do
		recalcShapeAndBoundingRect();
		return;
	}

	for (int i = 0; i < count; ++i) {
		m_boxRect[i].clear();
		m_medianLine[i] = QLineF();
		m_whiskersPath[i] = QPainterPath();
		m_whiskersCapPath[i] = QPainterPath();
		m_outlierPoints[i].clear();
		m_dataPoints[i].clear();
		m_farOutPoints[i].clear();
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
	PERFTRACE(name() + Q_FUNC_INFO);

	//resize the internal containers
	const int count = dataColumns.size();
	m_boxRect.resize(count);
	m_fillRect.resize(count);
	m_xMinBox.resize(count);
	m_xMaxBox.resize(count);
	m_yMinBox.resize(count);
	m_yMaxBox.resize(count);
	m_median.resize(count);
	m_medianLine.resize(count);
	m_whiskersPath.resize(count);
	m_whiskersCapPath.resize(count);
	m_whiskerMin.resize(count);
	m_whiskerMax.resize(count);
	m_outlierPointsLogical.resize(count);
	m_outlierPoints.resize(count);
	m_dataPointsLogical.resize(count);
	m_dataPoints.resize(count);
	m_farOutPointsLogical.resize(count);
	m_farOutPoints.resize(count);
	m_mean.resize(count);
	m_meanSymbolPoint.resize(count);
	m_meanSymbolPointVisible.resize(count);
	m_medianSymbolPoint.resize(count);
	m_medianSymbolPointVisible.resize(count);

	//calculate the new min and max values of the box plot
	//for the current sizes of the box and of the whiskers
	if (orientation == BoxPlot::Orientation::Vertical) {
		xMin = 0.5;
		xMax = count + 0.5;
		yMin = INFINITY;
		yMax = -INFINITY;
	} else { //horizontal
		xMin = INFINITY;
		xMax = -INFINITY;
		yMin = 0.5;
		yMax = count + 0.5;
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

	if (ordering == BoxPlot::Ordering::None)
		dataColumnsOrdered = dataColumns;
	else {
		std::vector<std::pair<double, int>> newOrdering;

		if (ordering == BoxPlot::Ordering::MedianAscending
			|| ordering == BoxPlot::Ordering::MedianDescending) {
			for (int i = 0; i < count; ++i) {
				auto* column = static_cast<const Column*>(dataColumns.at(i));
				newOrdering.push_back(std::make_pair(column->statistics().median, i));
			}
		} else {
			for (int i = 0; i < count; ++i) {
				auto* column = static_cast<const Column*>(dataColumns.at(i));
				newOrdering.push_back(std::make_pair(column->statistics().arithmeticMean, i));
			}
		}

		std::sort(newOrdering.begin(), newOrdering.end());
		dataColumnsOrdered.clear();

		if (ordering == BoxPlot::Ordering::MedianAscending
			|| ordering == BoxPlot::Ordering::MeanAscending) {
			for (int i = 0; i < count; ++i)
				dataColumnsOrdered << dataColumns.at(newOrdering.at(i).second);
		} else {
			for (int i = count - 1; i >= 0; --i)
				dataColumnsOrdered << dataColumns.at(newOrdering.at(i).second);
		}
	}

	for (int i = 0; i < count; ++i)
		recalc(i);

	//the size of the boxplot changed because of the actual
	//data changes or because of new boxplot settings.
	//Q_EMIT dataChanged() in order to recalculate everything
	//in the parent plot with the new size/shape of the boxplot
	Q_EMIT q->dataChanged();
}

QPointF BoxPlotPrivate::setOutlierPoint(double pos, double value) {
	QPointF point;
	if (orientation == BoxPlot::Orientation::Vertical) {
		point.setX(pos);
		point.setY(value);

		if (value > yMax)
			yMax = value;
		else if (value < yMin)
			yMin = value;
	} else {
		point.setX(value);
		point.setY(pos);

		if (value > xMax)
			xMax = value;
		else if (value < xMin)
			xMin = value;
	}

	return point;
}

void BoxPlotPrivate::recalc(int index) {
	PERFTRACE(name() + Q_FUNC_INFO);
	auto* column = static_cast<const Column*>(dataColumnsOrdered.at(index));
	if (!column)
		return;

	//clear the containers for outliers, etc. since the their number
	//can be changed because of the new settings for whiskers, etc.
	m_outlierPointsLogical[index].clear();
	m_outlierPoints[index].clear();
	m_dataPointsLogical[index].clear();
	m_dataPoints[index].clear();
	m_farOutPointsLogical[index].clear();
	m_farOutPoints[index].clear();

	const auto& statistics = column->statistics();
	double width = 0.5*widthFactor;
	if (variableWidth && m_widthScaleFactor != 0)
		width *= std::sqrt(statistics.size)/m_widthScaleFactor;

	const double x = index + 1.0;

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
		m_whiskerMax[index] = statistics.thirdQuartile + whiskersRangeParameter*statistics.iqr;
		m_whiskerMin[index] = statistics.firstQuartile - whiskersRangeParameter*statistics.iqr;
		break;
	}
	case BoxPlot::WhiskersType::SD: {
		m_whiskerMax[index] = statistics.arithmeticMean + whiskersRangeParameter*statistics.standardDeviation;
		m_whiskerMin[index] = statistics.arithmeticMean - whiskersRangeParameter*statistics.standardDeviation;
		break;
	}
	case BoxPlot::WhiskersType::MAD: {
		m_whiskerMax[index] = statistics.median + whiskersRangeParameter*statistics.meanDeviationAroundMedian;
		m_whiskerMin[index] = statistics.median - whiskersRangeParameter*statistics.meanDeviationAroundMedian;
		break;
	}
	case BoxPlot::WhiskersType::PERCENTILES_1_99: {
		m_whiskerMax[index] = statistics.percentile_99;
		m_whiskerMin[index] = statistics.percentile_1;
		break;
	}
	case BoxPlot::WhiskersType::PERCENTILES_5_95: {
		m_whiskerMax[index] = statistics.percentile_95;
		m_whiskerMin[index] = statistics.percentile_5;
		break;
	}
	case BoxPlot::WhiskersType::PERCENTILES_10_90: {
		m_whiskerMax[index] = statistics.percentile_90;
		m_whiskerMin[index] = statistics.percentile_10;
		break;
	}
	}

	//outliers symbols
	if (orientation == BoxPlot::Orientation::Vertical) {
		if (m_whiskerMax[index] > yMax)
			yMax = m_whiskerMax[index];
		if (m_whiskerMin[index] < yMin)
			yMin = m_whiskerMin[index];
	} else {
		if (m_whiskerMax[index] > xMax)
			xMax = m_whiskerMax[index];
		if (m_whiskerMin[index] < xMin)
			xMin = m_whiskerMin[index];
	}

	double whiskerMax = - qInf(); //upper adjacent value
	double whiskerMin = qInf(); //lower adjacent value
	const double outerFenceMax = statistics.thirdQuartile + 3.0*statistics.iqr;
	const double outerFenceMin = statistics.firstQuartile - 3.0*statistics.iqr;

	for (int row = 0; row < column->rowCount(); ++row) {
		if (!column->isValid(row) || column->isMasked(row) )
			continue;

		double value = 0.0;
		switch (column->columnMode()) {
		case AbstractColumn::ColumnMode::Double:
		case AbstractColumn::ColumnMode::Integer:
		case AbstractColumn::ColumnMode::BigInt:
			value = column->valueAt(row);
			break;
		case AbstractColumn::ColumnMode::DateTime:
			value = column->dateTimeAt(row).toMSecsSinceEpoch();
			break;
		case AbstractColumn::ColumnMode::Text:
		case AbstractColumn::ColumnMode::Month:
		case AbstractColumn::ColumnMode::Day:
			break;
		}

		double rand = 0.5;
		if (jitteringEnabled)
			rand = (double)std::rand() / ((double)RAND_MAX + 1);

		if (value > m_whiskerMax.at(index) || value < m_whiskerMin.at(index)) {
			if (whiskersType == BoxPlot::WhiskersType::IQR && (value > outerFenceMax || value < outerFenceMin))
				m_farOutPointsLogical[index] << setOutlierPoint(x - width/2 + rand*width, value);
			else
				m_outlierPointsLogical[index] << setOutlierPoint(x - width/2 + rand*width, value);
		} else {
			if (orientation == BoxPlot::Orientation::Vertical)
				m_dataPointsLogical[index] << QPointF(x - width/2 + rand*width, value);
			else
				m_dataPointsLogical[index] << QPointF(value, x - width/2 + rand*width);

			//determine the upper/lower adjucent values
			if (whiskersType == BoxPlot::WhiskersType::IQR) {
				if (value > whiskerMax)
					whiskerMax = value;

				if (value < whiskerMin)
					whiskerMin = value;
			}
		}
	}

	//set the whisker ends at the upper and lower adjacent values
	if (whiskersType == BoxPlot::WhiskersType::IQR) {
		if (whiskerMax != -qInf())
			m_whiskerMax[index] = whiskerMax;
		if (whiskerMin != qInf())
			m_whiskerMin[index] = whiskerMin;
	}
}

void BoxPlotPrivate::verticalBoxPlot(int index) {
	PERFTRACE(name() + Q_FUNC_INFO);

	QVector<QLineF> lines;
	const double x = index + 1.0;
	const double xMinBox = m_xMinBox.at(index);
	const double xMaxBox = m_xMaxBox.at(index);
	const double yMinBox = m_yMinBox.at(index);
	const double yMaxBox = m_yMaxBox.at(index);
	const double median = m_median.at(index);

	//box
	if (!notchesEnabled) {
		//first line starting at the top left corner of the box
		lines << QLineF(xMinBox, yMaxBox, xMaxBox, yMaxBox);
		lines << QLineF(xMaxBox, yMaxBox, xMaxBox, yMinBox);
		lines << QLineF(xMaxBox, yMinBox, xMinBox, yMinBox);
		lines << QLineF(xMinBox, yMinBox, xMinBox, yMaxBox);
	} else {
		auto* column = static_cast<const Column*>(dataColumnsOrdered.at(index));
		const auto& statistics = column->statistics();
		const double notch = 1.7*1.25*statistics.iqr/1.35/std::sqrt(statistics.size);
		const double notchMax = median + notch ;
		const double notchMin = median - notch ;
		double width = xMaxBox - xMinBox;

		//first line starting at the top left corner of the box
		lines << QLineF(xMinBox, yMaxBox, xMaxBox, yMaxBox);
		lines << QLineF(xMaxBox, yMaxBox, xMaxBox, notchMax);
		lines << QLineF(xMaxBox, notchMax, xMinBox + 0.9*width, median);
		lines << QLineF(xMinBox + 0.9*width, median, xMaxBox, notchMin);
		lines << QLineF(xMaxBox, notchMin, xMaxBox, yMinBox);
		lines << QLineF(xMaxBox, yMinBox, xMinBox, yMinBox);
		lines << QLineF(xMinBox, yMinBox, xMinBox, notchMin);
		lines << QLineF(xMinBox, notchMin, xMinBox + 0.1*width, median);
		lines << QLineF(xMinBox + 0.1*width, median, xMinBox, notchMax);
		lines << QLineF(xMinBox, notchMax, xMinBox, yMaxBox);
	}

	m_boxRect[index] = q->cSystem->mapLogicalToScene(lines);
	updateFillingRect(index, lines);

	//median line
	lines.clear();
	if (!notchesEnabled)
		lines << QLineF(xMinBox, median, xMaxBox, median);
	else {
		double width = xMaxBox - xMinBox;
		lines << QLineF(xMinBox + 0.1*width, median, m_xMaxBox.at(index) - 0.1*width, median);
	}

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
		bool visible = false;
		QPointF maxPoint = q->cSystem->mapLogicalToScene(QPointF(x, m_whiskerMax.at(index)), visible);
		if (visible) {
			m_whiskersCapPath[index].moveTo(QPointF(maxPoint.x() - whiskersCapSize/2., maxPoint.y()));
			m_whiskersCapPath[index].lineTo(QPointF(maxPoint.x() + whiskersCapSize/2., maxPoint.y()));
		}

		QPointF minPoint = q->cSystem->mapLogicalToScene(QPointF(x, m_whiskerMin.at(index)), visible);
		if (visible) {
			m_whiskersCapPath[index].moveTo(QPointF(minPoint.x() - whiskersCapSize/2., minPoint.y()));
			m_whiskersCapPath[index].lineTo(QPointF(minPoint.x() + whiskersCapSize/2., minPoint.y()));
		}
	}

	//outliers symbols
	mapOutliersToScene(index);

	//jitter values
	int size = m_dataPointsLogical[index].size();
	if (size > 0) {
		const int startIndex = 0;
		const int endIndex = m_dataPointsLogical[index].size() - 1;
		QVector<bool> m_pointVisible;
		m_pointVisible.resize(size);

		q->cSystem->mapLogicalToScene(startIndex, endIndex,
									m_dataPointsLogical[index],
									m_dataPoints[index],
									m_pointVisible);
	}

	//far out values
	size = m_farOutPointsLogical[index].size();
	if (size > 0) {
		const int startIndex = 0;
		const int endIndex = m_farOutPointsLogical[index].size() - 1;
		QVector<bool> m_pointVisible;
		m_pointVisible.resize(size);

		q->cSystem->mapLogicalToScene(startIndex, endIndex,
									m_farOutPointsLogical[index],
									m_farOutPoints[index],
									m_pointVisible);
	}

	//mean symbol
	QVector<QPointF> points = {QPointF(x, m_mean.at(index))};
	points = q->cSystem->mapLogicalToScene(points);
	if (points.count() == 1) {
		m_meanSymbolPoint[index] = points.at(0);
		m_meanSymbolPointVisible[index] = true;
	} else
		m_meanSymbolPointVisible[index] = false;

	//median symbol
	points = {QPointF(x, median)};
	points = q->cSystem->mapLogicalToScene(points);
	if (points.count() == 1) {
		m_medianSymbolPoint[index] = points.at(0);
		m_medianSymbolPointVisible[index] = true;
	} else
		m_medianSymbolPointVisible[index] = false;
}

void BoxPlotPrivate::horizontalBoxPlot(int index) {
	PERFTRACE(name() + Q_FUNC_INFO);

	QVector<QLineF> lines;
	const double y = index + 1.0;
	const double xMinBox = m_xMinBox.at(index);
	const double xMaxBox = m_xMaxBox.at(index);
	const double yMinBox = m_yMinBox.at(index);
	const double yMaxBox = m_yMaxBox.at(index);
	const double median = m_median.at(index);

	//box
	if (!notchesEnabled) {
		//first line starting at the top left corner of the box
		lines << QLineF(xMinBox, yMaxBox, xMaxBox, yMaxBox);
		lines << QLineF(xMaxBox, yMaxBox, xMaxBox, yMinBox);
		lines << QLineF(xMaxBox, yMinBox, xMinBox, yMinBox);
		lines << QLineF(xMinBox, yMinBox, xMinBox, yMaxBox);
	} else {
		auto* column = static_cast<const Column*>(dataColumnsOrdered.at(index));
		const auto& statistics = column->statistics();
		const double notch = 1.7*1.25*statistics.iqr/1.35/std::sqrt(statistics.size);
		const double notchMax = median + notch ;
		const double notchMin = median - notch ;
		double width = yMaxBox - yMinBox;

		lines << QLineF(xMinBox, yMaxBox, notchMin, yMaxBox);
		lines << QLineF(notchMin, yMaxBox, median, yMaxBox - 0.1*width);
		lines << QLineF(median, yMaxBox - 0.1*width, notchMax, yMaxBox);
		lines << QLineF(notchMax, yMaxBox, xMaxBox, yMaxBox);
		lines << QLineF(xMaxBox, yMaxBox, xMaxBox, yMinBox);
		lines << QLineF(xMaxBox, yMinBox, notchMax, yMinBox);
		lines << QLineF(notchMax, yMinBox, median, yMinBox + 0.1*width);
		lines << QLineF(median, yMinBox + 0.1*width, notchMin, yMinBox);
		lines << QLineF(notchMin, yMinBox, xMinBox, yMinBox);
		lines << QLineF(xMinBox, yMinBox, xMinBox, yMaxBox);
	}

	m_boxRect[index] = q->cSystem->mapLogicalToScene(lines);
	updateFillingRect(index, lines);

	//median line
	lines.clear();
	if (!notchesEnabled)
		lines << QLineF(median, yMinBox, median, yMaxBox);
	else {
		double width = yMaxBox - yMinBox;
		lines << QLineF(median, yMinBox + 0.1*width, median, yMaxBox - 0.1*width);
	}

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
		bool visible = false;
		QPointF maxPoint = q->cSystem->mapLogicalToScene(QPointF(m_whiskerMax.at(index), y), visible);
		if (visible) {
			m_whiskersCapPath[index].moveTo(QPointF(maxPoint.x(), maxPoint.y() - whiskersCapSize/2));
			m_whiskersCapPath[index].lineTo(QPointF(maxPoint.x(), maxPoint.y() + whiskersCapSize/2));
		}

		QPointF minPoint = q->cSystem->mapLogicalToScene(QPointF(m_whiskerMin.at(index), y), visible);
		if (visible) {
			m_whiskersCapPath[index].moveTo(QPointF(minPoint.x(), minPoint.y() - whiskersCapSize/2));
			m_whiskersCapPath[index].lineTo(QPointF(minPoint.x(), minPoint.y() + whiskersCapSize/2));
		}
	}

	//outliers symbols
	mapOutliersToScene(index);

	int size = m_dataPointsLogical[index].size();
	if (size > 0) {
		const int startIndex = 0;
		const int endIndex = m_dataPointsLogical[index].size() - 1;
		QVector<bool> m_pointVisible;
		m_pointVisible.resize(size);

		q->cSystem->mapLogicalToScene(startIndex, endIndex,
									m_dataPointsLogical[index],
									m_dataPoints[index],
									m_pointVisible);
	}

	size = m_farOutPointsLogical[index].size();
	if (size > 0) {
		const int startIndex = 0;
		const int endIndex = m_farOutPointsLogical[index].size() - 1;
		QVector<bool> m_pointVisible;
		m_pointVisible.resize(size);

		q->cSystem->mapLogicalToScene(startIndex, endIndex,
									m_farOutPointsLogical[index],
									m_farOutPoints[index],
									m_pointVisible);
	}

	//mean symbol
	QVector<QPointF> points = {QPointF(m_mean.at(index), y)};
	points = q->cSystem->mapLogicalToScene(points);
	if (points.count() == 1) {
		m_meanSymbolPoint[index] = points.at(0);
		m_meanSymbolPointVisible[index] = true;
	} else
		m_meanSymbolPointVisible[index] = false;

	//median symbol
	points = {QPointF(m_mean.at(index), y)};
	points = q->cSystem->mapLogicalToScene(points);
	if (points.count() == 1) {
		m_medianSymbolPoint[index] = points.at(0);
		m_medianSymbolPointVisible[index] = true;
	} else
		m_medianSymbolPointVisible[index] = false;
}

void BoxPlotPrivate::updateFillingRect(int index, const QVector<QLineF>& lines) {
	const auto& unclippedLines = q->cSystem->mapLogicalToScene(lines, AbstractCoordinateSystem::MappingFlag::SuppressPageClipping);

	if (unclippedLines.isEmpty()) {
		m_fillRect[index] = QRectF();
		return;
	}

	//we have four unclipped lines for the box.
	//clip the points to the plot data rect and create a new polygon
	//out of them that will be filled out.
	QPolygonF polygon;
	const QRectF& dataRect = static_cast<CartesianPlot*>(q->parentAspect())->dataRect();
	int i = 0;
	for (const auto& line : unclippedLines) {
		//clip the first point of the line
		QPointF p1 = line.p1();
		if (p1.x() < dataRect.left())
			p1.setX(dataRect.left());
		else if (p1.x() > dataRect.right())
			p1.setX(dataRect.right());

		if (p1.y() < dataRect.top())
			p1.setY(dataRect.top());
		else if (p1.y() > dataRect.bottom())
			p1.setY(dataRect.bottom());

		//clip the second point of the line
		QPointF p2 = line.p2();
		if (p2.x() < dataRect.left())
			p2.setX(dataRect.left());
		else if (p2.x() > dataRect.right())
			p2.setX(dataRect.right());

		if (p2.y() < dataRect.top())
			p2.setY(dataRect.top());
		else if (p2.y() > dataRect.bottom())
			p2.setY(dataRect.bottom());

		if (i != unclippedLines.size() - 1)
			polygon << p1;
		else {
			//close the polygon for the last line
			polygon << p1;
			polygon << p2;
		}

		++i;
	}

	m_fillRect[index] = polygon.boundingRect();
}

/*!
 * map the outlier points from logical to scene coordinates,
 * avoid drawing overlapping points, logic similar to
 * //XYCurvePrivate::retransform()
 */
void BoxPlotPrivate::mapOutliersToScene(int index) {
	const int numberOfPoints = m_outlierPointsLogical[index].size();
	if (numberOfPoints > 0) {
		const int startIndex = 0;
		const int endIndex = m_outlierPointsLogical[index].size() - 1;
		QVector<bool> m_pointVisible;
		m_pointVisible.resize(numberOfPoints);

		q->cSystem->mapLogicalToScene(startIndex, endIndex,
									m_outlierPointsLogical[index],
									m_outlierPoints[index],
									m_pointVisible);
	}
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

	for (int i = 0; i < dataColumnsOrdered.size(); ++i) {
		if (!dataColumnsOrdered.at(i)
			|| static_cast<const Column*>(dataColumnsOrdered.at(i))->statistics().size == 0)
			continue;

		QPainterPath boxPath;
		for (const auto& line : qAsConst(m_boxRect.at(i))) {
			boxPath.moveTo(line.p1());
			boxPath.lineTo(line.p2());
		}
		m_boxPlotShape.addPath(WorksheetElement::shapeFromPath(boxPath, borderPen));

		m_boxPlotShape.addPath(WorksheetElement::shapeFromPath(m_whiskersPath.at(i), whiskersPen));
		m_boxPlotShape.addPath(WorksheetElement::shapeFromPath(m_whiskersCapPath.at(i), whiskersCapPen));

		//add symbols outlier, jitter and far out values
		QPainterPath symbolsPath = QPainterPath();

		//outlier values
		if (symbolOutlier->style() != Symbol::Style::NoSymbols && !m_outlierPoints.at(i).isEmpty()) {
			QPainterPath path = Symbol::stylePath(symbolOutlier->style());
			QTransform trafo;
			trafo.scale(symbolOutlier->size(), symbolOutlier->size());
			path = trafo.map(path);
			trafo.reset();

			if (symbolOutlier->rotationAngle() != 0) {
				trafo.rotate(symbolOutlier->rotationAngle());
				path = trafo.map(path);
			}

			for (const auto& point : qAsConst(m_outlierPoints.at(i))) {
				trafo.reset();
				trafo.translate(point.x(), point.y());
				symbolsPath.addPath(trafo.map(path));
			}
		}

		//jitter values
		if (symbolData->style() != Symbol::Style::NoSymbols && !m_dataPoints.at(i).isEmpty()) {
			QPainterPath path = Symbol::stylePath(symbolData->style());
			QTransform trafo;
			trafo.scale(symbolData->size(), symbolData->size());
			path = trafo.map(path);
			trafo.reset();

			if (symbolData->rotationAngle() != 0) {
				trafo.rotate(symbolData->rotationAngle());
				path = trafo.map(path);
			}

			for (const auto& point : qAsConst(m_dataPoints.at(i))) {
				trafo.reset();
				trafo.translate(point.x(), point.y());
				symbolsPath.addPath(trafo.map(path));
			}
		}

		//far out values
		if (symbolFarOut->style() != Symbol::Style::NoSymbols && !m_farOutPoints.at(i).isEmpty()) {
			QPainterPath path = Symbol::stylePath(symbolFarOut->style());
			QTransform trafo;
			trafo.scale(symbolFarOut->size(), symbolFarOut->size());
			path = trafo.map(path);
			trafo.reset();

			if (symbolFarOut->rotationAngle() != 0) {
				trafo.rotate(symbolFarOut->rotationAngle());
				path = trafo.map(path);
			}

			for (const auto& point : qAsConst(m_farOutPoints.at(i))) {
				trafo.reset();
				trafo.translate(point.x(), point.y());
				symbolsPath.addPath(trafo.map(path));
			}
		}

		m_boxPlotShape.addPath(symbolsPath);
	}

	m_boundingRectangle = m_boxPlotShape.boundingRect();
	updatePixmap();
}

void BoxPlotPrivate::updatePixmap() {
	PERFTRACE(name() + Q_FUNC_INFO);
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
	PERFTRACE(name() + Q_FUNC_INFO);

	for (int i = 0; i < dataColumnsOrdered.size(); ++i) {
		if (!dataColumnsOrdered.at(i))
			continue;

		//no need to draw anything if the column doesn't have any valid values
		if (static_cast<const Column*>(dataColumnsOrdered.at(i))->statistics().size == 0)
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

		//draw the whiskers cap
		if (whiskersCapPen.style() != Qt::NoPen && !m_whiskersCapPath.at(i).isEmpty()) {
			painter->setPen(whiskersCapPen);
			painter->setBrush(Qt::NoBrush);
			painter->setOpacity(whiskersCapOpacity);
			painter->drawPath(m_whiskersCapPath.at(i));
		}

		//draw the symbols
		drawSymbols(painter, i);
	}
}

void BoxPlotPrivate::drawSymbols(QPainter* painter, int index) {
	//outlier values
	if (symbolOutlier->style() != Symbol::Style::NoSymbols && !m_outlierPoints.at(index).isEmpty()) {
		painter->setOpacity(symbolOutlier->opacity());
		painter->setPen(symbolOutlier->pen());
		painter->setBrush(symbolOutlier->brush());
		QPainterPath path = Symbol::stylePath(symbolOutlier->style());
		QTransform trafo;
		trafo.scale(symbolOutlier->size(), symbolOutlier->size());
		if (symbolOutlier->rotationAngle() != 0)
			trafo.rotate(-symbolOutlier->rotationAngle());

		path = trafo.map(path);

		for (const auto& point : qAsConst(m_outlierPoints.at(index))) {
			trafo.reset();
			trafo.translate(point.x(), point.y());
			painter->drawPath(trafo.map(path));
		}
	}

	//mean value
	if (symbolMean->style() != Symbol::Style::NoSymbols && m_meanSymbolPointVisible.at(index)) {
		painter->setOpacity(symbolMean->opacity());
		painter->setPen(symbolMean->pen());
		painter->setBrush(symbolMean->brush());
		QTransform trafo;
		trafo.scale(symbolMean->size(), symbolMean->size());
		QPainterPath path = Symbol::stylePath(symbolMean->style());
		if (symbolMean->rotationAngle() != 0)
			trafo.rotate(-symbolMean->rotationAngle());

		path = trafo.map(path);

		trafo.reset();
		trafo.translate(m_meanSymbolPoint.at(index).x(), m_meanSymbolPoint.at(index).y());
		painter->drawPath(trafo.map(path));
	}

	//median value
	if (symbolMedian->style() != Symbol::Style::NoSymbols && m_medianSymbolPointVisible.at(index)) {
		painter->setOpacity(symbolMedian->opacity());
		painter->setPen(symbolMedian->pen());
		painter->setBrush(symbolMedian->brush());
		QTransform trafo;
		trafo.scale(symbolMedian->size(), symbolMedian->size());
		QPainterPath path = Symbol::stylePath(symbolMedian->style());
		if (symbolMedian->rotationAngle() != 0)
			trafo.rotate(-symbolMedian->rotationAngle());

		path = trafo.map(path);

		trafo.reset();
		trafo.translate(m_medianSymbolPoint.at(index).x(), m_medianSymbolPoint.at(index).y());
		painter->drawPath(trafo.map(path));
	}

	//jitter values
	if (symbolData->style() != Symbol::Style::NoSymbols && !m_dataPoints.at(index).isEmpty()) {
		painter->setOpacity(symbolData->opacity());
		painter->setPen(symbolData->pen());
		painter->setBrush(symbolData->brush());
		QPainterPath path = Symbol::stylePath(symbolData->style());
		QTransform trafo;
		trafo.scale(symbolData->size(), symbolData->size());
		if (symbolData->rotationAngle() != 0)
			trafo.rotate(-symbolData->rotationAngle());

		path = trafo.map(path);

		for (const auto& point : qAsConst(m_dataPoints.at(index))) {
			trafo.reset();
			trafo.translate(point.x(), point.y());
			painter->drawPath(trafo.map(path));
		}
	}

	//far out values
	if (symbolFarOut->style() != Symbol::Style::NoSymbols && !m_farOutPoints.at(index).isEmpty()) {
		painter->setOpacity(symbolFarOut->opacity());
		painter->setPen(symbolFarOut->pen());
		painter->setBrush(symbolFarOut->brush());
		QPainterPath path = Symbol::stylePath(symbolFarOut->style());
		QTransform trafo;
		trafo.scale(symbolFarOut->size(), symbolFarOut->size());
		if (symbolFarOut->rotationAngle() != 0)
			trafo.rotate(-symbolFarOut->rotationAngle());

		path = trafo.map(path);

		for (const auto& point : qAsConst(m_farOutPoints.at(index))) {
			trafo.reset();
			trafo.translate(point.x(), point.y());
			painter->drawPath(trafo.map(path));
		}
	}
}

void BoxPlotPrivate::drawFilling(QPainter* painter, int index) {
	PERFTRACE(name() + Q_FUNC_INFO);

	const QRectF& rect = m_fillRect.at(index);

	if (fillingType == WorksheetElement::BackgroundType::Color) {
		switch (fillingColorStyle) {
		case WorksheetElement::BackgroundColorStyle::SingleColor: {
			painter->setBrush(QBrush(fillingFirstColor));
			break;
		}
		case WorksheetElement::BackgroundColorStyle::HorizontalLinearGradient: {
			QLinearGradient linearGrad(rect.topLeft(), rect.topRight());
			linearGrad.setColorAt(0, fillingFirstColor);
			linearGrad.setColorAt(1, fillingSecondColor);
			painter->setBrush(QBrush(linearGrad));
			break;
		}
		case WorksheetElement::BackgroundColorStyle::VerticalLinearGradient: {
			QLinearGradient linearGrad(rect.topLeft(), rect.bottomLeft());
			linearGrad.setColorAt(0, fillingFirstColor);
			linearGrad.setColorAt(1, fillingSecondColor);
			painter->setBrush(QBrush(linearGrad));
			break;
		}
		case WorksheetElement::BackgroundColorStyle::TopLeftDiagonalLinearGradient: {
			QLinearGradient linearGrad(rect.topLeft(), rect.bottomRight());
			linearGrad.setColorAt(0, fillingFirstColor);
			linearGrad.setColorAt(1, fillingSecondColor);
			painter->setBrush(QBrush(linearGrad));
			break;
		}
		case WorksheetElement::BackgroundColorStyle::BottomLeftDiagonalLinearGradient: {
			QLinearGradient linearGrad(rect.bottomLeft(), rect.topRight());
			linearGrad.setColorAt(0, fillingFirstColor);
			linearGrad.setColorAt(1, fillingSecondColor);
			painter->setBrush(QBrush(linearGrad));
			break;
		}
		case WorksheetElement::BackgroundColorStyle::RadialGradient: {
			QRadialGradient radialGrad(rect.center(), rect.width()/2);
			radialGrad.setColorAt(0, fillingFirstColor);
			radialGrad.setColorAt(1, fillingSecondColor);
			painter->setBrush(QBrush(radialGrad));
			break;
		}
		}
	} else if (fillingType == WorksheetElement::BackgroundType::Image) {
		if ( !fillingFileName.trimmed().isEmpty() ) {
			QPixmap pix(fillingFileName);
			switch (fillingImageStyle) {
			case WorksheetElement::BackgroundImageStyle::ScaledCropped:
				pix = pix.scaled(rect.size().toSize(),Qt::KeepAspectRatioByExpanding,Qt::SmoothTransformation);
				painter->setBrush(QBrush(pix));
				painter->setBrushOrigin(pix.size().width()/2,pix.size().height()/2);
				break;
			case WorksheetElement::BackgroundImageStyle::Scaled:
				pix = pix.scaled(rect.size().toSize(),Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
				painter->setBrush(QBrush(pix));
				painter->setBrushOrigin(pix.size().width()/2,pix.size().height()/2);
				break;
			case WorksheetElement::BackgroundImageStyle::ScaledAspectRatio:
				pix = pix.scaled(rect.size().toSize(),Qt::KeepAspectRatio,Qt::SmoothTransformation);
				painter->setBrush(QBrush(pix));
				painter->setBrushOrigin(pix.size().width()/2,pix.size().height()/2);
				break;
			case WorksheetElement::BackgroundImageStyle::Centered: {
				QPixmap backpix(rect.size().toSize());
				backpix.fill();
				QPainter p(&backpix);
				p.drawPixmap(QPointF(0,0),pix);
				p.end();
				painter->setBrush(QBrush(backpix));
				painter->setBrushOrigin(-pix.size().width()/2,-pix.size().height()/2);
				break;
			}
			case WorksheetElement::BackgroundImageStyle::Tiled:
				painter->setBrush(QBrush(pix));
				break;
			case WorksheetElement::BackgroundImageStyle::CenterTiled:
				painter->setBrush(QBrush(pix));
				painter->setBrushOrigin(pix.size().width()/2,pix.size().height()/2);
			}
		}
	} else if (fillingType == WorksheetElement::BackgroundType::Pattern)
		painter->setBrush(QBrush(fillingFirstColor,fillingBrushStyle));

	painter->drawRect(rect);
}

void BoxPlotPrivate::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget*) {
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
		Q_EMIT q->hovered();
		update();
	}
}

void BoxPlotPrivate::hoverLeaveEvent(QGraphicsSceneHoverEvent*) {
	if (m_hovered) {
		m_hovered = false;
		Q_EMIT q->unhovered();
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
	writer->writeAttribute("ordering", QString::number(static_cast<int>(d->ordering)));
	writer->writeAttribute("orientation", QString::number(static_cast<int>(d->orientation)));
	writer->writeAttribute("variableWidth", QString::number(d->variableWidth));
	writer->writeAttribute("widthFactor", QString::number(d->widthFactor));
	writer->writeAttribute("notches", QString::number(d->notchesEnabled));
	writer->writeAttribute("jitteringEnabled", QString::number(d->jitteringEnabled));
	writer->writeAttribute("plotRangeIndex", QString::number(m_cSystemIndex));
	writer->writeAttribute("xMin", QString::number(d->xMin));
	writer->writeAttribute("xMax", QString::number(d->xMax));
	writer->writeAttribute("yMin", QString::number(d->yMin));
	writer->writeAttribute("yMax", QString::number(d->yMax));
	for (auto* column : d->dataColumns) {
		writer->writeStartElement("column");
		writer->writeAttribute("path", column->path());
		writer->writeEndElement();
	}
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
	writer->writeAttribute( "opacity", QString::number(d->borderOpacity) );
	writer->writeEndElement();

	//median line
	writer->writeStartElement("medianLine");
	WRITE_QPEN(d->medianLinePen);
	writer->writeAttribute("opacity", QString::number(d->medianLineOpacity));
	writer->writeEndElement();

	//symbols for the outliers, mean, far out and jitter values
	d->symbolMean->save(writer);
	d->symbolMedian->save(writer);
	d->symbolOutlier->save(writer);
	d->symbolFarOut->save(writer);
	d->symbolData->save(writer);

	//whiskers
	writer->writeStartElement("whiskers");
	writer->writeAttribute("type", QString::number(static_cast<int>(d->whiskersType)));
	writer->writeAttribute("rangeParameter", QString::number(d->whiskersRangeParameter));
	WRITE_QPEN(d->whiskersPen);
	writer->writeAttribute("opacity", QString::number(d->whiskersOpacity));
	writer->writeEndElement();

	writer->writeStartElement("whiskersCap");
	writer->writeAttribute("size", QString::number(d->whiskersCapSize));
	WRITE_QPEN(d->whiskersCapPen);
	writer->writeAttribute("opacity", QString::number(d->whiskersCapOpacity));
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

			READ_INT_VALUE("ordering", ordering, BoxPlot::Ordering);
			READ_INT_VALUE("orientation", orientation, BoxPlot::Orientation);
			READ_INT_VALUE("variableWidth", variableWidth, bool);
			READ_DOUBLE_VALUE("widthFactor", widthFactor);
			READ_INT_VALUE("notches", notchesEnabled, bool);
			READ_INT_VALUE("jitteringEnabled", jitteringEnabled, bool);
			READ_INT_VALUE_DIRECT("plotRangeIndex", m_cSystemIndex, int);

			READ_DOUBLE_VALUE("xMin", xMin);
			READ_DOUBLE_VALUE("xMax", xMax);
			READ_DOUBLE_VALUE("yMin", yMin);
			READ_DOUBLE_VALUE("yMax", yMax);
		} else if (reader->name() == "column") {
			attribs = reader->attributes();

			str = attribs.value("path").toString();
			if (!str.isEmpty())
				d->dataColumnPaths << str;
// 			READ_COLUMN(dataColumn);
		} else if (!preview && reader->name() == "filling") {
			attribs = reader->attributes();

			READ_INT_VALUE("enabled", fillingEnabled, bool);
			READ_INT_VALUE("type", fillingType, WorksheetElement::BackgroundType);
			READ_INT_VALUE("colorStyle", fillingColorStyle, WorksheetElement::BackgroundColorStyle);
			READ_INT_VALUE("imageStyle", fillingImageStyle, WorksheetElement::BackgroundImageStyle);
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
		} else if (!preview && reader->name() == "symbolMean")
			d->symbolMean->load(reader, preview);
		else if (!preview && reader->name() == "symbolMedian")
			d->symbolMedian->load(reader, preview);
		else if (!preview && reader->name() == "symbolOutlier")
			d->symbolOutlier->load(reader, preview);
		else if (!preview && reader->name() == "symbolFarOut")
			d->symbolFarOut->load(reader, preview);
		else if (!preview && reader->name() == "symbolData")
			d->symbolData->load(reader, preview);
		else if (!preview && reader->name() == "whiskers") {
			attribs = reader->attributes();

			READ_INT_VALUE("type", whiskersType, BoxPlot::WhiskersType);
			READ_DOUBLE_VALUE("rangeParameter", whiskersRangeParameter);
			READ_QPEN(d->whiskersPen);
			READ_DOUBLE_VALUE("opacity", whiskersOpacity);
			READ_DOUBLE_VALUE("capSize", whiskersCapSize);
		} else if (!preview && reader->name() == "whiskersCap") {
			attribs = reader->attributes();

			READ_DOUBLE_VALUE("size", whiskersCapSize);
			READ_QPEN(d->whiskersCapPen);
			READ_DOUBLE_VALUE("opacity", whiskersCapOpacity);
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
	const QColor themeColor = plot->themeColorPalette(index);

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
	setFillingColorStyle((WorksheetElement::BackgroundColorStyle)group.readEntry("FillingColorStyle", static_cast<int>(WorksheetElement::BackgroundColorStyle::SingleColor)));
	setFillingOpacity(group.readEntry("FillingOpacity", 0.5));
	setFillingFirstColor(themeColor);
	setFillingSecondColor(group.readEntry("FillingSecondColor", QColor(Qt::black)));
	setFillingType((WorksheetElement::BackgroundType)group.readEntry("FillingType", static_cast<int>(WorksheetElement::BackgroundType::Color)));

	//median line
	setMedianLinePen(p);
	setMedianLineOpacity(group.readEntry("LineOpacity", 1.0));

	//whiskers
	setWhiskersPen(p);
	setWhiskersOpacity(group.readEntry("LineOpacity", 1.0));
	setWhiskersCapPen(p);
	setWhiskersCapOpacity(group.readEntry("LineOpacity", 1.0));

	//symbols
	d->symbolMean->loadThemeConfig(group, themeColor);
	d->symbolMedian->loadThemeConfig(group, themeColor);
	d->symbolOutlier->loadThemeConfig(group, themeColor);
	d->symbolFarOut->loadThemeConfig(group, themeColor);
	d->symbolData->loadThemeConfig(group, themeColor);

	//Tufte's theme goes beyond what we can implement when using the theme properties of XYCurve.
	//So, instead of introducing a dedicated section for BoxPlot, which would be a big overkill
	//for all other themes, we add here a special handling for "Tufte".
	if (plot->theme() == QLatin1String("Tufte")) {
		p.setStyle(Qt::NoPen);
		setBorderPen(p);
		setMedianLinePen(p);
		setFillingEnabled(false);
		d->symbolMean->setStyle(Symbol::Style::NoSymbols);
		d->symbolMedian->setStyle(Symbol::Style::Circle);
		d->symbolOutlier->setStyle(Symbol::Style::NoSymbols);
		d->symbolFarOut->setStyle(Symbol::Style::NoSymbols);
		d->symbolData->setStyle(Symbol::Style::NoSymbols);
		setWhiskersCapSize(0.0);
	}

	d->m_suppressRecalc = false;
	d->recalcShapeAndBoundingRect();
}
