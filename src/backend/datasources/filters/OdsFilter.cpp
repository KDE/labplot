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
#include <QStringList>
#include <QTreeWidgetItem>
#include <QVector>

#ifdef HAVE_ORCUS
#include <orcus/orcus_ods.hpp>
#include <orcus/spreadsheet/factory.hpp>
#include <orcus/spreadsheet/sheet.hpp>

//#include <ixion/address.hpp>
#include <ixion/model_context.hpp>

//#include <iostream>
//#include <cstdlib>

using namespace orcus;
#endif

#include <utility>

OdsFilter::OdsFilter()
	: AbstractFileFilter(FileType::Ods)
	, d(new OdsFilterPrivate(this)) {
}

OdsFilter::~OdsFilter() {
}

QString OdsFilter::fileInfoString(const QString& fileName) {
#ifdef HAVE_ORCUS
	// OdsFilter filter;

	spreadsheet::range_size_t ss{1048576, 16384};
	spreadsheet::document doc{ss};
	spreadsheet::import_factory factory{doc};
	orcus_ods loader(&factory);

	loader.read_file(fileName.toStdString());

	const size_t nrSheets = doc.get_sheet_count();
	// const auto sheetSize = doc.get_sheet_size();
	auto dt = doc.get_origin_date();

	QString info(i18n("Sheet count: %1", QString::number(nrSheets)));
	info += QStringLiteral("<br>");
	// info += i18n("Sheet: %1 %2", sheetSize.rows, sheetSize.columns);
	//  document_config:
	// info += i18n("Precision: %1", docConfig.output_precision);
	//  const styles& doc.get_styles()
	//  const pivot_collection& doc.get_pivot_collection()
	//  const shared_strings& doc.get_shared_strings()

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
#endif
	return {};
}

void OdsFilter::readDataFromFile(const QString& fileName, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode importMode) {
	d->readDataFromFile(fileName, dataSource, importMode);
}
void OdsFilter::write(const QString& fileName, AbstractDataSource* dataSource) {
	d->write(fileName, dataSource);
}

QVector<QStringList> OdsFilter::preview(const QString& sheetName, int lines) {
	return d->preview(sheetName, lines);
}

void OdsFilter::parse(const QString& fileName, QTreeWidgetItem* root) {
	d->parse(fileName, root);
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

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################

/*!
  Saves as XML.
*/

void OdsFilter::save(QXmlStreamWriter*) const {
}

bool OdsFilter::load(XmlStreamReader*) {
	return true;
}

//#####################################################################
//################### Private implementation ##########################
//#####################################################################

OdsFilterPrivate::OdsFilterPrivate(OdsFilter* owner)
	: q(owner) {
}

OdsFilterPrivate::~OdsFilterPrivate() {
}

void OdsFilterPrivate::write(const QString& fileName, AbstractDataSource* dataSource) {
	DEBUG(Q_FUNC_INFO << ", not implemented yet!")
	// TODO
}

void OdsFilterPrivate::readDataFromFile(const QString& fileName, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode importMode){
	DEBUG(Q_FUNC_INFO << ", not implemented yet!")
	// TODO
}

QVector<QStringList> OdsFilterPrivate::preview(const QString& sheetName, int lines) {
	QVector<QStringList> dataString;
#ifdef HAVE_ORCUS
	// get sheet index by name and read lines of data into dataString
	auto* sheet = m_document.get_sheet(sheetName.toStdString());
	const auto index = sheet->get_index();
	if (index != ixion::invalid_sheet) {
		const auto ranges = sheet->get_data_range();
		DEBUG(Q_FUNC_INFO << ", data range: col " << ranges.first.column << ".." << ranges.last.column << ", row " << ranges.first.row << ".."
						  << ranges.last.row)
		// TODO: std::string get_sheet_name(sheet_t sheet
		// TODO: rc_size_t get_sheet_size()	-> whole size?
		const auto& model = m_document.get_model_context();

		const int maxCols = 50;
		for (ixion::row_t row = ranges.first.row; row < std::min(lines + ranges.first.row, ranges.last.row + 1); row++) {
			DEBUG(Q_FUNC_INFO << ", row " << row)
			QStringList line;
			for (ixion::col_t col = ranges.first.column; col < std::min(maxCols + ranges.first.column, ranges.last.column + 1); col++) {
				ixion::abs_address_t pos(index, row, col);

				auto type = model.get_celltype(pos);
				switch (type) {
				case ixion::celltype_t::string: {
					auto value = model.get_string_value(pos);
					DEBUG(Q_FUNC_INFO << " " << value)
					line << QString::fromStdString(std::string(value));
					break;
				}
				case ixion::celltype_t::numeric: {
					double value = model.get_numeric_value(pos);
					DEBUG(Q_FUNC_INFO << " " << value)
					line << QLocale().toString(value);
					break;
				}
				case ixion::celltype_t::formula: {
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
					// TODO: not available in ixion 0.17 ?
					// case ixion::formula_result::result_type::boolean:
					case ixion::formula_result::result_type::matrix:
						// TODO
						DEBUG(Q_FUNC_INFO << ", formula type error, boolean or matrix not implemented yet.")
						break;
					}
					break;
				}
				case ixion::celltype_t::empty:
					line << QString();
					break;
				case ixion::celltype_t::unknown:
				case ixion::celltype_t::boolean:
					// TODO
					DEBUG(Q_FUNC_INFO << ", cell type unknown or boolean not implemented yet.")
					break;
				}
			}
			dataString << line;
		}
	}
#endif

	return dataString;
}

void OdsFilterPrivate::parse(const QString& fileName, QTreeWidgetItem* parentItem) {
	DEBUG(Q_FUNC_INFO)
#ifdef HAVE_ORCUS
	m_document.clear();
	spreadsheet::import_factory factory{m_document};
	orcus_ods loader(&factory);

	loader.read_file(fileName.toStdString());

	auto* fileNameItem = new QTreeWidgetItem(QStringList() << fileName);
	parentItem->addChild(fileNameItem);

	const size_t nrSheets = m_document.get_sheet_count();
	for (size_t i = 0; i < nrSheets; i++) {
		auto name = m_document.get_sheet_name(i);

		auto* sheetItem = new QTreeWidgetItem(QStringList() << QString::fromStdString(std::string(name)));
		sheetItem->setIcon(0, QIcon::fromTheme(QStringLiteral("folder")));

		fileNameItem->addChild(sheetItem);
	}
#else
	Q_UNUSED(fileName)
	Q_UNUSED(parentItem)
#endif
}
