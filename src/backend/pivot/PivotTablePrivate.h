/*
	File                 : PivotTablePrivate.h
	Project              : LabPlot
	Description          : Private members of Pivot Table
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2019-2025 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PIVOTTABLEPRIVATE_H
#define PIVOTTABLEPRIVATE_H

#include <backend/pivot/PivotTable.h>

class QStandardItemModel;
class QSqlQuery;
class HierarchicalHeaderModel;

class PivotTablePrivate {
public:
	explicit PivotTablePrivate(PivotTable*);
	virtual ~PivotTablePrivate();

	QString name() const;

	void addToRows(const QString&);
	void removeFromRows(const QString&);

	void addToColumns(const QString&);
	void removeFromColumns(const QString&);

	void recalculate();
	void createDb();

	PivotTable* const q;

	PivotTable::DataSourceType dataSourceType{PivotTable::DataSourceSpreadsheet};
	const Spreadsheet* dataSourceSpreadsheet{nullptr};
	QString dataSourceSpreadsheetPath; // path to the spreadsheet file, used for loading/saving
	QString dataSourceConnection;
	QString dataSourceTable;

	QStandardItemModel* dataModel{nullptr};
	HierarchicalHeaderModel* horizontalHeaderModel{nullptr};
	HierarchicalHeaderModel* verticalHeaderModel{nullptr};

	QStringList dimensions;
	QStringList measures;
	QStringList rows;
	QStringList columns;
	bool showNulls{false};
	bool showTotals{true};
	PivotTable::SortType sortType{PivotTable::NoSort};
	PivotTable::AggregationType aggregationType{PivotTable::AggregationCount};
	QString sortDimension;

private:
	bool m_dbCreated{false};
// 	QMap<QString, QStringList> m_members;

	QString createSQLQuery() const;
	void populateDataModels(QSqlQuery);
};

#endif
