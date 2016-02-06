/***************************************************************************
    File                 : MatrixView.cpp
    Project              : LabPlot
    Description          : View class for Matrix
    --------------------------------------------------------------------
    Copyright            : (C) 2015 Alexander Semke (alexander.semke@web.de)
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


#include "commonfrontend/matrix/MatrixView.h"
#include "backend/matrix/Matrix.h"
#include "backend/matrix/MatrixModel.h"
#include "backend/matrix/matrixcommands.h"
#include "backend/lib/macros.h"
#include "kdefrontend/matrix/MatrixFunctionDialog.h"

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
// #include <QElapsedTimer>
// #include <QDebug>

#include <KLocale>
#include <KAction>
#include <KIcon>

#include <float.h>
#include "math.h"

MatrixView::MatrixView(Matrix* matrix) : QWidget(),
	m_stackedWidget(new QStackedWidget(this)),
	m_tableView(new QTableView(this)),
	m_imageLabel(new QLabel(this)),
	m_matrix(matrix),
	m_model(new MatrixModel(matrix)),
	m_imageIsDirty(true) {

	init();
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

	QHBoxLayout* layout = new QHBoxLayout(this);
	layout->setContentsMargins(0,0,0,0);
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
	h_header->setResizeMode(QHeaderView::Interactive);
	h_header->setMovable(false);
	h_header->setDefaultSectionSize(m_matrix->defaultColumnWidth());
	h_header->installEventFilter(this);

	//vertical header
	QHeaderView* v_header = m_tableView->verticalHeader();
	v_header->setResizeMode(QHeaderView::Interactive);
	v_header->setMovable(false);
	v_header->setDefaultSectionSize(m_matrix->defaultRowHeight());
	v_header->installEventFilter(this);

	//set the header sizes to the (potentially user customized) sizes stored in Matrix
	adjustHeaders();

	//image view
	QScrollArea* area = new QScrollArea(this);
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
	action_cut_selection = new KAction(KIcon("edit-cut"), i18n("Cu&t"), this);
	action_copy_selection = new KAction(KIcon("edit-copy"), i18n("&Copy"), this);
	action_paste_into_selection = new KAction(KIcon("edit-paste"), i18n("Past&e"), this);
	action_clear_selection = new KAction(KIcon("edit-clear"), i18n("Clea&r Selection"), this);
	action_select_all = new KAction(KIcon("edit-select-all"), i18n("Select All"), this);

	// matrix related actions
	QActionGroup* viewActionGroup = new QActionGroup(this);
	viewActionGroup->setExclusive(true);
	action_data_view = new KAction(KIcon("labplot-matrix"), i18n("Data"), viewActionGroup);
	action_data_view->setCheckable(true);
	action_data_view->setChecked(true);
	action_image_view = new KAction(KIcon("image-x-generic"), i18n("Image"), viewActionGroup);
	action_image_view->setCheckable(true);
	connect(viewActionGroup, SIGNAL(triggered(QAction*)), this, SLOT(switchView(QAction*)));

	action_fill_function = new KAction(KIcon(""), i18n("Function Values"), this);
	action_fill_const = new KAction(KIcon(""), i18n("Const Values"), this);
	action_clear_matrix = new KAction(KIcon("edit-clear"), i18n("Clear Matrix"), this);
	action_go_to_cell = new KAction(KIcon("go-jump"), i18n("&Go to Cell"), this);

	action_transpose = new KAction(i18n("&Transpose"), this);
	action_mirror_horizontally = new KAction(KIcon("object-flip-horizontal"), i18n("Mirror &Horizontally"), this);
	action_mirror_vertically = new KAction(KIcon("object-flip-vertical"), i18n("Mirror &Vertically"), this);
// 	action_duplicate = new QAction(i18nc("duplicate matrix", "&Duplicate"), this);

	QActionGroup* headerFormatActionGroup = new QActionGroup(this);
	headerFormatActionGroup->setExclusive(true);
	action_header_format_1= new QAction(i18n("Rows and Columns"), headerFormatActionGroup);
	action_header_format_1->setCheckable(true);
	action_header_format_2= new QAction(i18n("xy-Values"), headerFormatActionGroup);
	action_header_format_2->setCheckable(true);
	action_header_format_3= new QAction(i18n("Rows, Columns and xy-Values"), headerFormatActionGroup);
	action_header_format_3->setCheckable(true);
	if (m_matrix->headerFormat() == Matrix::HeaderRowsColumns)
		action_header_format_1->setChecked(true);
	else if (m_matrix->headerFormat() == Matrix::HeaderValues)
		action_header_format_2->setChecked(true);
	else
		action_header_format_3->setChecked(true);
	connect(headerFormatActionGroup, SIGNAL(triggered(QAction*)), this, SLOT(headerFormatChanged(QAction*)));

	// column related actions
	action_add_columns = new KAction(KIcon("edit-table-insert-column-right"), i18n("&Add Columns"), this);
	action_insert_columns = new KAction(KIcon("edit-table-insert-column-left"), i18n("&Insert Empty Columns"), this);
	action_remove_columns = new KAction(KIcon("edit-table-delete-column"), i18n("Remo&ve Columns"), this);
	action_clear_columns = new KAction(KIcon("edit-clear"), i18n("Clea&r Columns"), this);

	// row related actions
	action_add_rows = new KAction(KIcon("edit-table-insert-row-above"), i18n("&Add Rows"), this);
	action_insert_rows = new KAction(KIcon("edit-table-insert-row-above") ,i18n("&Insert Empty Rows"), this);
	action_remove_rows = new KAction(KIcon("edit-table-delete-row"), i18n("Remo&ve Rows"), this);
	action_clear_rows = new KAction(KIcon("edit-clear"), i18n("Clea&r Rows"), this);
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

	// column related actions
	connect(action_add_columns, SIGNAL(triggered()), this, SLOT(addColumns()));
	connect(action_insert_columns, SIGNAL(triggered()), this, SLOT(insertEmptyColumns()));
	connect(action_remove_columns, SIGNAL(triggered()), this, SLOT(removeSelectedColumns()));
	connect(action_clear_columns, SIGNAL(triggered()), this, SLOT(clearSelectedColumns()));

	// row related actions
	connect(action_add_rows, SIGNAL(triggered()), this, SLOT(addRows()));
	connect(action_insert_rows, SIGNAL(triggered()), this, SLOT(insertEmptyRows()));
	connect(action_remove_rows, SIGNAL(triggered()), this, SLOT(removeSelectedRows()));
	connect(action_clear_rows, SIGNAL(triggered()), this, SLOT(clearSelectedRows()));
}

void MatrixView::initMenus() {
	//selection menu
	m_selectionMenu = new QMenu(i18n("Selection"));
	m_selectionMenu->addAction(action_cut_selection);
	m_selectionMenu->addAction(action_copy_selection);
	m_selectionMenu->addAction(action_paste_into_selection);
	m_selectionMenu->addAction(action_clear_selection);

	//column menu
	m_columnMenu = new QMenu();
	m_columnMenu->addAction(action_insert_columns);
	m_columnMenu->addAction(action_remove_columns);
	m_columnMenu->addAction(action_clear_columns);

	//row menu
	m_rowMenu = new QMenu();
	m_rowMenu->addAction(action_insert_rows);
	m_rowMenu->addAction(action_remove_rows);
	m_rowMenu->addAction(action_clear_rows);

	//matrix menu
	m_matrixMenu = new QMenu();

	m_matrixMenu->addMenu(m_selectionMenu);
	m_matrixMenu->addSeparator();

	QMenu* submenu = new QMenu(i18n("Generate Data"));
	submenu->addAction(action_fill_const);
	submenu->addAction(action_fill_function);
	m_matrixMenu->addMenu(submenu);
	m_matrixMenu->addSeparator();

	submenu = new QMenu(i18n("View"));
	submenu->addAction(action_data_view);
	submenu->addAction(action_image_view);
	m_matrixMenu->addMenu(submenu);
	m_matrixMenu->addSeparator();

	m_matrixMenu->addAction(action_select_all);
	m_matrixMenu->addAction(action_clear_matrix);
	m_matrixMenu->addSeparator();

	m_matrixMenu->addAction(action_transpose);
	m_matrixMenu->addAction(action_mirror_horizontally);
	m_matrixMenu->addAction(action_mirror_vertically);
	m_matrixMenu->addSeparator();

	m_headerFormatMenu = new QMenu(i18n("Header format"));
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

	QAction* firstAction = 0;
	// if we're populating the context menu for the project explorer, then
	//there're already actions available there. Skip the first title-action
	//and insert the action at the beginning of the menu.
	if (menu->actions().size()>1)
		firstAction = menu->actions().at(1);

	menu->insertMenu(firstAction, m_selectionMenu);
	menu->insertSeparator(firstAction);

	QMenu* submenu = new QMenu(i18n("Generate Data"));
	submenu->addAction(action_fill_const);
	submenu->addAction(action_fill_function);
	menu->insertMenu(firstAction, submenu);
	menu->insertSeparator(firstAction);

	submenu = new QMenu(i18n("View"));
	submenu->addAction(action_data_view);
	submenu->addAction(action_image_view);
	menu->insertMenu(firstAction, submenu);
	menu->insertSeparator(firstAction);

	menu->insertAction(firstAction, action_select_all);
	menu->insertAction(firstAction, action_clear_matrix);
	menu->insertSeparator(firstAction);
	menu->insertAction(firstAction, action_transpose);
	menu->insertAction(firstAction, action_mirror_horizontally);
	menu->insertAction(firstAction, action_mirror_vertically);
	menu->insertSeparator(firstAction);
// 	menu->insertAction(firstAction, action_duplicate);
	menu->insertMenu(firstAction, m_headerFormatMenu);
	menu->insertSeparator(firstAction);
	menu->insertAction(firstAction, action_go_to_cell);
	menu->insertSeparator(firstAction);

	// TODO:
	// Convert to Spreadsheet
	// Export
}

void MatrixView::adjustHeaders() {
	QHeaderView* h_header = m_tableView->horizontalHeader();
	QHeaderView* v_header = m_tableView->verticalHeader();

	disconnect(v_header, SIGNAL(sectionResized(int,int,int)), this, SLOT(handleVerticalSectionResized(int,int,int)));
	disconnect(h_header, SIGNAL(sectionResized(int,int,int)), this, SLOT(handleHorizontalSectionResized(int,int,int)));

	int cols = m_matrix->columnCount();
	for (int i=0; i<cols; i++)
		h_header->resizeSection(i, m_matrix->columnWidth(i));
	int rows = m_matrix->rowCount();
	for (int i=0; i<rows; i++)
		v_header->resizeSection(i, m_matrix->rowHeight(i));

	connect(v_header, SIGNAL(sectionResized(int,int,int)), this, SLOT(handleVerticalSectionResized(int,int,int)));
	connect(h_header, SIGNAL(sectionResized(int,int,int)), this, SLOT(handleHorizontalSectionResized(int,int,int)));
}

void MatrixView::setRowHeight(int row, int height) {
	m_tableView->verticalHeader()->resizeSection(row, height);
}

void MatrixView::setColumnWidth(int col, int width) {
	m_tableView->horizontalHeader()->resizeSection(col, width);
}

int MatrixView::rowHeight(int row) const {
	return m_tableView->verticalHeader()->sectionSize(row);
}

int MatrixView::columnWidth(int col) const {
	return m_tableView->horizontalHeader()->sectionSize(col);
}

/*!
	Returns how many columns are selected.
	If full is true, this function only returns the number of fully selected columns.
*/
int MatrixView::selectedColumnCount(bool full) const {
	int count = 0;
	int cols = m_matrix->columnCount();
	for (int i=0; i<cols; i++)
		if(isColumnSelected(i, full)) count++;
	return count;
}

/*!
	Returns true if column 'col' is selected; otherwise false.
	If full is true, this function only returns true if the whole column is selected.
*/
bool MatrixView::isColumnSelected(int col, bool full) const {
	if(full)
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
	for (int i=0; i<rows; i++)
		if(isRowSelected(i, full)) count++;
	return count;
}

/*!
	Returns true if row \c row is selected; otherwise false
	If full is true, this function only returns true if the whole row is selected.
*/
bool MatrixView::isRowSelected(int row, bool full) const {
	if(full)
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
	for (int i=0; i<cols; i++)
	{
		if(isColumnSelected(i, full))
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
	for(int i=cols-1; i>=0; i--)
		if(isColumnSelected(i, full)) return i;

	return -2;
}

/*!
	Return the index of the first selected row.
	If full is true, this function only looks for fully selected rows.
*/
int MatrixView::firstSelectedRow(bool full) const {
	int rows = m_matrix->rowCount();
	for (int i=0; i<rows; i++) 	{
		if(isRowSelected(i, full))
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
	for(int i=rows-1; i>=0; i--)
		if(isRowSelected(i, full)) return i;

	return -2;
}

bool MatrixView::isCellSelected(int row, int col) const {
	if(row < 0 || col < 0 || row >= m_matrix->rowCount() || col >= m_matrix->columnCount()) return false;

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
	if(index.isValid()) {
		*row = index.row();
		*col = index.column();
	} else {
		*row = -1;
		*col = -1;
	}
}

bool MatrixView::eventFilter(QObject * watched, QEvent * event) {
	if (event->type() == QEvent::ContextMenu){
		QContextMenuEvent* cm_event = static_cast<QContextMenuEvent*>(event);
		QPoint global_pos = cm_event->globalPos();
		if (watched == m_tableView->verticalHeader()) {
			m_rowMenu->exec(global_pos);
		} else if (watched == m_tableView->horizontalHeader()) {
			m_columnMenu->exec(global_pos);
		} else if (watched == this) {
			m_matrixMenu->exec(global_pos);
		} else {
			return QWidget::eventFilter(watched, event);
		}
		return true;
	} else {
		return QWidget::eventFilter(watched, event);
	}
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
    if(idx.row()+1 < m_matrix->rowCount())
		m_tableView->setCurrentIndex(idx.sibling(idx.row()+1, idx.column()));
}

void MatrixView::goToCell(){
	bool ok;

	int col = QInputDialog::getInteger(0, i18n("Go to Cell"), i18n("Enter column"),
			1, 1, m_matrix->columnCount(), 1, &ok);
	if ( !ok ) return;

	int row = QInputDialog::getInteger(0, i18n("Go to Cell"), i18n("Enter row"),
			1, 1, m_matrix->rowCount(), 1, &ok);
	if ( !ok ) return;

	goToCell(row-1, col-1);
}

void MatrixView::goToCell(int row, int col) {
	QModelIndex index = m_model->index(row, col);
	m_tableView->scrollTo(index);
	m_tableView->setCurrentIndex(index);
}

void MatrixView::handleHorizontalSectionResized(int logicalIndex, int oldSize, int newSize) {
	Q_UNUSED(oldSize)
	static bool inside = false;
	m_matrix->setColumnWidth(logicalIndex, newSize);
	if (inside) return;
	inside = true;

	QHeaderView* h_header = m_tableView->horizontalHeader();
	int cols = m_matrix->columnCount();
	for (int i=0; i<cols; i++)
		if(isColumnSelected(i, true))
			h_header->resizeSection(i, newSize);

	inside = false;
}

void MatrixView::handleVerticalSectionResized(int logicalIndex, int oldSize, int newSize) {
	Q_UNUSED(oldSize)
	static bool inside = false;
	m_matrix->setRowHeight(logicalIndex, newSize);
	if (inside) return;
	inside = true;

	QHeaderView* v_header = m_tableView->verticalHeader();
	int rows = m_matrix->rowCount();
	for (int i=0; i<rows; i++)
		if(isRowSelected(i, true))
			v_header->resizeSection(i, newSize);

	inside = false;
}

void MatrixView::fillWithFunctionValues() {
	MatrixFunctionDialog* dlg = new MatrixFunctionDialog(m_matrix);
	dlg->setAttribute(Qt::WA_DeleteOnClose);
	dlg->exec();
}

void MatrixView::fillWithConstValues() {
	bool ok = false;
	double value = QInputDialog::getDouble(this, i18n("Fill the matrix with constant value"),
												i18n("Value"), 0, -2147483647, 2147483647, 6, &ok);
	if (ok) {
		WAIT_CURSOR;
		QVector<QVector<double> > newData = m_matrix->data();
		for (int col=0; col<m_matrix->columnCount(); ++col) {
			for (int row=0; row<m_matrix->rowCount(); ++row) {
				newData[col][row] = value;
			}
		}
		m_matrix->setData(newData);
		RESET_CURSOR;
	}
}

//############################ selection related slots #########################
void MatrixView::cutSelection() {
	int first = firstSelectedRow();
	if( first < 0 ) return;

	WAIT_CURSOR;
	m_matrix->beginMacro(i18n("%1: cut selected cell(s)", m_matrix->name()));
	copySelection();
	clearSelectedCells();
	m_matrix->endMacro();
	RESET_CURSOR;
}

void MatrixView::copySelection() {
	int first_col = firstSelectedColumn(false);
	if(first_col == -1) return;
	int last_col = lastSelectedColumn(false);
	if(last_col == -2) return;
	int first_row = firstSelectedRow(false);
	if(first_row == -1)	return;
	int last_row = lastSelectedRow(false);
	if(last_row == -2) return;
	int cols = last_col - first_col +1;
	int rows = last_row - first_row +1;

	WAIT_CURSOR;
	QString output_str;

	for(int r=0; r<rows; r++) 	{
		for(int c=0; c<cols; c++) {
			if(isCellSelected(first_row + r, first_col + c))
				output_str += QLocale().toString(m_matrix->cell(first_row + r, first_col + c),
						m_matrix->numericFormat(), 16); // copy with max. precision
			if(c < cols-1)
				output_str += '\t';
		}
		if(r < rows-1)
			output_str += '\n';
	}
	QApplication::clipboard()->setText(output_str);
	RESET_CURSOR;
}

void MatrixView::pasteIntoSelection() {
	if(m_matrix->columnCount() < 1 || m_matrix->rowCount() < 1) return;

	const QMimeData* mime_data = QApplication::clipboard()->mimeData();
	if(!mime_data->hasFormat("text/plain"))
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
	for(int i=0; i<input_row_count; i++) {
		cell_texts.append(input_rows.at(i).split('\t'));
		if(cell_texts.at(i).count() > input_col_count) input_col_count = cell_texts.at(i).count();
	}

	// if the is no selection or only one cell selected, the
	// selection will be expanded to the needed size from the current cell
	if( (first_col == -1 || first_row == -1) ||
		(last_row == first_row && last_col == first_col) ) {
		int current_row, current_col;
		getCurrentCell(&current_row, &current_col);
		if(current_row == -1) current_row = 0;
		if(current_col == -1) current_col = 0;
		setCellSelected(current_row, current_col);
		first_col = current_col;
		first_row = current_row;
		last_row = first_row + input_row_count -1;
		last_col = first_col + input_col_count -1;
		// resize the matrix if necessary
		if(last_col >= m_matrix->columnCount())
			m_matrix->appendColumns(last_col+1-m_matrix->columnCount());
		if(last_row >= m_matrix->rowCount())
			m_matrix->appendRows(last_row+1-m_matrix->rowCount());
		// select the rectangle to be pasted in
		setCellsSelected(first_row, first_col, last_row, last_col);
	}

	rows = last_row - first_row + 1;
	cols = last_col - first_col + 1;
	for(int r=0; r<rows && r<input_row_count; r++) {
		for(int c=0; c<cols && c<input_col_count; c++) {
			if(isCellSelected(first_row + r, first_col + c) && (c < cell_texts.at(r).count()) ) {
				m_matrix->setCell(first_row + r, first_col + c, cell_texts.at(r).at(c).toDouble());
			}
		}
	}

	m_matrix->endMacro();
	RESET_CURSOR;
}

void MatrixView::clearSelectedCells() {
	int first_row = firstSelectedRow();
	if(first_row<0)
		return;

	int first_col = firstSelectedColumn();
	if(first_col<0)
		return;

	int last_row = lastSelectedRow();
	int last_col = lastSelectedColumn();

	WAIT_CURSOR;
	m_matrix->beginMacro(i18n("%1: clear selected cell(s)", m_matrix->name()));
	for(int i=first_row; i<=last_row; i++) {
		for(int j=first_col; j<=last_col; j++) {
			if(isCellSelected(i, j))
				m_matrix->clearCell(i, j);
		}
	}
	m_matrix->endMacro();
	RESET_CURSOR;
}


QMutex mutex;
class UpdateImageTask : public QRunnable {
	public:
		UpdateImageTask(int start, int end, QImage image, QVector<QVector<double> > matrixData, double scaleFactor, double min) : m_image(image), m_matrixData(matrixData) {
			m_start = start;
			m_end = end;
			m_scaleFactor = scaleFactor;
			m_min = min;
		};

		void run() {
			for (int row=m_start; row<m_end; ++row) {
				mutex.lock();
				QRgb* line = (QRgb*)m_image.scanLine(row);
				mutex.unlock();
				for (int col=0; col<m_image.width(); ++col) {
					const int gray = (m_matrixData[col][row]-m_min)*m_scaleFactor;
					line[col] = qRgb(gray, gray, gray);
				}
			}
		}

	private:
		int m_start;
		int m_end;
		QImage& m_image;
		const QVector<QVector<double> >& m_matrixData;
		double m_scaleFactor;
		double m_min;
};

void MatrixView::updateImage() {
	WAIT_CURSOR;
	m_image = QImage(m_matrix->columnCount(), m_matrix->rowCount(), QImage::Format_ARGB32);

	//find min/max value
// 	QTime timer;
// 	timer.start();
	double dmax= -DBL_MAX, dmin= DBL_MAX;
	const QVector<QVector<double> >& matrixData = m_matrix->data();
	const int width = m_matrix->columnCount();
	const int height = m_matrix->rowCount();
	for (int col=0; col<width; ++col) {
		for (int row=0; row<height; ++row) {
			const double value = matrixData[col][row];
			if (dmax<value) dmax=value;
			if (dmin>value) dmin=value;
		}
	}
// 	qDebug()<<"min/max determined in " << (float)timer.elapsed()/1000 << "s";

	//update the image
// 	timer.start();
	const double scaleFactor = 255.0/(dmax-dmin);
	QThreadPool* pool = QThreadPool::globalInstance();
	int range = ceil(double(m_image.height())/pool->maxThreadCount());
	for (int i=0; i<pool->maxThreadCount(); ++i) {
		const int start = i*range;
		int end = (i+1)*range;
		if (end>m_image.height()) end = m_image.height();
		UpdateImageTask* task = new UpdateImageTask(start, end, m_image, matrixData, scaleFactor, dmin);
		pool->start(task);
	}
	pool->waitForDone();
// 	qDebug()<<"image updated in " << (float)timer.elapsed()/1000 << "s";

	m_imageLabel->resize(width, height);
	m_imageLabel->setPixmap(QPixmap::fromImage(m_image));
	m_imageIsDirty = false;
	RESET_CURSOR;
}

//############################# matrix related slots ###########################
void MatrixView::switchView(QAction* action) {
	if (action == action_data_view) {
		m_stackedWidget->setCurrentIndex(0);
	} else {
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
	if( first < 0 ) return;
	int count, current = first;

	WAIT_CURSOR;
	m_matrix->beginMacro(i18n("%1: insert empty column(s)", m_matrix->name()));
	while( current <= last ) {
		current = first+1;
		while( current <= last && isColumnSelected(current) ) current++;
		count = current-first;
		m_matrix->insertColumns(first, count);
		current += count;
		last += count;
		while( current <= last && isColumnSelected(current) ) current++;
		first = current;
	}
	m_matrix->endMacro();
	RESET_CURSOR;
}

void MatrixView::removeSelectedColumns() {
	int first = firstSelectedColumn();
	int last = lastSelectedColumn();
	if( first < 0 ) return;

	WAIT_CURSOR;
	m_matrix->beginMacro(i18n("%1: remove selected column(s)", m_matrix->name()));
	for(int i=last; i>=first; i--)
		if(isColumnSelected(i, false)) m_matrix->removeColumns(i, 1);
	m_matrix->endMacro();
	RESET_CURSOR;
}

void MatrixView::clearSelectedColumns() {
	WAIT_CURSOR;
	m_matrix->beginMacro(i18n("%1: clear selected column(s)", m_matrix->name()));
	for(int i=0; i<m_matrix->columnCount(); i++) {
		if(isColumnSelected(i, false))
			m_matrix->clearColumn(i);
	}
	m_matrix->endMacro();
	RESET_CURSOR;
}

//############################## rows related slots ############################
/*!
  Append as many rows as are selected.
*/
void MatrixView::addRows(){
	m_matrix->appendRows(selectedRowCount(false));
}

void MatrixView::insertEmptyRows() {
	int first = firstSelectedRow();
	int last = lastSelectedRow();
	int count, current = first;

	if( first < 0 ) return;

	WAIT_CURSOR;
	m_matrix->beginMacro(i18n("%1: insert empty rows(s)", m_matrix->name()));
	while( current <= last ) {
		current = first+1;
		while( current <= last && isRowSelected(current) ) current++;
		count = current-first;
		m_matrix->insertRows(first, count);
		current += count;
		last += count;
		while( current <= last && !isRowSelected(current) ) current++;
		first = current;
	}
	m_matrix->endMacro();
	RESET_CURSOR;
}

void MatrixView::removeSelectedRows() {
	int first = firstSelectedRow();
	int last = lastSelectedRow();
	if( first < 0 ) return;

	WAIT_CURSOR;
	m_matrix->beginMacro(i18n("%1: remove selected rows(s)", m_matrix->name()));
	for(int i=last; i>=first; i--)
		if(isRowSelected(i, false)) m_matrix->removeRows(i, 1);
	m_matrix->endMacro();
	RESET_CURSOR;
}

void MatrixView::clearSelectedRows() {
	int first = firstSelectedRow();
	int last = lastSelectedRow();
	if( first < 0 ) return;

	WAIT_CURSOR;
	m_matrix->beginMacro(i18n("%1: clear selected rows(s)", m_matrix->name()));
	for(int i=first; i<=last; i++) {
		if(isRowSelected(i))
			m_matrix->clearRow(i);
	}
	m_matrix->endMacro();
	RESET_CURSOR;
}

/*!
  prints the complete matrix to \c printer.
 */
void MatrixView::print(QPrinter* printer) const {
	QPainter painter (printer);

	int dpiy = printer->logicalDpiY();
	const int margin = (int) ( (1/2.54)*dpiy ); // 1 cm margins

	QHeaderView* hHeader = m_tableView->horizontalHeader();
	QHeaderView* vHeader = m_tableView->verticalHeader();

	int rows = m_matrix->rowCount();
	int cols = m_matrix->columnCount();
	int height = margin;
	int i;
	int vertHeaderWidth = vHeader->width();
	int right = margin + vertHeaderWidth;

	//Paint the horizontal header first
	painter.setFont(hHeader->font());
	QString headerString = m_tableView->model()->headerData(0, Qt::Horizontal).toString();
	QRect br;
	br = painter.boundingRect(br, Qt::AlignCenter, headerString);
	painter.drawLine(right, height, right, height+br.height());
	QRect tr(br);

	int w;
	for (i=0; i<cols; ++i) {
		headerString = m_tableView->model()->headerData(i, Qt::Horizontal).toString();
		w = m_tableView->columnWidth(i);
		tr.setTopLeft(QPoint(right,height));
		tr.setWidth(w);
		tr.setHeight(br.height());

		painter.drawText(tr, Qt::AlignCenter, headerString);
		right += w;
		painter.drawLine(right, height, right, height+tr.height());

		if (right >= printer->pageRect().width()-2*margin )
			break;
	}

	painter.drawLine(margin + vertHeaderWidth, height, right-1, height);//first horizontal line
	height += tr.height();
	painter.drawLine(margin, height, right-1, height);


	// print table values
	const QVector<QVector<double> >& matrixData = m_matrix->data();
	QString cellText;
	for (i=0; i<rows; ++i) {
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

		for(int j=0;j<cols;j++){
			int w = m_tableView->columnWidth(j);
			cellText = QString::number(matrixData[j][i]) +'\t';
			tr = painter.boundingRect(tr,Qt::AlignCenter,cellText);
			br.setTopLeft(QPoint(right,height));
			br.setWidth(w);
			br.setHeight(tr.height());
			painter.drawText(br, Qt::AlignCenter, cellText);
			right += w;
			painter.drawLine(right, height, right, height+tr.height());

			if (right >= printer->width()-2*margin )
				break;
		}
		height += br.height();
		painter.drawLine(margin, height, right-1, height);

		if (height >= printer->height()-margin ){
			printer->newPage();
			height = margin;
			painter.drawLine(margin, height, right, height);
		}
	}
}

void MatrixView::exportToFile(const QString& path, const QString& separator) const {
	QFile file(path);
	if (!file.open(QFile::WriteOnly | QFile::Truncate))
		return;

	QTextStream out(&file);

	QString sep = separator;
	sep = sep.replace(QString("TAB"), QString("\t"), Qt::CaseInsensitive);
	sep = sep.replace(QString("SPACE"), QString(" "), Qt::CaseInsensitive);

	//export values
	const int cols = m_matrix->columnCount();
	const int rows = m_matrix->rowCount();
	const QVector<QVector<double> >& matrixData = m_matrix->data();
	for (int row=0; row<rows; ++row) {
		for (int col=0; col<cols; ++col) {
			out << matrixData[col][row];
			if (col!=cols-1)
				out<<sep;
		}
		out << '\n';
	}
}
