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

#include <KI18n/KLocalizedString>
#include <QStringList>
#include <QTreeWidgetItem>
#include <QVector>

#ifdef HAVE_ORCUS
#include <orcus/orcus_ods.hpp>
#include <orcus/spreadsheet/document.hpp>
#include <orcus/spreadsheet/factory.hpp>
#include <orcus/spreadsheet/sheet.hpp>

//#include <ixion/address.hpp>
//#include <ixion/model_context.hpp>

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

void OdsFilter::loadFilterSettings(const QString& filterName) {
	Q_UNUSED(filterName)
}

void OdsFilter::saveFilterSettings(const QString& filterName) const {
	Q_UNUSED(filterName)
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
	// TODO
}

void OdsFilterPrivate::readDataFromFile(const QString& fileName, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode importMode) {
	// TODO
}
