/***************************************************************************
File                 : AsciiFilter.h
Project              : LabPlot
Description          : Binary I/O-filter
--------------------------------------------------------------------
Copyright            : (C) 2015 Stefan Gerlach
Email (use @ for *)  : stefan.gerlach*uni.kn
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
#ifndef BINARYFILTER_H
#define BINARYFILTER_H

#include <QStringList>
#include "backend/datasources/filters/AbstractFileFilter.h"

class BinaryFilterPrivate;
class BinaryFilter : public AbstractFileFilter{
	Q_OBJECT

  public:
	enum DataFormat{INT8,INT16,INT24,INT32,INT64,INT128,UINT8,UINT16,UINT24,UINT32,UINT64,UINT128,REAL32,REAL64,REAL128};
	enum ByteOrder{LittleEndian, BigEndian};

	BinaryFilter();
	~BinaryFilter();

	static QStringList dataFormats();
	static QStringList byteOrders();

	void read(const QString & fileName, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode importMode=AbstractFileFilter::Replace);
	void write(const QString & fileName, AbstractDataSource* dataSource);

	BinaryFilter::DataFormat dataFormat() const;
	BinaryFilter::ByteOrder byteOrder() const;

  private:
	BinaryFilterPrivate* const d;
	friend class BinaryFilterPrivate;
};

#endif
