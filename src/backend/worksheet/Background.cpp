/*
	File                 : Background.cpp
	Project              : LabPlot
	Description          : Background
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022-2024 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

/*!
  \class Background
  \brief This class contains the background properties of worksheet elements like worksheet background,
  plot background, the area filling in XYCurve, Histogram, etc.

  \ingroup worksheet
*/

#include "Background.h"
#include "BackgroundPrivate.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/commandtemplates.h"

#include <KConfigGroup>
#include <KLocalizedString>
#include <QPainter>

Background::Background(const QString& name)
	: AbstractAspect(name, AspectType::AbstractAspect)
	, d_ptr(new BackgroundPrivate(this)) {
}

Background::~Background() {
	delete d_ptr;
}

void Background::setPrefix(const QString& prefix) {
	Q_D(Background);
	d->prefix = prefix;
}

const QString& Background::prefix() const {
	Q_D(const Background);
	return d->prefix;
}

void Background::init(const KConfigGroup& group) {
	Q_D(Background);

	d->type = (Type)group.readEntry(d->prefix + QStringLiteral("Type"), static_cast<int>(Type::Color));
	d->colorStyle = (ColorStyle)group.readEntry(d->prefix + QStringLiteral("ColorStyle"), static_cast<int>(ColorStyle::SingleColor));
	d->imageStyle = (ImageStyle)group.readEntry(d->prefix + QStringLiteral("ImageStyle"), static_cast<int>(ImageStyle::Scaled));
	d->brushStyle = (Qt::BrushStyle)group.readEntry(d->prefix + QStringLiteral("BrushStyle"), static_cast<int>(Qt::SolidPattern));
	d->fileName = group.readEntry(d->prefix + QStringLiteral("FileName"), QString());
	d->firstColor = group.readEntry(d->prefix + QStringLiteral("FirstColor"), QColor(Qt::white));
	d->secondColor = group.readEntry(d->prefix + QStringLiteral("SecondColor"), QColor(Qt::black));

	double defaultOpacity = 1.0;
	auto type = parentAspect()->type();
	if (type == AspectType::Histogram || type == AspectType::BoxPlot)
		defaultOpacity = 0.5;
	d->opacity = group.readEntry(d->prefix + QStringLiteral("Opacity"), defaultOpacity);

	// optional parameters
	if (d->enabledAvailable)
		d->enabled = group.readEntry(d->prefix + QStringLiteral("Enabled"), true);

	if (d->positionAvailable)
		d->position = (Position)group.readEntry(d->prefix + QStringLiteral("Position"), static_cast<int>(Position::No));
}

void Background::draw(QPainter* painter, const QPolygonF& polygon, double radius) const {
	Q_D(const Background);
	const QRectF& rect = polygon.boundingRect();

	// brush
	if (d->type == Background::Type::Color) {
		switch (d->colorStyle) {
		case Background::ColorStyle::SingleColor: {
			painter->setBrush(QBrush(d->firstColor));
			break;
		}
		case Background::ColorStyle::HorizontalLinearGradient: {
			QLinearGradient linearGrad(rect.topLeft(), rect.topRight());
			linearGrad.setColorAt(0, d->firstColor);
			linearGrad.setColorAt(1, d->secondColor);
			painter->setBrush(QBrush(linearGrad));
			break;
		}
		case Background::ColorStyle::VerticalLinearGradient: {
			QLinearGradient linearGrad(rect.topLeft(), rect.bottomLeft());
			linearGrad.setColorAt(0, d->firstColor);
			linearGrad.setColorAt(1, d->secondColor);
			painter->setBrush(QBrush(linearGrad));
			break;
		}
		case Background::ColorStyle::TopLeftDiagonalLinearGradient: {
			QLinearGradient linearGrad(rect.topLeft(), rect.bottomRight());
			linearGrad.setColorAt(0, d->firstColor);
			linearGrad.setColorAt(1, d->secondColor);
			painter->setBrush(QBrush(linearGrad));
			break;
		}
		case Background::ColorStyle::BottomLeftDiagonalLinearGradient: {
			QLinearGradient linearGrad(rect.bottomLeft(), rect.topRight());
			linearGrad.setColorAt(0, d->firstColor);
			linearGrad.setColorAt(1, d->secondColor);
			painter->setBrush(QBrush(linearGrad));
			break;
		}
		case Background::ColorStyle::RadialGradient: {
			QRadialGradient radialGrad(rect.center(), rect.width() / 2);
			radialGrad.setColorAt(0, d->firstColor);
			radialGrad.setColorAt(1, d->secondColor);
			painter->setBrush(QBrush(radialGrad));
			break;
		}
		}
	} else if (d->type == Background::Type::Image) {
		if (!d->fileName.trimmed().isEmpty()) {
			QPixmap pix(d->fileName);
			switch (d->imageStyle) {
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
	} else if (d->type == Background::Type::Pattern)
		painter->setBrush(QBrush(d->firstColor, d->brushStyle));

	// pen and opacity
	painter->setOpacity(d->opacity);
	painter->setPen(Qt::NoPen);


	if (qFuzzyIsNull(radius))
		painter->drawPolygon(polygon);
	else
		painter->drawRoundedRect(rect, radius, radius);
}

// ##############################################################################
// ##########################  getter methods  ##################################
// ##############################################################################
BASIC_SHARED_D_READER_IMPL(Background, bool, enabledAvailable, enabledAvailable)
BASIC_SHARED_D_READER_IMPL(Background, bool, positionAvailable, positionAvailable)

BASIC_SHARED_D_READER_IMPL(Background, bool, enabled, enabled)
BASIC_SHARED_D_READER_IMPL(Background, Background::Position, position, position)
BASIC_SHARED_D_READER_IMPL(Background, Background::Type, type, type)
BASIC_SHARED_D_READER_IMPL(Background, Background::ColorStyle, colorStyle, colorStyle)
BASIC_SHARED_D_READER_IMPL(Background, Background::ImageStyle, imageStyle, imageStyle)
BASIC_SHARED_D_READER_IMPL(Background, Qt::BrushStyle, brushStyle, brushStyle)
BASIC_SHARED_D_READER_IMPL(Background, QColor, firstColor, firstColor)
BASIC_SHARED_D_READER_IMPL(Background, QColor, secondColor, secondColor)
BASIC_SHARED_D_READER_IMPL(Background, QString, fileName, fileName)
BASIC_SHARED_D_READER_IMPL(Background, double, opacity, opacity)

// ##############################################################################
// #################  setter methods and undo commands ##########################
// ##############################################################################
void Background::setEnabledAvailable(bool available) {
	Q_D(Background);
	d->enabledAvailable = available;
}

void Background::setPositionAvailable(bool available) {
	Q_D(Background);
	d->positionAvailable = available;
}

STD_SETTER_CMD_IMPL_F_S(Background, SetEnabled, bool, enabled, update)
void Background::setEnabled(bool enabled) {
	Q_D(Background);
	if (enabled != d->enabled)
		exec(new BackgroundSetEnabledCmd(d, enabled, ki18n("%1: filling changed")));
}

STD_SETTER_CMD_IMPL_F_S(Background, SetPosition, Background::Position, position, updatePosition)
void Background::setPosition(Position position) {
	Q_D(Background);
	if (position != d->position)
		exec(new BackgroundSetPositionCmd(d, position, ki18n("%1: filling position changed")));
}

STD_SETTER_CMD_IMPL_F_S(Background, SetType, Background::Type, type, update)
void Background::setType(Background::Type type) {
	Q_D(Background);
	if (type != d->type)
		exec(new BackgroundSetTypeCmd(d, type, ki18n("%1: background type changed")));
}

STD_SETTER_CMD_IMPL_F_S(Background, SetColorStyle, Background::ColorStyle, colorStyle, update)
void Background::setColorStyle(Background::ColorStyle style) {
	Q_D(Background);
	if (style != d->colorStyle)
		exec(new BackgroundSetColorStyleCmd(d, style, ki18n("%1: background color style changed")));
}

STD_SETTER_CMD_IMPL_F_S(Background, SetImageStyle, Background::ImageStyle, imageStyle, update)
void Background::setImageStyle(Background::ImageStyle style) {
	Q_D(Background);
	if (style != d->imageStyle)
		exec(new BackgroundSetImageStyleCmd(d, style, ki18n("%1: background image style changed")));
}

STD_SETTER_CMD_IMPL_F_S(Background, SetBrushStyle, Qt::BrushStyle, brushStyle, update)
void Background::setBrushStyle(Qt::BrushStyle style) {
	Q_D(Background);
	if (style != d->brushStyle)
		exec(new BackgroundSetBrushStyleCmd(d, style, ki18n("%1: background brush style changed")));
}

STD_SETTER_CMD_IMPL_F_S(Background, SetFirstColor, QColor, firstColor, update)
void Background::setFirstColor(const QColor& color) {
	Q_D(Background);
	if (color != d->firstColor)
		exec(new BackgroundSetFirstColorCmd(d, color, ki18n("%1: set background first color")));
}

STD_SETTER_CMD_IMPL_F_S(Background, SetSecondColor, QColor, secondColor, update)
void Background::setSecondColor(const QColor& color) {
	Q_D(Background);
	if (color != d->secondColor)
		exec(new BackgroundSetSecondColorCmd(d, color, ki18n("%1: set background second color")));
}

STD_SETTER_CMD_IMPL_F_S(Background, SetFileName, QString, fileName, update)
void Background::setFileName(const QString& fileName) {
	Q_D(Background);
	if (fileName != d->fileName)
		exec(new BackgroundSetFileNameCmd(d, fileName, ki18n("%1: set background image")));
}

STD_SETTER_CMD_IMPL_F_S(Background, SetOpacity, double, opacity, update)
void Background::setOpacity(double opacity) {
	Q_D(Background);
	if (opacity != d->opacity)
		exec(new BackgroundSetOpacityCmd(d, opacity, ki18n("%1: set background opacity")));
}

// ##############################################################################
// ####################### Private implementation ###############################
// ##############################################################################
BackgroundPrivate::BackgroundPrivate(Background* owner)
	: q(owner) {
}

QString BackgroundPrivate::name() const {
	return q->parentAspect()->name();
}

void BackgroundPrivate::update() {
	Q_EMIT q->updateRequested();
}

void BackgroundPrivate::updatePosition() {
	Q_EMIT q->updatePositionRequested();
}

// ##############################################################################
// ##################  Serialization/Deserialization  ###########################
// ##############################################################################
//! Save as XML
void Background::save(QXmlStreamWriter* writer) const {
	Q_D(const Background);

	writer->writeStartElement(d->prefix.toLower());
	if (d->enabledAvailable)
		writer->writeAttribute(QStringLiteral("enabled"), QString::number(d->enabled));

	if (d->positionAvailable)
		writer->writeAttribute(QStringLiteral("position"), QString::number(static_cast<int>(d->position)));

	writer->writeAttribute(QStringLiteral("type"), QString::number(static_cast<int>(d->type)));
	writer->writeAttribute(QStringLiteral("colorStyle"), QString::number(static_cast<int>(d->colorStyle)));
	writer->writeAttribute(QStringLiteral("imageStyle"), QString::number(static_cast<int>(d->imageStyle)));
	writer->writeAttribute(QStringLiteral("brushStyle"), QString::number(d->brushStyle));
	writer->writeAttribute(QStringLiteral("firstColor_r"), QString::number(d->firstColor.red()));
	writer->writeAttribute(QStringLiteral("firstColor_g"), QString::number(d->firstColor.green()));
	writer->writeAttribute(QStringLiteral("firstColor_b"), QString::number(d->firstColor.blue()));
	writer->writeAttribute(QStringLiteral("secondColor_r"), QString::number(d->secondColor.red()));
	writer->writeAttribute(QStringLiteral("secondColor_g"), QString::number(d->secondColor.green()));
	writer->writeAttribute(QStringLiteral("secondColor_b"), QString::number(d->secondColor.blue()));
	writer->writeAttribute(QStringLiteral("fileName"), d->fileName);
	writer->writeAttribute(QStringLiteral("opacity"), QString::number(d->opacity));
	writer->writeEndElement();
}

//! Load from XML
bool Background::load(XmlStreamReader* reader, bool preview) {
	if (preview)
		return true;

	Q_D(Background);
	QString str;

	auto attribs = reader->attributes();

	if (d->enabledAvailable)
		READ_INT_VALUE("enabled", enabled, bool);

	if (d->positionAvailable)
		READ_INT_VALUE("position", position, Position);

	READ_INT_VALUE("type", type, Background::Type);
	READ_INT_VALUE("colorStyle", colorStyle, Background::ColorStyle);
	READ_INT_VALUE("imageStyle", imageStyle, Background::ImageStyle);
	READ_INT_VALUE("brushStyle", brushStyle, Qt::BrushStyle);

	str = attribs.value(QStringLiteral("firstColor_r")).toString();
	if (str.isEmpty())
		reader->raiseMissingAttributeWarning(QStringLiteral("firstColor_r"));
	else
		d->firstColor.setRed(str.toInt());

	str = attribs.value(QStringLiteral("firstColor_g")).toString();
	if (str.isEmpty())
		reader->raiseMissingAttributeWarning(QStringLiteral("firstColor_g"));
	else
		d->firstColor.setGreen(str.toInt());

	str = attribs.value(QStringLiteral("firstColor_b")).toString();
	if (str.isEmpty())
		reader->raiseMissingAttributeWarning(QStringLiteral("firstColor_b"));
	else
		d->firstColor.setBlue(str.toInt());

	str = attribs.value(QStringLiteral("secondColor_r")).toString();
	if (str.isEmpty())
		reader->raiseMissingAttributeWarning(QStringLiteral("secondColor_r"));
	else
		d->secondColor.setRed(str.toInt());

	str = attribs.value(QStringLiteral("secondColor_g")).toString();
	if (str.isEmpty())
		reader->raiseMissingAttributeWarning(QStringLiteral("secondColor_g"));
	else
		d->secondColor.setGreen(str.toInt());

	str = attribs.value(QStringLiteral("secondColor_b")).toString();
	if (str.isEmpty())
		reader->raiseMissingAttributeWarning(QStringLiteral("secondColor_b"));
	else
		d->secondColor.setBlue(str.toInt());

	str = attribs.value(QStringLiteral("fileName")).toString();
	d->fileName = str;

	READ_DOUBLE_VALUE("opacity", opacity);

	return true;
}

// ##############################################################################
// #########################  Theme management ##################################
// ##############################################################################
void Background::loadThemeConfig(const KConfigGroup& group) {
	Q_D(const Background);
	const QColor themeColor = group.readEntry(d->prefix + QStringLiteral("FirstColor"), QColor(Qt::white));
	loadThemeConfig(group, themeColor);
}

void Background::loadThemeConfig(const KConfigGroup& group, const QColor& themeColor) {
	Q_D(const Background);

	if (d->positionAvailable)
		setPosition((Position)group.readEntry(d->prefix + QStringLiteral("Position"), static_cast<int>(Position::No)));

	setType((Type)group.readEntry(d->prefix + QStringLiteral("Type"), static_cast<int>(Type::Color)));
	setColorStyle((ColorStyle)group.readEntry(d->prefix + QStringLiteral("ColorStyle"), static_cast<int>(ColorStyle::SingleColor)));
	setImageStyle((ImageStyle)group.readEntry(d->prefix + QStringLiteral("ImageStyle"), static_cast<int>(ImageStyle::Scaled)));
	setBrushStyle((Qt::BrushStyle)group.readEntry(d->prefix + QStringLiteral("BrushStyle"), static_cast<int>(Qt::SolidPattern)));
	setFirstColor(themeColor);
	setSecondColor(group.readEntry(d->prefix + QStringLiteral("SecondColor"), QColor(Qt::black)));

	double defaultOpacity = 1.0;
	auto type = parentAspect()->type();
	if (type == AspectType::Histogram || type == AspectType::BoxPlot || type == AspectType::BarPlot)
		defaultOpacity = 0.8;
	setOpacity(group.readEntry(d->prefix + QStringLiteral("Opacity"), defaultOpacity));
}

void Background::saveThemeConfig(KConfigGroup& group) const {
	Q_D(const Background);

	if (d->positionAvailable)
		group.writeEntry(d->prefix + QStringLiteral("Position"), static_cast<int>(d->position));

	group.writeEntry(d->prefix + QStringLiteral("Type"), static_cast<int>(d->type));
	group.writeEntry(d->prefix + QStringLiteral("ColorStyle"), static_cast<int>(d->colorStyle));
	group.writeEntry(d->prefix + QStringLiteral("BrushStyle"), static_cast<int>(d->brushStyle));
	group.writeEntry(d->prefix + QStringLiteral("ImageStyle"), static_cast<int>(d->imageStyle));
	group.writeEntry(d->prefix + QStringLiteral("FirstColor"), d->firstColor);
	group.writeEntry(d->prefix + QStringLiteral("SecondColor"), d->secondColor);
	group.writeEntry(d->prefix + QStringLiteral("Opacity"), d->opacity);
}
