/*
	File                 : OdsFilterPrivate.h
	Project              : LabPlot
	Description          : Ods I/O-filter
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Stefan Gerlach
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ODSFILTERPRIVATE_H
#define ODSFILTERPRIVATE_H

#include "backend/datasources/filters/AbstractFileFilter.h"

#ifdef HAVE_ORCUS
#include <orcus/spreadsheet/document.hpp>
#endif

class OdsFilter;
class QTreeWidgetItem;

class OdsFilterPrivate {
public:
	explicit OdsFilterPrivate(OdsFilter*);
	~OdsFilterPrivate();

	void readDataFromFile(const QString& fileName, AbstractDataSource* = nullptr, AbstractFileFilter::ImportMode = AbstractFileFilter::ImportMode::Replace);
	void write(const QString& fileName, AbstractDataSource*);
	QVector<QStringList> preview(const QString& sheetName, int lines);
	void parse(const QString& fileName, QTreeWidgetItem* root);
	/*	QStringList sheets() const;

	#ifdef HAVE_EXCEL
		void readDataRegion(const QXlsx::CellRange& region, AbstractDataSource*, AbstractFileFilter::ImportMode = AbstractFileFilter::ImportMode::Replace);
		QVector<QXlsx::CellRange> dataRegions(const QString& fileName, const QString& sheetName);
		QXlsx::CellRange cellContainedInRegions(const QXlsx::CellReference& cell, const QVector<QXlsx::CellRange>& regions) const;
		bool dataRangeCanBeExportedToMatrix(const QXlsx::CellRange& range) const;
		bool isColumnNumericInRange(const int column, const QXlsx::CellRange& range) const;

		QXlsx::CellRange dimension() const;
	#endif
	*/
	const OdsFilter* q;
	/*
		bool exportDataSourceAsNewSheet{true};
		bool columnNamesAsFirstRow{true};
		bool firstRowAsColumnNames{false};
		bool overwriteExportData{true};

		QString sheetToAppendSpreadsheetTo;
	*/
	int startRow{-1};
	int endRow{-1};
	int startColumn{-1};
	int endColumn{-1};
	/*	QString currentSheet;

	#ifdef HAVE_EXCEL
		QXlsx::CellRange currentRange;
		QXlsx::CellReference dataExportStartCell;
	#endif
*/
private:
#ifdef HAVE_EXCEL
	orcus::spreadsheet::range_size_t ss{1048576, 16384};
	orcus::spreadsheet::document m_document{ss};
#endif
	//	QString m_fileName;
};

#endif
