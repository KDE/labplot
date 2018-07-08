/***************************************************************************
File                 : NetCDFFilter.h
Project              : LabPlot
Description          : NetCDF I/O-filter
--------------------------------------------------------------------
Copyright            : (C) 2015 Stefan Gerlach (stefan.gerlach@uni.kn)
***************************************************************************/

/***************************************************************************
*                                                                         *
*  This program is free software; you can redistribute it and/or modify   *
*  it under the terms of the GNU General Public License as published by   *
*  the Free Software Foundation; either version 2 of the License, or      *
*  (at your option) any later version.                                    *
*                                                                         *
*  This program is distributed in the hope that it will be useful,        *
*  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
*  GNU General Public License for more details.                           *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the Free Software           *
*   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
*   Boston, MA  02110-1301  USA                                           *
*                                                                         *
***************************************************************************/
#ifndef NETCDFFILTER_H
#define NETCDFFILTER_H

#include "backend/datasources/filters/AbstractFileFilter.h"
#include <QStringList>
#include <QTreeWidgetItem>

class NetCDFFilterPrivate;
class NetCDFFilter : public AbstractFileFilter {
	Q_OBJECT

public:
	NetCDFFilter();
	~NetCDFFilter() override;

	void parse(const QString& fileName, QTreeWidgetItem* rootItem);
	QVector<QStringList> readDataFromFile(const QString& fileName, AbstractDataSource* = nullptr,
			AbstractFileFilter::ImportMode = AbstractFileFilter::Replace) override;
	QString readAttribute(const QString & fileName, const QString & name, const QString & varName);
	QVector<QStringList> readCurrentVar(const QString& fileName, AbstractDataSource* = nullptr,
			AbstractFileFilter::ImportMode = AbstractFileFilter::Replace, int lines = -1);
	void write(const QString& fileName, AbstractDataSource*) override;

	void loadFilterSettings(const QString&) override;
	void saveFilterSettings(const QString&) const override;

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
