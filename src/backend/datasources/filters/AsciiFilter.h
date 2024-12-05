/*
	File                 : AsciiFilter.h
	Project              : LabPlot
	Description          : ASCII I/O-filter
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Martin Marmsoler <martin.marmsoler@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

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
	bool open(OpenMode mode) override;
	qint64 readData(char*, qint64) override;
	qint64 writeData(const char*, qint64) override;
	bool atEnd() const override;

private:
	const QByteArray& m_message;
	int m_index{0};
};

class AsciiFilter : public AbstractFileFilter {
public:
	AsciiFilter();
	~AsciiFilter();

	enum class Status {
		Success,
		UnableToOpenDevice,
		DeviceAtEnd,
		NotEnoughRowsSelected,
		NoNewLine,
		SeparatorDeterminationFailed,
		SequentialDeviceHeaderEnabled,
		SequentialDeviceAutomaticSeparatorDetection,
		SequentialDeviceNoColumnModes,
		InvalidNumberDataColumns,
		InvalidNumberColumnNames,
		NotEnoughMemory,
		UnsupportedDataSource,
		UnableParsingHeader,
		MatrixUnsupportedColumnMode,
		NoDateTimeFormat,
		HeaderDetectionNotAllowed,
		SeparatorDetectionNotAllowed,
		InvalidSeparator,
		SequentialDeviceUninitialized,
		NoColumns,
		ColumnModeDeterminationFailed,
		WrongEndColumn,
		WrongEndRow,
		UTF16NotSupported,
		NoDataSource
	};

	void setDataSource(AbstractDataSource* dataSource);
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
	static QString statusToString(Status e);

	static QString autoSeparatorDetectionString();
	static QStringList separatorCharacters();
	static QStringList commentCharacters();

	static QString fileInfoString(const QString&);
	static int columnNumber(const QString& fileName, const QString& separator = QString());
	static size_t lineCount(const QString& fileName, size_t maxLines = std::numeric_limits<std::size_t>::max());

	static QStringList dataTypesString();
	static QPair<QString, QString> dataTypeString(const AbstractColumn::ColumnMode mode);
	static bool determineColumnModes(const QStringView& s, QVector<AbstractColumn::ColumnMode>& modes, QString& invalidString);

	// save/load in the project XML
	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*) override;

	// static QString fileInfoString(const QString&);
	// static int columnNumber(const QString& fileName, const QString& separator = QString());
	// static size_t lineNumber(const QString& fileName, size_t maxLines = std::numeric_limits<std::size_t>::max());

	// void readFromLiveDevice();

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
		QString columnNamesRaw; // String from the input dialog. From those columnNames is retrieved
		QStringList columnNames;
		QVector<AbstractColumn::ColumnMode> columnModes;
		QString columnModesString;

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

		bool isDirty() {
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
};

#endif // ASCIIFILTER_H
