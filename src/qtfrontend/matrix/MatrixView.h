/***************************************************************************
    File                 : MatrixView.h
    Project              : SciDAVis
    Description          : View class for Matrix
    --------------------------------------------------------------------
    Copyright            : (C) 2008 Tilman Benkert (thzs*gmx.net)
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

#ifndef MATRIXVIEW_H
#define MATRIXVIEW_H

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
#include "core/globals.h"
#include "ui_matrixcontroltabs.h"

class Matrix;
class MatrixModel;

//! Helper class for MatrixView
class MatrixViewWidget : public QTableView
{
    Q_OBJECT

	public:
		//! Constructor
		MatrixViewWidget(QWidget * parent = 0) : QTableView(parent) {};

	protected:
		//! Overloaded function (cf. Qt documentation)
		virtual void keyPressEvent(QKeyEvent * event);

	signals:
		void advanceCell();

		public slots:
			void selectAll();
};

//! View class for Matrix
class MatrixView : public QWidget
{
    Q_OBJECT

	public:
		//! Constructor
		MatrixView(Matrix *matrix);
		//! Destructor
		virtual ~MatrixView();
		bool isControlTabBarVisible() { return m_control_tabs->isVisible(); }

		//! \name selection related functions
		//@{
		//! Return how many columns are selected
		/**
		 * If full is true, this function only returns the number of fully 
		 * selected columns.
		 */
		int selectedColumnCount(bool full = false);
		//! Returns true if column 'col' is selected; otherwise false
		/**
		 * If full is true, this function only returns true if the whole 
		 * column is selected.
		 */
		bool isColumnSelected(int col, bool full = false);
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
		//! Select a cell
		void setCellSelected(int row, int col);
		//! Select a range of cells
		void setCellsSelected(int first_row, int first_col, int last_row, int last_col);
		//! Determine the current cell (-1 if no cell is designated as the current)
		void getCurrentCell(int * row, int * col);
		//@}
		
		void setRowHeight(int row, int height);
		void setColumnWidth(int col, int width);
		int rowHeight(int row) const;
		int columnWidth(int col) const;


	public slots:
		void rereadSectionSizes();
		void goToCell(int row, int col);
		void selectAll();
		void toggleControlTabBar();
		void showControlCoordinatesTab();
		void showControlFormatTab();
		void showControlFormulaTab();
		void applyCoordinates();
		void updateCoordinatesTab();
		void updateFormulaTab();
		void applyFormula();
		void updateFormatTab();
		void applyFormat();
		void handleHorizontalSectionResized(int logicalIndex, int oldSize, int newSize); 
		void handleVerticalSectionResized(int logicalIndex, int oldSize, int newSize); 

	signals:
		void controlTabBarStatusChanged(bool visible);

	protected slots:
		//! Advance current cell after [Return] or [Enter] was pressed
		void advanceCell();
		void updateTypeInfo();

	protected:
		//! Pointer to the current underlying model
		MatrixModel * m_model;

		virtual void changeEvent(QEvent * event);
		void retranslateStrings();

		bool eventFilter( QObject * watched, QEvent * event);

	private:
		Ui::MatrixControlTabs ui;
		//! The matrix view (first part of the UI)
		MatrixViewWidget * m_view_widget;
		//! Widget that contains the control tabs UI from #ui
		QWidget * m_control_tabs;
		//! Button to toogle the visibility of #m_tool_box
		QToolButton * m_hide_button;
		QHBoxLayout * m_main_layout;
		Matrix * m_matrix;

		//! Initialization
		void init();
};


#endif
