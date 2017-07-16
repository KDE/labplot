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
	enum HeaderFormat {HeaderRowsColumns, HeaderValues, HeaderRowsColumnsValues};

	Matrix(AbstractScriptingEngine* engine, const QString& name, bool loading = false);
	Matrix(AbstractScriptingEngine* engine, int rows, int cols, const QString& name);
	~Matrix();

	virtual QIcon icon() const override;
	virtual QMenu* createContextMenu() override;
	virtual QWidget* view() const override;

	virtual bool exportView() const override;
	virtual bool printView() override;
	virtual bool printPreview() const override;

	BASIC_D_ACCESSOR_DECL(int, rowCount, RowCount)
	BASIC_D_ACCESSOR_DECL(int, columnCount, ColumnCount)
	BASIC_D_ACCESSOR_DECL(double, xStart, XStart)
	BASIC_D_ACCESSOR_DECL(double, xEnd, XEnd)
	BASIC_D_ACCESSOR_DECL(double, yStart, YStart)
	BASIC_D_ACCESSOR_DECL(double, yEnd, YEnd)
	BASIC_D_ACCESSOR_DECL(char, numericFormat, NumericFormat)
	BASIC_D_ACCESSOR_DECL(int, precision, Precision)
	BASIC_D_ACCESSOR_DECL(HeaderFormat, headerFormat, HeaderFormat)
	CLASS_D_ACCESSOR_DECL(QString, formula, Formula)

//	QVector<QVector<double> >& data() const;
//	void setData(const QVector<QVector<double> >&);
	void* data() const;
	void setData(void*);

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

	double cell(int row, int col) const;
	QString text(int row, int col);
	void setCell(int row, int col, double value);
	void clearCell(int row, int col);

	//TODO: consider columnMode
	QVector<double> columnCells(int col, int first_row, int last_row);
	void setColumnCells(int col, int first_row, int last_row, const QVector<double>& values);
	QVector<double> rowCells(int row, int first_column, int last_column);
	void setRowCells(int row, int first_column, int last_column, const QVector<double>& values);

	void copy(Matrix* other);

	virtual void save(QXmlStreamWriter*) const override;
	virtual bool load(XmlStreamReader*) override;

	virtual int prepareImport(QVector<void*>& dataContainer, AbstractFileFilter::ImportMode,
		int rows, int cols, QStringList colNameList, QVector<AbstractColumn::ColumnMode>) override;
	virtual void finalizeImport(int columnOffset, int startColumn, int endColumn,
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

	friend class MatrixInsertRowsCmd;
	friend class MatrixRemoveRowsCmd;
	friend class MatrixInsertColumnsCmd;
	friend class MatrixRemoveColumnsCmd;
	void rowCountChanged(int);
	void columnCountChanged(int);

	friend class MatrixSetXStartCmd;
	friend class MatrixSetXEndCmd;
	friend class MatrixSetYStartCmd;
	friend class MatrixSetYEndCmd;
	void xStartChanged(double);
	void xEndChanged(double);
	void yStartChanged(double);
	void yEndChanged(double);

	friend class MatrixSetNumericFormatCmd;
	friend class MatrixSetPrecisionCmd;
	void numericFormatChanged(char);
	void precisionChanged(int);
	void headerFormatChanged(Matrix::HeaderFormat);

private:
	void init();

	MatrixPrivate* const d;
	friend class MatrixPrivate;
	mutable MatrixModel* m_model;
};

#endif
