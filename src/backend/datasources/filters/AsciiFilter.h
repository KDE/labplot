/*
    File                 : AsciiFilter.h
    Project              : LabPlot
    Description          : ASCII I/O-filter
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2009-2022 Alexander Semke <alexander.semke@web.de>
    SPDX-FileCopyrightText: 2017 Stefan Gerlach <stefan.gerlach@uni.kn>
    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef ASCIIFILTER_H
#define ASCIIFILTER_H

#include "backend/datasources/filters/AbstractFileFilter.h"
#include "backend/core/AbstractColumn.h"

class Spreadsheet;
class QStringList;
class QIODevice;
class AsciiFilterPrivate;
class QAbstractSocket;
class MQTTTopic;
class MQTTClient;

class AsciiFilter : public AbstractFileFilter {
	Q_OBJECT

public:
	AsciiFilter();
	~AsciiFilter() override;

	static QStringList separatorCharacters();
	static QStringList commentCharacters();
	static QStringList dataTypes();
	static QStringList predefinedFilters();

	static QString fileInfoString(const QString&);
	static int columnNumber(const QString& fileName, const QString& separator = QString());
	static size_t lineNumber(const QString& fileName);
	size_t lineNumber(QIODevice&) const;	// calculate number of lines if device supports it

	// read data from any device
	void readDataFromDevice(QIODevice& device, AbstractDataSource*,
	                        AbstractFileFilter::ImportMode = AbstractFileFilter::ImportMode::Replace, int lines = -1);
	void readFromLiveDeviceNotFile(QIODevice& device, AbstractDataSource*dataSource);
	qint64 readFromLiveDevice(QIODevice& device, AbstractDataSource*, qint64 from = -1);
	// overloaded function to read from file
	void readDataFromFile(const QString& fileName, AbstractDataSource* = nullptr,
	                      AbstractFileFilter::ImportMode = AbstractFileFilter::ImportMode::Replace) override;
	void write(const QString& fileName, AbstractDataSource*) override;

	QVector<QStringList> preview(const QString& fileName, int lines);
	QVector<QStringList> preview(QIODevice& device);

	void loadFilterSettings(const QString&) override;
	void saveFilterSettings(const QString&) const override;

#ifdef HAVE_MQTT
	QVector<QStringList> preview(const QString& message);
	void readMQTTTopic(const QString& message, AbstractDataSource*);
	void setPreparedForMQTT(bool, MQTTTopic*, const QString&);
#endif

	QString separator() const;
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
	void setHeaderLine(int);
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
	void setCreateTimestampEnabled(const bool);
	bool createTimestampEnabled() const;

	void setVectorNames(const QString&);
	void setVectorNames(const QStringList&);
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
