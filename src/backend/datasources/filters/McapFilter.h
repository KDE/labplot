/*
	File                 : MCAPFilter.h
	Project              : LabPlot
	Description          : MCAP I/O-filter.
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Raphael Wirth <wirthra@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MCAPFILTER_H
#define MCAPFILTER_H

#include "backend/core/AbstractColumn.h"
#include "backend/datasources/filters/AbstractFileFilter.h"

#include <QJsonValue>

class QJsonDocument;
class QJsonModel;
class McapFilterPrivate;
namespace mcap {
struct McapWriterOptions;
}

#ifdef SDK
#include "labplot_export.h"
class LABPLOT_EXPORT McapFilter : public AbstractFileFilter {
#else
class McapFilter : public AbstractFileFilter {
#endif
	Q_OBJECT

public:
	enum class DataContainerType { Array, Object };

	McapFilter();
	~McapFilter() override;

	static QStringList dataTypes();
	static QStringList dataRowTypes();
	static QString fileInfoString(const QString&);

	void
	readDataFromFile(const QString& fileName, AbstractDataSource* = nullptr, AbstractFileFilter::ImportMode = AbstractFileFilter::ImportMode::Replace) override;
	void write(const QString& fileName, AbstractDataSource*) override;
	void writeWithOptions(const QString& fileName, AbstractDataSource* datasource, int compressionMode, int compressionLevel);
	QVector<QStringList> preview(const QString& fileName, int lines);

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

	QJsonDocument getJsonDocument(const QString&);
	QVector<QString> getValidTopics(const QString& fileName);
	void setCurrentTopic(QString currentTopic);
	QString getCurrentTopic();

private:
	std::unique_ptr<McapFilterPrivate> const d;
	friend class McapFilterPrivate;
};

#endif
