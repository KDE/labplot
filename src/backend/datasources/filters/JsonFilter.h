/*
    File                 : JsonFilter.h
    Project              : LabPlot
    Description          : JSON I/O-filter.
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2018 Andrey Cygankov <craftplace.ms@gmail.com>
    SPDX-FileCopyrightText: 2018-2020 Alexander Semke <alexander.semke@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


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
	enum class DataContainerType {Array, Object};

	JsonFilter();
	~JsonFilter() override;

	static QStringList dataTypes();
	static QStringList dataRowTypes();
	static QString fileInfoString(const QString&);

	// read data from any device
	void readDataFromDevice(QIODevice& device, AbstractDataSource*, AbstractFileFilter::ImportMode = AbstractFileFilter::ImportMode::Replace, int lines = -1);
	// overloaded function to read from file
	void readDataFromFile(const QString& fileName, AbstractDataSource* = nullptr, AbstractFileFilter::ImportMode = AbstractFileFilter::ImportMode::Replace) override;
	void write(const QString& fileName, AbstractDataSource*) override;

	QVector<QStringList> preview(const QString& fileName, int lines);
	QVector<QStringList> preview(QIODevice& device, int lines);

	void loadFilterSettings(const QString&) override;
	void saveFilterSettings(const QString&) const override;

	void setDataRowType(const QJsonValue::Type);
	QJsonValue::Type dataRowType() const;

	void setModel(QJsonModel*);
	void setModelRows(const QVector<int>&);
	QVector<int> modelRows() const;

	void setDateTimeFormat(const QString&);
	QString dateTimeFormat() const;
	void setNumberFormat(QLocale::Language);
	QLocale::Language numberFormat() const;
	void setNaNValueToZero(const bool);
	bool NaNValueToZeroEnabled() const;
	void setCreateIndexEnabled(const bool);
	void setImportObjectNames(const bool);

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
