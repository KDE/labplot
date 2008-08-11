//LabPlot : ImportDialog.cc

#include <KDebug>
#include "ImportDialog.h"
#include "../MainWin.h"

ImportDialog::ImportDialog(MainWin *parent) : KDialog(parent), mainWin(parent) {
	kDebug()<<"ImportDialog()"<<endl;
	setCaption(i18n("Import Data"));

	setupGUI();
}

void ImportDialog::setupGUI() {
	kDebug()<<"ImportDialog::setupGUI()"<<endl;
	importWidget = new ImportWidget( this );
	this->setMainWidget( importWidget );

	this->setButtons( KDialog::Ok | KDialog::User1 | KDialog::User2 | KDialog::Cancel | KDialog::Apply );
        setButtonText(KDialog::User1,i18n("Save"));
        setButtonText(KDialog::User2,i18n("Show Options"));

	connect(this,SIGNAL(applyClicked()),SLOT(apply()));
	connect(this,SIGNAL(okClicked()),SLOT(apply()));
	connect(this,SIGNAL(user1Clicked()),importWidget,SLOT(save()));
	connect(this,SIGNAL(user2Clicked()),importWidget,SLOT(toggleOptions()));

	resize( QSize(500,200) );
}
