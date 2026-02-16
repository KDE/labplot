/*
	File                 : AsciiFilter.h
	Project              : LabPlot
	Description          : ASCII I/O-filter
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Martin Marmsoler <martin.marmsoler@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "FilterStatus.h"
#include "backend/datasources/filters/AbstractFileFilter.h"
#include <QIODevice>
#include <memory>

#ifndef ASCIIFILTER_H
#define ASCIIFILTER_H

class AsciiFilterPrivate;

/*!
 * \brief The BufferReader class
 * Simulate a qiodevice to expose a string to the filter
 */
class BufferReader : public QIODevice {
public:
	BufferReader(const QByteArray& buffer);

	bool isSequential() const override;
	bool open(OpenMode) override;
	qint64 readData(char*, qint64) override;
	qint64 writeData(const char*, qint64) override;
	bool atEnd() const override;

private:
	const QByteArray& m_message;
	int m_index{0};
};

#ifdef SDK
#include "labplot_export.h"
class LABPLOT_EXPORT AsciiFilter : public AbstractFileFilter {
#else
class AsciiFilter : public AbstractFileFilter {
#endif
public:
	// enum for data types that are supported during the import, not necesserily the same as
	// the column modes AbstractColumn::ColumnMode, mapped intternally to those.
	enum class DataType {
		Double = 0,
		Integer = 1,
		BigInt = 2,
		Text = 3,
		DateTime = 4,
		TimestampUnix = 5, // qint64, number of seconds since January 1, 1970 (UTC), converted to DateTime until we have an qin64 Timestamp
		TimestampWindows = 6 // qint64, number of 100-nanosecond intervals since January 1, 1601 (UTC), converted to DateTime until we have an qin64 Timestamp
	};

	AsciiFilter();
	~AsciiFilter();

	void setDataSource(AbstractDataSource*);
	void readDataFromFile(const QString& fileName, AbstractDataSource* = nullptr, ImportMode columnImportMode = ImportMode::Replace) override;
	qint64 readFromDevice(QIODevice& device,
						  ImportMode columnImportMode,
						  ImportMode rowImportMode,
						  qint64 from,
						  qint64 lines,
						  qint64 keepNRows = 0,
						  bool skipFirstLine = false);
	void write(const QString& fileName, AbstractDataSource*) override;
	QVector<QStringList> preview(QIODevice& device, int lines, bool reinit = true, bool skipFirstLine = false);
	QVector<QStringList> preview(const QString& fileName, int lines, bool reinit = true);

	static QString autoSeparatorDetectionString();
	static QStringList separatorCharacters();
	static QStringList commentCharacters();

	static QString fileInfoString(const QString&);
	static int columnNumber(const QString& fileName, const QString& separator = QString());
	static size_t lineCount(const QString& fileName, size_t maxLines = std::numeric_limits<std::size_t>::max());

	static QPair<QString, QString> dataTypeString(const DataType);
	static AbstractColumn::ColumnMode dataTypeToColumnMode(DataType);
	static DataType columnModeToDataType(AbstractColumn::ColumnMode);
	static bool validateDataTypes(const QStringView& s, QVector<DataType>&, QString& invalidString);

	// save/load in the project XML
	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*) override;

	struct Properties {
		QString commentCharacter{QStringLiteral("#")};
		int baseYear{1900}; // Base year when parsing datetime with only 2 year digits
		QString dateTimeFormat{
			QStringLiteral("yyyy-MM-dd hh:mm:ss")}; // ISO 8601 do not let it empty, because in automatic mode the initialization is really slow
		QLocale locale{QLocale::C};
		bool skipEmptyParts{false};
		bool simplifyWhitespaces{false};
		double nanValue{qQNaN()};
		bool removeQuotes{false};
		bool createIndex{false};
		bool createTimestamp{false};

		bool headerEnabled{true}; // read header from file
		int headerLine{1}; // line to read header from (ignoring comment lines). The data starts below this line

		QString columnNamesString; // String from the input dialog, parsed to determine the column names below
		QStringList columnNames; // the list of column names for each column

		QString dataTypesString; // String from the input dialog, parsed to determine the data types below
		QVector<DataType> dataTypes; // the list of data types for each column
		QVector<AbstractColumn::ColumnMode> columnModes; // column modes for each column, determined from the list of provided data types

		int startRow{1}; // Start row. If headerEnabled, it is the offset from that line
		int endRow{-1}; // Last row to read. A negative value means the complete file
		int startColumn{1}; // Start Column to read
		// number of columns to read. A negative value means the complete file, ...
		// The index and timestamp columns are not included in this number!
		int endColumn{-1}; // Last column to read. If negative all available shall be read
		int mqttPreviewFirstEmptyColCount{0};

		bool intAsDouble{true}; // Interpret all integer values as doubles

		bool automaticSeparatorDetection{true};
		QString separator{QStringLiteral(",")};

		void set_invalid() {
			m_dirty = true;
		}

		bool isDirty() const {
			return m_dirty;
		}

	private:
		bool m_dirty{true};
	};

	Status initialize(Properties p);
	bool initialized() const;
	Properties properties() const;
	Properties defaultProperties() const;
	void setProperties(Properties& p);
	QStringList columnNames() const;
	QVector<AbstractColumn::ColumnMode> columnModes() const;

private:
	typedef AsciiFilterPrivate Private;

	Q_DECLARE_PRIVATE(AsciiFilter)
	const std::unique_ptr<Private> d_ptr;

	friend class AsciiFilterTest;
};

#endif // ASCIIFILTER_H
