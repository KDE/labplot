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
NgspiceRawAsciiFilter::NgspiceRawAsciiFilter() : AbstractFileFilter(NgspiceRawAscii), d(new NgspiceRawAsciiFilterPrivate(this)) {}

NgspiceRawAsciiFilter::~NgspiceRawAsciiFilter() = default;

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
void NgspiceRawAsciiFilter::readDataFromFile(const QString& fileName, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode importMode) {
	d->readDataFromFile(fileName, dataSource, importMode);
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

QStringList NgspiceRawAsciiFilter::vectorNames() const {
	return d->vectorNames;
}

QVector<AbstractColumn::ColumnMode> NgspiceRawAsciiFilter::columnModes() {
	return d->columnModes;
}

//#####################################################################
//################### Private implementation ##########################
//#####################################################################
NgspiceRawAsciiFilterPrivate::NgspiceRawAsciiFilterPrivate(NgspiceRawAsciiFilter* owner) : q(owner) {
}

/*!
    reads the content of the file \c fileName to the data source \c dataSource. Uses the settings defined in the data source.
*/
void NgspiceRawAsciiFilterPrivate::readDataFromFile(const QString& fileName, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode importMode) {
	DEBUG("NgspiceRawAsciiFilterPrivate::readDataFromFile(): fileName = \'" << fileName.toStdString() << "\', dataSource = "
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
	for (int i = 0; i < vars; ++i) {
		line = file.readLine();
		QStringList tokens = line.split('\t');

		//skip lines that don't contain the proper number of tokens (wrong format, corrupted file)
		if (tokens.size() < 4)
			continue;

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

	file.readLine(); //skip the line with "Values"

	//prepare the data container
	const int actualEndRow = (endRow == -1 || endRow > points) ? points : endRow;
	const int actualRows = actualEndRow - startRow + 1;
	const int actualCols = hasComplexValues ? vars*2 : vars;
	const int columnOffset = dataSource->prepareImport(m_dataContainer, importMode, actualRows, actualCols, vectorNames, columnModes);

	//skip data lines, if required
	DEBUG("	Skipping " << startRow - 1 << " lines");
	for (int i = 0; i < startRow - 1; ++i) {
		for (int j = 0; j < vars; ++j)
			file.readLine();

		file.readLine(); //skip the empty line after each value block
	}

	//read the data points
	QStringList lineString;
	int currentRow = 0;	// indexes the position in the vector(column)
	QLocale locale(QLocale::C);
	bool isNumber(false);

	for (int i = 0; i < actualRows; ++i) {
		lineString.clear();
		for (int j = 0; j < vars; ++j) {
			line = file.readLine();
			QStringList tokens = line.split(QLatin1Char('\t'));

			//skip lines that don't contain the proper number of tokens (wrong format, corrupted file)
			if (tokens.size() < 2)
				continue;

			QString valueString = tokens.at(1).simplified(); //string containing the value(s)
			if (hasComplexValues) {
				QStringList realImgTokens = valueString.split(QLatin1Char(','));
				if (realImgTokens.size() == 2) { //sanity check to make sure we really have both parts
					//real part
					double value = locale.toDouble(realImgTokens.at(0), &isNumber);
					static_cast<QVector<double>*>(m_dataContainer[2*j])->operator[](currentRow) = (isNumber ? value : NAN);

					//imaginary part
					value = locale.toDouble(realImgTokens.at(1), &isNumber);
					static_cast<QVector<double>*>(m_dataContainer[2*j+1])->operator[](currentRow) = (isNumber ? value : NAN);
				}
			} else {
				const double value = locale.toDouble(valueString, &isNumber);
				static_cast<QVector<double>*>(m_dataContainer[j])->operator[](currentRow) = (isNumber ? value : NAN);
			}
		}

		file.readLine(); //skip the empty line after each value block

		currentRow++;
		emit q->completed(100 * currentRow/actualRows);
	}

	dataSource->finalizeImport(columnOffset, 1, actualCols, currentRow, "", importMode);
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
	for (int i = 0; i < vars; ++i) {
		line = file.readLine();
		QStringList tokens = line.split('\t');

		//skip lines that don't contain the proper number of tokens (wrong format, corrupted file)
		if (tokens.size() < 4)
			continue;

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

	file.readLine(); //skip the line with "Values"

	//read the data points
	QStringList lineString;
	for (int i = 0; i < qMin(lines, points); ++i) {
		lineString.clear();
		for (int j = 0; j < vars; ++j) {
			line = file.readLine();
			QStringList tokens = line.split(QLatin1Char('\t'));

			//skip lines that don't contain the proper number of tokens (wrong format, corrupted file)
			if (tokens.size() < 2)
				continue;

			QString value = tokens.at(1).simplified(); //string containing the value(s)
			if (hasComplexValues) {
				QStringList realImgTokens = value.split(QLatin1Char(','));
				if (realImgTokens.size() == 2) { //sanity check to make sure we really have both parts
					lineString << realImgTokens.at(0); //real part
					lineString << realImgTokens.at(1); //imaginary part
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
	//TODO: not implemented yet
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
