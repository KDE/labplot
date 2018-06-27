/***************************************************************************
File                 : AsciiFilter.h
Project              : LabPlot
Description          : ASCII I/O-filter
--------------------------------------------------------------------
Copyright            : (C) 2009-2013 Alexander Semke (alexander.semke@web.de)
Copyright            : (C) 2017 Stefan Gerlach (stefan.gerlach@uni.kn)
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
#ifndef ASCIIFILTER_H
#define ASCIIFILTER_H

#include "backend/datasources/filters/AbstractFileFilter.h"
#include "backend/core/AbstractColumn.h"

class Spreadsheet;
class QStringList;
class QIODevice;
class AsciiFilterPrivate;
class QAbstractSocket;
class AsciiFilter : public AbstractFileFilter {
	Q_OBJECT

public:
	AsciiFilter();
	~AsciiFilter() override;

	static QStringList separatorCharacters();
	static QStringList commentCharacters();
	static QStringList dataTypes();
	static QStringList predefinedFilters();

	static int columnNumber(const QString& fileName, const QString& separator = QString());
	static size_t lineNumber(const QString& fileName);
	static size_t lineNumber(QIODevice&);	// calculate number of lines if device supports it

	// read data from any device
	void readDataFromDevice(QIODevice& device, AbstractDataSource*,
	                        AbstractFileFilter::ImportMode = AbstractFileFilter::Replace, int lines = -1);
	void readFromLiveDeviceNotFile(QIODevice& device, AbstractDataSource*dataSource);
	qint64 readFromLiveDevice(QIODevice& device, AbstractDataSource*,
	                          qint64 from = -1);
	// overloaded function to read from file
	QVector<QStringList> readDataFromFile(const QString& fileName, AbstractDataSource* = nullptr,
	                                      AbstractFileFilter::ImportMode = AbstractFileFilter::Replace, int lines = -1) override;
	void write(const QString& fileName, AbstractDataSource*) override;

	QVector<QStringList> preview(const QString& fileName, int lines);
	QVector<QStringList> preview(QIODevice& device);

	void loadFilterSettings(const QString&) override;
	void saveFilterSettings(const QString&) const override;

#ifdef HAVE_MQTT
	void mqttPreview(QVector<QStringList>&, const QString&, const QString&);
	QString mqttColumnStatistics(const Spreadsheet* , AbstractAspect*) const;
	AbstractColumn::ColumnMode mqttColumnMode() const;
	void readMQTTTopic(const QString&, const QString&, AbstractDataSource*dataSource);
	void setPreparedForMQTT(bool, AbstractDataSource*, const QString&);
	QString separator() const;
#endif

	void setCommentCharacter(const QString&);
	QString commentCharacter() const;
	void setSeparatingCharacter(const QString&);
	QString separatingCharacter() const;
	void setDateTimeFormat(const QString&);
	QString dateTimeFormat() const;
	void setNumberFormat(QLocale::Language);
	QLocale::Language numberFormat() const;

	void setAutoModeEnabled(const bool);
	bool isAutoModeEnabled() const;
	void setHeaderEnabled(const bool);
	bool isHeaderEnabled() const;
	void setSkipEmptyParts(const bool);
	bool skipEmptyParts() const;
	void setSimplifyWhitespacesEnabled(const bool);
	bool simplifyWhitespacesEnabled() const;
	void setNaNValueToZero(const bool);
	bool NaNValueToZeroEnabled() const;
	void setRemoveQuotesEnabled(const bool);
	bool removeQuotesEnabled() const;
	void setCreateIndexEnabled(const bool);
	bool createIndexEnabled() const;

	void setVectorNames(const QString&);
	QStringList vectorNames() const;
	QVector<AbstractColumn::ColumnMode> columnModes();

	void setStartRow(const int);
	int startRow() const;
	void setEndRow(const int);
	int endRow() const;
	void setStartColumn(const int);
	int startColumn() const;
	void setEndColumn(const int);
	int endColumn() const;

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*) override;

    int isPrepared();

private:
	std::unique_ptr<AsciiFilterPrivate> const d;
	friend class AsciiFilterPrivate;
};

#endif
