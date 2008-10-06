/***************************************************************************
    File                 : TableView.cpp
    Project              : SciDAVis
    Description          : View class for Table
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

#include "table/Table.h"
#include "table/TableView.h"
#include "table/TableModel.h"
#include "table/TableItemDelegate.h"
#include "table/tablecommands.h"
#include "table/TableDoubleHeaderView.h"
#include "lib/ActionManager.h"
#include "lib/macros.h"
#include "table/SortDialog.h"

#include "core/column/Column.h"
#include "core/AbstractFilter.h"
#include "core/datatypes/SimpleCopyThroughFilter.h"
#include "core/datatypes/Double2StringFilter.h"
#include "core/datatypes/String2DoubleFilter.h"
#include "core/datatypes/DateTime2StringFilter.h"
#include "core/datatypes/String2DateTimeFilter.h"

#include "ui_DimensionsDialog.h"

#include <QKeyEvent>
#include <QtDebug>
#include <QHeaderView>
#include <QRect>
#include <QPoint>
#include <QSize>
#include <QFontMetrics>
#include <QFont>
#include <QItemSelectionModel>
#include <QItemSelection>
#include <QShortcut>
#include <QModelIndex>
#include <QGridLayout>
#include <QScrollArea>
#include <QMenu>
#include <QClipboard>
#include <QMenu>
#include <QItemSelection>
#include <QContextMenuEvent>
#include <QModelIndex>
#include <QModelIndexList>
#include <QMapIterator>
#include <QDialog>
#include <QInputDialog>
#include <QMenuBar>

TableView::TableView(Table *table)
 : m_plot_menu(0), m_table(table)
{
	m_model = new TableModel(table);
	init();
}

TableView::~TableView() 
{
	delete m_model;
}

void TableView::init()
{
	createActions();

	m_main_layout = new QHBoxLayout(this);
	m_main_layout->setSpacing(0);
	m_main_layout->setContentsMargins(0, 0, 0, 0);
	
	m_view_widget = new TableViewWidget(this);
	m_view_widget->setModel(m_model);
	connect(m_view_widget, SIGNAL(advanceCell()), this, SLOT(advanceCell()));
	m_main_layout->addWidget(m_view_widget);
	
	m_horizontal_header = new TableDoubleHeaderView();
    m_horizontal_header->setClickable(true);
    m_horizontal_header->setHighlightSections(true);
	m_view_widget->setHorizontalHeader(m_horizontal_header);

	m_hide_button = new QToolButton();
	m_hide_button->setArrowType(Qt::RightArrow);
	m_hide_button->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding));
	m_hide_button->setCheckable(false);
	m_main_layout->addWidget(m_hide_button);
	connect(m_hide_button, SIGNAL(pressed()), this, SLOT(toggleControlTabBar()));
	m_control_tabs = new QWidget();
    ui.setupUi(m_control_tabs);
	m_main_layout->addWidget(m_control_tabs);

	m_delegate = new TableItemDelegate(m_view_widget);
	m_view_widget->setItemDelegate(m_delegate);
	
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
	// Remark: ResizeToContents works in Qt 4.2.3 but is broken in 4.3.0
	// Should be fixed in 4.3.1 though, see:
	// http://trolltech.com/developer/task-tracker/index_html?method=entry&id=165567
	v_header->setResizeMode(QHeaderView::ResizeToContents);
	v_header->setMovable(false);
	m_horizontal_header->setResizeMode(QHeaderView::Interactive);
	m_horizontal_header->setMovable(true);
	connect(m_horizontal_header, SIGNAL(sectionMoved(int,int,int)), this, SLOT(handleHorizontalSectionMoved(int,int,int)));
	connect(m_horizontal_header, SIGNAL(sectionDoubleClicked(int)), this, SLOT(handleHorizontalHeaderDoubleClicked(int)));
	
	m_horizontal_header->setDefaultSectionSize(defaultColumnWidth());

	v_header->installEventFilter(this);
	m_horizontal_header->installEventFilter(this);
	m_view_widget->installEventFilter(this);

	connect(m_model, SIGNAL(headerDataChanged(Qt::Orientation,int,int)), m_view_widget, 
		SLOT(updateHeaderGeometry(Qt::Orientation,int,int)) ); 
	connect(m_model, SIGNAL(headerDataChanged(Qt::Orientation,int,int)), this, 
		SLOT(handleHeaderDataChanged(Qt::Orientation,int,int)) ); 

	rereadSectionSizes();
	
	// keyboard shortcuts
	QShortcut * sel_all = new QShortcut(QKeySequence(tr("Ctrl+A", "Table: select all")), m_view_widget);
	connect(sel_all, SIGNAL(activated()), m_view_widget, SLOT(selectAll()));

	connect(ui.type_box, SIGNAL(currentIndexChanged(int)), this, SLOT(updateFormatBox()));
	connect(ui.format_box, SIGNAL(currentIndexChanged(int)), this, SLOT(updateTypeInfo()));
	connect(ui.digits_box, SIGNAL(valueChanged(int)), this, SLOT(updateTypeInfo()));
	connect(ui.previous_column_button, SIGNAL(clicked()), this, SLOT(goToPreviousColumn()));
	connect(ui.next_column_button, SIGNAL(clicked()), this, SLOT(goToNextColumn()));
	retranslateStrings();

	QItemSelectionModel * sel_model = m_view_widget->selectionModel();

	connect(sel_model, SIGNAL(currentColumnChanged(const QModelIndex&, const QModelIndex&)), 
		this, SLOT(currentColumnChanged(const QModelIndex&, const QModelIndex&)));
	connect(sel_model, SIGNAL(selectionChanged(const QItemSelection&,const QItemSelection&)),
		this, SLOT(selectionChanged(const QItemSelection&,const QItemSelection&)));
	connect(ui.button_set_description, SIGNAL(pressed()), 
		this, SLOT(applyDescription()));
	connect(ui.button_set_type, SIGNAL(pressed()),
		this, SLOT(applyType()));
	
	connectActions();
	showComments(defaultCommentVisibility());

	connect(m_table, SIGNAL(sectionSizesChanged()), this, SLOT(rereadSectionSizes()));
	connect(m_table, SIGNAL(requestProjectMenu(QMenu*,bool*)), this, SLOT(fillProjectMenu(QMenu*,bool*)));
	connect(m_table, SIGNAL(requestProjectContextMenu(QMenu*)), this, SLOT(createContextMenu(QMenu*)));
}

TableView::TableView()
{
	m_model = NULL;
	createActions();
}


void TableView::rereadSectionSizes()
{
	disconnect(m_horizontal_header, SIGNAL(sectionResized(int, int, int)), this, SLOT(handleHorizontalSectionResized(int, int, int)));

	int cols = m_table->columnCount();
	for (int i=0; i<cols; i++)
		m_horizontal_header->resizeSection(i, m_table->columnWidth(i));
		
	connect(m_horizontal_header, SIGNAL(sectionResized(int, int, int)), this, SLOT(handleHorizontalSectionResized(int, int, int)));
}

void TableView::setColumnWidth(int col, int width) 
{ 
	m_horizontal_header->resizeSection(col, width);
}

int TableView::columnWidth(int col) const 
{ 
	return m_horizontal_header->sectionSize(col);
}

void TableView::handleHorizontalSectionResized(int logicalIndex, int oldSize, int newSize)
{	
	Q_UNUSED(oldSize);
	static bool inside = false;
	m_table->setColumnWidth(logicalIndex, newSize);
	if (inside) return;
	inside = true;

	int cols = m_table->columnCount();
	for (int i=0; i<cols; i++)
		if (isColumnSelected(i, true)) 
			m_horizontal_header->resizeSection(i, newSize);

	inside = false;
}

void TableView::changeEvent(QEvent * event)
{
	if (event->type() == QEvent::LanguageChange) 
		retranslateStrings();
	QWidget::changeEvent(event);	
}

void TableView::retranslateStrings()
{
	m_hide_button->setToolTip(tr("Show/hide control tabs"));
    ui.retranslateUi(m_control_tabs);

	ui.type_box->clear();
	ui.type_box->addItem(tr("Numeric"), QVariant(int(SciDAVis::Numeric)));
	ui.type_box->addItem(tr("Text"), QVariant(int(SciDAVis::Text)));
	ui.type_box->addItem(tr("Month names"), QVariant(int(SciDAVis::Month)));
	ui.type_box->addItem(tr("Day names"), QVariant(int(SciDAVis::Day)));
	ui.type_box->addItem(tr("Date and time"), QVariant(int(SciDAVis::DateTime)));

	ui.type_box->setCurrentIndex(0);

	// TODO: implement formula stuff
	//ui.formula_info->document()->setPlainText("not implemented yet");
}
	
void TableView::advanceCell()
{
	QModelIndex idx = m_view_widget->currentIndex();
    if (idx.row()+1 >= m_table->rowCount())
	{
		int new_size = m_table->rowCount()+1;
		m_table->setRowCount(new_size);
	}
	m_view_widget->setCurrentIndex(idx.sibling(idx.row()+1, idx.column()));
}

void TableView::goToCell(int row, int col)
{
	QModelIndex index = m_model->index(row, col);
	m_view_widget->scrollTo(index);
	m_view_widget->setCurrentIndex(index);
}

void TableView::selectAll()
{
	m_view_widget->selectAll();
}

void TableView::deselectAll()
{
	m_view_widget->clearSelection();
}

void TableView::toggleControlTabBar() 
{ 
	m_control_tabs->setVisible(!m_control_tabs->isVisible());
	if (m_control_tabs->isVisible())
		m_hide_button->setArrowType(Qt::RightArrow);
	else
		m_hide_button->setArrowType(Qt::LeftArrow);
}

void TableView::handleHorizontalSectionMoved(int index, int from, int to)
{
	static bool inside = false;
	if (inside) return;

	Q_ASSERT(index == from);

	inside = true;
	m_view_widget->horizontalHeader()->moveSection(to, from);
	inside = false;
	m_table->moveColumn(from, to);
}

void TableView::handleHorizontalHeaderDoubleClicked(int index)
{
	Q_UNUSED(index);
	showControlDescriptionTab();
}

bool TableView::areCommentsShown() const
{
	return m_horizontal_header->areCommentsShown();
}

void TableView::toggleComments()
{
	showComments(!areCommentsShown());
}

void TableView::showComments(bool on)
{
	m_horizontal_header->showComments(on);
}

void TableView::currentColumnChanged(const QModelIndex & current, const QModelIndex & previous)
{
	Q_UNUSED(previous);
	int col = current.column();	
	if (col < 0 || col >= m_table->columnCount()) return;
	setColumnForDescriptionTab(col);
}

void TableView::setColumnForDescriptionTab(int col)
{
	if (col < 0 || col >= m_table->columnCount()) return;
	Column *col_ptr = m_table->column(col);

	QString str = QString(tr("Current column:\nName: %1\nPosition: %2"))\
		.arg(col_ptr->name()).arg(col+1);
		
	// TODO: currently, this eats up considerable screen space for duplicate information - do we need it?
	//ui.column_info->document()->setPlainText(str);
	ui.name_edit->setText(col_ptr->name());
	ui.comment_box->document()->setPlainText(col_ptr->comment());
}

void TableView::selectionChanged(const QItemSelection & selected, const QItemSelection & deselected)
{
	Q_UNUSED(selected);
	Q_UNUSED(deselected);
}

void TableView::updateFormatBox()
{
	int type_index = ui.type_box->currentIndex();
	if (type_index < 0) return; // should never happen
	ui.format_box->clear();
	ui.digits_box->setEnabled(false);
	switch(ui.type_box->itemData(type_index).toInt())
	{
		case SciDAVis::Numeric:
			ui.digits_box->setEnabled(true);
			ui.format_box->addItem(tr("Decimal"), QVariant('f'));
			ui.format_box->addItem(tr("Scientific (e)"), QVariant('e'));
			ui.format_box->addItem(tr("Scientific (E)"), QVariant('E'));
			break;
		case SciDAVis::Text:
			ui.format_box->addItem(tr("Text"), QVariant());
			break;
		case SciDAVis::Month:
			ui.format_box->addItem(tr("Number without leading zero"), QVariant("M"));
			ui.format_box->addItem(tr("Number with leading zero"), QVariant("MM"));
			ui.format_box->addItem(tr("Abbreviated month name"), QVariant("MMM"));
			ui.format_box->addItem(tr("Full month name"), QVariant("MMMM"));
			break;
		case SciDAVis::Day:
			ui.format_box->addItem(tr("Number without leading zero"), QVariant("d"));
			ui.format_box->addItem(tr("Number with leading zero"), QVariant("dd"));
			ui.format_box->addItem(tr("Abbreviated day name"), QVariant("ddd"));
			ui.format_box->addItem(tr("Full day name"), QVariant("dddd"));
			break;
		case SciDAVis::DateTime:
			{
				// TODO: allow adding of the combo box entries here
				const char * date_strings[] = {
					"yyyy-MM-dd", 	
					"yyyy/MM/dd", 
					"dd/MM/yyyy", 
					"dd/MM/yy", 
					"dd.MM.yyyy", 	
					"dd.MM.yy",
					"MM/yyyy",
					"dd.MM.", 
					"yyyyMMdd",
					0
				};

				const char * time_strings[] = {
					"hh",
					"hh ap",
					"hh:mm",
					"hh:mm ap",
					"hh:mm:ss",
					"hh:mm:ss.zzz",
					"hh:mm:ss:zzz",
					"mm:ss.zzz",
					"hhmmss",
					0
				};
				int j,i;
				for (i=0; date_strings[i] != 0; i++)
					ui.format_box->addItem(QString(date_strings[i]), QVariant(date_strings[i]));
				for (j=0; time_strings[j] != 0; j++)
					ui.format_box->addItem(QString(time_strings[j]), QVariant(time_strings[j]));
				for (i=0; date_strings[i] != 0; i++)
					for (j=0; time_strings[j] != 0; j++)
						ui.format_box->addItem(QString("%1 %2").arg(date_strings[i]).arg(time_strings[j]), 
							QVariant(QString(date_strings[i]) + " " + QString(time_strings[j])));
				break;
			}
		default:
			ui.format_box->addItem(QString()); // just for savety to have at least one item in any case
	}
	ui.format_box->setCurrentIndex(0);
}

void TableView::updateTypeInfo()
{
	int format_index = ui.format_box->currentIndex();
	int type_index = ui.type_box->currentIndex();

	QString str = tr("Selected column type:\n");
	if (format_index >= 0 && type_index >= 0)
	{
		int type = ui.type_box->itemData(type_index).toInt();
		switch(type)
		{
			case SciDAVis::Numeric:
				str += tr("Double precision\nfloating point values\n");
				ui.digits_box->setEnabled(true);
				break;
			case SciDAVis::Text:
				str += tr("Text\n");
				break;
			case SciDAVis::Month:
				str += tr("Month names\n");
				break;
			case SciDAVis::Day:
				str += tr("Days of the week\n");
				break;
			case SciDAVis::DateTime:
				str += tr("Dates and/or times\n");
				break;
		}
		str += tr("Example: ");
		switch(type)
		{
			case SciDAVis::Numeric:
				str += QString::number(123.1234567890123456, ui.format_box->itemData(format_index).toChar().toLatin1(), ui.digits_box->value());
				break;
			case SciDAVis::Text:
				str += tr("Hello world!\n");
				break;
			case SciDAVis::Month:
				str += QLocale().toString(QDate(1900,1,1), ui.format_box->itemData(format_index).toString());
				break;
			case SciDAVis::Day:
				str += QLocale().toString(QDate(1900,1,1), ui.format_box->itemData(format_index).toString());
				break;
			case SciDAVis::DateTime:
				str += QDateTime(QDate(1900,1,1), QTime(23,59,59,999)).toString(ui.format_box->itemData(format_index).toString());
				break;
		}
	}
	ui.type_info->setText(str);
}

void TableView::showControlDescriptionTab()
{
	m_control_tabs->setVisible(true);
	m_hide_button->setArrowType(Qt::RightArrow);
	ui.tab_widget->setCurrentIndex(0);
}

void TableView::showControlTypeTab()
{
	m_control_tabs->setVisible(true);
	m_hide_button->setArrowType(Qt::RightArrow);
	ui.tab_widget->setCurrentIndex(1);
}

void TableView::showControlFormulaTab()
{
	m_control_tabs->setVisible(true);
	m_hide_button->setArrowType(Qt::RightArrow);
	ui.tab_widget->setCurrentIndex(2);
}

void TableView::applyDescription()
{
	QItemSelectionModel * sel_model = m_view_widget->selectionModel();
	int index = sel_model->currentIndex().column();
	if (index >= 0)
	{
		m_table->column(index)->setName(ui.name_edit->text());
		m_table->column(index)->setComment(ui.comment_box->document()->toPlainText());
	}
}

void TableView::applyType()
{
	int format_index = ui.format_box->currentIndex();
	int type_index = ui.type_box->currentIndex();
	if (format_index < 0 && type_index < 0) return;

	SciDAVis::ColumnMode mode = (SciDAVis::ColumnMode)ui.type_box->itemData(type_index).toInt();
	QList<Column*> list = selectedColumns();
	switch(mode)
	{
		case SciDAVis::Numeric:
			foreach(Column* col, list)
			{
				col->setColumnMode(mode);
				Double2StringFilter * filter = static_cast<Double2StringFilter*>(col->outputFilter());
				filter->setNumericFormat(ui.format_box->itemData(format_index).toChar().toLatin1());
				filter->setNumDigits(ui.digits_box->value());
				// TODO: make sure this is done by a signal from the filter to the column on to the table
	//			m_model->emitColumnChanged(col); 
				}
			break;
		case SciDAVis::Text:
			foreach(Column* col, list)
				col->setColumnMode(mode);
			break;
		case SciDAVis::Month:
		case SciDAVis::Day:
		case SciDAVis::DateTime:
			foreach(Column* col, list)
			{
				col->setColumnMode(mode);
				DateTime2StringFilter * filter = static_cast<DateTime2StringFilter*>(col->outputFilter());
				filter->setFormat(ui.format_box->itemData(format_index).toString());
				// TODO: make sure this is done by a signal from the filter to the column on to the table
	//			m_model->emitColumnChanged(col); 
			}
			break;
	}
}
		
void TableView::handleHeaderDataChanged(Qt::Orientation orientation, int first, int last)
{
	if (orientation != Qt::Horizontal) return;

	QItemSelectionModel * sel_model = m_view_widget->selectionModel();

	int col = sel_model->currentIndex().column();
	if (col < first || col > last) return;
	setColumnForDescriptionTab(col);
}

int TableView::selectedColumnCount(bool full)
{
	int count = 0;
	int cols = m_table->columnCount();
	for (int i=0; i<cols; i++)
		if (isColumnSelected(i, full)) count++;
	return count;
}

int TableView::selectedColumnCount(SciDAVis::PlotDesignation pd)
{
	int count = 0;
	int cols = m_table->columnCount();
	for (int i=0; i<cols; i++)
		if ( isColumnSelected(i, false) && (m_table->column(i)->plotDesignation() == pd) ) count++;

	return count;
}

bool TableView::isColumnSelected(int col, bool full)
{
	if (full)
		return m_view_widget->selectionModel()->isColumnSelected(col, QModelIndex());
	else
		return m_view_widget->selectionModel()->columnIntersectsSelection(col, QModelIndex());
}

QList<Column*> TableView::selectedColumns(bool full)
{
	QList<Column*> list;
	int cols = m_table->columnCount();
	for (int i=0; i<cols; i++)
		if (isColumnSelected(i, full)) list << m_table->column(i);

	return list;
}

int TableView::selectedRowCount(bool full)
{
	int count = 0;
	int rows = m_table->rowCount();
	for (int i=0; i<rows; i++)
		if (isRowSelected(i, full)) count++;
	return count;
}

bool TableView::isRowSelected(int row, bool full)
{
	if (full)
		return m_view_widget->selectionModel()->isRowSelected(row, QModelIndex());
	else
		return m_view_widget->selectionModel()->rowIntersectsSelection(row, QModelIndex());
}

int TableView::firstSelectedColumn(bool full)
{
	int cols = m_table->columnCount();
	for (int i=0; i<cols; i++)
	{
		if (isColumnSelected(i, full))
			return i;
	}
	return -1;
}

int TableView::lastSelectedColumn(bool full)
{
	int cols = m_table->columnCount();
	for (int i=cols-1; i>=0; i--)
		if (isColumnSelected(i, full)) return i;

	return -2;
}

int TableView::firstSelectedRow(bool full)
{
	int rows = m_table->rowCount();
	for (int i=0; i<rows; i++)
	{
		if (isRowSelected(i, full))
			return i;
	}
	return -1;
}

int TableView::lastSelectedRow(bool full)
{
	int rows = m_table->rowCount();
	for (int i=rows-1; i>=0; i--)
		if (isRowSelected(i, full)) return i;

	return -2;
}

bool TableView::isCellSelected(int row, int col)
{
	if (row < 0 || col < 0 || row >= m_table->rowCount() || col >= m_table->columnCount()) return false;

	return m_view_widget->selectionModel()->isSelected(m_model->index(row, col));
}

void TableView::setCellSelected(int row, int col, bool select)
{
	 m_view_widget->selectionModel()->select(m_model->index(row, col), 
			 select ? QItemSelectionModel::Select : QItemSelectionModel::Deselect);
}

void TableView::setCellsSelected(int first_row, int first_col, int last_row, int last_col, bool select)
{
	QModelIndex top_left = m_model->index(first_row, first_col);
	QModelIndex bottom_right = m_model->index(last_row, last_col);
	m_view_widget->selectionModel()->select(QItemSelection(top_left, bottom_right), 
			select ? QItemSelectionModel::SelectCurrent : QItemSelectionModel::Deselect);
}

void TableView::getCurrentCell(int * row, int * col)
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

bool TableView::eventFilter(QObject * watched, QEvent * event)
{
	QHeaderView * v_header = m_view_widget->verticalHeader();

	if (event->type() == QEvent::ContextMenu) 
	{
		QContextMenuEvent *cm_event = static_cast<QContextMenuEvent *>(event);
		QPoint global_pos = cm_event->globalPos();
		if (watched == v_header)	
			showTableViewRowContextMenu(global_pos);
		else if (watched == m_horizontal_header)
			showTableViewColumnContextMenu(global_pos);
		else if (watched == m_view_widget)
			showTableViewContextMenu(global_pos);
		else
			return QWidget::eventFilter(watched, event);

		return true;
	} 
	else 
		return QWidget::eventFilter(watched, event);
}

bool TableView::formulaModeActive() const 
{ 
	return m_model->formulaModeActive(); 
}

void TableView::activateFormulaMode(bool on) 
{ 
	m_model->activateFormulaMode(on); 
}

void TableView::goToNextColumn()
{
	if (m_table->columnCount() == 0) return;

	QModelIndex idx = m_view_widget->currentIndex();
	int col = idx.column()+1;
    if (col >= m_table->columnCount())
		col = 0;
	m_view_widget->setCurrentIndex(idx.sibling(idx.row(), col));
}

void TableView::goToPreviousColumn()
{
	if (m_table->columnCount() == 0) return;

	QModelIndex idx = m_view_widget->currentIndex();
	int col = idx.column()-1;
    if (col < 0)
		col = m_table->columnCount()-1;
	m_view_widget->setCurrentIndex(idx.sibling(idx.row(), col));
}

QMenu * TableView::createSelectionMenu(QMenu * append_to)
{
	QMenu * menu = append_to;
	if (!menu)
		menu = new QMenu();

	QMenu * submenu = new QMenu("Fi&ll Selection with");
	submenu->addAction(action_fill_row_numbers);
	submenu->addAction(action_fill_random);
	menu->addMenu(submenu);
	menu->addSeparator();

	menu->addAction(action_cut_selection);
	menu->addAction(action_copy_selection);
	menu->addAction(action_paste_into_selection);
	menu->addAction(action_clear_selection);
	menu->addSeparator();
	menu->addAction(action_mask_selection);
	menu->addAction(action_unmask_selection);
	menu->addSeparator();
	menu->addAction(action_normalize_selection);
	menu->addSeparator();
	menu->addAction(action_set_formula);
	menu->addAction(action_recalculate);
	menu->addSeparator();

	return menu;
}


QMenu * TableView::createColumnMenu(QMenu * append_to)
{
	QMenu * menu = append_to;
	if (!menu)
		menu = new QMenu();


	QMenu * submenu = new QMenu(tr("S&et Column(s) As"));
	submenu->addAction(action_set_as_x);
	submenu->addAction(action_set_as_y);
	submenu->addAction(action_set_as_z);
	submenu->addSeparator();
	submenu->addAction(action_set_as_xerr);
	submenu->addAction(action_set_as_yerr);
	submenu->addSeparator();
	submenu->addAction(action_set_as_none);
	menu->addMenu(submenu);
	menu->addSeparator();

	submenu = new QMenu("Fi&ll Selection with");
	submenu->addAction(action_fill_row_numbers);
	submenu->addAction(action_fill_random);
	menu->addMenu(submenu);
	menu->addSeparator();

	menu->addAction(action_insert_columns);
	menu->addAction(action_remove_columns);
	menu->addAction(action_clear_columns);
	menu->addAction(action_add_columns);
	menu->addSeparator();
	
	menu->addAction(action_normalize_columns);
	menu->addAction(action_sort_columns);
	menu->addSeparator();

	menu->addAction(action_edit_description);
	menu->addAction(action_type_format);
	connect(menu, SIGNAL(aboutToShow()), this, SLOT(adjustActionNames()));
	menu->addAction(action_toggle_comments);
	menu->addSeparator();

	menu->addAction(action_statistics_columns);

	return menu;
}

QMenu * TableView::createTableMenu(QMenu * append_to) 
{
	QMenu * menu = append_to;
	if (!menu)
		menu = new QMenu();

	connect(menu, SIGNAL(aboutToShow()), this, SLOT(adjustActionNames()));
	menu->addAction(action_toggle_comments);
	menu->addAction(action_toggle_tabbar);
	menu->addAction(action_formula_mode);
	menu->addSeparator();
	menu->addAction(action_select_all);
	menu->addAction(action_clear_table);
	menu->addAction(action_clear_masks);
	menu->addAction(action_sort_table);
	menu->addSeparator();
	menu->addAction(action_add_column);
	menu->addSeparator();
	menu->addAction(action_go_to_cell);

	return menu;
}

QMenu * TableView::createRowMenu(QMenu * append_to) 
{
	QMenu * menu = append_to;
	if (!menu)
		menu = new QMenu();

	menu->addAction(action_insert_rows);
	menu->addAction(action_remove_rows);
	menu->addAction(action_clear_rows);
	menu->addAction(action_add_rows);
	menu->addSeparator();
	QMenu *submenu = new QMenu("Fi&ll Selection with");
	submenu->addAction(action_fill_row_numbers);
	submenu->addAction(action_fill_random);
	menu->addMenu(submenu);
	menu->addSeparator();
	menu->addAction(action_statistics_rows);

	return menu;
}

void TableView::cutSelection()
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

void TableView::copySelection()
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

void TableView::pasteIntoSelection()
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

void TableView::maskSelection()
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

void TableView::unmaskSelection()
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

void TableView::setFormulaForSelection()
{
	showControlFormulaTab();
}

void TableView::recalculateSelectedCells()
{
	// TODO
	QMessageBox::information(0, "info", "not yet implemented");
}

void TableView::fillSelectedCellsWithRowNumbers()
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

void TableView::fillSelectedCellsWithRandomNumbers()
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

void TableView::sortTable()
{
	QList<Column*> cols;
	
	for (int i=0; i<m_table->columnCount(); i++)
		cols.append(m_table->column(i));

	sortDialog(cols);
}

void TableView::insertEmptyColumns()
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

void TableView::removeSelectedColumns()
{
	WAIT_CURSOR;
	m_table->beginMacro(QObject::tr("%1: remove selected column(s)").arg(m_table->name()));

	QList< Column* > list = selectedColumns();
	foreach(Column* ptr, list)
		m_table->removeColumn(ptr);

	m_table->endMacro();
	RESET_CURSOR;
}

void TableView::clearSelectedColumns()
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

void TableView::setSelectionAs(SciDAVis::PlotDesignation pd)
{
	WAIT_CURSOR;
	m_table->beginMacro(QObject::tr("%1: set plot designation(s)").arg(m_table->name()));

	QList< Column* > list = selectedColumns();
	foreach(Column* ptr, list)
		ptr->setPlotDesignation(pd);

	m_table->endMacro();
	RESET_CURSOR;
}

void TableView::setSelectedColumnsAsX()
{
	setSelectionAs(SciDAVis::X);
}

void TableView::setSelectedColumnsAsY()
{
	setSelectionAs(SciDAVis::Y);
}

void TableView::setSelectedColumnsAsZ()
{
	setSelectionAs(SciDAVis::Z);
}

void TableView::setSelectedColumnsAsYError()
{
	setSelectionAs(SciDAVis::yErr);
}

void TableView::setSelectedColumnsAsXError()
{
	setSelectionAs(SciDAVis::xErr);
}

void TableView::setSelectedColumnsAsNone()
{
	setSelectionAs(SciDAVis::noDesignation);
}

void TableView::normalizeSelectedColumns()
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

void TableView::normalizeSelection()
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

void TableView::sortSelectedColumns()
{
	QList< Column* > cols = selectedColumns();
	sortDialog(cols);
}

void TableView::statisticsOnSelectedColumns()
{
	// TODO
	QMessageBox::information(0, "info", "not yet implemented");
}

void TableView::statisticsOnSelectedRows()
{
	// TODO
	QMessageBox::information(0, "info", "not yet implemented");
}

void TableView::insertEmptyRows()
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

void TableView::removeSelectedRows()
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

void TableView::clearSelectedRows()
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

void TableView::editTypeAndFormatOfSelectedColumns()
{
	showControlTypeTab();
}

void TableView::editDescriptionOfCurrentColumn()
{
	showControlDescriptionTab();
}

void TableView::clearSelectedCells()
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

void TableView::fillProjectMenu(QMenu * menu, bool * rc)
{
	menu->setTitle(tr("&Table"));

	QMenu * submenu = new QMenu(tr("S&et Column(s) As"));
	submenu->addAction(action_set_as_x);
	submenu->addAction(action_set_as_y);
	submenu->addAction(action_set_as_z);
	submenu->addSeparator();
	submenu->addAction(action_set_as_xerr);
	submenu->addAction(action_set_as_yerr);
	submenu->addSeparator();
	submenu->addAction(action_set_as_none);
	menu->addMenu(submenu);
	menu->addSeparator();

	submenu = new QMenu("Fi&ll Selection with");
	submenu->addAction(action_fill_row_numbers);
	submenu->addAction(action_fill_random);
	menu->addMenu(submenu);
	menu->addSeparator();

	connect(menu, SIGNAL(aboutToShow()), this, SLOT(adjustActionNames()));
	menu->addAction(action_toggle_comments);
	menu->addAction(action_toggle_tabbar);
	menu->addAction(action_formula_mode);
	menu->addAction(action_edit_description);
	menu->addAction(action_type_format);
	menu->addSeparator();
	menu->addAction(action_clear_table);
#ifndef LEGACY_CODE_0_2_x
	menu->addAction(action_clear_masks);
#endif
	menu->addAction(action_sort_table);
	menu->addSeparator();
	menu->addAction(action_set_formula);
	menu->addAction(action_recalculate);
	menu->addSeparator();
	menu->addAction(action_add_column);
	menu->addAction(action_dimensions_dialog);
	menu->addSeparator();
	menu->addAction(action_go_to_cell);

	if (rc) *rc = true;

	// TODO:
	// Convert to Matrix
	// Export 
}

void TableView::createContextMenu(QMenu * menu)
{
	menu->addSeparator();
	
	// TODO
	// Export to ASCII
	// Print --> maybe should go to AbstractPart::createContextMenu()
	// ----
	// Rename --> AbstractAspect::createContextMenu(); maybe call this "Properties" and include changing comment/caption spec
	// Duplicate --> AbstractPart::createContextMenu()
	// Hide/Show --> Do we need hiding of views (in addition to minimizing)? How do we avoid confusion with hiding of Aspects?
	// Activate ?
	// Resize --> AbstractPart::createContextMenu()
}

void TableView::createActions()
{
	QIcon * icon_temp;

	// selection related actions
	action_cut_selection = new QAction(QIcon(QPixmap(":/cut.xpm")), tr("Cu&t"), this);
	actionManager()->addAction(action_cut_selection, "cut_selection");

	action_copy_selection = new QAction(QIcon(QPixmap(":/copy.xpm")), tr("&Copy"), this);
	actionManager()->addAction(action_copy_selection, "copy_selection");

	action_paste_into_selection = new QAction(QIcon(QPixmap(":/paste.xpm")), tr("Past&e"), this);
	actionManager()->addAction(action_paste_into_selection, "paste_into_selection"); 

	action_mask_selection = new QAction(QIcon(QPixmap(":/mask.xpm")), tr("&Mask","mask selection"), this);
	actionManager()->addAction(action_mask_selection, "mask_selection"); 

	action_unmask_selection = new QAction(QIcon(QPixmap(":/unmask.xpm")), tr("&Unmask","unmask selection"), this);
	actionManager()->addAction(action_unmask_selection, "unmask_selection"); 

	icon_temp = new QIcon();
	icon_temp->addPixmap(QPixmap(":/16x16/fx.png"));
	icon_temp->addPixmap(QPixmap(":/32x32/fx.png"));
	action_set_formula = new QAction(*icon_temp, tr("Assign &Formula"), this);
	actionManager()->addAction(action_set_formula, "set_formula"); 
	delete icon_temp;

	icon_temp = new QIcon();
	icon_temp->addPixmap(QPixmap(":/16x16/clear.png"));
	icon_temp->addPixmap(QPixmap(":/32x32/clear.png"));
	action_clear_selection = new QAction(*icon_temp, tr("Clea&r","clear selection"), this);
	actionManager()->addAction(action_clear_selection, "clear_selection"); 
	delete icon_temp;

	icon_temp = new QIcon();
	icon_temp->addPixmap(QPixmap(":/16x16/recalculate.png"));
	icon_temp->addPixmap(QPixmap(":/32x32/recalculate.png"));
	action_recalculate = new QAction(*icon_temp, tr("Recalculate"), this);
	actionManager()->addAction(action_recalculate, "recalculate"); 
	delete icon_temp;

	action_fill_row_numbers = new QAction(QIcon(QPixmap(":/rowNumbers.xpm")), tr("Row Numbers"), this);
	actionManager()->addAction(action_fill_row_numbers, "fill_row_numbers"); 

	action_fill_random = new QAction(QIcon(QPixmap(":/randomNumbers.xpm")), tr("Random Values"), this);
	actionManager()->addAction(action_fill_random, "fill_random"); 
	
	//table related actions
	icon_temp = new QIcon();
	icon_temp->addPixmap(QPixmap(":/16x16/table_header.png"));
	icon_temp->addPixmap(QPixmap(":/32x32/table_header.png"));
	action_toggle_comments = new QAction(*icon_temp, QString("Show/Hide comments"), this); // show/hide column comments
	actionManager()->addAction(action_toggle_comments, "toggle_comments"); 
	delete icon_temp;

	icon_temp = new QIcon();
	icon_temp->addPixmap(QPixmap(":/16x16/table_options.png"));
	icon_temp->addPixmap(QPixmap(":/32x32/table_options.png"));
	action_toggle_tabbar = new QAction(*icon_temp, QString("Show/Hide Controls"), this); // show/hide control tabs
	actionManager()->addAction(action_toggle_tabbar, "toggle_tabbar"); 
	delete icon_temp;

	action_formula_mode = new QAction(tr("Formula Edit Mode"), this);
	action_formula_mode->setCheckable(true);
	actionManager()->addAction(action_formula_mode, "formula_mode"); 

	icon_temp = new QIcon();
	icon_temp->addPixmap(QPixmap(":/16x16/select_all.png"));
	icon_temp->addPixmap(QPixmap(":/32x32/select_all.png"));
	action_select_all = new QAction(*icon_temp, tr("Select All"), this);
	actionManager()->addAction(action_select_all, "select_all"); 
	delete icon_temp;

	icon_temp = new QIcon();
	icon_temp->addPixmap(QPixmap(":/16x16/add_column.png"));
	icon_temp->addPixmap(QPixmap(":/32x32/add_column.png"));
	action_add_column = new QAction(*icon_temp, tr("&Add Column"), this);
	actionManager()->addAction(action_add_column, "add_column"); 
	delete icon_temp;

	icon_temp = new QIcon();
	icon_temp->addPixmap(QPixmap(":/16x16/clear_table.png"));
	icon_temp->addPixmap(QPixmap(":/32x32/clear_table.png"));
	action_clear_table = new QAction(*icon_temp, tr("Clear Table"), this);
	actionManager()->addAction(action_clear_table, "clear_table"); 
	delete icon_temp;

	action_clear_masks = new QAction(QIcon(QPixmap(":/unmask.xpm")), tr("Clear Masks"), this);
	actionManager()->addAction(action_clear_masks, "clear_masks"); 

	icon_temp = new QIcon();
	icon_temp->addPixmap(QPixmap(":/16x16/sort.png"));
	icon_temp->addPixmap(QPixmap(":/32x32/sort.png"));
	action_sort_table = new QAction(*icon_temp, tr("&Sort Table"), this);
	actionManager()->addAction(action_sort_table, "sort_table"); 
	delete icon_temp;

	icon_temp = new QIcon();
	icon_temp->addPixmap(QPixmap(":/16x16/go_to_cell.png"));
	icon_temp->addPixmap(QPixmap(":/32x32/go_to_cell.png"));
	action_go_to_cell = new QAction(*icon_temp, tr("&Go to Cell"), this);
	actionManager()->addAction(action_go_to_cell, "go_to_cell"); 
	delete icon_temp;

	action_dimensions_dialog = new QAction(QIcon(QPixmap(":/resize.xpm")), tr("&Dimensions", "table size"), this);
	actionManager()->addAction(action_dimensions_dialog, "dimensions_dialog"); 

	// column related actions
	icon_temp = new QIcon();
	icon_temp->addPixmap(QPixmap(":/16x16/insert_column.png"));
	icon_temp->addPixmap(QPixmap(":/32x32/insert_column.png"));
	action_insert_columns = new QAction(*icon_temp, tr("&Insert Empty Columns"), this);
	actionManager()->addAction(action_insert_columns, "insert_columns"); 
	delete icon_temp;

	icon_temp = new QIcon();
	icon_temp->addPixmap(QPixmap(":/16x16/remove_column.png"));
	icon_temp->addPixmap(QPixmap(":/32x32/remove_column.png"));
	action_remove_columns = new QAction(*icon_temp, tr("Remo&ve Columns"), this);
	actionManager()->addAction(action_remove_columns, "remove_columns"); 
	delete icon_temp;

	icon_temp = new QIcon();
	icon_temp->addPixmap(QPixmap(":/16x16/clear_column.png"));
	icon_temp->addPixmap(QPixmap(":/32x32/clear_column.png"));
	action_clear_columns = new QAction(*icon_temp, tr("Clea&r Columns"), this);
	actionManager()->addAction(action_clear_columns, "clear_columns"); 
	delete icon_temp;

	icon_temp = new QIcon();
	icon_temp->addPixmap(QPixmap(":/16x16/add_columns.png"));
	icon_temp->addPixmap(QPixmap(":/32x32/add_columns.png"));
	action_add_columns = new QAction(*icon_temp, tr("&Add Columns"), this);
	actionManager()->addAction(action_add_columns, "add_columns"); 
	delete icon_temp;

	action_set_as_x = new QAction(QIcon(QPixmap()), tr("X","plot designation"), this);
	actionManager()->addAction(action_set_as_x, "set_as_x"); 

	action_set_as_y = new QAction(QIcon(QPixmap()), tr("Y","plot designation"), this);
	actionManager()->addAction(action_set_as_y, "set_as_y"); 

	action_set_as_z = new QAction(QIcon(QPixmap()), tr("Z","plot designation"), this);
	actionManager()->addAction(action_set_as_z, "set_as_z"); 

	icon_temp = new QIcon();
	icon_temp->addPixmap(QPixmap(":/16x16/x_error.png"));
	icon_temp->addPixmap(QPixmap(":/32x32/x_error.png"));
	action_set_as_xerr = new QAction(*icon_temp, tr("X Error","plot designation"), this);
	actionManager()->addAction(action_set_as_xerr, "set_as_xerr"); 
	delete icon_temp;

	icon_temp = new QIcon();
	icon_temp->addPixmap(QPixmap(":/16x16/y_error.png"));
	icon_temp->addPixmap(QPixmap(":/32x32/y_error.png"));
	action_set_as_yerr = new QAction(*icon_temp, tr("Y Error","plot designation"), this);
	actionManager()->addAction(action_set_as_yerr, "set_as_yerr"); 
	delete icon_temp;

	action_set_as_none = new QAction(QIcon(QPixmap()), tr("None","plot designation"), this);
	actionManager()->addAction(action_set_as_none, "set_as_none"); 

	icon_temp = new QIcon();
	icon_temp->addPixmap(QPixmap(":/16x16/normalize.png"));
	icon_temp->addPixmap(QPixmap(":/32x32/normalize.png"));
	action_normalize_columns = new QAction(*icon_temp, tr("&Normalize Columns"), this);
	actionManager()->addAction(action_normalize_columns, "normalize_columns"); 
	delete icon_temp;

	icon_temp = new QIcon();
	icon_temp->addPixmap(QPixmap(":/16x16/normalize.png"));
	icon_temp->addPixmap(QPixmap(":/32x32/normalize.png"));
	action_normalize_selection = new QAction(*icon_temp, tr("&Normalize Selection"), this);
	actionManager()->addAction(action_normalize_selection, "normalize_selection"); 
	delete icon_temp;

	icon_temp = new QIcon();
	icon_temp->addPixmap(QPixmap(":/16x16/sort.png"));
	icon_temp->addPixmap(QPixmap(":/32x32/sort.png"));
	action_sort_columns = new QAction(*icon_temp, tr("&Sort Columns"), this);
	actionManager()->addAction(action_sort_columns, "sort_columns"); 
	delete icon_temp;

	action_statistics_columns = new QAction(QIcon(QPixmap(":/col_stat.xpm")), tr("Column Statisti&cs"), this);
	actionManager()->addAction(action_statistics_columns, "statistics_columns"); 

	icon_temp = new QIcon();
	icon_temp->addPixmap(QPixmap(":/16x16/column_format_type.png"));
	icon_temp->addPixmap(QPixmap(":/32x32/column_format_type.png"));
	action_type_format = new QAction(*icon_temp, tr("Change &Type && Format"), this);
	actionManager()->addAction(action_type_format, "type_format"); 
	delete icon_temp;

	icon_temp = new QIcon();
	icon_temp->addPixmap(QPixmap(":/16x16/column_description.png"));
	icon_temp->addPixmap(QPixmap(":/32x32/column_description.png"));
	action_edit_description = new QAction(*icon_temp, tr("Edit Column &Description"), this);
	actionManager()->addAction(action_edit_description, "edit_description"); 
	delete icon_temp;

	// row related actions
	icon_temp = new QIcon();
	icon_temp->addPixmap(QPixmap(":/16x16/insert_row.png"));
	icon_temp->addPixmap(QPixmap(":/32x32/insert_row.png"));
	action_insert_rows = new QAction(*icon_temp ,tr("&Insert Empty Rows"), this);
	actionManager()->addAction(action_insert_rows, "insert_rows"); 
	delete icon_temp;

	icon_temp = new QIcon();
	icon_temp->addPixmap(QPixmap(":/16x16/remove_row.png"));
	icon_temp->addPixmap(QPixmap(":/32x32/remove_row.png"));
	action_remove_rows = new QAction(*icon_temp, tr("Remo&ve Rows"), this);
	actionManager()->addAction(action_remove_rows, "remove_rows"); 
	delete icon_temp;

	icon_temp = new QIcon();
	icon_temp->addPixmap(QPixmap(":/16x16/clear_row.png"));
	icon_temp->addPixmap(QPixmap(":/32x32/clear_row.png"));
	action_clear_rows = new QAction(*icon_temp, tr("Clea&r Rows"), this);
	actionManager()->addAction(action_clear_rows, "clear_rows"); 
	delete icon_temp;

	icon_temp = new QIcon();
	icon_temp->addPixmap(QPixmap(":/16x16/add_rows.png"));
	icon_temp->addPixmap(QPixmap(":/32x32/add_rows.png"));
	action_add_rows = new QAction(*icon_temp, tr("&Add Rows"), this);
	actionManager()->addAction(action_add_rows, "add_rows"); 
	delete icon_temp;

	action_statistics_rows = new QAction(QIcon(QPixmap(":/stat_rows.xpm")), tr("Row Statisti&cs"), this);
	actionManager()->addAction(action_statistics_rows, "statistics_rows"); 
}

void TableView::connectActions()
{
	connect(action_cut_selection, SIGNAL(triggered()), this, SLOT(cutSelection()));
	connect(action_copy_selection, SIGNAL(triggered()), this, SLOT(copySelection()));
	connect(action_paste_into_selection, SIGNAL(triggered()), this, SLOT(pasteIntoSelection()));
	connect(action_mask_selection, SIGNAL(triggered()), this, SLOT(maskSelection()));
	connect(action_unmask_selection, SIGNAL(triggered()), this, SLOT(unmaskSelection()));
	connect(action_set_formula, SIGNAL(triggered()), this, SLOT(setFormulaForSelection()));
	connect(action_clear_selection, SIGNAL(triggered()), this, SLOT(clearSelectedCells()));
	connect(action_recalculate, SIGNAL(triggered()), this, SLOT(recalculateSelectedCells()));
	connect(action_fill_row_numbers, SIGNAL(triggered()), this, SLOT(fillSelectedCellsWithRowNumbers()));
	connect(action_fill_random, SIGNAL(triggered()), this, SLOT(fillSelectedCellsWithRandomNumbers()));
	connect(action_select_all, SIGNAL(triggered()), this, SLOT(selectAll()));
	connect(action_add_column, SIGNAL(triggered()), m_table, SLOT(addColumn()));
	connect(action_clear_table, SIGNAL(triggered()), m_table, SLOT(clear()));
	connect(action_clear_masks, SIGNAL(triggered()), m_table, SLOT(clearMasks()));
	connect(action_sort_table, SIGNAL(triggered()), this, SLOT(sortTable()));
	connect(action_go_to_cell, SIGNAL(triggered()), this, SLOT(goToCell()));
	connect(action_dimensions_dialog, SIGNAL(triggered()), this, SLOT(dimensionsDialog()));
	connect(action_insert_columns, SIGNAL(triggered()), this, SLOT(insertEmptyColumns()));
	connect(action_remove_columns, SIGNAL(triggered()), this, SLOT(removeSelectedColumns()));
	connect(action_clear_columns, SIGNAL(triggered()), this, SLOT(clearSelectedColumns()));
	connect(action_add_columns, SIGNAL(triggered()), this, SLOT(addColumns()));
	connect(action_set_as_x, SIGNAL(triggered()), this, SLOT(setSelectedColumnsAsX()));
	connect(action_set_as_y, SIGNAL(triggered()), this, SLOT(setSelectedColumnsAsY()));
	connect(action_set_as_z, SIGNAL(triggered()), this, SLOT(setSelectedColumnsAsZ()));
	connect(action_set_as_xerr, SIGNAL(triggered()), this, SLOT(setSelectedColumnsAsXError()));
	connect(action_set_as_yerr, SIGNAL(triggered()), this, SLOT(setSelectedColumnsAsYError()));
	connect(action_set_as_none, SIGNAL(triggered()), this, SLOT(setSelectedColumnsAsNone()));
	connect(action_normalize_columns, SIGNAL(triggered()), this, SLOT(normalizeSelectedColumns()));
	connect(action_normalize_selection, SIGNAL(triggered()), this, SLOT(normalizeSelection()));
	connect(action_sort_columns, SIGNAL(triggered()), this, SLOT(sortSelectedColumns()));
	connect(action_statistics_columns, SIGNAL(triggered()), this, SLOT(statisticsOnSelectedColumns()));
	connect(action_type_format, SIGNAL(triggered()), this, SLOT(editTypeAndFormatOfSelectedColumns()));
	connect(action_edit_description, SIGNAL(triggered()), this, SLOT(editDescriptionOfCurrentColumn()));
	connect(action_insert_rows, SIGNAL(triggered()), this, SLOT(insertEmptyRows()));
	connect(action_remove_rows, SIGNAL(triggered()), this, SLOT(removeSelectedRows()));
	connect(action_clear_rows, SIGNAL(triggered()), this, SLOT(clearSelectedRows()));
	connect(action_add_rows, SIGNAL(triggered()), this, SLOT(addRows()));
	connect(action_statistics_rows, SIGNAL(triggered()), this, SLOT(statisticsOnSelectedRows()));
	connect(action_toggle_comments, SIGNAL(triggered()), this, SLOT(toggleComments()));
	connect(action_toggle_tabbar, SIGNAL(triggered()), this, SLOT(toggleControlTabBar()));
	connect(action_formula_mode, SIGNAL(toggled(bool)), this, SLOT(activateFormulaMode(bool)));
}

void TableView::showTableViewContextMenu(const QPoint& pos)
{
	QMenu context_menu;
	
	if (m_plot_menu)
	{
		context_menu.addMenu(m_plot_menu);
		context_menu.addSeparator();
	}

	createSelectionMenu(&context_menu);
	context_menu.addSeparator();
	createTableMenu(&context_menu);
	context_menu.addSeparator();

	context_menu.exec(pos);
}

void TableView::showTableViewColumnContextMenu(const QPoint& pos)
{
	QMenu context_menu;
	
	if (m_plot_menu)
	{
		context_menu.addMenu(m_plot_menu);
		context_menu.addSeparator();
	}

	createColumnMenu(&context_menu);
	context_menu.addSeparator();

	context_menu.exec(pos);
}

void TableView::showTableViewRowContextMenu(const QPoint& pos)
{
	QMenu context_menu;

	createRowMenu(&context_menu);

	context_menu.exec(pos);
}

void TableView::goToCell()
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

void TableView::dimensionsDialog()
{
	Ui::DimensionsDialog ui;
	QDialog dialog;
	ui.setupUi(&dialog);
	dialog.setWindowTitle(tr("Set Table Dimensions"));
	ui.columnsSpinBox->setValue(m_table->columnCount());
	ui.rowsSpinBox->setValue(m_table->rowCount());
	connect(ui.buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));
	connect(ui.buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));

	if (dialog.exec())
	{
		m_table->setColumnCount(ui.columnsSpinBox->value());
		m_table->setRowCount(ui.rowsSpinBox->value());
	}
}

void TableView::setPlotMenu(QMenu * menu)
{
	m_plot_menu = menu;
}

void TableView::sortDialog(QList<Column*> cols)
{
	if (cols.isEmpty()) return;

	SortDialog *sortd = new SortDialog();
	sortd->setAttribute(Qt::WA_DeleteOnClose);
	connect(sortd, SIGNAL(sort(Column*,QList<Column*>,bool)), this, SLOT(sortColumns(Column*,QList<Column*>,bool)));
	sortd->setColumnsList(cols);
	sortd->exec();
}

void TableView::addColumns()
{
	m_table->addColumns(selectedColumnCount(false));
}

void TableView::addRows()
{
	m_table->addColumns(selectedRowCount(false));
}

void TableView::adjustActionNames()
{
	QString action_name;
	if(areCommentsShown()) 
		action_name = tr("Hide Comments");
	else
		action_name = tr("Show Comments");
	action_toggle_comments->setText(action_name);

	if(isControlTabBarVisible()) 
		action_name = tr("Hide Controls");
	else
		action_name = tr("Show Controls");
	action_toggle_tabbar->setText(action_name);
}


/* ========================= static methods ======================= */
ActionManager * TableView::action_manager = 0;

ActionManager * TableView::actionManager()
{
	if (!action_manager)
		initActionManager();
	
	return action_manager;
}

void TableView::initActionManager()
{
	if (!action_manager)
		action_manager = new ActionManager();

	action_manager->setTitle(tr("Table"));
	volatile TableView * action_creator = new TableView(); // initialize the action texts
	delete action_creator;
}

int TableView::defaultColumnWidth() 
{ 
	return Table::global("default_column_width").toInt(); 
}

void TableView::setDefaultColumnWidth(int width) 
{ 
	Table::setGlobal("default_column_width", width); 
}

void TableView::setDefaultCommentVisibility(bool visible) 
{ 
	Table::setGlobal("default_comment_visibility", visible); 
}

bool TableView::defaultCommentVisibility() 
{ 
	return Table::global("default_comment_visibility").toBool(); 
}

/* ================== TableViewWidget ================ */

void TableViewWidget::selectAll()
{
	// the original QTableView::selectAll() toggles all cells which is strange behavior IMHO - thzs
	QItemSelectionModel * sel_model = selectionModel();
	QItemSelection sel(model()->index(0, 0, QModelIndex()), model()->index(model()->rowCount()-1, 
		model()->columnCount()-1, QModelIndex()));
	sel_model->select(sel, QItemSelectionModel::Select);
}

void TableViewWidget::updateHeaderGeometry(Qt::Orientation o, int first, int last)
{
	Q_UNUSED(first)
	Q_UNUSED(last)
	if (o != Qt::Horizontal) return;
	horizontalHeader()->setStretchLastSection(true);  // ugly hack (flaw in Qt? Does anyone know a better way?)
	horizontalHeader()->updateGeometry();
	horizontalHeader()->setStretchLastSection(false); // ugly hack part 2
}

void TableViewWidget::keyPressEvent(QKeyEvent * event)
{
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
		emit advanceCell();
	QTableView::keyPressEvent(event);
}

