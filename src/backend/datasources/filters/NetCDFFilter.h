/*
	File                 : NetCDFFilter.h
	Project              : LabPlot
	Description          : NetCDF I/O-filter
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2015 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef NETCDFFILTER_H
#define NETCDFFILTER_H

#include "backend/datasources/filters/AbstractFileFilter.h"
#include <QTreeWidgetItem>

class NetCDFFilterPrivate;

class NetCDFFilter : public AbstractFileFilter {
	Q_OBJECT

public:
	NetCDFFilter();
	~NetCDFFilter() override;

	static QString fileInfoString(const QString&);
	static QString fileCDLString(const QString&);

	void parse(const QString& fileName, QTreeWidgetItem* rootItem);
	void readDataFromFile(const QString& fileName, AbstractDataSource* = nullptr, ImportMode = ImportMode::Replace) override;
	QString readAttribute(const QString& fileName, const QString& name, const QString& varName);
	QVector<QStringList> readCurrentVar(const QString& fileName, AbstractDataSource* = nullptr, ImportMode = ImportMode::Replace, int lines = -1);
	void write(const QString& fileName, AbstractDataSource*) override;

	void setCurrentVarName(const QString&);
	const QString currentVarName() const;

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
	std::unique_ptr<NetCDFFilterPrivate> const d;
	friend class NetCDFFilterPrivate;
};

#endif
