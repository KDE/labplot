/*
	File                 : McapFilterPrivate.h
	Project              : LabPlot
	Description          : Private implementation class for McapFilter.
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2018 Andrey Cygankov <craftplace.ms@gmail.com>
	SPDX-FileCopyrightText: 2018-2020 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MCAPFILTERPRIVATE_H
#define MCAPFILTERPRIVATE_H

#include "QJsonModel.h"
#include <limits.h>

class QJsonDocument;
class AbstractDataSource;
class AbstractColumn;

class McapFilterPrivate {
public:
	explicit McapFilterPrivate(McapFilter* owner);

	int checkRow(QJsonValueRef value, int& countCols);
	int parseColumnModes(const QJsonValue& row, const QString& rowName = QString());
	void setEmptyValue(int column, int row);
	void setValueFromString(int column, int row, const QString& value);

	int prepareDeviceToRead(QIODevice&);
	void
	readDataFromDevice(QIODevice&, AbstractDataSource* = nullptr, AbstractFileFilter::ImportMode = AbstractFileFilter::ImportMode::Replace, int lines = -1);
	void readDataFromFile(const QString& fileName, AbstractDataSource* = nullptr, AbstractFileFilter::ImportMode = AbstractFileFilter::ImportMode::Replace);
	void importData(AbstractDataSource* = nullptr, AbstractFileFilter::ImportMode = AbstractFileFilter::ImportMode::Replace, int lines = -1);
	int mcapToJson(const QString& fileName);
	QJsonDocument getJsonDocument(const QString&);

	void write(const QString& fileName, AbstractDataSource*);
	QVector<QStringList> preview(const QString& fileName, int lines);
	QVector<QStringList> preview(QIODevice& device, int lines);
	QVector<QStringList> preview(int lines);

	const McapFilter* q;
	QJsonModel* model{nullptr};

	McapFilter::DataContainerType containerType{McapFilter::DataContainerType::Object};
	QJsonValue::Type rowType{QJsonValue::Object};
	QVector<int> modelRows;

	QString dateTimeFormat;
	QLocale::Language numberFormat{QLocale::C};
	double nanValue{qQNaN()};
	bool createIndexEnabled{false};
	bool importObjectNames{false};
	QStringList vectorNames;
	QVector<AbstractColumn::ColumnMode> columnModes;

	int startRow{1}; // start row
	int endRow{-1}; // end row
	int startColumn{1}; // start column
	int endColumn{-1}; // end column

private:
	int m_actualRows{0};
	int m_actualCols{0};
	int m_prepared{false};
	int m_columnOffset{0}; // indexes the "start column" in the datasource. Data will be imported starting from this column.
	std::vector<void*> m_dataContainer; // pointers to the actual data containers (columns).
	QJsonDocument m_doc; // original and full JSON document
	QJsonDocument m_preparedDoc; // selected part of the full JSON document, the part that needs to be imported

	bool prepareDocumentToRead();

};

#endif
