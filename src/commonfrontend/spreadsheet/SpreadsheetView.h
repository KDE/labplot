/***************************************************************************
    File                 : SpreadsheetView.h
    Project              : SciDAVis
    Description          : View class for Spreadsheet
    --------------------------------------------------------------------
    Copyright            : (C) 2007 Tilman Benkert (thzs*gmx.net)
    Copyright            : (C) 2011 by Alexander Semke (alexander.semke*web.de)
                           (replace * with @ in the email addresses) 

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

#ifndef TABLEVIEW_H
#define TABLEVIEW_H

#include <QTableView>

#include "core/globals.h"
#include "lib/IntervalAttribute.h"

#ifndef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
class KAction;
#else
class QAction;
#endif

class Column;
class Spreadsheet;
class SpreadsheetModel;
class SpreadsheetItemDelegate;
class SpreadsheetDoubleHeaderView;
// class ActionManager;
class AbstractAspect;

class QMenu;

class SpreadsheetView : public QTableView{
    Q_OBJECT

	public:
		SpreadsheetView(Spreadsheet *spreadsheet);
		virtual ~SpreadsheetView();
				
		void showComments(bool on = true);
		bool areCommentsShown() const;

		int selectedColumnCount(bool full = false);
		int selectedColumnCount(SciDAVis::PlotDesignation pd);
		bool isColumnSelected(int col, bool full = false);
		QList<Column *> selectedColumns(bool full = false);
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
		void getCurrentCell(int * row, int * col);

		static int defaultColumnWidth();
		static void setDefaultColumnWidth(int width);

		static void setDefaultCommentVisibility(bool visible);
		static bool defaultCommentVisibility();
		
// 		static ActionManager * actionManager();
// 		static void initActionManager();

// 		void setPlotMenu(QMenu * menu);
		
	private:
	  	 void init();
		Spreadsheet * m_spreadsheet;
		SpreadsheetItemDelegate * m_delegate;
		SpreadsheetModel * m_model;
		SpreadsheetDoubleHeaderView * m_horizontalHeader;
		
		bool eventFilter( QObject * watched, QEvent * event);
		void keyPressEvent(QKeyEvent * event);
		
// 		static ActionManager * action_manager;
		SpreadsheetView();
		
		void initActions();
		void initMenus();
		void connectActions();

#ifdef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
		//! \name selection related actions
		//@{
		QAction * action_cut_selection;
		QAction * action_copy_selection;
		QAction * action_paste_into_selection;
		QAction * action_mask_selection;
		QAction * action_unmask_selection;
		QAction * action_set_formula;
		QAction * action_clear_selection;
		QAction * action_recalculate;
		QAction * action_fill_row_numbers;
		QAction * action_fill_random;
		//@}
		//! \name spreadsheet related actions
		//@{
		QAction * action_toggle_comments;
		QAction * action_toggle_tabbar;
		QAction * action_select_all;
		QAction * action_add_column;
		QAction * action_clear_spreadsheet;
		QAction * action_clear_masks;
		QAction * action_sort_spreadsheet;
		QAction * action_go_to_cell;
		QAction * action_dimensions_dialog;
		QAction * action_formula_mode;
		//@}
		//! \name column related actions
		//@{
		QAction * action_insert_columns;
		QAction * action_remove_columns;
		QAction * action_clear_columns;
		QAction * action_add_columns;
		QAction * action_set_as_x;
		QAction * action_set_as_y;
		QAction * action_set_as_z;
		QAction * action_set_as_xerr;
		QAction * action_set_as_yerr;
		QAction * action_set_as_none;
		QAction * action_normalize_columns;
		QAction * action_normalize_selection;
		QAction * action_sort_columns;
		QAction * action_statistics_columns;
		QAction * action_type_format;
		QAction * action_edit_description;
		//@}
		//! \name row related actions
		//@{
		QAction * action_insert_rows;
		QAction * action_remove_rows;
		QAction * action_clear_rows;
		QAction * action_add_rows;
		QAction * action_statistics_rows;
		//@}
#else
		//selection related actions
		KAction * action_cut_selection;
		KAction * action_copy_selection;
		KAction * action_paste_into_selection;
		KAction * action_mask_selection;
		KAction * action_unmask_selection;
		KAction * action_set_formula;
		KAction * action_clear_selection;
		KAction * action_recalculate;
		KAction * action_fill_row_numbers;
		KAction * action_fill_random;

		//spreadsheet related actions
		KAction * action_toggle_comments;
		KAction * action_select_all;
		KAction * action_add_column;
		KAction * action_clear_spreadsheet;
		KAction * action_clear_masks;
		KAction * action_sort_spreadsheet;
		KAction * action_go_to_cell;
		KAction * action_dimensions_dialog;

		//column related actions
		KAction * action_insert_columns;
		KAction * action_remove_columns;
		KAction * action_clear_columns;
		KAction * action_add_columns;
		KAction * action_set_as_x;
		KAction * action_set_as_y;
		KAction * action_set_as_z;
		KAction * action_set_as_xerr;
		KAction * action_set_as_yerr;
		KAction * action_set_as_none;
		KAction * action_normalize_columns;
		KAction * action_normalize_selection;
		KAction * action_sort_columns;
		KAction * action_statistics_columns;

		//row related actions
		KAction * action_insert_rows;
		KAction * action_remove_rows;
		KAction * action_clear_rows;
		KAction * action_add_rows;
		KAction * action_statistics_rows;
#endif

// 		QMenu * m_plotMenu;
		QMenu* m_selectionMenu;
		QMenu* m_columnMenu;
		QMenu* m_rowMenu;
		QMenu* m_spreadsheetMenu;
		
	public slots:
		void activateFormulaMode(bool on);
		void goToCell(int row, int col);
		void deselectAll();
		void toggleComments();
		void handleHorizontalSectionResized(int logicalIndex, int oldSize, int newSize); 
		void goToNextColumn();
		void goToPreviousColumn();
		void goToCell();
		void sortSpreadsheet();
		
		void setSelectionAs(SciDAVis::PlotDesignation pd);
		void cutSelection();
		void copySelection();
		void pasteIntoSelection();
		void clearSelectedCells();
		void maskSelection();
		void unmaskSelection();
		void recalculateSelectedCells();
		void fillSelectedCellsWithRowNumbers();
		void fillSelectedCellsWithRandomNumbers();
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
		void statisticsOnSelectedColumns();
		void statisticsOnSelectedRows();
		void insertEmptyRows();
		void removeSelectedRows();
		
		void addColumns();
		void addRows();
		
		void setColumnWidth(int col, int width);
		int columnWidth(int col) const;
		bool formulaModeActive() const;

		void createContextMenu(QMenu * menu);
		void fillProjectMenu(QMenu * menu, bool * rc);
		void sortDialog(QList<Column*> cols);

		void print(QPrinter*) const;

	private  slots:
		void advanceCell();
		void handleHorizontalSectionMoved(int index, int from, int to);
		void handleHorizontalHeaderDoubleClicked(int index);
		void handleHeaderDataChanged(Qt::Orientation orientation, int first, int last);
		void currentColumnChanged(const QModelIndex & current, const QModelIndex & previous);
		void handleAspectAdded(const AbstractAspect * aspect);
		void handleAspectAboutToBeRemoved(const AbstractAspect * aspect);
		void updateSectionSize(const Column* col);
		void updateHeaderGeometry(Qt::Orientation o, int first, int last);
		
		void selectColumn(int);
		void deselectColumn(int);
		void columnClicked(int);
		void selectionChanged(const QItemSelection & selected, const QItemSelection & deselected);
};

#endif
