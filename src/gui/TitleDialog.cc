#include "TitleDialog.h"
#include "LabelWidget.h"
#include "../MainWin.h"
#include <KDebug>

TitleDialog::TitleDialog(MainWin *mw, Label *label)	: KDialog(mw){

	setCaption(i18n("Title Dialog"));
	title=label;

	labelWidget = new LabelWidget(this);
	labelWidget->setLabel(title);
	this->setMainWidget( labelWidget );
	this->setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Apply);

	connect( this, SIGNAL( applyClicked() ), this, SLOT( apply() ) );
	connect( this, SIGNAL( okClicked() ), this, SLOT( save() ) );
	connect( this, SIGNAL( changed( bool ) ), this, SLOT( enableButtonApply( bool ) ) ); //TODO

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

	//TODO
	// mw->update()
// 	Worksheet *w = mw->activeWorksheet();
// 	if(w != 0)
// 		w->repaint();
	kDebug()<<"Changes applied."<<endl;
}
