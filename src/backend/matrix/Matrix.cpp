/*
	File                 : Matrix.cpp
	Project              : Matrix
	Description          : Spreadsheet with a MxN matrix data model
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2008-2009 Tilman Benkert <thzs@gmx.net>
	SPDX-FileCopyrightText: 2015-2017 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2017-2020 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "Matrix.h"
#include "MatrixPrivate.h"
#include "backend/core/Folder.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/commandtemplates.h"
#include "backend/matrix/MatrixModel.h"
#include "commonfrontend/matrix/MatrixView.h"
#include "kdefrontend/spreadsheet/ExportSpreadsheetDialog.h"
#include "matrixcommands.h"

#include <KConfig>
#include <KLocalizedString>

#include <QHeaderView>
#include <QPrintDialog>
#include <QPrintPreviewDialog>
#include <QPrinter>

/*!
	This class manages matrix based data (i.e., mathematically
	a MxN matrix with M rows, N columns). This data is typically
	used to for 3D plots.

	The values of the matrix are stored as generic values. Each column
	of the matrix is stored in a QVector<T> objects.

	\ingroup backend
*/
Matrix::Matrix(int rows, int cols, const QString& name, const AbstractColumn::ColumnMode mode)
	: AbstractDataSource(name, AspectType::Matrix)
	, d(new MatrixPrivate(this, mode)) {
	// set initial number of rows and columns
	appendColumns(cols);
	appendRows(rows);

	init();
}

Matrix::Matrix(const QString& name, bool loading, const AbstractColumn::ColumnMode mode)
	: AbstractDataSource(name, AspectType::Matrix)
	, d(new MatrixPrivate(this, mode)) {
	if (!loading)
		init();
}

Matrix::~Matrix() {
	delete d;
}

void Matrix::init() {
	KConfig config;
	KConfigGroup group = config.group("Matrix");

	// matrix dimension
	int rows = group.readEntry("RowCount", 10);
	int cols = group.readEntry("ColumnCount", 10);
	appendRows(rows);
	appendColumns(cols);

	// mapping to logical x- and y-coordinates
	d->xStart = group.readEntry("XStart", 0.0);
	d->xEnd = group.readEntry("XEnd", 1.0);
	d->yStart = group.readEntry("YStart", 0.0);
	d->yEnd = group.readEntry("YEnd", 1.0);

	// format
	QByteArray formatba = group.readEntry("NumericFormat", "f").toLatin1();
	d->numericFormat = *formatba.data();
	d->precision = group.readEntry("Precision", 3);
	d->headerFormat = (Matrix::HeaderFormat)group.readEntry("HeaderFormat", static_cast<int>(HeaderFormat::HeaderRowsColumns));
}

/*!
  Returns an icon to be used for decorating my views.
  */
QIcon Matrix::icon() const {
	return QIcon::fromTheme(QStringLiteral("labplot-matrix"));
}

/*!
  Returns a new context menu. The caller takes ownership of the menu.
*/
QMenu* Matrix::createContextMenu() {
	QMenu* menu = AbstractPart::createContextMenu();
	Q_EMIT requestProjectContextMenu(menu);
	return menu;
}

QWidget* Matrix::view() const {
	if (!m_partView) {
		m_view = new MatrixView(const_cast<Matrix*>(this));
		m_partView = m_view;
		m_model = m_view->model();
	}
	return m_partView;
}

bool Matrix::exportView() const {
	auto* dlg = new ExportSpreadsheetDialog(m_view);
	dlg->setFileName(name());
	dlg->setMatrixMode(true);

	// TODO FITS filter to decide if it can be exported to both
	dlg->setExportTo(QStringList() << i18n("FITS image") << i18n("FITS table"));
	if (m_view->selectedColumnCount() == 0) {
		dlg->setExportSelection(false);
	}

	bool ret;
	if ((ret = (dlg->exec() == QDialog::Accepted))) {
		const QString path = dlg->path();
		WAIT_CURSOR;

		if (dlg->format() == ExportSpreadsheetDialog::Format::LaTeX) {
			const bool verticalHeader = dlg->matrixVerticalHeader();
			const bool horizontalHeader = dlg->matrixHorizontalHeader();
			const bool latexHeader = dlg->exportHeader();
			const bool gridLines = dlg->gridLines();
			const bool entire = dlg->entireSpreadheet();
			const bool captions = dlg->captions();
			m_view->exportToLaTeX(path, verticalHeader, horizontalHeader, latexHeader, gridLines, entire, captions);
		} else if (dlg->format() == ExportSpreadsheetDialog::Format::FITS) {
			const int exportTo = dlg->exportToFits();
			m_view->exportToFits(path, exportTo);
		} else {
			const QString separator = dlg->separator();
			const QLocale::Language format = dlg->numberFormat();
			m_view->exportToFile(path, separator, format);
		}
		RESET_CURSOR;
	}
	delete dlg;

	return ret;
}

bool Matrix::printView() {
	QPrinter printer;
	auto* dlg = new QPrintDialog(&printer, m_view);
	bool ret;
	dlg->setWindowTitle(i18nc("@title:window", "Print Matrix"));
	if ((ret = (dlg->exec() == QDialog::Accepted)))
		m_view->print(&printer);

	delete dlg;

	return ret;
}

bool Matrix::printPreview() const {
	auto* dlg = new QPrintPreviewDialog(m_view);
	connect(dlg, &QPrintPreviewDialog::paintRequested, m_view, &MatrixView::print);
	return dlg->exec();
}

//##############################################################################
//##########################  getter methods  ##################################
//##############################################################################
void* Matrix::data() const {
	return d->data;
}

BASIC_D_READER_IMPL(Matrix, AbstractColumn::ColumnMode, mode, mode)
BASIC_D_READER_IMPL(Matrix, int, rowCount, rowCount)
BASIC_D_READER_IMPL(Matrix, int, columnCount, columnCount)
BASIC_D_READER_IMPL(Matrix, double, xStart, xStart)
BASIC_D_READER_IMPL(Matrix, double, xEnd, xEnd)
BASIC_D_READER_IMPL(Matrix, double, yStart, yStart)
BASIC_D_READER_IMPL(Matrix, double, yEnd, yEnd)
BASIC_D_READER_IMPL(Matrix, char, numericFormat, numericFormat)
BASIC_D_READER_IMPL(Matrix, int, precision, precision)
BASIC_D_READER_IMPL(Matrix, Matrix::HeaderFormat, headerFormat, headerFormat)
BASIC_D_READER_IMPL(Matrix, QString, formula, formula)

void Matrix::setSuppressDataChangedSignal(bool b) {
	if (m_model)
		m_model->setSuppressDataChangedSignal(b);
}

void Matrix::setChanged() {
	if (m_model)
		m_model->setChanged();
}

//##############################################################################
//#################  setter methods and undo commands ##########################
//##############################################################################
void Matrix::setRowCount(int count) {
	if (count == d->rowCount)
		return;

	const int diff = count - d->rowCount;
	if (diff > 0)
		appendRows(diff);
	else if (diff < 0)
		removeRows(rowCount() + diff, -diff);
}

void Matrix::setColumnCount(int count) {
	if (count == d->columnCount)
		return;

	const int diff = count - columnCount();
	if (diff > 0)
		appendColumns(diff);
	else if (diff < 0)
		removeColumns(columnCount() + diff, -diff);
}

STD_SETTER_CMD_IMPL_F_S(Matrix, SetXStart, double, xStart, updateViewHeader)
void Matrix::setXStart(double xStart) {
	if (xStart != d->xStart)
		exec(new MatrixSetXStartCmd(d, xStart, ki18n("%1: x-start changed")));
}

STD_SETTER_CMD_IMPL_F_S(Matrix, SetXEnd, double, xEnd, updateViewHeader)
void Matrix::setXEnd(double xEnd) {
	if (xEnd != d->xEnd)
		exec(new MatrixSetXEndCmd(d, xEnd, ki18n("%1: x-end changed")));
}

STD_SETTER_CMD_IMPL_F_S(Matrix, SetYStart, double, yStart, updateViewHeader)
void Matrix::setYStart(double yStart) {
	if (yStart != d->yStart)
		exec(new MatrixSetYStartCmd(d, yStart, ki18n("%1: y-start changed")));
}

STD_SETTER_CMD_IMPL_F_S(Matrix, SetYEnd, double, yEnd, updateViewHeader)
void Matrix::setYEnd(double yEnd) {
	if (yEnd != d->yEnd)
		exec(new MatrixSetYEndCmd(d, yEnd, ki18n("%1: y-end changed")));
}

STD_SETTER_CMD_IMPL_S(Matrix, SetNumericFormat, char, numericFormat)
void Matrix::setNumericFormat(char format) {
	if (format != d->numericFormat)
		exec(new MatrixSetNumericFormatCmd(d, format, ki18n("%1: numeric format changed")));
}

STD_SETTER_CMD_IMPL_S(Matrix, SetPrecision, int, precision)
void Matrix::setPrecision(int precision) {
	if (precision != d->precision)
		exec(new MatrixSetPrecisionCmd(d, precision, ki18n("%1: precision changed")));
}

// TODO: make this undoable?
void Matrix::setHeaderFormat(Matrix::HeaderFormat format) {
	d->headerFormat = format;
	m_model->updateHeader();

	if (m_view)
		m_view->resizeHeaders();

	Q_EMIT headerFormatChanged(format);
}

// columns
void Matrix::insertColumns(int before, int count) {
	if (count < 1 || before < 0 || before > columnCount())
		return;
	WAIT_CURSOR;
	exec(new MatrixInsertColumnsCmd(d, before, count));
	RESET_CURSOR;
}

void Matrix::appendColumns(int count) {
	insertColumns(columnCount(), count);
}

void Matrix::removeColumns(int first, int count) {
	if (count < 1 || first < 0 || first + count > columnCount())
		return;
	WAIT_CURSOR;
	switch (d->mode) {
	case AbstractColumn::ColumnMode::Double:
		exec(new MatrixRemoveColumnsCmd<double>(d, first, count));
		break;
	case AbstractColumn::ColumnMode::Text:
		exec(new MatrixRemoveColumnsCmd<QString>(d, first, count));
		break;
	case AbstractColumn::ColumnMode::Integer:
		exec(new MatrixRemoveColumnsCmd<int>(d, first, count));
		break;
	case AbstractColumn::ColumnMode::BigInt:
		exec(new MatrixRemoveColumnsCmd<qint64>(d, first, count));
		break;
	case AbstractColumn::ColumnMode::Day:
	case AbstractColumn::ColumnMode::Month:
	case AbstractColumn::ColumnMode::DateTime:
		exec(new MatrixRemoveColumnsCmd<QDateTime>(d, first, count));
		break;
	}
	RESET_CURSOR;
}

void Matrix::clearColumn(int c) {
	WAIT_CURSOR;
	switch (d->mode) {
	case AbstractColumn::ColumnMode::Double:
		exec(new MatrixClearColumnCmd<double>(d, c));
		break;
	case AbstractColumn::ColumnMode::Text:
		exec(new MatrixClearColumnCmd<QString>(d, c));
		break;
	case AbstractColumn::ColumnMode::Integer:
		exec(new MatrixClearColumnCmd<int>(d, c));
		break;
	case AbstractColumn::ColumnMode::BigInt:
		exec(new MatrixClearColumnCmd<qint64>(d, c));
		break;
	case AbstractColumn::ColumnMode::Day:
	case AbstractColumn::ColumnMode::Month:
	case AbstractColumn::ColumnMode::DateTime:
		exec(new MatrixClearColumnCmd<QDateTime>(d, c));
		break;
	}
	RESET_CURSOR;
}

// rows
void Matrix::insertRows(int before, int count) {
	if (count < 1 || before < 0 || before > rowCount())
		return;
	WAIT_CURSOR;
	exec(new MatrixInsertRowsCmd(d, before, count));
	RESET_CURSOR;
}

void Matrix::appendRows(int count) {
	insertRows(rowCount(), count);
}

void Matrix::removeRows(int first, int count) {
	if (count < 1 || first < 0 || first + count > rowCount())
		return;
	WAIT_CURSOR;
	switch (d->mode) {
	case AbstractColumn::ColumnMode::Double:
		exec(new MatrixRemoveRowsCmd<double>(d, first, count));
		break;
	case AbstractColumn::ColumnMode::Text:
		exec(new MatrixRemoveRowsCmd<QString>(d, first, count));
		break;
	case AbstractColumn::ColumnMode::Integer:
		exec(new MatrixRemoveRowsCmd<int>(d, first, count));
		break;
	case AbstractColumn::ColumnMode::BigInt:
		exec(new MatrixRemoveRowsCmd<qint64>(d, first, count));
		break;
	case AbstractColumn::ColumnMode::Day:
	case AbstractColumn::ColumnMode::Month:
	case AbstractColumn::ColumnMode::DateTime:
		exec(new MatrixRemoveRowsCmd<QDateTime>(d, first, count));
		break;
	}
	RESET_CURSOR;
}

void Matrix::clearRow(int r) {
	switch (d->mode) {
	case AbstractColumn::ColumnMode::Double:
		for (int c = 0; c < columnCount(); ++c)
			exec(new MatrixSetCellValueCmd<double>(d, r, c, 0.0));
		break;
	case AbstractColumn::ColumnMode::Text:
		for (int c = 0; c < columnCount(); ++c)
			exec(new MatrixSetCellValueCmd<QString>(d, r, c, QString()));
		break;
	case AbstractColumn::ColumnMode::Integer:
		for (int c = 0; c < columnCount(); ++c)
			exec(new MatrixSetCellValueCmd<int>(d, r, c, 0));
		break;
	case AbstractColumn::ColumnMode::BigInt:
		for (int c = 0; c < columnCount(); ++c)
			exec(new MatrixSetCellValueCmd<qint64>(d, r, c, 0));
		break;
	case AbstractColumn::ColumnMode::Day:
	case AbstractColumn::ColumnMode::Month:
	case AbstractColumn::ColumnMode::DateTime:
		for (int c = 0; c < columnCount(); ++c)
			exec(new MatrixSetCellValueCmd<QDateTime>(d, r, c, QDateTime()));
		break;
	}
}

//! Return the value in the given cell (needs explicit instantiation)
template<typename T>
T Matrix::cell(int row, int col) const {
	return d->cell<T>(row, col);
}
template double Matrix::cell<double>(int row, int col) const;
template int Matrix::cell<int>(int row, int col) const;
template qint64 Matrix::cell<qint64>(int row, int col) const;
template QDateTime Matrix::cell<QDateTime>(int row, int col) const;
template QString Matrix::cell<QString>(int row, int col) const;

//! Return the text displayed in the given cell (needs explicit instantiation)
template<typename T>
QString Matrix::text(int row, int col) {
	SET_NUMBER_LOCALE
	return numberLocale.toString(cell<T>(row, col));
}
template<>
QString Matrix::text<double>(int row, int col) {
	SET_NUMBER_LOCALE
	return numberLocale.toString(cell<double>(row, col), d->numericFormat, d->precision);
}
template<>
QString Matrix::text<QString>(int row, int col) {
	return cell<QString>(row, col);
}
template QString Matrix::text<int>(int row, int col);
template QString Matrix::text<qint64>(int row, int col);
template QString Matrix::text<QDateTime>(int row, int col);

//! Set the value of the cell (needs explicit instantiation)
template<typename T>
void Matrix::setCell(int row, int col, T value) {
	if (row < 0 || row >= rowCount())
		return;
	if (col < 0 || col >= columnCount())
		return;
	exec(new MatrixSetCellValueCmd<T>(d, row, col, value));
}
template void Matrix::setCell<double>(int row, int col, double value);
template void Matrix::setCell<int>(int row, int col, int value);
template void Matrix::setCell<qint64>(int row, int col, qint64 value);
template void Matrix::setCell<QString>(int row, int col, QString value);
template void Matrix::setCell<QDateTime>(int row, int col, QDateTime value);

void Matrix::clearCell(int row, int col) {
	switch (d->mode) {
	case AbstractColumn::ColumnMode::Double:
		exec(new MatrixSetCellValueCmd<double>(d, row, col, 0.0));
		break;
	case AbstractColumn::ColumnMode::Text:
		exec(new MatrixSetCellValueCmd<QString>(d, row, col, QString()));
		break;
	case AbstractColumn::ColumnMode::Integer:
		exec(new MatrixSetCellValueCmd<int>(d, row, col, 0));
		break;
	case AbstractColumn::ColumnMode::BigInt:
		exec(new MatrixSetCellValueCmd<qint64>(d, row, col, 0));
		break;
	case AbstractColumn::ColumnMode::Day:
	case AbstractColumn::ColumnMode::Month:
	case AbstractColumn::ColumnMode::DateTime:
		exec(new MatrixSetCellValueCmd<QDateTime>(d, row, col, QDateTime()));
		break;
	}
}

void Matrix::setDimensions(int rows, int cols) {
	if ((rows < 0) || (cols < 0) || (rows == rowCount() && cols == columnCount()))
		return;

	WAIT_CURSOR;
	beginMacro(i18n("%1: set matrix size to %2x%3", name(), rows, cols));

	int col_diff = cols - columnCount();
	if (col_diff > 0)
		insertColumns(columnCount(), col_diff);
	else if (col_diff < 0)
		removeColumns(columnCount() + col_diff, -col_diff);

	int row_diff = rows - rowCount();
	if (row_diff > 0)
		appendRows(row_diff);
	else if (row_diff < 0)
		removeRows(rowCount() + row_diff, -row_diff);

	endMacro();
	RESET_CURSOR;
}

void Matrix::copy(Matrix* other) {
	WAIT_CURSOR;
	beginMacro(i18n("%1: copy %2", name(), other->name()));

	int rows = other->rowCount();
	int columns = other->columnCount();
	setDimensions(rows, columns);

	for (int i = 0; i < rows; i++)
		setRowHeight(i, other->rowHeight(i));

	for (int i = 0; i < columns; i++)
		setColumnWidth(i, other->columnWidth(i));

	d->suppressDataChange = true;
	switch (d->mode) {
	case AbstractColumn::ColumnMode::Double:
		for (int i = 0; i < columns; i++)
			setColumnCells(i, 0, rows - 1, other->columnCells<double>(i, 0, rows - 1));
		break;
	case AbstractColumn::ColumnMode::Text:
		for (int i = 0; i < columns; i++)
			setColumnCells(i, 0, rows - 1, other->columnCells<QString>(i, 0, rows - 1));
		break;
	case AbstractColumn::ColumnMode::Integer:
		for (int i = 0; i < columns; i++)
			setColumnCells(i, 0, rows - 1, other->columnCells<int>(i, 0, rows - 1));
		break;
	case AbstractColumn::ColumnMode::BigInt:
		for (int i = 0; i < columns; i++)
			setColumnCells(i, 0, rows - 1, other->columnCells<qint64>(i, 0, rows - 1));
		break;
	case AbstractColumn::ColumnMode::Day:
	case AbstractColumn::ColumnMode::Month:
	case AbstractColumn::ColumnMode::DateTime:
		for (int i = 0; i < columns; i++)
			setColumnCells(i, 0, rows - 1, other->columnCells<QDateTime>(i, 0, rows - 1));
		break;
	}

	setCoordinates(other->xStart(), other->xEnd(), other->yStart(), other->yEnd());
	setNumericFormat(other->numericFormat());
	setPrecision(other->precision());
	d->formula = other->formula();
	d->suppressDataChange = false;
	Q_EMIT dataChanged(0, 0, rows - 1, columns - 1);
	if (m_view)
		m_view->adjustHeaders();

	endMacro();
	RESET_CURSOR;
}

//! Duplicate the matrix inside its folder
void Matrix::duplicate() {
	Matrix* matrix = new Matrix(rowCount(), columnCount(), name());
	matrix->copy(this);
	if (folder())
		folder()->addChild(matrix);
}

void Matrix::addRows() {
	if (!m_view)
		return;
	WAIT_CURSOR;
	int count = m_view->selectedRowCount(false);
	beginMacro(i18np("%1: add %2 rows", "%1: add %2 rows", name(), count));
	exec(new MatrixInsertRowsCmd(d, rowCount(), count));
	endMacro();
	RESET_CURSOR;
}

void Matrix::addColumns() {
	if (!m_view)
		return;
	WAIT_CURSOR;
	int count = m_view->selectedRowCount(false);
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

//! Return the values in the given cells as vector
template<typename T>
QVector<T> Matrix::columnCells(int col, int first_row, int last_row) {
	return d->columnCells<T>(col, first_row, last_row);
}

//! Set the values in the given cells from a type T vector
template<typename T>
void Matrix::setColumnCells(int col, int first_row, int last_row, const QVector<T>& values) {
	WAIT_CURSOR;
	exec(new MatrixSetColumnCellsCmd<T>(d, col, first_row, last_row, values));
	RESET_CURSOR;
}

//! Return the values in the given cells as vector (needs explicit instantiation)
template<typename T>
QVector<T> Matrix::rowCells(int row, int first_column, int last_column) {
	return d->rowCells<T>(row, first_column, last_column);
}
template QVector<double> Matrix::rowCells<double>(int row, int first_column, int last_column);
template QVector<QString> Matrix::rowCells<QString>(int row, int first_column, int last_column);
template QVector<int> Matrix::rowCells<int>(int row, int first_column, int last_column);
template QVector<QDateTime> Matrix::rowCells<QDateTime>(int row, int first_column, int last_column);

//! Set the values in the given cells from a type T vector
template<typename T>
void Matrix::setRowCells(int row, int first_column, int last_column, const QVector<T>& values) {
	WAIT_CURSOR;
	exec(new MatrixSetRowCellsCmd<T>(d, row, first_column, last_column, values));
	RESET_CURSOR;
}

void Matrix::setData(void* data) {
	bool isEmpty = false;

	switch (d->mode) {
	case AbstractColumn::ColumnMode::Double:
		if (static_cast<QVector<QVector<double>>*>(data)->isEmpty())
			isEmpty = true;
		break;
	case AbstractColumn::ColumnMode::Text:
		if (static_cast<QVector<QVector<QString>>*>(data)->isEmpty())
			isEmpty = true;
		break;
	case AbstractColumn::ColumnMode::Integer:
		if (static_cast<QVector<QVector<int>>*>(data)->isEmpty())
			isEmpty = true;
		break;
	case AbstractColumn::ColumnMode::BigInt:
		if (static_cast<QVector<QVector<qint64>>*>(data)->isEmpty())
			isEmpty = true;
		break;
	case AbstractColumn::ColumnMode::Day:
	case AbstractColumn::ColumnMode::Month:
	case AbstractColumn::ColumnMode::DateTime:
		if (static_cast<QVector<QVector<QDateTime>>*>(data)->isEmpty())
			isEmpty = true;
		break;
	}

	if (!isEmpty)
		exec(new MatrixReplaceValuesCmd(d, data));
}

QVector<AspectType> Matrix::dropableOn() const {
	auto vec = AbstractPart::dropableOn();
	vec << AspectType::Workbook;
	return vec;
}

//##############################################################################
//#########################  Public slots  #####################################
//##############################################################################
//! Clear the whole matrix (i.e. reset all cells)
void Matrix::clear() {
	WAIT_CURSOR;
	beginMacro(i18n("%1: clear", name()));
	switch (d->mode) {
	case AbstractColumn::ColumnMode::Double:
		exec(new MatrixClearCmd<double>(d));
		break;
	case AbstractColumn::ColumnMode::Text:
		exec(new MatrixClearCmd<QString>(d));
		break;
	case AbstractColumn::ColumnMode::Integer:
		exec(new MatrixClearCmd<int>(d));
		break;
	case AbstractColumn::ColumnMode::BigInt:
		exec(new MatrixClearCmd<qint64>(d));
		break;
	case AbstractColumn::ColumnMode::Day:
	case AbstractColumn::ColumnMode::Month:
	case AbstractColumn::ColumnMode::DateTime:
		exec(new MatrixClearCmd<QDateTime>(d));
		break;
	}
	endMacro();
	RESET_CURSOR;
}

void Matrix::transpose() {
	WAIT_CURSOR;
	switch (d->mode) {
	case AbstractColumn::ColumnMode::Double:
		exec(new MatrixTransposeCmd<double>(d));
		break;
	case AbstractColumn::ColumnMode::Text:
		exec(new MatrixTransposeCmd<QString>(d));
		break;
	case AbstractColumn::ColumnMode::Integer:
		exec(new MatrixTransposeCmd<int>(d));
		break;
	case AbstractColumn::ColumnMode::BigInt:
		exec(new MatrixTransposeCmd<qint64>(d));
		break;
	case AbstractColumn::ColumnMode::Day:
	case AbstractColumn::ColumnMode::Month:
	case AbstractColumn::ColumnMode::DateTime:
		exec(new MatrixTransposeCmd<QDateTime>(d));
		break;
	}
	RESET_CURSOR;
}

void Matrix::mirrorHorizontally() {
	WAIT_CURSOR;
	switch (d->mode) {
	case AbstractColumn::ColumnMode::Double:
		exec(new MatrixMirrorHorizontallyCmd<double>(d));
		break;
	case AbstractColumn::ColumnMode::Text:
		exec(new MatrixMirrorHorizontallyCmd<QString>(d));
		break;
	case AbstractColumn::ColumnMode::Integer:
		exec(new MatrixMirrorHorizontallyCmd<int>(d));
		break;
	case AbstractColumn::ColumnMode::BigInt:
		exec(new MatrixMirrorHorizontallyCmd<qint64>(d));
		break;
	case AbstractColumn::ColumnMode::Day:
	case AbstractColumn::ColumnMode::Month:
	case AbstractColumn::ColumnMode::DateTime:
		exec(new MatrixMirrorHorizontallyCmd<QDateTime>(d));
		break;
	}
	RESET_CURSOR;
}

void Matrix::mirrorVertically() {
	WAIT_CURSOR;
	switch (d->mode) {
	case AbstractColumn::ColumnMode::Double:
		exec(new MatrixMirrorVerticallyCmd<double>(d));
		break;
	case AbstractColumn::ColumnMode::Text:
		exec(new MatrixMirrorVerticallyCmd<QString>(d));
		break;
	case AbstractColumn::ColumnMode::Integer:
		exec(new MatrixMirrorVerticallyCmd<int>(d));
		break;
	case AbstractColumn::ColumnMode::BigInt:
		exec(new MatrixMirrorVerticallyCmd<qint64>(d));
		break;
	case AbstractColumn::ColumnMode::Day:
	case AbstractColumn::ColumnMode::Month:
	case AbstractColumn::ColumnMode::DateTime:
		exec(new MatrixMirrorVerticallyCmd<QDateTime>(d));
		break;
	}
	RESET_CURSOR;
}

//##############################################################################
//######################  Private implementation ###############################
//##############################################################################

MatrixPrivate::MatrixPrivate(Matrix* owner, const AbstractColumn::ColumnMode m)
	: q(owner)
	, data(nullptr)
	, mode(m)
	, rowCount(0)
	, columnCount(0)
	, suppressDataChange(false) {
	switch (mode) {
	case AbstractColumn::ColumnMode::Double:
		data = new QVector<QVector<double>>();
		break;
	case AbstractColumn::ColumnMode::Text:
		data = new QVector<QVector<QString>>();
		break;
	case AbstractColumn::ColumnMode::Integer:
		data = new QVector<QVector<int>>();
		break;
	case AbstractColumn::ColumnMode::BigInt:
		data = new QVector<QVector<qint64>>();
		break;
	case AbstractColumn::ColumnMode::Month:
	case AbstractColumn::ColumnMode::Day:
	case AbstractColumn::ColumnMode::DateTime:
		data = new QVector<QVector<QDateTime>>();
		break;
	}
}

MatrixPrivate::~MatrixPrivate() {
	if (data) {
		switch (mode) {
		case AbstractColumn::ColumnMode::Double:
			delete static_cast<QVector<QVector<double>>*>(data);
			break;
		case AbstractColumn::ColumnMode::Text:
			delete static_cast<QVector<QVector<QString>>*>(data);
			break;
		case AbstractColumn::ColumnMode::Integer:
			delete static_cast<QVector<QVector<int>>*>(data);
			break;
		case AbstractColumn::ColumnMode::BigInt:
			delete static_cast<QVector<QVector<qint64>>*>(data);
			break;
		case AbstractColumn::ColumnMode::Day:
		case AbstractColumn::ColumnMode::Month:
		case AbstractColumn::ColumnMode::DateTime:
			delete static_cast<QVector<QVector<QDateTime>>*>(data);
			break;
		}
	}
}

void MatrixPrivate::updateViewHeader() {
	q->m_view->model()->updateHeader();
}

/*!
	Insert \p count columns before column number \c before
*/
void MatrixPrivate::insertColumns(int before, int count) {
	Q_ASSERT(before >= 0);
	Q_ASSERT(before <= columnCount);

	Q_EMIT q->columnsAboutToBeInserted(before, count);
	switch (mode) {
	case AbstractColumn::ColumnMode::Double:
		for (int i = 0; i < count; i++) {
			static_cast<QVector<QVector<double>>*>(data)->insert(before + i, QVector<double>(rowCount));
			columnWidths.insert(before + i, 0);
		}
		break;
	case AbstractColumn::ColumnMode::Text:
		for (int i = 0; i < count; i++) {
			static_cast<QVector<QVector<QString>>*>(data)->insert(before + i, QVector<QString>(rowCount));
			columnWidths.insert(before + i, 0);
		}
		break;
	case AbstractColumn::ColumnMode::Integer:
		for (int i = 0; i < count; i++) {
			static_cast<QVector<QVector<int>>*>(data)->insert(before + i, QVector<int>(rowCount));
			columnWidths.insert(before + i, 0);
		}
		break;
	case AbstractColumn::ColumnMode::BigInt:
		for (int i = 0; i < count; i++) {
			static_cast<QVector<QVector<qint64>>*>(data)->insert(before + i, QVector<qint64>(rowCount));
			columnWidths.insert(before + i, 0);
		}
		break;
	case AbstractColumn::ColumnMode::Day:
	case AbstractColumn::ColumnMode::Month:
	case AbstractColumn::ColumnMode::DateTime:
		for (int i = 0; i < count; i++) {
			static_cast<QVector<QVector<QDateTime>>*>(data)->insert(before + i, QVector<QDateTime>(rowCount));
			columnWidths.insert(before + i, 0);
		}
		break;
	}

	columnCount += count;
	Q_EMIT q->columnsInserted(before, count);
}

/*!
	Remove \c count columns starting with column index \c first
*/
void MatrixPrivate::removeColumns(int first, int count) {
	Q_EMIT q->columnsAboutToBeRemoved(first, count);
	Q_ASSERT(first >= 0);
	Q_ASSERT(first + count <= columnCount);

	switch (mode) {
	case AbstractColumn::ColumnMode::Double:
		(static_cast<QVector<QVector<double>>*>(data))->remove(first, count);
		break;
	case AbstractColumn::ColumnMode::Text:
		(static_cast<QVector<QVector<QString>>*>(data))->remove(first, count);
		break;
	case AbstractColumn::ColumnMode::Integer:
		(static_cast<QVector<QVector<int>>*>(data))->remove(first, count);
		break;
	case AbstractColumn::ColumnMode::BigInt:
		(static_cast<QVector<QVector<qint64>>*>(data))->remove(first, count);
		break;
	case AbstractColumn::ColumnMode::Day:
	case AbstractColumn::ColumnMode::Month:
	case AbstractColumn::ColumnMode::DateTime:
		(static_cast<QVector<QVector<QDateTime>>*>(data))->remove(first, count);
		break;
	}

	for (int i = 0; i < count; i++)
		columnWidths.remove(first);
	columnCount -= count;
	Q_EMIT q->columnsRemoved(first, count);
}

/*!
	Insert \c count rows before row with the index \c before
*/
void MatrixPrivate::insertRows(int before, int count) {
	Q_EMIT q->rowsAboutToBeInserted(before, count);
	Q_ASSERT(before >= 0);
	Q_ASSERT(before <= rowCount);

	switch (mode) {
	case AbstractColumn::ColumnMode::Double:
		for (int col = 0; col < columnCount; col++)
			for (int i = 0; i < count; i++)
				(static_cast<QVector<QVector<double>>*>(data))->operator[](col).insert(before + i, 0.0);
		break;
	case AbstractColumn::ColumnMode::Text:
		for (int col = 0; col < columnCount; col++)
			for (int i = 0; i < count; i++)
				(static_cast<QVector<QVector<QString>>*>(data))->operator[](col).insert(before + i, QString());
		break;
	case AbstractColumn::ColumnMode::Integer:
		for (int col = 0; col < columnCount; col++)
			for (int i = 0; i < count; i++)
				(static_cast<QVector<QVector<int>>*>(data))->operator[](col).insert(before + i, 0);
		break;
	case AbstractColumn::ColumnMode::BigInt:
		for (int col = 0; col < columnCount; col++)
			for (int i = 0; i < count; i++)
				(static_cast<QVector<QVector<qint64>>*>(data))->operator[](col).insert(before + i, 0);
		break;
	case AbstractColumn::ColumnMode::Day:
	case AbstractColumn::ColumnMode::Month:
	case AbstractColumn::ColumnMode::DateTime:
		for (int col = 0; col < columnCount; col++)
			for (int i = 0; i < count; i++)
				(static_cast<QVector<QVector<QDateTime>>*>(data))->operator[](col).insert(before + i, QDateTime());
	}

	for (int i = 0; i < count; i++)
		rowHeights.insert(before + i, 0);

	rowCount += count;
	Q_EMIT q->rowsInserted(before, count);
}

/*!
	Remove \c count columns starting from the column with index \c first
*/
void MatrixPrivate::removeRows(int first, int count) {
	Q_EMIT q->rowsAboutToBeRemoved(first, count);
	Q_ASSERT(first >= 0);
	Q_ASSERT(first + count <= rowCount);

	switch (mode) {
	case AbstractColumn::ColumnMode::Double:
		for (int col = 0; col < columnCount; col++)
			(static_cast<QVector<QVector<double>>*>(data))->operator[](col).remove(first, count);
		break;
	case AbstractColumn::ColumnMode::Text:
		for (int col = 0; col < columnCount; col++)
			(static_cast<QVector<QVector<QString>>*>(data))->operator[](col).remove(first, count);
		break;
	case AbstractColumn::ColumnMode::Integer:
		for (int col = 0; col < columnCount; col++)
			(static_cast<QVector<QVector<int>>*>(data))->operator[](col).remove(first, count);
		break;
	case AbstractColumn::ColumnMode::BigInt:
		for (int col = 0; col < columnCount; col++)
			(static_cast<QVector<QVector<qint64>>*>(data))->operator[](col).remove(first, count);
		break;
	case AbstractColumn::ColumnMode::Day:
	case AbstractColumn::ColumnMode::Month:
	case AbstractColumn::ColumnMode::DateTime:
		for (int col = 0; col < columnCount; col++)
			(static_cast<QVector<QVector<QDateTime>>*>(data))->operator[](col).remove(first, count);
		break;
	}

	for (int i = 0; i < count; i++)
		rowHeights.remove(first);

	rowCount -= count;
	Q_EMIT q->rowsRemoved(first, count);
}

//! Fill column with zeroes
void MatrixPrivate::clearColumn(int col) {
	switch (mode) {
	case AbstractColumn::ColumnMode::Double:
		static_cast<QVector<QVector<double>>*>(data)->operator[](col).fill(0.0);
		break;
	case AbstractColumn::ColumnMode::Text:
		static_cast<QVector<QVector<QString>>*>(data)->operator[](col).fill(QString());
		break;
	case AbstractColumn::ColumnMode::Integer:
		static_cast<QVector<QVector<int>>*>(data)->operator[](col).fill(0);
		break;
	case AbstractColumn::ColumnMode::BigInt:
		static_cast<QVector<QVector<qint64>>*>(data)->operator[](col).fill(0);
		break;
	case AbstractColumn::ColumnMode::Day:
	case AbstractColumn::ColumnMode::Month:
	case AbstractColumn::ColumnMode::DateTime:
		static_cast<QVector<QVector<QDateTime>>*>(data)->operator[](col).fill(QDateTime());
		break;
	}

	if (!suppressDataChange)
		Q_EMIT q->dataChanged(0, col, rowCount - 1, col);
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################
void Matrix::save(QXmlStreamWriter* writer) const {
	DEBUG(Q_FUNC_INFO);
	writer->writeStartElement(QStringLiteral("matrix"));
	writeBasicAttributes(writer);
	writeCommentElement(writer);

	// formula
	writer->writeStartElement(QStringLiteral("formula"));
	writer->writeCharacters(d->formula);
	writer->writeEndElement();

	// format
	writer->writeStartElement(QStringLiteral("format"));
	writer->writeAttribute(QStringLiteral("mode"), QString::number(static_cast<int>(d->mode)));
	writer->writeAttribute(QStringLiteral("headerFormat"), QString::number(static_cast<int>(d->headerFormat)));
	writer->writeAttribute(QStringLiteral("numericFormat"), QChar::fromLatin1(d->numericFormat));
	writer->writeAttribute(QStringLiteral("precision"), QString::number(d->precision));
	writer->writeEndElement();

	// dimensions
	writer->writeStartElement(QStringLiteral("dimension"));
	writer->writeAttribute(QStringLiteral("columns"), QString::number(d->columnCount));
	writer->writeAttribute(QStringLiteral("rows"), QString::number(d->rowCount));
	writer->writeAttribute(QStringLiteral("x_start"), QString::number(d->xStart));
	writer->writeAttribute(QStringLiteral("x_end"), QString::number(d->xEnd));
	writer->writeAttribute(QStringLiteral("y_start"), QString::number(d->yStart));
	writer->writeAttribute(QStringLiteral("y_end"), QString::number(d->yEnd));
	writer->writeEndElement();

	// vector with row heights
	writer->writeStartElement(QStringLiteral("row_heights"));
	const char* data = reinterpret_cast<const char*>(d->rowHeights.constData());
	int size = d->rowHeights.size() * sizeof(int);
	writer->writeCharacters(QLatin1String(QByteArray::fromRawData(data, size).toBase64()));
	writer->writeEndElement();

	// vector with column widths
	writer->writeStartElement(QStringLiteral("column_widths"));
	data = reinterpret_cast<const char*>(d->columnWidths.constData());
	size = d->columnWidths.size() * sizeof(int);
	writer->writeCharacters(QLatin1String(QByteArray::fromRawData(data, size).toBase64()));
	writer->writeEndElement();

	// columns
	DEBUG("	mode = " << static_cast<int>(d->mode))
	switch (d->mode) {
	case AbstractColumn::ColumnMode::Double:
		size = d->rowCount * sizeof(double);
		for (int i = 0; i < d->columnCount; ++i) {
			data = reinterpret_cast<const char*>(static_cast<QVector<QVector<double>>*>(d->data)->at(i).constData());
			writer->writeStartElement(QStringLiteral("column"));
			writer->writeCharacters(QLatin1String(QByteArray::fromRawData(data, size).toBase64()));
			writer->writeEndElement();
		}
		break;
	case AbstractColumn::ColumnMode::Text:
		size = d->rowCount * sizeof(QString);
		for (int i = 0; i < d->columnCount; ++i) {
			QDEBUG("	string: " << static_cast<QVector<QVector<QString>>*>(d->data)->at(i));
			data = reinterpret_cast<const char*>(static_cast<QVector<QVector<QString>>*>(d->data)->at(i).constData());
			writer->writeStartElement(QStringLiteral("column"));
			writer->writeCharacters(QLatin1String(QByteArray::fromRawData(data, size).toBase64()));
			writer->writeEndElement();
		}
		break;
	case AbstractColumn::ColumnMode::Integer:
		size = d->rowCount * sizeof(int);
		for (int i = 0; i < d->columnCount; ++i) {
			data = reinterpret_cast<const char*>(static_cast<QVector<QVector<int>>*>(d->data)->at(i).constData());
			writer->writeStartElement(QStringLiteral("column"));
			writer->writeCharacters(QLatin1String(QByteArray::fromRawData(data, size).toBase64()));
			writer->writeEndElement();
		}
		break;
	case AbstractColumn::ColumnMode::BigInt:
		size = d->rowCount * sizeof(qint64);
		for (int i = 0; i < d->columnCount; ++i) {
			data = reinterpret_cast<const char*>(static_cast<QVector<QVector<qint64>>*>(d->data)->at(i).constData());
			writer->writeStartElement(QStringLiteral("column"));
			writer->writeCharacters(QLatin1String(QByteArray::fromRawData(data, size).toBase64()));
			writer->writeEndElement();
		}
		break;
	case AbstractColumn::ColumnMode::Day:
	case AbstractColumn::ColumnMode::Month:
	case AbstractColumn::ColumnMode::DateTime:
		size = d->rowCount * sizeof(QDateTime);
		for (int i = 0; i < d->columnCount; ++i) {
			data = reinterpret_cast<const char*>(static_cast<QVector<QVector<QDateTime>>*>(d->data)->at(i).constData());
			writer->writeStartElement(QStringLiteral("column"));
			writer->writeCharacters(QLatin1String(QByteArray::fromRawData(data, size).toBase64()));
			writer->writeEndElement();
		}
		break;
	}

	writer->writeEndElement(); // "matrix"
}

bool Matrix::load(XmlStreamReader* reader, bool preview) {
	DEBUG(Q_FUNC_INFO)
	if (!readBasicAttributes(reader))
		return false;

	KLocalizedString attributeWarning = ki18n("Attribute '%1' missing or empty, default value is used");
	QXmlStreamAttributes attribs;
	QString str;

	// read child elements
	while (!reader->atEnd()) {
		reader->readNext();

		if (reader->isEndElement() && reader->name() == QStringLiteral("matrix"))
			break;

		if (!reader->isStartElement())
			continue;

		if (reader->name() == QLatin1String("comment")) {
			if (!readCommentElement(reader))
				return false;
		} else if (!preview && reader->name() == QLatin1String("formula")) {
			d->formula = reader->text().toString().trimmed();
		} else if (!preview && reader->name() == QLatin1String("format")) {
			attribs = reader->attributes();

			str = attribs.value(QStringLiteral("mode")).toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs(QStringLiteral("mode")).toString());
			else
				d->mode = AbstractColumn::ColumnMode(str.toInt());

			str = attribs.value(QStringLiteral("headerFormat")).toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs(QStringLiteral("headerFormat")).toString());
			else
				d->headerFormat = Matrix::HeaderFormat(str.toInt());

			str = attribs.value(QStringLiteral("numericFormat")).toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs(QStringLiteral("numericFormat")).toString());
			else {
				QByteArray formatba = str.toLatin1();
				d->numericFormat = *formatba.data();
			}

			str = attribs.value(QStringLiteral("precision")).toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs(QStringLiteral("precision")).toString());
			else
				d->precision = str.toInt();

		} else if (!preview && reader->name() == QLatin1String("dimension")) {
			attribs = reader->attributes();

			str = attribs.value(QStringLiteral("columns")).toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs(QStringLiteral("columns")).toString());
			else
				d->columnCount = str.toInt();

			str = attribs.value(QStringLiteral("rows")).toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs(QStringLiteral("rows")).toString());
			else
				d->rowCount = str.toInt();

			str = attribs.value(QStringLiteral("x_start")).toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs(QStringLiteral("x_start")).toString());
			else
				d->xStart = str.toDouble();

			str = attribs.value(QStringLiteral("x_end")).toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs(QStringLiteral("x_end")).toString());
			else
				d->xEnd = str.toDouble();

			str = attribs.value(QStringLiteral("y_start")).toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs(QStringLiteral("y_start")).toString());
			else
				d->yStart = str.toDouble();

			str = attribs.value(QStringLiteral("y_end")).toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs(QStringLiteral("y_end")).toString());
			else
				d->yEnd = str.toDouble();
		} else if (!preview && reader->name() == QLatin1String("row_heights")) {
			reader->readNext();
			QString content = reader->text().toString().trimmed();
			QByteArray bytes = QByteArray::fromBase64(content.toLatin1());
			int count = bytes.size() / sizeof(int);
			d->rowHeights.resize(count);
			memcpy(d->rowHeights.data(), bytes.data(), count * sizeof(int));
		} else if (!preview && reader->name() == QLatin1String("column_widths")) {
			reader->readNext();
			QString content = reader->text().toString().trimmed();
			QByteArray bytes = QByteArray::fromBase64(content.toLatin1());
			int count = bytes.size() / sizeof(int);
			d->columnWidths.resize(count);
			memcpy(d->columnWidths.data(), bytes.data(), count * sizeof(int));
		} else if (!preview && reader->name() == QLatin1String("column")) {
			// TODO: parallelize reading of columns?
			reader->readNext();
			QString content = reader->text().toString().trimmed();
			QByteArray bytes = QByteArray::fromBase64(content.toLatin1());

			switch (d->mode) {
			case AbstractColumn::ColumnMode::Double: {
				int count = bytes.size() / sizeof(double);
				QVector<double> column;
				column.resize(count);
				memcpy(column.data(), bytes.data(), count * sizeof(double));
				static_cast<QVector<QVector<double>>*>(d->data)->append(column);
				break;
			}
			case AbstractColumn::ColumnMode::Text: {
				int count = bytes.size() / sizeof(char);
				QVector<QString> column;
				column.resize(count);
				// TODO: warning (GCC8): writing to an object of type 'class QString' with no trivial copy-assignment; use copy-assignment or
				// copy-initialization instead memcpy(column.data(), bytes.data(), count*sizeof(QString)); QDEBUG("	string: " << column.data());
				static_cast<QVector<QVector<QString>>*>(d->data)->append(column);
				break;
			}
			case AbstractColumn::ColumnMode::Integer: {
				int count = bytes.size() / sizeof(int);
				QVector<int> column;
				column.resize(count);
				memcpy(column.data(), bytes.data(), count * sizeof(int));
				static_cast<QVector<QVector<int>>*>(d->data)->append(column);
				break;
			}
			case AbstractColumn::ColumnMode::BigInt: {
				int count = bytes.size() / sizeof(qint64);
				QVector<qint64> column;
				column.resize(count);
				memcpy(column.data(), bytes.data(), count * sizeof(qint64));
				static_cast<QVector<QVector<qint64>>*>(d->data)->append(column);
				break;
			}
			case AbstractColumn::ColumnMode::Day:
			case AbstractColumn::ColumnMode::Month:
			case AbstractColumn::ColumnMode::DateTime: {
				int count = bytes.size() / sizeof(QDateTime);
				QVector<QDateTime> column;
				column.resize(count);
				// TODO: warning (GCC8): writing to an object of type 'class QDateTime' with no trivial copy-assignment; use copy-assignment or
				// copy-initialization instead memcpy(column.data(), bytes.data(), count*sizeof(QDateTime));
				static_cast<QVector<QVector<QDateTime>>*>(d->data)->append(column);
				break;
			}
			}
		} else { // unknown element
			reader->raiseWarning(i18n("unknown element '%1'", reader->name().toString()));
			if (!reader->skipToEndElement())
				return false;
		}
	}

	return true;
}

//##############################################################################
//########################  Data Import  #######################################
//##############################################################################
int Matrix::prepareImport(std::vector<void*>& dataContainer,
						  AbstractFileFilter::ImportMode mode,
						  int actualRows,
						  int actualCols,
						  QStringList /*colNameList*/,
						  QVector<AbstractColumn::ColumnMode> columnMode) {
	auto newColumnMode = columnMode.at(0); // only first column mode used
	DEBUG(Q_FUNC_INFO << ", rows = " << actualRows << " cols = " << actualCols << ", mode = " << ENUM_TO_STRING(AbstractFileFilter, ImportMode, mode)
					  << ", column mode = " << ENUM_TO_STRING(AbstractColumn, ColumnMode, newColumnMode))
	// QDEBUG("	column modes = " << columnMode);
	int columnOffset = 0;
	setUndoAware(false);

	setSuppressDataChangedSignal(true);

	// resize the matrix
	if (mode == AbstractFileFilter::ImportMode::Replace) {
		clear();
		setDimensions(actualRows, actualCols);
	} else { // Append
		// handle mismatch of modes
		DEBUG(Q_FUNC_INFO << ", TODO: matrix mode = " << ENUM_TO_STRING(AbstractColumn, ColumnMode, d->mode)
						  << ", columnMode = " << ENUM_TO_STRING(AbstractColumn, ColumnMode, columnMode.at(0)))
		// TODO: no way to convert types yet!
		if (d->mode != newColumnMode) {
			DEBUG(Q_FUNC_INFO << ", WARNING mismatch of types in append mode!")
		}
		// catch some cases
		if ((d->mode == AbstractColumn::ColumnMode::Integer || d->mode == AbstractColumn::ColumnMode::BigInt)
			&& newColumnMode == AbstractColumn::ColumnMode::Double)
			d->mode = newColumnMode;

		columnOffset = columnCount();
		actualCols += columnOffset;
		DEBUG(Q_FUNC_INFO << ", col count = " << columnCount() << ", actualCols = " << actualCols)
		if (rowCount() < actualRows)
			setDimensions(actualRows, actualCols);
		else
			setDimensions(rowCount(), actualCols);
	}

	DEBUG(Q_FUNC_INFO << ", actual rows/cols = " << actualRows << "/" << actualCols)
	// data() returns a void* which is a pointer to a matrix of any data type (see ColumnPrivate.cpp)
	dataContainer.resize(actualCols);
	switch (newColumnMode) { // prepare all columns
	case AbstractColumn::ColumnMode::Double:
		for (int n = 0; n < actualCols; n++) {
			QVector<double>* vector = &(static_cast<QVector<QVector<double>>*>(data())->operator[](n));
			vector->resize(actualRows);
			dataContainer[n] = static_cast<void*>(vector);
		}
		d->mode = AbstractColumn::ColumnMode::Double;
		break;
	case AbstractColumn::ColumnMode::Integer:
		for (int n = 0; n < actualCols; n++) {
			QVector<int>* vector = &(static_cast<QVector<QVector<int>>*>(data())->operator[](n));
			vector->resize(actualRows);
			dataContainer[n] = static_cast<void*>(vector);
		}
		d->mode = AbstractColumn::ColumnMode::Integer;
		break;
	case AbstractColumn::ColumnMode::BigInt:
		for (int n = 0; n < actualCols; n++) {
			QVector<qint64>* vector = &(static_cast<QVector<QVector<qint64>>*>(data())->operator[](n));
			vector->resize(actualRows);
			dataContainer[n] = static_cast<void*>(vector);
		}
		d->mode = AbstractColumn::ColumnMode::BigInt;
		break;
	case AbstractColumn::ColumnMode::Text:
		for (int n = 0; n < actualCols; n++) {
			QVector<QString>* vector = &(static_cast<QVector<QVector<QString>>*>(data())->operator[](n));
			vector->resize(actualRows);
			dataContainer[n] = static_cast<void*>(vector);
		}
		d->mode = AbstractColumn::ColumnMode::Text;
		break;
	case AbstractColumn::ColumnMode::Day:
	case AbstractColumn::ColumnMode::Month:
	case AbstractColumn::ColumnMode::DateTime:
		for (int n = 0; n < actualCols; n++) {
			QVector<QDateTime>* vector = &(static_cast<QVector<QVector<QDateTime>>*>(data())->operator[](n));
			vector->resize(actualRows);
			dataContainer[n] = static_cast<void*>(vector);
		}
		d->mode = AbstractColumn::ColumnMode::DateTime;
		break;
	}

	return columnOffset;
}

void Matrix::finalizeImport(size_t /*columnOffset*/,
							size_t /*startColumn*/,
							size_t /*endColumn*/,
							const QString& /*dateTimeFormat*/,
							AbstractFileFilter::ImportMode) {
	DEBUG(Q_FUNC_INFO)

	setSuppressDataChangedSignal(false);
	setChanged();
	setUndoAware(true);
}
