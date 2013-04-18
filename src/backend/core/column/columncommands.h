/***************************************************************************
    File                 : columncommands.h
    Project              : AbstractColumn
    Description          : Commands to be called by Column to modify Column::Private
    --------------------------------------------------------------------
    Copyright            : (C) 2007,2008 Tilman Benkert (thzs*gmx.net)
                           (C) 2010 by Knut Franke
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

#ifndef COLUMNCOMMANDS_H
#define COLUMNCOMMANDS_H

#include "lib/IntervalAttribute.h"
#include "core/column/Column.h"

#include <QUndoCommand>
#include <QStringList>
#include <QDateTime>

class AbstractSimpleFilter;

class ColumnSetModeCmd : public QUndoCommand
{
public:
	ColumnSetModeCmd(Column::Private * col, AbstractColumn::ColumnMode mode, QUndoCommand * parent = 0 );
	~ColumnSetModeCmd();

	virtual void redo();
	virtual void undo();

private:
	Column::Private * m_col;
	AbstractColumn::ColumnMode m_old_mode;	
	AbstractColumn::ColumnMode m_mode;
	void * m_old_data;
	void * m_new_data;
	AbstractSimpleFilter* m_new_in_filter;
	AbstractSimpleFilter* m_new_out_filter;
	AbstractSimpleFilter* m_old_in_filter;
	AbstractSimpleFilter* m_old_out_filter;
	bool m_undone;
	bool m_executed;
};

class ColumnFullCopyCmd : public QUndoCommand
{
public:
	ColumnFullCopyCmd(Column::Private * col, const AbstractColumn * src, QUndoCommand * parent = 0 );
	~ColumnFullCopyCmd();

	virtual void redo();
	virtual void undo();

private:
	Column::Private * m_col;
	const AbstractColumn * m_src;
	Column::Private * m_backup;
	Column * m_backup_owner;

};

class ColumnPartialCopyCmd : public QUndoCommand
{
public:
	ColumnPartialCopyCmd(Column::Private * col, const AbstractColumn * src, int src_start, int dest_start, int num_rows, QUndoCommand * parent = 0 );
	~ColumnPartialCopyCmd();

	virtual void redo();
	virtual void undo();

private:
	Column::Private * m_col;
	const AbstractColumn * m_src;
	Column::Private * m_col_backup;
	Column::Private * m_src_backup;
	Column * m_col_backup_owner;
	Column * m_src_backup_owner;
	int m_src_start;
	int m_dest_start;
	int m_num_rows;
	int m_old_row_count;
};

class ColumnInsertRowsCmd : public QUndoCommand
{
public:
	ColumnInsertRowsCmd(Column::Private * col, int before, int count, QUndoCommand * parent = 0 );
	~ColumnInsertRowsCmd();

	virtual void redo();
	virtual void undo();

private:
	Column::Private * m_col;
	int m_before, m_count;
};

class ColumnRemoveRowsCmd : public QUndoCommand
{
public:
	ColumnRemoveRowsCmd(Column::Private * col, int first, int count, QUndoCommand * parent = 0 );
	~ColumnRemoveRowsCmd();

	virtual void redo();
	virtual void undo();

private:
	Column::Private * m_col;
	int m_first, m_count;
	int m_data_row_count;
	int m_old_size;
	Column::Private * m_backup;
	Column * m_backup_owner;
	IntervalAttribute<QString> m_formulas;
};

class ColumnSetPlotDesignationCmd : public QUndoCommand
{
public:
	ColumnSetPlotDesignationCmd(Column::Private * col, AbstractColumn::PlotDesignation pd, QUndoCommand * parent = 0 );
	~ColumnSetPlotDesignationCmd();

	virtual void redo();
	virtual void undo();

private:
	Column::Private * m_col;
	AbstractColumn::PlotDesignation m_new_pd;
	AbstractColumn::PlotDesignation m_old_pd;
};

class ColumnSetWidthCmd : public QUndoCommand
{
public:
	ColumnSetWidthCmd(Column::Private * col, int new_value, QUndoCommand * parent = 0 );
	~ColumnSetWidthCmd();

	virtual void redo();
	virtual void undo();

private:
	Column::Private * m_col;
	int m_other_value;
};

class ColumnClearCmd : public QUndoCommand
{
public:
	ColumnClearCmd(Column::Private * col, QUndoCommand * parent = 0 );
	~ColumnClearCmd();

	virtual void redo();
	virtual void undo();

private:
	Column::Private * m_col;
	void * m_data;
	void * m_empty_data;
	bool m_undone;

};

class ColumnSetFormulaCmd : public QUndoCommand
{
public:
	ColumnSetFormulaCmd(Column::Private * col, Interval<int> interval, const QString& formula, QUndoCommand * parent = 0 );
	~ColumnSetFormulaCmd();

	virtual void redo();
	virtual void undo();

private:
	Column::Private * m_col;
	Interval<int> m_interval;
	QString m_formula;
	IntervalAttribute<QString> m_formulas;
	bool m_copied;

};

class ColumnClearFormulasCmd : public QUndoCommand
{
public:
	ColumnClearFormulasCmd(Column::Private * col, QUndoCommand * parent = 0 );
	~ColumnClearFormulasCmd();

	virtual void redo();
	virtual void undo();

private:
	Column::Private * m_col;
	IntervalAttribute<QString> m_formulas;
	bool m_copied;

};

class ColumnSetTextCmd : public QUndoCommand
{
public:
	ColumnSetTextCmd(Column::Private * col, int row, const QString& new_value, QUndoCommand * parent = 0 );
	~ColumnSetTextCmd();

	virtual void redo();
	virtual void undo();

private:
	Column::Private * m_col;
	int m_row;
	QString m_new_value;
	QString m_old_value;
	int m_row_count;
};

class ColumnSetValueCmd : public QUndoCommand
{
public:
	ColumnSetValueCmd(Column::Private * col, int row, double new_value, QUndoCommand * parent = 0 );
	~ColumnSetValueCmd();

	virtual void redo();
	virtual void undo();

private:
	Column::Private * m_col;
	int m_row;
	double m_new_value;
	double m_old_value;
	int m_row_count;
};

class ColumnSetDateTimeCmd : public QUndoCommand
{
public:
	ColumnSetDateTimeCmd(Column::Private * col, int row, const QDateTime& new_value, QUndoCommand * parent = 0 );
	~ColumnSetDateTimeCmd();

	virtual void redo();
	virtual void undo();

private:
	Column::Private * m_col;
	int m_row;
	QDateTime m_new_value;
	QDateTime m_old_value;
	int m_row_count;
};

class ColumnReplaceTextsCmd : public QUndoCommand
{
public:
	ColumnReplaceTextsCmd(Column::Private * col, int first, const QStringList& new_values, QUndoCommand * parent = 0 );
	~ColumnReplaceTextsCmd();

	virtual void redo();
	virtual void undo();

private:
	Column::Private * m_col;
	int m_first;
	QStringList m_new_values;
	QStringList m_old_values;
	bool m_copied;
	int m_row_count;
};

class ColumnReplaceValuesCmd : public QUndoCommand
{
public:
	ColumnReplaceValuesCmd(Column::Private * col, int first, const QVector<double>& new_values, QUndoCommand * parent = 0 );
	~ColumnReplaceValuesCmd();

	virtual void redo();
	virtual void undo();

private:
	Column::Private * m_col;
	int m_first;
	QVector<double> m_new_values;
	QVector<double> m_old_values;
	bool m_copied;
	int m_row_count;
};

class ColumnReplaceDateTimesCmd : public QUndoCommand
{
public:
	ColumnReplaceDateTimesCmd(Column::Private * col, int first, const QList<QDateTime>& new_values, QUndoCommand * parent = 0 );
	~ColumnReplaceDateTimesCmd();

	virtual void redo();
	virtual void undo();

private:
	Column::Private * m_col;
	int m_first;
	QList<QDateTime> m_new_values;
	QList<QDateTime> m_old_values;
	bool m_copied;
	int m_row_count;
};

#endif
