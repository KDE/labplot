/***************************************************************************
File                 : BinaryFilter.cpp
Project              : LabPlot
Description          : Binary I/O-filter
--------------------------------------------------------------------
Copyright            : (C) 2015-2017 by Stefan Gerlach (stefan.gerlach@uni.kn)
Copyright            : (C) 2017 Alexander Semke (alexander.semke@web.de)
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
#include "backend/datasources/filters/BinaryFilter.h"
#include "backend/datasources/filters/BinaryFilterPrivate.h"
#include "backend/datasources/FileDataSource.h"
#include "backend/core/column/Column.h"

#include <QDataStream>
#include <QDebug>
#include <KLocale>
#include <KFilterDev>
#include <cmath>

 /*!
	\class BinaryFilter
	\brief Manages the import/export of data organized as columns (vectors) from/to a binary file.

	\ingroup datasources
 */
BinaryFilter::BinaryFilter():AbstractFileFilter(), d(new BinaryFilterPrivate(this)) {
}

BinaryFilter::~BinaryFilter() {
	delete d;
}

/*!
  reads the content of the file \c fileName.
*/
QVector<QStringList> BinaryFilter::readDataFromFile(const QString& fileName, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode importMode,  int lines) {
	return d->readDataFromFile(fileName, dataSource, importMode, lines);
}

/*!
  reads the content of the device \c device.
*/
QVector<QStringList> BinaryFilter::readDataFromDevice(QIODevice& device, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode importMode,  int lines) {
	return d->readDataFromDevice(device, dataSource, importMode, lines);
}

/*!
writes the content of the data source \c dataSource to the file \c fileName.
*/
void BinaryFilter::write(const QString & fileName, AbstractDataSource* dataSource) {
 	d->write(fileName, dataSource);
// 	emit()
}

/*!
returns the list of all predefined data formats.
*/
QStringList BinaryFilter::dataTypes() {
	return (QStringList()<<"int8 (8 bit signed integer)"<<"int16 (16 bit signed integer)"<<"int32 (32 bit signed integer)"<<"int64 (64 bit signed integer)"
  	<<"uint8 (8 bit unsigned integer)"<<"uint16 (16 bit unsigned integer)"<<"uint32 (32 bit unsigned integer)"<<"uint64 (64 bit unsigned integer)"
	<<"real32 (single precision floats)"<<"real64 (double precision floats)");
}

/*!
returns the list of all predefined byte order.
*/
QStringList BinaryFilter::byteOrders() {
	return (QStringList()<<"Little endian"<<"Big endian");
}

/*!
returns the size of the predefined data types
*/
int BinaryFilter::dataSize(BinaryFilter::DataType type) {
	int sizes[]={1,2,4,8,1,2,4,8,4,8};

	return sizes[(int)type];
}

/*!
  returns the number of rows (length of vectors) in the file \c fileName.
*/
long BinaryFilter::rowNumber(const QString & fileName, const int vectors, const BinaryFilter::DataType type) {
	KFilterDev device(fileName);
	if (!device.open(QIODevice::ReadOnly))
		return 0;

	long rows=0;
	while (!device.atEnd()) {
		// one row
		for (int i=0; i < vectors; ++i){
			for (int j=0; j < BinaryFilter::dataSize(type); ++j)
				device.read(1);
		}
		rows++;
	}

	return rows;
}

///////////////////////////////////////////////////////////////////////
/*!
  loads the predefined filter settings for \c filterName
*/
void BinaryFilter::loadFilterSettings(const QString& filterName) {
	Q_UNUSED(filterName);
}

/*!
  saves the current settings as a new filter with the name \c filterName
*/
void BinaryFilter::saveFilterSettings(const QString& filterName) const {
	Q_UNUSED(filterName);
}

///////////////////////////////////////////////////////////////////////

void BinaryFilter::setVectors(const int v) {
	d->m_vectors = v;
}

int BinaryFilter::vectors() const {
	return d->m_vectors;
}

void BinaryFilter::setDataType(const BinaryFilter::DataType t) {
	d->m_dataType = t;
}

BinaryFilter::DataType BinaryFilter::dataType() const {
	return d->m_dataType;
}

void BinaryFilter::setByteOrder(const BinaryFilter::ByteOrder b) {
	d->m_byteOrder = b;
}

BinaryFilter::ByteOrder BinaryFilter::byteOrder() const{
	return d->m_byteOrder;
}

void BinaryFilter::setSkipStartBytes(const int s) {
	d->m_skipStartBytes = s;
}

int BinaryFilter::skipStartBytes() const {
	return d->m_skipStartBytes;
}

void BinaryFilter::setStartRow(const int s) {
	d->m_startRow = s;
}

int BinaryFilter::startRow() const {
	return d->m_startRow;
}

void BinaryFilter::setEndRow(const int e) {
	d->m_endRow = e;
}

int BinaryFilter::endRow() const {
	return d->m_endRow;
}

void BinaryFilter::setSkipBytes(const int s) {
	d->m_skipBytes = s;
}

int BinaryFilter::skipBytes() const {
	return d->m_skipBytes;
}

void BinaryFilter::setAutoModeEnabled(bool b) {
	d->m_autoModeEnabled = b;
}

bool BinaryFilter::isAutoModeEnabled() const {
	return d->m_autoModeEnabled;
}
//#####################################################################
//################### Private implementation ##########################
//#####################################################################

BinaryFilterPrivate::BinaryFilterPrivate(BinaryFilter* owner) :
	q(owner), m_vectors(2), m_dataType(BinaryFilter::INT8), m_byteOrder(BinaryFilter::LittleEndian),
	m_skipStartBytes(0), m_startRow(1), m_endRow(-1), m_skipBytes(0), m_autoModeEnabled(true) {
}


/*!
    reads the content of the device \c device to the data source \c dataSource or return as string for preview.
    Uses the settings defined in the data source.
*/
QVector<QStringList> BinaryFilterPrivate::readDataFromFile(const QString& fileName, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode importMode, int lines) {
	QVector<QStringList> dataStrings;

	KFilterDev device(fileName);
	if (! device.open(QIODevice::ReadOnly))
		return dataStrings << (QStringList() << i18n("could not open device"));

	m_numRows = BinaryFilter::rowNumber(fileName, m_vectors, m_dataType);

	return readDataFromDevice(device, dataSource, importMode, lines);
}
/*!
    reads the content of the file \c fileName to the data source \c dataSource or return as string for preview.
    Uses the settings defined in the data source.
*/
QVector<QStringList> BinaryFilterPrivate::readDataFromDevice(QIODevice& device, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode importMode, int lines) {
	QVector<QStringList> dataStrings;

	QDataStream in(&device);

	//TODO: check
	if (m_byteOrder == BinaryFilter::BigEndian)
		in.setByteOrder(QDataStream::BigEndian);
	else if (m_byteOrder == BinaryFilter::LittleEndian)
		in.setByteOrder(QDataStream::LittleEndian);

	// catch case that skipStartBytes or startRow is bigger than file
	if (m_skipStartBytes >= BinaryFilter::dataSize(m_dataType) * m_vectors * m_numRows || m_startRow > m_numRows) {
		if (dataSource != NULL)
			dataSource->clear();
		return dataStrings << (QStringList() << i18n("data selection empty"));
	}

	// skip bytes at start
	for (int i = 0; i < m_skipStartBytes; i++) {
		qint8 tmp;
		in >> tmp;
	}

	// skip until start row
	for (int i = 0; i < (m_startRow-1) * m_vectors; ++i) {
		for (int j = 0; j < BinaryFilter::dataSize(m_dataType); ++j) {
			qint8 tmp;
			in >> tmp;
		}
	}

	// set range of rows
	int actualRows;
	if (m_endRow == -1)
		actualRows = m_numRows - m_startRow + 1;
	else if (m_endRow > m_numRows - m_startRow + 1)
		actualRows = m_numRows;
	else
		actualRows = m_endRow - m_startRow + 1;
	int actualCols = m_vectors;
	if (lines == -1)
		lines = actualRows;

	//TODO: use DEBUG()
#ifndef NDEBUG
	qDebug()<<"	numRows ="<<m_numRows;
// 	qDebug()<<"	startRow ="<<m_startRow;
	qDebug()<<"	endRow ="<<m_endRow;
	qDebug()<<"	actualRows ="<<actualRows;
	qDebug()<<"	actualCols ="<<actualCols;
	qDebug()<<"	lines ="<<lines;
#endif

	QVector<void*> dataContainer;
	int columnOffset = 0;
	if (dataSource)
		columnOffset = dataSource->prepareImport(dataContainer, importMode, actualRows, actualCols);

	// read data
	//TODO: use ColumnMode ?
	for (int i = 0; i < qMin(actualRows, lines); i++) {
		QStringList lineString;
		for (int n = 0; n < actualCols; n++) {
			switch (m_dataType) {
			case BinaryFilter::INT8: {
				qint8 value;
				in >> value;
				if (dataSource)
					static_cast<QVector<qint8>*>(dataContainer[n])->operator[](i) = value;
				else
					lineString << QString::number(value);
				break;
			}
			case BinaryFilter::INT16: {
				qint16 value;
				in >> value;
				if (dataSource)
					static_cast<QVector<qint16>*>(dataContainer[n])->operator[](i) = value;
				else
					lineString << QString::number(value);
				break;
			}
			case BinaryFilter::INT32: {
				qint32 value;
				in >> value;
				if (dataSource)
					static_cast<QVector<qint32>*>(dataContainer[n])->operator[](i) = value;
				else
					lineString << QString::number(value);
				break;
			}
			case BinaryFilter::INT64: {
				qint64 value;
				in >> value;
				if (dataSource)
					static_cast<QVector<qint64>*>(dataContainer[n])->operator[](i) = value;
				else
					lineString << QString::number(value);
				break;
			}
			case BinaryFilter::UINT8: {
				quint8 value;
				in >> value;
				if (dataSource)
					static_cast<QVector<quint8>*>(dataContainer[n])->operator[](i) = value;
				else
					lineString << QString::number(value);
				break;
			}
			case BinaryFilter::UINT16: {
				quint16 value;
				in >> value;
				if (dataSource)
					static_cast<QVector<quint16>*>(dataContainer[n])->operator[](i) = value;
				else
					lineString << QString::number(value);
				break;
			}
			case BinaryFilter::UINT32: {
				quint32 value;
				in >> value;
				if (dataSource)
					static_cast<QVector<quint32>*>(dataContainer[n])->operator[](i) = value;
				else
					lineString << QString::number(value);
				break;
			}
			case BinaryFilter::UINT64: {
				quint64 value;
				in >> value;
				if (dataSource)
					static_cast<QVector<quint64>*>(dataContainer[n])->operator[](i) = value;
				else
					lineString << QString::number(value);
				break;
			}
			case BinaryFilter::REAL32: {
				float value;
				in >> value;
				if (dataSource)
					static_cast<QVector<float>*>(dataContainer[n])->operator[](i) = value;
				else
					lineString << QString::number(value);
				break;
			}
			case BinaryFilter::REAL64: {
				double value;
				in >> value;
				if (dataSource)
					static_cast<QVector<double>*>(dataContainer[n])->operator[](i) = value;
				else
					lineString << QString::number(value);
				break;
			}
			}
		}
		dataStrings << lineString;
		emit q->completed(100*i/actualRows);
	}

	if (!dataSource)
		return dataStrings;

	//make everything undo/redo-able again
	//set the comments for each of the columns
	Spreadsheet* spreadsheet = dynamic_cast<Spreadsheet*>(dataSource);
	if (spreadsheet) {
		Spreadsheet* spreadsheet = dynamic_cast<Spreadsheet*>(dataSource);
		QString comment = i18np("numerical data, %1 element", "numerical data, %1 elements", actualRows);
		for (int n=0; n < actualCols; n++) {
			Column* column = spreadsheet->column(columnOffset+n);
			column->setComment(comment);
			if (importMode == AbstractFileFilter::Replace) {
				column->setSuppressDataChangedSignal(false);
				column->setChanged();
			}
		}
	}

	dataSource->finalizeImport();
	return dataStrings;
}

/*!
    writes the content of \c dataSource to the file \c fileName.
*/
void BinaryFilterPrivate::write(const QString & fileName, AbstractDataSource* dataSource) {
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
void BinaryFilter::save(QXmlStreamWriter* writer) const {
	writer->writeStartElement("binaryFilter");
	writer->writeAttribute("vectors", QString::number(d->m_vectors) );
	writer->writeAttribute("dataType", QString::number(d->m_dataType) );
	writer->writeAttribute("byteOrder", QString::number(d->m_byteOrder) );
	writer->writeAttribute("autoMode", QString::number(d->m_autoModeEnabled) );
	writer->writeAttribute("startRow", QString::number(d->m_startRow) );
	writer->writeAttribute("endRow", QString::number(d->m_endRow) );
	writer->writeAttribute("skipStartBytes", QString::number(d->m_skipStartBytes) );
	writer->writeAttribute("skipBytes", QString::number(d->m_skipBytes) );
	writer->writeEndElement();
}

/*!
  Loads from XML.
*/
bool BinaryFilter::load(XmlStreamReader* reader) {
	if (!reader->isStartElement() || reader->name() != "binaryFilter") {
		reader->raiseError(i18n("no binary filter element found"));
		return false;
	}

	QString attributeWarning = i18n("Attribute '%1' missing or empty, default value is used");
	QXmlStreamAttributes attribs = reader->attributes();

	// read attributes
	QString str = attribs.value("vectors").toString();
	if (str.isEmpty())
		reader->raiseWarning(attributeWarning.arg("'vectors'"));
	else
		d->m_vectors = str.toInt();

	str = attribs.value("dataType").toString();
	if (str.isEmpty())
		reader->raiseWarning(attributeWarning.arg("'dataType'"));
	else
		d->m_dataType = (BinaryFilter::DataType) str.toInt();

	str = attribs.value("byteOrder").toString();
	if (str.isEmpty())
		reader->raiseWarning(attributeWarning.arg("'byteOrder'"));
	else
		d->m_byteOrder = (BinaryFilter::ByteOrder) str.toInt();

	str = attribs.value("autoMode").toString();
	if (str.isEmpty())
		reader->raiseWarning(attributeWarning.arg("'autoMode'"));
	else
		d->m_autoModeEnabled = str.toInt();

	str = attribs.value("startRow").toString();
	if (str.isEmpty())
		reader->raiseWarning(attributeWarning.arg("'startRow'"));
	else
		d->m_startRow = str.toInt();

	str = attribs.value("endRow").toString();
	if (str.isEmpty())
		reader->raiseWarning(attributeWarning.arg("'endRow'"));
	else
		d->m_endRow = str.toInt();

	str = attribs.value("skipStartBytes").toString();
	if (str.isEmpty())
		reader->raiseWarning(attributeWarning.arg("'skipStartBytes'"));
	else
		d->m_skipStartBytes = str.toInt();

	str = attribs.value("skipBytes").toString();
	if (str.isEmpty())
		reader->raiseWarning(attributeWarning.arg("'skipBytes'"));
	else
		d->m_skipBytes = str.toInt();

	return true;
}
