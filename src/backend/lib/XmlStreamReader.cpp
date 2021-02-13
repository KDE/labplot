/***************************************************************************
    File                 : XmlStreamReader.cpp
    Project              : LabPlot
    Description          : XML stream parser that supports errors and warnings
    --------------------------------------------------------------------
    Copyright            : (C) 2009 Tilman Benkert (thzs@gmx.net)
    Copyright            : (C) 2015-2016 Alexander Semke (alexander.semke@web.de)
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
#include <KLocalizedString>

/**
 * \class XmlStreamReader
 * \brief XML stream parser that supports errors as well as warnings.
 * This class also adds line and column numbers to the error message.
 */
XmlStreamReader::XmlStreamReader() = default;

XmlStreamReader::XmlStreamReader(QIODevice* device) : QXmlStreamReader(device) {
}

XmlStreamReader::XmlStreamReader(const QByteArray& data) : QXmlStreamReader(data) {
}

XmlStreamReader::XmlStreamReader(const QString& data) : QXmlStreamReader(data) {
}

XmlStreamReader::XmlStreamReader(const char* data) : QXmlStreamReader(data) {
}

const QStringList& XmlStreamReader::warningStrings() const {
	return m_warnings;
}

/*
 * returns the human readable string for the missing CAS plugins in case
 * the project has some CAS content but the application was either not compiled with
 * CAS/Cantor support or the correspongind plugins for the required backends are missing.
 *
 * The returned text is in the form "Octave" or "Octave and Maxima" or "Octave, Maxima and Python", etc.
 */
QString XmlStreamReader::missingCASWarning() const {
	const int count = m_missingCASPlugins.count();
	if (count == 1)
		return m_missingCASPlugins.constFirst();
	else {
		QString msg;
		for (int i = 0; i < count; ++ i) {
			if (!msg.isEmpty()) {
				if (i == count - 1)
					msg += QLatin1Char(' ') + i18n("and") + QLatin1Char(' ');
				else
					msg += QLatin1String(", ");
			}
			msg += m_missingCASPlugins.at(i);
		}
		return msg;
	}
}

bool XmlStreamReader::hasWarnings() const {
	return !m_warnings.isEmpty();
}

bool XmlStreamReader::hasMissingCASWarnings() const {
	return !m_missingCASPlugins.isEmpty();
}

void XmlStreamReader::raiseError(const QString & message) {
	QXmlStreamReader::raiseError(i18n("line %1, column %2: %3", lineNumber(), columnNumber(), message));
}

void XmlStreamReader::raiseWarning(const QString & message) {
	m_warnings.append(i18n("line %1, column %2: %3", lineNumber(), columnNumber(), message));
}

void XmlStreamReader::raiseMissingCASWarning(const QString& name) {
	m_missingCASPlugins.append(name);
}

/*!
  * Go to the next start or end element tag
  * If the end of the document is reached, an error is raised.
  * \return false if end of document reached, otherwise true
  */
bool XmlStreamReader::skipToNextTag() {
	if (atEnd()) {
		raiseError(i18n("unexpected end of document"));
		return false;
	}

	do {
		readNext();
	} while (!(isStartElement() || isEndElement() || atEnd()));

	if (atEnd()) {
		raiseError(i18n("unexpected end of document"));
		return false;
	}

	return true;
}

/*!
  * Go to the end element tag of the current element
  * If the end of the document is reached, an error is raised.
  * \return false if end of document reached, otherwise true
  */
bool XmlStreamReader::skipToEndElement() {
	int depth = 1;
	if (atEnd()) {
		raiseError(i18n("unexpected end of document"));
		return false;
	}

	do {
		readNext();
		if (isEndElement()) depth--;
		if (isStartElement()) depth++;
	} while (!((isEndElement() && depth == 0) || atEnd()));

	if (atEnd()) {
		raiseError(i18n("unexpected end of document"));
		return false;
	}

	return true;
}

/*!
 * Read an XML attribute and convert it to int
  * \param name attribute name
  * \param ok pointer to report back whether the attribute value could be determined (may be NULL)
  * \return the attribute value if found and converted, otherwise zero (in this case *ok is false)
  */
int XmlStreamReader::readAttributeInt(const QString& name, bool* ok) {
	QString str = attributes().value(namespaceUri().toString(), name).toString();
	if (str.isEmpty()) {
		if (ok) *ok = false;
		return 0;
	}

	return str.toInt(ok);
}
