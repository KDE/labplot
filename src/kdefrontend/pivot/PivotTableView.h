/***************************************************************************
    File                 : PivotTableView.h
    Project              : LabPlot
    Description          : View class for PivotTable
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

#ifndef PIVOTTABLEVIEW_H
#define PIVOTTABLEVIEW_H

#include <QWidget>

#include "backend/core/AbstractColumn.h"
#include "backend/lib/IntervalAttribute.h"

class Column;
class PivotTable;
class PivotTableModel;
class PivotTableItemDelegate;
class HierarchicalHeaderView;
class AbstractAspect;
class QTableView;

class QPrinter;
class QMenu;
class QToolBar;
class QModelIndex;
class QItemSelection;

class PivotTableView : public QWidget {
	Q_OBJECT

public:
	explicit PivotTableView(PivotTable*, bool readOnly = false);
	~PivotTableView() override;

	bool exportView();
	bool printView();
	bool printPreview();

private:
	void init();
	void initActions();
	void initMenus();
	void connectActions();

	void exportToFile(const QString&, const bool, const QString&, QLocale::Language) const;
	void exportToLaTeX(const QString&, const bool exportHeaders,
	                   const bool gridLines, const bool captions, const bool latexHeaders,
	                   const bool skipEmptyRows,const bool exportEntire) const;

	PivotTable* m_pivotTable;
	QTableView* m_tableView;
	HierarchicalHeaderView* m_horizontalHeader{nullptr};
	HierarchicalHeaderView* m_verticalHeader{nullptr};
	bool m_readOnly{false};

public slots:
	void createContextMenu(QMenu*);
	void fillToolBar(QToolBar*);
	void print(QPrinter*) const;

private slots:
	void goToCell();
	void goToCell(int row, int col);
};

#endif
