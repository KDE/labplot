/***************************************************************************
    File                 : SpreadsheetView.h
    Project              : LabPlot
    Description          : View class for Spreadsheet
    --------------------------------------------------------------------
    Copyright            : (C) 2010-2015 by Alexander Semke (alexander.semke@web.de)

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

#ifndef SPREADSHEETVIEW_H
#define SPREADSHEETVIEW_H

#include <QWidget>
#include <QPrinter>

#include "backend/core/AbstractColumn.h"
#include "backend/lib/IntervalAttribute.h"

class Column;
class Spreadsheet;
class SpreadsheetModel;
class SpreadsheetItemDelegate;
class SpreadsheetDoubleHeaderView;
class AbstractAspect;
class QTableView;

class QMenu;
class QToolBar;
class QModelIndex;
class QItemSelection;

class SpreadsheetView : public QWidget {
    Q_OBJECT

	public:
		explicit SpreadsheetView(Spreadsheet* spreadsheet);
		virtual ~SpreadsheetView();

		void showComments(bool on = true);
		bool areCommentsShown() const;

		int selectedColumnCount(bool full = false);
		int selectedColumnCount(AbstractColumn::PlotDesignation);
		bool isColumnSelected(int col, bool full = false);
		QList<Column*> selectedColumns(bool full = false);
		int selectedRowCount(bool full = false);
		bool isRowSelected(int row, bool full = false);
		int firstSelectedColumn(bool full = false);
		int lastSelectedColumn(bool full = false);
		int firstSelectedRow(bool full = false);
		int lastSelectedRow(bool full = false);
		IntervalAttribute<bool> selectedRows(bool full = false);
		bool isCellSelected(int row, int col);
		void setCellSelected(int row, int col, bool select = true);
		void setCellsSelected(int first_row, int first_col, int last_row, int last_col, bool select = true);
		void getCurrentCell(int* row, int* col);
		void exportToFile(const QString&, const bool, const QString&) const;

	private:
	  	void init();
		QTableView* m_tableView;
		Spreadsheet* m_spreadsheet;
		SpreadsheetItemDelegate* m_delegate;
		SpreadsheetModel* m_model;
		SpreadsheetDoubleHeaderView* m_horizontalHeader;
		bool m_suppressSelectionChangedEvent;

		bool eventFilter(QObject*, QEvent*);
		void keyPressEvent(QKeyEvent*);

		void initActions();
		void initMenus();
		void connectActions();

		//selection related actions
		QAction* action_cut_selection;
		QAction* action_copy_selection;
		QAction* action_paste_into_selection;
		QAction* action_mask_selection;
		QAction* action_unmask_selection;
		QAction* action_set_formula;
		QAction* action_clear_selection;
		QAction* action_recalculate;
		QAction* action_fill_row_numbers;
		QAction* action_fill_random;
		QAction* action_fill_equidistant;
		QAction* action_fill_random_nonuniform;
		QAction* action_fill_const;
		QAction* action_fill_function;

		//spreadsheet related actions
		QAction* action_toggle_comments;
		QAction* action_select_all;
		QAction* action_add_column;
		QAction* action_clear_spreadsheet;
		QAction* action_clear_masks;
		QAction* action_sort_spreadsheet;
		QAction* action_go_to_cell;

		//column related actions
		QAction* action_insert_columns;
		QAction* action_remove_columns;
		QAction* action_clear_columns;
		QAction* action_add_columns;
		QAction* action_set_as_x;
		QAction* action_set_as_y;
		QAction* action_set_as_z;
		QAction* action_set_as_xerr;
		QAction* action_set_as_yerr;
		QAction* action_set_as_none;
		QAction* action_normalize_columns;
		QAction* action_normalize_selection;
		QAction* action_sort_columns;
		QAction* action_sort_asc_column;
		QAction* action_sort_desc_column;
		QAction* action_statistics_columns;

		//row related actions
		QAction* action_insert_rows;
		QAction* action_remove_rows;
		QAction* action_clear_rows;
		QAction* action_add_rows;
		QAction* action_statistics_rows;

		//Menus
		QMenu* m_selectionMenu;
		QMenu* m_columnMenu;
		QMenu* m_rowMenu;
		QMenu* m_spreadsheetMenu;

	public slots:
		void activateFormulaMode(bool on);
		void goToCell(int row, int col);
		void toggleComments();
		void handleHorizontalSectionResized(int logicalIndex, int oldSize, int newSize);
		void goToNextColumn();
		void goToPreviousColumn();
		void goToCell();
		void sortSpreadsheet();

		void setSelectionAs(AbstractColumn::PlotDesignation);
		void cutSelection();
		void copySelection();
		void pasteIntoSelection();
		void clearSelectedCells();
		void maskSelection();
		void unmaskSelection();
		void recalculateSelectedCells();
		void fillSelectedCellsWithRowNumbers();
		void fillSelectedCellsWithRandomNumbers();
		void fillWithRandomValues();
		void fillWithEquidistantValues();
		void fillWithFunctionValues();
		void fillSelectedCellsWithConstValues();
		void insertEmptyColumns();
		void removeSelectedColumns();
		void clearSelectedColumns();
		void clearSelectedRows();

		void setSelectedColumnsAsX();
		void setSelectedColumnsAsY();
		void setSelectedColumnsAsZ();
		void setSelectedColumnsAsXError();
		void setSelectedColumnsAsYError();
		void setSelectedColumnsAsNone();
		void normalizeSelectedColumns();
		void normalizeSelection();
		void sortSelectedColumns();
		void sortColumnAscending();
		void sortColumnDescending();
		void statisticsOnSelectedColumns();
		void statisticsOnSelectedRows();
		void insertEmptyRows();
		void removeSelectedRows();

		void addColumns();
		void addRows();

		bool formulaModeActive() const;

		void createContextMenu(QMenu*) const;
		void fillToolBar(QToolBar*);
		void sortDialog(QList<Column*>);

		void print(QPrinter*) const;

	private  slots:
		void advanceCell();
		void handleHorizontalSectionMoved(int index, int from, int to);
		void handleHorizontalHeaderDoubleClicked(int index);
		void handleHeaderDataChanged(Qt::Orientation orientation, int first, int last);
		void currentColumnChanged(const QModelIndex& current, const QModelIndex & previous);
		void handleAspectAdded(const AbstractAspect* aspect);
		void handleAspectAboutToBeRemoved(const AbstractAspect* aspect);
		void updateSectionSize(const Column*);
		void updateHeaderGeometry(Qt::Orientation o, int first, int last);

		void selectColumn(int);
		void deselectColumn(int);
		void columnClicked(int);
		void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
};

#endif
