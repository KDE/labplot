/***************************************************************************
    File                 : DateTime2StringFilter.cpp
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2007 by Tilman Benkert (thzs@gmx.net)
    Copyright            : (C) 2007 by Knut Franke (knut.franke@gmx.de)
    Description          : Conversion filter QDateTime -> QString.

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

#include "DateTime2StringFilter.h"
#include "backend/lib/XmlStreamReader.h"
#include <QDateTime>
#include <QRegExp>
#include <QUndoCommand>

#include <KLocale>

class DateTime2StringFilterSetFormatCmd : public QUndoCommand
{
	public:
		DateTime2StringFilterSetFormatCmd(DateTime2StringFilter* target, const QString &new_format);

		virtual void redo();
		virtual void undo();

	private:
		DateTime2StringFilter* m_target;
		QString m_other_format;
};

void DateTime2StringFilter::setFormat(const QString& format)
{
	exec(new DateTime2StringFilterSetFormatCmd(static_cast<DateTime2StringFilter*>(this), format));
}

QString DateTime2StringFilter::textAt(int row) const {
	if (!m_inputs.value(0)) return QString();
	QDateTime input_value = m_inputs.value(0)->dateTimeAt(row);
	if (!input_value.isValid()) return QString();
#if QT_VERSION < 0x040302 // the bug seems to be fixed in Qt 4.3.2
	// QDate::toString produces shortened year numbers for "yyyy"
	// in violation of ISO 8601 and ambiguous with respect to "yy" format
	QString format(m_format);
	format.replace("yyyy","YYYYyyyyYYYY");
	QString result = input_value.toString(format);
	result.replace(QRegExp("YYYY(-)?(\\d\\d\\d\\d)YYYY"), "\\1\\2");
	result.replace(QRegExp("YYYY(-)?(\\d\\d\\d)YYYY"), "\\10\\2");
	result.replace(QRegExp("YYYY(-)?(\\d\\d)YYYY"), "\\100\\2");
	result.replace(QRegExp("YYYY(-)?(\\d)YYYY"), "\\1000\\2");
	return result;
#else
	return input_value.toString(m_format);
#endif
}

bool DateTime2StringFilter::inputAcceptable(int, const AbstractColumn *source) {
	return (source->columnMode() == AbstractColumn::DateTime)
		|| (source->columnMode() == AbstractColumn::Day)
		|| (source->columnMode() == AbstractColumn::Month);
}

DateTime2StringFilterSetFormatCmd::DateTime2StringFilterSetFormatCmd(DateTime2StringFilter* target, const QString &new_format)
	: m_target(target), m_other_format(new_format)
{
	if(m_target->parentAspect())
		setText(i18n("%1: set date-time format to %2", m_target->parentAspect()->name(), new_format));
	else
		setText(i18n("set date-time format to %1", new_format));
}

void DateTime2StringFilterSetFormatCmd::redo()
{
	QString tmp = m_target->m_format;
	m_target->m_format = m_other_format;
	m_other_format = tmp;
	emit m_target->formatChanged();
}

void DateTime2StringFilterSetFormatCmd::undo()
{
	redo();
}

void DateTime2StringFilter::writeExtraAttributes(QXmlStreamWriter * writer) const
{
	writer->writeAttribute("format", format());
}

bool DateTime2StringFilter::load(XmlStreamReader * reader)
{
	QXmlStreamAttributes attribs = reader->attributes();
	QString str = attribs.value(reader->namespaceUri().toString(), "format").toString();

	if (AbstractSimpleFilter::load(reader))
		setFormat(str);
	else
		return false;

	return !reader->hasError();
}

