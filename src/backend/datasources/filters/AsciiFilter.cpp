/***************************************************************************
File                 : AsciiFilter.cpp
Project              : LabPlot
Description          : ASCII I/O-filter
--------------------------------------------------------------------
Copyright            : (C) 2009-2018 Stefan Gerlach (stefan.gerlach@uni.kn)
Copyright            : (C) 2009-2017 Alexander Semke (alexander.semke@web.de)

***************************************************************************/

/***************************************************************************
*                                                                         *
*  This program is free software; you can redistribute it and/or modify   *
*  it under the terms of the GNU General Public License as published by   *
*  the Free Software Foundation; either version 2 of the License, or      *
*  (at your option) any later version.                                    *
*                                                                         *
*  This program is distributed in the hope that it will be useful,        *
*  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
*  GNU General Public License for more details.                           *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the Free Software           *
*   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
*   Boston, MA  02110-1301  USA                                           *
*                                                                         *
***************************************************************************/
#include "backend/datasources/LiveDataSource.h"
#include "backend/core/column/Column.h"
#include "backend/core/Project.h"
#include "backend/datasources/filters/AsciiFilter.h"
#include "backend/datasources/filters/AsciiFilterPrivate.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/worksheet/plots/cartesian/XYCurve.h"
#include "backend/lib/macros.h"
#include "backend/lib/trace.h"

#ifdef HAVE_MQTT
#include "backend/datasources/MQTTClient.h"
#include "backend/datasources/MQTTTopic.h"
#endif

#include <QTextStream>
#include <KLocalizedString>
#include <KFilterDev>
#include <QProcess>
#include <QDateTime>

/*!
\class AsciiFilter
\brief Manages the import/export of data organized as columns (vectors) from/to an ASCII-file.

\ingroup datasources
*/
AsciiFilter::AsciiFilter() : AbstractFileFilter(), d(new AsciiFilterPrivate(this)) {}

AsciiFilter::~AsciiFilter() {}

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

/*!
  reads the content of the file \c fileName.
*/
QVector<QStringList> AsciiFilter::readDataFromFile(const QString& fileName, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode importMode, int lines) {
	d->readDataFromFile(fileName, dataSource, importMode, lines);
	return QVector<QStringList>();  //TODO: remove this later once all read*-functions in the filter classes don't return any preview strings anymore
}

QVector<QStringList> AsciiFilter::preview(const QString& fileName, int lines) {
	return d->preview(fileName, lines);
}

QVector<QStringList> AsciiFilter::preview(QIODevice &device) {
	return d->preview(device);
}

/*!
  reads the content of the file \c fileName to the data source \c dataSource.
*/
//void AsciiFilter::read(const QString& fileName, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode importMode) {
//	d->read(fileName, dataSource, importMode);
//}


/*!
writes the content of the data source \c dataSource to the file \c fileName.
*/
void AsciiFilter::write(const QString& fileName, AbstractDataSource* dataSource) {
	d->write(fileName, dataSource);
// 	emit()
}

/*!
  loads the predefined filter settings for \c filterName
*/
void AsciiFilter::loadFilterSettings(const QString& filterName) {
	Q_UNUSED(filterName);
}

/*!
  saves the current settings as a new filter with the name \c filterName
*/
void AsciiFilter::saveFilterSettings(const QString& filterName) const {
	Q_UNUSED(filterName);
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
		DEBUG("Could not open file " << fileName.toStdString() << " for determining number of columns");
		return -1;
	}

	QString line = device.readLine();
	line.remove(QRegExp("[\\n\\r]"));

	QStringList lineStringList;
	if (separator.length() > 0)
		lineStringList = line.split(separator);
	else
		lineStringList = line.split(QRegExp("\\s+"));
	DEBUG("number of columns : " << lineStringList.size());

	return lineStringList.size();
}

size_t AsciiFilter::lineNumber(const QString& fileName) {
	KFilterDev device(fileName);

	if (!device.open(QIODevice::ReadOnly)) {
		DEBUG("Could not open file " << fileName.toStdString() << " to determine number of lines");

		return 0;
	}
	if (!device.canReadLine())
		return -1;

	size_t lineCount = 0;
	while (!device.atEnd()) {
		device.readLine();
		lineCount++;
	}

//TODO: wc is much faster but not portable
	/*	QElapsedTimer myTimer;
		myTimer.start();
		QProcess wc;
		wc.start(QString("wc"), QStringList() << "-l" << fileName);
		size_t lineCount = 0;
		while (wc.waitForReadyRead())
			lineCount = wc.readLine().split(' ')[0].toInt();
		lineCount++;	// last line not counted
		DEBUG(" Elapsed time counting lines : " << myTimer.elapsed() << " ms");
	*/
	return lineCount;
}

/*!
  returns the number of lines in the device \c device and 0 if sequential.
  resets the position to 0!
*/
size_t AsciiFilter::lineNumber(QIODevice &device) {
	if (device.isSequential())
		return 0;
	if (!device.canReadLine())
		DEBUG("WARNING in AsciiFilter::lineNumber(): device cannot 'readLine()' but using it anyway.");

	size_t lineCount = 0;
	device.seek(0);
	while (!device.atEnd()) {
		device.readLine();
		lineCount++;
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

bool AsciiFilter::createIndexEnabled() const{
	return d->createIndexEnabled;
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
		d->nanValue = NAN;
}
bool AsciiFilter::NaNValueToZeroEnabled() const {
	if (d->nanValue == 0)
		return true;
	return false;
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
AsciiFilterPrivate::AsciiFilterPrivate(AsciiFilter* owner) : q(owner),
	commentCharacter("#"),
	separatingCharacter("auto"),
	numberFormat(QLocale::C),
	autoModeEnabled(true),
	headerEnabled(true),
	skipEmptyParts(false),
	simplifyWhitespacesEnabled(true),
	nanValue(NAN),
	removeQuotesEnabled(false),
	createIndexEnabled(false),
	startRow(1),
	endRow(-1),
	startColumn(1),
	endColumn(-1),
	m_actualStartRow(1),
	m_actualRows(0),
	m_maxActualRows (0),
	m_actualCols(0),
	m_prepared(false),
	m_lastRowNum (0),
	mqttPreviewFirstEmptyColCount (0),
	m_columnOffset(0) {
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
	} while (line.startsWith(commentCharacter));

	line.remove(QRegExp("[\\n\\r]"));	// remove any newline
	if (simplifyWhitespacesEnabled)
		line = line.simplified();
	DEBUG("data line : \'" << line.toStdString() << '\'');
	QStringList lineStringList = line.split(m_separator, (QString::SplitBehavior)skipEmptyParts);
	//TODO: remove quotes here?
	QDEBUG("data line, parsed: " << lineStringList);

	return lineStringList;
}

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
	// Find first data line (ignoring comment lines)
	DEBUG("	Skipping " << startRow - 1 << " lines");
	for (int i = 0; i < startRow - 1; ++i) {
		QString line;
		if (!device.canReadLine())
			DEBUG("WARNING in AsciiFilterPrivate::prepareDeviceToRead(): device cannot 'readLine()' but using it anyway.");
		line = device.readLine();
		DEBUG("	line = " << line.toStdString());

		if (device.atEnd()) {
			if (device.isSequential())
				break;
			else
				return 1;
		}

		//TOOD: this logic seems to be wrong. If the user asks to read from line startRow, we should start here independent of any comments
		if (line.startsWith(commentCharacter))	// ignore commented lines before startRow
			i--;
	}

	// Parse the first line:
	// Determine the number of columns, create the columns and use (if selected) the first row to name them
	QString firstLine;
	do {	// skip comment lines
		if (!device.canReadLine())
			DEBUG("WARNING in AsciiFilterPrivate::prepareDeviceToRead(): device cannot 'readLine()' but using it anyway.");

		firstLine = device.readLine();

		if (device.atEnd()) {
			if (device.isSequential())
				break;
			else
				return 1;
		}
	} while (firstLine.startsWith(commentCharacter));

	DEBUG(" device position after first line and comments = " << device.pos());
	firstLine.remove(QRegExp("[\\n\\r]"));	// remove any newline
	if (simplifyWhitespacesEnabled)
		firstLine = firstLine.simplified();
	DEBUG("First line: \'" << firstLine.toStdString() << '\'');

	// determine separator and split first line
	QStringList firstLineStringList;
	if (separatingCharacter == "auto") {
		DEBUG("automatic separator");
		QRegExp regExp("(\\s+)|(,\\s+)|(;\\s+)|(:\\s+)");
		firstLineStringList = firstLine.split(regExp, (QString::SplitBehavior)skipEmptyParts);

		if (!firstLineStringList.isEmpty()) {
			int length1 = firstLineStringList.at(0).length();
			if (firstLineStringList.size() > 1) {
				int pos2 = firstLine.indexOf(firstLineStringList.at(1), length1);
				m_separator = firstLine.mid(length1, pos2 - length1);
			} else {
				//old: separator = line.right(line.length() - length1);
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
	DEBUG("separator: \'" << m_separator.toStdString() << '\'');
	DEBUG("number of columns: " << firstLineStringList.size());
	QDEBUG("first line: " << firstLineStringList);
	DEBUG("headerEnabled = " << headerEnabled);

	//optionally, remove potential spaces in the first line
	if (simplifyWhitespacesEnabled) {
		for (int i = 0; i < firstLineStringList.size(); ++i)
			firstLineStringList[i] = firstLineStringList[i].simplified();
	}

	if (headerEnabled) {	// use first line to name vectors
		vectorNames = firstLineStringList;
		QDEBUG("vector names =" << vectorNames);
		m_actualStartRow = startRow + 1;
	} else
		m_actualStartRow = startRow;

	// set range to read
	if (endColumn == -1) {
		if (headerEnabled || vectorNames.size() == 0)
			endColumn = firstLineStringList.size(); // last column
		else
			//number of vector names provided in the import dialog (not more than the maximal number of columns in the file)
			endColumn = qMin(vectorNames.size(), firstLineStringList.size());
	}
	if (createIndexEnabled) {
		vectorNames.prepend(i18n("Index"));
		endColumn++;
	}
	m_actualCols = endColumn - startColumn + 1;

//TEST: readline-seek-readline fails
	/*	qint64 testpos = device.pos();
		DEBUG("read data line @ pos " << testpos << " : " << device.readLine().toStdString());
		device.seek(testpos);
		testpos = device.pos();
		DEBUG("read data line again @ pos " << testpos << "  : " << device.readLine().toStdString());
	*/
/////////////////////////////////////////////////////////////////

	// parse first data line to determine data type for each column
	if (!device.isSequential())
		firstLineStringList = getLineString(device);

	columnModes.resize(m_actualCols);
	int col = 0;
	if (createIndexEnabled) {
		columnModes[0] = AbstractColumn::Integer;
		col = 1;
	}

	for (auto& valueString: firstLineStringList) { // parse columns available in first data line
		if (simplifyWhitespacesEnabled)
			valueString = valueString.simplified();
		if (col == m_actualCols)
			break;
		columnModes[col++] = AbstractFileFilter::columnMode(valueString, dateTimeFormat, numberFormat);
	}
	// parsing more lines to better determine data types
	for (unsigned int i = 0; i < m_dataTypeLines; ++i) {
		firstLineStringList = getLineString(device);

		if (createIndexEnabled)
			col = 1;
		else
			col = 0;
		for (auto& valueString: firstLineStringList) {
			if (simplifyWhitespacesEnabled)
				valueString = valueString.simplified();
			if (col == m_actualCols)
				break;
			AbstractColumn::ColumnMode mode = AbstractFileFilter::columnMode(valueString, dateTimeFormat, numberFormat);

			// numeric: integer -> numeric
			if (mode == AbstractColumn::Numeric && columnModes[col] == AbstractColumn::Integer)
				columnModes[col] = mode;
			// text: non text -> text
			if (mode == AbstractColumn::Text && columnModes[col] != AbstractColumn::Text)
				columnModes[col] = mode;

			col++;
		}
	}
	QDEBUG("column modes = " << columnModes);

	// ATTENTION: This resets the position in the device to 0
	m_actualRows = (int)AsciiFilter::lineNumber(device);

	// reset to start of file
	//TODO: seems to be redundant since it's already done in the lineNumber() call above
	if (!device.isSequential())
		device.seek(0);
/////////////////////////////////////////////////////////////////

	int actualEndRow = endRow;
	DEBUG("endRow(actualEndRow) = " << endRow << ", m_actualRows = " << m_actualRows);
	if (endRow == -1 || endRow > m_actualRows)
		actualEndRow = m_actualRows;

	if (m_actualRows > actualEndRow)
		m_actualRows = actualEndRow;

	DEBUG("start/end column: " << startColumn << ' ' << endColumn);
	DEBUG("start/end row: " << m_actualStartRow << ' ' << actualEndRow);
	DEBUG("actual cols/rows (w/o header incl. start rows): " << m_actualCols << ' ' << m_actualRows);

	if (m_actualRows == 0 && !device.isSequential())
		return 1;

	return 0;
}

/*!
    reads the content of the file \c fileName to the data source \c dataSource. Uses the settings defined in the data source.
*/
void AsciiFilterPrivate::readDataFromFile(const QString& fileName, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode importMode, int lines) {
	DEBUG("AsciiFilterPrivate::readDataFromFile(): fileName = \'" << fileName.toStdString() << "\', dataSource = "
	      << dataSource << ", mode = " << ENUM_TO_STRING(AbstractFileFilter, ImportMode, importMode) << ", lines = " << lines);

	KFilterDev device(fileName);
	readDataFromDevice(device, dataSource, importMode, lines);
}

qint64 AsciiFilterPrivate::readFromLiveDevice(QIODevice& device, AbstractDataSource* dataSource, qint64 from) {
	DEBUG("AsciiFilterPrivate::readFromLiveDevice(): bytes available = " << device.bytesAvailable() << ", from = " << from);
	if (!(device.bytesAvailable() > 0)) {
		DEBUG("	No new data available");
		return 0;
	}

	LiveDataSource* spreadsheet = dynamic_cast<LiveDataSource*>(dataSource);

	if (spreadsheet->sourceType() != LiveDataSource::SourceType::FileOrPipe)
		if (device.isSequential() && device.bytesAvailable() < (int)sizeof(quint16))
			return 0;

	if (!m_prepared) {
		DEBUG("Preparing ..");

		switch (spreadsheet->sourceType()) {
		case LiveDataSource::SourceType::FileOrPipe: {
			const int deviceError = prepareDeviceToRead(device);
			if (deviceError != 0) {
				DEBUG("Device error = " << deviceError);
				return 0;
			}
			break;
		}
		case LiveDataSource::SourceType::NetworkTcpSocket:
		case LiveDataSource::SourceType::NetworkUdpSocket:
		case LiveDataSource::SourceType::LocalSocket:
		case LiveDataSource::SourceType::SerialPort:
			m_actualRows = 1;
			if (createIndexEnabled) {
				m_actualCols = 2;
				columnModes << AbstractColumn::Integer << AbstractColumn::Numeric;
				vectorNames << i18n("Index") << i18n("Value");
			} else {
				m_actualCols = 1;
				columnModes << AbstractColumn::Numeric;
				vectorNames << i18n("Value");
			}
			QDEBUG("	vector names = " << vectorNames);
		}

		// prepare import for spreadsheet
		spreadsheet->setUndoAware(false);
		spreadsheet->resize(AbstractFileFilter::Replace, vectorNames, m_actualCols);
		DEBUG("	data source resized to col: " << m_actualCols);
		DEBUG("	data source rowCount: " << spreadsheet->rowCount());

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

		DEBUG("	Setting data ..");
		for (int n = 0; n < m_actualCols; ++n) {
			// data() returns a void* which is a pointer to any data type (see ColumnPrivate.cpp)
			spreadsheet->child<Column>(n)->setColumnMode(columnModes[n]);
			switch (columnModes[n]) {
			case AbstractColumn::Numeric: {
				QVector<double>* vector = static_cast<QVector<double>* >(spreadsheet->child<Column>(n)->data());
				vector->resize(m_actualRows);
				m_dataContainer[n] = static_cast<void *>(vector);
				break;
			}
			case AbstractColumn::Integer: {
				QVector<int>* vector = static_cast<QVector<int>* >(spreadsheet->child<Column>(n)->data());
				vector->resize(m_actualRows);
				m_dataContainer[n] = static_cast<void *>(vector);
				break;
			}
			case AbstractColumn::Text: {
				QVector<QString>* vector = static_cast<QVector<QString>*>(spreadsheet->child<Column>(n)->data());
				vector->resize(m_actualRows);
				m_dataContainer[n] = static_cast<void *>(vector);
				break;
			}
			case AbstractColumn::DateTime: {
				QVector<QDateTime>* vector = static_cast<QVector<QDateTime>* >(spreadsheet->child<Column>(n)->data());
				vector->resize(m_actualRows);
				m_dataContainer[n] = static_cast<void *>(vector);
				break;
			}
			//TODO
			case AbstractColumn::Month:
			case AbstractColumn::Day:
				break;
			}
		}

		DEBUG("Prepared!");
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
	DEBUG("	reading type = " << ENUM_TO_STRING(LiveDataSource, ReadingType, readingType));

	//move to the last read position, from == total bytes read
	//since the other source types are sequencial we cannot seek on them
	if (spreadsheet->sourceType() == LiveDataSource::SourceType::FileOrPipe)
		device.seek(from);
	DEBUG("	bytes available = " << device.bytesAvailable());

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
		while (!device.atEnd()) {
			DEBUG("	reading type = " << ENUM_TO_STRING(LiveDataSource, ReadingType, readingType));
			DEBUG("	source type = " << ENUM_TO_STRING(LiveDataSource, SourceType, spreadsheet->sourceType()));
			if (readingType != LiveDataSource::ReadingType::TillEnd) {
				switch (spreadsheet->sourceType()) {	// different sources need different read methods
				case LiveDataSource::SourceType::LocalSocket:
					newData[newDataIdx++] = device.readAll();
					break;
				case LiveDataSource::SourceType::NetworkUdpSocket:
					newData[newDataIdx++] = device.read(device.bytesAvailable());
					break;
				case LiveDataSource::SourceType::FileOrPipe:
				case LiveDataSource::SourceType::NetworkTcpSocket:
				//TODO: check serial port
				case LiveDataSource::SourceType::SerialPort:
					if (!device.canReadLine())
						DEBUG("WARNING in AsciiFilterPrivate::readFromLiveDevice(): device cannot 'readLine()' but using it anyway.");
					newData[newDataIdx++] = device.readLine();
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
				case LiveDataSource::SourceType::NetworkTcpSocket:
				//TODO: check serial port
				case LiveDataSource::SourceType::SerialPort:
					if (!device.canReadLine())
						DEBUG("WARNING in AsciiFilterPrivate::readFromLiveDevice(): device cannot 'readLine()' but using it anyway.");
					newData.push_back(device.readLine());
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
		QDEBUG("Removed empty lines: " << newData.removeAll(""));

	//back to the last read position before counting when reading from files
	if (spreadsheet->sourceType() == LiveDataSource::SourceType::FileOrPipe)
		device.seek(from);

	const int spreadsheetRowCountBeforeResize = spreadsheet->rowCount();

	int currentRow = 0; // indexes the position in the vector(column)
	int linesToRead = 0;
	int keepNValues = spreadsheet->keepNValues();

	DEBUG("Increase row count");
	if (m_prepared) {
		//increase row count if we don't have a fixed size
		//but only after the preparation step
		if (keepNValues == 0) {
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
			if (readingType == LiveDataSource::ReadingType::TillEnd) {
				//we had more lines than the fixed size, so we read m_actualRows number of lines
				if (newLinesTillEnd > m_actualRows) {
					linesToRead = m_actualRows;
					//TODO after reading we should skip the next data lines
					//because it's TillEnd actually
				} else
					linesToRead = newLinesTillEnd;
			} else {
				//we read max sample rate number of lines when the reading mode
				//is ContinuouslyFixed or FromEnd, WholeFile is disabled
				linesToRead = qMin(spreadsheet->sampleSize(), newLinesTillEnd);
			}
		}
		DEBUG("	actual row = " << m_actualRows);

		if (linesToRead == 0)
			return 0;
	} else {
		linesToRead = newLinesTillEnd;
		if (headerEnabled)
			--m_actualRows;
	}
	DEBUG("	lines to read = " << linesToRead);

	//TODO: check other source types
	if (spreadsheet->sourceType() == LiveDataSource::SourceType::NetworkUdpSocket) {
		if (m_actualRows < linesToRead) {
			DEBUG("	SET actual rows to " << linesToRead);
			m_actualRows = linesToRead;
		}
	}

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
		for (int n = 0; n < m_actualCols; ++n) {
			// data() returns a void* which is a pointer to any data type (see ColumnPrivate.cpp)
			switch (columnModes[n]) {
			case AbstractColumn::Numeric: {
				QVector<double>* vector = static_cast<QVector<double>* >(spreadsheet->child<Column>(n)->data());
				vector->resize(m_actualRows);
				m_dataContainer[n] = static_cast<void *>(vector);
				break;
			}
			case AbstractColumn::Integer: {
				QVector<int>* vector = static_cast<QVector<int>* >(spreadsheet->child<Column>(n)->data());
				vector->resize(m_actualRows);
				m_dataContainer[n] = static_cast<void *>(vector);
				break;
			}
			case AbstractColumn::Text: {
				QVector<QString>* vector = static_cast<QVector<QString>*>(spreadsheet->child<Column>(n)->data());
				vector->resize(m_actualRows);
				m_dataContainer[n] = static_cast<void *>(vector);
				break;
			}
			case AbstractColumn::DateTime: {
				QVector<QDateTime>* vector = static_cast<QVector<QDateTime>* >(spreadsheet->child<Column>(n)->data());
				vector->resize(m_actualRows);
				m_dataContainer[n] = static_cast<void *>(vector);
				break;
			}
			//TODO
			case AbstractColumn::Month:
			case AbstractColumn::Day:
				break;
			}
		}
	} else {
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
				//we read max sample rate number of lines when the reading mode
				//is ContinuouslyFixed or FromEnd
				currentRow = m_actualRows - qMin(spreadsheet->sampleSize(), newLinesTillEnd);
			}
		}

		if (m_prepared) {
#ifdef PERFTRACE_LIVE_IMPORT
			PERFTRACE("AsciiLiveDataImportPopping: ");
#endif
			for (int row = 0; row < linesToRead; ++row) {
				for (int col = 0; col < m_actualCols; ++col) {
					switch (columnModes[col]) {
					case AbstractColumn::Numeric: {
						QVector<double>* vector = static_cast<QVector<double>* >(spreadsheet->child<Column>(col)->data());
						vector->pop_front();
						vector->resize(m_actualRows);
						m_dataContainer[col] = static_cast<void *>(vector);
						break;
					}
					case AbstractColumn::Integer: {
						QVector<int>* vector = static_cast<QVector<int>* >(spreadsheet->child<Column>(col)->data());
						vector->pop_front();
						vector->resize(m_actualRows);
						m_dataContainer[col] = static_cast<void *>(vector);
						break;
					}
					case AbstractColumn::Text: {
						QVector<QString>* vector = static_cast<QVector<QString>*>(spreadsheet->child<Column>(col)->data());
						vector->pop_front();
						vector->resize(m_actualRows);
						m_dataContainer[col] = static_cast<void *>(vector);
						break;
					}
					case AbstractColumn::DateTime: {
						QVector<QDateTime>* vector = static_cast<QVector<QDateTime>* >(spreadsheet->child<Column>(col)->data());
						vector->pop_front();
						vector->resize(m_actualRows);
						m_dataContainer[col] = static_cast<void *>(vector);
						break;
					}
					//TODO
					case AbstractColumn::Month:
					case AbstractColumn::Day:
						break;
					}
				}
			}
		}
	}

	// from the last row we read the new data in the spreadsheet
	qDebug() << "reading from line"  << currentRow << " till end" << newLinesTillEnd;
	qDebug() << "Lines to read:" << linesToRead <<", actual rows:" << m_actualRows << ", actual cols:" << m_actualCols;
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
	qDebug() << "newDataIdx: " << newDataIdx;
	//TODO
	static int indexColumnIdx = 0;
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
			DEBUG("	row = " << row);
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

			//qDebug() << "line bytes: " << line.size() << " line: " << line;
			if (simplifyWhitespacesEnabled)
				line = line.simplified();

			if (line.isEmpty() || line.startsWith(commentCharacter)) // skip empty or commented lines
				continue;

			QLocale locale(numberFormat);

			QStringList lineStringList;
			// only FileOrPipe support multiple columns
			if (spreadsheet->sourceType() == LiveDataSource::SourceType::FileOrPipe)
				lineStringList = line.split(m_separator, (QString::SplitBehavior)skipEmptyParts);
			else
				lineStringList << line;
			QDEBUG(" line = " << lineStringList << ", separator = \'" << m_separator << "\'");

			if (createIndexEnabled) {
				if (spreadsheet->keepNValues() == 0)
					lineStringList.prepend(QString::number(currentRow));
				else
					lineStringList.prepend(QString::number(indexColumnIdx++));
			}

			QDEBUG("	column modes = " << columnModes);
			for (int n = 0; n < m_actualCols; ++n) {
				DEBUG("	actual col = " << n);
				if (n < lineStringList.size()) {
					QString valueString = lineStringList.at(n);
					DEBUG("	value string = " << valueString.toStdString());

					// set value depending on data type
					switch (columnModes[n]) {
					case AbstractColumn::Numeric: {
						DEBUG("	Numeric");
						bool isNumber;
						const double value = locale.toDouble(valueString, &isNumber);
						static_cast<QVector<double>*>(m_dataContainer[n])->operator[](currentRow) = (isNumber ? value : nanValue);
						qDebug() << "dataContainer[" << n << "] size:" << static_cast<QVector<double>*>(m_dataContainer[n])->size();
						break;
					}
					case AbstractColumn::Integer: {
						DEBUG("	Integer");
						bool isNumber;
						const int value = locale.toInt(valueString, &isNumber);
						DEBUG("	container size = " << m_dataContainer.size() << ", current row = " << currentRow);
						static_cast<QVector<int>*>(m_dataContainer[n])->operator[](currentRow) = (isNumber ? value : 0);
						qDebug() << "dataContainer[" << n << "] size:" << static_cast<QVector<int>*>(m_dataContainer[n])->size();

						break;
					}
					case AbstractColumn::DateTime: {
						const QDateTime valueDateTime = QDateTime::fromString(valueString, dateTimeFormat);
						static_cast<QVector<QDateTime>*>(m_dataContainer[n])->operator[](currentRow) = valueDateTime.isValid() ? valueDateTime : QDateTime();
						break;
					}
					case AbstractColumn::Text:
						if (removeQuotesEnabled)
							valueString.remove(QRegExp("[\"\']"));
						static_cast<QVector<QString>*>(m_dataContainer[n])->operator[](currentRow) = valueString;
						break;
					case AbstractColumn::Month:
						//TODO
						break;
					case AbstractColumn::Day:
						//TODO
						break;
					}
				} else {
					DEBUG("	missing columns in this line");
					switch (columnModes[n]) {
					case AbstractColumn::Numeric:
						static_cast<QVector<double>*>(m_dataContainer[n])->operator[](currentRow) = nanValue;
						break;
					case AbstractColumn::Integer:
						static_cast<QVector<int>*>(m_dataContainer[n])->operator[](currentRow) = 0;
						break;
					case AbstractColumn::DateTime:
						static_cast<QVector<QDateTime>*>(m_dataContainer[n])->operator[](currentRow) = QDateTime();
						break;
					case AbstractColumn::Text:
						static_cast<QVector<QString>*>(m_dataContainer[n])->operator[](currentRow) = "";
						break;
					case AbstractColumn::Month:
						//TODO
						break;
					case AbstractColumn::Day:
						//TODO
						break;
					}
				}
			}
			currentRow++;
		}
	}

	if (m_prepared) {
		//notify all affected columns and plots about the changes
		PERFTRACE("AsciiLiveDataImport, notify affected columns and plots");
		const Project* project = spreadsheet->project();
		QVector<const XYCurve*> curves = project->children<const XYCurve>(AbstractAspect::Recursive);
		QVector<CartesianPlot*> plots;
		for (int n = 0; n < m_actualCols; ++n) {
			Column* column = spreadsheet->column(n);

			//determine the plots where the column is consumed
			for (const auto* curve: curves) {
				if (curve->xColumn() == column || curve->yColumn() == column) {
					CartesianPlot* plot = dynamic_cast<CartesianPlot*>(curve->parentAspect());
					if (plots.indexOf(plot) == -1) {
						plots << plot;
						plot->setSuppressDataChangedSignal(true);
					}
				}
			}

			column->setChanged();
		}

		//loop over all affected plots and retransform them
		for (auto* plot: plots) {
			plot->setSuppressDataChangedSignal(false);
			plot->dataChanged();
		}
	}

	m_prepared = true;

	return bytesread;
}

/*!
    reads the content of device \c device to the data source \c dataSource. Uses the settings defined in the data source.
*/
void AsciiFilterPrivate::readDataFromDevice(QIODevice& device, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode importMode, int lines) {
	DEBUG("AsciiFilterPrivate::readDataFromDevice(): dataSource = " << dataSource
	      << ", mode = " << ENUM_TO_STRING(AbstractFileFilter, ImportMode, importMode) << ", lines = " << lines);

	if (!m_prepared) {
		const int deviceError = prepareDeviceToRead(device);
		if (deviceError != 0) {
			DEBUG("Device error = " << deviceError);
			return;
		}

		// matrix data has only one column mode (which is not text)
		if (dynamic_cast<Matrix*>(dataSource)) {
			auto mode = columnModes[0];
			if (mode == AbstractColumn::Text)
				mode = AbstractColumn::Numeric;
			for (auto& c: columnModes)
				if (c != mode)
					c = mode;
		}

		m_columnOffset = dataSource->prepareImport(m_dataContainer, importMode, m_actualRows - m_actualStartRow + 1,
		                 m_actualCols, vectorNames, columnModes);

		m_prepared = true;
	}

	DEBUG("locale = " << QLocale::languageToString(numberFormat).toStdString());
	QLocale locale(numberFormat);

	// Read the data
	int currentRow = 0;	// indexes the position in the vector(column)
	if (lines == -1)
		lines = m_actualRows;

	DEBUG("reading " << qMin(lines, m_actualRows)  << " lines");
	for (int i = 0; i < qMin(lines, m_actualRows); ++i) {
		QString line = device.readLine();

		// skip start lines
		if (m_actualStartRow > 1) {
			m_actualStartRow--;
			continue;
		}

		line.remove(QRegExp("[\\n\\r]"));	// remove any newline
		if (simplifyWhitespacesEnabled)
			line = line.simplified();

		if (line.isEmpty() || line.startsWith(commentCharacter)) // skip empty or commented lines
			continue;


		QStringList lineStringList = line.split(m_separator, (QString::SplitBehavior)skipEmptyParts);

		//prepend the index if required
		//TODO: come up maybe with a solution with adding the index inside of the loop below,
		//without conversion to string, prepending to the list and then conversion back to integer.
		if (createIndexEnabled)
			lineStringList.prepend(QString::number(i+1));

		// remove left white spaces
		if (skipEmptyParts) {
			for (int n = 0; n < lineStringList.size(); ++n) {
				QString valueString = lineStringList.at(n);
				if (!QString::compare(valueString, " ")) {
					lineStringList.removeAt(n);
					n--;
				}
			}
		}

		for (int n = 0; n < m_actualCols; ++n) {
			if (n < lineStringList.size()) {
				QString valueString = lineStringList.at(n);

				// set value depending on data type
				switch (columnModes[n]) {
				case AbstractColumn::Numeric: {
					bool isNumber;
					const double value = locale.toDouble(valueString, &isNumber);
					static_cast<QVector<double>*>(m_dataContainer[n])->operator[](currentRow) = (isNumber ? value : nanValue);
					break;
				}
				case AbstractColumn::Integer: {
					bool isNumber;
					const int value = locale.toInt(valueString, &isNumber);
					static_cast<QVector<int>*>(m_dataContainer[n])->operator[](currentRow) = (isNumber ? value : 0);
					break;
				}
				case AbstractColumn::DateTime: {
					const QDateTime valueDateTime = QDateTime::fromString(valueString, dateTimeFormat);
					static_cast<QVector<QDateTime>*>(m_dataContainer[n])->operator[](currentRow) = valueDateTime.isValid() ? valueDateTime : QDateTime();
					break;
				}
				case AbstractColumn::Text:
					if (removeQuotesEnabled)
						valueString.remove(QRegExp("[\"\']"));
					static_cast<QVector<QString>*>(m_dataContainer[n])->operator[](currentRow) = valueString;
					break;
				case AbstractColumn::Month:	// never happens
				case AbstractColumn::Day:
					break;
				}
			} else {	// missing columns in this line
				switch (columnModes[n]) {
				case AbstractColumn::Numeric:
					static_cast<QVector<double>*>(m_dataContainer[n])->operator[](currentRow) = nanValue;
					break;
				case AbstractColumn::Integer:
					static_cast<QVector<int>*>(m_dataContainer[n])->operator[](currentRow) = 0;
					break;
				case AbstractColumn::DateTime:
					static_cast<QVector<QDateTime>*>(m_dataContainer[n])->operator[](currentRow) = QDateTime();
					break;
				case AbstractColumn::Text:
					static_cast<QVector<QString>*>(m_dataContainer[n])->operator[](currentRow) = "";
					break;
				case AbstractColumn::Month:	// never happens
				case AbstractColumn::Day:
					break;
				}
			}
		}

		currentRow++;
		emit q->completed(100 * currentRow/m_actualRows);
	}

	dataSource->finalizeImport(m_columnOffset, startColumn, endColumn, dateTimeFormat, importMode);
}

/*!
 * preview for special devices (local/UDP/TCP socket or serial port)
 */
QVector<QStringList> AsciiFilterPrivate::preview(QIODevice &device) {
	DEBUG("AsciiFilterPrivate::preview(): bytesAvailable = " << device.bytesAvailable() << ", isSequential = " << device.isSequential());
	QVector<QStringList> dataStrings;

	if (!(device.bytesAvailable() > 0)) {
		DEBUG("No new data available");
		return dataStrings;
	}

	if (device.isSequential() && device.bytesAvailable() < (int)sizeof(quint16))
		return dataStrings;

#ifdef PERFTRACE_LIVE_IMPORT
	PERFTRACE("AsciiLiveDataImportTotal: ");
#endif

	int linesToRead = 0;
	QVector<QString> newData;

	while (!device.atEnd()) {
		if (device.canReadLine())
			newData.push_back(device.readLine());
		else	// UDP fails otherwise
			newData.push_back(device.readAll());
		linesToRead++;
	}
	QDEBUG("	data = " << newData);

	if (linesToRead == 0) return dataStrings;

	int col = 0;
	int colMax = newData.at(0).size();
	if (createIndexEnabled)
		colMax++;
	columnModes.resize(colMax);
	if (createIndexEnabled) {
		columnModes[0] = AbstractColumn::ColumnMode::Integer;
		col = 1;
		vectorNames.prepend(i18n("Index"));
	}
	vectorNames.append(i18n("Value"));
	QDEBUG("	vector names = " << vectorNames);

	for (const auto& valueString: newData.at(0).split(' ', QString::SkipEmptyParts)) {
		if (col == colMax)
			break;
		columnModes[col++] = AbstractFileFilter::columnMode(valueString, dateTimeFormat, numberFormat);
	}

	for (int i = 0; i < linesToRead; ++i) {
		QString line = newData.at(i);

		if (simplifyWhitespacesEnabled)
			line = line.simplified();

		if (line.isEmpty() || line.startsWith(commentCharacter)) // skip empty or commented lines
			continue;

		QLocale locale(numberFormat);

		QStringList lineStringList = line.split(' ', QString::SkipEmptyParts);
		if (createIndexEnabled)
			lineStringList.prepend(QString::number(i));

		QStringList lineString;
		for (int n = 0; n < lineStringList.size(); ++n) {
			if (n < lineStringList.size()) {
				QString valueString = lineStringList.at(n);

				switch (columnModes[n]) {
				case AbstractColumn::Numeric: {
					bool isNumber;
					const double value = locale.toDouble(valueString, &isNumber);
					lineString += QString::number(isNumber ? value : nanValue, 'g', 16);
					break;
				}
				case AbstractColumn::Integer: {
					bool isNumber;
					const int value = locale.toInt(valueString, &isNumber);
					lineString += QString::number(isNumber ? value : 0);
					break;
				}
				case AbstractColumn::DateTime: {
					const QDateTime valueDateTime = QDateTime::fromString(valueString, dateTimeFormat);
					lineString += valueDateTime.isValid() ? valueDateTime.toString(dateTimeFormat) : QLatin1String(" ");
					break;
				}
				case AbstractColumn::Text:
					if (removeQuotesEnabled)
						valueString.remove(QRegExp("[\"\']"));
					lineString += valueString;
					break;
				case AbstractColumn::Month:	// never happens
				case AbstractColumn::Day:
					break;
				}
			} else 	// missing columns in this line
				lineString += QLatin1String("");
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

	KFilterDev device(fileName);
	const int deviceError = prepareDeviceToRead(device);
	if (deviceError != 0) {
		DEBUG("Device error = " << deviceError);
		return dataStrings;
	}

	//number formatting
	DEBUG("locale = " << QLocale::languageToString(numberFormat).toStdString());
	QLocale locale(numberFormat);

	// Read the data
	if (lines == -1)
		lines = m_actualRows;

	// set column names for preview
	if (!headerEnabled) {
		int start = 0;
		if (createIndexEnabled)
			start = 1;
		for (int i=start;i<m_actualCols;i++)
				vectorNames << "Column " + QString::number(i+1);
	}
	QDEBUG("	column names = " << vectorNames);

	DEBUG("generating preview for " << qMin(lines, m_actualRows)  << " lines");
	for (int i = 0; i < qMin(lines, m_actualRows); ++i) {
		QString line = device.readLine();

		// skip start lines
		if (m_actualStartRow > 1) {
			m_actualStartRow--;
			continue;
		}

		line.remove(QRegExp("[\\n\\r]"));	// remove any newline
		if (simplifyWhitespacesEnabled)
			line = line.simplified();

		if (line.isEmpty() || line.startsWith(commentCharacter)) // skip empty or commented lines
			continue;


		QStringList lineStringList = line.split(m_separator, (QString::SplitBehavior)skipEmptyParts);
		QDEBUG(" line = " << lineStringList);

		//prepend index if required
		if (createIndexEnabled)
			lineStringList.prepend(QString::number(i+1));

		QStringList lineString;
		for (int n = 0; n < m_actualCols; ++n) {
			if (n < lineStringList.size()) {
				QString valueString = lineStringList.at(n);
				//DEBUG(" valueString = " << valueString.toStdString());
				if (skipEmptyParts && !QString::compare(valueString, " "))	// handle left white spaces
					continue;

				// set value depending on data type
				switch (columnModes[n]) {
				case AbstractColumn::Numeric: {
					bool isNumber;
					const double value = locale.toDouble(valueString, &isNumber);
					lineString += QString::number(isNumber ? value : nanValue, 'g', 15);
					break;
				}
				case AbstractColumn::Integer: {
					bool isNumber;
					const int value = locale.toInt(valueString, &isNumber);
					lineString += QString::number(isNumber ? value : 0);
					break;
				}
				case AbstractColumn::DateTime: {
					const QDateTime valueDateTime = QDateTime::fromString(valueString, dateTimeFormat);
					lineString += valueDateTime.isValid() ? valueDateTime.toString(dateTimeFormat) : QLatin1String(" ");
					break;
				}
				case AbstractColumn::Text:
					if (removeQuotesEnabled)
						valueString.remove(QRegExp("[\"\']"));
					lineString += valueString;
					break;
				case AbstractColumn::Month:	// never happens
				case AbstractColumn::Day:
					break;
				}
			} else 	// missing columns in this line
				lineString += QLatin1String("");
		}

		dataStrings << lineString;
	}

	return dataStrings;
}

/*!
    writes the content of \c dataSource to the file \c fileName.
*/
void AsciiFilterPrivate::write(const QString & fileName, AbstractDataSource* dataSource) {
	Q_UNUSED(fileName);
	Q_UNUSED(dataSource);

	//TODO: save data to ascii file
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
	if (!reader->isStartElement() || reader->name() != "asciiFilter") {
		reader->raiseError(i18n("no ascii filter element found"));
		return false;
	}

	KLocalizedString attributeWarning = ki18n("Attribute '%1' missing or empty, default value is used");
	QXmlStreamAttributes attribs = reader->attributes();

	QString str = attribs.value("commentCharacter").toString();
	if (str.isEmpty())
		reader->raiseWarning(attributeWarning.subs("commentCharacter").toString());
	else
		d->commentCharacter = str;

	str = attribs.value("separatingCharacter").toString();
	if (str.isEmpty())
		reader->raiseWarning(attributeWarning.subs("separatingCharacter").toString());
	else
		d->separatingCharacter = str;

	str = attribs.value("createIndex").toString();
	if (str.isEmpty())
		reader->raiseWarning(attributeWarning.subs("createIndex").toString());
	else
		d->createIndexEnabled = str.toInt();

	str = attribs.value("autoMode").toString();
	if (str.isEmpty())
		reader->raiseWarning(attributeWarning.subs("autoMode").toString());
	else
		d->autoModeEnabled = str.toInt();

	str = attribs.value("header").toString();
	if (str.isEmpty())
		reader->raiseWarning(attributeWarning.subs("header").toString());
	else
		d->headerEnabled = str.toInt();

	str = attribs.value("vectorNames").toString();
	d->vectorNames = str.split(' '); //may be empty

	str = attribs.value("simplifyWhitespaces").toString();
	if (str.isEmpty())
		reader->raiseWarning(attributeWarning.subs("simplifyWhitespaces").toString());
	else
		d->simplifyWhitespacesEnabled = str.toInt();

	str = attribs.value("nanValue").toString();
	if (str.isEmpty())
		reader->raiseWarning(attributeWarning.subs("nanValue").toString());
	else
		d->nanValue = str.toDouble();

	str = attribs.value("removeQuotes").toString();
	if (str.isEmpty())
		reader->raiseWarning(attributeWarning.subs("removeQuotes").toString());
	else
		d->removeQuotesEnabled = str.toInt();

	str = attribs.value("skipEmptyParts").toString();
	if (str.isEmpty())
		reader->raiseWarning(attributeWarning.subs("skipEmptyParts").toString());
	else
		d->skipEmptyParts = str.toInt();

	str = attribs.value("startRow").toString();
	if (str.isEmpty())
		reader->raiseWarning(attributeWarning.subs("startRow").toString());
	else
		d->startRow = str.toInt();

	str = attribs.value("endRow").toString();
	if (str.isEmpty())
		reader->raiseWarning(attributeWarning.subs("endRow").toString());
	else
		d->endRow = str.toInt();

	str = attribs.value("startColumn").toString();
	if (str.isEmpty())
		reader->raiseWarning(attributeWarning.subs("startColumn").toString());
	else
		d->startColumn = str.toInt();

	str = attribs.value("endColumn").toString();
	if (str.isEmpty())
		reader->raiseWarning(attributeWarning.subs("endColumn").toString());
	else
		d->endColumn = str.toInt();

	return true;
}

int AsciiFilterPrivate::isPrepared() {
    return m_prepared;
}

int AsciiFilter::isPrepared() {
    return d->isPrepared();
}

#ifdef HAVE_MQTT
void AsciiFilter::mqttPreview(QVector<QStringList>& list, const QString& message, const QString& topic) {
	d->mqttPreview(list, message, topic);
}

void AsciiFilterPrivate::mqttPreview(QVector<QStringList>& list, const QString& message, const QString& topic) {
	QVector<QStringList> dataStrings;

	if (!message.isEmpty()) {
		qDebug()<<"ascii mqtt preview" <<message<< "  " <<topic;

		int linesToRead = 0;
		QVector<QString> newData;
		QStringList newDataList = message.split(QRegExp("\n|\r\n|\r"), QString::SkipEmptyParts);
		for (auto& valueString: newDataList) {
			QStringList splitString = valueString.split(' ', QString::SkipEmptyParts);
			for ( const auto& valueString2: splitString) {
				if (!valueString2.isEmpty() && !valueString2.startsWith(commentCharacter) ) {
					linesToRead++;
					newData.push_back(valueString2);
				}
			}
		}
		qDebug() <<" data investigated, lines to read: "<<linesToRead;

		if (linesToRead == 0) {
			list = dataStrings;
			return;
		}

		if(!list.isEmpty()) {
			if(linesToRead < list.size()) {
				int oldSize = list.size();
				for(int i = 0; i < oldSize - linesToRead; ++i)
					list.removeLast();
				qDebug()<<"Lines to read smaller: "	<<linesToRead;
			}
			else if (linesToRead > list.size()) {
				linesToRead = list.size();
				qDebug()<<"Lines to bigger: "	<<linesToRead;
			}
		}

		int colSize = 0;
		if(list.isEmpty()) {
			if (createIndexEnabled)
				colSize = 2;
			else colSize = 1;
		}
		else
			colSize = columnModes.size() + 1;

		columnModes.resize(colSize);
		qDebug() << "Column modes resized: " << colSize;

		if(list.isEmpty())
			if (createIndexEnabled) {
				columnModes[0] = AbstractColumn::ColumnMode::Integer;
				vectorNames.prepend("index");
			}

		vectorNames.append( topic);

		qDebug()<<"vector name set on : " << topic;

		columnModes[colSize - 1] = AbstractFileFilter::columnMode(newData[0], dateTimeFormat, numberFormat);

		qDebug()<<"column mode "<<colSize - 1 << "set on: " << topic << "while column mode size: " <<columnModes.size();

		for(int i = 0; i < linesToRead; i++) {
			QString tempLine = newData[i];
			if (simplifyWhitespacesEnabled)
				tempLine = tempLine.simplified();
			AbstractColumn::ColumnMode mode = AbstractFileFilter::columnMode(tempLine, dateTimeFormat, numberFormat);

			// numeric: integer -> numeric
			if (mode == AbstractColumn::Numeric && columnModes[colSize - 1] == AbstractColumn::Integer) {
				columnModes[colSize - 1] = mode;
				qDebug()<<"-----setting column mode" << "   "<<columnModes[colSize - 1]<< "based on value  " << tempLine;
			}
			// text: non text -> text
			if ( (mode == AbstractColumn::Text) && (columnModes[colSize - 1] != AbstractColumn::Text) ) {
				columnModes[colSize - 1] = mode;
				qDebug()<<"-----setting column mode" << "   "<<columnModes[colSize - 1]<< "based on value  " << tempLine;
			}
		}
		int forStart = 0;
		int forEnd = 0;

		if (list.isEmpty()) {
			forStart = 0;
			forEnd = linesToRead;
		}
		else {
			if (mqttPreviewFirstEmptyColCount == 0) {
				dataStrings = list;
				forStart = 0;
				forEnd = dataStrings.size();
			}
			else {
				forStart = 1;
				forEnd = linesToRead;
				dataStrings = list;
				qDebug() << "first column empty";
				QLocale locale(numberFormat);
				qDebug()<<columnModes.size()<<"   colsize-1= "<<colSize-1;
				switch (columnModes[colSize - 1]) {
				case AbstractColumn::Numeric: {
					bool isNumber;
					const double value = locale.toDouble(newData[0], &isNumber);
					dataStrings[0] += QString::number(isNumber ? value : nanValue, 'g', 16);
					break;
				}
				case AbstractColumn::Integer: {
					bool isNumber;
					const int value = locale.toInt(newData[0], &isNumber);
					dataStrings[0]  += QString::number(isNumber ? value : 0);
					break;
				}
				case AbstractColumn::DateTime: {
					const QDateTime valueDateTime = QDateTime::fromString(newData[0], dateTimeFormat);
					dataStrings[0]  += valueDateTime.isValid() ? valueDateTime.toString(dateTimeFormat) : QLatin1String(" ");
					break;
				}
				case AbstractColumn::Text:
					if (removeQuotesEnabled)
						newData[0].remove(QRegExp("[\"\']"));
					dataStrings[0]  += newData[0];
					break;
				case AbstractColumn::Month:	// never happens
				case AbstractColumn::Day:
					break;
				}
				qDebug()<<"Column after first column empty done";
			}
		}
		for (int i = forStart; i < forEnd; ++i) {
			qDebug()<<"line = newData[i]";
			QString line = newData[i];

			if (simplifyWhitespacesEnabled)
				line = line.simplified();

			if (line.isEmpty() || line.startsWith(commentCharacter)) // skip empty or commented lines
				continue;

			QLocale locale(numberFormat);

			QStringList lineString;
			if(!list.isEmpty() &&  mqttPreviewFirstEmptyColCount == 0)
				lineString = dataStrings[i];

			if(list.isEmpty() || (!list.isEmpty() && mqttPreviewFirstEmptyColCount > 0) )
				if (createIndexEnabled)
					lineString += QString::number(i);

			if(!list.isEmpty() && mqttPreviewFirstEmptyColCount > 0){
				for(int j = 0; j < mqttPreviewFirstEmptyColCount; j++) {
					lineString += QString::number(nanValue, 'g', 16);
					qDebug()<<"first column updated with nan";
				}
			}

			switch (columnModes[colSize - 1]) {
			case AbstractColumn::Numeric: {
				bool isNumber;
				const double value = locale.toDouble(line, &isNumber);
				lineString += QString::number(isNumber ? value : nanValue, 'g', 16);
				break;
			}
			case AbstractColumn::Integer: {
				bool isNumber;
				const int value = locale.toInt(line, &isNumber);
				lineString += QString::number(isNumber ? value : 0);
				break;
			}
			case AbstractColumn::DateTime: {
				const QDateTime valueDateTime = QDateTime::fromString(line, dateTimeFormat);
				lineString += valueDateTime.isValid() ? valueDateTime.toString(dateTimeFormat) : QLatin1String(" ");
				break;
			}
			case AbstractColumn::Text:
				if (removeQuotesEnabled)
					line.remove(QRegExp("[\"\']"));
				lineString += line;
				break;
			case AbstractColumn::Month:	// never happens
			case AbstractColumn::Day:
				break;
			}
			qDebug()<<"column updated with value";

			if(list.isEmpty() || (!list.isEmpty() && mqttPreviewFirstEmptyColCount > 0))
				dataStrings << lineString;
			if (!list.isEmpty() && mqttPreviewFirstEmptyColCount == 0)
				dataStrings[i] = lineString;
		}
		if(mqttPreviewFirstEmptyColCount > 0) {
			mqttPreviewFirstEmptyColCount = 0;
		}
	}
	else if (list.isEmpty() || (!list.isEmpty() && mqttPreviewFirstEmptyColCount > 0) ) {
		mqttPreviewFirstEmptyColCount ++;
		qDebug()<<"first column empty: "<< mqttPreviewFirstEmptyColCount;
		int colSize;
		if (mqttPreviewFirstEmptyColCount == 1){
			if (createIndexEnabled)
				colSize = 2;
			else colSize = 1;
		}
		else
			colSize = columnModes.size() + 1;
		columnModes.resize(colSize);

		if (mqttPreviewFirstEmptyColCount == 1)
			if (createIndexEnabled) {
				columnModes[0] = AbstractColumn::ColumnMode::Integer;
				vectorNames.prepend("index");
			}

		vectorNames.append( topic);
		columnModes[colSize-1] = AbstractColumn::ColumnMode::Numeric;
		qDebug()<<"Column mode set for empty column";

		QStringList lineString;
		if (mqttPreviewFirstEmptyColCount == 1) {
			if(createIndexEnabled)
				lineString += QString::number(0);
			lineString += QString::number(nanValue, 'g', 16);
			dataStrings << lineString;
			qDebug()<<"new line added with nan";
		}
		else {
			dataStrings = list;
			dataStrings[0] += QString::number(nanValue, 'g', 16);
			qDebug()<<"line expanded with nan";
		}
	}
	else if(!list.isEmpty()) {

		vectorNames.append( topic);
		qDebug()<<"vector name set on : " << topic;

		int colSize = columnModes.size() + 1;
		columnModes.resize(colSize);
		columnModes[colSize-1] = AbstractColumn::ColumnMode::Numeric;
		dataStrings = list;
		for (int i = 0; i < dataStrings.size(); ++i) {
			dataStrings[i] += QString::number(nanValue, 'g', 16);
		}
	}
	list = dataStrings;
}

QString AsciiFilter::mqttColumnStatistics(const Spreadsheet* spreadsheet, AbstractAspect* aspect) const{
	return d->mqttColumnStatistics(spreadsheet, aspect);
}

QString AsciiFilterPrivate::mqttColumnStatistics(const Spreadsheet* spreadsheet, AbstractAspect* aspect) const{
	qDebug()<<"MQTT Column Statistics";
	MQTTClient* client = dynamic_cast<MQTTClient*>(aspect);

	QString statistics;
	Column* tempColumn = dynamic_cast<const MQTTTopic*>(spreadsheet)->child<Column>(m_actualCols - 1);

	QVector<bool> willStatistics = client->willStatistics();
	for(int i = 0; i <= willStatistics.count(); i++) {
		if(willStatistics[i]) {
			switch (static_cast<MQTTClient::WillStatistics>(i) ) {
			case MQTTClient::WillStatistics::ArithmeticMean:
				statistics += "Arithmetic mean: " + QString::number(tempColumn->statistics().arithmeticMean)+"\n";
				break;
			case MQTTClient::WillStatistics::ContraharmonicMean:
				statistics += "Contraharmonic mean: "+QString::number(tempColumn->statistics().contraharmonicMean)+"\n";
				break;
			case MQTTClient::WillStatistics::Entropy:
				statistics += "Entropy: "+QString::number(tempColumn->statistics().entropy)+"\n";
				break;
			case MQTTClient::WillStatistics::GeometricMean:
				statistics += "Geometric mean: "+QString::number(tempColumn->statistics().geometricMean)+"\n";
				break;
			case MQTTClient::WillStatistics::HarmonicMean:
				statistics += "Harmonic mean: "+QString::number(tempColumn->statistics().harmonicMean)+"\n";
				break;
			case MQTTClient::WillStatistics::Kurtosis:
				statistics += "Kurtosis: "+QString::number(tempColumn->statistics().kurtosis)+"\n";
				break;
			case MQTTClient::WillStatistics::Maximum:
				statistics += "Maximum: "+QString::number(tempColumn->statistics().maximum)+"\n";
				break;
			case MQTTClient::WillStatistics::MeanDeviation:
				statistics += "Mean deviation: "+QString::number(tempColumn->statistics().meanDeviation)+"\n";
				break;
			case MQTTClient::WillStatistics::MeanDeviationAroundMedian:
				statistics += "Mean deviation around median: "+QString::number(tempColumn->statistics().meanDeviationAroundMedian)+"\n";
				break;
			case MQTTClient::WillStatistics::Median:
				statistics += "Median: "+QString::number(tempColumn->statistics().median)+"\n";
				break;
			case MQTTClient::WillStatistics::MedianDeviation:
				statistics += "Median deviation: "+QString::number(tempColumn->statistics().medianDeviation)+"\n";
				break;
			case MQTTClient::WillStatistics::Minimum:
				statistics += "Minimum: "+QString::number(tempColumn->statistics().minimum)+"\n";
				break;
			case MQTTClient::WillStatistics::Skewness:
				statistics += "Skewness: "+QString::number(tempColumn->statistics().skewness)+"\n";
				break;
			case MQTTClient::WillStatistics::StandardDeviation:
				statistics += "Standard deviation: "+QString::number(tempColumn->statistics().standardDeviation)+"\n";
				break;
			case MQTTClient::WillStatistics::Variance:
				statistics += "Variance: "+QString::number(tempColumn->statistics().variance)+"\n";
				break;
			default:
				break;
			}
		}
	}
	return statistics;
}

AbstractColumn::ColumnMode AsciiFilter::mqttColumnMode() const{
	return d->mqttColumnMode();
}

AbstractColumn::ColumnMode AsciiFilterPrivate::mqttColumnMode() const{

	return columnModes[m_actualCols - 1];
}

void AsciiFilter::readMQTTTopic(const QString& message, const QString& topic, AbstractDataSource*dataSource) {
	d->readMQTTTopic(message, topic, dataSource);
}

void AsciiFilterPrivate::readMQTTTopic(const QString& message, const QString& topic, AbstractDataSource*dataSource) {
	if (message.isEmpty()) {
		DEBUG("No new data available");
		return;
	}

	MQTTTopic* spreadsheet = dynamic_cast<MQTTTopic*>(dataSource);

	int keepNValues = spreadsheet->keepNvalues();

	if (!m_prepared) {
		qDebug()<<"Start prepare mqtt";
		const int mqttPrepareError = prepareMQTTTopicToRead(message, topic);
		if (mqttPrepareError != 0) {
			DEBUG("Mqtt Prepare Error = " << mqttPrepareError);
			qDebug()<<mqttPrepareError<<"  itt van baj mqttPrepareError";
			return;
		}

		qDebug()<<"prepare mqtt done";

		// prepare import for spreadsheet
		spreadsheet->setUndoAware(false);
		spreadsheet->resize(AbstractFileFilter::Replace, vectorNames, m_actualCols);
		qDebug() << "fds resized to col: " << m_actualCols;
		qDebug() << "fds rowCount: " << spreadsheet->rowCount();

		//columns in a file data source don't have any manual changes.
		//make the available columns undo unaware and suppress the "data changed" signal.
		//data changes will be propagated via an explicit Column::setChanged() call once new data was read.
		for (int i = 0; i < spreadsheet->childCount<Column>(); i++) {
			spreadsheet->child<Column>(i)->setUndoAware(false);
			spreadsheet->child<Column>(i)->setSuppressDataChangedSignal(true);
		}

		if (keepNValues == 0)
			spreadsheet->setRowCount(m_actualRows > 1 ? m_actualRows : 1);
		else {
			spreadsheet->setRowCount(spreadsheet->keepNvalues());
			m_actualRows = spreadsheet->keepNvalues();
		}

		m_dataContainer.resize(m_actualCols);

		for (int n = 0; n < m_actualCols; ++n) {
			// data() returns a void* which is a pointer to any data type (see ColumnPrivate.cpp)
			spreadsheet->child<Column>(n)->setColumnMode(columnModes[n]);
			switch (columnModes[n]) {
			case AbstractColumn::Numeric: {
				QVector<double>* vector = static_cast<QVector<double>* >(spreadsheet->child<Column>(n)->data());
				vector->reserve(m_actualRows);
				vector->resize(m_actualRows);
				m_dataContainer[n] = static_cast<void *>(vector);
				break;
			}
			case AbstractColumn::Integer: {
				QVector<int>* vector = static_cast<QVector<int>* >(spreadsheet->child<Column>(n)->data());
				vector->reserve(m_actualRows);
				vector->resize(m_actualRows);
				m_dataContainer[n] = static_cast<void *>(vector);
				break;
			}
			case AbstractColumn::Text: {
				QVector<QString>* vector = static_cast<QVector<QString>*>(spreadsheet->child<Column>(n)->data());
				vector->reserve(m_actualRows);
				vector->resize(m_actualRows);
				m_dataContainer[n] = static_cast<void *>(vector);
				break;
			}
			case AbstractColumn::DateTime: {
				QVector<QDateTime>* vector = static_cast<QVector<QDateTime>* >(spreadsheet->child<Column>(n)->data());
				vector->reserve(m_actualRows);
				vector->resize(m_actualRows);
				m_dataContainer[n] = static_cast<void *>(vector);
				break;
			}
				//TODO
			case AbstractColumn::Month:
			case AbstractColumn::Day:
				break;
			}
		}

		qDebug() << "prepared!";
	}


#ifdef PERFTRACE_LIVE_IMPORT
	PERFTRACE("AsciiLiveDataImportTotal: ");
#endif
	MQTTClient::ReadingType readingType;
	if (!m_prepared) {
		readingType = MQTTClient::ReadingType::TillEnd;
	} else {
		//we have to read all the data when reading from end
		//so we set readingType to TillEnd
		if (spreadsheet->readingType() == MQTTClient::ReadingType::FromEnd) {
			readingType = MQTTClient::ReadingType::TillEnd;
		} else {
			readingType = static_cast<MQTTClient::ReadingType>(spreadsheet->readingType());
		}
	}

	//count the new lines, increase actualrows on each
	//now we read all the new lines, if we want to use sample rate
	//then here we can do it, if we have actually sample rate number of lines :-?
	int newLinesForSampleRateNotTillEnd = 0;
	int newLinesTillEnd = 0;
	QVector<QString> newData;
	if (readingType != MQTTClient::ReadingType::TillEnd) {
		newData.reserve(spreadsheet->sampleRate());
		newData.resize(spreadsheet->sampleRate());
	}

	int newDataIdx = 0;
	bool sampleRateReached = false;
	{
#ifdef PERFTRACE_LIVE_IMPORT
		PERFTRACE("AsciiLiveDataImportReadingFromFile: ");
#endif
		QStringList newDataList = message.split(QRegExp("\n|\r\n|\r"), QString::SkipEmptyParts);
		for (auto& valueString: newDataList) {
			QStringList splitString = valueString.split(m_separator, static_cast<QString::SplitBehavior>(skipEmptyParts));
			for (const auto& valueString2: splitString) {
				if (!valueString2.isEmpty() && !valueString2.startsWith(commentCharacter)) {
					if (readingType != MQTTClient::ReadingType::TillEnd)
						newData[newDataIdx++] = valueString2;
					else
						newData.push_back(valueString2);
					newLinesTillEnd++;

					if (readingType != MQTTClient::ReadingType::TillEnd) {
						newLinesForSampleRateNotTillEnd++;
						//for Continous reading and FromEnd we read sample rate number of lines if possible
						qDebug()<<"new lines for sample rate: "<<newLinesForSampleRateNotTillEnd;
						if (newLinesForSampleRateNotTillEnd == spreadsheet->sampleRate()) {
							sampleRateReached = true;
							break;
						}
					}
				}
			}
			if(sampleRateReached)
				break;
		}
	}
	qDebug()<<"Processing message done";
	//now we reset the readingType
	if (static_cast<MQTTClient::ReadingType>(spreadsheet->readingType()) == MQTTClient::ReadingType::FromEnd)
		readingType = static_cast<MQTTClient::ReadingType>(spreadsheet->readingType());

	//we had less new lines than the sample rate specified
	if (readingType != MQTTClient::ReadingType::TillEnd)
		qDebug() << "Removed empty lines: " << newData.removeAll("");

	qDebug()<<"Create index enabled:  "<<createIndexEnabled;

	const int spreadsheetRowCountBeforeResize = spreadsheet->rowCount();;

	if(m_prepared ) {
		if (keepNValues == 0)
			m_actualRows = spreadsheetRowCountBeforeResize;
		else {
			if(m_actualRows != spreadsheet->keepNvalues()) {
				if(m_actualRows < spreadsheet->keepNvalues()) {
					qDebug()<<m_actualRows;
					spreadsheet->setRowCount(spreadsheet->keepNvalues());
					qDebug()<<"rowcount set to" << spreadsheet->keepNvalues();
				}

				int rowDiff = 0;
				if(m_actualRows > spreadsheet->keepNvalues()) {
					rowDiff = m_actualRows -  spreadsheet->keepNvalues();
				}
				if(m_actualRows < spreadsheet->keepNvalues()) {
					rowDiff =spreadsheet->keepNvalues() - m_actualRows;
				}
				qDebug()<<"last value changed: "<<m_actualRows<<"  "<<spreadsheet->keepNvalues()<<"   "<<rowDiff;

				for (int n = 0; n < columnModes.size(); ++n) {
					// data() returns a void* which is a pointer to any data type (see ColumnPrivate.cpp)
					qDebug()<<"modifying column: "<<n;
					qDebug()<<"last value changed: "<<m_actualRows<<"  "<<spreadsheet->keepNvalues();
					switch (columnModes[n]) {
					case AbstractColumn::Numeric: {
						QVector<double>*  vector = static_cast<QVector<double>* >(spreadsheet->child<Column>(n)->data());
						m_dataContainer[n] = static_cast<void *>(vector);

						if(m_actualRows > spreadsheet->keepNvalues()) {
							for(int i = 0; i < spreadsheet->keepNvalues(); i++) {
								static_cast<QVector<double>*>(m_dataContainer[n])->operator[] (i) =
										static_cast<QVector<double>*>(m_dataContainer[n])->operator[](m_actualRows - spreadsheet->keepNvalues() + i);
								qDebug()<< "overwrite row"<<i<<"  "<<m_actualRows - spreadsheet->keepNvalues() + i;
							}
						}

						if(m_actualRows < spreadsheet->keepNvalues()) {
							vector->reserve( spreadsheet->keepNvalues());
							vector->resize( spreadsheet->keepNvalues());
							qDebug()<<"actual rows < keepn";
							for(int i = 1; i <= m_actualRows; i++) {
								static_cast<QVector<double>*>(m_dataContainer[n])->operator[] (spreadsheet->keepNvalues() - i) =
										static_cast<QVector<double>*>(m_dataContainer[n])->operator[](spreadsheet->keepNvalues() - i - rowDiff);
								qDebug()<< "overwrite row "<<spreadsheet->keepNvalues() - i<<"  "<<spreadsheet->keepNvalues() - i - rowDiff;
								qDebug()<<static_cast<QVector<double>*>(m_dataContainer[n])->operator[](spreadsheet->keepNvalues() - i - rowDiff);
							}
							for(int i = 0; i < rowDiff; i++) {
								static_cast<QVector<double>*>(m_dataContainer[n])->operator[](i) = nanValue;
								qDebug()<<i<<" row = null";
							}
						}
						break;
					}
					case AbstractColumn::Integer: {
						QVector<int>* vector = static_cast<QVector<int>* >(spreadsheet->child<Column>(n)->data());
						m_dataContainer[n] = static_cast<void *>(vector);

						if(m_actualRows > spreadsheet->keepNvalues()) {
							for(int i = 0; i < spreadsheet->keepNvalues(); i++) {
								static_cast<QVector<int>*>(m_dataContainer[n])->operator[] (i) =
										static_cast<QVector<int>*>(m_dataContainer[n])->operator[](m_actualRows - spreadsheet->keepNvalues() + i);
								qDebug()<< "overwrite row "<<i<<"  "<<m_actualRows - spreadsheet->keepNvalues() + i;
							}
						}
						if(m_actualRows < spreadsheet->keepNvalues()) {
							vector->reserve( spreadsheet->keepNvalues());
							vector->resize( spreadsheet->keepNvalues());
							for(int i = 1; i <= m_actualRows; i++) {
								static_cast<QVector<int>*>(m_dataContainer[n])->operator[] (spreadsheet->keepNvalues() - i) =
										static_cast<QVector<int>*>(m_dataContainer[n])->operator[](spreadsheet->keepNvalues() - i - rowDiff);
								qDebug()<< "overwrite row"<<spreadsheet->keepNvalues() - i<<"  "<<spreadsheet->keepNvalues() - i - rowDiff;
								qDebug()<<static_cast<QVector<int>*>(m_dataContainer[n])->operator[](spreadsheet->keepNvalues() - i - rowDiff);
							}
							for(int i = 0; i < rowDiff; i++){
								static_cast<QVector<int>*>(m_dataContainer[n])->operator[](i) = 0;
								qDebug()<<i<<"row = null";
							}

						}
						break;
					}
					case AbstractColumn::Text: {
						QVector<QString>* vector = static_cast<QVector<QString>*>(spreadsheet->child<Column>(n)->data());
						m_dataContainer[n] = static_cast<void *>(vector);

						if(m_actualRows > spreadsheet->keepNvalues()) {
							for(int i = 0; i < spreadsheet->keepNvalues(); i++) {
								static_cast<QVector<QString>*>(m_dataContainer[n])->operator[] (i) =
										static_cast<QVector<QString>*>(m_dataContainer[n])->operator[](m_actualRows - spreadsheet->keepNvalues() + i);
								qDebug()<< "overwrite row"<<i<<"  "<<m_actualRows - spreadsheet->keepNvalues() + i;
							}
						}

						if(m_actualRows < spreadsheet->keepNvalues()) {
							vector->reserve( spreadsheet->keepNvalues());
							vector->resize( spreadsheet->keepNvalues());
							for(int i = 1; i <= m_actualRows; i++) {
								static_cast<QVector<QString>*>(m_dataContainer[n])->operator[] (spreadsheet->keepNvalues() - i) =
										static_cast<QVector<QString>*>(m_dataContainer[n])->operator[](spreadsheet->keepNvalues() - i - rowDiff);
							}
							for(int i = 0; i < rowDiff; i++)
								static_cast<QVector<QString>*>(m_dataContainer[n])->operator[](i) = "";
						}
						break;
					}
					case AbstractColumn::DateTime: {
						QVector<QDateTime>* vector = static_cast<QVector<QDateTime>* >(spreadsheet->child<Column>(n)->data());
						m_dataContainer[n] = static_cast<void *>(vector);

						if(m_actualRows > spreadsheet->keepNvalues()) {
							for(int i = 0; i < spreadsheet->keepNvalues(); i++) {
								static_cast<QVector<QDateTime>*>(m_dataContainer[n])->operator[] (i) =
										static_cast<QVector<QDateTime>*>(m_dataContainer[n])->operator[](m_actualRows - spreadsheet->keepNvalues() + i);
								qDebug()<< "overwrite row"<<i<<"  "<<m_actualRows - spreadsheet->keepNvalues() + i;
							}
						}

						if(m_actualRows < spreadsheet->keepNvalues()) {
							vector->reserve( spreadsheet->keepNvalues());
							vector->resize( spreadsheet->keepNvalues());
							for(int i = 1; i <= m_actualRows; i++) {
								static_cast<QVector<QDateTime>*>(m_dataContainer[n])->operator[] (spreadsheet->keepNvalues() - i) =
										static_cast<QVector<QDateTime>*>(m_dataContainer[n])->operator[](spreadsheet->keepNvalues() - i - rowDiff);
							}
							for(int i = 0; i < rowDiff; i++)
								static_cast<QVector<QDateTime>*>(m_dataContainer[n])->operator[](i) = QDateTime();
						}
						break;
					}
						//TODO
					case AbstractColumn::Month:
					case AbstractColumn::Day:
						break;
					}
				}
				if(m_actualRows > spreadsheet->keepNvalues())
					spreadsheet->setRowCount(spreadsheet->keepNvalues());
				m_actualRows = spreadsheet->keepNvalues();
				qDebug()<<"actual rows: "<<m_actualRows;
			}
		}
	}


	qDebug()<<"starting m_actual rows calculated" << m_actualRows <<"new data size: "<<newData.size();

	int currentRow = 0; // indexes the position in the vector(column)
	int linesToRead = 0;

	if (m_prepared) {
		//increase row count if we don't have a fixed size
		//but only after the preparation step
		if (keepNValues == 0) {
			if (readingType != MQTTClient::ReadingType::TillEnd)
				m_actualRows += qMin(newData.size(), spreadsheet->sampleRate());
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
					//TODO after reading we should skip the next data lines
					//because it's TillEnd actually
				} else
					linesToRead = newLinesTillEnd;
			} else {
				//we read max sample rate number of lines when the reading mode
				//is ContinouslyFixed or FromEnd
				if(spreadsheet->sampleRate() <= spreadsheet->keepNvalues())
					linesToRead = qMin(spreadsheet->sampleRate(), newLinesTillEnd);
				else
					linesToRead = qMin(spreadsheet->keepNvalues(), newLinesTillEnd);
			}
		} else {
			linesToRead = m_actualRows - spreadsheetRowCountBeforeResize;
		}

		if (linesToRead == 0)
			return;

	} else {
		if(keepNValues != 0) {
			linesToRead = newLinesTillEnd > m_actualRows ? m_actualRows : newLinesTillEnd;
		}
		else {
			linesToRead = newLinesTillEnd;
		}
	}
	qDebug()<<"linestoread = "<<linesToRead;

	//new rows/resize columns if we don't have a fixed size
	//TODO if the user changes this value..m_resizedToFixedSize..setResizedToFixedSize
	if (keepNValues == 0) {
		qDebug()<<"---- keepNValues == 0";

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

		for (int n = 0; n < m_actualCols; ++n) {
			// data() returns a void* which is a pointer to any data type (see ColumnPrivate.cpp)
			switch (columnModes[n]) {
			case AbstractColumn::Numeric: {
				QVector<double>* vector = static_cast<QVector<double>* >(spreadsheet->child<Column>(n)->data());
				vector->reserve(m_actualRows);
				vector->resize(m_actualRows);
				m_dataContainer[n] = static_cast<void *>(vector);
				break;
			}
			case AbstractColumn::Integer: {
				QVector<int>* vector = static_cast<QVector<int>* >(spreadsheet->child<Column>(n)->data());
				vector->reserve(m_actualRows);
				vector->resize(m_actualRows);
				m_dataContainer[n] = static_cast<void *>(vector);
				break;
			}
			case AbstractColumn::Text: {
				QVector<QString>* vector = static_cast<QVector<QString>*>(spreadsheet->child<Column>(n)->data());
				vector->reserve(m_actualRows);
				vector->resize(m_actualRows);
				m_dataContainer[n] = static_cast<void *>(vector);
				break;
			}
			case AbstractColumn::DateTime: {
				QVector<QDateTime>* vector = static_cast<QVector<QDateTime>* >(spreadsheet->child<Column>(n)->data());
				vector->reserve(m_actualRows);
				vector->resize(m_actualRows);
				m_dataContainer[n] = static_cast<void *>(vector);
				break;
			}
				//TODO
			case AbstractColumn::Month:
			case AbstractColumn::Day:
				break;
			}
		}
	} else {
		//when we have a fixed size we have to pop sampleRate number of lines if specified
		//here popping, setting currentRow
		if (!m_prepared)
			currentRow = m_actualRows - qMin(newLinesTillEnd, m_actualRows);
		else {
			if (readingType == MQTTClient::ReadingType::TillEnd) {
				if (newLinesTillEnd > m_actualRows)
					currentRow = 0;
				else {
					currentRow = m_actualRows - newLinesTillEnd;
				}
			} else {
				//we read max sample rate number of lines when the reading mode
				//is ContinouslyFixed or FromEnd
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
					case AbstractColumn::Numeric: {
						QVector<double>* vector = static_cast<QVector<double>* >(spreadsheet->child<Column>(col)->data());
						vector->pop_front();
						vector->reserve(m_actualRows);
						vector->resize(m_actualRows);
						m_dataContainer[col] = static_cast<void *>(vector);
						break;
					}
					case AbstractColumn::Integer: {
						QVector<int>* vector = static_cast<QVector<int>* >(spreadsheet->child<Column>(col)->data());
						vector->pop_front();
						vector->reserve(m_actualRows);
						vector->resize(m_actualRows);
						m_dataContainer[col] = static_cast<void *>(vector);
						break;
					}
					case AbstractColumn::Text: {
						QVector<QString>* vector = static_cast<QVector<QString>*>(spreadsheet->child<Column>(col)->data());
						vector->pop_front();
						vector->reserve(m_actualRows);
						vector->resize(m_actualRows);
						m_dataContainer[col] = static_cast<void *>(vector);
						break;
					}
					case AbstractColumn::DateTime: {
						QVector<QDateTime>* vector = static_cast<QVector<QDateTime>* >(spreadsheet->child<Column>(col)->data());
						vector->pop_front();
						vector->reserve(m_actualRows);
						vector->resize(m_actualRows);
						m_dataContainer[col] = static_cast<void *>(vector);
						break;
					}
						//TODO
					case AbstractColumn::Month:
					case AbstractColumn::Day:
						break;
					}
				}
			}
		}
	}

	// from the last row we read the new data in the spreadsheet
	qDebug() << "reading from line: "  << currentRow << " lines till end: " << newLinesTillEnd;
	qDebug() << "Lines to read: " << linesToRead <<" actual rows: " << m_actualRows;
	newDataIdx = 0;
	if (readingType == MQTTClient::ReadingType::FromEnd) {
		if (m_prepared) {
			if (newData.size() > spreadsheet->sampleRate())
				newDataIdx = newData.size() - spreadsheet->sampleRate();
		}
	}
	qDebug() << "newDataIdx: " << newDataIdx;
	//TODO
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
			qDebug ()<<"line  "<<line;
			//when we read the whole file we don't care about the previous position
			//so we don't have to count those bytes


			//qDebug() << "line bytes: " << line.size() << " line: " << line;
			//qDebug() << "reading in row: " << currentRow;
			if (simplifyWhitespacesEnabled)
				line = line.simplified();

			if (line.isEmpty() || line.startsWith(commentCharacter))
			{
				qDebug()<<"found empty line     "<<currentRow;
				continue;
			}

			QLocale locale(numberFormat);

			int col = 0;
			if (createIndexEnabled) {
				col = 1;
				QString tempIndex;
				if (keepNValues != 0)
					tempIndex = QString::number(indexColumnIdx++);
				else
					tempIndex = QString::number(currentRow);
				switch (columnModes[0]) {
				case AbstractColumn::Numeric: {
					bool isNumber;
					const double value = locale.toDouble(tempIndex, &isNumber);
					static_cast<QVector<double>*>(m_dataContainer[0])->operator[](currentRow) = (isNumber ? value : nanValue);
					//qDebug() << "dataContainer[" << n << "] size:" << static_cast<QVector<double>*>(m_dataContainer[n])->size();
					break;
				}
				case AbstractColumn::Integer: {
					bool isNumber;
					const int value = locale.toInt(tempIndex, &isNumber);
					static_cast<QVector<int>*>(m_dataContainer[0])->operator[](currentRow) = (isNumber ? value : 0);
					//qDebug() << "dataContainer[" << n << "] size:" << static_cast<QVector<int>*>(m_dataContainer[n])->size();

					break;
				}
				case AbstractColumn::DateTime: {
					const QDateTime valueDateTime = QDateTime::fromString(tempIndex, dateTimeFormat);
					static_cast<QVector<QDateTime>*>(m_dataContainer[0])->operator[](currentRow) = valueDateTime.isValid() ? valueDateTime : QDateTime();
					break;
				}
				case AbstractColumn::Text:
					if (removeQuotesEnabled)
						tempIndex.remove(QRegExp("[\"\']"));
					static_cast<QVector<QString>*>(m_dataContainer[0])->operator[](currentRow) = tempIndex;
					break;
				case AbstractColumn::Month:
					//TODO
					break;
				case AbstractColumn::Day:
					//TODO
					break;
				}
			}

			//setting timestamp on current time
			static_cast<QVector<QDateTime>*>(m_dataContainer[col])->operator[](currentRow) =  QDateTime::currentDateTime();
			static_cast<QVector<int>*>(m_dataContainer[col+1])->operator[](currentRow) =  QDateTime::currentDateTime().time().msecsSinceStartOfDay();


			QString valueString = line;
			qDebug()<<"putting in this line" << valueString<< "    "<<currentRow;

			// set value depending on data type
			switch (columnModes[m_actualCols - 1]) {
			case AbstractColumn::Numeric: {
				bool isNumber;
				const double value = locale.toDouble(valueString, &isNumber);
				static_cast<QVector<double>*>(m_dataContainer[m_actualCols - 1])->operator[](currentRow) = (isNumber ? value : nanValue);
				//qDebug() << "dataContainer[" << n << "] size:" << static_cast<QVector<double>*>(m_dataContainer[n])->size();
				break;
			}
			case AbstractColumn::Integer: {
				bool isNumber;
				const int value = locale.toInt(valueString, &isNumber);
				static_cast<QVector<int>*>(m_dataContainer[m_actualCols - 1])->operator[](currentRow) = (isNumber ? value : 0);
				//qDebug() << "dataContainer[" << n << "] size:" << static_cast<QVector<int>*>(m_dataContainer[n])->size();

				break;
			}
			case AbstractColumn::DateTime: {
				const QDateTime valueDateTime = QDateTime::fromString(valueString, dateTimeFormat);
				static_cast<QVector<QDateTime>*>(m_dataContainer[m_actualCols - 1])->operator[](currentRow) = valueDateTime.isValid() ? valueDateTime : QDateTime();
				break;
			}
			case AbstractColumn::Text:
				if (removeQuotesEnabled)
					valueString.remove(QRegExp("[\"\']"));
				static_cast<QVector<QString>*>(m_dataContainer[m_actualCols - 1])->operator[](currentRow) = valueString;
				break;
			case AbstractColumn::Month:
				//TODO
				break;
			case AbstractColumn::Day:
				//TODO
				break;
			}


			currentRow++;
			qDebug()<<"adding data is ok, current row increment";
		}
	}

	if (m_prepared) {
		qDebug()<<"notifying plots";
		//notify all affected columns and plots about the changes
		PERFTRACE("AsciiLiveDataImport, notify affected columns and plots");
		qDebug()<<"project ";
		const Project* project = spreadsheet->project();
		qDebug()<<"Curves";
		QVector<const XYCurve*> curves = project->children<const XYCurve>(AbstractAspect::Recursive);
		QVector<CartesianPlot*> plots;

		qDebug()<<"Uploading plots";
		for (int n = 0; n < m_actualCols; ++n) {
			Column* column = spreadsheet->column(n);

			//determine the plots where the column is consumed
			for (const auto* curve: curves) {
				if (curve->xColumn() == column || curve->yColumn() == column) {
					CartesianPlot* plot = dynamic_cast<CartesianPlot*>(curve->parentAspect());
					if (plots.indexOf(plot) == -1) {
						plots << plot;
						plot->setSuppressDataChangedSignal(true);
					}
				}
			}

			column->setChanged();
		}

		//loop over all affected plots and retransform them
		for (auto* const plot: plots) {
			//TODO setting this back to true triggers again a lot of retransforms in the plot (one for each curve).
			// 				plot->setSuppressDataChangedSignal(false);
			plot->dataChanged();
		}
	}

	qDebug()<<"check m_prepared";
	if(!m_prepared)
		m_prepared = true;

	qDebug()<<"return , comes end";
}

int AsciiFilterPrivate::prepareMQTTTopicToRead(const QString& message,  const QString& topic) {
	vectorNames.append("value");
	if (endColumn == -1)
		endColumn = 1;
	else endColumn++;

	vectorNames.prepend("timestamp for plot");
	endColumn++;

	vectorNames.prepend("timestamp");
	endColumn++;

	if (createIndexEnabled) {
		vectorNames.prepend("index");
		endColumn++;
	}
	m_actualCols = endColumn - startColumn + 1;
	qDebug()<<"actual cols"<<m_actualCols;

	QStringList lineList = message.split(QRegExp("\n|\r\n|\r"), QString::SkipEmptyParts);
	QString firstLine = lineList.takeFirst();
	firstLine.remove(QRegExp("[\\n\\r]"));	// remove any newline
	if (simplifyWhitespacesEnabled)
		firstLine = firstLine.simplified();
	DEBUG("First line: \'" << firstLine.toStdString() << '\'');

	// determine separator and split first line
	QStringList firstLineStringList;
	if (separatingCharacter == "auto") {
		DEBUG("automatic separator");
		QRegExp regExp("(\\s+)|(,\\s+)|(;\\s+)|(:\\s+)");
		firstLineStringList = firstLine.split(regExp, (QString::SplitBehavior)skipEmptyParts);

		if (!firstLineStringList.isEmpty()) {
			int length1 = firstLineStringList.at(0).length();
			if (firstLineStringList.size() > 1) {
				int pos2 = firstLine.indexOf(firstLineStringList.at(1), length1);
				m_separator = firstLine.mid(length1, pos2 - length1);
			} else {
				//old: separator = line.right(line.length() - length1);
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
	DEBUG("separator: \'" << m_separator.toStdString() << '\'');
	DEBUG("number of columns: " << firstLineStringList.size());
	QDEBUG("first line: " << firstLineStringList);
	DEBUG("headerEnabled = " << headerEnabled);
	// parse first data line to determine data type for each column
	columnModes.resize(m_actualCols);
	int col = 0;
	if (createIndexEnabled) {
		columnModes[0] = AbstractColumn::Integer;
		col = 1;
	}

	//set column mode for timestamp
	columnModes[col] = AbstractColumn::DateTime;
	col++;

	columnModes[col] = AbstractColumn::Integer;
	col++;


	auto firstValue = firstLineStringList.takeFirst();//use first value to identify column mode
	while(firstValue.isEmpty() || firstValue.startsWith(commentCharacter) )
		firstValue = firstLineStringList.takeFirst();
	if (simplifyWhitespacesEnabled)
		firstValue = firstValue.simplified();
	columnModes[m_actualCols-1] = AbstractFileFilter::columnMode(firstValue, dateTimeFormat, numberFormat);
	qDebug()<<"-----setting column mode" << "   "<<columnModes[m_actualCols-1]<< "based on value  " << firstValue;

	for (auto& valueString : firstLineStringList) {
		if(!valueString.isEmpty() && !valueString.startsWith(commentCharacter) ){
			if (createIndexEnabled)
				col = 1;
			else
				col = 0;

			if (simplifyWhitespacesEnabled)
				valueString = valueString.simplified();
			AbstractColumn::ColumnMode mode = AbstractFileFilter::columnMode(valueString, dateTimeFormat, numberFormat);

			// numeric: integer -> numeric
			if (mode == AbstractColumn::Numeric && columnModes[m_actualCols-1] == AbstractColumn::Integer) {
				columnModes[m_actualCols-1] = mode;
				qDebug()<<"-----setting column mode" << "   "<<columnModes[m_actualCols-1]<< "based on value  " << valueString;
			}
			// text: non text -> text
			if (mode == AbstractColumn::Text && columnModes[m_actualCols-1] != AbstractColumn::Text) {
				columnModes[m_actualCols-1] = mode;
				qDebug()<<"-----setting column mode" << "   "<<columnModes[m_actualCols-1]<< "based on value  " << valueString;
			}
		}
	}
	for (auto& valueString: lineList) {
		QStringList splitString = valueString.split(m_separator, (QString::SplitBehavior)skipEmptyParts);
		for (auto& valueString2: splitString) {
			if (!valueString2.isEmpty() && !valueString2.startsWith(commentCharacter)) {

				if (simplifyWhitespacesEnabled)
					valueString2 = valueString2.simplified();
				AbstractColumn::ColumnMode mode = AbstractFileFilter::columnMode(valueString2, dateTimeFormat, numberFormat);

				// numeric: integer -> numeric
				if (mode == AbstractColumn::Numeric && columnModes[m_actualCols-1] == AbstractColumn::Integer) {
					columnModes[m_actualCols-1] = mode;
					qDebug()<<"-----setting column mode" << "   "<<columnModes[m_actualCols-1]<< "based on value  " << valueString;
				}
				// text: non text -> text
				if (mode == AbstractColumn::Text && columnModes[m_actualCols-1] != AbstractColumn::Text) {
					columnModes[m_actualCols-1] = mode;
					qDebug()<<"-----setting column mode" << "   "<<columnModes[m_actualCols-1]<< "based on value  " << valueString;
				}
			}
		}
	}

	int tempRowCount = 0;
	QStringList newDataList = message.split(QRegExp("\n|\r\n|\r"), QString::SkipEmptyParts);
	for (auto& valueString: newDataList) {
		QStringList splitString = valueString.split(m_separator, (QString::SplitBehavior)skipEmptyParts);
		for (auto& valueString2: splitString) {
			if (!valueString2.isEmpty() && !valueString2.startsWith(commentCharacter)) {
				tempRowCount ++;
			}
		}
	}



	QDEBUG("column modes = " << columnModes);

	// ATTENTION: This resets the position in the device to 0
	//m_actualRows = (int)AsciiFilter::lineNumber(device);
	m_actualRows = tempRowCount;

	/////////////////////////////////////////////////////////////////

	int actualEndRow = endRow;
	DEBUG("endRow = " << endRow);
	if (endRow == -1 || endRow > m_actualRows)
		actualEndRow = m_actualRows;

	if (m_actualRows > actualEndRow)
		m_actualRows = actualEndRow;


	DEBUG("start/end column: " << startColumn << ' ' << endColumn);
	DEBUG("start/end row: " << m_actualStartRow << ' ' << actualEndRow);
	DEBUG("actual cols/rows (w/o header incl. start rows): " << m_actualCols << ' ' << m_actualRows);


	qDebug()<<"-----final column mode" << "   "<<columnModes[m_actualCols-1];

	return 0;
}

void AsciiFilter::setPreparedForMQTT(bool prepared, AbstractDataSource* dataSource, const QString& separator) {
	d->setPreparedForMQTT(prepared, dataSource, separator);
}

void AsciiFilterPrivate::setPreparedForMQTT(bool prepared, AbstractDataSource* dataSource, const QString& separator) {
	m_prepared = prepared;
	if(prepared == true) {
		m_separator = separator;
		MQTTTopic* topic = dynamic_cast<MQTTTopic*>(dataSource);
		//m_actualRows = endRow - startRow + 1;
		m_actualCols = endColumn - startColumn + 1;
		m_actualRows = topic->rowCount();
		columnModes.resize(topic->columnCount());
		for(int i = 0; i < topic->columnCount(); ++i) {
			columnModes[i] = topic->column(i)->columnMode();
		}
		m_dataContainer.resize(m_actualCols);
		for (int n = 0; n < m_actualCols; ++n) {
			// data() returns a void* which is a pointer to any data type (see ColumnPrivate.cpp)
			topic->child<Column>(n)->setColumnMode(columnModes[n]);
			switch (columnModes[n]) {
			case AbstractColumn::Numeric: {
				QVector<double>* vector = static_cast<QVector<double>* >(topic->child<Column>(n)->data());
				vector->reserve(m_actualRows);
				vector->resize(m_actualRows);
				m_dataContainer[n] = static_cast<void *>(vector);
				break;
			}
			case AbstractColumn::Integer: {
				QVector<int>* vector = static_cast<QVector<int>* >(topic->child<Column>(n)->data());
				vector->reserve(m_actualRows);
				vector->resize(m_actualRows);
				m_dataContainer[n] = static_cast<void *>(vector);
				break;
			}
			case AbstractColumn::Text: {
				QVector<QString>* vector = static_cast<QVector<QString>*>(topic->child<Column>(n)->data());
				vector->reserve(m_actualRows);
				vector->resize(m_actualRows);
				m_dataContainer[n] = static_cast<void *>(vector);
				break;
			}
			case AbstractColumn::DateTime: {
				QVector<QDateTime>* vector = static_cast<QVector<QDateTime>* >(topic->child<Column>(n)->data());
				vector->reserve(m_actualRows);
				vector->resize(m_actualRows);
				m_dataContainer[n] = static_cast<void *>(vector);
				break;
			}
				//TODO
			case AbstractColumn::Month:
			case AbstractColumn::Day:
				break;
			}
		}
	}
}

QString AsciiFilter::separator() const {
	return d->separator();
}

QString AsciiFilterPrivate::separator() const {
	return m_separator;
}

#endif
