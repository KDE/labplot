/***************************************************************************
    File                 : Matrix.cpp
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
#include "Matrix.h"
#include "MatrixPrivate.h"
#include "matrixcommands.h"
#include "backend/matrix/MatrixModel.h"
#include "backend/core/AbstractScript.h"
#include "backend/core/Folder.h"
#include "backend/lib/XmlStreamReader.h"
#include "commonfrontend/matrix/MatrixView.h"
#include "backend/lib/commandtemplates.h"

#include <QApplication>
#include <QLocale>

#include <KIcon>
#include <KLocale>
#include <KConfigGroup>

/*!
	This class manages matrix based data (i.e., mathematically
	a MxN matrix with M rows, N columns). This data is typically
	used to for 3D plots.

	The values of the matrix are stored as double precision values. Each columng
	of the matrix is stored in a QVector<double> objects.

	\ingroup backend
*/
Matrix::Matrix(AbstractScriptingEngine* engine, int rows, int cols, const QString& name)
	: AbstractDataSource(engine, name), d(new MatrixPrivate(this)), m_view(0) {

	//set initial number of rows and columns
	appendColumns(cols);
	appendRows(rows);

	init();

}

Matrix::Matrix(AbstractScriptingEngine* engine, const QString& name, bool loading)
	: AbstractDataSource(engine, name), d(new MatrixPrivate(this)), m_view(0) {

	if (!loading)
		init();

}


Matrix::~Matrix() {
	delete d;
}

void Matrix::init() {
	KConfig config;
	KConfigGroup group = config.group("Matrix");

	//format
	d->numericFormat = *group.readEntry("NumericFormat", "f").toLatin1().data();
	d->precision = group.readEntry("Precision", 3);
	d->headerFormat = (Matrix::HeaderFormat)group.readEntry("HeaderFormat", (int)Matrix::HeaderRowsColumns);

	//x-range
	d->xStart = group.readEntry("xStart", 0.0);
	d->xEnd = group.readEntry("xEnd", 1.0);
	int rows = group.readEntry("rows", 10);
	appendRows(rows);

	//
	d->yStart = group.readEntry("yStart", 0.0);
	d->yEnd = group.readEntry("yEnd", 1.0);
	int cols = group.readEntry("columns", 10);
	appendColumns(cols);
}

/*!
  Returns an icon to be used for decorating my views.
  */
QIcon Matrix::icon() const {
	return KIcon("table");
}

/*!
  Returns a new context menu. The caller takes ownership of the menu.
*/
QMenu* Matrix::createContextMenu() {
	QMenu* menu = AbstractPart::createContextMenu();
	emit requestProjectContextMenu(menu);
	return menu;
}

QWidget* Matrix::view() const {
	if (!m_view) 	{
		m_view = new MatrixView(const_cast<Matrix*>(this));
	}
	return m_view;
}


/* ========================================== getter methods ============================================= */
BASIC_D_READER_IMPL(Matrix, char, numericFormat, numericFormat)
BASIC_D_READER_IMPL(Matrix, int, precision, precision)
BASIC_D_READER_IMPL(Matrix, Matrix::HeaderFormat, headerFormat, headerFormat)

QVector<QVector<double> >& Matrix::data() const {
	return d->matrixData;
}

int Matrix::defaultRowHeight() const {
	return 20;
}

int Matrix::defaultColumnWidth() const {
	return  100;
}


/* ========================================== setter methods ============================================= */
STD_SETTER_CMD_IMPL_S(Matrix, SetNumericFormat, char, numericFormat)
void Matrix::setNumericFormat(char format) {
	if (format != d->numericFormat)
		exec(new MatrixSetNumericFormatCmd(d, format, i18n("%1: numeric format changed")));
}

STD_SETTER_CMD_IMPL_S(Matrix, SetPrecision, int, precision)
void Matrix::setPrecision(int precision) {
	if (precision != d->precision)
		exec(new MatrixSetPrecisionCmd(d, precision, i18n("%1: precision changed")));
}

//TODO: made this undoable?
void Matrix::setHeaderFormat(Matrix::HeaderFormat format) {
	d->headerFormat = format;
	m_view->model()->updateHeader();
}

void Matrix::insertColumns(int before, int count) {
	if( count < 1 || before < 0 || before > columnCount()) return;
	WAIT_CURSOR;
	beginMacro(i18np("%1: insert %2 column", "%1: insert %2 columns", name(), count));
	exec(new MatrixInsertColumnsCmd(d, before, count));
	endMacro();
	RESET_CURSOR;
}

void Matrix::appendColumns(int count) {
	insertColumns(columnCount(), count);
}

void Matrix::removeColumns(int first, int count) {
	if( count < 1 || first < 0 || first+count > columnCount()) return;
	WAIT_CURSOR;
	beginMacro(i18np("%1: remove %2 column", "%1: remove %2 columns", name(), count));
	exec(new MatrixRemoveColumnsCmd(d, first, count));
	endMacro();
	RESET_CURSOR;
}

void Matrix::removeRows(int first, int count) {
	if( count < 1 || first < 0 || first+count > rowCount()) return;
	WAIT_CURSOR;
	beginMacro(i18np("%1: remove %2 row", "%1: remove %2 rows", name(), count));
	exec(new MatrixRemoveRowsCmd(d, first, count));
	endMacro();
	RESET_CURSOR;
}

void Matrix::insertRows(int before, int count) {
	if( count < 1 || before < 0 || before > rowCount()) return;
	WAIT_CURSOR;
	beginMacro(i18np("%1: insert %2 row", "%1: insert %2 rows", name(), count));
	exec(new MatrixInsertRowsCmd(d, before, count));
	endMacro();
	RESET_CURSOR;
}

void Matrix::appendRows(int count) {
	insertRows(rowCount(), count);
}

void Matrix::setDimensions(int rows, int cols) {
	if( (rows < 0) || (cols < 0 ) || (rows == rowCount() && cols == columnCount()) ) return;
	WAIT_CURSOR;
	beginMacro(i18n("%1: set matrix size to %2x%3", name(), rows, cols));
	int col_diff = cols - columnCount();
	int row_diff = rows - rowCount();
	if(col_diff > 0)
		exec(new MatrixInsertColumnsCmd(d, columnCount(), col_diff));
	else if(col_diff < 0)
		exec(new MatrixRemoveColumnsCmd(d, columnCount()+col_diff, -col_diff));
	if(row_diff > 0)
		exec(new MatrixInsertRowsCmd(d, rowCount(), row_diff));
	else if(row_diff < 0)
		exec(new MatrixRemoveRowsCmd(d, rowCount()+row_diff, -row_diff));
	endMacro();
	RESET_CURSOR;
}

int Matrix::columnCount() const {
	return d->columnCount;
}

int Matrix::rowCount() const {
	return d->rowCount;
}

//! Return the value in the given cell
double Matrix::cell(int row, int col) const {
	return d->cell(row, col);
}

void Matrix::copy(Matrix* other) {
	WAIT_CURSOR;
	beginMacro(i18n("%1: copy %2", name(), other->name()));
	int rows = other->rowCount();
	int columns = other->columnCount();
	setDimensions(rows, columns);
	for (int i=0; i<rows; i++)
		setRowHeight(i, other->rowHeight(i));
	for (int i=0; i<columns; i++)
		setColumnWidth(i, other->columnWidth(i));
	d->suppressDataChange = true;
	for (int i=0; i<columns; i++)
		setColumnCells(i, 0, rows-1, other->columnCells(i, 0, rows-1));
	setCoordinates(other->xStart(), other->xEnd(), other->yStart(), other->yEnd());
	setNumericFormat(other->numericFormat());
	setPrecision(other->precision());
	d->formula = other->formula();
	d->suppressDataChange = false;
	emit dataChanged(0, 0, rows-1, columns-1);
	if (m_view) m_view->adjustHeaders();
	endMacro();
	RESET_CURSOR;
}


//! Return the text displayed in the given cell
QString Matrix::text(int row, int col) {
	return QLocale().toString(cell(row,col), d->numericFormat, d->precision);
}

//! Set the value of the cell
void Matrix::setCell(int row, int col, double value) {
	if(row < 0 || row >= rowCount()) return;
	if(col < 0 || col >= columnCount()) return;
	exec(new MatrixSetCellValueCmd(d, row, col, value));
}

//! Duplicate the matrix inside its folder
void Matrix::duplicate() {
	Matrix* matrix = new Matrix(0, rowCount(), columnCount(), name());
	matrix->copy(this);
	if (folder())
		folder()->addChild(matrix);
}

void Matrix::addRows() {
	if (!m_view) return;
	WAIT_CURSOR;
	int count = m_view->selectedRowCount(false);
	beginMacro(i18np("%1: add %2 rows", "%1: add %2 rows", name(), count));
	exec(new MatrixInsertRowsCmd(d, rowCount(), count));
	endMacro();
	RESET_CURSOR;
}

void Matrix::addColumns() {
	if (!m_view) return;
	WAIT_CURSOR;
	int count = m_view->selectedRowCount(false);
	beginMacro(i18np("%1: add %2 column", "%1: add %2 columns", name(), count));
	exec(new MatrixInsertColumnsCmd(d, columnCount(), count));
	endMacro();
	RESET_CURSOR;
}

void Matrix::setXStart(double x) {
	WAIT_CURSOR;
	exec(new MatrixSetCoordinatesCmd(d, x, d->xEnd, d->yStart, d->yEnd));
	emit coordinatesChanged();
	RESET_CURSOR;
}

void Matrix::setXEnd(double x) {
	WAIT_CURSOR;
	exec(new MatrixSetCoordinatesCmd(d, d->xStart, x, d->yStart, d->yEnd));
	emit coordinatesChanged();
	RESET_CURSOR;
}

void Matrix::setYStart(double y) {
	WAIT_CURSOR;
	exec(new MatrixSetCoordinatesCmd(d, d->xStart, d->xEnd, y, d->yEnd));
	emit coordinatesChanged();
	RESET_CURSOR;
}

void Matrix::setYEnd(double y) {
	WAIT_CURSOR;
	exec(new MatrixSetCoordinatesCmd(d, d->xStart, d->xEnd, d->yStart, y));
	emit coordinatesChanged();
	RESET_CURSOR;
}

void Matrix::setCoordinates(double x1, double x2, double y1, double y2) {
	WAIT_CURSOR;
	exec(new MatrixSetCoordinatesCmd(d, x1, x2, y1, y2));
	RESET_CURSOR;
}

double Matrix::xStart() const {
	return d->xStart;
}

double Matrix::yStart() const {
	return d->yStart;
}

double Matrix::xEnd() const {
	return d->xEnd;
}

double Matrix::yEnd() const {
	return d->yEnd;
}

QString Matrix::formula() const {
	return d->formula;
}

void Matrix::setFormula(const QString& formula) {
	WAIT_CURSOR;
	exec(new MatrixSetFormulaCmd(d, formula));
	emit formulaChanged();
	RESET_CURSOR;
}



//! This method should only be called by the view.
/** This method does not change the view, it only changes the
	* values that are saved when the matrix is saved. The view
	* has to take care of reading and applying these values */
void Matrix::setRowHeight(int row, int height) {
	d->setRowHeight(row, height);
}

//! This method should only be called by the view.
/** This method does not change the view, it only changes the
	* values that are saved when the matrix is saved. The view
	* has to take care of reading and applying these values */
void Matrix::setColumnWidth(int col, int width) {
	d->setColumnWidth(col, width);
}

int Matrix::rowHeight(int row) const {
	return d->rowHeight(row);
}

int Matrix::columnWidth(int col) const {
	return d->columnWidth(col);
}

//! Return the values in the given cells as double vector
QVector<double> Matrix::columnCells(int col, int first_row, int last_row) {
	return d->columnCells(col, first_row, last_row);
}

//! Set the values in the given cells from a double vector
void Matrix::setColumnCells(int col, int first_row, int last_row, const QVector<double> & values) {
	WAIT_CURSOR;
	exec(new MatrixSetColumnCellsCmd(d, col, first_row, last_row, values));
	RESET_CURSOR;
}

//! Return the values in the given cells as double vector
QVector<double> Matrix::rowCells(int row, int first_column, int last_column) {
	return d->rowCells(row, first_column, last_column);
}

//! Set the values in the given cells from a double vector
void Matrix::setRowCells(int row, int first_column, int last_column, const QVector<double> & values) {
	WAIT_CURSOR;
	exec(new MatrixSetRowCellsCmd(d, row, first_column, last_column, values));
	RESET_CURSOR;
}

//##############################################################################
//#########################  Public slots  #####################################
//##############################################################################
//! Clear the whole matrix (i.e. set all cells to 0.0)
void Matrix::clear() {
	WAIT_CURSOR;
	beginMacro(i18n("%1: clear", name()));
	exec(new MatrixClearCmd(d));
	endMacro();
	RESET_CURSOR;
}

void Matrix::transpose() {
	WAIT_CURSOR;
	exec(new MatrixTransposeCmd(d));
	RESET_CURSOR;
}

void Matrix::mirrorHorizontally() {
	WAIT_CURSOR;
	exec(new MatrixMirrorHorizontallyCmd(d));
	RESET_CURSOR;
}

void Matrix::mirrorVertically() {
	WAIT_CURSOR;
	exec(new MatrixMirrorVerticallyCmd(d));
	RESET_CURSOR;
}


//##############################################################################
//######################  Private implementation ###############################
//##############################################################################

MatrixPrivate::MatrixPrivate(Matrix* owner) : q(owner), columnCount(0), rowCount(0), suppressDataChange(false) {
}

/*!
	Insert \count columns before column number \c before
*/
void MatrixPrivate::insertColumns(int before, int count) {
	Q_ASSERT(before >= 0);
	Q_ASSERT(before <= columnCount);

	emit q->columnsAboutToBeInserted(before, count);
	for(int i=0; i<count; i++) {
		matrixData.insert(before+i, QVector<double>(rowCount));
		m_column_widths.insert(before+i, q->defaultColumnWidth());
	}

	columnCount += count;
	emit q->columnsInserted(before, count);
}

/*!
	Remove \c count columns starting with column index \c first
*/
void MatrixPrivate::removeColumns(int first, int count) {
	emit q->columnsAboutToBeRemoved(first, count);
	Q_ASSERT(first >= 0);
	Q_ASSERT(first+count <= columnCount);
	matrixData.remove(first, count);
	for (int i=0; i<count; i++)
		m_column_widths.removeAt(first);
	columnCount -= count;
	emit q->columnsRemoved(first, count);
}

/*!
	Insert \c count rows before row with the index \c before
*/
void MatrixPrivate::insertRows(int before, int count) {
	emit q->rowsAboutToBeInserted(before, count);
	Q_ASSERT(before >= 0);
	Q_ASSERT(before <= rowCount);
	for(int col=0; col<columnCount; col++)
		for(int i=0; i<count; i++)
			matrixData[col].insert(before+i, 0.0);
	for(int i=0; i<count; i++)
		m_row_heights.insert(before+i, q->defaultRowHeight());

	rowCount += count;
	emit q->rowsInserted(before, count);
}

/*!
	Remove \c count columns starting from the column with index \c first
*/
void MatrixPrivate::removeRows(int first, int count) {
	emit q->rowsAboutToBeRemoved(first, count);
	Q_ASSERT(first >= 0);
	Q_ASSERT(first+count <= rowCount);
	for(int col=0; col<columnCount; col++)
		matrixData[col].remove(first, count);
	for (int i=0; i<count; i++)
		m_row_heights.removeAt(first);

	rowCount -= count;
	emit q->rowsRemoved(first, count);
}

//! Return the value in the given cell
double MatrixPrivate::cell(int row, int col) const {
	Q_ASSERT(row >= 0 && row < rowCount);
	Q_ASSERT(col >= 0 && col < columnCount);
// 	if(row < 0 || row >= rowCount() || col < 0 || col >= columnCount())
// 		return 0.0;

	return matrixData.at(col).at(row);
}

void MatrixPrivate::setCell(int row, int col, double value) {
	Q_ASSERT(row >= 0 && row < rowCount);
	Q_ASSERT(col >= 0 && col < columnCount);
	matrixData[col][row] = value;
	if (!suppressDataChange)
		emit q->dataChanged(row, col, row, col);
}

QVector<double> MatrixPrivate::columnCells(int col, int first_row, int last_row) {
	Q_ASSERT(first_row >= 0 && first_row < rowCount);
	Q_ASSERT(last_row >= 0 && last_row < rowCount);

	if(first_row == 0 && last_row == rowCount-1)
		return matrixData.at(col);

	QVector<double> result;
	for(int i=first_row; i<=last_row; i++)
		result.append(matrixData.at(col).at(i));
	return result;
}

void MatrixPrivate::setColumnCells(int col, int first_row, int last_row, const QVector<double> & values) {
	Q_ASSERT(first_row >= 0 && first_row < rowCount);
	Q_ASSERT(last_row >= 0 && last_row < rowCount);
	Q_ASSERT(values.count() > last_row - first_row);

	if(first_row == 0 && last_row == rowCount-1) {
		matrixData[col] = values;
		matrixData[col].resize(rowCount);  // values may be larger
		if (!suppressDataChange)
			emit q->dataChanged(first_row, col, last_row, col);
		return;
	}

	for(int i=first_row; i<=last_row; i++)
		matrixData[col][i] = values.at(i-first_row);
	if (!suppressDataChange)
		emit q->dataChanged(first_row, col, last_row, col);
}

QVector<double> MatrixPrivate::rowCells(int row, int first_column, int last_column) {
	Q_ASSERT(first_column >= 0 && first_column < columnCount);
	Q_ASSERT(last_column >= 0 && last_column < columnCount);

	QVector<double> result;
	for(int i=first_column; i<=last_column; i++)
		result.append(matrixData.at(i).at(row));
	return result;
}

void MatrixPrivate::setRowCells(int row, int first_column, int last_column, const QVector<double>& values) {
	Q_ASSERT(first_column >= 0 && first_column < columnCount);
	Q_ASSERT(last_column >= 0 && last_column < columnCount);
	Q_ASSERT(values.count() > last_column - first_column);

	for(int i=first_column; i<=last_column; i++)
		matrixData[i][row] = values.at(i-first_column);
	if (!suppressDataChange)
		emit q->dataChanged(row, first_column, row, last_column);
}

//! Fill column with zeroes
void MatrixPrivate::clearColumn(int col) {
	matrixData[col].fill(0.0);
	if (!suppressDataChange)
		emit q->dataChanged(0, col, rowCount-1, col);
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################
void Matrix::save(QXmlStreamWriter* writer) const {
	int cols = columnCount();
	int rows = rowCount();
	writer->writeStartElement("matrix");
	writeBasicAttributes(writer);
	writer->writeAttribute("columns", QString::number(cols));
	writer->writeAttribute("rows", QString::number(rows));
	writeCommentElement(writer);
	writer->writeStartElement("formula");
	writer->writeCharacters(formula());
	writer->writeEndElement();
	writer->writeStartElement("display");
	writer->writeAttribute("headerFormat", QString::number(d->headerFormat));
	writer->writeAttribute("numericFormat", QString(QChar(d->numericFormat)));
	writer->writeAttribute("precision", QString::number(d->precision));
	writer->writeEndElement();
	writer->writeStartElement("coordinates");
	writer->writeAttribute("x_start", QString::number(d->xStart));
	writer->writeAttribute("x_end", QString::number(d->xEnd));
	writer->writeAttribute("y_start", QString::number(d->yStart));
	writer->writeAttribute("y_end", QString::number(d->yEnd));
	writer->writeEndElement();

	for (int col=0; col<cols; col++) {
		for (int row=0; row<rows; row++) {
			writer->writeStartElement("cell");
			writer->writeAttribute("row", QString::number(row));
			writer->writeAttribute("column", QString::number(col));
			writer->writeCharacters(QString::number(cell(row, col), 'e', 16));
			writer->writeEndElement();
		}
	}
	for (int col=0; col<cols; col++) {
		writer->writeStartElement("column_width");
		writer->writeAttribute("column", QString::number(col));
		writer->writeCharacters(QString::number(columnWidth(col)));
		writer->writeEndElement();
	}
	for (int row=0; row<rows; row++) {
		writer->writeStartElement("row_height");
		writer->writeAttribute("row", QString::number(row));
		writer->writeCharacters(QString::number(rowHeight(row)));
		writer->writeEndElement();
	}
	writer->writeEndElement(); // "matrix"
}

bool Matrix::load(XmlStreamReader* reader) {
	if(!reader->isStartElement() || reader->name() != "matrix"){
        reader->raiseError(i18n("no matrix element found"));
        return false;
    }

	if (!readBasicAttributes(reader)) return false;

	// read dimensions
	bool ok1, ok2;
	int rows, cols;
	rows = reader->readAttributeInt("rows", &ok1);
	cols = reader->readAttributeInt("columns", &ok2);
	if(!ok1 || !ok2) {
		reader->raiseError(i18n("invalid row or column count"));
		return false;
	}
	d->suppressDataChange = true;
	setDimensions(rows, cols);

	// read child elements
	while (!reader->atEnd()) {
		reader->readNext();

		if (reader->isEndElement()) break;

		if (reader->isStartElement()) {
			bool ret_val = true;
			if (reader->name() == "comment")
				ret_val = readCommentElement(reader);
			else if(reader->name() == "formula")
				ret_val = readFormulaElement(reader);
			else if(reader->name() == "display")
				ret_val = readDisplayElement(reader);
			else if(reader->name() == "coordinates")
				ret_val = readCoordinatesElement(reader);
			else if(reader->name() == "cell")
				ret_val = readCellElement(reader);
			else if(reader->name() == "row_height")
				ret_val = readRowHeightElement(reader);
			else if(reader->name() == "column_width")
				ret_val = readColumnWidthElement(reader);
			else // unknown element
			{
				reader->raiseWarning(i18n("unknown element '%1'", reader->name().toString()));
				if (!reader->skipToEndElement()) return false;
			}
			if(!ret_val) return false;
		}
	}
	d->suppressDataChange = false;

	return true;
}

bool Matrix::readDisplayElement(XmlStreamReader* reader) {
	Q_ASSERT(reader->isStartElement() && reader->name() == "display");
	QXmlStreamAttributes attribs = reader->attributes();

	QString str = attribs.value(reader->namespaceUri().toString(), "numeric_format").toString();
	if(str.isEmpty() || str.length() != 1) {
		reader->raiseError(i18n("invalid or missing numeric format"));
		return false;
	}
	setNumericFormat(str.at(0).toAscii());

	bool ok;
	int digits = reader->readAttributeInt("displayed_digits", &ok);
	if(!ok) {
		reader->raiseError(i18n("invalid or missing number of displayed digits"));
		return false;
	}
	setPrecision(digits);
	if (!reader->skipToEndElement()) return false;

	return true;
}

bool Matrix::readCoordinatesElement(XmlStreamReader* reader) {
	Q_ASSERT(reader->isStartElement() && reader->name() == "coordinates");

	bool ok;
	int val;

	val = reader->readAttributeInt("x_start", &ok);
	if(!ok) {
		reader->raiseError(i18n("invalid x start value"));
		return false;
	}
	setXStart(val);

	val = reader->readAttributeInt("x_end", &ok);
	if(!ok) {
		reader->raiseError(i18n("invalid x end value"));
		return false;
	}
	setXEnd(val);

	val = reader->readAttributeInt("y_start", &ok);
	if(!ok) {
		reader->raiseError(i18n("invalid y start value"));
		return false;
	}
	setYStart(val);

	val = reader->readAttributeInt("y_end", &ok);
	if(!ok) {
		reader->raiseError(i18n("invalid y end value"));
		return false;
	}
	setYEnd(val);
	if (!reader->skipToEndElement()) return false;

	emit coordinatesChanged();//TODO: really required
	return true;
}

bool Matrix::readFormulaElement(XmlStreamReader* reader) {
	d->formula = reader->readElementText();
	return true;
}

bool Matrix::readRowHeightElement(XmlStreamReader* reader) {
	bool ok;
	int row = reader->readAttributeInt("row", &ok);
	if(!ok) {
		reader->raiseError(i18n("invalid or missing row index"));
		return false;
	}
	QString str = reader->readElementText();
	int value = str.toInt(&ok);
	if(!ok) {
		reader->raiseError(i18n("invalid row height"));
		return false;
	}
	if (m_view)
		m_view->setRowHeight(row, value);
	else
		setRowHeight(row, value);
	return true;
}

bool Matrix::readColumnWidthElement(XmlStreamReader* reader) {
	bool ok;
	int col = reader->readAttributeInt("column", &ok);
	if(!ok) {
		reader->raiseError(i18n("invalid or missing column index"));
		return false;
	}
	QString str = reader->readElementText();
	int value = str.toInt(&ok);
	if(!ok) {
		reader->raiseError(i18n("invalid column width"));
		return false;
	}
	if (m_view)
		m_view->setColumnWidth(col, value);
	else
		setColumnWidth(col, value);
	return true;
}

bool Matrix::readCellElement(XmlStreamReader* reader) {
	QString str;
	int row, col;
	bool ok;

	QXmlStreamAttributes attribs = reader->attributes();
	row = reader->readAttributeInt("row", &ok);
	if(!ok) {
		reader->raiseError(i18n("invalid or missing row index"));
		return false;
	}
	col = reader->readAttributeInt("column", &ok);
	if(!ok) {
		reader->raiseError(i18n("invalid or missing column index"));
		return false;
	}

	str = reader->readElementText();
	double value = str.toDouble(&ok);
	if(!ok) {
		reader->raiseError(i18n("invalid cell value"));
		return false;
	}
	setCell(row, col, value);

	return true;
}
