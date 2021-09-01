/*
File                 : NgspiceRawAsciiFilter.h
Project              : LabPlot
Description          : Ngspice RAW ASCII filter
--------------------------------------------------------------------
SPDX-FileCopyrightText: 2018 Alexander Semke (alexander.semke@web.de)
SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef NGSPICERAWASCIIFILTER_H
#define NGSPICERAWASCIIFILTER_H

#include "backend/datasources/filters/AbstractFileFilter.h"
#include "backend/core/AbstractColumn.h"

class QStringList;
class NgspiceRawAsciiFilterPrivate;

class NgspiceRawAsciiFilter : public AbstractFileFilter {
	Q_OBJECT

public:
	NgspiceRawAsciiFilter();
	~NgspiceRawAsciiFilter() override;

	static bool isNgspiceAsciiFile(const QString& fileName);
	static QString fileInfoString(const QString&);

	QVector<QStringList> preview(const QString& fileName, int lines);
	void readDataFromFile(const QString& fileName, AbstractDataSource* = nullptr, AbstractFileFilter::ImportMode = AbstractFileFilter::ImportMode::Replace) override;
	void write(const QString& fileName, AbstractDataSource*) override;

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
	std::unique_ptr<NgspiceRawAsciiFilterPrivate> const d;
	friend class NgspiceRawAsciiFilterPrivate;
};

#endif
