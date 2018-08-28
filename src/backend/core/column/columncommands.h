/***************************************************************************
    File                 : columncommands.h
    Project              : LabPlot
    Description          : Commands to be called by Column to modify ColumnPrivate
    --------------------------------------------------------------------
    Copyright            : (C) 2007,2008 Tilman Benkert (thzs@gmx.net)
    Copyright            : (C) 2010 by Knut Franke (knut.franke@gmx.de)
    Copyright            : (C) 2009-2017 Alexander Semke (alexander.semke@web.de)

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

#ifndef COLUMNCOMMANDS_H
#define COLUMNCOMMANDS_H

#include "backend/lib/IntervalAttribute.h"
#include "backend/core/column/Column.h"

#include <QUndoCommand>
#include <QStringList>
#include <QDateTime>

class AbstractSimpleFilter;

class ColumnSetModeCmd : public QUndoCommand {
public:
	explicit ColumnSetModeCmd(ColumnPrivate* col, AbstractColumn::ColumnMode mode, QUndoCommand* parent = nullptr);
	~ColumnSetModeCmd() override;

	void redo() override;
	void undo() override;

private:
	ColumnPrivate* m_col;
	AbstractColumn::ColumnMode m_old_mode;
	AbstractColumn::ColumnMode m_mode;
	void* m_old_data;
	void* m_new_data;
	AbstractSimpleFilter* m_new_in_filter;
	AbstractSimpleFilter* m_new_out_filter;
	AbstractSimpleFilter* m_old_in_filter;
	AbstractSimpleFilter* m_old_out_filter;
	bool m_undone;
	bool m_executed;
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
	ColumnPrivate* m_backup;
	Column* m_backup_owner;
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
	ColumnPrivate* m_col_backup;
	ColumnPrivate* m_src_backup;
	Column* m_col_backup_owner;
	Column* m_src_backup_owner;
	int m_src_start;
	int m_dest_start;
	int m_num_rows;
	int m_old_row_count;
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
	int m_data_row_count;
	int m_old_size;
	ColumnPrivate* m_backup;
	Column* m_backup_owner;
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
	AbstractColumn::PlotDesignation m_old_pd;
};

class ColumnClearCmd : public QUndoCommand {
public:
	explicit ColumnClearCmd(ColumnPrivate* col, QUndoCommand* parent = nullptr);
	~ColumnClearCmd() override;

	void redo() override;
	void undo() override;

private:
	ColumnPrivate* m_col;
	void* m_data;
	void* m_empty_data;
	bool m_undone;

};

class ColumnSetGlobalFormulaCmd : public QUndoCommand {
public:
	explicit ColumnSetGlobalFormulaCmd(ColumnPrivate* col, const QString& formula, const QStringList& variableNames, const QStringList& variableColumnPathes);

	void redo() override;
	void undo() override;

private:
	ColumnPrivate* m_col;
	QString m_formula;
	QStringList m_variableNames;
	QStringList m_variableColumnPathes;
	QString m_newFormula;
	QStringList m_newVariableNames;
	QStringList m_newVariableColumnPathes;
	bool m_copied;
};

class ColumnSetFormulaCmd : public QUndoCommand {
public:
	explicit ColumnSetFormulaCmd(ColumnPrivate* col, const Interval<int>& interval, const QString& formula, QUndoCommand* parent = nullptr);

	void redo() override;
	void undo() override;

private:
	ColumnPrivate* m_col;
	Interval<int> m_interval;
	QString m_oldFormula;
	QString m_newFormula;
	IntervalAttribute<QString> m_formulas;
	bool m_copied;
};

class ColumnClearFormulasCmd : public QUndoCommand {
public:
	explicit ColumnClearFormulasCmd(ColumnPrivate* col, QUndoCommand* parent = nullptr);

	void redo() override;
	void undo() override;

private:
	ColumnPrivate* m_col;
	IntervalAttribute<QString> m_formulas;
	bool m_copied;
};

class ColumnSetTextCmd : public QUndoCommand {
public:
	explicit ColumnSetTextCmd(ColumnPrivate* col, int row, const QString& new_value, QUndoCommand* parent = nullptr);

	void redo() override;
	void undo() override;

private:
	ColumnPrivate* m_col;
	int m_row;
	QString m_new_value;
	QString m_old_value;
	int m_row_count;
};

class ColumnSetValueCmd : public QUndoCommand {
public:
	explicit ColumnSetValueCmd(ColumnPrivate* col, int row, double new_value, QUndoCommand* parent = nullptr);

	void redo() override;
	void undo() override;

private:
	ColumnPrivate* m_col;
	int m_row;
	double m_new_value;
	double m_old_value;
	int m_row_count;
};

class ColumnSetIntegerCmd : public QUndoCommand {
public:
	explicit ColumnSetIntegerCmd(ColumnPrivate* col, int row, int new_value, QUndoCommand* parent = nullptr);

	void redo() override;
	void undo() override;

private:
	ColumnPrivate* m_col;
	int m_row;
	int m_new_value;
	int m_old_value;
	int m_row_count;
};

class ColumnSetDateTimeCmd : public QUndoCommand {
public:
	explicit ColumnSetDateTimeCmd(ColumnPrivate* col, int row, const QDateTime& new_value, QUndoCommand* parent = nullptr);

	void redo() override;
	void undo() override;

private:
	ColumnPrivate* m_col;
	int m_row;
	QDateTime m_new_value;
	QDateTime m_old_value;
	int m_row_count;
};

class ColumnReplaceTextsCmd : public QUndoCommand {
public:
	explicit ColumnReplaceTextsCmd(ColumnPrivate* col, int first, const QVector<QString>& new_values, QUndoCommand* parent = nullptr);

	void redo() override;
	void undo() override;

private:
	ColumnPrivate* m_col;
	int m_first;
	QVector<QString> m_new_values;
	QVector<QString> m_old_values;
	bool m_copied;
	int m_row_count;
};

class ColumnReplaceValuesCmd : public QUndoCommand {
public:
	explicit ColumnReplaceValuesCmd(ColumnPrivate* col, int first, const QVector<double>& new_values, QUndoCommand* parent = nullptr);

	void redo() override;
	void undo() override;

private:
	ColumnPrivate* m_col;
	int m_first;
	QVector<double> m_new_values;
	QVector<double> m_old_values;
	bool m_copied;
	int m_row_count;
};

class ColumnReplaceIntegersCmd : public QUndoCommand {
public:
	explicit ColumnReplaceIntegersCmd(ColumnPrivate* col, int first, const QVector<int>& new_values, QUndoCommand* parent = nullptr);

	void redo() override;
	void undo() override;

private:
	ColumnPrivate* m_col;
	int m_first;
	QVector<int> m_new_values;
	QVector<int> m_old_values;
	bool m_copied;
	int m_row_count;
};

class ColumnReplaceDateTimesCmd : public QUndoCommand {
public:
	explicit ColumnReplaceDateTimesCmd(ColumnPrivate* col, int first, const QVector<QDateTime>& new_values, QUndoCommand* parent = nullptr);

	void redo() override;
	void undo() override;

private:
	ColumnPrivate* m_col;
	int m_first;
	QVector<QDateTime> m_new_values;
	QVector<QDateTime> m_old_values;
	bool m_copied;
	int m_row_count;
};

#endif
