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
	void readCurrentSheet(const QString& fileName, AbstractDataSource*, AbstractFileFilter::ImportMode);
	void write(const QString& fileName, AbstractDataSource*);
	QVector<QStringList> preview(const QString& sheetName, int lines);
	void parse(const QString& fileName, QTreeWidgetItem* root);
	QString currentSheetName;
	QStringList selectedSheetNames;
	bool firstRowAsColumnNames{false};
	int startRow{1};
	int endRow{-1};
	int startColumn{1};
	int endColumn{-1};
	int firstColumn{1}; // actual start column (including range)

	OdsFilter* const q;

private:
#ifdef HAVE_ORCUS
	orcus::spreadsheet::range_size_t m_ss{1048576, 16384};
	orcus::spreadsheet::document m_document{m_ss};
#endif
};

#endif
