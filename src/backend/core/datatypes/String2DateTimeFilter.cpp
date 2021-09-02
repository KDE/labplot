/*
    File                 : String2DateTimeFilter.cpp
    Project              : LabPlot
    Description          : Conversion filter QString -> QDateTime.
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2007 Tilman Benkert <thzs@gmx.net>
    SPDX-FileCopyrightText: 2007 Knut Franke <knut.franke@gmx.de>
    SPDX-FileCopyrightText: 2017 Stefan Gerlach <stefan.gerlach@uni.kn>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "String2DateTimeFilter.h"
#include <QStringList>
#include "backend/lib/XmlStreamReader.h"
#include <QUndoCommand>
#include <QDateTime>
#include <QTime>
#include <QDate>

#include <KLocalizedString>

class String2DateTimeFilterSetFormatCmd : public QUndoCommand {

public:
	String2DateTimeFilterSetFormatCmd(String2DateTimeFilter* target, const QString &new_format);

	void redo() override;
	void undo() override;

private:
	String2DateTimeFilter* m_target;
	QString m_other_format;
};

AbstractColumn::ColumnMode String2DateTimeFilter::columnMode() const {
	return AbstractColumn::ColumnMode::DateTime;
}

QDateTime String2DateTimeFilter::dateTimeAt(int row) const {
	if (!m_inputs.value(0)) return QDateTime();
	QString input_value = m_inputs.value(0)->textAt(row);
	if (input_value.isEmpty()) return QDateTime();

	// first try the selected format string m_format
	QDateTime result = QDateTime::fromString(input_value, m_format);
	if (result.isValid())
		return result;

	// fallback:
	// try other format strings built from date_formats and time_formats
	// comma and space are valid separators between date and time
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
	QStringList strings = input_value.simplified().split(',', Qt::SkipEmptyParts);
	if (strings.size() == 1) strings = strings.at(0).split(' ', Qt::SkipEmptyParts);
#else
	QStringList strings = input_value.simplified().split(',', QString::SkipEmptyParts);
	if (strings.size() == 1) strings = strings.at(0).split(' ', QString::SkipEmptyParts);
#endif

	if (strings.size() < 1)
		return result; // invalid date/time from first attempt

	QDate date_result;
	QTime time_result;

	QString date_string = strings.at(0).trimmed();
	QString time_string;
	if (strings.size() > 1)
		time_string = strings.at(1).trimmed();
	else
		time_string = date_string;

	// try to find a valid date
	for (const auto& format : AbstractColumn::dateFormats()) {
		date_result = QDate::fromString(date_string, format);
		if (date_result.isValid())
			break;

	}
	// try to find a valid time
	for (const auto& format : AbstractColumn::timeFormats()) {
		time_result = QTime::fromString(time_string, format);
		if (time_result.isValid())
			break;
	}

	if (!date_result.isValid() && time_result.isValid())
		date_result.setDate(1900,1,1);	// this is what QDateTime does e.g. for QDateTime::fromString("00:00","hh:mm");
	else if (date_result.isValid() && !time_result.isValid())
		time_result = QTime(0, 0, 0, 0);

	return QDateTime(date_result, time_result);
}

QDate String2DateTimeFilter::dateAt(int row) const {
	return dateTimeAt(row).date();
}

QTime String2DateTimeFilter::timeAt(int row) const {
	return dateTimeAt(row).time();
}

bool String2DateTimeFilter::inputAcceptable(int, const AbstractColumn* source) {
	return source->columnMode() == AbstractColumn::ColumnMode::Text;
}

void String2DateTimeFilter::writeExtraAttributes(QXmlStreamWriter* writer) const {
	writer->writeAttribute("format", format());
}

bool String2DateTimeFilter::load(XmlStreamReader* reader, bool preview) {
	if (preview)
		return true;

	QXmlStreamAttributes attribs = reader->attributes();
	QString str = attribs.value(reader->namespaceUri().toString(), "format").toString();

	if (AbstractSimpleFilter::load(reader, preview))
		setFormat(str);
	else
		return false;

	return !reader->hasError();
}

void String2DateTimeFilter::setFormat(const QString& format) {
	exec(new String2DateTimeFilterSetFormatCmd(this, format));
}

String2DateTimeFilterSetFormatCmd::String2DateTimeFilterSetFormatCmd(String2DateTimeFilter* target, const QString &new_format)
	: m_target(target), m_other_format(new_format) {
	if (m_target->parentAspect())
		setText(i18n("%1: set date-time format to %2", m_target->parentAspect()->name(), new_format));
	else
		setText(i18n("set date-time format to %1", new_format));
}

void String2DateTimeFilterSetFormatCmd::redo() {
	QString tmp = m_target->m_format;
	m_target->m_format = m_other_format;
	m_other_format = tmp;
	emit m_target->formatChanged();
}

void String2DateTimeFilterSetFormatCmd::undo() {
	redo();
}

