/***************************************************************************
    File                 : AsciiFilterPrivate.h
    Project              : LabPlot/SciDAVis
    Description          : Private implementation class for AsciiFilter.
    --------------------------------------------------------------------
	Copyright            : (C) 2009-2013 Alexander Semke (alexander.semke*web.de)
						   (replace * with @ in the email addresses) 
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
#define ASCIIFILTERPPRIVATE_H

class AbstractDataSource;

class AsciiFilterPrivate {
  
  public:
    AsciiFilterPrivate(AsciiFilter*);

    void read(const QString & fileName, AbstractDataSource* dataSource,
			  AbstractFileFilter::ImportMode importMode = AbstractFileFilter::Replace);
    void write(const QString & fileName, AbstractDataSource* dataSource);

	const AsciiFilter* q;
	
    QString commentCharacter;
    QString separatingCharacter;
    bool autoModeEnabled;
    bool headerEnabled;
    QString vectorNames;
    bool skipEmptyParts;
    bool simplifyWhitespacesEnabled;
    bool transposed;

    int startRow;
    int endRow;
    int startColumn;
    int endColumn;
};

#endif
