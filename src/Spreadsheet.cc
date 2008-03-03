// LabPlot : Spreadsheet.cc

#include <KLocale>
#include <KDebug>
#include <QInputDialog>
#include <QHeaderView>
#include <QStandardItemModel>
#include <KAction>

#include "Spreadsheet.h"
#include "MainWin.h"
#include "ColumnDialog.h"
#include "pixmaps/pixmap.h"
#include "column.h"

Spreadsheet::Spreadsheet(MainWin *m)
	:QTableWidget(), mw(m)
{
	kdDebug()<<"Spreadsheet()"<<endl;
	type = SPREADSHEET;
	setColumnCount(2);
	setRowCount(100);
	setSortingEnabled(false);

	resetHeader();
	
/*	ld=0;
	graph=0;
	gtype=GRAPH2D;

	// find free worksheet number
	QWidgetList list = mw->getWorkspace()->windowList();
	int nr=0, sheets = mw->NrWorksheets()+mw->NrSpreadsheets();
	bool found_free=false;
	kdDebug()<<"	Number of sheets "<<sheets<<endl;
	while(nr <= sheets && !found_free ) {
		nr++;
		found_free=true;
		for(int i=0;i<sheets;i++) {
			if(list.at(i) && list.at(i)->caption() == QString(i18n("Spreadsheet")+QString(" ")+QString::number(nr)))
				found_free=false;
		}
	}

	kdDebug()<<"	Using number "<<nr<<endl;
	title = i18n("Spreadsheet")+QString(" ")+QString::number(nr);
	setCaption(title);
*/
	setWindowTitle("Spreadsheet");
	// TODO : set unique title "Spreadsheet 1", etc.

/*	table->setRowMovingEnabled (true);
	table->setColumnMovingEnabled (true);
	table->horizontalHeader()->installEventFilter(this );

	table->setFocusPolicy(QWidget::StrongFocus);
	table->setFocus();
	QObject::connect(table,SIGNAL(currentChanged(int,int)),this,SLOT(updateValuesDialog()));
	QObject::connect(table,SIGNAL(selectionChanged()),this,SLOT(selectionChanged()));

	if(mw) mw->updateSheetList();

	destination=-1;
	values_dialog=0;
	datafile=QString("");
*/
}

void Spreadsheet::contextMenuEvent(QContextMenuEvent *) {
	QMenu *menu = new QMenu(this);
	Menu(menu);
	menu->exec(QCursor::pos());
}

void Spreadsheet::Menu(QMenu *menu) {
	kdDebug()<<"Spreadsheet::Menu()"<<endl;
	menu->clear();

	KAction *spreadaction = new KAction(KIcon(QIcon(spreadsheet_xpm)),i18n("New spreadsheet"),this);
	spreadaction->setShortcut(Qt::CTRL+Qt::Key_Equal);
	menu->addAction(spreadaction);
	// TODO
	// getting action from mw crashes
	// this crashes too
	//connect(spreadaction, SIGNAL(triggered()),mw, SLOT(newSpreadsheet()));
	// Doesn't work in main menu but in context menu
	connect(spreadaction, SIGNAL(triggered()),SLOT(newSpreadsheet()));

	// TODO
	KAction *action = new KAction(KIcon(QIcon(plot2d_xpm)),i18n("Plot"),this);
	action->setEnabled(false);
	menu->addAction(action);
	
	action = new KAction(KIcon("select"),i18n("Set title"),this);
	menu->addAction(action);
	connect(action, SIGNAL(triggered()),SLOT(setTitle()));
	action = new KAction(KIcon("select"),i18n("Set row count"),this);
	menu->addAction(action);
	connect(action, SIGNAL(triggered()),SLOT(setRowNumber()));
	action = new KAction(KIcon("select"),i18n("Column properties"),this);
	menu->addAction(action);
	connect(action, SIGNAL(triggered()),SLOT(setProperties()));
}

void Spreadsheet::newSpreadsheet() { mw->newSpreadsheet(); }

void Spreadsheet::setTitle(QString title) {
	kdDebug()<<"Spreadsheet::setTitle("<<title<<")"<<endl;
	bool ok=true;
	if(title.isEmpty())
		title = QInputDialog::getText(this,"LabPlot", i18n("Spreadsheet title : "),QLineEdit::Normal, windowTitle(), &ok);

	if(ok && !title.isEmpty()) {
		setWindowTitle(title);
		//mw->updateSheetList();
	}
}

void Spreadsheet::setRowNumber(int row) {
	kdDebug()<<"Spreadsheet::setRowNumber("<<row<<")"<<endl;
	bool ok=true;
	if(row<=0)
		row = QInputDialog::getInteger(this,"LabPlot", i18n("Row count : "), rowCount(), 1, INT_MAX, 1, &ok);

	if(ok && row>0) {
		setRowCount(row);
	}
}

void Spreadsheet::setNotes(QString t) {
	kdDebug()<<"Spreadsheet::setNotes()"<<endl;
	bool ok=true;
	if(t.isEmpty())
		t = QInputDialog::getText(this,"LabPlot", i18n("Spreadsheet notes : "),QLineEdit::Normal, notes, &ok);
	if(!ok) return;

	notes = t;
}

QString Spreadsheet::columnHeader(int col) {
//	kdDebug()<<"Spreadsheet::columnheader("<<col<<")"<<endl;
	if(col<0) col=0; 
	return model()->headerData(col,Qt::Horizontal).toString(); 
} 

void Spreadsheet::setColumnHeader(int col, QString name) {
	QTableWidgetItem *item = horizontalHeaderItem(col);
	if(item==0) {
		item = new QTableWidgetItem();
		setHorizontalHeaderItem(col,item);
	}
	item->setText(name);
}

void Spreadsheet::setProperties(QString label, int type, int format) {
	if(label.isEmpty())
		(new ColumnDialog(mw,this))->show();
	else
		setColumnHeader(currentColumn(),label + ' ' + '{'+columnformatitems[format]+'}' + ' ' + '['+columntypeitems[type]+']');
}

void Spreadsheet::resetHeader() {
	for (int col=0;col<columnCount();col++) {
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

QString Spreadsheet::columnName(int col) {
	QString label = columnHeader(col);
	label.remove(QRegExp(" \\{.+\\]"));

	return label;
}

void Spreadsheet::setColumnName(int col, QString name) {
	kdDebug()<<"setColumnName() : col="<<col<<" name = "<<name<<endl;
	if(col<0) return;
	QString header = columnHeader(col);
	header.replace(QRegExp(".*\\{"),name+" {");
	model()->setHeaderData(col,Qt::Horizontal,header);
}


QString Spreadsheet::columnType(int col) {
	QString header = columnHeader(col);
	header.remove(QRegExp(".*\\["));
	header.remove(QRegExp("\\].*"));
	return header;
}

void Spreadsheet::setColumnType(int col, QString type) {
	kdDebug()<<"setColumnType() : col="<<col<<" type = "<<type<<endl;
	if(col<0) return;
	QString label = columnHeader(col);
	label.replace(QRegExp(" \\[.+\\]"),QString(" ["+type+"]"));
	setColumnHeader(col, label);
}

QString Spreadsheet::columnFormat(int col) {
	QString header = columnHeader(col);
	header.remove(QRegExp(".*\\{"));
	header.remove(QRegExp("\\}.*"));
	return header;
}

void Spreadsheet::setColumnFormat(int col, QString format) {
	kdDebug()<<"setColumnFormat() : col="<<col<<" type = "<<format<<endl;
	if(col<0) return;
	QString label = columnHeader(col);
	label.replace(QRegExp(" \\{.+\\}"),QString(" {"+format+"}"));
	setColumnHeader(col, label);
}

void Spreadsheet::addSet(Set *s) {
	//kdDebug()<<"Spreadsheet::addSet()"<<endl;
	kdDebug()<<"Spreadsheet::addSet() not implemented yet!"<<endl;
	// TODO
}
