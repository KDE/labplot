/*
	File                 : CartesianPlotLegend.cpp
	Project              : LabPlot
	Description          : Legend for the cartesian plot
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2013-2022 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

/*!
  \class CartesianPlotLegend
  \brief Legend for the cartesian plot.

  \ingroup kdefrontend
*/
#include "CartesianPlotLegend.h"
#include "backend/core/AbstractColumn.h"
#include "backend/core/Project.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/commandtemplates.h"
#include "backend/worksheet/Background.h"
#include "backend/worksheet/Line.h"
#include "backend/worksheet/TextLabel.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/BarPlot.h"
#include "backend/worksheet/plots/cartesian/BoxPlot.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/worksheet/plots/cartesian/CartesianPlotLegendPrivate.h"
#include "backend/worksheet/plots/cartesian/Histogram.h"
#include "backend/worksheet/plots/cartesian/Symbol.h"
#include "backend/worksheet/plots/cartesian/XYCurve.h"

#include <QGraphicsSceneContextMenuEvent>
#include <QKeyEvent>
#include <QMenu>
#include <QPainter>
#include <QPainterPath>

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>

CartesianPlotLegend::CartesianPlotLegend(const QString& name)
	: WorksheetElement(name, new CartesianPlotLegendPrivate(this), AspectType::CartesianPlotLegend) {
	init();
}

CartesianPlotLegend::CartesianPlotLegend(const QString& name, CartesianPlotLegendPrivate* dd)
	: WorksheetElement(name, dd, AspectType::CartesianPlotLegend) {
	init();
}

void CartesianPlotLegend::finalizeAdd() {
	Q_D(CartesianPlotLegend);
	d->plot = static_cast<const CartesianPlot*>(parentAspect());
}

// no need to delete the d-pointer here - it inherits from QGraphicsItem
// and is deleted during the cleanup in QGraphicsScene
CartesianPlotLegend::~CartesianPlotLegend() = default;

void CartesianPlotLegend::init() {
	Q_D(CartesianPlotLegend);

	KConfig config;
	KConfigGroup group = config.group("CartesianPlotLegend");

	d->labelFont = group.readEntry("LabelsFont", QFont());
	d->labelFont.setPixelSize(Worksheet::convertToSceneUnits(10, Worksheet::Unit::Point));

	d->labelColor = group.readEntry("FontColor", QColor(Qt::black));
	d->labelColumnMajor = true;
	d->lineSymbolWidth = group.readEntry("LineSymbolWidth", Worksheet::convertToSceneUnits(1, Worksheet::Unit::Centimeter));
	d->rowCount = 0;
	d->columnCount = 0;

	d->position.horizontalPosition = WorksheetElement::HorizontalPosition::Right;
	d->position.verticalPosition = WorksheetElement::VerticalPosition::Top;
	d->horizontalAlignment = WorksheetElement::HorizontalAlignment::Right;
	d->verticalAlignment = WorksheetElement::VerticalAlignment::Top;
	d->position.point = QPointF(0, 0);

	d->rotationAngle = group.readEntry("Rotation", 0.0);

	// Title
	d->title = new TextLabel(this->name(), TextLabel::Type::PlotLegendTitle);
	d->title->setBorderShape(TextLabel::BorderShape::NoBorder);
	addChild(d->title);
	d->title->setHidden(true);
	d->title->setParentGraphicsItem(d);
	d->title->graphicsItem()->setFlag(QGraphicsItem::ItemIsMovable, false);
	d->title->graphicsItem()->setFlag(QGraphicsItem::ItemIsFocusable, false);
	connect(d->title, &TextLabel::changed, this, &CartesianPlotLegend::retransform);

	// Background
	d->background = new Background(QString());
	addChild(d->background);
	d->background->setHidden(true);
	d->background->init(group);
	connect(d->background, &Background::updateRequested, [=] {
		d->update();
	});

	// Border
	d->borderPen = QPen(group.readEntry("BorderColor", QColor(Qt::black)),
						group.readEntry("BorderWidth", Worksheet::convertToSceneUnits(1.0, Worksheet::Unit::Point)),
						(Qt::PenStyle)group.readEntry("BorderStyle", (int)Qt::SolidLine));
	d->borderCornerRadius = group.readEntry("BorderCornerRadius", 0.0);
	d->borderOpacity = group.readEntry("BorderOpacity", 1.0);

	// Layout
	d->layoutTopMargin = group.readEntry("LayoutTopMargin", Worksheet::convertToSceneUnits(0.2f, Worksheet::Unit::Centimeter));
	d->layoutBottomMargin = group.readEntry("LayoutBottomMargin", Worksheet::convertToSceneUnits(0.2f, Worksheet::Unit::Centimeter));
	d->layoutLeftMargin = group.readEntry("LayoutLeftMargin", Worksheet::convertToSceneUnits(0.2f, Worksheet::Unit::Centimeter));
	d->layoutRightMargin = group.readEntry("LayoutRightMargin", Worksheet::convertToSceneUnits(0.2f, Worksheet::Unit::Centimeter));
	d->layoutVerticalSpacing = group.readEntry("LayoutVerticalSpacing", Worksheet::convertToSceneUnits(0.1f, Worksheet::Unit::Centimeter));
	d->layoutHorizontalSpacing = group.readEntry("LayoutHorizontalSpacing", Worksheet::convertToSceneUnits(0.1f, Worksheet::Unit::Centimeter));
	d->layoutColumnCount = group.readEntry("LayoutColumnCount", 1);

	this->initActions();
}

void CartesianPlotLegend::initActions() {
	visibilityAction = new QAction(QIcon::fromTheme(QStringLiteral("view-visible")), i18n("Visible"), this);
	visibilityAction->setCheckable(true);
	connect(visibilityAction, &QAction::triggered, this, &CartesianPlotLegend::visibilityChangedSlot);
}

QMenu* CartesianPlotLegend::createContextMenu() {
	QMenu* menu = WorksheetElement::createContextMenu();
	QAction* firstAction = menu->actions().at(1); // skip the first action because of the "title-action"

	visibilityAction->setChecked(isVisible());
	menu->insertAction(firstAction, visibilityAction);
	menu->insertSeparator(firstAction);

	return menu;
}

/*!
	Returns an icon to be used in the project explorer.
*/
QIcon CartesianPlotLegend::icon() const {
	return QIcon::fromTheme(QStringLiteral("text-field"));
}

QGraphicsItem* CartesianPlotLegend::graphicsItem() const {
	return d_ptr;
}

void CartesianPlotLegend::retransform() {
	d_ptr->retransform();
}

/*!
 * overrides the implementation in WorksheetElement and sets the z-value to the maximal possible,
 * legends are drawn on top of all other object in the plot.
 */
void CartesianPlotLegend::setZValue(qreal) {
	Q_D(CartesianPlotLegend);
	d->setZValue(std::numeric_limits<double>::max());
}

void CartesianPlotLegend::handleResize(double /*horizontalRatio*/, double /*verticalRatio*/, bool /*pageResize*/) {
	// TODO
	// 	Q_D(const CartesianPlotLegend);
}

//##############################################################################
//################################  getter methods  ############################
//##############################################################################
BASIC_SHARED_D_READER_IMPL(CartesianPlotLegend, QFont, labelFont, labelFont)
BASIC_SHARED_D_READER_IMPL(CartesianPlotLegend, QColor, labelColor, labelColor)
BASIC_SHARED_D_READER_IMPL(CartesianPlotLegend, bool, labelColumnMajor, labelColumnMajor)
BASIC_SHARED_D_READER_IMPL(CartesianPlotLegend, float, lineSymbolWidth, lineSymbolWidth)

// Title
TextLabel* CartesianPlotLegend::title() {
	D(CartesianPlotLegend);
	return d->title;
}

// background
Background* CartesianPlotLegend::background() const {
	Q_D(const CartesianPlotLegend);
	return d->background;
}

// Border
BASIC_SHARED_D_READER_IMPL(CartesianPlotLegend, QPen, borderPen, borderPen)
BASIC_SHARED_D_READER_IMPL(CartesianPlotLegend, float, borderCornerRadius, borderCornerRadius)
BASIC_SHARED_D_READER_IMPL(CartesianPlotLegend, float, borderOpacity, borderOpacity)

// Layout
BASIC_SHARED_D_READER_IMPL(CartesianPlotLegend, float, layoutTopMargin, layoutTopMargin)
BASIC_SHARED_D_READER_IMPL(CartesianPlotLegend, float, layoutBottomMargin, layoutBottomMargin)
BASIC_SHARED_D_READER_IMPL(CartesianPlotLegend, float, layoutLeftMargin, layoutLeftMargin)
BASIC_SHARED_D_READER_IMPL(CartesianPlotLegend, float, layoutRightMargin, layoutRightMargin)
BASIC_SHARED_D_READER_IMPL(CartesianPlotLegend, float, layoutHorizontalSpacing, layoutHorizontalSpacing)
BASIC_SHARED_D_READER_IMPL(CartesianPlotLegend, float, layoutVerticalSpacing, layoutVerticalSpacing)
BASIC_SHARED_D_READER_IMPL(CartesianPlotLegend, int, layoutColumnCount, layoutColumnCount)

//##############################################################################
//######################  setter methods and undo commands  ####################
//##############################################################################
STD_SETTER_CMD_IMPL_F_S(CartesianPlotLegend, SetLabelFont, QFont, labelFont, retransform)
void CartesianPlotLegend::setLabelFont(const QFont& font) {
	Q_D(CartesianPlotLegend);
	if (font != d->labelFont)
		exec(new CartesianPlotLegendSetLabelFontCmd(d, font, ki18n("%1: set font")));
}

STD_SETTER_CMD_IMPL_F_S(CartesianPlotLegend, SetLabelColor, QColor, labelColor, update)
void CartesianPlotLegend::setLabelColor(const QColor& color) {
	Q_D(CartesianPlotLegend);
	if (color != d->labelColor)
		exec(new CartesianPlotLegendSetLabelColorCmd(d, color, ki18n("%1: set font color")));
}

STD_SETTER_CMD_IMPL_F_S(CartesianPlotLegend, SetLabelColumnMajor, bool, labelColumnMajor, retransform)
void CartesianPlotLegend::setLabelColumnMajor(bool columnMajor) {
	Q_D(CartesianPlotLegend);
	if (columnMajor != d->labelColumnMajor)
		exec(new CartesianPlotLegendSetLabelColumnMajorCmd(d, columnMajor, ki18n("%1: change column order")));
}

STD_SETTER_CMD_IMPL_F_S(CartesianPlotLegend, SetLineSymbolWidth, float, lineSymbolWidth, retransform)
void CartesianPlotLegend::setLineSymbolWidth(float width) {
	Q_D(CartesianPlotLegend);
	if (width != d->lineSymbolWidth)
		exec(new CartesianPlotLegendSetLineSymbolWidthCmd(d, width, ki18n("%1: change line+symbol width")));
}

// Border
STD_SETTER_CMD_IMPL_F_S(CartesianPlotLegend, SetBorderPen, QPen, borderPen, update)
void CartesianPlotLegend::setBorderPen(const QPen& pen) {
	Q_D(CartesianPlotLegend);
	if (pen != d->borderPen)
		exec(new CartesianPlotLegendSetBorderPenCmd(d, pen, ki18n("%1: set border style")));
}

STD_SETTER_CMD_IMPL_F_S(CartesianPlotLegend, SetBorderCornerRadius, qreal, borderCornerRadius, update)
void CartesianPlotLegend::setBorderCornerRadius(float radius) {
	Q_D(CartesianPlotLegend);
	if (radius != d->borderCornerRadius)
		exec(new CartesianPlotLegendSetBorderCornerRadiusCmd(d, radius, ki18n("%1: set border corner radius")));
}

STD_SETTER_CMD_IMPL_F_S(CartesianPlotLegend, SetBorderOpacity, qreal, borderOpacity, update)
void CartesianPlotLegend::setBorderOpacity(float opacity) {
	Q_D(CartesianPlotLegend);
	if (opacity != d->borderOpacity)
		exec(new CartesianPlotLegendSetBorderOpacityCmd(d, opacity, ki18n("%1: set border opacity")));
}

// Layout
STD_SETTER_CMD_IMPL_F_S(CartesianPlotLegend, SetLayoutTopMargin, float, layoutTopMargin, retransform)
void CartesianPlotLegend::setLayoutTopMargin(float margin) {
	Q_D(CartesianPlotLegend);
	if (margin != d->layoutTopMargin)
		exec(new CartesianPlotLegendSetLayoutTopMarginCmd(d, margin, ki18n("%1: set layout top margin")));
}

STD_SETTER_CMD_IMPL_F_S(CartesianPlotLegend, SetLayoutBottomMargin, float, layoutBottomMargin, retransform)
void CartesianPlotLegend::setLayoutBottomMargin(float margin) {
	Q_D(CartesianPlotLegend);
	if (margin != d->layoutBottomMargin)
		exec(new CartesianPlotLegendSetLayoutBottomMarginCmd(d, margin, ki18n("%1: set layout bottom margin")));
}

STD_SETTER_CMD_IMPL_F_S(CartesianPlotLegend, SetLayoutLeftMargin, float, layoutLeftMargin, retransform)
void CartesianPlotLegend::setLayoutLeftMargin(float margin) {
	Q_D(CartesianPlotLegend);
	if (margin != d->layoutLeftMargin)
		exec(new CartesianPlotLegendSetLayoutLeftMarginCmd(d, margin, ki18n("%1: set layout left margin")));
}

STD_SETTER_CMD_IMPL_F_S(CartesianPlotLegend, SetLayoutRightMargin, float, layoutRightMargin, retransform)
void CartesianPlotLegend::setLayoutRightMargin(float margin) {
	Q_D(CartesianPlotLegend);
	if (margin != d->layoutRightMargin)
		exec(new CartesianPlotLegendSetLayoutRightMarginCmd(d, margin, ki18n("%1: set layout right margin")));
}

STD_SETTER_CMD_IMPL_F_S(CartesianPlotLegend, SetLayoutVerticalSpacing, float, layoutVerticalSpacing, retransform)
void CartesianPlotLegend::setLayoutVerticalSpacing(float spacing) {
	Q_D(CartesianPlotLegend);
	if (spacing != d->layoutVerticalSpacing)
		exec(new CartesianPlotLegendSetLayoutVerticalSpacingCmd(d, spacing, ki18n("%1: set layout vertical spacing")));
}

STD_SETTER_CMD_IMPL_F_S(CartesianPlotLegend, SetLayoutHorizontalSpacing, float, layoutHorizontalSpacing, retransform)
void CartesianPlotLegend::setLayoutHorizontalSpacing(float spacing) {
	Q_D(CartesianPlotLegend);
	if (spacing != d->layoutHorizontalSpacing)
		exec(new CartesianPlotLegendSetLayoutHorizontalSpacingCmd(d, spacing, ki18n("%1: set layout horizontal spacing")));
}

STD_SETTER_CMD_IMPL_F_S(CartesianPlotLegend, SetLayoutColumnCount, int, layoutColumnCount, retransform)
void CartesianPlotLegend::setLayoutColumnCount(int count) {
	Q_D(CartesianPlotLegend);
	if (count != d->layoutColumnCount)
		exec(new CartesianPlotLegendSetLayoutColumnCountCmd(d, count, ki18n("%1: set layout column count")));
}

//##############################################################################
//#################################  SLOTS  ####################################
//##############################################################################

//##############################################################################
//######  SLOTs for changes triggered via QActions in the context menu  ########
//##############################################################################
void CartesianPlotLegend::visibilityChangedSlot() {
	Q_D(const CartesianPlotLegend);
	this->setVisible(!d->isVisible());
}

//##############################################################################
//######################### Private implementation #############################
//##############################################################################
CartesianPlotLegendPrivate::CartesianPlotLegendPrivate(CartesianPlotLegend* owner)
	: WorksheetElementPrivate(owner)
	, q(owner) {
	setFlag(QGraphicsItem::ItemIsSelectable, true);
	setFlag(QGraphicsItem::ItemIsMovable);
	setFlag(QGraphicsItem::ItemSendsGeometryChanges);
	setFlag(QGraphicsItem::ItemIsFocusable);
	setAcceptHoverEvents(true);
}

QRectF CartesianPlotLegendPrivate::boundingRect() const {
	if (rotationAngle != 0) {
		QMatrix matrix;
		matrix.rotate(-rotationAngle);
		return matrix.mapRect(rect);
	} else
		return rect;
}

void CartesianPlotLegendPrivate::contextMenuEvent(QGraphicsSceneContextMenuEvent* event) {
	q->createContextMenu()->exec(event->screenPos());
}

/*!
  Returns the shape of the CartesianPlotLegend as a QPainterPath in local coordinates
*/
QPainterPath CartesianPlotLegendPrivate::shape() const {
	QPainterPath path;
	if (qFuzzyIsNull(borderCornerRadius))
		path.addRect(rect);
	else
		path.addRoundedRect(rect, borderCornerRadius, borderCornerRadius);

	if (rotationAngle != 0) {
		QTransform trafo;
		trafo.rotate(-rotationAngle);
		path = trafo.map(path);
	}

	return path;
}

void CartesianPlotLegendPrivate::recalcShapeAndBoundingRect() {
	retransform();
}

/*!
  recalculates the rectangular of the legend.
*/
void CartesianPlotLegendPrivate::retransform() {
	const bool suppress = suppressRetransform || !plot || q->isLoading();
	trackRetransformCalled(suppress);

	// Assert cannot be used, because the Textlabel sends the
	// changed signal during load and so a retransform is triggered
	// assert(!q->isLoading());
	if (suppress)
		return;

	prepareGeometryChange();

	m_curves.clear();
	m_names.clear();

	const auto& children = plot->children<WorksheetElement>();
	for (auto* child : children) {
		auto* curve = dynamic_cast<XYCurve*>(child);
		if (curve && curve->isVisible() && curve->legendVisible()) {
			m_curves << curve;
			m_names << curve->name();
			continue;
		}

		if ((child->type() == AspectType::Histogram || child->type() == AspectType::BoxPlot) && child->isVisible()) {
			m_curves << child;
			m_names << child->name();
			continue;
		}

		auto* barPlot = dynamic_cast<BarPlot*>(child);
		if (barPlot && barPlot->isVisible()) {
			m_curves << barPlot;
			const auto& columns = barPlot->dataColumns();
			for (auto* column : columns)
				m_names << column->name();
		}
	}

	int namesCount = m_names.count();
	columnCount = (namesCount < layoutColumnCount) ? namesCount : layoutColumnCount;
	if (columnCount == 0) // no curves available
		rowCount = 0;
	else
		rowCount = ceil(double(namesCount) / double(columnCount));

	maxColumnTextWidths.clear();

	// determine the width of the legend
	QFontMetrics fm(labelFont);
	float w;
	float h = fm.ascent();

	float legendWidth = 0;

	int index;
	for (int c = 0; c < columnCount; ++c) {
		float maxTextWidth = 0;
		for (int r = 0; r < rowCount; ++r) {
			if (labelColumnMajor)
				index = c * rowCount + r;
			else
				index = r * columnCount + c;

			if (index >= namesCount)
				break;

			w = fm.boundingRect(m_names.at(index)).width();
			if (w > maxTextWidth)
				maxTextWidth = w;
		}
		maxColumnTextWidths.append(maxTextWidth);
		legendWidth += maxTextWidth;
	}

	legendWidth += layoutLeftMargin + layoutRightMargin; // margins
	legendWidth += columnCount * (lineSymbolWidth + layoutHorizontalSpacing); // width of the columns without the text
	legendWidth += (columnCount - 1) * 2 * layoutHorizontalSpacing; // spacings between the columns

	// add title width if title is available
	if (title->isVisible() && !title->text().text.isEmpty()) {
		float titleWidth;
		if (rotationAngle == 0.0)
			titleWidth = title->graphicsItem()->boundingRect().width();
		else {
			QRectF rect = title->graphicsItem()->boundingRect();
			QMatrix matrix;
			matrix.rotate(-rotationAngle);
			rect = matrix.mapRect(rect);
			titleWidth = rect.width();
		}

		if (titleWidth > legendWidth)
			legendWidth = titleWidth;
	}

	// determine the height of the legend
	float legendHeight = layoutTopMargin + layoutBottomMargin; // margins
	legendHeight += rowCount * h; // height of the rows
	legendHeight += (rowCount - 1) * layoutVerticalSpacing; // spacing between the rows
	if (title->isVisible() && !title->text().text.isEmpty()) {
		if (rotationAngle == 0.0)
			legendHeight += title->graphicsItem()->boundingRect().height(); // legend title
		else {
			QRectF rect = title->graphicsItem()->boundingRect();
			QMatrix matrix;
			matrix.rotate(-rotationAngle);
			rect = matrix.mapRect(rect);
			legendHeight += rect.height(); // legend title
		}
	}

	rect.setX(-legendWidth / 2);
	rect.setY(-legendHeight / 2);
	rect.setWidth(legendWidth);
	rect.setHeight(legendHeight);

	updatePosition();
}

/*!
	calculates the position of the legend, when the position relative to the parent was specified (left, right, etc.)
*/
void CartesianPlotLegendPrivate::updatePosition() {
	WorksheetElementPrivate::updatePosition();

	suppressRetransform = true;
	title->retransform();
	suppressRetransform = false;
}

/*!
  Reimplementation of QGraphicsItem::paint(). This function does the actual painting of the legend.
  \sa QGraphicsItem::paint().
*/
void CartesianPlotLegendPrivate::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget*) {
	if (!isVisible())
		return;

	painter->save();
	painter->rotate(-rotationAngle);

	// draw the area
	painter->setOpacity(background->opacity());
	painter->setPen(Qt::NoPen);
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
				painter->drawPixmap(rect.topLeft(), pix);
				break;
			case Background::ImageStyle::Scaled:
				pix = pix.scaled(rect.size().toSize(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
				painter->drawPixmap(rect.topLeft(), pix);
				break;
			case Background::ImageStyle::ScaledAspectRatio:
				pix = pix.scaled(rect.size().toSize(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
				painter->drawPixmap(rect.topLeft(), pix);
				break;
			case Background::ImageStyle::Centered:
				painter->drawPixmap(QPointF(rect.center().x() - pix.size().width() / 2, rect.center().y() - pix.size().height() / 2), pix);
				break;
			case Background::ImageStyle::Tiled:
				painter->drawTiledPixmap(rect, pix);
				break;
			case Background::ImageStyle::CenterTiled:
				painter->drawTiledPixmap(rect, pix, QPoint(rect.size().width() / 2, rect.size().height() / 2));
			}
		}
	} else if (background->type() == Background::Type::Pattern) {
		painter->setBrush(QBrush(background->firstColor(), background->brushStyle()));
	}

	if (qFuzzyIsNull(borderCornerRadius))
		painter->drawRect(rect);
	else
		painter->drawRoundedRect(rect, borderCornerRadius, borderCornerRadius);

	// draw the border
	if (borderPen.style() != Qt::NoPen) {
		painter->setPen(borderPen);
		painter->setBrush(Qt::NoBrush);
		painter->setOpacity(borderOpacity);
		if (qFuzzyIsNull(borderCornerRadius))
			painter->drawRect(rect);
		else
			painter->drawRoundedRect(rect, borderCornerRadius, borderCornerRadius);
	}

	// draw curve's line+symbol and the names
	QFontMetrics fm(labelFont);
	float h = fm.ascent();
	painter->setFont(labelFont);

	// translate to left upper corner of the bounding rect plus the layout offset and the height of the title
	painter->translate(-rect.width() / 2 + layoutLeftMargin, -rect.height() / 2 + layoutTopMargin);
	if (title->isVisible() && !title->text().text.isEmpty())
		painter->translate(0, title->graphicsItem()->boundingRect().height());

	painter->save();

	int col = 0;
	int row = 0;
	for (auto* child : m_curves) {
		// process the curves
		const auto* curve = dynamic_cast<const XYCurve*>(child);
		const auto* hist = dynamic_cast<const Histogram*>(child);
		const auto* boxPlot = dynamic_cast<const BoxPlot*>(child);
		const auto* barPlot = dynamic_cast<const BarPlot*>(child);

		if (curve) { // draw the legend item for xy-curve
			// curve's line (painted at the half of the ascent size)
			if (curve->lineType() != XYCurve::LineType::NoLine) {
				painter->setPen(curve->line()->pen());
				painter->setOpacity(curve->line()->opacity());
				painter->drawLine(0, h / 2, lineSymbolWidth, h / 2);
			}

			// error bars
			if ((curve->xErrorType() != XYCurve::ErrorType::NoError && curve->xErrorPlusColumn())
				|| (curve->yErrorType() != XYCurve::ErrorType::NoError && curve->yErrorPlusColumn())) {
				painter->setOpacity(curve->errorBarsLine()->opacity());
				painter->setPen(curve->errorBarsLine()->pen());

				// curve's error bars for x
				float errorBarsSize = Worksheet::convertToSceneUnits(10, Worksheet::Unit::Point);
				if (curve->symbol()->style() != Symbol::Style::NoSymbols && errorBarsSize < curve->symbol()->size() * 1.4)
					errorBarsSize = curve->symbol()->size() * 1.4;

				switch (curve->errorBarsLine()->errorBarsType()) {
				case XYCurve::ErrorBarsType::Simple:
					// horiz. line
					if (curve->xErrorType() != XYCurve::ErrorType::NoError)
						painter->drawLine(lineSymbolWidth / 2 - errorBarsSize / 2, h / 2, lineSymbolWidth / 2 + errorBarsSize / 2, h / 2);
					// vert. line
					if (curve->yErrorType() != XYCurve::ErrorType::NoError)
						painter->drawLine(lineSymbolWidth / 2, h / 2 - errorBarsSize / 2, lineSymbolWidth / 2, h / 2 + errorBarsSize / 2);
					break;
				case XYCurve::ErrorBarsType::WithEnds:
					// horiz. line
					if (curve->xErrorType() != XYCurve::ErrorType::NoError) {
						painter->drawLine(lineSymbolWidth / 2 - errorBarsSize / 2, h / 2, lineSymbolWidth / 2 + errorBarsSize / 2, h / 2);

						// caps for the horiz. line
						painter->drawLine(lineSymbolWidth / 2 - errorBarsSize / 2,
										  h / 2 - errorBarsSize / 4,
										  lineSymbolWidth / 2 - errorBarsSize / 2,
										  h / 2 + errorBarsSize / 4);
						painter->drawLine(lineSymbolWidth / 2 + errorBarsSize / 2,
										  h / 2 - errorBarsSize / 4,
										  lineSymbolWidth / 2 + errorBarsSize / 2,
										  h / 2 + errorBarsSize / 4);
					}

					// vert. line
					if (curve->yErrorType() != XYCurve::ErrorType::NoError) {
						painter->drawLine(lineSymbolWidth / 2, h / 2 - errorBarsSize / 2, lineSymbolWidth / 2, h / 2 + errorBarsSize / 2);

						// caps for the vert. line
						painter->drawLine(lineSymbolWidth / 2 - errorBarsSize / 4,
										  h / 2 - errorBarsSize / 2,
										  lineSymbolWidth / 2 + errorBarsSize / 4,
										  h / 2 - errorBarsSize / 2);
						painter->drawLine(lineSymbolWidth / 2 - errorBarsSize / 4,
										  h / 2 + errorBarsSize / 2,
										  lineSymbolWidth / 2 + errorBarsSize / 4,
										  h / 2 + errorBarsSize / 2);
					}
					break;
				}
			}

			// curve's symbol
			const auto* symbol = curve->symbol();
			if (symbol->style() != Symbol::Style::NoSymbols) {
				painter->setOpacity(symbol->opacity());
				painter->setBrush(symbol->brush());
				painter->setPen(symbol->pen());

				QPainterPath path = Symbol::stylePath(symbol->style());
				QTransform trafo;
				trafo.scale(symbol->size(), symbol->size());
				path = trafo.map(path);

				if (symbol->rotationAngle() != 0) {
					trafo.reset();
					trafo.rotate(symbol->rotationAngle());
					path = trafo.map(path);
				}

				painter->translate(QPointF(lineSymbolWidth / 2, h / 2));
				painter->drawPath(path);
				painter->translate(-QPointF(lineSymbolWidth / 2, h / 2));
			}

			// curve's name
			painter->setPen(QPen(labelColor));
			painter->setOpacity(1.0);
			painter->drawText(QPoint(lineSymbolWidth + layoutHorizontalSpacing, h), curve->name());

			if (!translatePainter(painter, row, col, h))
				break;
		} else if (hist) { // draw the legend item for histogram (simple rectangular with the sizes of the ascent)
			// use line's pen (histogram bars, envelope or drop lines) if available,
			if (hist->line()->histogramLineType() != Histogram::NoLine && hist->line()->pen() != Qt::NoPen) {
				painter->setOpacity(hist->line()->opacity());
				painter->setPen(hist->line()->pen());
			}

			// for the brush, use the histogram filling or symbols filling or no brush
			if (hist->background()->enabled())
				painter->setBrush(QBrush(hist->background()->firstColor(), hist->background()->brushStyle()));
			else if (hist->symbol()->style() != Symbol::Style::NoSymbols)
				painter->setBrush(hist->symbol()->brush());
			else
				painter->setBrush(Qt::NoBrush);

			painter->translate(QPointF(lineSymbolWidth / 2, h / 2));
			painter->drawRect(QRectF(-h / 2, -h / 2, h, h));
			painter->translate(-QPointF(lineSymbolWidth / 2, h / 2));

			// curve's name
			painter->setPen(QPen(labelColor));
			painter->setOpacity(1.0);
			painter->drawText(QPoint(lineSymbolWidth + layoutHorizontalSpacing, h), hist->name());

			if (!translatePainter(painter, row, col, h))
				break;
		} else if (boxPlot) { // draw the legend item for box plot (name only at the moment)
			// curve's name
			painter->setPen(QPen(labelColor));
			painter->setOpacity(1.0);
			painter->drawText(QPoint(lineSymbolWidth + layoutHorizontalSpacing, h), boxPlot->name());

			if (!translatePainter(painter, row, col, h))
				break;
		} else if (barPlot) { // draw the legend item for every dataset bar in the bar plot
			const auto& columns = barPlot->dataColumns();
			int index = 0;
			for (auto* column : columns) {
				// draw the bar
				auto* background = barPlot->backgroundAt(index);
				painter->setBrush(QBrush(background->firstColor(), background->brushStyle()));
				painter->translate(QPointF(lineSymbolWidth / 2, h / 2));
				painter->drawRect(QRectF(-h * 0.25, -h / 2, h * 0.5, h));
				painter->translate(-QPointF(lineSymbolWidth / 2, h / 2));

				auto* line = barPlot->lineAt(index);
				painter->setPen(line->pen());
				painter->setOpacity(line->opacity());
				painter->setBrush(Qt::NoBrush);
				painter->translate(QPointF(lineSymbolWidth / 2, h / 2));
				painter->drawRect(QRectF(-h * 0.25, -h / 2, h * 0.5, h));
				painter->translate(-QPointF(lineSymbolWidth / 2, h / 2));

				// draw the name text
				painter->drawText(QPoint(lineSymbolWidth + layoutHorizontalSpacing, h), column->name());
				++index;
				if (!translatePainter(painter, row, col, h))
					break;
			}
		}
	}

	painter->restore();
	painter->restore();

	if (m_hovered && !isSelected() && !q->isPrinting()) {
		painter->setPen(QPen(QApplication::palette().color(QPalette::Shadow), 2, Qt::SolidLine));
		painter->drawPath(shape());
	}

	if (isSelected() && !q->isPrinting()) {
		painter->setPen(QPen(QApplication::palette().color(QPalette::Highlight), 2, Qt::SolidLine));
		painter->drawPath(shape());
	}
}

/*!
 * helper function translating the painter to the next row and column when painting the next legend item in paint().
 */
bool CartesianPlotLegendPrivate::translatePainter(QPainter* painter, int& row, int& col, int height) {
	if (labelColumnMajor) { // column major order
		++row;
		if (row != rowCount)
			painter->translate(0, layoutVerticalSpacing + height);
		else {
			++col;
			if (col == columnCount)
				return false;

			row = 0;
			painter->restore();
			int deltaX = lineSymbolWidth + layoutHorizontalSpacing + maxColumnTextWidths.at(col); // the width of the current columns
			deltaX += 2 * layoutHorizontalSpacing; // spacing between two columns
			painter->translate(deltaX, 0);
			painter->save();
		}
	} else { // row major order
		++col;
		if (col != columnCount) {
			int deltaX = lineSymbolWidth + layoutHorizontalSpacing + maxColumnTextWidths.at(col); // the width of the current columns
			deltaX += 2 * layoutHorizontalSpacing; // spacing between two columns
			painter->translate(deltaX, 0);
		} else {
			++row;
			if (row == rowCount)
				return false;

			painter->restore();
			painter->translate(0, layoutVerticalSpacing + height);
			painter->save();
		}
	}

	return true;
}

void CartesianPlotLegendPrivate::hoverEnterEvent(QGraphicsSceneHoverEvent*) {
	if (!isSelected()) {
		m_hovered = true;
		Q_EMIT q->hovered();
		update();
	}
}

void CartesianPlotLegendPrivate::hoverLeaveEvent(QGraphicsSceneHoverEvent*) {
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
void CartesianPlotLegend::save(QXmlStreamWriter* writer) const {
	Q_D(const CartesianPlotLegend);

	writer->writeStartElement(QStringLiteral("cartesianPlotLegend"));
	writeBasicAttributes(writer);
	writeCommentElement(writer);

	// general
	writer->writeStartElement(QStringLiteral("general"));
	WRITE_QCOLOR(d->labelColor);
	WRITE_QFONT(d->labelFont);
	writer->writeAttribute(QStringLiteral("columnMajor"), QString::number(d->labelColumnMajor));
	writer->writeAttribute(QStringLiteral("lineSymbolWidth"), QString::number(d->lineSymbolWidth));
	writer->writeAttribute(QStringLiteral("visible"), QString::number(d->isVisible()));
	writer->writeEndElement();

	// geometry
	writer->writeStartElement(QStringLiteral("geometry"));
	WorksheetElement::save(writer);
	writer->writeEndElement();

	// title
	d->title->save(writer);

	// background
	d->background->save(writer);

	// border
	writer->writeStartElement(QStringLiteral("border"));
	WRITE_QPEN(d->borderPen);
	writer->writeAttribute(QStringLiteral("borderOpacity"), QString::number(d->borderOpacity));
	writer->writeAttribute(QStringLiteral("borderCornerRadius"), QString::number(d->borderCornerRadius));
	writer->writeEndElement();

	// layout
	writer->writeStartElement(QStringLiteral("layout"));
	writer->writeAttribute(QStringLiteral("topMargin"), QString::number(d->layoutTopMargin));
	writer->writeAttribute(QStringLiteral("bottomMargin"), QString::number(d->layoutBottomMargin));
	writer->writeAttribute(QStringLiteral("leftMargin"), QString::number(d->layoutLeftMargin));
	writer->writeAttribute(QStringLiteral("rightMargin"), QString::number(d->layoutRightMargin));
	writer->writeAttribute(QStringLiteral("verticalSpacing"), QString::number(d->layoutVerticalSpacing));
	writer->writeAttribute(QStringLiteral("horizontalSpacing"), QString::number(d->layoutHorizontalSpacing));
	writer->writeAttribute(QStringLiteral("columnCount"), QString::number(d->layoutColumnCount));
	writer->writeEndElement();

	writer->writeEndElement(); // close "cartesianPlotLegend" section
}

//! Load from XML
bool CartesianPlotLegend::load(XmlStreamReader* reader, bool preview) {
	Q_D(CartesianPlotLegend);

	if (!readBasicAttributes(reader))
		return false;

	KLocalizedString attributeWarning = ki18n("Attribute '%1' missing or empty, default value is used");
	QXmlStreamAttributes attribs;
	QString str;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == QLatin1String("cartesianPlotLegend"))
			break;

		if (!reader->isStartElement())
			continue;

		if (!preview && reader->name() == QLatin1String("comment")) {
			if (!readCommentElement(reader))
				return false;
		} else if (!preview && reader->name() == QLatin1String("general")) {
			attribs = reader->attributes();

			READ_QCOLOR(d->labelColor);
			READ_QFONT(d->labelFont);
			READ_INT_VALUE("columnMajor", labelColumnMajor, int);
			READ_DOUBLE_VALUE("lineSymbolWidth", lineSymbolWidth);

			if (Project::xmlVersion() < 6) {
				str = attribs.value(QStringLiteral("visible")).toString();
				if (str.isEmpty())
					reader->raiseWarning(attributeWarning.subs(QStringLiteral("visible")).toString());
				else
					d->setVisible(str.toInt());
			}
		} else if (!preview && reader->name() == QLatin1String("geometry")) {
			if (Project::xmlVersion() >= 6)
				WorksheetElement::load(reader, preview);
			else {
				// Visible is in "general" before version 6
				// therefore WorksheetElement::load() cannot be used
				attribs = reader->attributes();

				str = attribs.value(QStringLiteral("x")).toString();
				if (str.isEmpty())
					reader->raiseWarning(attributeWarning.subs(QStringLiteral("x")).toString());
				else
					d->position.point.setX(str.toDouble());

				str = attribs.value(QStringLiteral("y")).toString();
				if (str.isEmpty())
					reader->raiseWarning(attributeWarning.subs(QStringLiteral("y")).toString());
				else
					d->position.point.setY(str.toDouble());

				str = attribs.value(QStringLiteral("horizontalPosition")).toString();
				if (str.isEmpty())
					reader->raiseWarning(attributeWarning.subs(QStringLiteral("horizontalPosition")).toString());
				else
					d->position.horizontalPosition = (WorksheetElement::HorizontalPosition)str.toInt();

				str = attribs.value(QStringLiteral("verticalPosition")).toString();
				if (str.isEmpty())
					reader->raiseWarning(attributeWarning.subs(QStringLiteral("verticalPosition")).toString());
				else
					d->position.verticalPosition = (WorksheetElement::VerticalPosition)str.toInt();

				READ_DOUBLE_VALUE("rotation", rotationAngle);
			}
		} else if (reader->name() == QLatin1String("textLabel")) {
			if (!d->title->load(reader, preview)) {
				delete d->title;
				d->title = nullptr;
				return false;
			}
		} else if (!preview && reader->name() == QLatin1String("background"))
			d->background->load(reader, preview);
		else if (!preview && reader->name() == QLatin1String("border")) {
			attribs = reader->attributes();
			READ_QPEN(d->borderPen);
			READ_DOUBLE_VALUE("borderCornerRadius", borderCornerRadius);
			READ_DOUBLE_VALUE("borderOpacity", borderOpacity);
		} else if (!preview && reader->name() == QLatin1String("layout")) {
			attribs = reader->attributes();
			READ_DOUBLE_VALUE("topMargin", layoutTopMargin);
			READ_DOUBLE_VALUE("bottomMargin", layoutBottomMargin);
			READ_DOUBLE_VALUE("leftMargin", layoutLeftMargin);
			READ_DOUBLE_VALUE("rightMargin", layoutRightMargin);
			READ_DOUBLE_VALUE("verticalSpacing", layoutVerticalSpacing);
			READ_DOUBLE_VALUE("horizontalSpacing", layoutHorizontalSpacing);
			READ_INT_VALUE("columnCount", layoutColumnCount, int);
		}
	}

	return true;
}

void CartesianPlotLegend::loadThemeConfig(const KConfig& config) {
	KConfigGroup group;

	// for the font color use the value defined in the theme config for Label
	if (config.hasGroup(QLatin1String("Theme")))
		group = config.group(QLatin1String("Label"));
	else
		group = config.group(QLatin1String("CartesianPlotLegend"));

	this->setLabelColor(group.readEntry("FontColor", QColor(Qt::black)));

	// for other theme dependent settings use the values defined in the theme config for CartesianPlot
	if (config.hasGroup(QLatin1String("Theme")))
		group = config.group("CartesianPlot");

	// background
	background()->loadThemeConfig(group);

	// border
	QPen pen;
	pen.setColor(group.readEntry("BorderColor", QColor(Qt::black)));
	pen.setStyle((Qt::PenStyle)(group.readEntry("BorderStyle", (int)Qt::SolidLine)));
	pen.setWidthF(group.readEntry("BorderWidth", Worksheet::convertToSceneUnits(1.0, Worksheet::Unit::Point)));
	this->setBorderPen(pen);
	this->setBorderCornerRadius(group.readEntry("BorderCornerRadius", 0.0));
	this->setBorderOpacity(group.readEntry("BorderOpacity", 1.0));

	title()->loadThemeConfig(config);
}
