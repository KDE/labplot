/*
	File                 : ParquetFilterPrivate.h
	Project              : LabPlot
	Description          : Private implementation class for ParquetFilter.
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2026 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef PARQUETFILTERPRIVATE_H
#define PARQUETFILTERPRIVATE_H

#include "backend/datasources/filters/AbstractFileFilter.h"

#ifdef HAVE_PARQUET
#include <arrow/api.h>
#include <arrow/io/api.h>
#endif

class AbstractDataSource;

class ParquetFilterPrivate {
public:
	explicit ParquetFilterPrivate(ParquetFilter*);

	void readDataFromFile(const QString& fileName, AbstractDataSource* = nullptr, AbstractFileFilter::ImportMode = AbstractFileFilter::ImportMode::Replace);
	QVector<QStringList> preview(const QString& fileName, int lines);
	void write(const QString& fileName, AbstractDataSource*);

	int startRow{1};
	int endRow{-1};
	int startColumn{1};
	int endColumn{-1};

	// cached after reading metadata
	QStringList columnNames;
	int numColumns{0};
	int numRows{0};

	ParquetFilter* const q;
	AbstractFileFilter::FileType fileType{AbstractFileFilter::FileType::Parquet};

private:
#ifdef HAVE_PARQUET
	std::shared_ptr<arrow::Table> readArrowTable(const QString& fileName);
	void importFromTable(const std::shared_ptr<arrow::Table>& table, AbstractDataSource*, AbstractFileFilter::ImportMode);
	QVector<QStringList> previewFromTable(const std::shared_ptr<arrow::Table>& table, int lines);
#endif
};

#endif
