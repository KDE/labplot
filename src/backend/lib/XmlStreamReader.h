/*
    File                 : XmlStreamReader.h
    Project              : LabPlot
    Description          : XML stream parser that supports errors and warnings
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2009 Tilman Benkert (thzs@gmx.net)
    SPDX-FileCopyrightText: 2015-2016 Alexander Semke (alexander.semke@web.de)

    SPDX-License-Identifier: GPL-2.0-or-later
*/

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

	void setFailedCASMissing(bool);
	bool failedCASMissing() const;

	void raiseWarning(const QString&);
	void raiseMissingCASWarning(const QString&);
	void raiseError(const QString&);

	bool skipToNextTag();
	bool skipToEndElement();
	int readAttributeInt(const QString& name, bool* ok);

private:
	QStringList m_warnings;
	QStringList m_missingCASPlugins;
	bool m_failedCASMissing{false};
	void init();
};

#endif // XML_STREAM_READER_H
