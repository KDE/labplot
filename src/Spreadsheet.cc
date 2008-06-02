// LabPlot : Spreadsheet.cc

#include <KLocale>
#include <KDebug>
#include <KInputDialog>
#include <QHeaderView>
#include <QStandardItemModel>
#include <KAction>

#include "Spreadsheet.h"
#include "MainWin.h"
#include "TableModel.h"
#include "ColumnDialog.h"
#include "pixmaps/pixmap.h"
#include "column.h"
#include "elements/Point3D.h"
#include "gui/FunctionPlotDialog.h"

Spreadsheet::Spreadsheet(MainWin *mw)
	:QTableView(), mw(mw)
{
	kDebug()<<"Spreadsheet()"<<endl;
	type = SPREADSHEET;
	setModel(new TableModel());

	//  set title
	int number=1;
	while(mw->getSpreadsheet(i18n("Spreadsheet %1").arg(number)) != 0)
		number++;
	setWindowTitle(i18n("Spreadsheet %1").arg(number));

	setColumnCount(2);
	setRowCount(100);
	setSortingEnabled(false);
	resetHeader();

//	QObject::connect(table,SIGNAL(currentChanged(int,int)),this,SLOT(updateValuesDialog()));
//	QObject::connect(table,SIGNAL(selectionChanged()),this,SLOT(selectionChanged()));
}

Spreadsheet::~Spreadsheet() {
	mw->updateGUI();
	delete model();
}

void Spreadsheet::contextMenuEvent(QContextMenuEvent *) {
	QMenu *menu = new QMenu(this);
	Menu(menu);
	menu->exec(QCursor::pos());
}

void Spreadsheet::Menu(QMenu *menu) {
	kDebug()<<"Spreadsheet::Menu()"<<endl;
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

	action = new KAction(KIcon("insert-table"),i18n("New spreadsheet"),this);
	action->setShortcut(Qt::CTRL+Qt::Key_Equal);
	menu->addAction(action);
	connect(action, SIGNAL(triggered()),mw, SLOT(newSpreadsheet()));
}

QDomElement Spreadsheet::save(QDomDocument doc) {
	kDebug()<<endl;
	QDomElement sstag = doc.createElement( "Spreadsheet" );
	QDomElement tag = doc.createElement( "Position" );
	tag.setAttribute("x",QString::number(parentWidget()->pos().x()));
	tag.setAttribute("y",QString::number(parentWidget()->pos().y()));
	sstag.appendChild( tag );
	tag = doc.createElement( "Size" );
	tag.setAttribute("rows",QString::number(rowCount()));
	tag.setAttribute("cols",QString::number(columnCount()));
	sstag.appendChild( tag );
	tag = doc.createElement( "Title" );
	sstag.appendChild( tag );
	QDomText t = doc.createTextNode( windowTitle() );
	tag.appendChild( t );
	tag = doc.createElement( "Notes" );
	sstag.appendChild( tag );
	t = doc.createTextNode( notes );
	tag.appendChild( t );
/*	tag = doc.createElement( "Datafile" );
	sstag.appendChild( tag );
	t = doc.createTextNode( datafile );
 	tag.appendChild( t );
*/
	// header
	for(int i=0;i < columnCount();i++) {
		tag = doc.createElement( "Column" );
		tag.setAttribute("nr",QString::number(i));
		sstag.appendChild( tag );
  		t = doc.createTextNode( columnHeader(i) );
 		tag.appendChild( t );
	}

	// data
	for(int i=0;i < rowCount();i++) {
		for(int j=0;j < columnCount();j++) {
			if(!text(i,j).isEmpty()) {	// only filled cells
				tag = doc.createElement( "Cell" );
				tag.setAttribute("row",QString::number(i));
				tag.setAttribute("col",QString::number(j));
//				tag.setAttribute("masked",QString::number(((LTableItem *)table->item(i,j))->Masked()));
				sstag.appendChild( tag );
  				t = doc.createTextNode( text(i,j) );
 				tag.appendChild( t );
			}
		}
	}

	return sstag;
}

void Spreadsheet::open(QDomNode node) {
	kDebug()<<endl;
	while(!node.isNull()) {
		QDomElement e = node.toElement();
//		kDebug()<<"SS TAG = "<<e.tagName()<<endl;
//		kDebug()<<"SS TEXT = "<<e.text()<<endl;

		if(e.tagName() == "Position") {
			int px = e.attribute("x").toInt();
			int py = e.attribute("y").toInt();
			parentWidget()->move(QPoint(px,py));
			if(px<0 || py<0) {	// fullscreen
				mw->getMdi()->cascadeSubWindows();
				showMaximized();
			}
		}
		else if(e.tagName() == "Size") {
			setRowCount(e.attribute("rows").toInt());
			setColumnCount(e.attribute("cols").toInt());
		}
		else if(e.tagName() == "Title")
			setTitle(e.text());
		else if(e.tagName() == "Notes")
			notes = e.text();
//		else if(e.tagName() == "Datafile")
//			datafile = e.text();
		else if(e.tagName() == "Column")
			model()->setHeaderData(e.attribute("nr").toInt(),Qt::Horizontal,e.text());
		else if(e.tagName() == "Cell") {
//			item->setMasked(e.attribute("masked").toInt());
			setText(e.attribute("row").toInt(), e.attribute("col").toInt(),e.text());
		}

		node = node.nextSibling();
	}
}

void Spreadsheet::setTitle(QString title) {
	kDebug()<<"title ="<<title<<endl;
	bool ok=true;
	if(title.isEmpty())
		title = KInputDialog::getText("LabPlot", i18n("Spreadsheet title : "), windowTitle(), &ok);

	if(ok && !title.isEmpty()) {
		setWindowTitle(title);
		mw->updateSheetList();
	}
}

void Spreadsheet::setRowNumber(int row) {
	kDebug()<<"row ="<<row<<endl;
	bool ok=true;
	if(row<=0)
		row = KInputDialog::getInteger("LabPlot", i18n("Row count : "), rowCount(), 1, INT_MAX, 1, 10, &ok);

	if(ok && row>0)
		setRowCount(row);
}

QString Spreadsheet::text(int row, int col) const {
//	kDebug()<<"Spreadsheet::text("<<row<<col<<")"<<endl;
	QModelIndex index=model()->index(row,col);
	return model()->data(index).toString();
}

void Spreadsheet::setText(int row, int col, QString text) {
	QModelIndex index=model()->index(row,col);
	model()->setData(index,text);
}

void Spreadsheet::setNotes(QString t) {
	kDebug()<<endl;
	bool ok=true;
	if(t.isEmpty())
		t = KInputDialog::getMultiLineText("LabPlot", i18n("Spreadsheet notes : "), notes, &ok);
	if(!ok) return;

	notes = t;
}

QString Spreadsheet::columnHeader(int col) const {
//	kDebug()<<"Spreadsheet::columnheader("<<col<<")"<<endl;
	if(col<0) col=0;
	return model()->headerData(col,Qt::Horizontal).toString();
}

void Spreadsheet::setProperties(QString label, int type, int format) {
	if(label.isEmpty())
		(new ColumnDialog(mw,this))->show();
	else
		setColumnHeader(currentColumn(),label + ' ' + '{'+columnformatitems[format]+'}' + ' ' + '['+columntypeitems[type]+']');
}

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

QString Spreadsheet::columnName(int col) const {
	QString label = columnHeader(col);
	label.remove(QRegExp(" \\{.+\\]"));

	return label;
}

void Spreadsheet::setColumnName(int col, QString name) {
	kDebug()<<"column ="<<col<<", name = "<<name<<endl;
	if(col<0) return;
	QString header = columnHeader(col);
	header.replace(QRegExp(".*\\{"),name+" {");
	model()->setHeaderData(col,Qt::Horizontal,header);
}


QString Spreadsheet::columnType(int col) const {
	QString header = columnHeader(col);
	header.remove(QRegExp(".*\\["));
	header.remove(QRegExp("\\].*"));
	return header;
}

void Spreadsheet::setColumnType(int col, QString type) {
	kDebug()<<"column ="<<col<<",type = "<<type<<endl;
	if(col<0) return;
	QString label = columnHeader(col);
	label.replace(QRegExp(" \\[.+\\]"),QString(" ["+type+"]"));
	setColumnHeader(col, label);
}

QString Spreadsheet::columnFormat(int col) const {
	QString header = columnHeader(col);
	header.remove(QRegExp(".*\\{"));
	header.remove(QRegExp("\\}.*"));
	return header;
}

void Spreadsheet::setColumnFormat(int col, QString format) {
	kDebug()<<"column ="<<col<<",type = "<<format<<endl;
	if(col<0) return;
	QString label = columnHeader(col);
	label.replace(QRegExp(" \\{.+\\}"),QString(" {"+format+"}"));
	setColumnHeader(col, label);
}

int Spreadsheet::filledRows(int col) const {
	kDebug()<<"Spreadsheet::filledRows("<<col<<")"<<endl;
	if(col < 0 || col > columnCount())
		return -1;

	for(int row=rowCount()-1;row>=0;row--) {
		if(!text(row,col).isEmpty())
			return row;
	}
	return 0;
}

int Spreadsheet::currentColumn() const {
	QModelIndex index = currentIndex();
	return index.column();
}

QList<int> Spreadsheet::currentColumns() const {
	QList<int> columns;
	QModelIndexList list = selectionModel()->selectedColumns();
	for (int i=0; i<list.size(); i++)
		columns.append(list.at(i).column());
	return columns;
}

int Spreadsheet::currentRow() const {
	QModelIndex index = currentIndex();
	return index.row();
}

QList<int> Spreadsheet::currentRows() const {
	QList<int> rows;
	QModelIndexList list = selectionModel()->selectedRows();
	for (int i=0; i<list.size(); i++)
		rows.append(list.at(i).row());
	return rows;
}

void Spreadsheet::deleteSelectedColumns() {
	QList<int> columns = currentColumns();
	for (int i=0; i<columns.size(); i++) {
//		kDebug()<<"removing column ="<<columns.at(i)<<endl;
		model()->removeColumns(columns.at(i)-i,1);
	}
}

void Spreadsheet::deleteSelectedRows() {
	QList<int> rows = currentRows();
	for (int i=0; i<rows.size(); i++) {
//		kDebug()<<"removing row ="<<rows.at(i)<<endl;
		model()->removeRows(rows.at(i)-i,1);
	}
}

void Spreadsheet::addSet(const Set s) {
	kDebug()<<endl;
	m_set=s;

	int columns=0;
	switch(s.type()) {
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

	if(s.data.size() > rowCount()) //TODO do we really need this if here?
		setRowCount(s.data.size());

	this->displaySet();
}

void Spreadsheet::displaySet(){
//TODO clear the content of the table!!!

	int columns=this->columnCount();
	switch(m_set.type()) {
	case Set::SET2D: {
		kDebug()<<"set2d is going to be shown"<<endl;
 		const Point* point;
		for(int i=0; i<m_set.data.size(); i++) {
			point=&m_set.data.at(i);
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
		for(int i=0; i<m_set.data.size(); i++) {
			point=static_cast<const Point3D*>(&m_set.data.at(i));
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

	//mw->addSet(g,sheetcb->currentIndex(),PLOT2D);
	//TODO mw->addSet(g,1,PLOT2D);
}

void Spreadsheet::exportData() {
	kDebug()<<"not implemented yet!"<<endl;
	// TODO : export dialog
}

void Spreadsheet::setColumnValues() {
	kDebug()<<"not implemented yet!"<<endl;
	// TODO : column values dialog
}

void Spreadsheet::editFunction(){
	PlotType type;
	//TODO rethink/reimplement this approach
	if (m_set.type()==Set::SET2D)
		type=PLOT2D;
	else if (m_set.type()==Set::SET3D)
		type=PLOT3D;

	FunctionPlotDialog* dlg=new FunctionPlotDialog(mw, type);
	dlg->setSet(&m_set);
	if (dlg->exec())
		this->displaySet(); //TODO trigger update only if data set was changed
}

