/***************************************************************************
    File                 : tablecommands.h
    Project              : SciDAVis
    Description          : Commands used in Table (part of the undo/redo framework)
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

#ifndef TABLE_COMMANDS_H
#define TABLE_COMMANDS_H

#include <QUndoCommand>
#include <QAbstractItemModel>
#include <QModelIndex>
#include <QItemSelection>
#include "core/column/Column.h"
#include "core/column/ColumnPrivate.h"
#include "table/Table.h"
#include "core/AbstractFilter.h"
#include "lib/IntervalAttribute.h"

///////////////////////////////////////////////////////////////////////////
// class TableInsertColumnsCmd
///////////////////////////////////////////////////////////////////////////
//! Insert columns
/**
 * The number of inserted columns is cols.size().
 */
class TableInsertColumnsCmd : public QUndoCommand
{
public:
	TableInsertColumnsCmd( Table::Private * private_obj, int before, QList<Column*> cols, QUndoCommand * parent = 0 );
	~TableInsertColumnsCmd();

	virtual void redo();
	virtual void undo();

private:
	//! The private object to modify
	Table::Private * m_private_obj;
	//! Column to insert before
	int m_before;
	//! The new columns
	QList<Column*> m_cols;
	//! Row count before the command
	int m_rows_before;

};

///////////////////////////////////////////////////////////////////////////
// end of class TableInsertColumnsCmd
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// class TableSetNumberOfRowsCmd
///////////////////////////////////////////////////////////////////////////
//! Set the number of rows in the table
class TableSetNumberOfRowsCmd : public QUndoCommand
{
public:
	TableSetNumberOfRowsCmd( Table::Private * private_obj, int rows, QUndoCommand * parent = 0 );
	~TableSetNumberOfRowsCmd();

	virtual void redo();
	virtual void undo();

private:
	//! The private object to modify
	Table::Private * m_private_obj;
	//! Number of rows
	int m_rows;
	//! Number of rows before
	int m_old_rows;
};

///////////////////////////////////////////////////////////////////////////
// end of class TableSetNumberOfRowsCmd
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// class TableRemoveColumnsCmd
///////////////////////////////////////////////////////////////////////////
//! Remove columns
class TableRemoveColumnsCmd : public QUndoCommand
{
public:
	TableRemoveColumnsCmd( Table::Private * private_obj, int first, int count, QList<Column*> cols, QUndoCommand * parent = 0 );
	~TableRemoveColumnsCmd();

	virtual void redo();
	virtual void undo();

private:
	//! The private object to modify
	Table::Private * m_private_obj;
	//! The first column
	int m_first;
	//! The number of columns to be removed
	int m_count;
	//! The removed columns
	QList<Column*> m_old_cols;
};

///////////////////////////////////////////////////////////////////////////
// end of class TableRemoveColumnsCmd
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// class TableMoveColumnCmd
///////////////////////////////////////////////////////////////////////////
//! Move a column
class TableMoveColumnCmd : public QUndoCommand
{
public:
	TableMoveColumnCmd( Table::Private * private_obj, int from, int to, QUndoCommand * parent = 0 );
	~TableMoveColumnCmd();

	virtual void redo();
	virtual void undo();

private:
	//! The private object to modify
	Table::Private * m_private_obj;
	//! The old column index
	int m_from;
	//! The new column index
	int m_to;
};

///////////////////////////////////////////////////////////////////////////
// end of class TableMoveColumnCmd
///////////////////////////////////////////////////////////////////////////

#endif // ifndef TABLE_COMMANDS_H
