/*
	File                 : Line.cpp
	Project              : LabPlot
	Description          : Line
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022 Alexander Semke <alexander.semke@web.de>
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
#include "backend/worksheet/Worksheet.h"

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

void Line::init(const KConfigGroup& group) {
	Q_D(Line);

	if (d->histogramLineTypeAvailable)
		d->histogramLineType = (Histogram::LineType)group.readEntry(d->prefix + "Type", (int)Histogram::Bars);

	if (d->errorBarsTypeAvailable) {
		d->errorBarsType = (XYCurve::ErrorBarsType)group.readEntry(d->prefix + "Type", static_cast<int>(XYCurve::ErrorBarsType::Simple));
		d->errorBarsCapSize = group.readEntry(d->prefix + "CapSize", Worksheet::convertToSceneUnits(10, Worksheet::Unit::Point));
	}

	d->pen = QPen(group.readEntry(d->prefix + "Color", QColor(Qt::black)),
				  group.readEntry(d->prefix + "Width", Worksheet::convertToSceneUnits(1.0, Worksheet::Unit::Point)),
				  (Qt::PenStyle)group.readEntry("BorderStyle", (int)Qt::SolidLine));
	d->opacity = group.readEntry(d->prefix + "Opacity", 1.0);
}

//##############################################################################
//##########################  getter methods  ##################################
//##############################################################################
BASIC_SHARED_D_READER_IMPL(Line, bool, histogramLineTypeAvailable, histogramLineTypeAvailable)
BASIC_SHARED_D_READER_IMPL(Line, Histogram::LineType, histogramLineType, histogramLineType)

BASIC_SHARED_D_READER_IMPL(Line, bool, errorBarsTypeAvailable, errorBarsTypeAvailable)
BASIC_SHARED_D_READER_IMPL(Line, XYCurve::ErrorBarsType, errorBarsType, errorBarsType)
BASIC_SHARED_D_READER_IMPL(Line, double, errorBarsCapSize, errorBarsCapSize)

BASIC_SHARED_D_READER_IMPL(Line, QPen, pen, pen)
BASIC_SHARED_D_READER_IMPL(Line, double, opacity, opacity)

//##############################################################################
//#################  setter methods and undo commands ##########################
//##############################################################################
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

void Line::setErrorBarsTypeAvailable(bool available) {
	Q_D(Line);
	d->errorBarsTypeAvailable = available;
}

STD_SETTER_CMD_IMPL_S(Line, SetErrorBarsCapSize, double, errorBarsCapSize)
void Line::setErrorBarsCapSize(qreal size) {
	Q_D(Line);
	if (size != d->errorBarsCapSize)
		exec(new LineSetErrorBarsCapSizeCmd(d, size, ki18n("%1: set error bar cap size")));
}

STD_SETTER_CMD_IMPL_S(Line, SetErrorBarsType, XYCurve::ErrorBarsType, errorBarsType)
void Line::setErrorBarsType(XYCurve::ErrorBarsType type) {
	Q_D(Line);
	if (type != d->errorBarsType)
		exec(new LineSetErrorBarsTypeCmd(d, type, ki18n("%1: error bar type changed")));
}

STD_SETTER_CMD_IMPL_F_S(Line, SetPen, QPen, pen, update)
void Line::setPen(const QPen& pen) {
	Q_D(Line);
	if (pen != d->pen)
		exec(new LineSetPenCmd(d, pen, ki18n("%1: set line pen")));
}

STD_SETTER_CMD_IMPL_F_S(Line, SetOpacity, double, opacity, updatePixmap)
void Line::setOpacity(qreal opacity) {
	Q_D(Line);
	if (opacity != d->opacity)
		exec(new LineSetOpacityCmd(d, opacity, ki18n("%1: set line opacity")));
}

//##############################################################################
//####################### Private implementation ###############################
//##############################################################################
LinePrivate::LinePrivate(Line* owner)
	: q(owner) {
}

QString LinePrivate::name() const {
	return q->parentAspect()->name();
}

void LinePrivate::update() {
	Q_EMIT q->updateRequested();
}

void LinePrivate::updatePixmap() {
	Q_EMIT q->updatePixmapRequested();
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################
//! Save as XML
void Line::save(QXmlStreamWriter* writer) const {
	Q_D(const Line);

	// no need to create a XML element for error bars and whiskers, the line attributes are part
	// of the same XML element together with other error bars and whiskers properties
	if (!d->errorBarsTypeAvailable || d->prefix == "Whiskers") {
		// for names in the XML file, the first letter is lower case but the camel case still remains.
		// so, we just convert the first character for lower case. e.g. MedianLine -> medianLine.
		QString newPrefix = d->prefix;
		newPrefix.replace(0, 1, d->prefix.at(0).toLower());
		writer->writeStartElement(newPrefix);
	}

	if (d->histogramLineTypeAvailable)
		writer->writeAttribute("type", QString::number(d->histogramLineType));
	else if (d->errorBarsTypeAvailable) {
		writer->writeAttribute("type", QString::number(static_cast<int>(d->errorBarsType)));
		writer->writeAttribute("capSize", QString::number(d->errorBarsCapSize));
	}

	WRITE_QPEN(d->pen);
	writer->writeAttribute("opacity", QString::number(d->opacity));

	if (!d->errorBarsTypeAvailable || d->prefix == "Whiskers")
		writer->writeEndElement();
}

//! Load from XML
bool Line::load(XmlStreamReader* reader, bool preview) {
	if (preview)
		return true;

	Q_D(Line);
	KLocalizedString attributeWarning = ki18n("Attribute '%1' missing or empty, default value is used");
	QString str;

	auto attribs = reader->attributes();

	if (d->histogramLineTypeAvailable)
		READ_INT_VALUE("type", histogramLineType, Histogram::LineType);

	if (d->errorBarsTypeAvailable) {
		READ_INT_VALUE("type", errorBarsType, XYCurve::ErrorBarsType);
		READ_DOUBLE_VALUE("capSize", errorBarsCapSize);
	}

	READ_QPEN(d->pen);
	READ_DOUBLE_VALUE("opacity", opacity);

	return true;
}

//##############################################################################
//#########################  Theme management ##################################
//##############################################################################
void Line::loadThemeConfig(const KConfigGroup& group, const QColor& themeColor) {
	Q_D(const Line);

	QPen p;
	p.setStyle((Qt::PenStyle)group.readEntry(d->prefix + "Style", (int)Qt::SolidLine));
	p.setWidthF(group.readEntry(d->prefix + "Width", Worksheet::convertToSceneUnits(1.0, Worksheet::Unit::Point)));
	p.setColor(themeColor);
	setPen(p);
	setOpacity(group.readEntry(d->prefix + "Opacity", 1.0));
}

void Line::saveThemeConfig(KConfigGroup& group) const {
	Q_D(const Line);
	group.writeEntry(d->prefix + "Style", static_cast<int>(d->pen.style()));
	group.writeEntry(d->prefix + "Width", d->pen.widthF());
	group.writeEntry(d->prefix + "Color", d->pen.color());
	group.writeEntry(d->prefix + "Opacity", d->opacity);
}
