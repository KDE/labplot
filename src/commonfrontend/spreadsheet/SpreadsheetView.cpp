/***************************************************************************
    File                 : SpreadsheetView.cpp
    Project              : LabPlot
    Description          : View class for Spreadsheet
    --------------------------------------------------------------------
    Copyright            : (C) 2011-2015 by Alexander Semke (alexander.semke@web.de)

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

#include "SpreadsheetView.h"
#include "backend/spreadsheet/SpreadsheetModel.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "commonfrontend/spreadsheet/SpreadsheetItemDelegate.h"
#include "commonfrontend/spreadsheet/SpreadsheetDoubleHeaderView.h"
#include "backend/lib/macros.h"

#include "backend/core/column/Column.h"
#include "backend/core/datatypes/SimpleCopyThroughFilter.h"
#include "backend/core/datatypes/Double2StringFilter.h"
#include "backend/core/datatypes/String2DoubleFilter.h"
#include "backend/core/datatypes/DateTime2StringFilter.h"
#include "backend/core/datatypes/String2DateTimeFilter.h"

#include <QTableView>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QClipboard>
#include <QInputDialog>
#include <QDate>
#include <QApplication>
#include <QMenu>
#include <QPainter>
#include <QPrinter>
#include <QToolBar>
#include <QTextStream>
// #include <QDebug>

#include <KAction>
#include <KLocale>
#include "kdefrontend/spreadsheet/DropValuesDialog.h"
#include "kdefrontend/spreadsheet/SortDialog.h"
#include "kdefrontend/spreadsheet/RandomValuesDialog.h"
#include "kdefrontend/spreadsheet/EquidistantValuesDialog.h"
#include "kdefrontend/spreadsheet/FunctionValuesDialog.h"

#include <algorithm>

/*!
	\class SpreadsheetView
	\brief View class for Spreadsheet

	\ingroup commonfrontend
 */
SpreadsheetView::SpreadsheetView(Spreadsheet *spreadsheet):QWidget(),
	m_tableView(new QTableView(this)),
	m_spreadsheet(spreadsheet),
	m_model( new SpreadsheetModel(spreadsheet) ),
	m_suppressSelectionChangedEvent(false){

	QHBoxLayout* layout = new QHBoxLayout(this);
	layout->setContentsMargins(0,0,0,0);
	layout->addWidget(m_tableView);

	init();
}

SpreadsheetView::~SpreadsheetView(){
	delete m_model;
}

void SpreadsheetView::init(){
	initActions();
	initMenus();

	m_tableView->setModel(m_model);
	m_tableView->setItemDelegate(new SpreadsheetItemDelegate(this));
	m_tableView->setSelectionMode(QAbstractItemView::ExtendedSelection);

	//horizontal header
	m_horizontalHeader = new SpreadsheetDoubleHeaderView(this);
    m_horizontalHeader->setClickable(true);
    m_horizontalHeader->setHighlightSections(true);
	m_tableView->setHorizontalHeader(m_horizontalHeader);
	m_horizontalHeader->setResizeMode(QHeaderView::Interactive);
	m_horizontalHeader->setMovable(true);
	m_horizontalHeader->installEventFilter(this);
	connect(m_horizontalHeader, SIGNAL(sectionMoved(int,int,int)), this, SLOT(handleHorizontalSectionMoved(int,int,int)));
	connect(m_horizontalHeader, SIGNAL(sectionDoubleClicked(int)), this, SLOT(handleHorizontalHeaderDoubleClicked(int)));
	connect(m_horizontalHeader, SIGNAL(sectionResized(int,int,int)), this, SLOT(handleHorizontalSectionResized(int,int,int)));
	connect(m_horizontalHeader, SIGNAL(sectionClicked(int)), this, SLOT(columnClicked(int)) );

	int i=0;
	foreach(Column * col, m_spreadsheet->children<Column>())
		m_horizontalHeader->resizeSection(i++, col->width());

	// vertical header
	QHeaderView * v_header = m_tableView->verticalHeader();
	v_header->setResizeMode(QHeaderView::Fixed);
	v_header->setDefaultSectionSize(22);
	v_header->setMovable(false);
	v_header->installEventFilter(this);


	setFocusPolicy(Qt::StrongFocus);
	setFocus();
	installEventFilter(this);
	connectActions();
	showComments(false);

	connect(m_model, SIGNAL(headerDataChanged(Qt::Orientation,int,int)), this,
		SLOT(updateHeaderGeometry(Qt::Orientation,int,int)) );
	connect(m_model, SIGNAL(headerDataChanged(Qt::Orientation,int,int)), this,
		SLOT(handleHeaderDataChanged(Qt::Orientation,int,int)) );
	connect(m_spreadsheet, SIGNAL(aspectAdded(const AbstractAspect*)),
			this, SLOT(handleAspectAdded(const AbstractAspect*)));
	connect(m_spreadsheet, SIGNAL(aspectAboutToBeRemoved(const AbstractAspect*)),
			this, SLOT(handleAspectAboutToBeRemoved(const AbstractAspect*)));
	connect(m_spreadsheet, SIGNAL(requestProjectContextMenu(QMenu*)), this, SLOT(createContextMenu(QMenu*)));


	//selection relevant connections
	QItemSelectionModel* sel_model = m_tableView->selectionModel();
	connect(sel_model, SIGNAL(currentColumnChanged(QModelIndex,QModelIndex)),
		this, SLOT(currentColumnChanged(QModelIndex,QModelIndex)));
	connect(sel_model, SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
		this, SLOT(selectionChanged(QItemSelection,QItemSelection)));
	connect(sel_model, SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
			this, SLOT(selectionChanged(QItemSelection,QItemSelection)) );

	connect(m_spreadsheet, SIGNAL(columnSelected(int)), this, SLOT(selectColumn(int)) );
	connect(m_spreadsheet, SIGNAL(columnDeselected(int)), this, SLOT(deselectColumn(int)) );
}

void SpreadsheetView::initActions(){
	// selection related actions
	action_cut_selection = new KAction(KIcon("edit-cut"), i18n("Cu&t"), this);
	action_copy_selection = new KAction(KIcon("edit-copy"), i18n("&Copy"), this);
	action_paste_into_selection = new KAction(KIcon("edit-paste"), i18n("Past&e"), this);
	action_mask_selection = new KAction(KIcon("edit-node"), i18n("&Mask Selection"), this);
	action_unmask_selection = new KAction(KIcon("format-remove-node"), i18n("&Unmask Selection"), this);
	action_clear_selection = new KAction(KIcon("edit-clear"), i18n("Clea&r Selection"), this);
	action_select_all = new KAction(KIcon("edit-select-all"), i18n("Select All"), this);

// 	action_set_formula = new KAction(KIcon(""), i18n("Assign &Formula"), this);
// 	action_recalculate = new KAction(KIcon(""), i18n("Recalculate"), this);
	action_fill_row_numbers = new KAction(KIcon(""), i18n("Row Numbers"), this);
	action_fill_random = new KAction(KIcon(""), i18n("Uniform Random Values"), this);
	action_fill_random_nonuniform = new KAction(KIcon(""), i18n("Random Values"), this);
	action_fill_equidistant = new KAction(KIcon(""), i18n("Equidistant Values"), this);
	action_fill_function = new KAction(KIcon(""), i18n("Function Values"), this);
	action_fill_const = new KAction(KIcon(""), i18n("Const Values"), this);

	//spreadsheet related actions
	action_toggle_comments = new KAction(KIcon("document-properties"), i18n("Show Comments"), this);
	action_add_column = new KAction(KIcon("edit-table-insert-column-left"), i18n("&Add Column"), this);
	action_clear_spreadsheet = new KAction(KIcon("edit-clear"), i18n("Clear Spreadsheet"), this);
	action_clear_masks = new KAction(KIcon("format-remove-node"), i18n("Clear Masks"), this);
	action_sort_spreadsheet = new KAction(KIcon("view-sort-ascending"), i18n("&Sort Spreadsheet"), this);
	action_go_to_cell = new KAction(KIcon("go-jump"), i18n("&Go to Cell"), this);

	// column related actions
	action_insert_columns = new KAction(KIcon("edit-table-insert-column-left"), i18n("&Insert Empty Columns"), this);
	action_remove_columns = new KAction(KIcon("edit-table-delete-column"), i18n("Remo&ve Columns"), this);
	action_clear_columns = new KAction(KIcon("edit-clear"), i18n("Clea&r Columns"), this);
	action_add_columns = new KAction(KIcon("edit-table-insert-column-right"), i18n("&Add Columns"), this);
// 	action_set_as_x = new KAction(KIcon(""), i18n("X, Plot Designation"), this);
// 	action_set_as_y = new KAction(KIcon(""), i18n("Y, Plot Designation"), this);
// 	action_set_as_z = new KAction(KIcon(""), i18n("Z, Plot Designation"), this);
// 	action_set_as_xerr = new KAction(KIcon(""), i18n("X Error, Plot Designation"), this);
// 	action_set_as_yerr = new KAction(KIcon(""), i18n("Y Error, Plot Designation"), this);
// 	action_set_as_none = new KAction(KIcon(""), i18n("None, Plot Designation"), this);
	action_reverse_columns = new KAction(KIcon(""), i18n("Reverse"), this);
	action_drop_values = new KAction(KIcon(""), i18n("Drop Values"), this);
	action_mask_values = new KAction(KIcon(""), i18n("Mask Values"), this);
// 	action_join_columns = new KAction(KIcon(""), i18n("Join"), this);
	action_normalize_columns = new KAction(KIcon(""), i18n("&Normalize"), this);
	action_normalize_selection = new KAction(KIcon(""), i18n("&Normalize Selection"), this);
	action_sort_columns = new KAction(KIcon(""), i18n("&Selected Columns"), this);
	action_sort_asc_column = new KAction(KIcon("view-sort-ascending"), i18n("&Ascending"), this);
	action_sort_desc_column = new KAction(KIcon("view-sort-descending"), i18n("&Descending"), this);
	action_statistics_columns = new KAction(KIcon("view-statistics"), i18n("Column Statisti&cs"), this);

	// row related actions
	action_insert_rows = new KAction(KIcon("edit-table-insert-row-above") ,i18n("&Insert Empty Rows"), this);
	action_remove_rows = new KAction(KIcon("edit-table-delete-row"), i18n("Remo&ve Rows"), this);
	action_clear_rows = new KAction(KIcon("edit-clear"), i18n("Clea&r Rows"), this);
	action_add_rows = new KAction(KIcon("edit-table-insert-row-above"), i18n("&Add Rows"), this);
	action_statistics_rows = new KAction(KIcon("view-statistics"), i18n("Row Statisti&cs"), this);
}

void SpreadsheetView::initMenus(){
	//Selection menu
	m_selectionMenu = new QMenu(i18n("Selection"));

	QMenu * submenu = new QMenu(i18n("Fi&ll Selection with"));
	submenu->addAction(action_fill_row_numbers);
	submenu->addAction(action_fill_const);
// 	submenu->addAction(action_fill_random);
	m_selectionMenu ->addMenu(submenu);
	m_selectionMenu ->addSeparator();

	m_selectionMenu ->addAction(action_cut_selection);
	m_selectionMenu ->addAction(action_copy_selection);
	m_selectionMenu ->addAction(action_paste_into_selection);
	m_selectionMenu ->addAction(action_clear_selection);
	m_selectionMenu ->addSeparator();
	m_selectionMenu ->addAction(action_mask_selection);
	m_selectionMenu ->addAction(action_unmask_selection);
	m_selectionMenu ->addSeparator();
	m_selectionMenu ->addAction(action_normalize_selection);
	//TODO
// 	m_selectionMenu ->addSeparator();
// 	m_selectionMenu ->addAction(action_set_formula);
// 	m_selectionMenu ->addAction(action_recalculate);


	//TODO add plot menu to spreadsheet- and column-menu, like in scidavis, origin etc.

	// Column menu
	m_columnMenu = new QMenu();

// 	submenu = new QMenu(i18n("S&et Column As"));
// 	submenu->addAction(action_set_as_x);
// 	submenu->addAction(action_set_as_y);
// 	submenu->addAction(action_set_as_z);
// 	submenu->addSeparator();
// 	submenu->addAction(action_set_as_xerr);
// 	submenu->addAction(action_set_as_yerr);
// 	submenu->addSeparator();
// 	submenu->addAction(action_set_as_none);
// 	m_columnMenu->addMenu(submenu);
// 	m_columnMenu->addSeparator();

	submenu = new QMenu(i18n("Generate Data"));
	submenu->addAction(action_fill_row_numbers);
	submenu->addAction(action_fill_const);
// 	submenu->addAction(action_fill_random);
	submenu->addAction(action_fill_equidistant);
	submenu->addAction(action_fill_random_nonuniform);
	submenu->addAction(action_fill_function);
	m_columnMenu->addMenu(submenu);
	m_columnMenu->addSeparator();

	m_columnMenu->addAction(action_reverse_columns);
	m_columnMenu->addAction(action_drop_values);
	m_columnMenu->addAction(action_mask_values);
// 	m_columnMenu->addAction(action_join_columns);
	m_columnMenu->addAction(action_normalize_columns);

	submenu = new QMenu(i18n("Sort"));
	submenu->setIcon(KIcon("view-sort-ascending"));
	submenu->addAction(action_sort_asc_column);
	submenu->addAction(action_sort_desc_column);
	submenu->addAction(action_sort_columns);
	m_columnMenu->addMenu(submenu);
	m_columnMenu->addSeparator();

	m_columnMenu->addAction(action_insert_columns);
	m_columnMenu->addAction(action_remove_columns);
	m_columnMenu->addAction(action_clear_columns);
	m_columnMenu->addAction(action_add_columns);
	m_columnMenu->addSeparator();

	m_columnMenu->addAction(action_toggle_comments);
	m_columnMenu->addSeparator();

// 	TODO: m_columnMenu->addAction(action_statistics_columns);


	//Spreadsheet menu
	m_spreadsheetMenu = new QMenu();
// 	m_selectionMenu->setTitle(i18n("Fi&ll Selection with"));
	m_spreadsheetMenu->addMenu(m_selectionMenu);
	m_spreadsheetMenu->addAction(action_toggle_comments);
	m_spreadsheetMenu->addSeparator();
	m_spreadsheetMenu->addAction(action_select_all);
	m_spreadsheetMenu->addAction(action_clear_spreadsheet);
	m_spreadsheetMenu->addAction(action_clear_masks);
	m_spreadsheetMenu->addAction(action_sort_spreadsheet);
	m_spreadsheetMenu->addSeparator();
	m_spreadsheetMenu->addAction(action_add_column);
	m_spreadsheetMenu->addSeparator();
	m_spreadsheetMenu->addAction(action_go_to_cell);


	//Row menu
	m_rowMenu = new QMenu();

	m_rowMenu->addAction(action_insert_rows);
	m_rowMenu->addAction(action_remove_rows);
	m_rowMenu->addAction(action_clear_rows);
	m_rowMenu->addAction(action_add_rows);
	m_rowMenu->addSeparator();

	submenu = new QMenu(i18n("Fi&ll Selection with"));
	submenu->addAction(action_fill_row_numbers);
// 	submenu->addAction(action_fill_random);
	submenu->addAction(action_fill_const);
	m_rowMenu->addMenu(submenu);

	//TODO
// 	m_rowMenu->addSeparator();
// 	m_rowMenu->addAction(action_statistics_rows);
}

void SpreadsheetView::connectActions(){
	connect(action_cut_selection, SIGNAL(triggered()), this, SLOT(cutSelection()));
	connect(action_copy_selection, SIGNAL(triggered()), this, SLOT(copySelection()));
	connect(action_paste_into_selection, SIGNAL(triggered()), this, SLOT(pasteIntoSelection()));
	connect(action_mask_selection, SIGNAL(triggered()), this, SLOT(maskSelection()));
	connect(action_unmask_selection, SIGNAL(triggered()), this, SLOT(unmaskSelection()));

	connect(action_clear_selection, SIGNAL(triggered()), this, SLOT(clearSelectedCells()));
// 	connect(action_recalculate, SIGNAL(triggered()), this, SLOT(recalculateSelectedCells()));
	connect(action_fill_row_numbers, SIGNAL(triggered()), this, SLOT(fillSelectedCellsWithRowNumbers()));
// 	connect(action_fill_random, SIGNAL(triggered()), this, SLOT(fillSelectedCellsWithRandomNumbers()));
	connect(action_fill_random_nonuniform, SIGNAL(triggered()), this, SLOT(fillWithRandomValues()));
	connect(action_fill_equidistant, SIGNAL(triggered()), this, SLOT(fillWithEquidistantValues()));
	connect(action_fill_function, SIGNAL(triggered()), this, SLOT(fillWithFunctionValues()));
	connect(action_fill_const, SIGNAL(triggered()), this, SLOT(fillSelectedCellsWithConstValues()));
	connect(action_select_all, SIGNAL(triggered()), m_tableView, SLOT(selectAll()));
	connect(action_add_column, SIGNAL(triggered()), m_spreadsheet, SLOT(appendColumn()));
	connect(action_clear_spreadsheet, SIGNAL(triggered()), m_spreadsheet, SLOT(clear()));
	connect(action_clear_masks, SIGNAL(triggered()), m_spreadsheet, SLOT(clearMasks()));
	connect(action_sort_spreadsheet, SIGNAL(triggered()), this, SLOT(sortSpreadsheet()));
	connect(action_go_to_cell, SIGNAL(triggered()), this, SLOT(goToCell()));

	connect(action_insert_columns, SIGNAL(triggered()), this, SLOT(insertEmptyColumns()));
	connect(action_remove_columns, SIGNAL(triggered()), this, SLOT(removeSelectedColumns()));
	connect(action_clear_columns, SIGNAL(triggered()), this, SLOT(clearSelectedColumns()));
	connect(action_add_columns, SIGNAL(triggered()), this, SLOT(addColumns()));
// 	connect(action_set_as_x, SIGNAL(triggered()), this, SLOT(setSelectedColumnsAsX()));
// 	connect(action_set_as_y, SIGNAL(triggered()), this, SLOT(setSelectedColumnsAsY()));
// 	connect(action_set_as_z, SIGNAL(triggered()), this, SLOT(setSelectedColumnsAsZ()));
// 	connect(action_set_as_xerr, SIGNAL(triggered()), this, SLOT(setSelectedColumnsAsXError()));
// 	connect(action_set_as_yerr, SIGNAL(triggered()), this, SLOT(setSelectedColumnsAsYError()));
// 	connect(action_set_as_none, SIGNAL(triggered()), this, SLOT(setSelectedColumnsAsNone()));
	connect(action_reverse_columns, SIGNAL(triggered()), this, SLOT(reverseColumns()));
	connect(action_drop_values, SIGNAL(triggered()), this, SLOT(dropColumnValues()));
	connect(action_mask_values, SIGNAL(triggered()), this, SLOT(maskColumnValues()));
// 	connect(action_join_columns, SIGNAL(triggered()), this, SLOT(joinColumns()));
	connect(action_normalize_columns, SIGNAL(triggered()), this, SLOT(normalizeSelectedColumns()));
	connect(action_normalize_selection, SIGNAL(triggered()), this, SLOT(normalizeSelection()));
	connect(action_sort_columns, SIGNAL(triggered()), this, SLOT(sortSelectedColumns()));
	connect(action_sort_asc_column, SIGNAL(triggered()), this, SLOT(sortColumnAscending()));
	connect(action_sort_desc_column, SIGNAL(triggered()), this, SLOT(sortColumnDescending()));
	connect(action_statistics_columns, SIGNAL(triggered()), this, SLOT(statisticsOnSelectedColumns()));

	connect(action_insert_rows, SIGNAL(triggered()), this, SLOT(insertEmptyRows()));
	connect(action_remove_rows, SIGNAL(triggered()), this, SLOT(removeSelectedRows()));
	connect(action_clear_rows, SIGNAL(triggered()), this, SLOT(clearSelectedRows()));
	connect(action_add_rows, SIGNAL(triggered()), this, SLOT(addRows()));
	connect(action_statistics_rows, SIGNAL(triggered()), this, SLOT(statisticsOnSelectedRows()));
	connect(action_toggle_comments, SIGNAL(triggered()), this, SLOT(toggleComments()));
}

void SpreadsheetView::fillToolBar(QToolBar* toolBar){
	toolBar->addAction(action_insert_rows);
	toolBar->addAction(action_add_rows);
	toolBar->addAction(action_remove_rows);
// 	toolBar->addAction(action_statistics_rows);

	toolBar->addSeparator();
	toolBar->addAction(action_insert_columns);
	toolBar->addAction(action_add_column);
	toolBar->addAction(action_remove_columns);
	//TODO
// 	toolBar->addAction(action_statistics_columns);

	toolBar->addSeparator();
	toolBar->addAction(action_sort_asc_column);
	toolBar->addAction(action_sort_desc_column);
}

/*!
 * Populates the menu \c menu with the spreadsheet and spreadsheet view relevant actions.
 * The menu is used
 *   - as the context menu in SpreadsheetView
 *   - as the "spreadsheet menu" in the main menu-bar (called form MainWin)
 *   - as a part of the spreadsheet context menu in project explorer
 */
void SpreadsheetView::createContextMenu(QMenu* menu) const {
	Q_ASSERT(menu);

	QAction* firstAction = 0;
	// if we're populating the context menu for the project explorer, then
	//there're already actions available there. Skip the first title-action
	//and insert the action at the beginning of the menu.
	if (menu->actions().size()>1)
		firstAction = menu->actions().at(1);

	menu->insertMenu(firstAction, m_selectionMenu);
	menu->insertAction(firstAction, action_toggle_comments);
	menu->insertSeparator(firstAction);
	menu->insertAction(firstAction, action_select_all);
	menu->insertAction(firstAction, action_clear_spreadsheet);
	menu->insertAction(firstAction, action_clear_masks);
	menu->insertAction(firstAction, action_sort_spreadsheet);
	menu->insertSeparator(firstAction);
	menu->insertAction(firstAction, action_add_column);
	menu->insertSeparator(firstAction);
	menu->insertAction(firstAction, action_go_to_cell);
	menu->insertSeparator(firstAction);

	// TODO
	// Export to ASCII
	//Export to latex
}

//SLOTS
void SpreadsheetView::handleAspectAdded(const AbstractAspect * aspect){
	const Column * col = qobject_cast<const Column*>(aspect);
	if (!col || col->parentAspect() != static_cast<AbstractAspect*>(m_spreadsheet))
		return;
	connect(col, SIGNAL(widthChanged(const Column*)), this, SLOT(updateSectionSize(const Column*)));
}

void SpreadsheetView::handleAspectAboutToBeRemoved(const AbstractAspect * aspect){
	const Column * col = qobject_cast<const Column*>(aspect);
	if (!col || col->parentAspect() != static_cast<AbstractAspect*>(m_spreadsheet))
		return;
	disconnect(col, 0, this, 0);
}

void SpreadsheetView::updateSectionSize(const Column* col){
	disconnect(m_horizontalHeader, SIGNAL(sectionResized(int,int,int)), this, SLOT(handleHorizontalSectionResized(int,int,int)));
	m_horizontalHeader->resizeSection(m_spreadsheet->indexOfChild<Column>(col), col->width());
	connect(m_horizontalHeader, SIGNAL(sectionResized(int,int,int)), this, SLOT(handleHorizontalSectionResized(int,int,int)));
}

//TODO what for?!?
void SpreadsheetView::handleHorizontalSectionResized(int logicalIndex, int oldSize, int newSize){
	Q_UNUSED(logicalIndex);
	Q_UNUSED(oldSize);
	static bool inside = false;
	if (inside) return;
	inside = true;

	int cols = m_spreadsheet->columnCount();
	for (int i=0; i<cols; i++)
		if (isColumnSelected(i, true))
			m_horizontalHeader->resizeSection(i, newSize);

	inside = false;
}

/*!
  Advance current cell after [Return] or [Enter] was pressed.
 */
void SpreadsheetView::advanceCell(){
	QModelIndex idx = m_tableView->currentIndex();
    if (idx.row()+1 >= m_spreadsheet->rowCount()) {
		int new_size = m_spreadsheet->rowCount()+1;
		m_spreadsheet->setRowCount(new_size);
	}
	m_tableView->setCurrentIndex(idx.sibling(idx.row()+1, idx.column()));
}

void SpreadsheetView::goToCell(int row, int col){
	QModelIndex index = m_model->index(row, col);
	m_tableView->scrollTo(index);
	m_tableView->setCurrentIndex(index);
}

void SpreadsheetView::handleHorizontalSectionMoved(int index, int from, int to){
	Q_UNUSED(index);

	static bool inside = false;
	if (inside) return;

	Q_ASSERT(index == from);

	inside = true;
	m_tableView->horizontalHeader()->moveSection(to, from);
	inside = false;
	m_spreadsheet->moveColumn(from, to);
}

//TODO Implement the "change of the column name"-mode  upon a double click
void SpreadsheetView::handleHorizontalHeaderDoubleClicked(int index){
	Q_UNUSED(index);
}

/*!
  Returns whether comments are show currently or not.
*/
bool SpreadsheetView::areCommentsShown() const{
	return m_horizontalHeader->areCommentsShown();
}

/*!
  toggles the column comment in the horizontal header
*/
void SpreadsheetView::toggleComments(){
	showComments(!areCommentsShown());
	//TODO
	if(areCommentsShown())
		action_toggle_comments->setText(i18n("Hide Comments"));
	else
		action_toggle_comments->setText(i18n("Show Comments"));
}

//! Shows (\c on=true) or hides (\c on=false) the column comments in the horizontal header
void SpreadsheetView::showComments(bool on){
	m_horizontalHeader->showComments(on);
}

void SpreadsheetView::currentColumnChanged(const QModelIndex & current, const QModelIndex & previous){
	Q_UNUSED(previous);
	int col = current.column();
	if (col < 0 || col >= m_spreadsheet->columnCount())
	  return;
}

//TODO
void SpreadsheetView::handleHeaderDataChanged(Qt::Orientation orientation, int first, int last){
	if (orientation != Qt::Horizontal) return;

	QItemSelectionModel * sel_model = m_tableView->selectionModel();

	int col = sel_model->currentIndex().column();
	if (col < first || col > last) return;
}

/*!
  Returns the number of selected columns.
  If \c full is \c true, this function only returns the number of fully selected columns.
*/
int SpreadsheetView::selectedColumnCount(bool full){
	int count = 0;
	int cols = m_spreadsheet->columnCount();
	for (int i=0; i<cols; i++)
		if (isColumnSelected(i, full)) count++;
	return count;
}

/*!
  Returns the number of (at least partly) selected columns with the plot designation \param pd.
 */
int SpreadsheetView::selectedColumnCount(AbstractColumn::PlotDesignation pd){
	int count = 0;
	int cols = m_spreadsheet->columnCount();
	for (int i=0; i<cols; i++)
		if ( isColumnSelected(i, false) && (m_spreadsheet->column(i)->plotDesignation() == pd) ) count++;

	return count;
}

/*!
  Returns \c true if column \param col is selected, otherwise returns \c false.
  If \param full is \c true, this function only returns true if the whole column is selected.
*/
bool SpreadsheetView::isColumnSelected(int col, bool full){
	if (full)
		return m_tableView->selectionModel()->isColumnSelected(col, QModelIndex());
	else
		return m_tableView->selectionModel()->columnIntersectsSelection(col, QModelIndex());
}

/*!
  Returns all selected columns.
  If \param full is true, this function only returns a column if the whole column is selected.
  */
QList<Column*> SpreadsheetView::selectedColumns(bool full){
	QList<Column*> list;
	int cols = m_spreadsheet->columnCount();
	for (int i=0; i<cols; i++)
		if (isColumnSelected(i, full)) list << m_spreadsheet->column(i);

	return list;
}

/*!
  Returns the number of (at least partly) selected rows.
  If \param full is \c true, this function only returns the number of fully selected rows.
*/
int SpreadsheetView::selectedRowCount(bool full){
	int count = 0;
	int rows = m_spreadsheet->rowCount();
	for (int i=0; i<rows; i++)
		if (isRowSelected(i, full)) count++;
	return count;
}

/*!
  Returns \c true if row \param row is selected; otherwise returns \c false
  If \param full is \c true, this function only returns \c true if the whole row is selected.
*/
bool SpreadsheetView::isRowSelected(int row, bool full){
	if (full)
		return m_tableView->selectionModel()->isRowSelected(row, QModelIndex());
	else
		return m_tableView->selectionModel()->rowIntersectsSelection(row, QModelIndex());
}

/*!
  Return the index of the first selected column.
  If \param full is \c true, this function only looks for fully selected columns.
*/
int SpreadsheetView::firstSelectedColumn(bool full){
	int cols = m_spreadsheet->columnCount();
	for (int i=0; i<cols; i++)
	{
		if (isColumnSelected(i, full))
			return i;
	}
	return -1;
}

/*!
  Return the index of the last selected column.
  If \param full is \c true, this function only looks for fully selected columns.
  */
int SpreadsheetView::lastSelectedColumn(bool full){
	int cols = m_spreadsheet->columnCount();
	for (int i=cols-1; i>=0; i--)
		if (isColumnSelected(i, full)) return i;

	return -2;
}

/*!
  Return the index of the first selected row.
  If \param full is \c true, this function only looks for fully selected rows.
  */
int SpreadsheetView::firstSelectedRow(bool full){
	int rows = m_spreadsheet->rowCount();
	for (int i=0; i<rows; i++)
	{
		if (isRowSelected(i, full))
			return i;
	}
	return -1;
}

/*!
  Return the index of the last selected row.
  If \param full is \c true, this function only looks for fully selected rows.
  */
int SpreadsheetView::lastSelectedRow(bool full){
	int rows = m_spreadsheet->rowCount();
	for (int i=rows-1; i>=0; i--)
		if (isRowSelected(i, full)) return i;

	return -2;
}

/*!
  Return whether a cell is selected
 */
bool SpreadsheetView::isCellSelected(int row, int col){
	if (row < 0 || col < 0 || row >= m_spreadsheet->rowCount() || col >= m_spreadsheet->columnCount()) return false;

	return m_tableView->selectionModel()->isSelected(m_model->index(row, col));
}

/*!
  Get the complete set of selected rows.
 */
IntervalAttribute<bool> SpreadsheetView::selectedRows(bool full){
	IntervalAttribute<bool> result;
	int rows = m_spreadsheet->rowCount();
	for (int i=0; i<rows; i++)
		if (isRowSelected(i, full))
			result.setValue(i, true);
	return result;
}

/*!
  Select/Deselect a cell.
 */
void SpreadsheetView::setCellSelected(int row, int col, bool select){
	m_tableView->selectionModel()->select(m_model->index(row, col),
			 select ? QItemSelectionModel::Select : QItemSelectionModel::Deselect);
}

/*!
  Select/Deselect a range of cells.
 */
void SpreadsheetView::setCellsSelected(int first_row, int first_col, int last_row, int last_col, bool select){
	QModelIndex top_left = m_model->index(first_row, first_col);
	QModelIndex bottom_right = m_model->index(last_row, last_col);
	m_tableView->selectionModel()->select(QItemSelection(top_left, bottom_right),
			select ? QItemSelectionModel::SelectCurrent : QItemSelectionModel::Deselect);
}

/*!
  Determine the current cell (-1 if no cell is designated as the current).
 */
void SpreadsheetView::getCurrentCell(int * row, int * col){
	QModelIndex index = m_tableView->selectionModel()->currentIndex();
	if (index.isValid())	{
		*row = index.row();
		*col = index.column();
	}else{
		*row = -1;
		*col = -1;
	}
}

bool SpreadsheetView::eventFilter(QObject* watched, QEvent* event) {
	if (event->type() == QEvent::ContextMenu){
		QContextMenuEvent *cm_event = static_cast<QContextMenuEvent*>(event);
		QPoint global_pos = cm_event->globalPos();
		if (watched == m_tableView->verticalHeader()){
			m_rowMenu->exec(global_pos);
		}else if (watched == m_horizontalHeader) {
			int col = m_horizontalHeader->logicalIndexAt(cm_event->pos());
			if (!isColumnSelected(col, true)) {
				QItemSelectionModel *sel_model = m_tableView->selectionModel();
				sel_model->clearSelection();
				sel_model->select(QItemSelection(m_model->index(0, col, QModelIndex()),
							m_model->index(m_model->rowCount()-1, col, QModelIndex())),
						QItemSelectionModel::Select);
			}

			if (selectedColumns().size()==1){
				action_sort_columns->setVisible(false);
				action_sort_asc_column->setVisible(true);
				action_sort_desc_column->setVisible(true);
			}else{
				action_sort_columns->setVisible(true);
				action_sort_asc_column->setVisible(false);
				action_sort_desc_column->setVisible(false);
			}

			//check whether we have non-numeric columns selected and deactivate actions for numeric columns
			bool numeric = true;
			foreach(Column* col, selectedColumns()) {
				if (col->columnMode() != AbstractColumn::Numeric) {
					numeric = false;
					break;
				}
			}
			action_fill_equidistant->setEnabled(numeric);
			action_fill_random_nonuniform->setEnabled(numeric);
			action_fill_function->setEnabled(numeric);

			m_columnMenu->exec(global_pos);
		}else if (watched == this){
			m_spreadsheetMenu->exec(global_pos);
		}else{
			return QWidget::eventFilter(watched, event);
		}
		return true;
	}
	else
		return QWidget::eventFilter(watched, event);
}

bool SpreadsheetView::formulaModeActive() const{
	return m_model->formulaModeActive();
}

void SpreadsheetView::activateFormulaMode(bool on){
	m_model->activateFormulaMode(on);
}

void SpreadsheetView::goToNextColumn(){
	if (m_spreadsheet->columnCount() == 0) return;

	QModelIndex idx = m_tableView->currentIndex();
	int col = idx.column()+1;
    if (col >= m_spreadsheet->columnCount())
		col = 0;

	m_tableView->setCurrentIndex(idx.sibling(idx.row(), col));
}

void SpreadsheetView::goToPreviousColumn(){
	if (m_spreadsheet->columnCount() == 0)
	  return;

	QModelIndex idx = m_tableView->currentIndex();
	int col = idx.column()-1;
    if (col < 0)
		col = m_spreadsheet->columnCount()-1;

	m_tableView->setCurrentIndex(idx.sibling(idx.row(), col));
}

void SpreadsheetView::cutSelection(){
	int first = firstSelectedRow();
	if ( first < 0 )
	  return;

	WAIT_CURSOR;
	m_spreadsheet->beginMacro(i18n("%1: cut selected cells", m_spreadsheet->name()));
	copySelection();
	clearSelectedCells();
	m_spreadsheet->endMacro();
	RESET_CURSOR;
}

void SpreadsheetView::copySelection(){
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

	for (int r=0; r<rows; r++)
	{
		for (int c=0; c<cols; c++)
		{
			Column *col_ptr = m_spreadsheet->column(first_col + c);
			if (isCellSelected(first_row + r, first_col + c))
			{
				if (formulaModeActive())
				{
					output_str += col_ptr->formula(first_row + r);
				}
				else if (col_ptr->columnMode() == AbstractColumn::Numeric)
				{
					Double2StringFilter * out_fltr = static_cast<Double2StringFilter *>(col_ptr->outputFilter());
					output_str += QLocale().toString(col_ptr->valueAt(first_row + r),
							out_fltr->numericFormat(), 16); // copy with max. precision
				}
				else
				{
					output_str += m_spreadsheet->column(first_col+c)->asStringColumn()->textAt(first_row + r);
				}
			}
			if (c < cols-1)
				output_str += '\t';
		}
		if (r < rows-1)
			output_str += '\n';
	}
	QApplication::clipboard()->setText(output_str);
	RESET_CURSOR;
}

void SpreadsheetView::pasteIntoSelection(){
	if (m_spreadsheet->columnCount() < 1 || m_spreadsheet->rowCount() < 1)
	  return;

	WAIT_CURSOR;
	m_spreadsheet->beginMacro(i18n("%1: paste from clipboard", m_spreadsheet->name()));
	const QMimeData * mime_data = QApplication::clipboard()->mimeData();

	if (mime_data->hasFormat("text/plain")){
		int first_col = firstSelectedColumn(false);
		int last_col = lastSelectedColumn(false);
		int first_row = firstSelectedRow(false);
		int last_row = lastSelectedRow(false);
		int input_row_count = 0;
		int input_col_count = 0;
		int rows, cols;

		QString input_str = QString(mime_data->data("text/plain")).trimmed();
		QList< QStringList > cellTexts;
		QStringList input_rows(input_str.split('\n'));
		input_row_count = input_rows.count();
		input_col_count = 0;
		for (int i=0; i<input_row_count; i++)
		{
			cellTexts.append(input_rows.at(i).trimmed().split(QRegExp("\\s+")));
			if (cellTexts.at(i).count() > input_col_count) input_col_count = cellTexts.at(i).count();
		}

		if ( (first_col == -1 || first_row == -1) ||
			(last_row == first_row && last_col == first_col) )
		// if the is no selection or only one cell selected, the
		// selection will be expanded to the needed size from the current cell
		{
			int current_row, current_col;
			getCurrentCell(&current_row, &current_col);
			if (current_row == -1) current_row = 0;
			if (current_col == -1) current_col = 0;
			setCellSelected(current_row, current_col);
			first_col = current_col;
			first_row = current_row;
			last_row = first_row + input_row_count -1;
			last_col = first_col + input_col_count -1;
			// resize the spreadsheet if necessary
			if (last_col >= m_spreadsheet->columnCount())
			{
				for (int i=0; i<last_col+1-m_spreadsheet->columnCount(); i++)
				{
					Column * new_col = new Column(QString::number(i+1), AbstractColumn::Text);
					new_col->setPlotDesignation(AbstractColumn::Y);
					new_col->insertRows(0, m_spreadsheet->rowCount());
					m_spreadsheet->addChild(new_col);
				}
			}
			if (last_row >= m_spreadsheet->rowCount())
				m_spreadsheet->appendRows(last_row+1-m_spreadsheet->rowCount());
			// select the rectangle to be pasted in
			setCellsSelected(first_row, first_col, last_row, last_col);
		}

		rows = last_row - first_row + 1;
		cols = last_col - first_col + 1;
		for (int r=0; r<rows && r<input_row_count; r++)
		{
			for (int c=0; c<cols && c<input_col_count; c++)
			{
				//TODO c->setSuppressDataChangedSignal(true);
				if (isCellSelected(first_row + r, first_col + c) && (c < cellTexts.at(r).count()) )
				{
					Column * col_ptr = m_spreadsheet->column(first_col + c);
					if (formulaModeActive())
					{
						col_ptr->setFormula(first_row + r, cellTexts.at(r).at(c));
					}
					else
						col_ptr->asStringColumn()->setTextAt(first_row+r, cellTexts.at(r).at(c));
				}
			}
		}
	}
	m_spreadsheet->endMacro();
	RESET_CURSOR;
}

void SpreadsheetView::maskSelection(){
	int first = firstSelectedRow();
	int last = lastSelectedRow();
	if ( first < 0 ) return;

	WAIT_CURSOR;
	m_spreadsheet->beginMacro(i18n("%1: mask selected cells", m_spreadsheet->name()));
	QList<Column*> list = selectedColumns();
	foreach(Column * col_ptr, list)
	{
		int col = m_spreadsheet->indexOfChild<Column>(col_ptr);
		for (int row=first; row<=last; row++)
			if (isCellSelected(row, col)) col_ptr->setMasked(row);
	}
	m_spreadsheet->endMacro();
	RESET_CURSOR;
}

void SpreadsheetView::unmaskSelection(){
	int first = firstSelectedRow();
	int last = lastSelectedRow();
	if ( first < 0 ) return;

	WAIT_CURSOR;
	m_spreadsheet->beginMacro(i18n("%1: unmask selected cells", m_spreadsheet->name()));
	QList<Column*> list = selectedColumns();
	foreach(Column * col_ptr, list)	{
		int col = m_spreadsheet->indexOfChild<Column>(col_ptr);
		for (int row=first; row<=last; row++)
			if (isCellSelected(row, col)) col_ptr->setMasked(row, false);
	}
	m_spreadsheet->endMacro();
	RESET_CURSOR;
}

// void SpreadsheetView::recalculateSelectedCells(){
// }

void SpreadsheetView::fillSelectedCellsWithRowNumbers(){
	if (selectedColumnCount() < 1) return;
	int first = firstSelectedRow();
	int last = lastSelectedRow();
	if ( first < 0 ) return;

	WAIT_CURSOR;
	m_spreadsheet->beginMacro(i18n("%1: fill cells with row numbers", m_spreadsheet->name()));
	foreach(Column* col_ptr, selectedColumns()) {
		int col = m_spreadsheet->indexOfChild<Column>(col_ptr);
		col_ptr->setSuppressDataChangedSignal(true);
		switch (col_ptr->columnMode()) {
			case AbstractColumn::Numeric:
				{
					QVector<double> results(last-first+1);
					for (int row=first; row<=last; row++)
						if(isCellSelected(row, col))
							results[row-first] = row+1;
						else
							results[row-first] = col_ptr->valueAt(row);
					col_ptr->replaceValues(first, results);
					break;
				}
			case AbstractColumn::Text:
				{
					QStringList results;
					for (int row=first; row<=last; row++)
						if (isCellSelected(row, col))
							results << QString::number(row+1);
						else
							results << col_ptr->textAt(row);
					col_ptr->replaceTexts(first, results);
					break;
				}
			//TODO: handle other modes
			default: break;
		}

		col_ptr->setSuppressDataChangedSignal(false);
		col_ptr->setChanged();
	}
	m_spreadsheet->endMacro();
	RESET_CURSOR;
}

void SpreadsheetView::fillSelectedCellsWithRandomNumbers(){
	if (selectedColumnCount() < 1) return;
	int first = firstSelectedRow();
	int last = lastSelectedRow();
	if ( first < 0 ) return;

	WAIT_CURSOR;
	m_spreadsheet->beginMacro(i18n("%1: fill cells with random values", m_spreadsheet->name()));
	qsrand(QTime::currentTime().msec());
	foreach(Column* col_ptr, selectedColumns()) {
		int col = m_spreadsheet->indexOfChild<Column>(col_ptr);
		col_ptr->setSuppressDataChangedSignal(true);
		switch (col_ptr->columnMode()) {
			case AbstractColumn::Numeric:
				{
					QVector<double> results(last-first+1);
					for (int row=first; row<=last; row++)
						if (isCellSelected(row, col))
							results[row-first] = double(qrand())/double(RAND_MAX);
						else
							results[row-first] = col_ptr->valueAt(row);
					col_ptr->replaceValues(first, results);
					break;
				}
			case AbstractColumn::Text:
				{
					QStringList results;
					for (int row=first; row<=last; row++)
						if (isCellSelected(row, col))
							results << QString::number(double(qrand())/double(RAND_MAX));
						else
							results << col_ptr->textAt(row);
					col_ptr->replaceTexts(first, results);
					break;
				}
			case AbstractColumn::DateTime:
			case AbstractColumn::Month:
			case AbstractColumn::Day:
				{
					QList<QDateTime> results;
					QDate earliestDate(1,1,1);
					QDate latestDate(2999,12,31);
					QTime midnight(0,0,0,0);
					for (int row=first; row<=last; row++)
						if (isCellSelected(row, col))
							results << QDateTime(
									earliestDate.addDays(((double)qrand())*((double)earliestDate.daysTo(latestDate))/((double)RAND_MAX)),
									midnight.addMSecs(((qint64)qrand())*1000*60*60*24/RAND_MAX));
						else
							results << col_ptr->dateTimeAt(row);
					col_ptr->replaceDateTimes(first, results);
					break;
				}
		}

		col_ptr->setSuppressDataChangedSignal(false);
		col_ptr->setChanged();
	}
	m_spreadsheet->endMacro();
	RESET_CURSOR;
}

void SpreadsheetView::fillWithRandomValues() {
	if (selectedColumnCount() < 1) return;
	RandomValuesDialog* dlg = new RandomValuesDialog(m_spreadsheet);
	dlg->setAttribute(Qt::WA_DeleteOnClose);
	dlg->setColumns(selectedColumns());
	dlg->exec();
}

void SpreadsheetView::fillWithEquidistantValues() {
	if (selectedColumnCount() < 1) return;
	EquidistantValuesDialog* dlg = new EquidistantValuesDialog(m_spreadsheet);
	dlg->setAttribute(Qt::WA_DeleteOnClose);
	dlg->setColumns(selectedColumns());
	dlg->exec();
}

void SpreadsheetView::fillWithFunctionValues() {
	if (selectedColumnCount() < 1) return;
	FunctionValuesDialog* dlg = new FunctionValuesDialog(m_spreadsheet);
	dlg->setAttribute(Qt::WA_DeleteOnClose);
	dlg->setColumns(selectedColumns());
	dlg->exec();
}

void SpreadsheetView::fillSelectedCellsWithConstValues(){
	if (selectedColumnCount() < 1) return;
	int first = firstSelectedRow();
	int last = lastSelectedRow();
	if ( first < 0 )
		return;

	bool doubleOk = false;
	bool stringOk = false;
	double doubleValue = 0;
	QString stringValue;

	m_spreadsheet->beginMacro(i18n("%1: fill cells with const values", m_spreadsheet->name()));
	foreach(Column* col_ptr, selectedColumns()) {
		int col = m_spreadsheet->indexOfChild<Column>(col_ptr);
		col_ptr->setSuppressDataChangedSignal(true);
		switch (col_ptr->columnMode()) {
			case AbstractColumn::Numeric: {
				if (!doubleOk)
					doubleValue = QInputDialog::getDouble(this, i18n("Fill the selection with constant value"),
															i18n("Value"), 0, -2147483647, 2147483647, 6, &doubleOk);
				if (doubleOk) {
					WAIT_CURSOR;
					QVector<double> results(last-first+1);
					for (int row=first; row<=last; row++) {
						if(isCellSelected(row, col))
							results[row-first] = doubleValue;
						else
							results[row-first] = col_ptr->valueAt(row);
					}
					col_ptr->replaceValues(first, results);
					RESET_CURSOR;
				}
				break;
			}
			case AbstractColumn::Text: {
				if (!stringOk)
					stringValue = QInputDialog::getText(this, i18n("Fill the selection with constant value"),
															i18n("Value"), QLineEdit::Normal, 0, &stringOk);
				if (stringOk && !stringValue.isEmpty()) {
					WAIT_CURSOR;
					QStringList results;
					for (int row=first; row<=last; row++) {
						if (isCellSelected(row, col))
							results << stringValue;
						else
							results << col_ptr->textAt(row);
					}
					col_ptr->replaceTexts(first, results);
					RESET_CURSOR;
				}
				break;
				}
			//TODO: handle other modes
			default:
				break;
		}

		col_ptr->setSuppressDataChangedSignal(false);
		col_ptr->setChanged();
	}
	m_spreadsheet->endMacro();
}

/*!
	Open the sort dialog for all columns.
*/
void SpreadsheetView::sortSpreadsheet(){
	sortDialog(m_spreadsheet->children<Column>());
}

/*!
  Insert columns depending on the selection.
 */
void SpreadsheetView::insertEmptyColumns(){
	int first = firstSelectedColumn();
	if ( first < 0 ) return;
	int last = lastSelectedColumn();
	int count, current = first;

	WAIT_CURSOR;
	m_spreadsheet->beginMacro(i18n("%1: insert empty columns", m_spreadsheet->name()));
	int rows = m_spreadsheet->rowCount();
	while( current <= last )
	{
		current = first+1;
		while( current <= last && isColumnSelected(current) ) current++;
		count = current-first;
		Column *first_col = m_spreadsheet->child<Column>(first);
		for (int i=0; i<count; i++)
		{
			Column * new_col = new Column(QString::number(i+1), AbstractColumn::Numeric);
			new_col->setPlotDesignation(AbstractColumn::Y);
			new_col->insertRows(0, rows);
			m_spreadsheet->insertChildBefore(new_col, first_col);
		}
		current += count;
		last += count;
		while( current <= last && !isColumnSelected(current) ) current++;
		first = current;
	}
	m_spreadsheet->endMacro();
	RESET_CURSOR;
}

void SpreadsheetView::removeSelectedColumns(){
	WAIT_CURSOR;
	m_spreadsheet->beginMacro(i18n("%1: remove selected columns", m_spreadsheet->name()));

	QList< Column* > list = selectedColumns();
	foreach(Column* ptr, list)
		m_spreadsheet->removeChild(ptr);

	m_spreadsheet->endMacro();
	RESET_CURSOR;
}

void SpreadsheetView::clearSelectedColumns(){
	WAIT_CURSOR;
	m_spreadsheet->beginMacro(i18n("%1: clear selected columns", m_spreadsheet->name()));

	QList< Column* > list = selectedColumns();
	if (formulaModeActive())	{
		foreach(Column* ptr, list) {
			ptr->setSuppressDataChangedSignal(true);
			ptr->clearFormulas();
			ptr->setSuppressDataChangedSignal(false);
			ptr->setChanged();
		}
	}else{
		foreach(Column* ptr, list) {
			ptr->setSuppressDataChangedSignal(true);
			ptr->clear();
			ptr->setSuppressDataChangedSignal(false);
			ptr->setChanged();
		}
	}

	m_spreadsheet->endMacro();
	RESET_CURSOR;
}

// void SpreadsheetView::setSelectionAs(AbstractColumn::PlotDesignation pd){
// 	WAIT_CURSOR;
// 	m_spreadsheet->beginMacro(i18n("%1: set plot designation", m_spreadsheet->name()));
//
// 	QList< Column* > list = selectedColumns();
// 	foreach(Column* ptr, list)
// 		ptr->setPlotDesignation(pd);
//
// 	m_spreadsheet->endMacro();
// 	RESET_CURSOR;
// }

// void SpreadsheetView::setSelectedColumnsAsX(){
// 	setSelectionAs(AbstractColumn::X);
// }
//
// void SpreadsheetView::setSelectedColumnsAsY(){
// 	setSelectionAs(AbstractColumn::Y);
// }
//
// void SpreadsheetView::setSelectedColumnsAsZ(){
// 	setSelectionAs(AbstractColumn::Z);
// }
//
// void SpreadsheetView::setSelectedColumnsAsYError(){
// 	setSelectionAs(AbstractColumn::yErr);
// }
//
// void SpreadsheetView::setSelectedColumnsAsXError(){
// 	setSelectionAs(AbstractColumn::xErr);
// }
//
// void SpreadsheetView::setSelectedColumnsAsNone(){
// 	setSelectionAs(AbstractColumn::noDesignation);
// }

void SpreadsheetView::reverseColumns() {
	WAIT_CURSOR;
	QList<Column*> cols = selectedColumns();
	m_spreadsheet->beginMacro(i18np("%1: reverse column",
								"%1: reverse columns",
								m_spreadsheet->name(),
								cols.size()));
	foreach(Column* col, cols) {
		if (col->columnMode() != AbstractColumn::Numeric)
			continue;

		QVector<double>* data = static_cast<QVector<double>* >(col->data());
		QVector<double> new_data(*data);
		std::reverse(new_data.begin(), new_data.end());
		col->replaceValues(0, new_data);
	}
	m_spreadsheet->endMacro();
	RESET_CURSOR;
}

void SpreadsheetView::dropColumnValues() {
	if (selectedColumnCount() < 1) return;
	DropValuesDialog* dlg = new DropValuesDialog(m_spreadsheet);
	dlg->setAttribute(Qt::WA_DeleteOnClose);
	dlg->setColumns(selectedColumns());
	dlg->exec();
}

void SpreadsheetView::maskColumnValues() {
	if (selectedColumnCount() < 1) return;
	DropValuesDialog* dlg = new DropValuesDialog(m_spreadsheet, true);
	dlg->setAttribute(Qt::WA_DeleteOnClose);
	dlg->setColumns(selectedColumns());
	dlg->exec();
}

void SpreadsheetView::joinColumns()
{

}

void SpreadsheetView::normalizeSelectedColumns(){
	WAIT_CURSOR;
	m_spreadsheet->beginMacro(i18n("%1: normalize columns", m_spreadsheet->name()));
	QList< Column* > cols = selectedColumns();
	foreach(Column* col, cols)	{
		if (col->columnMode() == AbstractColumn::Numeric) {
			col->setSuppressDataChangedSignal(true);
			double max = col->maximum();
			if (max != 0.0) {// avoid division by zero
				for (int row=0; row<col->rowCount(); row++)
					col->setValueAt(row, col->valueAt(row) / max);
			}
			col->setSuppressDataChangedSignal(false);
			col->setChanged();
		}
	}
	m_spreadsheet->endMacro();
	RESET_CURSOR;
}

void SpreadsheetView::normalizeSelection(){
	WAIT_CURSOR;
	m_spreadsheet->beginMacro(i18n("%1: normalize selection", m_spreadsheet->name()));
	double max = 0.0;
	for (int col=firstSelectedColumn(); col<=lastSelectedColumn(); col++)
		if (m_spreadsheet->column(col)->columnMode() == AbstractColumn::Numeric)
			for (int row=0; row<m_spreadsheet->rowCount(); row++)
			{
				if (isCellSelected(row, col) && m_spreadsheet->column(col)->valueAt(row) > max)
					max = m_spreadsheet->column(col)->valueAt(row);
			}

	if (max != 0.0) // avoid division by zero
	{
		//TODO setSuppressDataChangedSignal
		for (int col=firstSelectedColumn(); col<=lastSelectedColumn(); col++)
			if (m_spreadsheet->column(col)->columnMode() == AbstractColumn::Numeric)
				for (int row=0; row<m_spreadsheet->rowCount(); row++)
				{
					if (isCellSelected(row, col))
						m_spreadsheet->column(col)->setValueAt(row, m_spreadsheet->column(col)->valueAt(row) / max);
				}
	}
	m_spreadsheet->endMacro();
	RESET_CURSOR;
}

void SpreadsheetView::sortSelectedColumns(){
	QList< Column* > cols = selectedColumns();
	sortDialog(cols);
}


// TODO
void SpreadsheetView::statisticsOnSelectedColumns(){
// 	QMessageBox::information(0, "info", "not yet implemented");
}

// TODO
void SpreadsheetView::statisticsOnSelectedRows(){
// 	QMessageBox::information(0, "info", "not yet implemented");
}

/*!
  Insert rows depending on the selection.
*/
void SpreadsheetView::insertEmptyRows(){
	int first = firstSelectedRow();
	if ( first < 0 ) return;
	int last = lastSelectedRow();
	int count, current = first;

	WAIT_CURSOR;
	m_spreadsheet->beginMacro(i18n("%1: insert empty rows", m_spreadsheet->name()));
	while( current <= last ){
		current = first+1;
		while( current <= last && isRowSelected(current) ) current++;
		count = current-first;
		m_spreadsheet->insertRows(first, count);
		current += count;
		last += count;
		while( current <= last && !isRowSelected(current) ) current++;
		first = current;
	}
	m_spreadsheet->endMacro();
	RESET_CURSOR;
}

void SpreadsheetView::removeSelectedRows(){
	if ( firstSelectedRow() < 0 ) return;

	WAIT_CURSOR;
	m_spreadsheet->beginMacro(i18n("%1: remove selected rows", m_spreadsheet->name()));
	//TODO setSuppressDataChangedSignal
	foreach(const Interval<int>& i, selectedRows().intervals())
		m_spreadsheet->removeRows(i.start(), i.size());
	m_spreadsheet->endMacro();
	RESET_CURSOR;
}

void SpreadsheetView::clearSelectedRows(){
	if ( firstSelectedRow() < 0 ) return;

	WAIT_CURSOR;
	m_spreadsheet->beginMacro(i18n("%1: clear selected rows", m_spreadsheet->name()));
	QList<Column*> list = selectedColumns();
	foreach(Column* col_ptr, list) {
		col_ptr->setSuppressDataChangedSignal(true);
		if (formulaModeActive()) {
			foreach(const Interval<int>& i, selectedRows().intervals())
				col_ptr->setFormula(i, "");
		} else {
			foreach(const Interval<int>& i, selectedRows().intervals()) {
				if (i.end() == col_ptr->rowCount()-1)
					col_ptr->removeRows(i.start(), i.size());
				else {
					QStringList empties;
					for (int j=0; j<i.size(); j++)
						empties << QString();
					col_ptr->asStringColumn()->replaceTexts(i.start(), empties);
				}
			}
		}

		col_ptr->setSuppressDataChangedSignal(false);
		col_ptr->setChanged();
	}
	m_spreadsheet->endMacro();
	RESET_CURSOR;
}

void SpreadsheetView::clearSelectedCells(){
	int first = firstSelectedRow();
	int last = lastSelectedRow();
	if ( first < 0 ) return;

	WAIT_CURSOR;
	m_spreadsheet->beginMacro(i18n("%1: clear selected cells", m_spreadsheet->name()));
	QList<Column*> list = selectedColumns();
	foreach(Column* col_ptr, list) {
		col_ptr->setSuppressDataChangedSignal(true);
		if (formulaModeActive())
		{
			int col = m_spreadsheet->indexOfChild<Column>(col_ptr);
			for (int row=last; row>=first; row--)
				if (isCellSelected(row, col))
				{
					col_ptr->setFormula(row, "");
				}
		}
		else
		{
			int col = m_spreadsheet->indexOfChild<Column>(col_ptr);
			for (int row=last; row>=first; row--)
				if (isCellSelected(row, col))
				{
					if (row < col_ptr->rowCount())
						col_ptr->asStringColumn()->setTextAt(row, QString());
				}
		}
		col_ptr->setSuppressDataChangedSignal(false);
		col_ptr->setChanged();
	}
	m_spreadsheet->endMacro();
	RESET_CURSOR;
}

void SpreadsheetView::goToCell(){
	bool ok;

	int col = QInputDialog::getInteger(0, i18n("Go to Cell"), i18n("Enter column"),
			1, 1, m_spreadsheet->columnCount(), 1, &ok);
	if ( !ok ) return;

	int row = QInputDialog::getInteger(0, i18n("Go to Cell"), i18n("Enter row"),
			1, 1, m_spreadsheet->rowCount(), 1, &ok);
	if ( !ok ) return;

	goToCell(row-1, col-1);
}

//! Open the sort dialog for the given columns
void SpreadsheetView::sortDialog(QList<Column*> cols){
	if (cols.isEmpty()) return;

	foreach(Column* col, cols)
		col->setSuppressDataChangedSignal(true);

	SortDialog* dlg = new SortDialog();
	dlg->setAttribute(Qt::WA_DeleteOnClose);
	connect(dlg, SIGNAL(sort(Column*,QList<Column*>,bool)), m_spreadsheet, SLOT(sortColumns(Column*,QList<Column*>,bool)));
	dlg->setColumnsList(cols);
	int rc = dlg->exec();

	foreach(Column* col, cols) {
		col->setSuppressDataChangedSignal(false);
		if (rc==QDialog::Accepted)
			col->setChanged();
	}
}

void SpreadsheetView::sortColumnAscending(){
	QList< Column* > cols = selectedColumns();
	foreach(Column* col, cols) {
		col->setSuppressDataChangedSignal(true);
	}
	m_spreadsheet->sortColumns(cols.first(), cols, true);
	foreach(Column* col, cols) {
		col->setSuppressDataChangedSignal(false);
		col->setChanged();
	}
}

void SpreadsheetView::sortColumnDescending(){
	QList< Column* > cols = selectedColumns();
	foreach(Column* col, cols) {
		col->setSuppressDataChangedSignal(true);
	}
	m_spreadsheet->sortColumns(cols.first(), cols, false);
	foreach(Column* col, cols) {
		col->setSuppressDataChangedSignal(false);
		col->setChanged();
	}
}

/*!
  Append as many columns as are selected.
*/
void SpreadsheetView::addColumns(){
	m_spreadsheet->appendColumns(selectedColumnCount(false));
}

/*!
  Append as many rows as are selected.
*/
void SpreadsheetView::addRows(){
	m_spreadsheet->appendRows(selectedRowCount(false));
}

/*!
  Cause a repaint of the header.
*/
void SpreadsheetView::updateHeaderGeometry(Qt::Orientation o, int first, int last){
	Q_UNUSED(first)
	Q_UNUSED(last)
	//TODO
	if (o != Qt::Horizontal) return;
	m_tableView->horizontalHeader()->setStretchLastSection(true);  // ugly hack (flaw in Qt? Does anyone know a better way?)
	m_tableView->horizontalHeader()->updateGeometry();
	m_tableView->horizontalHeader()->setStretchLastSection(false); // ugly hack part 2
}

void SpreadsheetView::keyPressEvent(QKeyEvent * event){
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
	  advanceCell();
}

/*!
  selects the column \c column in the speadsheet view .
*/
void SpreadsheetView::selectColumn(int column){
  QItemSelection selection(m_model->index(0, column), m_model->index(m_spreadsheet->rowCount()-1, column) );
  m_suppressSelectionChangedEvent = true;
  m_tableView->selectionModel()->select(selection, QItemSelectionModel::Select);
  m_suppressSelectionChangedEvent = false;
}

/*!
  deselects the column \c column in the speadsheet view .
*/
void SpreadsheetView::deselectColumn(int column){
  QItemSelection selection(m_model->index(0, column), m_model->index(m_spreadsheet->rowCount()-1, column) );
  m_suppressSelectionChangedEvent = true;
  m_tableView->selectionModel()->select(selection, QItemSelectionModel::Deselect);
  m_suppressSelectionChangedEvent = false;
}

/*!
  called when a column in the speadsheet view was clicked (click in the header).
  Propagates the selection of the column to the \c Spreadsheet object
  (a click in the header always selects the column).
*/
void SpreadsheetView::columnClicked(int column){
   m_spreadsheet->setColumnSelectedInView(column, true);
}

/*!
  called on selections changes. Propagates the selection/deselection of columns to the \c Spreadsheet object.
*/
 void SpreadsheetView::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected){
  Q_UNUSED(selected);
  Q_UNUSED(deselected);

  if (m_suppressSelectionChangedEvent)
	  return;

  QItemSelectionModel* selModel = m_tableView->selectionModel();
  for (int i=0; i<m_spreadsheet->columnCount(); i++){
	m_spreadsheet->setColumnSelectedInView(i, selModel->isColumnSelected(i, QModelIndex()));
  }
}

/*!
  prints the complete spreadsheet to \c printer.
 */
void SpreadsheetView::print(QPrinter* printer) const{
	QPainter painter (printer);

	int dpiy = printer->logicalDpiY();
	const int margin = (int) ( (1/2.54)*dpiy ); // 1 cm margins

	QHeaderView *hHeader = m_tableView->horizontalHeader();
	QHeaderView *vHeader = m_tableView->verticalHeader();

	int rows = m_spreadsheet->rowCount();
	int cols = m_spreadsheet->columnCount();
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
			cellText = m_spreadsheet->text(i,j)+'\t';
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

void SpreadsheetView::exportToFile(const QString& path, const bool exportHeader, const QString& separator) const {
	QFile file(path);
	if (!file.open(QFile::WriteOnly | QFile::Truncate))
		return;

	QTextStream out(&file);
	const int cols = m_spreadsheet->columnCount();

	QString sep = separator;
	sep = sep.replace(QString("TAB"), QString("\t"), Qt::CaseInsensitive);
	sep = sep.replace(QString("SPACE"), QString(" "), Qt::CaseInsensitive);

	//export header (column names)
	if (exportHeader) {
		for (int j=0; j<cols; ++j) {
			out << m_spreadsheet->column(j)->name();
			if (j!=cols-1)
				out<<sep;
		}
		out << '\n';
	}

	//export values
	for (int i=0; i<m_spreadsheet->rowCount(); ++i) {
		for (int j=0; j<cols; ++j) {
			out << m_spreadsheet->column(j)->asStringColumn()->textAt(i);
			if (j!=cols-1)
				out<<sep;
		}
		out << '\n';
	}
}
