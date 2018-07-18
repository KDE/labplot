/***************************************************************************
File                 : NgspiceRawBinaryFilter.cpp
Project              : LabPlot
Description          : Ngspice RAW Binary filter
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
#include "backend/datasources/filters/NgspiceRawBinaryFilter.h"
#include "backend/datasources/filters/NgspiceRawBinaryFilterPrivate.h"
#include "backend/lib/trace.h"

#include <QFile>

/*!
\class NgspiceRawBinaryFilter
\brief Import of data stored in Ngspice's raw formant, ASCCI version of it.

\ingroup datasources
*/
NgspiceRawBinaryFilter::NgspiceRawBinaryFilter() : AbstractFileFilter(), d(new NgspiceRawBinaryFilterPrivate(this)) {}

NgspiceRawBinaryFilter::~NgspiceRawBinaryFilter() {}

bool NgspiceRawBinaryFilter::isNgspiceBinaryFile(const QString& fileName) {
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

QString NgspiceRawBinaryFilter::fileInfoString(const QString& fileName) {
	QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return QString();

	QString info;
	while (!file.atEnd()) {
		QString line = file.readLine();
		if (line.simplified() == QLatin1String("Binary:"))
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
void NgspiceRawBinaryFilter::readDataFromFile(const QString& fileName, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode importMode) {
	d->readDataFromFile(fileName, dataSource, importMode);
}

QVector<QStringList> NgspiceRawBinaryFilter::preview(const QString& fileName, int lines) {
	return d->preview(fileName, lines);
}

/*!
  writes the content of the data source \c dataSource to the file \c fileName.
*/
void NgspiceRawBinaryFilter::write(const QString& fileName, AbstractDataSource* dataSource) {
	d->write(fileName, dataSource);
}

/*!
  loads the predefined filter settings for \c filterName
*/
void NgspiceRawBinaryFilter::loadFilterSettings(const QString& filterName) {
	Q_UNUSED(filterName);
}

/*!
  saves the current settings as a new filter with the name \c filterName
*/
void NgspiceRawBinaryFilter::saveFilterSettings(const QString& filterName) const {
	Q_UNUSED(filterName);
}

void NgspiceRawBinaryFilter::setStartRow(const int r) {
	d->startRow = r;
}
int NgspiceRawBinaryFilter::startRow() const {
	return d->startRow;
}

void NgspiceRawBinaryFilter::setEndRow(const int r) {
	d->endRow = r;
}
int NgspiceRawBinaryFilter::endRow() const {
	return d->endRow;
}

QStringList NgspiceRawBinaryFilter::vectorNames() const {
	return d->vectorNames;
}

QVector<AbstractColumn::ColumnMode> NgspiceRawBinaryFilter::columnModes() {
	return d->columnModes;
}

//#####################################################################
//################### Private implementation ##########################
//#####################################################################
NgspiceRawBinaryFilterPrivate::NgspiceRawBinaryFilterPrivate(NgspiceRawBinaryFilter* owner) : q(owner),
	startRow(1),
	endRow(-1) {
}

/*!
    reads the content of the file \c fileName to the data source \c dataSource. Uses the settings defined in the data source.
*/
void NgspiceRawBinaryFilterPrivate::readDataFromFile(const QString& fileName, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode importMode) {
	DEBUG("NgspiceRawBinaryFilterPrivate::readDataFromFile(): fileName = \'" << fileName.toStdString() << "\', dataSource = "
	      << dataSource << ", mode = " << ENUM_TO_STRING(AbstractFileFilter, ImportMode, importMode));

	QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		DEBUG("Failed to open the file " << fileName.toStdString());
		return;
	}

	//skip the first three lines in the header
	file.readLine(); //"Title"
	file.readLine(); //"Date"
	file.readLine(); //"Plotname"

	//evaluate the "Flags" line to check whether we have complex numbers
	QString line = file.readLine();
	bool hasComplexValues = line.endsWith(QLatin1String("complex\n"));

	//number of variables
	line = file.readLine();
	const int vars = line.right(line.length() - 15).toInt(); //remove the "No. Variables: " sub-string

	//number of points
	line = file.readLine();
	const int points = line.right(line.length() - 12).toInt(); //remove the "No. Points: " sub-string

	//add names of the variables
	vectorNames.clear();
	columnModes.clear();
	file.readLine();
	for (int i = 0; i<vars; ++i) {
		line = file.readLine();
		QStringList tokens = line.split('\t');
		QString name = tokens.at(2) + QLatin1String(", ") + tokens.at(3).simplified();
		if (hasComplexValues) {
			vectorNames << name + QLatin1String(" REAL");
			vectorNames << name + QLatin1String(" IMAGINARY");
			columnModes << AbstractColumn::Numeric;
			columnModes << AbstractColumn::Numeric;
		} else {
			vectorNames << name;
			columnModes << AbstractColumn::Numeric;
		}
	}

	file.readLine(); //skip the line with "Binary:"

	//read the data
	//TODO
}

/*!
 * generates the preview for the file \c fileName reading the provided number of \c lines.
 */
QVector<QStringList> NgspiceRawBinaryFilterPrivate::preview(const QString& fileName, int lines) {
	QVector<QStringList> dataStrings;

	QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		DEBUG("Failed to open the file " << fileName.toStdString());
		return dataStrings;
	}

	//skip the first three lines in the header
	file.readLine(); //"Title"
	file.readLine(); //"Date"
	file.readLine(); //"Plotname"

	//evaluate the "Flags" line to check whether we have complex numbers
	QString line = file.readLine();
	bool hasComplexValues = line.endsWith(QLatin1String("complex\n"));

	//number of variables
	line = file.readLine();
	const int vars = line.right(line.length() - 15).toInt(); //remove the "No. Variables: " sub-string

	//number of points
	line = file.readLine();
	const int points = line.right(line.length() - 12).toInt(); //remove the "No. Points: " sub-string

	//add names of the variables
	vectorNames.clear();
	columnModes.clear();
	file.readLine();
	for (int i = 0; i<vars; ++i) {
		line = file.readLine();
		QStringList tokens = line.split('\t');
		QString name = tokens.at(2) + QLatin1String(", ") + tokens.at(3).simplified();
		if (hasComplexValues) {
			vectorNames << name + QLatin1String(" REAL");
			vectorNames << name + QLatin1String(" IMAGINARY");
			columnModes << AbstractColumn::Numeric;
			columnModes << AbstractColumn::Numeric;
		} else {
			vectorNames << name;
			columnModes << AbstractColumn::Numeric;
		}
	}

	file.readLine(); //skip the line with "Binary"

	//read the data
	//TODO

	return dataStrings;
}

/*!
    writes the content of \c dataSource to the file \c fileName.
*/
void NgspiceRawBinaryFilterPrivate::write(const QString & fileName, AbstractDataSource* dataSource) {
	Q_UNUSED(fileName);
	Q_UNUSED(dataSource);
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################
/*!
  Saves as XML.
 */
void NgspiceRawBinaryFilter::save(QXmlStreamWriter* writer) const {
	Q_UNUSED(writer);
}

/*!
  Loads from XML.
*/
bool NgspiceRawBinaryFilter::load(XmlStreamReader* reader) {
	Q_UNUSED(reader);
	return true;
}
