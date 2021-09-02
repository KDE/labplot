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
//#include <QTreeWidgetItem>

#ifdef HAVE_READSTAT
#include <readstat.h>
#endif

//class QStringList;
class ReadStatFilterPrivate;

class ReadStatFilter : public AbstractFileFilter {
	Q_OBJECT

public:
	ReadStatFilter();
	~ReadStatFilter() override;

#ifdef HAVE_READSTAT
	static int getMetaData(readstat_metadata_t *, void *);
#endif

	static QString fileInfoString(const QString&);

	QVector<QStringList> preview(const QString& fileName, int lines);
	void readDataFromFile(const QString& fileName, AbstractDataSource* = nullptr, AbstractFileFilter::ImportMode = AbstractFileFilter::ImportMode::Replace) override;
	void write(const QString& fileName, AbstractDataSource*) override;


	void loadFilterSettings(const QString&) override;
	void saveFilterSettings(const QString&) const override;

	QStringList vectorNames() const;
	QVector<AbstractColumn::ColumnMode> columnModes() const;

	//TODO: put into base class?
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
