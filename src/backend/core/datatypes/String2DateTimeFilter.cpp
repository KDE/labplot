/***************************************************************************
    File                 : String2DateTimeFilter.cpp
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2007 by Tilman Benkert,
                           Knut Franke
    Email (use @ for *)  : thzs*gmx.net, knut.franke*gmx.de
    Description          : Conversion filter QString -> QDateTime.
                           
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/
#include "String2DateTimeFilter.h"
#include <QStringList>
#include "lib/XmlStreamReader.h"
#include <QXmlStreamWriter>

const char * String2DateTimeFilter::date_formats[] = {
	"yyyy-M-d", // ISO 8601 w/ and w/o leading zeros
	"yyyy/M/d", 
	"d/M/yyyy", // European style day/month order (this order seems to be used in more countries than the US style M/d/yyyy)
	"d/M/yy", 
	"d-M-yyyy",
	"d-M-yy",
	"d.M.yyyy", // German style
	"d.M.yy",
	"M/yyyy",
	"d.M.", // German form w/o year
	"yyyyMMdd",
	0
};

const char * String2DateTimeFilter::time_formats[] = {
	"h",
	"h ap",
	"h:mm",
	"h:mm ap",
	"h:mm:ss",
	"h:mm:ss.zzz",
	"h:mm:ss:zzz",
	"mm:ss.zzz",
	"hmmss",
	0
};

QDateTime String2DateTimeFilter::dateTimeAt(int row) const
{
	if (!m_inputs.value(0)) return QDateTime();
	QString input_value = m_inputs.value(0)->textAt(row);

	// first try the selected format string m_format
	QDateTime result = QDateTime::fromString(input_value, m_format);
	if(result.date().isValid() || result.time().isValid())
		return result;

	// fallback:
	// try other format strings built from date_formats and time_formats
	// comma and space are valid separators between date and time
	QStringList strings = input_value.simplified().split(",", QString::SkipEmptyParts);
	if(strings.size() == 1) strings = strings.at(0).split(" ", QString::SkipEmptyParts);

	if(strings.size() < 1)
		return result; // invalid date/time from first attempt

	QDate date_result;
	QTime time_result;

	QString date_string = strings.at(0).trimmed();
	QString time_string;
	if(strings.size() > 1)
		time_string = strings.at(1).trimmed();
	else
		time_string = date_string;

	// try to find a valid date
	for (const char **format = date_formats; *format != 0; format++) {
		date_result = QDate::fromString(date_string, *format);
		if (date_result.isValid())
			break;
	}
	// try to find a valid time
	for (const char **format = time_formats; *format != 0; format++) {
		time_result = QTime::fromString(time_string, *format);
		if (time_result.isValid())
			break;
	}

	if(!date_result.isValid())
		date_result.setDate(1900,1,1);	// this is what QDateTime does e.g. for
													// QDateTime::fromString("00:00","hh:mm");
	return QDateTime(date_result, time_result);
}

void String2DateTimeFilter::writeExtraAttributes(QXmlStreamWriter * writer) const
{
	writer->writeAttribute("format", format());
}

bool String2DateTimeFilter::load(XmlStreamReader * reader)
{
	QXmlStreamAttributes attribs = reader->attributes();
	QString str = attribs.value(reader->namespaceUri().toString(), "format").toString();

	if (AbstractSimpleFilter::load(reader))
		setFormat(str);
	else
		return false;

	return !reader->hasError();
}

void String2DateTimeFilter::setFormat(const QString& format) 
{ 
	exec(new String2DateTimeFilterSetFormatCmd(this, format));
}

String2DateTimeFilterSetFormatCmd::String2DateTimeFilterSetFormatCmd(String2DateTimeFilter* target, const QString &new_format)
	: m_target(target), m_other_format(new_format) 
{
	if(m_target->parentAspect())
		setText(QObject::tr("%1: set date-time format to %2").arg(m_target->parentAspect()->name()).arg(new_format));
	else
		setText(QObject::tr("set date-time format to %1").arg(new_format));
}

void String2DateTimeFilterSetFormatCmd::redo() 
{
	QString tmp = m_target->m_format;
	m_target->m_format = m_other_format;
	m_other_format = tmp;
	emit m_target->formatChanged();
}

void String2DateTimeFilterSetFormatCmd::undo() 
{ 
	redo(); 
}

