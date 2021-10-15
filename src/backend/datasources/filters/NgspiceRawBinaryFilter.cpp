/*
    File                 : NgspiceRawBinaryFilter.cpp
    Project              : LabPlot
    Description          : Ngspice RAW Binary filter
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2018 Alexander Semke <alexander.semke@web.de>
    SPDX-FileCopyrightText: 2018 Stefan Gerlach <stefan.gerlach@uni.kn>
    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "backend/datasources/LiveDataSource.h"
#include "backend/datasources/filters/NgspiceRawBinaryFilter.h"
#include "backend/datasources/filters/NgspiceRawBinaryFilterPrivate.h"
#include "backend/lib/trace.h"

#include <QFile>
#include <QDataStream>

/*!
\class NgspiceRawBinaryFilter
\brief Import of data stored in Ngspice's raw formant, ASCCI version of it.

\ingroup datasources
*/
NgspiceRawBinaryFilter::NgspiceRawBinaryFilter() : AbstractFileFilter(FileType::NgspiceRawBinary), d(new NgspiceRawBinaryFilterPrivate(this)) {}

NgspiceRawBinaryFilter::~NgspiceRawBinaryFilter() = default;

bool NgspiceRawBinaryFilter::isNgspiceBinaryFile(const QString& fileName) {
	QFile file(fileName);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		DEBUG("Failed to open the file " << STDSTRING(fileName));
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
NgspiceRawBinaryFilterPrivate::NgspiceRawBinaryFilterPrivate(NgspiceRawBinaryFilter* owner) : q(owner) {
}

/*!
    reads the content of the file \c fileName to the data source \c dataSource. Uses the settings defined in the data source.
*/
void NgspiceRawBinaryFilterPrivate::readDataFromFile(const QString& fileName, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode importMode) {
	DEBUG("NgspiceRawBinaryFilterPrivate::readDataFromFile(): fileName = \'" << STDSTRING(fileName) << "\', dataSource = "
	      << dataSource << ", mode = " << ENUM_TO_STRING(AbstractFileFilter, ImportMode, importMode));

	QFile file(fileName);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		DEBUG("Failed to open the file " << STDSTRING(fileName));
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
	const int vars = line.rightRef(line.length() - 15).toInt(); //remove the "No. Variables: " sub-string

	//number of points
	line = file.readLine();
	const int points = line.rightRef(line.length() - 12).toInt(); //remove the "No. Points: " sub-string

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
			columnModes << AbstractColumn::ColumnMode::Double;
			columnModes << AbstractColumn::ColumnMode::Double;
		} else {
			vectorNames << name;
			columnModes << AbstractColumn::ColumnMode::Double;
		}
	}

	file.readLine(); //skip the line with "Binary:"
	file.setTextModeEnabled(false);	// the rest is binary

	//prepare the data container
	const int actualEndRow = (endRow == -1 || endRow > points) ? points : endRow;
	const int actualRows = actualEndRow - startRow + 1;
	const int actualCols = hasComplexValues ? 2 * vars : vars;
	const int columnOffset = dataSource->prepareImport(m_dataContainer, importMode, actualRows, actualCols, vectorNames, columnModes);

	//skip data lines, if required
	const int skip = hasComplexValues ? 2 * vars * (startRow - 1) : vars * (startRow - 1);
	if (skip > 0) {
		DEBUG("	Skipping " << startRow - 1 << " lines");
		file.read(BYTE_SIZE * skip);
	}

	//read the data points
	int currentRow = 0;	// indexes the position in the vector(column)
	for (int i = 0; i < actualRows; ++i) {
		for (int j = 0; j < vars; ++j) {
			double value;
			QDataStream s(file.read(BYTE_SIZE));
			s.setByteOrder(QDataStream::LittleEndian);
			s >> value;
			if (hasComplexValues) {
				//real part
				static_cast<QVector<double>*>(m_dataContainer[2*j])->operator[](currentRow) = value;

				//imaginary part
				QDataStream sim(file.read(BYTE_SIZE));
				sim.setByteOrder(QDataStream::LittleEndian);
				sim >> value;
				static_cast<QVector<double>*>(m_dataContainer[2*j+1])->operator[](currentRow) = value;
			} else
				static_cast<QVector<double>*>(m_dataContainer[j])->operator[](currentRow) = value;
		}

		currentRow++;
		emit q->completed(100 * currentRow/actualRows);
	}

	dataSource->finalizeImport(columnOffset, 1, actualCols, QString(), importMode);
}

/*!
 * generates the preview for the file \c fileName reading the provided number of \c lines.
 */
QVector<QStringList> NgspiceRawBinaryFilterPrivate::preview(const QString& fileName, int lines) {
	DEBUG("NgspiceRawBinaryFilterPrivate::preview()");
	QVector<QStringList> dataStrings;

	QFile file(fileName);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		DEBUG("Failed to open the file " << STDSTRING(fileName));
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
	const int vars = line.rightRef(line.length() - 15).toInt(); //remove the "No. Variables: " sub-string
	DEBUG("	vars = " << vars);

	//number of points
	line = file.readLine();
	const int points = line.rightRef(line.length() - 12).toInt(); //remove the "No. Points: " sub-string
	DEBUG("	points = " << points);

	//add names of the variables
	vectorNames.clear();
	columnModes.clear();
	file.readLine();
	for (int i = 0; i < vars; ++i) {
		line = file.readLine();
		QStringList tokens = line.split('\t');
		QString name = tokens.at(2) + QLatin1String(", ") + tokens.at(3).simplified();
		if (hasComplexValues) {
			vectorNames << name + QLatin1String(" REAL");
			vectorNames << name + QLatin1String(" IMAGINARY");
			columnModes << AbstractColumn::ColumnMode::Double;
			columnModes << AbstractColumn::ColumnMode::Double;
		} else {
			vectorNames << name;
			columnModes << AbstractColumn::ColumnMode::Double;
		}
	}

	file.readLine(); //skip the line with "Binary"

	//read the binary data
	file.setTextModeEnabled(false);
	QStringList lineString;
	for (int i = 0; i < qMin(lines, points); ++i) {
		lineString.clear();
		for (int j = 0; j < vars; ++j) {
			double v;
			QDataStream s(file.read(BYTE_SIZE));
			s.setByteOrder(QDataStream::LittleEndian);
			s >> v;
			lineString << QString::number(v, 'e', 15); //real part
			if (hasComplexValues) {
				QDataStream sim(file.read(BYTE_SIZE));
				sim.setByteOrder(QDataStream::LittleEndian);
				sim >> v;
				lineString << QString::number(v, 'e', 15); //imaginary part
			}
		}

		dataStrings << lineString;
	}

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
