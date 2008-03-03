//LabPlot : FileInfoDialog.cc

#include "FileInfoDialog.h"

FileInfoDialog::FileInfoDialog(MainWin *mw, QString filename)
	: Dialog(mw)
{
	kdDebug()<<"FileInfoDialog()"<<endl;
	setCaption(i18n("File Info")+i18n(" : ")+filename);

	layout->addWidget(new QLabel(i18n("Not implemented yet!")),1,0);

	showButton(KDialog::User1,false);
	showButton(KDialog::Apply,false);
	showButton(KDialog::Cancel,false);
}

