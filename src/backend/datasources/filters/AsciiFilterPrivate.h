/*
	File                 : AsciiFilterPrivate.h
	Project              : LabPlot
	Description          : Private implementation class for AsciiFilter.
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2009-2020 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2017-2022 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ASCIIFILTERPRIVATE_H
#define ASCIIFILTERPRIVATE_H

class AbstractDataSource;
class AbstractColumn;
class AbstractAspect;
class Spreadsheet;
class MQTTTopic;

class AsciiFilterPrivate {
public:
	explicit AsciiFilterPrivate(AsciiFilter*);

	int isPrepared();
	QString separator() const;

	// preview
	QVector<QStringList> preview(const QString& fileName, int lines);
	QVector<QStringList> preview(QIODevice&);

	// read
	void
	readDataFromDevice(QIODevice&, AbstractDataSource* = nullptr, AbstractFileFilter::ImportMode = AbstractFileFilter::ImportMode::Replace, int lines = -1);
	void readFromLiveDeviceNotFile(QIODevice&, AbstractDataSource*, AbstractFileFilter::ImportMode = AbstractFileFilter::ImportMode::Replace);
	qint64 readFromLiveDevice(QIODevice&, AbstractDataSource*, qint64 from = -1);
	void readDataFromFile(const QString& fileName, AbstractDataSource* = nullptr, AbstractFileFilter::ImportMode = AbstractFileFilter::ImportMode::Replace);

	// write
	void write(const QString& fileName, AbstractDataSource*);

	// helpers
	int prepareDeviceToRead(QIODevice&, size_t maxLines = std::numeric_limits<std::size_t>::max());
	void initDataContainer(Spreadsheet*);
	QString previewValue(const QString&, AbstractColumn::ColumnMode);
	void setValue(int col, int row, QStringView value);
	QString getLine(QIODevice&);
	QStringList getLineString(QIODevice&);

#ifdef HAVE_MQTT
	int prepareToRead(const QString&);
	QVector<QStringList> preview(const QString& message);
	AbstractColumn::ColumnMode MQTTColumnMode() const;
	QString MQTTColumnStatistics(const MQTTTopic*) const;
	void readMQTTTopic(const QString& message, AbstractDataSource*);
	void setPreparedForMQTT(bool, MQTTTopic*, const QString&);
#endif

	AsciiFilter* const q;

	QString commentCharacter{QStringLiteral("#")};
	QString separatingCharacter{QStringLiteral("auto")};
	QString dateTimeFormat;
	QLocale::Language numberFormat{QLocale::C};
	QLocale locale{QLocale::C};
	bool autoModeEnabled{true};
	bool headerEnabled{true}; // read header from file
	int headerLine{1}; // line to read header from
	bool skipEmptyParts{false};
	bool simplifyWhitespacesEnabled{false};
	double nanValue{qQNaN()};
	bool removeQuotesEnabled{false};
	bool createIndexEnabled{false};
	bool createTimestampEnabled{false};
	QStringList columnNames;
	QVector<AbstractColumn::ColumnMode> columnModes;
	int startRow{1};
	int endRow{-1};
	int startColumn{1};
	int endColumn{-1};
	int mqttPreviewFirstEmptyColCount{0};

	// TODO: redesign and remove this later
	bool readingFile{false};
	QString readingFileName;

private:
	static const unsigned int m_dataTypeLines = 10; // maximum lines to read for determining data types
	QString m_separator;
	int m_actualStartRow{1};
	int m_actualRows{0};
	int m_actualCols{0};
	bool m_prepared{false};
	bool m_firstRead{true};
	int m_columnOffset{0}; // indexes the "start column" in the datasource. Data will be imported starting from this column.
	std::vector<void*> m_dataContainer; // pointers to the actual data containers

	QStringList split(const QString&, bool autoSeparator = true);
	QDateTime parseDateTime(const QString& string, const QString& format);
};

#endif
