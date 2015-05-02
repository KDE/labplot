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
QStringList BinaryFilter::dataFormats(){
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

int BinaryFilter::dataSize(BinaryFilter::DataFormat format) {
	int sizes[]={1,2,4,8,1,2,4,8,4,8};

	return sizes[(int)format];
}

/*!
  returns the number of rows (length of vectors) in the file \c fileName.
*/
long BinaryFilter::rowNumber(const QString & fileName, const int vectors, const BinaryFilter::DataFormat format) {
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
			for(int j=0;j<BinaryFilter::dataSize(format);++j) {
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

void BinaryFilter::setDataFormat(const BinaryFilter::DataFormat f) {
	d->dataFormat = f;
} 

BinaryFilter::DataFormat BinaryFilter::dataFormat() const{
	return d->dataFormat;
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
	q(owner), vectors(2), dataFormat(BinaryFilter::INT8), byteOrder(BinaryFilter::LittleEndian), skipStartBytes(0), startRow(0), endRow(-1), skipBytes(0) {
}

/*!
    reads the content of the file \c fileName to the data source \c dataSource.
    Uses the settings defined in the data source.
*/
void BinaryFilterPrivate::read(const QString & fileName, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode mode){
	qDebug()<<"BinaryFilterPrivate::read()";
	
	QFile file(fileName);
	if ( !file.exists() )
		return;

	if (!file.open(QIODevice::ReadOnly))
        	return;

	QDataStream in(&file);

	//TODO: catch case that skipStartBytes or startValue is bigger than file

	// skip bytes at start
	for (int i=0; i<skipStartBytes; i++){
		qint8 tmp;
		in >> tmp;
	}

	// skip until start row
	for (int i=0; i<(startRow-1)*vectors; ++i){
		for(int j=0;j<BinaryFilter::dataSize(dataFormat);++j) {
			qint8 tmp;
			in >> tmp;
		}
	}

	QStringList vectorNameList;
	for (int k=0; k<vectors; k++ )
		vectorNameList.append( "Column " + QString::number(k+1) );

	qDebug()<<"OK";

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

		if (columns>(vectors)){
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
				dataSource->child<Column>(i)->setName(vectorNameList.at(i+1));
				dataSource->child<Column>(i)->setSuppressDataChangedSignal(true);
			}

			//create additional columns if needed
			for(int i=0; i<=(vectors-columns-1); i++) {
				newColumn = new Column(vectorNameList.at(columns+i+1), AbstractColumn::Numeric);
				newColumn->setUndoAware(false);
				dataSource->addChild(newColumn);
				dataSource->child<Column>(i)->setSuppressDataChangedSignal(true);
			}
		}
	}

	qDebug()<<"OK";

	// set range of rows
	int numRows=BinaryFilter::rowNumber(fileName,vectors,dataFormat);
	qDebug()<<"	numRows ="<<numRows;
	int actualEndRow;
	if (endRow == -1)
		actualEndRow = numRows;
	else if (endRow > numRows-1)
		actualEndRow = numRows-1;
	else
		actualEndRow = endRow;

	// resize the spreadsheet
	Spreadsheet* spreadsheet = dynamic_cast<Spreadsheet*>(dataSource);
	if (mode==AbstractFileFilter::Replace){
		spreadsheet->clear();
		spreadsheet->setRowCount(numRows);
	}else{
		if (spreadsheet->rowCount()<numRows)
			spreadsheet->setRowCount(numRows);
	}

	// pointers to the actual data containers
	QVector<QVector<double>*> dataPointers;
	for ( int n=1; n<=vectors; n++ ){
		QVector<double>* vector = static_cast<QVector<double>* >(dataSource->child<Column>(columnOffset+n-1)->data());
		vector->reserve(numRows);
		vector->resize(numRows);
		dataPointers.push_back(vector);
	}

	// read data
	for (int i=0; i<numRows; i++){
		//TODO
		//lineStringList = line.split( separator, QString::SplitBehavior(skipEmptyParts) );

		for ( int n=1; n<=vectors; n++ ){
/*			if (n<lineStringList.size()) {
				const double value = lineStringList.at(n).toDouble(&isNumber);
				isNumber ? dataPointers[n-startColumn]->operator[](currentRow) = value : dataPointers[n-startColumn]->operator[](currentRow) = NAN;
			} else {
				dataPointers[n-startColumn]->operator[](currentRow) = NAN;
			}
*/
		}

	}

	for ( int n=1; n<=vectors; n++ ){
		Column* column = spreadsheet->column(columnOffset+n-1);
// 		column->setUndoAware(false);
//		column->setComment(comment);
		column->setUndoAware(true);
		if (mode==AbstractFileFilter::Replace) {
			column->setSuppressDataChangedSignal(true);
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
	Q_UNUSED(writer);
	//TODO
}

/*!
  Loads from XML.
*/
bool BinaryFilter::load(XmlStreamReader* reader) {
	Q_UNUSED(reader);
	//TODO
	//TODO
	return true;
}
