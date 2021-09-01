/*
File                 : FITSFilterPrivate.cpp
Project              : LabPlot
Description          : FITS I/O-filter
--------------------------------------------------------------------
SPDX-FileCopyrightText: 2016 Fabian Kristof (fkristofszabolcs@gmail.com)
*/

/***************************************************************************
*                                                                         *
*  SPDX-License-Identifier: GPL-2.0-or-later
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
	                              AbstractFileFilter::ImportMode = AbstractFileFilter::ImportMode::Replace, bool* okToMatrix = nullptr, int lines = -1);
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
