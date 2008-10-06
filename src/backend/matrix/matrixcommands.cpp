/***************************************************************************
    File                 : matrixcommands.cpp
    Project              : SciDAVis
    Description          : Commands used in Matrix (part of the undo/redo framework)
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

#include "matrixcommands.h"

///////////////////////////////////////////////////////////////////////////
// class MatrixInsertColumnsCmd
///////////////////////////////////////////////////////////////////////////
MatrixInsertColumnsCmd::MatrixInsertColumnsCmd( Matrix::Private * private_obj, int before, int count, QUndoCommand * parent)
 : QUndoCommand( parent ), m_private_obj(private_obj), m_before(before), m_count(count)
{
	setText(QObject::tr("%1: insert %2 column(s)").arg(m_private_obj->name()).arg(m_count));
}

MatrixInsertColumnsCmd::~MatrixInsertColumnsCmd()
{
}

void MatrixInsertColumnsCmd::redo()
{
	m_private_obj->insertColumns(m_before, m_count);
}

void MatrixInsertColumnsCmd::undo()
{
	m_private_obj->removeColumns(m_before, m_count);
}
///////////////////////////////////////////////////////////////////////////
// end of class MatrixInsertColumnsCmd
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// class MatrixInsertRowsCmd
///////////////////////////////////////////////////////////////////////////
MatrixInsertRowsCmd::MatrixInsertRowsCmd( Matrix::Private * private_obj, int before, int count, QUndoCommand * parent)
 : QUndoCommand( parent ), m_private_obj(private_obj), m_before(before), m_count(count)
{
	setText(QObject::tr("%1: insert %2 row(s)").arg(m_private_obj->name()).arg(m_count));
}

MatrixInsertRowsCmd::~MatrixInsertRowsCmd()
{
}

void MatrixInsertRowsCmd::redo()
{
	m_private_obj->insertRows(m_before, m_count);
}

void MatrixInsertRowsCmd::undo()
{
	m_private_obj->removeRows(m_before, m_count);
}
///////////////////////////////////////////////////////////////////////////
// end of class MatrixInsertRowsCmd
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// class MatrixRemoveColumnsCmd
///////////////////////////////////////////////////////////////////////////
MatrixRemoveColumnsCmd::MatrixRemoveColumnsCmd( Matrix::Private * private_obj, int first, int count, QUndoCommand * parent)
 : QUndoCommand( parent ), m_private_obj(private_obj), m_first(first), m_count(count)
{
	setText(QObject::tr("%1: remove %2 column(s)").arg(m_private_obj->name()).arg(m_count));
}

MatrixRemoveColumnsCmd::~MatrixRemoveColumnsCmd()
{
}

void MatrixRemoveColumnsCmd::redo()
{
	if(m_backups.isEmpty())
	{
		int last_row = m_private_obj->rowCount()-1;
		for(int i=0; i<m_count; i++)
			m_backups.append(m_private_obj->columnCells(m_first+i, 0, last_row));
	}
	m_private_obj->removeColumns(m_first, m_count);
}

void MatrixRemoveColumnsCmd::undo()
{
	m_private_obj->insertColumns(m_first, m_count);
	int last_row = m_private_obj->rowCount()-1;
	for(int i=0; i<m_count; i++)
		m_private_obj->setColumnCells(m_first+i, 0, last_row, m_backups.at(i));
}
///////////////////////////////////////////////////////////////////////////
// end of class MatrixRemoveColumnsCmd
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// class MatrixRemoveRowsCmd
///////////////////////////////////////////////////////////////////////////
MatrixRemoveRowsCmd::MatrixRemoveRowsCmd( Matrix::Private * private_obj, int first, int count, QUndoCommand * parent)
 : QUndoCommand( parent ), m_private_obj(private_obj), m_first(first), m_count(count)
{
	setText(QObject::tr("%1: remove %2 row(s)").arg(m_private_obj->name()).arg(m_count));
}

MatrixRemoveRowsCmd::~MatrixRemoveRowsCmd()
{
}

void MatrixRemoveRowsCmd::redo()
{
	if(m_backups.isEmpty())
	{
		int last_row = m_first+m_count-1;
		for(int col=0; col<m_private_obj->columnCount(); col++)
			m_backups.append(m_private_obj->columnCells(col, m_first, last_row));
	}
	m_private_obj->removeRows(m_first, m_count);
}

void MatrixRemoveRowsCmd::undo()
{
	m_private_obj->insertRows(m_first, m_count);
	int last_row = m_first+m_count-1;
	for(int col=0; col<m_private_obj->columnCount(); col++)
		m_private_obj->setColumnCells(col, m_first, last_row, m_backups.at(col));
}
///////////////////////////////////////////////////////////////////////////
// end of class MatrixRemoveRowsCmd
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// class MatrixClearCmd
///////////////////////////////////////////////////////////////////////////
MatrixClearCmd::MatrixClearCmd( Matrix::Private * private_obj, QUndoCommand * parent)
 : QUndoCommand( parent ), m_private_obj(private_obj)
{
	setText(QObject::tr("%1: clear").arg(m_private_obj->name()));
}

MatrixClearCmd::~MatrixClearCmd()
{
}

void MatrixClearCmd::redo()
{
	if(m_backups.isEmpty())
	{
		int last_row = m_private_obj->rowCount()-1;
		for(int i=0; i<m_private_obj->columnCount(); i++)
			m_backups.append(m_private_obj->columnCells(i, 0, last_row));
	}
	for(int i=0; i<m_private_obj->columnCount(); i++)
		m_private_obj->clearColumn(i);
}

void MatrixClearCmd::undo()
{
	int last_row = m_private_obj->rowCount()-1;
	for(int i=0; i<m_private_obj->columnCount(); i++)
		m_private_obj->setColumnCells(i, 0, last_row, m_backups.at(i));
}
///////////////////////////////////////////////////////////////////////////
// end of class MatrixClearCmd
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// class MatrixClearColumnCmd
///////////////////////////////////////////////////////////////////////////
MatrixClearColumnCmd::MatrixClearColumnCmd( Matrix::Private * private_obj, int col, QUndoCommand * parent)
 : QUndoCommand( parent ), m_private_obj(private_obj), m_col(col)
{
	setText(QObject::tr("%1: clear column %2").arg(m_private_obj->name()).arg(m_col+1));
}

MatrixClearColumnCmd::~MatrixClearColumnCmd()
{
}

void MatrixClearColumnCmd::redo()
{
	if(m_backup.isEmpty())
		m_backup = m_private_obj->columnCells(m_col, 0, m_private_obj->rowCount()-1);
	m_private_obj->clearColumn(m_col);
}

void MatrixClearColumnCmd::undo()
{
	m_private_obj->setColumnCells(m_col, 0, m_private_obj->rowCount()-1, m_backup);
}
///////////////////////////////////////////////////////////////////////////
// end of class MatrixClearColumnCmd
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// class MatrixSetCellValueCmd
///////////////////////////////////////////////////////////////////////////
MatrixSetCellValueCmd::MatrixSetCellValueCmd( Matrix::Private * private_obj, int row, int col, double value, QUndoCommand * parent)
 : QUndoCommand( parent ), m_private_obj(private_obj), m_row(row), m_col(col), m_value(value)
{
	// remark: don't use many QString::arg() calls in ctors of commands that might be called often,
	// they use a lot of execution time
	setText(QObject::tr("%1: set cell value").arg(m_private_obj->name()));
}

MatrixSetCellValueCmd::~MatrixSetCellValueCmd()
{
}

void MatrixSetCellValueCmd::redo()
{
	m_old_value = m_private_obj->cell(m_row, m_col);
	m_private_obj->setCell(m_row, m_col, m_value);
}

void MatrixSetCellValueCmd::undo()
{
	m_private_obj->setCell(m_row, m_col, m_old_value);
}
///////////////////////////////////////////////////////////////////////////
// end of class MatrixSetCellValueCmd
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// class MatrixSetCoordinatesCmd
///////////////////////////////////////////////////////////////////////////
MatrixSetCoordinatesCmd::MatrixSetCoordinatesCmd( Matrix::Private * private_obj, double x1, double x2, double y1, double y2, QUndoCommand * parent)
 : QUndoCommand( parent ), m_private_obj(private_obj), m_new_x1(x1), m_new_x2(x2), m_new_y1(y1), m_new_y2(y2)
{
	setText(QObject::tr("%1: set matrix coordinates").arg(m_private_obj->name()));
}

MatrixSetCoordinatesCmd::~MatrixSetCoordinatesCmd()
{
}

void MatrixSetCoordinatesCmd::redo()
{
	m_old_x1 = m_private_obj->xStart();
	m_old_x2 = m_private_obj->xEnd();
	m_old_y1 = m_private_obj->yStart();
	m_old_y2 = m_private_obj->yEnd();
	m_private_obj->setXStart(m_new_x1);
	m_private_obj->setXEnd(m_new_x2);
	m_private_obj->setYStart(m_new_y1);
	m_private_obj->setYEnd(m_new_y2);
}

void MatrixSetCoordinatesCmd::undo()
{
	m_private_obj->setXStart(m_old_x1);
	m_private_obj->setXEnd(m_old_x2);
	m_private_obj->setYStart(m_old_y1);
	m_private_obj->setYEnd(m_old_y2);
}

///////////////////////////////////////////////////////////////////////////
// end of class MatrixSetCoordinatesCmd
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// class MatrixSetFormatCmd
///////////////////////////////////////////////////////////////////////////
MatrixSetFormatCmd::MatrixSetFormatCmd(Matrix::Private * private_obj, char new_format)
	: m_private_obj(private_obj), m_other_format(new_format) 
{
	setText(QObject::tr("%1: set numeric format to '%2'").arg(m_private_obj->name()).arg(new_format));
}

void MatrixSetFormatCmd::redo() 
{
	char tmp = m_private_obj->numericFormat();
	m_private_obj->setNumericFormat(m_other_format);
	m_other_format = tmp;
}

void MatrixSetFormatCmd::undo() 
{ 
	redo(); 
}

///////////////////////////////////////////////////////////////////////////
// end of class MatrixSetFormatCmd
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// class MatrixSetDigitsCmd
///////////////////////////////////////////////////////////////////////////
MatrixSetDigitsCmd::MatrixSetDigitsCmd(Matrix::Private * private_obj, int new_digits)
	: m_private_obj(private_obj), m_other_digits(new_digits) 
{
	setText(QObject::tr("%1: set decimal digits to %2").arg(m_private_obj->name()).arg(new_digits));
}

void MatrixSetDigitsCmd::redo() 
{
	int tmp = m_private_obj->displayedDigits();
	m_private_obj->setDisplayedDigits(m_other_digits);
	m_other_digits = tmp;
}

void MatrixSetDigitsCmd::undo() 
{ 
	redo(); 
}

///////////////////////////////////////////////////////////////////////////
// end of class MatrixSetDigitsCmd
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// class MatrixSetFormulaCmd
///////////////////////////////////////////////////////////////////////////
MatrixSetFormulaCmd::MatrixSetFormulaCmd(Matrix::Private * private_obj, QString formula)
	: m_private_obj(private_obj), m_other_formula(formula) 
{
	setText(QObject::tr("%1: set formula").arg(m_private_obj->name()));
}

void MatrixSetFormulaCmd::redo() 
{
	QString tmp = m_private_obj->formula();
	m_private_obj->setFormula(m_other_formula);
	m_other_formula = tmp;
}

void MatrixSetFormulaCmd::undo() 
{ 
	redo(); 
}

///////////////////////////////////////////////////////////////////////////
// end of class MatrixSetFormulaCmd
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// class MatrixSetColumnCellsCmd
///////////////////////////////////////////////////////////////////////////
MatrixSetColumnCellsCmd::MatrixSetColumnCellsCmd( Matrix::Private * private_obj, int col, int first_row, 
		int last_row, const QVector<double> & values, QUndoCommand * parent)
 : QUndoCommand( parent ), m_private_obj(private_obj), m_col(col), m_first_row(first_row), 
 		m_last_row(last_row), m_values(values)
{
	setText(QObject::tr("%1: set cell values").arg(m_private_obj->name()));
}

MatrixSetColumnCellsCmd::~MatrixSetColumnCellsCmd()
{
}

void MatrixSetColumnCellsCmd::redo()
{
	if (m_old_values.isEmpty())
		m_old_values = m_private_obj->columnCells(m_col, m_first_row, m_last_row);
	m_private_obj->setColumnCells(m_col, m_first_row, m_last_row, m_values);
}

void MatrixSetColumnCellsCmd::undo()
{
	m_private_obj->setColumnCells(m_col, m_first_row, m_last_row, m_old_values);
}
///////////////////////////////////////////////////////////////////////////
// end of class MatrixSetColumnCellsCmd
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// class MatrixSetRowCellsCmd
///////////////////////////////////////////////////////////////////////////
MatrixSetRowCellsCmd::MatrixSetRowCellsCmd( Matrix::Private * private_obj, int row, int first_column, 
		int last_column, const QVector<double> & values, QUndoCommand * parent)
 : QUndoCommand( parent ), m_private_obj(private_obj), m_row(row), m_first_column(first_column), 
 		m_last_column(last_column), m_values(values)
{
	setText(QObject::tr("%1: set cell values").arg(m_private_obj->name()));
}

MatrixSetRowCellsCmd::~MatrixSetRowCellsCmd()
{
}

void MatrixSetRowCellsCmd::redo()
{
	if (m_old_values.isEmpty())
		m_old_values = m_private_obj->rowCells(m_row, m_first_column, m_last_column);
	m_private_obj->setRowCells(m_row, m_first_column, m_last_column, m_values);
}

void MatrixSetRowCellsCmd::undo()
{
	m_private_obj->setRowCells(m_row, m_first_column, m_last_column, m_old_values);
}
///////////////////////////////////////////////////////////////////////////
// end of class MatrixSetRowCellsCmd
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// class MatrixTransposeCmd
///////////////////////////////////////////////////////////////////////////
MatrixTransposeCmd::MatrixTransposeCmd( Matrix::Private * private_obj, QUndoCommand * parent)
 : QUndoCommand( parent ), m_private_obj(private_obj)
{
	setText(QObject::tr("%1: transpose").arg(m_private_obj->name()));
}

MatrixTransposeCmd::~MatrixTransposeCmd()
{
}

void MatrixTransposeCmd::redo()
{
	int rows = m_private_obj->rowCount();
	int cols = m_private_obj->columnCount();
	int temp_size = qMax(rows, cols);
	m_private_obj->blockChangeSignals(true);
	if (cols < rows)
		m_private_obj->insertColumns(cols, temp_size - cols);
	else if (cols > rows)
		m_private_obj->insertRows(rows, temp_size - rows);
	for(int i = 1; i<temp_size; i++)
	{
		QVector<double> row = m_private_obj->rowCells(i, 0, i-1);
		QVector<double> col = m_private_obj->columnCells(i, 0, i-1);
		m_private_obj->setRowCells(i, 0, i-1, col);
		m_private_obj->setColumnCells(i, 0, i-1, row);
	}
	if (cols < rows)
		m_private_obj->removeRows(cols, temp_size - cols);
	else if (cols > rows)
		m_private_obj->removeColumns(rows, temp_size - rows);
	m_private_obj->blockChangeSignals(false);
	m_private_obj->emitDataChanged(0, 0, m_private_obj->rowCount()-1, m_private_obj->columnCount()-1);
}

void MatrixTransposeCmd::undo()
{
	redo();
}
///////////////////////////////////////////////////////////////////////////
// end of class MatrixTransposeCmd
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// class MatrixMirrorHorizontallyCmd
///////////////////////////////////////////////////////////////////////////
MatrixMirrorHorizontallyCmd::MatrixMirrorHorizontallyCmd( Matrix::Private * private_obj, QUndoCommand * parent)
 : QUndoCommand( parent ), m_private_obj(private_obj)
{
	setText(QObject::tr("%1: mirror horizontally").arg(m_private_obj->name()));
}

MatrixMirrorHorizontallyCmd::~MatrixMirrorHorizontallyCmd()
{
}

void MatrixMirrorHorizontallyCmd::redo()
{
	int rows = m_private_obj->rowCount();
	int cols = m_private_obj->columnCount();
	int middle = cols/2;
	m_private_obj->blockChangeSignals(true);
	for(int i = 0; i<middle; i++)
	{
		QVector<double> temp = m_private_obj->columnCells(i, 0, rows-1);
		m_private_obj->setColumnCells(i, 0, rows-1, m_private_obj->columnCells(cols-i-1, 0, rows-1));
		m_private_obj->setColumnCells(cols-i-1, 0, rows-1, temp);
	}
	m_private_obj->blockChangeSignals(false);
	m_private_obj->emitDataChanged(0, 0, rows-1, cols-1);
}

void MatrixMirrorHorizontallyCmd::undo()
{
	redo();
}
///////////////////////////////////////////////////////////////////////////
// end of class MatrixMirrorHorizontallyCmd
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// class MatrixMirrorVerticallyCmd
///////////////////////////////////////////////////////////////////////////
MatrixMirrorVerticallyCmd::MatrixMirrorVerticallyCmd( Matrix::Private * private_obj, QUndoCommand * parent)
 : QUndoCommand( parent ), m_private_obj(private_obj)
{
	setText(QObject::tr("%1: mirror vertically").arg(m_private_obj->name()));
}

MatrixMirrorVerticallyCmd::~MatrixMirrorVerticallyCmd()
{
}

void MatrixMirrorVerticallyCmd::redo()
{
	int rows = m_private_obj->rowCount();
	int cols = m_private_obj->columnCount();
	int middle = rows/2;
	m_private_obj->blockChangeSignals(true);
	for(int i = 0; i<middle; i++)
	{
		QVector<double> temp = m_private_obj->rowCells(i, 0, cols-1);
		m_private_obj->setRowCells(i, 0, cols-1, m_private_obj->rowCells(rows-i-1, 0, cols-1));
		m_private_obj->setRowCells(rows-i-1, 0, cols-1, temp);
	}
	m_private_obj->blockChangeSignals(false);
	m_private_obj->emitDataChanged(0, 0, rows-1, cols-1);
}

void MatrixMirrorVerticallyCmd::undo()
{
	redo();
}
///////////////////////////////////////////////////////////////////////////
// end of class MatrixMirrorVerticallyCmd
///////////////////////////////////////////////////////////////////////////

