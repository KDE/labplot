/***************************************************************************
    File                 : columncommands.cpp
    Project              : AbstractColumn
    Description          : Commands to be called by Column to modify ColumnPrivate
    --------------------------------------------------------------------
    Copyright            : (C) 2007,2008 Tilman Benkert (thzs@gmx.net)
    Copyright            : (C) 2010 by Knut Franke (knut.franke@gmx.de)
    Copyright            : (C) 2009-2017 Alexander Semke (alexander.semke@web.de)
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

#include "columncommands.h"
#include "ColumnPrivate.h"
#include <KLocale>
#include <cmath>

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
ColumnSetModeCmd::ColumnSetModeCmd(ColumnPrivate* col, AbstractColumn::ColumnMode mode, QUndoCommand* parent)
	: QUndoCommand(parent), m_col(col), m_mode(mode) {
	setText(i18n("%1: change column type", col->name()));
	m_undone = false;
	m_executed = false;
}

/**
 * \brief Dtor
 */
ColumnSetModeCmd::~ColumnSetModeCmd() {
	if(m_undone) {
		if(m_new_data != m_old_data)
			switch (m_mode) {
			case AbstractColumn::Numeric:
				delete static_cast<QVector<double>*>(m_new_data);
				break;
			case AbstractColumn::Integer:
				delete static_cast<QVector<int>*>(m_new_data);
				break;
			case AbstractColumn::Text:
				delete static_cast<QVector<QString>*>(m_new_data);
				break;
			case AbstractColumn::DateTime:
			case AbstractColumn::Month:
			case AbstractColumn::Day:
				delete static_cast<QVector<QDateTime>*>(m_new_data);
				break;
			}
	} else {
		if(m_new_data != m_old_data)
			switch (m_old_mode) {
			case AbstractColumn::Numeric:
				delete static_cast<QVector<double>*>(m_old_data);
				break;
			case AbstractColumn::Integer:
				delete static_cast<QVector<int>*>(m_old_data);
				break;
			case AbstractColumn::Text:
				delete static_cast<QVector<QString>*>(m_old_data);
				break;
			case AbstractColumn::DateTime:
			case AbstractColumn::Month:
			case AbstractColumn::Day:
				delete static_cast<QVector<QDateTime>*>(m_old_data);
				break;
			}
	}
}

/**
 * \brief Execute the command
 */
void ColumnSetModeCmd::redo() {
	if(!m_executed) {
		// save old values
		m_old_mode = m_col->columnMode();
		m_old_data = m_col->data();
		m_old_in_filter = m_col->inputFilter();
		m_old_out_filter = m_col->outputFilter();

		// do the conversion
		m_col->setColumnMode(m_mode);

		// save new values
		m_new_data = m_col->data();
		m_new_in_filter = m_col->inputFilter();
		m_new_out_filter = m_col->outputFilter();
		m_executed = true;
	} else {
		// set to saved new values
		m_col->replaceModeData(m_mode, m_new_data, m_new_in_filter, m_new_out_filter);
	}
	m_undone = false;
}

/**
 * \brief Undo the command
 */
void ColumnSetModeCmd::undo() {
	// reset to old values
	m_col->replaceModeData(m_old_mode, m_old_data, m_old_in_filter, m_old_out_filter);

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
 * This is needed because a ColumnPrivate must have an owner. We want access
 * to the ColumnPrivate object to access its data pointer for fast data
 * replacement without too much copying.
 */

/**
 * \brief Ctor
 */
ColumnFullCopyCmd::ColumnFullCopyCmd(ColumnPrivate* col, const AbstractColumn* src, QUndoCommand* parent)
	: QUndoCommand(parent), m_col(col), m_src(src), m_backup(0), m_backup_owner(0) {
	setText(i18n("%1: change cell values", col->name()));
}

/**
 * \brief Dtor
 */
ColumnFullCopyCmd::~ColumnFullCopyCmd() {
	delete m_backup;
	delete m_backup_owner;
}

/**
 * \brief Execute the command
 */
void ColumnFullCopyCmd::redo() {
	if(m_backup == 0) {
		m_backup_owner = new Column("temp", m_src->columnMode());
		m_backup = new ColumnPrivate(m_backup_owner, m_src->columnMode());
		m_backup->copy(m_col);
		m_col->copy(m_src);
	} else {
		// swap data of orig. column and backup
		void * data_temp = m_col->data();
		m_col->replaceData(m_backup->data());
		m_backup->replaceData(data_temp);
	}
}

/**
 * \brief Undo the command
 */
void ColumnFullCopyCmd::undo() {
	// swap data of orig. column and backup
	void * data_temp = m_col->data();
	m_col->replaceData(m_backup->data());
	m_backup->replaceData(data_temp);
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
 * This is needed because a ColumnPrivate must have an owner and
 * we must have a ColumnPrivate object as backup.
 * Using a Column object as backup would lead to an inifinite loop.
 */

/**
 * \var ColumnPartialCopyCmd::m_src_backup_owner
 * \brief A dummy owner for the source backup column
 *
 * This is needed because a ColumnPrivate must have an owner and
 * we must have a ColumnPrivate object as backup.
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
 * \brief Ctor
 */
ColumnPartialCopyCmd::ColumnPartialCopyCmd(ColumnPrivate* col, const AbstractColumn* src, int src_start, int dest_start, int num_rows, QUndoCommand* parent)
	: QUndoCommand(parent), m_col(col), m_src(src), m_col_backup(0), m_src_backup(0), m_col_backup_owner(0), m_src_backup_owner(0), m_src_start(src_start), m_dest_start(dest_start), m_num_rows(num_rows) {
	setText(i18n("%1: change cell values", col->name()));
}

/**
 * \brief Dtor
 */
ColumnPartialCopyCmd::~ColumnPartialCopyCmd() {
	delete m_src_backup;
	delete m_col_backup;
	delete m_src_backup_owner;
	delete m_col_backup_owner;
}

/**
 * \brief Execute the command
 */
void ColumnPartialCopyCmd::redo() {
	if(m_src_backup == 0) {
		// copy the relevant rows of source and destination column into backup columns
		m_src_backup_owner = new Column("temp", m_col->columnMode());
		m_src_backup = new ColumnPrivate(m_src_backup_owner, m_col->columnMode());
		m_src_backup->copy(m_src, m_src_start, 0, m_num_rows);
		m_col_backup_owner = new Column("temp", m_col->columnMode());
		m_col_backup = new ColumnPrivate(m_col_backup_owner, m_col->columnMode());
		m_col_backup->copy(m_col, m_dest_start, 0, m_num_rows);
		m_old_row_count = m_col->rowCount();
	}
	m_col->copy(m_src_backup, 0, m_dest_start, m_num_rows);
}

/**
 * \brief Undo the command
 */
void ColumnPartialCopyCmd::undo() {
	m_col->copy(m_col_backup, 0, m_dest_start, m_num_rows);
	m_col->resizeTo(m_old_row_count);
	m_col->replaceData(m_col->data());
}

/** ***************************************************************************
 * \class ColumnInsertRowsCmd
 * \brief Insert empty rows
 ** ***************************************************************************/

/**
 * \var ColumnInsertRowsCmd::m_col
 * \brief The private column data to modify
 */

/**
 * \brief Ctor
 */
ColumnInsertRowsCmd::ColumnInsertRowsCmd(ColumnPrivate* col, int before, int count, QUndoCommand* parent)
	: QUndoCommand(parent), m_col(col), m_before(before), m_count(count) {
}

/**
 * \brief Execute the command
 */
void ColumnInsertRowsCmd::redo() {
	m_col->insertRows(m_before, m_count);
}

/**
 * \brief Undo the command
 */
void ColumnInsertRowsCmd::undo() {
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
 * This is needed because a ColumnPrivate must have an owner. We want access
 * to the ColumnPrivate object to access its data pointer for fast data
 * replacement without too much copying.
 */

/**
 * \var ColumnRemoveRowsCmd::m_formulas
 * \brief Backup of the formula attribute
 */

/**
 * \brief Ctor
 */
ColumnRemoveRowsCmd::ColumnRemoveRowsCmd(ColumnPrivate* col, int first, int count, QUndoCommand* parent)
	: QUndoCommand(parent), m_col(col), m_first(first), m_count(count), m_backup(0) {
}

/**
 * \brief Dtor
 */
ColumnRemoveRowsCmd::~ColumnRemoveRowsCmd() {
	delete m_backup;
	delete m_backup_owner;
}

/**
 * \brief Execute the command
 */
void ColumnRemoveRowsCmd::redo() {
	if(m_backup == 0) {
		if(m_first >= m_col->rowCount())
			m_data_row_count = 0;
		else if(m_first + m_count > m_col->rowCount())
			m_data_row_count = m_col->rowCount() - m_first;
		else
			m_data_row_count = m_count;

		m_old_size = m_col->rowCount();
		m_backup_owner = new Column("temp", m_col->columnMode());
		m_backup = new ColumnPrivate(m_backup_owner, m_col->columnMode());
		m_backup->copy(m_col, m_first, 0, m_data_row_count);
		m_formulas = m_col->formulaAttribute();
	}
	m_col->removeRows(m_first, m_count);
}

/**
 * \brief Undo the command
 */
void ColumnRemoveRowsCmd::undo() {
	m_col->insertRows(m_first, m_count);
	m_col->copy(m_backup, 0, m_first, m_data_row_count);
	m_col->resizeTo(m_old_size);
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
ColumnSetPlotDesignationCmd::ColumnSetPlotDesignationCmd(ColumnPrivate* col, AbstractColumn::PlotDesignation pd, QUndoCommand* parent)
	: QUndoCommand(parent), m_col(col), m_new_pd(pd) {
	setText(i18n("%1: set plot designation", col->name()));
}

/**
 * \brief Execute the command
 */
void ColumnSetPlotDesignationCmd::redo() {
	m_old_pd = m_col->plotDesignation();
	m_col->setPlotDesignation(m_new_pd);
}

/**
 * \brief Undo the command
 */
void ColumnSetPlotDesignationCmd::undo() {
	m_col->setPlotDesignation(m_old_pd);
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
 * \var ColumnClearCmd::m_data
 * \brief Pointer to the old data pointer
 */

/**
 * \var ColumnClearCmd::m_empty_data
 * \brief Pointer to an empty data vector
 */

/**
 * \var ColumnClearCmd::m_undone
 * \brief Status flag
 */

/**
 * \brief Ctor
 */
ColumnClearCmd::ColumnClearCmd(ColumnPrivate* col, QUndoCommand* parent)
	: QUndoCommand(parent), m_col(col) {
	setText(i18n("%1: clear column", col->name()));
	m_empty_data = 0;
	m_data = 0;
	m_undone = false;
}

/**
 * \brief Dtor
 */
ColumnClearCmd::~ColumnClearCmd() {
	if(m_undone) {
		if (!m_empty_data) return;
		switch(m_col->columnMode()) {
		case AbstractColumn::Numeric:
			delete static_cast<QVector<double>*>(m_empty_data);
			break;
		case AbstractColumn::Integer:
			delete static_cast<QVector<int>*>(m_empty_data);
			break;
		case AbstractColumn::Text:
			delete static_cast<QVector<QString>*>(m_empty_data);
			break;
		case AbstractColumn::DateTime:
		case AbstractColumn::Month:
		case AbstractColumn::Day:
			delete static_cast<QVector<QDateTime>*>(m_empty_data);
			break;
		}
	} else {
		if (!m_data) return;
		switch(m_col->columnMode()) {
		case AbstractColumn::Numeric:
			delete static_cast<QVector<double>*>(m_data);
			break;
		case AbstractColumn::Integer:
			delete static_cast<QVector<int>*>(m_data);
			break;
		case AbstractColumn::Text:
			delete static_cast<QVector<QString>*>(m_data);
			break;
		case AbstractColumn::DateTime:
		case AbstractColumn::Month:
		case AbstractColumn::Day:
			delete static_cast<QVector<QDateTime>*>(m_data);
			break;
		}
	}
}

/**
 * \brief Execute the command
 */
void ColumnClearCmd::redo() {
	if(!m_empty_data) {
		const int rowCount = m_col->rowCount();
		switch(m_col->columnMode()) {
		case AbstractColumn::Numeric: {
			QVector<double>* vec = new QVector<double>(rowCount);
			m_empty_data = vec;
			for (int i = 0; i < rowCount; ++i)
				vec->operator[](i) = NAN;
			break;
		}
		case AbstractColumn::Integer: {
			QVector<int>* vec = new QVector<int>(rowCount);
			m_empty_data = vec;
			for (int i = 0; i < rowCount; ++i)
				vec->operator[](i) = 0;
			break;
		}
		case AbstractColumn::DateTime:
		case AbstractColumn::Month:
		case AbstractColumn::Day:
			m_empty_data = new QVector<QDateTime>();
			for (int i = 0; i < rowCount; ++i)
				static_cast< QVector<QDateTime>*>(m_empty_data)->append(QDateTime());
			break;
		case AbstractColumn::Text:
			m_empty_data = new QVector<QString>();
			for (int i = 0; i < rowCount; ++i)
				static_cast<QVector<QString>*>(m_empty_data)->append(QString());
			break;
		}
		m_data = m_col->data();
	}
	m_col->replaceData(m_empty_data);
	m_undone = false;
}

/**
 * \brief Undo the command
 */
void ColumnClearCmd::undo() {
	m_col->replaceData(m_data);
	m_undone = true;
}


/** ***************************************************************************
 * \class ColumSetGlobalFormulaCmd
 * \brief Set the formula for the entire column (global formula)
 ** ***************************************************************************/
ColumnSetGlobalFormulaCmd::ColumnSetGlobalFormulaCmd(ColumnPrivate* col, const QString& formula, const QStringList& variableNames, const QStringList& variableColumns)
	: QUndoCommand(), m_col(col), m_newFormula(formula), m_newVariableNames(variableNames), m_newVariableColumnPathes(variableColumns), m_copied(false) {
	setText(i18n("%1: set formula", col->name()));
}

void ColumnSetGlobalFormulaCmd::redo() {
	if(!m_copied) {
		m_formula = m_col->formula();
		m_variableNames = m_col->formulaVariableNames();
		m_variableColumnPathes = m_col->formulaVariableColumnPathes();
		m_copied = true;
	}

	m_col->setFormula(m_newFormula, m_newVariableNames, m_newVariableColumnPathes);
}

void ColumnSetGlobalFormulaCmd::undo() {
	m_col->setFormula(m_formula, m_variableNames, m_variableColumnPathes);
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
ColumnSetFormulaCmd::ColumnSetFormulaCmd(ColumnPrivate* col, const Interval<int>& interval, const QString& formula, QUndoCommand* parent)
	: QUndoCommand(parent), m_col(col), m_interval(interval), m_newFormula(formula), m_copied(false) {
	setText(i18n("%1: set cell formula", col->name()));
}


void ColumnSetFormulaCmd::redo() {
	if(!m_copied) {
		m_formulas = m_col->formulaAttribute();
		m_copied = true;
	}

	m_col->setFormula(m_interval, m_newFormula);
}

void ColumnSetFormulaCmd::undo() {
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
ColumnClearFormulasCmd::ColumnClearFormulasCmd(ColumnPrivate* col, QUndoCommand* parent)
	: QUndoCommand(parent), m_col(col) {
	setText(i18n("%1: clear all formulas", col->name()));
	m_copied = false;
}

/**
 * \brief Execute the command
 */
void ColumnClearFormulasCmd::redo() {
	if(!m_copied) {
		m_formulas = m_col->formulaAttribute();
		m_copied = true;
	}
	m_col->clearFormulas();
}

/**
 * \brief Undo the command
 */
void ColumnClearFormulasCmd::undo() {
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
 * \brief Ctor
 */
ColumnSetTextCmd::ColumnSetTextCmd(ColumnPrivate* col, int row, const QString& new_value, QUndoCommand* parent)
	: QUndoCommand(parent), m_col(col), m_row(row), m_new_value(new_value) {
	setText(i18n("%1: set text for row %2", col->name(), row));
}

/**
 * \brief Execute the command
 */
void ColumnSetTextCmd::redo() {
	m_old_value = m_col->textAt(m_row);
	m_row_count = m_col->rowCount();
	m_col->setTextAt(m_row, m_new_value);
}

/**
 * \brief Undo the command
 */
void ColumnSetTextCmd::undo() {
	m_col->setTextAt(m_row, m_old_value);
	m_col->resizeTo(m_row_count);
	m_col->replaceData(m_col->data());
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
 * \brief Ctor
 */
ColumnSetValueCmd::ColumnSetValueCmd(ColumnPrivate* col, int row, double new_value, QUndoCommand* parent)
	: QUndoCommand(parent), m_col(col), m_row(row), m_new_value(new_value) {
	setText(i18n("%1: set value for row %2", col->name(), row));
}

/**
 * \brief Execute the command
 */
void ColumnSetValueCmd::redo() {
	m_old_value = m_col->valueAt(m_row);
	m_row_count = m_col->rowCount();
	m_col->setValueAt(m_row, m_new_value);
}

/**
 * \brief Undo the command
 */
void ColumnSetValueCmd::undo() {
	m_col->setValueAt(m_row, m_old_value);
	m_col->resizeTo(m_row_count);
	m_col->replaceData(m_col->data());
}

/** ***************************************************************************
 * \class ColumnSetIntegerCmd
 * \brief Set the value for a int cell
 ** ***************************************************************************/

ColumnSetIntegerCmd::ColumnSetIntegerCmd(ColumnPrivate* col, int row, int new_value, QUndoCommand* parent)
	: QUndoCommand(parent), m_col(col), m_row(row), m_new_value(new_value) {
		DEBUG("ColumnSetIntegerCmd::ColumnSetIntegerCmd()");
	setText(i18n("%1: set value for row %2", col->name(), row));
}

/**
 * \brief Execute the command
 */
void ColumnSetIntegerCmd::redo() {
	m_old_value = m_col->integerAt(m_row);
	m_row_count = m_col->rowCount();
	m_col->setIntegerAt(m_row, m_new_value);
}

/**
 * \brief Undo the command
 */
void ColumnSetIntegerCmd::undo() {
	m_col->setIntegerAt(m_row, m_old_value);
	m_col->resizeTo(m_row_count);
 	m_col->replaceData(m_col->data());
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
 * \brief Ctor
 */
ColumnSetDateTimeCmd::ColumnSetDateTimeCmd(ColumnPrivate* col, int row, const QDateTime& new_value, QUndoCommand* parent)
	: QUndoCommand(parent), m_col(col), m_row(row), m_new_value(new_value) {
	setText(i18n("%1: set value for row %2", col->name(), row));
}

/**
 * \brief Execute the command
 */
void ColumnSetDateTimeCmd::redo() {
	m_old_value = m_col->dateTimeAt(m_row);
	m_row_count = m_col->rowCount();
	m_col->setDateTimeAt(m_row, m_new_value);
}

/**
 * \brief Undo the command
 */
void ColumnSetDateTimeCmd::undo() {
	m_col->setDateTimeAt(m_row, m_old_value);
	m_col->resizeTo(m_row_count);
	m_col->replaceData(m_col->data());
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
 * \brief Ctor
 */
ColumnReplaceTextsCmd::ColumnReplaceTextsCmd(ColumnPrivate* col, int first, const QVector<QString>& new_values, QUndoCommand* parent)
	: QUndoCommand(parent), m_col(col), m_first(first), m_new_values(new_values), m_copied(false), m_row_count(0) {
	setText(i18n("%1: replace the texts for rows %2 to %3", col->name(), first, first + new_values.count() - 1));
}

/**
 * \brief Execute the command
 */
void ColumnReplaceTextsCmd::redo() {
	if(!m_copied) {
		m_old_values = static_cast<QVector<QString>*>(m_col->data())->mid(m_first, m_new_values.count());
		m_row_count = m_col->rowCount();
		m_copied = true;
	}
	m_col->replaceTexts(m_first, m_new_values);
}

/**
 * \brief Undo the command
 */
void ColumnReplaceTextsCmd::undo() {
	m_col->replaceTexts(m_first, m_old_values);
	m_col->resizeTo(m_row_count);
	m_col->replaceData(m_col->data());
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
 * \brief Ctor
 */
ColumnReplaceValuesCmd::ColumnReplaceValuesCmd(ColumnPrivate* col, int first, const QVector<double>& new_values, QUndoCommand* parent)
	: QUndoCommand(parent), m_col(col), m_first(first), m_new_values(new_values) {
	setText(i18n("%1: replace the values for rows %2 to %3", col->name(), first, first + new_values.count() -1));
	m_copied = false;
}

/**
 * \brief Execute the command
 */
void ColumnReplaceValuesCmd::redo() {
	if(!m_copied) {
		m_old_values = static_cast<QVector<double>*>(m_col->data())->mid(m_first, m_new_values.count());
		m_row_count = m_col->rowCount();
		m_copied = true;
	}
	m_col->replaceValues(m_first, m_new_values);
}

/**
 * \brief Undo the command
 */
void ColumnReplaceValuesCmd::undo() {
	m_col->replaceValues(m_first, m_old_values);
	m_col->resizeTo(m_row_count);
	m_col->replaceData(m_col->data());
}

/** ***************************************************************************
 * \class ColumnReplaceIntegersCmd
 * \brief Replace a range of integers in a int column
 ** ***************************************************************************/

ColumnReplaceIntegersCmd::ColumnReplaceIntegersCmd(ColumnPrivate* col, int first, const QVector<int>& new_values, QUndoCommand* parent)
	: QUndoCommand(parent), m_col(col), m_first(first), m_new_values(new_values), m_copied(false), m_row_count(0) {
	setText(i18n("%1: replace the values for rows %2 to %3", col->name(), first, first + new_values.count() -1));
}

/**
 * \brief Execute the command
 */
void ColumnReplaceIntegersCmd::redo() {
	if(!m_copied) {
		m_old_values = static_cast<QVector<int>*>(m_col->data())->mid(m_first, m_new_values.count());
		m_row_count = m_col->rowCount();
		m_copied = true;
	}
	m_col->replaceInteger(m_first, m_new_values);
}

/**
 * \brief Undo the command
 */
void ColumnReplaceIntegersCmd::undo() {
	m_col->replaceInteger(m_first, m_old_values);
	m_col->resizeTo(m_row_count);
	m_col->replaceData(m_col->data());
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
 * \brief Ctor
 */
ColumnReplaceDateTimesCmd::ColumnReplaceDateTimesCmd(ColumnPrivate* col, int first, const QVector<QDateTime>& new_values, QUndoCommand* parent)
	: QUndoCommand(parent), m_col(col), m_first(first), m_new_values(new_values) {
	setText(i18n("%1: replace the values for rows %2 to %3", col->name(), first, first + new_values.count() -1));
	m_copied = false;
}

/**
 * \brief Execute the command
 */
void ColumnReplaceDateTimesCmd::redo() {
	if(!m_copied) {
		m_old_values = static_cast<QVector<QDateTime>*>(m_col->data())->mid(m_first, m_new_values.count());
		m_row_count = m_col->rowCount();
		m_copied = true;
	}
	m_col->replaceDateTimes(m_first, m_new_values);
}

/**
 * \brief Undo the command
 */
void ColumnReplaceDateTimesCmd::undo() {
	m_col->replaceDateTimes(m_first, m_old_values);
	m_col->replaceData(m_col->data());
	m_col->resizeTo(m_row_count);
}

