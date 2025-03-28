/*
	File                 : MatrixPrivate.h
	Project              : LabPlot
	Description          : Private members of Matrix.
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2008-2009 Tilman Benkert <thzs@gmx.net>
	SPDX-FileCopyrightText: 2015 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2017 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MATRIXPRIVATE_H
#define MATRIXPRIVATE_H

class MatrixPrivate {
public:
	explicit MatrixPrivate(Matrix*, AbstractColumn::ColumnMode);
	~MatrixPrivate();

	void insertColumns(int before, int count);
	void removeColumns(int first, int count);
	void insertRows(int before, int count);
	void removeRows(int first, int count);

	QString name() const {
		return q->name();
	}

	// get value of cell at row/col (must be defined in header)
	template<typename T>
	T cell(int row, int col) const {
		Q_ASSERT(row >= 0 && row < rowCount());
		Q_ASSERT(col >= 0 && col < columnCount());

		return (static_cast<QVector<QVector<T>>*>(data))->at(col).at(row);
	}

	// Set value of cell at row/col (must be defined in header)
	template<typename T>
	void setCell(int row, int col, T value) {
		Q_ASSERT(row >= 0 && row < rowCount());
		Q_ASSERT(col >= 0 && col < columnCount());

		static_cast<QVector<QVector<T>>*>(data)->operator[](col)[row] = value;

		if (!suppressDataChange)
			Q_EMIT q->dataChanged(row, col, row, col);
	}
	// get column cells (must be defined in header)
	template<typename T>
	QVector<T> columnCells(int col, int first_row, int last_row) const {
		const auto currRowCount = rowCount();
		Q_ASSERT(first_row >= 0 && first_row < currRowCount);
		Q_ASSERT(last_row >= 0 && last_row < currRowCount);

		if (first_row == 0 && last_row == currRowCount - 1)
			return (static_cast<QVector<QVector<T>>*>(data))->at(col);

		QVector<T> result;
		for (int i = first_row; i <= last_row; i++)
			result.append(static_cast<QVector<QVector<T>>*>(data)->at(col).at(i));
		return result;
	}
	// set column cells (must be defined in header)
	template<typename T>
	void setColumnCells(int col, int first_row, int last_row, const QVector<T>& values) {
		const auto currRowCount = rowCount();
		Q_ASSERT(first_row >= 0 && first_row < currRowCount);
		Q_ASSERT(last_row >= 0 && last_row < currRowCount);
		Q_ASSERT(values.count() > last_row - first_row);

		if (first_row == 0 && last_row == currRowCount - 1) {
			static_cast<QVector<QVector<T>>*>(data)->operator[](col) = values;
			static_cast<QVector<QVector<T>>*>(data)->operator[](col).resize(currRowCount); // values may be larger
			if (!suppressDataChange)
				Q_EMIT q->dataChanged(first_row, col, last_row, col);
			return;
		}

		for (int i = first_row; i <= last_row; i++)
			static_cast<QVector<QVector<T>>*>(data)->operator[](col)[i] = values.at(i - first_row);

		if (!suppressDataChange)
			Q_EMIT q->dataChanged(first_row, col, last_row, col);
	}
	// get row cells (must be defined in header)
	template<typename T>
	QVector<T> rowCells(int row, int first_column, int last_column) const {
		Q_ASSERT(first_column >= 0 && first_column < columnCount());
		Q_ASSERT(last_column >= 0 && last_column < columnCount());

		QVector<T> result;
		for (int i = first_column; i <= last_column; i++)
			result.append(static_cast<QVector<QVector<T>>*>(data)->operator[](i)[row]);
		return result;
	}
	// set row cells (must be defined in header)
	template<typename T>
	void setRowCells(int row, int first_column, int last_column, const QVector<T>& values) {
		Q_ASSERT(first_column >= 0 && first_column < columnCount());
		Q_ASSERT(last_column >= 0 && last_column < columnCount());
		Q_ASSERT(values.count() > last_column - first_column);

		for (int i = first_column; i <= last_column; i++)
			static_cast<QVector<QVector<T>>*>(data)->operator[](i)[row] = values.at(i - first_column);
		if (!suppressDataChange)
			Q_EMIT q->dataChanged(row, first_column, row, last_column);
	}

	void clearColumn(int col);

	void setRowHeight(int row, int height) {
		rowHeights[row] = height;
	}
	void setColumnWidth(int col, int width) {
		columnWidths[col] = width;
	}
	int rowHeight(int row) const {
		return rowHeights.at(row);
	}
	int columnWidth(int col) const {
		return columnWidths.at(col);
	}

	int rowCount() const;
	int columnCount() const;
	void updateViewHeader();
	void emitDataChanged(int top, int left, int bottom, int right) {
		Q_EMIT q->dataChanged(top, left, bottom, right);
	}

	Matrix* q;
	void* data;
	AbstractColumn::ColumnMode mode; // mode (data type) of values

	QVector<int> rowHeights; //!< Row widths
	QVector<int> columnWidths; //!< Columns widths
	Matrix::HeaderFormat headerFormat{Matrix::HeaderFormat::HeaderRowsColumns};

	char numericFormat{'f'}; //!< Format code for displaying numbers
	int precision{3}; //!< Number of significant digits
	double xStart{0.0}, xEnd{1.0};
	double yStart{0.0}, yEnd{1.0};
	QString formula; //!< formula used to calculate the cells
	bool suppressDataChange;
};

#endif
