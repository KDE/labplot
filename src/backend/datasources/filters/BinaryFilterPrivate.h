/***************************************************************************
File                 : BinaryFilterPrivate.h
Project              : LabPlot
Description          : Private implementation class for BinaryFilter.
--------------------------------------------------------------------
Copyright            : (C) 2015 Stefan Gerlach (stefan.gerlach@uni.kn)
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
#define BINARYFILTERPPRIVATE_H

class AbstractDataSource;

class BinaryFilterPrivate {

	public:
		explicit BinaryFilterPrivate(BinaryFilter*);

		void read(const QString & fileName, AbstractDataSource* dataSource,
					AbstractFileFilter::ImportMode importMode = AbstractFileFilter::Replace);
		void write(const QString & fileName, AbstractDataSource* dataSource);

		const BinaryFilter* q;

		int vectors;
		BinaryFilter::DataFormat dataFormat;
		BinaryFilter::ByteOrder byteOrder;

		int skipStartBytes;	// bytes to skip at start
		int startRow;		// start row (value*vectors) to read
		int endRow;		// end row to (value*vectors) read
		int skipBytes;		// bytes to skip after each value

		bool autoModeEnabled;

	private:
		void clearDataSource(AbstractDataSource*) const;
};

#endif
