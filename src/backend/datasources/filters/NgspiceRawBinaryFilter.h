/*
File                 : NgspiceRawBinaryFilter.h
Project              : LabPlot
Description          : Ngspice RAW Binary filter
--------------------------------------------------------------------
SPDX-FileCopyrightText: 2018 Alexander Semke (alexander.semke@web.de)
SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef NGSPICERAWBINARYFILTER_H
#define NGSPICERAWBINARYFILTER_H

#include "backend/datasources/filters/AbstractFileFilter.h"
#include "backend/core/AbstractColumn.h"

class QStringList;
class NgspiceRawBinaryFilterPrivate;

class NgspiceRawBinaryFilter : public AbstractFileFilter {
	Q_OBJECT

public:
	NgspiceRawBinaryFilter();
	~NgspiceRawBinaryFilter() override;

	static bool isNgspiceBinaryFile(const QString& fileName);
	static QString fileInfoString(const QString&);

	void readDataFromFile(const QString& fileName, AbstractDataSource* = nullptr, AbstractFileFilter::ImportMode = AbstractFileFilter::ImportMode::Replace) override;
	void write(const QString& fileName, AbstractDataSource*) override;

	QVector<QStringList> preview(const QString& fileName, int lines);

	void loadFilterSettings(const QString&) override;
	void saveFilterSettings(const QString&) const override;

	QStringList vectorNames() const;
	QVector<AbstractColumn::ColumnMode> columnModes();

	void setStartRow(const int);
	int startRow() const;
	void setEndRow(const int);
	int endRow() const;

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*) override;

private:
	std::unique_ptr<NgspiceRawBinaryFilterPrivate> const d;
	friend class NgspiceRawBinaryFilterPrivate;
};

#endif
