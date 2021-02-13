/***************************************************************************
    File                 : XmlStreamReader.h
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

#ifndef XML_STREAM_READER_H
#define XML_STREAM_READER_H

#include <QXmlStreamReader>

class QString;
class QStringList;

class XmlStreamReader : public QXmlStreamReader {
public:
	XmlStreamReader();
	explicit XmlStreamReader(QIODevice* device);
	explicit XmlStreamReader(const QByteArray& data);
	explicit XmlStreamReader(const QString& data);
	explicit XmlStreamReader(const char* data);

	const QStringList& warningStrings() const;
	QString missingCASWarning() const;
	bool hasWarnings() const;
	bool hasMissingCASWarnings() const;
	void raiseWarning(const QString&);
	void raiseMissingCASWarning(const QString&);
	void raiseError(const QString&);

	bool skipToNextTag();
	bool skipToEndElement();
	int readAttributeInt(const QString& name, bool* ok);

private:
	QStringList m_warnings;
	QStringList m_missingCASPlugins;
	void init();
};

#endif // XML_STREAM_READER_H
