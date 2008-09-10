//LabPlot : ColumnDialog.cc

#include <KDebug>
#include "ColumnDialog.h"
#include "../column.h"
#include "../MainWin.h"

ColumnDialog::ColumnDialog(MainWin *parent, Spreadsheet *s) : KDialog(parent), s(s) {
	kDebug()<<"ColumnDialog()"<<endl;

	setupGUI();
}

void ColumnDialog::setupGUI() {
	kDebug()<<endl;
	QWidget *widget = new QWidget(this);
	ui.setupUi(widget);
	
	ui.leLabel->setText(s->columnName(s->currentColumn()));
	ui.cbFormat->insertItems(0,columnformatitems);
	ui.cbFormat->setCurrentItem(s->columnFormat(s->currentColumn()));
	ui.cbType->insertItems(0,columntypeitems);
	ui.cbType->setCurrentItem(s->columnType(s->currentColumn()));

	setMainWidget(widget);
	setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Apply );
	setCaption(i18n("Column settings"));
	connect(this,SIGNAL(applyClicked()),SLOT(apply()));
	connect(this,SIGNAL(okClicked()),SLOT(apply()));
}

void ColumnDialog::apply() {
	int col = s->currentColumn();
	s->setColumnName(col,ui.leLabel->text());
	s->setColumnFormat(col,ui.cbFormat->currentText());
	s->setColumnType(col,ui.cbType->currentText());
}
