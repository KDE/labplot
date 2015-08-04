/***************************************************************************
    File                 : MatrixPrivate.h
    Project              : LabPlot
    Description          : Private members of Matrix.
    --------------------------------------------------------------------
    Copyright            : (C) 2015 by Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2008-2009 Tilman Benkert (thzs*gmx.net)

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

#ifndef MATRIXPRIVATE_H
#define MATRIXPRIVATE_H

#include <QVector>

class MatrixPrivate {
	public:
		explicit MatrixPrivate(Matrix*);

		void insertColumns(int before, int count);
		void removeColumns(int first, int count);
		void insertRows(int before, int count);
		void removeRows(int first, int count);

		QString name() const { return q->name(); }

		double cell(int row, int col) const;
		void setCell(int row, int col, double value);
		QVector<double> columnCells(int col, int first_row, int last_row);
		void setColumnCells(int col, int first_row, int last_row, const QVector<double> & values);
		QVector<double> rowCells(int row, int first_column, int last_column);
		void setRowCells(int row, int first_column, int last_column, const QVector<double> & values);
		void clearColumn(int col);

		void setRowHeight(int row, int height) { rowHeights[row] = height; }
		void setColumnWidth(int col, int width) { columnWidths[col] = width; }
		int rowHeight(int row) const { return rowHeights.at(row); }
		int columnWidth(int col) const { return columnWidths.at(col); }

		void updateViewHeader();
		void emitDataChanged(int top, int left, int bottom, int right) { emit q->dataChanged(top, left, bottom, right); }

		Matrix* q;
		int columnCount;
		int rowCount;
		QVector< QVector<double> > matrixData;
		QVector<int> rowHeights;//!< Row widths
		QVector<int> columnWidths;//!< Columns widths
		Matrix::HeaderFormat headerFormat;
		char numericFormat; //!< Format code for displaying numbers
		int precision; //!< Number of significant digits
		QString formula; //!<formula used to calculate the cells
		double xStart;
		double xEnd;
		double yStart;
		double yEnd;
		bool suppressDataChange;
};

#endif
