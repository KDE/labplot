/*
	File                 : Background.cpp
	Project              : LabPlot
	Description          : Background
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

/*!
  \class Background
  \brief

  \ingroup worksheet
*/

#include "Background.h"
#include "BackgroundPrivate.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/commandtemplates.h"
#include "backend/worksheet/Background.h"

#include <KLocalizedString>

Background::Background(const QString& name)
	: AbstractAspect(name, AspectType::AbstractAspect)
	, d_ptr(new BackgroundPrivate(this)) {
}

Background::~Background() {
	delete d_ptr;
}

void Background::init(const KConfigGroup& group) {
	Q_D(Background);

	d->type = (Type)group.readEntry("Type", static_cast<int>(Type::Color));
	d->colorStyle = (ColorStyle)group.readEntry("ColorStyle", static_cast<int>(ColorStyle::SingleColor));
	d->imageStyle = (ImageStyle)group.readEntry("ImageStyle", static_cast<int>(ImageStyle::Scaled));
	d->brushStyle = (Qt::BrushStyle)group.readEntry("BrushStyle", static_cast<int>(Qt::SolidPattern));
	d->fileName = group.readEntry("FileName", QString());
	d->firstColor = group.readEntry("FirstColor", QColor(Qt::white));
	d->secondColor = group.readEntry("SecondColor", QColor(Qt::black));
	d->opacity = group.readEntry("Opacity", 1.0);
}

//##############################################################################
//##########################  getter methods  ##################################
//##############################################################################
BASIC_SHARED_D_READER_IMPL(Background, bool, showEnabled, showEnabled)
BASIC_SHARED_D_READER_IMPL(Background, bool, enabled, enabled)
BASIC_SHARED_D_READER_IMPL(Background, Background::Type, type, type)
BASIC_SHARED_D_READER_IMPL(Background, Background::ColorStyle, colorStyle, colorStyle)
BASIC_SHARED_D_READER_IMPL(Background, Background::ImageStyle, imageStyle, imageStyle)
BASIC_SHARED_D_READER_IMPL(Background, Qt::BrushStyle, brushStyle, brushStyle)
BASIC_SHARED_D_READER_IMPL(Background, QColor, firstColor, firstColor)
BASIC_SHARED_D_READER_IMPL(Background, QColor, secondColor, secondColor)
BASIC_SHARED_D_READER_IMPL(Background, QString, fileName, fileName)
BASIC_SHARED_D_READER_IMPL(Background, double, opacity, opacity)

//##############################################################################
//#################  setter methods and undo commands ##########################
//##############################################################################
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
		exec(new BackgroundSetOpacityCmd(d, opacity, ki18n("%1: set opacity")));
}

//##############################################################################
//####################### Private implementation ###############################
//##############################################################################
BackgroundPrivate::BackgroundPrivate(Background* owner)
	: q(owner) {
}

QString BackgroundPrivate::name() const {
	return q->parentAspect()->name();
}

void BackgroundPrivate::update() {
	Q_EMIT q->updateRequested();
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################
//! Save as XML
void Background::save(QXmlStreamWriter* writer) const {
	Q_D(const Background);

	writer->writeStartElement("background");
	writer->writeAttribute("type", QString::number(static_cast<int>(d->type)));
	writer->writeAttribute("colorStyle", QString::number(static_cast<int>(d->colorStyle)));
	writer->writeAttribute("imageStyle", QString::number(static_cast<int>(d->imageStyle)));
	writer->writeAttribute("brushStyle", QString::number(d->brushStyle));
	writer->writeAttribute("firstColor_r", QString::number(d->firstColor.red()));
	writer->writeAttribute("firstColor_g", QString::number(d->firstColor.green()));
	writer->writeAttribute("firstColor_b", QString::number(d->firstColor.blue()));
	writer->writeAttribute("secondColor_r", QString::number(d->secondColor.red()));
	writer->writeAttribute("secondColor_g", QString::number(d->secondColor.green()));
	writer->writeAttribute("secondColor_b", QString::number(d->secondColor.blue()));
	writer->writeAttribute("fileName", d->fileName);
	writer->writeAttribute("opacity", QString::number(d->opacity));
	writer->writeEndElement();
}

//! Load from XML
bool Background::load(XmlStreamReader* reader, bool preview) {
	if (preview)
		return true;

	Q_D(Background);
	KLocalizedString attributeWarning = ki18n("Attribute '%1' missing or empty, default value is used");
	QString str;

	auto attribs = reader->attributes();

	READ_INT_VALUE("type", type, Background::Type);
	READ_INT_VALUE("colorStyle", colorStyle, Background::ColorStyle);
	READ_INT_VALUE("imageStyle", imageStyle, Background::ImageStyle);
	READ_INT_VALUE("brushStyle", brushStyle, Qt::BrushStyle);

	str = attribs.value("firstColor_r").toString();
	if (str.isEmpty())
		reader->raiseWarning(attributeWarning.subs("firstColor_r").toString());
	else
		d->firstColor.setRed(str.toInt());

	str = attribs.value("firstColor_g").toString();
	if (str.isEmpty())
		reader->raiseWarning(attributeWarning.subs("firstColor_g").toString());
	else
		d->firstColor.setGreen(str.toInt());

	str = attribs.value("firstColor_b").toString();
	if (str.isEmpty())
		reader->raiseWarning(attributeWarning.subs("firstColor_b").toString());
	else
		d->firstColor.setBlue(str.toInt());

	str = attribs.value("secondColor_r").toString();
	if (str.isEmpty())
		reader->raiseWarning(attributeWarning.subs("secondColor_r").toString());
	else
		d->secondColor.setRed(str.toInt());

	str = attribs.value("secondColor_g").toString();
	if (str.isEmpty())
		reader->raiseWarning(attributeWarning.subs("secondColor_g").toString());
	else
		d->secondColor.setGreen(str.toInt());

	str = attribs.value("secondColor_b").toString();
	if (str.isEmpty())
		reader->raiseWarning(attributeWarning.subs("secondColor_b").toString());
	else
		d->secondColor.setBlue(str.toInt());

	str = attribs.value("fileName").toString();
	d->fileName = str;

	READ_DOUBLE_VALUE("opacity", opacity);

	return true;
}

//##############################################################################
//#########################  Theme management ##################################
//##############################################################################
void Background::loadThemeConfig(const KConfigGroup& group) {
	setType((Type)group.readEntry("BackgroundType", static_cast<int>(Type::Color)));
	setColorStyle((ColorStyle)group.readEntry("BackgroundColorStyle", static_cast<int>(ColorStyle::SingleColor)));
	setImageStyle((ImageStyle)group.readEntry("BackgroundImageStyle", static_cast<int>(ImageStyle::Scaled)));
	setBrushStyle((Qt::BrushStyle)group.readEntry("BackgroundBrushStyle", static_cast<int>(Qt::SolidPattern)));
	setFirstColor(group.readEntry("BackgroundFirstColor", QColor(Qt::white)));
	setSecondColor(group.readEntry("BackgroundSecondColor", QColor(Qt::black)));
	setOpacity(group.readEntry("BackgroundOpacity", 1.0));
}

void Background::saveThemeConfig(KConfigGroup& group) const {
	Q_D(const Background);
	group.writeEntry("BackgroundType", static_cast<int>(d->type));
	group.writeEntry("BackgroundColorStyle", static_cast<int>(d->colorStyle));
	group.writeEntry("BackgroundBrushStyle", static_cast<int>(d->brushStyle));
	group.writeEntry("BackgroundImageStyle", static_cast<int>(d->imageStyle));
	group.writeEntry("BackgroundFirstColor", d->firstColor);
	group.writeEntry("BackgroundSecondColor", d->secondColor);
	group.writeEntry("BackgroundOpacity", d->opacity);
}
