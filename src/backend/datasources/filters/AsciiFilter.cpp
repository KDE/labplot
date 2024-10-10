/*
	File                 : AsciiFilter.cpp
	Project              : LabPlot
	Description          : ASCII I/O-filter
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Martin Marmsoler <martin.marmsoler@gmail.com>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "AsciiFilter.h"
#include "AsciiFilterPrivate.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/core/Project.h"
#include <KCompressionDevice>
#include <KLocalizedString>

#include <QDateTime>

namespace {
// Simple object to automatically closing a device when this object
// goes out of scope
struct IODeviceHandler {
	IODeviceHandler(QIODevice& d): device(d) {}

	~IODeviceHandler() {
		device.reset(); // Seek to the start
		device.close();
	}
private:
	QIODevice& device;
};
}

AsciiFilter::AsciiFilter(): AbstractFileFilter(FileType::Ascii), d_ptr(std::make_unique<AsciiFilterPrivate>(this)) {

}

AsciiFilter::Properties AsciiFilter::properties() const {
	Q_D(const AsciiFilter);
	return d->properties;
}

void AsciiFilter::setProperties(Properties& properties) {
	Q_D(AsciiFilter);
	d->initialized = false;
	d->properties = properties;
}

void AsciiFilter::readDataFromFile(const QString& fileName, AbstractDataSource* dataSource, ImportMode importMode) {
	KCompressionDevice file(fileName);
	readFromDevice(file, dataSource, importMode, -1);
}

qint64 AsciiFilter::readFromDevice(QIODevice& device, AbstractDataSource* dataSource, ImportMode importMode, int lines) {
	Q_D(AsciiFilter);
	const auto status = d->readFromDevice(device, dataSource, importMode, lines);
	// TODO: do something with it!
	return 0;
}

void AsciiFilter::write(const QString& fileName, AbstractDataSource*) {
	// TODO
}

QVector<QStringList> AsciiFilter::preview(const QString& fileName, int lines) {
	Q_D(AsciiFilter);
	return d->preview(fileName, lines);
}

QString AsciiFilter::statusToString(Status e) {
	switch (e) {
	case AsciiFilter::Status::Success:
		return i18n("Success");
	case AsciiFilter::Status::DeviceAtEnd:
		return i18n("Device at end");
	case AsciiFilter::Status::NotEnoughRowsSelected:
		return i18n("Not enough rows selected. Increase number of rows.");
	case AsciiFilter::Status::UnableToOpenDevice:
		return i18n("Unable to open device");
	}
	return i18n("Unhandled case");
}

void AsciiFilter::save(QXmlStreamWriter* writer) const {
	Q_D(const AsciiFilter);
	const auto& p = d->properties;

	writer->writeStartElement(QStringLiteral("asciiFilter"));
	writer->writeAttribute(QStringLiteral("commentCharacter"), p.commentCharacter);
	writer->writeAttribute(QStringLiteral("separatingCharacterDetection"), QString::number(p.automaticSeparatorDetection)); // NEW
	writer->writeAttribute(QStringLiteral("separatingCharacter"), p.separator);
	// writer->writeAttribute(QStringLiteral("autoMode"), QString::number(d->autoModeEnabled));
	writer->writeAttribute(QStringLiteral("createIndex"), QString::number(p.createIndexEnabled));
	writer->writeAttribute(QStringLiteral("createTimestamp"), QString::number(p.createTimestampEnabled));
	writer->writeAttribute(QStringLiteral("header"), QString::number(p.headerEnabled));
	writer->writeAttribute(QStringLiteral("vectorNames"), p.columnNamesRaw); // Changed!
	writer->writeAttribute(QStringLiteral("skipEmptyParts"), QString::number(p.skipEmptyParts));
	writer->writeAttribute(QStringLiteral("simplifyWhitespaces"), QString::number(p.simplifyWhitespacesEnabled));
	writer->writeAttribute(QStringLiteral("nanValue"), QString::number(p.nanValue)); // TODO: not yet handled
	writer->writeAttribute(QStringLiteral("removeQuotes"), QString::number(p.removeQuotesEnabled));
	writer->writeAttribute(QStringLiteral("startRow"), QString::number(p.startRow));
	//writer->writeAttribute(QStringLiteral("endRow"), QString::number(p.endRow));
	writer->writeAttribute(QStringLiteral("numberRows"), QString::number(p.numberRows)); // NEW
	writer->writeAttribute(QStringLiteral("startColumn"), QString::number(p.startColumn));
	// writer->writeAttribute(QStringLiteral("endColumn"), QString::number(d->endColumn));
	writer->writeAttribute(QStringLiteral("numberColumns"), QString::number(p.numberColumns)); // NEW
	writer->writeEndElement();
}

bool AsciiFilter::load(XmlStreamReader* reader) {
	Q_D(AsciiFilter);
	const auto& attribs = reader->attributes();
	QString str;

	READ_STRING_VALUE("commentCharacter", properties.commentCharacter);
	READ_STRING_VALUE("separatingCharacter", properties.separator);

	READ_INT_VALUE("createIndex", properties.createIndexEnabled, bool);
	READ_INT_VALUE("createTimestamp", properties.createTimestampEnabled, bool);
	// READ_INT_VALUE("autoMode", autoModeEnabled, bool);
	READ_INT_VALUE("header", properties.headerEnabled, bool);

	str = attribs.value(QStringLiteral("vectorNames")).toString();
	if (Project::xmlVersion() < 15)
		d->properties.columnNamesRaw = str.split(QLatin1Char(' ')).join(d->properties.separator); // may be empty

	READ_INT_VALUE("simplifyWhitespaces", properties.simplifyWhitespacesEnabled, bool);
	READ_DOUBLE_VALUE("nanValue", properties.nanValue);
	READ_INT_VALUE("removeQuotes", properties.removeQuotesEnabled, bool);
	READ_INT_VALUE("skipEmptyParts", properties.skipEmptyParts, bool);
	READ_INT_VALUE("startRow", properties.startRow, int);
	// READ_INT_VALUE("endRow", endRow, int);
	READ_INT_VALUE("startColumn", properties.startColumn, int);
	// READ_INT_VALUE("endColumn", endColumn, int);
	return true;
}

//########################################################################################################################
//##  PRIVATE IMPLEMENTATIONS  ###########################################################################################
//########################################################################################################################
AsciiFilterPrivate::AsciiFilterPrivate(AsciiFilter* owner): q(owner) {
}

/*!
 * \brief AsciiFilter::initialize
 * Determine all automatic values like separator, numberRows, numberColumns
 */
AsciiFilter::Status AsciiFilterPrivate::initialize(QIODevice& device) {
	using Status = AsciiFilter::Status;

	IODeviceHandler d(device); // closes device automatically. TODO: check that it gets not optimized out

	if (!properties.automaticSeparatorDetection && properties.numberColumns > 0 && properties.columnModes.size() == properties.numberColumns)
		return Status::Success; // Nothing to do since all unknows are determined

	bool removeQuotes = properties.removeQuotesEnabled;
	bool simplifyWhiteSpace = properties.simplifyWhitespacesEnabled;
	bool skipEmptyParts = properties.skipEmptyParts;

	if (device.isSequential()) {
		// Initialization not required. Assuming that all parameters are set,
		// makes no sense for sequential devices, because you never know if
		// the line is really the first line
		if (properties.headerEnabled)
			return Status::SequentialDeviceHeaderEnabled;
		else if (properties.automaticSeparatorDetection)
			return Status::SequentialDeviceAutomaticSeparatorDetection;
		else if (properties.columnModes.isEmpty())
			return Status::SequentialDeviceNoColumnModes;

		properties.columnNames = determineColumns(properties.columnNamesRaw, properties);

		return Status::Success;
	}

	if (!device.open(QIODevice::ReadOnly))
		return Status::UnableToOpenDevice;

	if (device.atEnd())
		return Status::DeviceAtEnd;

	properties.columnModes.clear();
	properties.columnNames.clear();

	// Determine header line
	QString line;
	int validRowCounter = 0;
	do {
		const auto status = getLine(device, line);
		if (status != Status::Success)
			return status;

		if (line.isEmpty() || (!properties.commentCharacter.isEmpty() && line.startsWith(properties.commentCharacter)))
			continue;

		validRowCounter++;
		if (properties.headerEnabled && properties.headerLine > 0 && validRowCounter != properties.headerLine)
			continue;

		break;
	} while (true);

	// Determine separator
	if (properties.automaticSeparatorDetection) {
		QString separator;
		const auto status = determineSeparator(line, removeQuotes, simplifyWhiteSpace, separator);
		if (status != Status::Success)
			return status;
		properties.separator = separator;
	} else {
		DEBUG(Q_FUNC_INFO << ", using GIVEN separator: " << STDSTRING(properties.separator));
		// replace symbolic "TAB" with '\t'
		properties.separator = properties.separator.replace(QLatin1String("2xTAB"), QLatin1String("\t\t"), Qt::CaseInsensitive);
		properties.separator = properties.separator.replace(QLatin1String("TAB"), QLatin1String("\t"), Qt::CaseInsensitive);
		// replace symbolic "SPACE" with ' '
		properties.separator = properties.separator.replace(QLatin1String("2xSPACE"), QLatin1String("  "), Qt::CaseInsensitive);
		properties.separator = properties.separator.replace(QLatin1String("3xSPACE"), QLatin1String("   "), Qt::CaseInsensitive);
		properties.separator = properties.separator.replace(QLatin1String("4xSPACE"), QLatin1String("    "), Qt::CaseInsensitive);
		properties.separator = properties.separator.replace(QLatin1String("SPACE"), QLatin1String(" "), Qt::CaseInsensitive);
	}

	// Determine column names
	if (properties.headerEnabled)
		properties.columnNames = determineColumns(line, properties);
	else {
		if (properties.columnNamesRaw.isEmpty())
			return Status::HeaderEmpty;
		// Determine column names from the names specified in the dialog
		// StartColumn is always one. Because otherwise I would need to specify column names for not required columns
		properties.columnNames = determineColumns(properties.columnNamesRaw, QStringLiteral(","), removeQuotes, true, skipEmptyParts, 1, properties.numberColumns);
		if (properties.columnNames.isEmpty())
			return Status::UnableParsingHeader;
		//properties.columnNames = determineColumns(properties.columnNamesRaw, separator, removeQuotes, simplifyWhiteSpace, skipEmptyParts, properties.startColumn, properties.numberColumns);

	}

	// Determine number of columns
	if (properties.numberColumns < 0) {
		// properties.headerEnabled = true: Determine the column count from the number of columns in the file
		// properties.headerEnabled = false: Use number of columns specified
		properties.numberColumns = properties.columnNames.count();
	}

	// add time stamp and index column
	if (properties.createTimestampEnabled) {
		properties.columnNames.prepend(i18n("Timestamp"));
		properties.columnModes.prepend(AbstractColumn::ColumnMode::DateTime);
	}
	if (properties.createIndexEnabled) {
		properties.columnNames.prepend(i18n("Index"));
		properties.columnModes.prepend(AbstractColumn::ColumnMode::Integer);
	}

	int startDataRow = properties.startRow;
	if (properties.headerEnabled)
		startDataRow += properties.headerLine;

	// Determine column modes
	QVector<QStringList> rows;
	size_t i = 0;
	if (!properties.headerEnabled) {
		rows.append(determineColumns(line, properties));
		if (rows.last().count() != properties.numberColumns)
			return Status::InvalidNumberDataColumns;
		i++;
	} else {
		// Skip all lines until startRow line
		int j = properties.startRow - 1;
		while (j > 0) {
			const auto status = getLine(device, line);
			if (status == Status::DeviceAtEnd)
				break; // No more data to read. So we determine from the others
			if (status != Status::Success)
				return status;

			if (line.isEmpty() || (!properties.commentCharacter.isEmpty() && line.startsWith(properties.commentCharacter)))
				continue;

			j--;
		}
	}
	while (i < m_dataTypeLines) {
		const auto status = getLine(device, line);
		if (status == Status::DeviceAtEnd)
			break; // No more data to read. So we determine from the others
		if (status != Status::Success)
			return status;
		if (line.isEmpty() || (!properties.commentCharacter.isEmpty() && line.startsWith(properties.commentCharacter)))
			continue;
		rows.append(determineColumns(line, properties));
		if (rows.last().count() != properties.numberColumns)
			return Status::InvalidNumberDataColumns;
		i++;
	}
	QString dateTimeFormat;
	properties.columnModes.append(determineColumnModes(rows, properties, dateTimeFormat));
	if (properties.dateTimeFormat.isEmpty())
		properties.dateTimeFormat = dateTimeFormat;

	return Status::Success;
}

AsciiFilter::Status AsciiFilterPrivate::readFromDevice(QIODevice& device, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode importMode, int lines) {
	using Status = AsciiFilter::Status;

	auto* spreadsheet = dynamic_cast<Spreadsheet*>(dataSource);
	if (!spreadsheet)
		return Status::UnsupportedDataSource;

	// TODO: check. How to check that m_DataContainer
	initialized = false;

	int dataContainerStartIndex = importMode == AbstractFileFilter::ImportMode::Replace ? 0 : m_DataContainer.elementCount();
	if (!initialized) {
		const auto status = initialize(device);
		if (status != Status::Success)
			return status;

		// Update
		// Initialize m_DataContainer. So m_DataContainer must not free up the data afterwards
		std::vector<void*> dataContainer;
		bool ok;
		spreadsheet->prepareImport(dataContainer, importMode, 0, properties.columnModes.size(), properties.columnNames, properties.columnModes, ok, true);
		m_DataContainer.clear();
		for (size_t i=0; i < dataContainer.size(); i++) {
			m_DataContainer.appendVector(dataContainer.at(i), properties.columnModes.at(i));
		}
		try {
			if (importMode == AbstractFileFilter::ImportMode::Replace) {
				m_DataContainer.resize(10000); // reserve to not having to reallocate all the time
				dataContainerStartIndex = 0;
			} else {
				m_DataContainer.resize(m_DataContainer.elementCount() * 2); // Reserve the double as right now is allocated
				dataContainerStartIndex = m_DataContainer.elementCount();
			}
		} catch (std::bad_alloc&) {
			//q->setLastError(i18n("Not enough memory."));
			return Status::NotEnoughMemory;
		}
		initialized = true;
	}

	if (!device.open(QIODevice::ReadOnly))
		return Status::UnableToOpenDevice;
	else if (device.atEnd() && !device.isSequential())
		return Status::DeviceAtEnd;

	if (!device.isSequential()) {
		// TODO: Seeking

		QString line;
		int counter = 0;
		int startDataRow = 1;
		int rowIndex = dataContainerStartIndex;
		// Iterate over all rows
		do {
			const auto status = getLine(device, line);
			if (status == Status::DeviceAtEnd)
				break;
			else if (status != Status::Success)
				return status;

			if (line.isEmpty() || (!properties.commentCharacter.isEmpty() && line.startsWith(properties.commentCharacter)))
				continue;

			counter ++;
			if (properties.headerEnabled && properties.headerLine > 0) {
				if (counter <= properties.headerLine) {
					if (counter == properties.headerLine)
						startDataRow = counter + 1;
					continue;
				}
			}

			if ((counter - startDataRow + 1) < properties.startRow)
				continue;

			// Now we get to the data rows
			const auto& values = determineColumns(line, properties);
			assert(values.size() == m_DataContainer.size() - properties.createIndexEnabled - properties.createTimestampEnabled);

			if (properties.createIndexEnabled)
				m_DataContainer.setData(0, rowIndex, rowIndex - dataContainerStartIndex + 1);
			if (properties.createTimestampEnabled) {
				// If create index is enabled +1 the timestamp is in the second column
				m_DataContainer.setData(properties.createIndexEnabled, rowIndex, QDateTime::currentDateTime());
			}

			int columnIndex = 0 + properties.createIndexEnabled + properties.createTimestampEnabled;
			// Iterate over all columns
			for (const auto& value: values) {
				bool conversionOk = false;
				switch (properties.columnModes[columnIndex]) {
				case AbstractColumn::ColumnMode::Double: {
					double d = properties.locale.toDouble(value, &conversionOk);
					if (!conversionOk) {d = properties.nanValue;}
					m_DataContainer.setData(columnIndex, rowIndex, d);
					break;
				}
				case AbstractColumn::ColumnMode::Integer: {
					int i = properties.locale.toInt(value, &conversionOk);
					if (!conversionOk) {i = 0;}
					m_DataContainer.setData(columnIndex, rowIndex, i);
					break;
				}
				case AbstractColumn::ColumnMode::BigInt: {
					qint64 i = properties.locale.toLongLong(value, &conversionOk);
					if (!conversionOk) {i = 0;}
					m_DataContainer.setData(columnIndex, rowIndex, i);
					break;
				}
				case AbstractColumn::ColumnMode::Text:
					m_DataContainer.setData(columnIndex, rowIndex, value);
					break;
				case AbstractColumn::ColumnMode::Month:
				case AbstractColumn::ColumnMode::Day:
				case AbstractColumn::ColumnMode::DateTime:
					m_DataContainer.setData(columnIndex, rowIndex, QDateTime::fromString(value, properties.dateTimeFormat, properties.baseYear));
					break;
				}
				columnIndex ++;
			}
			rowIndex ++;
			if (rowIndex >= m_DataContainer.elementCount()) {
				try {
					m_DataContainer.resize(2 * m_DataContainer.elementCount()); // Always double
				} catch (std::bad_alloc&) {
					//q->setLastError(i18n("Not enough memory."));
					return Status::NotEnoughMemory;
				}
			}

			if (lines >= 0 && (rowIndex - dataContainerStartIndex) > lines)
				break;

			// ask to update the progress bar only if we have more than 1000 lines
			// only in 1% steps
			// progressIndex++;
			// if (lines > 1000 && progressIndex > progressInterval) {
			// 	double value = 100. * currentRow / lines;
			// 	Q_EMIT q->completed(static_cast<int>(value));
			// 	progressIndex = 0;
			// 	QApplication::processEvents(QEventLoop::AllEvents, 0);
			// }

		} while (true);

		// Shrink, because some data might got skipped
		m_DataContainer.resize(rowIndex);
	}
	m_DataContainer = DataContainer(); // Reset datacontainer. The data is already stored in the columns, so no freeing of memory is required

	dataSource->finalizeImport(0, 0, properties.columnNames.size() - 1, properties.dateTimeFormat, importMode);
	return Status::Success;
}

/*!
 * Determines the column modes from the provided rows
 *
 * \brief AsciiFilterPrivate::determineColumnModes
 * \param dateTimeFormat The datetime format will be updated if it is empty by the detected format
 * \return
 */
QVector<AbstractColumn::ColumnMode> AsciiFilterPrivate::determineColumnModes(const QVector<QStringList>& rows, const AsciiFilter::Properties& properties, QString& dateTimeFormat) {
	using Mode = AbstractColumn::ColumnMode;

	dateTimeFormat = properties.dateTimeFormat;

	QVector<Mode> modes;
	if (rows.length() == 0)
		return modes;

	int columnCount = rows.first().count();
	for (int i=0; i < columnCount; i++)
		modes.append(Mode::Integer);

	for (const auto& row : rows) {
		int columnIndex = 0;
		for (auto column: row) {
			if (columnIndex >= columnCount)
				break;

			if (properties.simplifyWhitespacesEnabled)
				column = column.simplified();
			if (properties.removeQuotesEnabled)
				column.remove(QLatin1Char('"'));
			auto mode = AbstractFileFilter::columnMode(column, dateTimeFormat, properties.numberFormat, properties.baseYear);

			if (intAsDouble) {
				if (mode == Mode::Integer || mode == Mode::BigInt)
					mode = Mode::Double;
			}

			if (mode == Mode::Double && modes[columnIndex] == Mode::Integer) {
				// numeric: integer -> numeric
				modes[columnIndex] = mode;
			} else if (mode == Mode::Text && modes[columnIndex] != Mode::Text) {
				// text: non text -> text
				modes[columnIndex] = mode;
			} else if (mode != Mode::Text && modes[columnIndex] == Mode::Text) {
				// numeric: text -> numeric/integer
				modes[columnIndex] = mode;
			} else
				modes[columnIndex] = mode;
			columnIndex ++;
		}
	}
	return modes;
}

QStringList AsciiFilterPrivate::determineColumns(const QString& line, const AsciiFilter::Properties& properties) {
	return determineColumns(line, properties.separator, properties.removeQuotesEnabled, properties.simplifyWhitespacesEnabled, properties.skipEmptyParts, properties.startColumn, properties.numberColumns);
}

QStringList AsciiFilterPrivate::determineColumns(const QString& line, const QString& separator, bool removeQuotesEnabled, bool simplifyWhiteSpaces, bool skipEmptyParts, int startColumn, int numberColumns) {
	enum class State {
		Column,
		QuotedText,
	};
	auto state = State::Column;

	QStringList columnNames;
	QString columnName;
	int columnCount = 1;
	for (auto c: line) {
		if (c == QLatin1Char('\n') || c == QLatin1Char('\r'))
			break;
		if (removeQuotesEnabled && c == QLatin1Char('"')) {
			switch (state) {
			case State::Column:
				state = State::QuotedText;
				break;
			case State::QuotedText:
				state = State::Column;
				break;
			}
			continue;
		}

		if (simplifyWhiteSpaces && state != State::QuotedText && (c == QLatin1Char(' ') || c == QLatin1Char('\t')))
			continue;
		else
			columnName.append(c);

		switch (state) {
		case State::Column: {
			if (columnName.endsWith(separator)) {
				columnName.remove(columnName.length() - separator.length(), separator.length());
				if (!skipEmptyParts || !columnName.isEmpty()) {
					if (columnCount >= startColumn && (columnCount < startColumn + numberColumns || numberColumns < 0))
						columnNames << columnName;
					columnCount ++;
				}
				columnName.clear();
			}
			break;
		}
		case State::QuotedText:
			continue;
		}
	}
	if (!columnName.isEmpty() && columnCount >= startColumn && (columnCount < startColumn + numberColumns || numberColumns < 0))
		columnNames.append(columnName);
	return columnNames;
}

AsciiFilter::Status AsciiFilterPrivate::determineSeparator(const QString& line, bool removeQuotesEnabled, bool simplifyWhiteSpaces, QString& separator) {
	using Status = AsciiFilter::Status;

	enum class State {
		Column,
		QuotedText,
	};
	QVector<QChar> allowedSeparatorCharactersNonWhiteSpace {QLatin1Char(','), QLatin1Char(';'), QLatin1Char('|')};

	separator.clear();

	auto state = State::Column;
	QString separatorSequence;
	for (auto c: line) {
		if (removeQuotesEnabled && c == QLatin1Char('"')) {
			switch (state) {
				case State::Column:
					state = State::QuotedText;
					break;
				case State::QuotedText:
					state = State::Column;
					break;
			}
			continue;
		}

		switch (state) {
		case State::Column: {
			for (auto asc: allowedSeparatorCharactersNonWhiteSpace) {
				// Simple non whitespace characters as separators
				if (asc == c) {
					separator = c; // This is the separator character
					return Status::Success;
				}
			}
			if (!simplifyWhiteSpaces) {
				// Complex whitespace characters as separators
				// It can be any combination of those whitespaces
				if (c == QLatin1Char(' ') || c == QLatin1Char('\t'))
					separatorSequence.append(c);
				else if (!separatorSequence.isEmpty()) {
					separator = separatorSequence;
					return Status::Success;
				}
			}
			break;
		}
		case State::QuotedText:
			continue; // Nothing to do, since we are inside of a quoted text
		}
	}

	return Status::SeparatorDeterminationFailed;
}

AsciiFilter::Status AsciiFilterPrivate::getLine(QIODevice& device, QString& line) {
	using Status = AsciiFilter::Status;

	if (device.atEnd())
		return Status::DeviceAtEnd;

	if (!device.canReadLine()) {
		DEBUG(Q_FUNC_INFO << ", WARNING: device cannot 'readLine()' but using it anyway.");
		// return Status::UnableToReadLine;
	}

	line = QString::fromUtf8(device.readLine());
	if (line.endsWith(QStringLiteral("\n")))
		line.removeLast();
	else if (line.endsWith(QStringLiteral("\r\n"))) {
		line.removeLast();
		line.removeLast();
	}
	return Status::Success;
}

QVector<QStringList> AsciiFilterPrivate::preview(const QString& fileName, int lines) {
	KCompressionDevice file(fileName);
	Spreadsheet spreadsheet(QStringLiteral("AsciiFilterPreviewSpreadsheet"));
	const auto read_lines = readFromDevice(file, &spreadsheet, AbstractFileFilter::ImportMode::Replace, lines);
	// TODO: handle if not enough lines can be read

	QVector<QStringList> p;
	for (int row = 0; row < spreadsheet.rowCount(); row++) {
		QStringList line;
		for (int column = 0; column < spreadsheet.columnCount(); column++) {
			const auto c = spreadsheet.column(column);
			switch (c->columnMode()) {
			case AbstractColumn::ColumnMode::BigInt:
				line << QString::number(c->bigIntAt(row));
				break;
			case AbstractColumn::ColumnMode::Integer:
				line << QString::number(c->integerAt(row));
				break;
			case AbstractColumn::ColumnMode::Double:
				line << QString::number(c->valueAt(row));
				break;
			case AbstractColumn::ColumnMode::Day:
			case AbstractColumn::ColumnMode::Month:
			case AbstractColumn::ColumnMode::DateTime:
				line << c->dateTimeAt(row).toString(properties.dateTimeFormat);
				break;
			}
		}
		p << line;
	}
	return p;
}

void AsciiFilterPrivate::setLastError(AsciiFilter::Status status) {
	const auto s = AsciiFilter::statusToString(status);
	q->setLastError(s);
}

// #################################################################################################
// ###  Data container  ############################################################################
// #################################################################################################
/*!
 * \brief dataContainer
 * Do not modify outside as long as DataContainer exists!
 * \return
 */
std::vector<void *> &AsciiFilterPrivate::DataContainer::dataContainer() {
	return m_dataContainer;
}

AbstractColumn::ColumnMode AsciiFilterPrivate::DataContainer::columnMode(int index) const {
	return m_columnModes.at(index);
}

const void* AsciiFilterPrivate::DataContainer::datas(size_t index) const {
	if (index < size())
		return m_dataContainer.at(index);
	return nullptr;
}

bool AsciiFilterPrivate::DataContainer::resize(uint32_t s) const {
	for (uint32_t i = 0; i < m_dataContainer.size(); i++) {
		switch (m_columnModes.at(i)) {
		case AbstractColumn::ColumnMode::BigInt:
			static_cast<QVector<qint64>*>(m_dataContainer[i])->resize(s);
			break;
		case AbstractColumn::ColumnMode::Integer: {
			static_cast<QVector<qint32>*>(m_dataContainer[i])->resize(s);
			break;
		}
		case AbstractColumn::ColumnMode::Double:
			static_cast<QVector<double>*>(m_dataContainer[i])->resize(s);
			break;
		case AbstractColumn::ColumnMode::Text:
			static_cast<QVector<QString>*>(m_dataContainer[i])->resize(s);
			break;
		case AbstractColumn::ColumnMode::Month:
		case AbstractColumn::ColumnMode::Day:
		case AbstractColumn::ColumnMode::DateTime:
			static_cast<QVector<QDateTime>*>(m_dataContainer[i])->resize(s);
			break;
		}
	}

	if (m_dataContainer.size() == 0)
		return true;

		   // Check that all vectors have same length
	int size = elementCount(0);
	if (size == -1)
		return false;

	for (uint32_t i = 1; i < m_dataContainer.size(); i++) {
		if (size != elementCount(i))
			return false;
	}
	return true;
}

bool AsciiFilterPrivate::DataContainer::reserve(uint32_t s) const {
	for (uint32_t i = 0; i < m_dataContainer.size(); i++) {
		switch (m_columnModes.at(i)) {
		case AbstractColumn::ColumnMode::BigInt:
			static_cast<QVector<qint64>*>(m_dataContainer[i])->reserve(s);
			break;
		case AbstractColumn::ColumnMode::Integer:
			static_cast<QVector<qint32>*>(m_dataContainer[i])->reserve(s);
			break;
		case AbstractColumn::ColumnMode::Double:
			static_cast<QVector<double>*>(m_dataContainer[i])->reserve(s);
			break;
		case AbstractColumn::ColumnMode::Text:
			static_cast<QVector<QString>*>(m_dataContainer[i])->reserve(s);
			break;
		case AbstractColumn::ColumnMode::Month:
		case AbstractColumn::ColumnMode::Day:
		case AbstractColumn::ColumnMode::DateTime:
			static_cast<QVector<QDateTime>*>(m_dataContainer[i])->reserve(s);
			break;
		}
	}

	if (m_dataContainer.size() == 0)
		return true;

	// Check that all vectors have same length
	int size = elementCount(0);
	if (size == -1)
		return false;

	for (uint32_t i = 1; i < m_dataContainer.size(); i++) {
		if (size != elementCount(i))
			return false;
	}
	return true;
}

void AsciiFilterPrivate::DataContainer::appendVector(AbstractColumn::ColumnMode cm) {
	void* vector;
	switch (cm) {
	case AbstractColumn::ColumnMode::Double:
		vector = new QVector<double>();
		break;
	case AbstractColumn::ColumnMode::Integer:
		vector = new QVector<int>();
		break;
	case AbstractColumn::ColumnMode::BigInt:
		vector = new QVector<qint64>();
		break;
	case AbstractColumn::ColumnMode::Text:
		vector = new QVector<QString>();
		break;
	case AbstractColumn::ColumnMode::DateTime:
	case AbstractColumn::ColumnMode::Day:
	case AbstractColumn::ColumnMode::Month:
		vector = new QVector<QDateTime>();
		break;
	}

	m_dataContainer.push_back(vector);
	m_columnModes.append(cm);
}

size_t AsciiFilterPrivate::DataContainer::elementCount(size_t index) const {
	if (index >= m_dataContainer.size())
		return -1;

	switch (m_columnModes.at(index)) {
	case AbstractColumn::ColumnMode::BigInt:
		return static_cast<QVector<qint64>*>(m_dataContainer[index])->size();
		break;
	case AbstractColumn::ColumnMode::Integer:
		return static_cast<QVector<qint32>*>(m_dataContainer[index])->size();
		break;
	case AbstractColumn::ColumnMode::Double:
		return static_cast<QVector<double>*>(m_dataContainer[index])->size();
		break;
	case AbstractColumn::ColumnMode::Text:
		return static_cast<QVector<QString>*>(m_dataContainer[index])->size();
		break;
	case AbstractColumn::ColumnMode::Month:
	case AbstractColumn::ColumnMode::Day:
	case AbstractColumn::ColumnMode::DateTime:
		return static_cast<QVector<QDateTime>*>(m_dataContainer[index])->size();
		break;
	}
	assert(false);
	return -1;
}

size_t AsciiFilterPrivate::DataContainer::size() const {
	return m_dataContainer.size();
}

void AsciiFilterPrivate::DataContainer::clear() {
	for (uint i = 0; i < m_dataContainer.size(); i++) {
		switch (m_columnModes.at(i)) {
		case AbstractColumn::ColumnMode::BigInt:
			delete static_cast<QVector<qint64>*>(m_dataContainer[i]);
			break;
		case AbstractColumn::ColumnMode::Integer:
			delete static_cast<QVector<qint32>*>(m_dataContainer[i]);
			break;
		case AbstractColumn::ColumnMode::Double:
			delete static_cast<QVector<double>*>(m_dataContainer[i]);
			break;
		// TODO: implement missing cases
		case AbstractColumn::ColumnMode::Text:
		case AbstractColumn::ColumnMode::Month:
		case AbstractColumn::ColumnMode::Day:
		case AbstractColumn::ColumnMode::DateTime:
			break;
		}
	}
	m_columnModes.clear();
	m_dataContainer.clear();
}
