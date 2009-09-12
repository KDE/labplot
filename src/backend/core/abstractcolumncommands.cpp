/***************************************************************************
    File                 : abstractcolumncommands.cpp
    Project              : SciDAVis/LabPlot
    Description          : Commands to be called by AbstractColumn to modify AbstractColumn::Private
    --------------------------------------------------------------------
    Copyright            : (C) 2007-2009 Tilman Benkert (thzs*gmx.net),
	                                      Knut Franke (knut.franke*gmx.de)
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

#include "abstractcolumncommands.h"

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
AbstractColumnClearMasksCmd::AbstractColumnClearMasksCmd(AbstractColumn::Private * col, QUndoCommand * parent )
: QUndoCommand( parent ), m_col(col)
{
	setText(QObject::tr("%1: clear masks").arg(col->name()));
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
		m_masking = m_col->masking();
		m_copied = true;
	}
	emit m_col->owner()->maskingAboutToChange(m_col->owner());
	m_col->masking().clear();
	emit m_col->owner()->maskingChanged(m_col->owner());
}

/**
 * \brief Undo the command
 */
void AbstractColumnClearMasksCmd::undo()
{
	emit m_col->owner()->maskingAboutToChange(m_col->owner());
	m_col->replaceMasking(m_masking);
	emit m_col->owner()->maskingChanged(m_col->owner());
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
AbstractColumnSetMaskedCmd::AbstractColumnSetMaskedCmd(AbstractColumn::Private * col, Interval<int> interval, bool masked, QUndoCommand * parent )
: QUndoCommand( parent ), m_col(col), m_interval(interval), m_masked(masked)
{
	if(masked)
		setText(QObject::tr("%1: mask cells").arg(col->name()));
	else
		setText(QObject::tr("%1: unmask cells").arg(col->name()));
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
		m_masking = m_col->masking();
		m_copied = true;
	}
	emit m_col->owner()->maskingAboutToChange(m_col->owner());
	m_col->masking().setValue(m_interval, m_masked);
	emit m_col->owner()->maskingChanged(m_col->owner());
}

/**
 * \brief Undo the command
 */
void AbstractColumnSetMaskedCmd::undo()
{
	emit m_col->owner()->maskingAboutToChange(m_col->owner());
	m_col->replaceMasking(m_masking);
	emit m_col->owner()->maskingChanged(m_col->owner());
}

/** ***************************************************************************
 * \class AbstractColumnInsertRowsCmd
 * \brief Insert empty rows into a column
 *
 * When inserting new rows, the following things should happen:
 *
 *   - emitting AbstractColumn::rowsAboutToBeInserted()
 *   - updating internal state of AbstractColumn
 *   - updating internal state of subclasses
 *   - emitting AbstractColumn::rowsInserted()
 *
 * Since this needs to happen on every undo/redo of the command, the following approach is taken:
 * AbstractColumnInsertRowsCmd::redo() is responsible for emmitting the signals and dispatching
 * the state updates to primaryRedo(). If a subclass of AbstractColumn needs to update its state
 * when rows are inserted, it must provide a subclass of AbstractColumnInsertRowsCmd which
 * reimplements primaryRedo(). The reimplementation must call the superclasses' version at some
 * point in order to allow all classes in the inheritance chain to update their internal state.
 * undo() and primaryUndo() work completely analogously. AbstractColumn::insertRows() must be also
 * reimplemented to execute an instance of the appropriate subclass instead of
 * AbstractColumnInsertRowsCmd.
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
	m_col(col->m_abstract_column_private),
	m_before(before),
	m_count(count) {
	setText(QObject::tr("%1: insert %2 row(s)").arg(col->name()).arg(count));
}

/**
 * \brief Dtor
 */
AbstractColumnInsertRowsCmd::~AbstractColumnInsertRowsCmd() {
}

/**
 * \brief Execute command - don't override in subclasses, use primaryRedo() instead.
 *
 */
void AbstractColumnInsertRowsCmd::redo() {
	emit m_col->owner()->rowsAboutToBeInserted(m_col->owner(), m_before, m_count);
	primaryRedo();
	emit m_col->owner()->rowsInserted(m_col->owner(), m_before, m_count);
}

/**
 * \brief Undo command - don't override in subclasses, use primaryUndo() instead.
 *
 */
void AbstractColumnInsertRowsCmd::undo() {
	emit m_col->owner()->rowsAboutToBeRemoved(m_col->owner(), m_before, m_count);
	primaryUndo();
	emit m_col->owner()->rowsRemoved(m_col->owner(), m_before, m_count);
}

void AbstractColumnInsertRowsCmd::primaryRedo() {
	m_col->masking().insertRows(m_before, m_count);
}

void AbstractColumnInsertRowsCmd::primaryUndo() {
	m_col->masking().removeRows(m_before, m_count);
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
	m_col(col->m_abstract_column_private),
	m_first(first),
	m_count(count) {
	setText(QObject::tr("%1: remove %2 row(s)").arg(col->name()).arg(count));
}

/**
 * \brief Dtor
 */
AbstractColumnRemoveRowsCmd::~AbstractColumnRemoveRowsCmd() {
}

/**
 * \brief Execute command - don't override in subclasses, use primaryRedo() instead.
 *
 */
void AbstractColumnRemoveRowsCmd::redo() {
	emit m_col->owner()->rowsAboutToBeRemoved(m_col->owner(), m_first, m_count);
	primaryRedo();
	emit m_col->owner()->rowsRemoved(m_col->owner(), m_first, m_count);
}

/**
 * \brief Undo command - don't override in subclasses, use primaryUndo() instead.
 *
 */
void AbstractColumnRemoveRowsCmd::undo() {
	emit m_col->owner()->rowsAboutToBeInserted(m_col->owner(), m_first, m_count);
	primaryUndo();
	emit m_col->owner()->rowsInserted(m_col->owner(), m_first, m_count);
}

void AbstractColumnRemoveRowsCmd::primaryRedo() {
	m_masking = m_col->masking();
	m_col->masking().removeRows(m_first, m_count);
}

void AbstractColumnRemoveRowsCmd::primaryUndo() {
	m_col->replaceMasking(m_masking);
}

