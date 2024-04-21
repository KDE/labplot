/*
	File                 : PlotArea.cpp
	Project              : LabPlot
	Description          : Plot area (for background filling and clipping).
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2011-2022 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "backend/worksheet/plots/PlotArea.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/macros.h"
#include "backend/worksheet/Background.h"
#include "backend/worksheet/Line.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/PlotAreaPrivate.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <QPainter>
#include <QPalette>

/**
 * \class PlotArea
 * \brief Plot area (for background filling and clipping).
 *
 * \ingroup worksheet
 */

PlotArea::PlotArea(const QString& name, CartesianPlot* parent)
	: WorksheetElement(name, new PlotAreaPrivate(this), AspectType::PlotArea)
	, m_parent(parent) {
	init();
}

PlotArea::PlotArea(const QString& name, CartesianPlot* parent, PlotAreaPrivate* dd)
	: WorksheetElement(name, dd, AspectType::PlotArea)
	, m_parent(parent) {
	init();
}

// no need to delete the d-pointer here - it inherits from QGraphicsItem
// and is deleted during the cleanup in QGraphicsScene
PlotArea::~PlotArea() = default;

void PlotArea::init() {
	Q_D(PlotArea);

	setHidden(true); // we don't show PlotArea aspect in the model view.
	d->rect = QRectF(0, 0, 1, 1);
	d->setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);

	KConfig config;
	KConfigGroup group = config.group(QStringLiteral("PlotArea"));

	// Background
	d->background = new Background(QStringLiteral("background"));
	addChild(d->background);
	d->background->setHidden(true);
	d->background->init(group);
	connect(d->background, &Background::updateRequested, [=] {
		d->update();
	});

	// Border
	PlotArea::BorderType type; // default value
	type.setFlag(PlotArea::BorderTypeFlags::BorderLeft);
	type.setFlag(PlotArea::BorderTypeFlags::BorderTop);
	type.setFlag(PlotArea::BorderTypeFlags::BorderRight);
	type.setFlag(PlotArea::BorderTypeFlags::BorderBottom);
	d->borderType = static_cast<PlotArea::BorderType>(group.readEntry(QStringLiteral("BorderType"), static_cast<int>(type)));

	d->borderLine = new Line(QStringLiteral("borderLine"));
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
		Q_EMIT changed();
	});

	d->borderCornerRadius = group.readEntry(QStringLiteral("BorderCornerRadius"), 0.0);
}

QGraphicsItem* PlotArea::graphicsItem() const {
	return d_ptr;
}

bool PlotArea::isHovered() const {
	return m_parent->isHovered();
}

bool PlotArea::isSelected() const {
	return m_parent->isSelected();
}

void PlotArea::handleResize(double horizontalRatio, double verticalRatio, bool /*pageResize*/) {
	DEBUG(Q_FUNC_INFO);
	Q_D(PlotArea);

	d->rect.setWidth(d->rect.width() * horizontalRatio);
	d->rect.setHeight(d->rect.height() * verticalRatio);

	// TODO: scale line width
}

void PlotArea::retransform() {
	Q_D(PlotArea);
	d->retransform();
}

/* ============================ getter methods ================= */
BASIC_SHARED_D_READER_IMPL(PlotArea, bool, clippingEnabled, clippingEnabled())
BASIC_SHARED_D_READER_IMPL(PlotArea, QRectF, rect, rect)

// background
Background* PlotArea::background() const {
	Q_D(const PlotArea);
	return d->background;
}

BASIC_SHARED_D_READER_IMPL(PlotArea, PlotArea::BorderType, borderType, borderType)
BASIC_SHARED_D_READER_IMPL(PlotArea, qreal, borderCornerRadius, borderCornerRadius)

Line* PlotArea::borderLine() const {
	Q_D(const PlotArea);
	return d->borderLine;
}

/* ============================ setter methods and undo commands ================= */

STD_SWAP_METHOD_SETTER_CMD_IMPL(PlotArea, SetClippingEnabled, bool, toggleClipping)
void PlotArea::setClippingEnabled(bool on) {
	Q_D(PlotArea);
	if (d->clippingEnabled() != on)
		exec(new PlotAreaSetClippingEnabledCmd(d, on, ki18n("%1: toggle clipping")));
}

/*!
 * sets plot area rect in scene coordinates.
 */
void PlotArea::setRect(const QRectF& newRect) {
	Q_D(PlotArea);
	d->setRect(newRect);
}

// Border
STD_SETTER_CMD_IMPL_F_S(PlotArea, SetBorderType, PlotArea::BorderType, borderType, update)
void PlotArea::setBorderType(BorderType type) {
	Q_D(PlotArea);
	if (type != d->borderType)
		exec(new PlotAreaSetBorderTypeCmd(d, type, ki18n("%1: border type changed")));
}

STD_SETTER_CMD_IMPL_F_S(PlotArea, SetBorderCornerRadius, qreal, borderCornerRadius, update)
void PlotArea::setBorderCornerRadius(qreal radius) {
	Q_D(PlotArea);
	if (radius != d->borderCornerRadius)
		exec(new PlotAreaSetBorderCornerRadiusCmd(d, radius, ki18n("%1: set plot area corner radius")));
}

// #####################################################################
// ################### Private implementation ##########################
// #####################################################################
PlotAreaPrivate::PlotAreaPrivate(PlotArea* owner)
	: WorksheetElementPrivate(owner)
	, q(owner) {
}

bool PlotAreaPrivate::clippingEnabled() const {
	return (flags() & QGraphicsItem::ItemClipsChildrenToShape);
}

bool PlotAreaPrivate::toggleClipping(bool on) {
	bool oldValue = clippingEnabled();
	setFlag(QGraphicsItem::ItemClipsChildrenToShape, on);
	return oldValue;
}

void PlotAreaPrivate::setRect(const QRectF& r) {
	prepareGeometryChange();
	rect = mapRectFromScene(r);
}

QRectF PlotAreaPrivate::boundingRect() const {
	if (borderLine->pen().style() != Qt::NoPen) {
		const qreal width = rect.width();
		const qreal height = rect.height();
		const double penWidth = borderLine->pen().width();
		return QRectF{-width / 2 - penWidth / 2, -height / 2 - penWidth / 2, width + penWidth, height + penWidth};
	} else
		return rect;
}

QPainterPath PlotAreaPrivate::shape() const {
	QPainterPath path;
	if (qFuzzyIsNull(borderCornerRadius))
		path.addRect(rect);
	else
		path.addRoundedRect(rect, borderCornerRadius, borderCornerRadius);

	return path;
}

void PlotAreaPrivate::contextMenuEvent(QGraphicsSceneContextMenuEvent* event) {
	// Ignore context menu event and forward to next
	QGraphicsItem::contextMenuEvent(event);
}

void PlotAreaPrivate::update() {
	QGraphicsItem::update();
	Q_EMIT q->changed();
}

void PlotAreaPrivate::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget*) {
	if (!isVisible())
		return;

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
				painter->setBrush(QBrush(pix));
				painter->setBrushOrigin(pix.size().width() / 2, pix.size().height() / 2);
				painter->drawRoundedRect(rect, borderCornerRadius, borderCornerRadius);
				break;
			case Background::ImageStyle::Scaled:
				pix = pix.scaled(rect.size().toSize(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
				painter->setBrush(QBrush(pix));
				painter->setBrushOrigin(pix.size().width() / 2, pix.size().height() / 2);
				painter->drawRoundedRect(rect, borderCornerRadius, borderCornerRadius);
				break;
			case Background::ImageStyle::ScaledAspectRatio:
				pix = pix.scaled(rect.size().toSize(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
				painter->setBrush(QBrush(pix));
				painter->setBrushOrigin(pix.size().width() / 2, pix.size().height() / 2);
				painter->drawRoundedRect(rect, borderCornerRadius, borderCornerRadius);
				break;
			case Background::ImageStyle::Centered:
				painter->drawPixmap(QPointF(rect.center().x() - pix.size().width() / 2, rect.center().y() - pix.size().height() / 2), pix);
				break;
			case Background::ImageStyle::Tiled:
				painter->setBrush(QBrush(pix));
				painter->drawRoundedRect(rect, borderCornerRadius, borderCornerRadius);
				break;
			case Background::ImageStyle::CenterTiled:
				painter->setBrush(QBrush(pix));
				painter->setBrushOrigin(pix.size().width() / 2, pix.size().height() / 2);
				painter->drawRoundedRect(rect, borderCornerRadius, borderCornerRadius);
			}
		}
	} else if (background->type() == Background::Type::Pattern) {
		painter->setBrush(QBrush(background->firstColor(), background->brushStyle()));
	}

	// draw the background
	if (qFuzzyIsNull(borderCornerRadius))
		painter->drawRect(rect);
	else
		painter->drawRoundedRect(rect, borderCornerRadius, borderCornerRadius);

	// draw the border
	if (borderLine->pen().style() != Qt::NoPen) {
		painter->setPen(borderLine->pen());
		painter->setBrush(Qt::NoBrush);
		painter->setOpacity(borderLine->opacity());
		if (qFuzzyIsNull(borderCornerRadius)) {
			const double w = rect.width();
			const double h = rect.height();
			if (borderType.testFlag(PlotArea::BorderTypeFlags::BorderLeft))
				painter->drawLine(-w / 2, -h / 2, -w / 2, h / 2);
			if (borderType.testFlag(PlotArea::BorderTypeFlags::BorderTop))
				painter->drawLine(-w / 2, -h / 2, w / 2, -h / 2);
			if (borderType.testFlag(PlotArea::BorderTypeFlags::BorderRight))
				painter->drawLine(-w / 2 + w, -h / 2, w / 2, h / 2);
			if (borderType.testFlag(PlotArea::BorderTypeFlags::BorderBottom))
				painter->drawLine(w / 2, h / 2, -w / 2, h / 2);
		} else
			painter->drawRoundedRect(rect, borderCornerRadius, borderCornerRadius);
	}

	if (q->isHovered() || q->isSelected()) {
		const double penWidth = 6.;
		QRectF rect = boundingRect();
		rect = QRectF(-rect.width() / 2 + penWidth / 2, -rect.height() / 2 + penWidth / 2, rect.width() - penWidth, rect.height() - penWidth);

		if (q->isHovered() && !q->isSelected() && !q->isPrinting()) {
			painter->setPen(QPen(QApplication::palette().color(QPalette::Shadow), penWidth, Qt::SolidLine));
			painter->drawRect(rect);
		}

		if (q->isSelected() && !q->isPrinting()) {
			painter->setPen(QPen(QApplication::palette().color(QPalette::Highlight), penWidth, Qt::SolidLine));
			painter->drawRect(rect);
		}
	}
}

// ##############################################################################
// ##################  Serialization/Deserialization  ###########################
// ##############################################################################

//! Save as XML
void PlotArea::save(QXmlStreamWriter* writer) const {
	Q_D(const PlotArea);

	writer->writeStartElement(QStringLiteral("plotArea"));
	writeBasicAttributes(writer);
	writeCommentElement(writer);

	// background
	d->background->save(writer);

	// border
	writer->writeStartElement(QStringLiteral("border"));
	writer->writeAttribute(QStringLiteral("borderType"), QString::number(d->borderType));
	d->borderLine->save(writer);
	writer->writeAttribute(QStringLiteral("borderCornerRadius"), QString::number(d->borderCornerRadius));
	writer->writeEndElement();

	writer->writeEndElement();
}

//! Load from XML
bool PlotArea::load(XmlStreamReader* reader, bool preview) {
	Q_D(PlotArea);

	if (!readBasicAttributes(reader))
		return false;

	QXmlStreamAttributes attribs;
	QString str;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == QLatin1String("plotArea"))
			break;

		if (!reader->isStartElement())
			continue;

		if (!preview && reader->name() == QLatin1String("comment")) {
			if (!readCommentElement(reader))
				return false;
		} else if (!preview && reader->name() == QLatin1String("background"))
			d->background->load(reader, preview);
		else if (!preview && reader->name() == QLatin1String("border")) {
			attribs = reader->attributes();

			READ_INT_VALUE("borderType", borderType, PlotArea::BorderType);
			d->borderLine->load(reader, preview);

			str = attribs.value(QStringLiteral("borderCornerRadius")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("borderCornerRadius"));
			else
				d->borderCornerRadius = str.toDouble();
		} else { // unknown element
			reader->raiseUnknownElementWarning();
			if (!reader->skipToEndElement())
				return false;
		}
	}

	return true;
}

void PlotArea::loadThemeConfig(const KConfig& config) {
	KConfigGroup group;
	if (config.hasGroup(QStringLiteral("Theme")))
		group = config.group(QStringLiteral("CartesianPlot"));
	else
		group = config.group(QStringLiteral("PlotArea"));

	// background
	background()->loadThemeConfig(group);

	// border
	Q_D(PlotArea);
	d->borderLine->loadThemeConfig(group);
	this->setBorderCornerRadius(group.readEntry(QStringLiteral("BorderCornerRadius"), 0.0));
}

void PlotArea::saveThemeConfig(const KConfig& config) {
	KConfigGroup group = config.group(QStringLiteral("CartesianPlot"));

	// background
	background()->saveThemeConfig(group);

	// border
	Q_D(PlotArea);
	d->borderLine->saveThemeConfig(group);
	group.writeEntry(QStringLiteral("BorderCornerRadius"), this->borderCornerRadius());
}
