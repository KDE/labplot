/***************************************************************************
    File                 : SpreadsheetView.h
    Project              : SciDAVis
    Description          : View class for Spreadsheet
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

#include <QTableView>
#include <QHeaderView>

#include "core/globals.h"
#include "lib/IntervalAttribute.h"

class Column;
class Spreadsheet;
class SpreadsheetModel;
class SpreadsheetItemDelegate;
class SpreadsheetDoubleHeaderView;
class ActionManager;
class AbstractAspect;


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
		
		static ActionManager * actionManager();
		static void initActionManager();
		
	private:
	  	 void init();
		Spreadsheet * m_spreadsheet;
		SpreadsheetItemDelegate * m_delegate;
		SpreadsheetModel * m_model;
		SpreadsheetDoubleHeaderView * m_horizontalHeader;
		
		bool eventFilter( QObject * watched, QEvent * event);
		void keyPressEvent(QKeyEvent * event);
		
		static ActionManager * action_manager;
		SpreadsheetView();

	public slots:
		void activateFormulaMode(bool on);
		void goToCell(int row, int col);
		void deselectAll();
		void toggleComments();
		void handleHorizontalSectionResized(int logicalIndex, int oldSize, int newSize); 
		void goToNextColumn();
		void goToPreviousColumn();
		void goToCell();
		
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
		void statisticsOnSelectedColumns();
		void statisticsOnSelectedRows();
		void insertEmptyRows();
		void removeSelectedRows();
		
		void addColumns();
		void addRows();
		
		void setColumnWidth(int col, int width);
		int columnWidth(int col) const;
		bool formulaModeActive() const;

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
