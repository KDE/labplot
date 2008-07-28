#include "TitleDialog.h"
#include "LabelWidget.h"
#include "../MainWin.h"
#include <KDebug>

TitleDialog::TitleDialog(MainWin *mw, Label *label)	: KDialog(mw){

	this->setCaption(i18n("Title Dialog"));
	this->setWindowIcon( KIcon("draw-text") );
	mainWin=mw;
	title=label;

	labelWidget = new LabelWidget(this);
	labelWidget->setLabel(title);
	this->setMainWidget( labelWidget );
	this->setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Apply);

	connect( labelWidget, SIGNAL(dataWasChanged(bool)), SLOT(enableButtonApply(bool)) );
	connect( this, SIGNAL( applyClicked() ), this, SLOT( apply() ) );
	connect( this, SIGNAL( okClicked() ), this, SLOT( save() ) );

	this->enableButtonApply( false );
	resize( QSize(300,400) );
	kDebug()<<"Initialization done."<<endl;
}

void TitleDialog::save() {
	this->apply();
	this->close();
	kDebug()<<"Changes saved."<<endl;
}

void TitleDialog::apply() {
	labelWidget->saveLabel(title);

	//TODO rewrite/redesign
	Worksheet *w = mainWin->activeWorksheet();
	if(w != 0)
		w->repaint();

	this->enableButtonApply( false );
	kDebug()<<"Changes applied."<<endl;
}
