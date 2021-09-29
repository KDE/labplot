/***************************************************************************
File                 : ExcelFilter.cpp
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
#include "backend/core/column/Column.h"
#include "backend/datasources/AbstractDataSource.h"
#include "backend/datasources/filters/ExcelFilter.h"
#include "backend/datasources/filters/ExcelFilterPrivate.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/matrix/Matrix.h"

#include <utility>

#include <KI18n/KLocalizedString>
#include <QVector>
#include <QTreeWidgetItem>
#include <QStringList>

ExcelFilter::ExcelFilter() : AbstractFileFilter(FileType::Excel), d(new ExcelFilterPrivate(this)) {}

ExcelFilter::~ExcelFilter() {

}
QString ExcelFilter::fileInfoString(const QString& fileName) {
#ifdef HAVE_EXCEL
    QXlsx::Document doc {fileName};

    ExcelFilter filter;

    QVector<int> rangesPerSheet;
    for (const auto& sheet : doc.sheetNames()) {
        rangesPerSheet.push_back(filter.dataRegions(fileName, sheet).size());
    }

    const QStringList& sheetNames = doc.sheetNames();
    QString info(i18n("Sheets count: %1", QString::number(sheetNames.size())));
    info += QLatin1String("<br>");
    info += i18n("Sheets: ");
    info += QLatin1String("<br>");

    for (int i = 0; i < sheetNames.size(); ++i) {
        info += sheetNames.at(i);
        info += QLatin1String(", ranges count:  ");
        info += QString::number(rangesPerSheet.at(i));
        info += QLatin1String("<br>");
    }
    info += QLatin1String("<br>");

    return info;
#else
    Q_UNUSED(fileName)
#endif
    return "";
}

QStringList ExcelFilter::sheets() const
{
    return d->sheets();
}


QStringList ExcelFilter::sheets(const QString& fileName, bool* ok) {
#ifdef HAVE_EXCEL
    QXlsx::Document doc {fileName};
    if (ok) {
        *ok = doc.isLoadPackage();
    }
    return doc.sheetNames();
#else
    Q_UNUSED(fileName)
#endif
    return {};
}

bool ExcelFilter::isValidCellReference(const QString& cellRefString) {
#ifdef HAVE_EXCEL
    QXlsx::CellReference ref {cellRefString};

    return ref.isValid();
#else
    Q_UNUSED(cellRefString)
#endif
    return false;
}

void ExcelFilter::readDataFromFile(const QString& fileName, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode importMode) {
    d->readDataFromFile(fileName, dataSource, importMode);
}
void ExcelFilter::write(const QString& fileName, AbstractDataSource* dataSource) {
    d->write(fileName, dataSource);
}

QVector<QStringList> ExcelFilter::previewForDataRegion(const QString& sheet, const QXlsx::CellRange& region, bool* okToMatrix, int lines) {
    return d->previewForDataRegion(sheet, region, okToMatrix, lines);
}

QVector<QStringList> ExcelFilter::previewForCurrentDataRegion(int lines, bool* okToMatrix) {
    return d->previewForDataRegion(d->currentSheet, d->currentRange, okToMatrix, lines);
}

void ExcelFilter::setExportAsNewSheet(const bool exportAsNewSheet) {
    d->exportDataSourceAsNewSheet = exportAsNewSheet;
}

void ExcelFilter::setSheetToAppendTo(const QString& sheetName) {
    d->sheetToAppendSpreadsheetTo = sheetName;
}

void ExcelFilter::setOverwriteData(const bool overwriteData) {
    d->overwriteExportData = overwriteData;
}

void ExcelFilter::setFirstRowAsColumnNames(const bool firstRowAsColumnNames) {
    d->firstRowAsColumnNames = firstRowAsColumnNames;
}

void ExcelFilter::setDataExportStartPos(const QString& dataStartPos) {
    const auto cell = QXlsx::CellReference(dataStartPos);
    if (cell.isValid()) {
        d->dataExportStartCell.setColumn(cell.column());
        d->dataExportStartCell.setRow(cell.row());
    }
}

QVector<QXlsx::CellRange> ExcelFilter::dataRegions(const QString& fileName, const QString& sheetName)  {
    return d->dataRegions(fileName, sheetName);
}

void ExcelFilter::parse(const QString& fileName, QTreeWidgetItem* root) {
    d->parse(fileName, root);
}

QXlsx::CellRange ExcelFilter::dimension() const {
    return d->dimension();
}

/*!
 * \brief Sets the startColumn to \a column
 * \param column the column to be set
 */
void ExcelFilter::setStartColumn(const int column) {
    d->startColumn = column;
}

/*!
 * \brief Returns startColumn
 * \return The startColumn
 */
int ExcelFilter::startColumn() const {
    return d->startColumn;
}

/*!
 * \brief Sets the endColumn to \a column
 * \param column the column to be set
 */
void ExcelFilter::setEndColumn(const int column) {
    d->endColumn = column;
}

/*!
 * \brief Returns endColumn
 * \return The endColumn
 */
int ExcelFilter::endColumn() const {
    return d->endColumn;
}

/*!
 * \brief Sets the startRow to \a row
 * \param row the row to be set
 */
void ExcelFilter::setStartRow(const int row) {
    d->startRow = row;
}

/*!
 * \brief Returns startRow
 * \return The startRow
 */
int ExcelFilter::startRow() const {
    return d->startRow;
}

/*!
 * \brief Sets the endRow to \a row
 * \param row the row to be set
 */
void ExcelFilter::setEndRow(const int row) {
    d->endRow = row;
}

/*!
 * \brief Returns endRow
 * \return The endRow
 */
int ExcelFilter::endRow() const {
    return d->endRow;
}

void ExcelFilter::setCurrentRange(const QString& range) {
    d->currentRange = {range};
}

void ExcelFilter::setCurrentSheet(const QString& sheet) {
    d->currentSheet = sheet;
}

void ExcelFilter::loadFilterSettings(const QString& filterName) {
    Q_UNUSED(filterName)
}

void ExcelFilter::saveFilterSettings(const QString& filterName) const {
    Q_UNUSED(filterName)
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################

/*!
  Saves as XML.
*/


void ExcelFilter::save(QXmlStreamWriter*) const {
}

bool ExcelFilter::load(XmlStreamReader*) {
    return true;
}

//#####################################################################
//################### Private implementation ##########################
//#####################################################################

ExcelFilterPrivate::ExcelFilterPrivate(ExcelFilter* owner) : q(owner) {}

ExcelFilterPrivate::~ExcelFilterPrivate() {
#ifdef HAVE_EXCEL
    if (m_document)
        delete m_document;
#endif
}

// TODO
//  alternating row colors?
//  bold "header" - colum names
void ExcelFilterPrivate::write(const QString& fileName, AbstractDataSource* dataSource) {
#ifdef HAVE_EXCEL
    if (!m_document || fileName.compare(m_fileName)) {
        //delete m_document;
        m_document = new QXlsx::Document(fileName);
        m_fileName = fileName;
    }

    auto dataSourceName = dataSource->name();
    if (exportDataSourceAsNewSheet) {
        const auto& sheets = m_document->sheetNames();
        if (sheets.contains(dataSourceName)) {
            dataSourceName += QLatin1String("_1");
        }
        m_document->addSheet(dataSourceName);
    } else {
        // there is (should be) a selected sheet in the widget
        if (!sheetToAppendSpreadsheetTo.isEmpty()) {
            // select the sheet
            if (!m_document->selectSheet(sheetToAppendSpreadsheetTo)) {
                // couldn't select sheet
            }
        }
    }

    int startRow = 1;
    int startCol = 1;

    // we're exporting in an existing sheet
    if (!exportDataSourceAsNewSheet) {
        // don't overwrite data
        if (!overwriteExportData) {
            // write data in the "end" of the currently occupied data range
            const auto& dimension = m_document->dimension();
            // +2 to leave an empty row
            startRow = dimension.lastRow() + 2;
        } else {
            // if there was a valid starting cell
            // otherwise we will just overwrite from row 1 col 1
            if (dataExportStartCell.isValid()) {
                startRow = dataExportStartCell.row();
                startCol = dataExportStartCell.column();
            }
        }
    }

    if (auto* const spreadsheet = dynamic_cast<Spreadsheet*>(dataSource)) {
        const int columns = spreadsheet->columnCount();
        const int rows = spreadsheet->rowCount();

        for (int col = 0; col < columns; ++col) {
            const auto* const column = spreadsheet->column(col);
            const int actualCol = startCol + col;

            if (columnNamesAsFirstRow) {
                if (!m_document->write(startRow, startCol + col, column->name())) {
                    // failed to write column name
                }
            }
            for (int row = 0; row < rows; ++row) {
                const int actualRow = columnNamesAsFirstRow ? startRow + row + 1 : startRow + row;
                const QString text = column->asStringColumn()->textAt(row);

                if (!m_document->write(actualRow, actualCol, text)) {
                    // failed to write
                }
            }
        }

    } else if (auto* const matrix = dynamic_cast<Matrix*>(dataSource)) {
        const int columns = matrix->columnCount();
        const int rows = matrix->rowCount();
        const QVector<QVector<double> >* const data = static_cast<QVector<QVector<double>>*>(matrix->data());

        for (int col = 0; col < columns; ++col) {
            const int actualCol = startCol + col;
            const auto& column = data->at(col);
            for (int row = 0; row < rows; ++row) {
                const int actualRow = startRow + row;
                const auto& val = column.at(row);

                if (!m_document->write(actualRow, actualCol, val)) {
                    // failed to write
                }
            }
        }
    }

    if (!m_document->save()) {
        // failed to save file
    }
#else
    Q_UNUSED(fileName)
    Q_UNUSED(dataSource)
#endif
}

void ExcelFilterPrivate::readDataFromFile(const QString& fileName, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode importMode) {
#ifdef HAVE_EXCEL
    if (!m_document || fileName.compare(m_fileName)) {
        delete m_document;
        m_document = new QXlsx::Document(fileName);
        m_fileName = fileName;
    }

    if (m_document->selectSheet(currentSheet)) {
        if (endRow != -1) {
            int row = currentRange.firstRow() + endRow - 1;
            if (row <= currentRange.lastRow())
                currentRange.setLastRow(row);
        }

        if (startRow > 1) {
            int rrow = currentRange.firstRow() + startRow - 1;
            if (rrow <= currentRange.lastRow())
                currentRange.setFirstRow(rrow);
            else
                currentRange.setFirstRow(currentRange.lastRow());
        }

        if (endColumn != -1) {
            int col = currentRange.firstColumn() + endColumn - 1;
            if (col <= currentRange.lastColumn())
                currentRange.setLastColumn(col);
        }

        if (startColumn > 1) {
            int col = currentRange.firstColumn() + startColumn - 1;
            if (col <= currentRange.lastColumn())
                currentRange.setFirstColumn(col);
            else
                currentRange.setFirstColumn(currentRange.lastColumn());
        }

        readDataRegion(currentRange, dataSource, importMode);
    }
    else {
        // invalid sheet
    }

#else
    Q_UNUSED(fileName)
    Q_UNUSED(dataSource)
    Q_UNUSED(importMode)
#endif
}

#ifdef HAVE_EXCEL
void ExcelFilterPrivate::readDataRegion(const QXlsx::CellRange& region, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode importMode) {
    DEBUG("HAS DataSource");

    int columnOffset = 0;
    const auto rowCount = currentRange.rowCount();
    const auto colCount = currentRange.columnCount();
    auto regionToRead = region;

    if (auto* spreadsheet = dynamic_cast<Spreadsheet*>(dataSource)) {
        QVector<QVector<QString>*> stringDataPointers;
        std::vector<void*> numericDataPointers;
        QList<bool> columnNumericTypes;
        QStringList columnNames;
        if (firstRowAsColumnNames)
            regionToRead.setFirstRow(region.firstRow() + 1);

        for (int col = regionToRead.firstColumn(); col <= regionToRead.lastColumn(); ++col) {

            if (firstRowAsColumnNames) {
                columnNumericTypes.push_back(isColumnNumericInRange(col, regionToRead));

                columnNames.push_back(m_document->read(regionToRead.firstRow() - 1, col).toString());
            } else {
                columnNumericTypes.push_back(isColumnNumericInRange(col, regionToRead));
                columnNames.push_back(ExcelFilter::convertFromNumberToExcelColumn(col));
            }
        }

        spreadsheet->setUndoAware(false);
        columnOffset = spreadsheet->resize(importMode, columnNames, colCount);

        if (importMode == AbstractFileFilter::ImportMode::Replace) {
            spreadsheet->clear();
            spreadsheet->setRowCount(rowCount);
        } else {
            if (spreadsheet->rowCount() < (rowCount))
                spreadsheet->setRowCount(rowCount);
        }

        numericDataPointers.reserve(colCount);
        stringDataPointers.reserve(colCount);

        for (int n = 0; n < colCount; ++n) {
            if (columnNumericTypes.at(n)) {
                spreadsheet->column(columnOffset+ n)->setColumnMode(AbstractColumn::ColumnMode::Numeric);
                auto* datap = static_cast<QVector<double>* >(spreadsheet->column(columnOffset+n)->data());
                numericDataPointers.push_back(datap);
                if (importMode == AbstractFileFilter::ImportMode::Replace)
                    datap->clear();
            } else {
                spreadsheet->column(columnOffset+ n)->setColumnMode(AbstractColumn::ColumnMode::Text);
                auto* list = static_cast<QVector<QString>*>(spreadsheet->column(columnOffset+n)->data());
                stringDataPointers.push_back(list);
                if (importMode == AbstractFileFilter::ImportMode::Replace)
                    list->clear();
            }
        }

        for (int row = regionToRead.firstRow(); row <= regionToRead.lastRow(); ++row) {
            int j = 0;
            int numericixd = 0;
            int stringidx = 0;
            for (int col = regionToRead.firstColumn(); col <= regionToRead.lastColumn(); ++col) {
                if (columnNumericTypes.at(j)) {
                    if (numericixd < numericDataPointers.size()) {
                        static_cast<QVector<double>*>(numericDataPointers[numericixd++])->push_back(m_document->read(row, col).toDouble());
                    }
                } else {
                    if (!stringDataPointers.isEmpty() && stringidx < stringDataPointers.size()) {
                        const auto val = m_document->read(row, col).toString();
                        stringDataPointers[stringidx++]->operator<<(val);
                    }
                }
                ++j;
            }
        }
    } else if (auto* matrix = dynamic_cast<Matrix*>(dataSource)){
        QVector<AbstractColumn::ColumnMode> columnModes;
        QStringList vectorNames;

        columnModes.resize(colCount);
        std::vector<void*> dataContainer;
        dataContainer.reserve(colCount);
        columnOffset = dataSource->prepareImport(dataContainer, importMode, rowCount, colCount, vectorNames, columnModes);

        int i = 0;
        for (int row = region.firstRow(); row <= region.lastRow(); ++row) {
            int j = 0;
            for (int col = region.firstColumn(); col <= region.lastColumn(); ++col) {

                static_cast<QVector<double>*>(dataContainer[j++])->operator[](i) = m_document->read(row, col).toDouble();
            }
            ++i;
        }
    }

    if (dataSource)
        dataSource->finalizeImport(columnOffset, 1, colCount, QString(), importMode);
}
#endif

#ifdef HAVE_EXCEL
QVector<QXlsx::CellRange> ExcelFilterPrivate::dataRegions(const QString& fileName, const QString& sheetName) {
    QVector<QXlsx::CellRange> regions;

    if (!m_document || fileName.compare(m_fileName)) {
        delete m_document;
        m_document = new QXlsx::Document(fileName);
        m_fileName = fileName;
    }

    if (!m_document->selectSheet(sheetName))
        return regions;

    const auto& sheetDimension = m_document->dimension();

    for (int row = sheetDimension.firstRow(); row <= sheetDimension.lastRow(); ++row) {
        for (int col = sheetDimension.firstColumn(); col <= sheetDimension.lastColumn(); ++col) {

            const auto& region = cellContainedInRegions({row, col}, regions);
            // if the cell is contained in a range already
            if (region.isValid()) {

                // skip the columns of the already found range
                col += region.columnCount();
                continue;
            }

            auto cellData = m_document->read(row, col);

            // if a cell with data was found
            if (!cellData.isNull()) {
                int _row = row;
                int _col = col;

                // find the last column of this data region
                do {
                    ++_col;
                    cellData = m_document->read(row, _col);

                } while(!cellData.isNull());

                // find the last row of this data region
                do {
                    ++_row;
                    cellData = m_document->read(_row, col);
                } while(!cellData.isNull());

                // _col and _row will be incremented even at the last cell of the region (which was empty)
                // now decrement that
                --_col;
                --_row;

                QXlsx::CellRange range;
                // it's not just a single cell
                if (_row > row || _col > col) {
                    range.setFirstColumn(col);
                    range.setLastColumn(_col);
                    range.setFirstRow(row);
                    range.setLastRow(_row);
                } else {
                    range.setFirstColumn(col);
                    range.setLastColumn(col);
                    range.setFirstRow(row);
                    range.setLastRow(row);
                }

                if (range.isValid()) {
                    regions.push_back(range);
                    // jump to the end of range
                    const int jump = _col - col + 1;
                    col += jump;

                    // special case - range goes until the last column
                    if (col == m_document->dimension().lastColumn()) {
                        row = _row;
                    }
                }
            }
        }
    }
    return regions;
}
#endif

#ifdef HAVE_EXCEL
QVector<QStringList> ExcelFilterPrivate::previewForDataRegion(const QString &sheet, const QXlsx::CellRange& region, bool* okToMatrix, int lines) {
    QVector<QStringList> infoString;

    if (!m_document) {
        delete m_document;
        m_document = new QXlsx::Document(m_fileName);
    }

    if (!m_document->selectSheet(sheet)) {
        // invalid sheet name
    } else {
        if (region.isValid()) {
            if (okToMatrix)
                if (dataRangeCanBeExportedToMatrix(region)) {
                    *okToMatrix = true;
                }
            const auto& documentRegion = m_document->dimension();
            if (region.lastRow() <= documentRegion.lastRow() &&
                    region.lastColumn() <= documentRegion.lastColumn()) {
                const int rows = std::min(lines, region.lastRow());
                for (int row = region.firstRow(); row <= rows; ++row) {
                    QStringList line;
                    for (int col = region.firstColumn(); col <= region.lastColumn(); ++col) {
                        const auto val = m_document->read(row, col);
                        line << val.toString();
                    }
                    infoString << line;
                }
            }
        }
    }

    return infoString;
}
#endif

#ifdef HAVE_EXCEL
QXlsx::CellRange ExcelFilterPrivate::cellContainedInRegions(const QXlsx::CellReference &cell, const QVector<QXlsx::CellRange>& regions) const {
    for (const auto& region : regions) {
        if (cell.column() >= region.firstColumn() && cell.column() <= region.lastColumn() &&
                cell.row() >= region.firstRow() && cell.row() <= region.lastRow())
            return region;
    }
    return {};
}
#endif

void ExcelFilterPrivate::parse(const QString& fileName, QTreeWidgetItem* parentItem) {
#ifdef HAVE_EXCEL
    m_document = new QXlsx::Document(fileName);
    m_fileName = fileName;

    const auto sheets = m_document->sheetNames();
    auto* fileNameItem = new QTreeWidgetItem(QStringList() << fileName);
    parentItem->addChild(fileNameItem);

    for (const auto& sheet: sheets) {
        const auto regionsForSheet = dataRegions(fileName, sheet);

        auto* sheetItem = new QTreeWidgetItem(QStringList() << sheet);
        sheetItem->setIcon(0, QIcon::fromTheme("folder"));
        sheetItem->setFlags(sheetItem->flags() & ~Qt::ItemIsSelectable);

        fileNameItem->addChild(sheetItem);

        for (const auto& region : regionsForSheet) {
            auto* regionItem = new QTreeWidgetItem(QStringList() << QString(region.toString()));
            regionItem->setIcon(0, QIcon::fromTheme("x-office-spreadsheet"));
            regionItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

            sheetItem->addChild(regionItem);
        }
    }
#else
    Q_UNUSED(fileName)
    Q_UNUSED(parentItem)
#endif
}

#ifdef HAVE_EXCEL
bool ExcelFilterPrivate::dataRangeCanBeExportedToMatrix(const QXlsx::CellRange& range) const {
    for (int i = range.firstRow(); i <= range.lastRow(); ++i) {
        for (int j = range.firstColumn(); j <= range.lastColumn(); ++j) {
            const auto* cell = m_document->cellAt(i, j);
            if (cell->cellType() != QXlsx::Cell::CellType::NumberType) {
                if (cell->cellType() == QXlsx::Cell::CellType::CustomType) {
                    bool ok = false;
                    cell->value().toDouble(&ok);
                    if (ok)
                        continue;
                }
                return false;
            }
        }
    }
    return true;
}
#endif

QStringList ExcelFilterPrivate::sheets() const {
#ifdef HAVE_EXCEL
    return m_document ? m_document->sheetNames(): QStringList();
#endif
}

#ifdef HAVE_EXCEL
QXlsx::CellRange ExcelFilterPrivate::dimension() const {
    return m_document ? m_document->dimension() : QXlsx::CellRange();
}
#endif

#ifdef HAVE_EXCEL
bool ExcelFilterPrivate::isColumnNumericInRange(const int column, const QXlsx::CellRange& range) const {
    if (column >= range.firstColumn() && column <= range.lastColumn()) {
        for (int row = range.firstRow(); row <= range.lastRow(); ++row) {
            const auto* cell = m_document->cellAt(row, column);
            if (cell->cellType() != QXlsx::Cell::CellType::NumberType) {
                if (cell->cellType() == QXlsx::Cell::CellType::CustomType) {
                    bool ok = false;
                    cell->value().toDouble(&ok);
                    if (ok)
                        continue;
                }
                return false;
            }
        }
    } else
        return false;

    return true;
}
#endif

QString ExcelFilter::convertFromNumberToExcelColumn(int n) {
    // main code from https://www.geeksforgeeks.org/find-excel-column-name-given-number/
    // Function to print Excel column name for a given column number

    QString string;

    char str[1000]; // To store result (Excel column name)
    int i = 0; // To store current index in str which is result

    while ( n > 0 )
    {
        // Find remainder
        int rem = n % 26;

        // If remainder is 0, then a 'Z' must be there in output
        if ( rem == 0 )
        {
            str[i++] = 'Z';
            n = (n/26) - 1;
        }
        else // If remainder is non-zero
        {
            str[i++] = (rem-1) + 'A';
            n = n / 26;
        }
    }
    str[i] = '\0';

    // Reverse the string and print result
    std::reverse( str, str + strlen(str));

    string = str;
    return string;
}
