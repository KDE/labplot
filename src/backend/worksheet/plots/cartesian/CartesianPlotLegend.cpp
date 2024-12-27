/*
	File                 : CartesianPlotLegend.cpp
	Project              : LabPlot
	Description          : Legend for the cartesian plot
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2013-2024 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "CartesianPlotLegend.h"
#include "backend/core/AbstractColumn.h"
#include "backend/core/Project.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/commandtemplates.h"
#include "backend/worksheet/Background.h"
#include "backend/worksheet/Line.h"
#include "backend/worksheet/TextLabel.h"
#include "backend/worksheet/plots/cartesian/CartesianPlotLegendPrivate.h"
#include "backend/worksheet/plots/cartesian/ErrorBar.h"
#include "backend/worksheet/plots/cartesian/Symbol.h"
#include "backend/worksheet/plots/cartesian/plots.h"

#include <QMenu>
#include <QPainter>
#include <QPainterPath>

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>

/*!
 * \class CartesianPlotLegend
 * \brief Legend for the cartesian plot.
 *
 * \ingroup CartesianPlotArea
 */
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
	KConfigGroup group = config.group(QStringLiteral("CartesianPlotLegend"));

	d->labelFont = group.readEntry(QStringLiteral("LabelsFont"), QFont());
	d->labelFont.setPointSizeF(Worksheet::convertToSceneUnits(10, Worksheet::Unit::Point));
	d->usePlotColor = group.readEntry(QStringLiteral("UsePlotColor"), true);
	d->labelColor = group.readEntry(QStringLiteral("FontColor"), QColor(Qt::black));
	d->labelColumnMajor = true;
	d->lineSymbolWidth = group.readEntry(QStringLiteral("LineSymbolWidth"), Worksheet::convertToSceneUnits(1, Worksheet::Unit::Centimeter));
	d->rowCount = 0;
	d->columnCount = 0;

	d->position.horizontalPosition = WorksheetElement::HorizontalPosition::Right;
	d->position.verticalPosition = WorksheetElement::VerticalPosition::Top;
	d->horizontalAlignment = WorksheetElement::HorizontalAlignment::Right;
	d->verticalAlignment = WorksheetElement::VerticalAlignment::Top;
	d->position.point = QPointF(0, 0);

	d->setRotation(group.readEntry(QStringLiteral("Rotation"), 0.0));

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
	d->background = new Background(QStringLiteral("background"));
	addChild(d->background);
	d->background->setHidden(true);
	d->background->init(group);
	connect(d->background, &Background::updateRequested, [=] {
		d->update();
	});

	// Border
	d->borderLine = new Line(QStringLiteral("border"));
	d->borderLine->setPrefix(QStringLiteral("Border"));
	d->borderLine->setCreateXmlElement(false);
	d->borderLine->setHidden(true);
	addChild(d->borderLine);
	d->borderLine->init(group);
	connect(d->borderLine, &Line::updatePixmapRequested, [=] {
		d->update();
	});
	connect(d->borderLine, &Line::updateRequested, [=] {
		d->recalcShapeAndBoundingRect();
	});

	d->borderCornerRadius = group.readEntry(QStringLiteral("BorderCornerRadius"), 0.0);

	// Layout
	d->layoutTopMargin = group.readEntry(QStringLiteral("LayoutTopMargin"), Worksheet::convertToSceneUnits(0.2, Worksheet::Unit::Centimeter));
	d->layoutBottomMargin = group.readEntry(QStringLiteral("LayoutBottomMargin"), Worksheet::convertToSceneUnits(0.2, Worksheet::Unit::Centimeter));
	d->layoutLeftMargin = group.readEntry(QStringLiteral("LayoutLeftMargin"), Worksheet::convertToSceneUnits(0.2, Worksheet::Unit::Centimeter));
	d->layoutRightMargin = group.readEntry(QStringLiteral("LayoutRightMargin"), Worksheet::convertToSceneUnits(0.2, Worksheet::Unit::Centimeter));
	d->layoutVerticalSpacing = group.readEntry(QStringLiteral("LayoutVerticalSpacing"), Worksheet::convertToSceneUnits(0.1, Worksheet::Unit::Centimeter));
	d->layoutHorizontalSpacing = group.readEntry(QStringLiteral("LayoutHorizontalSpacing"), Worksheet::convertToSceneUnits(0.1, Worksheet::Unit::Centimeter));
	d->layoutColumnCount = group.readEntry(QStringLiteral("LayoutColumnCount"), 1);

	this->initActions();
}

void CartesianPlotLegend::initActions() {
}

/*!
	Returns an icon to be used in the project explorer.
*/
QIcon CartesianPlotLegend::icon() const {
	return QIcon::fromTheme(QStringLiteral("text-field"));
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

// ##############################################################################
// ################################  getter methods  ############################
// ##############################################################################
BASIC_SHARED_D_READER_IMPL(CartesianPlotLegend, QFont, labelFont, labelFont)
BASIC_SHARED_D_READER_IMPL(CartesianPlotLegend, bool, usePlotColor, usePlotColor)
BASIC_SHARED_D_READER_IMPL(CartesianPlotLegend, QColor, labelColor, labelColor)
BASIC_SHARED_D_READER_IMPL(CartesianPlotLegend, bool, labelColumnMajor, labelColumnMajor)
BASIC_SHARED_D_READER_IMPL(CartesianPlotLegend, qreal, lineSymbolWidth, lineSymbolWidth)

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
Line* CartesianPlotLegend::borderLine() const {
	Q_D(const CartesianPlotLegend);
	return d->borderLine;
}

BASIC_SHARED_D_READER_IMPL(CartesianPlotLegend, qreal, borderCornerRadius, borderCornerRadius)

// Layout
BASIC_SHARED_D_READER_IMPL(CartesianPlotLegend, qreal, layoutTopMargin, layoutTopMargin)
BASIC_SHARED_D_READER_IMPL(CartesianPlotLegend, qreal, layoutBottomMargin, layoutBottomMargin)
BASIC_SHARED_D_READER_IMPL(CartesianPlotLegend, qreal, layoutLeftMargin, layoutLeftMargin)
BASIC_SHARED_D_READER_IMPL(CartesianPlotLegend, qreal, layoutRightMargin, layoutRightMargin)
BASIC_SHARED_D_READER_IMPL(CartesianPlotLegend, qreal, layoutHorizontalSpacing, layoutHorizontalSpacing)
BASIC_SHARED_D_READER_IMPL(CartesianPlotLegend, qreal, layoutVerticalSpacing, layoutVerticalSpacing)
BASIC_SHARED_D_READER_IMPL(CartesianPlotLegend, int, layoutColumnCount, layoutColumnCount)

// ##############################################################################
// ######################  setter methods and undo commands  ####################
// ##############################################################################
STD_SETTER_CMD_IMPL_F_S(CartesianPlotLegend, SetLabelFont, QFont, labelFont, retransform)
void CartesianPlotLegend::setLabelFont(const QFont& font) {
	Q_D(CartesianPlotLegend);
	if (font != d->labelFont)
		exec(new CartesianPlotLegendSetLabelFontCmd(d, font, ki18n("%1: set font")));
}

STD_SETTER_CMD_IMPL_F_S(CartesianPlotLegend, SetUsePlotColor, bool, usePlotColor, retransform)
void CartesianPlotLegend::setUsePlotColor(bool usePlotColor) {
	Q_D(CartesianPlotLegend);
	if (usePlotColor != d->usePlotColor)
		exec(new CartesianPlotLegendSetUsePlotColorCmd(d, usePlotColor, ki18n("%1: use plot's color")));
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

STD_SETTER_CMD_IMPL_F_S(CartesianPlotLegend, SetLineSymbolWidth, double, lineSymbolWidth, retransform)
void CartesianPlotLegend::setLineSymbolWidth(double width) {
	Q_D(CartesianPlotLegend);
	if (width != d->lineSymbolWidth)
		exec(new CartesianPlotLegendSetLineSymbolWidthCmd(d, width, ki18n("%1: change line+symbol width")));
}

// Border
STD_SETTER_CMD_IMPL_F_S(CartesianPlotLegend, SetBorderCornerRadius, qreal, borderCornerRadius, update)
void CartesianPlotLegend::setBorderCornerRadius(qreal radius) {
	Q_D(CartesianPlotLegend);
	if (radius != d->borderCornerRadius)
		exec(new CartesianPlotLegendSetBorderCornerRadiusCmd(d, radius, ki18n("%1: set border corner radius")));
}

// Layout
STD_SETTER_CMD_IMPL_F_S(CartesianPlotLegend, SetLayoutTopMargin, double, layoutTopMargin, retransform)
void CartesianPlotLegend::setLayoutTopMargin(double margin) {
	Q_D(CartesianPlotLegend);
	if (margin != d->layoutTopMargin)
		exec(new CartesianPlotLegendSetLayoutTopMarginCmd(d, margin, ki18n("%1: set layout top margin")));
}

STD_SETTER_CMD_IMPL_F_S(CartesianPlotLegend, SetLayoutBottomMargin, double, layoutBottomMargin, retransform)
void CartesianPlotLegend::setLayoutBottomMargin(double margin) {
	Q_D(CartesianPlotLegend);
	if (margin != d->layoutBottomMargin)
		exec(new CartesianPlotLegendSetLayoutBottomMarginCmd(d, margin, ki18n("%1: set layout bottom margin")));
}

STD_SETTER_CMD_IMPL_F_S(CartesianPlotLegend, SetLayoutLeftMargin, double, layoutLeftMargin, retransform)
void CartesianPlotLegend::setLayoutLeftMargin(double margin) {
	Q_D(CartesianPlotLegend);
	if (margin != d->layoutLeftMargin)
		exec(new CartesianPlotLegendSetLayoutLeftMarginCmd(d, margin, ki18n("%1: set layout left margin")));
}

STD_SETTER_CMD_IMPL_F_S(CartesianPlotLegend, SetLayoutRightMargin, double, layoutRightMargin, retransform)
void CartesianPlotLegend::setLayoutRightMargin(double margin) {
	Q_D(CartesianPlotLegend);
	if (margin != d->layoutRightMargin)
		exec(new CartesianPlotLegendSetLayoutRightMarginCmd(d, margin, ki18n("%1: set layout right margin")));
}

STD_SETTER_CMD_IMPL_F_S(CartesianPlotLegend, SetLayoutVerticalSpacing, double, layoutVerticalSpacing, retransform)
void CartesianPlotLegend::setLayoutVerticalSpacing(double spacing) {
	Q_D(CartesianPlotLegend);
	if (spacing != d->layoutVerticalSpacing)
		exec(new CartesianPlotLegendSetLayoutVerticalSpacingCmd(d, spacing, ki18n("%1: set layout vertical spacing")));
}

STD_SETTER_CMD_IMPL_F_S(CartesianPlotLegend, SetLayoutHorizontalSpacing, double, layoutHorizontalSpacing, retransform)
void CartesianPlotLegend::setLayoutHorizontalSpacing(double spacing) {
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

// ##############################################################################
// #################################  SLOTS  ####################################
// ##############################################################################

// ##############################################################################
// ######################### Private implementation #############################
// ##############################################################################
CartesianPlotLegendPrivate::CartesianPlotLegendPrivate(CartesianPlotLegend* owner)
	: WorksheetElementPrivate(owner)
	, q(owner) {
	setFlag(QGraphicsItem::ItemIsSelectable, true);
	setFlag(QGraphicsItem::ItemIsMovable);
	setFlag(QGraphicsItem::ItemSendsGeometryChanges);
	setFlag(QGraphicsItem::ItemIsFocusable);
	setAcceptHoverEvents(true);
}

/*!
  Returns the shape of the CartesianPlotLegend as a QPainterPath in local coordinates
*/
QPainterPath CartesianPlotLegendPrivate::shape() const {
	QPainterPath path;
	if (qFuzzyIsNull(borderCornerRadius))
		path.addRect(m_boundingRectangle);
	else
		path.addRoundedRect(m_boundingRectangle, borderCornerRadius, borderCornerRadius);

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

	m_plots.clear();
	m_names.clear();

	const auto& plots = this->plot->children<Plot>();
	for (auto* plot : plots) {
		if (!plot->isVisible() || !plot->legendVisible())
			continue;

		// add the names for plot types which can show multiple datasets
		auto* boxPlot = dynamic_cast<BoxPlot*>(plot);
		if (boxPlot) {
			m_plots << boxPlot;
			const auto& columns = boxPlot->dataColumns();
			for (auto* column : columns)
				m_names << column->name();

			continue;
		}

		auto* barPlot = dynamic_cast<BarPlot*>(plot);
		if (barPlot) {
			m_plots << barPlot;
			const auto& columns = barPlot->dataColumns();
			for (auto* column : columns)
				if (column)
					m_names << column->name();

			continue;
		}

		auto* lollipopPlot = dynamic_cast<LollipopPlot*>(plot);
		if (lollipopPlot) {
			m_plots << lollipopPlot;
			const auto& columns = lollipopPlot->dataColumns();
			for (auto* column : columns)
				m_names << column->name();

			continue;
		}

		m_plots << plot;
		m_names << plot->name();
		continue;
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

	qreal legendWidth = 0;
	for (int c = 0; c < columnCount; ++c) {
		int maxTextWidth = 0, index;
		for (int r = 0; r < rowCount; ++r) {
			if (labelColumnMajor)
				index = c * rowCount + r;
			else
				index = r * columnCount + c;

			if (index >= namesCount)
				break;

			int w = fm.boundingRect(m_names.at(index)).width();
			if (w > maxTextWidth)
				maxTextWidth = w;
		}
		maxColumnTextWidths.append(maxTextWidth);
		legendWidth += maxTextWidth;
	}

	legendWidth += layoutLeftMargin + layoutRightMargin; // margins
	legendWidth += columnCount * (lineSymbolWidth + layoutHorizontalSpacing); // width of the columns without the text
	legendWidth += (columnCount - 1) * 2. * layoutHorizontalSpacing; // spacings between the columns

	// add title width if title is available
	if (title->isVisible() && !title->text().text.isEmpty()) {
		qreal titleWidth;
		titleWidth = title->graphicsItem()->boundingRect().width();

		if (titleWidth > legendWidth)
			legendWidth = titleWidth;
	}

	// determine the height of the legend
	int h = fm.ascent();
	qreal legendHeight = layoutTopMargin + layoutBottomMargin; // margins
	legendHeight += rowCount * h; // height of the rows
	legendHeight += (rowCount - 1) * layoutVerticalSpacing; // spacing between the rows
	if (title->isVisible() && !title->text().text.isEmpty())
		legendHeight += title->graphicsItem()->boundingRect().height(); // legend titl

	m_boundingRectangle.setX(-legendWidth / 2.);
	m_boundingRectangle.setY(-legendHeight / 2.);
	m_boundingRectangle.setWidth(legendWidth);
	m_boundingRectangle.setHeight(legendHeight);

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

	// draw the background area
	background->draw(painter, m_boundingRectangle, borderCornerRadius);

	// draw the border
	if (borderLine->style() != Qt::NoPen) {
		painter->setPen(borderLine->pen());
		painter->setBrush(Qt::NoBrush);
		painter->setOpacity(borderLine->opacity());
		if (qFuzzyIsNull(borderCornerRadius))
			painter->drawRect(m_boundingRectangle);
		else
			painter->drawRoundedRect(m_boundingRectangle, borderCornerRadius, borderCornerRadius);
	}

	// draw curve's line+symbol and the names
	QFontMetrics fm(labelFont);
	float h = fm.ascent();
	painter->setFont(labelFont);

	// translate to left upper corner of the bounding rect plus the layout offset and the height of the title
	painter->translate(-m_boundingRectangle.width() / 2 + layoutLeftMargin, -m_boundingRectangle.height() / 2 + layoutTopMargin);
	if (title->isVisible() && !title->text().text.isEmpty())
		painter->translate(0, title->graphicsItem()->boundingRect().height());

	painter->save();

	int col = 0, row = 0;
	for (auto* plot : m_plots) {
		// process the curves
		// TODO: move the logic below into the plot classes
		const auto* curve = dynamic_cast<const XYCurve*>(plot);
		const auto* hist = dynamic_cast<const Histogram*>(plot);
		const auto* boxPlot = dynamic_cast<const BoxPlot*>(plot);
		const auto* barPlot = dynamic_cast<const BarPlot*>(plot);
		const auto* lollipopPlot = dynamic_cast<const LollipopPlot*>(plot);
		const auto* kdePlot = dynamic_cast<const KDEPlot*>(plot);
		const auto* qqPlot = dynamic_cast<const QQPlot*>(plot);

		if (curve) { // draw the legend item for xy-curve
			// curve's line (painted at the half of the ascent size)
			if (curve->lineType() != XYCurve::LineType::NoLine) {
				painter->setPen(curve->line()->pen());
				painter->setOpacity(curve->line()->opacity());
				painter->drawLine(0, h / 2, lineSymbolWidth, h / 2);
			}

			// error bars
			const auto xErrorType = curve->errorBar()->xErrorType();
			const auto yErrorType = curve->errorBar()->yErrorType();
			const auto* errorBarsLine = curve->errorBar()->line();
			if ((xErrorType != ErrorBar::ErrorType::NoError && curve->errorBar()->xPlusColumn())
				|| (yErrorType != ErrorBar::ErrorType::NoError && curve->errorBar()->yPlusColumn())) {
				painter->setOpacity(errorBarsLine->opacity());
				painter->setPen(errorBarsLine->pen());

				// curve's error bars for x
				float errorBarsSize = Worksheet::convertToSceneUnits(10, Worksheet::Unit::Point);
				if (curve->symbol()->style() != Symbol::Style::NoSymbols && errorBarsSize < curve->symbol()->size() * 1.4)
					errorBarsSize = curve->symbol()->size() * 1.4;

				switch (curve->errorBar()->type()) {
				case ErrorBar::Type::Simple:
					// horiz. line
					if (xErrorType != ErrorBar::ErrorType::NoError)
						painter->drawLine(lineSymbolWidth / 2 - errorBarsSize / 2, h / 2, lineSymbolWidth / 2 + errorBarsSize / 2, h / 2);
					// vert. line
					if (yErrorType != ErrorBar::ErrorType::NoError)
						painter->drawLine(lineSymbolWidth / 2, h / 2 - errorBarsSize / 2, lineSymbolWidth / 2, h / 2 + errorBarsSize / 2);
					break;
				case ErrorBar::Type::WithEnds:
					// horiz. line
					if (xErrorType != ErrorBar::ErrorType::NoError) {
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
					if (yErrorType != ErrorBar::ErrorType::NoError) {
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
				painter->translate(QPointF(lineSymbolWidth / 2, h / 2));
				symbol->draw(painter, QPointF(0., 0.));
				painter->translate(-QPointF(lineSymbolWidth / 2, h / 2));
			}

			// curve's name
			usePlotColor ? painter->setPen(QPen(curve->color())) : painter->setPen(QPen(labelColor));
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
			usePlotColor ? painter->setPen(QPen(hist->color())) : painter->setPen(QPen(labelColor));
			painter->setOpacity(1.0);
			painter->drawText(QPoint(lineSymbolWidth + layoutHorizontalSpacing, h), hist->name());

			if (!translatePainter(painter, row, col, h))
				break;
		} else if (boxPlot) { // draw a legend item for every dataset in the box plot
			const auto& columns = boxPlot->dataColumns();
			int index = 0;
			for (auto* column : columns) {
				// draw the whiskers
				painter->setOpacity(boxPlot->whiskersLine()->opacity());
				painter->setPen(boxPlot->whiskersLine()->pen());
				painter->drawLine(lineSymbolWidth / 2, 0, lineSymbolWidth / 2, 0.3 * h);
				painter->drawLine(lineSymbolWidth / 2, 0.7 * h, lineSymbolWidth / 2, h);

				// draw the box
				auto* background = boxPlot->backgroundAt(index);
				painter->setOpacity(background->opacity());
				painter->setBrush(QBrush(background->firstColor(), background->brushStyle()));
				painter->setPen(Qt::NoPen);
				painter->translate(QPointF(lineSymbolWidth / 2, h / 2));
				painter->drawRect(QRectF(-h * 0.25, -0.2 * h, 0.5 * h, 0.4 * h));
				painter->translate(-QPointF(lineSymbolWidth / 2, h / 2));

				auto* borderLine = boxPlot->borderLineAt(index);
				painter->setOpacity(borderLine->opacity());
				// painter->setBrush(QBrush(background->firstColor(), background->brushStyle()));
				painter->setPen(borderLine->pen());
				painter->translate(QPointF(lineSymbolWidth / 2, h / 2));
				painter->drawRect(QRectF(-h * 0.25, -0.2 * h, 0.5 * h, 0.4 * h));
				painter->translate(-QPointF(lineSymbolWidth / 2, h / 2));

				// draw the name text
				usePlotColor ? painter->setPen(QPen(boxPlot->colorAt(index))) : painter->setPen(QPen(labelColor));
				painter->setOpacity(1.0);
				painter->drawText(QPoint(lineSymbolWidth + layoutHorizontalSpacing, h), column->name());
				++index;

				if (!translatePainter(painter, row, col, h))
					break;
			}
		} else if (barPlot) { // draw a legend item for every dataset in the bar plot
			const auto& columns = barPlot->dataColumns();
			int index = 0;
			for (auto* column : columns) {
				if (!column)
					continue;

				// draw the bar
				auto* background = barPlot->backgroundAt(index);
				painter->setOpacity(background->opacity());
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
				usePlotColor ? painter->setPen(QPen(barPlot->colorAt(index))) : painter->setPen(QPen(labelColor));
				painter->setOpacity(1.0);
				painter->drawText(QPoint(lineSymbolWidth + layoutHorizontalSpacing, h), column->name());
				++index;

				if (!translatePainter(painter, row, col, h))
					break;
			}
		} else if (lollipopPlot) { // draw a legend item for every dataset in the lollipop plot
			const auto& columns = lollipopPlot->dataColumns();
			int index = 0;
			for (auto* column : columns) {
				// draw the line
				auto* line = lollipopPlot->lineAt(index);
				painter->setPen(line->pen());
				painter->setOpacity(line->opacity());
				painter->setBrush(Qt::NoBrush);
				painter->drawLine(lineSymbolWidth / 2, h * 0.25, lineSymbolWidth / 2, h);

				// draw the symbol
				const auto* symbol = lollipopPlot->symbolAt(index);
				if (symbol->style() != Symbol::Style::NoSymbols) {
					painter->translate(QPointF(lineSymbolWidth / 2, h * 0.25));
					symbol->draw(painter, QPointF(0., 0.));
					painter->translate(-QPointF(lineSymbolWidth / 2, h * 0.25));
				}

				// draw the name text
				usePlotColor ? painter->setPen(QPen(lollipopPlot->colorAt(index))) : painter->setPen(QPen(labelColor));
				painter->setOpacity(1.0);
				painter->drawText(QPoint(lineSymbolWidth + layoutHorizontalSpacing, h), column->name());
				++index;
				if (!translatePainter(painter, row, col, h))
					break;
			}
		} else if (kdePlot) {
			// line
			const auto* line = kdePlot->estimationCurve()->line();
			painter->setPen(line->pen());
			painter->setOpacity(line->opacity());
			painter->drawLine(0, h / 2, lineSymbolWidth, h / 2);

			// name
			usePlotColor ? painter->setPen(QPen(kdePlot->color())) : painter->setPen(QPen(labelColor));
			painter->setOpacity(1.0);
			painter->drawText(QPoint(lineSymbolWidth + layoutHorizontalSpacing, h), kdePlot->name());

			if (!translatePainter(painter, row, col, h))
				break;
		} else if (qqPlot) {
			// line
			const auto* line = qqPlot->line();
			painter->setPen(line->pen());
			painter->setOpacity(line->opacity());
			painter->drawLine(0, h / 2, lineSymbolWidth, h / 2);

			// symbol
			const auto* symbol = qqPlot->symbol();
			if (symbol->style() != Symbol::Style::NoSymbols) {
				painter->translate(QPointF(lineSymbolWidth / 2, h / 2));
				symbol->draw(painter, QPointF(0., 0.));
				painter->translate(-QPointF(lineSymbolWidth / 2, h / 2));
			}

			// name
			usePlotColor ? painter->setPen(QPen(qqPlot->color())) : painter->setPen(QPen(labelColor));
			painter->setOpacity(1.0);
			painter->drawText(QPoint(lineSymbolWidth + layoutHorizontalSpacing, h), qqPlot->name());

			if (!translatePainter(painter, row, col, h))
				break;
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

			double deltaX =
				lineSymbolWidth + layoutHorizontalSpacing + maxColumnTextWidths.at(col - 1); // width of the current column (subtract 1 because of ++col above)
			deltaX += 2 * layoutHorizontalSpacing; // spacing between two columns
			painter->translate(deltaX, 0);
			painter->save();
		}
	} else { // row major order
		++col;
		if (col != columnCount) {
			double deltaX =
				lineSymbolWidth + layoutHorizontalSpacing + maxColumnTextWidths.at(col - 1); // width of the current column (subtract 1 because of ++col above)
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

// ##############################################################################
// ##################  Serialization/Deserialization  ###########################
// ##############################################################################
//! Save as XML
void CartesianPlotLegend::save(QXmlStreamWriter* writer) const {
	Q_D(const CartesianPlotLegend);

	writer->writeStartElement(QStringLiteral("cartesianPlotLegend"));
	writeBasicAttributes(writer);
	writeCommentElement(writer);

	// general
	writer->writeStartElement(QStringLiteral("general"));
	writer->writeAttribute(QStringLiteral("usePlotColor"), QString::number(d->usePlotColor));
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
	d->borderLine->save(writer);
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

			READ_INT_VALUE("usePlotColor", usePlotColor, bool);
			READ_QCOLOR(d->labelColor);
			READ_QFONT(d->labelFont);
			READ_INT_VALUE("columnMajor", labelColumnMajor, int);
			READ_DOUBLE_VALUE("lineSymbolWidth", lineSymbolWidth);

			if (Project::xmlVersion() < 6) {
				str = attribs.value(QStringLiteral("visible")).toString();
				if (str.isEmpty())
					reader->raiseMissingAttributeWarning(QStringLiteral("visible"));
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
					reader->raiseMissingAttributeWarning(QStringLiteral("x"));
				else
					d->position.point.setX(str.toDouble());

				str = attribs.value(QStringLiteral("y")).toString();
				if (str.isEmpty())
					reader->raiseMissingAttributeWarning(QStringLiteral("y"));
				else
					d->position.point.setY(str.toDouble());

				str = attribs.value(QStringLiteral("horizontalPosition")).toString();
				if (str.isEmpty())
					reader->raiseMissingAttributeWarning(QStringLiteral("horizontalPosition"));
				else {
					const auto pos = (WorksheetElement::HorizontalPosition)str.toInt();
					if (pos == WorksheetElement::HorizontalPosition::Relative)
						d->position.horizontalPosition = WorksheetElement::HorizontalPosition::Center;
					else
						d->position.horizontalPosition = pos;
				}

				str = attribs.value(QStringLiteral("verticalPosition")).toString();
				if (str.isEmpty())
					reader->raiseMissingAttributeWarning(QStringLiteral("verticalPosition"));
				else {
					const auto pos = (WorksheetElement::VerticalPosition)str.toInt();
					if (pos == WorksheetElement::VerticalPosition::Relative)
						d->position.verticalPosition = WorksheetElement::VerticalPosition::Center;
					else
						d->position.verticalPosition = pos;
				}

				// in the old format the order was reversed, multiple by -1 here
				d->position.point.setY(-d->position.point.y());

				d->horizontalAlignment = WorksheetElement::HorizontalAlignment::Center;
				d->verticalAlignment = WorksheetElement::VerticalAlignment::Center;

				QGRAPHICSITEM_READ_DOUBLE_VALUE("rotation", Rotation);
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
			d->borderLine->load(reader, preview);
			READ_DOUBLE_VALUE("borderCornerRadius", borderCornerRadius);
		} else if (!preview && reader->name() == QLatin1String("layout")) {
			attribs = reader->attributes();
			READ_DOUBLE_VALUE("topMargin", layoutTopMargin);
			READ_DOUBLE_VALUE("bottomMargin", layoutBottomMargin);
			READ_DOUBLE_VALUE("leftMargin", layoutLeftMargin);
			READ_DOUBLE_VALUE("rightMargin", layoutRightMargin);
			READ_DOUBLE_VALUE("verticalSpacing", layoutVerticalSpacing);
			READ_DOUBLE_VALUE("horizontalSpacing", layoutHorizontalSpacing);
			READ_INT_VALUE("columnCount", layoutColumnCount, int);
		} else { // unknown element
			reader->raiseUnknownElementWarning();
			if (!reader->skipToEndElement())
				return false;
		}
	}

	return true;
}

void CartesianPlotLegend::loadThemeConfig(const KConfig& config) {
	KConfigGroup group;

	// for the font color use the value defined in the theme config for Label
	if (config.hasGroup(QStringLiteral("Theme")))
		group = config.group(QStringLiteral("Label"));
	else
		group = config.group(QStringLiteral("CartesianPlotLegend"));

	this->setLabelColor(group.readEntry(QStringLiteral("FontColor"), QColor(Qt::black)));

	// for other theme dependent settings use the values defined in the theme config for CartesianPlot
	if (config.hasGroup(QStringLiteral("Theme")))
		group = config.group(QStringLiteral("CartesianPlot"));

	// background
	background()->loadThemeConfig(group);

	// border
	borderLine()->loadThemeConfig(group);
	this->setBorderCornerRadius(group.readEntry(QStringLiteral("BorderCornerRadius"), 0.0));

	title()->loadThemeConfig(config);
}
