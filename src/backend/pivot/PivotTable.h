/*
	File                 : PivotTable.ch
	Project              : LabPlot
	Description          : Aspect providing pivot table functionality
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2019-2025 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PIVOTTABLE_H
#define PIVOTTABLE_H

#include "backend/core/AbstractPart.h"
#include "backend/lib/macros.h"

class QAbstractItemModel;
class HierarchicalHeaderModel;
class PivotTableView;
class PivotTablePrivate;
class Spreadsheet;

class PivotTable : public AbstractPart {
	Q_OBJECT

public:
	explicit PivotTable(const QString& name, bool loading = false);

	enum DataSourceType {DataSourceSpreadsheet, DataSourceDatabase};
	enum AggregationType {AggregationCount, AggregationSum, AggregationMin, AggregationMax};
	enum SortType {NoSort, SortAscending, SortDescending};

	BASIC_D_ACCESSOR_DECL(DataSourceType, dataSourceType, DataSourceType)
	POINTER_D_ACCESSOR_DECL(const Spreadsheet, dataSourceSpreadsheet, DataSourceSpreadsheet)
	CLASS_D_ACCESSOR_DECL(QString, dataSourceConnection, DataSourceConnection)
	CLASS_D_ACCESSOR_DECL(QString, dataSourceTable, DataSourceTable)

	QAbstractItemModel* dataModel() const;
	void setDataModel(QAbstractItemModel*);
	void setHorizontalHeaderModel(QAbstractItemModel*);
	void setVerticalHeaderModel(QAbstractItemModel*);

	const QStringList& dimensions() const;
	const QStringList& measures() const;

	const QStringList& rows() const;
	void addToRows(const QString&);
	void removeFromRows(const QString&);

	const QStringList& columns() const;
	void addToColumns(const QString&);
	void removeFromColumns(const QString&);

	QIcon icon() const override;
	QMenu* createContextMenu() override;
	QWidget* view() const override;

	bool exportView() const override;
	bool printView() override;
	bool printPreview() const override;

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

	typedef PivotTablePrivate Private;

private:
	Q_DECLARE_PRIVATE(PivotTable)
	PivotTablePrivate* const d_ptr;
	void init();

	mutable PivotTableView* m_view{nullptr};
	friend class PivotTablePrivate;

Q_SIGNALS:
	void changed();
	void requestProjectContextMenu(QMenu*);
	void dataSourceTypeChanged(PivotTable::DataSourceType);
	void dataSourceSpreadsheetChanged(Spreadsheet*);
};

#endif
