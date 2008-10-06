/***************************************************************************
    File                 : tablecommands.cpp
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

#include "table/tablecommands.h"
#include "table/Table.h"
#include "core/column/Column.h"
#include "lib/Interval.h"
#include "core/datatypes/Double2StringFilter.h"
#include <QObject>
#include <QtDebug>

///////////////////////////////////////////////////////////////////////////
// class TableInsertColumnsCmd
///////////////////////////////////////////////////////////////////////////
TableInsertColumnsCmd::TableInsertColumnsCmd( Table::Private * private_obj, int before, QList<Column*> cols, QUndoCommand * parent)
 : QUndoCommand( parent ), m_private_obj(private_obj), m_before(before), m_cols(cols)
{
	setText(QObject::tr("%1: insert %2 column(s)").arg(m_private_obj->name()).arg(m_cols.size()));
}

TableInsertColumnsCmd::~TableInsertColumnsCmd()
{
}

void TableInsertColumnsCmd::redo()
{
	m_rows_before = m_private_obj->rowCount();
	m_private_obj->insertColumns(m_before, m_cols);
}

void TableInsertColumnsCmd::undo()
{
	m_private_obj->removeColumns(m_before, m_cols.size());
	m_private_obj->setRowCount(m_rows_before);
}

///////////////////////////////////////////////////////////////////////////
// end of class TableInsertColumnsCmd
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// class TableSetNumberOfRowsCmd
///////////////////////////////////////////////////////////////////////////
TableSetNumberOfRowsCmd::TableSetNumberOfRowsCmd( Table::Private * private_obj, int rows, QUndoCommand * parent )
 : QUndoCommand( parent ), m_private_obj(private_obj), m_rows(rows)
{
	setText(QObject::tr("%1: set the number of rows to %2").arg(m_private_obj->name()).arg(rows));
}

TableSetNumberOfRowsCmd::~TableSetNumberOfRowsCmd()
{
}

void TableSetNumberOfRowsCmd::redo()
{
	m_old_rows = m_private_obj->rowCount();
	m_private_obj->setRowCount(m_rows);
}

void TableSetNumberOfRowsCmd::undo()
{
	m_private_obj->setRowCount(m_old_rows);
}

///////////////////////////////////////////////////////////////////////////
// end of class TableSetNumberOfRowsCmd
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// class TableRemoveColumnsCmd
///////////////////////////////////////////////////////////////////////////
TableRemoveColumnsCmd::TableRemoveColumnsCmd( Table::Private * private_obj, int first, int count, QList<Column*> cols, QUndoCommand * parent )
 : QUndoCommand( parent ), m_private_obj(private_obj), m_first(first), m_count(count), m_old_cols(cols)
{
	setText(QObject::tr("%1: remove %2 column(s)").arg(m_private_obj->name()).arg(count));
}

TableRemoveColumnsCmd::~TableRemoveColumnsCmd()
{
}

void TableRemoveColumnsCmd::redo()
{
	m_private_obj->removeColumns(m_first, m_count);
}

void TableRemoveColumnsCmd::undo()
{
	m_private_obj->insertColumns(m_first, m_old_cols);
}

///////////////////////////////////////////////////////////////////////////
// end of class TableRemoveColumnsCmd
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// class TableMoveColumnCmd
///////////////////////////////////////////////////////////////////////////
TableMoveColumnCmd::TableMoveColumnCmd( Table::Private * private_obj, int from, int to, QUndoCommand * parent )
 : QUndoCommand( parent ), m_private_obj(private_obj), m_from(from), m_to(to)
{
	setText(QObject::tr("%1: move column %2 from position %3 to %4")
			.arg(m_private_obj->name())
			.arg(m_private_obj->column(from)->name())
			.arg(m_from+1).arg(m_to+1));
}

TableMoveColumnCmd::~TableMoveColumnCmd()
{
}

void TableMoveColumnCmd::redo()
{
	m_private_obj->moveColumn(m_from, m_to);
}

void TableMoveColumnCmd::undo()
{
	m_private_obj->moveColumn(m_to, m_from);
}

///////////////////////////////////////////////////////////////////////////
// end of class TableMoveColumnCmd
///////////////////////////////////////////////////////////////////////////

