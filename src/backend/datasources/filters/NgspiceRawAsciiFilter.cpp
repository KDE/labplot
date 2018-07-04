/***************************************************************************
File                 : NgspiceRawAsciiFilter.cpp
Project              : LabPlot
Description          : Ngspice RAW ASCII filter
--------------------------------------------------------------------
Copyright            : (C) 2018 Alexander Semke (alexander.semke@web.de)
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
#include "backend/datasources/filters/NgspiceRawAsciiFilter.h"
#include "backend/datasources/filters/NgspiceRawAsciiFilterPrivate.h"
#include "backend/lib/trace.h"

#include <QFile>

/*!
\class NgspiceRawAsciiFilter
\brief Import of data stored in Ngspice's raw formant, ASCCI version of it.

\ingroup datasources
*/
NgspiceRawAsciiFilter::NgspiceRawAsciiFilter() : AbstractFileFilter(), d(new NgspiceRawAsciiFilterPrivate(this)) {}

NgspiceRawAsciiFilter::~NgspiceRawAsciiFilter() {}

bool NgspiceRawAsciiFilter::isNgspiceAsciiFile(const QString& fileName) {
	QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		DEBUG("Failed to open the file " << fileName.toStdString());
		return false;
	}

	QString line = file.readLine();
	if (!line.startsWith(QLatin1String("Title:")))
		return false;

	line = file.readLine();
	if (!line.startsWith(QLatin1String("Date:")))
		return false;

	line = file.readLine();
	if (!line.startsWith(QLatin1String("Plotname:")))
		return false;

	line = file.readLine();
	if (!line.startsWith(QLatin1String("Flags:")))
		return false;

	line = file.readLine();
	if (!line.startsWith(QLatin1String("No. Variables:")))
		return false;

	line = file.readLine();
	if (!line.startsWith(QLatin1String("No. Points:")))
		return false;

	line = file.readLine();
	if (!line.startsWith(QLatin1String("Variables:")))
		return false;

	return true;
}

QString NgspiceRawAsciiFilter::fileInfoString(const QString& fileName) {
	QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return QString();

	QString info;
	while (!file.atEnd()) {
		QString line = file.readLine();
		if (line.simplified() == QLatin1String("Values:"))
			break;

		if (!info.isEmpty())
			info += QLatin1String("<br>");

		info += line;
	}

	return info;
}


/*!
  reads the content of the file \c fileName.
*/
QVector<QStringList> NgspiceRawAsciiFilter::readDataFromFile(const QString& fileName, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode importMode, int lines) {
	d->readDataFromFile(fileName, dataSource, importMode, lines);
	return QVector<QStringList>();  //TODO: remove this later once all read*-functions in the filter classes don't return any preview strings anymore
}

QVector<QStringList> NgspiceRawAsciiFilter::preview(const QString& fileName, int lines) {
	return d->preview(fileName, lines);
}

/*!
  writes the content of the data source \c dataSource to the file \c fileName.
*/
void NgspiceRawAsciiFilter::write(const QString& fileName, AbstractDataSource* dataSource) {
	d->write(fileName, dataSource);
}

/*!
  loads the predefined filter settings for \c filterName
*/
void NgspiceRawAsciiFilter::loadFilterSettings(const QString& filterName) {
	Q_UNUSED(filterName);
}

/*!
  saves the current settings as a new filter with the name \c filterName
*/
void NgspiceRawAsciiFilter::saveFilterSettings(const QString& filterName) const {
	Q_UNUSED(filterName);
}


void NgspiceRawAsciiFilter::setStartRow(const int r) {
	d->startRow = r;
}
int NgspiceRawAsciiFilter::startRow() const {
	return d->startRow;
}

void NgspiceRawAsciiFilter::setEndRow(const int r) {
	d->endRow = r;
}
int NgspiceRawAsciiFilter::endRow() const {
	return d->endRow;
}

void NgspiceRawAsciiFilter::setStartColumn(const int c) {
	d->startColumn = c;
}
int NgspiceRawAsciiFilter::startColumn() const {
	return d->startColumn;
}

void NgspiceRawAsciiFilter::setEndColumn(const int c) {
	d->endColumn = c;
}
int NgspiceRawAsciiFilter::endColumn() const {
	return d->endColumn;
}

QStringList NgspiceRawAsciiFilter::vectorNames() const {
	return d->vectorNames;
}

QVector<AbstractColumn::ColumnMode> NgspiceRawAsciiFilter::columnModes() {
	return d->columnModes;
}

//#####################################################################
//################### Private implementation ##########################
//#####################################################################
NgspiceRawAsciiFilterPrivate::NgspiceRawAsciiFilterPrivate(NgspiceRawAsciiFilter* owner) : q(owner),
	startRow(1),
	endRow(-1),
	startColumn(1),
	endColumn(-1),
	m_actualStartRow(1),
	m_actualRows(0),
	m_actualCols(0),
	m_prepared(false),
	m_columnOffset(0) {
}

/*!
    reads the content of the file \c fileName to the data source \c dataSource. Uses the settings defined in the data source.
*/
void NgspiceRawAsciiFilterPrivate::readDataFromFile(const QString& fileName, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode importMode, int lines) {
	DEBUG("NgspiceRawAsciiFilterPrivate::readDataFromFile(): fileName = \'" << fileName.toStdString() << "\', dataSource = "
	      << dataSource << ", mode = " << ENUM_TO_STRING(AbstractFileFilter, ImportMode, importMode) << ", lines = " << lines);

}

/*!
 * generates the preview for the file \c fileName reading the provided number of \c lines.
 */
QVector<QStringList> NgspiceRawAsciiFilterPrivate::preview(const QString& fileName, int lines) {
	QVector<QStringList> dataStrings;

	QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		DEBUG("Failed to open the file " << fileName.toStdString());
		return dataStrings;
	}

	//skip the first four lines in the header
	file.readLine();
	file.readLine();
	file.readLine();
	file.readLine();

	//number of variables
	QString line = file.readLine();
	int vars = line.right(line.length() - 15).toInt(); //remove the "No. Variables: " sub-string

	//number of points
	line = file.readLine();
	int points = line.right(line.length() - 12).toInt(); //remove the "No. Points: " sub-string

	//add names of the variables
	vectorNames.clear();
	columnModes.clear();
	file.readLine();
	for (int i = 0; i<vars; ++i) {
		line = file.readLine();
		QStringList tokens = line.split('\t');
		vectorNames << tokens.at(2) + QLatin1String(", ") + tokens.at(3).simplified();
		columnModes << AbstractColumn::Numeric;
	}

	file.readLine(); //skip the line with "Values"

	//read the first value to check whether we have complex numbers
	qint64 pos = file.pos();
	line = file.readLine();
	bool hasComplexValues = (line.indexOf(QLatin1Char(',')) != -1);
	if (hasComplexValues) {
		//add column names and types for the imaginary parts
		QStringList newVectorNames;
		for (int i = 0; i<vars; ++i) {
			columnModes << AbstractColumn::Numeric;
			newVectorNames << vectorNames.at(i) + QLatin1String(" REAL");
			newVectorNames << vectorNames.at(i) + QLatin1String(" IMAGINARY");
		}
		vectorNames = newVectorNames;
	}
	file.seek(pos);

	//add the data points
	QStringList lineString;
	for (int i = 0; i< qMin(lines, points); ++i) {
		lineString.clear();
		for (int j = 0; j < vars; ++j) {
			line = file.readLine();
			QStringList tokens = line.split(QLatin1Char('\t'));
			QString value = tokens.at(1).simplified(); //string containing the value(s)
			if (hasComplexValues) {
				QStringList realImgTokens = value.split(QLatin1Char(','));
				if (realImgTokens.size() == 2) { //sanity check to make sure we really have both parts
					lineString << realImgTokens.at(0); //real part
					lineString << realImgTokens.at(0); //imaginary part
				}
			} else
				lineString << value;
		}

		dataStrings << lineString;
		file.readLine(); //skip the empty line after each value block
	}

	return dataStrings;
}

/*!
    writes the content of \c dataSource to the file \c fileName.
*/
void NgspiceRawAsciiFilterPrivate::write(const QString & fileName, AbstractDataSource* dataSource) {
	Q_UNUSED(fileName);
	Q_UNUSED(dataSource);
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################
/*!
  Saves as XML.
 */
void NgspiceRawAsciiFilter::save(QXmlStreamWriter* writer) const {
	Q_UNUSED(writer);
}

/*!
  Loads from XML.
*/
bool NgspiceRawAsciiFilter::load(XmlStreamReader* reader) {
	Q_UNUSED(reader);
	return true;
}
