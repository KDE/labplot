/***************************************************************************
    File                 : JsonFilterPrivate.h
    Project              : LabPlot
    Description          : Private implementation class for JsonFilter.
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

#ifndef JSONFILTERPRIVATE_H
#define JSONFILTERPRIVATE_H

#include <QJsonDocument>
#include "QJsonModel.h"
class KFilterDev;
class AbstractDataSource;
class AbstractColumn;

class JsonFilterPrivate {

public:
	explicit JsonFilterPrivate (JsonFilter* owner);

	int checkRow(QJsonValueRef value, int &countCols);
	int parseColumnModes(QJsonValue row, QString rowName = "");
	void setEmptyValue(int column, int row);
	void setValueFromString(int column, int row, QString value);

	int prepareDeviceToRead(QIODevice&);
	int prepareDocumentToRead(const QJsonDocument&);

	void readDataFromDevice(QIODevice& device, AbstractDataSource* = nullptr,
			AbstractFileFilter::ImportMode = AbstractFileFilter::Replace, int lines = -1);
	void readDataFromFile(const QString& fileName, AbstractDataSource* = nullptr,
			AbstractFileFilter::ImportMode = AbstractFileFilter::Replace);
	void readDataFromDocument(const QJsonDocument& doc, AbstractDataSource* = nullptr,
	                          AbstractFileFilter::ImportMode = AbstractFileFilter::Replace, int lines = -1);

	void importData(AbstractDataSource* = nullptr, AbstractFileFilter::ImportMode = AbstractFileFilter::Replace,
	                int lines = -1);

	void write(const QString& fileName, AbstractDataSource*);
	QVector<QStringList> preview(const QString& fileName);
	QVector<QStringList> preview(QIODevice& device);
	QVector<QStringList> preview(QJsonDocument& doc);
	QVector<QStringList> preview();

	const JsonFilter* q;
	QJsonModel* model;

	JsonFilter::DataContainerType containerType{JsonFilter::Object};
	QJsonValue::Type rowType{QJsonValue::Object};
	QVector<int> modelRows;

	QString dateTimeFormat;
	QLocale::Language numberFormat{QLocale::C};
	double nanValue{NAN};
	bool createIndexEnabled{false};
	bool importObjectNames{false};
	QStringList vectorNames;
	QVector<AbstractColumn::ColumnMode> columnModes;

	int startRow{1};	// start row
	int endRow{-1};		// end row
	int startColumn{1};	// start column
	int endColumn{-1};	// end column

private:
	int m_actualRows{0};
	int m_actualCols{0};
	int m_prepared{false};
	int m_columnOffset{0}; // indexes the "start column" in the datasource. Data will be imported starting from this column.
	QVector<void*> m_dataContainer; // pointers to the actual data containers (columns).
	QJsonDocument m_preparedDoc; // parsed Json document
};

#endif
