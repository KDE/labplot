/***************************************************************************
    File                 : TableView.h
    Project              : SciDAVis
    Description          : View class for Table
    --------------------------------------------------------------------
    Copyright            : (C) 2007 Tilman Benkert (thzs*gmx.net)
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

#include <QWidget>
#include <QTableView>
#include <QMessageBox>
#include <QHeaderView>
#include <QSize>
#include <QTabWidget>
#include <QPushButton>
#include <QToolButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QComboBox>
#include <QSpinBox> 
#include <QScrollArea>
#include "ui_controltabs.h"
#include <QtDebug>
#include "core/globals.h"

class Column;
class Table;
class TableModel;
class TableItemDelegate;
class TableDoubleHeaderView;
class ActionManager;

//! Helper class for TableView
class TableViewWidget : public QTableView
{
    Q_OBJECT

	public:
		//! Constructor
		TableViewWidget(QWidget * parent = 0) : QTableView(parent) {};

	protected:
		//! Overloaded function (cf. Qt documentation)
		virtual void keyPressEvent(QKeyEvent * event);

	signals:
		void advanceCell();

		protected slots:
			//! Cause a repaint of the header
			void updateHeaderGeometry(Qt::Orientation o, int first, int last);
		public slots:
			void selectAll();
};

//! View class for Table
class TableView : public QWidget
{
    Q_OBJECT

	public:
		//! Constructor
		TableView(Table *table);
		//! Destructor
		virtual ~TableView();
		bool isControlTabBarVisible() { return m_control_tabs->isVisible(); }
		//! Show or hide (if on = false) the column comments
		void showComments(bool on = true);
		//! Return whether comments are show currently
		bool areCommentsShown() const;

		//! \name selection related functions
		//@{
		//! Return how many columns are selected
		/**
		 * If full is true, this function only returns the number of fully 
		 * selected columns.
		 */
		int selectedColumnCount(bool full = false);
		//! Return how many columns with the given plot designation are (at least partly) selected
		int selectedColumnCount(SciDAVis::PlotDesignation pd);
		//! Returns true if column 'col' is selected; otherwise false
		/**
		 * If full is true, this function only returns true if the whole 
		 * column is selected.
		 */
		bool isColumnSelected(int col, bool full = false);
		//! Return all selected columns
		/**
		 * If full is true, this function only returns a column if the whole 
		 * column is selected.
		 */
		QList<Column *> selectedColumns(bool full = false);
		//! Return how many rows are (at least partly) selected
		/**
		 * If full is true, this function only returns the number of fully 
		 * selected rows.
		 */
		int selectedRowCount(bool full = false);
		//! Returns true if row 'row' is selected; otherwise false
		/**
		 * If full is true, this function only returns true if the whole 
		 * row is selected.
		 */
		bool isRowSelected(int row, bool full = false);
		//! Return the index of the first selected column
		/**
		 * If full is true, this function only looks for fully 
		 * selected columns.
		 */
		int firstSelectedColumn(bool full = false);
		//! Return the index of the last selected column
		/**
		 * If full is true, this function only looks for fully 
		 * selected columns.
		 */
		int lastSelectedColumn(bool full = false);
		//! Return the index of the first selected row
		/**
		 * If full is true, this function only looks for fully 
		 * selected rows.
		 */
		int firstSelectedRow(bool full = false);
		//! Return the index of the last selected row
		/**
		 * If full is true, this function only looks for fully 
		 * selected rows.
		 */
		int lastSelectedRow(bool full = false);
		//! Return whether a cell is selected
		bool isCellSelected(int row, int col);
		//! Select/Deselect a cell
		void setCellSelected(int row, int col, bool select = true);
		//! Select/Deselect a range of cells
		void setCellsSelected(int first_row, int first_col, int last_row, int last_col, bool select = true);
		//! Determine the current cell (-1 if no cell is designated as the current)
		void getCurrentCell(int * row, int * col);
		//@}

		//! Create a menu with selection related operations
		/**
		 * \param append_to if a pointer to a QMenu is passed
		 * to the function, the actions are appended to
		 * it instead of the creation of a new menu.
		 */
		QMenu * createSelectionMenu(QMenu * append_to = 0);
		//! Create a menu with column related operations
		/**
		 * \param append_to if a pointer to a QMenu is passed
		 * to the function, the actions are appended to
		 * it instead of the creation of a new menu.
		 */
		QMenu * createColumnMenu(QMenu * append_to = 0);
		//! Create a menu with row related operations
		/**
		 * \param append_to if a pointer to a QMenu is passed
		 * to the function, the actions are appended to
		 * it instead of the creation of a new menu.
		 */
		QMenu * createRowMenu(QMenu * append_to = 0);
		//! Create a menu with table related operations
		/**
		 * \param append_to if a pointer to a QMenu is passed
		 * to the function, the actions are appended to
		 * it instead of the creation of a new menu.
		 */
		QMenu * createTableMenu(QMenu * append_to = 0);
		//! Set a plot menu 
		/**
		 * The table takes ownership of the menu.
		 */
		void setPlotMenu(QMenu * menu);
		
	public slots:
		void setSelectionAs(SciDAVis::PlotDesignation pd);
		void cutSelection();
		void copySelection();
		void pasteIntoSelection();
		void clearSelectedCells();
		void maskSelection();
		void unmaskSelection();
		void setFormulaForSelection();
		void recalculateSelectedCells();
		void fillSelectedCellsWithRowNumbers();
		void fillSelectedCellsWithRandomNumbers();
		//! Insert columns depending on the selection
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
		//! Insert rows depending on the selection
		void insertEmptyRows();
		void removeSelectedRows();
		void editTypeAndFormatOfSelectedColumns();
		void editDescriptionOfCurrentColumn();
		//! Append as many columns as are selected
		void addColumns();
		//! Append as many rows as are selected
		void addRows();
		//! Show a context menu for the selected cells
		/**
		 * \param pos global position of the event 
		*/
		void showTableViewContextMenu(const QPoint& pos);
		//! Show a context menu for the selected columns
		/**
		 * \param pos global position of the event 
		*/
		void showTableViewColumnContextMenu(const QPoint& pos);
		//! Show a context menu for the selected rows
		/**
		 * \param pos global position of the event 
		*/
		void showTableViewRowContextMenu(const QPoint& pos);
		void createActions();
		void connectActions();

		void setColumnWidth(int col, int width);
		int columnWidth(int col) const;
		bool formulaModeActive() const;

		//! Return a new context menu.
		/**
		 * The caller takes ownership of the menu.
		 */
		void createContextMenu(QMenu * menu);
		//! Fill the part specific menu for the main window including setting the title
		/**
		 * \param menu the menu to append the actions to
		 * \param rc return code: true on success, otherwise false (e.g. part has no actions).
		 */
		void fillProjectMenu(QMenu * menu, bool * rc);
		//! Open the sort dialog for the given columns
		void sortDialog(QList<Column*> cols);

	public:
		static int defaultColumnWidth();
		static void setDefaultColumnWidth(int width);
		//! Set default for comment visibility for table views
		static void setDefaultCommentVisibility(bool visible);
		//! Return the default for comment visibility for table views
		static bool defaultCommentVisibility();
		static ActionManager * actionManager();
		static void initActionManager();
	private:
		static ActionManager * action_manager;
		//! Private ctor for initActionManager() only
		TableView();


	public slots:
		void activateFormulaMode(bool on);
		void goToCell(int row, int col);
		void rereadSectionSizes();
		void selectAll();
		void deselectAll();
		void toggleControlTabBar();
		void toggleComments();
		void showControlDescriptionTab();
		void showControlTypeTab();
		void showControlFormulaTab();
		void handleHorizontalSectionResized(int logicalIndex, int oldSize, int newSize); 
		void goToNextColumn();
		void goToPreviousColumn();
		void dimensionsDialog();
		//! Open the sort dialog for all columns
		void sortTable();
		void goToCell();

	protected slots:
		//! Advance current cell after [Return] or [Enter] was pressed
		void advanceCell();
		void handleHorizontalSectionMoved(int index, int from, int to);
		void handleHorizontalHeaderDoubleClicked(int index);
		void updateTypeInfo();
		void updateFormatBox();
		void handleHeaderDataChanged(Qt::Orientation orientation, int first, int last);
		void currentColumnChanged(const QModelIndex & current, const QModelIndex & previous);
		void selectionChanged(const QItemSelection & selected, const QItemSelection & deselected);
		void applyDescription();
		void applyType();
		void adjustActionNames();

	protected:
		//! Pointer to the item delegate
		TableItemDelegate * m_delegate;
		//! Pointer to the current underlying model
		TableModel * m_model;

		virtual void changeEvent(QEvent * event);
		void retranslateStrings();
		void setColumnForDescriptionTab(int col);

		bool eventFilter( QObject * watched, QEvent * event);

		//! UI with options tabs (description, format, formula etc.)
		Ui::ControlTabs ui;
		//! The table view (first part of the UI)
		TableViewWidget * m_view_widget;
		//! Widget that contains the control tabs UI from #ui
		QWidget * m_control_tabs;
		//! Button to toogle the visibility of #m_tool_box
		QToolButton * m_hide_button;
		QHBoxLayout * m_main_layout;
		TableDoubleHeaderView * m_horizontal_header;
		Table * m_table;

		//! Initialization
		void init();

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
		//! \name table related actions
		//@{
		QAction * action_toggle_comments;
		QAction * action_toggle_tabbar;
		QAction * action_select_all;
		QAction * action_add_column;
		QAction * action_clear_table;
		QAction * action_clear_masks;
		QAction * action_sort_table;
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

		QMenu * m_plot_menu;
};


#endif
