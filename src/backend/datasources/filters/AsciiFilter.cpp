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
#include "backend/core/Project.h"
#include "backend/core/column/Column.h"
#include "backend/datasources/filters/FilterStatus.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/hostprocess.h"
#include "backend/lib/trace.h"
#include "backend/matrix/Matrix.h"
#include "backend/spreadsheet/Spreadsheet.h"

#include <KCompressionDevice>
#include <KLocalizedString>

#include <QDateTime>
#include <QFile>
#ifdef HAVE_QTSERIALPORT
#include <QSerialPort>
#endif
#include <QNetworkDatagram>
#include <QTcpSocket>
#include <QTimer>
#include <QUdpSocket>

#include <fstream>

namespace {
// Simple object to automatically closing a device when this object
// goes out of scope
struct IODeviceHandler {
	IODeviceHandler(QIODevice& d, bool reset)
		: device(d)
		, reset(reset) {
	}

	~IODeviceHandler() {
		if (!device.isSequential()) {
			if (reset)
				device.reset(); // Seek to the start
			if (device.isOpen())
				device.close();
		}
	}

private:
	QIODevice& device;
	const bool reset;
};
const QLatin1String INTERNAL_SEPARATOR(",");
} // Anonymous namespace

BufferReader::BufferReader(const QByteArray& buffer)
	: m_message(buffer) {
}

bool BufferReader::isSequential() const {
	return true;
}

bool BufferReader::open(OpenMode mode) {
	if (mode == QIODevice::OpenModeFlag::ReadOnly) {
		return QIODevice::open(mode);
	}
	return false;
}

bool BufferReader::atEnd() const {
	if (m_index >= m_message.length()) {
		// No data available here, but maybe the qiodevice buffer has still data
		return QIODevice::atEnd();
	}
	return false;
}

// Not required functions yet, so ignore them until they are required
qint64 BufferReader::readData(char* out, qint64 maxLen) {
	if (m_index + maxLen > m_message.length())
		maxLen = m_message.length() - m_index;

	if (maxLen < 0)
		return 0;

	for (int i = 0; i < maxLen; i++) {
		*out++ = *(const_cast<char*>(m_message.data()) + m_index + i);
	}
	m_index += maxLen;
	return maxLen;
}

qint64 BufferReader::writeData(const char*, qint64) {
	Q_ASSERT(false);
	return 0;
}

AsciiFilter::AsciiFilter()
	: AbstractFileFilter(FileType::Ascii)
	, d_ptr(std::make_unique<AsciiFilterPrivate>(this)) {
}

AsciiFilter::~AsciiFilter() = default;

Status AsciiFilter::initialize(AsciiFilter::Properties p) {
	Q_D(AsciiFilter);
	return d->initialize(p);
}

AsciiFilter::Properties AsciiFilter::properties() const {
	Q_D(const AsciiFilter);
	return d->properties;
}

AsciiFilter::Properties AsciiFilter::defaultProperties() const {
	return Properties();
}

void AsciiFilter::setProperties(Properties& properties) {
	Q_D(AsciiFilter);
	d->initialized = false;
	d->properties = properties;
}

QStringList AsciiFilter::columnNames() const {
	Q_D(const AsciiFilter);
	return d->properties.columnNames;
}

QVector<AbstractColumn::ColumnMode> AsciiFilter::columnModes() const {
	Q_D(const AsciiFilter);
	return d->properties.columnModes;
}

void AsciiFilter::setDataSource(AbstractDataSource* dataSource) {
	Q_D(AsciiFilter);
	d->setDataSource(dataSource);
}

void AsciiFilter::readDataFromFile(const QString& fileName, AbstractDataSource* dataSource, ImportMode columnImportMode) {
	Q_D(AsciiFilter);
	d->fileNumberLines = lineCount(fileName);

	KCompressionDevice file(fileName);

	if (d->isUTF16(file)) {
		d->setLastError(Status::UTF16NotSupported());
		return;
	}

	const int lines = d->properties.endRow < 0 ? -1 : d->properties.endRow - d->properties.startRow + 1;
	auto rowImportMode = ImportMode::Replace;
	if (columnImportMode != ImportMode::Replace)
		rowImportMode = ImportMode::Append;
	setDataSource(dataSource);
	readFromDevice(file, columnImportMode, rowImportMode, 0, lines, 0);
}

qint64 AsciiFilter::readFromDevice(QIODevice& device,
								   ImportMode columnImportMode,
								   ImportMode rowImportMode,
								   qint64 from,
								   qint64 lines,
								   qint64 keepNRows,
								   bool skipFirstLine) {
	Q_D(AsciiFilter);
	qint64 bytes_read;
	const auto status = d->readFromDevice(device, columnImportMode, rowImportMode, from, lines, keepNRows, bytes_read, skipFirstLine);
	d->setLastError(status);
	// TODO: do something with it!
	return bytes_read;
}

void AsciiFilter::write(const QString& /*fileName*/, AbstractDataSource*) {
	// TODO
}

QVector<QStringList> AsciiFilter::preview(QIODevice& device, int lines, bool reinit, bool skipFirstLine) {
	Q_D(AsciiFilter);
	return d->preview(device, lines, reinit, skipFirstLine);
}

QVector<QStringList> AsciiFilter::preview(const QString& fileName, int lines, bool reinit) {
	Q_D(AsciiFilter);
	return d->preview(fileName, lines, reinit);
}

bool AsciiFilter::initialized() const {
	Q_D(const AsciiFilter);
	return d->initialized;
}

QString AsciiFilter::autoSeparatorDetectionString() {
	return QStringLiteral("auto");
}

/*!
  returns the list of all predefined separator characters.
*/
QStringList AsciiFilter::separatorCharacters() {
	return (QStringList() << autoSeparatorDetectionString() << QStringLiteral("TAB") << QStringLiteral("SPACE") << QStringLiteral(",") << QStringLiteral(";")
						  << QStringLiteral(":") << QStringLiteral(",TAB") << QStringLiteral(";TAB") << QStringLiteral(":TAB") << QStringLiteral(",SPACE")
						  << QStringLiteral(";SPACE") << QStringLiteral(":SPACE") << QStringLiteral("2xSPACE") << QStringLiteral("3xSPACE")
						  << QStringLiteral("4xSPACE") << QStringLiteral("2xTAB"));
}

/*!
 * returns the list of all predefined comment characters.
 */
QStringList AsciiFilter::commentCharacters() {
	return (QStringList() << QStringLiteral("#") << QStringLiteral("!") << QStringLiteral("//") << QStringLiteral("+") << QStringLiteral("c")
						  << QStringLiteral(":") << QStringLiteral(";"));
}

QString AsciiFilter::fileInfoString(const QString& fileName) {
	QString info(i18n("Number of columns: %1", AsciiFilter::columnNumber(fileName)));
	info += QStringLiteral("<br>");
	info += i18n("Number of lines: %1", AsciiFilter::lineCount(fileName));
	return info;
}

int AsciiFilter::columnNumber(const QString& /*fileName*/, const QString& /*separator*/) {
	return -1; // TODO: implement
}

/*!
 * returns number of lines in file 'fileName'
 * optional: only check 'maxLines' (returns minimum of line count and maxLines)
 */
size_t AsciiFilter::lineCount(const QString& fileName, const size_t maxLines) {
	DEBUG(Q_FUNC_INFO << ", max lines = " << maxLines)
	PERFTRACE(QLatin1String(Q_FUNC_INFO))

	KCompressionDevice device(fileName);

	if (!device.open(QIODevice::ReadOnly)) {
		DEBUG(Q_FUNC_INFO << ", Could not open file " << STDSTRING(fileName) << " to determine number of lines");
		return 0;
	}

	if (device.compressionType() == KCompressionDevice::None) { // uncompressed
		device.close();

		std::ifstream file(fileName.toStdString());

		size_t count = 0;
		std::string line;
		while (std::getline(file, line) && count < maxLines)
			++count;
		DEBUG(Q_FUNC_INFO << "Number of lines: " << count)
		return count;
	}

	// fallback for compressed data
	size_t count = 0;
	while (!device.atEnd()) {
		if (count >= maxLines) // stop when maxLines available
			return count;
		device.readLine();
		count++;
	}

	return count;
}

void AsciiFilter::save(QXmlStreamWriter* writer) const {
	Q_D(const AsciiFilter);
	const auto& p = d->properties;

	writer->writeStartElement(QStringLiteral("asciiFilter"));
	writer->writeAttribute(QStringLiteral("commentCharacter"), p.commentCharacter);
	writer->writeAttribute(QStringLiteral("separatingCharacterDetection"), QString::number(p.automaticSeparatorDetection));
	writer->writeAttribute(QStringLiteral("separatingCharacter"), p.separator);
	writer->writeAttribute(QStringLiteral("createIndex"), QString::number(p.createIndex));
	writer->writeAttribute(QStringLiteral("createTimestamp"), QString::number(p.createTimestamp));
	writer->writeAttribute(QStringLiteral("header"), QString::number(p.headerEnabled));
	writer->writeAttribute(QStringLiteral("headerLine"), QString::number(p.headerLine));
	writer->writeAttribute(QStringLiteral("vectorNames"), p.columnNamesRaw);
	writer->writeAttribute(QStringLiteral("skipEmptyParts"), QString::number(p.skipEmptyParts));
	writer->writeAttribute(QStringLiteral("simplifyWhitespaces"), QString::number(p.simplifyWhitespaces));
	writer->writeAttribute(QStringLiteral("nanValue"), QString::number(p.nanValue));
	writer->writeAttribute(QStringLiteral("removeQuotes"), QString::number(p.removeQuotes));
	writer->writeAttribute(QStringLiteral("startRow"), QString::number(p.startRow));
	writer->writeAttribute(QStringLiteral("endRow"), QString::number(p.endRow));
	writer->writeAttribute(QStringLiteral("startColumn"), QString::number(p.startColumn));
	writer->writeAttribute(QStringLiteral("endColumn"), QString::number(p.endColumn));
	writer->writeAttribute(QStringLiteral("columnModes"),
						   AsciiFilterPrivate::convertTranslatedColumnModesToNative(p.columnModesString)); // Manually specified column modes
	writer->writeAttribute(QStringLiteral("baseYear"), QString::number(p.baseYear));
	writer->writeAttribute(QStringLiteral("dateTimeFormat"), p.dateTimeFormat);
	writer->writeAttribute(QStringLiteral("intAsDouble"), QString::number(p.intAsDouble));
	writer->writeEndElement();
}

bool AsciiFilter::load(XmlStreamReader* reader) {
	Q_D(AsciiFilter);
	const auto& attribs = reader->attributes();
	QString str;

	READ_STRING_VALUE("commentCharacter", properties.commentCharacter);
	READ_INT_VALUE("separatingCharacterDetection", properties.automaticSeparatorDetection, bool);
	READ_STRING_VALUE("separatingCharacter", properties.separator);

	READ_INT_VALUE("createIndex", properties.createIndex, bool);
	READ_INT_VALUE("createTimestamp", properties.createTimestamp, bool);
	// READ_INT_VALUE("autoMode", autoModeEnabled, bool);
	READ_INT_VALUE("header", properties.headerEnabled, bool);
	READ_INT_VALUE("headerLine", properties.headerLine, int);

	str = attribs.value(QStringLiteral("vectorNames")).toString();
	if (Project::xmlVersion() < 15)
		d->properties.columnNamesRaw = str.split(QLatin1Char(' ')).join(d->properties.separator); // may be empty
	else
		d->properties.columnNamesRaw = str;

	READ_INT_VALUE("simplifyWhitespaces", properties.simplifyWhitespaces, bool);
	READ_DOUBLE_VALUE("nanValue", properties.nanValue);
	READ_INT_VALUE("removeQuotes", properties.removeQuotes, bool);
	READ_INT_VALUE("skipEmptyParts", properties.skipEmptyParts, bool);
	READ_INT_VALUE("startRow", properties.startRow, int);
	READ_INT_VALUE("endRow", properties.endRow, int);
	READ_INT_VALUE("startColumn", properties.startColumn, int);
	READ_INT_VALUE("endColumn", properties.endColumn, int);
	READ_STRING_VALUE("columnModes", properties.columnModesString);
	READ_INT_VALUE("baseYear", properties.baseYear, int);
	READ_STRING_VALUE("dateTimeFormat", properties.dateTimeFormat);
	READ_INT_VALUE("intAsDouble", properties.intAsDouble, bool);
	return true;
}

QStringList AsciiFilter::dataTypesString() {
	QStringList list;
	const auto& map = AsciiFilterPrivate::modeMap();
	for (const auto& m : map)
		list.append(m.first);
	return list;
}

QPair<QString, QString> AsciiFilter::dataTypeString(const AbstractColumn::ColumnMode mode) {
	const auto& modeMap = AsciiFilterPrivate::modeMap();
	for (auto it = modeMap.cbegin(), end = modeMap.cend(); it != end; it++) {
		if (it.value().second == mode) {
			return QPair<QString, QString>(it.key(), it.value().first);
		}
	}
	DEBUG("Mode not found");
	Q_ASSERT(false);
	return QPair<QString, QString>(QStringLiteral(""), QStringLiteral(""));
}

bool AsciiFilter::determineColumnModes(const QStringView& s, QVector<AbstractColumn::ColumnMode>& modes, QString& invalidString) {
	return AsciiFilterPrivate::determineColumnModes(s, modes, invalidString);
}

// ########################################################################################################################
// ##  PRIVATE IMPLEMENTATIONS  ###########################################################################################
// ########################################################################################################################
AsciiFilterPrivate::AsciiFilterPrivate(AsciiFilter* owner)
	: lastStatus(Status::Success())
	, q(owner) {
}

/*!
 * \brief AsciiFilter::initialize
 * Determine all automatic values like separator, endRow, endColumn
 */
Status AsciiFilterPrivate::initialize(QIODevice& device) {
	IODeviceHandler d(device, true); // closes device automatically

	if (!properties.automaticSeparatorDetection && properties.endColumn > 0
		&& properties.columnModes.size() == properties.endColumn - properties.startColumn + 1)
		return Status::Success(); // Nothing to do since all unknowns are determined

	const bool removeQuotes = properties.removeQuotes;
	const bool simplifyWhiteSpace = properties.simplifyWhitespaces;
	const bool skipEmptyParts = properties.skipEmptyParts;

#ifdef HAVE_QTSERIALPORT
	if (dynamic_cast<QSerialPort*>(&device)) {
		// Initialization not required. Assuming that all parameters are set,
		// makes no sense for serial port, because you never know if
		// the line is really the first line and if it is a complete line
		return Status::SerialDeviceUninitialized();
	}
#endif

	if (!device.isOpen()) {
		if (!device.open(QIODevice::ReadOnly))
			return Status::UnableToOpenDevice();
	}

	if (properties.endColumn > 0 && properties.endColumn < properties.startColumn)
		return Status::WrongEndColumn();

	if (properties.endRow > 0 && properties.endRow < properties.startRow)
		return Status::WrongEndRow();

	properties.columnModes.clear();
	properties.columnNames.clear();

	// Determine header line
	QString line;
	int validRowCounter = 0;
	do {
		const auto status = getLine(device, line);
		if (!status.success())
			return status;

		if (ignoringLine(line, properties))
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
		if (!status.success())
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
		properties.columnNames = determineColumnsSimplifyWhiteSpace(line, properties);
	else if (!properties.columnNamesRaw.isEmpty()) {
		// Determine column names from the names specified in the dialog
		// StartColumn is always one. Because otherwise I would need to specify column names for not required columns
		properties.columnNames = determineColumnsSimplifyWhiteSpace(properties.columnNamesRaw,
																	QLatin1String(INTERNAL_SEPARATOR),
																	removeQuotes,
																	true,
																	skipEmptyParts,
																	1,
																	properties.endColumn);
		if (properties.columnNames.isEmpty())
			return Status::UnableParsingHeader();
	} else {
		// Create default column names
		properties.columnNames.clear();
		const auto& values = determineColumnsSimplifyWhiteSpace(line, properties);
		for (int i = 0; i < values.length(); i++) {
			if (properties.endColumn > 0 && i >= (properties.endColumn - properties.startColumn + 1))
				break;
			properties.columnNames.append(i18n("Column %1", QString::number(i + 1)));
		}
	}

	const auto& lineSplit = determineColumnsSimplifyWhiteSpace(line, properties);
	const int numberColumns = lineSplit.count();

	// Determine end of columns
	if (properties.endColumn < 0) {
		// properties.headerEnabled = true: Determine the column count from the number of columns in the file
		// properties.headerEnabled = false: Use number of columns specified
		properties.endColumn = properties.startColumn + numberColumns;
	}

	if (properties.columnModesString.isEmpty()) {
		// Determine column modes
		QVector<QStringList> rows;
		size_t i = 0;
		if (!properties.headerEnabled) {
			rows.append(determineColumnsSimplifyWhiteSpace(line, properties));
			if (rows.last().count() != numberColumns)
				return Status::InvalidNumberDataColumns(numberColumns, rows.last().count(), i + 1);
			i++;
		} else {
			// Skip all lines until startRow line
			int j = properties.startRow - 1;
			while (j > 0) {
				const auto status = getLine(device, line);
				if (status.type() == Status::Type::DeviceAtEnd || status.type() == Status::Type::NoNewLine)
					break; // No more data to read. So we determine from the others
				if (!status.success())
					return status;

				if (ignoringLine(line, properties))
					continue;

				j--;
			}
		}
		while (i < m_dataTypeLines) {
			const auto status = getLine(device, line);
			if (status.type() == Status::Type::DeviceAtEnd || status.type() == Status::Type::NoNewLine)
				break; // No more data to read. So we determine from the others
			if (!status.success())
				return status;
			if (ignoringLine(line, properties))
				continue;
			rows.append(determineColumnsSimplifyWhiteSpace(line, properties));
			if (rows.last().count() != numberColumns) {
				if (properties.headerEnabled) {
					i += properties.startRow;
				}
				return Status::InvalidNumberDataColumns(numberColumns, rows.last().count(), i + 1);
			}
			i++;
		}
		QString dateTimeFormat;
		properties.columnModes.append(determineColumnModes(rows, properties, dateTimeFormat));
		if (properties.dateTimeFormat.isEmpty())
			properties.dateTimeFormat = dateTimeFormat;
	} else {
		QString invalidString;
		if (!determineColumnModes(properties.columnModesString, properties.columnModes, invalidString))
			return Status::ColumnModeDeterminationFailed();
	}

	if (properties.columnModes.size() != lineSplit.size())
		return Status::InvalidNumberDataColumns(numberColumns, properties.columnModes.size(), 1);

	if (properties.columnNames.size() != lineSplit.size())
		return Status::InvalidNumberColumnNames();

	// add time stamp and index column
	if (properties.createTimestamp) {
		properties.columnNames.prepend(i18n("Timestamp"));
		properties.columnModes.prepend(AbstractColumn::ColumnMode::DateTime);
	}
	if (properties.createIndex) {
		properties.columnNames.prepend(i18n("Index"));
		properties.columnModes.prepend(AbstractColumn::ColumnMode::BigInt);
	}

	initialized = true;
	return Status::Success();
}

QMap<QString, QPair<QString, AbstractColumn::ColumnMode>> AsciiFilterPrivate::modeMap() {
	using Mode = AbstractColumn::ColumnMode;
	return QMap<QString, QPair<QString, Mode>>{
		{QStringLiteral("Double"), {i18n("Double"), Mode::Double}},
		{QStringLiteral("Text"), {i18n("Text"), Mode::Text}},
		{QStringLiteral("DateTime"), {i18n("DateTime"), Mode::DateTime}},
		{QStringLiteral("Int"), {i18n("Int"), Mode::Integer}},
		{QStringLiteral("Int64"), {i18n("Int64"), Mode::BigInt}},
	};
}

bool AsciiFilterPrivate::determineColumnModes(const QStringView& s, QVector<AbstractColumn::ColumnMode>& modes, QString& invalidString) {
	const auto& modes_string = determineColumnsSimplifyWhiteSpace(s, QLatin1String(INTERNAL_SEPARATOR), false, true, false, 1, -1);

	const auto& modeMap = AsciiFilterPrivate::modeMap();

	for (const auto& m : modes_string) {
		bool found = false;
		for (auto it = modeMap.cbegin(), end = modeMap.cend(); it != end; it++) {
			if (it.key() == m || it.value().first == m) {
				modes << it.value().second;
				found = true;
				break;
			}
		}
		if (!found) {
			invalidString = m;
			return false;
		}
	}
	return true;
}

/*!
 * \brief AsciiFilterPrivate::convertTranslatedColumnModesToNative
 * Convert the string to the native string. So always common mode names are available
 * and any translated once
 * \param s
 * \return
 */
QString AsciiFilterPrivate::convertTranslatedColumnModesToNative(const QStringView s) {
	QVector<AbstractColumn::ColumnMode> modes;
	QString invalidString;
	QString res;
	if (!determineColumnModes(s, modes, invalidString))
		return res;

	const auto& modeMap = AsciiFilterPrivate::modeMap();
	for (const auto mode : modes) {
		for (auto it = modeMap.cbegin(), end = modeMap.cend(); it != end; it++) {
			if (it.value().second == mode)
				res += it.key() + QLatin1String(INTERNAL_SEPARATOR);
		}
	}
	if (res.endsWith(QLatin1String(INTERNAL_SEPARATOR)))
		res.removeLast();

	return res;
}

bool AsciiFilterPrivate::ignoringLine(QStringView line, const AsciiFilter::Properties& p) {
	return line.isEmpty() || (line.size() == 1 && line.at(0) == QLatin1Char('\n')) || (!p.commentCharacter.isEmpty() && line.startsWith(p.commentCharacter))
		|| (line.size() == 2 && line.at(0) == QLatin1Char('\r') && line.at(1) == QLatin1Char('\n'));
}

void AsciiFilterPrivate::setDataSource(AbstractDataSource* dataSource) {
	m_dataSource = dataSource;
	m_DataContainer = DataContainer();
}

/*!
 * \brief AsciiFilterPrivate::readFromDevice
 * \param device
 * \param dataSource
 * \param columnImportMode
 * \param from
 * \param lines
 * \param keepNRows After reading, keep n rows
 * \param bytes_read
 * \return
 */
Status AsciiFilterPrivate::readFromDevice(QIODevice& device,
										  AbstractFileFilter::ImportMode columnImportMode,
										  AbstractFileFilter::ImportMode rowImportMode,
										  qint64 from,
										  qint64 lines,
										  qint64 keepNRows,
										  qint64& bytes_read,
										  bool skipFirstLine) {
	bytes_read = 0;

	bool ok;
	if (!initialized) {
		const auto status = initialize(device);
		if (!status.success())
			return status;

		// matrix data has only one column mode
		if (dynamic_cast<Matrix*>(m_dataSource)) {
			for (auto& c : properties.columnModes)
				if (c != AbstractColumn::ColumnMode::Double)
					return Status::MatrixUnsupportedColumnMode();
		}
	}

	// TODO: This is dangerous, because it could be that now a different dataContainer is used than before.
	if (m_DataContainer.size() == 0) {
		std::vector<void*> dataContainer;
		if (!m_dataSource) {
			Q_ASSERT(false);
			return Status::NoDataSource();
		}
		// The column offset is already subtracted, so dataContainer contains only the new columns
		m_dataSource
			->prepareImport(dataContainer, columnImportMode, 0, properties.columnModes.size(), properties.columnNames, properties.columnModes, ok, true);

		if (dataContainer.size() == 0)
			return Status::NoColumns();

		// This must be done all the time, because it could be that the datacontainer of the datasource changed and then the datacontainer points to
		// wrong data locations.
		// Update
		// Initialize m_DataContainer. So m_DataContainer must not free up the data afterwards
		for (size_t i = 0; i < dataContainer.size(); i++)
			m_DataContainer.appendVector(dataContainer.at(i), properties.columnModes.at(i));
	}

	qsizetype dataContainerStartIndex = 0;
	if (rowImportMode == AbstractFileFilter::ImportMode::Replace) {
		// Replace all rows
		dataContainerStartIndex = 0;
	} else if (columnImportMode == AbstractFileFilter::ImportMode::Replace)
		dataContainerStartIndex = m_DataContainer.rowCount();
	// This is not implemented
	Q_ASSERT(rowImportMode != AbstractFileFilter::ImportMode::Prepend);

	try {
		const auto newRowCount = qMax(dataContainerStartIndex * 2, numberRowsReallocation);
		m_DataContainer.resize(newRowCount); // reserve to not having to reallocate all the time
	} catch (std::bad_alloc&) {
		return Status::NotEnoughMemory();
	}

	auto handleError = [this](Status status) {
		setLastError(status);
		m_DataContainer.resize(0);
		return status;
	};

	if (!device.isOpen()) {
		if (!device.open(QIODevice::ReadOnly))
			return handleError(Status::UnableToOpenDevice());
	}

	if (!device.isSequential())
		device.seek(from);

	if (device.atEnd() && !device.isSequential())
		return handleError(Status::DeviceAtEnd()); // File empty

	QString line;
	if (skipFirstLine) {
		const auto status = getLine(device, line);
		if (!status.success())
			return handleError(status);
	}

	int counter = 0;
	int startDataRow = 1;
	int rowIndex = dataContainerStartIndex;
	const size_t columnCountExpected = m_DataContainer.size() - properties.createIndex - properties.createTimestamp;
	QVector<QStringView> columnValues(columnCountExpected);
	const auto separatorLength = properties.separator.size();
	bool separatorSingleCharacter = separatorLength == 1;
	QChar separatorCharacter;
	if (separatorLength)
		separatorCharacter = properties.separator[separatorLength - 1];
	// Iterate over all rows
	do {
		const auto status = getLine(device, line);
		if (status.type() == Status::Type::DeviceAtEnd || status.type() == Status::Type::NoNewLine)
			break;
		else if (!status.success())
			return status;
		bytes_read += line.count();

		if (ignoringLine(line, properties))
			continue;

		counter++;
		if (properties.headerEnabled && properties.headerLine > 0) {
			if (counter <= properties.headerLine) {
				if (counter == properties.headerLine)
					startDataRow = counter + 1;
				continue;
			}
		}

		if ((counter - startDataRow + 1) < properties.startRow)
			continue;

		if (properties.createIndex) {
			m_DataContainer.setData(0, rowIndex, m_index);
			m_index++;
		}
		if (properties.createTimestamp) {
			// If create index is enabled +1 the timestamp is in the second column
			m_DataContainer.setData(properties.createIndex, rowIndex, QDateTime::currentDateTime());
		}

		// Now we get to the data rows
		if (properties.simplifyWhitespaces) {
			const auto& values = determineColumnsSimplifyWhiteSpace(line, properties);
			if ((size_t)values.size() < columnCountExpected)
				continue; // return Status::InvalidNumberDataColumns();
			setValues(values, rowIndex, properties);
		} else {
			// Higher performance if no whitespaces are available
			const auto columnCount = determineColumns(line, properties, separatorSingleCharacter, separatorCharacter, columnValues);
			if (columnCount < columnCountExpected)
				continue; // return Status::InvalidNumberDataColumns();
			setValues(columnValues, rowIndex, properties);
		}

		rowIndex++;
		if (rowIndex >= m_DataContainer.rowCount()) {
			try {
				m_DataContainer.resize(2 * m_DataContainer.rowCount()); // Always double
			} catch (std::bad_alloc&) {
				// q->setLastError(i18n("Not enough memory."));
				return Status::NotEnoughMemory();
			}
		}

		if (lines >= 0 && (rowIndex - dataContainerStartIndex) >= lines)
			break;

		if (rowIndex % 1000 == 0) {
			// ask to update the progress bar only if we have more than 1000 lines
			// only in 1% steps
			if (!device.isSequential()) {
				if (fileNumberLines > 0) {
					const auto value = 100. * rowIndex / fileNumberLines;
					Q_EMIT q->completed(static_cast<int>(value));
				}
			}
			QApplication::processEvents(QEventLoop::AllEvents, 0);
		}

	} while (true);

	int removedRows = rowIndex - keepNRows;
	if (keepNRows > 0 && removedRows > 0) {
		// Just keep the last n rows
		m_DataContainer.removeFirst(removedRows);
		m_DataContainer.resize(keepNRows);
	} else
		m_DataContainer.resize(rowIndex);

	m_dataSource->finalizeImport(0, 0, properties.columnNames.size() - 1, properties.dateTimeFormat, columnImportMode);
	return Status::Success();
}

template<typename T>
void AsciiFilterPrivate::setValues(const QVector<T>& values, int rowIndex, const AsciiFilter::Properties& properties) {
	int columnIndex = 0 + properties.createIndex + properties.createTimestamp;
	// Iterate over all columns
	for (const auto& value : values) {
		bool conversionOk = false;
		if (columnIndex >= properties.columnModes.length())
			return;
		switch (properties.columnModes[columnIndex]) {
		case AbstractColumn::ColumnMode::Double: {
			double d = properties.locale.toDouble(value, &conversionOk);
			if (!conversionOk) {
				d = properties.nanValue;
			}
			m_DataContainer.setData(columnIndex, rowIndex, d);
			break;
		}
		case AbstractColumn::ColumnMode::Integer: {
			int i = properties.locale.toInt(value, &conversionOk);
			if (!conversionOk) {
				i = 0;
			}
			m_DataContainer.setData(columnIndex, rowIndex, i);
			break;
		}
		case AbstractColumn::ColumnMode::BigInt: {
			qint64 i = properties.locale.toLongLong(value, &conversionOk);
			if (!conversionOk) {
				i = 0;
			}
			m_DataContainer.setData(columnIndex, rowIndex, i);
			break;
		}
		case AbstractColumn::ColumnMode::Text:
			m_DataContainer.setData(columnIndex, rowIndex, value); // Because value can be QString or QStringView
			break;
		case AbstractColumn::ColumnMode::Month:
		case AbstractColumn::ColumnMode::Day:
		case AbstractColumn::ColumnMode::DateTime: {
			auto dt = QDateTime::fromString(value, properties.dateTimeFormat, properties.baseYear);
			dt.setTimeSpec(Qt::UTC);
			m_DataContainer.setData(columnIndex, rowIndex, dt);
			break;
		}
		}
		columnIndex++;
	}
}

/*!
 * Determines the column modes from the provided rows
 *
 * \brief AsciiFilterPrivate::determineColumnModes
 * \param dateTimeFormat The datetime format will be updated if it is empty by the detected format
 * \return
 */
QVector<AbstractColumn::ColumnMode>
AsciiFilterPrivate::determineColumnModes(const QVector<QStringList>& rows, const AsciiFilter::Properties& properties, QString& dateTimeFormat) {
	using Mode = AbstractColumn::ColumnMode;

	dateTimeFormat = properties.dateTimeFormat;

	QVector<Mode> modes;
	if (rows.length() == 0)
		return modes;

	int columnCount = rows.first().count();
	for (int i = 0; i < columnCount; i++)
		modes.append(Mode::Integer);

	bool first = true;
	for (const auto& row : rows) {
		int columnIndex = 0;
		for (auto column : row) {
			if (columnIndex >= columnCount)
				break;

			if (properties.simplifyWhitespaces)
				column = column.simplified();
			if (properties.removeQuotes)
				column.remove(QLatin1Char('"'));
			auto mode = AbstractFileFilter::columnMode(column, dateTimeFormat, properties.locale, properties.intAsDouble, properties.baseYear);

			if (properties.intAsDouble) {
				if (mode == Mode::Integer || mode == Mode::BigInt)
					mode = Mode::Double;
			}

			if (first)
				modes[columnIndex] = mode;
			else if (!column.isEmpty()) {
				if (mode == Mode::Double && modes[columnIndex] == Mode::Integer) {
					// numeric: integer -> numeric
					modes[columnIndex] = mode;
				} else if (mode == Mode::Text && modes[columnIndex] != Mode::Text) {
					// text: non text -> text
					modes[columnIndex] = mode;
				} else if (mode == Mode::BigInt && modes[columnIndex] == Mode::Integer)
					modes[columnIndex] = mode;
				/* else if (mode != Mode::Text && modes[columnIndex] == Mode::Text) {
					// numeric: text -> numeric/integer
					modes[columnIndex] = mode;
				}*/
			}
			columnIndex++;
		}
		first = false;
	}
	return modes;
}

QStringList AsciiFilterPrivate::determineColumnsSimplifyWhiteSpace(const QStringView& line, const AsciiFilter::Properties& properties) {
	return determineColumnsSimplifyWhiteSpace(line,
											  properties.separator,
											  properties.removeQuotes,
											  properties.simplifyWhitespaces,
											  properties.skipEmptyParts,
											  properties.startColumn,
											  properties.endColumn);
}

namespace {
// Have them already in QChar leads that below any conversion must be done
const QChar newlineChar(QLatin1Char('\n'));
const QChar carriageReturnChar(QLatin1Char('\r'));
const QChar quoteChar(QLatin1Char('"'));
const QChar spaceChar(QLatin1Char(' '));
const QChar tabChar(QLatin1Char('\t'));
}

size_t AsciiFilterPrivate::determineColumns(const QStringView& line,
											const AsciiFilter::Properties& properties,
											bool separatorSingleCharacter,
											const QChar separatorCharacter,
											QVector<QStringView>& columnValues) {
	// Simplified
	if (properties.simplifyWhitespaces)
		return 0;

	enum class State {
		Column,
		QuotedText,
	};

	const auto maxColumnCount = columnValues.size();
	auto state = State::Column;
	int columnCount = 1;
	bool separatorLast = false;
	size_t counter = 0;
	size_t startColumnIndex = 0; // Start of the column name
	size_t numberCharacters = 0; // Number of characters of the column name
	int columnIndex = 0;
	for (auto c : line) {
		counter++;
		if (c == newlineChar || c == carriageReturnChar)
			break;
		if (properties.removeQuotes && c == quoteChar) {
			switch (state) {
			case State::Column:
				state = State::QuotedText;
				startColumnIndex = counter;
				numberCharacters = 0;
				continue;
			case State::QuotedText:
				state = State::Column;
				// Do not increase numberCharacters
				continue;
			}
		}

		switch (state) {
		case State::Column: {
			bool separatorFound;
			if (separatorSingleCharacter)
				separatorFound = c == separatorCharacter;
			else
				separatorFound = !properties.separator.isEmpty() && line.sliced(startColumnIndex, counter - startColumnIndex).endsWith(properties.separator);
			if (separatorFound) {
				separatorLast = true;
				const auto columnName = line.sliced(startColumnIndex, numberCharacters);
				// columnName.remove(columnName.length() - separator.length(), separator.length());
				if (!properties.skipEmptyParts || !columnName.isEmpty()) {
					if (columnCount >= properties.startColumn && (columnCount <= properties.endColumn || properties.endColumn < 0)
						&& columnIndex < maxColumnCount) {
						columnValues[columnIndex] = columnName;
						columnIndex++;
					}
					columnCount++;
				}
				// columnName.clear();
				startColumnIndex = counter;
				numberCharacters = 0;
			} else {
				separatorLast = false;
				numberCharacters++;
			}
			break;
		}
		case State::QuotedText:
			numberCharacters++;
			break;
		}
	}
	if (columnCount >= properties.startColumn && (columnCount <= properties.endColumn || properties.endColumn < 0)) {
		// If columnName is empty(): After the separator the line was finished, but there should be a value so add a placeholder (invalid value)
		if ((numberCharacters != 0 || (!properties.skipEmptyParts && separatorLast)) && columnIndex < maxColumnCount) {
			columnValues[columnIndex] = line.sliced(startColumnIndex, numberCharacters);
			return columnIndex + 1;
		}
	}
	return columnIndex;
}

QStringList AsciiFilterPrivate::determineColumnsSimplifyWhiteSpace(QStringView line,
																   const QString& separator,
																   bool removeQuotes,
																   bool simplifyWhiteSpaces,
																   bool skipEmptyParts,
																   int startColumn,
																   int endColumn) {
	enum class State {
		Column,
		QuotedText,
	};

	QStringList columnNames;
	auto state = State::Column;
	// More complicated algorithm because whitespaces are removed
	QString columnName;
	int columnCount = 1;
	bool separatorLast = false;
	QChar lastCharacter;

	for (auto c : line) {
		if (c == newlineChar || c == carriageReturnChar)
			break;
		if (removeQuotes && c == quoteChar) {
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

		if (simplifyWhiteSpaces && state != State::QuotedText && (c == spaceChar || c == tabChar)) {
			if (lastCharacter == spaceChar)
				continue;
			else
				c = spaceChar; // Replace by whitespace
		}
		columnName.append(c);

		lastCharacter = c;

		switch (state) {
		case State::Column: {
			if (!separator.isEmpty() && columnName.endsWith(separator)) {
				separatorLast = true;
				columnName.remove(columnName.length() - separator.length(), separator.length());
				if (!skipEmptyParts || !columnName.isEmpty()) {
					if (columnCount >= startColumn && (columnCount <= endColumn || endColumn < 0)) {
						if (simplifyWhiteSpaces)
							columnName = columnName.simplified();
						columnNames << columnName;
					}
					columnCount++;
				}
				columnName.clear();
			} else
				separatorLast = false;
			break;
		}
		case State::QuotedText:
			continue;
		}
	}
	if (columnCount >= startColumn && (columnCount <= endColumn || endColumn < 0)) {
		while (simplifyWhiteSpaces && columnName.size() > 0 && (columnName.last(1) == spaceChar || columnName.last(1) == tabChar))
			columnName.removeLast();
		// If columnName is empty(): After the separator the line was finished, but there should be a value so add a placeholder (invalid value)
		if (!columnName.isEmpty() || (!skipEmptyParts && separatorLast)) {
			if (simplifyWhiteSpaces)
				columnName = columnName.simplified();
			columnNames.append(columnName);
		}
	}
	return columnNames;
}

Status AsciiFilterPrivate::determineSeparator(const QString& line, bool removeQuotes, bool simplifyWhiteSpaces, QString& separator) {
	enum class State {
		Column,
		QuotedText,
	};
	QVector<QChar> allowedSeparatorCharactersNonWhiteSpace{QLatin1Char(','), QLatin1Char(';'), QLatin1Char('|')};
	QStringView lineView = line;

	separator.clear();

	// Remove whitespaces from the beginning
	if (simplifyWhiteSpaces) {
		int counter = 0;
		for (auto c : lineView) {
			if (c == QLatin1Char(' ') || c == QLatin1Char('\t')) {
				counter++;
				continue;
			}
			if (counter == 0)
				break;
			if (counter < lineView.size()) {
				lineView = lineView.sliced(counter, lineView.size() - counter);
			} else
				return Status::SeparatorDeterminationFailed(); // Nothing found
			break;
		}
	}

	auto state = State::Column;
	QString separatorSequence;
	for (auto c : lineView) {
		if (removeQuotes && c == QLatin1Char('"')) {
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
			for (auto asc : allowedSeparatorCharactersNonWhiteSpace) {
				// Simple non whitespace characters as separators
				if (asc == c) {
					separator = c; // This is the separator character
					return Status::Success();
				}
			}
			// Complex whitespace characters as separators
			// It can be any combination of those whitespaces
			if (c == QLatin1Char(' ') || c == QLatin1Char('\t')) {
				if (simplifyWhiteSpaces) {
					// First whitespace found. Since simplifyWhitespaces all following whitespaces
					// get merged into a single space and therefore a simple space must be the separator
					separator = QLatin1Char(' ');
					return Status::Success();
				}
				separatorSequence.append(c);
			} else if (!separatorSequence.isEmpty()) {
				separator = separatorSequence;
				return Status::Success();
			}
			break;
		}
		case State::QuotedText:
			continue; // Nothing to do, since we are inside of a quoted text
		}
	}

	return Status::Success(); // Only one column, so no separator exists
}

Status AsciiFilterPrivate::getLine(QIODevice& device, QString& line) {
	auto* udpSocket = dynamic_cast<QUdpSocket*>(&device);
	if (udpSocket) {
		if (udpSocket->hasPendingDatagrams()) {
			// TODO: Maybe using readDatagram and a const size array?
			const auto& datagram = udpSocket->receiveDatagram();
			line = QString::fromUtf8(datagram.data());
			return Status::Success();
		} else
			return Status::DeviceAtEnd();
	}

	if (device.atEnd()) {
		return Status::DeviceAtEnd();
	}

	// This is important especially for serial port because readLine reads everything from the buffer
	// even if it is not a complete line. So without we would get partly lines which get parsed wrongly
	if (!device.canReadLine()) {
		// Seems to be that KCompressionDevice has problems with this function
		if (!dynamic_cast<KCompressionDevice*>(&device) && !dynamic_cast<QFile*>(&device) && !dynamic_cast<BufferReader*>(&device))
			return Status::NoNewLine();
	}

	line = QString::fromUtf8(device.readLine());
	return Status::Success();
}

/*!
 * \brief AsciiFilterPrivate::initialize
 * Initialize the Asciifilter so that it can be parsed without any further knowledge
 * This is used for the live data because there everything must be specified prior
 * starting the parsing
 * \param p
 * \return
 */
Status AsciiFilterPrivate::initialize(AsciiFilter::Properties p) {
	using ColumnMode = AbstractColumn::ColumnMode;

	if (properties.endColumn > 0 && properties.endColumn < properties.startColumn)
		return setLastError(Status::WrongEndColumn());

	if (properties.endRow > 0 && properties.endRow < properties.startRow)
		return setLastError(Status::WrongEndRow());

	if (p.automaticSeparatorDetection)
		return setLastError(Status::SeparatorDetectionNotAllowed());

	if (p.separator.isEmpty())
		return setLastError(Status::InvalidSeparator());

	if (p.columnModes.isEmpty()) {
		if (p.columnModesString.isEmpty())
			return setLastError(Status::SequentialDeviceNoColumnModes());

		QString invalidString;
		if (!determineColumnModes(p.columnModesString, p.columnModes, invalidString))
			return setLastError(Status::SequentialDeviceNoColumnModes());
	}

	if (p.columnNamesRaw.isEmpty() && p.columnNames.isEmpty()) {
		// Create default column names
		p.columnNames.clear();
		for (int i = 0; i < p.columnModes.count(); i++) {
			if (p.endColumn > 0 && i >= (p.endColumn - p.startColumn + 1))
				break;
			p.columnNames.append(i18n("Column %1").arg(QString::number(i + 1)));
		}
	} else if (p.columnNames.isEmpty())
		p.columnNames =
			determineColumnsSimplifyWhiteSpace(p.columnNamesRaw, QLatin1String(INTERNAL_SEPARATOR), p.removeQuotes, true, p.skipEmptyParts, 1, p.endColumn);

	if (p.columnNames.isEmpty())
		return setLastError(Status::UnableParsingHeader());

	if (p.columnModes.count() != p.columnNames.count())
		return setLastError(Status::SequentialDeviceNoColumnModes());

	if (p.headerEnabled)
		return setLastError(Status::HeaderDetectionNotAllowed());

	if (p.dateTimeFormat.isEmpty()) {
		for (const auto m : p.columnModes) {
			if (m == ColumnMode::DateTime || m == ColumnMode::Month || m == ColumnMode::Day)
				return setLastError(Status::NoDateTimeFormat());
		}
	}

	if (p.createTimestamp) {
		p.columnNames.prepend(i18n("Timestamp"));
		p.columnModes.prepend(AbstractColumn::ColumnMode::DateTime);
	}
	if (p.createIndex) {
		p.columnNames.prepend(i18n("Index"));
		p.columnModes.prepend(AbstractColumn::ColumnMode::BigInt);
	}

	properties = p;
	initialized = true;
	return Status::Success();
}

QVector<QStringList> AsciiFilterPrivate::preview(const QString& fileName, int lines, bool reinit) {
	KCompressionDevice file(fileName);
	return preview(file, lines, reinit, false);
}

/*!
 * \brief AsciiFilterPrivate::preview
 * \param device
 * \param lines
 * \param reinit
 * \param skipFirstLine Skip first line without reading. Important for example for the Serial port where it might be that the first line does not contain a
 * complete line \return
 */
QVector<QStringList> AsciiFilterPrivate::preview(QIODevice& device, int lines, bool reinit, bool skipFirstLine) {
	Spreadsheet spreadsheet(QStringLiteral("AsciiFilterPreviewSpreadsheet"));
	if (reinit)
		initialized = false;

	if (isUTF16(device)) {
		setLastError(Status::UTF16NotSupported());
		return {};
	}

	q->clearLastError();
	qint64 bytes_read;
	setDataSource(&spreadsheet);
	CleanupNoArguments cleanup([this]() {
		this->setDataSource(nullptr);
	});
	const auto status =
		readFromDevice(device, AbstractFileFilter::ImportMode::Replace, AbstractFileFilter::ImportMode::Replace, 0, lines, 0, bytes_read, skipFirstLine);
	QVector<QStringList> p;
	if (!status.success()) {
		setLastError(status);
		return p;
	}

	// If data are coming slowly, wait until filled
	constexpr int sleep_ms = 500;
	constexpr int timeout_ms = 3000;
	int counter = 0;
	while (device.isSequential() && spreadsheet.rowCount() < 10 && spreadsheet.rowCount() < lines) {
		QEventLoop loop;
		QTimer t;
		t.connect(&t, &QTimer::timeout, &loop, &QEventLoop::quit);
		t.start(sleep_ms);
		loop.exec();

		const auto status =
			readFromDevice(device, AbstractFileFilter::ImportMode::Replace, AbstractFileFilter::ImportMode::Append, 0, lines, 0, bytes_read, skipFirstLine);
		if (!status.success()) {
			setLastError(status);
			return p;
		}
		counter++;
		if (counter * sleep_ms >= timeout_ms)
			break;
	}

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
				line << QString::number(c->valueAt(row), 'g', q->previewPrecision());
				break;
			case AbstractColumn::ColumnMode::Day:
			case AbstractColumn::ColumnMode::Month:
			case AbstractColumn::ColumnMode::DateTime:
				line << c->dateTimeAt(row).toString(properties.dateTimeFormat);
				break;
			case AbstractColumn::ColumnMode::Text:
				line << c->textAt(row);
				break;
			}
		}
		p << line;
	}
	return p;
}

Status AsciiFilterPrivate::setLastError(Status status) {
	lastStatus = status;
	if (!status.success())
		q->setLastError(status.message());
	return status;
}

/*!
 * returns \true if the the data coming from the device is UTF-16 encoded, returns \c false otherwise.
 * In case the device couldn't be opened or the number of the available bytes is not sufficient to determine the encoding,
 * \c false is returned and the caller needs to handle these error cases properly.
 */
bool AsciiFilterPrivate::isUTF16(QIODevice& device) {
	if (!device.isOpen()) {
		if (!device.open(QIODevice::ReadOnly))
			return false;
	}

	// read the first two bytes to check the "byte order mark" (BOM),
	// UTF-16 encoded files typically start with a BOM to indicate the endianness of the encoding.
	char buffer[2];
	if (!device.read(buffer, 2))
		return false;

	const auto byte1 = static_cast<unsigned char>(buffer[0]);
	const auto byte2 = static_cast<unsigned char>(buffer[1]);

	if (!device.isSequential())
		device.reset(); // seek to the start

	// Check for UTF-16 BOM (0xFEFF for UTF-16 Big Endian, 0xFFFE for UTF-16 Little Endian)
	if ((byte1 == 0xFE && byte2 == 0xFF) || (byte1 == 0xFF && byte2 == 0xFE))
		return true;

	return false;
}

// #################################################################################################
// ###  Data container  ############################################################################
// #################################################################################################
/*!
 * \brief dataContainer
 * Do not modify outside as long as DataContainer exists!
 * \return
 */
std::vector<void*>& AsciiFilterPrivate::DataContainer::dataContainer() {
	return m_dataContainer;
}

AbstractColumn::ColumnMode AsciiFilterPrivate::DataContainer::columnMode(int index) const {
	return m_columnModes.at(index);
}

const void* AsciiFilterPrivate::DataContainer::datas(qsizetype index) const {
	if (index < size())
		return m_dataContainer.at(index);
	return nullptr;
}

void AsciiFilterPrivate::DataContainer::removeFirst(int n) {
	n = qMin(n, rowCount());
	if (n <= 0)
		return;
	for (uint32_t i = 0; i < m_dataContainer.size(); i++) {
		switch (m_columnModes.at(i)) {
		case AbstractColumn::ColumnMode::BigInt:
			static_cast<QVector<qint64>*>(m_dataContainer[i])->remove(0, n);
			break;
		case AbstractColumn::ColumnMode::Integer: {
			static_cast<QVector<qint32>*>(m_dataContainer[i])->remove(0, n);
			break;
		}
		case AbstractColumn::ColumnMode::Double:
			static_cast<QVector<double>*>(m_dataContainer[i])->remove(0, n);
			break;
		case AbstractColumn::ColumnMode::Text:
			static_cast<QVector<QString>*>(m_dataContainer[i])->remove(0, n);
			break;
		case AbstractColumn::ColumnMode::Month:
		case AbstractColumn::ColumnMode::Day:
		case AbstractColumn::ColumnMode::DateTime:
			static_cast<QVector<QDateTime>*>(m_dataContainer[i])->remove(0, n);
			break;
		}
	}
}

bool AsciiFilterPrivate::DataContainer::resize(qsizetype s) const {
	for (unsigned long i = 0; i < m_dataContainer.size(); i++) {
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
	int size = rowCount(0);
	if (size == -1)
		return false;

	for (size_t i = 1; i < m_dataContainer.size(); i++) {
		if (size != rowCount(i))
			return false;
	}
	return true;
}

bool AsciiFilterPrivate::DataContainer::reserve(qsizetype s) const {
	for (unsigned long i = 0; i < m_dataContainer.size(); i++) {
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
	int size = rowCount(0);
	if (size == -1)
		return false;

	for (uint32_t i = 1; i < m_dataContainer.size(); i++) {
		if (size != rowCount(i))
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

int AsciiFilterPrivate::DataContainer::rowCount(unsigned long index) const {
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
	Q_ASSERT(false);
	return -1;
}

qsizetype AsciiFilterPrivate::DataContainer::size() const {
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
