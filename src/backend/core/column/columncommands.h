/*
    File                 : columncommands.h
    Project              : LabPlot
    Description          : Commands to be called by Column to modify ColumnPrivate
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2007, 2008 Tilman Benkert <thzs@gmx.net>
    SPDX-FileCopyrightText: 2010 Knut Franke <knut.franke@gmx.de>
    SPDX-FileCopyrightText: 2009-2017 Alexander Semke <alexander.semke@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef COLUMNCOMMANDS_H
#define COLUMNCOMMANDS_H

#include "backend/lib/IntervalAttribute.h"
#include "backend/core/column/Column.h"
#include "backend/core/column/ColumnPrivate.h"

#include <KLocalizedString>

#include <QUndoCommand>
#include <QDateTime>

class QStringList;
class AbstractSimpleFilter;

class ColumnSetModeCmd : public QUndoCommand {
public:
	explicit ColumnSetModeCmd(ColumnPrivate* col, AbstractColumn::ColumnMode mode, QUndoCommand* parent = nullptr);
	~ColumnSetModeCmd() override;

	void redo() override;
	void undo() override;

private:
	ColumnPrivate* m_col;
	AbstractColumn::ColumnMode m_old_mode{AbstractColumn::ColumnMode::Double};
	AbstractColumn::ColumnMode m_mode;
	void* m_old_data{nullptr};
	void* m_new_data{nullptr};
	AbstractSimpleFilter* m_new_in_filter{nullptr};
	AbstractSimpleFilter* m_new_out_filter{nullptr};
	AbstractSimpleFilter* m_old_in_filter{nullptr};
	AbstractSimpleFilter* m_old_out_filter{nullptr};
	bool m_undone{false};
	bool m_executed{false};
};

class ColumnFullCopyCmd : public QUndoCommand {
public:
	explicit ColumnFullCopyCmd(ColumnPrivate* col, const AbstractColumn* src, QUndoCommand* parent = nullptr);
	~ColumnFullCopyCmd() override;

	void redo() override;
	void undo() override;

private:
	ColumnPrivate* m_col;
	const AbstractColumn* m_src;
	ColumnPrivate* m_backup{nullptr};
	Column* m_backup_owner{nullptr};
};

class ColumnPartialCopyCmd : public QUndoCommand {
public:
	explicit ColumnPartialCopyCmd(ColumnPrivate* col, const AbstractColumn* src, int src_start, int dest_start, int num_rows, QUndoCommand* parent = nullptr);
	~ColumnPartialCopyCmd() override;

	void redo() override;
	void undo() override;

private:
	ColumnPrivate* m_col;
	const AbstractColumn * m_src;
	ColumnPrivate* m_col_backup{nullptr};
	ColumnPrivate* m_src_backup{nullptr};
	Column* m_col_backup_owner{nullptr};
	Column* m_src_backup_owner{nullptr};
	int m_src_start;
	int m_dest_start;
	int m_num_rows;
	int m_old_row_count{0};
};

class ColumnInsertRowsCmd : public QUndoCommand {
public:
	explicit ColumnInsertRowsCmd(ColumnPrivate* col, int before, int count, QUndoCommand* parent = nullptr);

	void redo() override;
	void undo() override;

private:
	ColumnPrivate* m_col;
	int m_before, m_count;
};

class ColumnRemoveRowsCmd : public QUndoCommand {
public:
	explicit ColumnRemoveRowsCmd(ColumnPrivate* col, int first, int count, QUndoCommand* parent = nullptr);
	~ColumnRemoveRowsCmd() override;

	void redo() override;
	void undo() override;

private:
	ColumnPrivate* m_col;
	int m_first, m_count;
	int m_data_row_count{0};
	int m_old_size{0};
	ColumnPrivate* m_backup{nullptr};
	Column* m_backup_owner{nullptr};
	IntervalAttribute<QString> m_formulas;
};

class ColumnSetPlotDesignationCmd : public QUndoCommand {
public:
	explicit ColumnSetPlotDesignationCmd(ColumnPrivate* col, AbstractColumn::PlotDesignation pd, QUndoCommand* parent = nullptr);

	void redo() override;
	void undo() override;

private:
	ColumnPrivate* m_col;
	AbstractColumn::PlotDesignation m_new_pd;
	AbstractColumn::PlotDesignation m_old_pd{AbstractColumn::PlotDesignation::X};
};

class ColumnClearCmd : public QUndoCommand {
public:
	explicit ColumnClearCmd(ColumnPrivate* col, QUndoCommand* parent = nullptr);
	~ColumnClearCmd() override;

	void redo() override;
	void undo() override;

private:
	ColumnPrivate* m_col;
	void* m_data{nullptr};
	void* m_empty_data{nullptr};
	bool m_undone{false};

};

class ColumnSetGlobalFormulaCmd : public QUndoCommand {
public:
	explicit ColumnSetGlobalFormulaCmd(ColumnPrivate* col, QString formula,
									   QStringList variableNames, QVector<Column*> columns,
									   bool autoUpdate);

	void redo() override;
	void undo() override;

private:
	ColumnPrivate* m_col;
	QString m_formula;
	QStringList m_variableNames;
	QVector<Column*> m_variableColumns;
	bool m_autoUpdate{false};
	QString m_newFormula;
	QStringList m_newVariableNames;
	QVector<Column*> m_newVariableColumns;
	bool m_newAutoUpdate{false};
	bool m_copied{false};
};

class ColumnSetFormulaCmd : public QUndoCommand {
public:
	explicit ColumnSetFormulaCmd(ColumnPrivate* col, const Interval<int>& interval, QString formula, QUndoCommand* parent = nullptr);

	void redo() override;
	void undo() override;

private:
	ColumnPrivate* m_col;
	Interval<int> m_interval;
	QString m_oldFormula;
	QString m_newFormula;
	IntervalAttribute<QString> m_formulas;
	bool m_copied{false};
};

class ColumnClearFormulasCmd : public QUndoCommand {
public:
	explicit ColumnClearFormulasCmd(ColumnPrivate* col, QUndoCommand* parent = nullptr);

	void redo() override;
	void undo() override;

private:
	ColumnPrivate* m_col;
	IntervalAttribute<QString> m_formulas;
	bool m_copied{false};
};

template <typename T>
class ColumnSetCmd : public QUndoCommand {
public:
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
	explicit ColumnSetCmd(ColumnPrivate* col, int row, const T& old_value, const T& new_value, QUndoCommand* parent = nullptr)
		: QUndoCommand(parent), m_col(col), m_row(row), m_new_value(std::move(new_value)) , m_old_value(std::move(old_value)){
		setText(i18n("%1: set value for row %2", col->name(), row));
	}

	void redo() override  {
		m_row_count = m_col->rowCount();
		m_col->setValueAt(m_row, m_new_value);
	}
	void undo() override {
		m_col->setValueAt(m_row, m_old_value);
		// TODO: resizeTo and replaceData needed?
		m_col->resizeTo(m_row_count);
		m_col->replaceData(m_col->data());
	}

private:
	ColumnPrivate* m_col;
	int m_row;
	T m_new_value;
	T m_old_value;
	int m_row_count{0};
};

template<typename T>
class ColumnReplaceCmd : public QUndoCommand {
public:
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
	explicit ColumnReplaceCmd(ColumnPrivate* col, int first, const QVector<T>& new_values, QUndoCommand* parent = nullptr)
		: QUndoCommand(parent), m_col(col), m_first(first), m_new_values(new_values) {
		setText(i18n("%1: replace the values for rows %2 to %3", col->name(), first, first + new_values.count() - 1));
	}

	void redo() override {
		if (!m_copied) {
			if (m_first < 0)
				m_old_values = *static_cast<QVector<T>*>(m_col->data());
			else
				m_old_values = static_cast<QVector<T>*>(m_col->data())->mid(m_first, m_new_values.count());
			m_row_count = m_col->rowCount();
			m_copied = true;
		}
		m_col->replaceValues(m_first, m_new_values);
	}
	void undo() override {
		m_col->replaceValues(m_first, m_old_values);
		// TODO: resizeTo and replaceData needed?
		m_col->resizeTo(m_row_count);
		m_col->replaceData(m_col->data());
	}

private:
	ColumnPrivate* m_col;
	int m_first;
	QVector<T> m_new_values;
	QVector<T> m_old_values;
	bool m_copied{false};
	int m_row_count{0};
};

#endif
