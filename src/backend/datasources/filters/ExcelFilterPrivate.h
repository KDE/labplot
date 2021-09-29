/***************************************************************************
File                 : ExcelFilterPrivate.h
Project              : LabPlot
Description          : Excel I/O-filter
--------------------------------------------------------------------
Copyright            : (C) 2021 by Fabian Kristof (fkristofszabolcs@gmail.com)
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

#ifndef EXCELFILTERPRIVATE_H
#define EXCELFILTERPRIVATE_H

#include "backend/datasources/filters/AbstractFileFilter.h"

class ExcelFilter;
class QTreeWidgetItem;

class ExcelFilterPrivate {
public:
    explicit ExcelFilterPrivate(ExcelFilter*);
    ~ExcelFilterPrivate();

    void readDataFromFile(const QString& fileName, AbstractDataSource* = nullptr, AbstractFileFilter::ImportMode = AbstractFileFilter::ImportMode::Replace);
    void write(const QString& fileName, AbstractDataSource*);

    void parse(const QString& fileName, QTreeWidgetItem* root);
    QStringList sheets() const;

#ifdef HAVE_EXCEL
    void readDataRegion(const QXlsx::CellRange& region, AbstractDataSource*, AbstractFileFilter::ImportMode = AbstractFileFilter::ImportMode::Replace);
    QVector<QXlsx::CellRange> dataRegions(const QString& fileName, const QString& sheetName);
    QVector<QStringList> previewForDataRegion(const QString& sheet, const QXlsx::CellRange& region, bool *okToMatrix, int lines);
    QXlsx::CellRange cellContainedInRegions(const QXlsx::CellReference& cell, const QVector<QXlsx::CellRange>& regions) const;
    bool dataRangeCanBeExportedToMatrix(const QXlsx::CellRange& range) const;
    bool isColumnNumericInRange(const int column, const QXlsx::CellRange& range) const;

    QXlsx::CellRange dimension() const;
#endif
    const ExcelFilter* q;

    bool exportDataSourceAsNewSheet {true};
    bool columnNamesAsFirstRow {true};
    bool firstRowAsColumnNames {false};
    bool overwriteExportData {true};

    QString sheetToAppendSpreadsheetTo;

    int startRow{-1};
    int endRow{-1};
    int startColumn{-1};
    int endColumn{-1};
    QString currentSheet;

#ifdef HAVE_EXCEL
    QXlsx::CellRange currentRange;
    QXlsx::CellReference dataExportStartCell;
#endif
private:
#ifdef HAVE_EXCEL
    QXlsx::Document* m_document = nullptr;
#endif
    QString m_fileName;
};


#endif // EXCELFILTERPRIVATE_H
