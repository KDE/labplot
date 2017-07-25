/***************************************************************************
    File                 : matrixcommands.cpp
    Project              : LabPlot
    Description          : Commands used in Matrix (part of the undo/redo framework)
    --------------------------------------------------------------------
    Copyright            : (C) 2008 Tilman Benkert (thzs@gmx.net)
    Copyright            : (C) 2015 Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2017 Stefan Gerlach (stefan.gerlach@uni.kn)

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
#include "MatrixPrivate.h"
#include <KLocale>

//Insert columns
MatrixInsertColumnsCmd::MatrixInsertColumnsCmd( MatrixPrivate * private_obj, int before, int count, QUndoCommand * parent)
 : QUndoCommand( parent ), m_private_obj(private_obj), m_before(before), m_count(count)
{
	setText(i18np("%1: insert %2 column", "%1: insert %2 columns", m_private_obj->name(), m_count));
}

void MatrixInsertColumnsCmd::redo() {
	m_private_obj->insertColumns(m_before, m_count);
	emit m_private_obj->q->columnCountChanged(m_private_obj->columnCount);
}

void MatrixInsertColumnsCmd::undo() {
	m_private_obj->removeColumns(m_before, m_count);
	emit m_private_obj->q->columnCountChanged(m_private_obj->columnCount);
}

//Insert rows
MatrixInsertRowsCmd::MatrixInsertRowsCmd( MatrixPrivate * private_obj, int before, int count, QUndoCommand * parent)
 : QUndoCommand( parent ), m_private_obj(private_obj), m_before(before), m_count(count)
{
	setText(i18np("%1: insert %2 row", "%1: insert %2 rows", m_private_obj->name(), m_count));
}

void MatrixInsertRowsCmd::redo() {
	m_private_obj->insertRows(m_before, m_count);
	emit m_private_obj->q->rowCountChanged(m_private_obj->rowCount);
}

void MatrixInsertRowsCmd::undo() {
	m_private_obj->removeRows(m_before, m_count);
	emit m_private_obj->q->rowCountChanged(m_private_obj->rowCount);
}

//Remove columns
MatrixRemoveColumnsCmd::MatrixRemoveColumnsCmd(MatrixPrivate* private_obj, int first, int count, QUndoCommand* parent)
 : QUndoCommand(parent), m_private_obj(private_obj), m_first(first), m_count(count) {
	setText(i18np("%1: remove %2 column", "%1: remove %2 columns", m_private_obj->name(), m_count));
}

void MatrixRemoveColumnsCmd::redo() {
	if(m_backups.isEmpty()) {
		int last_row = m_private_obj->rowCount-1;
		//TODO: consider mode
		for (int i = 0; i < m_count; i++)
			m_backups.append(m_private_obj->columnCells<double>(m_first+i, 0, last_row));
	}
	m_private_obj->removeColumns(m_first, m_count);
	emit m_private_obj->q->columnCountChanged(m_private_obj->columnCount);
}

void MatrixRemoveColumnsCmd::undo() {
	m_private_obj->insertColumns(m_first, m_count);
	int last_row = m_private_obj->rowCount-1;
	//TODO: use memcopy to copy from the backup vector
	for (int i = 0; i < m_count; i++)
		m_private_obj->setColumnCells(m_first+i, 0, last_row, m_backups.at(i));

	emit m_private_obj->q->columnCountChanged(m_private_obj->columnCount);
}

//Remove rows
MatrixRemoveRowsCmd::MatrixRemoveRowsCmd(MatrixPrivate* private_obj, int first, int count, QUndoCommand* parent)
 : QUndoCommand(parent), m_private_obj(private_obj), m_first(first), m_count(count) {
	setText(i18np("%1: remove %2 row", "%1: remove %2 rows", m_private_obj->name(), m_count));
}

void MatrixRemoveRowsCmd::redo() {
	if(m_backups.isEmpty()) {
		int last_row = m_first+m_count-1;
		//TODO: consider mode
		for (int col = 0; col < m_private_obj->columnCount; col++)
			m_backups.append(m_private_obj->columnCells<double>(col, m_first, last_row));
	}
	m_private_obj->removeRows(m_first, m_count);
	emit m_private_obj->q->rowCountChanged(m_private_obj->rowCount);
}

void MatrixRemoveRowsCmd::undo() {
	m_private_obj->insertRows(m_first, m_count);
	int last_row = m_first+m_count-1;
	for (int col = 0; col < m_private_obj->columnCount; col++)
		m_private_obj->setColumnCells(col, m_first, last_row, m_backups.at(col));
	emit m_private_obj->q->rowCountChanged(m_private_obj->rowCount);
}

// clear matrix
MatrixClearCmd::MatrixClearCmd(MatrixPrivate* private_obj, QUndoCommand* parent)
 : QUndoCommand( parent ), m_private_obj(private_obj) {
	setText(i18n("%1: clear", m_private_obj->name()));
}

void MatrixClearCmd::redo() {
	if(m_backups.isEmpty()) {
		int last_row = m_private_obj->rowCount-1;
		//TODO: mode
		for (int i = 0; i < m_private_obj->columnCount; i++)
			m_backups.append(m_private_obj->columnCells<double>(i, 0, last_row));
	}
	for (int i = 0; i < m_private_obj->columnCount; i++)
		m_private_obj->clearColumn(i);
}

void MatrixClearCmd::undo() {
	int last_row = m_private_obj->rowCount-1;
	for (int i = 0; i < m_private_obj->columnCount; i++)
		m_private_obj->setColumnCells(i, 0, last_row, m_backups.at(i));
}

//clear column
MatrixClearColumnCmd::MatrixClearColumnCmd(MatrixPrivate* private_obj, int col, QUndoCommand* parent)
 : QUndoCommand(parent), m_private_obj(private_obj), m_col(col) {
	setText(i18n("%1: clear column %2", m_private_obj->name(), m_col+1));
}

void MatrixClearColumnCmd::redo() {
	//TODO: mode
	if(m_backup.isEmpty())
		m_backup = m_private_obj->columnCells<double>(m_col, 0, m_private_obj->rowCount-1);
	m_private_obj->clearColumn(m_col);
}

void MatrixClearColumnCmd::undo() {
	m_private_obj->setColumnCells(m_col, 0, m_private_obj->rowCount-1, m_backup);
}

//set cell value
MatrixSetCellValueCmd::MatrixSetCellValueCmd(MatrixPrivate* private_obj, int row, int col, double value, QUndoCommand* parent)
 : QUndoCommand(parent), m_private_obj(private_obj), m_row(row), m_col(col), m_value(value) {
	// remark: don't use many QString::arg() calls in ctors of commands that might be called often,
	// they use a lot of execution time
	setText(i18n("%1: set cell value", m_private_obj->name()));
}

void MatrixSetCellValueCmd::redo() {
	m_old_value = m_private_obj->cell<double>(m_row, m_col);
	m_private_obj->setCell(m_row, m_col, m_value);
}

void MatrixSetCellValueCmd::undo() {
	m_private_obj->setCell(m_row, m_col, m_old_value);
}

//set coordinates
MatrixSetCoordinatesCmd::MatrixSetCoordinatesCmd(MatrixPrivate* private_obj, double x1, double x2, double y1, double y2, QUndoCommand* parent)
 : QUndoCommand( parent ), m_private_obj(private_obj), m_new_x1(x1), m_new_x2(x2), m_new_y1(y1), m_new_y2(y2) {
	setText(i18n("%1: set matrix coordinates", m_private_obj->name()));
}

void MatrixSetCoordinatesCmd::redo() {
	m_old_x1 = m_private_obj->xStart;
	m_old_x2 = m_private_obj->xEnd;
	m_old_y1 = m_private_obj->yStart;
	m_old_y2 = m_private_obj->yEnd;
	m_private_obj->xStart = m_new_x1;
	m_private_obj->xEnd = m_new_x2;
	m_private_obj->yStart = m_new_y1;
	m_private_obj->yEnd = m_new_y2;
}

void MatrixSetCoordinatesCmd::undo() {
	m_private_obj->xStart = m_old_x1;
	m_private_obj->xEnd = m_old_x2;
	m_private_obj->yStart = m_old_y1;
	m_private_obj->yEnd = m_old_y2;
}


//set formula
MatrixSetFormulaCmd::MatrixSetFormulaCmd(MatrixPrivate * private_obj, QString formula)
	: m_private_obj(private_obj), m_other_formula(formula)
{
	setText(i18n("%1: set formula", m_private_obj->name()));
}

void MatrixSetFormulaCmd::redo() {
	QString tmp = m_private_obj->formula;
	m_private_obj->formula = m_other_formula;
	m_other_formula = tmp;
}

void MatrixSetFormulaCmd::undo() {
	redo();
}

//set column cells
//TODO: consider columnMode
MatrixSetColumnCellsCmd::MatrixSetColumnCellsCmd( MatrixPrivate * private_obj, int col, int first_row,
		int last_row, const QVector<double> & values, QUndoCommand * parent)
 : QUndoCommand( parent ), m_private_obj(private_obj), m_col(col), m_first_row(first_row),
 		m_last_row(last_row), m_values(values)
{
	setText(i18n("%1: set cell values", m_private_obj->name()));
}

void MatrixSetColumnCellsCmd::redo() {
	//TODO: mode
	if (m_old_values.isEmpty())
		m_old_values = m_private_obj->columnCells<double>(m_col, m_first_row, m_last_row);
	m_private_obj->setColumnCells(m_col, m_first_row, m_last_row, m_values);
}
void MatrixSetColumnCellsCmd::undo() {
	m_private_obj->setColumnCells(m_col, m_first_row, m_last_row, m_old_values);
}

//set row cells
MatrixSetRowCellsCmd::MatrixSetRowCellsCmd( MatrixPrivate * private_obj, int row, int first_column,
		int last_column, const QVector<double> & values, QUndoCommand * parent)
 : QUndoCommand( parent ), m_private_obj(private_obj), m_row(row), m_first_column(first_column),
 		m_last_column(last_column), m_values(values)
{
	setText(i18n("%1: set cell values", m_private_obj->name()));
}

void MatrixSetRowCellsCmd::redo() {
	//TODO: mode
	if (m_old_values.isEmpty())
		m_old_values = m_private_obj->rowCells<double>(m_row, m_first_column, m_last_column);
	m_private_obj->setRowCells(m_row, m_first_column, m_last_column, m_values);
}

void MatrixSetRowCellsCmd::undo() {
	m_private_obj->setRowCells(m_row, m_first_column, m_last_column, m_old_values);
}


//transpose matrix
MatrixTransposeCmd::MatrixTransposeCmd( MatrixPrivate * private_obj, QUndoCommand * parent)
 : QUndoCommand( parent ), m_private_obj(private_obj)
{
	setText(i18n("%1: transpose", m_private_obj->name()));
}

void MatrixTransposeCmd::redo() {
	int rows = m_private_obj->rowCount;
	int cols = m_private_obj->columnCount;
	int temp_size = qMax(rows, cols);
	m_private_obj->suppressDataChange = true;
	if (cols < rows)
		m_private_obj->insertColumns(cols, temp_size - cols);
	else if (cols > rows)
		m_private_obj->insertRows(rows, temp_size - rows);
	//TODO: mode
	for (int i = 1; i < temp_size; i++) {
		QVector<double> row = m_private_obj->rowCells<double>(i, 0, i-1);
		QVector<double> col = m_private_obj->columnCells<double>(i, 0, i-1);
		m_private_obj->setRowCells(i, 0, i-1, col);
		m_private_obj->setColumnCells(i, 0, i-1, row);
	}
	if (cols < rows)
		m_private_obj->removeRows(cols, temp_size - cols);
	else if (cols > rows)
		m_private_obj->removeColumns(rows, temp_size - rows);
	m_private_obj->suppressDataChange = false;
	m_private_obj->emitDataChanged(0, 0, m_private_obj->rowCount-1, m_private_obj->columnCount-1);
}

void MatrixTransposeCmd::undo() {
	redo();
}


//mirror horizontally
MatrixMirrorHorizontallyCmd::MatrixMirrorHorizontallyCmd(MatrixPrivate* private_obj, QUndoCommand* parent)
 : QUndoCommand( parent ), m_private_obj(private_obj) {
	setText(i18n("%1: mirror horizontally", m_private_obj->name()));
}

void MatrixMirrorHorizontallyCmd::redo() {
	int rows = m_private_obj->rowCount;
	int cols = m_private_obj->columnCount;
	int middle = cols/2;
	m_private_obj->suppressDataChange = true;
	//TODO: mode
	for (int i = 0; i<middle; i++) {
		QVector<double> temp = m_private_obj->columnCells<double>(i, 0, rows-1);
		m_private_obj->setColumnCells(i, 0, rows-1, m_private_obj->columnCells<double>(cols-i-1, 0, rows-1));
		m_private_obj->setColumnCells(cols-i-1, 0, rows-1, temp);
	}
	m_private_obj->suppressDataChange = false;
	m_private_obj->emitDataChanged(0, 0, rows-1, cols-1);
}

void MatrixMirrorHorizontallyCmd::undo() {
	redo();
}


//mirror vertically
MatrixMirrorVerticallyCmd::MatrixMirrorVerticallyCmd(MatrixPrivate* private_obj, QUndoCommand* parent)
 : QUndoCommand( parent ), m_private_obj(private_obj) {
	setText(i18n("%1: mirror vertically", m_private_obj->name()));
}

void MatrixMirrorVerticallyCmd::redo() {
	int rows = m_private_obj->rowCount;
	int cols = m_private_obj->columnCount;
	int middle = rows/2;
	m_private_obj->suppressDataChange = true;
	//TODO: mode
	for (int i = 0; i < middle; i++) {
		QVector<double> temp = m_private_obj->rowCells<double>(i, 0, cols-1);
		m_private_obj->setRowCells(i, 0, cols-1, m_private_obj->rowCells<double>(rows-i-1, 0, cols-1));
		m_private_obj->setRowCells(rows-i-1, 0, cols-1, temp);
	}
	m_private_obj->suppressDataChange = false;
	m_private_obj->emitDataChanged(0, 0, rows-1, cols-1);
}

void MatrixMirrorVerticallyCmd::undo() {
	redo();
}

//replace values
MatrixReplaceValuesCmd::MatrixReplaceValuesCmd(MatrixPrivate* private_obj, void* new_values, QUndoCommand* parent)
 : QUndoCommand(parent), m_private_obj(private_obj), m_new_values(new_values) {
	setText(i18n("%1: replace values", m_private_obj->name()));
}

void MatrixReplaceValuesCmd::redo() {
	m_old_values = m_private_obj->data;
	m_private_obj->data = m_new_values;
	m_private_obj->emitDataChanged(0, 0, m_private_obj->rowCount -1, m_private_obj->columnCount-1);
}

void MatrixReplaceValuesCmd::undo() {
	m_new_values = m_private_obj->data;
	m_private_obj->data = m_old_values;
	m_private_obj->emitDataChanged(0, 0, m_private_obj->rowCount -1, m_private_obj->columnCount-1);
}
