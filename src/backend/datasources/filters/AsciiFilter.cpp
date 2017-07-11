/***************************************************************************
File                 : AsciiFilter.cpp
Project              : LabPlot
Description          : ASCII I/O-filter
--------------------------------------------------------------------
Copyright            : (C) 2009-2017 Stefan Gerlach (stefan.gerlach@uni.kn)
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
#include "backend/datasources/FileDataSource.h"
#include "backend/core/column/Column.h"
#include "backend/datasources/filters/AsciiFilter.h"
#include "backend/datasources/filters/AsciiFilterPrivate.h"
#include "backend/lib/macros.h"

#include <QTextStream>
#include <KLocale>
#include <KFilterDev>
#include <QElapsedTimer>
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
		<< ",TAB" << ";TAB" << ":TAB" << ",SPACE" << ";SPACE" << ":SPACE");
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
	for (int i = 0; i <= 100; i++)	// me.keyCount() does not work because we have holes in enum
		if (me.valueToKey(i))
			list << me.valueToKey(i);
	return list;
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
		DEBUG("Could not open file " << fileName.toStdString() << " for determining number of lines");
		return 0;
	}

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
  returns the number of lines in the device \c device or 0 if not available.
  resets the position to 0!
*/
size_t AsciiFilter::lineNumber(QIODevice &device) {
	// device.hasReadLine() always returns 0 for KFilterDev!
	if (device.isSequential())
		return 0;

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

void AsciiFilter::setNumbersFormat(AbstractFileFilter::Locale locale) {
	d->locale = locale;
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

void AsciiFilter::setSimplifyWhitespacesEnabled(bool b) {
	d->simplifyWhitespacesEnabled = b;
}
bool AsciiFilter::simplifyWhitespacesEnabled() const {
	return d->simplifyWhitespacesEnabled;
}

void AsciiFilter::setVectorNames(const QString s) {
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
	autoModeEnabled(true),
	headerEnabled(true),
	skipEmptyParts(false),
	simplifyWhitespacesEnabled(true),
	startRow(1),
	endRow(-1),
	startColumn(1),
	endColumn(-1),
	m_prepared(false),
	m_columnOffset(0) {
}

/*!
 * returns -1 if the device couldn't be opened, 1 if the current read position in the device is at the end and 0 otherwise.
 */
int AsciiFilterPrivate::prepareDeviceToRead(QIODevice& device) {
	if (!device.open(QIODevice::ReadOnly))
		return -1;

	if (device.atEnd()) // empty file
		return 1;

	// Parse the first line:
	// Determine the number of columns, create the columns and use (if selected) the first row to name them
	QString firstLine;
	do {	// skip comment lines
		firstLine = device.readLine();
		if (device.atEnd())
			return 1;
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
		firstLineStringList = firstLine.split(regExp, QString::SkipEmptyParts);

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
		m_separator = separatingCharacter.replace(QLatin1String("TAB"), "\t", Qt::CaseInsensitive);
		// replace symbolic "SPACE" with ' '
		m_separator = m_separator.replace(QLatin1String("SPACE"), QLatin1String(" "), Qt::CaseInsensitive);
		firstLineStringList = firstLine.split(m_separator, QString::SkipEmptyParts);
	}
	DEBUG("separator: \'" << m_separator.toStdString() << '\'');
	DEBUG("number of columns: " << firstLineStringList.size());
	DEBUG("headerEnabled = " << headerEnabled);

	if (headerEnabled) {	// use first line to name vectors
		vectorNames = firstLineStringList;
		QDEBUG("vector names =" << vectorNames);
		startRow++;
	}

	// set range to read
	if (endColumn == -1)
		endColumn = firstLineStringList.size(); // last column
	m_actualCols = endColumn - startColumn + 1;

//TEST: readline-seek-readline fails
/*	qint64 testpos = device.pos();
	DEBUG("read data line @ pos " << testpos << " : " << device.readLine().toStdString());
	device.seek(testpos);
	testpos = device.pos();
	DEBUG("read data line again @ pos " << testpos << "  : " << device.readLine().toStdString());
*/
	// this also resets position to start of file
	m_actualRows = AsciiFilter::lineNumber(device);

	// Find first data line (ignoring comment lines)
	DEBUG("Skipping " << startRow - 1 << " lines");
	for (int i = 0; i < startRow - 1; i++) {
		QString line = device.readLine();

		if (device.atEnd())
			return 1;
		if (line.startsWith(commentCharacter))	// ignore commented lines
			i--;
	}

	// parse first data line to determine data type for each column
	firstLine = device.readLine();
	firstLine.remove(QRegExp("[\\n\\r]"));	// remove any newline
	if (simplifyWhitespacesEnabled)
		firstLine = firstLine.simplified();
	DEBUG("first data line : \'" << firstLine.toStdString() << '\'');
	firstLineStringList = firstLine.split(m_separator, QString::SkipEmptyParts);
	columnModes.resize(m_actualCols);
	int col = 0;
	for (const auto& valueString: firstLineStringList) {// only parse columns available in first data line
		if (col == m_actualCols)
			break;
		columnModes[col++] = AbstractFileFilter::columnMode(valueString, dateTimeFormat, locale);
	}
	QDEBUG("column modes = " << columnModes);

	int actualEndRow = endRow;
	DEBUG("endRow = " << endRow);
	if (endRow == -1 || endRow > m_actualRows)
		actualEndRow = m_actualRows;

	if (m_actualRows > actualEndRow)
		m_actualRows = actualEndRow;

	// reset to start of file
	device.seek(0);

	DEBUG("start/end column: " << startColumn << ' ' << endColumn);
	DEBUG("start/end row: " << startRow << ' ' << actualEndRow);
	DEBUG("actual cols/rows (w/o header incl. start rows): " << m_actualCols << ' ' << m_actualRows);

	if (m_actualRows == 0)
		return 1;

	return 0;
}

/*!
    reads the content of the file \c fileName to the data source \c dataSource. Uses the settings defined in the data source.
*/
void AsciiFilterPrivate::readDataFromFile(const QString& fileName, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode importMode, int lines) {
	DEBUG("AsciiFilterPrivate::readDataFromFile(): fileName = \'" << fileName.toStdString() << "\', dataSource = " << dataSource
		<< ", mode = " << ENUM_TO_STRING(AbstractFileFilter, ImportMode, importMode) << ", lines = " << lines);

	KFilterDev device(fileName);
	readDataFromDevice(device, dataSource, importMode, lines);
}

/*!
    reads the content of device \c device to the data source \c dataSource. Uses the settings defined in the data source.
*/
void AsciiFilterPrivate::readDataFromDevice(QIODevice& device, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode importMode, int lines) {
	DEBUG("AsciiFilterPrivate::readDataFromDevice(): dataSource = " << dataSource
		<< ", mode = " << ENUM_TO_STRING(AbstractFileFilter, ImportMode, importMode) << ", lines = " << lines);
	Q_ASSERT(dataSource != nullptr);

	if (!m_prepared) {
		DEBUG("device is sequential = " << device.isSequential());
		const int deviceError = prepareDeviceToRead(device);
		if (deviceError != 0)
			DEBUG("Device error = " << deviceError);

		if (deviceError == 1 && importMode == AbstractFileFilter::Replace && dataSource)
			dataSource->clear();
		if (deviceError)
			return;

		if (dynamic_cast<Matrix*>(dataSource)) {
			// avoid text data in Matrix
			for (auto& c: columnModes)
				if (c == AbstractColumn::Text)
					c = AbstractColumn::Numeric;
		}

		m_columnOffset = dataSource->prepareImport(m_dataContainer, importMode, m_actualRows - startRow + 1,
							m_actualCols, vectorNames, columnModes);

		//number formatting
		if (locale == AbstractFileFilter::LocaleC)
			m_numberFormat = QLocale(QLocale::C);
		else
			m_numberFormat = QLocale::system();

		m_prepared = true;
	}

	// Read the data
	int currentRow = 0;	// indexes the position in the vector(column)
	if (lines == -1)
		lines = m_actualRows;

	DEBUG("reading " << qMin(lines, m_actualRows)  << " lines");
	for (int i = 0; i < qMin(lines, m_actualRows); i++) {
		QString line = device.readLine();
		if (simplifyWhitespacesEnabled)
			line = line.simplified();

		if (line.isEmpty() || line.startsWith(commentCharacter)) // skip empty or commented lines
			continue;

		if (startRow > 1) {	// skip start lines
			startRow--;
			continue;
		}

		QStringList lineStringList = line.split(m_separator, QString::SkipEmptyParts);
		QStringList lineString;
		for (int n = 0; n < m_actualCols; n++) {
			if (n < lineStringList.size()) {
				const QString& valueString = lineStringList.at(n);

				// set value depending on data type
				switch (columnModes[n]) {
				case AbstractColumn::Numeric: {
					bool isNumber;
					const double value = m_numberFormat.toDouble(valueString, &isNumber);
					static_cast<QVector<double>*>(m_dataContainer[n])->operator[](currentRow) = (isNumber ? value : NAN);
					break;
				}
				case AbstractColumn::DateTime: {
					const QDateTime valueDateTime = QDateTime::fromString(valueString, dateTimeFormat);
					static_cast<QVector<QDateTime>*>(m_dataContainer[n])->operator[](currentRow) = valueDateTime.isValid() ? valueDateTime : QDateTime();
					break;
				}
				case AbstractColumn::Text:
					static_cast<QVector<QString>*>(m_dataContainer[n])->operator[](currentRow) = valueString;
					break;
				case AbstractColumn::Month:
					//TODO
					break;
				case AbstractColumn::Day:
					//TODO
					break;
				}
			} else {	// missing columns in this line
				if (dataSource) {
					switch (columnModes[n]) {
					case AbstractColumn::Numeric:
						static_cast<QVector<double>*>(m_dataContainer[n])->operator[](currentRow) = NAN;
						break;
					case AbstractColumn::DateTime:
						static_cast<QVector<QDateTime>*>(m_dataContainer[n])->operator[](currentRow) = QDateTime();
						break;
					case AbstractColumn::Text:
						static_cast<QVector<QString>*>(m_dataContainer[n])->operator[](currentRow) = "NAN";
						break;
					case AbstractColumn::Month:
						//TODO
						break;
					case AbstractColumn::Day:
						//TODO
						break;
					}
				} else
					lineString += QLatin1String("NAN");
			}
		}

		currentRow++;
		emit q->completed(100 * currentRow/m_actualRows);
	}

	dataSource->finalizeImport(m_columnOffset, startColumn, endColumn, dateTimeFormat, importMode);
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
	if (locale == AbstractFileFilter::LocaleC)
		m_numberFormat = QLocale(QLocale::C);
	else
		m_numberFormat = QLocale::system();

	// Read the data
	if (lines == -1)
		lines = m_actualRows;

	DEBUG("generating preview for " << qMin(lines, m_actualRows)  << " lines");
	for (int i = 0; i < qMin(lines, m_actualRows); i++) {
		QString line = device.readLine();
		if (simplifyWhitespacesEnabled)
			line = line.simplified();

		if (line.isEmpty() || line.startsWith(commentCharacter)) // skip empty or commented lines
			continue;

		if (startRow > 1) {	// skip start lines
			startRow--;
			continue;
		}

		QStringList lineStringList = line.split(m_separator, QString::SkipEmptyParts);
		QStringList lineString;
		for (int n = 0; n < m_actualCols; n++) {
			if (n < lineStringList.size()) {
				const QString& valueString = lineStringList.at(n);

				// set value depending on data type
				switch (columnModes[n]) {
				case AbstractColumn::Numeric: {
					bool isNumber;
					const double value = m_numberFormat.toDouble(valueString, &isNumber);
					lineString += QString::number(isNumber ? value : NAN);
					break;
				}
				case AbstractColumn::DateTime: {
					const QDateTime valueDateTime = QDateTime::fromString(valueString, dateTimeFormat);
					lineString += valueDateTime.isValid() ? valueDateTime.toString(dateTimeFormat) : QLatin1String(" ");
					break;
				}
				case AbstractColumn::Text:
					lineString += valueString;
					break;
				case AbstractColumn::Month:
					//TODO
					break;
				case AbstractColumn::Day:
					//TODO
					break;
				}
			} else {	// missing columns in this line
				lineString += QLatin1String("NAN");
			}
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
	//TODO
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################
/*!
  Saves as XML.
 */
void AsciiFilter::save(QXmlStreamWriter* writer) const {
	writer->writeStartElement( "asciiFilter" );
	writer->writeAttribute( "commentCharacter", d->commentCharacter );
	writer->writeAttribute( "separatingCharacter", d->separatingCharacter );
	writer->writeAttribute( "autoMode", QString::number(d->autoModeEnabled) );
	writer->writeAttribute( "header", QString::number(d->headerEnabled) );
	writer->writeAttribute( "vectorNames", d->vectorNames.join(' '));
	writer->writeAttribute( "skipEmptyParts", QString::number(d->skipEmptyParts) );
	writer->writeAttribute( "simplifyWhitespaces", QString::number(d->simplifyWhitespacesEnabled) );
	writer->writeAttribute( "startRow", QString::number(d->startRow) );
	writer->writeAttribute( "endRow", QString::number(d->endRow) );
	writer->writeAttribute( "startColumn", QString::number(d->startColumn) );
	writer->writeAttribute( "endColumn", QString::number(d->endColumn) );
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

	QString attributeWarning = i18n("Attribute '%1' missing or empty, default value is used");
	QXmlStreamAttributes attribs = reader->attributes();

	QString str = attribs.value("commentCharacter").toString();
	if (str.isEmpty())
		reader->raiseWarning(attributeWarning.arg("'commentCharacter'"));
	else
		d->commentCharacter = str;

	str = attribs.value("separatingCharacter").toString();
	if (str.isEmpty())
		reader->raiseWarning(attributeWarning.arg("'separatingCharacter'"));
	else
		d->separatingCharacter = str;

	str = attribs.value("autoMode").toString();
	if (str.isEmpty())
		reader->raiseWarning(attributeWarning.arg("'autoMode'"));
	else
		d->autoModeEnabled = str.toInt();

	str = attribs.value("header").toString();
	if (str.isEmpty())
		reader->raiseWarning(attributeWarning.arg("'header'"));
	else
		d->headerEnabled = str.toInt();

	str = attribs.value("vectorNames").toString();
	d->vectorNames = str.split(' '); //may be empty

	str = attribs.value("simplifyWhitespaces").toString();
	if (str.isEmpty())
		reader->raiseWarning(attributeWarning.arg("'simplifyWhitespaces'"));
	else
		d->simplifyWhitespacesEnabled = str.toInt();

	str = attribs.value("skipEmptyParts").toString();
	if (str.isEmpty())
		reader->raiseWarning(attributeWarning.arg("'skipEmptyParts'"));
	else
		d->skipEmptyParts = str.toInt();

	str = attribs.value("startRow").toString();
	if (str.isEmpty())
		reader->raiseWarning(attributeWarning.arg("'startRow'"));
	else
		d->startRow = str.toInt();

	str = attribs.value("endRow").toString();
	if (str.isEmpty())
		reader->raiseWarning(attributeWarning.arg("'endRow'"));
	else
		d->endRow = str.toInt();

	str = attribs.value("startColumn").toString();
	if (str.isEmpty())
		reader->raiseWarning(attributeWarning.arg("'startColumn'"));
	else
		d->startColumn = str.toInt();

	str = attribs.value("endColumn").toString();
	if (str.isEmpty())
		reader->raiseWarning(attributeWarning.arg("'endColumn'"));
	else
		d->endColumn = str.toInt();

	return true;
}
