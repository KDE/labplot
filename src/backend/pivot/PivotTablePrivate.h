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

	QString name() const;

	void addToRows(const QString&);
	void removeFromRows(const QString&);

	void addToColumns(const QString&);
	void removeFromColumns(const QString&);

	void addToValues(const QString&);
	void removeFromValues(const QString&);

	void recalculate();
	void createDb();

	PivotTable* const q;

	PivotTable::DataSourceType dataSourceType{PivotTable::DataSourceType::Spreadsheet};
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
	QVector<PivotTable::Value> values{{QString(), PivotTable::Aggregation::Count}};
	bool showNulls{false};
	bool showTotals{true};
	PivotTable::Sort sortType{PivotTable::Sort::NoSort};
	PivotTable::Aggregation aggregationType{PivotTable::Aggregation::Count};
	QString sortDimension;

private:
	QString m_dbTableName;
	// QMap<QString, QStringList> m_members;

	QString createSQLQuery() const;
	void populateDataModels(QSqlQuery);
	QString headerText(PivotTable::Value value) const;
};

#endif
