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

#ifndef ASCIIFILTERNEW_H
#define ASCIIFILTERNEW_H

class AsciiFilterNewPrivate;

class AsciiFilterNew: public AbstractFileFilter {
public:
	AsciiFilterNew();

	enum class Status { Success, UnableToOpenDevice, DeviceAtEnd, NotEnoughRowsSelected, UnableToReadLine };

	void initialize(QIODevice &device);
	void readDataFromFile(const QString& fileName, AbstractDataSource* = nullptr, ImportMode = ImportMode::Replace) override;
	qint64 readFromDevice(QIODevice& device, AbstractDataSource* dataSource, ImportMode importMode, int lines);
	void write(const QString& fileName, AbstractDataSource*) override;
	QVector<QStringList> preview(const QString& fileName, int lines);
	static QString statusToString(Status e);

	// static QString fileInfoString(const QString&);
	// static int columnNumber(const QString& fileName, const QString& separator = QString());
	// static size_t lineNumber(const QString& fileName, size_t maxLines = std::numeric_limits<std::size_t>::max());

	//void readFromLiveDevice();

	struct Properties {
		QString commentCharacter{QStringLiteral("#")};
		QString dateTimeFormat;
		QLocale::Language numberFormat{QLocale::C};
		QLocale locale{QLocale::C};
		bool headerEnabled{true}; // read header from file
		int headerLine{1}; // line to read header from (ignoring comment lines). The data starts below this line
		bool skipEmptyParts{false};
		bool simplifyWhitespacesEnabled{false};
		double nanValue{qQNaN()};
		bool removeQuotesEnabled{false};
		bool createIndexEnabled{false};
		bool createTimestampEnabled{false};
		QStringList columnNames;
		QVector<AbstractColumn::ColumnMode> columnModes;
		int startRow{1}; // Start row. If headerEnabled, it is the offset from that line
		int numberRows{-1}; // number of rows to read. A negative value means the complete file, ...
		int startColumn{1}; // Start Column to read
		int numberColumns{-1}; // number of columns to read. A negative value means the complete file, ...
		int mqttPreviewFirstEmptyColCount{0};

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
	void setProperties(Properties& p);



private:
	typedef AsciiFilterNewPrivate Private;

	Q_DECLARE_PRIVATE(AsciiFilterNew)
	const std::unique_ptr<Private> d_ptr;

};

#endif // ASCIIFILTERNEW_H
