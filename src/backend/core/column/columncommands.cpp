/*
    File                 : columncommands.cpp
    Project              : AbstractColumn
    Description          : Commands to be called by Column to modify ColumnPrivate
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2007, 2008 Tilman Benkert <thzs@gmx.net>
    SPDX-FileCopyrightText: 2010 Knut Franke <knut.franke@gmx.de>
    SPDX-FileCopyrightText: 2009-2017 Alexander Semke <alexander.semke@web.de>
    SPDX-FileCopyrightText: 2017-2020 Stefan Gerlach <stefan.gerlach@uni.kn>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "columncommands.h"
#include "ColumnPrivate.h"
#include "backend/lib/macros.h"
#include <KLocalizedString>

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
 * \var ColumnSetModeCmd::m_executed
 * \brief Flag indicating whether the command has been executed at least once.
 */

/**
 * \brief Ctor
 */
ColumnSetModeCmd::ColumnSetModeCmd(ColumnPrivate* col, AbstractColumn::ColumnMode mode, QUndoCommand* parent)
	: QUndoCommand(parent), m_col(col), m_mode(mode) {
	setText(i18n("%1: change column type", col->name()));
}

/**
 * \brief Dtor
 */
ColumnSetModeCmd::~ColumnSetModeCmd() {
	if (m_undone) {
		if (m_new_data != m_old_data)
			switch (m_mode) {
			case AbstractColumn::ColumnMode::Double:
				delete static_cast<QVector<double>*>(m_new_data);
				break;
			case AbstractColumn::ColumnMode::Integer:
				delete static_cast<QVector<int>*>(m_new_data);
				break;
			case AbstractColumn::ColumnMode::BigInt:
				delete static_cast<QVector<qint64>*>(m_new_data);
				break;
			case AbstractColumn::ColumnMode::Text:
				delete static_cast<QVector<QString>*>(m_new_data);
				break;
			case AbstractColumn::ColumnMode::DateTime:
			case AbstractColumn::ColumnMode::Month:
			case AbstractColumn::ColumnMode::Day:
				delete static_cast<QVector<QDateTime>*>(m_new_data);
				break;
			}
	} else {
		if (m_new_data != m_old_data)
			switch (m_old_mode) {
			case AbstractColumn::ColumnMode::Double:
				delete static_cast<QVector<double>*>(m_old_data);
				break;
			case AbstractColumn::ColumnMode::Integer:
				delete static_cast<QVector<int>*>(m_old_data);
				break;
			case AbstractColumn::ColumnMode::BigInt:
				delete static_cast<QVector<qint64>*>(m_old_data);
				break;
			case AbstractColumn::ColumnMode::Text:
				delete static_cast<QVector<QString>*>(m_old_data);
				break;
			case AbstractColumn::ColumnMode::DateTime:
			case AbstractColumn::ColumnMode::Month:
			case AbstractColumn::ColumnMode::Day:
				delete static_cast<QVector<QDateTime>*>(m_old_data);
				break;
			}
	}
}

/**
 * \brief Execute the command
 */
void ColumnSetModeCmd::redo() {
	if (!m_executed) {
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
	: QUndoCommand(parent), m_col(col), m_src(src) {
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
	if (m_backup == nullptr) {
		m_backup_owner = new Column("temp", m_src->columnMode());
		m_backup = new ColumnPrivate(m_backup_owner, m_src->columnMode());
		m_backup->copy(m_col);
		m_col->copy(m_src);
	} else {
		// swap data of orig. column and backup
		void* data_temp = m_col->data();
		m_col->replaceData(m_backup->data());
		m_backup->replaceData(data_temp);
	}
}

/**
 * \brief Undo the command
 */
void ColumnFullCopyCmd::undo() {
	// swap data of orig. column and backup
	void* data_temp = m_col->data();
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
 * Using a Column object as backup would lead to an infinite loop.
 */

/**
 * \var ColumnPartialCopyCmd::m_src_backup_owner
 * \brief A dummy owner for the source backup column
 *
 * This is needed because a ColumnPrivate must have an owner and
 * we must have a ColumnPrivate object as backup.
 * Using a Column object as backup would lead to an infinite loop.
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
	: QUndoCommand(parent), m_col(col), m_src(src), m_src_start(src_start), m_dest_start(dest_start), m_num_rows(num_rows) {
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
	if (m_src_backup == nullptr) {
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
	: QUndoCommand(parent), m_col(col), m_first(first), m_count(count) {
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
	if (m_backup == nullptr) {
		if (m_first >= m_col->rowCount())
			m_data_row_count = 0;
		else if (m_first + m_count > m_col->rowCount())
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
}

/**
 * \brief Dtor
 */
ColumnClearCmd::~ColumnClearCmd() {
	if (m_undone) {
		if (!m_empty_data) return;
		switch (m_col->columnMode()) {
		case AbstractColumn::ColumnMode::Double:
			delete static_cast<QVector<double>*>(m_empty_data);
			break;
		case AbstractColumn::ColumnMode::Integer:
			delete static_cast<QVector<int>*>(m_empty_data);
			break;
		case AbstractColumn::ColumnMode::BigInt:
			delete static_cast<QVector<qint64>*>(m_empty_data);
			break;
		case AbstractColumn::ColumnMode::Text:
			delete static_cast<QVector<QString>*>(m_empty_data);
			break;
		case AbstractColumn::ColumnMode::DateTime:
		case AbstractColumn::ColumnMode::Month:
		case AbstractColumn::ColumnMode::Day:
			delete static_cast<QVector<QDateTime>*>(m_empty_data);
			break;
		}
	} else {
		if (!m_data) return;
		switch (m_col->columnMode()) {
		case AbstractColumn::ColumnMode::Double:
			delete static_cast<QVector<double>*>(m_data);
			break;
		case AbstractColumn::ColumnMode::Integer:
			delete static_cast<QVector<int>*>(m_data);
			break;
		case AbstractColumn::ColumnMode::BigInt:
			delete static_cast<QVector<qint64>*>(m_data);
			break;
		case AbstractColumn::ColumnMode::Text:
			delete static_cast<QVector<QString>*>(m_data);
			break;
		case AbstractColumn::ColumnMode::DateTime:
		case AbstractColumn::ColumnMode::Month:
		case AbstractColumn::ColumnMode::Day:
			delete static_cast<QVector<QDateTime>*>(m_data);
			break;
		}
	}
}

/**
 * \brief Execute the command
 */
void ColumnClearCmd::redo() {
	if (!m_empty_data) {
		const int rowCount = m_col->rowCount();
		switch (m_col->columnMode()) {
		case AbstractColumn::ColumnMode::Double: {
			auto* vec = new QVector<double>(rowCount);
			m_empty_data = vec;
			for (int i = 0; i < rowCount; ++i)
				vec->operator[](i) = NAN;
			break;
		}
		case AbstractColumn::ColumnMode::Integer: {
			auto* vec = new QVector<int>(rowCount);
			m_empty_data = vec;
			for (int i = 0; i < rowCount; ++i)
				vec->operator[](i) = 0;
			break;
		}
		case AbstractColumn::ColumnMode::BigInt: {
			auto* vec = new QVector<qint64>(rowCount);
			m_empty_data = vec;
			for (int i = 0; i < rowCount; ++i)
				vec->operator[](i) = 0;
			break;
		}
		case AbstractColumn::ColumnMode::DateTime:
		case AbstractColumn::ColumnMode::Month:
		case AbstractColumn::ColumnMode::Day:
			m_empty_data = new QVector<QDateTime>();
			for (int i = 0; i < rowCount; ++i)
				static_cast< QVector<QDateTime>*>(m_empty_data)->append(QDateTime());
			break;
		case AbstractColumn::ColumnMode::Text:
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
ColumnSetGlobalFormulaCmd::ColumnSetGlobalFormulaCmd(ColumnPrivate* col, QString formula, QStringList variableNames,
													 QVector<Column*> variableColumns, bool autoUpdate)
	: QUndoCommand(),
	m_col(col),
	m_newFormula(std::move(formula)),
	m_newVariableNames(std::move(variableNames)),
	m_newVariableColumns(std::move(variableColumns)),
	m_newAutoUpdate(autoUpdate)
{
	setText(i18n("%1: set formula", col->name()));
}

void ColumnSetGlobalFormulaCmd::redo() {
	if (!m_copied) {
		m_formula = m_col->formula();
		for (auto& d: m_col->formulaData()) {
			m_variableNames << d.variableName();
			m_variableColumns << d.m_column;
		}
		m_autoUpdate = m_col->formulaAutoUpdate();
		m_copied = true;
	}

	QVector<Column::FormulaData> formulaData;
	for (int i = 0; i < m_newVariableNames.count(); i++)
		formulaData << Column::FormulaData(m_newVariableNames.at(i), m_newVariableColumns.at(i));

	m_col->setFormula(m_newFormula, formulaData, m_newAutoUpdate);
}

void ColumnSetGlobalFormulaCmd::undo() {
	QVector<Column::FormulaData> formulaData;
	for (int i = 0; i < m_variableNames.count(); i++)
		formulaData << Column::FormulaData(m_variableNames.at(i), m_variableColumns.at(i));
	m_col->setFormula(m_formula, formulaData, m_newAutoUpdate);
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
ColumnSetFormulaCmd::ColumnSetFormulaCmd(ColumnPrivate* col, const Interval<int>& interval, QString formula, QUndoCommand* parent)
	: QUndoCommand(parent), m_col(col), m_interval(interval), m_newFormula(std::move(formula)) {
	setText(i18n("%1: set cell formula", col->name()));
}


void ColumnSetFormulaCmd::redo() {
	if (!m_copied) {
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
}

/**
 * \brief Execute the command
 */
void ColumnClearFormulasCmd::redo() {
	if (!m_copied) {
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

