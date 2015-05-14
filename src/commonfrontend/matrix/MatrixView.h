/***************************************************************************
    File                 : MatrixView.cpp
    Project              : LabPlot
    Description          : View class for Matrix
    --------------------------------------------------------------------
    Copyright            : (C) 2015 Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2008-2009 Tilman Benkert (thzs@gmx.net)

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

#ifndef MATRIXVIEW_H
#define MATRIXVIEW_H

#include <QWidget>

class Matrix;
class MatrixModel;
class QAction;
class QMenu;
class QTableView;

class MatrixView : public QWidget {
    Q_OBJECT

	public:
		MatrixView(Matrix*);
		virtual ~MatrixView();

		MatrixModel* model() const;
		void setRowHeight(int row, int height);
		void setColumnWidth(int col, int width);
		int rowHeight(int row) const;
		int columnWidth(int col) const;

		int selectedColumnCount(bool full = false);
		bool isColumnSelected(int col, bool full = false);
		int selectedRowCount(bool full = false);
		bool isRowSelected(int row, bool full = false);
		int firstSelectedColumn(bool full = false);
		int lastSelectedColumn(bool full = false);
		int firstSelectedRow(bool full = false);
		int lastSelectedRow(bool full = false);
		bool isCellSelected(int row, int col);
		void setCellSelected(int row, int col);
		void setCellsSelected(int first_row, int first_col, int last_row, int last_col);
		void getCurrentCell(int* row, int* col);

		void adjustHeaders();

	public slots:
		void createContextMenu(QMenu*) const;

	private:
		void init();
		void initActions();
		void initMenus();
		void connectActions();
		void goToCell(int row, int col);

		bool eventFilter(QObject*, QEvent*);
		virtual void keyPressEvent(QKeyEvent*);

		QTableView* m_tableView;
		Matrix* m_matrix;
		MatrixModel* m_model;

		//Actions
		QAction* action_cut_selection;
		QAction* action_copy_selection;
		QAction* action_paste_into_selection;
		QAction* action_clear_selection;

		QAction* action_select_all;
		QAction* action_clear_matrix;
		QAction* action_go_to_cell;
		QAction* action_dimensions_dialog;
		QAction* action_edit_format;
		QAction* action_edit_coordinates;
		QAction* action_set_formula;
		QAction* action_recalculate;
		QAction* action_import_image;
		QAction* action_duplicate;
		QAction* action_transpose;
		QAction* action_mirror_vertically;
		QAction* action_mirror_horizontally;

		QAction* action_header_format_1;
		QAction* action_header_format_2;
		QAction* action_header_format_3;

		QAction* action_insert_columns;
		QAction* action_remove_columns;
		QAction* action_clear_columns;
		QAction* action_add_columns;

		QAction* action_insert_rows;
		QAction* action_remove_rows;
		QAction* action_clear_rows;
		QAction* action_add_rows;

		//Menus
		QMenu* m_selectionMenu;
		QMenu* m_columnMenu;
		QMenu* m_rowMenu;
		QMenu* m_matrixMenu;
		QMenu* m_headerFormatMenu;

	private slots:
		void goToCell();
		void advanceCell();
		void handleHorizontalSectionResized(int logicalIndex, int oldSize, int newSize);
		void handleVerticalSectionResized(int logicalIndex, int oldSize, int newSize);

		void cutSelection();
		void copySelection();
		void pasteIntoSelection();
		void clearSelectedCells();

		void dimensionsDialog();
		void headerFormatChanged(QAction*);

		void addColumns();
		void insertEmptyColumns();
		void removeSelectedColumns();
		void clearSelectedColumns();
		void addRows();
		void insertEmptyRows();
		void removeSelectedRows();
		void clearSelectedRows();
};

#endif
