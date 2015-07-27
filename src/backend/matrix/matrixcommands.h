/***************************************************************************
    File                 : matrixcommands.h
    Project              : LabPlot
    Description          : Commands used in Matrix (part of the undo/redo framework)
    --------------------------------------------------------------------
    Copyright            : (C) 2015 Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2008 Tilman Benkert (thzs@gmx.net)

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

#ifndef MATRIX_COMMANDS_H
#define MATRIX_COMMANDS_H

#include <QUndoCommand>
#include <QVector>
#include "Matrix.h"

//! Insert columns
class MatrixInsertColumnsCmd : public QUndoCommand {
	public:
		MatrixInsertColumnsCmd(MatrixPrivate* private_obj, int before, int count, QUndoCommand* parent = 0);
		virtual void redo();
		virtual void undo();

	private:
		MatrixPrivate* m_private_obj;
		int m_before; //! Column to insert before
		int m_count; //! The number of new columns
};


//! Insert rows
class MatrixInsertRowsCmd : public QUndoCommand {
	public:
		MatrixInsertRowsCmd(MatrixPrivate* private_obj, int before, int count, QUndoCommand* parent = 0);
		virtual void redo();
		virtual void undo();

	private:
		MatrixPrivate* m_private_obj;
		int m_before; //! Row to insert before
		int m_count; //! The number of new rows
};


//! Remove columns
class MatrixRemoveColumnsCmd : public QUndoCommand {
	public:
		MatrixRemoveColumnsCmd(MatrixPrivate* private_obj, int first, int count, QUndoCommand* parent = 0);
		virtual void redo();
		virtual void undo();

	private:
		MatrixPrivate* m_private_obj;

		int m_first; //! First column to remove
		int m_count; //! The number of columns to remove
		QVector< QVector<double> > m_backups; //! Backups of the removed columns
};


//! Remove rows
class MatrixRemoveRowsCmd : public QUndoCommand {
	public:
		MatrixRemoveRowsCmd(MatrixPrivate* private_obj, int first, int count, QUndoCommand* parent = 0);
		virtual void redo();
		virtual void undo();

	private:
		MatrixPrivate* m_private_obj;
		int m_first; //! First row to remove
		int m_count; //! The number of rows to remove
		QVector< QVector<double> > m_backups; //! Backups of the removed rows
};


//! Clear matrix
class MatrixClearCmd : public QUndoCommand {
	public:
		explicit MatrixClearCmd(MatrixPrivate* private_obj, QUndoCommand* parent = 0);
		virtual void redo();
		virtual void undo();

	private:
		MatrixPrivate* m_private_obj;
		QVector< QVector<double> > m_backups; //! Backups of the cleared cells
};


//! Clear matrix column
class MatrixClearColumnCmd : public QUndoCommand {
	public:
		MatrixClearColumnCmd(MatrixPrivate* private_obj, int col, QUndoCommand* parent = 0);
		virtual void redo();
		virtual void undo();

	private:
		MatrixPrivate* m_private_obj;
		int m_col; //! The index of the column
		QVector<double> m_backup; //! Backup of the cleared column
};


// Set cell value
class MatrixSetCellValueCmd : public QUndoCommand {
	public:
		MatrixSetCellValueCmd(MatrixPrivate* private_obj, int row, int col, double value, QUndoCommand* parent = 0);
		virtual void redo();
		virtual void undo();

	private:
		MatrixPrivate* m_private_obj;
		int m_row; //! The index of the row
		int m_col; //! The index of the column
		double m_value; //! New cell value
		double m_old_value; //! Backup of the changed value
};


// Set matrix coordinates
class MatrixSetCoordinatesCmd : public QUndoCommand {
	public:
		MatrixSetCoordinatesCmd(MatrixPrivate* private_obj, double x1, double x2, double y1, double y2, QUndoCommand* parent = 0);
		virtual void redo();
		virtual void undo();

	private:
		MatrixPrivate* m_private_obj;
		double m_new_x1;
		double m_new_x2;
		double m_new_y1;
		double m_new_y2;
		double m_old_x1;
		double m_old_x2;
		double m_old_y1;
		double m_old_y2;
};


//! Set matrix formula
class MatrixSetFormulaCmd : public QUndoCommand {
	public:
		MatrixSetFormulaCmd(MatrixPrivate* private_obj, QString formula);
		virtual void redo();
		virtual void undo();

	private:
		MatrixPrivate* m_private_obj;
		QString m_other_formula;
};


// Set cell values for (a part of) a column at once
class MatrixSetColumnCellsCmd : public QUndoCommand {
	public:
		MatrixSetColumnCellsCmd(MatrixPrivate* private_obj, int col, int first_row, int last_row, const QVector<double>& values, QUndoCommand* parent = 0);
		virtual void redo();
		virtual void undo();

	private:
		MatrixPrivate* m_private_obj;
		int m_col; //! The index of the column
		int m_first_row; //! The index of the first row
		int m_last_row; //! The index of the last row
		QVector<double> m_values; //! New cell values
		QVector<double> m_old_values; //! Backup of the changed values
};


//! Set cell values for (a part of) a row at once
class MatrixSetRowCellsCmd : public QUndoCommand {
	public:
		MatrixSetRowCellsCmd(MatrixPrivate* private_obj, int row, int first_column, int last_column, const QVector<double>& values, QUndoCommand* parent = 0);
		virtual void redo();
		virtual void undo();

	private:
		MatrixPrivate* m_private_obj;
		int m_row; //! The index of the row
		int m_first_column; //! The index of the first column
		int m_last_column; //! The index of the last column
		QVector<double> m_values; //! New cell values
		QVector<double> m_old_values; //! Backup of the changed values
};


//! Transpose the matrix
class MatrixTransposeCmd : public QUndoCommand {
	public:
		explicit MatrixTransposeCmd(MatrixPrivate* private_obj, QUndoCommand* parent = 0);
		virtual void redo();
		virtual void undo();

	private:
		MatrixPrivate* m_private_obj;
};


//! Mirror the matrix horizontally
class MatrixMirrorHorizontallyCmd : public QUndoCommand {
	public:
		explicit MatrixMirrorHorizontallyCmd(MatrixPrivate* private_obj, QUndoCommand* parent = 0);
		virtual void redo();
		virtual void undo();

	private:
		MatrixPrivate* m_private_obj;
};

// Mirror the matrix vertically
class MatrixMirrorVerticallyCmd : public QUndoCommand {
	public:
		explicit MatrixMirrorVerticallyCmd(MatrixPrivate* private_obj, QUndoCommand* parent = 0);
		virtual void redo();
		virtual void undo();

	private:
		MatrixPrivate* m_private_obj;
};


// Replace matrix values
class MatrixReplaceValuesCmd : public QUndoCommand {
	public:
		explicit MatrixReplaceValuesCmd(MatrixPrivate* private_obj, const QVector<QVector<double> >& new_values, QUndoCommand* parent = 0);
		virtual void redo();
		virtual void undo();

	private:
		MatrixPrivate* m_private_obj;
		QVector<QVector<double> > m_old_values;
		QVector<QVector<double> > m_new_values;
};


#endif // MATRIX_COMMANDS_H
