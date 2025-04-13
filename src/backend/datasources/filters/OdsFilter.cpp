/*
	File                 : OdsFilter.cpp
	Project              : LabPlot
	Description          : Ods I/O-filter
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "backend/datasources/filters/OdsFilter.h"
#include "backend/core/column/Column.h"
#include "backend/datasources/AbstractDataSource.h"
#include "backend/datasources/filters/OdsFilterPrivate.h"
#include "backend/matrix/Matrix.h"
#include "backend/spreadsheet/Spreadsheet.h"

#include <KLocalizedString>
#include <QTreeWidgetItem>

#ifdef HAVE_ORCUS
#include <orcus/orcus_ods.hpp>
#include <orcus/spreadsheet/factory.hpp>
// env.hpp of libixion currently contains a bug (#if _WIN32) which breaks the build with -Werror
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wundef"
#include <orcus/spreadsheet/sheet.hpp>
#pragma GCC diagnostic pop

#include <ixion/model_context.hpp>

#ifndef HAVE_AT_LEAST_IXION_0_20_0
namespace ixion {
using cell_t = celltype_t;
}
#endif

using namespace orcus;
#endif

// TODO:
// * export data when Orcus support is stable
// * datetime support?
OdsFilter::OdsFilter()
	: AbstractFileFilter(FileType::Ods)
	, d(new OdsFilterPrivate(this)) {
}

OdsFilter::~OdsFilter() {
}

QString OdsFilter::fileInfoString(const QString& fileName) {
#ifdef HAVE_ORCUS
	spreadsheet::range_size_t ss{1048576, 16384};
	spreadsheet::document doc{ss};
	spreadsheet::import_factory factory{doc};
	orcus_ods loader(&factory);

	loader.read_file(fileName.toStdString());

	const size_t nrSheets = doc.get_sheet_count();
	auto dt = doc.get_origin_date();

	QString info(i18n("Sheet count: %1", QString::number(nrSheets)));
	info += QStringLiteral("<br>");

	for (size_t i = 0; i < nrSheets; ++i) {
		auto name = doc.get_sheet_name(i);
		info += QString::fromStdString(std::string(name));
		const auto* s = doc.get_sheet(i);
		auto r = s->get_data_range();

		info += QStringLiteral(" (") + QString::number(r.last.row - r.first.row + 1) + QStringLiteral(" x ")
			+ QString::number(r.last.column - r.first.column + 1) + QStringLiteral(")");
		if (i < nrSheets - 1)
			info += QStringLiteral(", ");
	}
	info += QStringLiteral("<br>");

	return info;
#else
	Q_UNUSED(fileName)
	return {};
#endif
}

void OdsFilter::readDataFromFile(const QString& fileName, AbstractDataSource* dataSource, ImportMode importMode) {
	d->readDataFromFile(fileName, dataSource, importMode);
}
void OdsFilter::write(const QString& fileName, AbstractDataSource* dataSource) {
	d->write(fileName, dataSource);
}

///////////////////////////////////////////////////////////////////////

void OdsFilter::setSelectedSheetNames(const QStringList& names) {
	d->currentSheetName = names.first();
	d->selectedSheetNames = names;
}
const QStringList OdsFilter::selectedSheetNames() const {
	return d->selectedSheetNames;
}

void OdsFilter::setFirstRowAsColumnNames(const bool b) {
	d->firstRowAsColumnNames = b;
}

QVector<QStringList> OdsFilter::preview(const QString& sheetName, int lines) {
	return d->preview(sheetName, lines);
}

bool OdsFilter::parse(const QString& fileName, QTreeWidgetItem* root) {
	return d->parse(fileName, root);
}

/*!
 * \brief Sets the startColumn to \a column
 * \param column the column to be set
 */
void OdsFilter::setStartColumn(const int column) {
	d->startColumn = column;
}

/*!
 * \brief Returns startColumn
 * \return The startColumn
 */
int OdsFilter::startColumn() const {
	return d->startColumn;
}

/*!
 * \brief Sets the endColumn to \a column
 * \param column the column to be set
 */
void OdsFilter::setEndColumn(const int column) {
	d->endColumn = column;
}

/*!
 * \brief Returns endColumn
 * \return The endColumn
 */
int OdsFilter::endColumn() const {
	return d->endColumn;
}

/* actual start column (including range) */
int OdsFilter::firstColumn() const {
	return d->firstColumn;
}

/*!
 * \brief Sets the startRow to \a row
 * \param row the row to be set
 */
void OdsFilter::setStartRow(const int row) {
	d->startRow = row;
}

/*!
 * \brief Returns startRow
 * \return The startRow
 */
int OdsFilter::startRow() const {
	return d->startRow;
}

/*!
 * \brief Sets the endRow to \a row
 * \param row the row to be set
 */
void OdsFilter::setEndRow(const int row) {
	d->endRow = row;
}

/*!
 * \brief Returns endRow
 * \return The endRow
 */
int OdsFilter::endRow() const {
	return d->endRow;
}

void OdsFilter::setCurrentSheetName(const QString& sheetName) {
	d->currentSheetName = sheetName;
}

// ##############################################################################
// ##################  Serialization/Deserialization  ###########################
// ##############################################################################

/*!
  Saves as XML.
*/

void OdsFilter::save(QXmlStreamWriter*) const {
}

bool OdsFilter::load(XmlStreamReader*) {
	return true;
}

// #####################################################################
// ################### Private implementation ##########################
// #####################################################################

OdsFilterPrivate::OdsFilterPrivate(OdsFilter* owner)
	: q(owner) {
}

OdsFilterPrivate::~OdsFilterPrivate() {
}

void OdsFilterPrivate::write(const QString& /*fileName*/, AbstractDataSource* /*dataSource*/) {
	DEBUG(Q_FUNC_INFO)
	// TODO: "The export functionality of the Orcus library is highly experimental."
	DEBUG(Q_FUNC_INFO << ", not implemented yet!")
}

void OdsFilterPrivate::readDataFromFile(const QString& fileName, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode importMode) {
	DEBUG(Q_FUNC_INFO)

	if (selectedSheetNames.isEmpty()) {
		DEBUG(Q_FUNC_INFO << ", WARNING: no sheet selected");
		q->setLastError(i18n("No sheet selected."));
		return;
	}

	// read data from selected sheets into dataSource using importMode
	// QDEBUG(Q_FUNC_INFO << ", Reading sheets" << selectedSheetNames)
	for (const auto& sheetName : selectedSheetNames) {
		// DEBUG(Q_FUNC_INFO << ", sheet " << sheetName.toStdString())
		currentSheetName = sheetName.split(QLatin1Char('!')).last();
		readCurrentSheet(fileName, dataSource, importMode);
		importMode = AbstractFileFilter::ImportMode::Append; // columns of other sheets are appended
	}
}

void OdsFilterPrivate::readCurrentSheet(const QString& fileName, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode importMode) {
	DEBUG(Q_FUNC_INFO << ", current sheet name = " << currentSheetName.toStdString() << ", first row as column names = " << firstRowAsColumnNames)

	if (!dataSource)
		return;

#ifdef HAVE_ORCUS
	DEBUG(Q_FUNC_INFO << ", sheet count = " << m_document.get_sheet_count())
	if (m_document.get_sheet_count() == 0) { // not loaded yet
		DEBUG(Q_FUNC_INFO << ", loading file " << fileName.toStdString())
		m_document.clear();
		spreadsheet::import_factory factory{m_document};
		orcus_ods loader(&factory);

		loader.read_file(fileName.toStdString());
	}

	// get sheet index from name and read data into dataSource
	auto* sheet = m_document.get_sheet(currentSheetName.toStdString());
	if (!sheet) {
		DEBUG(Q_FUNC_INFO << ", sheet not found: " << currentSheetName.toStdString())
		q->setLastError(i18n("Selected sheet not found."));
		return;
	}

	const auto sheetIndex = sheet->get_index();
	if (sheetIndex == ixion::invalid_sheet) {
		DEBUG(Q_FUNC_INFO << ", invalid sheet")
		q->setLastError(i18n("Invalid sheet."));
		return;
	}

	auto ranges = sheet->get_data_range();
	DEBUG(Q_FUNC_INFO << ", data range: col " << ranges.first.column << ".." << ranges.last.column << ", row " << ranges.first.row << ".." << ranges.last.row)
	if (firstRowAsColumnNames) // skip first row
		ranges.first.row++;
	size_t actualRows = ranges.last.row - ranges.first.row + 1;
	size_t actualEndRow = (endRow == -1 ? ranges.last.row + 1 : endRow);
	if ((size_t)startRow > actualRows)
		startRow = 1; // start from the begining
	DEBUG(Q_FUNC_INFO << ", start/end row = " << startRow << " " << endRow)
	DEBUG(Q_FUNC_INFO << ", start/end col = " << startColumn << " " << endColumn)
	actualRows = std::min(actualRows - startRow, (size_t)(actualEndRow - startRow)) + 1;

	size_t actualCols = ranges.last.column - ranges.first.column + 1;
	size_t actualEndColumn = (endColumn == -1 ? ranges.last.column + 1 : endColumn);
	if ((size_t)startColumn > actualCols)
		startColumn = 1; // start from the begining
	actualCols = std::min(actualCols - startColumn, (size_t)(actualEndColumn - startColumn)) + 1;

	DEBUG(Q_FUNC_INFO << ", actual rows/cols = " << actualRows << " " << actualCols)
	if (actualRows < 1 || actualCols < 1) {
		DEBUG(Q_FUNC_INFO << ", no actual rows of columns")
		return;
	}

	// column modes
	QVector<AbstractColumn::ColumnMode> columnModes;
	columnModes.resize(actualCols);

	// set column modes (only for spreadsheet, matrix uses default: Double)
	const auto& model = m_document.get_model_context();
	if (dynamic_cast<Spreadsheet*>(dataSource)) {
		for (size_t col = 0; col < actualCols; col++) {
			// check start row
			ixion::abs_address_t pos(sheetIndex, ranges.first.row + startRow - 1, ranges.first.column + startColumn - 1 + col);

			auto type = model.get_celltype(pos);
			switch (type) {
			case ixion::cell_t::string:
				columnModes[col] = AbstractColumn::ColumnMode::Text;
				break;
			case ixion::cell_t::numeric: // numeric values are always double (can't detect if integer)
				// default: Double
				break;
			case ixion::cell_t::formula: {
				auto formula = model.get_formula_result(pos);
				switch (formula.get_type()) { // conside formula type
				case ixion::formula_result::result_type::value:
					columnModes[col] = AbstractColumn::ColumnMode::Double;
					break;
				case ixion::formula_result::result_type::string:
					columnModes[col] = AbstractColumn::ColumnMode::Text;
					break;
				case ixion::formula_result::result_type::error:
#ifdef HAVE_AT_LEAST_IXION_0_18_0
				case ixion::formula_result::result_type::boolean:
#endif
				case ixion::formula_result::result_type::matrix:
					DEBUG(Q_FUNC_INFO << ", formula type not supported yet.")
					q->setLastError(i18n("Formulas not supported yet."));
					break;
				}
				break;
			}
			case ixion::cell_t::boolean:
			case ixion::cell_t::empty:
			case ixion::cell_t::unknown: // default: Double
				break;
			}
		}
	}

	QStringList vectorNames;
	if (firstRowAsColumnNames) {
		for (size_t col = 0; col < actualCols; col++) {
			ixion::abs_address_t pos(sheetIndex, ranges.first.row - 1 + startRow - 1, ranges.first.column + startColumn - 1 + col);

			auto type = model.get_celltype(pos);
			switch (type) {
			case ixion::cell_t::string: {
				auto value = model.get_string_value(pos);
				vectorNames << QString::fromStdString(std::string(value));
				break;
			}
			case ixion::cell_t::numeric: {
				double value = model.get_numeric_value(pos);
				vectorNames << QLocale().toString(value);
				break;
			}
			case ixion::cell_t::formula: {
				auto formula = model.get_formula_result(pos);
				switch (formula.get_type()) {
				case ixion::formula_result::result_type::value: {
					auto value = formula.get_value();
					vectorNames << QLocale().toString(value);
					break;
				}
				case ixion::formula_result::result_type::string:
					vectorNames << QString::fromStdString(formula.get_string());
					break;
				case ixion::formula_result::result_type::error:
#ifdef HAVE_AT_LEAST_IXION_0_18_0
				case ixion::formula_result::result_type::boolean:
#endif
				case ixion::formula_result::result_type::matrix:
					vectorNames << AbstractFileFilter::convertFromNumberToColumn(ranges.first.column + startColumn - 1 + col);
					;
					break;
				}
				// TODO
				break;
			}
			case ixion::cell_t::empty:
			case ixion::cell_t::unknown:
			case ixion::cell_t::boolean:
				vectorNames << AbstractFileFilter::convertFromNumberToColumn(ranges.first.column + startColumn - 1 + col);
				;
			}
		}
	} else {
		for (size_t col = 0; col < actualCols; col++)
			vectorNames << AbstractFileFilter::convertFromNumberToColumn(ranges.first.column + startColumn - 1 + col);
	}

	std::vector<void*> dataContainer;

	// prepare import
	bool ok = false;
	int columnOffset = dataSource->prepareImport(dataContainer, importMode, actualRows, actualCols, vectorNames, columnModes, ok);
	if (!ok) {
		q->setLastError(i18n("Not enough memory."));
		return;
	}

	DEBUG(Q_FUNC_INFO << ", column offset = " << columnOffset)

	// import data
	for (size_t row = 0; row < actualRows; row++) {
		for (size_t col = 0; col < actualCols; col++) {
			ixion::abs_address_t pos(sheetIndex, ranges.first.row + row + startRow - 1, ranges.first.column + col + startColumn - 1);

			auto type = model.get_celltype(pos);
			switch (type) {
			case ixion::cell_t::numeric: {
				double value = model.get_numeric_value(pos);
				// column mode may be non-numeric
				if (columnModes.at(col) == AbstractColumn::ColumnMode::Double)
					(*static_cast<QVector<double>*>(dataContainer[col]))[row] = value;
				else if (columnModes.at(col) == AbstractColumn::ColumnMode::Text)
					(*static_cast<QVector<QString>*>(dataContainer[col]))[row] = QLocale().toString(value);
				break;
			}
			case ixion::cell_t::formula: {
				// read formula result. We can't handle formulas yet (?)
				auto formula = model.get_formula_result(pos);
				switch (formula.get_type()) {
				case ixion::formula_result::result_type::value: {
					DEBUG(Q_FUNC_INFO << ", value formula found")
					auto value = formula.get_value();
					// text column may have value-type formula
					if (columnModes.at(col) == AbstractColumn::ColumnMode::Double)
						(*static_cast<QVector<double>*>(dataContainer[col]))[row] = value;
					else if (columnModes.at(col) == AbstractColumn::ColumnMode::Text)
						(*static_cast<QVector<QString>*>(dataContainer[col]))[row] = QLocale().toString(value);
					break;
				}
				case ixion::formula_result::result_type::string:
					DEBUG(Q_FUNC_INFO << ", string formula found")
					// column mode may be numeric
					if (columnModes.at(col) == AbstractColumn::ColumnMode::Double)
						(*static_cast<QVector<double>*>(dataContainer[col]))[row] = formula.get_value();
					else if (columnModes.at(col) == AbstractColumn::ColumnMode::Text)
						(*static_cast<QVector<QString>*>(dataContainer[col]))[row] = QString::fromStdString(formula.get_string());
					break;
				case ixion::formula_result::result_type::error:
#ifdef HAVE_AT_LEAST_IXION_0_18_0
				case ixion::formula_result::result_type::boolean:
#endif
				case ixion::formula_result::result_type::matrix:
					DEBUG(Q_FUNC_INFO << ", formula type not supported yet.")
					break;
				}
				break;
			}
			case ixion::cell_t::string: {
				// column mode may be numeric
				if (columnModes.at(col) == AbstractColumn::ColumnMode::Double)
					(*static_cast<QVector<double>*>(dataContainer[col]))[row] = model.get_numeric_value(pos);
				else if (columnModes.at(col) == AbstractColumn::ColumnMode::Text)
					(*static_cast<QVector<QString>*>(dataContainer[col]))[row] = QString::fromStdString(std::string(model.get_string_value(pos)));
				break;
			}
			case ixion::cell_t::empty: // nothing to do
				break;
			case ixion::cell_t::unknown:
			case ixion::cell_t::boolean:
				DEBUG(Q_FUNC_INFO << ", cell type unknown or boolean not supported yet.")
			}
		}
	}

	dataSource->finalizeImport(columnOffset, 1, actualCols, QString(), importMode);
#else
	Q_UNUSED(fileName)
	Q_UNUSED(dataSource)
	Q_UNUSED(importMode)
#endif
}

QVector<QStringList> OdsFilterPrivate::preview(const QString& sheetName, int lines) {
	QVector<QStringList> dataString;
#ifdef HAVE_ORCUS
	// get sheet index by name and read lines of data into dataString
	const auto* sheet = m_document.get_sheet(sheetName.toStdString());
	if (!sheet) {
		DEBUG(Q_FUNC_INFO << ", sheet not found: " << sheetName.toStdString())
		q->setLastError(i18n("Sheet not found."));
		return {};
	}

	const auto sheetIndex = sheet->get_index();
	if (sheetIndex == ixion::invalid_sheet) {
		DEBUG(Q_FUNC_INFO << ", invalid sheet index " << sheetIndex)
		q->setLastError(i18n("Invalid sheet index."));
		return {};
	}

	const auto ranges = sheet->get_data_range();
	DEBUG(Q_FUNC_INFO << ", data range: col " << ranges.first.column << ".." << ranges.last.column << ", row " << ranges.first.row << ".." << ranges.last.row)

	const int maxCols = 100; // max. columns to preview
	DEBUG(Q_FUNC_INFO << ", start/end row = " << startRow << " " << endRow)
	DEBUG(Q_FUNC_INFO << ", start/end col = " << startColumn << " " << endColumn)
	int actualStartRow = (startRow > (ranges.last.row - ranges.first.row + 1) ? ranges.first.row : ranges.first.row + startRow - 1);
	const int actualEndRow = (endRow == -1 ? ranges.last.row : std::min(ranges.last.row, ranges.first.row + endRow - 1));
	int actualStartCol = (startColumn > (ranges.last.column - ranges.first.column + 1) ? ranges.first.column : ranges.first.column + startColumn - 1);
	firstColumn = actualStartCol;
	const int actualEndCol = (endColumn == -1 ? ranges.last.column : std::min(ranges.last.column, ranges.first.column + endColumn - 1));

	const auto& model = m_document.get_model_context();
	for (ixion::row_t row = actualStartRow; row <= std::min(actualEndRow, actualStartRow + lines); row++) {
		DEBUG(Q_FUNC_INFO << ", row " << row)
		QStringList line;
		for (ixion::col_t col = actualStartCol; col <= std::min(actualEndCol, actualStartCol + maxCols); col++) {
			ixion::abs_address_t pos(sheetIndex, row, col);

			auto type = model.get_celltype(pos);
			switch (type) {
			case ixion::cell_t::string: {
				auto value = model.get_string_value(pos);
				DEBUG(Q_FUNC_INFO << " " << value)
				line << QString::fromStdString(std::string(value));
				break;
			}
			case ixion::cell_t::numeric: {
				double value = model.get_numeric_value(pos);
				DEBUG(Q_FUNC_INFO << " " << value)
				line << QLocale().toString(value);
				break;
			}
			case ixion::cell_t::formula: {
				// read formula result. We can't handle formulas yet (?)
				auto formula = model.get_formula_result(pos);
				switch (formula.get_type()) {
				case ixion::formula_result::result_type::value:
					line << QLocale().toString(formula.get_value());
					break;
				case ixion::formula_result::result_type::string:
					line << QString::fromStdString(formula.get_string());
					break;
				case ixion::formula_result::result_type::error:
#ifdef HAVE_AT_LEAST_IXION_0_18_0
				case ixion::formula_result::result_type::boolean:
#endif
				case ixion::formula_result::result_type::matrix:
					line << QString();
					DEBUG(Q_FUNC_INFO << ", formula type error, boolean or matrix not implemented yet.")
					break;
				}
				break;
			}
			case ixion::cell_t::empty:
				line << QString();
				break;
			case ixion::cell_t::unknown:
			case ixion::cell_t::boolean:
				line << QString();
				DEBUG(Q_FUNC_INFO << ", cell type unknown or boolean not implemented yet.")
				break;
			}
		}
		dataString << line;
	}
#else
	Q_UNUSED(sheetName)
	Q_UNUSED(lines)
#endif

	return dataString;
}

bool OdsFilterPrivate::parse(const QString& fileName, QTreeWidgetItem* parentItem) {
	DEBUG(Q_FUNC_INFO)
#ifdef HAVE_ORCUS
	m_document.clear();
	spreadsheet::import_factory factory{m_document};
	orcus_ods loader(&factory);

	try {
		loader.read_file(fileName.toStdString());
	} catch (const std::exception& e) {
		DEBUG(Q_FUNC_INFO << ", not a valid ODS file: " << fileName.toStdString())
		return false;
	}

	auto* fileNameItem = new QTreeWidgetItem(QStringList() << fileName);
	parentItem->addChild(fileNameItem);

	const size_t nrSheets = m_document.get_sheet_count();
	for (size_t i = 0; i < nrSheets; i++) {
		auto name = m_document.get_sheet_name(i);

		auto* sheetItem = new QTreeWidgetItem(QStringList() << QString::fromStdString(std::string(name)));
		sheetItem->setIcon(0, QIcon::fromTheme(QStringLiteral("folder")));

		fileNameItem->addChild(sheetItem);
	}

	return true;
#else
	Q_UNUSED(fileName)
	Q_UNUSED(parentItem)

	return false;
#endif
}
