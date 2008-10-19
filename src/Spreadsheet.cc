/***************************************************************************
    File                 : Spreadsheet.cc
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Stefan Gerlach (stefan.gerlach*uni-konstanz.de)
	Copyright            : (C) 2008 by Alexander Semke (alexander.semke*web.de)
    Copyright            : (C) 2007-2008 Tilman Benkert (thzs*gmx.net)
                           (replace * with @ in the email addresses) 
    Description          : spreadsheet class
                           
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

#include "Spreadsheet.h"
#include "MainWin.h"
#include "kdefrontend/ColumnDialog.h"
#include "pixmaps/pixmap.h"
#include "elements/Point3D.h"
#include "kdefrontend/ExportDialog.h"
#include "kdefrontend/FunctionPlotDialog.h"

#include "table/Table.h"
#include "table/TableModel.h"
#include "table/tablecommands.h"
#include "lib/macros.h"
#include "core/Project.h"
#include "core/column/Column.h"
#include "core/AbstractFilter.h"
#include "core/datatypes/SimpleCopyThroughFilter.h"
#include "core/datatypes/Double2StringFilter.h"
#include "core/datatypes/String2DoubleFilter.h"
#include "core/datatypes/DateTime2StringFilter.h"
#include "core/datatypes/String2DateTimeFilter.h"

Spreadsheet::Spreadsheet(Table *table)
 :  m_table(table)
{
	kDebug()<<"Spreadsheet()"<<endl;
	m_type = SPREADSHEET;
	m_model = new TableModel(table);
	init();
}

Spreadsheet::~Spreadsheet() 
{
	delete m_model;
}

void Spreadsheet::init()
{
//	createActions();

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

	rereadSectionSizes();
	
	// keyboard shortcuts
	QShortcut * sel_all = new QShortcut(QKeySequence(tr("Ctrl+A", "Table: select all")), m_view_widget);
	connect(sel_all, SIGNAL(activated()), m_view_widget, SLOT(selectAll()));

	connect(m_table, SIGNAL(sectionSizesChanged()), this, SLOT(rereadSectionSizes()));
}

void Spreadsheet::rereadSectionSizes()
{
	QHeaderView *h_header = m_view_widget->horizontalHeader();
	disconnect(h_header, SIGNAL(sectionResized(int, int, int)), this, SLOT(handleHorizontalSectionResized(int, int, int)));

	int cols = m_table->columnCount();
	for (int i=0; i<cols; i++)
		h_header->resizeSection(i, m_table->columnWidth(i));
		
	connect(h_header, SIGNAL(sectionResized(int, int, int)), this, SLOT(handleHorizontalSectionResized(int, int, int)));
}

void Spreadsheet::selectAll()
{
	m_view_widget->selectAll();
}

void Spreadsheet::deselectAll()
{
	m_view_widget->clearSelection();
}

void Spreadsheet::setColumnWidth(int col, int width) 
{ 
	m_view_widget->horizontalHeader()->resizeSection(col, width);
}

int Spreadsheet::columnWidth(int col) const 
{ 
	return m_view_widget->horizontalHeader()->sectionSize(col);
}

void Spreadsheet::contextMenuEvent(QContextMenuEvent *) {
	QMenu *menu = new QMenu(this);
	Menu(menu);
	menu->exec(QCursor::pos());
}

void Spreadsheet::Menu(QMenu *menu) {
	kDebug()<<endl;
	menu->clear();
	// sum=1, max=1

	KAction *action = new KAction(KIcon(QIcon(plot2d_xpm)),i18n("Plot"),this);
	connect(action, SIGNAL(triggered()),SLOT(plot()));
	menu->addAction(action);
	action = new KAction(KIcon("document-export"),i18n("Export"),this);
	connect(action, SIGNAL(triggered()),SLOT(exportData()));
	menu->addAction(action);
	action = new KAction(KIcon("edit-rename"),i18n("Edit Function"),this);
	connect(action, SIGNAL(triggered()),SLOT(editFunction()));
	menu->addAction(action);
	menu->addSeparator();

	action = new KAction(KIcon("select"),i18n("Column properties"),this);
	menu->addAction(action);
	connect(action, SIGNAL(triggered()),SLOT(setProperties()));
	menu->addSeparator();

	action = new KAction(KIcon("select"),i18n("Set title"),this);
	menu->addAction(action);
	connect(action, SIGNAL(triggered()),SLOT(setTitle()));
	action = new KAction(KIcon("select"),i18n("Set row count"),this);
	menu->addAction(action);
	connect(action, SIGNAL(triggered()),SLOT(setRowNumber()));
	action = new KAction(KIcon("draw-freehand"),i18n("Set notes"),this);
	menu->addAction(action);
	connect(action, SIGNAL(triggered()),SLOT(setNotes()));
	menu->addSeparator();

	action = new KAction(KIcon("edit-cut"),i18n("Cut"),this);
	menu->addAction(action);
	action->setEnabled(false);
	action = new KAction(KIcon("edit-copy"),i18n("Copy"),this);
	menu->addAction(action);
	action->setEnabled(false);
	action = new KAction(KIcon("edit-paste"),i18n("Paste"),this);
	menu->addAction(action);
	action->setEnabled(false);
	action = new KAction(KIcon("edit-clear"),i18n("Clear"),this);
	menu->addAction(action);
	action->setEnabled(false);
	action = new KAction(KIcon("edit-select-all"),i18n("Select"),this);
	menu->addAction(action);
	action->setEnabled(false);
	// .. all, nothing, invert
	menu->addSeparator();

	action = new KAction(KIcon(QIcon(plot2d_xpm)),i18n("Set column values"),this);
	connect(action, SIGNAL(triggered()),SLOT(setColumnValues()));
	menu->addAction(action);
	menu->addSeparator();

	action = new KAction(KIcon(QIcon(plot2d_xpm)),i18n("Fill with"),this);
	action->setEnabled(false);
	// row number, random value
	menu->addAction(action);
	action = new KAction(KIcon("transform-rotate"),i18n("Convert"),this);
	action->setEnabled(false);
	// ...
	menu->addAction(action);
	action = new KAction(KIcon(QIcon(plot2d_xpm)),i18n("Edit with"),this);
	action->setEnabled(false);
	menu->addAction(action);
	// ...
	menu->addSeparator();

	action = new KAction(KIcon("list-add"),i18n("Add column"),this);
	menu->addAction(action);
	connect(action, SIGNAL(triggered()),SLOT(addColumn()));
	action = new KAction(KIcon("list-remove"),i18n("Delete selected column"),this);
	connect(action, SIGNAL(triggered()),SLOT(deleteSelectedColumns()));
	menu->addAction(action);
	action = new KAction(KIcon("list-remove"),i18n("Delete selected row"),this);
	connect(action, SIGNAL(triggered()),SLOT(deleteSelectedRows()));
	menu->addAction(action);
	action = new KAction(KIcon(QIcon(plot2d_xpm)),i18n("Masking"),this);
	action->setEnabled(false);
	menu->addAction(action);
	// ...
	menu->addSeparator();

	action = new KAction(KIcon(QIcon(plot2d_xpm)),i18n("Normalize"),this);
	action->setEnabled(false);
	// sum=1, max=1
	menu->addAction(action);
	action = new KAction(KIcon(QIcon(plot2d_xpm)),i18n("Sort"),this);
	action->setEnabled(false);
	// ascending, descending
	menu->addAction(action);
	action = new KAction(KIcon(QIcon(plot2d_xpm)),i18n("Statistics on column"),this);
	action->setEnabled(false);
	menu->addAction(action);
	action = new KAction(KIcon(QIcon(plot2d_xpm)),i18n("Statistics on row"),this);
	action->setEnabled(false);
	menu->addAction(action);
	menu->addSeparator();

#if 0 // TODO: Do you really need this action inside Spreadsheet? If yes, I recommend a signal to avoid depending on MainWin - Tilman
	action = new KAction(KIcon("insert-table"),i18n("New spreadsheet"),this);
	action->setShortcut(Qt::CTRL+Qt::Key_Equal);
	menu->addAction(action);
	connect(action, SIGNAL(triggered()),mw, SLOT(newSpreadsheet()));
#endif
}


void Spreadsheet::setTitle(QString title) {
	kDebug()<<"title ="<<title<<endl;
	bool ok=true;
	if(title.isEmpty())
		title = KInputDialog::getText("LabPlot", i18n("Spreadsheet title : "), m_table->name(), &ok);

	if(ok && !title.isEmpty()) {
		m_table->setName(title);
	}
}

void Spreadsheet::setRowNumber(int row) {
	kDebug()<<"row ="<<row<<endl;
	bool ok=true;
	if(row<=0)
		row = KInputDialog::getInteger("LabPlot", i18n("Row count : "), rowCount(), 1, INT_MAX, 1, 10, &ok);

	if(ok && row>0)
		m_table->setRowCount(row);
}

QString Spreadsheet::text(int row, int col) const {
//	kDebug()<<"Spreadsheet::text("<<row<<col<<")"<<endl;
	QModelIndex index=m_view_widget->model()->index(row,col);
	return m_view_widget->model()->data(index).toString();
}

void Spreadsheet::setText(int row, int col, QString text) {
	QModelIndex index=m_view_widget->model()->index(row,col);
	m_view_widget->model()->setData(index,text);
}

void Spreadsheet::setNotes(QString t) {
	kDebug()<<endl;
	bool ok=true;
	if(t.isEmpty())
		t = KInputDialog::getMultiLineText("LabPlot", i18n("Spreadsheet notes : "), 
			Notes(), &ok);
	if(!ok) return;

	m_table->setComment(t);
}

QString Spreadsheet::columnHeader(int col) const {
	if(col<0) col=0;
	return m_view_widget->model()->headerData(col,Qt::Horizontal).toString();
}

void Spreadsheet::setProperties(QString label, int type, int format) {
#if 0 // TODO: I strongly recommend not to store such information in a string, see SciDAVis' TableModel for a better way to generate the header - Tilman
	if(label.isEmpty())
		(new ColumnDialog(this,this))->show();
	else
		// NEW : m_table->column(currentColumn())->setName(label);
		setColumnHeader(currentColumn(),label + ' ' + '{'+columnformatitems[format]+'}' + ' ' + '['+columntypeitems[type]+']');
#endif
}

#if 0  // TODO: rewrite the TableModel to show the header in this way
void Spreadsheet::resetHeader(int from) {
	kDebug()<<endl;
	for (int col=from;col<columnCount();col++) {
		QString l;
		if(col==0)
			l=(QChar(col+65)+' '+'{'+columnformatitems[0]+'}'+' '+'['+columntypeitems[0]+']');
		else if(col<26)
			l=(QChar(col+65)+' '+'{'+columnformatitems[0]+'}'+' '+'['+columntypeitems[1]+']');
		else
			l=(QString(QChar(65+col/26-1)) +QString(QChar(65+col%26))+' '+'{'+columnformatitems[0]+'}'+' '+'['+columntypeitems[1]+']');
		setColumnHeader(col,l);
	}
}
#endif

QString Spreadsheet::columnName(int col) const {
	return m_table->column(col)->name();
}

void Spreadsheet::setColumnName(int col, QString name) {
	kDebug()<<"column ="<<col<<", name = "<<name<<endl;
	if(col<0) return;
	m_table->column(col)->setName(name);
}


QString Spreadsheet::columnType(int col) const {
#if 0 // TODO: see comment in setProperties
	QString header = columnHeader(col);
	header.remove(QRegExp(".*\\["));
	header.remove(QRegExp("\\].*"));
	return header;
#endif
	return QString();
}

void Spreadsheet::setColumnType(int col, QString type) {
#if 0 // TODO: see comment in setProperties
	kDebug()<<"column ="<<col<<",type = "<<type<<endl;
	if(col<0) return;
	QString label = columnHeader(col);
	label.replace(QRegExp(" \\[.+\\]"),QString(" ["+type+"]"));
	setColumnHeader(col, label);
#endif
}

QString Spreadsheet::columnFormat(int col) const {
#if 0 // TODO: see comment in setProperties
	QString header = columnHeader(col);
	header.remove(QRegExp(".*\\{"));
	header.remove(QRegExp("\\}.*"));
	return header;
#endif
}

void Spreadsheet::setColumnFormat(int col, QString format) {
#if 0 // TODO: see comment in setProperties
	kDebug()<<"column ="<<col<<",type = "<<format<<endl;
	if(col<0) return;
	QString label = columnHeader(col);
	label.replace(QRegExp(" \\{.+\\}"),QString(" {"+format+"}"));
	setColumnHeader(col, label);
#endif
}

int Spreadsheet::filledRows(int col) const {
	kDebug()<<"Spreadsheet::filledRows("<<col<<")"<<endl;
	if(col < 0 || col > columnCount())
		return -1;

	return m_table->column(col)->rowCount();
}

int Spreadsheet::currentColumn() const {
	QModelIndex index = m_view_widget->currentIndex();
	return index.column();
}

QList<int> Spreadsheet::currentColumns() const {
	QList<int> columns;
	QModelIndexList list = m_view_widget->selectionModel()->selectedColumns();
	for (int i=0; i<list.size(); i++)
		columns.append(list.at(i).column());
	return columns;
}

int Spreadsheet::currentRow() const {
	QModelIndex index = m_view_widget->currentIndex();
	return index.row();
}

QList<int> Spreadsheet::currentRows() const {
	QList<int> rows;
	QModelIndexList list = m_view_widget->selectionModel()->selectedRows();
	for (int i=0; i<list.size(); i++)
		rows.append(list.at(i).row());
	return rows;
}

void Spreadsheet::deleteSelectedColumns() {
	removeSelectedColumns();
}

void Spreadsheet::deleteSelectedRows() {
	removeSelectedRows();
}

void Spreadsheet::addSet(Set* s) {
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

void Spreadsheet::displaySet(){
//TODO clear the content of the table!!!

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

void Spreadsheet::plot() {
	kDebug()<<"not implemented yet!"<<endl;
	// TODO : use SpreadsheetPlotDialog
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

void Spreadsheet::exportData() {
	(new ExportDialog(this))->exec();
}

void Spreadsheet::setColumnValues() {
	kDebug()<<"not implemented yet!"<<endl;
	// TODO : column values dialog
}

void Spreadsheet::editFunction(){
	Plot::PlotType type;
	//TODO rethink/reimplement this approach
	if (m_set->type()==Set::SET2D)
		type=Plot::PLOT2D;
	else if (m_set->type()==Set::SET3D)
		type=Plot::PLOT3D;

	// TODO: As I stated in other places: I would be better for Spreadsheet not depend on MainWin at all.
	// But if you really need it, this is the "SciDAVis way" of getting a pointer to the main window: - Tilman
	if (m_table->project() == NULL) return;
	MainWin * mw = static_cast<MainWin *>(m_table->project()->view());
	FunctionPlotDialog* dlg=new FunctionPlotDialog(mw, type);
	dlg->setSet(m_set);
	if (dlg->exec())
		this->displaySet(); //TODO trigger update only if data set was changed
}

int Spreadsheet::selectedColumnCount(bool full)
{
	int count = 0;
	int cols = m_table->columnCount();
	for (int i=0; i<cols; i++)
		if (isColumnSelected(i, full)) count++;
	return count;
}

int Spreadsheet::selectedColumnCount(SciDAVis::PlotDesignation pd)
{
	int count = 0;
	int cols = m_table->columnCount();
	for (int i=0; i<cols; i++)
		if ( isColumnSelected(i, false) && (m_table->column(i)->plotDesignation() == pd) ) count++;

	return count;
}

bool Spreadsheet::isColumnSelected(int col, bool full)
{
	if (full)
		return m_view_widget->selectionModel()->isColumnSelected(col, QModelIndex());
	else
		return m_view_widget->selectionModel()->columnIntersectsSelection(col, QModelIndex());
}

QList<Column*> Spreadsheet::selectedColumns(bool full)
{
	QList<Column*> list;
	int cols = m_table->columnCount();
	for (int i=0; i<cols; i++)
		if (isColumnSelected(i, full)) list << m_table->column(i);

	return list;
}

int Spreadsheet::selectedRowCount(bool full)
{
	int count = 0;
	int rows = m_table->rowCount();
	for (int i=0; i<rows; i++)
		if (isRowSelected(i, full)) count++;
	return count;
}

bool Spreadsheet::isRowSelected(int row, bool full)
{
	if (full)
		return m_view_widget->selectionModel()->isRowSelected(row, QModelIndex());
	else
		return m_view_widget->selectionModel()->rowIntersectsSelection(row, QModelIndex());
}

int Spreadsheet::firstSelectedColumn(bool full)
{
	int cols = m_table->columnCount();
	for (int i=0; i<cols; i++)
	{
		if (isColumnSelected(i, full))
			return i;
	}
	return -1;
}

int Spreadsheet::lastSelectedColumn(bool full)
{
	int cols = m_table->columnCount();
	for (int i=cols-1; i>=0; i--)
		if (isColumnSelected(i, full)) return i;

	return -2;
}

int Spreadsheet::firstSelectedRow(bool full)
{
	int rows = m_table->rowCount();
	for (int i=0; i<rows; i++)
	{
		if (isRowSelected(i, full))
			return i;
	}
	return -1;
}

int Spreadsheet::lastSelectedRow(bool full)
{
	int rows = m_table->rowCount();
	for (int i=rows-1; i>=0; i--)
		if (isRowSelected(i, full)) return i;

	return -2;
}

bool Spreadsheet::isCellSelected(int row, int col)
{
	if (row < 0 || col < 0 || row >= m_table->rowCount() || col >= m_table->columnCount()) return false;

	return m_view_widget->selectionModel()->isSelected(m_model->index(row, col));
}

void Spreadsheet::setCellSelected(int row, int col, bool select)
{
	 m_view_widget->selectionModel()->select(m_model->index(row, col), 
			 select ? QItemSelectionModel::Select : QItemSelectionModel::Deselect);
}

void Spreadsheet::setCellsSelected(int first_row, int first_col, int last_row, int last_col, bool select)
{
	QModelIndex top_left = m_model->index(first_row, first_col);
	QModelIndex bottom_right = m_model->index(last_row, last_col);
	m_view_widget->selectionModel()->select(QItemSelection(top_left, bottom_right), 
			select ? QItemSelectionModel::SelectCurrent : QItemSelectionModel::Deselect);
}

void Spreadsheet::getCurrentCell(int * row, int * col)
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

bool Spreadsheet::formulaModeActive() const 
{ 
	return m_model->formulaModeActive(); 
}

void Spreadsheet::activateFormulaMode(bool on) 
{ 
	m_model->activateFormulaMode(on); 
}

void Spreadsheet::goToNextColumn()
{
	if (m_table->columnCount() == 0) return;

	QModelIndex idx = m_view_widget->currentIndex();
	int col = idx.column()+1;
    if (col >= m_table->columnCount())
		col = 0;
	m_view_widget->setCurrentIndex(idx.sibling(idx.row(), col));
}

void Spreadsheet::goToPreviousColumn()
{
	if (m_table->columnCount() == 0) return;

	QModelIndex idx = m_view_widget->currentIndex();
	int col = idx.column()-1;
    if (col < 0)
		col = m_table->columnCount()-1;
	m_view_widget->setCurrentIndex(idx.sibling(idx.row(), col));
}

void Spreadsheet::cutSelection()
{
	int first = firstSelectedRow();
	if ( first < 0 ) return;

	WAIT_CURSOR;
	m_table->beginMacro(tr("%1: cut selected cell(s)").arg(m_table->name()));
	copySelection();
	clearSelectedCells();
	m_table->endMacro();
	RESET_CURSOR;
}

void Spreadsheet::copySelection()
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
			Column *col_ptr = m_table->column(first_col + c);
			if (isCellSelected(first_row + r, first_col + c))
			{
				if (formulaModeActive())
				{
					output_str += col_ptr->formula(first_row + r);
				}
				else if (col_ptr->dataType() == SciDAVis::TypeDouble)
				{
					Double2StringFilter * out_fltr = static_cast<Double2StringFilter *>(col_ptr->outputFilter());
					output_str += QLocale().toString(col_ptr->valueAt(first_row + r), 
							out_fltr->numericFormat(), 16); // copy with max. precision
				}
				else
				{
					output_str += m_table->text(first_row + r, first_col + c);
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

void Spreadsheet::pasteIntoSelection()
{
	if (m_table->columnCount() < 1 || m_table->rowCount() < 1) return;

	WAIT_CURSOR;
	m_table->beginMacro(tr("%1: paste from clipboard").arg(m_table->name()));
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
			// resize the table if necessary
			if (last_col >= m_table->columnCount())
			{
				QList<Column*> cols;
				for (int i=0; i<last_col+1-m_table->columnCount(); i++)
				{
					Column * new_col = new Column(QString::number(i+1), SciDAVis::Text);
					new_col->setPlotDesignation(SciDAVis::Y);
					cols << new_col;
				}
				m_table->appendColumns(cols);
			}
			if (last_row >= m_table->rowCount())
				m_table->appendRows(last_row+1-m_table->rowCount());
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
					Column * col_ptr = m_table->column(first_col + c);
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
	m_table->endMacro();
	RESET_CURSOR;
}

void Spreadsheet::maskSelection()
{
	int first = firstSelectedRow();
	int last = lastSelectedRow();
	if ( first < 0 ) return;

	WAIT_CURSOR;
	m_table->beginMacro(tr("%1: mask selected cell(s)").arg(m_table->name()));
	QList<Column*> list = selectedColumns();
	foreach(Column * col_ptr, list)
	{
		int col = m_table->columnIndex(col_ptr);
		for (int row=first; row<=last; row++)
			if (isCellSelected(row, col)) col_ptr->setMasked(row);  
	}
	m_table->endMacro();
	RESET_CURSOR;
}

void Spreadsheet::unmaskSelection()
{
	int first = firstSelectedRow();
	int last = lastSelectedRow();
	if ( first < 0 ) return;

	WAIT_CURSOR;
	m_table->beginMacro(tr("%1: unmask selected cell(s)").arg(m_table->name()));
	QList<Column*> list = selectedColumns();
	foreach(Column * col_ptr, list)
	{
		int col = m_table->columnIndex(col_ptr);
		for (int row=first; row<=last; row++)
			if (isCellSelected(row, col)) col_ptr->setMasked(row, false);  
	}
	m_table->endMacro();
	RESET_CURSOR;
}

void Spreadsheet::fillSelectedCellsWithRowNumbers()
{
	if (selectedColumnCount() < 1) return;
	int first = firstSelectedRow();
	int last = lastSelectedRow();
	if ( first < 0 ) return;
	
	WAIT_CURSOR;
	m_table->beginMacro(tr("%1: fill cells with row numbers").arg(m_table->name()));
	QList<Column*> list = selectedColumns();
	foreach(Column * col_ptr, list)
	{
		int col = m_table->columnIndex(col_ptr);
		for (int row=first; row<=last; row++)
			if (isCellSelected(row, col)) 
				col_ptr->asStringColumn()->setTextAt(row, QString::number(row+1));
	}
	m_table->endMacro();
	RESET_CURSOR;
}

void Spreadsheet::fillSelectedCellsWithRandomNumbers()
{
	if (selectedColumnCount() < 1) return;
	int first = firstSelectedRow();
	int last = lastSelectedRow();
	if ( first < 0 ) return;
	
	WAIT_CURSOR;
	m_table->beginMacro(tr("%1: fill cells with random values").arg(m_table->name()));
	qsrand(QTime::currentTime().msec());
	QList<Column*> list = selectedColumns();
	foreach(Column * col_ptr, list)
	{
		int col = m_table->columnIndex(col_ptr);
		for (int row=first; row<=last; row++)
			if (isCellSelected(row, col)) 
			{
				if (col_ptr->columnMode() == SciDAVis::Numeric)
					col_ptr->setValueAt(row, double(qrand())/double(RAND_MAX));
				else if (col_ptr->dataType() == SciDAVis::TypeQDateTime)
				{
					QDate date(1,1,1);
					QTime time(0,0,0,0);
					int days = (int)( (double)date.daysTo(QDate(2999,12,31)) * double(qrand())/double(RAND_MAX) );
					qint64 msecs = (qint64)(double(qrand())/double(RAND_MAX) * 1000.0 * 60.0 * 60.0 * 24.0);
					col_ptr->setDateTimeAt(row, QDateTime(date.addDays(days), time.addMSecs(msecs)));
				}
				else
					col_ptr->setTextAt(row, QString::number(double(qrand())/double(RAND_MAX)));
			}
	}
	m_table->endMacro();
	RESET_CURSOR;
}

void Spreadsheet::insertEmptyColumns()
{
	int first = firstSelectedColumn();
	int last = lastSelectedColumn();
	if ( first < 0 ) return;
	int count, current = first;
	QList<Column*> cols;

	WAIT_CURSOR;
	m_table->beginMacro(QObject::tr("%1: insert empty column(s)").arg(m_table->name()));
	while( current <= last )
	{
		current = first+1;
		while( current <= last && isColumnSelected(current) ) current++;
		count = current-first;
		for (int i=0; i<count; i++)
		{
			Column * new_col = new Column(QString::number(i+1), SciDAVis::Numeric);
			new_col->setPlotDesignation(SciDAVis::Y);
			cols << new_col;
		}
		m_table->insertColumns(first, cols);
		cols.clear();
		current += count;
		last += count;
		while( current <= last && !isColumnSelected(current) ) current++;
		first = current;
	}
	m_table->endMacro();
	RESET_CURSOR;
}

void Spreadsheet::removeSelectedColumns()
{
	WAIT_CURSOR;
	m_table->beginMacro(QObject::tr("%1: remove selected column(s)").arg(m_table->name()));

	QList< Column* > list = selectedColumns();
	foreach(Column* ptr, list)
		m_table->removeColumn(ptr);

	m_table->endMacro();
	RESET_CURSOR;
}

void Spreadsheet::clearSelectedColumns()
{
	WAIT_CURSOR;
	m_table->beginMacro(QObject::tr("%1: clear selected column(s)").arg(m_table->name()));

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

	m_table->endMacro();
	RESET_CURSOR;
}

void Spreadsheet::setSelectionAs(SciDAVis::PlotDesignation pd)
{
	WAIT_CURSOR;
	m_table->beginMacro(QObject::tr("%1: set plot designation(s)").arg(m_table->name()));

	QList< Column* > list = selectedColumns();
	foreach(Column* ptr, list)
		ptr->setPlotDesignation(pd);

	m_table->endMacro();
	RESET_CURSOR;
}

void Spreadsheet::setSelectedColumnsAsX()
{
	setSelectionAs(SciDAVis::X);
}

void Spreadsheet::setSelectedColumnsAsY()
{
	setSelectionAs(SciDAVis::Y);
}

void Spreadsheet::setSelectedColumnsAsZ()
{
	setSelectionAs(SciDAVis::Z);
}

void Spreadsheet::setSelectedColumnsAsYError()
{
	setSelectionAs(SciDAVis::yErr);
}

void Spreadsheet::setSelectedColumnsAsXError()
{
	setSelectionAs(SciDAVis::xErr);
}

void Spreadsheet::setSelectedColumnsAsNone()
{
	setSelectionAs(SciDAVis::noDesignation);
}

void Spreadsheet::normalizeSelectedColumns()
{
	WAIT_CURSOR;
	m_table->beginMacro(QObject::tr("%1: normalize column(s)").arg(m_table->name()));
	QList< Column* > cols = selectedColumns();
	foreach(Column * col, cols)
	{
		if (col->dataType() == SciDAVis::TypeDouble)
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
	m_table->endMacro();
	RESET_CURSOR;
}

void Spreadsheet::normalizeSelection()
{
	WAIT_CURSOR;
	m_table->beginMacro(QObject::tr("%1: normalize selection").arg(m_table->name()));
	double max = 0.0;
	for (int col=firstSelectedColumn(); col<=lastSelectedColumn(); col++)
		if (m_table->column(col)->dataType() == SciDAVis::TypeDouble)
			for (int row=0; row<m_table->rowCount(); row++)
			{
				if (isCellSelected(row, col) && m_table->column(col)->valueAt(row) > max)
					max = m_table->column(col)->valueAt(row);
			}

	if (max != 0.0) // avoid division by zero
	{
		for (int col=firstSelectedColumn(); col<=lastSelectedColumn(); col++)
			if (m_table->column(col)->dataType() == SciDAVis::TypeDouble)
				for (int row=0; row<m_table->rowCount(); row++)
				{
					if (isCellSelected(row, col))
						m_table->column(col)->setValueAt(row, m_table->column(col)->valueAt(row) / max);
				}
	}
	m_table->endMacro();
	RESET_CURSOR;
}

void Spreadsheet::insertEmptyRows()
{
	int first = firstSelectedRow();
	int last = lastSelectedRow();
	int count, current = first;

	if ( first < 0 ) return;

	WAIT_CURSOR;
	m_table->beginMacro(QObject::tr("%1: insert empty rows(s)").arg(m_table->name()));
	while( current <= last )
	{
		current = first+1;
		while( current <= last && isRowSelected(current) ) current++;
		count = current-first;
		m_table->insertRows(first, count);
		current += count;
		last += count;
		while( current <= last && !isRowSelected(current) ) current++;
		first = current;
	}
	m_table->endMacro();
	RESET_CURSOR;
}

void Spreadsheet::removeSelectedRows()
{
	int first = firstSelectedRow();
	int last = lastSelectedRow();
	if ( first < 0 ) return;

	WAIT_CURSOR;
	m_table->beginMacro(QObject::tr("%1: remove selected rows(s)").arg(m_table->name()));
	for (int i=last; i>=first; i--)
		if (isRowSelected(i, false)) m_table->removeRows(i, 1);
	m_table->endMacro();
	RESET_CURSOR;
}

void Spreadsheet::clearSelectedRows()
{
	int first = firstSelectedRow();
	int last = lastSelectedRow();
	if ( first < 0 ) return;

	WAIT_CURSOR;
	m_table->beginMacro(QObject::tr("%1: clear selected rows(s)").arg(m_table->name()));
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
	m_table->endMacro();
	RESET_CURSOR;
}

void Spreadsheet::clearSelectedCells()
{
	int first = firstSelectedRow();
	int last = lastSelectedRow();
	if ( first < 0 ) return;

	WAIT_CURSOR;
	m_table->beginMacro(tr("%1: clear selected cell(s)").arg(m_table->name()));
	QList<Column*> list = selectedColumns();
	foreach(Column * col_ptr, list)
	{
		if (formulaModeActive())
		{
			int col = m_table->columnIndex(col_ptr);
			for (int row=last; row>=first; row--)
				if (isCellSelected(row, col))
				{
					col_ptr->setFormula(row, "");  
				}
		}
		else
		{
			int col = m_table->columnIndex(col_ptr);
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
	m_table->endMacro();
	RESET_CURSOR;
}

void Spreadsheet::goToCell()
{
	bool ok;

	int col = QInputDialog::getInteger(0, tr("Go to Cell"), tr("Enter column"),
			1, 1, m_table->columnCount(), 1, &ok);
	if ( !ok ) return;

	int row = QInputDialog::getInteger(0, tr("Go to Cell"), tr("Enter row"),
			1, 1, m_table->rowCount(), 1, &ok);
	if ( !ok ) return;

	goToCell(row-1, col-1);
}

void Spreadsheet::goToCell(int row, int col)
{
	QModelIndex index = m_model->index(row, col);
	m_view_widget->scrollTo(index);
	m_view_widget->setCurrentIndex(index);
}

void Spreadsheet::dimensionsDialog()
{
	// TODO: Design a nicer dialog for this
	bool ok;

	int cols = QInputDialog::getInteger(0, tr("Set Table Dimensions"), tr("Enter number of columns"),
			m_table->columnCount(), 1, 1e9, 1, &ok);
	if ( !ok ) return;

	int rows = QInputDialog::getInteger(0, tr("Set Table Dimensions"), tr("Enter number of rows"),
			m_table->rowCount(), 1, 1e9, 1, &ok);
	if ( !ok ) return;
	
	m_table->setColumnCount(cols);
	m_table->setRowCount(rows);
}

void Spreadsheet::addColumns()
{
	m_table->addColumns(selectedColumnCount(false));
}

void Spreadsheet::addRows()
{
	m_table->addColumns(selectedRowCount(false));
}

int Spreadsheet::defaultColumnWidth() 
{ 
	return Table::global("default_column_width").toInt(); 
}

void Spreadsheet::setDefaultColumnWidth(int width) 
{ 
	Table::setGlobal("default_column_width", width); 
}

void Spreadsheet::setDefaultCommentVisibility(bool visible) 
{ 
	Table::setGlobal("default_comment_visibility", visible); 
}

bool Spreadsheet::defaultCommentVisibility() 
{ 
	return Table::global("default_comment_visibility").toBool(); 
}

void Spreadsheet::handleHorizontalSectionResized(int logicalIndex, int oldSize, int newSize)
{	
	Q_UNUSED(oldSize);
	static bool inside = false;
	m_table->setColumnWidth(logicalIndex, newSize);
	if (inside) return;
	inside = true;

	int cols = m_table->columnCount();
	for (int i=0; i<cols; i++)
		if (isColumnSelected(i, true)) 
			m_view_widget->horizontalHeader()->resizeSection(i, newSize);

	inside = false;
}


