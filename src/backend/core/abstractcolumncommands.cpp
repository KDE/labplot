/***************************************************************************
    File                 : abstractcolumncommands.cpp
    Project              : LabPlot
    Description          : Commands to be called by AbstractColumn to modify AbstractColumnPrivate
    --------------------------------------------------------------------
    Copyright            : (C) 2007-2009 Tilman Benkert (thzs@gmx.net)
	Copyright            : (C) 2010 Knut Franke (knut.franke@gmx.de)

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

#include "abstractcolumncommands.h"
#include <KLocale>

/** ***************************************************************************
 * \class AbstractColumnClearMasksCmd
 * \brief Clear masking information
 ** ***************************************************************************/

/**
 * \var AbstractColumnClearMasksCmd::m_col
 * \brief The private column data to modify
 */

/**
 * \var AbstractColumnClearMasksCmd::m_masking
 * \brief The old masks
 */

/**
 * \var AbstractColumnClearMasksCmd::m_copied
 * \brief A status flag
 */

/**
 * \brief Ctor
 */
AbstractColumnClearMasksCmd::AbstractColumnClearMasksCmd(AbstractColumnPrivate* col, QUndoCommand* parent)
: QUndoCommand( parent ), m_col(col)
{
	setText(i18n("%1: clear masks", col->name()));
	m_copied = false;
}

/**
 * \brief Dtor
 */
AbstractColumnClearMasksCmd::~AbstractColumnClearMasksCmd()
{
}

/**
 * \brief Execute the command
 */
void AbstractColumnClearMasksCmd::redo()
{
	if(!m_copied)
	{
		m_masking = m_col->m_masking;
		m_copied = true;
	}
	m_col->m_masking.clear();
	emit m_col->owner()->dataChanged(m_col->owner());
}

/**
 * \brief Undo the command
 */
void AbstractColumnClearMasksCmd::undo()
{
	m_col->m_masking = m_masking;
	emit m_col->owner()->dataChanged(m_col->owner());
}

/** ***************************************************************************
 * \class AbstractColumnSetMaskedCmd
 * \brief Mark an interval of rows as masked
 ** ***************************************************************************/

/**
 * \var AbstractColumnSetMaskedCmd::m_col
 * \brief The private AbstractColumn data to modify
 */

/**
 * \var AbstractColumnSetMaskedCmd::m_interval
 * \brief The interval
 */

/**
 * \var AbstractColumnSetMaskedCmd::m_masked
 * \brief Mask/unmask flag
 */

/**
 * \var AbstractColumnSetMaskedCmd::m_masking
 * \brief Interval attribute backup
 */

/**
 * \var AbstractColumnSetMaskedCmd::m_copied
 * \brief A status flag
 */

/**
 * \brief Ctor
 */
AbstractColumnSetMaskedCmd::AbstractColumnSetMaskedCmd(AbstractColumnPrivate * col, const Interval<int>& interval, bool masked, QUndoCommand * parent )
: QUndoCommand( parent ), m_col(col), m_interval(interval), m_masked(masked)
{
	if(masked)
		setText(i18n("%1: mask cells", col->name()));
	else
		setText(i18n("%1: unmask cells", col->name()));
	m_copied = false;
}

/**
 * \brief Dtor
 */
AbstractColumnSetMaskedCmd::~AbstractColumnSetMaskedCmd()
{
}

/**
 * \brief Execute the command
 */
void AbstractColumnSetMaskedCmd::redo()
{
	if(!m_copied)
	{
		m_masking = m_col->m_masking;
		m_copied = true;
	}
	m_col->m_masking.setValue(m_interval, m_masked);
	emit m_col->owner()->dataChanged(m_col->owner());
}

/**
 * \brief Undo the command
 */
void AbstractColumnSetMaskedCmd::undo()
{
	m_col->m_masking = m_masking;
	emit m_col->owner()->dataChanged(m_col->owner());
}

/** ***************************************************************************
 * \class AbstractColumnInsertRowsCmd
 * \brief Insert empty rows into a column
 ** ***************************************************************************/

/**
 * \var AbstractColumnInsertRowsCmd::m_col
 * \brief Private object of AbstractColumn to be modified.
 */

/**
 * \var AbstractColumnInsertRowsCmd::m_before
 * \brief Row number before which to insert the new rows.
 */

/**
 * \var AbstractColumnInsertRowsCmd::m_count
 * \brief Number of rows to be inserted.
 */

/**
 * \brief Ctor
 */
AbstractColumnInsertRowsCmd::AbstractColumnInsertRowsCmd(AbstractColumn *col, int before,
		int count, QUndoCommand *parent) :
	QUndoCommand(parent),
	m_col(col->d),
	m_before(before),
	m_count(count) {
}

/**
 * \brief Dtor
 */
AbstractColumnInsertRowsCmd::~AbstractColumnInsertRowsCmd() {
}

void AbstractColumnInsertRowsCmd::redo() {
	m_col->m_masking.insertRows(m_before, m_count);
}

void AbstractColumnInsertRowsCmd::undo() {
	m_col->m_masking.removeRows(m_before, m_count);
}

/** ***************************************************************************
 * \class AbstractColumnRemoveRowsCmd
 * \brief Remove rows from a column
 *
 * See AbstractColumnInsertRowsCmd for a discussion of the design.
 ** ***************************************************************************/

/**
 * \var AbstractColumnRemoveRowsCmd::m_col
 * \brief Private object of AbstractColumn to be modified.
 */

/**
 * \var AbstractColumnRemoveRowsCmd::m_first
 * \brief First row number to be removed.
 */

/**
 * \var AbstractColumnRemoveRowsCmd::m_count
 * \brief Number of rows to be removed.
 */

/**
 * \brief Ctor
 */
AbstractColumnRemoveRowsCmd::AbstractColumnRemoveRowsCmd(AbstractColumn *col, int first,
		int count, QUndoCommand *parent) :
	QUndoCommand(parent),
	m_col(col->d),
	m_first(first),
	m_count(count) {
}

/**
 * \brief Dtor
 */
AbstractColumnRemoveRowsCmd::~AbstractColumnRemoveRowsCmd() {
}

void AbstractColumnRemoveRowsCmd::redo() {
	m_masking = m_col->m_masking;
	m_col->m_masking.removeRows(m_first, m_count);
}

void AbstractColumnRemoveRowsCmd::undo() {
	m_col->m_masking = m_masking;
}

