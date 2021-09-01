/*
    File                 : matrixcommands.h
    Project              : LabPlot
    Description          : Commands used in Matrix (part of the undo/redo framework)
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2008 Tilman Benkert <thzs@gmx.net>
    SPDX-FileCopyrightText: 2015 Alexander Semke <alexander.semke@web.de>
    SPDX-FileCopyrightText: 2017 Stefan Gerlach <stefan.gerlach@uni.kn>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef MATRIX_COMMANDS_H
#define MATRIX_COMMANDS_H

#include <QUndoCommand>
#include <KLocalizedString>
#include "Matrix.h"
#include "MatrixPrivate.h"

//! Insert columns
class MatrixInsertColumnsCmd : public QUndoCommand {
public:
	MatrixInsertColumnsCmd(MatrixPrivate*, int before, int count, QUndoCommand* = nullptr);
	void redo() override;
	void undo() override;

private:
	MatrixPrivate* m_private_obj;
	int m_before; //! Column to insert before
	int m_count; //! The number of new columns
};

//! Insert rows
class MatrixInsertRowsCmd : public QUndoCommand {
public:
	MatrixInsertRowsCmd(MatrixPrivate*, int before, int count, QUndoCommand* = nullptr);
	void redo() override;
	void undo() override;

private:
	MatrixPrivate* m_private_obj;
	int m_before; //! Row to insert before
	int m_count; //! The number of new rows
};

//! Remove columns
template <typename T>
class MatrixRemoveColumnsCmd : public QUndoCommand {
public:
	MatrixRemoveColumnsCmd(MatrixPrivate* private_obj, int first, int count, QUndoCommand* parent = nullptr)
			: QUndoCommand(parent), m_private_obj(private_obj), m_first(first), m_count(count) {
		setText(i18np("%1: remove %2 column", "%1: remove %2 columns", m_private_obj->name(), m_count));
	}
	void redo() override {
		if(m_backups.isEmpty()) {
			int last_row = m_private_obj->rowCount-1;
			for (int i = 0; i < m_count; i++)
				m_backups.append(m_private_obj->columnCells<T>(m_first+i, 0, last_row));
		}
		m_private_obj->removeColumns(m_first, m_count);
		emit m_private_obj->q->columnCountChanged(m_private_obj->columnCount);
	}
	void undo() override {
		m_private_obj->insertColumns(m_first, m_count);
		int last_row = m_private_obj->rowCount-1;
		//TODO: use memcopy to copy from the backup vector
		for (int i = 0; i < m_count; i++)
			m_private_obj->setColumnCells(m_first+i, 0, last_row, m_backups.at(i));

		emit m_private_obj->q->columnCountChanged(m_private_obj->columnCount);
	}

private:
	MatrixPrivate* m_private_obj;

	int m_first; //! First column to remove
	int m_count; //! The number of columns to remove
	QVector<QVector<T>> m_backups; //! Backups of the removed columns
};

//! Remove rows
template <typename T>
class MatrixRemoveRowsCmd : public QUndoCommand {
public:
	MatrixRemoveRowsCmd(MatrixPrivate* private_obj, int first, int count, QUndoCommand* parent = nullptr)
			: QUndoCommand(parent), m_private_obj(private_obj), m_first(first), m_count(count) {
		setText(i18np("%1: remove %2 row", "%1: remove %2 rows", m_private_obj->name(), m_count));
	}
	void redo() override {
		if(m_backups.isEmpty()) {
			int last_row = m_first+m_count-1;
				for (int col = 0; col < m_private_obj->columnCount; col++)
					m_backups.append(m_private_obj->columnCells<T>(col, m_first, last_row));
		}
		m_private_obj->removeRows(m_first, m_count);
		emit m_private_obj->q->rowCountChanged(m_private_obj->rowCount);
	}
	void undo() override {
		m_private_obj->insertRows(m_first, m_count);
		int last_row = m_first+m_count-1;
		for (int col = 0; col < m_private_obj->columnCount; col++)
			m_private_obj->setColumnCells(col, m_first, last_row, m_backups.at(col));
		emit m_private_obj->q->rowCountChanged(m_private_obj->rowCount);
	}

private:
	MatrixPrivate* m_private_obj;
	int m_first; //! First row to remove
	int m_count; //! The number of rows to remove
	QVector< QVector<T> > m_backups; //! Backups of the removed rows
};

//! Clear matrix
template <typename T>
class MatrixClearCmd : public QUndoCommand {
public:
	explicit MatrixClearCmd(MatrixPrivate* private_obj, QUndoCommand* parent = nullptr)
			: QUndoCommand(parent), m_private_obj(private_obj) {
		setText(i18n("%1: clear", m_private_obj->name()));
	}
	void redo() override {
		if(m_backups.isEmpty()) {
			int last_row = m_private_obj->rowCount-1;

			for (int i = 0; i < m_private_obj->columnCount; i++)
				m_backups.append(m_private_obj->columnCells<T>(i, 0, last_row));
		}

		for (int i = 0; i < m_private_obj->columnCount; i++)
			m_private_obj->clearColumn(i);
	}
	void undo() override {
		int last_row = m_private_obj->rowCount-1;
		for (int i = 0; i < m_private_obj->columnCount; i++)
			m_private_obj->setColumnCells(i, 0, last_row, m_backups.at(i));
	}

private:
	MatrixPrivate* m_private_obj;
	QVector<QVector<T>> m_backups; //! Backups of the cleared cells
};

//! Clear matrix column
template <typename T>
class MatrixClearColumnCmd : public QUndoCommand {
public:
	MatrixClearColumnCmd(MatrixPrivate* private_obj, int col, QUndoCommand* parent = nullptr)
			: QUndoCommand(parent), m_private_obj(private_obj), m_col(col) {
		setText(i18n("%1: clear column %2", m_private_obj->name(), m_col+1));
	}
	void redo() override {
		if(m_backup.isEmpty())
			m_backup = m_private_obj->columnCells<T>(m_col, 0, m_private_obj->rowCount-1);
		m_private_obj->clearColumn(m_col);
	}
	void undo() override {
		m_private_obj->setColumnCells(m_col, 0, m_private_obj->rowCount-1, m_backup);
	}

private:
	MatrixPrivate* m_private_obj;
	int m_col; //! The index of the column
	QVector<T> m_backup; //! Backup of the cleared column
};

// Set cell value
template <typename T>
class MatrixSetCellValueCmd : public QUndoCommand {
public:
	MatrixSetCellValueCmd(MatrixPrivate* private_obj, int row, int col, T value, QUndoCommand* parent = nullptr)
			: QUndoCommand(parent), m_private_obj(private_obj), m_row(row), m_col(col), m_value(value), m_old_value(value) {
		// remark: don't use many QString::arg() calls in ctors of commands that might be called often,
		// they use a lot of execution time
		setText(i18n("%1: set cell value", m_private_obj->name()));
	}
	void redo() override {
		m_old_value = m_private_obj->cell<T>(m_row, m_col);
		m_private_obj->setCell(m_row, m_col, m_value);
	}
	void undo() override {
		m_private_obj->setCell(m_row, m_col, m_old_value);
	}

private:
	MatrixPrivate* m_private_obj;
	int m_row; //! The index of the row
	int m_col; //! The index of the column
	T m_value; //! New cell value
	T m_old_value; //! Backup of the changed value
};

// Set matrix coordinates
class MatrixSetCoordinatesCmd : public QUndoCommand {
public:
	MatrixSetCoordinatesCmd(MatrixPrivate*, double x1, double x2, double y1, double y2, QUndoCommand* = nullptr);
	void redo() override;
	void undo() override;

private:
	MatrixPrivate* m_private_obj;
	double m_new_x1;
	double m_new_x2;
	double m_new_y1;
	double m_new_y2;
	double m_old_x1{-1};
	double m_old_x2{-1};
	double m_old_y1{-1};
	double m_old_y2{-1};
};

//! Set matrix formula
class MatrixSetFormulaCmd : public QUndoCommand {
public:
	MatrixSetFormulaCmd(MatrixPrivate*, QString formula);
	void redo() override;
	void undo() override;

private:
	MatrixPrivate* m_private_obj;
	QString m_other_formula;
};

// Set cell values for (a part of) a column at once
template <typename T>
class MatrixSetColumnCellsCmd : public QUndoCommand {
public:
	MatrixSetColumnCellsCmd(MatrixPrivate* private_obj, int col, int first_row, int last_row, const QVector<T>& values, QUndoCommand* parent = nullptr)
			: QUndoCommand(parent), m_private_obj(private_obj), m_col(col), m_first_row(first_row), m_last_row(last_row), m_values(values) {
		setText(i18n("%1: set cell values", m_private_obj->name()));
	}
	void redo() override {
		if (m_old_values.isEmpty())
			m_old_values = m_private_obj->columnCells<T>(m_col, m_first_row, m_last_row);
		m_private_obj->setColumnCells(m_col, m_first_row, m_last_row, m_values);
	}
	void undo() override {
		m_private_obj->setColumnCells(m_col, m_first_row, m_last_row, m_old_values);
	}

private:
	MatrixPrivate* m_private_obj;
	int m_col; //! The index of the column
	int m_first_row; //! The index of the first row
	int m_last_row; //! The index of the last row
	QVector<T> m_values; //! New cell values
	QVector<T> m_old_values; //! Backup of the changed values
};

//! Set cell values for (a part of) a row at once
template <typename T>
class MatrixSetRowCellsCmd : public QUndoCommand {
public:
	MatrixSetRowCellsCmd(MatrixPrivate* private_obj, int row, int first_column, int last_column, const QVector<T>& values, QUndoCommand* parent = nullptr)
			: QUndoCommand(parent), m_private_obj(private_obj), m_row(row), m_first_column(first_column),
				m_last_column(last_column), m_values(values) {
		setText(i18n("%1: set cell values", m_private_obj->name()));
	}
	void redo() override {
		if (m_old_values.isEmpty())
			m_old_values = m_private_obj->rowCells<T>(m_row, m_first_column, m_last_column);
		m_private_obj->setRowCells(m_row, m_first_column, m_last_column, m_values);
	}
	void undo() override {
		m_private_obj->setRowCells(m_row, m_first_column, m_last_column, m_old_values);
	}

private:
	MatrixPrivate* m_private_obj;
	int m_row; //! The index of the row
	int m_first_column; //! The index of the first column
	int m_last_column; //! The index of the last column
	QVector<T> m_values; //! New cell values
	QVector<T> m_old_values; //! Backup of the changed values
};

//! Transpose the matrix
template <typename T>
class MatrixTransposeCmd : public QUndoCommand {
public:
	explicit MatrixTransposeCmd(MatrixPrivate* private_obj, QUndoCommand* parent = nullptr)
			: QUndoCommand(parent), m_private_obj(private_obj) {
		setText(i18n("%1: transpose", m_private_obj->name()));
	}
	void redo() override {
		int rows = m_private_obj->rowCount;
		int cols = m_private_obj->columnCount;
		int temp_size = qMax(rows, cols);
		m_private_obj->suppressDataChange = true;
		if (cols < rows)
			m_private_obj->insertColumns(cols, temp_size - cols);
		else if (cols > rows)
			m_private_obj->insertRows(rows, temp_size - rows);

		for (int i = 1; i < temp_size; i++) {
			QVector<T> row = m_private_obj->rowCells<T>(i, 0, i-1);
			QVector<T> col = m_private_obj->columnCells<T>(i, 0, i-1);
			m_private_obj->setRowCells(i, 0, i-1, col);
			m_private_obj->setColumnCells(i, 0, i-1, row);
		}

		if (cols < rows)
			m_private_obj->removeRows(cols, temp_size - cols);
		else if (cols > rows)
			m_private_obj->removeColumns(rows, temp_size - rows);
		m_private_obj->suppressDataChange = false;
		m_private_obj->emitDataChanged(0, 0, m_private_obj->rowCount-1, m_private_obj->columnCount-1);
	}
	void undo() override {
		redo();
	}

private:
	MatrixPrivate* m_private_obj;
};

//! Mirror the matrix horizontally
template <typename T>
class MatrixMirrorHorizontallyCmd : public QUndoCommand {
public:
	explicit MatrixMirrorHorizontallyCmd(MatrixPrivate* private_obj, QUndoCommand* parent = nullptr)
			: QUndoCommand(parent), m_private_obj(private_obj) {
		setText(i18n("%1: mirror horizontally", m_private_obj->name()));
	}
	void redo() override {
		int rows = m_private_obj->rowCount;
		int cols = m_private_obj->columnCount;
		int middle = cols/2;
		m_private_obj->suppressDataChange = true;

		for (int i = 0; i<middle; i++) {
			QVector<T> temp = m_private_obj->columnCells<T>(i, 0, rows-1);
			m_private_obj->setColumnCells(i, 0, rows-1, m_private_obj->columnCells<T>(cols-i-1, 0, rows-1));
			m_private_obj->setColumnCells(cols-i-1, 0, rows-1, temp);
		}
		m_private_obj->suppressDataChange = false;
		m_private_obj->emitDataChanged(0, 0, rows-1, cols-1);
	}
	void undo() override {
		redo();
	}

private:
	MatrixPrivate* m_private_obj;
};

// Mirror the matrix vertically
template <typename T>
class MatrixMirrorVerticallyCmd : public QUndoCommand {
public:
	explicit MatrixMirrorVerticallyCmd(MatrixPrivate* private_obj, QUndoCommand* parent = nullptr)
		: QUndoCommand(parent), m_private_obj(private_obj) {
			setText(i18n("%1: mirror vertically", m_private_obj->name()));
	}
	void redo() override {
		int rows = m_private_obj->rowCount;
		int cols = m_private_obj->columnCount;
		int middle = rows/2;
		m_private_obj->suppressDataChange = true;

		for (int i = 0; i < middle; i++) {
			QVector<T> temp = m_private_obj->rowCells<T>(i, 0, cols-1);
			m_private_obj->setRowCells(i, 0, cols-1, m_private_obj->rowCells<T>(rows-i-1, 0, cols-1));
			m_private_obj->setRowCells(rows-i-1, 0, cols-1, temp);
		}

		m_private_obj->suppressDataChange = false;
		m_private_obj->emitDataChanged(0, 0, rows-1, cols-1);
	}
	void undo() override {
		redo();
	}

private:
	MatrixPrivate* m_private_obj;
};

// Replace matrix values
class MatrixReplaceValuesCmd : public QUndoCommand {
public:
	explicit MatrixReplaceValuesCmd(MatrixPrivate*, void* new_values, QUndoCommand* = nullptr);
	void redo() override;
	void undo() override;

private:
	MatrixPrivate* m_private_obj;
	void* m_old_values{nullptr};
	void* m_new_values;
};

#endif // MATRIX_COMMANDS_H
