//LabPlot : TitleDialog.cc

#include "TitleDialog.h"

TitleDialog::TitleDialog(MainWin *mw, Label *title)
	: Dialog(mw), title(title)
{
	kdDebug()<<"TitleDialog()"<<endl;
	if(title != 0) {
		kdDebug()<<"	title="<<title->Text()<<endl;
	}
	setCaption(i18n("Title"));

	QWidget *widget = new QWidget;
	labelWidget(widget, title);
	layout->addWidget(widget);

	showButton(KDialog::User1,false);
	showButton(KDialog::User2,false);
        QObject::connect(this,SIGNAL(applyClicked()),SLOT(Apply()));
        QObject::connect(this,SIGNAL(okClicked()),SLOT(Apply()));
        QObject::connect(this,SIGNAL(user1Clicked()),SLOT(saveSettings()));
}

void TitleDialog::saveSettings() {
	// TODO
	// save as title ?, use Label::saveSettings() ?
}

void TitleDialog::Apply() {
	// TODO : set title settings
	// use Dialog::setupLabel() !
	setupLabel(title);

	// mw->update()
	Worksheet *w = mw->activeWorksheet();
	if(w != 0)
		w->repaint();
}
