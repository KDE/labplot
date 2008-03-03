//LabPlot : ColumnDialog.cc

#include "ColumnDialog.h"
#include "column.h"

ColumnDialog::ColumnDialog(MainWin *mw, Spreadsheet *s)
	: Dialog(mw), s(s)
{
	kdDebug()<<"ColumnDialog()"<<endl;
	setCaption(i18n("Column settings"));

	setupGUI();
	showButton(KDialog::User2,false);
	showButton(KDialog::User1,false);
	QObject::connect(this,SIGNAL(applyClicked()),SLOT(Apply()));
	QObject::connect(this,SIGNAL(okClicked()),SLOT(Apply()));
}

void ColumnDialog::setupGUI() {
	kdDebug()<<"ColumnDialog::setupGUI()"<<endl;
	layout->addWidget(new QLabel(i18n("Label : ")),0,0);
	labelle =  new KLineEdit(s->columnName(s->currentColumn()));
	layout->addWidget(labelle,0,1);

	layout->addWidget(new QLabel(i18n("Format : ")),1,0);
	formatcb = new KComboBox();
	layout->addWidget(formatcb,1,1);
	formatcb->insertItems(0,columnformatitems);
	formatcb->setCurrentItem(s->columnFormat(s->currentColumn()));
	
	layout->addWidget(new QLabel(i18n("Type : ")),2,0);
	typecb = new KComboBox();
	layout->addWidget(typecb,2,1);
	typecb->insertItems(0,columntypeitems);
	typecb->setCurrentItem(s->columnType(s->currentColumn()));
}

void ColumnDialog::Apply() {
	int col = s->currentColumn();
	s->setColumnName(col,labelle->text());
	s->setColumnType(col,typecb->currentText());
	s->setColumnFormat(col,formatcb->currentText());
}
