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

class AbstractDataSource;

class BinaryFilterPrivate {

public:
	explicit BinaryFilterPrivate(BinaryFilter*);

	QVector<QStringList> readDataFromDevice(QIODevice& device, AbstractDataSource* = nullptr,
		AbstractFileFilter::ImportMode = AbstractFileFilter::Replace, int lines = -1);
	QVector<QStringList> readDataFromFile(const QString& fileName, AbstractDataSource* = nullptr,
		AbstractFileFilter::ImportMode = AbstractFileFilter::Replace, int lines = -1);
	void write(const QString& fileName, AbstractDataSource*);

	const BinaryFilter* q;

	int m_vectors;
	BinaryFilter::DataType m_dataType;
	BinaryFilter::ByteOrder m_byteOrder;

	int m_skipStartBytes;	// bytes to skip at start
	int m_startRow;		// start row (value*vectors) to read
	int m_endRow;		// end row to (value*vectors) read
	int m_skipBytes;		// bytes to skip after each value
	int m_numRows;		// number of rows

	bool m_autoModeEnabled;

private:
	void clearDataSource(AbstractDataSource*) const;
};

#endif
