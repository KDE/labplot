/*
	File                 : Value.cpp
	Project              : LabPlot
	Description          : Value
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022-2023 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

/*!
  \class Value
  \brief This class contains the properties of values that are shown besides the data points in XYCurve, etc.

  \ingroup worksheet
*/

#include "Value.h"
#include "ValuePrivate.h"
#include "backend/core/AbstractColumn.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/commandtemplates.h"

#include <KConfigGroup>
#include <KLocalizedString>
#include <QPainter>

Value::Value(const QString& name)
	: AbstractAspect(name, AspectType::AbstractAspect)
	, d_ptr(new ValuePrivate(this)) {
}

Value::~Value() {
	delete d_ptr;
}

void Value::init(const KConfigGroup& group) {
	Q_D(Value);

	d->type = (Value::Type)group.readEntry("ValueType", (int)Value::NoValues);
	d->position = (Value::Position)group.readEntry("ValuePosition", (int)Value::Position::Above);
	d->distance = group.readEntry("ValueDistance", Worksheet::convertToSceneUnits(5, Worksheet::Unit::Point));
	d->rotationAngle = group.readEntry("ValueRotation", 0.0);
	d->opacity = group.readEntry("ValueOpacity", 1.0);
	d->numericFormat = group.readEntry("ValueNumericFormat", "f").at(0).toLatin1();
	d->precision = group.readEntry("ValuePrecision", 2);
	d->dateTimeFormat = group.readEntry("ValueDateTimeFormat", "yyyy-MM-dd");
	d->prefix = group.readEntry("ValuePrefix", "");
	d->suffix = group.readEntry("ValueSuffix", "");
	auto defaultFont = QFont();
	d->font = group.readEntry("ValueFont", QFont());
	d->font.setPixelSize(Worksheet::convertToSceneUnits(defaultFont.pointSizeF(), Worksheet::Unit::Point));
	d->color = group.readEntry("ValueColor", QColor(Qt::black));
}

void Value::draw(QPainter* painter, const QVector<QPointF>& points, const QVector<QString>& strings) {
	Q_D(Value);

	if (d->type == Value::NoValues)
		return;

	painter->setOpacity(d->opacity);
	painter->setPen(QPen(d->color));
	painter->setFont(d->font);

	int i = 0;
	for (const auto& point : points) {
		painter->translate(point);
		if (d->rotationAngle != 0.)
			painter->rotate(-d->rotationAngle);

		painter->drawText(QPoint(0, 0), strings.at(i++));

		if (d->rotationAngle != 0.)
			painter->rotate(d->rotationAngle);
		painter->translate(-point);
	}
}

// ##############################################################################
// ##########################  getter methods  ##################################
// ##############################################################################
BASIC_SHARED_D_READER_IMPL(Value, Value::Type, type, type)
BASIC_SHARED_D_READER_IMPL(Value, const AbstractColumn*, column, column)
QString& Value::columnPath() const {
	D(Value);
	return d->columnPath;
}
void Value::setColumnPath(const QString& path) {
	D(Value);
	d->columnPath = path;
}

BASIC_SHARED_D_READER_IMPL(Value, Value::Position, position, position)
BASIC_SHARED_D_READER_IMPL(Value, bool, centerPositionAvailable, centerPositionAvailable)
BASIC_SHARED_D_READER_IMPL(Value, double, distance, distance)
BASIC_SHARED_D_READER_IMPL(Value, double, rotationAngle, rotationAngle)
BASIC_SHARED_D_READER_IMPL(Value, double, opacity, opacity)
BASIC_SHARED_D_READER_IMPL(Value, char, numericFormat, numericFormat)
BASIC_SHARED_D_READER_IMPL(Value, int, precision, precision)
BASIC_SHARED_D_READER_IMPL(Value, QString, dateTimeFormat, dateTimeFormat)
BASIC_SHARED_D_READER_IMPL(Value, QString, prefix, prefix)
BASIC_SHARED_D_READER_IMPL(Value, QString, suffix, suffix)
BASIC_SHARED_D_READER_IMPL(Value, QColor, color, color)
BASIC_SHARED_D_READER_IMPL(Value, QFont, font, font)

// ##############################################################################
// #################  setter methods and undo commands ##########################
// ##############################################################################
STD_SETTER_CMD_IMPL_F_S(Value, SetType, Value::Type, type, updateValue)
void Value::setType(Value::Type type) {
	Q_D(Value);
	if (type != d->type)
		exec(new ValueSetTypeCmd(d, type, ki18n("%1: set values type")));
}
void Value::setcenterPositionAvailable(bool available) {
	Q_D(Value);
	d->centerPositionAvailable = available;
}

STD_SETTER_CMD_IMPL_F_S(Value, SetColumn, const AbstractColumn*, column, updateValue)
void Value::setColumn(const AbstractColumn* column) {
	Q_D(Value);
	if (column != d->column) {
		exec(new ValueSetColumnCmd(d, column, ki18n("%1: set values column")));
		if (column) {
			connect(column, &AbstractColumn::dataChanged, this, &Value::updateRequested);
			connect(column->parentAspect(), &AbstractAspect::childAspectAboutToBeRemoved, this, &Value::columnAboutToBeRemoved);
		}
	}
}

STD_SETTER_CMD_IMPL_F_S(Value, SetPosition, Value::Position, position, updateValue)
void Value::setPosition(Position position) {
	Q_D(Value);
	if (position != d->position)
		exec(new ValueSetPositionCmd(d, position, ki18n("%1: set values position")));
}

STD_SETTER_CMD_IMPL_F_S(Value, SetDistance, double, distance, updateValue)
void Value::setDistance(double distance) {
	Q_D(Value);
	if (distance != d->distance)
		exec(new ValueSetDistanceCmd(d, distance, ki18n("%1: set values distance")));
}

STD_SETTER_CMD_IMPL_F_S(Value, SetRotationAngle, double, rotationAngle, updateValue)
void Value::setRotationAngle(double angle) {
	Q_D(Value);
	if (!qFuzzyCompare(1 + angle, 1 + d->rotationAngle))
		exec(new ValueSetRotationAngleCmd(d, angle, ki18n("%1: rotate values")));
}

STD_SETTER_CMD_IMPL_F_S(Value, SetOpacity, double, opacity, updatePixmap)
void Value::setOpacity(double opacity) {
	Q_D(Value);
	if (opacity != d->opacity)
		exec(new ValueSetOpacityCmd(d, opacity, ki18n("%1: set values opacity")));
}

STD_SETTER_CMD_IMPL_F_S(Value, SetNumericFormat, char, numericFormat, updateValue)
void Value::setNumericFormat(char format) {
	Q_D(Value);
	if (format != d->numericFormat)
		exec(new ValueSetNumericFormatCmd(d, format, ki18n("%1: set values numeric format")));
}

STD_SETTER_CMD_IMPL_F_S(Value, SetPrecision, int, precision, updateValue)
void Value::setPrecision(int precision) {
	Q_D(Value);
	if (precision != d->precision)
		exec(new ValueSetPrecisionCmd(d, precision, ki18n("%1: set values precision")));
}

STD_SETTER_CMD_IMPL_F_S(Value, SetDateTimeFormat, QString, dateTimeFormat, updateValue)
void Value::setDateTimeFormat(const QString& format) {
	Q_D(Value);
	if (format != d->dateTimeFormat)
		exec(new ValueSetDateTimeFormatCmd(d, format, ki18n("%1: set values datetime format")));
}

STD_SETTER_CMD_IMPL_F_S(Value, SetPrefix, QString, prefix, updateValue)
void Value::setPrefix(const QString& prefix) {
	Q_D(Value);
	if (prefix != d->prefix)
		exec(new ValueSetPrefixCmd(d, prefix, ki18n("%1: set values prefix")));
}

STD_SETTER_CMD_IMPL_F_S(Value, SetSuffix, QString, suffix, updateValue)
void Value::setSuffix(const QString& suffix) {
	Q_D(Value);
	if (suffix != d->suffix)
		exec(new ValueSetSuffixCmd(d, suffix, ki18n("%1: set values suffix")));
}

STD_SETTER_CMD_IMPL_F_S(Value, SetFont, QFont, font, updateValue)
void Value::setFont(const QFont& font) {
	Q_D(Value);
	if (font != d->font)
		exec(new ValueSetFontCmd(d, font, ki18n("%1: set values font")));
}

STD_SETTER_CMD_IMPL_F_S(Value, SetColor, QColor, color, updatePixmap)
void Value::setColor(const QColor& color) {
	Q_D(Value);
	if (color != d->color)
		exec(new ValueSetColorCmd(d, color, ki18n("%1: set values color")));
}

void Value::columnAboutToBeRemoved(const AbstractAspect* aspect) {
	Q_D(Value);
	if (aspect == d->column) {
		d->column = nullptr;
		d->updateValue();
	}
}

// ##############################################################################
// ####################### Private implementation ###############################
// ##############################################################################
ValuePrivate::ValuePrivate(Value* owner)
	: q(owner) {
}

QString ValuePrivate::name() const {
	return q->parentAspect()->name();
}

void ValuePrivate::updateValue() {
	Q_EMIT q->updateRequested();
}

void ValuePrivate::updatePixmap() {
	Q_EMIT q->updatePixmapRequested();
}

// ##############################################################################
// ##################  Serialization/Deserialization  ###########################
// ##############################################################################
//! Save as XML
void Value::save(QXmlStreamWriter* writer) const {
	Q_D(const Value);

	writer->writeStartElement(QStringLiteral("values"));
	writer->writeAttribute(QStringLiteral("type"), QString::number(d->type));
	WRITE_COLUMN(d->column, column);
	writer->writeAttribute(QStringLiteral("position"), QString::number(d->position));
	writer->writeAttribute(QStringLiteral("distance"), QString::number(d->distance));
	writer->writeAttribute(QStringLiteral("rotation"), QString::number(d->rotationAngle));
	writer->writeAttribute(QStringLiteral("opacity"), QString::number(d->opacity));
	writer->writeAttribute(QStringLiteral("numericFormat"), QChar::fromLatin1(d->numericFormat));
	writer->writeAttribute(QStringLiteral("dateTimeFormat"), d->dateTimeFormat);
	writer->writeAttribute(QStringLiteral("precision"), QString::number(d->precision));
	writer->writeAttribute(QStringLiteral("prefix"), d->prefix);
	writer->writeAttribute(QStringLiteral("suffix"), d->suffix);
	WRITE_QCOLOR(d->color);
	WRITE_QFONT(d->font);
	writer->writeEndElement();
}

//! Load from XML
bool Value::load(XmlStreamReader* reader, bool preview) {
	if (preview)
		return true;

	Q_D(Value);
	QString str;

	auto attribs = reader->attributes();

	READ_INT_VALUE("type", type, Value::Type);
	READ_COLUMN(column);
	READ_INT_VALUE("position", position, Value::Position);
	READ_DOUBLE_VALUE("distance", rotationAngle);
	READ_DOUBLE_VALUE("rotation", rotationAngle);
	READ_DOUBLE_VALUE("opacity", opacity);

	str = attribs.value(QStringLiteral("numericFormat")).toString();
	if (str.isEmpty())
		reader->raiseMissingAttributeWarning(QStringLiteral("numericFormat"));
	else
		d->numericFormat = *(str.toLatin1().data());

	READ_STRING_VALUE("dateTimeFormat", dateTimeFormat);
	READ_INT_VALUE("precision", precision, int);

	// don't produce any warning if no prefix or suffix is set (empty string is allowed here in xml)
	d->prefix = attribs.value(QStringLiteral("prefix")).toString();
	d->suffix = attribs.value(QStringLiteral("suffix")).toString();

	READ_QCOLOR(d->color);
	READ_QFONT(d->font);

	return true;
}

// ##############################################################################
// #########################  Theme management ##################################
// ##############################################################################
void Value::loadThemeConfig(const KConfigGroup& group, const QColor& themeColor) {
	setOpacity(group.readEntry("ValueOpacity", 1.0));
	setColor(group.readEntry("ValueColor", themeColor));
}

void Value::saveThemeConfig(KConfigGroup& group) const {
	Q_D(const Value);
	group.writeEntry("ValueOpacity", d->opacity);
	group.writeEntry("ValueColor", d->color);
	group.writeEntry("ValueFont", d->font);
}
