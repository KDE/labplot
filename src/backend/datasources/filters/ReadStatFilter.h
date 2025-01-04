/*
	File                 : ReadStatFilter.h
	Project              : LabPlot
	Description          : ReadStat I/O-filter
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2021 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef READSTATFILTER_H
#define READSTATFILTER_H

#include "backend/datasources/filters/AbstractFileFilter.h"

class ReadStatFilterPrivate;

class ReadStatFilter : public AbstractFileFilter {
	Q_OBJECT

public:
	ReadStatFilter();
	~ReadStatFilter() override;

	static QString fileInfoString(const QString&);

	QVector<QStringList> preview(const QString& fileName, int lines);
	void readDataFromFile(const QString& fileName, AbstractDataSource* = nullptr, ImportMode = ImportMode::Replace) override;
	void write(const QString& fileName, AbstractDataSource*) override;

	QStringList vectorNames() const;
	QVector<AbstractColumn::ColumnMode> columnModes() const;

	// TODO: put into base class?
	void setStartRow(const int);
	int startRow() const;
	void setEndRow(const int);
	int endRow() const;
	void setStartColumn(const int);
	int startColumn() const;
	void setEndColumn(const int);
	int endColumn() const;

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*) override;

private:
	std::unique_ptr<ReadStatFilterPrivate> const d;
	friend class ReadStatFilterPrivate;
};

#endif
