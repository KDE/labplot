/*
	File                 : OdsFilter.h
	Project              : LabPlot
	Description          : Ods I/O-filter
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ODSFILTER_H
#define ODSFILTER_H
#include "backend/datasources/filters/AbstractFileFilter.h"

#include <QObject>

class OdsFilterPrivate;
class QTreeWidgetItem;

#ifdef SDK
#include "labplot_export.h"
class LABPLOT_EXPORT OdsFilter : public AbstractFileFilter {
#else
class OdsFilter : public AbstractFileFilter {
#endif
	Q_OBJECT
public:
	explicit OdsFilter();
	virtual ~OdsFilter() override;
	static QString fileInfoString(const QString& fileName);
	QVector<QStringList> preview(const QString& sheetName, int lines);
	void setFirstRowAsColumnNames(const bool);
	bool parse(const QString& fileName, QTreeWidgetItem* root);
	void setCurrentSheetName(const QString&);
	virtual void readDataFromFile(const QString& fileName, AbstractDataSource* = nullptr, ImportMode = ImportMode::Replace) override;
	virtual void write(const QString& fileName, AbstractDataSource*) override;

	void setSelectedSheetNames(const QStringList&);
	const QStringList selectedSheetNames() const;

	virtual void save(QXmlStreamWriter*) const override;
	virtual bool load(XmlStreamReader*) override;

	void setStartRow(int);
	int startRow() const;
	void setEndRow(int);
	int endRow() const;
	void setStartColumn(int);
	int startColumn() const;
	void setEndColumn(int);
	int endColumn() const;
	int firstColumn() const;

private:
	std::unique_ptr<OdsFilterPrivate> const d;

	friend class OdsFilterPrivate;
};

#endif
