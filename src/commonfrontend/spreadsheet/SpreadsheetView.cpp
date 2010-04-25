/***************************************************************************
    File                 : SpreadsheetView.cpp
    Project              : SciDAVis
    Description          : View class for Spreadsheet
    --------------------------------------------------------------------
    Copyright            : (C) 2007 Tilman Benkert (thzs*gmx.net)
                           (replace * with @ in the email addresses) 

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
#include "spreadsheet/SpreadsheetModel.h"
#include "spreadsheet/Spreadsheet.h"
#include "SpreadsheetItemDelegate.h"
#include "SpreadsheetDoubleHeaderView.h"

#include "lib/ActionManager.h"
#include "lib/macros.h"

#include "core/column/Column.h"
#include "core/AbstractFilter.h"
#include "core/datatypes/SimpleCopyThroughFilter.h"
#include "core/datatypes/Double2StringFilter.h"
#include "core/datatypes/String2DoubleFilter.h"
#include "core/datatypes/DateTime2StringFilter.h"
#include "core/datatypes/String2DateTimeFilter.h"

#include <QKeyEvent>
#include <QMessageBox>
#include <QClipboard>
#include <QInputDialog>
#include <QDate>
#include <QApplication>

 /*!
	\class SpreahsheetView
	\brief View class for Spreadsheet

	\ingroup commonfrontend
 */
SpreadsheetView::SpreadsheetView(Spreadsheet *spreadsheet)
  : m_spreadsheet(spreadsheet){
	m_model = new SpreadsheetModel(spreadsheet);
	init();
}

SpreadsheetView::~SpreadsheetView(){
	delete m_model;
}

void SpreadsheetView::init(){
//TODO 	createActions();

	setModel(m_model);
	
	m_horizontalHeader = new SpreadsheetDoubleHeaderView();
    m_horizontalHeader->setClickable(true);
    m_horizontalHeader->setHighlightSections(true);
	setHorizontalHeader(m_horizontalHeader);

	m_delegate = new SpreadsheetItemDelegate(this);
	setItemDelegate(m_delegate);

	setFocusPolicy(Qt::StrongFocus);
	setFocus();
	setCornerButtonEnabled(true);
	setSelectionMode(QAbstractItemView::ExtendedSelection);

	QHeaderView * v_header = verticalHeader();
	// Remark: ResizeToContents works in Qt 4.2.3 but is broken in 4.3.0
	// Should be fixed in 4.3.1 though, see:
	// http://trolltech.com/developer/task-tracker/index_html?method=entry&id=165567
	v_header->setResizeMode(QHeaderView::ResizeToContents);
	v_header->setMovable(false);
	m_horizontalHeader->setResizeMode(QHeaderView::Interactive);
	m_horizontalHeader->setMovable(true);
	connect(m_horizontalHeader, SIGNAL(sectionMoved(int,int,int)), this, SLOT(handleHorizontalSectionMoved(int,int,int)));
	connect(m_horizontalHeader, SIGNAL(sectionDoubleClicked(int)), this, SLOT(handleHorizontalHeaderDoubleClicked(int)));
	connect(m_horizontalHeader, SIGNAL(sectionResized(int, int, int)), this, SLOT(handleHorizontalSectionResized(int, int, int)));
	
	m_horizontalHeader->setDefaultSectionSize(defaultColumnWidth());

	v_header->installEventFilter(this);
	m_horizontalHeader->installEventFilter(this);
	installEventFilter(this);

	connect(m_model, SIGNAL(headerDataChanged(Qt::Orientation,int,int)), this, 
		SLOT(updateHeaderGeometry(Qt::Orientation,int,int)) ); 
	connect(m_model, SIGNAL(headerDataChanged(Qt::Orientation,int,int)), this, 
		SLOT(handleHeaderDataChanged(Qt::Orientation,int,int)) ); 

	int i=0;
	foreach(Column * col, m_spreadsheet->children<Column>())
		m_horizontalHeader->resizeSection(i++, col->width());


	QItemSelectionModel * sel_model = selectionModel();

	connect(sel_model, SIGNAL(currentColumnChanged(const QModelIndex&, const QModelIndex&)), 
		this, SLOT(currentColumnChanged(const QModelIndex&, const QModelIndex&)));
	connect(sel_model, SIGNAL(selectionChanged(const QItemSelection&,const QItemSelection&)),
		this, SLOT(selectionChanged(const QItemSelection&,const QItemSelection&)));
	
//TODO 	connectActions();
	showComments(defaultCommentVisibility());

	connect(m_spreadsheet, SIGNAL(aspectAdded(const AbstractAspect*)),
			this, SLOT(handleAspectAdded(const AbstractAspect*)));
	connect(m_spreadsheet, SIGNAL(aspectAboutToBeRemoved(const AbstractAspect*)),
			this, SLOT(handleAspectAboutToBeRemoved(const AbstractAspect*)));
	connect(m_spreadsheet, SIGNAL(requestProjectMenu(QMenu*,bool*)), this, SLOT(fillProjectMenu(QMenu*,bool*)));
	connect(m_spreadsheet, SIGNAL(requestProjectContextMenu(QMenu*)), this, SLOT(createContextMenu(QMenu*)));
	
	connect(m_spreadsheet, SIGNAL(columnSelected(int)), this, SLOT(selectColumn(int)) ); 
	connect(m_spreadsheet, SIGNAL(columnDeselected(int)), this, SLOT(deselectColumn(int)) ); 
}

//! Private ctor for initActionManager() only
SpreadsheetView::SpreadsheetView(){
	m_model = NULL;
// 	TODO createActions();
}

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
	disconnect(m_horizontalHeader, SIGNAL(sectionResized(int, int, int)), this, SLOT(handleHorizontalSectionResized(int, int, int)));
	m_horizontalHeader->resizeSection(m_spreadsheet->indexOfChild<Column>(col), col->width());
	connect(m_horizontalHeader, SIGNAL(sectionResized(int, int, int)), this, SLOT(handleHorizontalSectionResized(int, int, int)));
}

void SpreadsheetView::setColumnWidth(int col, int width){ 
	m_horizontalHeader->resizeSection(col, width);
}

int SpreadsheetView::columnWidth(int col) const{ 
	return m_horizontalHeader->sectionSize(col);
}

void SpreadsheetView::handleHorizontalSectionResized(int logicalIndex, int oldSize, int newSize){	
	Q_UNUSED(oldSize);
	static bool inside = false;
	m_spreadsheet->column(logicalIndex)->setWidth(newSize);
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
	QModelIndex idx = currentIndex();
    if (idx.row()+1 >= m_spreadsheet->rowCount())	{
		int new_size = m_spreadsheet->rowCount()+1;
		m_spreadsheet->setRowCount(new_size);
	}
	setCurrentIndex(idx.sibling(idx.row()+1, idx.column()));
}

void SpreadsheetView::goToCell(int row, int col){
	QModelIndex index = m_model->index(row, col);
	scrollTo(index);
	setCurrentIndex(index);
}

//TODO for what?
void SpreadsheetView::deselectAll(){
	clearSelection();
}

void SpreadsheetView::handleHorizontalSectionMoved(int index, int from, int to){
	static bool inside = false;
	if (inside) return;

	Q_ASSERT(index == from);

	inside = true;
	horizontalHeader()->moveSection(to, from);
	inside = false;
	m_spreadsheet->moveColumn(from, to);
}

//TODO
void SpreadsheetView::handleHorizontalHeaderDoubleClicked(int index){
	Q_UNUSED(index);
// 	showControlDescriptionTab();
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

void SpreadsheetView::selectionChanged(const QItemSelection & selected, const QItemSelection & deselected){
	Q_UNUSED(selected);
	Q_UNUSED(deselected);
}

//TODO		
void SpreadsheetView::handleHeaderDataChanged(Qt::Orientation orientation, int first, int last){
	if (orientation != Qt::Horizontal) return;

	QItemSelectionModel * sel_model = selectionModel();

	int col = sel_model->currentIndex().column();
	if (col < first || col > last) return;
// 	TODO setColumnForControlTabs(col);
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
int SpreadsheetView::selectedColumnCount(SciDAVis::PlotDesignation pd){
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
		return selectionModel()->isColumnSelected(col, QModelIndex());
	else
		return selectionModel()->columnIntersectsSelection(col, QModelIndex());
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
		return selectionModel()->isRowSelected(row, QModelIndex());
	else
		return selectionModel()->rowIntersectsSelection(row, QModelIndex());
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

	return selectionModel()->isSelected(m_model->index(row, col));
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
	 selectionModel()->select(m_model->index(row, col), 
			 select ? QItemSelectionModel::Select : QItemSelectionModel::Deselect);
}

/*!
  Select/Deselect a range of cells.
 */
void SpreadsheetView::setCellsSelected(int first_row, int first_col, int last_row, int last_col, bool select){
	QModelIndex top_left = m_model->index(first_row, first_col);
	QModelIndex bottom_right = m_model->index(last_row, last_col);
	selectionModel()->select(QItemSelection(top_left, bottom_right), 
			select ? QItemSelectionModel::SelectCurrent : QItemSelectionModel::Deselect);
}

/*!
  Determine the current cell (-1 if no cell is designated as the current).
 */
void SpreadsheetView::getCurrentCell(int * row, int * col){
	QModelIndex index = selectionModel()->currentIndex();
	if (index.isValid())	{
		*row = index.row();
		*col = index.column();
	}else{
		*row = -1;
		*col = -1;
	}
}

//TODO
bool SpreadsheetView::eventFilter(QObject * watched, QEvent * event){
	QHeaderView * v_header = verticalHeader();

	if (event->type() == QEvent::ContextMenu){
		QContextMenuEvent *cm_event = static_cast<QContextMenuEvent *>(event);
		QPoint global_pos = cm_event->globalPos();
		if (watched == v_header){	
// TODO			showSpreadsheetViewRowContextMenu(global_pos);
		}else if (watched == m_horizontalHeader) {	
			int col = m_horizontalHeader->logicalIndexAt(cm_event->pos());
			if (!isColumnSelected(col, true)) {
				QItemSelectionModel *sel_model = selectionModel();
				sel_model->clearSelection();
				sel_model->select(QItemSelection(m_model->index(0, col, QModelIndex()),
							m_model->index(m_model->rowCount()-1, col, QModelIndex())),
						QItemSelectionModel::Select);
			}
//TODO			showSpreadsheetViewColumnContextMenu(global_pos);
		}else if (watched == this){
//TODO			showSpreadsheetViewContextMenu(global_pos);
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

void SpreadsheetView::activateFormulaMode(bool on) 
{ 
	m_model->activateFormulaMode(on); 
}

void SpreadsheetView::goToNextColumn(){
	if (m_spreadsheet->columnCount() == 0) return;

	QModelIndex idx = currentIndex();
	int col = idx.column()+1;
    if (col >= m_spreadsheet->columnCount())
		col = 0;
	
	setCurrentIndex(idx.sibling(idx.row(), col));
}

void SpreadsheetView::goToPreviousColumn(){
	if (m_spreadsheet->columnCount() == 0)
	  return;

	QModelIndex idx = currentIndex();
	int col = idx.column()-1;
    if (col < 0)
		col = m_spreadsheet->columnCount()-1;
	
	setCurrentIndex(idx.sibling(idx.row(), col));
}


void SpreadsheetView::cutSelection(){
	int first = firstSelectedRow();
	if ( first < 0 )
	  return;

	WAIT_CURSOR;
	m_spreadsheet->beginMacro(tr("%1: cut selected cell(s)").arg(m_spreadsheet->name()));
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
				else if (col_ptr->columnMode() == SciDAVis::Numeric)
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
				output_str += "\t";
		}
		if (r < rows-1)
			output_str += "\n";
	}
	QApplication::clipboard()->setText(output_str);
	RESET_CURSOR;
}

void SpreadsheetView::pasteIntoSelection(){
	if (m_spreadsheet->columnCount() < 1 || m_spreadsheet->rowCount() < 1)
	  return;

	WAIT_CURSOR;
	m_spreadsheet->beginMacro(tr("%1: paste from clipboard").arg(m_spreadsheet->name()));
	const QMimeData * mime_data = QApplication::clipboard()->mimeData();

	int first_col = firstSelectedColumn(false);
	int last_col = lastSelectedColumn(false);
	int first_row = firstSelectedRow(false);
	int last_row = lastSelectedRow(false);
	int input_row_count = 0;
	int input_col_count = 0;
	int rows, cols;

	if (mime_data->hasFormat("text/plain"))
	{
		QString input_str = QString(mime_data->data("text/plain")).trimmed();
		QList< QStringList > cell_texts;
		QStringList input_rows(input_str.split("\n"));
		input_row_count = input_rows.count();
		input_col_count = 0;
		for (int i=0; i<input_row_count; i++)
		{
			cell_texts.append(input_rows.at(i).trimmed().split(QRegExp("\\s+")));
			if (cell_texts.at(i).count() > input_col_count) input_col_count = cell_texts.at(i).count();
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
					Column * new_col = new Column(QString::number(i+1), SciDAVis::Text);
					new_col->setPlotDesignation(SciDAVis::Y);
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
				if (isCellSelected(first_row + r, first_col + c) && (c < cell_texts.at(r).count()) )
				{
					Column * col_ptr = m_spreadsheet->column(first_col + c);
					if (formulaModeActive())
					{
						col_ptr->setFormula(first_row + r, cell_texts.at(r).at(c));  
					}
					else
						col_ptr->asStringColumn()->setTextAt(first_row+r, cell_texts.at(r).at(c));
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
	m_spreadsheet->beginMacro(tr("%1: mask selected cell(s)").arg(m_spreadsheet->name()));
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
	m_spreadsheet->beginMacro(tr("%1: unmask selected cell(s)").arg(m_spreadsheet->name()));
	QList<Column*> list = selectedColumns();
	foreach(Column * col_ptr, list)	{
		int col = m_spreadsheet->indexOfChild<Column>(col_ptr);
		for (int row=first; row<=last; row++)
			if (isCellSelected(row, col)) col_ptr->setMasked(row, false);  
	}
	m_spreadsheet->endMacro();
	RESET_CURSOR;
}

void SpreadsheetView::recalculateSelectedCells(){
	// TODO
	QMessageBox::information(0, "info", "not yet implemented");
}

void SpreadsheetView::fillSelectedCellsWithRowNumbers(){
	if (selectedColumnCount() < 1) return;
	int first = firstSelectedRow();
	int last = lastSelectedRow();
	if ( first < 0 ) return;
	
	WAIT_CURSOR;
	m_spreadsheet->beginMacro(tr("%1: fill cells with row numbers").arg(m_spreadsheet->name()));
	foreach(Column * col_ptr, selectedColumns()) {
		int col = m_spreadsheet->indexOfChild<Column>(col_ptr);

		switch (col_ptr->columnMode()) {
			case SciDAVis::Numeric:
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
			case SciDAVis::Text:
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
	m_spreadsheet->beginMacro(tr("%1: fill cells with random values").arg(m_spreadsheet->name()));
	qsrand(QTime::currentTime().msec());
	foreach(Column *col_ptr, selectedColumns()) {
		int col = m_spreadsheet->indexOfChild<Column>(col_ptr);
		switch (col_ptr->columnMode()) {
			case SciDAVis::Numeric:
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
			case SciDAVis::Text:
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
			case SciDAVis::DateTime:
			case SciDAVis::Month:
			case SciDAVis::Day:
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
	}
	m_spreadsheet->endMacro();
	RESET_CURSOR;
}

/*!
  Insert columns depending on the selection.
 */
void SpreadsheetView::insertEmptyColumns(){
	int first = firstSelectedColumn();
	int last = lastSelectedColumn();
	if ( first < 0 ) return;
	int count, current = first;

	WAIT_CURSOR;
	m_spreadsheet->beginMacro(QObject::tr("%1: insert empty column(s)").arg(m_spreadsheet->name()));
	int rows = m_spreadsheet->rowCount();
	while( current <= last )
	{
		current = first+1;
		while( current <= last && isColumnSelected(current) ) current++;
		count = current-first;
		Column *first_col = m_spreadsheet->child<Column>(first);
		for (int i=0; i<count; i++)
		{
			Column * new_col = new Column(QString::number(i+1), SciDAVis::Numeric);
			new_col->setPlotDesignation(SciDAVis::Y);
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
	m_spreadsheet->beginMacro(QObject::tr("%1: remove selected column(s)").arg(m_spreadsheet->name()));

	QList< Column* > list = selectedColumns();
	foreach(Column* ptr, list)
		m_spreadsheet->removeChild(ptr);

	m_spreadsheet->endMacro();
	RESET_CURSOR;
}

void SpreadsheetView::clearSelectedColumns(){
	WAIT_CURSOR;
	m_spreadsheet->beginMacro(QObject::tr("%1: clear selected column(s)").arg(m_spreadsheet->name()));

	QList< Column* > list = selectedColumns();
	if (formulaModeActive())	{
		foreach(Column* ptr, list)
			ptr->clearFormulas();
	}else{
		foreach(Column* ptr, list)
			ptr->clear();
	}

	m_spreadsheet->endMacro();
	RESET_CURSOR;
}

void SpreadsheetView::setSelectionAs(SciDAVis::PlotDesignation pd){
	WAIT_CURSOR;
	m_spreadsheet->beginMacro(QObject::tr("%1: set plot designation(s)").arg(m_spreadsheet->name()));

	QList< Column* > list = selectedColumns();
	foreach(Column* ptr, list)
		ptr->setPlotDesignation(pd);

	m_spreadsheet->endMacro();
	RESET_CURSOR;
}

void SpreadsheetView::setSelectedColumnsAsX(){
	setSelectionAs(SciDAVis::X);
}

void SpreadsheetView::setSelectedColumnsAsY(){
	setSelectionAs(SciDAVis::Y);
}

void SpreadsheetView::setSelectedColumnsAsZ(){
	setSelectionAs(SciDAVis::Z);
}

void SpreadsheetView::setSelectedColumnsAsYError(){
	setSelectionAs(SciDAVis::yErr);
}

void SpreadsheetView::setSelectedColumnsAsXError(){
	setSelectionAs(SciDAVis::xErr);
}

void SpreadsheetView::setSelectedColumnsAsNone(){
	setSelectionAs(SciDAVis::noDesignation);
}

void SpreadsheetView::normalizeSelectedColumns(){
	WAIT_CURSOR;
	m_spreadsheet->beginMacro(QObject::tr("%1: normalize column(s)").arg(m_spreadsheet->name()));
	QList< Column* > cols = selectedColumns();
	foreach(Column * col, cols)	{
		if (col->columnMode() == SciDAVis::Numeric)
		{
			double max = 0.0;
			for (int row=0; row<col->rowCount(); row++)
			{
				if (col->valueAt(row) > max)
					max = col->valueAt(row);
			}
			if (max != 0.0) // avoid division by zero
				for (int row=0; row<col->rowCount(); row++)
					col->setValueAt(row, col->valueAt(row) / max);
		}
	}
	m_spreadsheet->endMacro();
	RESET_CURSOR;
}

void SpreadsheetView::normalizeSelection(){
	WAIT_CURSOR;
	m_spreadsheet->beginMacro(QObject::tr("%1: normalize selection").arg(m_spreadsheet->name()));
	double max = 0.0;
	for (int col=firstSelectedColumn(); col<=lastSelectedColumn(); col++)
		if (m_spreadsheet->column(col)->columnMode() == SciDAVis::Numeric)
			for (int row=0; row<m_spreadsheet->rowCount(); row++)
			{
				if (isCellSelected(row, col) && m_spreadsheet->column(col)->valueAt(row) > max)
					max = m_spreadsheet->column(col)->valueAt(row);
			}

	if (max != 0.0) // avoid division by zero
	{
		for (int col=firstSelectedColumn(); col<=lastSelectedColumn(); col++)
			if (m_spreadsheet->column(col)->columnMode() == SciDAVis::Numeric)
				for (int row=0; row<m_spreadsheet->rowCount(); row++)
				{
					if (isCellSelected(row, col))
						m_spreadsheet->column(col)->setValueAt(row, m_spreadsheet->column(col)->valueAt(row) / max);
				}
	}
	m_spreadsheet->endMacro();
	RESET_CURSOR;
}


void SpreadsheetView::statisticsOnSelectedColumns(){
	// TODO
	QMessageBox::information(0, "info", "not yet implemented");
}

void SpreadsheetView::statisticsOnSelectedRows(){
	// TODO
	QMessageBox::information(0, "info", "not yet implemented");
}

/*!
  Insert rows depending on the selection.
*/
void SpreadsheetView::insertEmptyRows(){
	int first = firstSelectedRow();
	int last = lastSelectedRow();
	int count, current = first;

	if ( first < 0 ) return;

	WAIT_CURSOR;
	m_spreadsheet->beginMacro(QObject::tr("%1: insert empty rows(s)").arg(m_spreadsheet->name()));
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
	m_spreadsheet->beginMacro(QObject::tr("%1: remove selected rows(s)").arg(m_spreadsheet->name()));
	foreach(Interval<int> i, selectedRows().intervals())
		m_spreadsheet->removeRows(i.start(), i.size());
	m_spreadsheet->endMacro();
	RESET_CURSOR;
}

void SpreadsheetView::clearSelectedRows(){
	if ( firstSelectedRow() < 0 ) return;

	WAIT_CURSOR;
	m_spreadsheet->beginMacro(QObject::tr("%1: clear selected rows(s)").arg(m_spreadsheet->name()));
	QList<Column*> list = selectedColumns();
	foreach(Column * col_ptr, list)
	{
		if (formulaModeActive()) {
			foreach(Interval<int> i, selectedRows().intervals())
				col_ptr->setFormula(i, "");
		} else {
			foreach(Interval<int> i, selectedRows().intervals()) {
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
	}
	m_spreadsheet->endMacro();
	RESET_CURSOR;
}

void SpreadsheetView::clearSelectedCells(){
	int first = firstSelectedRow();
	int last = lastSelectedRow();
	if ( first < 0 ) return;

	WAIT_CURSOR;
	m_spreadsheet->beginMacro(tr("%1: clear selected cell(s)").arg(m_spreadsheet->name()));
	QList<Column*> list = selectedColumns();
	foreach(Column * col_ptr, list)
	{
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
	}
	m_spreadsheet->endMacro();
	RESET_CURSOR;
}

void SpreadsheetView::goToCell(){
	bool ok;

	int col = QInputDialog::getInteger(0, tr("Go to Cell"), tr("Enter column"),
			1, 1, m_spreadsheet->columnCount(), 1, &ok);
	if ( !ok ) return;

	int row = QInputDialog::getInteger(0, tr("Go to Cell"), tr("Enter row"),
			1, 1, m_spreadsheet->rowCount(), 1, &ok);
	if ( !ok ) return;

	goToCell(row-1, col-1);
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


/* ========================= static methods ======================= */
ActionManager * SpreadsheetView::action_manager = 0;

ActionManager * SpreadsheetView::actionManager(){
	if (!action_manager)
		initActionManager();
	
	return action_manager;
}

void SpreadsheetView::initActionManager(){
	if (!action_manager)
		action_manager = new ActionManager();

	action_manager->setTitle(tr("Spreadsheet"));
	volatile SpreadsheetView * action_creator = new SpreadsheetView(); // initialize the action texts
	delete action_creator;
}

int SpreadsheetView::defaultColumnWidth() { 
	return Column::global("default_width").toInt();
}

/*!
  Set default for comment visibility for spreadsheet views.
*/
void SpreadsheetView::setDefaultCommentVisibility(bool visible){ 
	Spreadsheet::setGlobal("default_comment_visibility", visible); 
}

/*!
  Return the default for comment visibility for spreadsheet views.
 */
bool SpreadsheetView::defaultCommentVisibility() { 
	return Spreadsheet::global("default_comment_visibility").toBool(); 
}

//TODO 
// void SpreadsheetView::selectAll(){
// 	// the original QSpreadsheetView::selectAll() toggles all cells which is strange behavior IMHO - thzs
// 	QItemSelectionModel * sel_model = selectionModel();
// 	QItemSelection sel(model()->index(0, 0, QModelIndex()), model()->index(model()->rowCount()-1, 
// 		model()->columnCount()-1, QModelIndex()));
// 		
// 	sel_model->select(sel, QItemSelectionModel::Select);
// }

/*!
  Cause a repaint of the header.
*/
void SpreadsheetView::updateHeaderGeometry(Qt::Orientation o, int first, int last){
	Q_UNUSED(first)
	Q_UNUSED(last)
	//TODO
	if (o != Qt::Horizontal) return;
	horizontalHeader()->setStretchLastSection(true);  // ugly hack (flaw in Qt? Does anyone know a better way?)
	horizontalHeader()->updateGeometry();
	horizontalHeader()->setStretchLastSection(false); // ugly hack part 2
}

void SpreadsheetView::keyPressEvent(QKeyEvent * event){
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
	  advanceCell();
	
	QTableView::keyPressEvent(event);
}

/*!
  selects the column \c column in the speadsheet view .
*/
void SpreadsheetView::selectColumn(int column){
  QItemSelection selection(m_model->index(0, column), m_model->index(m_spreadsheet->rowCount()-1, column) );
  selectionModel()->select(selection, QItemSelectionModel::Select);
}

/*!
  deselects the column \c column in the speadsheet view .
*/
void SpreadsheetView::deselectColumn(int column){
  QItemSelection selection(m_model->index(0, column), m_model->index(m_spreadsheet->rowCount()-1, column) );
  selectionModel()->select(selection, QItemSelectionModel::Deselect);
}

/*!
  called when a column in the speadsheet view was clicked (click in the header).
  Propagates the selection of the columnt to the selection change to the \c Spreadsheet object
  (a click in the header always selects the column).
*/
void SpreadsheetView::columnClicked(int column){
//   bool selected = m_view_widget->selectionModel()->isColumnSelected(column, QModelIndex());
//    qDebug()<<"selected column in view:  "<<column;
   m_spreadsheet->setColumnSelectedInView(column, true);
}
