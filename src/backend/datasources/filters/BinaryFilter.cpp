/***************************************************************************
File                 : AsciiFilter.cpp
Project              : LabPlot
Description          : Binary I/O-filter
--------------------------------------------------------------------
Copyright            : (C) 2015 by Stefan Gerlach (stefan.gerlach@uni-konstanz.de)

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
  return (QStringList()<<"int8 (8 bit signed integer)"<<"int16 (16 bit signed integer)"<<"int24 (24 bit signed integer)"<<"int32 (32 bit signed integer)"
	<<"int64 (64 bit signed integer)"<<"int128 (128 bit signed integer)"
  	<<"uint8 (8 bit unsigned integer)"<<"uint16 (16 bit unsigned integer)"<<"uint24 (24 bit unsigned integer)"<<"uint32 (32 bit unsigned integer)"
	<<"uint64 (64 bit unsigned integer)"<<"uint128 (128 bit unsigned integer)"
	<<"real32 (single precision numbers)"<<"real64 (double precision numbers)"<<"real128 (quad precision numbers)"
	);
}

/*!
returns the list of all predefined byte order.
*/
QStringList BinaryFilter::byteOrders(){
  return (QStringList()<<"Little endian"<<"Big endian");
}

BinaryFilter::DataFormat BinaryFilter::dataFormat() const{
  return d->dataFormat;
}

BinaryFilter::ByteOrder BinaryFilter::byteOrder() const{
  return d->byteOrder;
}



//#####################################################################
//################### Private implementation ##########################
//#####################################################################
BinaryFilterPrivate::BinaryFilterPrivate(BinaryFilter* owner) : 
	q(owner), dataFormat(BinaryFilter::UINT16), byteOrder(BinaryFilter::LittleEndian) {

}
/*!
    reads the content of the file \c fileName to the data source \c dataSource.
    Uses the settings defined in the data source.
*/
void BinaryFilterPrivate::read(const QString & fileName, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode mode){
	//TODO
}

/*!
    writes the content of \c dataSource to the file \c fileName.
*/
void BinaryFilterPrivate::write(const QString & fileName, AbstractDataSource* dataSource){
    Q_UNUSED(fileName);
    Q_UNUSED(dataSource);
}
