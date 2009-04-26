/***************************************************************************
    File                 : SpreadsheetView.cpp
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Stefan Gerlach (stefan.gerlach*uni-konstanz.de)
    Copyright            : (C) 2008 by Alexander Semke (alexander.semke*web.de)
    Copyright            : (C) 2007-2008 Tilman Benkert (thzs*gmx.net)
                           (replace * with @ in the email addresses)
    Description          : spreadsheet view class

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

#include <KLocale>
#include <KDebug>
#include <KInputDialog>
#include <QHeaderView>
#include <QStandardItemModel>
#include <KAction>
#include <QHBoxLayout>

#include "SpreadsheetView.h"
#include "MainWin.h"
#include "ColumnDialog.h"
#include "ExportDialog.h"
#include "FunctionPlotDialog.h"
#include "pixmaps/pixmap.h"
#include "elements/Point3D.h"

#include "lib/macros.h"
#include "core/Project.h"
#include "core/AbstractFilter.h"
#include "core/column/Column.h"
#include "core/datatypes/SimpleCopyThroughFilter.h"
#include "core/datatypes/Double2StringFilter.h"
#include "core/datatypes/String2DoubleFilter.h"
#include "core/datatypes/DateTime2StringFilter.h"
#include "core/datatypes/String2DateTimeFilter.h"

SpreadsheetView::SpreadsheetView(Spreadsheet *spreadsheet)
 :  m_spreadsheet(spreadsheet)
{
	kDebug()<<"SpreadsheetView()"<<endl;
	m_type = SPREADSHEET;
	m_model = new SpreadsheetModel(spreadsheet);
	init();
}

SpreadsheetView::~SpreadsheetView() {
	delete m_model;
}

void SpreadsheetView::init()
{
	createActions();

	m_main_layout = new QHBoxLayout(this);
	m_main_layout->setSpacing(0);
	m_main_layout->setContentsMargins(0, 0, 0, 0);

	m_view_widget = new QTableView(this);
	m_view_widget->setModel(m_model);
	m_main_layout->addWidget(m_view_widget);

	m_view_widget->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
	m_main_layout->setStretchFactor(m_view_widget, 1);

	setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));

	m_view_widget->setFocusPolicy(Qt::StrongFocus);
	setFocusPolicy(Qt::StrongFocus);
	setFocus();
#if QT_VERSION >= 0x040300
	m_view_widget->setCornerButtonEnabled(true);
#endif

	m_view_widget->setSelectionMode(QAbstractItemView::ExtendedSelection);

	QHeaderView * v_header = m_view_widget->verticalHeader();
	QHeaderView * h_header = m_view_widget->horizontalHeader();
	v_header->setResizeMode(QHeaderView::ResizeToContents);
	v_header->setMovable(false);
	h_header->setResizeMode(QHeaderView::Interactive);
	h_header->setMovable(false);

	h_header->setDefaultSectionSize(defaultColumnWidth());

	int i = 0;
	foreach(Column * col, m_spreadsheet->children<Column>())
		h_header->resizeSection(i++, col->width());

	// keyboard shortcuts
	QShortcut * sel_all = new QShortcut(QKeySequence(tr("Ctrl+A", "Spreadsheet: select all")), m_view_widget);
	connect(sel_all, SIGNAL(activated()), m_view_widget, SLOT(selectAll()));

	connect(m_spreadsheet, SIGNAL(aspectAdded(const AbstractAspect*)),
			this, SLOT(handleAspectAdded(const AbstractAspect*)));
	connect(m_spreadsheet, SIGNAL(aspectAboutToBeRemoved(const AbstractAspect*)),
			this, SLOT(handleAspectAboutToBeRemoved(const AbstractAspect*)));
}

void SpreadsheetView::handleAspectAdded(const AbstractAspect * aspect)
{
	const Column * col = qobject_cast<const Column*>(aspect);
	if (!col || col->parentAspect() != static_cast<AbstractAspect*>(m_spreadsheet))
		return;
	connect(col, SIGNAL(widthChanged(const Column*)), this, SLOT(updateSectionSize(const Column*)));
}

void SpreadsheetView::handleAspectAboutToBeRemoved(const AbstractAspect * aspect)
{
	const Column * col = qobject_cast<const Column*>(aspect);
	if (!col || col->parentAspect() != static_cast<AbstractAspect*>(m_spreadsheet))
		return;
	disconnect(col, 0, this, 0);
}

void SpreadsheetView::updateSectionSize(const Column* col)
{
	QHeaderView *h_header = m_view_widget->horizontalHeader();
	disconnect(h_header, SIGNAL(sectionResized(int, int, int)), this, SLOT(handleHorizontalSectionResized(int, int, int)));
	h_header->resizeSection(m_spreadsheet->indexOfChild<Column>(col), col->width());
	connect(h_header, SIGNAL(sectionResized(int, int, int)), this, SLOT(handleHorizontalSectionResized(int, int, int)));
}

void SpreadsheetView::selectAll()
{
	m_view_widget->selectAll();
}

void SpreadsheetView::deselectAll()
{
	m_view_widget->clearSelection();
}

void SpreadsheetView::setColumnWidth(int col, int width)
{
	m_view_widget->horizontalHeader()->resizeSection(col, width);
}

int SpreadsheetView::columnWidth(int col) const
{
	return m_view_widget->horizontalHeader()->sectionSize(col);
}

void SpreadsheetView::contextMenuEvent(QContextMenuEvent* e) {
	Q_UNUSED(e)
	QMenu *menu = new QMenu(this);
	this->createMenu(menu);
	menu->exec(QCursor::pos());
}

/*!
	initializes KActions used in the context menu of SpreadsheetView and in the menu of MainWin.
*/
void SpreadsheetView::createActions(){
	plotAction = new KAction(KIcon(QIcon(plot2d_xpm)),i18n("Plot"),this);
	connect(plotAction, SIGNAL(triggered()),SLOT(plot()));

	exportDataAction= new KAction(KIcon("document-export"),i18n("Export"),this);
	connect(exportDataAction, SIGNAL(triggered()),SLOT(exportData()));

	editFunctionAction = new KAction(KIcon("edit-rename"),i18n("Edit Function"),this);
	connect(editFunctionAction, SIGNAL(triggered()),SLOT(editFunction()));

	columnPropertiesAction = new KAction(KIcon("select"),i18n("Column properties"),this);
	connect(columnPropertiesAction, SIGNAL(triggered()),SLOT(setProperties()));

	titleAction = new KAction(KIcon("select"),i18n("Set title"),this);
	connect(titleAction, SIGNAL(triggered()),SLOT(setTitle()));

	rowCountAction = new KAction(KIcon("select"),i18n("Set row count"),this);
	connect(rowCountAction, SIGNAL(triggered()),SLOT(setRowNumber()));

	notesAction = new KAction(KIcon("draw-freehand"),i18n("Set notes"),this);
	connect(notesAction, SIGNAL(triggered()),SLOT(setNotes()));

	//TODO add connects
	cutAction = new KAction(KIcon("edit-cut"),i18n("Cut"),this);
	cutAction->setEnabled(false);
	copyAction = new KAction(KIcon("edit-copy"),i18n("Copy"),this);
	copyAction->setEnabled(false);
	pasteAction = new KAction(KIcon("edit-paste"),i18n("Paste"),this);
	pasteAction->setEnabled(false);
	clearAction = new KAction(KIcon("edit-clear"),i18n("Clear"),this);
	clearAction->setEnabled(false);
	selectAction = new KAction(KIcon("edit-select-all"),i18n("Select"),this);
	selectAction->setEnabled(false);
	// .. all, nothing, invert

	columnValuesAction = new KAction(KIcon(QIcon(plot2d_xpm)),i18n("Set column values"),this);
	connect(columnValuesAction, SIGNAL(triggered()),SLOT(setColumnValues()));

	fillWithAction = new KAction(KIcon(QIcon(plot2d_xpm)),i18n("Fill with"),this);
	fillWithAction->setEnabled(false);

	// row number, random value

	convertAction = new KAction(KIcon("transform-rotate"),i18n("Convert"),this);
	convertAction->setEnabled(false);

	// ...
	editWithAction = new KAction(KIcon(QIcon(plot2d_xpm)),i18n("Edit with"),this);
	// ...

	addColumnAction = new KAction(KIcon("list-add"),i18n("Add column"),this);
	connect(addColumnAction, SIGNAL(triggered()),SLOT(addColumn()));

	deleteColumnAction = new KAction(KIcon("list-remove"),i18n("Delete selected column"),this);
	connect(deleteColumnAction, SIGNAL(triggered()),SLOT(deleteSelectedColumns()));

	deleteRowAction = new KAction(KIcon("list-remove"),i18n("Delete selected row"),this);
	connect(deleteRowAction, SIGNAL(triggered()),SLOT(deleteSelectedRows()));

	maskingAction = new KAction(KIcon(QIcon(plot2d_xpm)),i18n("Masking"),this);
	maskingAction->setEnabled(false);
	// ...

	normalizeAction = new KAction(KIcon(QIcon(plot2d_xpm)),i18n("Normalize"),this);
	normalizeAction->setEnabled(false);

	// sum=1, max=1

	sortAction = new KAction(KIcon(QIcon(plot2d_xpm)),i18n("Sort"),this);
	sortAction->setEnabled(false);

	// ascending, descending

	columnStatisticsAction = new KAction(KIcon(QIcon(plot2d_xpm)),i18n("Statistics on column"),this);
	columnStatisticsAction ->setEnabled(false);
	rowStatisticsAction = new KAction(KIcon(QIcon(plot2d_xpm)),i18n("Statistics on row"),this);
	rowStatisticsAction ->setEnabled(false);
}

void SpreadsheetView::createMenu(QMenu *menu) const{
	if (!menu)
		menu=new QMenu();

	menu->addAction(plotAction);
	menu->addAction(exportDataAction);
	menu->addAction(editFunctionAction);
	menu->addSeparator();
	menu->addAction(columnPropertiesAction);

	menu->addSeparator();
	menu->addAction(titleAction);
	menu->addAction(rowCountAction);
	menu->addAction(notesAction);

	menu->addSeparator();
	menu->addAction(cutAction);
	menu->addAction(copyAction);
	menu->addAction(pasteAction);
	menu->addAction(clearAction);
	menu->addAction(selectAction);

	menu->addSeparator();
	menu->addAction(columnValuesAction);

	menu->addSeparator();
	menu->addAction(fillWithAction);
	menu->addAction(convertAction);
	menu->addAction(editWithAction);

	menu->addSeparator();
	menu->addAction(addColumnAction);
	menu->addAction(deleteColumnAction);
	menu->addAction(deleteRowAction);
	menu->addAction(maskingAction);

	menu->addSeparator();
	menu->addAction(normalizeAction);
	menu->addAction(sortAction);
	menu->addAction(columnStatisticsAction);
	menu->addAction(rowStatisticsAction);

	kDebug()<<"SpreadsheetView menu created"<<endl;
}


void SpreadsheetView::setTitle(QString title) {
	kDebug()<<"title ="<<title<<endl;
	bool ok=true;
	if(title.isEmpty())
		title = KInputDialog::getText("LabPlot", i18n("SpreadsheetView title : "), m_spreadsheet->name(), &ok);

	if(ok && !title.isEmpty()) {
		m_spreadsheet->setName(title);
	}
}

void SpreadsheetView::setRowNumber(int row) {
	kDebug()<<"row ="<<row<<endl;
	bool ok=true;
	if(row<=0)
		row = KInputDialog::getInteger("LabPlot", i18n("Row count : "), rowCount(), 1, INT_MAX, 1, 10, &ok);

	if(ok && row>0)
		m_spreadsheet->setRowCount(row);
}

QString SpreadsheetView::text(int row, int col) const {
//	kDebug()<<"SpreadsheetView::text("<<row<<col<<")"<<endl;
	QModelIndex index=m_view_widget->model()->index(row,col);
	return m_view_widget->model()->data(index).toString();
}

void SpreadsheetView::setText(int row, int col, QString text) {
	QModelIndex index=m_view_widget->model()->index(row,col);
	m_view_widget->model()->setData(index,text);
}

void SpreadsheetView::setNotes(QString t) {
	kDebug()<<endl;
	bool ok=true;
	if(t.isEmpty())
		t = KInputDialog::getMultiLineText("LabPlot", i18n("SpreadsheetView notes : "),
			Notes(), &ok);
	if(!ok) return;

	m_spreadsheet->setComment(t);
}

QString SpreadsheetView::columnHeader(int col) const {
	if(col<0) col=0;
	return m_view_widget->model()->headerData(col,Qt::Horizontal).toString();
}

void SpreadsheetView::setProperties(QString label,
	SciDAVis::PlotDesignation type, SciDAVis::ColumnMode format) {
	kDebug()<<label<<type<<format<<endl;

	if(label.isEmpty())
		(new ColumnDialog(this,this))->show();
	else {
		setColumnName(currentColumn(),label);
		setColumnType(currentColumn(),type);
		setColumnFormat(currentColumn(),format);
	}
}

void SpreadsheetView::resetHeader(int from) {
	kDebug()<<endl;
	for (int col=from;col<columnCount();col++) {
		setColumnFormat(col,SciDAVis::Numeric);
		if(col==0)
			setColumnType(col,SciDAVis::X);
		else
			setColumnType(col,SciDAVis::Y);
		if(col<26)
			setColumnName(col,QChar(col+65));
		else
			setColumnName(col,QString(QChar(col/26-1+65))+QString(QChar(65+col%26)));
	}
}

QString SpreadsheetView::columnName(int col) const {
	if(col<0 || col > columnCount()) return QString();
	return m_spreadsheet->column(col)->name();
}

void SpreadsheetView::setColumnName(int col, QString name) {
	if(col<0 || col > columnCount()) return;
//	kDebug()<<"column ="<<col<<", name = "<<name<<endl;
	if(col<0) return;
	m_spreadsheet->column(col)->setName(name);
}

SciDAVis::PlotDesignation SpreadsheetView::columnType(int col) const {
	if(col<0 || col > columnCount()) return SciDAVis::noDesignation;
	return m_spreadsheet->column(col)->plotDesignation();
}

void SpreadsheetView::setColumnType(int col, SciDAVis::PlotDesignation type) {
	m_spreadsheet->column(col)->setPlotDesignation(type);
}

SciDAVis::ColumnMode SpreadsheetView::columnFormat(int col) const {
	return m_spreadsheet->column(col)->columnMode();
}

void SpreadsheetView::setColumnFormat(int col, SciDAVis::ColumnMode format) {
	m_spreadsheet->column(col)->setColumnMode(format);
}

int SpreadsheetView::filledRows(int col) const {
	kDebug()<<"SpreadsheetView::filledRows("<<col<<")"<<endl;
	if(col < 0 || col > columnCount())
		return -1;

	return m_spreadsheet->column(col)->rowCount();
}

int SpreadsheetView::currentColumn() const {
	QModelIndex index = m_view_widget->currentIndex();
	return index.column();
}

QList<int> SpreadsheetView::currentColumns() const {
	QList<int> columns;
	QModelIndexList list = m_view_widget->selectionModel()->selectedColumns();
	for (int i=0; i<list.size(); i++)
		columns.append(list.at(i).column());
	return columns;
}

int SpreadsheetView::currentRow() const {
	QModelIndex index = m_view_widget->currentIndex();
	return index.row();
}

QList<int> SpreadsheetView::currentRows() const {
	QList<int> rows;
	QModelIndexList list = m_view_widget->selectionModel()->selectedRows();
	for (int i=0; i<list.size(); i++)
		rows.append(list.at(i).row());
	return rows;
}

void SpreadsheetView::deleteSelectedColumns() {
	removeSelectedColumns();
}

void SpreadsheetView::deleteSelectedRows() {
	removeSelectedRows();
}

void SpreadsheetView::addSet(Set* s) {
	kDebug()<<endl;
	m_set=s;

	int columns=0;
	switch(s->type()) {
	case Set::SET2D:
		columns=2;
		break;
	case Set::SET3D:
		columns=3;
		break;
	default: break;
	}

	if(columnCount()<columns)
		setColumnCount(columns);

	// add empty columns if necessary
	for(int i=0;i<columns;i++) {
		if(filledRows(columnCount()-columns)>1)
			setColumnCount(columnCount()+1);
	}

	if(s->list_data.size() > rowCount()) //TODO do we really need this if here?
		setRowCount(s->list_data.size());

	this->displaySet();
}

void SpreadsheetView::displaySet(){
// TODO : use m_spreadsheet
//TODO clear the content of the spreadsheet!!!

	int columns=this->columnCount();
	switch(m_set->type()) {
	case Set::SET2D: {
		kDebug()<<"set2d is going to be shown"<<endl;
 		const Point* point;
		for(int i=0; i<m_set->list_data.size(); i++) {
			point=&m_set->list_data.at(i);
			this->setText(i, columns-2, QString::number(point->x(), 'g', 10));
			this->setText(i, columns-1, QString::number(point->y(), 'g', 10));
/*			if(data[i].Masked()) {
				xitem->setMasked();
				yitem->setMasked();
			}
*/
		}
		break;
	}
	case Set::SET3D: {
		kDebug()<<"set3d is going to be shown"<<endl;
		const Point3D* point;
		for(int i=0; i<m_set->list_data.size(); i++) {
			point=static_cast<const Point3D*>(&m_set->list_data.at(i));
			this->setText(i, columns-3, QString::number(point->x(), 'g', 10));
			this->setText(i, columns-2, QString::number(point->y(), 'g', 10));
			this->setText(i, columns-1, QString::number(point->z(), 'g', 10));
/*			if(data[i].Masked()) {
				xitem->setMasked();
				yitem->setMasked();
			zitem->setMasked();
			}
*/
		}
		break;
	}
	default:
		 break;
	}

	kDebug()<<"set displayed"<<endl;
}

void SpreadsheetView::plot() {
	kDebug()<<"not implemented yet!"<<endl;
	// TODO : use SpreadsheetViewPlotDialog
	// plot type, which columns, destination

	// create set
// 	int N=rowCount();
// 	Point *data = new Point[N];
// 	for(int i = 0;i < N;i++) {
// 		data[i].setPoint(text(i,0).toDouble(),text(i,1).toDouble());
// 	}
// 	Set2D *g = new Set2D(windowTitle(),data,N);

	//mw->addSet(g,sheetcb->currentIndex(),Plot::PLOT2D);
	//TODO mw->addSet(g,1,Plot::PLOT2D);
}

void SpreadsheetView::exportData() {
	(new ExportDialog(this))->exec();
}

void SpreadsheetView::setColumnValues() {
	kDebug()<<"not implemented yet!"<<endl;
	// TODO : column values dialog
}

void SpreadsheetView::editFunction(){
	Plot::PlotType type;
	//TODO rethink/reimplement this approach
	if (m_set->type()==Set::SET2D)
		type=Plot::PLOT2D;
	else if (m_set->type()==Set::SET3D)
		type=Plot::PLOT3D;

	// TODO: As I stated in other places: I would be better for SpreadsheetView not depend on MainWin at all.
	// But if you really need it, this is the "SciDAVis way" of getting a pointer to the main window: - Tilman
	if (m_spreadsheet->project() == NULL) return;
	MainWin * mw = static_cast<MainWin *>(m_spreadsheet->project()->view());
	FunctionPlotDialog* dlg=new FunctionPlotDialog(mw, type);
	dlg->setSet(m_set);
	if (dlg->exec())
		this->displaySet(); //TODO trigger update only if data set was changed
}

int SpreadsheetView::selectedColumnCount(bool full) {
	int count = 0;
	int cols = m_spreadsheet->columnCount();
	for (int i=0; i<cols; i++)
		if (isColumnSelected(i, full)) count++;
	return count;
}

int SpreadsheetView::selectedColumnCount(SciDAVis::PlotDesignation pd) {
	int count = 0;
	int cols = m_spreadsheet->columnCount();
	for (int i=0; i<cols; i++)
		if ( isColumnSelected(i, false) && (m_spreadsheet->column(i)->plotDesignation() == pd) ) count++;

	return count;
}

bool SpreadsheetView::isColumnSelected(int col, bool full) {
	if (full)
		return m_view_widget->selectionModel()->isColumnSelected(col, QModelIndex());
	else
		return m_view_widget->selectionModel()->columnIntersectsSelection(col, QModelIndex());
}

QList<Column*> SpreadsheetView::selectedColumns(bool full) {
	QList<Column*> list;
	int cols = m_spreadsheet->columnCount();
	for (int i=0; i<cols; i++)
		if (isColumnSelected(i, full)) list << m_spreadsheet->column(i);

	return list;
}

int SpreadsheetView::selectedRowCount(bool full) {
	int count = 0;
	int rows = m_spreadsheet->rowCount();
	for (int i=0; i<rows; i++)
		if (isRowSelected(i, full)) count++;
	return count;
}

bool SpreadsheetView::isRowSelected(int row, bool full) {
	if (full)
		return m_view_widget->selectionModel()->isRowSelected(row, QModelIndex());
	else
		return m_view_widget->selectionModel()->rowIntersectsSelection(row, QModelIndex());
}

int SpreadsheetView::firstSelectedColumn(bool full)
{
	int cols = m_spreadsheet->columnCount();
	for (int i=0; i<cols; i++) {
		if (isColumnSelected(i, full))
			return i;
	}
	return -1;
}

int SpreadsheetView::lastSelectedColumn(bool full)
{
	int cols = m_spreadsheet->columnCount();
	for (int i=cols-1; i>=0; i--)
		if (isColumnSelected(i, full)) return i;

	return -2;
}

int SpreadsheetView::firstSelectedRow(bool full)
{
	int rows = m_spreadsheet->rowCount();
	for (int i=0; i<rows; i++)
	{
		if (isRowSelected(i, full))
			return i;
	}
	return -1;
}

int SpreadsheetView::lastSelectedRow(bool full)
{
	int rows = m_spreadsheet->rowCount();
	for (int i=rows-1; i>=0; i--)
		if (isRowSelected(i, full)) return i;

	return -2;
}

bool SpreadsheetView::isCellSelected(int row, int col)
{
	if (row < 0 || col < 0 || row >= m_spreadsheet->rowCount() || col >= m_spreadsheet->columnCount()) return false;

	return m_view_widget->selectionModel()->isSelected(m_model->index(row, col));
}

void SpreadsheetView::setCellSelected(int row, int col, bool select)
{
	 m_view_widget->selectionModel()->select(m_model->index(row, col),
			 select ? QItemSelectionModel::Select : QItemSelectionModel::Deselect);
}

void SpreadsheetView::setCellsSelected(int first_row, int first_col, int last_row, int last_col, bool select)
{
	QModelIndex top_left = m_model->index(first_row, first_col);
	QModelIndex bottom_right = m_model->index(last_row, last_col);
	m_view_widget->selectionModel()->select(QItemSelection(top_left, bottom_right),
			select ? QItemSelectionModel::SelectCurrent : QItemSelectionModel::Deselect);
}

void SpreadsheetView::getCurrentCell(int * row, int * col)
{
	QModelIndex index = m_view_widget->selectionModel()->currentIndex();
	if (index.isValid())
	{
		*row = index.row();
		*col = index.column();
	}
	else
	{
		*row = -1;
		*col = -1;
	}
}

bool SpreadsheetView::formulaModeActive() const
{
	return m_model->formulaModeActive();
}

void SpreadsheetView::activateFormulaMode(bool on)
{
	m_model->activateFormulaMode(on);
}

void SpreadsheetView::goToNextColumn()
{
	if (m_spreadsheet->columnCount() == 0) return;

	QModelIndex idx = m_view_widget->currentIndex();
	int col = idx.column()+1;
    if (col >= m_spreadsheet->columnCount())
		col = 0;
	m_view_widget->setCurrentIndex(idx.sibling(idx.row(), col));
}

void SpreadsheetView::goToPreviousColumn()
{
	if (m_spreadsheet->columnCount() == 0) return;

	QModelIndex idx = m_view_widget->currentIndex();
	int col = idx.column()-1;
    if (col < 0)
		col = m_spreadsheet->columnCount()-1;
	m_view_widget->setCurrentIndex(idx.sibling(idx.row(), col));
}

void SpreadsheetView::cutSelection()
{
	int first = firstSelectedRow();
	if ( first < 0 ) return;

	WAIT_CURSOR;
	m_spreadsheet->beginMacro(tr("%1: cut selected cell(s)").arg(m_spreadsheet->name()));
	copySelection();
	clearSelectedCells();
	m_spreadsheet->endMacro();
	RESET_CURSOR;
}

void SpreadsheetView::copySelection()
{
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
					output_str += m_spreadsheet->text(first_row + r, first_col + c);
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

void SpreadsheetView::pasteIntoSelection()
{
	if (m_spreadsheet->columnCount() < 1 || m_spreadsheet->rowCount() < 1) return;

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
		QString input_str = QString(mime_data->data("text/plain"));
		QList< QStringList > cell_texts;
		QStringList input_rows(input_str.split("\n"));
		input_row_count = input_rows.count();
		input_col_count = 0;
		for (int i=0; i<input_row_count; i++)
		{
			cell_texts.append(input_rows.at(i).split("\t"));
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

void SpreadsheetView::maskSelection()
{
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

void SpreadsheetView::unmaskSelection()
{
	int first = firstSelectedRow();
	int last = lastSelectedRow();
	if ( first < 0 ) return;

	WAIT_CURSOR;
	m_spreadsheet->beginMacro(tr("%1: unmask selected cell(s)").arg(m_spreadsheet->name()));
	QList<Column*> list = selectedColumns();
	foreach(Column * col_ptr, list)
	{
		int col = m_spreadsheet->indexOfChild<Column>(col_ptr);
		for (int row=first; row<=last; row++)
			if (isCellSelected(row, col)) col_ptr->setMasked(row, false);
	}
	m_spreadsheet->endMacro();
	RESET_CURSOR;
}

void SpreadsheetView::fillSelectedCellsWithRowNumbers()
{
	if (selectedColumnCount() < 1) return;
	int first = firstSelectedRow();
	int last = lastSelectedRow();
	if ( first < 0 ) return;

	WAIT_CURSOR;
	m_spreadsheet->beginMacro(tr("%1: fill cells with row numbers").arg(m_spreadsheet->name()));
	QList<Column*> list = selectedColumns();
	foreach(Column * col_ptr, list)
	{
		int col = m_spreadsheet->indexOfChild<Column>(col_ptr);
		for (int row=first; row<=last; row++)
			if (isCellSelected(row, col))
				col_ptr->asStringColumn()->setTextAt(row, QString::number(row+1));
	}
	m_spreadsheet->endMacro();
	RESET_CURSOR;
}

void SpreadsheetView::fillSelectedCellsWithRandomNumbers()
{
	if (selectedColumnCount() < 1) return;
	int first = firstSelectedRow();
	int last = lastSelectedRow();
	if ( first < 0 ) return;

	WAIT_CURSOR;
	m_spreadsheet->beginMacro(tr("%1: fill cells with random values").arg(m_spreadsheet->name()));
	qsrand(QTime::currentTime().msec());
	QList<Column*> list = selectedColumns();
	foreach(Column * col_ptr, list)
	{
		int col = m_spreadsheet->indexOfChild<Column>(col_ptr);
		for (int row=first; row<=last; row++)
			if (isCellSelected(row, col))
				switch (col_ptr->columnMode()) {
					case SciDAVis::Numeric:
						col_ptr->setValueAt(row, double(qrand())/double(RAND_MAX));
						break;
					case SciDAVis::DateTime:
					case SciDAVis::Month:
					case SciDAVis::Day:
						{
							QDate date(1,1,1);
							QTime time(0,0,0,0);
							int days = (int)( (double)date.daysTo(QDate(2999,12,31)) * double(qrand())/double(RAND_MAX) );
							qint64 msecs = (qint64)(double(qrand())/double(RAND_MAX) * 1000.0 * 60.0 * 60.0 * 24.0);
							col_ptr->setDateTimeAt(row, QDateTime(date.addDays(days), time.addMSecs(msecs)));
							break;
						}
					case SciDAVis::Text:
						col_ptr->setTextAt(row, QString::number(double(qrand())/double(RAND_MAX)));
						break;
				}
	}
	m_spreadsheet->endMacro();
	RESET_CURSOR;
}

void SpreadsheetView::insertEmptyColumns()
{
	int first = firstSelectedColumn();
	int last = lastSelectedColumn();
	if ( first < 0 ) return;
	int count, current = first;

	WAIT_CURSOR;
	m_spreadsheet->beginMacro(QObject::tr("%1: insert empty column(s)").arg(m_spreadsheet->name()));
	while( current <= last )
	{
		current = first+1;
		while( current <= last && isColumnSelected(current) ) current++;
		count = current-first;
		for (int i=0; i<count; i++)
		{
			Column * new_col = new Column(QString::number(i+1), SciDAVis::Numeric);
			new_col->setPlotDesignation(SciDAVis::Y);
			m_spreadsheet->addChild(new_col);
		}
		current += count;
		last += count;
		while( current <= last && !isColumnSelected(current) ) current++;
		first = current;
	}
	m_spreadsheet->endMacro();
	RESET_CURSOR;
}

void SpreadsheetView::removeSelectedColumns()
{
	WAIT_CURSOR;
	m_spreadsheet->beginMacro(QObject::tr("%1: remove selected column(s)").arg(m_spreadsheet->name()));

	QList< Column* > list = selectedColumns();
	foreach(Column* col, list)
		col->remove();

	m_spreadsheet->endMacro();
	RESET_CURSOR;
}

void SpreadsheetView::clearSelectedColumns()
{
	WAIT_CURSOR;
	m_spreadsheet->beginMacro(QObject::tr("%1: clear selected column(s)").arg(m_spreadsheet->name()));

	QList< Column* > list = selectedColumns();
	if (formulaModeActive())
	{
		foreach(Column* ptr, list)
			ptr->clearFormulas();
	}
	else
	{
		foreach(Column* ptr, list)
			ptr->clear();
	}

	m_spreadsheet->endMacro();
	RESET_CURSOR;
}

void SpreadsheetView::setSelectionAs(SciDAVis::PlotDesignation pd) {
	WAIT_CURSOR;
	m_spreadsheet->beginMacro(QObject::tr("%1: set plot designation(s)").arg(m_spreadsheet->name()));

	QList< Column* > list = selectedColumns();
	foreach(Column* ptr, list)
		ptr->setPlotDesignation(pd);

	m_spreadsheet->endMacro();
	RESET_CURSOR;
}

void SpreadsheetView::setSelectedColumnsAsX() {
	setSelectionAs(SciDAVis::X);
}

void SpreadsheetView::setSelectedColumnsAsY() {
	setSelectionAs(SciDAVis::Y);
}

void SpreadsheetView::setSelectedColumnsAsZ() {
	setSelectionAs(SciDAVis::Z);
}

void SpreadsheetView::setSelectedColumnsAsYError() {
	setSelectionAs(SciDAVis::yErr);
}

void SpreadsheetView::setSelectedColumnsAsXError() {
	setSelectionAs(SciDAVis::xErr);
}

void SpreadsheetView::setSelectedColumnsAsNone() {
	setSelectionAs(SciDAVis::noDesignation);
}

void SpreadsheetView::normalizeSelectedColumns() {
	WAIT_CURSOR;
	m_spreadsheet->beginMacro(QObject::tr("%1: normalize column(s)").arg(m_spreadsheet->name()));
	QList< Column* > cols = selectedColumns();
	foreach(Column * col, cols)
	{
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

void SpreadsheetView::normalizeSelection()
{
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

void SpreadsheetView::insertEmptyRows()
{
	int first = firstSelectedRow();
	int last = lastSelectedRow();
	int count, current = first;

	if ( first < 0 ) return;

	WAIT_CURSOR;
	m_spreadsheet->beginMacro(QObject::tr("%1: insert empty rows(s)").arg(m_spreadsheet->name()));
	while( current <= last )
	{
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

void SpreadsheetView::removeSelectedRows()
{
	int first = firstSelectedRow();
	int last = lastSelectedRow();
	if ( first < 0 ) return;

	WAIT_CURSOR;
	m_spreadsheet->beginMacro(QObject::tr("%1: remove selected rows(s)").arg(m_spreadsheet->name()));
	for (int i=last; i>=first; i--)
		if (isRowSelected(i, false)) m_spreadsheet->removeRows(i, 1);
	m_spreadsheet->endMacro();
	RESET_CURSOR;
}

void SpreadsheetView::clearSelectedRows()
{
	int first = firstSelectedRow();
	int last = lastSelectedRow();
	if ( first < 0 ) return;

	WAIT_CURSOR;
	m_spreadsheet->beginMacro(QObject::tr("%1: clear selected rows(s)").arg(m_spreadsheet->name()));
	QList<Column*> list = selectedColumns();
	foreach(Column * col_ptr, list)
	{
		if (formulaModeActive())
		{
			for (int row=last; row>=first; row--)
				if (isRowSelected(row, false))
				{
					col_ptr->setFormula(row, "");
				}
		}
		else
		{
			for (int row=last; row>=first; row--)
				if (isRowSelected(row, false))
				{
					if (row == (col_ptr->rowCount()-1) )
						col_ptr->removeRows(row,1);
					else if (row < col_ptr->rowCount())
						col_ptr->asStringColumn()->setTextAt(row, "");
				}
		}
	}
	m_spreadsheet->endMacro();
	RESET_CURSOR;
}

void SpreadsheetView::clearSelectedCells()
{
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
					if (row == (col_ptr->rowCount()-1) )
						col_ptr->removeRows(row,1);
					else if (row < col_ptr->rowCount())
						col_ptr->asStringColumn()->setTextAt(row, "");
				}
		}
	}
	m_spreadsheet->endMacro();
	RESET_CURSOR;
}

void SpreadsheetView::goToCell()
{
	bool ok;

	int col = QInputDialog::getInteger(0, tr("Go to Cell"), tr("Enter column"),
			1, 1, m_spreadsheet->columnCount(), 1, &ok);
	if ( !ok ) return;

	int row = QInputDialog::getInteger(0, tr("Go to Cell"), tr("Enter row"),
			1, 1, m_spreadsheet->rowCount(), 1, &ok);
	if ( !ok ) return;

	goToCell(row-1, col-1);
}

void SpreadsheetView::goToCell(int row, int col)
{
	QModelIndex index = m_model->index(row, col);
	m_view_widget->scrollTo(index);
	m_view_widget->setCurrentIndex(index);
}

void SpreadsheetView::dimensionsDialog() {
	// TODO: Design a nicer dialog for this
	bool ok;

	int cols = QInputDialog::getInteger(0, tr("Set Spreadsheet Dimensions"), tr("Enter number of columns"),
			m_spreadsheet->columnCount(), 1, 1e9, 1, &ok);
	if ( !ok ) return;

	int rows = QInputDialog::getInteger(0, tr("Set Spreadsheet Dimensions"), tr("Enter number of rows"),
			m_spreadsheet->rowCount(), 1, 1e9, 1, &ok);
	if ( !ok ) return;

	m_spreadsheet->setColumnCount(cols);
	m_spreadsheet->setRowCount(rows);
}

void SpreadsheetView::addColumns() {
	m_spreadsheet->appendColumns(selectedColumnCount(false));
}

void SpreadsheetView::addRows() {
	m_spreadsheet->appendColumns(selectedRowCount(false));
}

int SpreadsheetView::defaultColumnWidth() {
	return Column::global("default_width").toInt();
}

void SpreadsheetView::setDefaultColumnWidth(int width) {
	Column::setGlobal("default_width", width);
}

void SpreadsheetView::setDefaultCommentVisibility(bool visible) {
	Spreadsheet::setGlobal("default_comment_visibility", visible);
}

bool SpreadsheetView::defaultCommentVisibility() {
	return Spreadsheet::global("default_comment_visibility").toBool();
}

void SpreadsheetView::handleHorizontalSectionResized(int logicalIndex, int oldSize, int newSize)
{
	Q_UNUSED(oldSize);
	static bool inside = false;
	m_spreadsheet->column(logicalIndex)->setWidth(newSize);
	if (inside) return;
	inside = true;

	int cols = m_spreadsheet->columnCount();
	for (int i=0; i<cols; i++)
		if (isColumnSelected(i, true))
			m_view_widget->horizontalHeader()->resizeSection(i, newSize);

	inside = false;
}

