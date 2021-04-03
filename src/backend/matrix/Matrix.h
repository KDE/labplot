/***************************************************************************
    File                 : Matrix.h
    Project              : Matrix
    Description          : Spreadsheet with a MxN matrix data model
    --------------------------------------------------------------------
    Copyright            : (C) 2008-2009 Tilman Benkert (thzs@gmx.net)
    Copyright            : (C) 2015-2017 Alexander Semke (alexander.semke@web.de)
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
#ifndef MATRIX_H
#define MATRIX_H

#include "backend/datasources/AbstractDataSource.h"
#include "backend/datasources/filters/AbstractFileFilter.h"
#include "backend/lib/macros.h"

class MatrixPrivate;
class MatrixModel;
class MatrixView;

class Matrix : public AbstractDataSource {
	Q_OBJECT
	Q_ENUMS(HeaderFormat)

public:
	enum class HeaderFormat {HeaderRowsColumns, HeaderValues, HeaderRowsColumnsValues};

	explicit Matrix(const QString& name, bool loading = false,
		   const AbstractColumn::ColumnMode = AbstractColumn::ColumnMode::Numeric);
	Matrix(int rows, int cols, const QString& name,
		   const AbstractColumn::ColumnMode = AbstractColumn::ColumnMode::Numeric);
	~Matrix() override;

	QIcon icon() const override;
	QMenu* createContextMenu() override;
	QWidget* view() const override;

	bool exportView() const override;
	bool printView() override;
	bool printPreview() const override;

	void* data() const;
	void setData(void*);

	QVector<AspectType> dropableOn() const override;

	BASIC_D_ACCESSOR_DECL(AbstractColumn::ColumnMode, mode, Mode)
	BASIC_D_ACCESSOR_DECL(int, rowCount, RowCount)
	BASIC_D_ACCESSOR_DECL(int, columnCount, ColumnCount)
	BASIC_D_ACCESSOR_DECL(char, numericFormat, NumericFormat)
	BASIC_D_ACCESSOR_DECL(int, precision, Precision)
	BASIC_D_ACCESSOR_DECL(HeaderFormat, headerFormat, HeaderFormat)
	BASIC_D_ACCESSOR_DECL(double, xStart, XStart)
	BASIC_D_ACCESSOR_DECL(double, xEnd, XEnd)
	BASIC_D_ACCESSOR_DECL(double, yStart, YStart)
	BASIC_D_ACCESSOR_DECL(double, yEnd, YEnd)
	CLASS_D_ACCESSOR_DECL(QString, formula, Formula)

	void setSuppressDataChangedSignal(bool);
	void setChanged();

	int rowHeight(int row) const;
	void setRowHeight(int row, int height);
	int columnWidth(int col) const;
	void setColumnWidth(int col, int width);

	void setDimensions(int rows, int cols);
	void setCoordinates(double x1, double x2, double y1, double y2);

	void insertColumns(int before, int count);
	void appendColumns(int count);
	void removeColumns(int first, int count);
	void clearColumn(int);

	void insertRows(int before, int count);
	void appendRows(int count);
	void removeRows(int first, int count);
	void clearRow(int);

	template <typename T> T cell(int row, int col) const;
	template <typename T> QString text(int row, int col);
	template <typename T> void setCell(int row, int col, T value);
	void clearCell(int row, int col);

	template <typename T> QVector<T> columnCells(int col, int first_row, int last_row);
	template <typename T> void setColumnCells(int col, int first_row, int last_row, const QVector<T>& values);
	template <typename T> QVector<T> rowCells(int row, int first_column, int last_column);
	template <typename T> void setRowCells(int row, int first_column, int last_column, const QVector<T>& values);

	void copy(Matrix* other);

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

	int prepareImport(std::vector<void*>& dataContainer, AbstractFileFilter::ImportMode,
		int rows, int cols, QStringList colNameList, QVector<AbstractColumn::ColumnMode>) override;
	void finalizeImport(int columnOffset, int startColumn, int endColumn,
		const QString& dateTimeFormat, AbstractFileFilter::ImportMode) override;

	typedef MatrixPrivate Private;

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

	void rowCountChanged(int);
	void columnCountChanged(int);

	void xStartChanged(double);
	void xEndChanged(double);
	void yStartChanged(double);
	void yEndChanged(double);

	void numericFormatChanged(char);
	void precisionChanged(int);
	void headerFormatChanged(Matrix::HeaderFormat);

private:
	void init();

	MatrixPrivate* const d;
	mutable MatrixModel* m_model{nullptr};
	mutable MatrixView* m_view{nullptr};

	friend class MatrixPrivate;
};

#endif
