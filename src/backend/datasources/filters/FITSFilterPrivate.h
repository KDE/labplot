/***************************************************************************
File                 : FITSFilterPrivate.cpp
Project              : LabPlot
Description          : FITS I/O-filter
--------------------------------------------------------------------
Copyright            : (C) 2016 by Fabian Kristof (fkristofszabolcs@gmail.com)
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

#ifndef FITSFILTERPRIVATE_H
#define FITSFILTERPRIVATE_H

#ifdef HAVE_FITS
#include "fitsio.h"
#endif

class AbstractDataSource;

class FITSFilterPrivate {

public:
	explicit FITSFilterPrivate(FITSFilter*);
	~FITSFilterPrivate();

	QVector<QStringList> readCHDU(const QString& fileName, AbstractDataSource* = nullptr,
	                              AbstractFileFilter::ImportMode = AbstractFileFilter::Replace, bool* okToMatrix = nullptr, int lines = -1);
	void writeCHDU(const QString& fileName, AbstractDataSource*);

	static QMultiMap<QString, QString> extensionNames(const QString &fileName);
	void updateKeywords(const QString& fileName, const QList<FITSFilter::Keyword>& originals, const QVector<FITSFilter::Keyword>& updates);
	void addNewKeyword(const QString& fileName, const QList<FITSFilter::Keyword>& keywords);
	void addKeywordUnit(const QString& fileName, const QList<FITSFilter::Keyword>& keywords);
	void deleteKeyword(const QString& fileName, const QList<FITSFilter::Keyword>& keywords);
	void removeExtensions(const QStringList& extensions);
	const QString valueOf(const QString& fileName, const char* key);
	QList<FITSFilter::Keyword> chduKeywords(const QString& fileName);
	void parseHeader(const QString& fileName, QTableWidget* headerEditTable,
	                 bool readKeys = true,
	                 const QList<FITSFilter::Keyword>& keys = QList<FITSFilter::Keyword>());
	void parseExtensions(const QString& fileName, QTreeWidget*, bool checkPrimary = false);

	const FITSFilter* q;

	int startRow{-1};
	int endRow{-1};
	int startColumn{-1};
	int endColumn{-1};

	bool commentsAsUnits{false};
	int exportTo{0};
private:
	void printError(int status) const;

#ifdef HAVE_FITS
	fitsfile* m_fitsFile{nullptr};
#endif
};

#endif // FITSFILTERPRIVATE_H
