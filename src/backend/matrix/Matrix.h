/***************************************************************************
    File                 : Matrix.h
    Project              : SciDAVis
    Description          : Aspect providing a spreadsheet to manage MxN matrix data
    --------------------------------------------------------------------
    Copyright            : (C) 2006-2008 Tilman Benkert (thzs*gmx.net)
    Copyright            : (C) 2006-2008 Knut Franke (knut.franke*gmx.de)
    Copyright            : (C) 2006-2007 Ion Vasilief (ion_vasilief*yahoo.fr)
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
#ifndef MATRIX_H
#define MATRIX_H

#include "core/AbstractScriptingEngine.h"
#include "core/AbstractPart.h"
#include "matrix/MatrixView.h"
#include "lib/macros.h"

class QContextMenuEvent;
class QEvent;
class ActionManager;

// (maximum) initial matrix size (this is the size of the window, not the number of cells)
#define _Matrix_initial_rows_ 10
#define _Matrix_initial_columns_ 3

// TODO: move all selection related stuff to the primary view

//! Aspect providing a spreadsheet to manage MxN matrix data
class Matrix : public AbstractPart, public scripted
{
    Q_OBJECT

	public:
		class Private; 
		friend class Private;

		/*!
		 * \brief Constructor
		 *
		 * \param engine scripting engine
		 * \param rows initial number of rows
		 * \param cols initial number of columns
		 * \param name object name
		 */
		Matrix(AbstractScriptingEngine *engine, int rows, int cols, const QString& name);
		~Matrix();

		//! Return an icon to be used for decorating my views.
		virtual QIcon icon() const;
		//! Return a new context menu.
		/**
		 * The caller takes ownership of the menu.
		 */
		virtual QMenu *createContextMenu();
		//! Construct a primary view on me.
		/**
		 * This method may be called multiple times during the life time of an Aspect, or it might not get
		 * called at all. Aspects must not depend on the existence of a view for their operation.
		 */
		virtual QWidget *view() const;
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
		QMenu * createMatrixMenu(QMenu * append_to = 0);
		//! Fill the part specific menu for the main window including setting the title
		/**
		 * \return true on success, otherwise false (e.g. part has no actions).
		 */
		virtual bool fillProjectMenu(QMenu * menu);

		void insertColumns(int before, int count);
		void appendColumns(int count) { insertColumns(columnCount(), count); }
		void removeColumns(int first, int count);
		void insertRows(int before, int count);
		void appendRows(int count) { insertRows(rowCount(), count); }
		void removeRows(int first, int count);
		//! Set the number of rows and columns
		void setDimensions(int rows, int cols);
		//! Return the total number of columns
		int columnCount() const;
		//! Return the total number of rows
		int rowCount() const;

		//! Set a plot menu 
		/**
		 * The matrix takes ownership of the menu.
		 */
		void setPlotMenu(QMenu * menu);
		//! Return the value in the given cell
		double cell(int row, int col) const;
		//! Set the value of the cell
		void setCell(int row, int col, double value );
		//! Return the values in the given cells as double vector
		QVector<double> columnCells(int col, int first_row, int last_row);
		//! Set the values in the given cells from a double vector
		void setColumnCells(int col, int first_row, int last_row, const QVector<double> & values);
		//! Return the values in the given cells as double vector
		QVector<double> rowCells(int row, int first_column, int last_column);
		//! Set the values in the given cells from a double vector
		void setRowCells(int row, int first_column, int last_column, const QVector<double> & values);
		//! Return the text displayed in the given cell
		QString text(int row, int col);
		void copy(Matrix * other);
		double xStart() const;
		double yStart() const;
		double xEnd() const;
		double yEnd() const;
		QString formula() const;
		void setFormula(const QString & formula);
		void setXStart(double x);
		void setXEnd(double x);
		void setYStart(double y);
		void setYEnd(double y);
		void setCoordinates(double x1, double x2, double y1, double y2);
		char numericFormat() const;
		int displayedDigits()  const;
		void setNumericFormat(char format);
		void setDisplayedDigits(int digits);

		//! \name serialize/deserialize
		//@{
		//! Save as XML
		virtual void save(QXmlStreamWriter *) const;
		//! Load from XML
		virtual bool load(XmlStreamReader *);
		//@}

		//! This method should only be called by the view.
		/** This method does not change the view, it only changes the
		 * values that are saved when the matrix is saved. The view
		 * has to take care of reading and applying these values */
		void setRowHeight(int row, int height);
		//! This method should only be called by the view.
		/** This method does not change the view, it only changes the
		 * values that are saved when the matrix is saved. The view
		 * has to take care of reading and applying these values */
		void setColumnWidth(int col, int width);
		int rowHeight(int row) const;
		int columnWidth(int col) const;

	public:
		static ActionManager * actionManager();
		static void initActionManager();
		static int defaultColumnWidth() { return default_column_width; }
		static int defaultRowHeight() { return default_row_height; }
		static void setDefaultColumnWidth(int width) { default_column_width = width; }
		static void setDefaultRowHeight(int height) { default_row_height = height; }

	private:
		static ActionManager * action_manager;
		//! Private ctor for initActionManager() only
		Matrix();
		// TODO: the default sizes are to be controlled by the global Matrix settings
		static int default_column_width;
		static int default_row_height;
	public:
		static Matrix * fromImage(const QImage & image);

	public slots:
		//! Clear the whole matrix (i.e. set all cells to 0.0)
		void clear();
		void transpose();
		void mirrorVertically();
		void mirrorHorizontally();

		void cutSelection();
		void copySelection();
		void pasteIntoSelection();
		void clearSelectedCells();
		void dimensionsDialog();
		void goToCell();
		//! Insert columns depending on the selection
		void insertEmptyColumns();
		//! Insert rows depending on the selection
		void insertEmptyRows();
		void removeSelectedColumns();
		void removeSelectedRows();
		void clearSelectedColumns();
		void clearSelectedRows();
		void selectAll();
		//! Show a context menu for the selected cells
		/**
		 * \param pos global position of the event 
		*/
		void showMatrixViewContextMenu(const QPoint& pos);
		//! Show a context menu for the selected columns
		/**
		 * \param pos global position of the event 
		*/
		void showMatrixViewColumnContextMenu(const QPoint& pos);
		//! Show a context menu for the selected rows
		/**
		 * \param pos global position of the event 
		*/
		void showMatrixViewRowContextMenu(const QPoint& pos);
		void editFormat();
		void editCoordinates();
		void editFormula();
		//! Append as many columns as are selected
		void addColumns();
		//! Append as many rows as are selected
		void addRows();
		void importImageDialog();
		//! Duplicate the matrix inside its folder
		void duplicate();

	signals:
		void columnsAboutToBeInserted(int before, int count);
		void columnsInserted(int first, int count);
		void columnsAboutToBeRemoved(int first, int count);
		void columnsRemoved(int first, int count);
		void rowsAboutToBeInserted(int before, int count);
		void rowsInserted(int first, int count);
		void rowsAboutToBeRemoved(int first, int count);
		void rowsRemoved(int first, int count);
		void dataChanged(int top, int left, int bottom, int right);
		void coordinatesChanged();
		void formulaChanged();
		void formatChanged();

	private slots:
		void adjustTabBarAction(bool visible);

	private:
		void createActions();
		void connectActions();
		void addActionsToView() const;

		//! Read XML display element
		bool readDisplayElement(XmlStreamReader * reader);
		//! Read XML coodinates element
		bool readCoordinatesElement(XmlStreamReader * reader);
		//! Read XML formula element
		bool readFormulaElement(XmlStreamReader * reader);
		//! Read XML cell element
		bool readCellElement(XmlStreamReader * reader);
		bool readRowHeightElement(XmlStreamReader * reader);
		bool readColumnWidthElement(XmlStreamReader * reader);

		QMenu * m_plot_menu;

		//! \name selection related actions
		//@{
		QAction * action_cut_selection;
		QAction * action_copy_selection;
		QAction * action_paste_into_selection;
		QAction * action_clear_selection;
		//@}
		//! \name matrix related actions
		//@{
		QAction * action_toggle_tabbar;
		QAction * action_select_all;
		QAction * action_clear_matrix;
		QAction * action_go_to_cell;
		QAction * action_dimensions_dialog;
		QAction * action_edit_format;
		QAction * action_edit_coordinates;
		QAction * action_set_formula;
		QAction * action_recalculate;
		QAction * action_import_image;
		QAction * action_duplicate;
		QAction * action_transpose;
		QAction * action_mirror_vertically;
		QAction * action_mirror_horizontally;
		//@}
		//! \name column related actions
		//@{
		QAction * action_insert_columns;
		QAction * action_remove_columns;
		QAction * action_clear_columns;
		QAction * action_add_columns;
		//@}
		//! \name row related actions
		//@{
		QAction * action_insert_rows;
		QAction * action_remove_rows;
		QAction * action_clear_rows;
		QAction * action_add_rows;
		//@}

		mutable MatrixView *m_view;
		Private *m_matrix_private;
};

/**
  This private class manages matrix based data (i.e., mathematically
  a MxN matrix with M rows, N columns). These data are typically
  used to for 3D plots.
  
  The API of this private class is to be called by Matrix and matrix
  commands only. Matrix may only call the reading functions to ensure 
  that undo/redo is possible for all data changing operations.

  The values of the matrix are stored as double precision values. They
  are managed by QVector<double> objects. Although rows and columns
  are equally important in a matrix, the columns are chosen to
  be contiguous in memory to allow easier copying between
  column and matrix data.
  */
class Matrix::Private
{
	public:
		Private(Matrix *owner); 
		//! Insert columns before column number 'before'
		/**
		 * If 'first' is equal to the current number of columns,
		 * the columns will be appended.
		 * \param before index of the column to insert before
		 * \param count the number of columns to be inserted
		 */
		void insertColumns(int before, int count);
		//! Remove Columns
		/**
		 * \param first index of the first column to be removed
		 * \param count number of columns to remove
		 */
		void removeColumns(int first, int count);
		//! Insert rows before row number 'before'
		/**
		 * If 'first' is equal to the current number of rows,
		 * the rows will be appended.
		 * \param before index of the row to insert before
		 * \param count the number of rows to be inserted
		 */
		void insertRows(int before, int count);
		//! Remove Columns
		/**
		 * \param first index of the first row to be removed
		 * \param count number of rows to remove
		 */
		void removeRows(int first, int count);
		//! Return the number of columns in the table
		int columnCount() const { return m_column_count; }
		//! Return the number of rows in the table
		int rowCount() const { return m_row_count; }
		QString name() const { return m_owner->name(); }
		//! Return the value in the given cell
		double cell(int row, int col) const;
		//! Set the value in the given cell
		void setCell(int row, int col, double value);
		//! Return the values in the given cells as double vector
		QVector<double> columnCells(int col, int first_row, int last_row);
		//! Set the values in the given cells from a double vector
		void setColumnCells(int col, int first_row, int last_row, const QVector<double> & values);
		//! Return the values in the given cells as double vector
		QVector<double> rowCells(int row, int first_column, int last_column);
		//! Set the values in the given cells from a double vector
		void setRowCells(int row, int first_column, int last_column, const QVector<double> & values);
		char numericFormat() const { return m_numeric_format; }
		void setNumericFormat(char format) { m_numeric_format = format; emit m_owner->formatChanged(); }
		int displayedDigits()  const { return m_displayed_digits; }
		void setDisplayedDigits(int digits) { m_displayed_digits = digits;  emit m_owner->formatChanged(); }
		//! Fill column with zeroes
		void clearColumn(int col);
		double xStart() const;
		double yStart() const;
		double xEnd() const;
		double yEnd() const;
		QString formula() const;
		void setFormula(const QString & formula);
		void setXStart(double x);
		void setXEnd(double x);
		void setYStart(double y);
		void setYEnd(double y);
		void setRowHeight(int row, int height) { m_row_heights[row] = height; }
		void setColumnWidth(int col, int width) { m_column_widths[col] = width; }
		int rowHeight(int row) const { return m_row_heights.at(row); }
		int columnWidth(int col) const { return m_column_widths.at(col); }
		//! Enable/disable the emission of dataChanged signals.
		/** This can be used to suppress the emission of dataChanged signals
		 * temporally. It does not suppress any other signals however.
		 * Typical code:
		 * <code>
		 * m_matrix_private->blockChangeSignals(true);
		 * for (...)
		 *     for(...)
		 *         setCell(...);
		 * m_matrix_private->blockChangeSignals(false);
		 * emit dataChanged(0, 0, rowCount()-1, columnCount()-1);
		 * </code>
		 */
		void blockChangeSignals(bool block) { m_block_change_signals = block; }
		//! Access to the dataChanged signal for commands
		void emitDataChanged(int top, int left, int bottom, int right) { emit m_owner->dataChanged(top, left, bottom, right); }
		
	private:
		//! The owner aspect
		Matrix *m_owner;
		//! The number of columns
		int m_column_count;
		//! The number of rows
		int m_row_count;
		//! The matrix data
		QVector< QVector<double> > m_data;	
		//! Row widths
		QList<int> m_row_heights;
		//! Columns widths
		QList<int> m_column_widths;
		//! Last formula used to calculate cell values
		QString m_formula; // TODO: should we support interval/rectangle based formulas?
		//! Format code for displaying numbers
		char m_numeric_format;
		//! Number of significant digits
		int m_displayed_digits;
		double m_x_start, //!< X value corresponding to column 1
			   m_x_end,  //!< X value corresponding to the last column
			   m_y_start,  //!< Y value corresponding to row 1
			   m_y_end;  //!< Y value corresponding to the last row
		bool m_block_change_signals;

};

#endif
