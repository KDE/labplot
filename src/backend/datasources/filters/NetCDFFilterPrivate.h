/*
File                 : NetCDFFilterPrivate.h
Project              : LabPlot
Description          : Private implementation class for NetCDFFilter.
--------------------------------------------------------------------
SPDX-FileCopyrightText: 2015-2018 Stefan Gerlach (stefan.gerlach@uni.kn)

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef NETCDFFILTERPRIVATE_H
#define NETCDFFILTERPRIVATE_H

#ifdef HAVE_NETCDF
#include <netcdf.h>
#endif

class AbstractDataSource;

class NetCDFFilterPrivate {

public:
	explicit NetCDFFilterPrivate(NetCDFFilter*);

	void parse(const QString & fileName, QTreeWidgetItem* rootItem);
	QVector<QStringList> readDataFromFile(const QString& fileName, AbstractDataSource* = nullptr,
			AbstractFileFilter::ImportMode = AbstractFileFilter::ImportMode::Replace);
	QString readAttribute(const QString& fileName, const QString& name, const QString& varName);
	QVector<QStringList> readCurrentVar(const QString& fileName, AbstractDataSource* = nullptr,
			AbstractFileFilter::ImportMode = AbstractFileFilter::ImportMode::Replace, int lines = -1);
	void write(const QString& fileName, AbstractDataSource*);
#ifdef HAVE_NETCDF
	static void handleError(int status, const QString& function);
	static QString translateFormat(int format);
	static QString translateDataType(nc_type type);
#endif

	const NetCDFFilter* q;

	QString currentVarName;
	int startRow{1};
	int endRow{-1};
	int startColumn{1};
	int endColumn{-1};

private:
#ifdef HAVE_NETCDF
	int m_status;

	QString scanAttrs(int ncid, int varid, int attid, QTreeWidgetItem* parentItem = nullptr);
	void scanDims(int ncid, int ndims, QTreeWidgetItem* parentItem);
	void scanVars(int ncid, int nvars, QTreeWidgetItem* parentItem);
#endif
};

#endif
