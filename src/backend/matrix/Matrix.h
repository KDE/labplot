/***************************************************************************
    File                 : Matrix.h
    Project              : Matrix
    Description          : Spreadsheet with a MxN matrix data model
    --------------------------------------------------------------------
    Copyright            : (C) 2015 Alexander Semke (alexander.semke@web.de)
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
#ifndef MATRIX_H
#define MATRIX_H

#include "backend/datasources/AbstractDataSource.h"

class MatrixPrivate;
class MatrixView;

class Matrix : public AbstractDataSource {
    Q_OBJECT

	public:
		Matrix(AbstractScriptingEngine* engine, const QString& name, bool loading = false);
		Matrix(AbstractScriptingEngine* engine, int rows, int cols, const QString& name);
		~Matrix();

		enum HeaderFormat {HeaderRowsColumns, HeaderValues, HeaderRowsColumnsValues};
		virtual QIcon icon() const;
		virtual QMenu* createContextMenu();
		virtual QWidget* view() const;

		QVector<QVector<double> >& data() const;

		int defaultRowHeight() const;
		int defaultColumnWidth() const;

		int rowHeight(int row) const;
		void setRowHeight(int row, int height);
		int columnWidth(int col) const;
		void setColumnWidth(int col, int width);

		int columnCount() const;
		int rowCount() const;

		void insertColumns(int before, int count);
		void appendColumns(int count);
		void removeColumns(int first, int count);
		void insertRows(int before, int count);
		void appendRows(int count);
		void removeRows(int first, int count);

		void setDimensions(int rows, int cols);

		double cell(int row, int col) const;
		void setCell(int row, int col, double value );
		QVector<double> columnCells(int col, int first_row, int last_row);
		void setColumnCells(int col, int first_row, int last_row, const QVector<double>& values);
		QVector<double> rowCells(int row, int first_column, int last_column);
		void setRowCells(int row, int first_column, int last_column, const QVector<double>& values);

		QString text(int row, int col);
		void copy(Matrix* other);
		double xStart() const;
		double yStart() const;
		double xEnd() const;
		double yEnd() const;
		QString formula() const;
		void setFormula(const QString&);
		void setXStart(double x);
		void setXEnd(double x);
		void setYStart(double y);
		void setYEnd(double y);
		void setCoordinates(double x1, double x2, double y1, double y2);

		HeaderFormat headerFormat();
		void setHeaderFormat(HeaderFormat);

		char numericFormat() const;
		void setNumericFormat(char format);

		int displayedDigits()  const;
		void setDisplayedDigits(int digits);

		virtual void save(QXmlStreamWriter*) const;
		virtual bool load(XmlStreamReader*);

	public slots:
		void clear();
		void transpose();
		void mirrorVertically();
		void mirrorHorizontally();

		void addColumns();
		void addRows();
		void duplicate();

	signals:
		void requestProjectContextMenu(QMenu*);
		void columnsAboutToBeInserted(int before, int count);
		void columnsInserted(int first, int count);
		void columnsAboutToBeRemoved(int first, int count);
		void columnsRemoved(int first, int count);
		void rowsAboutToBeInserted(int before, int count);
		void rowsInserted(int first, int count);
		void rowsAboutToBeRemoved(int first, int count);
		void rowsRemoved(int first, int count);
		void dataChanged(int top, int left, int bottom, int right);
		void coordinatesChanged();
		void formulaChanged();
		void formatChanged();

	private:
		void init();
		bool readDisplayElement(XmlStreamReader*);
		bool readCoordinatesElement(XmlStreamReader*);
		bool readFormulaElement(XmlStreamReader*);
		bool readCellElement(XmlStreamReader*);
		bool readRowHeightElement(XmlStreamReader*);
		bool readColumnWidthElement(XmlStreamReader*);

		MatrixPrivate* const d;
		friend class MatrixPrivate;
		mutable MatrixView* m_view;
};

#endif
