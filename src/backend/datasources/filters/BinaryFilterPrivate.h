/***************************************************************************
File                 : BinaryFilterPrivate.h
Project              : LabPlot
Description          : Private implementation class for BinaryFilter.
--------------------------------------------------------------------
Copyright            : (C) 2015-2017 Stefan Gerlach (stefan.gerlach@uni.kn)
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
#ifndef BINARYFILTERPRIVATE_H
#define BINARYFILTERPRIVATE_H

#include <QVector>

class AbstractDataSource;
class AbstractColumn;

class BinaryFilterPrivate {

public:
	explicit BinaryFilterPrivate(BinaryFilter*);

	int prepareStreamToRead(QDataStream&);
	void readDataFromDevice(QIODevice& device, AbstractDataSource* = nullptr,
			AbstractFileFilter::ImportMode = AbstractFileFilter::Replace, int lines = -1);
	void readDataFromFile(const QString& fileName, AbstractDataSource* = nullptr,
			AbstractFileFilter::ImportMode = AbstractFileFilter::Replace);
	void write(const QString& fileName, AbstractDataSource*);
	QVector<QStringList> preview(const QString& fileName, int lines);

	const BinaryFilter* q;

	size_t vectors;
	BinaryFilter::DataType dataType;
	BinaryFilter::ByteOrder byteOrder;
	QVector<AbstractColumn::ColumnMode> columnModes;

	int startRow;			// start row (value*vectors) to read (can be -1)
	int endRow;			// end row to (value*vectors) read (can be -1)
	size_t numRows;			// number of rows
	size_t skipStartBytes;		// bytes to skip at start
	size_t skipBytes;		// bytes to skip after each value
	bool createIndexEnabled;	// if create index column

	bool autoModeEnabled;

private:
	int m_actualRows;
	int m_actualCols;
};

#endif
