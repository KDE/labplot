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

///////////////////////////////////////////////////////////////////////////
// class ColumnSetModeCmd
///////////////////////////////////////////////////////////////////////////
ColumnSetModeCmd::ColumnSetModeCmd(Column::Private * col, SciDAVis::ColumnMode mode, QUndoCommand * parent )
: QUndoCommand( parent ), m_col(col), m_mode(mode)
{
	setText(QObject::tr("%1: change column type").arg(col->name()));
	m_undone = false;
	m_executed = false;
}

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

void ColumnSetModeCmd::undo()
{
	// reset to old values
	m_col->replaceModeData(m_old_mode, m_old_type, m_old_data, m_old_in_filter, m_old_out_filter, m_old_validity);

	m_undone = true;
}

///////////////////////////////////////////////////////////////////////////
// end of class ColumnSetModeCmd
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// class ColumnFullCopyCmd
///////////////////////////////////////////////////////////////////////////
ColumnFullCopyCmd::ColumnFullCopyCmd(Column::Private * col, const AbstractColumn * src, QUndoCommand * parent )
: QUndoCommand( parent ), m_col(col), m_src(src), m_backup(0), m_backup_owner(0)
{
	setText(QObject::tr("%1: change cell value(s)").arg(col->name()));
}

ColumnFullCopyCmd::~ColumnFullCopyCmd()
{
	delete m_backup;
	delete m_backup_owner;
}

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

void ColumnFullCopyCmd::undo()
{
	// swap data + validity of orig. column and backup
	IntervalAttribute<bool> val_temp = m_col->validityAttribute();
	void * data_temp = m_col->dataPointer();
	m_col->replaceData(m_backup->dataPointer(), m_backup->validityAttribute());
	m_backup->replaceData(data_temp, val_temp);
}

///////////////////////////////////////////////////////////////////////////
// end of class ColumnFullCopyCmd
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// class ColumnPartialCopyCmd
///////////////////////////////////////////////////////////////////////////
ColumnPartialCopyCmd::ColumnPartialCopyCmd(Column::Private * col, const AbstractColumn * src, int src_start, int dest_start, int num_rows, QUndoCommand * parent )
: QUndoCommand( parent ), m_col(col), m_src(src), m_src_start(src_start), m_dest_start(dest_start), m_num_rows(num_rows), m_col_backup(0), m_src_backup(0), m_col_backup_owner(0), m_src_backup_owner(0)
{
	setText(QObject::tr("%1: change cell value(s)").arg(col->name()));
}

ColumnPartialCopyCmd::~ColumnPartialCopyCmd()
{
	delete m_src_backup;
	delete m_col_backup;
	delete m_src_backup_owner;
	delete m_col_backup_owner;
}

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

void ColumnPartialCopyCmd::undo()
{
	m_col->copy(m_col_backup, 0, m_dest_start, m_num_rows);
	m_col->resizeTo(m_old_row_count);
	m_col->replaceData(m_col->dataPointer(), m_old_validity);
}

///////////////////////////////////////////////////////////////////////////
// end of class ColumnPartialCopyCmd
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// class ColumnInsertEmptyRowsCmd
///////////////////////////////////////////////////////////////////////////
ColumnInsertEmptyRowsCmd::ColumnInsertEmptyRowsCmd(Column::Private * col, int before, int count, QUndoCommand * parent )
: QUndoCommand( parent ), m_col(col), m_before(before), m_count(count)
{
	setText(QObject::tr("%1: insert %2 row(s)").arg(col->name()).arg(count));
}

ColumnInsertEmptyRowsCmd::~ColumnInsertEmptyRowsCmd()
{
}

void ColumnInsertEmptyRowsCmd::redo()
{
	m_col->insertRows(m_before, m_count);
}

void ColumnInsertEmptyRowsCmd::undo()
{
	m_col->removeRows(m_before, m_count);
}

///////////////////////////////////////////////////////////////////////////
// end of class ColumnInsertEmptyRowsCmd
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
// class ColumnRemoveRowsCmd
///////////////////////////////////////////////////////////////////////////
ColumnRemoveRowsCmd::ColumnRemoveRowsCmd(Column::Private * col, int first, int count, QUndoCommand * parent )
: QUndoCommand( parent ), m_col(col), m_first(first), m_count(count), m_backup(0)
{
	setText(QObject::tr("%1: remove %2 row(s)").arg(col->name()).arg(count));
}

ColumnRemoveRowsCmd::~ColumnRemoveRowsCmd()
{
	delete m_backup;
	delete m_backup_owner;
}

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

void ColumnRemoveRowsCmd::undo()
{
	m_col->insertRows(m_first, m_count);
	m_col->copy(m_backup, 0, m_first, m_data_row_count);
	m_col->resizeTo(m_old_size);
	m_col->replaceMasking(m_masking);
	m_col->replaceFormulas(m_formulas);
}

///////////////////////////////////////////////////////////////////////////
// end of class ColumnRemoveRowsCmd
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// class ColumnSetPlotDesignationCmd
///////////////////////////////////////////////////////////////////////////
ColumnSetPlotDesignationCmd::ColumnSetPlotDesignationCmd( Column::Private * col, SciDAVis::PlotDesignation pd , QUndoCommand * parent )
: QUndoCommand( parent ), m_col(col), m_new_pd(pd)
{
	setText(QObject::tr("%1: set plot designation").arg(col->name()));
}

ColumnSetPlotDesignationCmd::~ColumnSetPlotDesignationCmd()
{
}

void ColumnSetPlotDesignationCmd::redo()
{
	m_old_pd = m_col->plotDesignation();
	m_col->setPlotDesignation(m_new_pd);
}

void ColumnSetPlotDesignationCmd::undo()
{
	m_col->setPlotDesignation(m_old_pd);
}

///////////////////////////////////////////////////////////////////////////
// end of class ColumnSetPlotDesignationCmd
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// class ColumnSetWidthCmd
///////////////////////////////////////////////////////////////////////////
ColumnSetWidthCmd::ColumnSetWidthCmd( Column::Private * col, int new_value , QUndoCommand * parent )
: QUndoCommand( parent ), m_col(col), m_other_value(new_value)
{
	setText(QObject::tr("%1: set width").arg(col->name()));
}

ColumnSetWidthCmd::~ColumnSetWidthCmd()
{
}

void ColumnSetWidthCmd::redo()
{
	int tmp = m_col->width();
	m_col->setWidth(m_other_value);
	m_other_value = tmp;
}

void ColumnSetWidthCmd::undo()
{
	redo();
}

///////////////////////////////////////////////////////////////////////////
// end of class ColumnSetWidthCmd
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// class ColumnClearCmd
///////////////////////////////////////////////////////////////////////////
ColumnClearCmd::ColumnClearCmd(Column::Private * col, QUndoCommand * parent )
: QUndoCommand( parent ), m_col(col)
{
	setText(QObject::tr("%1: clear column").arg(col->name()));
	m_empty_data = 0;
	m_undone = false;
}

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

void ColumnClearCmd::redo()
{
	if(!m_empty_data)
	{
		m_type = m_col->dataType();
		switch(m_type)
		{
			case SciDAVis::TypeDouble:
				m_empty_data = new QVector<double>();
				break;
			case SciDAVis::TypeQDateTime:
				m_empty_data = new QList<QDateTime>();
				break;
			case SciDAVis::TypeQString:
				m_empty_data = new QStringList();
				break;
		}
		m_data = m_col->dataPointer();
		m_validity = m_col->validityAttribute();
	}
	m_col->replaceData(m_empty_data, IntervalAttribute<bool>());
	m_undone = false;
}

void ColumnClearCmd::undo()
{
	m_col->replaceData(m_data, m_validity);
	m_undone = true;
}

///////////////////////////////////////////////////////////////////////////
// end of class ColumnClearCmd
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// class ColumnClearValidityCmd
///////////////////////////////////////////////////////////////////////////
ColumnClearValidityCmd::ColumnClearValidityCmd(Column::Private * col, QUndoCommand * parent )
: QUndoCommand( parent ), m_col(col)
{
	setText(QObject::tr("%1: mark all cells valid").arg(col->name()));
	m_copied = false;
}

ColumnClearValidityCmd::~ColumnClearValidityCmd()
{
}

void ColumnClearValidityCmd::redo()
{
	if(!m_copied)
	{
		m_validity = m_col->validityAttribute();
		m_copied = true;
	}
	m_col->clearValidity();
}

void ColumnClearValidityCmd::undo()
{
	m_col->replaceData(m_col->dataPointer(), m_validity);
}

///////////////////////////////////////////////////////////////////////////
// end of class ColumnClearValidityCmd
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// class ColumnClearMasksCmd
///////////////////////////////////////////////////////////////////////////
ColumnClearMasksCmd::ColumnClearMasksCmd(Column::Private * col, QUndoCommand * parent )
: QUndoCommand( parent ), m_col(col)
{
	setText(QObject::tr("%1: clear masks").arg(col->name()));
	m_copied = false;
}

ColumnClearMasksCmd::~ColumnClearMasksCmd()
{
}

void ColumnClearMasksCmd::redo()
{
	if(!m_copied)
	{
		m_masking = m_col->maskingAttribute();
		m_copied = true;
	}
	m_col->clearMasks();
}

void ColumnClearMasksCmd::undo()
{
	m_col->replaceMasking(m_masking);
}

///////////////////////////////////////////////////////////////////////////
// end of class ColumnClearMasksCmd
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// class ColumnSetInvalidCmd
///////////////////////////////////////////////////////////////////////////
ColumnSetInvalidCmd::ColumnSetInvalidCmd(Column::Private * col, Interval<int> interval, bool invalid, QUndoCommand * parent )
: QUndoCommand( parent ), m_col(col), m_interval(interval), m_invalid(invalid)
{
	if(invalid)
		setText(QObject::tr("%1: mark cells invalid").arg(col->name()));
	else
		setText(QObject::tr("%1: mark cells valid").arg(col->name()));
	m_copied = false;
}

ColumnSetInvalidCmd::~ColumnSetInvalidCmd()
{
}

void ColumnSetInvalidCmd::redo()
{
	if(!m_copied)
	{
		m_validity = m_col->validityAttribute();
		m_copied = true;
	}
	m_col->setInvalid(m_interval, m_invalid);
}

void ColumnSetInvalidCmd::undo()
{
	m_col->replaceData(m_col->dataPointer(), m_validity);
}

///////////////////////////////////////////////////////////////////////////
// end of class ColumnSetInvalidCmd
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// class ColumnSetMaskedCmd
///////////////////////////////////////////////////////////////////////////
ColumnSetMaskedCmd::ColumnSetMaskedCmd(Column::Private * col, Interval<int> interval, bool masked, QUndoCommand * parent )
: QUndoCommand( parent ), m_col(col), m_interval(interval), m_masked(masked)
{
	if(masked)
		setText(QObject::tr("%1: mask cells").arg(col->name()));
	else
		setText(QObject::tr("%1: unmask cells").arg(col->name()));
	m_copied = false;
}

ColumnSetMaskedCmd::~ColumnSetMaskedCmd()
{
}

void ColumnSetMaskedCmd::redo()
{
	if(!m_copied)
	{
		m_masking = m_col->maskingAttribute();
		m_copied = true;
	}
	m_col->setMasked(m_interval, m_masked);
}

void ColumnSetMaskedCmd::undo()
{
	m_col->replaceMasking(m_masking);
}

///////////////////////////////////////////////////////////////////////////
// end of class ColumnSetMaskedCmd
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// class ColumnSetFormulaCmd
///////////////////////////////////////////////////////////////////////////
ColumnSetFormulaCmd::ColumnSetFormulaCmd(Column::Private * col, Interval<int> interval, const QString& formula, QUndoCommand * parent )
: QUndoCommand( parent ), m_col(col), m_interval(interval), m_formula(formula)
{
	setText(QObject::tr("%1: set cell formula").arg(col->name()));
	m_copied = false;
}

ColumnSetFormulaCmd::~ColumnSetFormulaCmd()
{
}

void ColumnSetFormulaCmd::redo()
{
	if(!m_copied)
	{
		m_formulas = m_col->formulaAttribute();
		m_copied = true;
	}
	m_col->setFormula(m_interval, m_formula);
}

void ColumnSetFormulaCmd::undo()
{
	m_col->replaceFormulas(m_formulas);
}

///////////////////////////////////////////////////////////////////////////
// end of class ColumnSetFormulaCmd
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// class ColumnClearFormulasCmd
///////////////////////////////////////////////////////////////////////////
ColumnClearFormulasCmd::ColumnClearFormulasCmd(Column::Private * col, QUndoCommand * parent )
: QUndoCommand( parent ), m_col(col)
{
	setText(QObject::tr("%1: clear all formulas").arg(col->name()));
	m_copied = false;
}

ColumnClearFormulasCmd::~ColumnClearFormulasCmd()
{
}

void ColumnClearFormulasCmd::redo()
{
	if(!m_copied)
	{
		m_formulas = m_col->formulaAttribute();
		m_copied = true;
	}
	m_col->clearFormulas();
}

void ColumnClearFormulasCmd::undo()
{
	m_col->replaceFormulas(m_formulas);
}

///////////////////////////////////////////////////////////////////////////
// end of class ColumnClearFormulasCmd
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// class ColumnSetTextCmd
///////////////////////////////////////////////////////////////////////////
ColumnSetTextCmd::ColumnSetTextCmd(Column::Private * col, int row, const QString& new_value, QUndoCommand * parent )
: QUndoCommand( parent ), m_col(col), m_row(row), m_new_value(new_value)
{
	setText(QObject::tr("%1: set text for row %2").arg(col->name()).arg(row));
}

ColumnSetTextCmd::~ColumnSetTextCmd()
{
}

void ColumnSetTextCmd::redo()
{
	m_old_value = m_col->textAt(m_row);
	m_row_count = m_col->rowCount();
	m_validity = m_col->validityAttribute();
	m_col->setTextAt(m_row, m_new_value);
}

void ColumnSetTextCmd::undo()
{
	m_col->setTextAt(m_row, m_old_value);
	m_col->resizeTo(m_row_count);
	m_col->replaceData(m_col->dataPointer(), m_validity);
}

///////////////////////////////////////////////////////////////////////////
// end of class ColumnSetTextCmd
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// class ColumnSetValueCmd
///////////////////////////////////////////////////////////////////////////
ColumnSetValueCmd::ColumnSetValueCmd(Column::Private * col, int row, double new_value, QUndoCommand * parent )
: QUndoCommand( parent ), m_col(col), m_row(row), m_new_value(new_value)
{
	setText(QObject::tr("%1: set value for row %2").arg(col->name()).arg(row));
}

ColumnSetValueCmd::~ColumnSetValueCmd()
{
}

void ColumnSetValueCmd::redo()
{
	m_old_value = m_col->valueAt(m_row);
	m_row_count = m_col->rowCount();
	m_validity = m_col->validityAttribute();
	m_col->setValueAt(m_row, m_new_value);
}

void ColumnSetValueCmd::undo()
{
	m_col->setValueAt(m_row, m_old_value);
	m_col->resizeTo(m_row_count);
	m_col->replaceData(m_col->dataPointer(), m_validity);
}

///////////////////////////////////////////////////////////////////////////
// end of class ColumnSetValueCmd
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// class ColumnSetDateTimeCmd
///////////////////////////////////////////////////////////////////////////
ColumnSetDateTimeCmd::ColumnSetDateTimeCmd(Column::Private * col, int row, const QDateTime& new_value, QUndoCommand * parent )
: QUndoCommand( parent ), m_col(col), m_row(row), m_new_value(new_value)
{
	setText(QObject::tr("%1: set value for row %2").arg(col->name()).arg(row));
}

ColumnSetDateTimeCmd::~ColumnSetDateTimeCmd()
{
}

void ColumnSetDateTimeCmd::redo()
{
	m_old_value = m_col->dateTimeAt(m_row);
	m_row_count = m_col->rowCount();
	m_validity = m_col->validityAttribute();
	m_col->setDateTimeAt(m_row, m_new_value);
}

void ColumnSetDateTimeCmd::undo()
{
	m_col->setDateTimeAt(m_row, m_old_value);
	m_col->resizeTo(m_row_count);
	m_col->replaceData(m_col->dataPointer(), m_validity);
}

///////////////////////////////////////////////////////////////////////////
// end of class ColumnSetDateTimeCmd
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// class ColumnReplaceTextsCmd
///////////////////////////////////////////////////////////////////////////
ColumnReplaceTextsCmd::ColumnReplaceTextsCmd(Column::Private * col, int first, const QStringList& new_values, QUndoCommand * parent )
 : QUndoCommand( parent ), m_col(col), m_first(first), m_new_values(new_values)
{
	setText(QObject::tr("%1: replace the texts for rows %2 to %3").arg(col->name()).arg(first).arg(first + new_values.count() -1));
	m_copied = false;
}

ColumnReplaceTextsCmd::~ColumnReplaceTextsCmd()
{
}

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

void ColumnReplaceTextsCmd::undo()
{
	m_col->replaceTexts(m_first, m_old_values);
	m_col->resizeTo(m_row_count);
	m_col->replaceData(m_col->dataPointer(), m_validity);
}

///////////////////////////////////////////////////////////////////////////
// end of class ColumnReplaceTextsCmd
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// class ColumnReplaceValuesCmd
///////////////////////////////////////////////////////////////////////////
ColumnReplaceValuesCmd::ColumnReplaceValuesCmd(Column::Private * col, int first, const QVector<double>& new_values, QUndoCommand * parent )
 : QUndoCommand( parent ), m_col(col), m_first(first), m_new_values(new_values)
{
	setText(QObject::tr("%1: replace the values for rows %2 to %3").arg(col->name()).arg(first).arg(first + new_values.count() -1));
	m_copied = false;
}

ColumnReplaceValuesCmd::~ColumnReplaceValuesCmd()
{
}

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

void ColumnReplaceValuesCmd::undo()
{
	m_col->replaceValues(m_first, m_old_values);
	m_col->resizeTo(m_row_count);
	m_col->replaceData(m_col->dataPointer(), m_validity);
}

///////////////////////////////////////////////////////////////////////////
// end of class ColumnReplaceValuesCmd
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// class ColumnReplaceDateTimesCmd
///////////////////////////////////////////////////////////////////////////
ColumnReplaceDateTimesCmd::ColumnReplaceDateTimesCmd(Column::Private * col, int first, const QList<QDateTime>& new_values, QUndoCommand * parent )
 : QUndoCommand( parent ), m_col(col), m_first(first), m_new_values(new_values)
{
	setText(QObject::tr("%1: replace the values for rows %2 to %3").arg(col->name()).arg(first).arg(first + new_values.count() -1));
	m_copied = false;
}

ColumnReplaceDateTimesCmd::~ColumnReplaceDateTimesCmd()
{
}

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

void ColumnReplaceDateTimesCmd::undo()
{
	m_col->replaceDateTimes(m_first, m_old_values);
	m_col->replaceData(m_col->dataPointer(), m_validity);
	m_col->resizeTo(m_row_count);
}

///////////////////////////////////////////////////////////////////////////
// end of class ColumnReplaceDateTimesCmd
///////////////////////////////////////////////////////////////////////////



