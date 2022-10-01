/*
	File                 : BoxPlot.cpp
	Project              : LabPlot
	Description          : Box Plot
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2021-2022 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "BoxPlot.h"
#include "BoxPlotPrivate.h"
#include "backend/core/AbstractColumn.h"
#include "backend/core/Folder.h"
#include "backend/core/column/Column.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/trace.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/worksheet/Background.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/CartesianCoordinateSystem.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/worksheet/plots/cartesian/Symbol.h"
#include "tools/ImageTools.h"

#include <QActionGroup>
#include <QApplication>
#include <QGraphicsSceneMouseEvent>
#include <QMenu>
#include <QPainter>

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>

/**
 * \class BoxPlot
 * \brief Box Plot
 */

BoxPlot::BoxPlot(const QString& name)
	: WorksheetElement(name, new BoxPlotPrivate(this), AspectType::BoxPlot) {
	init();
}

BoxPlot::BoxPlot(const QString& name, BoxPlotPrivate* dd)
	: WorksheetElement(name, dd, AspectType::BoxPlot) {
	init();
}

// no need to delete the d-pointer here - it inherits from QGraphicsItem
// and is deleted during the cleanup in QGraphicsScene
BoxPlot::~BoxPlot() = default;

void BoxPlot::init() {
	Q_D(BoxPlot);

	KConfig config;
	KConfigGroup group = config.group("BoxPlot");

	// general
	d->ordering = (BoxPlot::Ordering)group.readEntry("Ordering", (int)BoxPlot::Ordering::None);
	d->whiskersType = (BoxPlot::WhiskersType)group.readEntry("WhiskersType", (int)BoxPlot::WhiskersType::IQR);
	d->whiskersRangeParameter = group.readEntry("WhiskersIQRParameter", 1.5);
	d->orientation = (BoxPlot::Orientation)group.readEntry("Orientation", (int)BoxPlot::Orientation::Vertical);
	d->variableWidth = group.readEntry("VariableWidth", false);
	d->widthFactor = group.readEntry("WidthFactor", 1.0);
	d->notchesEnabled = group.readEntry("NotchesEnabled", false);

	// box filling
	d->background = new Background(QString());
	d->background->setPrefix(QLatin1String("Filling"));
	d->background->setEnabledAvailable(true);
	addChild(d->background);
	d->background->setHidden(true);
	d->background->init(group);
	connect(d->background, &Background::updateRequested, [=] {
		d->updatePixmap();
	});

	// median line
	d->medianLinePen = QPen(group.readEntry("MedianLineColor", QColor(Qt::black)),
							group.readEntry("MedianLineWidth", Worksheet::convertToSceneUnits(1.0, Worksheet::Unit::Point)),
							(Qt::PenStyle)group.readEntry("BorderStyle", (int)Qt::SolidLine));
	d->medianLineOpacity = group.readEntry("BorderOpacity", 1.0);

	// box border
	d->borderPen = QPen(group.readEntry("BorderColor", QColor(Qt::black)),
						group.readEntry("BorderWidth", Worksheet::convertToSceneUnits(1.0, Worksheet::Unit::Point)),
						(Qt::PenStyle)group.readEntry("BorderStyle", (int)Qt::SolidLine));
	d->borderOpacity = group.readEntry("BorderOpacity", 1.0);

	// markers
	d->symbolMean = new Symbol("symbolMean");
	addChild(d->symbolMean);
	d->symbolMean->setHidden(true);
	d->symbolMean->init(group);
	d->symbolMean->setStyle(Symbol::Style::Square);
	connect(d->symbolMean, &Symbol::updateRequested, [=] {
		d->recalcShapeAndBoundingRect();
	});
	connect(d->symbolMean, &Symbol::updatePixmapRequested, [=] {
		d->updatePixmap();
	});

	d->symbolMedian = new Symbol("symbolMedian");
	addChild(d->symbolMedian);
	d->symbolMedian->setHidden(true);
	d->symbolMedian->init(group);
	d->symbolMedian->setStyle(Symbol::Style::NoSymbols);
	connect(d->symbolMedian, &Symbol::updateRequested, [=] {
		d->recalcShapeAndBoundingRect();
	});
	connect(d->symbolMedian, &Symbol::updatePixmapRequested, [=] {
		d->updatePixmap();
	});

	d->symbolOutlier = new Symbol("symbolOutlier");
	addChild(d->symbolOutlier);
	d->symbolOutlier->setHidden(true);
	d->symbolOutlier->init(group);
	connect(d->symbolOutlier, &Symbol::updateRequested, [=] {
		d->recalcShapeAndBoundingRect();
	});
	connect(d->symbolOutlier, &Symbol::updatePixmapRequested, [=] {
		d->updatePixmap();
	});

	d->symbolFarOut = new Symbol("symbolFarOut");
	addChild(d->symbolFarOut);
	d->symbolFarOut->setHidden(true);
	d->symbolFarOut->init(group);
	d->symbolFarOut->setStyle(Symbol::Style::Plus);
	connect(d->symbolFarOut, &Symbol::updateRequested, [=] {
		d->recalcShapeAndBoundingRect();
	});
	connect(d->symbolFarOut, &Symbol::updatePixmapRequested, [=] {
		d->updatePixmap();
	});

	d->symbolData = new Symbol("symbolData");
	addChild(d->symbolData);
	d->symbolData->setHidden(true);
	d->symbolData->init(group);
	d->symbolData->setStyle(Symbol::Style::NoSymbols);
	d->symbolData->setOpacity(0.5);
	connect(d->symbolData, &Symbol::updateRequested, [=] {
		d->recalcShapeAndBoundingRect();
	});
	connect(d->symbolData, &Symbol::updatePixmapRequested, [=] {
		d->updatePixmap();
	});

	d->symbolWhiskerEnd = new Symbol("symbolWhiskerEnd");
	addChild(d->symbolWhiskerEnd);
	d->symbolWhiskerEnd->setHidden(true);
	d->symbolWhiskerEnd->init(group);
	d->symbolWhiskerEnd->setStyle(Symbol::Style::NoSymbols);
	connect(d->symbolWhiskerEnd, &Symbol::updateRequested, [=] {
		d->recalcShapeAndBoundingRect();
	});
	connect(d->symbolWhiskerEnd, &Symbol::updatePixmapRequested, [=] {
		d->updatePixmap();
	});

	d->jitteringEnabled = group.readEntry("JitteringEnabled", true);

	// whiskers
	d->whiskersPen = QPen(group.readEntry("WhiskersColor", QColor(Qt::black)),
						  group.readEntry("WhiskersWidth", Worksheet::convertToSceneUnits(1.0, Worksheet::Unit::Point)),
						  (Qt::PenStyle)group.readEntry("WhiskersStyle", (int)Qt::SolidLine));
	d->whiskersOpacity = group.readEntry("WhiskersOpacity", 1.0);
	d->whiskersCapSize = group.readEntry("WhiskersCapSize", Worksheet::convertToSceneUnits(5.0, Worksheet::Unit::Point));
	d->whiskersCapPen = QPen(group.readEntry("WhiskersCapColor", QColor(Qt::black)),
							 group.readEntry("WhiskersCapWidth", Worksheet::convertToSceneUnits(1.0, Worksheet::Unit::Point)),
							 (Qt::PenStyle)group.readEntry("WhiskersCapStyle", (int)Qt::SolidLine));
	d->whiskersCapOpacity = group.readEntry("WhiskersCapOpacity", 1.0);

	// marginal plots (rug, BoxPlot, boxplot)
	d->rugEnabled = group.readEntry("RugEnabled", false);
	d->rugLength = group.readEntry("RugLength", Worksheet::convertToSceneUnits(5, Worksheet::Unit::Point));
	d->rugWidth = group.readEntry("RugWidth", 0.0);
	d->rugOffset = group.readEntry("RugOffset", 0.0);
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
	pa.drawRect(6, 6, 8, 8); // box
	pa.drawLine(10, 6, 10, 0); // upper whisker
	pa.drawLine(10, 14, 10, 20); // lower whisker
	pa.end();

	return {pm};
}

void BoxPlot::initActions() {
	visibilityAction = new QAction(QIcon::fromTheme("view-visible"), i18n("Visible"), this);
	visibilityAction->setCheckable(true);
	connect(visibilityAction, &QAction::triggered, this, &BoxPlot::visibilityChangedSlot);

	// Orientation
	auto* orientationActionGroup = new QActionGroup(this);
	orientationActionGroup->setExclusive(true);
	connect(orientationActionGroup, &QActionGroup::triggered, this, &BoxPlot::orientationChangedSlot);

	orientationHorizontalAction = new QAction(QIcon::fromTheme(QLatin1String("transform-move-horizontal")), i18n("Horizontal"), orientationActionGroup);
	orientationHorizontalAction->setCheckable(true);

	orientationVerticalAction = new QAction(QIcon::fromTheme(QLatin1String("transform-move-vertical")), i18n("Vertical"), orientationActionGroup);
	orientationVerticalAction->setCheckable(true);
}

void BoxPlot::initMenus() {
	this->initActions();

	// Orientation
	orientationMenu = new QMenu(i18n("Orientation"));
	orientationMenu->setIcon(QIcon::fromTheme(QLatin1String("draw-cross")));
	orientationMenu->addAction(orientationHorizontalAction);
	orientationMenu->addAction(orientationVerticalAction);
}

QMenu* BoxPlot::createContextMenu() {
	if (!orientationMenu)
		initMenus();

	QMenu* menu = WorksheetElement::createContextMenu();
	QAction* firstAction = menu->actions().at(1); // skip the first action because of the "title-action"

	// Visibility
	visibilityAction->setChecked(isVisible());
	menu->insertAction(firstAction, visibilityAction);

	// Orientation
	Q_D(const BoxPlot);
	if (d->orientation == Orientation::Horizontal)
		orientationHorizontalAction->setChecked(true);
	else
		orientationVerticalAction->setChecked(true);
	menu->insertMenu(firstAction, orientationMenu);
	menu->insertSeparator(firstAction);

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

/*!
 * creates a new spreadsheet having the data with the positions and the values of the bins.
 * the new spreadsheet is added to the current folder.
 */
void BoxPlot::createDataSpreadsheet() {
	if (dataColumns().isEmpty())
		return;

	// create a new spreadsheet for the following 9 metrics:
	// index
	// 1st quartile
	// 3rd quartile
	// median
	// whiskers min
	// whiskers max
	// data points count
	// outliers count
	// far out points count

	auto* spreadsheet = new Spreadsheet(i18n("%1 - Data", name()));
	spreadsheet->setColumnCount(9);
	spreadsheet->setRowCount(dataColumns().count());
	spreadsheet->column(0)->setColumnMode(AbstractColumn::ColumnMode::Integer);

	spreadsheet->column(0)->setName(i18n("index"));
	spreadsheet->column(1)->setName(i18n("1st quartile"));
	spreadsheet->column(2)->setName(i18n("3rd quartile"));
	spreadsheet->column(3)->setName(i18n("median"));
	spreadsheet->column(4)->setName(i18n("whiskers min"));
	spreadsheet->column(5)->setName(i18n("whiskers max"));
	spreadsheet->column(6)->setName(i18n("data points count"));
	spreadsheet->column(7)->setName(i18n("outliers count"));
	spreadsheet->column(8)->setName(i18n("far out points count"));

	Q_D(const BoxPlot);
	d->fillDataSpreadsheet(spreadsheet);

	// add the new spreadsheet to the current folder
	folder()->addChild(spreadsheet);
}

/* ============================ getter methods ================= */
// general
BASIC_SHARED_D_READER_IMPL(BoxPlot, QVector<const AbstractColumn*>, dataColumns, dataColumns)
BASIC_SHARED_D_READER_IMPL(BoxPlot, BoxPlot::Ordering, ordering, ordering)
BASIC_SHARED_D_READER_IMPL(BoxPlot, BoxPlot::Orientation, orientation, orientation)
BASIC_SHARED_D_READER_IMPL(BoxPlot, bool, variableWidth, variableWidth)
BASIC_SHARED_D_READER_IMPL(BoxPlot, double, widthFactor, widthFactor)
BASIC_SHARED_D_READER_IMPL(BoxPlot, bool, notchesEnabled, notchesEnabled)

// box filling
Background* BoxPlot::background() const {
	Q_D(const BoxPlot);
	return d->background;
}

// border
BASIC_SHARED_D_READER_IMPL(BoxPlot, QPen, borderPen, borderPen)
BASIC_SHARED_D_READER_IMPL(BoxPlot, qreal, borderOpacity, borderOpacity)

// median line
BASIC_SHARED_D_READER_IMPL(BoxPlot, QPen, medianLinePen, medianLinePen)
BASIC_SHARED_D_READER_IMPL(BoxPlot, qreal, medianLineOpacity, medianLineOpacity)

// markers
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

Symbol* BoxPlot::symbolWhiskerEnd() const {
	Q_D(const BoxPlot);
	return d->symbolWhiskerEnd;
}

// whiskers
BASIC_SHARED_D_READER_IMPL(BoxPlot, BoxPlot::WhiskersType, whiskersType, whiskersType)
BASIC_SHARED_D_READER_IMPL(BoxPlot, double, whiskersRangeParameter, whiskersRangeParameter)
BASIC_SHARED_D_READER_IMPL(BoxPlot, QPen, whiskersPen, whiskersPen)
BASIC_SHARED_D_READER_IMPL(BoxPlot, qreal, whiskersOpacity, whiskersOpacity)
BASIC_SHARED_D_READER_IMPL(BoxPlot, double, whiskersCapSize, whiskersCapSize)
BASIC_SHARED_D_READER_IMPL(BoxPlot, QPen, whiskersCapPen, whiskersCapPen)
BASIC_SHARED_D_READER_IMPL(BoxPlot, qreal, whiskersCapOpacity, whiskersCapOpacity)

// margin plots
BASIC_SHARED_D_READER_IMPL(BoxPlot, bool, rugEnabled, rugEnabled)
BASIC_SHARED_D_READER_IMPL(BoxPlot, double, rugLength, rugLength)
BASIC_SHARED_D_READER_IMPL(BoxPlot, double, rugWidth, rugWidth)
BASIC_SHARED_D_READER_IMPL(BoxPlot, double, rugOffset, rugOffset)

QVector<QString>& BoxPlot::dataColumnPaths() const {
	D(BoxPlot);
	return d->dataColumnPaths;
}

double BoxPlot::minimum(const Dimension dim) const {
	Q_D(const BoxPlot);
	switch (dim) {
	case Dimension::X:
		return d->xMin;
	case Dimension::Y:
		return d->yMin;
	}
	return NAN;
}

double BoxPlot::maximum(const Dimension dim) const {
	Q_D(const BoxPlot);
	switch (dim) {
	case Dimension::X:
		return d->xMax;
	case Dimension::Y:
		return d->yMax;
	}
	return NAN;
}

/* ============================ setter methods and undo commands ================= */

// General
STD_SETTER_CMD_IMPL_F_S(BoxPlot, SetDataColumns, QVector<const AbstractColumn*>, dataColumns, recalc)
void BoxPlot::setDataColumns(const QVector<const AbstractColumn*> columns) {
	Q_D(BoxPlot);
	if (columns != d->dataColumns) {
		exec(new BoxPlotSetDataColumnsCmd(d, columns, ki18n("%1: set data columns")));

		for (auto* column : columns) {
			if (!column)
				continue;

			connect(column, &AbstractColumn::dataChanged, this, &BoxPlot::dataChanged);

			// update the curve itself on changes
			connect(column, &AbstractColumn::dataChanged, this, &BoxPlot::recalc);
			connect(column->parentAspect(), &AbstractAspect::aspectAboutToBeRemoved, this, &BoxPlot::dataColumnAboutToBeRemoved);
			// TODO: add disconnect in the undo-function
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

// box border
STD_SETTER_CMD_IMPL_F_S(BoxPlot, SetBorderPen, QPen, borderPen, recalcShapeAndBoundingRect)
void BoxPlot::setBorderPen(const QPen& pen) {
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

// median line
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

// whiskers
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

// Symbols
STD_SETTER_CMD_IMPL_F_S(BoxPlot, SetJitteringEnabled, bool, jitteringEnabled, recalc)
void BoxPlot::setJitteringEnabled(bool enabled) {
	Q_D(BoxPlot);
	if (enabled != d->jitteringEnabled)
		exec(new BoxPlotSetJitteringEnabledCmd(d, enabled, ki18n("%1: jitterring changed")));
}

// margin plots
STD_SETTER_CMD_IMPL_F_S(BoxPlot, SetRugEnabled, bool, rugEnabled, updateRug)
void BoxPlot::setRugEnabled(bool enabled) {
	Q_D(BoxPlot);
	if (enabled != d->rugEnabled)
		exec(new BoxPlotSetRugEnabledCmd(d, enabled, ki18n("%1: change rug enabled")));
}

STD_SETTER_CMD_IMPL_F_S(BoxPlot, SetRugWidth, double, rugWidth, updatePixmap)
void BoxPlot::setRugWidth(double width) {
	Q_D(BoxPlot);
	if (width != d->rugWidth)
		exec(new BoxPlotSetRugWidthCmd(d, width, ki18n("%1: change rug width")));
}

STD_SETTER_CMD_IMPL_F_S(BoxPlot, SetRugLength, double, rugLength, updateRug)
void BoxPlot::setRugLength(double length) {
	Q_D(BoxPlot);
	if (length != d->rugLength)
		exec(new BoxPlotSetRugLengthCmd(d, length, ki18n("%1: change rug length")));
}

STD_SETTER_CMD_IMPL_F_S(BoxPlot, SetRugOffset, double, rugOffset, updateRug)
void BoxPlot::setRugOffset(double offset) {
	Q_D(BoxPlot);
	if (offset != d->rugOffset)
		exec(new BoxPlotSetRugOffsetCmd(d, offset, ki18n("%1: change rug offset")));
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
	if (action == orientationHorizontalAction)
		this->setOrientation(Axis::Orientation::Horizontal);
	else
		this->setOrientation(Axis::Orientation::Vertical);
}

void BoxPlot::visibilityChangedSlot() {
	Q_D(const BoxPlot);
	this->setVisible(!d->isVisible());
}

//##############################################################################
//####################### Private implementation ###############################
//##############################################################################
BoxPlotPrivate::BoxPlotPrivate(BoxPlot* owner)
	: WorksheetElementPrivate(owner)
	, q(owner) {
	setFlag(QGraphicsItem::ItemIsSelectable);
	setAcceptHoverEvents(false);
}

bool BoxPlotPrivate::activateCurve(QPointF mouseScenePos, double /*maxDist*/) {
	if (!isVisible())
		return false;

	return shape().contains(mouseScenePos);
}

void BoxPlotPrivate::setHover(bool on) {
	if (on == m_hovered)
		return; // don't update if state not changed

	m_hovered = on;
	on ? Q_EMIT q->hovered() : emit q->unhovered();
	update();
}

void BoxPlotPrivate::fillDataSpreadsheet(Spreadsheet* spreadsheet) const {
	// index
	// 1st quartile
	// 3rd quartile
	// median
	// whiskers min
	// whiskers max
	// data points count
	// outliers count
	// far out points count

	for (int i = 0; i < q->dataColumns().count(); ++i) {
		const auto* column = static_cast<const Column*>(q->dataColumns().at(i));
		const auto& statistics = column->statistics();

		spreadsheet->column(0)->setIntegerAt(i, i + 1);
		spreadsheet->column(1)->setValueAt(i, statistics.firstQuartile);
		spreadsheet->column(2)->setValueAt(i, statistics.thirdQuartile);
		spreadsheet->column(3)->setValueAt(i, statistics.median);
		spreadsheet->column(4)->setValueAt(i, m_whiskerMin.at(i));
		spreadsheet->column(5)->setValueAt(i, m_whiskerMax.at(i));
		spreadsheet->column(6)->setValueAt(i, m_dataPointsLogical.at(i).count());
		spreadsheet->column(7)->setValueAt(i, m_outlierPointsLogical.at(i).count());
		spreadsheet->column(8)->setValueAt(i, m_farOutPointsLogical.at(i).count());
	}
}
/*!
  called when the size of the plot or its data ranges (manual changes, zooming, etc.) were changed.
  recalculates the position of the scene points to be drawn.
  triggers the update of lines, drop lines, symbols etc.
*/
void BoxPlotPrivate::retransform() {
	if (suppressRetransform || !isVisible() || q->isLoading())
		return;

	PERFTRACE(name() + Q_FUNC_INFO);

	const int count = dataColumns.size();
	if (!count || m_boxRect.size() != count) {
		// no columns or recalc() was not called yet, nothing to do
		recalcShapeAndBoundingRect();
		return;
	}

	// clear the containers holding the information in scene coordinates
	for (int i = 0; i < count; ++i) {
		m_boxRect[i].clear();
		m_medianLine[i] = QLineF();
		m_whiskersPath[i] = QPainterPath();
		m_whiskersCapPath[i] = QPainterPath();
		m_rugPath[i] = QPainterPath();
		m_outlierPoints[i].clear();
		m_dataPoints[i].clear();
		m_farOutPoints[i].clear();
		m_whiskerEndPoints[i].clear();
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

		updateRug();
	}

	recalcShapeAndBoundingRect();
}

void BoxPlotPrivate::recalc() {
	PERFTRACE(name() + Q_FUNC_INFO);

	// resize the internal containers
	const int count = dataColumns.size();
	m_boxRect.resize(count);
	m_fillPolygon.resize(count);
	m_xMinBox.resize(count);
	m_xMaxBox.resize(count);
	m_yMinBox.resize(count);
	m_yMaxBox.resize(count);
	m_median.resize(count);
	m_medianLine.resize(count);
	m_whiskersPath.resize(count);
	m_whiskersCapPath.resize(count);
	m_rugPath.resize(count);
	m_whiskerMin.resize(count);
	m_whiskerMax.resize(count);
	m_outlierPointsLogical.resize(count);
	m_outlierPoints.resize(count);
	m_dataPointsLogical.resize(count);
	m_dataPoints.resize(count);
	m_farOutPointsLogical.resize(count);
	m_farOutPoints.resize(count);
	m_whiskerEndPointsLogical.resize(count);
	m_whiskerEndPoints.resize(count);
	m_mean.resize(count);
	m_meanPointLogical.resize(count);
	m_meanPoint.resize(count);
	m_meanPointVisible.resize(count);
	m_medianPointLogical.resize(count);
	m_medianPoint.resize(count);
	m_medianPointVisible.resize(count);

	// calculate the new min and max values of the box plot
	// for the current sizes of the box and of the whiskers
	if (orientation == BoxPlot::Orientation::Vertical) {
		xMin = 0.5;
		xMax = count + 0.5;
		yMin = INFINITY;
		yMax = -INFINITY;
	} else { // horizontal
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

		if (ordering == BoxPlot::Ordering::MedianAscending || ordering == BoxPlot::Ordering::MedianDescending) {
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

		if (ordering == BoxPlot::Ordering::MedianAscending || ordering == BoxPlot::Ordering::MeanAscending) {
			for (int i = 0; i < count; ++i)
				dataColumnsOrdered << dataColumns.at(newOrdering.at(i).second);
		} else {
			for (int i = count - 1; i >= 0; --i)
				dataColumnsOrdered << dataColumns.at(newOrdering.at(i).second);
		}
	}

	for (int i = 0; i < count; ++i)
		recalc(i);

	// the size of the boxplot changed because of the actual
	// data changes or because of new boxplot settings.
	// Q_EMIT dataChanged() in order to recalculate everything
	// in the parent plot with the new size/shape of the boxplot
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

	// clear the containers for outliers, etc. since their number
	// can be changed because of the new settings for whiskers, etc.
	m_outlierPointsLogical[index].clear();
	m_outlierPoints[index].clear();
	m_dataPointsLogical[index].clear();
	m_dataPoints[index].clear();
	m_farOutPointsLogical[index].clear();
	m_farOutPoints[index].clear();
	m_whiskerEndPointsLogical[index].clear();
	m_whiskerEndPoints[index].clear();

	const auto& statistics = column->statistics();
	double width = 0.5 * widthFactor;
	if (variableWidth && m_widthScaleFactor != 0)
		width *= std::sqrt(statistics.size) / m_widthScaleFactor;

	const double x = index + 1.0;

	// box
	if (orientation == BoxPlot::Orientation::Vertical) {
		m_xMinBox[index] = x - width / 2;
		m_xMaxBox[index] = x + width / 2;
		m_yMinBox[index] = statistics.firstQuartile;
		m_yMaxBox[index] = statistics.thirdQuartile;
	} else {
		m_xMinBox[index] = statistics.firstQuartile;
		m_xMaxBox[index] = statistics.thirdQuartile;
		m_yMinBox[index] = x - width / 2;
		m_yMaxBox[index] = x + width / 2;
	}

	// mean and median
	m_median[index] = statistics.median;
	m_mean[index] = statistics.arithmeticMean;

	// whiskers
	switch (whiskersType) {
	case BoxPlot::WhiskersType::MinMax: {
		m_whiskerMax[index] = statistics.maximum;
		m_whiskerMin[index] = statistics.minimum;
		break;
	}
	case BoxPlot::WhiskersType::IQR: {
		m_whiskerMax[index] = statistics.thirdQuartile + whiskersRangeParameter * statistics.iqr;
		m_whiskerMin[index] = statistics.firstQuartile - whiskersRangeParameter * statistics.iqr;
		break;
	}
	case BoxPlot::WhiskersType::SD: {
		m_whiskerMax[index] = statistics.arithmeticMean + whiskersRangeParameter * statistics.standardDeviation;
		m_whiskerMin[index] = statistics.arithmeticMean - whiskersRangeParameter * statistics.standardDeviation;
		break;
	}
	case BoxPlot::WhiskersType::MAD: {
		m_whiskerMax[index] = statistics.median + whiskersRangeParameter * statistics.meanDeviationAroundMedian;
		m_whiskerMin[index] = statistics.median - whiskersRangeParameter * statistics.meanDeviationAroundMedian;
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

	// outliers symbols
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

	double whiskerMax = -qInf(); // upper adjacent value
	double whiskerMin = qInf(); // lower adjacent value
	const double outerFenceMax = statistics.thirdQuartile + 3.0 * statistics.iqr;
	const double outerFenceMin = statistics.firstQuartile - 3.0 * statistics.iqr;

	for (int row = 0; row < column->rowCount(); ++row) {
		if (!column->isValid(row) || column->isMasked(row))
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
				m_farOutPointsLogical[index] << setOutlierPoint(x - width / 2 + rand * width, value);
			else
				m_outlierPointsLogical[index] << setOutlierPoint(x - width / 2 + rand * width, value);
		} else {
			if (orientation == BoxPlot::Orientation::Vertical)
				m_dataPointsLogical[index] << QPointF(x - width / 2 + rand * width, value);
			else
				m_dataPointsLogical[index] << QPointF(value, x - width / 2 + rand * width);

			// determine the upper/lower adjucent values
			if (whiskersType == BoxPlot::WhiskersType::IQR) {
				if (value > whiskerMax)
					whiskerMax = value;

				if (value < whiskerMin)
					whiskerMin = value;
			}
		}
	}

	// set the whisker ends at the upper and lower adjacent values
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

	// box
	if (!notchesEnabled) {
		// first line starting at the top left corner of the box
		lines << QLineF(xMinBox, yMaxBox, xMaxBox, yMaxBox);
		lines << QLineF(xMaxBox, yMaxBox, xMaxBox, yMinBox);
		lines << QLineF(xMaxBox, yMinBox, xMinBox, yMinBox);
		lines << QLineF(xMinBox, yMinBox, xMinBox, yMaxBox);
	} else {
		auto* column = static_cast<const Column*>(dataColumnsOrdered.at(index));
		const auto& statistics = column->statistics();
		const double notch = 1.7 * 1.25 * statistics.iqr / 1.35 / std::sqrt(statistics.size);
		const double notchMax = median + notch;
		const double notchMin = median - notch;
		double width = xMaxBox - xMinBox;

		// first line starting at the top left corner of the box
		lines << QLineF(xMinBox, yMaxBox, xMaxBox, yMaxBox);
		lines << QLineF(xMaxBox, yMaxBox, xMaxBox, notchMax);
		lines << QLineF(xMaxBox, notchMax, xMinBox + 0.9 * width, median);
		lines << QLineF(xMinBox + 0.9 * width, median, xMaxBox, notchMin);
		lines << QLineF(xMaxBox, notchMin, xMaxBox, yMinBox);
		lines << QLineF(xMaxBox, yMinBox, xMinBox, yMinBox);
		lines << QLineF(xMinBox, yMinBox, xMinBox, notchMin);
		lines << QLineF(xMinBox, notchMin, xMinBox + 0.1 * width, median);
		lines << QLineF(xMinBox + 0.1 * width, median, xMinBox, notchMax);
		lines << QLineF(xMinBox, notchMax, xMinBox, yMaxBox);
	}

	m_boxRect[index] = q->cSystem->mapLogicalToScene(lines);
	updateFillingRect(index, lines);

	// median line
	lines.clear();
	if (!notchesEnabled)
		lines << QLineF(xMinBox, median, xMaxBox, median);
	else {
		double width = xMaxBox - xMinBox;
		lines << QLineF(xMinBox + 0.1 * width, median, m_xMaxBox.at(index) - 0.1 * width, median);
	}

	lines = q->cSystem->mapLogicalToScene(lines);
	if (!lines.isEmpty())
		m_medianLine[index] = lines.first();

	// whisker lines
	lines.clear();
	lines << QLineF(x, m_yMaxBox.at(index), x, m_whiskerMax.at(index)); // upper whisker
	lines << QLineF(x, m_yMinBox.at(index), x, m_whiskerMin.at(index)); // lower whisker
	lines = q->cSystem->mapLogicalToScene(lines);
	for (const auto& line : qAsConst(lines)) {
		m_whiskersPath[index].moveTo(line.p1());
		m_whiskersPath[index].lineTo(line.p2());
	}

	// whisker caps
	if (!m_whiskersPath[index].isEmpty()) {
		bool visible = false;
		QPointF maxPoint = q->cSystem->mapLogicalToScene(QPointF(x, m_whiskerMax.at(index)), visible);
		if (visible) {
			m_whiskersCapPath[index].moveTo(QPointF(maxPoint.x() - whiskersCapSize / 2., maxPoint.y()));
			m_whiskersCapPath[index].lineTo(QPointF(maxPoint.x() + whiskersCapSize / 2., maxPoint.y()));
			m_whiskerEndPointsLogical[index] << QPointF(x, m_whiskerMax.at(index));
		}

		QPointF minPoint = q->cSystem->mapLogicalToScene(QPointF(x, m_whiskerMin.at(index)), visible);
		if (visible) {
			m_whiskersCapPath[index].moveTo(QPointF(minPoint.x() - whiskersCapSize / 2., minPoint.y()));
			m_whiskersCapPath[index].lineTo(QPointF(minPoint.x() + whiskersCapSize / 2., minPoint.y()));
			m_whiskerEndPointsLogical[index] << QPointF(x, m_whiskerMin.at(index));
		}
	}

	// map the logical points of symbols to scene coordinates
	m_meanPointLogical[index] = QPointF(x, m_mean.at(index));
	m_medianPointLogical[index] = QPointF(x, m_median.at(index));
	mapSymbolsToScene(index);
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

	// box
	if (!notchesEnabled) {
		// first line starting at the top left corner of the box
		lines << QLineF(xMinBox, yMaxBox, xMaxBox, yMaxBox);
		lines << QLineF(xMaxBox, yMaxBox, xMaxBox, yMinBox);
		lines << QLineF(xMaxBox, yMinBox, xMinBox, yMinBox);
		lines << QLineF(xMinBox, yMinBox, xMinBox, yMaxBox);
	} else {
		auto* column = static_cast<const Column*>(dataColumnsOrdered.at(index));
		const auto& statistics = column->statistics();
		const double notch = 1.7 * 1.25 * statistics.iqr / 1.35 / std::sqrt(statistics.size);
		const double notchMax = median + notch;
		const double notchMin = median - notch;
		double width = yMaxBox - yMinBox;

		lines << QLineF(xMinBox, yMaxBox, notchMin, yMaxBox);
		lines << QLineF(notchMin, yMaxBox, median, yMaxBox - 0.1 * width);
		lines << QLineF(median, yMaxBox - 0.1 * width, notchMax, yMaxBox);
		lines << QLineF(notchMax, yMaxBox, xMaxBox, yMaxBox);
		lines << QLineF(xMaxBox, yMaxBox, xMaxBox, yMinBox);
		lines << QLineF(xMaxBox, yMinBox, notchMax, yMinBox);
		lines << QLineF(notchMax, yMinBox, median, yMinBox + 0.1 * width);
		lines << QLineF(median, yMinBox + 0.1 * width, notchMin, yMinBox);
		lines << QLineF(notchMin, yMinBox, xMinBox, yMinBox);
		lines << QLineF(xMinBox, yMinBox, xMinBox, yMaxBox);
	}

	m_boxRect[index] = q->cSystem->mapLogicalToScene(lines);
	updateFillingRect(index, lines);

	// median line
	lines.clear();
	if (!notchesEnabled)
		lines << QLineF(median, yMinBox, median, yMaxBox);
	else {
		double width = yMaxBox - yMinBox;
		lines << QLineF(median, yMinBox + 0.1 * width, median, yMaxBox - 0.1 * width);
	}

	lines = q->cSystem->mapLogicalToScene(lines);
	if (!lines.isEmpty())
		m_medianLine[index] = lines.first();

	// whisker lines
	lines.clear();
	lines << QLineF(m_xMaxBox.at(index), y, m_whiskerMax.at(index), y); // upper whisker
	lines << QLineF(m_xMinBox.at(index), y, m_whiskerMin.at(index), y); // lower whisker
	lines = q->cSystem->mapLogicalToScene(lines);
	for (const auto& line : qAsConst(lines)) {
		m_whiskersPath[index].moveTo(line.p1());
		m_whiskersPath[index].lineTo(line.p2());
	}

	// whisker caps
	if (!m_whiskersPath[index].isEmpty()) {
		bool visible = false;
		QPointF maxPoint = q->cSystem->mapLogicalToScene(QPointF(m_whiskerMax.at(index), y), visible);
		if (visible) {
			m_whiskersCapPath[index].moveTo(QPointF(maxPoint.x(), maxPoint.y() - whiskersCapSize / 2));
			m_whiskersCapPath[index].lineTo(QPointF(maxPoint.x(), maxPoint.y() + whiskersCapSize / 2));
			m_whiskerEndPointsLogical[index] << QPointF(m_whiskerMax.at(index), y);
		}

		QPointF minPoint = q->cSystem->mapLogicalToScene(QPointF(m_whiskerMin.at(index), y), visible);
		if (visible) {
			m_whiskersCapPath[index].moveTo(QPointF(minPoint.x(), minPoint.y() - whiskersCapSize / 2));
			m_whiskersCapPath[index].lineTo(QPointF(minPoint.x(), minPoint.y() + whiskersCapSize / 2));
			m_whiskerEndPointsLogical[index] << QPointF(m_whiskerMin.at(index), y);
		}
	}

	// outliers symbols
	m_meanPointLogical[index] = QPointF(m_mean.at(index), y);
	m_medianPointLogical[index] = QPointF(m_median.at(index), y);
	mapSymbolsToScene(index);
}

void BoxPlotPrivate::updateRug() {
	if (!rugEnabled || !q->plot()) {
		recalcShapeAndBoundingRect();
		return;
	}

	auto cs = q->plot()->coordinateSystem(q->coordinateSystemIndex());
	const double xMin = q->plot()->range(Dimension::X, cs->index(Dimension::X)).start();
	const double yMin = q->plot()->range(Dimension::Y, cs->index(Dimension::Y)).start();

	QPainterPath rugPath;
	QVector<QPointF> points;

	for (int i = 0; i < q->dataColumns().count(); ++i) {
		const auto* column = static_cast<const Column*>(q->dataColumns().at(i));
#if (QT_VERSION >= QT_VERSION_CHECK(5, 13, 0))
		rugPath.clear();
#else
		rugPath = QPainterPath();
#endif
		points.clear();

		if (orientation == BoxPlot::Orientation::Horizontal) {
			for (int row = 0; row < column->rowCount(); ++row) {
				if (column->isValid(row) && !column->isMasked(row))
					points << QPointF(column->valueAt(row), yMin);
			}

			// map the points to scene coordinates
			points = q->cSystem->mapLogicalToScene(points);

			// path for the vertical rug lines
			for (const auto& point : qAsConst(points)) {
				rugPath.moveTo(point.x(), point.y() - rugOffset);
				rugPath.lineTo(point.x(), point.y() - rugOffset - rugLength);
			}
		} else { // horizontal
			for (int row = 0; row < column->rowCount(); ++row) {
				if (column->isValid(row) && !column->isMasked(row))
					points << QPointF(xMin, column->valueAt(row));
			}

			// map the points to scene coordinates
			points = q->cSystem->mapLogicalToScene(points);

			// path for the horizontal rug lines
			for (const auto& point : qAsConst(points)) {
				rugPath.moveTo(point.x() + rugOffset, point.y());
				rugPath.lineTo(point.x() + rugOffset + rugLength, point.y());
			}
		}

		m_rugPath[i] = rugPath;
	}

	recalcShapeAndBoundingRect();
}

void BoxPlotPrivate::updateFillingRect(int index, const QVector<QLineF>& lines) {
	const auto& unclippedLines = q->cSystem->mapLogicalToScene(lines, AbstractCoordinateSystem::MappingFlag::SuppressPageClipping);

	if (unclippedLines.isEmpty()) {
		m_fillPolygon[index] = QPolygonF();
		return;
	}

	// we have four unclipped lines for the box.
	// clip the points to the plot data rect and create a new polygon
	// out of them that will be filled out.
	QPolygonF polygon;
	const QRectF& dataRect = static_cast<CartesianPlot*>(q->parentAspect())->dataRect();
	int i = 0;
	for (const auto& line : unclippedLines) {
		// clip the first point of the line
		QPointF p1 = line.p1();
		if (p1.x() < dataRect.left())
			p1.setX(dataRect.left());
		else if (p1.x() > dataRect.right())
			p1.setX(dataRect.right());

		if (p1.y() < dataRect.top())
			p1.setY(dataRect.top());
		else if (p1.y() > dataRect.bottom())
			p1.setY(dataRect.bottom());

		// clip the second point of the line
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
			// close the polygon for the last line
			polygon << p1;
			polygon << p2;
		}

		++i;
	}

	m_fillPolygon[index] = polygon;
}

/*!
 * map the outlier points from logical to scene coordinates,
 * avoid drawing overlapping points, logic similar to
 * //XYCurvePrivate::retransform()
 */
void BoxPlotPrivate::mapSymbolsToScene(int index) {
	// outliers
	int size = m_outlierPointsLogical[index].size();
	if (size > 0) {
		const int startIndex = 0;
		const int endIndex = m_outlierPointsLogical[index].size() - 1;
		std::vector<bool> m_pointVisible;
		m_pointVisible.resize(size);

		q->cSystem->mapLogicalToScene(startIndex, endIndex, m_outlierPointsLogical[index], m_outlierPoints[index], m_pointVisible);
	}

	// data points
	size = m_dataPointsLogical[index].size();
	if (size > 0) {
		const int startIndex = 0;
		const int endIndex = m_dataPointsLogical[index].size() - 1;
		std::vector<bool> pointVisible;
		pointVisible.resize(size);

		q->cSystem->mapLogicalToScene(startIndex, endIndex, m_dataPointsLogical[index], m_dataPoints[index], pointVisible);
	}

	// far out points
	size = m_farOutPointsLogical[index].size();
	if (size > 0) {
		const int startIndex = 0;
		const int endIndex = m_farOutPointsLogical[index].size() - 1;
		std::vector<bool> pointVisible;
		pointVisible.resize(size);

		q->cSystem->mapLogicalToScene(startIndex, endIndex, m_farOutPointsLogical[index], m_farOutPoints[index], pointVisible);
	}

	// whisker ends
	size = m_whiskerEndPointsLogical[index].size();
	if (size > 0) {
		const int startIndex = 0;
		const int endIndex = m_whiskerEndPointsLogical[index].size() - 1;
		std::vector<bool> pointVisible;
		pointVisible.resize(size);

		q->cSystem->mapLogicalToScene(startIndex, endIndex, m_whiskerEndPointsLogical[index], m_whiskerEndPoints[index], pointVisible);
	}

	// mean
	bool visible;
	m_meanPoint[index] = q->cSystem->mapLogicalToScene(m_meanPointLogical[index], visible);
	m_meanPointVisible[index] = visible;

	// median
	m_medianPoint[index] = q->cSystem->mapLogicalToScene(m_medianPointLogical[index], visible);
	m_medianPointVisible[index] = visible;
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
		if (!dataColumnsOrdered.at(i) || static_cast<const Column*>(dataColumnsOrdered.at(i))->statistics().size == 0)
			continue;

		QPainterPath boxPath;
		for (const auto& line : qAsConst(m_boxRect.at(i))) {
			boxPath.moveTo(line.p1());
			boxPath.lineTo(line.p2());
		}
		m_boxPlotShape.addPath(WorksheetElement::shapeFromPath(boxPath, borderPen));

		m_boxPlotShape.addPath(WorksheetElement::shapeFromPath(m_whiskersPath.at(i), whiskersPen));
		m_boxPlotShape.addPath(WorksheetElement::shapeFromPath(m_whiskersCapPath.at(i), whiskersCapPen));

		m_boxPlotShape.addPath(WorksheetElement::shapeFromPath(m_rugPath.at(i), borderPen));

		// add symbols outlier, jitter and far out values
		QPainterPath symbolsPath = QPainterPath();

		// outlier values
		if (symbolOutlier->style() != Symbol::Style::NoSymbols && !m_outlierPoints.at(i).isEmpty()) {
			QPainterPath path = Symbol::stylePath(symbolOutlier->style());
			QTransform trafo;
			trafo.scale(symbolOutlier->size(), symbolOutlier->size());
			path = trafo.map(path);
			trafo.reset();

			if (symbolOutlier->rotationAngle() != 0.) {
				trafo.rotate(symbolOutlier->rotationAngle());
				path = trafo.map(path);
			}

			for (const auto& point : qAsConst(m_outlierPoints.at(i))) {
				trafo.reset();
				trafo.translate(point.x(), point.y());
				symbolsPath.addPath(trafo.map(path));
			}
		}

		// jitter values
		if (symbolData->style() != Symbol::Style::NoSymbols && !m_dataPoints.at(i).isEmpty()) {
			QPainterPath path = Symbol::stylePath(symbolData->style());
			QTransform trafo;
			trafo.scale(symbolData->size(), symbolData->size());
			path = trafo.map(path);
			trafo.reset();

			if (symbolData->rotationAngle() != 0.) {
				trafo.rotate(symbolData->rotationAngle());
				path = trafo.map(path);
			}

			for (const auto& point : qAsConst(m_dataPoints.at(i))) {
				trafo.reset();
				trafo.translate(point.x(), point.y());
				symbolsPath.addPath(trafo.map(path));
			}
		}

		// far out values
		if (symbolFarOut->style() != Symbol::Style::NoSymbols && !m_farOutPoints.at(i).isEmpty()) {
			QPainterPath path = Symbol::stylePath(symbolFarOut->style());
			QTransform trafo;
			trafo.scale(symbolFarOut->size(), symbolFarOut->size());
			path = trafo.map(path);
			trafo.reset();

			if (symbolFarOut->rotationAngle() != 0.) {
				trafo.rotate(symbolFarOut->rotationAngle());
				path = trafo.map(path);
			}

			for (const auto& point : qAsConst(m_farOutPoints.at(i))) {
				trafo.reset();
				trafo.translate(point.x(), point.y());
				symbolsPath.addPath(trafo.map(path));
			}
		}

		// whisker ends
		if (symbolWhiskerEnd->style() != Symbol::Style::NoSymbols && !m_whiskerEndPoints.at(i).isEmpty()) {
			QPainterPath path = Symbol::stylePath(symbolWhiskerEnd->style());
			QTransform trafo;
			trafo.scale(symbolWhiskerEnd->size(), symbolWhiskerEnd->size());
			path = trafo.map(path);
			trafo.reset();

			if (symbolWhiskerEnd->rotationAngle() != 0.) {
				trafo.rotate(symbolWhiskerEnd->rotationAngle());
				path = trafo.map(path);
			}

			for (const auto& point : qAsConst(m_whiskerEndPoints.at(i))) {
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
	if (m_boundingRectangle.width() == 0. || m_boundingRectangle.height() == 0.) {
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

		// no need to draw anything if the column doesn't have any valid values
		if (static_cast<const Column*>(dataColumnsOrdered.at(i))->statistics().size == 0)
			continue;

		if (!m_boxRect.at(i).isEmpty()) {
			// draw the box filling
			if (background->enabled()) {
				painter->setOpacity(background->opacity());
				painter->setPen(Qt::NoPen);
				drawFilling(painter, i);
			}

			// draw the border
			if (borderPen.style() != Qt::NoPen) {
				painter->setPen(borderPen);
				painter->setBrush(Qt::NoBrush);
				painter->setOpacity(borderOpacity);
				for (const auto& line : m_boxRect.at(i))
					painter->drawLine(line);
			}

			// draw the median line
			if (medianLinePen.style() != Qt::NoPen) {
				painter->setPen(medianLinePen);
				painter->setBrush(Qt::NoBrush);
				painter->setOpacity(medianLineOpacity);
				painter->drawLine(m_medianLine.at(i));
			}
		}

		// draw the whiskers
		if (whiskersPen.style() != Qt::NoPen && !m_whiskersPath.at(i).isEmpty()) {
			painter->setPen(whiskersPen);
			painter->setBrush(Qt::NoBrush);
			painter->setOpacity(whiskersOpacity);
			painter->drawPath(m_whiskersPath.at(i));
		}

		// draw the whiskers cap
		if (whiskersCapPen.style() != Qt::NoPen && !m_whiskersCapPath.at(i).isEmpty()) {
			painter->setPen(whiskersCapPen);
			painter->setBrush(Qt::NoBrush);
			painter->setOpacity(whiskersCapOpacity);
			painter->drawPath(m_whiskersCapPath.at(i));
		}

		// draw rug
		if (rugEnabled && !m_rugPath.at(i).isEmpty()) {
			QPen pen;
			pen.setColor(borderPen.color());
			pen.setWidthF(rugWidth);
			painter->setPen(pen);
			painter->setOpacity(borderOpacity);
			painter->drawPath(m_rugPath.at(i));
		}

		// draw the symbols
		drawSymbols(painter, i);
	}
}

void BoxPlotPrivate::drawSymbols(QPainter* painter, int index) {
	// outlier values
	symbolOutlier->draw(painter, m_outlierPoints.at(index));

	// mean value
	if (m_meanPointVisible.at(index))
		symbolMean->draw(painter, m_meanPoint.at(index));

	// median value
	if (m_medianPointVisible.at(index))
		symbolMedian->draw(painter, m_medianPoint.at(index));

	// jitter values
	symbolData->draw(painter, m_dataPoints.at(index));

	// far out values
	symbolFarOut->draw(painter, m_farOutPoints.at(index));

	// whisker end points
	symbolWhiskerEnd->draw(painter, m_whiskerEndPoints.at(index));
}

void BoxPlotPrivate::drawFilling(QPainter* painter, int index) {
	PERFTRACE(name() + Q_FUNC_INFO);

	const QPolygonF& polygon = m_fillPolygon.at(index);
	const QRectF& rect = polygon.boundingRect();

	if (background->type() == Background::Type::Color) {
		switch (background->colorStyle()) {
		case Background::ColorStyle::SingleColor: {
			painter->setBrush(QBrush(background->firstColor()));
			break;
		}
		case Background::ColorStyle::HorizontalLinearGradient: {
			QLinearGradient linearGrad(rect.topLeft(), rect.topRight());
			linearGrad.setColorAt(0, background->firstColor());
			linearGrad.setColorAt(1, background->secondColor());
			painter->setBrush(QBrush(linearGrad));
			break;
		}
		case Background::ColorStyle::VerticalLinearGradient: {
			QLinearGradient linearGrad(rect.topLeft(), rect.bottomLeft());
			linearGrad.setColorAt(0, background->firstColor());
			linearGrad.setColorAt(1, background->secondColor());
			painter->setBrush(QBrush(linearGrad));
			break;
		}
		case Background::ColorStyle::TopLeftDiagonalLinearGradient: {
			QLinearGradient linearGrad(rect.topLeft(), rect.bottomRight());
			linearGrad.setColorAt(0, background->firstColor());
			linearGrad.setColorAt(1, background->secondColor());
			painter->setBrush(QBrush(linearGrad));
			break;
		}
		case Background::ColorStyle::BottomLeftDiagonalLinearGradient: {
			QLinearGradient linearGrad(rect.bottomLeft(), rect.topRight());
			linearGrad.setColorAt(0, background->firstColor());
			linearGrad.setColorAt(1, background->secondColor());
			painter->setBrush(QBrush(linearGrad));
			break;
		}
		case Background::ColorStyle::RadialGradient: {
			QRadialGradient radialGrad(rect.center(), rect.width() / 2);
			radialGrad.setColorAt(0, background->firstColor());
			radialGrad.setColorAt(1, background->secondColor());
			painter->setBrush(QBrush(radialGrad));
			break;
		}
		}
	} else if (background->type() == Background::Type::Image) {
		if (!background->fileName().trimmed().isEmpty()) {
			QPixmap pix(background->fileName());
			switch (background->imageStyle()) {
			case Background::ImageStyle::ScaledCropped:
				pix = pix.scaled(rect.size().toSize(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
				painter->setBrush(QBrush(pix));
				painter->setBrushOrigin(pix.size().width() / 2, pix.size().height() / 2);
				break;
			case Background::ImageStyle::Scaled:
				pix = pix.scaled(rect.size().toSize(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
				painter->setBrush(QBrush(pix));
				painter->setBrushOrigin(pix.size().width() / 2, pix.size().height() / 2);
				break;
			case Background::ImageStyle::ScaledAspectRatio:
				pix = pix.scaled(rect.size().toSize(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
				painter->setBrush(QBrush(pix));
				painter->setBrushOrigin(pix.size().width() / 2, pix.size().height() / 2);
				break;
			case Background::ImageStyle::Centered: {
				QPixmap backpix(rect.size().toSize());
				backpix.fill();
				QPainter p(&backpix);
				p.drawPixmap(QPointF(0, 0), pix);
				p.end();
				painter->setBrush(QBrush(backpix));
				painter->setBrushOrigin(-pix.size().width() / 2, -pix.size().height() / 2);
				break;
			}
			case Background::ImageStyle::Tiled:
				painter->setBrush(QBrush(pix));
				break;
			case Background::ImageStyle::CenterTiled:
				painter->setBrush(QBrush(pix));
				painter->setBrushOrigin(pix.size().width() / 2, pix.size().height() / 2);
			}
		}
	} else if (background->type() == Background::Type::Pattern)
		painter->setBrush(QBrush(background->firstColor(), background->brushStyle()));

	painter->drawPolygon(polygon);
}

void BoxPlotPrivate::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget*) {
	if (!isVisible())
		return;

	painter->setPen(Qt::NoPen);
	painter->setBrush(Qt::NoBrush);
	painter->setRenderHint(QPainter::SmoothPixmapTransform, true);

	if (KSharedConfig::openConfig()->group("Settings_Worksheet").readEntry<bool>("DoubleBuffering", true))
		painter->drawPixmap(m_boundingRectangle.topLeft(), m_pixmap); // draw the cached pixmap (fast)
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

	// general
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

	// box filling
	d->background->save(writer);

	// box border
	writer->writeStartElement("border");
	WRITE_QPEN(d->borderPen);
	writer->writeAttribute("opacity", QString::number(d->borderOpacity));
	writer->writeEndElement();

	// median line
	writer->writeStartElement("medianLine");
	WRITE_QPEN(d->medianLinePen);
	writer->writeAttribute("opacity", QString::number(d->medianLineOpacity));
	writer->writeEndElement();

	// symbols for the outliers, mean, far out and jitter values
	d->symbolMean->save(writer);
	d->symbolMedian->save(writer);
	d->symbolOutlier->save(writer);
	d->symbolFarOut->save(writer);
	d->symbolData->save(writer);
	d->symbolWhiskerEnd->save(writer);

	// whiskers
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

	// margin plots
	writer->writeStartElement("margins");
	writer->writeAttribute("rugEnabled", QString::number(d->rugEnabled));
	writer->writeAttribute("rugLength", QString::number(d->rugLength));
	writer->writeAttribute("rugWidth", QString::number(d->rugWidth));
	writer->writeAttribute("rugOffset", QString::number(d->rugOffset));
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
			if (!readCommentElement(reader))
				return false;
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
		} else if (!preview && reader->name() == "filling")
			d->background->load(reader, preview);
		else if (!preview && reader->name() == "border") {
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
		else if (!preview && reader->name() == "symbolWhiskerEnd")
			d->symbolWhiskerEnd->load(reader, preview);
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
		} else if (!preview && reader->name() == "margins") {
			attribs = reader->attributes();

			READ_INT_VALUE("rugEnabled", rugEnabled, bool);
			READ_DOUBLE_VALUE("rugLength", rugLength);
			READ_DOUBLE_VALUE("rugWidth", rugWidth);
			READ_DOUBLE_VALUE("rugOffset", rugOffset);
		} else { // unknown element
			reader->raiseWarning(i18n("unknown element '%1'", reader->name().toString()));
			if (!reader->skipToEndElement())
				return false;
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
		group = config.group("XYCurve"); // when loading from the theme config, use the same properties as for XYCurve
	else
		group = config.group("BoxPlot");

	const auto* plot = static_cast<const CartesianPlot*>(parentAspect());
	int index = plot->curveChildIndex(this);
	const QColor themeColor = plot->themeColorPalette(index);

	QPen p;

	Q_D(BoxPlot);
	d->m_suppressRecalc = false;

	// box border
	p.setStyle((Qt::PenStyle)group.readEntry("LineStyle", (int)Qt::SolidLine));
	p.setWidthF(group.readEntry("LineWidth", Worksheet::convertToSceneUnits(1.0, Worksheet::Unit::Point)));
	p.setColor(themeColor);
	setBorderPen(p);
	setBorderOpacity(group.readEntry("LineOpacity", 1.0));

	// box filling
	d->background->loadThemeConfig(group);
	d->background->setFirstColor(themeColor);

	// median line
	setMedianLinePen(p);
	setMedianLineOpacity(group.readEntry("LineOpacity", 1.0));

	// whiskers
	setWhiskersPen(p);
	setWhiskersOpacity(group.readEntry("LineOpacity", 1.0));
	setWhiskersCapPen(p);
	setWhiskersCapOpacity(group.readEntry("LineOpacity", 1.0));

	// symbols
	d->symbolMean->loadThemeConfig(group, themeColor);
	d->symbolMedian->loadThemeConfig(group, themeColor);
	d->symbolOutlier->loadThemeConfig(group, themeColor);
	d->symbolFarOut->loadThemeConfig(group, themeColor);
	d->symbolData->loadThemeConfig(group, themeColor);

	// Tufte's theme goes beyond what we can implement when using the theme properties of XYCurve.
	// So, instead of introducing a dedicated section for BoxPlot, which would be a big overkill
	// for all other themes, we add here a special handling for "Tufte".
	if (plot->theme() == QLatin1String("Tufte")) {
		p.setStyle(Qt::NoPen);
		setBorderPen(p);
		setMedianLinePen(p);
		d->background->setEnabled(false);
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
