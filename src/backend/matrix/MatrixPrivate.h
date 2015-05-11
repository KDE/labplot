/***************************************************************************
    File                 : MatrixPrivate.h
    Project              : LabPlot
    Description          : Private members of Matrix.
    --------------------------------------------------------------------
    Copyright            : (C) 2015 by Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2008-2009 Tilman Benkert (thzs*gmx.net)

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

#ifndef MATRIXPRIVATE_H
#define MATRIXPRIVATE_H

#include <QVector>

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
class MatrixPrivate{
	public:
		MatrixPrivate(Matrix*);
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
		QString name() const { return q->name(); }
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
		void setNumericFormat(char format) { m_numeric_format = format; emit q->formatChanged(); }
		int displayedDigits()  const { return m_displayed_digits; }
		void setDisplayedDigits(int digits) { m_displayed_digits = digits;  emit q->formatChanged(); }
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
		void emitDataChanged(int top, int left, int bottom, int right) { emit q->dataChanged(top, left, bottom, right); }

	private:
		//! The owner aspect
		Matrix* q;
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
