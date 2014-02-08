/***************************************************************************
    File                 : XmlStreamReader.cpp
    Project              : SciDAVis
    Description          : XML stream parser that supports errors as well as warnings
    --------------------------------------------------------------------
    Copyright            : (C) 2008 Tilman Benkert (thzs*gmx.net)
                           (replace * with @ in the email addresses) 
                           
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

#include "backend/lib/XmlStreamReader.h"

#include <KLocale>

XmlStreamReader::XmlStreamReader()
{
	init();
}

XmlStreamReader::XmlStreamReader(QIODevice * device)
: QXmlStreamReader(device)
{
	init();
}

XmlStreamReader::XmlStreamReader(const QByteArray & data)
: QXmlStreamReader(data)
{
	init();
}

XmlStreamReader::XmlStreamReader(const QString & data)
: QXmlStreamReader(data)
{
	init();
}

XmlStreamReader::XmlStreamReader(const char * data)
: QXmlStreamReader(data)
{
	init();
}

void XmlStreamReader::init()
{
	m_error_prefix = i18nc("prefix for XML error messages", "XML reader error: ");
	m_error_postfix = i18nc("postfix for XML error messages", " (loading failed)");
	m_warning_prefix = i18nc("prefix for XML warning messages", "XML reader warning: ");
	m_warning_postfix = i18nc("postfix for XML warning messages", " ");
}

QStringList XmlStreamReader::warningStrings() const
{
	return m_warnings;
}

bool XmlStreamReader::hasWarnings() const
{
	return !(m_warnings.isEmpty());
}

void XmlStreamReader::raiseError(const QString & message)
{
	QString prefix2 = QString(i18n("line %1, column %2: ").arg(lineNumber()).arg(columnNumber()));
	QXmlStreamReader::raiseError(m_error_prefix+prefix2+message+m_error_postfix);
}

void XmlStreamReader::raiseWarning(const QString & message)
{
	QString prefix2 = QString(i18n("line %1, column %2: ").arg(lineNumber()).arg(columnNumber()));
	m_warnings.append(m_warning_prefix+prefix2+message+m_warning_postfix);
}

bool XmlStreamReader::skipToNextTag()
{
	if (atEnd())
	{
		raiseError(i18n("unexpected end of document"));
		return false;
	}
	do {
		readNext();
	} while (!(isStartElement() || isEndElement() || atEnd()));
	if (atEnd())
	{
		raiseError(i18n("unexpected end of document"));
		return false;
	}
	return true;
}

int XmlStreamReader::readAttributeInt(const QString & name, bool * ok)
{
	QString str = attributes().value(namespaceUri().toString(), name).toString();
	if (str.isEmpty())
	{
		if (ok) *ok = false;
		return 0;
	}
	return str.toInt(ok);
}

bool XmlStreamReader::skipToEndElement()
{
	int depth = 1;
	if (atEnd())
	{
		raiseError(i18n("unexpected end of document"));
		return false;
	}
	do {
		readNext();
		if (isEndElement()) depth--;
		if (isStartElement()) depth++; 
	} while (!((isEndElement() && depth == 0) || atEnd()));
	if (atEnd())
	{
		raiseError(i18n("unexpected end of document"));
		return false;
	}
	return true;
}
