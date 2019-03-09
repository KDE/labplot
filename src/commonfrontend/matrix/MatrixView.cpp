/***************************************************************************
    File                 : MatrixView.cpp
    Project              : LabPlot
    Description          : View class for Matrix
    --------------------------------------------------------------------
    Copyright            : (C) 2008-2009 Tilman Benkert (thzs@gmx.net)
    Copyright            : (C) 2015 Alexander Semke (alexander.semke@web.de)
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


#include "commonfrontend/matrix/MatrixView.h"
#include "backend/matrix/Matrix.h"
#include "backend/matrix/MatrixModel.h"
#include "backend/matrix/matrixcommands.h"
#include "backend/lib/macros.h"
#include "backend/core/column/Column.h"
#include "backend/core/column/ColumnPrivate.h"

#include "kdefrontend/spreadsheet/AddSubtractValueDialog.h"
#include "kdefrontend/matrix/MatrixFunctionDialog.h"
#include "kdefrontend/spreadsheet/StatisticsDialog.h"

#include <QAction>
#include <QStackedWidget>
#include <QTableView>
#include <QKeyEvent>
#include <QShortcut>
#include <QMenu>
#include <QPainter>
#include <QPrinter>
#include <QScrollArea>
#include <QInputDialog>
#include <QClipboard>
#include <QMimeData>
#include <QTextStream>
#include <QThreadPool>
#include <QMutex>
#include <QProcess>
#include <QHeaderView>

#include <KLocalizedString>
#include <QIcon>

#include <cfloat>
#include <cmath>

MatrixView::MatrixView(Matrix* matrix) : QWidget(),
	m_stackedWidget(new QStackedWidget(this)),
	m_tableView(new QTableView(this)),
	m_imageLabel(new QLabel(this)),
	m_matrix(matrix),
	m_model(new MatrixModel(matrix)) {

	init();

	//resize the view to show a 10x10 region of the matrix.
	//no need to resize the view when the project is being opened,
	//all views will be resized to the stored values at the end
	if (!m_matrix->isLoading()) {
		int w = m_tableView->horizontalHeader()->sectionSize(0)*10 + m_tableView->verticalHeader()->width();
		int h = m_tableView->verticalHeader()->sectionSize(0)*10 + m_tableView->horizontalHeader()->height();
		resize(w+50, h+50);
	}
}

MatrixView::~MatrixView() {
	delete m_model;
}

MatrixModel* MatrixView::model() const {
	return m_model;
}

void MatrixView::init() {
	initActions();
	connectActions();
	initMenus();

	auto* layout = new QHBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);
	setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
	setFocusPolicy(Qt::StrongFocus);
	setFocus();
	installEventFilter(this);

	layout->addWidget(m_stackedWidget);

	//table data view
	m_tableView->setModel(m_model);
	m_stackedWidget->addWidget(m_tableView);

	//horizontal header
	QHeaderView* h_header = m_tableView->horizontalHeader();
	h_header->setSectionsMovable(false);
	h_header->installEventFilter(this);

	//vertical header
	QHeaderView* v_header = m_tableView->verticalHeader();
	v_header->setSectionsMovable(false);
	v_header->installEventFilter(this);

	//set the header sizes to the (potentially user customized) sizes stored in Matrix
	adjustHeaders();

	//image view
	auto* area = new QScrollArea(this);
	m_stackedWidget->addWidget(area);
	area->setWidget(m_imageLabel);

	//SLOTs
	connect(m_matrix, SIGNAL(requestProjectContextMenu(QMenu*)), this, SLOT(createContextMenu(QMenu*)));
	connect(m_model, SIGNAL(changed()), this, SLOT(matrixDataChanged()));

	//keyboard shortcuts
	QShortcut* sel_all = new QShortcut(QKeySequence(tr("Ctrl+A", "Matrix: select all")), m_tableView);
	connect(sel_all, SIGNAL(activated()), m_tableView, SLOT(selectAll()));

	//TODO: add shortcuts for copy&paste,
	//for a single shortcut we need to descriminate between copy&paste for columns, rows or selected cells.
}

void MatrixView::initActions() {
	// selection related actions
	action_cut_selection = new QAction(QIcon::fromTheme("edit-cut"), i18n("Cu&t"), this);
	action_copy_selection = new QAction(QIcon::fromTheme("edit-copy"), i18n("&Copy"), this);
	action_paste_into_selection = new QAction(QIcon::fromTheme("edit-paste"), i18n("Past&e"), this);
	action_clear_selection = new QAction(QIcon::fromTheme("edit-clear"), i18n("Clea&r Selection"), this);
	action_select_all = new QAction(QIcon::fromTheme("edit-select-all"), i18n("Select All"), this);

	// matrix related actions
	auto* viewActionGroup = new QActionGroup(this);
	viewActionGroup->setExclusive(true);
	action_data_view = new QAction(QIcon::fromTheme("labplot-matrix"), i18n("Data"), viewActionGroup);
	action_data_view->setCheckable(true);
	action_data_view->setChecked(true);
	action_image_view = new QAction(QIcon::fromTheme("image-x-generic"), i18n("Image"), viewActionGroup);
	action_image_view->setCheckable(true);
	connect(viewActionGroup, SIGNAL(triggered(QAction*)), this, SLOT(switchView(QAction*)));

	action_fill_function = new QAction(QIcon::fromTheme(QString()), i18n("Function Values"), this);
	action_fill_const = new QAction(QIcon::fromTheme(QString()), i18n("Const Values"), this);
	action_clear_matrix = new QAction(QIcon::fromTheme("edit-clear"), i18n("Clear Matrix"), this);
	action_go_to_cell = new QAction(QIcon::fromTheme("go-jump"), i18n("&Go to Cell"), this);

	action_transpose = new QAction(i18n("&Transpose"), this);
	action_mirror_horizontally = new QAction(QIcon::fromTheme("object-flip-horizontal"), i18n("Mirror &Horizontally"), this);
	action_mirror_vertically = new QAction(QIcon::fromTheme("object-flip-vertical"), i18n("Mirror &Vertically"), this);

	action_add_value = new QAction(i18n("Add Value"), this);
	action_add_value->setData(AddSubtractValueDialog::Add);
	action_subtract_value = new QAction(i18n("Subtract Value"), this);
	action_subtract_value->setData(AddSubtractValueDialog::Subtract);
	action_multiply_value = new QAction(i18n("Multiply Value"), this);
	action_multiply_value->setData(AddSubtractValueDialog::Multiply);
	action_divide_value = new QAction(i18n("Divide Value"), this);
	action_divide_value->setData(AddSubtractValueDialog::Divide);

// 	action_duplicate = new QAction(i18nc("duplicate matrix", "&Duplicate"), this);
	//TODO
	//icon
	auto* headerFormatActionGroup = new QActionGroup(this);
	headerFormatActionGroup->setExclusive(true);
	action_header_format_1= new QAction(i18n("Rows and Columns"), headerFormatActionGroup);
	action_header_format_1->setCheckable(true);
	action_header_format_2= new QAction(i18n("xy-Values"), headerFormatActionGroup);
	action_header_format_2->setCheckable(true);
	action_header_format_3= new QAction(i18n("Rows, Columns and xy-Values"), headerFormatActionGroup);
	action_header_format_3->setCheckable(true);
	connect(headerFormatActionGroup, SIGNAL(triggered(QAction*)), this, SLOT(headerFormatChanged(QAction*)));

	// column related actions
	action_add_columns = new QAction(QIcon::fromTheme("edit-table-insert-column-right"), i18n("&Add Columns"), this);
	action_insert_columns = new QAction(QIcon::fromTheme("edit-table-insert-column-left"), i18n("&Insert Empty Columns"), this);
	action_remove_columns = new QAction(QIcon::fromTheme("edit-table-delete-column"), i18n("Remo&ve Columns"), this);
	action_clear_columns = new QAction(QIcon::fromTheme("edit-clear"), i18n("Clea&r Columns"), this);
	action_statistics_columns = new QAction(QIcon::fromTheme("view-statistics"), i18n("Statisti&cs"), this);

	// row related actions
	action_add_rows = new QAction(QIcon::fromTheme("edit-table-insert-row-above"), i18n("&Add Rows"), this);
	action_insert_rows = new QAction(QIcon::fromTheme("edit-table-insert-row-above") ,i18n("&Insert Empty Rows"), this);
	action_remove_rows = new QAction(QIcon::fromTheme("edit-table-delete-row"), i18n("Remo&ve Rows"), this);
	action_clear_rows = new QAction(QIcon::fromTheme("edit-clear"), i18n("Clea&r Rows"), this);
	action_statistics_rows = new QAction(QIcon::fromTheme("view-statistics"), i18n("Statisti&cs"), this);
}

void MatrixView::modifyValues() {
	const QAction* action = dynamic_cast<const QAction*>(QObject::sender());
	AddSubtractValueDialog::Operation op = (AddSubtractValueDialog::Operation)action->data().toInt();
	auto* dlg = new AddSubtractValueDialog(m_matrix, op);
	dlg->setMatrices();
	dlg->exec();
}


void MatrixView::connectActions() {
	// selection related actions
	connect(action_cut_selection, SIGNAL(triggered()), this, SLOT(cutSelection()));
	connect(action_copy_selection, SIGNAL(triggered()), this, SLOT(copySelection()));
	connect(action_paste_into_selection, SIGNAL(triggered()), this, SLOT(pasteIntoSelection()));
	connect(action_clear_selection, SIGNAL(triggered()), this, SLOT(clearSelectedCells()));
	connect(action_select_all, SIGNAL(triggered()), m_tableView, SLOT(selectAll()));

	// matrix related actions
	connect(action_fill_function, SIGNAL(triggered()), this, SLOT(fillWithFunctionValues()));
	connect(action_fill_const, SIGNAL(triggered()), this, SLOT(fillWithConstValues()));

	connect(action_go_to_cell, SIGNAL(triggered()), this, SLOT(goToCell()));
	//connect(action_duplicate, SIGNAL(triggered()), this, SLOT(duplicate()));
	connect(action_clear_matrix, SIGNAL(triggered()), m_matrix, SLOT(clear()));
	connect(action_transpose, SIGNAL(triggered()), m_matrix, SLOT(transpose()));
	connect(action_mirror_horizontally, SIGNAL(triggered()), m_matrix, SLOT(mirrorHorizontally()));
	connect(action_mirror_vertically, SIGNAL(triggered()), m_matrix, SLOT(mirrorVertically()));
	connect(action_add_value, &QAction::triggered, this, &MatrixView::modifyValues);
	connect(action_subtract_value, &QAction::triggered, this, &MatrixView::modifyValues);
	connect(action_multiply_value, &QAction::triggered, this, &MatrixView::modifyValues);
	connect(action_divide_value, &QAction::triggered, this, &MatrixView::modifyValues);

	// column related actions
	connect(action_add_columns, SIGNAL(triggered()), this, SLOT(addColumns()));
	connect(action_insert_columns, SIGNAL(triggered()), this, SLOT(insertEmptyColumns()));
	connect(action_remove_columns, SIGNAL(triggered()), this, SLOT(removeSelectedColumns()));
	connect(action_clear_columns, SIGNAL(triggered()), this, SLOT(clearSelectedColumns()));
	connect(action_statistics_columns, SIGNAL(triggered()), this, SLOT(showColumnStatistics()));

	// row related actions
	connect(action_add_rows, SIGNAL(triggered()), this, SLOT(addRows()));
	connect(action_insert_rows, SIGNAL(triggered()), this, SLOT(insertEmptyRows()));
	connect(action_remove_rows, SIGNAL(triggered()), this, SLOT(removeSelectedRows()));
	connect(action_clear_rows, SIGNAL(triggered()), this, SLOT(clearSelectedRows()));
	connect(action_statistics_rows, SIGNAL(triggered()), this, SLOT(showRowStatistics()));
}

void MatrixView::initMenus() {
	//selection menu
	m_selectionMenu = new QMenu(i18n("Selection"), this);
	m_selectionMenu->addAction(action_cut_selection);
	m_selectionMenu->addAction(action_copy_selection);
	m_selectionMenu->addAction(action_paste_into_selection);
	m_selectionMenu->addAction(action_clear_selection);

	//column menu
	m_columnMenu = new QMenu(this);
	m_columnMenu->addAction(action_insert_columns);
	m_columnMenu->addAction(action_remove_columns);
	m_columnMenu->addAction(action_clear_columns);
	m_columnMenu->addAction(action_statistics_columns);

	//row menu
	m_rowMenu = new QMenu(this);
	m_rowMenu->addAction(action_insert_rows);
	m_rowMenu->addAction(action_remove_rows);
	m_rowMenu->addAction(action_clear_rows);
	m_rowMenu->addAction(action_statistics_rows);

	//matrix menu
	m_matrixMenu = new QMenu(this);

	m_matrixMenu->addMenu(m_selectionMenu);
	m_matrixMenu->addSeparator();

	QMenu* submenu = new QMenu(i18n("Generate Data"), this);
	submenu->addAction(action_fill_const);
	submenu->addAction(action_fill_function);
	m_matrixMenu->addMenu(submenu);
	m_matrixMenu->addSeparator();

	// Data manipulation sub-menu
	QMenu* dataManipulationMenu = new QMenu(i18n("Manipulate Data"), this);
	dataManipulationMenu->addAction(action_transpose);
	dataManipulationMenu->addAction(action_mirror_horizontally);
	dataManipulationMenu->addAction(action_mirror_vertically);
	dataManipulationMenu->addAction(action_add_value);
	dataManipulationMenu->addAction(action_subtract_value);
	dataManipulationMenu->addAction(action_multiply_value);
	dataManipulationMenu->addAction(action_divide_value);

	m_matrixMenu->addMenu(dataManipulationMenu);
	m_matrixMenu->addSeparator();

	submenu = new QMenu(i18n("View"), this);
	submenu->addAction(action_data_view);
	submenu->addAction(action_image_view);
	m_matrixMenu->addMenu(submenu);
	m_matrixMenu->addSeparator();


	m_matrixMenu->addAction(action_select_all);
	m_matrixMenu->addAction(action_clear_matrix);
	m_matrixMenu->addSeparator();

	m_headerFormatMenu = new QMenu(i18n("Header Format"), this);
	m_headerFormatMenu->addAction(action_header_format_1);
	m_headerFormatMenu->addAction(action_header_format_2);
	m_headerFormatMenu->addAction(action_header_format_3);

	m_matrixMenu->addMenu(m_headerFormatMenu);
	m_matrixMenu->addSeparator();
	m_matrixMenu->addAction(action_go_to_cell);
}

/*!
 * Populates the menu \c menu with the spreadsheet and spreadsheet view relevant actions.
 * The menu is used
 *   - as the context menu in MatrixView
 *   - as the "matrix menu" in the main menu-bar (called form MainWin)
 *   - as a part of the matrix context menu in project explorer
 */
void MatrixView::createContextMenu(QMenu* menu) const {
	Q_ASSERT(menu);

	QAction* firstAction = nullptr;
	// if we're populating the context menu for the project explorer, then
	//there're already actions available there. Skip the first title-action
	//and insert the action at the beginning of the menu.
	if (menu->actions().size()>1)
		firstAction = menu->actions().at(1);

	menu->insertMenu(firstAction, m_selectionMenu);
	menu->insertSeparator(firstAction);

	QMenu* submenu = new QMenu(i18n("Generate Data"), const_cast<MatrixView*>(this));
	submenu->addAction(action_fill_const);
	submenu->addAction(action_fill_function);
	menu->insertMenu(firstAction, submenu);
	menu->insertSeparator(firstAction);


	// Data manipulation sub-menu
	submenu = new QMenu(i18n("Manipulate Data"), const_cast<MatrixView*>(this));
	submenu->addAction(action_transpose);
	submenu->addAction(action_mirror_horizontally);
	submenu->addAction(action_mirror_vertically);
	submenu->addAction(action_add_value);
	submenu->addAction(action_subtract_value);
	submenu->addAction(action_multiply_value);
	submenu->addAction(action_divide_value);

	menu->insertMenu(firstAction, submenu);
	menu->insertSeparator(firstAction);

	submenu = new QMenu(i18n("View"), const_cast<MatrixView*>(this));
	submenu->addAction(action_data_view);
	submenu->addAction(action_image_view);
	menu->insertMenu(firstAction, submenu);
	menu->insertSeparator(firstAction);

	menu->insertAction(firstAction, action_select_all);
	menu->insertAction(firstAction, action_clear_matrix);
	menu->insertSeparator(firstAction);
// 	menu->insertAction(firstAction, action_duplicate);
	menu->insertMenu(firstAction, m_headerFormatMenu);

	menu->insertSeparator(firstAction);
	menu->insertAction(firstAction, action_go_to_cell);
	menu->insertSeparator(firstAction);
}

/*!
	set the row and column size to the saved sizes.
 */
void MatrixView::adjustHeaders() {
	QHeaderView* h_header = m_tableView->horizontalHeader();
	QHeaderView* v_header = m_tableView->verticalHeader();

	disconnect(v_header, &QHeaderView::sectionResized, this, &MatrixView::handleVerticalSectionResized);
	disconnect(h_header, &QHeaderView::sectionResized, this, &MatrixView::handleHorizontalSectionResized);

	//resize columns to the saved sizes or to fit the contents if the widht is 0
	int cols = m_matrix->columnCount();
	for (int i = 0; i < cols; i++) {
		if (m_matrix->columnWidth(i) == 0)
			m_tableView->resizeColumnToContents(i);
		else
			m_tableView->setColumnWidth(i, m_matrix->columnWidth(i));
	}

	//resize rows to the saved sizes or to fit the contents if the height is 0
	int rows = m_matrix->rowCount();
	for (int i = 0; i < rows; i++) {
		if (m_matrix->rowHeight(i) == 0)
			m_tableView->resizeRowToContents(i);
		else
			m_tableView->setRowHeight(i, m_matrix->rowHeight(i));
	}

	connect(v_header, SIGNAL(sectionResized(int,int,int)), this, SLOT(handleVerticalSectionResized(int,int,int)));
	connect(h_header, SIGNAL(sectionResized(int,int,int)), this, SLOT(handleHorizontalSectionResized(int,int,int)));
}

/*!
	Resizes the headers/columns to fit the new content. Called on changes of the header format in Matrix.
*/
void MatrixView::resizeHeaders() {
	m_tableView->resizeColumnsToContents();
	m_tableView->resizeRowsToContents();

	if (m_matrix->headerFormat() == Matrix::HeaderRowsColumns)
		action_header_format_1->setChecked(true);
	else if (m_matrix->headerFormat() == Matrix::HeaderValues)
		action_header_format_2->setChecked(true);
	else
		action_header_format_3->setChecked(true);
}

/*!
	Returns how many columns are selected.
	If full is true, this function only returns the number of fully selected columns.
*/
int MatrixView::selectedColumnCount(bool full) const {
	int count = 0;
	int cols = m_matrix->columnCount();
	for (int i = 0; i < cols; i++)
		if (isColumnSelected(i, full)) count++;
	return count;
}

/*!
	Returns true if column 'col' is selected; otherwise false.
	If full is true, this function only returns true if the whole column is selected.
*/
bool MatrixView::isColumnSelected(int col, bool full) const {
	if (full)
		return m_tableView->selectionModel()->isColumnSelected(col, QModelIndex());
	else
		return m_tableView->selectionModel()->columnIntersectsSelection(col, QModelIndex());
}

/*!
	Return how many rows are (at least partly) selected
	If full is true, this function only returns the number of fully selected rows.
*/
int MatrixView::selectedRowCount(bool full) const {
	int count = 0;
	int rows = m_matrix->rowCount();
	for (int i = 0; i < rows; i++)
		if (isRowSelected(i, full)) count++;
	return count;
}

/*!
	Returns true if row \c row is selected; otherwise false
	If full is true, this function only returns true if the whole row is selected.
*/
bool MatrixView::isRowSelected(int row, bool full) const {
	if (full)
		return m_tableView->selectionModel()->isRowSelected(row, QModelIndex());
	else
		return m_tableView->selectionModel()->rowIntersectsSelection(row, QModelIndex());
}

/*!
	Return the index of the first selected column.
	If full is true, this function only looks for fully selected columns.
*/
int MatrixView::firstSelectedColumn(bool full) const {
	int cols = m_matrix->columnCount();
	for (int i = 0; i < cols; i++) {
		if (isColumnSelected(i, full))
			return i;
	}
	return -1;
}

/*!
	Return the index of the last selected column
	If full is true, this function only looks for fully selected columns.
*/
int MatrixView::lastSelectedColumn(bool full) const {
	int cols = m_matrix->columnCount();
	for (int i = cols-1; i >= 0; i--)
		if (isColumnSelected(i, full)) return i;

	return -2;
}

/*!
	Return the index of the first selected row.
	If full is true, this function only looks for fully selected rows.
*/
int MatrixView::firstSelectedRow(bool full) const {
	int rows = m_matrix->rowCount();
	for (int i = 0; i < rows; i++) 	{
		if (isRowSelected(i, full))
			return i;
	}
	return -1;
}

/*!
	Return the index of the last selected row
	If full is true, this function only looks for fully selected rows.
*/
int MatrixView::lastSelectedRow(bool full) const {
	int rows = m_matrix->rowCount();
	for (int i = rows-1; i >= 0; i--)
		if (isRowSelected(i, full)) return i;

	return -2;
}

bool MatrixView::isCellSelected(int row, int col) const {
	if (row < 0 || col < 0 || row >= m_matrix->rowCount() || col >= m_matrix->columnCount()) return false;

	return m_tableView->selectionModel()->isSelected(m_model->index(row, col));
}

void MatrixView::setCellSelected(int row, int col) {
	m_tableView->selectionModel()->select(m_model->index(row, col), QItemSelectionModel::Select);
}

void MatrixView::setCellsSelected(int first_row, int first_col, int last_row, int last_col) {
	QModelIndex top_left = m_model->index(first_row, first_col);
	QModelIndex bottom_right = m_model->index(last_row, last_col);
	m_tableView->selectionModel()->select(QItemSelection(top_left, bottom_right), QItemSelectionModel::SelectCurrent);
}

/*!
	Determine the current cell (-1 if no cell is designated as the current)
*/
void MatrixView::getCurrentCell(int* row, int* col) const {
	QModelIndex index = m_tableView->selectionModel()->currentIndex();
	if (index.isValid()) {
		*row = index.row();
		*col = index.column();
	} else {
		*row = -1;
		*col = -1;
	}
}

bool MatrixView::eventFilter(QObject * watched, QEvent * event) {
	if (event->type() == QEvent::ContextMenu) {
		auto* cm_event = static_cast<QContextMenuEvent*>(event);
		QPoint global_pos = cm_event->globalPos();
		if (watched == m_tableView->verticalHeader())
			m_rowMenu->exec(global_pos);
		else if (watched == m_tableView->horizontalHeader())
			m_columnMenu->exec(global_pos);
		else if (watched == this)
			m_matrixMenu->exec(global_pos);
		else
			return QWidget::eventFilter(watched, event);
		return true;
	} else
		return QWidget::eventFilter(watched, event);
}

void MatrixView::keyPressEvent(QKeyEvent* event) {
	if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
		advanceCell();
}

//##############################################################################
//####################################  SLOTs   ################################
//##############################################################################
/*!
	Advance current cell after [Return] or [Enter] was pressed
*/
void MatrixView::advanceCell() {
	QModelIndex idx = m_tableView->currentIndex();
	if (idx.row()+1 < m_matrix->rowCount())
		m_tableView->setCurrentIndex(idx.sibling(idx.row()+1, idx.column()));
}

void MatrixView::goToCell() {
	bool ok;

	int col = QInputDialog::getInt(nullptr, i18n("Go to Cell"), i18n("Enter column"), 1, 1, m_matrix->columnCount(), 1, &ok);
	if (!ok) return;

	int row = QInputDialog::getInt(nullptr, i18n("Go to Cell"), i18n("Enter row"), 1, 1, m_matrix->rowCount(), 1, &ok);
	if (!ok) return;

	goToCell(row-1, col-1);
}

void MatrixView::goToCell(int row, int col) {
	QModelIndex index = m_model->index(row, col);
	m_tableView->scrollTo(index);
	m_tableView->setCurrentIndex(index);
}

void MatrixView::handleHorizontalSectionResized(int logicalIndex, int oldSize, int newSize) {
	Q_UNUSED(oldSize)
	m_matrix->setColumnWidth(logicalIndex, newSize);
}

void MatrixView::handleVerticalSectionResized(int logicalIndex, int oldSize, int newSize) {
	Q_UNUSED(oldSize)
	m_matrix->setRowHeight(logicalIndex, newSize);
}

void MatrixView::fillWithFunctionValues() {
	auto* dlg = new MatrixFunctionDialog(m_matrix);
	dlg->exec();
}

void MatrixView::fillWithConstValues() {
	bool ok = false;
	double value = QInputDialog::getDouble(this, i18n("Fill the matrix with constant value"),
	                                       i18n("Value"), 0, -2147483647, 2147483647, 6, &ok);
	if (ok) {
		WAIT_CURSOR;
		auto* newData = static_cast<QVector<QVector<double>>*>(m_matrix->data());
		for (int col = 0; col < m_matrix->columnCount(); ++col) {
			for (int row = 0; row < m_matrix->rowCount(); ++row)
				(newData->operator[](col))[row] = value;
		}
		m_matrix->setData(newData);
		RESET_CURSOR;
	}
}

//############################ selection related slots #########################
void MatrixView::cutSelection() {
	if (firstSelectedRow() < 0) return;

	WAIT_CURSOR;
	m_matrix->beginMacro(i18n("%1: cut selected cell(s)", m_matrix->name()));
	copySelection();
	clearSelectedCells();
	m_matrix->endMacro();
	RESET_CURSOR;
}

void MatrixView::copySelection() {
	int first_col = firstSelectedColumn(false);
	if (first_col == -1) return;
	int last_col = lastSelectedColumn(false);
	if (last_col == -2) return;
	int first_row = firstSelectedRow(false);
	if (first_row == -1)	return;
	int last_row = lastSelectedRow(false);
	if (last_row == -2) return;
	int cols = last_col - first_col +1;
	int rows = last_row - first_row +1;

	WAIT_CURSOR;
	QString output_str;

	for (int r = 0; r < rows; r++) 	{
		for (int c = 0; c < cols; c++) {
			//TODO: mode
			if (isCellSelected(first_row + r, first_col + c))
				output_str += QLocale().toString(m_matrix->cell<double>(first_row + r, first_col + c),
				                                 m_matrix->numericFormat(), 16); // copy with max. precision
			if (c < cols-1)
				output_str += '\t';
		}
		if (r < rows-1)
			output_str += '\n';
	}
	QApplication::clipboard()->setText(output_str);
	RESET_CURSOR;
}

void MatrixView::pasteIntoSelection() {
	if (m_matrix->columnCount() < 1 || m_matrix->rowCount() < 1) return;

	const QMimeData* mime_data = QApplication::clipboard()->mimeData();
	if (!mime_data->hasFormat("text/plain"))
		return;

	WAIT_CURSOR;
	m_matrix->beginMacro(i18n("%1: paste from clipboard", m_matrix->name()));

	int first_col = firstSelectedColumn(false);
	int last_col = lastSelectedColumn(false);
	int first_row = firstSelectedRow(false);
	int last_row = lastSelectedRow(false);
	int input_row_count = 0;
	int input_col_count = 0;
	int rows, cols;

	QString input_str = QString(mime_data->data("text/plain"));
	QList< QStringList > cell_texts;
	QStringList input_rows(input_str.split('\n'));
	input_row_count = input_rows.count();
	input_col_count = 0;
	for (int i = 0; i < input_row_count; i++) {
		cell_texts.append(input_rows.at(i).split('\t'));
		if (cell_texts.at(i).count() > input_col_count) input_col_count = cell_texts.at(i).count();
	}

	// if the is no selection or only one cell selected, the
	// selection will be expanded to the needed size from the current cell
	if ( (first_col == -1 || first_row == -1) ||
	        (last_row == first_row && last_col == first_col) ) {
		int current_row, current_col;
		getCurrentCell(&current_row, &current_col);
		if (current_row == -1) current_row = 0;
		if (current_col == -1) current_col = 0;
		setCellSelected(current_row, current_col);
		first_col = current_col;
		first_row = current_row;
		last_row = first_row + input_row_count -1;
		last_col = first_col + input_col_count -1;
		// resize the matrix if necessary
		if (last_col >= m_matrix->columnCount())
			m_matrix->appendColumns(last_col+1-m_matrix->columnCount());
		if (last_row >= m_matrix->rowCount())
			m_matrix->appendRows(last_row+1-m_matrix->rowCount());
		// select the rectangle to be pasted in
		setCellsSelected(first_row, first_col, last_row, last_col);
	}

	rows = last_row - first_row + 1;
	cols = last_col - first_col + 1;
	for (int r = 0; r < rows && r < input_row_count; r++) {
		for (int c = 0; c < cols && c < input_col_count; c++) {
			if (isCellSelected(first_row + r, first_col + c) && (c < cell_texts.at(r).count()) )
				m_matrix->setCell(first_row + r, first_col + c, cell_texts.at(r).at(c).toDouble());
		}
	}

	m_matrix->endMacro();
	RESET_CURSOR;
}

void MatrixView::clearSelectedCells() {
	int first_row = firstSelectedRow();
	if (first_row<0)
		return;

	int first_col = firstSelectedColumn();
	if (first_col<0)
		return;

	int last_row = lastSelectedRow();
	int last_col = lastSelectedColumn();

	WAIT_CURSOR;
	m_matrix->beginMacro(i18n("%1: clear selected cell(s)", m_matrix->name()));
	for (int i = first_row; i <= last_row; i++) {
		for (int j = first_col; j <= last_col; j++) {
			if (isCellSelected(i, j))
				m_matrix->clearCell(i, j);
		}
	}
	m_matrix->endMacro();
	RESET_CURSOR;
}


class UpdateImageTask : public QRunnable {
public:
	UpdateImageTask(int start, int end, QImage& image, const void* data, double scaleFactor, double min) : m_image(image), m_data(data) {
		m_start = start;
		m_end = end;
		m_scaleFactor = scaleFactor;
		m_min = min;
	};

	void run() override {
		for (int row = m_start; row < m_end; ++row) {
			m_mutex.lock();
			QRgb* line = reinterpret_cast<QRgb*>(m_image.scanLine(row));
			m_mutex.unlock();
			for (int col = 0; col < m_image.width(); ++col) {
				const int gray = (static_cast<const QVector<QVector<double>>*>(m_data)->at(col).at(row)-m_min)*m_scaleFactor;
				line[col] = qRgb(gray, gray, gray);
			}
		}
	}

private:
	QMutex m_mutex;
	int m_start;
	int m_end;
	QImage& m_image;
	const void* m_data;
	double m_scaleFactor;
	double m_min;
};

void MatrixView::updateImage() {
	WAIT_CURSOR;
	m_image = QImage(m_matrix->columnCount(), m_matrix->rowCount(), QImage::Format_ARGB32);

	//find min/max value
	double dmax = -DBL_MAX, dmin = DBL_MAX;
	const QVector<QVector<double>>* data = static_cast<QVector<QVector<double>>*>(m_matrix->data());
	const int width = m_matrix->columnCount();
	const int height = m_matrix->rowCount();
	for (int col = 0; col < width; ++col) {
		for (int row = 0; row < height; ++row) {
			const double value = (data->operator[](col))[row];
			if (dmax < value) dmax = value;
			if (dmin > value) dmin = value;
		}
	}

	//update the image
	const double scaleFactor = 255.0/(dmax-dmin);
	QThreadPool* pool = QThreadPool::globalInstance();
	int range = ceil(double(m_image.height())/pool->maxThreadCount());
	for (int i = 0; i < pool->maxThreadCount(); ++i) {
		const int start = i*range;
		int end = (i+1)*range;
		if (end > m_image.height()) end = m_image.height();
		auto* task = new UpdateImageTask(start, end, m_image, data, scaleFactor, dmin);
		pool->start(task);
	}
	pool->waitForDone();

	m_imageLabel->resize(width, height);
	m_imageLabel->setPixmap(QPixmap::fromImage(m_image));
	m_imageIsDirty = false;
	RESET_CURSOR;
}

//############################# matrix related slots ###########################
void MatrixView::switchView(QAction* action) {
	if (action == action_data_view)
		m_stackedWidget->setCurrentIndex(0);
	else {
		if (m_imageIsDirty)
			this->updateImage();

		m_stackedWidget->setCurrentIndex(1);
	}
}

void MatrixView::matrixDataChanged() {
	m_imageIsDirty = true;
	if (m_stackedWidget->currentIndex() == 1)
		this->updateImage();
}

void MatrixView::headerFormatChanged(QAction* action) {
	if (action == action_header_format_1)
		m_matrix->setHeaderFormat(Matrix::HeaderRowsColumns);
	else if (action == action_header_format_2)
		m_matrix->setHeaderFormat(Matrix::HeaderValues);
	else
		m_matrix->setHeaderFormat(Matrix::HeaderRowsColumnsValues);
}

//############################# column related slots ###########################
/*!
  Append as many columns as are selected.
*/
void MatrixView::addColumns() {
	m_matrix->appendColumns(selectedColumnCount(false));
}

void MatrixView::insertEmptyColumns() {
	int first = firstSelectedColumn();
	int last = lastSelectedColumn();
	if (first < 0) return;
	int current = first;

	WAIT_CURSOR;
	m_matrix->beginMacro(i18n("%1: insert empty column(s)", m_matrix->name()));
	while (current <= last) {
		current = first+1;
		while (current <= last && isColumnSelected(current)) current++;
		const int count = current-first;
		m_matrix->insertColumns(first, count);
		current += count;
		last += count;
		while (current <= last && isColumnSelected(current)) current++;
		first = current;
	}
	m_matrix->endMacro();
	RESET_CURSOR;
}

void MatrixView::removeSelectedColumns() {
	int first = firstSelectedColumn();
	int last = lastSelectedColumn();
	if (first < 0) return;

	WAIT_CURSOR;
	m_matrix->beginMacro(i18n("%1: remove selected column(s)", m_matrix->name()));
	for (int i = last; i >= first; i--)
		if (isColumnSelected(i, false)) m_matrix->removeColumns(i, 1);
	m_matrix->endMacro();
	RESET_CURSOR;
}

void MatrixView::clearSelectedColumns() {
	WAIT_CURSOR;
	m_matrix->beginMacro(i18n("%1: clear selected column(s)", m_matrix->name()));
	for (int i = 0; i < m_matrix->columnCount(); i++) {
		if (isColumnSelected(i, false))
			m_matrix->clearColumn(i);
	}
	m_matrix->endMacro();
	RESET_CURSOR;
}

//############################## rows related slots ############################
/*!
  Append as many rows as are selected.
*/
void MatrixView::addRows() {
	m_matrix->appendRows(selectedRowCount(false));
}

void MatrixView::insertEmptyRows() {
	int first = firstSelectedRow();
	int last = lastSelectedRow();
	int current = first;

	if (first < 0) return;

	WAIT_CURSOR;
	m_matrix->beginMacro(i18n("%1: insert empty rows(s)", m_matrix->name()));
	while (current <= last) {
		current = first+1;
		while (current <= last && isRowSelected(current)) current++;
		const int count = current-first;
		m_matrix->insertRows(first, count);
		current += count;
		last += count;
		while (current <= last && !isRowSelected(current)) current++;
		first = current;
	}
	m_matrix->endMacro();
	RESET_CURSOR;
}

void MatrixView::removeSelectedRows() {
	int first = firstSelectedRow();
	int last = lastSelectedRow();
	if (first < 0) return;

	WAIT_CURSOR;
	m_matrix->beginMacro(i18n("%1: remove selected rows(s)", m_matrix->name()));
	for (int i = last; i >= first; i--)
		if (isRowSelected(i, false)) m_matrix->removeRows(i, 1);
	m_matrix->endMacro();
	RESET_CURSOR;
}

void MatrixView::clearSelectedRows() {
	int first = firstSelectedRow();
	int last = lastSelectedRow();
	if (first < 0) return;

	WAIT_CURSOR;
	m_matrix->beginMacro(i18n("%1: clear selected rows(s)", m_matrix->name()));
	for (int i = first; i <= last; i++) {
		if (isRowSelected(i))
			m_matrix->clearRow(i);
	}
	m_matrix->endMacro();
	RESET_CURSOR;
}

/*!
  prints the complete matrix to \c printer.
 */

void MatrixView::print(QPrinter* printer) const {
	WAIT_CURSOR;
	QPainter painter (printer);

	const int dpiy = printer->logicalDpiY();
	const int margin = (int) ( (1/2.54)*dpiy ); // 1 cm margins

	QHeaderView* hHeader = m_tableView->horizontalHeader();
	QHeaderView* vHeader = m_tableView->verticalHeader();
	auto* data = static_cast<QVector<QVector<double>>*>(m_matrix->data());

	const int rows = m_matrix->rowCount();
	const int cols = m_matrix->columnCount();
	int height = margin;
	const int vertHeaderWidth = vHeader->width();
	int right = margin + vertHeaderWidth;

	int columnsPerTable = 0;
	int headerStringWidth = 0;
	int firstRowStringWidth = vertHeaderWidth;
	bool tablesNeeded = false;
	QVector<int> firstRowCeilSizes;
	firstRowCeilSizes.reserve(data[0].size());
	firstRowCeilSizes.resize(data[0].size());
	QRect br;

	for (int i = 0; i < data->size(); ++i) {
		br = painter.boundingRect(br, Qt::AlignCenter,QString::number(data->at(i)[0]) + '\t');
		firstRowCeilSizes[i] = br.width() > m_tableView->columnWidth(i) ?
		                       br.width() : m_tableView->columnWidth(i);
	}
	for (int col = 0; col < cols; ++col) {
		headerStringWidth += m_tableView->columnWidth(col);
		br = painter.boundingRect(br, Qt::AlignCenter,QString::number(data->at(col)[0]) + '\t');
		firstRowStringWidth += br.width();
		if ((headerStringWidth >= printer->pageRect().width() -2*margin) ||
		        (firstRowStringWidth >= printer->pageRect().width() - 2*margin)) {
			tablesNeeded = true;
			break;
		}
		columnsPerTable++;
	}

	int tablesCount = (columnsPerTable != 0) ? cols/columnsPerTable : 0;
	const int remainingColumns = (columnsPerTable != 0) ? cols % columnsPerTable : cols;

	if (!tablesNeeded) {
		tablesCount = 1;
		columnsPerTable = cols;
	}
	if (remainingColumns > 0)
		tablesCount++;
	for (int table = 0; table < tablesCount; ++table) {
		right = margin + vertHeaderWidth;
		//Paint the horizontal header first
		painter.setFont(hHeader->font());
		QString headerString = m_tableView->model()->headerData(0, Qt::Horizontal).toString();
		QRect br;
		br = painter.boundingRect(br, Qt::AlignCenter, headerString);
		QRect tr(br);
		if (table != 0)
			height += tr.height();
		painter.drawLine(right, height, right, height+br.height());

		int i = table * columnsPerTable;
		int toI = table * columnsPerTable + columnsPerTable;
		if ((remainingColumns > 0) && (table == tablesCount-1)) {
			i = (tablesCount-1)*columnsPerTable;
			toI = (tablesCount-1)* columnsPerTable + remainingColumns;
		}

		for (; i<toI; ++i) {
			headerString = m_tableView->model()->headerData(i, Qt::Horizontal).toString();
			const int w = /*m_tableView->columnWidth(i)*/ firstRowCeilSizes[i];
			tr.setTopLeft(QPoint(right,height));
			tr.setWidth(w);
			tr.setHeight(br.height());

			painter.drawText(tr, Qt::AlignCenter, headerString);
			right += w;
			painter.drawLine(right, height, right, height+tr.height());

		}
		//first horizontal line
		painter.drawLine(margin + vertHeaderWidth, height, right-1, height);
		height += tr.height();
		painter.drawLine(margin, height, right-1, height);

		// print table values
		QString cellText;
		for (i = 0; i < rows; ++i) {
			right = margin;
			cellText = m_tableView->model()->headerData(i, Qt::Vertical).toString()+'\t';
			tr = painter.boundingRect(tr, Qt::AlignCenter, cellText);
			painter.drawLine(right, height, right, height+tr.height());

			br.setTopLeft(QPoint(right,height));
			br.setWidth(vertHeaderWidth);
			br.setHeight(tr.height());
			painter.drawText(br, Qt::AlignCenter, cellText);
			right += vertHeaderWidth;
			painter.drawLine(right, height, right, height+tr.height());
			int j = table * columnsPerTable;
			int toJ = table * columnsPerTable + columnsPerTable;
			if ((remainingColumns > 0) && (table == tablesCount-1)) {
				j = (tablesCount-1)*columnsPerTable;
				toJ = (tablesCount-1)* columnsPerTable + remainingColumns;
			}
			for (; j< toJ; j++) {
				int w = /*m_tableView->columnWidth(j)*/ firstRowCeilSizes[j];
				cellText = QString::number(data->at(j)[i]) + '\t';
				tr = painter.boundingRect(tr,Qt::AlignCenter,cellText);
				br.setTopLeft(QPoint(right,height));
				br.setWidth(w);
				br.setHeight(tr.height());
				painter.drawText(br, Qt::AlignCenter, cellText);
				right += w;
				painter.drawLine(right, height, right, height+tr.height());
			}
			height += br.height();
			painter.drawLine(margin, height, right-1, height);

			if (height >= printer->height()-margin ) {
				printer->newPage();
				height = margin;
				painter.drawLine(margin, height, right, height);
			}
		}
	}
	RESET_CURSOR;
}

void MatrixView::exportToFile(const QString& path, const QString& separator, QLocale::Language language) const {
	QFile file(path);
	if (!file.open(QFile::WriteOnly | QFile::Truncate))
		return;

	QTextStream out(&file);

	QString sep = separator;
	sep = sep.replace(QLatin1String("TAB"), QLatin1String("\t"), Qt::CaseInsensitive);
	sep = sep.replace(QLatin1String("SPACE"), QLatin1String(" "), Qt::CaseInsensitive);

	//export values
	const int cols = m_matrix->columnCount();
	const int rows = m_matrix->rowCount();
	const QVector<QVector<double> >* data = static_cast<QVector<QVector<double>>*>(m_matrix->data());
	QLocale locale(language);
	for (int row = 0; row < rows; ++row) {
		for (int col = 0; col < cols; ++col) {
			out << locale.toString(data->at(col)[row], m_matrix->numericFormat(), m_matrix->precision());

			out << data->at(col)[row];
			if (col != cols-1)
				out << sep;
		}
		out << '\n';
	}
}

void MatrixView::exportToLaTeX(const QString& path, const bool verticalHeaders, const bool horizontalHeaders,
                               const bool latexHeaders, const bool gridLines, const bool entire, const bool captions) const {
	QFile file(path);
	if (!file.open(QFile::WriteOnly | QFile::Truncate))
		return;

	QVector<QVector<QString> > toExport;

	int firstSelectedCol = 0;
	int firstSelectedRowi = 0;
	int totalRowCount = 0;
	int cols = 0;
	if (entire) {
		cols = m_matrix->columnCount();
		totalRowCount = m_matrix->rowCount();
		toExport.reserve(totalRowCount);
		toExport.resize(totalRowCount);
		for (int row = 0; row < totalRowCount; ++row) {
			toExport[row].reserve(cols);
			toExport[row].resize(cols);
			//TODO: mode
			for (int col = 0; col < cols; ++col)
				toExport[row][col] = m_matrix->text<double>(row,col);
		}
		firstSelectedCol = 0;
		firstSelectedRowi = 0;
	} else {
		cols = selectedColumnCount();
		totalRowCount = selectedRowCount();

		firstSelectedCol = firstSelectedColumn();
		if (firstSelectedCol == -1)
			return;
		firstSelectedRowi = firstSelectedRow();
		if (firstSelectedRowi == -1)
			return;
		const int lastSelectedCol = lastSelectedColumn();
		const int lastSelectedRowi = lastSelectedRow();

		toExport.reserve(lastSelectedRowi - firstSelectedRowi+1);
		toExport.resize(lastSelectedRowi - firstSelectedRowi+1);
		int r = 0;
		for (int row = firstSelectedRowi; row <= lastSelectedRowi; ++row, ++r) {
			toExport[r].reserve(lastSelectedCol - firstSelectedCol+1);
			toExport[r].resize(lastSelectedCol - firstSelectedCol+1);
			int c = 0;
			//TODO: mode
			for (int col = firstSelectedCol; col <= lastSelectedCol; ++col,++c)
				toExport[r][c] = m_matrix->text<double>(row, col);
		}
	}

	int columnsStringSize = 0;
	int headerStringSize = 0;
	int columnsPerTable = 0;
	const int firstHHeaderSectionLength = m_tableView->model()->headerData(0, Qt::Horizontal).toString().length();
	const int firstSelectedVHeaderSectionLength = m_tableView->model()->headerData(firstSelectedRow(), Qt::Vertical).toString().length();
	if (verticalHeaders) {
		if (entire)
			headerStringSize += firstHHeaderSectionLength;
		else
			headerStringSize += firstSelectedVHeaderSectionLength;
	}
	if (!horizontalHeaders && verticalHeaders) {
		if (entire)
			columnsStringSize += firstHHeaderSectionLength;
		else
			columnsStringSize += firstSelectedVHeaderSectionLength;
	}

	for (int col = 0; col < cols; ++col) {
		int maxSize = -1;
		for (auto row : toExport) {
			if (row.at(col).size() > maxSize)
				maxSize = row.at(col).size();
		}
		columnsStringSize += maxSize;
		if (horizontalHeaders)
			headerStringSize += m_tableView->model()->headerData(col, Qt::Horizontal).toString().length();
		if ((columnsStringSize > 65) || (headerStringSize > 65))
			break;
		++columnsPerTable;
	}

	int tablesCount = (columnsPerTable != 0) ? cols/columnsPerTable : 0;
	const int remainingColumns = (columnsPerTable != 0) ? cols % columnsPerTable : cols;

	bool columnsSeparating = (cols > columnsPerTable);
	QTextStream out(&file);

	QProcess tex;
	tex.start("latex", QStringList() << "--version", QProcess::ReadOnly);
	tex.waitForFinished(500);
	QString texVersionOutput = QString(tex.readAllStandardOutput());
	texVersionOutput = texVersionOutput.split('\n')[0];

	int yearidx = -1;
	for (int i = texVersionOutput.size() - 1; i >= 0; --i) {
		if (texVersionOutput.at(i) == QChar('2')) {
			yearidx = i;
			break;
		}
	}

	if (texVersionOutput.at(yearidx+1) == QChar('/'))
		yearidx-=3;

	bool ok;
	texVersionOutput.midRef(yearidx, 4).toInt(&ok);
	int version = -1;
	if (ok)
		version = texVersionOutput.midRef(yearidx, 4).toInt(&ok);

	if (latexHeaders) {
		out << QLatin1String("\\documentclass[11pt,a4paper]{article} \n");
		out << QLatin1String("\\usepackage{geometry} \n");
		out << QLatin1String("\\usepackage{xcolor,colortbl} \n");
		if (version >= 2015)
			out << QLatin1String("\\extrafloats{1280} \n");
		out << QLatin1String("\\definecolor{HeaderBgColor}{rgb}{0.81,0.81,0.81} \n");
		out << QLatin1String("\\geometry{ \n");
		out << QLatin1String("a4paper, \n");
		out << QLatin1String("total={170mm,257mm}, \n");
		out << QLatin1String("left=10mm, \n");
		out << QLatin1String("top=10mm } \n");

		out << QLatin1String("\\begin{document} \n");
		out << QLatin1String("\\title{LabPlot Matrix Export to \\LaTeX{} } \n");
		out << QLatin1String("\\author{LabPlot} \n");
		out << QLatin1String("\\date{\\today} \n");
		// out << "\\maketitle \n";
	}

	const QString endTabularTable ("\\end{tabular} \n \\end{table} \n");
	const QString tableCaption ("\\caption{"+ m_matrix->name() + "} \n");
	const QString beginTable ("\\begin{table}[ht] \n");
	const QString centeredColumn( gridLines ? QLatin1String(" c |") : QLatin1String(" c "));
	int rowCount = 0;
	const int maxRows = 45;
	bool captionRemoved = false;

	if (columnsSeparating) {
		for (int table = 0; table < tablesCount; ++table) {
			QStringList textable;
			captionRemoved = false;

			textable << beginTable;
			if (captions)
				textable << tableCaption;
			textable << QLatin1String("\\centering \n");
			textable << QLatin1String("\\begin{tabular}{");
			textable << (gridLines ? QStringLiteral("|") : QString());
			for (int i = 0; i < columnsPerTable; ++i)
				textable << centeredColumn;
			if (verticalHeaders)
				textable << centeredColumn;
			textable << QLatin1String("} \n");
			if (gridLines)
				textable << QLatin1String("\\hline \n");

			if (horizontalHeaders) {
				if (latexHeaders)
					textable << QLatin1String("\\rowcolor{HeaderBgColor} \n");
				if (verticalHeaders)
					textable << QLatin1String(" & ");
				for (int col = table*columnsPerTable; col < (table * columnsPerTable) + columnsPerTable; ++col) {
					textable << m_tableView->model()->headerData(col + firstSelectedCol, Qt::Horizontal).toString();
					if (col != ((table * columnsPerTable)+ columnsPerTable)-1)
						textable << QLatin1String(" & ");
				}
				textable << QLatin1String("\\\\ \n");
				if (gridLines)
					textable << QLatin1String("\\hline \n");
			}
			for (const auto& s : textable)
				out << s;
			for (int row = 0; row < totalRowCount; ++row) {
				if (verticalHeaders) {
					out << "\\cellcolor{HeaderBgColor} ";
					out << m_tableView->model()->headerData(row + firstSelectedRowi, Qt::Vertical).toString();
					out << QLatin1String(" & ");
				}
				for (int col = table*columnsPerTable; col < (table * columnsPerTable) + columnsPerTable; ++col ) {
					out << toExport.at(row).at(col);
					if (col != ((table * columnsPerTable)+ columnsPerTable)-1)
						out << QLatin1String(" & ");
				}

				out << QLatin1String("\\\\ \n");
				if (gridLines)
					out << QLatin1String("\\hline \n");
				rowCount++;
				if (rowCount == maxRows) {
					out << endTabularTable;
					out << QLatin1String("\\newpage \n");
					if (captions)
						if (!captionRemoved)
							textable.removeAt(1);
					for (const auto& s : textable)
						out << s;
					rowCount = 0;
					if (!captionRemoved)
						captionRemoved = true;
				}
			}
			out << endTabularTable;
		}
		captionRemoved = false;

		QStringList remainingTable;
		remainingTable << beginTable;
		if (captions)
			remainingTable << tableCaption;
		remainingTable << QLatin1String("\\centering \n");
		remainingTable << QLatin1String("\\begin{tabular}{") << (gridLines ? QStringLiteral("|") : QString());
		for (int c = 0; c < remainingColumns; ++c)
			remainingTable << centeredColumn;
		if (verticalHeaders)
			remainingTable << centeredColumn;
		remainingTable << QLatin1String("} \n");
		if (gridLines)
			remainingTable << QLatin1String("\\hline \n");

		if (horizontalHeaders) {
			if (latexHeaders)
				remainingTable << QLatin1String("\\rowcolor{HeaderBgColor} \n");
			if (verticalHeaders)
				remainingTable << QLatin1String(" & ");
			for (int col = 0; col < remainingColumns; ++col) {
				remainingTable << m_tableView->model()->headerData(firstSelectedCol+col + (tablesCount * columnsPerTable), Qt::Horizontal).toString();
				if (col != remainingColumns-1)
					remainingTable << QLatin1String(" & ");
			}
			remainingTable << QLatin1String("\\\\ \n");
			if (gridLines)
				remainingTable << QLatin1String("\\hline \n");
		}

		for (const auto& s : remainingTable)
			out << s;

		for (int row = 0; row < totalRowCount; ++row) {
			if (verticalHeaders) {
				out << "\\cellcolor{HeaderBgColor}";
				out << m_tableView->model()->headerData(row+ firstSelectedRowi, Qt::Vertical).toString();
				out << QLatin1String(" & ");
			}
			for (int col = 0; col < remainingColumns; ++col ) {
				out << toExport.at(row).at(col + (tablesCount * columnsPerTable));
				if (col != remainingColumns-1)
					out << QLatin1String(" & ");
			}

			out << QLatin1String("\\\\ \n");
			if (gridLines)
				out << QLatin1String("\\hline \n");
			rowCount++;
			if (rowCount == maxRows) {
				out << endTabularTable;
				out << QLatin1String("\\pagebreak[4] \n");
				if (captions)
					if (!captionRemoved)
						remainingTable.removeAt(1);
				for (const auto& s : remainingTable)
					out << s;
				rowCount = 0;
				if (!captionRemoved)
					captionRemoved = true;
			}
		}
		out << endTabularTable;
	} else {
		QStringList textable;
		textable << beginTable;
		if (captions)
			textable << tableCaption;
		textable << QLatin1String("\\centering \n");
		textable << QLatin1String("\\begin{tabular}{") << (gridLines ? QStringLiteral("|") : QString());
		for (int c = 0; c < cols; ++c)
			textable << centeredColumn;
		if (verticalHeaders)
			textable << centeredColumn;
		textable << QLatin1String("} \n");
		if (gridLines)
			textable << QLatin1String("\\hline \n");

		if (horizontalHeaders) {
			if (latexHeaders)
				textable << QLatin1String("\\rowcolor{HeaderBgColor} \n");
			if (verticalHeaders)
				textable << QLatin1String(" & ");
			for (int col = 0; col < cols ; ++col) {
				textable << m_tableView->model()->headerData(col+firstSelectedCol, Qt::Horizontal).toString();
				if (col != cols-1)
					textable << QLatin1String(" & ");
			}
			textable << QLatin1String("\\\\ \n");
			if (gridLines)
				textable << QLatin1String("\\hline \n");
		}

		for (const auto& s : textable)
			out << s;
		for (int row = 0; row < totalRowCount; ++row) {
			if (verticalHeaders) {
				out << "\\cellcolor{HeaderBgColor}";
				out << m_tableView->model()->headerData(row + firstSelectedRowi, Qt::Vertical).toString();
				out << QLatin1String(" & ");
			}
			for (int col = 0; col < cols; ++col ) {
				out << toExport.at(row).at(col);
				if (col != cols-1)
					out << " & ";
			}
			out << QLatin1String("\\\\ \n");
			if (gridLines)
				out << QLatin1String("\\hline \n");
			rowCount++;
			if (rowCount == maxRows) {
				out << endTabularTable;
				out << QLatin1String("\\newpage \n");
				if (captions)
					if (!captionRemoved)
						textable.removeAt(1);
				for (const auto& s : textable)
					out << s;
				if (!captionRemoved)
					captionRemoved = true;
				rowCount = 0;
				if (!captionRemoved)
					captionRemoved = true;
			}
		}
		out << endTabularTable;
	}
	if (latexHeaders)
		out << QLatin1String("\\end{document} \n");
}

void MatrixView::showColumnStatistics() {
	if (selectedColumnCount() > 0) {
		QString dlgTitle (m_matrix->name() + " column statistics");
		auto* dlg = new StatisticsDialog(dlgTitle);
		QVector<Column*> columns;
		for (int col = 0; col < m_matrix->columnCount(); ++col) {
			if (isColumnSelected(col, false)) {
				QString headerString = m_tableView->model()->headerData(col, Qt::Horizontal).toString();
				columns << new Column(headerString, static_cast<QVector<QVector<double>>*>(m_matrix->data())->at(col));
			}
		}
		dlg->setColumns(columns);
		if (dlg->exec() == QDialog::Accepted) {
			qDeleteAll(columns);
			columns.clear();
		}
	}
}

void MatrixView::showRowStatistics() {
	if (selectedRowCount() > 0) {
		QString dlgTitle (m_matrix->name() + " row statistics");
		auto* dlg = new StatisticsDialog(dlgTitle);
		QVector<Column*> columns;
		for (int row = 0; row < m_matrix->rowCount(); ++row) {
			if (isRowSelected(row, false)) {
				QString headerString = m_tableView->model()->headerData(row, Qt::Vertical).toString();
				//TODO: mode
				columns << new Column(headerString, m_matrix->rowCells<double>(row, 0, m_matrix->columnCount()-1));
			}
		}
		dlg->setColumns(columns);
		if (dlg->exec() == QDialog::Accepted) {
			qDeleteAll(columns);
			columns.clear();
		}
	}
}

void MatrixView::exportToFits(const QString &fileName, const int exportTo) const {
	auto* filter = new FITSFilter;
	filter->setExportTo(exportTo);
	filter->write(fileName, m_matrix);

	delete filter;
}

