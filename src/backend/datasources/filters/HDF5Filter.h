/*
    File                 : HDF5Filter.h
    Project              : LabPlot
    Description          : HDF5 I/O-filter
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2015-2017 Stefan Gerlach <stefan.gerlach@uni.kn>
    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef HDF5FILTER_H
#define HDF5FILTER_H

#include "backend/datasources/filters/AbstractFileFilter.h"

class QStringList;
class QTreeWidgetItem;
class HDF5FilterPrivate;

class HDF5Filter : public AbstractFileFilter {
	Q_OBJECT

public:
	HDF5Filter();
	~HDF5Filter() override;

	static QString fileInfoString(const QString&);
	static QString fileDDLString(const QString&);

	void parse(const QString& fileName, QTreeWidgetItem* rootItem);
	void readDataFromFile(const QString& fileName, AbstractDataSource* = nullptr, AbstractFileFilter::ImportMode = AbstractFileFilter::ImportMode::Replace) override;
	QVector<QStringList> readCurrentDataSet(const QString& fileName, AbstractDataSource*, bool& ok,
						AbstractFileFilter::ImportMode = AbstractFileFilter::ImportMode::Replace, int lines = -1);
	void write(const QString& fileName, AbstractDataSource*) override;

	void loadFilterSettings(const QString&) override;
	void saveFilterSettings(const QString&) const override;

	void setCurrentDataSetName(const QString&);
	const QString currentDataSetName() const;

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
	std::unique_ptr<HDF5FilterPrivate> const d;
	friend class HDF5FilterPrivate;
};

#endif
