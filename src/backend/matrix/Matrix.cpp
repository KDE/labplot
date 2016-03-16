
/***************************************************************************
    File                 : Matrix.cpp
    Project              : Matrix
    Description          : Spreadsheet with a MxN matrix data model
    --------------------------------------------------------------------
    Copyright            : (C) 2015-2016 Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2008-2009 Tilman Benkert (thzs@gmx.net)

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
#include "backend/core/Folder.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/XmlStreamReader.h"
#include "commonfrontend/matrix/MatrixView.h"
#include "kdefrontend/spreadsheet/ExportSpreadsheetDialog.h"

#include <QHeaderView>
#include <QLocale>
#include <QPrinter>
#include <QPrintDialog>
#include <QPrintPreviewDialog>

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
	: AbstractDataSource(engine, name), d(new MatrixPrivate(this)), m_model(0) {

	//set initial number of rows and columns
	appendColumns(cols);
	appendRows(rows);

	init();

}

Matrix::Matrix(AbstractScriptingEngine* engine, const QString& name, bool loading)
	: AbstractDataSource(engine, name), d(new MatrixPrivate(this)), m_model(0) {

	if (!loading)
		init();
}

Matrix::~Matrix() {
	delete d;
}

void Matrix::init() {
	KConfig config;
	KConfigGroup group = config.group("Matrix");

	//matrix dimension
	int rows = group.readEntry("RowCount", 10);
	int cols = group.readEntry("ColumnCount", 10);
	appendRows(rows);
	appendColumns(cols);

	//mapping to logical x- and y-coordinates
	d->xStart = group.readEntry("XStart", 0.0);
	d->xEnd = group.readEntry("XEnd", 1.0);
	d->yStart = group.readEntry("YStart", 0.0);
	d->yEnd = group.readEntry("YEnd", 1.0);

	//format
	d->numericFormat = *group.readEntry("NumericFormat", "f").toLatin1().data();
	d->precision = group.readEntry("Precision", 3);
	d->headerFormat = (Matrix::HeaderFormat)group.readEntry("HeaderFormat", (int)Matrix::HeaderRowsColumns);
}

/*!
  Returns an icon to be used for decorating my views.
  */
QIcon Matrix::icon() const {
    return QIcon::fromTheme("labplot-matrix");
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
	if (!m_view) {
		MatrixView* view = new MatrixView(const_cast<Matrix*>(this));
		m_model = view->model();
		m_view = view;
	}
	return m_view;
}

void Matrix::exportView() const {
	ExportSpreadsheetDialog* dlg = new ExportSpreadsheetDialog(m_view);
	dlg->setFileName(name());
	dlg->setMatrixMode(true);
	if (dlg->exec()==QDialog::Accepted) {
		const QString path = dlg->path();
		const QString separator = dlg->separator();

		const MatrixView* view = reinterpret_cast<const MatrixView*>(m_view);
		WAIT_CURSOR;
		view->exportToFile(path, separator);
		RESET_CURSOR;
	}
	delete dlg;
}

void Matrix::printView() {
	QPrinter printer;
	QPrintDialog* dlg = new QPrintDialog(&printer, m_view);
	dlg->setWindowTitle(i18n("Print Matrix"));
	if (dlg->exec() == QDialog::Accepted) {
		const MatrixView* view = reinterpret_cast<const MatrixView*>(m_view);
		view->print(&printer);
	}
	delete dlg;
}

void Matrix::printPreview() const {
	const MatrixView* view = reinterpret_cast<const MatrixView*>(m_view);
	QPrintPreviewDialog* dlg = new QPrintPreviewDialog(m_view);
	connect(dlg, SIGNAL(paintRequested(QPrinter*)), view, SLOT(print(QPrinter*)));
	dlg->exec();
}

//##############################################################################
//##########################  getter methods  ##################################
//##############################################################################
BASIC_D_READER_IMPL(Matrix, int, columnCount, columnCount)
BASIC_D_READER_IMPL(Matrix, int, rowCount, rowCount)
BASIC_D_READER_IMPL(Matrix, double, xStart, xStart)
BASIC_D_READER_IMPL(Matrix, double, xEnd, xEnd)
BASIC_D_READER_IMPL(Matrix, double, yStart, yStart)
BASIC_D_READER_IMPL(Matrix, double, yEnd, yEnd)
BASIC_D_READER_IMPL(Matrix, char, numericFormat, numericFormat)
BASIC_D_READER_IMPL(Matrix, int, precision, precision)
BASIC_D_READER_IMPL(Matrix, Matrix::HeaderFormat, headerFormat, headerFormat)
CLASS_D_READER_IMPL(Matrix, QString, formula, formula)

QVector<QVector<double> >& Matrix::data() const {
	return d->matrixData;
}

void Matrix::setSuppressDataChangedSignal(bool b) {
	if (m_model)
		m_model->setSuppressDataChangedSignal(b);
}

void Matrix::setChanged() {
	if (m_model)
		m_model->setChanged();
}

int Matrix::defaultRowHeight() const {
	return d->defaultRowHeight;
}

int Matrix::defaultColumnWidth() const {
	return d->defaultRowHeight*3;
}

//##############################################################################
//#################  setter methods and undo commands ##########################
//##############################################################################
void Matrix::setRowCount(int count) {
	if (count == d->rowCount)
		return;

	int diff = count - d->rowCount;
	if(diff > 0)
		exec(new MatrixInsertRowsCmd(d, rowCount(), diff));
	else if(diff < 0)
		exec(new MatrixRemoveRowsCmd(d, rowCount()+diff, -diff));
}

void Matrix::setColumnCount(int count) {
	if (count == d->columnCount)
		return;

	int diff = count - columnCount();
	if(diff > 0)
		exec(new MatrixInsertColumnsCmd(d, columnCount(), diff));
	else if(diff < 0)
		exec(new MatrixRemoveColumnsCmd(d, columnCount()+diff, -diff));
}

STD_SETTER_CMD_IMPL_F_S(Matrix, SetXStart, double, xStart, updateViewHeader)
void Matrix::setXStart(double xStart) {
	if (xStart != d->xStart)
		exec(new MatrixSetXStartCmd(d, xStart, i18n("%1: x-start changed")));
}

STD_SETTER_CMD_IMPL_F_S(Matrix, SetXEnd, double, xEnd, updateViewHeader)
void Matrix::setXEnd(double xEnd) {
	if (xEnd != d->xEnd)
		exec(new MatrixSetXEndCmd(d, xEnd, i18n("%1: x-end changed")));
}

STD_SETTER_CMD_IMPL_F_S(Matrix, SetYStart, double, yStart, updateViewHeader)
void Matrix::setYStart(double yStart) {
	if (yStart != d->yStart)
		exec(new MatrixSetYStartCmd(d, yStart, i18n("%1: y-start changed")));
}

STD_SETTER_CMD_IMPL_F_S(Matrix, SetYEnd, double, yEnd, updateViewHeader)
void Matrix::setYEnd(double yEnd) {
	if (yEnd != d->yEnd)
		exec(new MatrixSetYEndCmd(d, yEnd, i18n("%1: y-end changed")));
}

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

//TODO: make this undoable?
void Matrix::setHeaderFormat(Matrix::HeaderFormat format) {
	d->headerFormat = format;
	m_model->updateHeader();

	if (m_view)
		(reinterpret_cast<MatrixView*>(m_view))->resizeHeaders();

	emit headerFormatChanged(format);
}

//columns
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

void Matrix::clearColumn(int c) {
	exec(new MatrixClearColumnCmd(d, c));
}

//rows
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

void Matrix::removeRows(int first, int count) {
	if( count < 1 || first < 0 || first+count > rowCount()) return;
	WAIT_CURSOR;
	beginMacro(i18np("%1: remove %2 row", "%1: remove %2 rows", name(), count));
	exec(new MatrixRemoveRowsCmd(d, first, count));
	endMacro();
	RESET_CURSOR;
}
void Matrix::clearRow(int r) {
	for(int c=0; c<columnCount(); ++c)
		exec(new MatrixSetCellValueCmd(d, r, c, 0.0));
}

//cell
double Matrix::cell(int row, int col) const {
	return d->cell(row, col);
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

void Matrix::clearCell(int row, int col) {
	exec(new MatrixSetCellValueCmd(d, row, col, 0.0));
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
	if (m_view) reinterpret_cast<MatrixView*>(m_view)->adjustHeaders();
	endMacro();
	RESET_CURSOR;
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
	int count = reinterpret_cast<MatrixView*>(m_view)->selectedRowCount(false);
	beginMacro(i18np("%1: add %2 rows", "%1: add %2 rows", name(), count));
	exec(new MatrixInsertRowsCmd(d, rowCount(), count));
	endMacro();
	RESET_CURSOR;
}

void Matrix::addColumns() {
	if (!m_view) return;
	WAIT_CURSOR;
	int count = reinterpret_cast<MatrixView*>(m_view)->selectedRowCount(false);
	beginMacro(i18np("%1: add %2 column", "%1: add %2 columns", name(), count));
	exec(new MatrixInsertColumnsCmd(d, columnCount(), count));
	endMacro();
	RESET_CURSOR;
}

void Matrix::setCoordinates(double x1, double x2, double y1, double y2) {
	exec(new MatrixSetCoordinatesCmd(d, x1, x2, y1, y2));
}

void Matrix::setFormula(const QString& formula) {
	exec(new MatrixSetFormulaCmd(d, formula));
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

void Matrix::setData(const QVector<QVector<double> >& data) {
	if (!data.isEmpty())
		exec(new MatrixReplaceValuesCmd(d, data));
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
	QFont font;
	font.setFamily(font.defaultFamily());
	QFontMetrics fm(font);
	defaultRowHeight = fm.height();
}

void MatrixPrivate::updateViewHeader() {
	reinterpret_cast<MatrixView*>(q->m_view)->model()->updateHeader();
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
		columnWidths.insert(before+i, q->defaultColumnWidth());
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
		columnWidths.remove(first);
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
		rowHeights.insert(before+i, defaultRowHeight);

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
		rowHeights.remove(first);

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
	writer->writeStartElement("matrix");
	writeBasicAttributes(writer);
	writeCommentElement(writer);

	//formula
	writer->writeStartElement("formula");
	writer->writeCharacters(d->formula);
	writer->writeEndElement();

	//format
	writer->writeStartElement("format");
	writer->writeAttribute("headerFormat", QString::number(d->headerFormat));
	writer->writeAttribute("numericFormat", QString(QChar(d->numericFormat)));
	writer->writeAttribute("precision", QString::number(d->precision));
	writer->writeEndElement();

	//dimensions
	writer->writeStartElement("dimension");
	writer->writeAttribute("columns", QString::number(d->columnCount));
	writer->writeAttribute("rows", QString::number(d->rowCount));
	writer->writeAttribute("x_start", QString::number(d->xStart));
	writer->writeAttribute("x_end", QString::number(d->xEnd));
	writer->writeAttribute("y_start", QString::number(d->yStart));
	writer->writeAttribute("y_end", QString::number(d->yEnd));
	writer->writeEndElement();

	//vector with row heights
	writer->writeStartElement("row_heights");
	const char* data = reinterpret_cast<const char*>(d->rowHeights.constData());
	int size = d->rowHeights.size()*sizeof(int);
	writer->writeCharacters(QByteArray::fromRawData(data,size).toBase64());
	writer->writeEndElement();

	//vector with column widths
	writer->writeStartElement("column_widths");
	data = reinterpret_cast<const char*>(d->columnWidths.constData());
	size = d->columnWidths.size()*sizeof(int);
	writer->writeCharacters(QByteArray::fromRawData(data,size).toBase64());
	writer->writeEndElement();

	//columns
	size = d->rowCount*sizeof(double);
	for (int i=0; i<d->columnCount; ++i) {
		data = reinterpret_cast<const char*>(d->matrixData.at(i).constData());
		writer->writeStartElement("column");
		writer->writeCharacters(QByteArray::fromRawData(data,size).toBase64());
		writer->writeEndElement();
	}

	writer->writeEndElement(); // "matrix"
}

bool Matrix::load(XmlStreamReader* reader) {
	if(!reader->isStartElement() || reader->name() != "matrix") {
		reader->raiseError(i18n("no matrix element found"));
		return false;
	}

	if (!readBasicAttributes(reader)) return false;

	QString attributeWarning = i18n("Attribute '%1' missing or empty, default value is used");
	QXmlStreamAttributes attribs;
	QString str;

	// read child elements
	while (!reader->atEnd()) {
		reader->readNext();

		if (reader->isEndElement() && reader->name() == "matrix")
			break;

		if (!reader->isStartElement())
			continue;


		if (reader->name() == "comment") {
			if (!readCommentElement(reader)) return false;
		} else if(reader->name() == "formula") {
			d->formula = reader->text().toString().trimmed();
		} else if (reader->name() == "format") {
			attribs = reader->attributes();

			str = attribs.value("headerFormat").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'headerFormat'"));
			else
				d->headerFormat = Matrix::HeaderFormat(str.toInt());

			str = attribs.value("numericFormat").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'numericFormat'"));
			else
				d->numericFormat = *str.toLatin1().data();

			str = attribs.value("precision").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'precision'"));
			else
				d->precision = str.toInt();

		} else if (reader->name() == "dimension") {
			attribs = reader->attributes();

			str = attribs.value("columns").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'columns'"));
			else
				d->columnCount = str.toInt();

			str = attribs.value("rows").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'rows'"));
			else
				d->rowCount = str.toInt();

			str = attribs.value("x_start").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'x_start'"));
			else
				d->xStart = str.toDouble();

			str = attribs.value("x_end").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'x_end'"));
			else
				d->xEnd = str.toDouble();

			str = attribs.value("y_start").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'y_start'"));
			else
				d->yStart = str.toDouble();

			str = attribs.value("y_end").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'y_end'"));
			else
				d->yEnd = str.toDouble();
		} else if (reader->name() == "row_heights") {
			reader->readNext();
			QString content = reader->text().toString().trimmed();
			QByteArray bytes = QByteArray::fromBase64(content.toAscii());
			int count = bytes.size()/sizeof(int);
			d->rowHeights.resize(count);
			memcpy(d->rowHeights.data(), bytes.data(), count*sizeof(int));
		} else if (reader->name() == "column_widths") {
			reader->readNext();
			QString content = reader->text().toString().trimmed();
			QByteArray bytes = QByteArray::fromBase64(content.toAscii());
			int count = bytes.size()/sizeof(int);
			d->columnWidths.resize(count);
			memcpy(d->columnWidths.data(), bytes.data(), count*sizeof(int));
		} else if (reader->name() == "column") {
			//TODO: parallelize reading of columns?
			reader->readNext();
			QString content = reader->text().toString().trimmed();
			QByteArray bytes = QByteArray::fromBase64(content.toAscii());
			int count = bytes.size()/sizeof(double);
			QVector<double> column;
			column.resize(count);
			memcpy(column.data(), bytes.data(), count*sizeof(double));
			d->matrixData.append(column);
		} else { // unknown element
			reader->raiseWarning(i18n("unknown element '%1'", reader->name().toString()));
			if (!reader->skipToEndElement())
				return false;
		}
	}

	return true;
}

