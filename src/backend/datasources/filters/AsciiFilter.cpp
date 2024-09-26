/*
	File                 : AsciiFilter.cpp
	Project              : LabPlot
	Description          : ASCII I/O-filter
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2009-2024 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-FileCopyrightText: 2009-2024 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "backend/datasources/filters/AsciiFilter.h"
#include "backend/core/Project.h"
#include "backend/core/column/Column.h"
#include "backend/datasources/LiveDataSource.h"
#include "backend/datasources/filters/AsciiFilterPrivate.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/hostprocess.h"
#include "backend/lib/macros.h"
#include "backend/lib/trace.h"
#include "backend/matrix/Matrix.h"

#include "3rdparty/stringtokenizer/qstringtokenizer.h"

#ifdef HAVE_MQTT
#include "backend/datasources/MQTTClient.h"
#include "backend/datasources/MQTTTopic.h"
#endif

#include <KCompressionDevice>
#include <KLocalizedString>
#include <QDateTime>

#if defined(Q_OS_LINUX) || defined(Q_OS_BSD4)
#include <QProcess>
#include <QStandardPaths>
#endif

#include <QRegularExpression>

/*!
\class AsciiFilter
\brief Manages the import/export of data organized as columns (vectors) from/to an ASCII-file.

\ingroup datasources
*/
AsciiFilter::AsciiFilter()
	: AbstractFileFilter(FileType::Ascii)
	, d(new AsciiFilterPrivate(this)) {
}

AsciiFilter::~AsciiFilter() = default;

/*!
  reads the content of the device \c device.
*/
void AsciiFilter::readDataFromDevice(QIODevice& device, AbstractDataSource* dataSource, ImportMode importMode, int lines) {
	d->readDataFromDevice(device, dataSource, importMode, lines);
}

void AsciiFilter::readFromLiveDeviceNotFile(QIODevice& device, AbstractDataSource* dataSource) {
	d->readFromLiveDevice(device, dataSource);
}

qint64 AsciiFilter::readFromLiveDevice(QIODevice& device, AbstractDataSource* dataSource, qint64 from) {
	return d->readFromLiveDevice(device, dataSource, from);
}

#ifdef HAVE_MQTT
QVector<QStringList> AsciiFilter::preview(const QString& message) {
	return d->preview(message);
}

/*!
  reads the content of a message received by the topic.
*/
void AsciiFilter::readMQTTTopic(const QString& message, AbstractDataSource* dataSource) {
	d->readMQTTTopic(message, dataSource);
}

/*!
  After the MQTTTopic is loaded, prepares the filter for reading.
*/
void AsciiFilter::setPreparedForMQTT(bool prepared, MQTTTopic* topic, const QString& separator) {
	d->setPreparedForMQTT(prepared, topic, separator);
}
#endif

/*!
  returns the separator used by the filter.
*/
QString AsciiFilter::separator() const {
	return d->separator();
}

/*!
  returns the separator used by the filter.
*/
int AsciiFilter::isPrepared() {
	return d->isPrepared();
}

/*!
  reads the content of the file \c fileName.
*/
void AsciiFilter::readDataFromFile(const QString& fileName, AbstractDataSource* dataSource, ImportMode importMode) {
	d->readDataFromFile(fileName, dataSource, importMode);
}

QVector<QStringList> AsciiFilter::preview(const QString& fileName, int lines) {
	return d->preview(fileName, lines);
}

QVector<QStringList> AsciiFilter::preview(QIODevice& device) {
	return d->preview(device);
}

/*!
writes the content of the data source \c dataSource to the file \c fileName.
*/
void AsciiFilter::write(const QString& fileName, AbstractDataSource* dataSource) {
	d->write(fileName, dataSource);
}

/*!
  returns the list with the names of all saved
  (system wide or user defined) filter settings.
*/
QStringList AsciiFilter::predefinedFilters() {
	// TODO?
	return {};
}

/*!
  returns the list of all predefined separator characters.
*/
QStringList AsciiFilter::separatorCharacters() {
	return (QStringList() << QStringLiteral("auto") << QStringLiteral("TAB") << QStringLiteral("SPACE") << QStringLiteral(",") << QStringLiteral(";")
						  << QStringLiteral(":") << QStringLiteral(",TAB") << QStringLiteral(";TAB") << QStringLiteral(":TAB") << QStringLiteral(",SPACE")
						  << QStringLiteral(";SPACE") << QStringLiteral(":SPACE") << QStringLiteral("2xSPACE") << QStringLiteral("3xSPACE")
						  << QStringLiteral("4xSPACE") << QStringLiteral("2xTAB"));
}

/*!
returns the list of all predefined comment characters.
*/
QStringList AsciiFilter::commentCharacters() {
	return (QStringList() << QStringLiteral("#") << QStringLiteral("!") << QStringLiteral("//") << QStringLiteral("+") << QStringLiteral("c")
						  << QStringLiteral(":") << QStringLiteral(";"));
}

/*!
returns the list of all predefined data types.
*/
QStringList AsciiFilter::dataTypes() {
	const QMetaObject& mo = AbstractColumn::staticMetaObject;
	const QMetaEnum& me = mo.enumerator(mo.indexOfEnumerator("ColumnMode"));
	QStringList list;
	for (int i = 0; i <= 100; ++i) // me.keyCount() does not work because we have holes in enum
		if (me.valueToKey(i))
			list << QLatin1String(me.valueToKey(i));
	return list;
}

QString AsciiFilter::fileInfoString(const QString& fileName) {
	QString info(i18n("Number of columns: %1", AsciiFilter::columnNumber(fileName)));
	info += QStringLiteral("<br>");
	info += i18n("Number of lines: %1", AsciiFilter::lineNumber(fileName));
	return info;
}

/*!
	returns the number of columns in the file \c fileName.
*/
int AsciiFilter::columnNumber(const QString& fileName, const QString& separator) {
	KCompressionDevice device(fileName);
	if (!device.open(QIODevice::ReadOnly)) {
		DEBUG(Q_FUNC_INFO << ", Could not open file " << STDSTRING(fileName) << " for determining number of columns");
		return -1;
	}

	QString line = QLatin1String(device.readLine());
	line.remove(QRegularExpression(QStringLiteral("[\\n\\r]")));

	QStringList lineStringList;
	if (separator.length() > 0)
		lineStringList = line.split(separator);
	else
		lineStringList = line.split(QRegularExpression(QStringLiteral("\\s+")));
	DEBUG(Q_FUNC_INFO << ", number of columns : " << lineStringList.size());

	return lineStringList.size();
}

size_t AsciiFilter::lineNumber(const QString& fileName, const size_t maxLines) {
	KCompressionDevice device(fileName);

	if (!device.open(QIODevice::ReadOnly)) {
		DEBUG(Q_FUNC_INFO << ", Could not open file " << STDSTRING(fileName) << " to determine number of lines");

		return 0;
	}
	// 	if (!device.canReadLine())
	// 		return -1;

#if defined(Q_OS_LINUX) || defined(Q_OS_BSD4)
	if (maxLines == std::numeric_limits<std::size_t>::max()) { // only when reading all lines
		// on Linux and BSD use grep, if available, which is much faster than counting lines in the file
		// wc -l does not count last line when not ending in line break!
		DEBUG(Q_FUNC_INFO << ", using 'grep' or 'sed' to count lines")
		QString cmdFullPath = safeExecutableName(QStringLiteral("grep"));
		QStringList options;
		options << QStringLiteral("-e") << QStringLiteral("^") << QStringLiteral("-c") << fileName;
		if (cmdFullPath.isEmpty()) { // alternative: sed -n '$='
			DEBUG(Q_FUNC_INFO << ", 'grep' not found using 'sed' instead")
			cmdFullPath = safeExecutableName(QStringLiteral("sed"));
			options.clear();
			options << QStringLiteral("-n") << QStringLiteral("$=") << fileName;
		}
		if (device.compressionType() == KCompressionDevice::None && !cmdFullPath.isEmpty()) {
			QProcess cmd;
			startHostProcess(cmd, cmdFullPath, options);
			size_t lineCount = 0;
			while (cmd.waitForReadyRead()) {
				QString line = QLatin1String(cmd.readLine());
				QDEBUG("line count command output: " << line)
				// wc on macOS has leading spaces: use SkipEmptyParts
				lineCount = line.split(QLatin1Char(' '), Qt::SkipEmptyParts).at(0).toInt();
			}
			return lineCount;
		} else {
			DEBUG(Q_FUNC_INFO << ", 'grep' or 'sed' not found using readLine()")
		}
	}
#endif

	size_t lineCount = 0;
	while (!device.atEnd()) {
		if (lineCount >= maxLines) // stop when maxLines available
			return lineCount;
		device.readLine();
		lineCount++;
	}

	return lineCount;
}

/*!
  returns the number of lines in the device \c device (limited by maxLines) and 0 if sequential.
  resets the position to 0!
*/
size_t AsciiFilter::lineNumber(QIODevice& device, const size_t maxLines) const {
	if (device.isSequential())
		return 0;
	// 	if (!device.canReadLine())
	// 		DEBUG("WARNING in AsciiFilter::lineNumber(): device cannot 'readLine()' but using it anyway.");

	size_t lineCount = 0;
	device.seek(0);
	if (d->readingFile)
		lineCount = lineNumber(d->readingFileName, maxLines);
	else {
		while (!device.atEnd()) {
			if (lineCount >= maxLines) // stop when maxLines available
				break;
			device.readLine();
			lineCount++;
		}
	}
	device.seek(0);

	return lineCount;
}

void AsciiFilter::setCommentCharacter(const QString& s) {
	d->commentCharacter = s;
}
QString AsciiFilter::commentCharacter() const {
	return d->commentCharacter;
}

void AsciiFilter::setSeparatingCharacter(const QString& s) {
	d->separatingCharacter = s;
}
QString AsciiFilter::separatingCharacter() const {
	return d->separatingCharacter;
}

void AsciiFilter::setDateTimeFormat(const QString& f) {
	d->dateTimeFormat = f;
}
QString AsciiFilter::dateTimeFormat() const {
	return d->dateTimeFormat;
}

void AsciiFilter::setNumberFormat(QLocale::Language lang) {
	d->numberFormat = lang;
	d->locale = QLocale(lang);
}
QLocale::Language AsciiFilter::numberFormat() const {
	return d->numberFormat;
}

void AsciiFilter::setAutoModeEnabled(const bool b) {
	d->autoModeEnabled = b;
}
bool AsciiFilter::isAutoModeEnabled() const {
	return d->autoModeEnabled;
}

void AsciiFilter::setHeaderEnabled(const bool b) {
	d->headerEnabled = b;
}
bool AsciiFilter::isHeaderEnabled() const {
	return d->headerEnabled;
}

void AsciiFilter::setHeaderLine(int line) {
	d->headerLine = line;
	DEBUG(Q_FUNC_INFO << ", line = " << line << ", startRow = " << d->startRow)
}

void AsciiFilter::setSkipEmptyParts(const bool b) {
	d->skipEmptyParts = b;
}
bool AsciiFilter::skipEmptyParts() const {
	return d->skipEmptyParts;
}

void AsciiFilter::setCreateIndexEnabled(bool b) {
	d->createIndexEnabled = b;
}

bool AsciiFilter::createIndexEnabled() const {
	return d->createIndexEnabled;
}

void AsciiFilter::setCreateTimestampEnabled(bool b) {
	d->createTimestampEnabled = b;
}

bool AsciiFilter::createTimestampEnabled() const {
	return d->createTimestampEnabled;
}

void AsciiFilter::setSimplifyWhitespacesEnabled(bool b) {
	d->simplifyWhitespacesEnabled = b;
}
bool AsciiFilter::simplifyWhitespacesEnabled() const {
	return d->simplifyWhitespacesEnabled;
}

void AsciiFilter::setNaNValueToZero(bool b) {
	if (b)
		d->nanValue = 0.;
	else
		d->nanValue = std::numeric_limits<double>::quiet_NaN();
}
bool AsciiFilter::NaNValueToZeroEnabled() const {
	return (d->nanValue == 0.);
}

void AsciiFilter::setRemoveQuotesEnabled(bool b) {
	d->removeQuotesEnabled = b;
}
bool AsciiFilter::removeQuotesEnabled() const {
	return d->removeQuotesEnabled;
}

void AsciiFilter::setVectorNames(const QString& s) {
	d->columnNames.clear();
	if (!s.simplified().isEmpty())
		d->columnNames = s.simplified().split(QLatin1Char(' '));
}
void AsciiFilter::setVectorNames(const QStringList& list) {
	d->columnNames = list;
}
QStringList AsciiFilter::vectorNames() const {
	return d->columnNames;
}

QVector<AbstractColumn::ColumnMode> AsciiFilter::columnModes() {
	return d->columnModes;
}

void AsciiFilter::setStartRow(const int r) {
	DEBUG(Q_FUNC_INFO << " row:" << r)
	d->startRow = r;
}
int AsciiFilter::startRow() const {
	return d->startRow;
}

void AsciiFilter::setEndRow(const int r) {
	d->endRow = r;
}
int AsciiFilter::endRow() const {
	return d->endRow;
}

void AsciiFilter::setStartColumn(const int c) {
	d->startColumn = c;
}
int AsciiFilter::startColumn() const {
	return d->startColumn;
}

void AsciiFilter::setEndColumn(const int c) {
	d->endColumn = c;
}
int AsciiFilter::endColumn() const {
	return d->endColumn;
}

// #####################################################################
// ################### Private implementation ##########################
// #####################################################################
AsciiFilterPrivate::AsciiFilterPrivate(AsciiFilter* owner)
	: q(owner) {
}

int AsciiFilterPrivate::isPrepared() {
	return m_prepared;
}

/*!
 * \brief Returns the separator used by the filter
 * \return
 */
QString AsciiFilterPrivate::separator() const {
	return m_separator;
}

// #####################################################################
// ############################# Read ##################################
// #####################################################################
/*!
 * returns -1 if the device couldn't be opened, 1 if the current read position in the device is at the end and 0 otherwise.
 */
int AsciiFilterPrivate::prepareDeviceToRead(QIODevice& device, const size_t maxLines) {
	DEBUG(Q_FUNC_INFO << ", is sequential = " << device.isSequential() << ", can readLine = " << device.canReadLine());

	if (!device.open(QIODevice::ReadOnly)) {
		DEBUG(Q_FUNC_INFO << ", ERROR: could not open file for reading!")
		return -1;
	}

	if (device.atEnd() && !device.isSequential()) { // empty file
		DEBUG(Q_FUNC_INFO << ", ERROR: file is empty!")
		return 1;
	}

	// NEW method
	// 1. First read header (vector names) at given headerLine (counting comment lines)
	// 2. Go further to first data line (at given startRow)
	// startRow==1: first row after header, etc.

	/////////////////////////////////////////////////////////////////
	DEBUG(Q_FUNC_INFO << ", headerEnabled = " << headerEnabled << ", header line: " << headerLine << ", start row: " << startRow)

	QString firstLine;
	if (headerEnabled && headerLine) {
		// go to header line (counting comment lines)
		for (int l = 0; l < headerLine; l++)
			firstLine = getLine(device);
		DEBUG(Q_FUNC_INFO << ", first line (header) = \"" << STDSTRING(firstLine.remove(QRegularExpression(QStringLiteral("[\\n\\r]")))) << "\"");
	} else { // read first data line (skipping comments)
		if (!commentCharacter.isEmpty()) {
			do {
				firstLine = getLine(device);
			} while (firstLine.startsWith(commentCharacter) || firstLine.simplified().isEmpty());
		} else
			firstLine = QLatin1String(device.readLine());
	}

	firstLine.remove(QRegularExpression(QStringLiteral("[\\n\\r]"))); // remove any newline
	if (removeQuotesEnabled)
		firstLine.remove(QLatin1Char('"'));
	QDEBUG(Q_FUNC_INFO << ", first line = " << firstLine);

	// determine separator and split first line
	QStringList firstLineStringList;
	if (separatingCharacter == QLatin1String("auto")) {
		DEBUG(Q_FUNC_INFO << ", using AUTOMATIC separator");
		if (firstLine.indexOf(QLatin1Char('\t')) != -1) {
			// in case we have a mix of tabs and spaces in the header, give the tab character preference
			m_separator = QLatin1Char('\t');
			firstLineStringList = split(firstLine, false);
		} else {
			firstLineStringList = split(firstLine);

			if (!firstLineStringList.isEmpty()) {
				int length1 = firstLineStringList.at(0).length();
				if (firstLineStringList.size() > 1)
					m_separator = firstLine.mid(length1, 1);
				else { // no spaces, use comma as default (CSV) and split
					m_separator = QLatin1Char(',');
					firstLineStringList = split(firstLine, false);
				}
			}
		}
	} else { // use given separator
		DEBUG(Q_FUNC_INFO << ", using GIVEN separator: " << STDSTRING(m_separator))
		// replace symbolic "TAB" with '\t'
		m_separator = separatingCharacter.replace(QLatin1String("2xTAB"), QLatin1String("\t\t"), Qt::CaseInsensitive);
		m_separator = separatingCharacter.replace(QLatin1String("TAB"), QLatin1String("\t"), Qt::CaseInsensitive);
		// replace symbolic "SPACE" with ' '
		m_separator = m_separator.replace(QLatin1String("2xSPACE"), QLatin1String("  "), Qt::CaseInsensitive);
		m_separator = m_separator.replace(QLatin1String("3xSPACE"), QLatin1String("   "), Qt::CaseInsensitive);
		m_separator = m_separator.replace(QLatin1String("4xSPACE"), QLatin1String("    "), Qt::CaseInsensitive);
		m_separator = m_separator.replace(QLatin1String("SPACE"), QLatin1String(" "), Qt::CaseInsensitive);
		firstLineStringList = split(firstLine, false);
	}
	QDEBUG(Q_FUNC_INFO << ", separator: \'" << m_separator << '\'');
	DEBUG(Q_FUNC_INFO << ", number of columns: " << firstLineStringList.size());
	QDEBUG(Q_FUNC_INFO << ", first line split: " << firstLineStringList);

	// remove whitespaces at start and end
	if (simplifyWhitespacesEnabled) {
		for (auto& s : firstLineStringList)
			s = s.simplified();
	}

	if (headerEnabled && headerLine) { // use first line to name vectors (starting from startColumn)
		columnNames = firstLineStringList.mid(startColumn - 1);
		QDEBUG(Q_FUNC_INFO << ", COLUMN NAMES: " << columnNames);
	}

	// set range to read
	if (endColumn == -1) {
		if ((headerEnabled && headerLine) || columnNames.size() == 0)
			endColumn = firstLineStringList.size(); // last column
		else // number of column names provided in the import dialog (not more than the maximum number of columns in the file)
			endColumn = std::min(columnNames.size(), firstLineStringList.size());
	}

	if (endColumn < startColumn)
		m_actualCols = 0;
	else
		m_actualCols = endColumn - startColumn + 1;

	// add time stamp and index column
	if (createTimestampEnabled) {
		columnNames.prepend(i18n("Timestamp"));
		m_actualCols++;
	}
	if (createIndexEnabled) {
		columnNames.prepend(i18n("Index"));
		m_actualCols++;
	}
	QDEBUG(Q_FUNC_INFO << ", ALL COLUMN NAMES: " << columnNames);

	/////////////////////////////////////////////////////////////////

	// navigate to the line where we asked to start reading from
	int skipRows = startRow - 1; // skip to start row
	if (headerEnabled && headerLine) // skip header too
		skipRows++;
	DEBUG(Q_FUNC_INFO << ", Skipping " << skipRows << " line(s) (including header)");
	for (int i = 0; i < skipRows; ++i) {
		DEBUG(Q_FUNC_INFO << ", skipping line: " << STDSTRING(firstLine));
		firstLine = getLine(device);
	}

	// DEBUG(Q_FUNC_INFO << ", device position after first line and comments = " << device.pos());
	// valgrind: Conditional jump or move depends on uninitialised value(s)
	firstLine.remove(QRegularExpression(QStringLiteral("[\\n\\r]"))); // remove any newline

	if (removeQuotesEnabled)
		firstLine = firstLine.remove(QLatin1Char('"'));
	DEBUG(Q_FUNC_INFO << ", Actual first line: \'" << STDSTRING(firstLine) << '\'');

	// actual start row is after header
	m_actualStartRow = startRow;
	if (headerEnabled && headerLine)
		m_actualStartRow += headerLine;
	DEBUG("actual start row = " << m_actualStartRow)

	// TEST: readline-seek-readline fails
	/*	qint64 testpos = device.pos();
		DEBUG("read data line @ pos " << testpos << " : " << STDSTRING(device.readLine()));
		device.seek(testpos);
		testpos = device.pos();
		DEBUG("read data line again @ pos " << testpos << "  : " << STDSTRING(device.readLine()));
	*/
	/////////////////////////////////////////////////////////////////

	// parse first data line to determine data type for each column
	firstLineStringList = split(firstLine, false);
	QDEBUG("firstLineStringList = " << firstLineStringList)

	columnModes.resize(m_actualCols);
	int col = 0;
	if (createIndexEnabled) {
		columnModes[0] = AbstractColumn::ColumnMode::Integer;
		++col;
	}

	if (createTimestampEnabled) {
		columnModes[col] = AbstractColumn::ColumnMode::DateTime;
		++col;
	}

	for (auto& valueString : firstLineStringList) { // parse columns available in first data line
		const int index = col - startColumn + 1;
		if (index < (int)createIndexEnabled + (int)createTimestampEnabled) {
			col++;
			continue;
		}
		if (index == m_actualCols)
			break;

		if (simplifyWhitespacesEnabled)
			valueString = valueString.simplified();
		if (removeQuotesEnabled)
			valueString.remove(QLatin1Char('"'));
		QDEBUG("value string = " << valueString)
		columnModes[index] = AbstractFileFilter::columnMode(valueString, dateTimeFormat, numberFormat);
		col++;
	}
#ifndef NDEBUG
	for (const auto mode : columnModes)
		DEBUG(Q_FUNC_INFO << ", column mode = " << ENUM_TO_STRING(AbstractColumn, ColumnMode, mode));
#endif

	// parsing more lines to better determine data types
	DEBUG(Q_FUNC_INFO << ", Parsing more lines to determine data types")
	for (unsigned int i = 0; i < m_dataTypeLines; ++i) {
		if (device.atEnd()) // EOF reached
			break;
		firstLineStringList = getLineString(device);

		col = (int)createIndexEnabled + (int)createTimestampEnabled;

		for (auto& valueString : firstLineStringList) {
			const int index{col - startColumn + 1};
			if (index < (int)createIndexEnabled + (int)createTimestampEnabled) {
				col++;
				continue;
			}
			if (index == m_actualCols)
				break;

			if (simplifyWhitespacesEnabled)
				valueString = valueString.simplified();
			if (removeQuotesEnabled)
				valueString.remove(QLatin1Char('"'));
			auto mode = AbstractFileFilter::columnMode(valueString, dateTimeFormat, numberFormat);

			// numeric: integer -> numeric
			if (mode == AbstractColumn::ColumnMode::Double && columnModes[index] == AbstractColumn::ColumnMode::Integer)
				columnModes[index] = mode;
			// text: non text -> text
			if (mode == AbstractColumn::ColumnMode::Text && columnModes[index] != AbstractColumn::ColumnMode::Text)
				columnModes[index] = mode;
			// numeric: text -> numeric/integer
			if (mode != AbstractColumn::ColumnMode::Text && columnModes[index] == AbstractColumn::ColumnMode::Text)
				columnModes[index] = mode;
			col++;
		}
	}
#ifndef NDEBUG
	for (const auto mode : columnModes)
		DEBUG(Q_FUNC_INFO << ", column mode (after checking more lines) = " << ENUM_TO_STRING(AbstractColumn, ColumnMode, mode));
#endif

	// ATTENTION: This resets the position in the device to 0
	m_actualRows = (int)q->lineNumber(device, maxLines);
	DEBUG(Q_FUNC_INFO << ", number of lines found: " << m_actualRows << ", startRow (after header): " << startRow << ", endRow: " << endRow)

	DEBUG(Q_FUNC_INFO << ", headerEnabled = " << headerEnabled << ", headerLine = " << headerLine << ", m_actualStartRow = " << m_actualStartRow)
	if ((!headerEnabled || headerLine < 1) && startRow <= 2 && m_actualStartRow > 1) // take header line
		m_actualStartRow--;

	const int actualEndRow = (endRow == -1 || endRow > m_actualRows) ? m_actualRows : endRow;
	if (actualEndRow >= m_actualStartRow)
		m_actualRows = actualEndRow - m_actualStartRow + 1;
	else
		m_actualRows = 0;

	DEBUG("start/end column: " << startColumn << ' ' << endColumn);
	DEBUG("start/end row: " << m_actualStartRow << ' ' << actualEndRow);
	DEBUG("actual cols/rows (w/o header): " << m_actualCols << ' ' << m_actualRows);

	if (m_actualRows == 0 && !device.isSequential())
		return 1;

	return 0;
}

/*!
	reads the content of the file \c fileName to the data source \c dataSource. Uses the settings defined in the data source.
*/
void AsciiFilterPrivate::readDataFromFile(const QString& fileName, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode importMode) {
	//	DEBUG(Q_FUNC_INFO << ", fileName = \'" << STDSTRING(fileName) << "\', dataSource = "
	//	      << dataSource << ", mode = " << ENUM_TO_STRING(AbstractFileFilter, ImportMode, importMode));

	// dirty hack: set readingFile and readingFileName in order to know in lineNumber(QIODevice)
	// that we're reading from a file and to benefit from much faster wc on linux
	// TODO: redesign the APIs and remove this later
	readingFile = true;
	readingFileName = fileName;
	KCompressionDevice device(fileName);
	readDataFromDevice(device, dataSource, importMode);
	readingFile = false;
}

qint64 AsciiFilterPrivate::readFromLiveDevice(QIODevice& device, AbstractDataSource* dataSource, qint64 from) {
	DEBUG(Q_FUNC_INFO << ", bytes available = " << device.bytesAvailable() << ", from = " << from);
	if (device.bytesAvailable() <= 0) {
		DEBUG("	No new data available");
		return 0;
	}

	// TODO: may be also a matrix?
	auto* spreadsheet = dynamic_cast<LiveDataSource*>(dataSource);
	if (!spreadsheet)
		return 0;

	const auto sourceType = spreadsheet->sourceType();
	if (sourceType != LiveDataSource::SourceType::FileOrPipe)
		if (device.isSequential() && device.bytesAvailable() < (int)sizeof(quint16))
			return 0;

	if (!m_prepared) {
		DEBUG(Q_FUNC_INFO << ", Preparing ..");

		switch (sourceType) {
		case LiveDataSource::SourceType::FileOrPipe: {
			const int deviceError = prepareDeviceToRead(device);
			if (deviceError != 0) {
				DEBUG(Q_FUNC_INFO << ", Device ERROR: " << deviceError);
				q->setLastError(i18n("Failed to open the device/file or it's empty."));
				return 0;
			}
			break;
		}
		case LiveDataSource::SourceType::NetworkTCPSocket:
		case LiveDataSource::SourceType::NetworkUDPSocket:
		case LiveDataSource::SourceType::LocalSocket:
		case LiveDataSource::SourceType::SerialPort:
			m_actualRows = 1;
			m_actualCols = 1;
			columnModes.clear();
			if (createIndexEnabled) {
				columnModes << AbstractColumn::ColumnMode::Integer;
				columnNames << i18n("Index");
				m_actualCols++;
			}

			if (createTimestampEnabled) {
				columnModes << AbstractColumn::ColumnMode::DateTime;
				columnNames << i18n("Timestamp");
				m_actualCols++;
			}

			// add column for the actual value
			columnModes << AbstractColumn::ColumnMode::Double;
			columnNames << i18n("Value");

			QDEBUG(Q_FUNC_INFO << ", column names = " << columnNames);
			break;
		case LiveDataSource::SourceType::MQTT:
			break;
		}

		// prepare import for spreadsheet
		spreadsheet->setUndoAware(false);
		spreadsheet->resize(AbstractFileFilter::ImportMode::Replace, columnNames, m_actualCols);

		// set the plot designation for columns Index, Timestamp and Values, if available
		// index column
		if (createIndexEnabled)
			spreadsheet->column(0)->setPlotDesignation(AbstractColumn::PlotDesignation::X);

		// timestamp column
		if (createTimestampEnabled) {
			const int index = (int)createIndexEnabled;
			spreadsheet->column(index)->setPlotDesignation(AbstractColumn::PlotDesignation::X);
		}

		// value column, available only when reading from sockets and serial port
		if (sourceType == LiveDataSource::SourceType::NetworkTCPSocket || sourceType == LiveDataSource::SourceType::NetworkUDPSocket
			|| sourceType == LiveDataSource::SourceType::LocalSocket || sourceType == LiveDataSource::SourceType::SerialPort) {
			const int index = (int)createIndexEnabled + (int)createTimestampEnabled;
			spreadsheet->column(index)->setPlotDesignation(AbstractColumn::PlotDesignation::Y);
		}

		// columns in a file data source don't have any manual changes.
		// make the available columns undo unaware and suppress the "data changed" signal.
		// data changes will be propagated via an explicit Column::setChanged() call once new data was read.
		for (auto* c : spreadsheet->children<Column>()) {
			c->setUndoAware(false);
			c->setSuppressDataChangedSignal(true);
		}

		int keepNValues = spreadsheet->keepNValues();
		if (keepNValues == 0)
			spreadsheet->setRowCount(m_actualRows > 1 ? m_actualRows : 1);
		else {
			spreadsheet->setRowCount(keepNValues);
			m_actualRows = keepNValues;
		}

		m_dataContainer.resize(m_actualCols);
		initDataContainer(spreadsheet);

		DEBUG(Q_FUNC_INFO << ", data source resized to col: " << m_actualCols);
		DEBUG(Q_FUNC_INFO << ", data source rowCount: " << spreadsheet->rowCount());
		DEBUG(Q_FUNC_INFO << ", Prepared!");
		m_prepared = true;
	}

#ifdef PERFTRACE_LIVE_IMPORT
	PERFTRACE(QStringLiteral("AsciiLiveDataImportTotal: "));
#endif
	// read until the end of the file if it's the first read or FromEnd or WholeFile.
	// TODO: this temporarliy changes readingType, redesign this part.
	auto readingType = spreadsheet->readingType();
	if (m_firstRead || spreadsheet->readingType() == LiveDataSource::ReadingType::FromEnd
		|| spreadsheet->readingType() == LiveDataSource::ReadingType::WholeFile)
		readingType = LiveDataSource::ReadingType::TillEnd;
	DEBUG("	Reading type = " << ENUM_TO_STRING(LiveDataSource, ReadingType, readingType));

	// move to the last read position, from == total bytes read
	// since the other source types are sequential we cannot seek on them
	if (sourceType == LiveDataSource::SourceType::FileOrPipe)
		device.seek(from);

	// count the new lines, increase actualrows on each
	// now we read all the new lines, if we want to use sample rate
	// then here we can do it, if we have actually sample rate number of lines :-?
	QVector<QString> newData;
	if (readingType != LiveDataSource::ReadingType::TillEnd)
		newData.resize(spreadsheet->sampleSize());

	int newDataIdx = 0;
	int newLinesForSampleSizeNotTillEnd = 0;
	int newLinesTillEnd = 0;
	{
#ifdef PERFTRACE_LIVE_IMPORT
		PERFTRACE(QStringLiteral("AsciiLiveDataImportReadingFromFile: "));
#endif
		DEBUG("	source type = " << ENUM_TO_STRING(LiveDataSource, SourceType, sourceType));
		while (!device.atEnd()) {
			if (readingType != LiveDataSource::ReadingType::TillEnd) {
				switch (sourceType) { // different sources need different read methods
				case LiveDataSource::SourceType::LocalSocket:
					newData[newDataIdx++] = QString::fromUtf8(device.readAll());
					break;
				case LiveDataSource::SourceType::NetworkUDPSocket:
					newData[newDataIdx++] = QString::fromUtf8(device.read(device.bytesAvailable()));
					break;
				case LiveDataSource::SourceType::FileOrPipe:
					newData.push_back(QString::fromUtf8(device.readLine()));
					break;
				case LiveDataSource::SourceType::NetworkTCPSocket:
				// TODO: check serial port
				case LiveDataSource::SourceType::SerialPort:
					newData[newDataIdx++] = QString::fromUtf8(device.read(device.bytesAvailable()));
					break;
				case LiveDataSource::SourceType::MQTT:
					break;
				}
			} else { // ReadingType::TillEnd
				switch (sourceType) { // different sources need different read methods
				case LiveDataSource::SourceType::LocalSocket:
					newData.push_back(QString::fromUtf8(device.readAll()));
					break;
				case LiveDataSource::SourceType::NetworkUDPSocket:
					newData.push_back(QString::fromUtf8(device.read(device.bytesAvailable())));
					break;
				case LiveDataSource::SourceType::FileOrPipe:
					newData.push_back(QString::fromUtf8(device.readLine()));
					break;
				case LiveDataSource::SourceType::NetworkTCPSocket:
				// TODO: check serial port
				case LiveDataSource::SourceType::SerialPort:
					newData.push_back(QString::fromUtf8(device.read(device.bytesAvailable())));
					break;
				case LiveDataSource::SourceType::MQTT:
					break;
				}
			}
			newLinesTillEnd++;

			if (readingType != LiveDataSource::ReadingType::TillEnd) {
				newLinesForSampleSizeNotTillEnd++;
				// for Continuous reading and FromEnd we read sample rate number of lines if possible
				// here TillEnd and Whole file behave the same
				if (newLinesForSampleSizeNotTillEnd == spreadsheet->sampleSize())
					break;
			}
		}
		QDEBUG(Q_FUNC_INFO << ", data read:" << newData);
	}

	// now we reset the readingType
	if (spreadsheet->readingType() == LiveDataSource::ReadingType::FromEnd)
		readingType = spreadsheet->readingType();

	// we have less new lines than the sample size specified
	if (readingType != LiveDataSource::ReadingType::TillEnd) {
		int nrEmptyLines = newData.removeAll(QString());
		QDEBUG("	Removed " << nrEmptyLines << " empty lines");
	}

	// back to the last read position before counting when reading from files
	if (sourceType == LiveDataSource::SourceType::FileOrPipe)
		device.seek(from);

	// split newData to get data columns (only TCP atm)
	if (m_firstRead && sourceType == LiveDataSource::SourceType::NetworkTCPSocket) {
		DEBUG("TCP: COLUMN count = " << m_actualCols)
		QString firstRowData = newData.at(0);
		QStringList dataStringList;
		QDEBUG("TCP: sep char = " << separatingCharacter)
		if (separatingCharacter == QLatin1String("auto")) {
			DEBUG(Q_FUNC_INFO << ", using AUTOMATIC separator");
			if (firstRowData.indexOf(QLatin1Char('\t')) != -1) {
				// in case we have a mix of tabs and spaces in the header, give the tab character preference
				m_separator = QLatin1Char('\t');
				dataStringList = split(firstRowData, false);
			} else {
				dataStringList = split(firstRowData);
			}
		} else {
			DEBUG(Q_FUNC_INFO << ", using GIVEN separator: " << STDSTRING(m_separator))
			// replace symbolic "TAB" with '\t'
			m_separator = separatingCharacter.replace(QLatin1String("2xTAB"), QLatin1String("\t\t"), Qt::CaseInsensitive);
			m_separator = separatingCharacter.replace(QLatin1String("TAB"), QLatin1String("\t"), Qt::CaseInsensitive);
			// replace symbolic "SPACE" with ' '
			m_separator = m_separator.replace(QLatin1String("2xSPACE"), QLatin1String("  "), Qt::CaseInsensitive);
			m_separator = m_separator.replace(QLatin1String("3xSPACE"), QLatin1String("   "), Qt::CaseInsensitive);
			m_separator = m_separator.replace(QLatin1String("4xSPACE"), QLatin1String("    "), Qt::CaseInsensitive);
			m_separator = m_separator.replace(QLatin1String("SPACE"), QLatin1String(" "), Qt::CaseInsensitive);
			dataStringList = split(firstRowData, false);
		}
		QDEBUG(Q_FUNC_INFO << ", separator: \'" << m_separator << '\'');
		DEBUG(Q_FUNC_INFO << ", number of data columns: " << dataStringList.size());
		QDEBUG(Q_FUNC_INFO << ", first data row split: " << dataStringList);
		int defaultCols = (int)createIndexEnabled + (int)createTimestampEnabled; // automatic columns
		m_actualCols += dataStringList.size() - 1; // one data column already counted
		columnModes.resize(m_actualCols);

		// column header
		columnNames.clear();
		if (createIndexEnabled)
			columnNames << i18n("Index");
		if (createTimestampEnabled)
			columnNames << i18n("Timestamp");

		for (int i = 0; i < m_actualCols - defaultCols; i++) {
			columnModes[i + defaultCols] = AbstractFileFilter::columnMode(dataStringList.at(i), dateTimeFormat, numberFormat);
			if (dataStringList.size() == 1)
				columnNames << i18n("Value");
			else
				columnNames << i18n("Value %1", i + 1);
		}
		QDEBUG("COLUMN names: " << columnNames)

		for (int i = 0; i < columnModes.size(); i++)
			QDEBUG("Data column mode " << i << " : " << ENUM_TO_STRING(AbstractColumn, ColumnMode, columnModes.at(i)))

		spreadsheet->setUndoAware(false);
		spreadsheet->resize(AbstractFileFilter::ImportMode::Replace, columnNames, m_actualCols);
	}

	// determine the number of rows to read
	const int rowCountBeforeResize = spreadsheet->rowCount();
	int currentRow = 0; // indexes the position in the vector(column)
	int linesToRead = 0;
	int keepNValues = spreadsheet->keepNValues();
	DEBUG(Q_FUNC_INFO << ", Increase row count. keepNValues = " << keepNValues);
	if (!m_firstRead) {
		// increase row count if we don't have a fixed size
		// but only after the preparation step
		if (keepNValues == 0) {
			DEBUG("	keep All values");
			if (readingType != LiveDataSource::ReadingType::TillEnd)
				m_actualRows += std::min(static_cast<qsizetype>(newData.size()), static_cast<qsizetype>(spreadsheet->sampleSize()));
			else {
				// we don't increase it if we reread the whole file, we reset it
				if (spreadsheet->readingType() != LiveDataSource::ReadingType::WholeFile)
					m_actualRows += newData.size();
				else {
					m_actualRows = newData.size();
					if (headerEnabled)
						m_actualRows -= headerLine;
				}
			}

			// appending
			if (spreadsheet->readingType() == LiveDataSource::ReadingType::WholeFile) {
				linesToRead = m_actualRows;
				if (headerEnabled)
					linesToRead += headerLine;
			} else
				linesToRead = m_actualRows - rowCountBeforeResize;
		} else { // fixed size
			DEBUG("	keep " << keepNValues << " values");
			if (readingType == LiveDataSource::ReadingType::TillEnd) {
				// we had more lines than the fixed size, so we read m_actualRows number of lines
				if (newLinesTillEnd > m_actualRows) {
					linesToRead = m_actualRows;
					// TODO after reading we should skip the next data lines because it's TillEnd actually
				} else
					linesToRead = newLinesTillEnd;
			} else {
				// we read max sample size number of lines when the reading mode
				// is ContinuouslyFixed or FromEnd, WholeFile is disabled
				linesToRead = std::min(spreadsheet->sampleSize(), newLinesTillEnd);
			}
		}

		if (linesToRead == 0)
			return 0;
	} else // first initial read, read all data
		linesToRead = newLinesTillEnd;

	DEBUG(Q_FUNC_INFO << ", lines to read = " << linesToRead);
	DEBUG(Q_FUNC_INFO << ", actual rows (w/o header) = " << m_actualRows);

	// TODO
	// 	if (sourceType == LiveDataSource::SourceType::FileOrPipe || sourceType == LiveDataSource::SourceType::NetworkUdpSocket) {
	// 		if (m_actualRows < linesToRead) {
	// 			DEBUG("	SET lines to read to " << m_actualRows);
	// 			linesToRead = m_actualRows;
	// 		}
	// 	}

	// new rows/resize columns if we don't have a fixed size
	// TODO if the user changes this value..m_resizedToFixedSize..setResizedToFixedSize
	if (keepNValues == 0) {
#ifdef PERFTRACE_LIVE_IMPORT
		PERFTRACE(QLatin1String("AsciiLiveDataImportResizing: "));
#endif
		if (spreadsheet->rowCount() < m_actualRows)
			spreadsheet->setRowCount(m_actualRows);

		if (m_firstRead)
			currentRow = 0;
		else {
			// indexes the position in the vector(column)
			if (spreadsheet->readingType() == LiveDataSource::ReadingType::WholeFile)
				currentRow = 0;
			else
				currentRow = rowCountBeforeResize;
		}

		// if we have fixed size, we do this only once in preparation, here we can use
		// m_firstRead and we need something to decide whether it has a fixed size or increasing
		initDataContainer(spreadsheet);
	} else { // fixed size
		// when we have a fixed size we have to pop sampleSize number of lines if specified
		// here popping, setting currentRow
		if (m_firstRead) {
			if (spreadsheet->readingType() == LiveDataSource::ReadingType::WholeFile)
				currentRow = 0;
			else
				currentRow = m_actualRows - std::min(newLinesTillEnd, m_actualRows);
		} else {
			if (readingType == LiveDataSource::ReadingType::TillEnd) {
				if (newLinesTillEnd > m_actualRows) {
					currentRow = 0;
				} else {
					if (spreadsheet->readingType() == LiveDataSource::ReadingType::WholeFile)
						currentRow = 0;
					else
						currentRow = m_actualRows - newLinesTillEnd;
				}
			} else {
				// we read max sample size number of lines when the reading mode
				// is ContinuouslyFixed or FromEnd
				currentRow = m_actualRows - std::min(spreadsheet->sampleSize(), newLinesTillEnd);
			}
		}

		// TODO: ???
		if (!m_firstRead) {
#ifdef PERFTRACE_LIVE_IMPORT
			PERFTRACE(QLatin1String("AsciiLiveDataImportPopping: "));
#endif
			// disable data change signal
			const auto& columns = spreadsheet->children<Column>();
			for (int col = 0; col < m_actualCols; ++col)
				columns.at(col)->setSuppressDataChangedSignal(false);

			for (int row = 0; row < linesToRead; ++row) {
				for (int col = 0; col < m_actualCols; ++col) {
					switch (columnModes.at(col)) {
					case AbstractColumn::ColumnMode::Double: {
						auto* vector = static_cast<QVector<double>*>(columns.at(col)->data());
						vector->pop_front();
						vector->resize(m_actualRows);
						m_dataContainer[col] = static_cast<void*>(vector);
						break;
					}
					case AbstractColumn::ColumnMode::Integer: {
						auto* vector = static_cast<QVector<int>*>(columns.at(col)->data());
						vector->pop_front();
						vector->resize(m_actualRows);
						m_dataContainer[col] = static_cast<void*>(vector);
						break;
					}
					case AbstractColumn::ColumnMode::BigInt: {
						auto* vector = static_cast<QVector<qint64>*>(columns.at(col)->data());
						vector->pop_front();
						vector->resize(m_actualRows);
						m_dataContainer[col] = static_cast<void*>(vector);
						break;
					}
					case AbstractColumn::ColumnMode::Text: {
						auto* vector = static_cast<QVector<QString>*>(columns.at(col)->data());
						vector->pop_front();
						vector->resize(m_actualRows);
						m_dataContainer[col] = static_cast<void*>(vector);
						break;
					}
					case AbstractColumn::ColumnMode::DateTime: {
						auto* vector = static_cast<QVector<QDateTime>*>(columns.at(col)->data());
						vector->pop_front();
						vector->resize(m_actualRows);
						m_dataContainer[col] = static_cast<void*>(vector);
						break;
					}
					// TODO
					case AbstractColumn::ColumnMode::Month:
					case AbstractColumn::ColumnMode::Day:
						break;
					}
				}
			}
		}
	}

	// from the last row we read the new data in the spreadsheet
	DEBUG(Q_FUNC_INFO << ", reading from line " << currentRow << " till end line " << newLinesTillEnd);
	DEBUG(Q_FUNC_INFO << ", lines to read:" << linesToRead << ", actual rows:" << m_actualRows << ", actual cols:" << m_actualCols);
	newDataIdx = 0;
	qint64 bytesread = 0;
	if (readingType == LiveDataSource::ReadingType::FromEnd && !m_firstRead) {
		if (newData.size() > spreadsheet->sampleSize())
			newDataIdx = newData.size() - spreadsheet->sampleSize();
		// since we skip a couple of lines, we need to count those bytes too
		for (int i = 0; i < newDataIdx; ++i)
			bytesread += newData.at(i).size();
	}
	DEBUG("	newDataIdx: " << newDataIdx);

	// read the new data into the data container
	{
#ifdef PERFTRACE_LIVE_IMPORT
		PERFTRACE(QLatin1String("AsciiLiveDataImportFillingContainers: "));
#endif
		// handle the header and determine the start row in the new data
		int row = 0;
		if (headerEnabled && sourceType == LiveDataSource::SourceType::FileOrPipe) {
			// only handle the header if we're reading the file for the first time or re-reading the whole file again
			if (m_firstRead || spreadsheet->readingType() == LiveDataSource::ReadingType::WholeFile) {
				row = m_actualStartRow - 1; // TODO: ???
				bytesread += newData.at(row - 1).size();
			}
		}

		for (; row < linesToRead; ++row) {
			// DEBUG("\n	Reading row " << row << " of " << linesToRead);
			QString line;
			if (readingType == LiveDataSource::ReadingType::FromEnd)
				line = newData.at(newDataIdx++);
			else
				line = newData.at(row);

			// when we read the whole file we don't care about the previous position
			// so we don't have to count those bytes
			if (readingType != LiveDataSource::ReadingType::WholeFile && sourceType == LiveDataSource::SourceType::FileOrPipe)
				bytesread += line.size();

			if (removeQuotesEnabled)
				line.remove(QLatin1Char('"'));

			if (line.isEmpty() || (!commentCharacter.isEmpty() && line.startsWith(commentCharacter))) // skip empty or commented lines
				continue;

			QStringList lineStringList;
			// only FileOrPipe and TCPSocket support multiple columns
			if (sourceType == LiveDataSource::SourceType::FileOrPipe || sourceType == LiveDataSource::SourceType::NetworkTCPSocket) {
				// QDEBUG("separator = " << m_separator << " , size = " << m_separator.size())
				if (m_separator.size() > 0)
					lineStringList = split(line, false);
				else
					lineStringList = split(line);
			} else
				lineStringList << line;

			// QDEBUG("	line = " << lineStringList << ", separator = \'" << m_separator << "\'");
			// DEBUG("	Line bytes: " << line.size() << " line: " << STDSTRING(line));

			if (simplifyWhitespacesEnabled) {
				for (int i = 0; i < lineStringList.size(); ++i)
					lineStringList[i] = lineStringList.at(i).simplified();
			}

			// add index if required
			static int indexColumnIdx = 1;
			int offset = 0;
			if (createIndexEnabled) {
				int index = (spreadsheet->keepNValues() == 0) ? currentRow + 1 : indexColumnIdx++;
				(*static_cast<QVector<int>*>(m_dataContainer[0]))[currentRow] = index;
				++offset;
			}

			// add current timestamp if required
			if (createTimestampEnabled) {
				// DEBUG("current row = " << currentRow << ", container size = " << static_cast<QVector<QDateTime>*>(m_dataContainer[offset])->size())
				(*static_cast<QVector<QDateTime>*>(m_dataContainer[offset]))[currentRow] = QDateTime::currentDateTime();
				++offset;
			}

			// parse columns
			QDEBUG("lineStringList = " << lineStringList)
			for (int n = offset; n < m_actualCols; ++n) {
				QString valueString;
				if (n - offset < lineStringList.size())
					valueString = lineStringList.at(n - offset);

				setValue(n, currentRow, valueString);
			}
			currentRow++;
		}
	}

	m_firstRead = false;
	DEBUG(Q_FUNC_INFO << ", DONE");
	return bytesread;
}

/*!
	reads the content of device \c device to the data source \c dataSource. Uses the settings defined in the data source.
*/
void AsciiFilterPrivate::readDataFromDevice(QIODevice& device, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode importMode, int lines) {
	DEBUG(Q_FUNC_INFO << ", dataSource = " << dataSource << ", mode = " << ENUM_TO_STRING(AbstractFileFilter, ImportMode, importMode) << ", lines = " << lines);
	DEBUG(Q_FUNC_INFO << ", start row: " << startRow)

	if (!m_prepared) {
		const int deviceError = prepareDeviceToRead(device);
		if (deviceError) {
			DEBUG(Q_FUNC_INFO << ", DEVICE ERROR = " << deviceError);
			q->setLastError(i18n("Failed to open the device/file or it's empty."));
			return;
		}

		// matrix data has only one column mode
		if (dynamic_cast<Matrix*>(dataSource)) {
			auto mode = columnModes[0];
			// TODO: remove this when Matrix supports text type
			if (mode == AbstractColumn::ColumnMode::Text)
				mode = AbstractColumn::ColumnMode::Double;
			for (auto& c : columnModes)
				if (c != mode)
					c = mode;
		}

		Q_ASSERT(dataSource);
		bool ok = false;
		m_columnOffset = dataSource->prepareImport(m_dataContainer, importMode, m_actualRows, m_actualCols, columnNames, columnModes, ok);
		if (!ok) {
			q->setLastError(i18n("Not enough memory."));
			return;
		}
		m_prepared = true;
	}

	DEBUG(Q_FUNC_INFO << ", locale = " << STDSTRING(QLocale::languageToString(numberFormat)));

	// Read the data
	int currentRow = 0; // indexes the position in the vector(column)
	if (lines == -1)
		lines = m_actualRows;
	DEBUG(Q_FUNC_INFO << ", lines = " << lines << ", m_actualRows = " << m_actualRows)

	// skip data lines, if required
	int skipLines = m_actualStartRow - 1;
	if ((!headerEnabled || headerLine < 1) && skipLines <= 1) { // read header as normal line
		skipLines--;
		m_actualRows++;
	}
	DEBUG(Q_FUNC_INFO << ", skipping " << skipLines << " line(s)");
	for (int i = 0; i < skipLines; ++i)
		device.readLine();

	lines = std::min(lines, m_actualRows);
	DEBUG(Q_FUNC_INFO << ", reading " << lines << " lines, " << m_actualCols << " columns");

	if (lines == 0 || m_actualCols == 0)
		return;

	QString line;
	// Don't put the definition QStringList lineStringList outside of the for-loop,
	// the compiler doesn't seem to optimize the destructor of QList well enough in this case.

	int progressIndex = 0;
	const qreal progressInterval = 0.01 * lines; // update on every 1% only

	// when reading numerical data the options removeQuotesEnabled, simplifyWhitespacesEnabled and skipEmptyParts
	// are not relevant and we can provide a more faster version that avoids many of string allocations, etc.
	if (!removeQuotesEnabled && !simplifyWhitespacesEnabled && !skipEmptyParts) {
		for (int i = 0; i < lines; ++i) {
			line = QString::fromUtf8(device.readLine());

			// remove any newline
			line.remove(QLatin1Char('\n'));
			line.remove(QLatin1Char('\r'));

			// DEBUG("1 Line bytes: " << line.size() << " line: " << STDSTRING(line));

			if (line.isEmpty() || (!commentCharacter.isEmpty() && line.startsWith(commentCharacter))) // skip empty or commented lines
				continue;

			int readColumns = 0;

			// index column if required
			if (createIndexEnabled) {
				(*static_cast<QVector<int>*>(m_dataContainer[0]))[currentRow] = i + 1;
				readColumns = 1;
			}

			// parse the columns in the current line
			int currentColumn = 0;
			for (const auto& valueString : qTokenize(line, m_separator, (Qt::SplitBehavior)skipEmptyParts)) {
				// skip the first columns up to the start column
				if (currentColumn < startColumn - 1) {
					++currentColumn;
					continue;
				}

				// leave the loop if all required columns were read
				if (readColumns == m_actualCols)
					break;

				// set the column value
				setValue(readColumns, currentRow, valueString);

				++readColumns;
				++currentColumn;
			}
			// set not available values
			for (int c = readColumns; c < m_actualCols; c++)
				setValue(c, currentRow, QString());

			currentRow++;

			// ask to update the progress bar only if we have more than 1000 lines
			// only in 1% steps
			progressIndex++;
			if (lines > 1000 && progressIndex > progressInterval) {
				double value = 100. * currentRow / lines;
				Q_EMIT q->completed(static_cast<int>(value));
				progressIndex = 0;
				QApplication::processEvents(QEventLoop::AllEvents, 0);
			}
		}
	} else {
		QString valueString;
		for (int i = 0; i < lines; ++i) {
			line = QString::fromUtf8(device.readLine());

			// remove any newline
			line.remove(QLatin1Char('\n'));
			line.remove(QLatin1Char('\r'));

			if (line.isEmpty() || (!commentCharacter.isEmpty() && line.startsWith(commentCharacter))) // skip empty or commented lines
				continue;

			auto lineStringList = split(line, false);
			// DEBUG("2 Line bytes: " << line.size() << " line: " << STDSTRING(line));

			if (removeQuotesEnabled) {
				for (int i = 0; i < lineStringList.size(); ++i)
					lineStringList[i] = lineStringList[i].remove(QLatin1Char('"'));
			}

			if (simplifyWhitespacesEnabled) {
				for (int i = 0; i < lineStringList.size(); ++i)
					lineStringList[i] = lineStringList[i].simplified();
			}

			// remove left white spaces
			if (skipEmptyParts) {
				for (int n = 0; n < lineStringList.size(); ++n) {
					valueString = lineStringList.at(n);
					if (!QString::compare(valueString, QLatin1String(" "))) {
						lineStringList.removeAt(n);
						n--;
					}
				}
			}

			// parse columns
			// 		DEBUG(Q_FUNC_INFO << ", actual cols = " << m_actualCols)
			for (int n = 0; n < m_actualCols; ++n) {
				// index column if required
				if (n == 0 && createIndexEnabled) {
					(*static_cast<QVector<int>*>(m_dataContainer[0]))[currentRow] = i + 1;
					continue;
				}

				// column counting starts with 1, subtract 1 as well as another 1 for the index column if required
				int col = createIndexEnabled ? n + startColumn - 2 : n + startColumn - 1;
				if (col < lineStringList.size())
					setValue(n, currentRow, lineStringList.at(col));
				else
					setValue(n, currentRow, QStringView());
			}

			currentRow++;

			// ask to update the progress bar only if we have more than 1000 lines
			// only in 1% steps
			progressIndex++;
			if (lines > 1000 && progressIndex > progressInterval) {
				double value = 100. * currentRow / lines;
				Q_EMIT q->completed(static_cast<int>(value));
				progressIndex = 0;
				QApplication::processEvents(QEventLoop::AllEvents, 0);
			}
		}
	}

	DEBUG(Q_FUNC_INFO << ", Read " << currentRow << " lines");

	// we might have skipped empty lines above. shrink the spreadsheet if the number of read lines (=currentRow)
	// is smaller than the initial size of the spreadsheet (=m_actualRows).
	// TODO: should also be relevant for Matrix
	auto* s = dynamic_cast<Spreadsheet*>(dataSource);
	if (s && currentRow != m_actualRows && importMode == AbstractFileFilter::ImportMode::Replace)
		s->setRowCount(currentRow);

	Q_ASSERT(dataSource);
	dataSource->finalizeImport(m_columnOffset, startColumn, startColumn + m_actualCols - 1, dateTimeFormat, importMode);
}

// #####################################################################
// ############################ Preview ################################
// #####################################################################
/*!
 * preview for special devices (local/UDP/TCP socket or serial port)
 */
QVector<QStringList> AsciiFilterPrivate::preview(QIODevice& device) {
	DEBUG(Q_FUNC_INFO << ", bytesAvailable = " << device.bytesAvailable() << ", isSequential = " << device.isSequential());
	QVector<QStringList> dataStrings;

	if (!(device.bytesAvailable() > 0)) {
		DEBUG("No new data available");
		return dataStrings;
	}

	if (device.isSequential() && device.bytesAvailable() < (int)sizeof(quint16))
		return dataStrings;

	int linesToRead = 0;
	QVector<QString> newData;

	// TODO: serial port "read(nBytes)"?
	while (!device.atEnd()) {
		if (device.canReadLine())
			newData.push_back(QString::fromUtf8(device.readLine()));
		else // UDP fails otherwise
			newData.push_back(QString::fromUtf8(device.readAll()));
		linesToRead++;
	}
	QDEBUG("	data = " << newData);

	if (linesToRead == 0)
		return dataStrings;

	columnNames.clear();
	columnModes.clear();

	if (createIndexEnabled) {
		columnModes << AbstractColumn::ColumnMode::Integer;
		columnNames << i18n("Index");
	}

	if (createTimestampEnabled) {
		columnModes << AbstractColumn::ColumnMode::DateTime;
		columnNames << i18n("Timestamp");
	}

	// parse the first data line to determine data type for each column
	QStringList firstLineStringList = newData.at(0).split(separatingCharacter, Qt::SkipEmptyParts);
	int i = 1;
	for (auto& valueString : firstLineStringList) {
		if (simplifyWhitespacesEnabled)
			valueString = valueString.simplified();
		if (removeQuotesEnabled)
			valueString.remove(QLatin1Char('"'));
		if (skipEmptyParts && !QString::compare(valueString, QLatin1String(" "))) // handle left white spaces
			continue;

		if (firstLineStringList.size() == 1)
			columnNames << i18n("Value");
		else
			columnNames << i18n("Value %1", i++);
		columnModes << AbstractFileFilter::columnMode(valueString, dateTimeFormat, numberFormat);
	}

	int offset = int(createIndexEnabled) + int(createTimestampEnabled);
	QString line;

	// loop over all lines in the new data in the device and parse the available columns
	for (int i = 0; i < linesToRead; ++i) {
		line = newData.at(i);

		// remove any newline
		line = line.remove(QLatin1Char('\n'));
		line = line.remove(QLatin1Char('\r'));

		if (simplifyWhitespacesEnabled)
			line = line.simplified();

		if (line.isEmpty() || (!commentCharacter.isEmpty() && line.startsWith(commentCharacter))) // skip empty or commented lines
			continue;

		QStringList lineString;

		// index column if required
		if (createIndexEnabled)
			lineString += QString::number(i + 1);

		// timestamp column if required
		if (createTimestampEnabled)
			lineString += QDateTime::currentDateTime().toString();

		// TODO: use separator
		QStringList lineStringList = line.split(separatingCharacter, Qt::SkipEmptyParts);
		QDEBUG(" line = " << lineStringList);

		// parse columns
		DEBUG(Q_FUNC_INFO << ", number of columns = " << lineStringList.size())
		for (int n = 0; n < lineStringList.size(); ++n) {
			if (n < lineStringList.size()) {
				QString valueString = lineStringList.at(n);
				if (removeQuotesEnabled)
					valueString.remove(QLatin1Char('"'));

				if (skipEmptyParts && !QString::compare(valueString, QLatin1String(" "))) // handle left white spaces
					continue;

				lineString += previewValue(valueString, columnModes[n + offset]);
			} else // missing columns in this line
				lineString += QString();
		}

		dataStrings << lineString;
	}

	return dataStrings;
}

/*!
 * generates the preview for the file \c fileName reading the provided number of \c lines.
 */
QVector<QStringList> AsciiFilterPrivate::preview(const QString& fileName, int lines) {
	DEBUG(Q_FUNC_INFO)

	KCompressionDevice device(fileName);
	const int deviceError = prepareDeviceToRead(device, lines);

	if (deviceError != 0) {
		DEBUG("Device error = " << deviceError);
		q->setLastError(i18n("Failed to open the device/file or it's empty."));
		return {};
	}

	// number formatting
	DEBUG(Q_FUNC_INFO << ", locale = " << STDSTRING(QLocale::languageToString(numberFormat)));

	// Read the data
	if (lines == -1)
		lines = m_actualRows;

	// set column names for preview
	if (!headerEnabled || headerLine < 1) {
		int start = 0;
		if (createIndexEnabled)
			start = 1;
		for (int i = start; i < m_actualCols; i++)
			columnNames << QStringLiteral("Column ") + QString::number(i + 1);
	}
	QDEBUG(Q_FUNC_INFO << ", column names = " << columnNames);

	// skip data lines, if required
	DEBUG("m_actualStartRow = " << m_actualStartRow)
	DEBUG("m_actualRows = " << m_actualRows)
	int skipLines = m_actualStartRow - 1;
	if (!headerEnabled || headerLine < 1) { // read header as normal line
		skipLines--;
		m_actualRows++;
	}
	DEBUG(Q_FUNC_INFO << ", skipping " << skipLines << " line(s)");
	for (int i = 0; i < skipLines; ++i)
		device.readLine();

	lines = std::min(lines, m_actualRows);
	DEBUG(Q_FUNC_INFO << ", preview " << lines << " line(s)");
	QString line;

	// loop over the preview lines in the file and parse the available columns
	QVector<QStringList> dataStrings;
	for (int i = 0; i < lines; ++i) {
		line = QString::fromUtf8(device.readLine());

		// remove any newline
		line = line.remove(QLatin1Char('\n'));
		line = line.remove(QLatin1Char('\r'));

		if (line.isEmpty() || (!commentCharacter.isEmpty() && line.startsWith(commentCharacter))) // skip empty or commented lines
			continue;

		auto lineStringList = split(line, false);
		// QDEBUG(" line = " << lineStringList);
		// DEBUG("	Line bytes: " << line.size() << " line: " << STDSTRING(line));

		if (simplifyWhitespacesEnabled) {
			for (int i = 0; i < lineStringList.size(); ++i)
				lineStringList[i] = lineStringList[i].simplified();
		}

		QStringList lineString;

		// parse columns
		for (int n = 0; n < m_actualCols; ++n) {
			// index column if required
			if (n == 0 && createIndexEnabled) {
				lineString += QString::number(i + 1);
				continue;
			}

			// column counting starts with 1, subtract 1 as well as another 1 for the index column if required
			int col = createIndexEnabled ? n + startColumn - 2 : n + startColumn - 1;

			if (col < lineStringList.size()) {
				QString valueString = lineStringList.at(col);
				if (removeQuotesEnabled)
					valueString.remove(QLatin1Char('"'));

				if (skipEmptyParts && !QString::compare(valueString, QLatin1String(" "))) // handle left white spaces
					continue;

				lineString += previewValue(valueString, columnModes[n]);
			} else // missing columns in this line
				lineString += QString();
		}

		dataStrings << lineString;
	}

	return dataStrings;
}

// #####################################################################
// ####################### Helper functions ############################
// #####################################################################
/*!
 * converts \c valueString to the date type according to \c mode and \c locale
 * and returns its string representation.
 */
QString AsciiFilterPrivate::previewValue(const QString& valueString, AbstractColumn::ColumnMode mode) {
	//	DEBUG(Q_FUNC_INFO << ", valueString = " << STDSTRING(valueString) << ", mode = " << (int)mode)

	QString result;
	switch (mode) {
	case AbstractColumn::ColumnMode::Double: {
		bool isNumber;
		const double value = locale.toDouble(valueString, &isNumber);
		result = QString::number(isNumber ? value : nanValue, 'g', 15);
		break;
	}
	case AbstractColumn::ColumnMode::Integer: {
		bool isNumber;
		const int value = locale.toInt(valueString, &isNumber);
		result = QString::number(isNumber ? value : 0);
		break;
	}
	case AbstractColumn::ColumnMode::BigInt: {
		bool isNumber;
		const qint64 value = locale.toLongLong(valueString, &isNumber);
		result = QString::number(isNumber ? value : 0);
		break;
	}
	case AbstractColumn::ColumnMode::DateTime: {
		QDateTime valueDateTime = parseDateTime(valueString, dateTimeFormat);
		result = valueDateTime.isValid() ? valueDateTime.toString(dateTimeFormat) : QLatin1String(" ");
		break;
	}
	case AbstractColumn::ColumnMode::Text:
		result = valueString;
		break;
	case AbstractColumn::ColumnMode::Month: // never happens
	case AbstractColumn::ColumnMode::Day:
		break;
	}
	return result;
}

// set value depending on data type
void AsciiFilterPrivate::setValue(int col, int row, QStringView valueString) {
	// 	QDEBUG(Q_FUNC_INFO << ", string = " << valueString)
	if (!valueString.isEmpty()) {
		switch (columnModes.at(col)) {
		case AbstractColumn::ColumnMode::Double: {
			bool isNumber;
			const double value = locale.toDouble(valueString, &isNumber);
			(*static_cast<QVector<double>*>(m_dataContainer[col]))[row] = (isNumber ? value : nanValue);
			break;
		}
		case AbstractColumn::ColumnMode::Integer: {
			bool isNumber;
			const int value = locale.toInt(valueString, &isNumber);
			(*static_cast<QVector<int>*>(m_dataContainer[col]))[row] = (isNumber ? value : 0);
			break;
		}
		case AbstractColumn::ColumnMode::BigInt: {
			bool isNumber;
			const qint64 value = locale.toLongLong(valueString, &isNumber);
			(*static_cast<QVector<qint64>*>(m_dataContainer[col]))[row] = (isNumber ? value : 0);
			break;
		}
		case AbstractColumn::ColumnMode::DateTime: {
			const auto valueDateTime = parseDateTime(valueString.toString(), dateTimeFormat);
			(*static_cast<QVector<QDateTime>*>(m_dataContainer[col]))[row] = valueDateTime.isValid() ? std::move(valueDateTime) : QDateTime();
			break;
		}
		case AbstractColumn::ColumnMode::Text: {
			auto* colData = static_cast<QVector<QString>*>(m_dataContainer[col]);
			(*colData)[row] = valueString.toString();
			break;
		}
		case AbstractColumn::ColumnMode::Month: // never happens
		case AbstractColumn::ColumnMode::Day:
			break;
		}
	} else { // missing columns in this line
		switch (columnModes.at(col)) {
		case AbstractColumn::ColumnMode::Double:
			(*static_cast<QVector<double>*>(m_dataContainer[col]))[row] = nanValue;
			break;
		case AbstractColumn::ColumnMode::Integer:
			(*static_cast<QVector<int>*>(m_dataContainer[col]))[row] = 0;
			break;
		case AbstractColumn::ColumnMode::BigInt:
			(*static_cast<QVector<qint64>*>(m_dataContainer[col]))[row] = 0;
			break;
		case AbstractColumn::ColumnMode::DateTime:
			(*static_cast<QVector<QDateTime>*>(m_dataContainer[col]))[row] = QDateTime();
			break;
		case AbstractColumn::ColumnMode::Text:
			(*static_cast<QVector<QString>*>(m_dataContainer[col]))[row].clear();
			break;
		case AbstractColumn::ColumnMode::Month: // never happens
		case AbstractColumn::ColumnMode::Day:
			break;
		}
	}
}

void AsciiFilterPrivate::initDataContainer(Spreadsheet* spreadsheet) {
	DEBUG(Q_FUNC_INFO);
	const auto& columns = spreadsheet->children<Column>();
	for (int n = 0; n < m_actualCols; ++n) {
		// data() returns a void* which is a pointer to any data type (see ColumnPrivate.cpp)
		columns.at(n)->setColumnMode(columnModes.at(n));
		switch (columnModes.at(n)) {
		case AbstractColumn::ColumnMode::Double: {
			auto* vector = static_cast<QVector<double>*>(columns.at(n)->data());
			vector->reserve(m_actualRows);
			vector->resize(m_actualRows);
			m_dataContainer[n] = static_cast<void*>(vector);
			break;
		}
		case AbstractColumn::ColumnMode::Integer: {
			auto* vector = static_cast<QVector<int>*>(columns.at(n)->data());
			vector->resize(m_actualRows);
			m_dataContainer[n] = static_cast<void*>(vector);
			break;
		}
		case AbstractColumn::ColumnMode::BigInt: {
			auto* vector = static_cast<QVector<qint64>*>(columns.at(n)->data());
			vector->resize(m_actualRows);
			m_dataContainer[n] = static_cast<void*>(vector);
			break;
		}
		case AbstractColumn::ColumnMode::Text: {
			auto* vector = static_cast<QVector<QString>*>(columns.at(n)->data());
			vector->resize(m_actualRows);
			m_dataContainer[n] = static_cast<void*>(vector);
			break;
		}
		case AbstractColumn::ColumnMode::DateTime: {
			auto* vector = static_cast<QVector<QDateTime>*>(columns.at(n)->data());
			vector->resize(m_actualRows);
			m_dataContainer[n] = static_cast<void*>(vector);
			break;
		}
		// TODO
		case AbstractColumn::ColumnMode::Month:
		case AbstractColumn::ColumnMode::Day:
			break;
		}
	}
}

/*!
 * get a single line from device
 */
QString AsciiFilterPrivate::getLine(QIODevice& device) {
	if (!device.canReadLine())
		DEBUG(Q_FUNC_INFO << ", WARNING: device cannot 'readLine()' but using it anyway.");

	if (device.atEnd()) {
		DEBUG(Q_FUNC_INFO << ", device at end! Giving up.");
		return {};
	}

	QString line = QString::fromUtf8(device.readLine());
	// DEBUG(Q_FUNC_INFO << ", line = " << STDSTRING(line));

	return line;
}

/*!
 * get a single line from device
 */
QStringList AsciiFilterPrivate::getLineString(QIODevice& device) {
	QString line;
	do { // skip comment lines in data lines
		if (!device.canReadLine())
			DEBUG("WARNING in AsciiFilterPrivate::getLineString(): device cannot 'readLine()' but using it anyway.");
		//			line = device.readAll();
		line = QString::fromUtf8(device.readLine());
		QDEBUG("LINE:" << line)
	} while (!commentCharacter.isEmpty() && line.startsWith(commentCharacter));

	line.remove(QRegularExpression(QStringLiteral("[\\n\\r]"))); // remove any newline
	DEBUG("data line : \'" << STDSTRING(line) << '\'');
	auto lineStringList = split(line, false);

	// TODO: remove quotes here?
	if (simplifyWhitespacesEnabled) {
		for (int i = 0; i < lineStringList.size(); ++i)
			lineStringList[i] = lineStringList[i].simplified();
	}

	QDEBUG("data line, parsed: " << lineStringList);

	return lineStringList;
}

QStringList AsciiFilterPrivate::split(const QString& line, bool autoSeparator) {
	QStringList lineStringList;
	if (autoSeparator) {
		static const QRegularExpression regExp(QStringLiteral("[,;:]?\\s+"));
		lineStringList = line.split(regExp, (Qt::SplitBehavior)skipEmptyParts);
		// TODO: determine the separator here and perform the merge of columns as in the else-case, if needed
	} else {
		lineStringList = line.split(m_separator, (Qt::SplitBehavior)skipEmptyParts);

		// merge the columns if they were splitted because of the separators inside the quotes
		for (int i = 0; i < lineStringList.size(); ++i) {
			if (lineStringList.at(i).simplified().startsWith(QLatin1Char('"')) && !lineStringList.at(i).simplified().endsWith(QLatin1Char('"'))) {
				int mergeStart = i;
				int mergeEnd = i;
				for (; mergeEnd < lineStringList.size(); ++mergeEnd) {
					if (lineStringList.at(mergeEnd).simplified().endsWith(QLatin1Char('"')))
						break;
				}

				if (mergeStart != mergeEnd) {
					for (int i = 0; i < (mergeEnd - mergeStart); ++i) {
						if (mergeStart + 1 < lineStringList.count())
							lineStringList[mergeStart] += m_separator + lineStringList.takeAt(mergeStart + 1);
					}
				}
			}
		}
	}

	return lineStringList;
}

/*!
	writes the content of \c dataSource to the file \c fileName.
*/
void AsciiFilterPrivate::write(const QString& /*fileName*/, AbstractDataSource* /*dataSource*/) {
	// TODO: save data to ascii file
}

/*!
 * create datetime from \c string using \c format considering corner cases
 */
QDateTime AsciiFilterPrivate::parseDateTime(const QString& string, const QString& format) {
	// DEBUG(Q_FUNC_INFO << ", string = " << STDSTRING(string) << ", format = " << STDSTRING(format))
	QString fixedString(string);
	QString fixedFormat(format);
	if (!format.contains(QLatin1String("yy"))) { // no year given: set temporary to 2000 (must be a leap year to parse "Feb 29")
		fixedString.append(QLatin1String(" 2000"));
		fixedFormat.append(QLatin1String(" yyyy"));
	}

	QDateTime dateTime = QDateTime::fromString(fixedString, fixedFormat);
	dateTime.setTimeSpec(Qt::UTC);
	// QDEBUG("fromString() =" << dateTime)
	//  interpret 2-digit year smaller than 50 as 20XX
	if (dateTime.date().year() < 1950 && !format.contains(QLatin1String("yyyy")))
		dateTime = dateTime.addYears(100);
	// QDEBUG("dateTime fixed =" << dateTime)
	// DEBUG(Q_FUNC_INFO << ", dateTime.toString = " << STDSTRING(dateTime.toString(format)))

	return dateTime;
}

// ##############################################################################
// ##################  Serialization/Deserialization  ###########################
// ##############################################################################
/*!
  Saves as XML.
 */
void AsciiFilter::save(QXmlStreamWriter* writer) const {
	writer->writeStartElement(QStringLiteral("asciiFilter"));
	writer->writeAttribute(QStringLiteral("commentCharacter"), d->commentCharacter);
	writer->writeAttribute(QStringLiteral("separatingCharacter"), d->separatingCharacter);
	writer->writeAttribute(QStringLiteral("autoMode"), QString::number(d->autoModeEnabled));
	writer->writeAttribute(QStringLiteral("createIndex"), QString::number(d->createIndexEnabled));
	writer->writeAttribute(QStringLiteral("createTimestamp"), QString::number(d->createTimestampEnabled));
	writer->writeAttribute(QStringLiteral("header"), QString::number(d->headerEnabled));
	writer->writeAttribute(QStringLiteral("vectorNames"), d->columnNames.join(QLatin1Char(' ')));
	writer->writeAttribute(QStringLiteral("skipEmptyParts"), QString::number(d->skipEmptyParts));
	writer->writeAttribute(QStringLiteral("simplifyWhitespaces"), QString::number(d->simplifyWhitespacesEnabled));
	writer->writeAttribute(QStringLiteral("nanValue"), QString::number(d->nanValue));
	writer->writeAttribute(QStringLiteral("removeQuotes"), QString::number(d->removeQuotesEnabled));
	writer->writeAttribute(QStringLiteral("startRow"), QString::number(d->startRow));
	writer->writeAttribute(QStringLiteral("endRow"), QString::number(d->endRow));
	writer->writeAttribute(QStringLiteral("startColumn"), QString::number(d->startColumn));
	writer->writeAttribute(QStringLiteral("endColumn"), QString::number(d->endColumn));
	writer->writeEndElement();
}

/*!
  Loads from XML.
*/
bool AsciiFilter::load(XmlStreamReader* reader) {
	const auto& attribs = reader->attributes();
	QString str;

	READ_STRING_VALUE("commentCharacter", commentCharacter);
	READ_STRING_VALUE("separatingCharacter", separatingCharacter);

	READ_INT_VALUE("createIndex", createIndexEnabled, bool);
	READ_INT_VALUE("createTimestamp", createTimestampEnabled, bool);
	READ_INT_VALUE("autoMode", autoModeEnabled, bool);
	READ_INT_VALUE("header", headerEnabled, bool);

	str = attribs.value(QStringLiteral("vectorNames")).toString();
	d->columnNames = str.split(QLatin1Char(' ')); // may be empty

	READ_INT_VALUE("simplifyWhitespaces", simplifyWhitespacesEnabled, bool);
	READ_DOUBLE_VALUE("nanValue", nanValue);
	READ_INT_VALUE("removeQuotes", removeQuotesEnabled, bool);
	READ_INT_VALUE("skipEmptyParts", skipEmptyParts, bool);
	READ_INT_VALUE("startRow", startRow, int);
	READ_INT_VALUE("endRow", endRow, int);
	READ_INT_VALUE("startColumn", startColumn, int);
	READ_INT_VALUE("endColumn", endColumn, int);
	return true;
}

// ##############################################################################
// ########################## MQTT releated code  ###############################
// ##############################################################################
#ifdef HAVE_MQTT
int AsciiFilterPrivate::prepareToRead(const QString& message) {
	QStringList lines = message.split(QLatin1Char('\n'));
	if (lines.isEmpty())
		return 1;

	// Parse the first line:
	// Determine the number of columns, create the columns and use (if selected) the first row to name them
	QString firstLine = lines.at(0);
	if (simplifyWhitespacesEnabled)
		firstLine = firstLine.simplified();
	DEBUG("First line: \'" << STDSTRING(firstLine) << '\'');

	// determine separator and split first line
	QStringList firstLineStringList;
	if (separatingCharacter == QLatin1String("auto")) {
		DEBUG("automatic separator");
		firstLineStringList = split(firstLine);
	} else { // use given separator
		// replace symbolic "TAB" with '\t'
		m_separator = separatingCharacter.replace(QLatin1String("2xTAB"), QLatin1String("\t\t"), Qt::CaseInsensitive);
		m_separator = separatingCharacter.replace(QLatin1String("TAB"), QLatin1String("\t"), Qt::CaseInsensitive);
		// replace symbolic "SPACE" with ' '
		m_separator = m_separator.replace(QLatin1String("2xSPACE"), QLatin1String("  "), Qt::CaseInsensitive);
		m_separator = m_separator.replace(QLatin1String("3xSPACE"), QLatin1String("   "), Qt::CaseInsensitive);
		m_separator = m_separator.replace(QLatin1String("4xSPACE"), QLatin1String("    "), Qt::CaseInsensitive);
		m_separator = m_separator.replace(QLatin1String("SPACE"), QLatin1String(" "), Qt::CaseInsensitive);
		firstLineStringList = split(firstLine, false);
	}
	DEBUG("separator: \'" << STDSTRING(m_separator) << '\'');
	DEBUG("number of columns: " << firstLineStringList.size());
	QDEBUG("first line: " << firstLineStringList);

	// all columns are read plus the optional column for the index and for the timestamp
	m_actualCols = firstLineStringList.size() + int(createIndexEnabled) + int(createTimestampEnabled);

	// column names:
	// when reading the message strings for different topics, it's not possible to specify vector names
	// since the different topics can have different content and different number of columns/vectors
	//->we always set the vector names here to fixed values
	columnNames.clear();
	columnModes.clear();

	// add index column
	if (createIndexEnabled) {
		columnNames << i18n("Index");
		columnModes << AbstractColumn::ColumnMode::Integer;
	}

	// add timestamp column
	if (createTimestampEnabled) {
		columnNames << i18n("Timestamp");
		columnModes << AbstractColumn::ColumnMode::DateTime;
	}

	// parse the first data line to determine data type for each column
	int i = 1;
	for (auto& valueString : firstLineStringList) {
		if (simplifyWhitespacesEnabled)
			valueString = valueString.simplified();
		if (removeQuotesEnabled)
			valueString.remove(QLatin1Char('"'));

		if (firstLineStringList.size() == 1)
			columnNames << i18n("Value");
		else
			columnNames << i18n("Value %1", i++);
		columnModes << AbstractFileFilter::columnMode(valueString, dateTimeFormat, numberFormat);
	}

	m_actualStartRow = startRow;
	m_actualRows = lines.size();

	// 	QDEBUG("column modes = " << columnModes);
	DEBUG("actual cols/rows (w/o header): " << m_actualCols << ' ' << m_actualRows);

	return 0;
}

/*!
 * generates the preview for the string \s message.
 */
QVector<QStringList> AsciiFilterPrivate::preview(const QString& message) {
	QVector<QStringList> dataStrings;
	prepareToRead(message);

	// number formatting
	DEBUG("locale = " << STDSTRING(QLocale::languageToString(numberFormat)));

	// Read the data
	QStringList lines = message.split(QLatin1Char('\n'));

	// loop over all lines in the message and parse the available columns
	int i = 0;
	for (auto line : lines) {
		if (simplifyWhitespacesEnabled)
			line = line.simplified();

		if (line.isEmpty() || (!commentCharacter.isEmpty() && line.startsWith(commentCharacter))) // skip empty or commented lines
			continue;

		auto lineStringList = split(line, false);
		QDEBUG(" line = " << lineStringList);

		QStringList lineString;

		// index column if required
		if (createIndexEnabled)
			lineString += QString::number(i + 1);

		// timestamp column if required
		if (createTimestampEnabled)
			lineString += QDateTime::currentDateTime().toString();

		int offset = int(createIndexEnabled) + int(createTimestampEnabled);

		// parse columns
		for (int n = 0; n < m_actualCols - offset; ++n) {
			if (n < lineStringList.size()) {
				QString valueString = lineStringList.at(n);
				if (removeQuotesEnabled)
					valueString.remove(QLatin1Char('"'));
				if (skipEmptyParts && !QString::compare(valueString, QLatin1String(" "))) // handle left white spaces
					continue;

				lineString += previewValue(valueString, columnModes[n + offset]);
			} else // missing columns in this line
				lineString += QString();
		}

		++i;
		dataStrings << lineString;
	}

	return dataStrings;
}

/*!
 * \brief reads the content of a message received by the topic.
 * Uses the settings defined in the MQTTTopic's MQTTClient
 * \param message
 * \param topic
 * \param dataSource
 */
void AsciiFilterPrivate::readMQTTTopic(const QString& message, AbstractDataSource* dataSource) {
	// If the message is empty, there is nothing to do
	if (message.isEmpty()) {
		DEBUG("No new data available");
		return;
	}

	auto* spreadsheet = dynamic_cast<MQTTTopic*>(dataSource);
	if (!spreadsheet)
		return;

	const int keepNValues = spreadsheet->mqttClient()->keepNValues();

	if (!m_prepared) {
		DEBUG("Start preparing filter for: " << STDSTRING(spreadsheet->topicName()));

		// Prepare the filter
		const int mqttPrepareError = prepareToRead(message);
		if (mqttPrepareError != 0) {
			DEBUG("Mqtt Prepare Error = " << mqttPrepareError);
			return;
		}

		// prepare import for spreadsheet
		spreadsheet->setUndoAware(false);
		spreadsheet->resize(AbstractFileFilter::ImportMode::Replace, columnNames, m_actualCols);

		// columns in a MQTTTopic don't have any manual changes.
		// make the available columns undo unaware and suppress the "data changed" signal.
		// data changes will be propagated via an explicit Column::setChanged() call once new data was read.
		const auto& columns = spreadsheet->children<Column>();
		for (auto* column : columns) {
			column->setUndoAware(false);
			column->setSuppressDataChangedSignal(true);
		}

		if (keepNValues == 0)
			spreadsheet->setRowCount(m_actualRows > 1 ? m_actualRows : 1);
		else {
			spreadsheet->setRowCount(keepNValues);
			m_actualRows = keepNValues;
		}

		m_dataContainer.resize(m_actualCols);
		initDataContainer(spreadsheet);
	}

	MQTTClient::ReadingType readingType;
	if (!m_prepared) {
		// if filter is not prepared we read till the end
		readingType = MQTTClient::ReadingType::TillEnd;
	} else {
		// we have to read all the data when reading from end
		// so we set readingType to TillEnd
		if (spreadsheet->mqttClient()->readingType() == MQTTClient::ReadingType::FromEnd)
			readingType = MQTTClient::ReadingType::TillEnd;
		else
			readingType = spreadsheet->mqttClient()->readingType();
	}

	// count the new lines, increase actualrows on each
	// now we read all the new lines, if we want to use sample rate
	// then here we can do it, if we have actually sample rate number of lines :-?
	int newLinesForSampleSizeNotTillEnd = 0;
	int newLinesTillEnd = 0;
	QVector<QString> newData;
	if (readingType != MQTTClient::ReadingType::TillEnd) {
		newData.reserve(spreadsheet->mqttClient()->sampleSize());
		newData.resize(spreadsheet->mqttClient()->sampleSize());
	}

	// TODO: bool sampleSizeReached = false;
	const QStringList newDataList = message.split(QRegularExpression(QStringLiteral("\n|\r\n|\r")), Qt::SkipEmptyParts);
	for (auto& line : newDataList) {
		newData.push_back(line);
		newLinesTillEnd++;

		if (readingType != MQTTClient::ReadingType::TillEnd) {
			newLinesForSampleSizeNotTillEnd++;
			// for Continuous reading and FromEnd we read sample rate number of lines if possible
			if (newLinesForSampleSizeNotTillEnd == spreadsheet->mqttClient()->sampleSize()) {
				// TODO: sampleSizeReached = true;
				break;
			}
		}
	}

	qDebug() << "Processing message done";
	// now we reset the readingType
	if (spreadsheet->mqttClient()->readingType() == MQTTClient::ReadingType::FromEnd)
		readingType = static_cast<MQTTClient::ReadingType>(spreadsheet->mqttClient()->readingType());

	// we had less new lines than the sample rate specified
	if (readingType != MQTTClient::ReadingType::TillEnd)
		qDebug() << "Removed empty lines: " << newData.removeAll(QString());

	const int rowCountBeforeResize = spreadsheet->rowCount();

	if (m_prepared) {
		if (keepNValues == 0)
			m_actualRows = rowCountBeforeResize;
		else {
			// if the keepNValues changed since the last read we have to manage the columns accordingly
			if (m_actualRows != keepNValues) {
				if (m_actualRows < keepNValues) {
					spreadsheet->setRowCount(keepNValues);
					qDebug() << "rowcount set to: " << keepNValues;
				}

				// Calculate the difference between the old and new keepNValues
				int rowDiff = 0;
				if (m_actualRows > keepNValues)
					rowDiff = m_actualRows - keepNValues;

				if (m_actualRows < keepNValues)
					rowDiff = keepNValues - m_actualRows;

				const auto& columns = spreadsheet->children<Column>();

				for (int n = 0; n < columnModes.size(); ++n) {
					// data() returns a void* which is a pointer to any data type (see ColumnPrivate.cpp)
					switch (columnModes.at(n)) {
					case AbstractColumn::ColumnMode::Double: {
						auto* vector = static_cast<QVector<double>*>(columns.at(n)->data());
						m_dataContainer[n] = static_cast<void*>(vector);

						// if the keepNValues got smaller then we move the last keepNValues count of data
						// in the first keepNValues places
						if (m_actualRows > keepNValues) {
							for (int i = 0; i < keepNValues; i++) {
								static_cast<QVector<double>*>(m_dataContainer[n])->operator[](i) =
									static_cast<QVector<double>*>(m_dataContainer[n])->operator[](m_actualRows - keepNValues + i);
							}
						}

						// if the keepNValues got bigger we move the existing values to the last m_actualRows positions
						// then fill the remaining lines with NaN
						if (m_actualRows < keepNValues) {
							vector->reserve(keepNValues);
							vector->resize(keepNValues);

							for (int i = 1; i <= m_actualRows; i++) {
								static_cast<QVector<double>*>(m_dataContainer[n])->operator[](keepNValues - i) =
									static_cast<QVector<double>*>(m_dataContainer[n])->operator[](keepNValues - i - rowDiff);
							}
							for (int i = 0; i < rowDiff; i++)
								static_cast<QVector<double>*>(m_dataContainer[n])->operator[](i) = nanValue;
						}
						break;
					}
					case AbstractColumn::ColumnMode::Integer: {
						auto* vector = static_cast<QVector<int>*>(columns.at(n)->data());
						m_dataContainer[n] = static_cast<void*>(vector);

						// if the keepNValues got smaller then we move the last keepNValues count of data
						// in the first keepNValues places
						if (m_actualRows > keepNValues) {
							for (int i = 0; i < keepNValues; i++) {
								static_cast<QVector<int>*>(m_dataContainer[n])->operator[](i) =
									static_cast<QVector<int>*>(m_dataContainer[n])->operator[](m_actualRows - keepNValues + i);
							}
						}

						// if the keepNValues got bigger we move the existing values to the last m_actualRows positions
						// then fill the remaining lines with 0
						if (m_actualRows < keepNValues) {
							vector->reserve(keepNValues);
							vector->resize(keepNValues);
							for (int i = 1; i <= m_actualRows; i++) {
								static_cast<QVector<int>*>(m_dataContainer[n])->operator[](keepNValues - i) =
									static_cast<QVector<int>*>(m_dataContainer[n])->operator[](keepNValues - i - rowDiff);
							}
							for (int i = 0; i < rowDiff; i++)
								static_cast<QVector<int>*>(m_dataContainer[n])->operator[](i) = 0;
						}
						break;
					}
					case AbstractColumn::ColumnMode::BigInt: {
						auto* vector = static_cast<QVector<qint64>*>(columns.at(n)->data());
						m_dataContainer[n] = static_cast<void*>(vector);

						// if the keepNValues got smaller then we move the last keepNValues count of data
						// in the first keepNValues places
						if (m_actualRows > keepNValues) {
							for (int i = 0; i < keepNValues; i++) {
								static_cast<QVector<qint64>*>(m_dataContainer[n])->operator[](i) =
									static_cast<QVector<qint64>*>(m_dataContainer[n])->operator[](m_actualRows - keepNValues + i);
							}
						}

						// if the keepNValues got bigger we move the existing values to the last m_actualRows positions
						// then fill the remaining lines with 0
						if (m_actualRows < keepNValues) {
							vector->reserve(keepNValues);
							vector->resize(keepNValues);
							for (int i = 1; i <= m_actualRows; i++) {
								static_cast<QVector<qint64>*>(m_dataContainer[n])->operator[](keepNValues - i) =
									static_cast<QVector<qint64>*>(m_dataContainer[n])->operator[](keepNValues - i - rowDiff);
							}
							for (int i = 0; i < rowDiff; i++)
								static_cast<QVector<qint64>*>(m_dataContainer[n])->operator[](i) = 0;
						}
						break;
					}
					case AbstractColumn::ColumnMode::Text: {
						auto* vector = static_cast<QVector<QString>*>(columns.at(n)->data());
						m_dataContainer[n] = static_cast<void*>(vector);

						// if the keepNValues got smaller then we move the last keepNValues count of data
						// in the first keepNValues places
						if (m_actualRows > keepNValues) {
							for (int i = 0; i < keepNValues; i++) {
								static_cast<QVector<QString>*>(m_dataContainer[n])->operator[](i) =
									static_cast<QVector<QString>*>(m_dataContainer[n])->operator[](m_actualRows - keepNValues + i);
							}
						}

						// if the keepNValues got bigger we move the existing values to the last m_actualRows positions
						// then fill the remaining lines with empty lines
						if (m_actualRows < keepNValues) {
							vector->reserve(keepNValues);
							vector->resize(keepNValues);
							for (int i = 1; i <= m_actualRows; i++) {
								static_cast<QVector<QString>*>(m_dataContainer[n])->operator[](keepNValues - i) =
									static_cast<QVector<QString>*>(m_dataContainer[n])->operator[](keepNValues - i - rowDiff);
							}
							for (int i = 0; i < rowDiff; i++)
								static_cast<QVector<QString>*>(m_dataContainer[n])->operator[](i).clear();
						}
						break;
					}
					case AbstractColumn::ColumnMode::DateTime: {
						auto* vector = static_cast<QVector<QDateTime>*>(columns.at(n)->data());
						m_dataContainer[n] = static_cast<void*>(vector);

						// if the keepNValues got smaller then we move the last keepNValues count of data
						// in the first keepNValues places
						if (m_actualRows > keepNValues) {
							for (int i = 0; i < keepNValues; i++) {
								static_cast<QVector<QDateTime>*>(m_dataContainer[n])->operator[](i) =
									static_cast<QVector<QDateTime>*>(m_dataContainer[n])->operator[](m_actualRows - keepNValues + i);
							}
						}

						// if the keepNValues got bigger we move the existing values to the last m_actualRows positions
						// then fill the remaining lines with null datetime
						if (m_actualRows < keepNValues) {
							vector->reserve(keepNValues);
							vector->resize(keepNValues);
							for (int i = 1; i <= m_actualRows; i++) {
								static_cast<QVector<QDateTime>*>(m_dataContainer[n])->operator[](keepNValues - i) =
									static_cast<QVector<QDateTime>*>(m_dataContainer[n])->operator[](keepNValues - i - rowDiff);
							}
							for (int i = 0; i < rowDiff; i++)
								static_cast<QVector<QDateTime>*>(m_dataContainer[n])->operator[](i) = QDateTime();
						}
						break;
					}
					// TODO
					case AbstractColumn::ColumnMode::Month:
					case AbstractColumn::ColumnMode::Day:
						break;
					}
				}
				// if the keepNValues got smaller resize the spreadsheet
				if (m_actualRows > keepNValues)
					spreadsheet->setRowCount(keepNValues);

				// set the new row count
				m_actualRows = keepNValues;
				qDebug() << "actual rows: " << m_actualRows;
			}
		}
	}

	qDebug() << "starting m_actual rows calculated: " << m_actualRows << ", new data size: " << newData.size();

	int currentRow = 0; // indexes the position in the vector(column)
	int linesToRead = 0;

	if (m_prepared) {
		// increase row count if we don't have a fixed size
		// but only after the preparation step
		if (keepNValues == 0) {
			if (readingType != MQTTClient::ReadingType::TillEnd)
				m_actualRows += std::min(newData.size(), static_cast<qsizetype>(spreadsheet->mqttClient()->sampleSize()));
			else
				m_actualRows += newData.size();
		}

		// fixed size
		if (keepNValues != 0) {
			if (readingType == MQTTClient::ReadingType::TillEnd) {
				// we had more lines than the fixed size, so we read m_actualRows number of lines
				if (newLinesTillEnd > m_actualRows) {
					linesToRead = m_actualRows;
				} else
					linesToRead = newLinesTillEnd;
			} else {
				// we read max sample size number of lines when the reading mode
				// is ContinuouslyFixed or FromEnd
				if (spreadsheet->mqttClient()->sampleSize() <= keepNValues)
					linesToRead = std::min(spreadsheet->mqttClient()->sampleSize(), newLinesTillEnd);
				else
					linesToRead = std::min(keepNValues, newLinesTillEnd);
			}
		} else
			linesToRead = m_actualRows - rowCountBeforeResize;

		if (linesToRead == 0)
			return;
	} else {
		if (keepNValues != 0)
			linesToRead = newLinesTillEnd > m_actualRows ? m_actualRows : newLinesTillEnd;
		else
			linesToRead = newLinesTillEnd;
	}
	qDebug() << "linestoread = " << linesToRead;

	// new rows/resize columns if we don't have a fixed size
	if (keepNValues == 0) {
#ifdef PERFTRACE_LIVE_IMPORT
		PERFTRACE(QStringLiteral("AsciiLiveDataImportResizing: "));
#endif
		if (spreadsheet->rowCount() < m_actualRows)
			spreadsheet->setRowCount(m_actualRows);

		if (!m_prepared)
			currentRow = 0;
		else {
			// indexes the position in the vector(column)
			currentRow = rowCountBeforeResize;
		}

		// if we have fixed size, we do this only once in preparation, here we can use
		// m_prepared and we need something to decide whether it has a fixed size or increasing

		initDataContainer(spreadsheet);
	} else {
		// when we have a fixed size we have to pop sampleSize number of lines if specified
		// here popping, setting currentRow
		if (!m_prepared)
			currentRow = m_actualRows - std::min(newLinesTillEnd, m_actualRows);
		else {
			if (readingType == MQTTClient::ReadingType::TillEnd) {
				if (newLinesTillEnd > m_actualRows)
					currentRow = 0;
				else
					currentRow = m_actualRows - newLinesTillEnd;
			} else {
				// we read max sample rate number of lines when the reading mode
				// is ContinuouslyFixed or FromEnd
				currentRow = m_actualRows - linesToRead;
			}
		}

		if (m_prepared) {
#ifdef PERFTRACE_LIVE_IMPORT
			PERFTRACE(QStringLiteral("AsciiLiveDataImportPopping: "));
#endif
			const auto& columns = spreadsheet->children<Column>();
			for (int row = 0; row < linesToRead; ++row) {
				for (int col = 0; col < m_actualCols; ++col) {
					switch (columnModes[col]) {
					case AbstractColumn::ColumnMode::Double: {
						auto* vector = static_cast<QVector<double>*>(columns.at(col)->data());
						vector->pop_front();
						vector->reserve(m_actualRows);
						vector->resize(m_actualRows);
						m_dataContainer[col] = static_cast<void*>(vector);
						break;
					}
					case AbstractColumn::ColumnMode::Integer: {
						auto* vector = static_cast<QVector<int>*>(columns.at(col)->data());
						vector->pop_front();
						vector->reserve(m_actualRows);
						vector->resize(m_actualRows);
						m_dataContainer[col] = static_cast<void*>(vector);
						break;
					}
					case AbstractColumn::ColumnMode::BigInt: {
						auto* vector = static_cast<QVector<qint64>*>(columns.at(col)->data());
						vector->pop_front();
						vector->reserve(m_actualRows);
						vector->resize(m_actualRows);
						m_dataContainer[col] = static_cast<void*>(vector);
						break;
					}
					case AbstractColumn::ColumnMode::Text: {
						auto* vector = static_cast<QVector<QString>*>(columns.at(col)->data());
						vector->pop_front();
						vector->reserve(m_actualRows);
						vector->resize(m_actualRows);
						m_dataContainer[col] = static_cast<void*>(vector);
						break;
					}
					case AbstractColumn::ColumnMode::DateTime: {
						auto* vector = static_cast<QVector<QDateTime>*>(columns.at(col)->data());
						vector->pop_front();
						vector->reserve(m_actualRows);
						vector->resize(m_actualRows);
						m_dataContainer[col] = static_cast<void*>(vector);
						break;
					}
					// TODO
					case AbstractColumn::ColumnMode::Month:
					case AbstractColumn::ColumnMode::Day:
						break;
					}
				}
			}
		}
	}

	// from the last row we read the new data in the spreadsheet
	qDebug() << "reading from line: " << currentRow << " lines till end: " << newLinesTillEnd;
	qDebug() << "Lines to read: " << linesToRead << " actual rows: " << m_actualRows;
	int newDataIdx = 0;
	// From end means that we read the last sample size amount of data
	if (readingType == MQTTClient::ReadingType::FromEnd) {
		if (m_prepared) {
			if (newData.size() > spreadsheet->mqttClient()->sampleSize())
				newDataIdx = newData.size() - spreadsheet->mqttClient()->sampleSize();
		}
	}

	qDebug() << "newDataIdx: " << newDataIdx;

	// read the data
	static int indexColumnIdx = 0;
	{
#ifdef PERFTRACE_LIVE_IMPORT
		PERFTRACE(QStringLiteral("AsciiLiveDataImportFillingContainers: "));
#endif
		int row = 0;
		for (; row < linesToRead; ++row) {
			QString line;
			if (readingType == MQTTClient::ReadingType::FromEnd)
				line = newData.at(newDataIdx++);
			else
				line = newData.at(row);

			if (removeQuotesEnabled)
				line.remove(QLatin1Char('"'));

			if (line.isEmpty() || (!commentCharacter.isEmpty() && line.startsWith(commentCharacter)))
				continue;

			auto lineStringList = split(line, false);

			if (simplifyWhitespacesEnabled) {
				for (int i = 0; i < lineStringList.size(); ++i)
					lineStringList[i] = lineStringList[i].simplified();
			}

			// add index if required
			int offset = 0;
			if (createIndexEnabled) {
				int index = (keepNValues == 0) ? currentRow + 1 : indexColumnIdx++;
				static_cast<QVector<int>*>(m_dataContainer[0])->operator[](currentRow) = index;
				++offset;
			}

			// add current timestamp if required
			if (createTimestampEnabled) {
				static_cast<QVector<QDateTime>*>(m_dataContainer[offset])->operator[](currentRow) = QDateTime::currentDateTime();
				++offset;
			}

			// parse the columns
			for (int n = 0; n < m_actualCols - offset; ++n) {
				int col = n + offset;
				QString valueString;
				if (n < lineStringList.size())
					valueString = lineStringList.at(n);

				setValue(col, currentRow, valueString);
			}
			currentRow++;
		}
	}

	m_prepared = true;
	DEBUG(Q_FUNC_INFO << ", DONE");
}

/*!
 * \brief After the MQTTTopic was loaded, the filter is prepared for reading
 * \param prepared
 * \param topic
 * \param separator
 */
void AsciiFilterPrivate::setPreparedForMQTT(bool prepared, MQTTTopic* topic, const QString& separator) {
	m_prepared = prepared;
	// If originally it was prepared we have to restore the settings
	if (prepared) {
		m_separator = separator;
		m_actualCols = endColumn - startColumn + 1;
		m_actualRows = topic->rowCount();
		// set the column modes
		columnModes.resize(topic->columnCount());
		for (int i = 0; i < topic->columnCount(); ++i)
			columnModes[i] = topic->column(i)->columnMode();

		// set the data containers
		m_dataContainer.resize(m_actualCols);
		initDataContainer(topic);
	}
}
#endif
