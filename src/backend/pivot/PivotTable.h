/***************************************************************************
    File                 : PivotTable.h
    Project              : LabPlot
    Description          : Aspect providing a pivot table functionality
    --------------------------------------------------------------------
    Copyright            : (C) 2019 Alexander Semke(alexander.semke@web.de)

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
#ifndef PIVOTTABLE_H
#define PIVOTTABLE_H

#include "backend/core/AbstractPart.h"
#include "backend/lib/macros.h"

class QAbstractItemModel;
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
	POINTER_D_ACCESSOR_DECL(Spreadsheet, dataSourceSpreadsheet, DataSourceSpreadsheet)
	CLASS_D_ACCESSOR_DECL(QString, dataSourceConnection, DataSourceConnection)
	CLASS_D_ACCESSOR_DECL(QString, dataSourceTable, DataSourceTable)

// 	POINTER_D_ACCESSOR_DECL(const QAbstractItemModel, dataModel, dataModel)
// 	POINTER_D_ACCESSOR_DECL(const QAbstractItemModel, horizontalHeaderModel, horizontalHeaderModel)
// 	POINTER_D_ACCESSOR_DECL(const QAbstractItemModel, verticalHeaderModel, verticalHeaderModel)
	QAbstractItemModel* dataModel() const;
	QAbstractItemModel* horizontalHeaderModel() const;
	QAbstractItemModel* verticalHeaderModel() const;

	const QStringList& dimensions() const;
	const QStringList& measures() const;

	const QStringList& rows() const;
	void addToRows(const QString&);

	const QStringList& columns() const;
	void addToColumns(const QString&);

	QIcon icon() const override;
	QMenu* createContextMenu() override;
	QWidget* view() const override;

	bool exportView() const override;
	bool printView() override;
	bool printPreview() const override;

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

	typedef PivotTablePrivate Private;

public slots:

private:
	void init();

	PivotTablePrivate* const d;
	mutable PivotTableView* m_view{nullptr};
	friend class PivotTablePrivate;

signals:
	void requestProjectContextMenu(QMenu*);

	void dataSourceTypeChanged(PivotTable::DataSourceType);
	void dataSourceSpreadsheetChanged(Spreadsheet*);
};

#endif
