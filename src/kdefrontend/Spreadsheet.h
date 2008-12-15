/***************************************************************************
    File                 : Spreadsheet.h
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Stefan Gerlach (stefan.gerlach*uni-konstanz.de)
    Copyright            : (C) 2008 by Alexander Semke (alexander.semke*web.de)
    Copyright            : (C) 2007-2008 Tilman Benkert (thzs*gmx.net)
                           (replace * with @ in the email addresses)
    Description          : spreadsheet class

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
#ifndef SPREADSHEET_H
#define SPREADSHEET_H

#include <QWidget>
#include <QMenu>
#include <QDomElement>
#include <QTableView>

#include "table/TableModel.h"
#include "table/Table.h"
#include "elements/Set.h"
#include "elements/sheettype.h"

class QHBoxLayout;
class MainWin;

class Spreadsheet: public QWidget // remark: you could inherit from QTableView here, but that makes it really hard to add any other widget to the view later
{
	Q_OBJECT
public:
	Spreadsheet(Table *table);
	~Spreadsheet();
	SheetType sheetType() const { return m_type; }
	void resetHeader(int from=0);
	void addSet(Set* set);
	Set* set() { return m_set; }
	Table* table() { return m_table; }

	QString columnName(int col) const;
	void setColumnName(int col, QString name);
	SciDAVis::PlotDesignation columnType(int col) const;
	void setColumnType(int col, SciDAVis::PlotDesignation type);
	SciDAVis::ColumnMode columnFormat(int col) const;
	void setColumnFormat(int col, SciDAVis::ColumnMode name);

	int rowCount() const { return m_table->rowCount(); }
	void setRowCount(int count) { m_table->setRowCount(count); }
	int columnCount() const { return m_table->columnCount(); }
	void setColumnCount(int count){
		m_table->setColumnCount(count);
	}

	QString text(int row, int col) const;
	void setText(int row, int col, QString text);
	int currentRow() const;			//!< returns current row (latest selection)
	int currentColumn() const;		//!< returns current column (latest selection)
	QList<int> currentRows() const;		//!< returns a sorted list of selected rows
	QList<int> currentColumns() const;	//!< returns a sorted list of selected columns

private:
	Table *m_table;
	SheetType m_type;			//!< needed for mw->active{Work,Spread}sheet()
	Set* m_set;
	void contextMenuEvent(QContextMenuEvent *);
	QString columnHeader(int col) const;
	int filledRows(int col) const;	//!< returns number of filled rows in column col
	void displaySet();

public slots:
	void Menu(QMenu *menu);
	void setTitle(QString title="");
	void setRowNumber(int row=0);
	void addColumn() { setColumnCount(columnCount()+1); }
	QString Notes() const { return m_table->comment(); }
	void setNotes(QString notes="");
	void setProperties(QString label=0,
		SciDAVis::PlotDesignation type=SciDAVis::X,
		SciDAVis::ColumnMode format=SciDAVis::Numeric);

private slots:
	void plot();		//!< create a plot from the selected data
	void exportData();	//!< export selected data (ExportDialog)
	void editFunction();
	void setColumnValues();	//!< set colum  values (ColumnValuesDialog)
	void deleteSelectedColumns();
	void deleteSelectedRows();

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

	public slots:
		void setSelectionAs(SciDAVis::PlotDesignation pd);
		void cutSelection();
		void copySelection();
		void pasteIntoSelection();
		void clearSelectedCells();
		void maskSelection();
		void unmaskSelection();
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
		//! Insert rows depending on the selection
		void insertEmptyRows();
		void removeSelectedRows();
		//! Append as many columns as are selected
		void addColumns();
		//! Append as many rows as are selected
		void addRows();

		void setColumnWidth(int col, int width);
		int columnWidth(int col) const;
		bool formulaModeActive() const;

	public:
		static int defaultColumnWidth();
		static void setDefaultColumnWidth(int width);
		//! Set default for comment visibility for table views
		static void setDefaultCommentVisibility(bool visible);
		//! Return the default for comment visibility for table views
		static bool defaultCommentVisibility();

	public slots:
		void activateFormulaMode(bool on);
		void goToCell(int row, int col);
		void rereadSectionSizes();
		void selectAll();
		void deselectAll();
		void handleHorizontalSectionResized(int logicalIndex, int oldSize, int newSize);
		void goToNextColumn();
		void goToPreviousColumn();
		void dimensionsDialog();
		void goToCell();

	protected:
		//! Pointer to the current underlying model
		TableModel * m_model;

		//! The table view
		QTableView * m_view_widget;
		QHBoxLayout * m_main_layout;

		//! Initialization
		void init();

};

#endif //SPREADSHEET
