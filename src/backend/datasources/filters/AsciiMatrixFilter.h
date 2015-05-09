/***************************************************************************
File                 : AsciiMatrixFilter.h
Project              : LabPlot
Description          : ASCII Matrix I/O-filter
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
#ifndef ASCIIMATRIXFILTER_H
#define ASCIIMATRIXFILTER_H

#include <QStringList>
#include "backend/datasources/filters/AbstractFileFilter.h"

class AsciiMatrixFilterPrivate;
class AsciiMatrixFilter : public AbstractFileFilter{
	Q_OBJECT

  public:
	AsciiMatrixFilter();
	~AsciiMatrixFilter();

	static QStringList separatorCharacters();
	static QStringList commentCharacters();
	static QStringList predefinedFilters();

	static int columnNumber(const QString & fileName);
	static long lineNumber(const QString & fileName);

	void read(const QString & fileName, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode importMode=AbstractFileFilter::Replace);
	void write(const QString & fileName, AbstractDataSource* dataSource);

	void loadFilterSettings(const QString&);
	void saveFilterSettings(const QString&) const;

	void setTransposed(const bool);
	bool isTransposed() const;

	void setCommentCharacter(const QString&);
	QString commentCharacter() const;

	void setSeparatingCharacter(const QString&);
	QString separatingCharacter() const;

	void setAutoModeEnabled(const bool);
	bool isAutoModeEnabled() const;

	void setHeaderEnabled(const bool);
	bool isHeaderEnabled() const;

	void setVectorNames(const QString);
	QString vectorNames() const;

	void setSkipEmptyParts(const bool);
	bool skipEmptyParts() const;

	void setSimplifyWhitespacesEnabled(const bool);
	bool simplifyWhitespacesEnabled() const;

	void setStartRow(const int);
	int startRow() const;

	void setEndRow(const int);
	int endRow() const;

	void setStartColumn(const int);
	int startColumn() const;

	void setEndColumn(const int);
	int endColumn() const;

	virtual void save(QXmlStreamWriter*) const;
	virtual bool load(XmlStreamReader*);

  private:
	AsciiMatrixFilterPrivate* const d;
	friend class AsciiMatrixFilterPrivate;
};

#endif
