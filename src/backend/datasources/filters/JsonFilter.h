/***************************************************************************
    File                 : JsonFilter.h
    Project              : LabPlot
    Description          : JSON I/O-filter.
    --------------------------------------------------------------------
    --------------------------------------------------------------------
    Copyright            : (C) 2018 Andrey Cygankov (craftplace.ms@gmail.com)

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

#ifndef JSONFILTER_H
#define JSONFILTER_H

#include "backend/datasources/filters/AbstractFileFilter.h"
#include "backend/core/AbstractColumn.h"

#include <QJsonValue>

class QStringList;
class QIODevice;
class QJsonDocument;
class QJsonModel;
class JsonFilterPrivate;

class JsonFilter : public AbstractFileFilter {
	Q_OBJECT

public:
	enum DataContainerType {Array, Object};

	JsonFilter();
	~JsonFilter() override;

	static QStringList dataTypes();
	static QStringList dataRowTypes();

	// read data from any device
	void readDataFromDevice(QIODevice& device, AbstractDataSource*,
	                        AbstractFileFilter::ImportMode = AbstractFileFilter::Replace, int lines = -1);
	// overloaded function to read from file
	QVector<QStringList> readDataFromFile(const QString& fileName, AbstractDataSource* = nullptr,
	                                      AbstractFileFilter::ImportMode = AbstractFileFilter::Replace, int lines = -1) override;
	void write(const QString& fileName, AbstractDataSource*) override;

	QVector<QStringList> preview(const QString& fileName);
	QVector<QStringList> preview(QIODevice& device);
	QVector<QStringList> preview(QJsonDocument& doc);

	void loadFilterSettings(const QString&) override;
	void saveFilterSettings(const QString&) const override;

	void setDataRowType(const QJsonValue::Type);
	QJsonValue::Type dataRowType() const;
	void setModelRows(const QVector<int>);
	QVector<int> modelRows() const;

	void setDateTimeFormat(const QString&);
	QString dateTimeFormat() const;
	void setNumberFormat(QLocale::Language);
	QLocale::Language numberFormat() const;
	void setNaNValueToZero(const bool);
	bool NaNValueToZeroEnabled() const;
    void setCreateIndexEnabled(const bool);
	void setParseRowsName(const bool);

	void setVectorNames(const QString&);
	QStringList vectorNames() const;
	QVector<AbstractColumn::ColumnMode> columnModes();

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
	std::unique_ptr<JsonFilterPrivate> const d;
	friend class JsonFilterPrivate;
};

#endif
