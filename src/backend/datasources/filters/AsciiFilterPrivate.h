/***************************************************************************
    File                 : AsciiFilterPrivate.h
    Project              : LabPlot
    Description          : Private implementation class for AsciiFilter.
    --------------------------------------------------------------------
    Copyright            : (C) 2009-2013 Alexander Semke (alexander.semke@web.de)

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

#ifndef ASCIIFILTERPRIVATE_H
#define ASCIIFILTERPRIVATE_H

class KFilterDev;
class AbstractDataSource;

class AsciiFilterPrivate {

public:
	explicit AsciiFilterPrivate(AsciiFilter*);

	int prepareDeviceToRead(KFilterDev&);
	QVector<QStringList> readDataFromFile(const QString& fileName, AbstractDataSource* = nullptr,
					      AbstractFileFilter::ImportMode = AbstractFileFilter::Replace, int lines = -1);
	void write(const QString& fileName, AbstractDataSource*);

	const AsciiFilter* q;

	// TODO: m_*
	QString commentCharacter;
	QString separatingCharacter;
	QString separator;
	bool autoModeEnabled;
	bool headerEnabled;
	QString vectorNames;
	QStringList vectorNameList;
	bool skipEmptyParts;
	bool simplifyWhitespacesEnabled;
	bool transposed;

	int startRow;
	int endRow;
	int actualRows;
	int startColumn;
	int endColumn;
	int actualCols;

private:
	void clearDataSource(AbstractDataSource*) const;
};

#endif
