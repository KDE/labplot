/***************************************************************************
File                 : BinaryFilter.cpp
Project              : LabPlot
Description          : Binary I/O-filter
--------------------------------------------------------------------
Copyright            : (C) 2015 by Stefan Gerlach (stefan.gerlach@uni.kn)
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

#include <math.h>

#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <KLocale>

 /*!
	\class BinaryFilter
	\brief Manages the import/export of data organized as columns (vectors) from/to a binary file.

	\ingroup datasources
 */
BinaryFilter::BinaryFilter():AbstractFileFilter(), d(new BinaryFilterPrivate(this)){

}

BinaryFilter::~BinaryFilter(){
	delete d;
}

/*!
  reads the content of the file \c fileName to the data source \c dataSource.
*/
void BinaryFilter::read(const QString & fileName, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode importMode){
  d->read(fileName, dataSource, importMode);
}

/*!
writes the content of the data source \c dataSource to the file \c fileName.
*/
void BinaryFilter::write(const QString & fileName, AbstractDataSource* dataSource){
 	d->write(fileName, dataSource);
// 	emit()
}

/*!
returns the list of all predefined data formats.
*/
QStringList BinaryFilter::dataTypes(){
  return (QStringList()<<"int8 (8 bit signed integer)"<<"int16 (16 bit signed integer)"<<"int32 (32 bit signed integer)"<<"int64 (64 bit signed integer)"
  	<<"uint8 (8 bit unsigned integer)"<<"uint16 (16 bit unsigned integer)"<<"uint32 (32 bit unsigned integer)"<<"uint64 (64 bit unsigned integer)"
	<<"real32 (single precision floats)"<<"real64 (double precision floats)");
}

/*!
returns the list of all predefined byte order.
*/
QStringList BinaryFilter::byteOrders(){
  return (QStringList()<<"Little endian"<<"Big endian");
}

int BinaryFilter::dataSize(BinaryFilter::DataType type) {
	int sizes[]={1,2,4,8,1,2,4,8,4,8};

	return sizes[(int)type];
}

/*!
  returns the number of rows (length of vectors) in the file \c fileName.
*/
long BinaryFilter::rowNumber(const QString & fileName, const int vectors, const BinaryFilter::DataType type) {
	QFile file(fileName);
	if ( !file.exists() )
		return 0;

	if (!file.open(QIODevice::ReadOnly))
		return 0;

	QDataStream in(&file);
	long rows=0;
	while (!in.atEnd()){
		// one row
		for (int i=0; i<vectors; ++i){
			for(int j=0;j<BinaryFilter::dataSize(type);++j) {
				qint8 tmp;
				in >> tmp;
			}
		}
		rows++;
	}

	return rows;
}

///////////////////////////////////////////////////////////////////////
/*!
  loads the predefined filter settings for \c filterName
*/
void BinaryFilter::loadFilterSettings(const QString& filterName){
    Q_UNUSED(filterName);
}

/*!
  saves the current settings as a new filter with the name \c filterName
*/
void BinaryFilter::saveFilterSettings(const QString& filterName) const{
    Q_UNUSED(filterName);
}

///////////////////////////////////////////////////////////////////////

void BinaryFilter::setVectors(const int v){
	d->vectors = v;
}

int BinaryFilter::vectors() const{
	return d->vectors;
}

void BinaryFilter::setDataType(const BinaryFilter::DataType t) {
	d->dataType = t;
}

BinaryFilter::DataType BinaryFilter::dataType() const{
	return d->dataType;
}

void BinaryFilter::setByteOrder(const BinaryFilter::ByteOrder b) {
	d->byteOrder = b;
}

BinaryFilter::ByteOrder BinaryFilter::byteOrder() const{
	return d->byteOrder;
}

void BinaryFilter::setSkipStartBytes(const int s) {
	d->skipStartBytes = s;
}

int BinaryFilter::skipStartBytes() const{
	return d->skipStartBytes;
}

void BinaryFilter::setStartRow(const int s) {
	d->startRow = s;
}

int BinaryFilter::startRow() const{
	return d->startRow;
}

void BinaryFilter::setEndRow(const int e) {
	d->endRow = e;
}

int BinaryFilter::endRow() const{
	return d->endRow;
}

void BinaryFilter::setSkipBytes(const int s) {
	d->skipBytes = s;
}

int BinaryFilter::skipBytes() const{
	return d->skipBytes;
}

void BinaryFilter::setAutoModeEnabled(bool b){
	d->autoModeEnabled = b;
}

bool BinaryFilter::isAutoModeEnabled() const{
	return d->autoModeEnabled;
}
//#####################################################################
//################### Private implementation ##########################
//#####################################################################

BinaryFilterPrivate::BinaryFilterPrivate(BinaryFilter* owner) :
	q(owner), vectors(2), dataType(BinaryFilter::INT8), byteOrder(BinaryFilter::LittleEndian), skipStartBytes(0), startRow(1), endRow(-1), skipBytes(0) {
}

/*!
    reads the content of the file \c fileName to the data source \c dataSource.
    Uses the settings defined in the data source.
*/
void BinaryFilterPrivate::read(const QString & fileName, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode mode){
#ifdef QT_DEBUG
	qDebug()<<"BinaryFilterPrivate::read()";
#endif

	QFile file(fileName);
	if ( !file.exists() )
		return;

	if (!file.open(QIODevice::ReadOnly))
        	return;

	QDataStream in(&file);

	if (byteOrder == BinaryFilter::BigEndian)
		in.setByteOrder(QDataStream::BigEndian);
	else if (byteOrder == BinaryFilter::LittleEndian)
		in.setByteOrder(QDataStream::LittleEndian);

	int numRows=BinaryFilter::rowNumber(fileName,vectors,dataType);

	// catch case that skipStartBytes or startRow is bigger than file
	if(skipStartBytes >= BinaryFilter::dataSize(dataType)*vectors*numRows || startRow > numRows) {
		return;
	}

	// skip bytes at start
	for (int i=0; i<skipStartBytes; i++){
		qint8 tmp;
		in >> tmp;
	}

	// skip until start row
	for (int i=0; i<(startRow-1)*vectors; ++i){
		for(int j=0;j<BinaryFilter::dataSize(dataType);++j) {
			qint8 tmp;
			in >> tmp;
		}
	}

	QStringList vectorNameList;
	for (int k=0; k<vectors; k++ )
		vectorNameList.append( "Column " + QString::number(k+1) );

	//make sure we have enough columns in the data source.
	Column * newColumn;
	int columnOffset=0; //indexes the "start column" in the spreadsheet. Starting from this column the data will be imported.
	dataSource->setUndoAware(false);

	if (mode==AbstractFileFilter::Append){
		columnOffset=dataSource->childCount<Column>();
		for ( int n=1; n<=vectors; n++ ){
			newColumn = new Column(vectorNameList.at(n-1), AbstractColumn::Numeric);
			newColumn->setUndoAware(false);
			dataSource->addChild(newColumn);
		}
	}else if (mode==AbstractFileFilter::Prepend){
		Column* firstColumn = dataSource->child<Column>(0);
		for ( int n=1; n<=vectors; n++ ){
			newColumn = new Column(vectorNameList.at(n-1), AbstractColumn::Numeric);
			newColumn->setUndoAware(false);
			dataSource->insertChildBefore(newColumn, firstColumn);
		}
	}else if (mode==AbstractFileFilter::Replace){
		//replace completely the previous content of the data source with the content to be imported.
		int columns = dataSource->childCount<Column>();

		if (columns > vectors){
			//there're more columns in the data source then required
			//-> remove the superfluous columns
			for(int i=0;i<columns-(vectors);i++) {
				dataSource->removeChild(dataSource->child<Column>(0));
			}

			//rename the columns, that are already available
			for (int i=0; i<vectors-1; i++){
				dataSource->child<Column>(i)->setUndoAware(false);
				dataSource->child<Column>(i)->setColumnMode( AbstractColumn::Numeric);
				dataSource->child<Column>(i)->setName(vectorNameList.at(i+1));
				dataSource->child<Column>(i)->setSuppressDataChangedSignal(true);
			}
		}else{
			//rename the columns, that are already available
			for (int i=0; i<columns; i++){
				dataSource->child<Column>(i)->setUndoAware(false);
				dataSource->child<Column>(i)->setColumnMode( AbstractColumn::Numeric);
				dataSource->child<Column>(i)->setName(vectorNameList.at(i));
				dataSource->child<Column>(i)->setSuppressDataChangedSignal(true);
			}

			//create additional columns if needed
			for(int i=0; i < (vectors-columns); i++) {
				newColumn = new Column(vectorNameList.at(columns+i+1), AbstractColumn::Numeric);
				newColumn->setUndoAware(false);
				dataSource->addChild(newColumn);
				dataSource->child<Column>(i)->setSuppressDataChangedSignal(true);
			}
		}
	}

	// set range of rows
	int actualRows;
	if (endRow == -1)
		actualRows = numRows-startRow+1;
	else if (endRow > numRows-startRow+1)
		actualRows = numRows;
	else
		actualRows = endRow-startRow+1;
#ifdef QT_DEBUG
	qDebug()<<"	numRows ="<<numRows;
	qDebug()<<"	startRow ="<<startRow;
	qDebug()<<"	endRow ="<<endRow;
	qDebug()<<"	actualRows ="<<actualRows;
#endif

	// resize the spreadsheet
	Spreadsheet* spreadsheet = dynamic_cast<Spreadsheet*>(dataSource);
	if (mode==AbstractFileFilter::Replace) {
		spreadsheet->clear();
		spreadsheet->setRowCount(actualRows);
	}else{
		if (spreadsheet->rowCount() < actualRows)
			spreadsheet->setRowCount(actualRows);
	}

	// pointers to the actual data containers
	QVector<QVector<double>*> dataPointers;
	for ( int n=1; n<=vectors; n++ ){
		QVector<double>* vector = static_cast<QVector<double>* >(dataSource->child<Column>(columnOffset+n-1)->data());
		vector->reserve(actualRows);
		vector->resize(actualRows);
		dataPointers.push_back(vector);
	}

	//qDebug()<<" data format = "<<BinaryFilter::dataTypes()[dataType];

	// read data
	for (int i=0; i<actualRows; i++){
		for ( int n=0; n<vectors; n++ ){
			switch(dataType) {
			case BinaryFilter::INT8: {
				qint8 value;
				in >> value;
				dataPointers[n]->operator[](i) = value;
				break;
			}
			case BinaryFilter::INT16: {
				qint16 value;
				in >> value;
				dataPointers[n]->operator[](i) = value;
				break;
			}
			case BinaryFilter::INT32: {
				qint32 value;
				in >> value;
				dataPointers[n]->operator[](i) = value;
				break;
			}
			case BinaryFilter::INT64: {
				qint64 value;
				in >> value;
				dataPointers[n]->operator[](i) = value;
				break;
			}
			case BinaryFilter::UINT8: {
				quint8 value;
				in >> value;
				dataPointers[n]->operator[](i) = value;
				break;
			}
			case BinaryFilter::UINT16: {
				quint16 value;
				in >> value;
				dataPointers[n]->operator[](i) = value;
				break;
			}
			case BinaryFilter::UINT32: {
				quint32 value;
				in >> value;
				dataPointers[n]->operator[](i) = value;
				break;
			}
			case BinaryFilter::UINT64: {
				quint64 value;
				in >> value;
				dataPointers[n]->operator[](i) = value;
				break;
			}
			case BinaryFilter::REAL32: {
				float value;
				in >> value;
				dataPointers[n]->operator[](i) = value;
				break;
			}
			case BinaryFilter::REAL64: {
				double value;
				in >> value;
				dataPointers[n]->operator[](i) = value;
				break;
			}
			}
		}
	}

	QString comment = i18np("numerical data, %1 element", "numerical data, %1 elements", actualRows);
	for ( int n=1; n<=vectors; n++ ){
		Column* column = spreadsheet->column(columnOffset+n-1);
		column->setComment(comment);
		column->setUndoAware(true);
		if (mode==AbstractFileFilter::Replace) {
			column->setSuppressDataChangedSignal(false);
			column->setChanged();
		}
	}

	spreadsheet->setUndoAware(true);
}

/*!
    writes the content of \c dataSource to the file \c fileName.
*/
void BinaryFilterPrivate::write(const QString & fileName, AbstractDataSource* dataSource){
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
	writer->writeAttribute("vectors", QString::number(d->vectors) );
	writer->writeAttribute("dataType", QString::number(d->dataType) );
	writer->writeAttribute("byteOrder", QString::number(d->byteOrder) );
	writer->writeAttribute("autoMode", QString::number(d->autoModeEnabled) );
	writer->writeAttribute("startRow", QString::number(d->startRow) );
	writer->writeAttribute("endRow", QString::number(d->endRow) );
	writer->writeAttribute("skipStartBytes", QString::number(d->skipStartBytes) );
	writer->writeAttribute("skipBytes", QString::number(d->skipBytes) );
	writer->writeEndElement();
}

/*!
  Loads from XML.
*/
bool BinaryFilter::load(XmlStreamReader* reader) {
	if(!reader->isStartElement() || reader->name() != "binaryFilter"){
		reader->raiseError(i18n("no binary filter element found"));
		return false;
	}

	QString attributeWarning = i18n("Attribute '%1' missing or empty, default value is used");
	QXmlStreamAttributes attribs = reader->attributes();

	// read attributes
	QString str = attribs.value("vectors").toString();
	if(str.isEmpty())
		reader->raiseWarning(attributeWarning.arg("'vectors'"));
	else
		d->vectors = str.toInt();

	str = attribs.value("dataType").toString();
	if(str.isEmpty())
		reader->raiseWarning(attributeWarning.arg("'dataType'"));
	else
		d->dataType = (BinaryFilter::DataType) str.toInt();

	str = attribs.value("byteOrder").toString();
	if(str.isEmpty())
		reader->raiseWarning(attributeWarning.arg("'byteOrder'"));
	else
		d->byteOrder = (BinaryFilter::ByteOrder) str.toInt();

	str = attribs.value("autoMode").toString();
	if(str.isEmpty())
		reader->raiseWarning(attributeWarning.arg("'autoMode'"));
	else
		d->autoModeEnabled = str.toInt();

	str = attribs.value("startRow").toString();
	if(str.isEmpty())
		reader->raiseWarning(attributeWarning.arg("'startRow'"));
	else
		d->startRow = str.toInt();

	str = attribs.value("endRow").toString();
	if(str.isEmpty())
		reader->raiseWarning(attributeWarning.arg("'endRow'"));
	else
		d->endRow = str.toInt();

	str = attribs.value("skipStartBytes").toString();
	if(str.isEmpty())
		reader->raiseWarning(attributeWarning.arg("'skipStartBytes'"));
	else
		d->skipStartBytes = str.toInt();

	str = attribs.value("skipBytes").toString();
	if(str.isEmpty())
		reader->raiseWarning(attributeWarning.arg("'skipBytes'"));
	else
		d->skipBytes = str.toInt();

	return true;
}
