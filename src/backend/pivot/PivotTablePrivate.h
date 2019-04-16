/***************************************************************************
    File                 : PivotTablePrivate.h
    Project              : LabPlot
    Description          : Private members of Pivot Table
    --------------------------------------------------------------------
    Copyright            : (C) 2019 by Alexander Semke (alexander.semke@web.de)

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

#ifndef PIVOTTABLEPRIVATE_H
#define PIVOTTABLEPRIVATE_H

#include <backend/pivot/PivotTable.h>
class QStandardItemModel;

class PivotTablePrivate {
public:
	explicit PivotTablePrivate(PivotTable*);
	virtual ~PivotTablePrivate();

	QString name() const;

	void addToRows(const QString&);
	void addToColumns(const QString&);

	void recalculate();
	void createDb();

	PivotTable* const q;

	PivotTable::DataSourceType dataSourceType{PivotTable::DataSourceSpreadsheet};
	Spreadsheet* dataSourceSpreadsheet{nullptr};
	QString dataSourceConnection;
	QString dataSourceTable;

	QStandardItemModel* dataModel{nullptr};
	QStandardItemModel* horizontalHeaderModel{nullptr};
	QStandardItemModel* verticalHeaderModel{nullptr};

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
};

#endif
