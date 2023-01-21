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

	QString info(i18n("Sheet count: %1", QString::number(0)));
	info += QLatin1String("<br>");
	info += i18n("Sheets: ");
	info += QLatin1String("<br>");

	info += QLatin1String("<br>");

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
