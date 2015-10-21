/***************************************************************************
    File                 : XmlStreamReader.h
    Project           : LabPlot
    Description    : XML stream parser that supports errors as well as warnings
    --------------------------------------------------------------------
    Copyright            : (C) 2009 Tilman Benkert (thzs@gmx.net)
    Copyright            : (C) 2015 Alexander Semke (alexander.semke@web.de)
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

#ifndef XML_STREAM_READER_H
#define XML_STREAM_READER_H

#include <QXmlStreamReader>
#include <QString>
#include <QStringList>
#include "backend/lib/macros.h"

class XmlStreamReader : public QXmlStreamReader {
	public:
		XmlStreamReader();
		explicit XmlStreamReader(QIODevice* device);
		explicit XmlStreamReader(const QByteArray& data);
		explicit XmlStreamReader(const QString& data);
		explicit XmlStreamReader(const char* data);

		QStringList warningStrings() const;
		bool hasWarnings() const;
		void raiseWarning(const QString & message = QString());
		void raiseError(const QString & message = QString());
		CLASS_ACCESSOR(QString, m_error_prefix, errorPrefix, ErrorPrefix);
		CLASS_ACCESSOR(QString, m_error_postfix, errorPostfix, ErrorPostfix);
		CLASS_ACCESSOR(QString, m_warning_prefix, warningPrefix, WarningPrefix);
		CLASS_ACCESSOR(QString, m_warning_postfix, warningPostfix, WarningPostfix);


		bool skipToNextTag();
		bool skipToEndElement();
		int readAttributeInt(const QString& name, bool* ok);

	private:
		QStringList m_warnings;
		QString m_error_prefix;
		QString m_error_postfix;
		QString m_warning_prefix;
		QString m_warning_postfix;

		void init();
};

#endif // XML_STREAM_READER_H
