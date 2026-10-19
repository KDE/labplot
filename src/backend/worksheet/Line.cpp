/*
	File                 : Line.cpp
	Project              : LabPlot
	Description          : Line
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022-2024 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

/*!
  \class Line
  \brief This class contains the line properties of worksheet elements.

  \ingroup worksheet
*/

#include "Line.h"
#include "LinePrivate.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/commandtemplates.h"
#include "backend/worksheet/plots/cartesian/ErrorBar.h"

#include <KConfigGroup>
#include <KLocalizedString>

Line::Line(const QString& name)
	: AbstractAspect(name, AspectType::AbstractAspect)
	, d_ptr(new LinePrivate(this)) {
}

Line::~Line() {
	delete d_ptr;
}

void Line::setPrefix(const QString& prefix) {
	Q_D(Line);
	d->prefix = prefix;
}

const QString& Line::prefix() const {
	Q_D(const Line);
	return d->prefix;
}

/*!
 * defines whether an XML element needs to be create in write(). For objects where the line
 * properties are serialized together with some other properties, the XML element is created
 * in objects's save() already and there is not need to create it once more in Line::save():
 */
void Line::setCreateXmlElement(bool create) {
	Q_D(Line);
	d->createXmlElement = create;
}

void Line::init(const KConfigGroup& group) {
	Q_D(Line);

	if (d->histogramLineTypeAvailable)
		d->histogramLineType = (Histogram::LineType)group.readEntry(d->prefix + QStringLiteral("Type"), (int)Histogram::Bars);

	if (d->prefix == QLatin1String("DropLine"))
		d->dropLineType = (XYCurve::DropLineType)group.readEntry(d->prefix + QStringLiteral("Type"), (int)XYCurve::DropLineType::NoDropLine);

	d->style = static_cast<Line::Style>(group.readEntry(d->prefix + QStringLiteral("Style"), static_cast<int>(Line::Style::SolidLine)));
	d->width = group.readEntry(d->prefix + QStringLiteral("Width"), Worksheet::convertToSceneUnits(1.0, Worksheet::Unit::Point));
	d->color = group.readEntry(d->prefix + QStringLiteral("Color"), QColor(Qt::black));
	d->opacity = group.readEntry(d->prefix + QStringLiteral("Opacity"), 1.0);
}

// ##############################################################################
// ##########################  getter methods  ##################################
// ##############################################################################
BASIC_SHARED_D_READER_IMPL(Line, bool, histogramLineTypeAvailable, histogramLineTypeAvailable)
BASIC_SHARED_D_READER_IMPL(Line, Histogram::LineType, histogramLineType, histogramLineType)
BASIC_SHARED_D_READER_IMPL(Line, XYCurve::DropLineType, dropLineType, dropLineType)

QPen Line::pen() const {
	Q_D(const Line);
	QPen pen;
	pen.setColor(d->color);
	pen.setWidthF(d->width);
	pen.setJoinStyle(Qt::MiterJoin);

	if (d->style == Style::RoundDotLine) {
		// Round dots: near-zero dash with RoundCap creates circles
		pen.setCapStyle(Qt::RoundCap);
		pen.setStyle(Qt::CustomDashLine);
		// Dash pattern in pen-width units: tiny dash + gap of 2x width
		pen.setDashPattern({0.001, 2.0});
	} else {
		pen.setCapStyle(Qt::FlatCap);
		pen.setStyle(static_cast<Qt::PenStyle>(d->style));
	}

	return pen;
}

BASIC_SHARED_D_READER_IMPL(Line, Line::Style, style, style)
BASIC_SHARED_D_READER_IMPL(Line, QColor, color, color)
BASIC_SHARED_D_READER_IMPL(Line, double, width, width)
BASIC_SHARED_D_READER_IMPL(Line, double, opacity, opacity)

// ##############################################################################
// #################  setter methods and undo commands ##########################
// ##############################################################################
void Line::setHistogramLineTypeAvailable(bool available) {
	Q_D(Line);
	d->histogramLineTypeAvailable = available;
}

STD_SETTER_CMD_IMPL_S(Line, SetHistogramLineType, Histogram::LineType, histogramLineType)
void Line::setHistogramLineType(Histogram::LineType type) {
	Q_D(Line);
	if (type != d->histogramLineType)
		exec(new LineSetHistogramLineTypeCmd(d, type, ki18n("%1: line type changed")));
}

STD_SETTER_CMD_IMPL_S(Line, SetDropLineType, XYCurve::DropLineType, dropLineType)
void Line::setDropLineType(XYCurve::DropLineType type) {
	Q_D(Line);
	if (type != d->dropLineType)
		exec(new LineSetDropLineTypeCmd(d, type, ki18n("%1: drop line type changed")));
}

STD_SETTER_CMD_IMPL_F_S(Line, SetStyle, Line::Style, style, update)
void Line::setStyle(Line::Style style) {
	Q_D(Line);
	if (style != d->style)
		exec(new LineSetStyleCmd(d, style, ki18n("%1: set line style")));
}

STD_SETTER_CMD_IMPL_F_S(Line, SetWidth, double, width, update)
void Line::setWidth(double width) {
	Q_D(Line);
	if (width != d->width)
		exec(new LineSetWidthCmd(d, width, ki18n("%1: set line width")));
}

STD_SETTER_CMD_IMPL_F_S(Line, SetColor, QColor, color, update)
void Line::setColor(const QColor& color) {
	Q_D(Line);
	if (color != d->color)
		exec(new LineSetColorCmd(d, color, ki18n("%1: set line color")));
}

STD_SETTER_CMD_IMPL_F_S(Line, SetOpacity, double, opacity, updatePixmap)
void Line::setOpacity(qreal opacity) {
	Q_D(Line);
	if (opacity != d->opacity)
		exec(new LineSetOpacityCmd(d, opacity, ki18n("%1: set line opacity")));
}

// ##############################################################################
// ####################### Private implementation ###############################
// ##############################################################################
LinePrivate::LinePrivate(Line* owner)
	: q(owner) {
}

QString LinePrivate::name() const {
	if (dynamic_cast<ErrorBar*>(q->parentAspect()))
		return q->parentAspect()->parentAspect()->name(); // for error bars we need to go one level higher to get the curve/plot name
	else
		return q->parentAspect()->name();
}

void LinePrivate::update() {
	Q_EMIT q->updateRequested();
}

void LinePrivate::updatePixmap() {
	Q_EMIT q->updatePixmapRequested();
}

// ##############################################################################
// ##################  Serialization/Deserialization  ###########################
// ##############################################################################
//! Save as XML
void Line::save(QXmlStreamWriter* writer) const {
	Q_D(const Line);

	if (d->createXmlElement) {
		// for names in the XML file, the first letter is lower case but the camel case still remains.
		// so, we just convert the first character for lower case. e.g. MedianLine -> medianLine.
		if (d->prefix != QLatin1String("DropLine")) {
			QString newPrefix = d->prefix;
			newPrefix.replace(0, 1, d->prefix.at(0).toLower());
			writer->writeStartElement(newPrefix);
		} else
			writer->writeStartElement(QLatin1String("dropLines")); // use "dropLines" for backward compatibility in XYCurve
	}

	if (d->histogramLineTypeAvailable)
		writer->writeAttribute(QStringLiteral("type"), QString::number(d->histogramLineType));
	else if (d->prefix == QLatin1String("DropLine"))
		writer->writeAttribute(QStringLiteral("type"), QString::number(static_cast<int>(d->dropLineType)));

	writer->writeAttribute(QStringLiteral("style"), QString::number(static_cast<int>(d->style)));
	writer->writeAttribute(QStringLiteral("color_r"), QString::number(d->color.red()));
	writer->writeAttribute(QStringLiteral("color_g"), QString::number(d->color.green()));
	writer->writeAttribute(QStringLiteral("color_b"), QString::number(d->color.blue()));
	writer->writeAttribute(QStringLiteral("width"), QString::number(d->width));
	writer->writeAttribute(QStringLiteral("opacity"), QString::number(d->opacity));

	if (d->createXmlElement)
		writer->writeEndElement();
}

//! Load from XML
bool Line::load(XmlStreamReader* reader, bool preview) {
	if (preview)
		return true;

	Q_D(Line);
	QString str;
	auto attribs = reader->attributes();

	if (d->histogramLineTypeAvailable)
		READ_INT_VALUE("type", histogramLineType, Histogram::LineType);

	if (d->prefix == QLatin1String("DropLine"))
		READ_INT_VALUE("type", dropLineType, XYCurve::DropLineType);

	READ_INT_VALUE("style", style, Line::Style);

	QColor color;
	str = attribs.value(QStringLiteral("color_r")).toString();
	if (str.isEmpty())
		reader->raiseMissingAttributeWarning(QStringLiteral("color_r"));
	else
		color.setRed(str.toInt());

	str = attribs.value(QStringLiteral("color_g")).toString();
	if (str.isEmpty())
		reader->raiseMissingAttributeWarning(QStringLiteral("color_g"));
	else
		color.setGreen(str.toInt());

	str = attribs.value(QStringLiteral("color_b")).toString();
	if (str.isEmpty())
		reader->raiseMissingAttributeWarning(QStringLiteral("color_b"));
	else
		color.setBlue(str.toInt());
	d->color = color;

	READ_DOUBLE_VALUE("width", width);
	READ_DOUBLE_VALUE("opacity", opacity);

	return true;
}

// ##############################################################################
// #########################  Theme management ##################################
// ##############################################################################
void Line::loadThemeConfig(const KConfigGroup& group) {
	Q_D(const Line);
	const auto& themeColor = group.readEntry(d->prefix + QStringLiteral("Color"), QColor(Qt::black));
	loadThemeConfig(group, themeColor);
}

void Line::loadThemeConfig(const KConfigGroup& group, const QColor& themeColor) {
	Q_D(const Line);
	setStyle(static_cast<Line::Style>(group.readEntry(d->prefix + QStringLiteral("Style"), static_cast<int>(Line::Style::SolidLine))));
	setWidth(group.readEntry(d->prefix + QStringLiteral("Width"), Worksheet::convertToSceneUnits(1.0, Worksheet::Unit::Point)));
	setColor(themeColor);
	setOpacity(group.readEntry(d->prefix + QStringLiteral("Opacity"), 1.0));
}

void Line::saveThemeConfig(KConfigGroup& group) const {
	Q_D(const Line);
	group.writeEntry(d->prefix + QStringLiteral("Style"), static_cast<int>(d->style));
	group.writeEntry(d->prefix + QStringLiteral("Width"), d->width);
	group.writeEntry(d->prefix + QStringLiteral("Color"), d->color);
	group.writeEntry(d->prefix + QStringLiteral("Opacity"), d->opacity);
}
