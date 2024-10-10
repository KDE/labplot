/*
	File                 : AsciiFilter.h
	Project              : LabPlot
	Description          : ASCII I/O-filter
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Martin Marmsoler <martin.marmsoler@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "backend/datasources/filters/AbstractFileFilter.h"
#include <memory>

#ifndef ASCIIFILTER_H
#define ASCIIFILTER_H

class AsciiFilterPrivate;

class AsciiFilter: public AbstractFileFilter {
public:
	AsciiFilter();

	enum class Status { Success, UnableToOpenDevice, DeviceAtEnd, NotEnoughRowsSelected, UnableToReadLine, SeparatorDeterminationFailed,
						SequentialDeviceHeaderEnabled, SequentialDeviceAutomaticSeparatorDetection, SequentialDeviceNoColumnModes, InvalidNumberDataColumns,
						NotEnoughMemory, UnsupportedDataSource, UnableParsingHeader, MatrixUnsupportedColumnMode };

	void readDataFromFile(const QString& fileName, AbstractDataSource* = nullptr, ImportMode = ImportMode::Replace) override;
	qint64 readFromDevice(QIODevice& device, AbstractDataSource* dataSource, ImportMode importMode, int lines);
	void write(const QString& fileName, AbstractDataSource*) override;
	QVector<QStringList> preview(const QString& fileName, int lines);
	static QString statusToString(Status e);

	static QString autoSeparatorDetectionString();
	static QStringList separatorCharacters();
	static QStringList commentCharacters();

	static QString fileInfoString(const QString&);
	static int columnNumber(const QString& fileName, const QString& separator = QString());
	static size_t lineNumber(const QString& fileName, size_t maxLines = std::numeric_limits<std::size_t>::max());


	// save/load in the project XML
	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*) override;

	// static QString fileInfoString(const QString&);
	// static int columnNumber(const QString& fileName, const QString& separator = QString());
	// static size_t lineNumber(const QString& fileName, size_t maxLines = std::numeric_limits<std::size_t>::max());

	//void readFromLiveDevice();

	struct Properties {
		QString commentCharacter{QStringLiteral("#")};
		int baseYear{1900}; // Base year when parsing datetime with only 2 year digits
		QString dateTimeFormat;
		QLocale::Language numberFormat{QLocale::C};
		QLocale locale{QLocale::C};
		bool skipEmptyParts{false};
		bool simplifyWhitespacesEnabled{false};
		double nanValue{qQNaN()};
		bool removeQuotesEnabled{false};
		bool createIndexEnabled{false};
		bool createTimestampEnabled{false};

		bool headerEnabled{true}; // read header from file
		int headerLine{1}; // line to read header from (ignoring comment lines). The data starts below this line
		QString columnNamesRaw; // String from the input dialog. From those columnNames is retrieved
		QStringList columnNames;
		QVector<AbstractColumn::ColumnMode> columnModes;

		int startRow{1}; // Start row. If headerEnabled, it is the offset from that line
		int numberRows{-1}; // number of rows to read. A negative value means the complete file, ...
		int startColumn{1}; // Start Column to read
		// number of columns to read. A negative value means the complete file, ...
		// The index and timestamp columns are not included in this number!
		int numberColumns{-1};
		int mqttPreviewFirstEmptyColCount{0};

		bool intAsDouble{true}; // Interpret all integer values as doubles

		bool automaticSeparatorDetection{true};
		QString separator{QStringLiteral(",")};

		int m_columnOffset{0}; // indexes the "start column" in the datasource. Data will be imported starting from this column.

		void set_invalid() {
			m_dirty = true;
		}

		bool isDirty() {
			return m_dirty;
		}

	private:
		bool m_dirty{true};
	};
	Properties properties() const;
	void setProperties(Properties& p);
	QStringList columnNames() const;
	QVector<AbstractColumn::ColumnMode> columnModes() const;



private:
	typedef AsciiFilterPrivate Private;

	Q_DECLARE_PRIVATE(AsciiFilter)
	const std::unique_ptr<Private> d_ptr;

};

#endif // ASCIIFILTER_H
