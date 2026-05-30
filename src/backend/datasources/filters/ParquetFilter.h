/*
	File                 : ParquetFilter.h
	Project              : LabPlot
	Description          : Parquet/Arrow IPC/ORC I/O-filter
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2026 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef PARQUETFILTER_H
#define PARQUETFILTER_H

#include "backend/datasources/filters/AbstractFileFilter.h"

class ParquetFilterPrivate;

#ifdef SDK
#include "labplot_export.h"
class LABPLOT_EXPORT ParquetFilter : public AbstractFileFilter {
#else
class ParquetFilter : public AbstractFileFilter {
#endif
	Q_OBJECT

public:
	explicit ParquetFilter(FileType type = FileType::Parquet);
	~ParquetFilter() override;

	static QString fileInfoString(const QString&);

	void readDataFromFile(const QString& fileName, AbstractDataSource* = nullptr, ImportMode = ImportMode::Replace) override;
	QVector<QStringList> preview(const QString& fileName, int lines);
	void write(const QString& fileName, AbstractDataSource*) override;

	QStringList columnNames() const;
	int columnCount() const;
	int rowCount() const;

	void setStartRow(int);
	int startRow() const;
	void setEndRow(int);
	int endRow() const;
	void setStartColumn(int);
	int startColumn() const;
	void setEndColumn(int);
	int endColumn() const;

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*) override;

private:
	std::unique_ptr<ParquetFilterPrivate> const d;

	friend class ParquetFilterPrivate;
};

#endif
