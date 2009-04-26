/***************************************************************************
    File                 : columncommands.cpp
    Project              : SciDAVis
    Description          : Commands to be called by Column to modify Column::Private
    --------------------------------------------------------------------
    Copyright            : (C) 2007,2008 Tilman Benkert (thzs*gmx.net)
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

#include "ColumnPrivate.h"
#include "columncommands.h"

/** ***************************************************************************
 * \class ColumnSetModeCmd
 * \brief Set the column mode 
 ** ***************************************************************************/

/**
 * \var ColumnSetModeCmd::m_col
 * \brief The private column data to modify
 */

/**
 * \var ColumnSetModeCmd::m_old_mode
 * \brief The previous mode
 */

/**
 * \var ColumnSetModeCmd::m_mode
 * \brief The new mode
 */

/**
 * \var ColumnSetModeCmd::m_old_type
 * \brief The old data type
 */

/**
 * \var ColumnSetModeCmd::m_new_type
 * \brief The new data type
 */

/**
 * \var ColumnSetModeCmd::m_old_data
 * \brief Pointer to old data
 */

/**
 * \var ColumnSetModeCmd::m_new_data
 * \brief Pointer to new data
 */

/**
 * \var ColumnSetModeCmd::m_new_in_filter
 * \brief The new input filter
 */

/**
 * \var ColumnSetModeCmd::m_new_out_filter
 * \brief The new output filter
 */

/**
 * \var ColumnSetModeCmd::m_old_in_filter
 * \brief The old input filter
 */

/**
 * \var ColumnSetModeCmd::m_old_out_filter
 * \brief The old output filter
 */

/**
 * \var ColumnSetModeCmd::m_old_validity
 * \brief The old validity information
 */

/**
 * \var ColumnSetModeCmd::m_new_validity
 * \brief The new validity information
 */

/**
 * \var ColumnSetModeCmd::m_undone
 * \brief Flag indicating whether this command has been undone (and not redone).
 */

/**
 * \var ColumnSetModeCmd::m_excecuted
 * \brief Flag indicating whether the command has been executed at least once.
 */

/**
 * \brief Ctor
 */
ColumnSetModeCmd::ColumnSetModeCmd(Column::Private * col, SciDAVis::ColumnMode mode, QUndoCommand * parent )
: QUndoCommand( parent ), m_col(col), m_mode(mode)
{
	setText(QObject::tr("%1: change column type").arg(col->name()));
	m_undone = false;
	m_executed = false;
}

/**
 * \brief Dtor
 */
ColumnSetModeCmd::~ColumnSetModeCmd()
{
	if(m_undone)
	{
		if(m_new_data != m_old_data)
		{
			if(m_new_type == SciDAVis::TypeDouble)
				delete static_cast< QVector<double>* >(m_new_data);
			else if(m_new_type == SciDAVis::TypeQString)
				delete static_cast< QStringList* >(m_new_data);
			else if(m_new_type == SciDAVis::TypeQDateTime)
				delete static_cast< QList<QDateTime>* >(m_new_data);
		}
	}
	else
	{
		if(m_new_data != m_old_data)
		{
			if(m_old_type == SciDAVis::TypeDouble)
				delete static_cast< QVector<double>* >(m_old_data);
			else if(m_old_type == SciDAVis::TypeQString)
				delete static_cast< QStringList* >(m_old_data);
			else if(m_old_type == SciDAVis::TypeQDateTime)
				delete static_cast< QList<QDateTime>* >(m_old_data);
		}
	}

}

/**
 * \brief Execute the command
 */
void ColumnSetModeCmd::redo()
{
	if(!m_executed)
	{
		// save old values
		m_old_mode = m_col->columnMode();	
		m_old_type = m_col->dataType();
		m_old_data = m_col->dataPointer();
		m_old_in_filter = m_col->inputFilter();
		m_old_out_filter = m_col->outputFilter();
		m_old_validity = m_col->validityAttribute();

		// do the conversion
		m_col->setColumnMode(m_mode);

		// save new values
		m_new_type = m_col->dataType();
		m_new_data = m_col->dataPointer();
		m_new_in_filter = m_col->inputFilter();
		m_new_out_filter = m_col->outputFilter();
		m_new_validity = m_col->validityAttribute();
		m_executed = true;
	}
	else
	{
		// set to saved new values
		m_col->replaceModeData(m_mode, m_new_type, m_new_data, m_new_in_filter, m_new_out_filter, m_new_validity);
	}
	m_undone = false;
}

/**
 * \brief Undo the command
 */
void ColumnSetModeCmd::undo()
{
	// reset to old values
	m_col->replaceModeData(m_old_mode, m_old_type, m_old_data, m_old_in_filter, m_old_out_filter, m_old_validity);

	m_undone = true;
}

/** ***************************************************************************
 * \class ColumnFullCopyCmd
 * \brief Copy a complete column 
 ** ***************************************************************************/

/**
 * \var ColumnFullCopyCmd::m_col
 * \brief The private column data to modify
 */

/**
 * \var ColumnFullCopyCmd::m_src
 * \brief The column to copy
 */

/**
 * \var ColumnFullCopyCmd::m_backup
 * \brief A backup column
 */

/**
 * \var ColumnFullCopyCmd::m_backup_owner
 * \brief A dummy owner for the backup column
 *
 * This is needed because a Column::Private must have an owner. We want access
 * to the Column::Private object to access its data pointer for fast data
 * replacement without too much copying.
 */

/**
 * \brief Ctor
 */
ColumnFullCopyCmd::ColumnFullCopyCmd(Column::Private * col, const AbstractColumn * src, QUndoCommand * parent )
: QUndoCommand( parent ), m_col(col), m_src(src), m_backup(0), m_backup_owner(0)
{
	setText(QObject::tr("%1: change cell value(s)").arg(col->name()));
}

/**
 * \brief Dtor
 */
ColumnFullCopyCmd::~ColumnFullCopyCmd()
{
	delete m_backup;
	delete m_backup_owner;
}

/**
 * \brief Execute the command
 */
void ColumnFullCopyCmd::redo()
{
	if(m_backup == 0)
	{
		m_backup_owner = new Column("temp", m_src->columnMode());
		m_backup = new Column::Private(m_backup_owner, m_src->columnMode()); 
		m_backup->copy(m_col);
		m_col->copy(m_src);
	}
	else
	{
		// swap data + validity of orig. column and backup
		IntervalAttribute<bool> val_temp = m_col->invalidIntervals();
		void * data_temp = m_col->dataPointer();
		m_col->replaceData(m_backup->dataPointer(), m_backup->validityAttribute());
		m_backup->replaceData(data_temp, val_temp);
	}
}

/**
 * \brief Undo the command
 */
void ColumnFullCopyCmd::undo()
{
	// swap data + validity of orig. column and backup
	IntervalAttribute<bool> val_temp = m_col->validityAttribute();
	void * data_temp = m_col->dataPointer();
	m_col->replaceData(m_backup->dataPointer(), m_backup->validityAttribute());
	m_backup->replaceData(data_temp, val_temp);
}

/** ***************************************************************************
 * \class ColumnPartialCopyCmd
 * \brief Copy parts of a column
 ** ***************************************************************************/

/**
 * \var ColumnPartialCopyCmd::m_col
 * \brief The private column data to modify
 */

/**
 * \var ColumnPartialCopyCmd::m_src
 * \brief The column to copy
 */

/**
 * \var ColumnPartialCopyCmd::m_col_backup
 * \brief A backup of the original column
 */

/**
 * \var ColumnPartialCopyCmd::m_src_backup
 * \brief A backup of the source column
 */

/**
 * \var ColumnPartialCopyCmd::m_col_backup_owner
 * \brief A dummy owner for the backup column
 *
 * This is needed because a Column::Private must have an owner and
 * we must have a Column::Private object as backup.
 * Using a Column object as backup would lead to an inifinite loop.
 */

/**
 * \var ColumnPartialCopyCmd::m_src_backup_owner
 * \brief A dummy owner for the source backup column
 *
 * This is needed because a Column::Private must have an owner and
 * we must have a Column::Private object as backup.
 * Using a Column object as backup would lead to an inifinite loop.
 */

/**
 * \var ColumnPartialCopyCmd::m_src_start
 * \brief Start index in source column
 */

/**
 * \var ColumnPartialCopyCmd::m_dest_start
 * \brief Start index in destination column
 */

/**
 * \var ColumnPartialCopyCmd::m_num_rows
 * \brief Number of rows to copy
 */

/**
 * \var ColumnPartialCopyCmd::m_old_row_count
 * \brief Previous number of rows in the destination column
 */

/**
 * \var ColumnPartialCopyCmd::m_old_validity
 * \brief The old validity information
 */

/**
 * \brief Ctor
 */
ColumnPartialCopyCmd::ColumnPartialCopyCmd(Column::Private * col, const AbstractColumn * src, int src_start, int dest_start, int num_rows, QUndoCommand * parent )
: QUndoCommand( parent ), m_col(col), m_src(src), m_src_start(src_start), m_dest_start(dest_start), m_num_rows(num_rows), m_col_backup(0), m_src_backup(0), m_col_backup_owner(0), m_src_backup_owner(0)
{
	setText(QObject::tr("%1: change cell value(s)").arg(col->name()));
}

/**
 * \brief Dtor
 */
ColumnPartialCopyCmd::~ColumnPartialCopyCmd()
{
	delete m_src_backup;
	delete m_col_backup;
	delete m_src_backup_owner;
	delete m_col_backup_owner;
}

/**
 * \brief Execute the command
 */
void ColumnPartialCopyCmd::redo()
{
	if(m_src_backup == 0)
	{
		// copy the relevant rows of source and destination column into backup columns
		m_src_backup_owner = new Column("temp", m_col->columnMode());
		m_src_backup = new Column::Private(m_src_backup_owner, m_col->columnMode());
		m_src_backup->copy(m_src, m_src_start, 0, m_num_rows);
		m_col_backup_owner = new Column("temp", m_col->columnMode());
		m_col_backup = new Column::Private(m_col_backup_owner, m_col->columnMode());
		m_col_backup->copy(m_col, m_dest_start, 0, m_num_rows);
		m_old_row_count = m_col->rowCount();
		m_old_validity = m_col->validityAttribute();
	}
	m_col->copy(m_src_backup, 0, m_dest_start, m_num_rows);
}

/**
 * \brief Undo the command
 */
void ColumnPartialCopyCmd::undo()
{
	m_col->copy(m_col_backup, 0, m_dest_start, m_num_rows);
	m_col->resizeTo(m_old_row_count);
	m_col->replaceData(m_col->dataPointer(), m_old_validity);
}

/** ***************************************************************************
 * \class ColumnInsertEmptyRowsCmd
 * \brief Insert empty rows 
 ** ***************************************************************************/

/**
 * \var ColumnInsertEmptyRowsCmd::m_col
 * \brief The private column data to modify
 */

/**
 * \var ColumnInsertEmptyRowsCmd::m_before
 * \brief Row to insert before
 */

/**
 * \var ColumnInsertEmptyRowsCmd::m_count
 * \brief Number of rows to insert
 */

/**
 * \brief Ctor
 */
ColumnInsertEmptyRowsCmd::ColumnInsertEmptyRowsCmd(Column::Private * col, int before, int count, QUndoCommand * parent )
: QUndoCommand( parent ), m_col(col), m_before(before), m_count(count)
{
	setText(QObject::tr("%1: insert %2 row(s)").arg(col->name()).arg(count));
}

/**
 * \brief Dtor
 */
ColumnInsertEmptyRowsCmd::~ColumnInsertEmptyRowsCmd()
{
}

/**
 * \brief Execute the command
 */
void ColumnInsertEmptyRowsCmd::redo()
{
	m_col->insertRows(m_before, m_count);
}

/**
 * \brief Undo the command
 */
void ColumnInsertEmptyRowsCmd::undo()
{
	m_col->removeRows(m_before, m_count);
}

/** ***************************************************************************
 * \class ColumnRemoveRowsCmd
 * \brief Remove consecutive rows from a column
 ** ***************************************************************************/

/**
 * \var ColumnRemoveRowsCmd::m_col
 * \brief The private column data to modify
 */

/**
 * \var ColumnRemoveRowsCmd::m_first
 * \brief The first row
 */

/**
 * \var ColumnRemoveRowsCmd::m_count
 * \brief The number of rows to be removed
 */

/**
 * \var ColumnRemoveRowsCmd::m_data_row_count
 * \brief Number of removed rows actually containing data
 */

/**
 * \var ColumnRemoveRowsCmd::m_old_size
 * \brief The number of rows before the removal
 */

/**
 * \var ColumnRemoveRowsCmd::m_backup
 * \brief Column saving the removed rows
 */

/**
 * \var ColumnRemoveRowsCmd::m_backup_owner
 * \brief A dummy owner for the backup column
 *
 * This is needed because a Column::Private must have an owner. We want access
 * to the Column::Private object to access its data pointer for fast data
 * replacement without too much copying.
 */

/**
 * \var ColumnRmeoveRowsCmd::m_masking
 * \brief Backup of the masking attribute
 */

/**
 * \var ColumnRemoveRowsCmd::m_formulas
 * \brief Backup of the formula attribute
 */

/**
 * \brief Ctor
 */
ColumnRemoveRowsCmd::ColumnRemoveRowsCmd(Column::Private * col, int first, int count, QUndoCommand * parent )
: QUndoCommand( parent ), m_col(col), m_first(first), m_count(count), m_backup(0)
{
	setText(QObject::tr("%1: remove %2 row(s)").arg(col->name()).arg(count));
}

/**
 * \brief Dtor
 */
ColumnRemoveRowsCmd::~ColumnRemoveRowsCmd()
{
	delete m_backup;
	delete m_backup_owner;
}

/**
 * \brief Execute the command
 */
void ColumnRemoveRowsCmd::redo()
{
	if(m_backup == 0)
	{
		if(m_first >= m_col->rowCount())
			m_data_row_count = 0;
		else if(m_first + m_count > m_col->rowCount()) 
			m_data_row_count = m_col->rowCount() - m_first;
		else
			m_data_row_count = m_count;

		m_old_size = m_col->rowCount();
		m_backup_owner = new Column("temp", m_col->columnMode());
		m_backup = new Column::Private(m_backup_owner, m_col->columnMode()); 
		m_backup->copy(m_col, m_first, 0, m_data_row_count);
		m_masking = m_col->maskingAttribute();
		m_formulas = m_col->formulaAttribute();
	}
	m_col->removeRows(m_first, m_count);
}

/**
 * \brief Undo the command
 */
void ColumnRemoveRowsCmd::undo()
{
	m_col->insertRows(m_first, m_count);
	m_col->copy(m_backup, 0, m_first, m_data_row_count);
	m_col->resizeTo(m_old_size);
	m_col->replaceMasking(m_masking);
	m_col->replaceFormulas(m_formulas);
}

/** ***************************************************************************
 * \class ColumnSetPlotDesignationCmd
 * \brief Sets a column's plot designation
 ** ***************************************************************************/

/**
 * \var ColumnSetPlotDesignationCmd::m_col
 * \brief The private column data to modify
 */

/**
 * \var ColumnSetPlotDesignation::m_new_pd
 * \brief New plot designation
 */

/**
 * \var ColumnSetPlotDesignation::m_old_pd
 * \brief Old plot designation
 */

/**
 * \brief Ctor
 */
ColumnSetPlotDesignationCmd::ColumnSetPlotDesignationCmd( Column::Private * col, SciDAVis::PlotDesignation pd , QUndoCommand * parent )
: QUndoCommand( parent ), m_col(col), m_new_pd(pd)
{
	setText(QObject::tr("%1: set plot designation").arg(col->name()));
}

/**
 * \brief Dtor
 */
ColumnSetPlotDesignationCmd::~ColumnSetPlotDesignationCmd()
{
}

/**
 * \brief Execute the command
 */
void ColumnSetPlotDesignationCmd::redo()
{
	m_old_pd = m_col->plotDesignation();
	m_col->setPlotDesignation(m_new_pd);
}

/**
 * \brief Undo the command
 */
void ColumnSetPlotDesignationCmd::undo()
{
	m_col->setPlotDesignation(m_old_pd);
}

/** ***************************************************************************
 * \class ColumnSetWidthCmd
 * \brief Sets a column's width
 ** ***************************************************************************/

/**
 * \var ColumnSetWidthCmd::m_col
 * \brief The private column data to modify
 */

/**
 * \brief Ctor
 */
ColumnSetWidthCmd::ColumnSetWidthCmd( Column::Private * col, int new_value , QUndoCommand * parent )
: QUndoCommand( parent ), m_col(col), m_other_value(new_value)
{
	setText(QObject::tr("%1: set width").arg(col->name()));
}

/**
 * \brief Dtor
 */
ColumnSetWidthCmd::~ColumnSetWidthCmd()
{
}

/**
 * \brief Execute the command
 */
void ColumnSetWidthCmd::redo()
{
	int tmp = m_col->width();
	m_col->setWidth(m_other_value);
	m_other_value = tmp;
}

/**
 * \brief Undo the command
 */
void ColumnSetWidthCmd::undo()
{
	redo();
}

/** ***************************************************************************
 * \class ColumnClearCmd
 * \brief Clear the column 
 ** ***************************************************************************/

/**
 * \var ColumnClearCmd::m_col
 * \brief The private column data to modify
 */

/**
 * \var ColumnClearCmd::m_type
 * \brief The column's data type
 */

/**
 * \var ColumnClearCmd::m_data
 * \brief Pointer to the old data pointer
 */

/**
 * \var ColumnClearCmd::m_empty_data
 * \brief Pointer to an empty data vector
 */

/**
 * \var ColumnClearCmd::m_validity
 * \brief The old validity
 */

/**
 * \var ColumnClearCmd::m_new_validity
 * \brief The new validity
 */

/**
 * \var ColumnClearCmd::m_undone
 * \brief Status flag
 */

/**
 * \brief Ctor
 */
ColumnClearCmd::ColumnClearCmd(Column::Private * col, QUndoCommand * parent )
: QUndoCommand( parent ), m_col(col)
{
	setText(QObject::tr("%1: clear column").arg(col->name()));
	m_empty_data = 0;
	m_undone = false;
}

/**
 * \brief Dtor
 */
ColumnClearCmd::~ColumnClearCmd()
{
	if(m_undone)
	{
		if(m_type == SciDAVis::TypeDouble)
			delete static_cast< QVector<double>* >(m_empty_data);
		else if(m_type == SciDAVis::TypeQString)
			delete static_cast< QStringList* >(m_empty_data);
		else if(m_type == SciDAVis::TypeQDateTime)
			delete static_cast< QList<QDateTime>* >(m_empty_data);
	}
	else
	{
		if(m_type == SciDAVis::TypeDouble)
			delete static_cast< QVector<double>* >(m_data);
		else if(m_type == SciDAVis::TypeQString)
			delete static_cast< QStringList* >(m_data);
		else if(m_type == SciDAVis::TypeQDateTime)
			delete static_cast< QList<QDateTime>* >(m_data);
	}
}

/**
 * \brief Execute the command
 */
void ColumnClearCmd::redo()
{
	if(!m_empty_data)
	{
		m_type = m_col->dataType();
		int rowCount = m_col->rowCount();
		switch(m_type)
		{
			case SciDAVis::TypeDouble:
				m_empty_data = new QVector<double>(rowCount);
				break;
			case SciDAVis::TypeQDateTime:
				m_empty_data = new QList<QDateTime>();
				for(int i=0; i<rowCount; i++)
					static_cast< QList<QDateTime> *>(m_empty_data)->append(QDateTime());
				break;
			case SciDAVis::TypeQString:
				m_empty_data = new QStringList();
				for(int i=0; i<rowCount; i++)
					static_cast< QStringList *>(m_empty_data)->append(QString());
				break;
		}
		m_data = m_col->dataPointer();
		m_validity = m_col->validityAttribute();
		if (m_col->rowCount() > 0)
			m_new_validity.setValue(Interval<int>(0, m_col->rowCount()-1));
	}
	m_col->replaceData(m_empty_data, m_new_validity);
	m_undone = false;
}

/**
 * \brief Undo the command
 */
void ColumnClearCmd::undo()
{
	m_col->replaceData(m_data, m_validity);
	m_undone = true;
}

/** ***************************************************************************
 * \class ColumnClearValidityCmd
 * \brief Clear validity information 
 ** ***************************************************************************/

/**
 * \var ColumnClearValidityCmd::m_col
 * \brief The private column data to modify
 */

/**
 * \var ColumnClearValidityCmd::m_validity
 * \brief The old validity
 */

/**
 * \var ColumnClearValidityCmd::m_copied
 * \brief A status flag
 */

/**
 * \brief Ctor
 */
ColumnClearValidityCmd::ColumnClearValidityCmd(Column::Private * col, QUndoCommand * parent )
: QUndoCommand( parent ), m_col(col)
{
	setText(QObject::tr("%1: mark all cells valid").arg(col->name()));
	m_copied = false;
}

/**
 * \brief Dtor
 */
ColumnClearValidityCmd::~ColumnClearValidityCmd()
{
}

/**
 * \brief Execute the command
 */
void ColumnClearValidityCmd::redo()
{
	if(!m_copied)
	{
		m_validity = m_col->validityAttribute();
		m_copied = true;
	}
	m_col->clearValidity();
}

/**
 * \brief Undo the command
 */
void ColumnClearValidityCmd::undo()
{
	m_col->replaceData(m_col->dataPointer(), m_validity);
}

/** ***************************************************************************
 * \class ColumnClearMasksCmd
 * \brief Clear masking information 
 ** ***************************************************************************/

/**
 * \var ColumnClearMasksCmd::m_col
 * \brief The private column data to modify
 */

/**
 * \var ColumnClearMasksCmd::m_masking
 * \brief The old masks
 */

/**
 * \var ColumnClearMasksCmd::m_copied
 * \brief A status flag
 */

/**
 * \brief Ctor
 */
ColumnClearMasksCmd::ColumnClearMasksCmd(Column::Private * col, QUndoCommand * parent )
: QUndoCommand( parent ), m_col(col)
{
	setText(QObject::tr("%1: clear masks").arg(col->name()));
	m_copied = false;
}

/**
 * \brief Dtor
 */
ColumnClearMasksCmd::~ColumnClearMasksCmd()
{
}

/**
 * \brief Execute the command
 */
void ColumnClearMasksCmd::redo()
{
	if(!m_copied)
	{
		m_masking = m_col->maskingAttribute();
		m_copied = true;
	}
	m_col->clearMasks();
}

/**
 * \brief Undo the command
 */
void ColumnClearMasksCmd::undo()
{
	m_col->replaceMasking(m_masking);
}

/** ***************************************************************************
 * \class ColumnSetInvalidCmd
 * \brief Mark an interval of rows as invalid 
 ** ***************************************************************************/

/**
 * \var ColumnSetInvalidCmd::m_col
 * \brief The private column data to modify
 */

/**
 * \var ColumnSetInvalidCmd::m_interval
 * \brief The interval
 */

/**
 * \var ColumnSetInvalidCmd::m_invalid
 * \brief Valid/invalid flag
 */

/**
 * \var ColumnSetInvalidCmd::m_validity
 * \brief Interval attribute backup
 */

/**
 * \var ColumnSetInvalidCmd::m_copied
 * \brief A status flag
 */

/**
 * \brief Ctor
 */
ColumnSetInvalidCmd::ColumnSetInvalidCmd(Column::Private * col, Interval<int> interval, bool invalid, QUndoCommand * parent )
: QUndoCommand( parent ), m_col(col), m_interval(interval), m_invalid(invalid)
{
	if(invalid)
		setText(QObject::tr("%1: mark cells invalid").arg(col->name()));
	else
		setText(QObject::tr("%1: mark cells valid").arg(col->name()));
	m_copied = false;
}

/**
 * \brief Dtor
 */
ColumnSetInvalidCmd::~ColumnSetInvalidCmd()
{
}

/**
 * \brief Execute the command
 */
void ColumnSetInvalidCmd::redo()
{
	if(!m_copied)
	{
		m_validity = m_col->validityAttribute();
		m_copied = true;
	}
	m_col->setInvalid(m_interval, m_invalid);
}

/**
 * \brief Undo the command
 */
void ColumnSetInvalidCmd::undo()
{
	m_col->replaceData(m_col->dataPointer(), m_validity);
}

/** ***************************************************************************
 * \class ColumnSetMaskedCmd
 * \brief Mark an interval of rows as masked
 ** ***************************************************************************/

/**
 * \var ColumnSetMaskedCmd::m_col
 * \brief The private column data to modify
 */

/**
 * \var ColumnSetMaskedCmd::m_interval
 * \brief The interval
 */

/**
 * \var ColumnSetMaskedCmd::m_masked
 * \brief Mask/unmask flag
 */

/**
 * \var ColumnSetMaskedCmd::m_masking
 * \brief Interval attribute backup
 */

/**
 * \var ColumnSetMaskedCmd::m_copied
 * \brief A status flag
 */

/**
 * \brief Ctor
 */
ColumnSetMaskedCmd::ColumnSetMaskedCmd(Column::Private * col, Interval<int> interval, bool masked, QUndoCommand * parent )
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
ColumnSetMaskedCmd::~ColumnSetMaskedCmd()
{
}

/**
 * \brief Execute the command
 */
void ColumnSetMaskedCmd::redo()
{
	if(!m_copied)
	{
		m_masking = m_col->maskingAttribute();
		m_copied = true;
	}
	m_col->setMasked(m_interval, m_masked);
}

/**
 * \brief Undo the command
 */
void ColumnSetMaskedCmd::undo()
{
	m_col->replaceMasking(m_masking);
}

/** ***************************************************************************
 * \class ColumSetFormulaCmd
 * \brief Set the formula for a given interval
 ** ***************************************************************************/

/**
 * \var ColumnSetFormulaCmd::m_col
 * \brief The private column data to modify
 */

/**
 * \var ColumnSetFormulaCmd::m_interval
 * \brief The interval
 */

/**
 * \var ColumnSetFormulaCmd::m_formula
 * \brief The new formula
 */

/**
 * \var ColumnSetFormulaCmd::m_formulas
 * \brief Interval attribute backup
 */

/**
 * \var ColumnSetFormulaCmd::m_copied
 * \brief A status flag
 */

/**
 * \brief Ctor
 */
ColumnSetFormulaCmd::ColumnSetFormulaCmd(Column::Private * col, Interval<int> interval, const QString& formula, QUndoCommand * parent )
: QUndoCommand( parent ), m_col(col), m_interval(interval), m_formula(formula)
{
	setText(QObject::tr("%1: set cell formula").arg(col->name()));
	m_copied = false;
}

/**
 * \brief Dtor
 */
ColumnSetFormulaCmd::~ColumnSetFormulaCmd()
{
}

/**
 * \brief Execute the command
 */
void ColumnSetFormulaCmd::redo()
{
	if(!m_copied)
	{
		m_formulas = m_col->formulaAttribute();
		m_copied = true;
	}
	m_col->setFormula(m_interval, m_formula);
}

/**
 * \brief Undo the command
 */
void ColumnSetFormulaCmd::undo()
{
	m_col->replaceFormulas(m_formulas);
}

/** ***************************************************************************
 * \class ColumnClearFormulasCmd
 * \brief Clear all associated formulas 
 ** ***************************************************************************/

/**
 * \var ColumClearFormulasCmd::m_col
 * \brief The private column data to modify
 */

/**
 * \var ColumnClearFormulasCmd::m_formulas
 * \brief The old formulas
 */

/**
 * \var ColumnClearFormulasCmd::m_copied
 * \brief A status flag
 */

/**
 * \brief Ctor
 */
ColumnClearFormulasCmd::ColumnClearFormulasCmd(Column::Private * col, QUndoCommand * parent )
: QUndoCommand( parent ), m_col(col)
{
	setText(QObject::tr("%1: clear all formulas").arg(col->name()));
	m_copied = false;
}

/**
 * \brief Dtor
 */
ColumnClearFormulasCmd::~ColumnClearFormulasCmd()
{
}

/**
 * \brief Execute the command
 */
void ColumnClearFormulasCmd::redo()
{
	if(!m_copied)
	{
		m_formulas = m_col->formulaAttribute();
		m_copied = true;
	}
	m_col->clearFormulas();
}

/**
 * \brief Undo the command
 */
void ColumnClearFormulasCmd::undo()
{
	m_col->replaceFormulas(m_formulas);
}

/** ***************************************************************************
 * \class ColumnSetTextCmd
 * \brief Set the text for a string cell 
 ** ***************************************************************************/

/**
 * \var ColumnSetTextCmd::m_col
 * \brief The private column data to modify
 */

/**
 * \var ColumnSetTextCmd::m_row
 * \brief The row to modify
 */

/**
 * \var ColumnSetTextCmd::m_new_value
 * \brief The new value
 */

/**
 * \var ColumnSetTextCmd::m_old_value
 * \brief The old value
 */

/**
 * \var ColumnSetTextCmd::m_row_count
 * \brief The old number of rows
 */

/**
 * \var ColumnSetTextCmd::m_validity
 * \brief The old validity
 */

/**
 * \brief Ctor
 */
ColumnSetTextCmd::ColumnSetTextCmd(Column::Private * col, int row, const QString& new_value, QUndoCommand * parent )
: QUndoCommand( parent ), m_col(col), m_row(row), m_new_value(new_value)
{
	setText(QObject::tr("%1: set text for row %2").arg(col->name()).arg(row));
}

/**
 * \brief Dtor
 */
ColumnSetTextCmd::~ColumnSetTextCmd()
{
}

/**
 * \brief Execute the command
 */
void ColumnSetTextCmd::redo()
{
	m_old_value = m_col->textAt(m_row);
	m_row_count = m_col->rowCount();
	m_validity = m_col->validityAttribute();
	m_col->setTextAt(m_row, m_new_value);
}

/**
 * \brief Undo the command
 */
void ColumnSetTextCmd::undo()
{
	m_col->setTextAt(m_row, m_old_value);
	m_col->resizeTo(m_row_count);
	m_col->replaceData(m_col->dataPointer(), m_validity);
}

/** ***************************************************************************
 * \class ColumnSetValueCmd
 * \brief Set the value for a double cell 
 ** ***************************************************************************/

/**
 * \var ColumnSetValueCmd::m_col
 * \brief The private column data to modify
 */

/**
 * \var ColumnSetValueCmd::m_row
 * \brief The row to modify
 */

/**
 * \var ColumnSetValueCmd::m_new_value
 * \brief The new value
 */

/**
 * \var ColumnSetValueCmd::m_old_value
 * \brief The old value
 */

/**
 * \var ColumnSetValueCmd::m_row_count
 * \brief The old number of rows
 */

/**
 * \var ColumnSetValueCmd::m_validity
 * \brief The old validity
 */

/**
 * \brief Ctor
 */
ColumnSetValueCmd::ColumnSetValueCmd(Column::Private * col, int row, double new_value, QUndoCommand * parent )
: QUndoCommand( parent ), m_col(col), m_row(row), m_new_value(new_value)
{
	setText(QObject::tr("%1: set value for row %2").arg(col->name()).arg(row));
}

/**
 * \brief Dtor
 */
ColumnSetValueCmd::~ColumnSetValueCmd()
{
}

/**
 * \brief Execute the command
 */
void ColumnSetValueCmd::redo()
{
	m_old_value = m_col->valueAt(m_row);
	m_row_count = m_col->rowCount();
	m_validity = m_col->validityAttribute();
	m_col->setValueAt(m_row, m_new_value);
}

/**
 * \brief Undo the command
 */
void ColumnSetValueCmd::undo()
{
	m_col->setValueAt(m_row, m_old_value);
	m_col->resizeTo(m_row_count);
	m_col->replaceData(m_col->dataPointer(), m_validity);
}

/** ***************************************************************************
 * \class ColumnSetDataTimeCmd
 * \brief Set the value of a date-time cell 
 ** ***************************************************************************/

/**
 * \var ColumnSetDateTimeCmd::m_col
 * \brief The private column data to modify
 */

/**
 * \var ColumnSetDateTimeCmd::m_row
 * \brief The row to modify
 */

/**
 * \var ColumnSetDateTimeCmd::m_new_value
 * \brief The new value
 */

/**
 * \var ColumnSetDateTimeCmd::m_old_value
 * \brief The old value
 */

/**
 * \var ColumnSetDateTimeCmd::m_row_count
 * \brief The old number of rows
 */

/**
 * \var ColumnSetDateTimeCmd::m_validity
 * \brief The old validity
 */

/**
 * \brief Ctor
 */
ColumnSetDateTimeCmd::ColumnSetDateTimeCmd(Column::Private * col, int row, const QDateTime& new_value, QUndoCommand * parent )
: QUndoCommand( parent ), m_col(col), m_row(row), m_new_value(new_value)
{
	setText(QObject::tr("%1: set value for row %2").arg(col->name()).arg(row));
}

/**
 * \brief Dtor
 */
ColumnSetDateTimeCmd::~ColumnSetDateTimeCmd()
{
}

/**
 * \brief Execute the command
 */
void ColumnSetDateTimeCmd::redo()
{
	m_old_value = m_col->dateTimeAt(m_row);
	m_row_count = m_col->rowCount();
	m_validity = m_col->validityAttribute();
	m_col->setDateTimeAt(m_row, m_new_value);
}

/**
 * \brief Undo the command
 */
void ColumnSetDateTimeCmd::undo()
{
	m_col->setDateTimeAt(m_row, m_old_value);
	m_col->resizeTo(m_row_count);
	m_col->replaceData(m_col->dataPointer(), m_validity);
}

/** ***************************************************************************
 * \class ColumnReplaceTextsCmd
 * \brief Replace a range of strings in a string column 
 ** ***************************************************************************/

/**
 * \var ColumnReplaceTextsCmd::m_col
 * \brief The private column data to modify
 */

/**
 * \var ColumnReplaceTextsCmd::m_first
 * \brief The first row to replace
 */

/**
 * \var ColumnReplaceTextsCmd::m_new_values
 * \brief The new values
 */

/**
 * \var ColumnReplaceTextsCmd::m_old_values
 * \brief The old values
 */

/**
 * \var ColumnReplaceTextsCmd::m_copied
 * \brief Status flag
 */

/**
 * \var ColumnReplaceTextsCmd::m_row_count
 * \brief The old number of rows
 */

/**
 * \var ColumnReplaceTextsCmd::m_validity
 * \brief The old validity
 */

/**
 * \brief Ctor
 */
ColumnReplaceTextsCmd::ColumnReplaceTextsCmd(Column::Private * col, int first, const QStringList& new_values, QUndoCommand * parent )
 : QUndoCommand( parent ), m_col(col), m_first(first), m_new_values(new_values)
{
	setText(QObject::tr("%1: replace the texts for rows %2 to %3").arg(col->name()).arg(first).arg(first + new_values.count() -1));
	m_copied = false;
}

/**
 * \brief Dtor
 */
ColumnReplaceTextsCmd::~ColumnReplaceTextsCmd()
{
}

/**
 * \brief Execute the command
 */
void ColumnReplaceTextsCmd::redo()
{
	if(!m_copied)
	{
		m_old_values = static_cast< QStringList* >(m_col->dataPointer())->mid(m_first, m_new_values.count());
		m_row_count = m_col->rowCount();
		m_validity = m_col->validityAttribute();
		m_copied = true;
	}
	m_col->replaceTexts(m_first, m_new_values);
}

/**
 * \brief Undo the command
 */
void ColumnReplaceTextsCmd::undo()
{
	m_col->replaceTexts(m_first, m_old_values);
	m_col->resizeTo(m_row_count);
	m_col->replaceData(m_col->dataPointer(), m_validity);
}

/** ***************************************************************************
 * \class ColumnReplaceValuesCmd
 * \brief Replace a range of doubles in a double column 
 ** ***************************************************************************/

/**
 * \var ColumnReplaceValuesCmd::m_col
 * \brief The private column data to modify
 */

/**
 * \var ColumnReplaceValuesCmd::m_first
 * \brief The first row to replace
 */

/**
 * \var ColumnReplaceValuesCmd::m_new_values
 * \brief The new values
 */

/**
 * \var ColumnReplaceValuesCmd::m_old_values
 * \brief The old values
 */

/**
 * \var ColumnReplaceValuesCmd::m_copied
 * \brief Status flag
 */

/**
 * \var ColumnReplaceValuesCmd::m_row_count
 * \brief The old number of rows
 */

/**
 * \var ColumnReplaceValuesCmd::m_validity
 * \brief The old validity
 */

/**
 * \brief Ctor
 */
ColumnReplaceValuesCmd::ColumnReplaceValuesCmd(Column::Private * col, int first, const QVector<double>& new_values, QUndoCommand * parent )
 : QUndoCommand( parent ), m_col(col), m_first(first), m_new_values(new_values)
{
	setText(QObject::tr("%1: replace the values for rows %2 to %3").arg(col->name()).arg(first).arg(first + new_values.count() -1));
	m_copied = false;
}

/**
 * \brief Dtor
 */
ColumnReplaceValuesCmd::~ColumnReplaceValuesCmd()
{
}

/**
 * \brief Execute the command
 */
void ColumnReplaceValuesCmd::redo()
{
	if(!m_copied)
	{
		m_old_values = static_cast< QVector<double>* >(m_col->dataPointer())->mid(m_first, m_new_values.count());
		m_row_count = m_col->rowCount();
		m_validity = m_col->validityAttribute();
		m_copied = true;
	}
	m_col->replaceValues(m_first, m_new_values);
}

/**
 * \brief Undo the command
 */
void ColumnReplaceValuesCmd::undo()
{
	m_col->replaceValues(m_first, m_old_values);
	m_col->resizeTo(m_row_count);
	m_col->replaceData(m_col->dataPointer(), m_validity);
}

/** ***************************************************************************
 * \class ColumnReplaceDateTimesCmd
 * \brief Replace a range of date-times in a date-time column 
 ** ***************************************************************************/

/**
 * \var ColumnReplaceDateTimesCmd::m_col
 * \brief The private column data to modify
 */

/**
 * \var ColumnReplaceDateTimesCmd::m_first
 * \brief The first row to replace
 */

/**
 * \var ColumnReplaceDateTimesCmd::m_new_values
 * \brief The new values
 */

/**
 * \var ColumnReplaceDateTimesCmd::m_old_values
 * \brief The old values
 */

/**
 * \var ColumnReplaceDateTimesCmd::m_copied
 * \brief Status flag
 */

/**
 * \var ColumnReplaceDateTimesCmd::m_row_count
 * \brief The old number of rows
 */

/**
 * \var ColumnReplaceDateTimesCmd::m_validity
 * \brief The old validity
 */

/**
 * \brief Ctor
 */
ColumnReplaceDateTimesCmd::ColumnReplaceDateTimesCmd(Column::Private * col, int first, const QList<QDateTime>& new_values, QUndoCommand * parent )
 : QUndoCommand( parent ), m_col(col), m_first(first), m_new_values(new_values)
{
	setText(QObject::tr("%1: replace the values for rows %2 to %3").arg(col->name()).arg(first).arg(first + new_values.count() -1));
	m_copied = false;
}

/**
 * \brief Dtor
 */
ColumnReplaceDateTimesCmd::~ColumnReplaceDateTimesCmd()
{
}

/**
 * \brief Execute the command
 */
void ColumnReplaceDateTimesCmd::redo()
{
	if(!m_copied)
	{
		m_old_values = static_cast< QList<QDateTime>* >(m_col->dataPointer())->mid(m_first, m_new_values.count());
		m_row_count = m_col->rowCount();
		m_validity = m_col->validityAttribute();
		m_copied = true;
	}
	m_col->replaceDateTimes(m_first, m_new_values);
}

/**
 * \brief Undo the command
 */
void ColumnReplaceDateTimesCmd::undo()
{
	m_col->replaceDateTimes(m_first, m_old_values);
	m_col->replaceData(m_col->dataPointer(), m_validity);
	m_col->resizeTo(m_row_count);
}

