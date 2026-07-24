/*
	File                 : XLSXFilter.cpp
	Project              : LabPlot
	Description          : XLSX I/O-filter
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2021 Fabian Kristof (fkristofszabolcs@gmail.com)
	SPDX-FileCopyrightText: 2022-2023 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifdef HAVE_QXLSX
#include "xlsxcellrange.h"
#include "xlsxcellreference.h"
#include "xlsxdocument.h"
#endif

#include "backend/core/column/ColumnStringIO.h"
#include "backend/datasources/filters/XLSXFilter.h"
#include "backend/datasources/filters/XLSXFilterPrivate.h"
#include "backend/matrix/Matrix.h"
#include "backend/spreadsheet/Spreadsheet.h"

#include <KLocalizedString>
#include <QTreeWidgetItem>

XLSXFilter::XLSXFilter()
	: AbstractFileFilter(FileType::XLSX)
	, d(new XLSXFilterPrivate(this)) {
}

XLSXFilter::~XLSXFilter() {
}

QString XLSXFilter::fileInfoString(const QString& fileName) {
#ifdef HAVE_QXLSX
	QXlsx::Document doc{fileName};

	XLSXFilter filter;

	QVector<int> rangesPerSheet;
	for (const auto& sheet : doc.sheetNames())
		rangesPerSheet.push_back(filter.d->dataRegions(fileName, sheet).size());

	const QStringList& sheetNames = doc.sheetNames();
	const int nrSheets = sheetNames.size();
	QString info(i18n("Sheet count: %1", QString::number(nrSheets)));
	info += QLatin1String("<br>");
	info += i18n("Sheets: ");
	info += QLatin1String("<br>");

	for (int i = 0; i < nrSheets; ++i) {
		info += sheetNames.at(i);
		info += QLatin1String(", ranges count:  ");
		info += QString::number(rangesPerSheet.at(i));
		info += QLatin1String("<br>");
	}
	info += QLatin1String("<br>");

	return info;
#else
	Q_UNUSED(fileName)
	return {};
#endif
}

QStringList XLSXFilter::sheets() const {
	return d->sheets();
}

QStringList XLSXFilter::sheets(const QString& fileName, bool* ok) {
#ifdef HAVE_QXLSX
	QXlsx::Document doc{fileName};
	if (ok)
		*ok = doc.isLoadPackage();
	return doc.sheetNames();
#else
	Q_UNUSED(fileName)
	Q_UNUSED(ok)
	return {};
#endif
}

bool XLSXFilter::isValidCellReference(const QString& cellRefString) {
#ifdef HAVE_QXLSX
	QXlsx::CellReference ref{cellRefString};

	return ref.isValid();
#else
	Q_UNUSED(cellRefString)
#endif
	return false;
}

void XLSXFilter::readDataFromFile(const QString& fileName, AbstractDataSource* dataSource, ImportMode importMode) {
	d->readDataFromFile(fileName, dataSource, importMode);
}
void XLSXFilter::write(const QString& fileName, AbstractDataSource* dataSource) {
	d->write(fileName, dataSource);
}

#ifdef HAVE_QXLSX
QVector<QStringList> XLSXFilter::previewForDataRegion(const QString& sheet, const QString& region, bool* okToMatrix, int lines) {
	return d->previewForDataRegion(sheet, region, okToMatrix, lines);
}
#endif

QVector<QStringList> XLSXFilter::previewForCurrentDataRegion(int lines, bool* okToMatrix) {
#ifdef HAVE_QXLSX
	return d->previewForDataRegion(d->currentSheet, d->currentRange, okToMatrix, lines);
#else
	Q_UNUSED(lines)
	Q_UNUSED(okToMatrix)
	return {};
#endif
}

void XLSXFilter::setSheetToAppendTo(const QString& sheetName) {
	d->sheetToAppendSpreadsheetTo = sheetName;
}

void XLSXFilter::setExportAsNewSheet(const bool b) {
	d->exportDataSourceAsNewSheet = b;
}

void XLSXFilter::setOverwriteData(const bool b) {
	d->overwriteExportData = b;
}

void XLSXFilter::setFirstRowAsColumnNames(const bool b) {
	d->firstRowAsColumnNames = b;
}
void XLSXFilter::setColumnNamesAsFirstRow(const bool b) {
	d->columnNamesAsFirstRow = b;
}

void XLSXFilter::setDataExportStartPos(const QString& dataStartPos) {
#ifdef HAVE_QXLSX
	const auto cell = QXlsx::CellReference(dataStartPos);
	if (cell.isValid()) {
		d->dataExportStartCell.setColumn(cell.column());
		d->dataExportStartCell.setRow(cell.row());
	}
#else
	Q_UNUSED(dataStartPos)
#endif
}

void XLSXFilter::parse(const QString& fileName, QTreeWidgetItem* root) {
	d->parse(fileName, root);
}

/*!
 * \brief Sets the startColumn to \a column
 * \param column the column to be set
 */
void XLSXFilter::setStartColumn(const int column) {
	d->startColumn = column;
}

/*!
 * \brief Returns startColumn
 * \return The startColumn
 */
int XLSXFilter::startColumn() const {
	return d->startColumn;
}

/*!
 * \brief Sets the endColumn to \a column
 * \param column the column to be set
 */
void XLSXFilter::setEndColumn(const int column) {
	d->endColumn = column;
}

/*!
 * \brief Returns endColumn
 * \return The endColumn
 */
int XLSXFilter::endColumn() const {
	return d->endColumn;
}

/* actual start column (including range) */
int XLSXFilter::firstColumn() const {
	return d->firstColumn;
}

/*!
 * \brief Sets the startRow to \a row
 * \param row the row to be set
 */
void XLSXFilter::setStartRow(const int row) {
	d->startRow = row;
}

/*!
 * \brief Returns startRow
 * \return The startRow
 */
int XLSXFilter::startRow() const {
	return d->startRow;
}

/*!
 * \brief Sets the endRow to \a row
 * \param row the row to be set
 */
void XLSXFilter::setEndRow(const int row) {
	d->endRow = row;
}

/*!
 * \brief Returns endRow
 * \return The endRow
 */
int XLSXFilter::endRow() const {
	return d->endRow;
}

void XLSXFilter::setCurrentRange(const QString& range) {
#ifdef HAVE_QXLSX
	d->currentRange = {range};
#else
	Q_UNUSED(range)
#endif
}

void XLSXFilter::setCurrentSheet(const QString& sheet) {
	d->currentSheet = sheet;
}

// ##############################################################################
// ##################  Serialization/Deserialization  ###########################
// ##############################################################################

/*!
  Saves as XML.
*/

void XLSXFilter::save(QXmlStreamWriter*) const {
}

bool XLSXFilter::load(XmlStreamReader*) {
	return true;
}

// #####################################################################
// ################### Private implementation ##########################
// #####################################################################

XLSXFilterPrivate::XLSXFilterPrivate(XLSXFilter* owner)
	: q(owner) {
}

XLSXFilterPrivate::~XLSXFilterPrivate() {
#ifdef HAVE_QXLSX
	if (m_document)
		delete m_document;
#endif
}

// TODO
//  alternating row colors?
//  bold "header" - column names
void XLSXFilterPrivate::write(const QString& fileName, AbstractDataSource* dataSource) {
#ifdef HAVE_QXLSX
	if (!m_document || fileName.compare(m_fileName)) {
		// delete m_document;
		m_document = new QXlsx::Document(fileName);
		m_fileName = fileName;
	}

	auto dataSourceName = dataSource->name();
	if (exportDataSourceAsNewSheet) {
		const auto& sheets = m_document->sheetNames();
		if (sheets.contains(dataSourceName))
			dataSourceName += QLatin1String("_1");

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
		const int columnCount = spreadsheet->columnCount();
		const int rowCount = spreadsheet->rowCount();

		for (int col = 0; col < columnCount; ++col) {
			const auto* const column = spreadsheet->column(col);
			const int actualCol = startCol + col;

			if (columnNamesAsFirstRow) {
				if (!m_document->write(startRow, startCol + col, column->name())) {
					// failed to write column name
				}
			}
			for (int row = 0; row < rowCount; ++row) {
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
		const QVector<QVector<double>>* const data = static_cast<QVector<QVector<double>>*>(matrix->data());

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

void XLSXFilterPrivate::readDataFromFile(const QString& fileName, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode importMode) {
	DEBUG(Q_FUNC_INFO)
#ifdef HAVE_QXLSX
	if (!m_document || fileName.compare(m_fileName)) {
		delete m_document;
		m_document = new QXlsx::Document(fileName);
		m_fileName = fileName;
	}

	if (m_document->selectSheet(currentSheet)) {
		if (endRow != -1) {
			int lastRow = currentRange.firstRow() + endRow - 1;
			if (lastRow <= currentRange.lastRow())
				currentRange.setLastRow(lastRow);
		}

		if (startRow > 1) {
			int firstRow = currentRange.firstRow() + startRow - 1;
			if (firstRow <= currentRange.lastRow())
				currentRange.setFirstRow(firstRow);
		}

		if (endColumn != -1) {
			int lastCol = currentRange.firstColumn() + endColumn - 1;
			if (lastCol <= currentRange.lastColumn())
				currentRange.setLastColumn(lastCol);
		}

		if (startColumn > 1) {
			int firstCol = currentRange.firstColumn() + startColumn - 1;
			if (firstCol <= currentRange.lastColumn())
				currentRange.setFirstColumn(firstCol);
		}

		readDataRegion(currentRange, dataSource, importMode);
	} else {
		DEBUG(Q_FUNC_INFO << ", INVALID sheet")
	}

#else
	Q_UNUSED(fileName)
	Q_UNUSED(dataSource)
	Q_UNUSED(importMode)
#endif
}

#ifdef HAVE_QXLSX
void XLSXFilterPrivate::readDataRegion(const QXlsx::CellRange& region, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode importMode) {
	DEBUG(Q_FUNC_INFO << ", col/row range = " << region.firstColumn() << " .. " << region.lastColumn() << ", " << region.firstRow() << " .. "
					  << region.lastRow() << ". first row as column names = " << firstRowAsColumnNames)

	int columnOffset = 0;
	const auto rowCount = currentRange.rowCount();
	const auto colCount = currentRange.columnCount();
	auto regionToRead = region;
	bool isDateOnly = true;

	if (auto* spreadsheet = dynamic_cast<Spreadsheet*>(dataSource)) {
		std::vector<void*> numericDataPointers;
		QVector<QVector<QDateTime>*> datetimeDataPointers;
		QVector<QVector<QString>*> stringDataPointers;
		QList<QXlsx::Cell::CellType> columnNumericTypes;
		QStringList columnNames;
		if (firstRowAsColumnNames)
			regionToRead.setFirstRow(region.firstRow() + 1);

		// determine column type (numeric or not)
		for (int col = regionToRead.firstColumn(); col <= regionToRead.lastColumn(); ++col) {
			columnNumericTypes.push_back(columnTypeInRange(col, regionToRead));
			if (firstRowAsColumnNames)
				columnNames.push_back(read(regionToRead.firstRow() - 1, col).toString());
			else
				columnNames.push_back(AbstractFileFilter::convertFromNumberToColumn(col));
		}

		spreadsheet->setUndoAware(false);

		// resize spreadsheet (columns only)
		columnOffset = spreadsheet->resize(importMode, columnNames, colCount);
		DEBUG(Q_FUNC_INFO << ", column count = " << colCount << ", offset = " << columnOffset)

		// set new row count
		if (importMode == AbstractFileFilter::ImportMode::Replace) {
			spreadsheet->clear();
			spreadsheet->setRowCount(rowCount - firstRowAsColumnNames);
		} else {
			// grow if necessary
			if (spreadsheet->rowCount() < rowCount - firstRowAsColumnNames)
				spreadsheet->setRowCount(rowCount - firstRowAsColumnNames);
		}

		// handle new columns
		for (int n = 0; n < colCount; ++n) {
			auto* col = spreadsheet->column(columnOffset + n);
			if (columnNumericTypes.at(n) == QXlsx::Cell::CellType::NumberType) {
				col->setColumnMode(AbstractColumn::ColumnMode::Double);
				auto* data = static_cast<QVector<double>*>(col->data());
				data->clear();
				numericDataPointers.push_back(data);
			} else if (columnNumericTypes.at(n) == QXlsx::Cell::CellType::DateType) {
				col->setColumnMode(AbstractColumn::ColumnMode::DateTime);
				auto* data = static_cast<QVector<QDateTime>*>(col->data());
				data->clear();
				datetimeDataPointers.push_back(data);
			} else {
				col->setColumnMode(AbstractColumn::ColumnMode::Text);
				auto* data = static_cast<QVector<QString>*>(col->data());
				data->clear();
				stringDataPointers.push_back(data);
			}
		}

		// add data from data region
		for (int row = regionToRead.firstRow(); row <= regionToRead.lastRow(); ++row) {
			int j = 0;
			unsigned int numericidx = 0;
			int datetimeidx = 0;
			int stringidx = 0;
			for (int col = regionToRead.firstColumn(); col <= regionToRead.lastColumn(); ++col) {
				const auto val = read(row, col);
				if (columnNumericTypes.at(j) == QXlsx::Cell::CellType::NumberType) {
					if (numericidx < numericDataPointers.size())
						static_cast<QVector<double>*>(numericDataPointers[numericidx++])->push_back(val.toDouble());
				} else if (columnNumericTypes.at(j) == QXlsx::Cell::CellType::DateType) {
					// QDEBUG("DATETIME:" << val.toDateTime())
					if (datetimeidx < datetimeDataPointers.size()) {
						if (val.toDateTime().time() != QTime(0, 0))
							isDateOnly = false;
						static_cast<QVector<QDateTime>*>(datetimeDataPointers[datetimeidx++])->push_back(val.toDateTime());
					}
				} else {
					if (!stringDataPointers.isEmpty() && stringidx < stringDataPointers.size()) {
						const auto s = val.toString();
						stringDataPointers[stringidx++]->operator<<(s);
					}
				}
				++j;
			}
		}
	} else if (dynamic_cast<Matrix*>(dataSource)) {
		QVector<AbstractColumn::ColumnMode> columnModes;
		QStringList vectorNames;

		columnModes.resize(colCount);
		std::vector<void*> dataContainer;
		dataContainer.reserve(colCount);
		bool ok = false;
		columnOffset = dataSource->prepareImport(dataContainer, importMode, rowCount, colCount, vectorNames, columnModes, ok);
		if (!ok) {
			q->setLastError(i18n("Not enough memory."));
			return;
		}

		int i = 0;
		for (int row = region.firstRow(); row <= region.lastRow(); ++row) {
			int j = 0;
			for (int col = region.firstColumn(); col <= region.lastColumn(); ++col)
				static_cast<QVector<double>*>(dataContainer[j++])->operator[](i) = read(row, col).toDouble();
			++i;
		}
	}

	QLatin1String datetimeFormat;
	if (isDateOnly)
		datetimeFormat = QLatin1String("ddd MMM d yyyy");
	else
		datetimeFormat = QLatin1String("ddd MMM dd hh:mm:ss yyyy");

	if (dataSource)
		dataSource->finalizeImport(columnOffset, 1, colCount, datetimeFormat, importMode);
}
#endif

#ifdef HAVE_QXLSX
QVector<QXlsx::CellRange> XLSXFilterPrivate::dataRegions(const QString& fileName, const QString& sheetName) {
	DEBUG(Q_FUNC_INFO << ", sheet = " << STDSTRING(sheetName))
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

			auto cellData = read(row, col);

			// if a cell with data was found
			if (!cellData.isNull()) {
				int _row = row;
				int _col = col;

				// find the last column of this data region
				do {
					++_col;
					cellData = read(row, _col);

				} while (!cellData.isNull());

				// find the last row of this data region
				do {
					++_row;
					cellData = read(_row, col);
				} while (!cellData.isNull());

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

#ifdef HAVE_QXLSX
QVector<QStringList> XLSXFilterPrivate::previewForDataRegion(const QString& sheetName, const QXlsx::CellRange& region, bool* okToMatrix, int lines) {
	DEBUG(Q_FUNC_INFO << ", sheet name = " << STDSTRING(sheetName))

	if (!m_document) {
		delete m_document;
		m_document = new QXlsx::Document(m_fileName);
	}

	if (!m_document->selectSheet(sheetName)) {
		q->setLastError(i18n("No sheet selected."));
		return {};
	}
	if (!region.isValid()) {
		q->setLastError(i18n("Invalid region specified."));
		return {};
	}

	if (okToMatrix && dataRangeCanBeExportedToMatrix(region))
		*okToMatrix = true;

	const auto& documentRegion = m_document->dimension();
	QVector<QStringList> infoString;
	if (region.lastRow() <= documentRegion.lastRow() && region.lastColumn() <= documentRegion.lastColumn()) {
		DEBUG(Q_FUNC_INFO << ", region first/last row = " << region.firstRow() << " " << region.lastRow())
		DEBUG(Q_FUNC_INFO << ", region first/last column = " << region.firstColumn() << " " << region.lastColumn())
		DEBUG(Q_FUNC_INFO << ", start/end row = " << startRow << " " << endRow)
		DEBUG(Q_FUNC_INFO << ", start/end col = " << startColumn << " " << endColumn)

		int rows = region.lastRow() - region.firstRow() + 1;
		if (startRow > rows) // if startRow is bigger than available rows -> show all
			startRow = 1;
		if (endRow == -1 || endRow < startRow || endRow > rows)
			endRow = rows;
		int cols = region.lastColumn() - region.firstColumn() + 1;
		if (startColumn > cols) // if startColumn is bigger than available columns -> show all
			startColumn = 1;
		if (endColumn == -1 || endColumn < startColumn || endColumn > cols)
			endColumn = cols;
		firstColumn = startColumn;

		const int maxCols = 100;
		rows = std::min(lines, endRow);
		cols = std::min(maxCols, endColumn);
		for (int row = region.firstRow() + startRow - 1; row < region.firstRow() + rows; ++row) {
			QStringList line;
			for (int col = region.firstColumn() + startColumn - 1; col < region.firstColumn() + cols; ++col) {
				// see https://github.com/QtExcel/QXlsx/wiki for read() vs. cellAt()->value()
				const auto val = read(row, col);
				if (val.isValid())
					QDEBUG("value =" << val)

				// Excel: double is always QString
				// OO: can always convert to Double

				// correctly read values and show with locale
				if (val.userType() == QMetaType::Double)
					line << QLocale().toString(val.toDouble());
				else if (val.userType() == QMetaType::QString) {
					QString valueString = val.toString();

					bool ok; // check if double value
					double value = valueString.toDouble(&ok);
					if (ok)
						line << QLocale().toString(value);
					else
						line << valueString;
				} else if (val.canConvert<QDateTime>()) {
					QDateTime dt = val.toDateTime();
					// TODO: use certain date/datetime format?
					if (dt.time() == QTime(0, 0)) // just a date
						line << val.toDate().toString();
					else
						line << dt.toString();
				} else if (val.canConvert<QTime>()) {
					QTime t = val.toTime();
					// TODO: use certain time format?
					line << t.toString();
				} else {
					QString valueString = val.toString();
					line << valueString;
				}
			}
			infoString << line;
		}
	}

	return infoString;
}
#endif

#ifdef HAVE_QXLSX
QXlsx::CellRange XLSXFilterPrivate::cellContainedInRegions(const QXlsx::CellReference& cell, const QVector<QXlsx::CellRange>& regions) const {
	for (const auto& region : regions) {
		if (cell.column() >= region.firstColumn() && cell.column() <= region.lastColumn() && cell.row() >= region.firstRow() && cell.row() <= region.lastRow())
			return region;
	}
	return {};
}
#endif

void XLSXFilterPrivate::parse(const QString& fileName, QTreeWidgetItem* parentItem) {
	DEBUG(Q_FUNC_INFO)
#ifdef HAVE_QXLSX
	m_document = new QXlsx::Document(fileName);
	m_fileName = fileName;

	const auto sheets = m_document->sheetNames();
	auto* fileNameItem = new QTreeWidgetItem(QStringList() << fileName);
	parentItem->addChild(fileNameItem);

	for (const auto& sheet : sheets) {
		const auto regionsForSheet = dataRegions(fileName, sheet);

		auto* sheetItem = new QTreeWidgetItem(QStringList() << sheet);
		sheetItem->setIcon(0, QIcon::fromTheme(QStringLiteral("folder")));
		sheetItem->setFlags(sheetItem->flags() & ~Qt::ItemIsSelectable);

		fileNameItem->addChild(sheetItem);

		for (const auto& region : regionsForSheet) {
			auto* regionItem = new QTreeWidgetItem(QStringList() << QString(region.toString()));
			regionItem->setIcon(0, QIcon::fromTheme(QStringLiteral("x-office-spreadsheet")));
			regionItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

			sheetItem->addChild(regionItem);
		}
	}
#else
	Q_UNUSED(fileName)
	Q_UNUSED(parentItem)
#endif
}

#ifdef HAVE_QXLSX
bool XLSXFilterPrivate::dataRangeCanBeExportedToMatrix(const QXlsx::CellRange& range) const {
	for (int i = range.firstRow(); i <= range.lastRow(); ++i) {
		for (int j = range.firstColumn(); j <= range.lastColumn(); ++j) {
			const auto cell = m_document->cellAt(i, j);
			if (cell && cell->cellType() != QXlsx::Cell::CellType::NumberType) {
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

QStringList XLSXFilterPrivate::sheets() const {
#ifdef HAVE_QXLSX
	return m_document ? m_document->sheetNames() : QStringList();
#else
	return {};
#endif
}

#ifdef HAVE_QXLSX
QXlsx::CellRange XLSXFilterPrivate::dimension() const {
	return m_document ? m_document->dimension() : QXlsx::CellRange();
}
#endif

#ifdef HAVE_QXLSX
QXlsx::Cell::CellType XLSXFilterPrivate::columnTypeInRange(const int column, const QXlsx::CellRange& range) const {
	bool numeric = false, datetime = false;
	if (column >= range.firstColumn() && column <= range.lastColumn()) {
		for (int row = range.firstRow(); row <= range.lastRow(); ++row) {
			const auto cell = m_document->cellAt(row, column);
			if (!cell)
				continue;

			// QDEBUG(" cell type =" << cell->cellType())
			if (cell->cellType() == QXlsx::Cell::CellType::StringType)
				return QXlsx::Cell::CellType::StringType;
			if (cell->cellType() == QXlsx::Cell::CellType::NumberType)
				numeric = true;
			else if (cell->cellType() == QXlsx::Cell::CellType::DateType) // switch to date if date cell
				datetime = true;
			else if (cell->cellType() == QXlsx::Cell::CellType::CustomType) {
				bool ok = false;
				cell->value().toDouble(&ok);
				if (ok)
					numeric = true;
				else
					return QXlsx::Cell::CellType::StringType;
			}
		}
	}
	if (numeric && !datetime)
		return QXlsx::Cell::CellType::NumberType;
	if (datetime && !numeric)
		return QXlsx::Cell::CellType::DateType;

	// numeric and datetime
	return QXlsx::Cell::CellType::StringType;
}
#endif

#ifdef HAVE_QXLSX
QVariant XLSXFilterPrivate::read(int row, int column) const {
	auto cell = m_document->cellAt(row, column);
	if (!cell)
		return QVariant();

	if (cell->isDateTime())
		return cell->dateTime();

	return cell->value();
}
#endif
