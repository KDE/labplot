/*
    File                 : AsciiFilter.cpp
    Project              : LabPlot
    Description          : ASCII I/O-filter
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2009-2021 Stefan Gerlach <stefan.gerlach@uni.kn>
    SPDX-FileCopyrightText: 2009-2019 Alexander Semke <alexander.semke@web.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "backend/datasources/LiveDataSource.h"
#include "backend/core/column/Column.h"
#include "backend/core/Project.h"
#include "backend/datasources/filters/AsciiFilter.h"
#include "backend/datasources/filters/AsciiFilterPrivate.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/worksheet/plots/cartesian/XYCurve.h"
#include "backend/lib/macros.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/trace.h"

#ifdef HAVE_MQTT
#include "backend/datasources/MQTTClient.h"
#include "backend/datasources/MQTTTopic.h"
#endif

#include <KLocalizedString>
#include <KFilterDev>
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
AsciiFilter::AsciiFilter() : AbstractFileFilter(FileType::Ascii), d(new AsciiFilterPrivate(this)) {}

AsciiFilter::~AsciiFilter() = default;

/*!
  reads the content of the device \c device.
*/
void AsciiFilter::readDataFromDevice(QIODevice& device, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode importMode, int lines) {
	d->readDataFromDevice(device, dataSource, importMode, lines);
}

void AsciiFilter::readFromLiveDeviceNotFile(QIODevice &device, AbstractDataSource* dataSource) {
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
void AsciiFilter::readDataFromFile(const QString& fileName, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode importMode) {
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
  loads the predefined filter settings for \c filterName
*/
void AsciiFilter::loadFilterSettings(const QString& /*filterName*/) {
}

/*!
  saves the current settings as a new filter with the name \c filterName
*/
void AsciiFilter::saveFilterSettings(const QString& /*filterName*/) const {
}

/*!
  returns the list with the names of all saved
  (system wide or user defined) filter settings.
*/
QStringList AsciiFilter::predefinedFilters() {
	return QStringList();
}

/*!
  returns the list of all predefined separator characters.
*/
QStringList AsciiFilter::separatorCharacters() {
	return (QStringList() << "auto" << "TAB" << "SPACE" << "," << ";" << ":"
	        << ",TAB" << ";TAB" << ":TAB" << ",SPACE" << ";SPACE" << ":SPACE" << "2xSPACE" << "3xSPACE" << "4xSPACE" << "2xTAB");
}

/*!
returns the list of all predefined comment characters.
*/
QStringList AsciiFilter::commentCharacters() {
	return (QStringList() << "#" << "!" << "//" << "+" << "c" << ":" << ";");
}

/*!
returns the list of all predefined data types.
*/
QStringList AsciiFilter::dataTypes() {
	const QMetaObject& mo = AbstractColumn::staticMetaObject;
	const QMetaEnum& me = mo.enumerator(mo.indexOfEnumerator("ColumnMode"));
	QStringList list;
	for (int i = 0; i <= 100; ++i)	// me.keyCount() does not work because we have holes in enum
		if (me.valueToKey(i))
			list << me.valueToKey(i);
	return list;
}

QString AsciiFilter::fileInfoString(const QString& fileName) {
	QString info(i18n("Number of columns: %1", AsciiFilter::columnNumber(fileName)));
	info += QLatin1String("<br>");
	info += i18n("Number of lines: %1", AsciiFilter::lineNumber(fileName));
	return info;
}

/*!
    returns the number of columns in the file \c fileName.
*/
int AsciiFilter::columnNumber(const QString& fileName, const QString& separator) {
	KFilterDev device(fileName);
	if (!device.open(QIODevice::ReadOnly)) {
		DEBUG("Could not open file " << STDSTRING(fileName) << " for determining number of columns");
		return -1;
	}

	QString line = device.readLine();
	line.remove(QRegularExpression(QStringLiteral("[\\n\\r]")));

	QStringList lineStringList;
	if (separator.length() > 0)
		lineStringList = line.split(separator);
	else
		lineStringList = line.split(QRegularExpression(QStringLiteral("\\s+")));
	DEBUG("number of columns : " << lineStringList.size());

	return lineStringList.size();
}

size_t AsciiFilter::lineNumber(const QString& fileName) {
	KFilterDev device(fileName);

	if (!device.open(QIODevice::ReadOnly)) {
		DEBUG("Could not open file " << STDSTRING(fileName) << " to determine number of lines");

		return 0;
	}
// 	if (!device.canReadLine())
// 		return -1;

	size_t lineCount = 0;
#if defined(Q_OS_LINUX) || defined(Q_OS_BSD4)
	//on linux and BSD use wc, if available, which is much faster than counting lines in the file
	if (device.compressionType() == KCompressionDevice::None && !QStandardPaths::findExecutable(QLatin1String("wc")).isEmpty()) {
		QProcess wc;
		wc.start(QLatin1String("wc"), QStringList() << QLatin1String("-l") << fileName);
		size_t lineCount = 0;
		while (wc.waitForReadyRead()) {
			QString line(wc.readLine());
			// wc on macOS has leading spaces: use SkipEmptyParts
			lineCount = line.split(' ', QString::SkipEmptyParts)[0].toInt();
		}
		return lineCount;
	}
#endif

	while (!device.atEnd()) {
		device.readLine();
		lineCount++;
	}

	return lineCount;
}

/*!
  returns the number of lines in the device \c device and 0 if sequential.
  resets the position to 0!
*/
size_t AsciiFilter::lineNumber(QIODevice& device) const {
	if (device.isSequential())
		return 0;
// 	if (!device.canReadLine())
// 		DEBUG("WARNING in AsciiFilter::lineNumber(): device cannot 'readLine()' but using it anyway.");

	size_t lineCount = 0;
	device.seek(0);
	if (d->readingFile)
		lineCount = lineNumber(d->readingFileName);
	else {
		while (!device.atEnd()) {
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

void AsciiFilter::setDateTimeFormat(const QString &f) {
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
		d->nanValue = 0;
	else
		d->nanValue = std::numeric_limits<double>::quiet_NaN();
}
bool AsciiFilter::NaNValueToZeroEnabled() const {
	return (d->nanValue == 0);
}

void AsciiFilter::setRemoveQuotesEnabled(bool b) {
	d->removeQuotesEnabled = b;
}
bool AsciiFilter::removeQuotesEnabled() const {
	return d->removeQuotesEnabled;
}

void AsciiFilter::setVectorNames(const QString& s) {
	d->vectorNames.clear();
	if (!s.simplified().isEmpty())
		d->vectorNames = s.simplified().split(' ');
}
void AsciiFilter::setVectorNames(const QStringList& list) {
	d->vectorNames = list;
}
QStringList AsciiFilter::vectorNames() const {
	return d->vectorNames;
}

QVector<AbstractColumn::ColumnMode> AsciiFilter::columnModes() {
	return d->columnModes;
}

void AsciiFilter::setStartRow(const int r) {
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

//#####################################################################
//################### Private implementation ##########################
//#####################################################################
AsciiFilterPrivate::AsciiFilterPrivate(AsciiFilter* owner) : q(owner) {
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


//#####################################################################
//############################# Read ##################################
//#####################################################################
/*!
 * returns -1 if the device couldn't be opened, 1 if the current read position in the device is at the end and 0 otherwise.
 */
int AsciiFilterPrivate::prepareDeviceToRead(QIODevice& device) {
	DEBUG("AsciiFilterPrivate::prepareDeviceToRead(): is sequential = " << device.isSequential() << ", can readLine = " << device.canReadLine());

	if (!device.open(QIODevice::ReadOnly))
		return -1;

	if (device.atEnd() && !device.isSequential()) // empty file
		return 1;

/////////////////////////////////////////////////////////////////
	// Parse the first line:
	// Determine the number of columns, create the columns and use (if selected) the first row to name them
	QString firstLine;

	// skip the comment lines and read the first line
	if (!commentCharacter.isEmpty()) {
		do {
			if (!device.canReadLine())
				DEBUG(Q_FUNC_INFO << ", WARNING: device cannot 'readLine()' but using it anyway.");

			if (device.atEnd()) {
				DEBUG("device at end! Giving up.");
				if (device.isSequential())
					break;
				else
					return 1;
			}

			firstLine = device.readLine();
		} while (firstLine.startsWith(commentCharacter) || firstLine.simplified().isEmpty());
	} else
		firstLine = device.readLine();

	// navigate to the line where we asked to start reading from
	DEBUG(Q_FUNC_INFO << ", Skipping " << startRow - 1 << " lines");
	for (int i = 0; i < startRow - 1; ++i) {
		if (!device.canReadLine())
			DEBUG(Q_FUNC_INFO << ", WARNING: device cannot 'readLine()' but using it anyway.");

		if (device.atEnd()) {
			DEBUG("device at end! Giving up.");
			if (device.isSequential())
				break;
			else
				return 1;
		}

		firstLine = device.readLine();
		DEBUG("	line = " << STDSTRING(firstLine));
	}

	DEBUG(" device position after first line and comments = " << device.pos());
	firstLine.remove(QRegularExpression(QStringLiteral("[\\n\\r]")));	// remove any newline
	if (removeQuotesEnabled)
		firstLine = firstLine.remove(QLatin1Char('"'));

	//TODO: this doesn't work, the split below introduces whitespaces again
// 	if (simplifyWhitespacesEnabled)
// 		firstLine = firstLine.simplified();
	DEBUG("First line: \'" << STDSTRING(firstLine) << '\'');

	// determine separator and split first line
	QStringList firstLineStringList;
	if (separatingCharacter == "auto") {
		DEBUG("automatic separator");
		if (firstLine.indexOf(QLatin1Char('\t')) != -1) {
			//in case we have a mix of tabs and spaces in the header, give the tab character the preference
			m_separator = QLatin1Char('\t');
			firstLineStringList = firstLine.split(m_separator, (QString::SplitBehavior)skipEmptyParts);
		} else {
			const QRegularExpression regExp(QStringLiteral("[,;:]?\\s+"));
			firstLineStringList = firstLine.split(regExp, (QString::SplitBehavior)skipEmptyParts);

			if (!firstLineStringList.isEmpty()) {
				int length1 = firstLineStringList.at(0).length();
				if (firstLineStringList.size() > 1)
					m_separator = firstLine.mid(length1, 1);
				else
					m_separator = ' ';
			}
		}
	} else {	// use given separator
		// replace symbolic "TAB" with '\t'
		m_separator = separatingCharacter.replace(QLatin1String("2xTAB"), "\t\t", Qt::CaseInsensitive);
		m_separator = separatingCharacter.replace(QLatin1String("TAB"), "\t", Qt::CaseInsensitive);
		// replace symbolic "SPACE" with ' '
		m_separator = m_separator.replace(QLatin1String("2xSPACE"), QLatin1String("  "), Qt::CaseInsensitive);
		m_separator = m_separator.replace(QLatin1String("3xSPACE"), QLatin1String("   "), Qt::CaseInsensitive);
		m_separator = m_separator.replace(QLatin1String("4xSPACE"), QLatin1String("    "), Qt::CaseInsensitive);
		m_separator = m_separator.replace(QLatin1String("SPACE"), QLatin1String(" "), Qt::CaseInsensitive);
		firstLineStringList = firstLine.split(m_separator, (QString::SplitBehavior)skipEmptyParts);
	}
	DEBUG(Q_FUNC_INFO << ", separator: \'" << STDSTRING(m_separator) << '\'');
	DEBUG(Q_FUNC_INFO << ", number of columns: " << firstLineStringList.size());
	QDEBUG(Q_FUNC_INFO << ", first line: " << firstLineStringList);
	DEBUG(Q_FUNC_INFO << ", headerEnabled: " << headerEnabled);

	//optionally, remove potential spaces in the first line
	//TODO: this part should be obsolete actually if we do firstLine = firstLine.simplified(); above...
	if (simplifyWhitespacesEnabled) {
		for (int i = 0; i < firstLineStringList.size(); ++i)
			firstLineStringList[i] = firstLineStringList[i].simplified();
	}

	//in GUI in AsciiOptionsWidget we start counting from 1, subtract 1 here to start from zero
	m_actualStartRow = startRow - 1;

	if (headerEnabled) {	// use first line to name vectors (starting from startColumn)
		vectorNames = firstLineStringList.mid(startColumn - 1);
		++m_actualStartRow;
	}

	// set range to read
	if (endColumn == -1) {
		if (headerEnabled || vectorNames.size() == 0)
			endColumn = firstLineStringList.size(); // last column
		else
			//number of vector names provided in the import dialog (not more than the maximal number of columns in the file)
			endColumn = qMin(vectorNames.size(), firstLineStringList.size());
	}

	if (endColumn < startColumn)
		m_actualCols = 0;
	else
		m_actualCols = endColumn - startColumn + 1;

	//add index column
	if (createTimestampEnabled) {
		vectorNames.prepend(i18n("Timestamp"));
		m_actualCols++;
	}

	//add index column
	if (createIndexEnabled) {
		vectorNames.prepend(i18n("Index"));
		m_actualCols++;
	}

	QDEBUG(Q_FUNC_INFO << ", vector names =" << vectorNames);

//TEST: readline-seek-readline fails
	/*	qint64 testpos = device.pos();
		DEBUG("read data line @ pos " << testpos << " : " << STDSTRING(device.readLine()));
		device.seek(testpos);
		testpos = device.pos();
		DEBUG("read data line again @ pos " << testpos << "  : " << STDSTRING(device.readLine()));
	*/
/////////////////////////////////////////////////////////////////

	// parse first data line to determine data type for each column
	// if the first line was already parsed as the header, read the next line
	if (headerEnabled && !device.isSequential())
		firstLineStringList = getLineString(device);

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
		const int index{ col - startColumn + 1 };
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
		columnModes[index] = AbstractFileFilter::columnMode(valueString, dateTimeFormat, numberFormat);
		col++;
	}
#ifndef NDEBUG
	for (const auto mode : columnModes)
		DEBUG(Q_FUNC_INFO << ", column mode = " << static_cast<int>(mode));
#endif

	// parsing more lines to better determine data types
	for (unsigned int i = 0; i < m_dataTypeLines; ++i) {
		if (device.atEnd())	// EOF reached
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
		DEBUG(Q_FUNC_INFO << ", column mode = " << static_cast<int>(mode));
#endif

	// ATTENTION: This resets the position in the device to 0
	m_actualRows = (int)q->lineNumber(device);

	const int actualEndRow = (endRow == -1 || endRow > m_actualRows) ? m_actualRows : endRow;
	if (actualEndRow > m_actualStartRow)
		m_actualRows = actualEndRow - m_actualStartRow;
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

	//dirty hack: set readingFile and readingFileName in order to know in lineNumber(QIODevice)
	//that we're reading from a file and to benefit from much faster wc on linux
	//TODO: redesign the APIs and remove this later
	readingFile = true;
	readingFileName = fileName;
	KFilterDev device(fileName);
	readDataFromDevice(device, dataSource, importMode);
	readingFile = false;
}

qint64 AsciiFilterPrivate::readFromLiveDevice(QIODevice& device, AbstractDataSource* dataSource, qint64 from) {
	DEBUG(Q_FUNC_INFO << ", bytes available = " << device.bytesAvailable() << ", from = " << from);
	if (device.bytesAvailable() <= 0) {
		DEBUG("	No new data available");
		return 0;
	}

	//TODO: may be also a matrix?
	auto* spreadsheet = dynamic_cast<LiveDataSource*>(dataSource);
	if (!spreadsheet)
		return 0;

	if (spreadsheet->sourceType() != LiveDataSource::SourceType::FileOrPipe)
		if (device.isSequential() && device.bytesAvailable() < (int)sizeof(quint16))
			return 0;

	if (!m_prepared) {
		DEBUG("	Preparing ..");

		switch (spreadsheet->sourceType()) {
		case LiveDataSource::SourceType::FileOrPipe: {
			const int deviceError = prepareDeviceToRead(device);
			if (deviceError != 0) {
				DEBUG("	Device error = " << deviceError);
				return 0;
			}
			break;
		}
		case LiveDataSource::SourceType::NetworkTcpSocket:
		case LiveDataSource::SourceType::NetworkUdpSocket:
		case LiveDataSource::SourceType::LocalSocket:
		case LiveDataSource::SourceType::SerialPort:
			m_actualRows = 1;
			m_actualCols = 1;
			columnModes.clear();
			if (createIndexEnabled) {
				columnModes << AbstractColumn::ColumnMode::Integer;
				vectorNames << i18n("Index");
				m_actualCols++;
			}

			if (createTimestampEnabled) {
				columnModes << AbstractColumn::ColumnMode::DateTime;
				vectorNames << i18n("Timestamp");
				m_actualCols++;
			}

			//add column for the actual value
			columnModes << AbstractColumn::ColumnMode::Double;
			vectorNames << i18n("Value");

			QDEBUG("	vector names = " << vectorNames);
			break;
		case LiveDataSource::SourceType::MQTT:
			break;
		}

		// prepare import for spreadsheet
		spreadsheet->setUndoAware(false);
		spreadsheet->resize(AbstractFileFilter::ImportMode::Replace, vectorNames, m_actualCols);

		//columns in a file data source don't have any manual changes.
		//make the available columns undo unaware and suppress the "data changed" signal.
		//data changes will be propagated via an explicit Column::setChanged() call once new data was read.
		for (int i = 0; i < spreadsheet->childCount<Column>(); i++) {
			spreadsheet->child<Column>(i)->setUndoAware(false);
			spreadsheet->child<Column>(i)->setSuppressDataChangedSignal(true);
		}

		int keepNValues = spreadsheet->keepNValues();
		if (keepNValues == 0)
			spreadsheet->setRowCount(m_actualRows > 1 ? m_actualRows : 1);
		else {
			spreadsheet->setRowCount(keepNValues);
			m_actualRows = keepNValues;
		}

		m_dataContainer.resize(m_actualCols);
		initDataContainers(spreadsheet);

		DEBUG("	data source resized to col: " << m_actualCols);
		DEBUG("	data source rowCount: " << spreadsheet->rowCount());
		DEBUG("	Prepared!");
	}

	qint64 bytesread = 0;

#ifdef PERFTRACE_LIVE_IMPORT
	PERFTRACE("AsciiLiveDataImportTotal: ");
#endif
	LiveDataSource::ReadingType readingType;
	if (!m_prepared) {
		readingType = LiveDataSource::ReadingType::TillEnd;
	} else {
		//we have to read all the data when reading from end
		//so we set readingType to TillEnd
		if (spreadsheet->readingType() == LiveDataSource::ReadingType::FromEnd)
			readingType = LiveDataSource::ReadingType::TillEnd;
		//if we read the whole file we just start from the beginning of it
		//and read till end
		else if (spreadsheet->readingType() == LiveDataSource::ReadingType::WholeFile)
			readingType = LiveDataSource::ReadingType::TillEnd;
		else
			readingType = spreadsheet->readingType();
	}
	DEBUG("	Reading type = " << ENUM_TO_STRING(LiveDataSource, ReadingType, readingType));

	//move to the last read position, from == total bytes read
	//since the other source types are sequential we cannot seek on them
	if (spreadsheet->sourceType() == LiveDataSource::SourceType::FileOrPipe)
		device.seek(from);

	//count the new lines, increase actualrows on each
	//now we read all the new lines, if we want to use sample rate
	//then here we can do it, if we have actually sample rate number of lines :-?
	int newLinesForSampleSizeNotTillEnd = 0;
	int newLinesTillEnd = 0;
	QVector<QString> newData;
	if (readingType != LiveDataSource::ReadingType::TillEnd)
		newData.resize(spreadsheet->sampleSize());

	int newDataIdx = 0;
	{
#ifdef PERFTRACE_LIVE_IMPORT
		PERFTRACE("AsciiLiveDataImportReadingFromFile: ");
#endif
		DEBUG("	source type = " << ENUM_TO_STRING(LiveDataSource, SourceType, spreadsheet->sourceType()));
		while (!device.atEnd()) {
			if (readingType != LiveDataSource::ReadingType::TillEnd) {
				switch (spreadsheet->sourceType()) {	// different sources need different read methods
				case LiveDataSource::SourceType::LocalSocket:
					newData[newDataIdx++] = device.readAll();
					break;
				case LiveDataSource::SourceType::NetworkUdpSocket:
					newData[newDataIdx++] = device.read(device.bytesAvailable());
					break;
				case LiveDataSource::SourceType::FileOrPipe:
					newData.push_back(device.readLine());
					break;
				case LiveDataSource::SourceType::NetworkTcpSocket:
				//TODO: check serial port
				case LiveDataSource::SourceType::SerialPort:
					newData[newDataIdx++] = device.read(device.bytesAvailable());
					break;
				case LiveDataSource::SourceType::MQTT:
					break;
				}
			} else {	// ReadingType::TillEnd
				switch (spreadsheet->sourceType()) {	// different sources need different read methods
				case LiveDataSource::SourceType::LocalSocket:
					newData.push_back(device.readAll());
					break;
				case LiveDataSource::SourceType::NetworkUdpSocket:
					newData.push_back(device.read(device.bytesAvailable()));
					break;
				case LiveDataSource::SourceType::FileOrPipe:
					newData.push_back(device.readLine());
					break;
				case LiveDataSource::SourceType::NetworkTcpSocket:
				//TODO: check serial port
				case LiveDataSource::SourceType::SerialPort:
					newData.push_back(device.read(device.bytesAvailable()));
					break;
				case LiveDataSource::SourceType::MQTT:
					break;
				}
			}
			newLinesTillEnd++;

			if (readingType != LiveDataSource::ReadingType::TillEnd) {
				newLinesForSampleSizeNotTillEnd++;
				//for Continuous reading and FromEnd we read sample rate number of lines if possible
				//here TillEnd and Whole file behave the same
				if (newLinesForSampleSizeNotTillEnd == spreadsheet->sampleSize())
					break;
			}
		}
		QDEBUG("	data read: " << newData);
	}

	//now we reset the readingType
	if (spreadsheet->readingType() == LiveDataSource::ReadingType::FromEnd)
		readingType = spreadsheet->readingType();

	//we had less new lines than the sample size specified
	if (readingType != LiveDataSource::ReadingType::TillEnd)
		QDEBUG("	Removed empty lines: " << newData.removeAll(QString()));

	//back to the last read position before counting when reading from files
	if (spreadsheet->sourceType() == LiveDataSource::SourceType::FileOrPipe)
		device.seek(from);

	const int spreadsheetRowCountBeforeResize = spreadsheet->rowCount();

	int currentRow = 0; // indexes the position in the vector(column)
	int linesToRead = 0;
	int keepNValues = spreadsheet->keepNValues();

	DEBUG("	Increase row count. keepNValues = " << keepNValues);
	if (m_prepared) {
		//increase row count if we don't have a fixed size
		//but only after the preparation step
		if (keepNValues == 0) {
			DEBUG("	keep All values");
			if (readingType != LiveDataSource::ReadingType::TillEnd)
				m_actualRows += qMin(newData.size(), spreadsheet->sampleSize());
			else {
				//we don't increase it if we reread the whole file, we reset it
				if (!(spreadsheet->readingType() == LiveDataSource::ReadingType::WholeFile))
					m_actualRows += newData.size();
				else
					m_actualRows = newData.size();
			}

			//appending
			if (spreadsheet->readingType() == LiveDataSource::ReadingType::WholeFile)
				linesToRead = m_actualRows;
			else
				linesToRead = m_actualRows - spreadsheetRowCountBeforeResize;
		} else {	// fixed size
			DEBUG("	keep " << keepNValues << " values");
			if (readingType == LiveDataSource::ReadingType::TillEnd) {
				//we had more lines than the fixed size, so we read m_actualRows number of lines
				if (newLinesTillEnd > m_actualRows) {
					linesToRead = m_actualRows;
					//TODO after reading we should skip the next data lines
					//because it's TillEnd actually
				} else
					linesToRead = newLinesTillEnd;
			} else {
				//we read max sample size number of lines when the reading mode
				//is ContinuouslyFixed or FromEnd, WholeFile is disabled
				linesToRead = qMin(spreadsheet->sampleSize(), newLinesTillEnd);
			}
		}

		if (linesToRead == 0)
			return 0;
	} else	// not prepared
		linesToRead = newLinesTillEnd;

	DEBUG("	lines to read = " << linesToRead);
	DEBUG("	actual rows (w/o header) = " << m_actualRows);

	//TODO
// 	if (spreadsheet->sourceType() == LiveDataSource::SourceType::FileOrPipe || spreadsheet->sourceType() == LiveDataSource::SourceType::NetworkUdpSocket) {
// 		if (m_actualRows < linesToRead) {
// 			DEBUG("	SET lines to read to " << m_actualRows);
// 			linesToRead = m_actualRows;
// 		}
// 	}

	//new rows/resize columns if we don't have a fixed size
	//TODO if the user changes this value..m_resizedToFixedSize..setResizedToFixedSize
	if (keepNValues == 0) {
#ifdef PERFTRACE_LIVE_IMPORT
		PERFTRACE("AsciiLiveDataImportResizing: ");
#endif
		if (spreadsheet->rowCount() < m_actualRows)
			spreadsheet->setRowCount(m_actualRows);

		if (!m_prepared)
			currentRow = 0;
		else {
			// indexes the position in the vector(column)
			if (spreadsheet->readingType() == LiveDataSource::ReadingType::WholeFile)
				currentRow = 0;
			else
				currentRow = spreadsheetRowCountBeforeResize;
		}

		// if we have fixed size, we do this only once in preparation, here we can use
		// m_prepared and we need something to decide whether it has a fixed size or increasing
		initDataContainers(spreadsheet);
	} else {	// fixed size
		//when we have a fixed size we have to pop sampleSize number of lines if specified
		//here popping, setting currentRow
		if (!m_prepared) {
			if (spreadsheet->readingType() == LiveDataSource::ReadingType::WholeFile)
				currentRow = 0;
			else
				currentRow = m_actualRows - qMin(newLinesTillEnd, m_actualRows);
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
				//we read max sample size number of lines when the reading mode
				//is ContinuouslyFixed or FromEnd
				currentRow = m_actualRows - qMin(spreadsheet->sampleSize(), newLinesTillEnd);
			}
		}

		if (m_prepared) {
#ifdef PERFTRACE_LIVE_IMPORT
			PERFTRACE("AsciiLiveDataImportPopping: ");
#endif
			// enable data change signal
			for (int col = 0; col < m_actualCols; ++col)
				spreadsheet->child<Column>(col)->setSuppressDataChangedSignal(false);

			for (int row = 0; row < linesToRead; ++row) {
				for (int col = 0; col < m_actualCols; ++col) {
					switch (columnModes[col]) {
					case AbstractColumn::ColumnMode::Double: {
						auto* vector = static_cast<QVector<double>* >(spreadsheet->child<Column>(col)->data());
						vector->pop_front();
						vector->resize(m_actualRows);
						m_dataContainer[col] = static_cast<void *>(vector);
						break;
					}
					case AbstractColumn::ColumnMode::Integer: {
						auto* vector = static_cast<QVector<int>* >(spreadsheet->child<Column>(col)->data());
						vector->pop_front();
						vector->resize(m_actualRows);
						m_dataContainer[col] = static_cast<void *>(vector);
						break;
					}
					case AbstractColumn::ColumnMode::BigInt: {
						auto* vector = static_cast<QVector<qint64>* >(spreadsheet->child<Column>(col)->data());
						vector->pop_front();
						vector->resize(m_actualRows);
						m_dataContainer[col] = static_cast<void *>(vector);
						break;
					}
					case AbstractColumn::ColumnMode::Text: {
						auto* vector = static_cast<QVector<QString>*>(spreadsheet->child<Column>(col)->data());
						vector->pop_front();
						vector->resize(m_actualRows);
						m_dataContainer[col] = static_cast<void *>(vector);
						break;
					}
					case AbstractColumn::ColumnMode::DateTime: {
						auto* vector = static_cast<QVector<QDateTime>* >(spreadsheet->child<Column>(col)->data());
						vector->pop_front();
						vector->resize(m_actualRows);
						m_dataContainer[col] = static_cast<void *>(vector);
						break;
					}
					//TODO
					case AbstractColumn::ColumnMode::Month:
					case AbstractColumn::ColumnMode::Day:
						break;
					}
				}
			}
		}
	}

	// from the last row we read the new data in the spreadsheet
	DEBUG("	Reading from line "  << currentRow << " till end line " << newLinesTillEnd);
	DEBUG("	Lines to read:" << linesToRead <<", actual rows:" << m_actualRows << ", actual cols:" << m_actualCols);
	newDataIdx = 0;
	if (readingType == LiveDataSource::ReadingType::FromEnd) {
		if (m_prepared) {
			if (newData.size() > spreadsheet->sampleSize())
				newDataIdx = newData.size() - spreadsheet->sampleSize();
			//since we skip a couple of lines, we need to count those bytes too
			for (int i = 0; i < newDataIdx; ++i)
				bytesread += newData.at(i).size();
		}
	}
	DEBUG("	newDataIdx: " << newDataIdx);

	static int indexColumnIdx = 1;
	{
#ifdef PERFTRACE_LIVE_IMPORT
		PERFTRACE("AsciiLiveDataImportFillingContainers: ");
#endif
		int row = 0;

		if (readingType == LiveDataSource::ReadingType::TillEnd || (readingType == LiveDataSource::ReadingType::ContinuousFixed)) {
			if (headerEnabled) {
				if (!m_prepared) {
					row = 1;
					bytesread += newData.at(0).size();
				}
			}
		}
		if (spreadsheet->sourceType() == LiveDataSource::SourceType::FileOrPipe) {
			if (readingType == LiveDataSource::ReadingType::WholeFile) {
				if (headerEnabled) {
					row = 1;
					bytesread += newData.at(0).size();
				}
			}
		}

		for (; row < linesToRead; ++row) {
			DEBUG("\n	Reading row " << row + 1 << " of " << linesToRead);
			QString line;
			if (readingType == LiveDataSource::ReadingType::FromEnd)
				line = newData.at(newDataIdx++);
			else
				line = newData.at(row);
			//when we read the whole file we don't care about the previous position
			//so we don't have to count those bytes
			if (readingType != LiveDataSource::ReadingType::WholeFile) {
				if (spreadsheet->sourceType() == LiveDataSource::SourceType::FileOrPipe) {
					bytesread += line.size();
				}
			}

			if (removeQuotesEnabled)
				line.remove(QLatin1Char('"'));

			if (line.isEmpty() || (!commentCharacter.isEmpty() && line.startsWith(commentCharacter))) // skip empty or commented lines
				continue;

			QStringList lineStringList;
			// only FileOrPipe support multiple columns
			if (spreadsheet->sourceType() == LiveDataSource::SourceType::FileOrPipe)
				lineStringList = line.split(m_separator, (QString::SplitBehavior)skipEmptyParts);
			else
				lineStringList << line;
			QDEBUG("	line = " << lineStringList << ", separator = \'" << m_separator << "\'");

			DEBUG("	Line bytes: " << line.size() << " line: " << STDSTRING(line));
			if (simplifyWhitespacesEnabled) {
				for (int i = 0; i < lineStringList.size(); ++i)
					lineStringList[i] = lineStringList[i].simplified();
			}

			//add index if required
			int offset = 0;
			if (createIndexEnabled) {
				int index = (spreadsheet->keepNValues() == 0) ? currentRow + 1 : indexColumnIdx++;
				static_cast<QVector<int>*>(m_dataContainer[0])->operator[](currentRow) = index;
				++offset;
			}

			//add current timestamp if required
			if (createTimestampEnabled) {
				static_cast<QVector<QDateTime>*>(m_dataContainer[offset])->operator[](currentRow) = QDateTime::currentDateTime();
				++offset;
			}

			//parse columns
			for (int n = offset; n < m_actualCols; ++n) {
				QString valueString;
				if (n - offset < lineStringList.size())
					valueString = lineStringList.at(n - offset);

				setValue(n, currentRow, valueString);
			}
			currentRow++;
		}
	}

	if (m_prepared) {
		//notify all affected columns and plots about the changes
		PERFTRACE("AsciiLiveDataImport, notify affected columns and plots");

		//determine the dependent plots
		QVector<CartesianPlot*> plots;
		for (int n = 0; n < m_actualCols; ++n)
			spreadsheet->column(n)->addUsedInPlots(plots);

		//suppress retransform in the dependent plots
		for (auto* plot : plots)
			plot->setSuppressRetransform(true);

		for (int n = 0; n < m_actualCols; ++n)
			spreadsheet->column(n)->setChanged();

		//retransform the dependent plots
		for (auto* plot : plots) {
			plot->setSuppressRetransform(false);
			plot->dataChanged(-1, -1); // TODO: check if all ranges must be updated!
		}
	} else
		m_prepared = true;

	DEBUG("AsciiFilterPrivate::readFromLiveDevice() DONE");
	return bytesread;
}

/*!
    reads the content of device \c device to the data source \c dataSource. Uses the settings defined in the data source.
*/
void AsciiFilterPrivate::readDataFromDevice(QIODevice& device, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode importMode, int lines) {
	DEBUG(Q_FUNC_INFO << ", dataSource = " << dataSource
	      << ", mode = " << ENUM_TO_STRING(AbstractFileFilter, ImportMode, importMode) << ", lines = " << lines);

	if (!m_prepared) {
		const int deviceError = prepareDeviceToRead(device);
		if (deviceError != 0) {
			DEBUG(Q_FUNC_INFO << ", DEVICE ERROR = " << deviceError);
			return;
		}

		// matrix data has only one column mode
		if (dynamic_cast<Matrix*>(dataSource)) {
			auto mode = columnModes[0];
			//TODO: remove this when Matrix supports text type
			if (mode == AbstractColumn::ColumnMode::Text)
				mode = AbstractColumn::ColumnMode::Double;
			for (auto& c : columnModes)
				if (c != mode)
					c = mode;
		}

		Q_ASSERT(dataSource);
		m_columnOffset = dataSource->prepareImport(m_dataContainer, importMode, m_actualRows, m_actualCols, vectorNames, columnModes);
		m_prepared = true;
	}

	DEBUG("locale = " << STDSTRING(QLocale::languageToString(numberFormat)));

	// Read the data
	int currentRow = 0;	// indexes the position in the vector(column)
	if (lines == -1)
		lines = m_actualRows;

	//skip data lines, if required
	DEBUG("	Skipping " << m_actualStartRow << " lines");
	for (int i = 0; i < m_actualStartRow; ++i)
		device.readLine();

	DEBUG("	Reading " << qMin(lines, m_actualRows)  << " lines, " << m_actualCols << " columns");

	if (qMin(lines, m_actualRows) == 0 || m_actualCols == 0)
		return;

	QString line;
	QString valueString;
	//Don't put the definition QStringList lineStringList outside of the for-loop,
	//the compiler doesn't seem to optimize the destructor of QList well enough in this case.

	lines = qMin(lines, m_actualRows);
	int progressIndex = 0;
	const qreal progressInterval = 0.01*lines; //update on every 1% only

	for (int i = 0; i < lines; ++i) {
		line = device.readLine();

		// remove any newline
		line.remove(QLatin1Char('\n'));
		line.remove(QLatin1Char('\r'));

		if (removeQuotesEnabled)
			line.remove(QLatin1Char('"'));

		if (line.isEmpty() || (!commentCharacter.isEmpty() && line.startsWith(commentCharacter))) // skip empty or commented lines
			continue;

		QStringList lineStringList = line.split(m_separator, (QString::SplitBehavior)skipEmptyParts);
// 		DEBUG("	Line bytes: " << line.size() << " line: " << STDSTRING(line));

		if (simplifyWhitespacesEnabled) {
			for (int i = 0; i < lineStringList.size(); ++i)
				lineStringList[i] = lineStringList[i].simplified();
		}

		// remove left white spaces
		if (skipEmptyParts) {
			for (int n = 0; n < lineStringList.size(); ++n) {
				valueString = lineStringList.at(n);
				if (!QString::compare(valueString, " ")) {
					lineStringList.removeAt(n);
					n--;
				}
			}
		}

		//parse columns
		DEBUG(Q_FUNC_INFO << ", actual cols = " << m_actualCols)
		for (int n = 0; n < m_actualCols; ++n) {
			// index column if required
			if (n == 0 && createIndexEnabled) {
				static_cast<QVector<int>*>(m_dataContainer[0])->operator[](currentRow) = i + 1;
				continue;
			}

			//column counting starts with 1, subtract 1 as well as another 1 for the index column if required
			int col = createIndexEnabled ? n + startColumn - 2: n + startColumn - 1;
			QString valueString;
			if (col < lineStringList.size())
				valueString = lineStringList.at(col);

			setValue(n, currentRow, valueString);
		}
		currentRow++;

		//ask to update the progress bar only if we have more than 1000 lines
		//only in 1% steps
		progressIndex++;
		if (lines > 1000 && progressIndex > progressInterval) {
			Q_EMIT q->completed(100 * currentRow/lines);
			progressIndex = 0;
			QApplication::processEvents(QEventLoop::AllEvents, 0);
		}
	}

	DEBUG(Q_FUNC_INFO <<", Read " << currentRow << " lines");

	//we might have skipped empty lines above. shrink the spreadsheet if the number of read lines (=currentRow)
	//is smaller than the initial size of the spreadsheet (=m_actualRows).
	//TODO: should also be relevant for Matrix
	auto* s = dynamic_cast<Spreadsheet*>(dataSource);
	if (s && currentRow != m_actualRows && importMode == AbstractFileFilter::ImportMode::Replace)
		s->setRowCount(currentRow);

	Q_ASSERT(dataSource);
	dataSource->finalizeImport(m_columnOffset, startColumn, startColumn + m_actualCols - 1, dateTimeFormat, importMode);
}

//#####################################################################
//############################ Preview ################################
//#####################################################################

/*!
 * preview for special devices (local/UDP/TCP socket or serial port)
 */
QVector<QStringList> AsciiFilterPrivate::preview(QIODevice &device) {
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

	//TODO: serial port "read(nBytes)"?
	while (!device.atEnd()) {
		if (device.canReadLine())
			newData.push_back(device.readLine());
		else	// UDP fails otherwise
			newData.push_back(device.readAll());
		linesToRead++;
	}
	QDEBUG("	data = " << newData);

	if (linesToRead == 0)
		return dataStrings;

	vectorNames.clear();
	columnModes.clear();

	if (createIndexEnabled) {
		columnModes << AbstractColumn::ColumnMode::Integer;
		vectorNames << i18n("Index");
	}

	if (createTimestampEnabled) {
		columnModes << AbstractColumn::ColumnMode::DateTime;
		vectorNames << i18n("Timestamp");
	}

	//parse the first data line to determine data type for each column
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
	QStringList firstLineStringList = newData.at(0).split(' ', Qt::SkipEmptyParts);
#else
	QStringList firstLineStringList = newData.at(0).split(' ', QString::SkipEmptyParts);
#endif
	int i = 1;
	for (auto& valueString : firstLineStringList) {
		if (simplifyWhitespacesEnabled)
			valueString = valueString.simplified();
		if (removeQuotesEnabled)
			valueString.remove(QLatin1Char('"'));
		if (skipEmptyParts && !QString::compare(valueString, " "))	// handle left white spaces
			continue;

		vectorNames << i18n("Value %1", i);
		columnModes << AbstractFileFilter::columnMode(valueString, dateTimeFormat, numberFormat);
		++i;
	}

	int offset = int(createIndexEnabled) + int(createTimestampEnabled);
	QString line;

	//loop over all lines in the new data in the device and parse the available columns
	for (int i = 0; i < linesToRead; ++i) {
		line = newData.at(i);

		// remove any newline
		line = line.remove('\n');
		line = line.remove('\r');

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

#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
		QStringList lineStringList = line.split(' ', Qt::SkipEmptyParts);
#else
		QStringList lineStringList = line.split(' ', QString::SkipEmptyParts);
#endif
		QDEBUG(" line = " << lineStringList);

		//parse columns
		DEBUG(Q_FUNC_INFO << ", number of columns = " << lineStringList.size())
		for (int n = 0; n < lineStringList.size(); ++n) {
			if (n < lineStringList.size()) {
				QString valueString = lineStringList.at(n);
				if (removeQuotesEnabled)
					valueString.remove(QLatin1Char('"'));

				if (skipEmptyParts && !QString::compare(valueString, " "))	// handle left white spaces
					continue;

				lineString += previewValue(valueString, columnModes[n+offset]);
			} else 	// missing columns in this line
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
	QVector<QStringList> dataStrings;

	//dirty hack: set readingFile and readingFileName in order to know in lineNumber(QIODevice)
	//that we're reading from a file and to benefit from much faster wc on linux
	//TODO: redesign the APIs and remove this later
	readingFile = true;
	readingFileName = fileName;
	KFilterDev device(fileName);
	const int deviceError = prepareDeviceToRead(device);
	readingFile = false;

	if (deviceError != 0) {
		DEBUG("Device error = " << deviceError);
		return dataStrings;
	}

	//number formatting
	DEBUG("locale = " << STDSTRING(QLocale::languageToString(numberFormat)));

	// Read the data
	if (lines == -1)
		lines = m_actualRows;

	// set column names for preview
	if (!headerEnabled) {
		int start = 0;
		if (createIndexEnabled)
			start = 1;
		for (int i = start; i < m_actualCols; i++)
			vectorNames << "Column " + QString::number(i + 1);
	}
	QDEBUG("	column names = " << vectorNames);

	//skip data lines, if required
	DEBUG("	Skipping " << m_actualStartRow << " line(s)");
	for (int i = 0; i < m_actualStartRow; ++i)
		device.readLine();

	DEBUG("	Generating preview for " << qMin(lines, m_actualRows)  << " lines");
	QString line;

	//loop over the preview lines in the file and parse the available columns
	for (int i = 0; i < qMin(lines, m_actualRows); ++i) {
		line = device.readLine();

		// remove any newline
		line = line.remove('\n');
		line = line.remove('\r');

		if (line.isEmpty() || (!commentCharacter.isEmpty() && line.startsWith(commentCharacter))) // skip empty or commented lines
			continue;

		QStringList lineStringList = line.split(m_separator, (QString::SplitBehavior)skipEmptyParts);
		//QDEBUG(" line = " << lineStringList);
		//DEBUG("	Line bytes: " << line.size() << " line: " << STDSTRING(line));

		if (simplifyWhitespacesEnabled) {
			for (int i = 0; i < lineStringList.size(); ++i)
				lineStringList[i] = lineStringList[i].simplified();
		}

		QStringList lineString;

		//parse columns
		for (int n = 0; n < m_actualCols; ++n) {
			// index column if required
			if (n == 0 && createIndexEnabled) {
				lineString += QString::number(i + 1);
				continue;
			}

			//column counting starts with 1, subtract 1 as well as another 1 for the index column if required
			int col = createIndexEnabled ? n + startColumn - 2 : n + startColumn - 1;

			if (col < lineStringList.size()) {
				QString valueString = lineStringList.at(col);
				if (removeQuotesEnabled)
					valueString.remove(QLatin1Char('"'));

				if (skipEmptyParts && !QString::compare(valueString, " "))	// handle left white spaces
					continue;

				lineString += previewValue(valueString, columnModes[n]);
			} else 	// missing columns in this line
				lineString += QString();
		}

		dataStrings << lineString;
	}

	return dataStrings;
}

//#####################################################################
//####################### Helper functions ############################
//#####################################################################
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
	case AbstractColumn::ColumnMode::Month:	// never happens
	case AbstractColumn::ColumnMode::Day:
		break;
	}
	return result;
}

//set value depending on data type
void AsciiFilterPrivate::setValue(int col, int row, const QString& valueString) {
	if (!valueString.isEmpty()) {
		switch (columnModes.at(col)) {
		case AbstractColumn::ColumnMode::Double: {
			bool isNumber;
			const double value = locale.toDouble(valueString, &isNumber);
			static_cast<QVector<double>*>(m_dataContainer[col])->operator[](row) = (isNumber ? value : nanValue);
			break;
		}
		case AbstractColumn::ColumnMode::Integer: {
			bool isNumber;
			const int value = locale.toInt(valueString, &isNumber);
			static_cast<QVector<int>*>(m_dataContainer[col])->operator[](row) = (isNumber ? value : 0);
			break;
		}
		case AbstractColumn::ColumnMode::BigInt: {
			bool isNumber;
			const qint64 value = locale.toLongLong(valueString, &isNumber);
			static_cast<QVector<qint64>*>(m_dataContainer[col])->operator[](row) = (isNumber ? value : 0);
			break;
		}
		case AbstractColumn::ColumnMode::DateTime: {
			QDateTime valueDateTime = parseDateTime(valueString, dateTimeFormat);
			static_cast<QVector<QDateTime>*>(m_dataContainer[col])->operator[](row) = valueDateTime.isValid() ? valueDateTime : QDateTime();
			break;
		}
		case AbstractColumn::ColumnMode::Text: {
			auto* colData = static_cast<QVector<QString>*>(m_dataContainer[col]);
			colData->operator[](row) = valueString;
			break;
		}
		case AbstractColumn::ColumnMode::Month:	// never happens
		case AbstractColumn::ColumnMode::Day:
			break;
		}
	} else {	// missing columns in this line
		switch (columnModes.at(col)) {
		case AbstractColumn::ColumnMode::Double:
			static_cast<QVector<double>*>(m_dataContainer[col])->operator[](row) = nanValue;
			break;
		case AbstractColumn::ColumnMode::Integer:
			static_cast<QVector<int>*>(m_dataContainer[col])->operator[](row) = 0;
			break;
		case AbstractColumn::ColumnMode::BigInt:
			static_cast<QVector<qint64>*>(m_dataContainer[col])->operator[](row) = 0;
			break;
		case AbstractColumn::ColumnMode::DateTime:
			static_cast<QVector<QDateTime>*>(m_dataContainer[col])->operator[](row) = QDateTime();
			break;
		case AbstractColumn::ColumnMode::Text:
			static_cast<QVector<QString>*>(m_dataContainer[col])->operator[](row).clear();
			break;
		case AbstractColumn::ColumnMode::Month:	// never happens
		case AbstractColumn::ColumnMode::Day:
			break;
		}
	}
}

void AsciiFilterPrivate::initDataContainers(Spreadsheet* spreadsheet) {
	DEBUG("	Initializing the data containers ..");
	for (int n = 0; n < m_actualCols; ++n) {
		// data() returns a void* which is a pointer to any data type (see ColumnPrivate.cpp)
		spreadsheet->child<Column>(n)->setColumnMode(columnModes[n]);
		switch (columnModes[n]) {
		case AbstractColumn::ColumnMode::Double: {
			auto* vector = static_cast<QVector<double>* >(spreadsheet->child<Column>(n)->data());
			vector->reserve(m_actualRows);
			vector->resize(m_actualRows);
			m_dataContainer[n] = static_cast<void *>(vector);
			break;
		}
		case AbstractColumn::ColumnMode::Integer: {
			auto* vector = static_cast<QVector<int>* >(spreadsheet->child<Column>(n)->data());
			vector->resize(m_actualRows);
			m_dataContainer[n] = static_cast<void *>(vector);
			break;
		}
		case AbstractColumn::ColumnMode::BigInt: {
			auto* vector = static_cast<QVector<qint64>* >(spreadsheet->child<Column>(n)->data());
			vector->resize(m_actualRows);
			m_dataContainer[n] = static_cast<void *>(vector);
			break;
		}
		case AbstractColumn::ColumnMode::Text: {
			auto* vector = static_cast<QVector<QString>*>(spreadsheet->child<Column>(n)->data());
			vector->resize(m_actualRows);
			m_dataContainer[n] = static_cast<void *>(vector);
			break;
		}
		case AbstractColumn::ColumnMode::DateTime: {
			auto* vector = static_cast<QVector<QDateTime>* >(spreadsheet->child<Column>(n)->data());
			vector->resize(m_actualRows);
			m_dataContainer[n] = static_cast<void *>(vector);
			break;
		}
		//TODO
		case AbstractColumn::ColumnMode::Month:
		case AbstractColumn::ColumnMode::Day:
			break;
		}
	}
}

/*!
 * get a single line from device
 */
QStringList AsciiFilterPrivate::getLineString(QIODevice& device) {
	QString line;
	do {	// skip comment lines in data lines
		if (!device.canReadLine())
			DEBUG("WARNING in AsciiFilterPrivate::getLineString(): device cannot 'readLine()' but using it anyway.");
//			line = device.readAll();
		line = device.readLine();
	} while (!commentCharacter.isEmpty() && line.startsWith(commentCharacter));

	line.remove(QRegularExpression(QStringLiteral("[\\n\\r]")));	// remove any newline
	DEBUG("data line : \'" << STDSTRING(line) << '\'');
	QStringList lineStringList = line.split(m_separator, (QString::SplitBehavior)skipEmptyParts);
	//TODO: remove quotes here?
	if (simplifyWhitespacesEnabled) {
		for (int i = 0; i < lineStringList.size(); ++i)
			lineStringList[i] = lineStringList[i].simplified();
	}
	QDEBUG("data line, parsed: " << lineStringList);

	return lineStringList;
}

/*!
    writes the content of \c dataSource to the file \c fileName.
*/
void AsciiFilterPrivate::write(const QString& /*fileName*/, AbstractDataSource* /*dataSource*/) {
	//TODO: save data to ascii file
}

/*!
 * create datetime from \c string using \c format considering corner cases
 */
QDateTime AsciiFilterPrivate::parseDateTime(const QString& string, const QString& format) {
	//DEBUG("string = " << STDSTRING(string) << ", format = " << STDSTRING(format))
	QString fixedString(string);
	QString fixedFormat(format);
	if (!format.contains("yy")) {	// no year given: set temporary to 2000 (must be a leap year to parse "Feb 29")
		fixedString.append(" 2000");
		fixedFormat.append(" yyyy");
	}

	QDateTime dateTime = QDateTime::fromString(fixedString, fixedFormat);
	//QDEBUG("fromString() =" << dateTime)
	// interpret 2-digit year smaller than 50 as 20XX
	if (dateTime.date().year() < 1950 && !format.contains("yyyy"))
		dateTime = dateTime.addYears(100);
	//QDEBUG("dateTime fixed =" << dateTime)
	//DEBUG("dateTime.toString =" << STDSTRING(dateTime.toString(format)))

	return dateTime;
}


//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################
/*!
  Saves as XML.
 */
void AsciiFilter::save(QXmlStreamWriter* writer) const {
	writer->writeStartElement( "asciiFilter");
	writer->writeAttribute( "commentCharacter", d->commentCharacter);
	writer->writeAttribute( "separatingCharacter", d->separatingCharacter);
	writer->writeAttribute( "autoMode", QString::number(d->autoModeEnabled));
	writer->writeAttribute( "createIndex", QString::number(d->createIndexEnabled));
	writer->writeAttribute( "createTimestamp", QString::number(d->createTimestampEnabled));
	writer->writeAttribute( "header", QString::number(d->headerEnabled));
	writer->writeAttribute( "vectorNames", d->vectorNames.join(' '));
	writer->writeAttribute( "skipEmptyParts", QString::number(d->skipEmptyParts));
	writer->writeAttribute( "simplifyWhitespaces", QString::number(d->simplifyWhitespacesEnabled));
	writer->writeAttribute( "nanValue", QString::number(d->nanValue));
	writer->writeAttribute( "removeQuotes", QString::number(d->removeQuotesEnabled));
	writer->writeAttribute( "startRow", QString::number(d->startRow));
	writer->writeAttribute( "endRow", QString::number(d->endRow));
	writer->writeAttribute( "startColumn", QString::number(d->startColumn));
	writer->writeAttribute( "endColumn", QString::number(d->endColumn));
	writer->writeEndElement();
}

/*!
  Loads from XML.
*/
bool AsciiFilter::load(XmlStreamReader* reader) {
	KLocalizedString attributeWarning = ki18n("Attribute '%1' missing or empty, default value is used");
	QXmlStreamAttributes attribs = reader->attributes();
	QString str;

	READ_STRING_VALUE("commentCharacter", commentCharacter);
	READ_STRING_VALUE("separatingCharacter", separatingCharacter);

	READ_INT_VALUE("createIndex", createIndexEnabled, bool);
	READ_INT_VALUE("createTimestamp", createTimestampEnabled, bool);
	READ_INT_VALUE("autoMode", autoModeEnabled, bool);
	READ_INT_VALUE("header", headerEnabled, bool);

	str = attribs.value("vectorNames").toString();
	d->vectorNames = str.split(' '); //may be empty

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

//##############################################################################
//########################## MQTT releated code  ###############################
//##############################################################################
#ifdef HAVE_MQTT
int AsciiFilterPrivate::prepareToRead(const QString& message) {
	QStringList lines = message.split('\n');
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
	if (separatingCharacter == "auto") {
		DEBUG("automatic separator");
		const QRegularExpression regExp(QStringLiteral("[,;:]?\\s+"));
		firstLineStringList = firstLine.split(regExp, (QString::SplitBehavior)skipEmptyParts);
	} else {	// use given separator
		// replace symbolic "TAB" with '\t'
		m_separator = separatingCharacter.replace(QLatin1String("2xTAB"), "\t\t", Qt::CaseInsensitive);
		m_separator = separatingCharacter.replace(QLatin1String("TAB"), "\t", Qt::CaseInsensitive);
		// replace symbolic "SPACE" with ' '
		m_separator = m_separator.replace(QLatin1String("2xSPACE"), QLatin1String("  "), Qt::CaseInsensitive);
		m_separator = m_separator.replace(QLatin1String("3xSPACE"), QLatin1String("   "), Qt::CaseInsensitive);
		m_separator = m_separator.replace(QLatin1String("4xSPACE"), QLatin1String("    "), Qt::CaseInsensitive);
		m_separator = m_separator.replace(QLatin1String("SPACE"), QLatin1String(" "), Qt::CaseInsensitive);
		firstLineStringList = firstLine.split(m_separator, (QString::SplitBehavior)skipEmptyParts);
	}
	DEBUG("separator: \'" << STDSTRING(m_separator) << '\'');
	DEBUG("number of columns: " << firstLineStringList.size());
	QDEBUG("first line: " << firstLineStringList);

	//all columns are read plus the optional column for the index and for the timestamp
	m_actualCols = firstLineStringList.size() + int(createIndexEnabled) + int(createTimestampEnabled);

	//column names:
	//when reading the message strings for different topics, it's not possible to specify vector names
	//since the different topics can have different content and different number of columns/vectors
	//->we always set the vector names here to fixed values
	vectorNames.clear();
	columnModes.clear();

	//add index column
	if (createIndexEnabled) {
		vectorNames << i18n("Index");
		columnModes << AbstractColumn::ColumnMode::Integer;
	}

	//add timestamp column
	if (createTimestampEnabled) {
		vectorNames << i18n("Timestamp");
		columnModes << AbstractColumn::ColumnMode::DateTime;
	}

	//parse the first data line to determine data type for each column
	int i = 1;
	for (auto& valueString : firstLineStringList) {
		if (simplifyWhitespacesEnabled)
			valueString = valueString.simplified();
		if (removeQuotesEnabled)
			valueString.remove(QLatin1Char('"'));

		vectorNames << i18n("Value %1", i);
		columnModes << AbstractFileFilter::columnMode(valueString, dateTimeFormat, numberFormat);
		++i;
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

	//number formatting
	DEBUG("locale = " << STDSTRING(QLocale::languageToString(numberFormat)));

	// Read the data
	QStringList lines = message.split('\n');

	//loop over all lines in the message and parse the available columns
	int i = 0;
	for (auto line : lines) {
		if (simplifyWhitespacesEnabled)
			line = line.simplified();

		if (line.isEmpty() || (!commentCharacter.isEmpty() && line.startsWith(commentCharacter))) // skip empty or commented lines
			continue;

		const QStringList& lineStringList = line.split(m_separator, (QString::SplitBehavior)skipEmptyParts);
		QDEBUG(" line = " << lineStringList);

		QStringList lineString;

		// index column if required
		if (createIndexEnabled)
			lineString += QString::number(i + 1);

		// timestamp column if required
		if (createTimestampEnabled)
			lineString += QDateTime::currentDateTime().toString();

		int offset = int(createIndexEnabled) + int(createTimestampEnabled);

		//parse columns
		for (int n = 0; n < m_actualCols - offset; ++n) {
			if (n < lineStringList.size()) {
				QString valueString = lineStringList.at(n);
				if (removeQuotesEnabled)
					valueString.remove(QLatin1Char('"'));
				if (skipEmptyParts && !QString::compare(valueString, " "))	// handle left white spaces
					continue;

				lineString += previewValue(valueString, columnModes[n+offset]);
			} else 	// missing columns in this line
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
	//If the message is empty, there is nothing to do
	if (message.isEmpty()) {
		DEBUG("No new data available");
		return;
	}

	MQTTTopic* spreadsheet = dynamic_cast<MQTTTopic*>(dataSource);
	if (!spreadsheet)
		return;

	const int keepNValues = spreadsheet->mqttClient()->keepNValues();

	if (!m_prepared) {
		DEBUG("Start preparing filter for: " << STDSTRING(spreadsheet->topicName()));

		//Prepare the filter
		const int mqttPrepareError = prepareToRead(message);
		if (mqttPrepareError != 0) {
			DEBUG("Mqtt Prepare Error = " << mqttPrepareError);
			return;
		}

		// prepare import for spreadsheet
		spreadsheet->setUndoAware(false);
		spreadsheet->resize(AbstractFileFilter::ImportMode::Replace, vectorNames, m_actualCols);

		//columns in a MQTTTopic don't have any manual changes.
		//make the available columns undo unaware and suppress the "data changed" signal.
		//data changes will be propagated via an explicit Column::setChanged() call once new data was read.
		for (int i = 0; i < spreadsheet->childCount<Column>(); i++) {
			spreadsheet->child<Column>(i)->setUndoAware(false);
			spreadsheet->child<Column>(i)->setSuppressDataChangedSignal(true);
		}

		if (keepNValues == 0)
			spreadsheet->setRowCount(m_actualRows > 1 ? m_actualRows : 1);
		else {
			spreadsheet->setRowCount(spreadsheet->mqttClient()->keepNValues());
			m_actualRows = spreadsheet->mqttClient()->keepNValues();
		}

		m_dataContainer.resize(m_actualCols);
		initDataContainers(spreadsheet);
	}

	MQTTClient::ReadingType readingType;
	if (!m_prepared) {
		//if filter is not prepared we read till the end
		readingType = MQTTClient::ReadingType::TillEnd;
	} else {
		//we have to read all the data when reading from end
		//so we set readingType to TillEnd
		if (spreadsheet->mqttClient()->readingType() == MQTTClient::ReadingType::FromEnd)
			readingType = MQTTClient::ReadingType::TillEnd;
		else
			readingType = spreadsheet->mqttClient()->readingType();
	}

	//count the new lines, increase actualrows on each
	//now we read all the new lines, if we want to use sample rate
	//then here we can do it, if we have actually sample rate number of lines :-?
	int newLinesForSampleSizeNotTillEnd = 0;
	int newLinesTillEnd = 0;
	QVector<QString> newData;
	if (readingType != MQTTClient::ReadingType::TillEnd) {
		newData.reserve(spreadsheet->mqttClient()->sampleSize());
		newData.resize(spreadsheet->mqttClient()->sampleSize());
	}

	//TODO: bool sampleSizeReached = false;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
	const QStringList newDataList = message.split(QRegularExpression(QStringLiteral("\n|\r\n|\r")), Qt::SkipEmptyParts);
#else
	const QStringList newDataList = message.split(QRegularExpression(QStringLiteral("\n|\r\n|\r")), QString::SkipEmptyParts);
#endif
	for (auto& line : newDataList) {
		newData.push_back(line);
		newLinesTillEnd++;

		if (readingType != MQTTClient::ReadingType::TillEnd) {
			newLinesForSampleSizeNotTillEnd++;
			//for Continuous reading and FromEnd we read sample rate number of lines if possible
			if (newLinesForSampleSizeNotTillEnd == spreadsheet->mqttClient()->sampleSize()) {
				//TODO: sampleSizeReached = true;
				break;
			}
		}
	}

	qDebug()<<"Processing message done";
	//now we reset the readingType
	if (spreadsheet->mqttClient()->readingType() == MQTTClient::ReadingType::FromEnd)
		readingType = static_cast<MQTTClient::ReadingType>(spreadsheet->mqttClient()->readingType());

	//we had less new lines than the sample rate specified
	if (readingType != MQTTClient::ReadingType::TillEnd)
		qDebug() << "Removed empty lines: " << newData.removeAll(QString());

	const int spreadsheetRowCountBeforeResize = spreadsheet->rowCount();

	if (m_prepared ) {
		if (keepNValues == 0)
			m_actualRows = spreadsheetRowCountBeforeResize;
		else {
			//if the keepNValues changed since the last read we have to manage the columns accordingly
			if (m_actualRows != spreadsheet->mqttClient()->keepNValues()) {
				if (m_actualRows < spreadsheet->mqttClient()->keepNValues()) {
					spreadsheet->setRowCount(spreadsheet->mqttClient()->keepNValues());
					qDebug()<<"rowcount set to: " << spreadsheet->mqttClient()->keepNValues();
				}

				//Calculate the difference between the old and new keepNValues
				int rowDiff = 0;
				if (m_actualRows > spreadsheet->mqttClient()->keepNValues())
					rowDiff = m_actualRows -  spreadsheet->mqttClient()->keepNValues();

				if (m_actualRows < spreadsheet->mqttClient()->keepNValues())
					rowDiff = spreadsheet->mqttClient()->keepNValues() - m_actualRows;

				for (int n = 0; n < columnModes.size(); ++n) {
					// data() returns a void* which is a pointer to any data type (see ColumnPrivate.cpp)
					switch (columnModes[n]) {
					case AbstractColumn::ColumnMode::Double: {
						QVector<double>*  vector = static_cast<QVector<double>* >(spreadsheet->child<Column>(n)->data());
						m_dataContainer[n] = static_cast<void *>(vector);

						//if the keepNValues got smaller then we move the last keepNValues count of data
						//in the first keepNValues places
						if (m_actualRows > spreadsheet->mqttClient()->keepNValues()) {
							for (int i = 0; i < spreadsheet->mqttClient()->keepNValues(); i++) {
								static_cast<QVector<double>*>(m_dataContainer[n])->operator[] (i) =
								    static_cast<QVector<double>*>(m_dataContainer[n])->operator[](m_actualRows - spreadsheet->mqttClient()->keepNValues() + i);
							}
						}

						//if the keepNValues got bigger we move the existing values to the last m_actualRows positions
						//then fill the remaining lines with NaN
						if (m_actualRows < spreadsheet->mqttClient()->keepNValues()) {
							vector->reserve( spreadsheet->mqttClient()->keepNValues());
							vector->resize( spreadsheet->mqttClient()->keepNValues());

							for (int i = 1; i <= m_actualRows; i++) {
								static_cast<QVector<double>*>(m_dataContainer[n])->operator[] (spreadsheet->mqttClient()->keepNValues() - i) =
								    static_cast<QVector<double>*>(m_dataContainer[n])->operator[](spreadsheet->mqttClient()->keepNValues() - i - rowDiff);
							}
							for (int i = 0; i < rowDiff; i++)
								static_cast<QVector<double>*>(m_dataContainer[n])->operator[](i) = nanValue;
						}
						break;
					}
					case AbstractColumn::ColumnMode::Integer: {
						QVector<int>* vector = static_cast<QVector<int>* >(spreadsheet->child<Column>(n)->data());
						m_dataContainer[n] = static_cast<void *>(vector);

						//if the keepNValues got smaller then we move the last keepNValues count of data
						//in the first keepNValues places
						if (m_actualRows > spreadsheet->mqttClient()->keepNValues()) {
							for (int i = 0; i < spreadsheet->mqttClient()->keepNValues(); i++) {
								static_cast<QVector<int>*>(m_dataContainer[n])->operator[] (i) =
								    static_cast<QVector<int>*>(m_dataContainer[n])->operator[](m_actualRows - spreadsheet->mqttClient()->keepNValues() + i);
							}
						}

						//if the keepNValues got bigger we move the existing values to the last m_actualRows positions
						//then fill the remaining lines with 0
						if (m_actualRows < spreadsheet->mqttClient()->keepNValues()) {
							vector->reserve( spreadsheet->mqttClient()->keepNValues());
							vector->resize( spreadsheet->mqttClient()->keepNValues());
							for (int i = 1; i <= m_actualRows; i++) {
								static_cast<QVector<int>*>(m_dataContainer[n])->operator[] (spreadsheet->mqttClient()->keepNValues() - i) =
								    static_cast<QVector<int>*>(m_dataContainer[n])->operator[](spreadsheet->mqttClient()->keepNValues() - i - rowDiff);
							}
							for (int i = 0; i < rowDiff; i++)
								static_cast<QVector<int>*>(m_dataContainer[n])->operator[](i) = 0;
						}
						break;
					}
					case AbstractColumn::ColumnMode::BigInt: {
						QVector<qint64>* vector = static_cast<QVector<qint64>* >(spreadsheet->child<Column>(n)->data());
						m_dataContainer[n] = static_cast<void *>(vector);

						//if the keepNValues got smaller then we move the last keepNValues count of data
						//in the first keepNValues places
						if (m_actualRows > spreadsheet->mqttClient()->keepNValues()) {
							for (int i = 0; i < spreadsheet->mqttClient()->keepNValues(); i++) {
								static_cast<QVector<qint64>*>(m_dataContainer[n])->operator[] (i) =
								    static_cast<QVector<qint64>*>(m_dataContainer[n])->operator[](m_actualRows - spreadsheet->mqttClient()->keepNValues() + i);
							}
						}

						//if the keepNValues got bigger we move the existing values to the last m_actualRows positions
						//then fill the remaining lines with 0
						if (m_actualRows < spreadsheet->mqttClient()->keepNValues()) {
							vector->reserve( spreadsheet->mqttClient()->keepNValues());
							vector->resize( spreadsheet->mqttClient()->keepNValues());
							for (int i = 1; i <= m_actualRows; i++) {
								static_cast<QVector<qint64>*>(m_dataContainer[n])->operator[] (spreadsheet->mqttClient()->keepNValues() - i) =
								    static_cast<QVector<qint64>*>(m_dataContainer[n])->operator[](spreadsheet->mqttClient()->keepNValues() - i - rowDiff);
							}
							for (int i = 0; i < rowDiff; i++)
								static_cast<QVector<qint64>*>(m_dataContainer[n])->operator[](i) = 0;
						}
						break;
					}
					case AbstractColumn::ColumnMode::Text: {
						QVector<QString>* vector = static_cast<QVector<QString>*>(spreadsheet->child<Column>(n)->data());
						m_dataContainer[n] = static_cast<void *>(vector);

						//if the keepNValues got smaller then we move the last keepNValues count of data
						//in the first keepNValues places
						if (m_actualRows > spreadsheet->mqttClient()->keepNValues()) {
							for (int i = 0; i < spreadsheet->mqttClient()->keepNValues(); i++) {
								static_cast<QVector<QString>*>(m_dataContainer[n])->operator[] (i) =
								    static_cast<QVector<QString>*>(m_dataContainer[n])->operator[](m_actualRows - spreadsheet->mqttClient()->keepNValues() + i);
							}
						}

						//if the keepNValues got bigger we move the existing values to the last m_actualRows positions
						//then fill the remaining lines with empty lines
						if (m_actualRows < spreadsheet->mqttClient()->keepNValues()) {
							vector->reserve( spreadsheet->mqttClient()->keepNValues());
							vector->resize( spreadsheet->mqttClient()->keepNValues());
							for (int i = 1; i <= m_actualRows; i++) {
								static_cast<QVector<QString>*>(m_dataContainer[n])->operator[] (spreadsheet->mqttClient()->keepNValues() - i) =
								    static_cast<QVector<QString>*>(m_dataContainer[n])->operator[](spreadsheet->mqttClient()->keepNValues() - i - rowDiff);
							}
							for (int i = 0; i < rowDiff; i++)
								static_cast<QVector<QString>*>(m_dataContainer[n])->operator[](i).clear();
						}
						break;
					}
					case AbstractColumn::ColumnMode::DateTime: {
						QVector<QDateTime>* vector = static_cast<QVector<QDateTime>* >(spreadsheet->child<Column>(n)->data());
						m_dataContainer[n] = static_cast<void *>(vector);

						//if the keepNValues got smaller then we move the last keepNValues count of data
						//in the first keepNValues places
						if (m_actualRows > spreadsheet->mqttClient()->keepNValues()) {
							for (int i = 0; i < spreadsheet->mqttClient()->keepNValues(); i++) {
								static_cast<QVector<QDateTime>*>(m_dataContainer[n])->operator[] (i) =
								    static_cast<QVector<QDateTime>*>(m_dataContainer[n])->operator[](m_actualRows - spreadsheet->mqttClient()->keepNValues() + i);
							}
						}

						//if the keepNValues got bigger we move the existing values to the last m_actualRows positions
						//then fill the remaining lines with null datetime
						if (m_actualRows < spreadsheet->mqttClient()->keepNValues()) {
							vector->reserve( spreadsheet->mqttClient()->keepNValues());
							vector->resize( spreadsheet->mqttClient()->keepNValues());
							for (int i = 1; i <= m_actualRows; i++) {
								static_cast<QVector<QDateTime>*>(m_dataContainer[n])->operator[] (spreadsheet->mqttClient()->keepNValues() - i) =
								    static_cast<QVector<QDateTime>*>(m_dataContainer[n])->operator[](spreadsheet->mqttClient()->keepNValues() - i - rowDiff);
							}
							for (int i = 0; i < rowDiff; i++)
								static_cast<QVector<QDateTime>*>(m_dataContainer[n])->operator[](i) = QDateTime();
						}
						break;
					}
					//TODO
					case AbstractColumn::ColumnMode::Month:
					case AbstractColumn::ColumnMode::Day:
						break;
					}
				}
				//if the keepNValues got smaller resize the spreadsheet
				if (m_actualRows > spreadsheet->mqttClient()->keepNValues())
					spreadsheet->setRowCount(spreadsheet->mqttClient()->keepNValues());

				//set the new row count
				m_actualRows = spreadsheet->mqttClient()->keepNValues();
				qDebug()<<"actual rows: "<<m_actualRows;
			}
		}
	}

	qDebug()<<"starting m_actual rows calculated: " << m_actualRows <<", new data size: "<<newData.size();

	int currentRow = 0; // indexes the position in the vector(column)
	int linesToRead = 0;

	if (m_prepared) {
		//increase row count if we don't have a fixed size
		//but only after the preparation step
		if (keepNValues == 0) {
			if (readingType != MQTTClient::ReadingType::TillEnd)
				m_actualRows += qMin(newData.size(), spreadsheet->mqttClient()->sampleSize());
			else {
				m_actualRows += newData.size();
			}
		}

		//fixed size
		if (keepNValues != 0) {
			if (readingType == MQTTClient::ReadingType::TillEnd) {
				//we had more lines than the fixed size, so we read m_actualRows number of lines
				if (newLinesTillEnd > m_actualRows) {
					linesToRead = m_actualRows;
				} else
					linesToRead = newLinesTillEnd;
			} else {
				//we read max sample size number of lines when the reading mode
				//is ContinuouslyFixed or FromEnd
				if (spreadsheet->mqttClient()->sampleSize() <= spreadsheet->mqttClient()->keepNValues())
					linesToRead = qMin(spreadsheet->mqttClient()->sampleSize(), newLinesTillEnd);
				else
					linesToRead = qMin(spreadsheet->mqttClient()->keepNValues(), newLinesTillEnd);
			}
		} else
			linesToRead = m_actualRows - spreadsheetRowCountBeforeResize;

		if (linesToRead == 0)
			return;
	} else {
		if (keepNValues != 0)
			linesToRead = newLinesTillEnd > m_actualRows ? m_actualRows : newLinesTillEnd;
		else
			linesToRead = newLinesTillEnd;
	}
	qDebug()<<"linestoread = " << linesToRead;

	//new rows/resize columns if we don't have a fixed size
	if (keepNValues == 0) {

#ifdef PERFTRACE_LIVE_IMPORT
		PERFTRACE("AsciiLiveDataImportResizing: ");
#endif
		if (spreadsheet->rowCount() < m_actualRows)
			spreadsheet->setRowCount(m_actualRows);

		if (!m_prepared)
			currentRow = 0;
		else {
			// indexes the position in the vector(column)
			currentRow = spreadsheetRowCountBeforeResize;
		}

		// if we have fixed size, we do this only once in preparation, here we can use
		// m_prepared and we need something to decide whether it has a fixed size or increasing

		initDataContainers(spreadsheet);
	} else {
		//when we have a fixed size we have to pop sampleSize number of lines if specified
		//here popping, setting currentRow
		if (!m_prepared)
			currentRow = m_actualRows - qMin(newLinesTillEnd, m_actualRows);
		else {
			if (readingType == MQTTClient::ReadingType::TillEnd) {
				if (newLinesTillEnd > m_actualRows)
					currentRow = 0;
				else
					currentRow = m_actualRows - newLinesTillEnd;
			} else {
				//we read max sample rate number of lines when the reading mode
				//is ContinuouslyFixed or FromEnd
				currentRow = m_actualRows - linesToRead;
			}
		}

		if (m_prepared) {
#ifdef PERFTRACE_LIVE_IMPORT
			PERFTRACE("AsciiLiveDataImportPopping: ");
#endif
			for (int row = 0; row < linesToRead; ++row) {
				for (int col = 0;  col < m_actualCols; ++col) {
					switch (columnModes[col]) {
					case AbstractColumn::ColumnMode::Double: {
						QVector<double>* vector = static_cast<QVector<double>* >(spreadsheet->child<Column>(col)->data());
						vector->pop_front();
						vector->reserve(m_actualRows);
						vector->resize(m_actualRows);
						m_dataContainer[col] = static_cast<void *>(vector);
						break;
					}
					case AbstractColumn::ColumnMode::Integer: {
						QVector<int>* vector = static_cast<QVector<int>* >(spreadsheet->child<Column>(col)->data());
						vector->pop_front();
						vector->reserve(m_actualRows);
						vector->resize(m_actualRows);
						m_dataContainer[col] = static_cast<void *>(vector);
						break;
					}
					case AbstractColumn::ColumnMode::BigInt: {
						QVector<qint64>* vector = static_cast<QVector<qint64>* >(spreadsheet->child<Column>(col)->data());
						vector->pop_front();
						vector->reserve(m_actualRows);
						vector->resize(m_actualRows);
						m_dataContainer[col] = static_cast<void *>(vector);
						break;
					}
					case AbstractColumn::ColumnMode::Text: {
						QVector<QString>* vector = static_cast<QVector<QString>*>(spreadsheet->child<Column>(col)->data());
						vector->pop_front();
						vector->reserve(m_actualRows);
						vector->resize(m_actualRows);
						m_dataContainer[col] = static_cast<void *>(vector);
						break;
					}
					case AbstractColumn::ColumnMode::DateTime: {
						QVector<QDateTime>* vector = static_cast<QVector<QDateTime>* >(spreadsheet->child<Column>(col)->data());
						vector->pop_front();
						vector->reserve(m_actualRows);
						vector->resize(m_actualRows);
						m_dataContainer[col] = static_cast<void *>(vector);
						break;
					}
					//TODO
					case AbstractColumn::ColumnMode::Month:
					case AbstractColumn::ColumnMode::Day:
						break;
					}
				}
			}
		}
	}

	// from the last row we read the new data in the spreadsheet
	qDebug() << "reading from line: "  << currentRow << " lines till end: " << newLinesTillEnd;
	qDebug() << "Lines to read: " << linesToRead <<" actual rows: " << m_actualRows;
	int newDataIdx = 0;
	//From end means that we read the last sample size amount of data
	if (readingType == MQTTClient::ReadingType::FromEnd) {
		if (m_prepared) {
			if (newData.size() > spreadsheet->mqttClient()->sampleSize())
				newDataIdx = newData.size() - spreadsheet->mqttClient()->sampleSize();
		}
	}

	qDebug() << "newDataIdx: " << newDataIdx;

	//read the data
	static int indexColumnIdx = 0;
	{
#ifdef PERFTRACE_LIVE_IMPORT
		PERFTRACE("AsciiLiveDataImportFillingContainers: ");
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

			QStringList lineStringList = line.split(m_separator, (QString::SplitBehavior)skipEmptyParts);

			if (simplifyWhitespacesEnabled) {
				for (int i = 0; i < lineStringList.size(); ++i)
					lineStringList[i] = lineStringList[i].simplified();
			}

			//add index if required
			int offset = 0;
			if (createIndexEnabled) {
				int index = (keepNValues == 0) ? currentRow + 1 : indexColumnIdx++;
				static_cast<QVector<int>*>(m_dataContainer[0])->operator[](currentRow) = index;
				++offset;
			}

			//add current timestamp if required
			if (createTimestampEnabled) {
				static_cast<QVector<QDateTime>*>(m_dataContainer[offset])->operator[](currentRow) = QDateTime::currentDateTime();
				++offset;
			}

			//parse the columns
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

	if (m_prepared) {
		//notify all affected columns and plots about the changes
		PERFTRACE("AsciiLiveDataImport, notify affected columns and plots");

		const Project* project = spreadsheet->project();
		QVector<const XYCurve*> curves = project->children<const XYCurve>(AbstractAspect::ChildIndexFlag::Recursive);
		QVector<CartesianPlot*> plots;

		for (int n = 0; n < m_actualCols; ++n) {
			Column* column = spreadsheet->column(n);

			//determine the plots where the column is consumed
			for (const auto* curve : curves) {
				if (curve->xColumn() == column || curve->yColumn() == column) {
					CartesianPlot* plot = static_cast<CartesianPlot*>(curve->parentAspect());
					if (plots.indexOf(plot) == -1) {
						plots << plot;
						plot->setSuppressRetransform(true);
					}
				}
			}

			column->setChanged();
		}

		//loop over all affected plots and retransform them
		for (auto* const plot : plots) {
			//TODO setting this back to true triggers again a lot of retransforms in the plot (one for each curve).
			// 				plot->setSuppressDataChangedSignal(false);
			plot->dataChanged(-1, -1); // TODO: check if all ranges must be updated!
		}
	} else
		m_prepared = true;

	DEBUG("AsciiFilterPrivate::readFromMQTTTopic() DONE");
}

/*!
 * \brief After the MQTTTopic was loaded, the filter is prepared for reading
 * \param prepared
 * \param topic
 * \param separator
 */
void AsciiFilterPrivate::setPreparedForMQTT(bool prepared, MQTTTopic* topic, const QString& separator) {
	m_prepared = prepared;
	//If originally it was prepared we have to restore the settings
	if (prepared) {
		m_separator = separator;
		m_actualCols = endColumn - startColumn + 1;
		m_actualRows = topic->rowCount();
		//set the column modes
		columnModes.resize(topic->columnCount());
		for (int i = 0; i < topic->columnCount(); ++i)
			columnModes[i] = topic->column(i)->columnMode();

		//set the data containers
		m_dataContainer.resize(m_actualCols);
		initDataContainers(topic);
	}
}
#endif
