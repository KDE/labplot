//LabPlot : ExportDialog.cc

#include <KDebug>
#include "ExportDialog.h"
#include "../MainWin.h"

ExportDialog::ExportDialog(MainWin *parent) : KDialog(parent), mainWin(parent) {
	kDebug()<<endl;
	setCaption(i18n("Export Data"));

	setupGUI();
}

void ExportDialog::setupGUI() {
	kDebug()<<endl;
	exportWidget = new ExportWidget( this );
	setMainWidget( exportWidget );

	setButtons( KDialog::Ok | KDialog::User1 | KDialog::User2 | KDialog::Cancel | KDialog::Apply );
        setButtonText(KDialog::User1,i18n("Save"));
        setButtonText(KDialog::User2,i18n("Show Options"));

	connect(this,SIGNAL(applyClicked()),SLOT(apply()));
	connect(this,SIGNAL(okClicked()),SLOT(apply()));
	connect(this,SIGNAL(user1Clicked()),exportWidget,SLOT(save()));
	connect(this,SIGNAL(user2Clicked()),exportWidget,SLOT(toggleOptions()));

	resize( QSize(500,200) );
}
