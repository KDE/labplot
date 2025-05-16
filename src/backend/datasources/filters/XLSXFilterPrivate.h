/*
	File                 : XLSXFilterPrivate.h
	Project              : LabPlot
	Description          : XLSX I/O-filter
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2021 Fabian Kristof (fkristofszabolcs@gmail.com)
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef XLSXFILTERPRIVATE_H
#define XLSXFILTERPRIVATE_H

#include "backend/datasources/filters/AbstractFileFilter.h"

class XLSXFilter;
class QTreeWidgetItem;

class XLSXFilterPrivate {
public:
	explicit XLSXFilterPrivate(XLSXFilter*);
	~XLSXFilterPrivate();

	void readDataFromFile(const QString& fileName, AbstractDataSource* = nullptr, AbstractFileFilter::ImportMode = AbstractFileFilter::ImportMode::Replace);
	void write(const QString& fileName, AbstractDataSource*);

	void parse(const QString& fileName, QTreeWidgetItem* root);
	QStringList sheets() const;

#ifdef HAVE_QXLSX
	void readDataRegion(const QXlsx::CellRange& region, AbstractDataSource*, AbstractFileFilter::ImportMode = AbstractFileFilter::ImportMode::Replace);
	QVector<QXlsx::CellRange> dataRegions(const QString& fileName, const QString& sheetName);
	QVector<QStringList> previewForDataRegion(const QString& sheet, const QXlsx::CellRange& region, bool* okToMatrix, int lines);
	QXlsx::CellRange cellContainedInRegions(const QXlsx::CellReference& cell, const QVector<QXlsx::CellRange>& regions) const;
	bool dataRangeCanBeExportedToMatrix(const QXlsx::CellRange& range) const;
	QXlsx::Cell::CellType columnTypeInRange(const int column, const QXlsx::CellRange& range) const;
	QVariant read(int row, int column) const;
	QXlsx::CellRange dimension() const;
#endif
	bool exportDataSourceAsNewSheet{true};
	bool firstRowAsColumnNames{false}; // import first row as column names
	bool columnNamesAsFirstRow{true}; // export column names a first row
	bool overwriteExportData{true};

	QString sheetToAppendSpreadsheetTo;

	int startRow{1};
	int endRow{-1};
	int startColumn{1};
	int endColumn{-1};
	int firstColumn{1}; // actual start column (including range)
	QString currentSheet;

#ifdef HAVE_QXLSX
	QXlsx::CellRange currentRange;
	QXlsx::CellReference dataExportStartCell;
#endif

	XLSXFilter* const q;

private:
#ifdef HAVE_QXLSX
	QXlsx::Document* m_document{nullptr};
#endif
	QString m_fileName;
};

#endif // XLSXFILTERPRIVATE_H
