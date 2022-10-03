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

	d->pen = QPen(group.readEntry(d->prefix + "Color", QColor(Qt::black)),
						group.readEntry(d->prefix + "Width", Worksheet::convertToSceneUnits(1.0, Worksheet::Unit::Point)),
						(Qt::PenStyle)group.readEntry("BorderStyle", (int)Qt::SolidLine));
	d->opacity = group.readEntry(d->prefix + "Opacity", 1.0);
}

//##############################################################################
//##########################  getter methods  ##################################
//##############################################################################
BASIC_SHARED_D_READER_IMPL(Line, QPen, pen, pen)
BASIC_SHARED_D_READER_IMPL(Line, double, opacity, opacity)

//##############################################################################
//#################  setter methods and undo commands ##########################
//##############################################################################
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

	writer->writeStartElement(d->prefix.toLower());
	WRITE_QPEN(d->pen);
	writer->writeAttribute("opacity", QString::number(d->opacity));
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
