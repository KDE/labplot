//LabPlot : ProjectDialog.cc

#include "ProjectDialog.h"

ProjectDialog::ProjectDialog(MainWin *mw)
	: Dialog(mw)
{
	setCaption(i18n("Project settings"));
	project = mw->getProject();

	setupGUI();
	showButton(KDialog::User2,false);
	showButton(KDialog::User1,false);
	QObject::connect(this,SIGNAL(applyClicked()),SLOT(Apply()));
	QObject::connect(this,SIGNAL(okClicked()),SLOT(Apply()));
}

void ProjectDialog::setupGUI() {
	layout->addWidget(new QLabel(i18n("File name : ")),0,0);
	layout->addWidget(new QLabel(project->Filename()),0,1);

	layout->addWidget(new QLabel(i18n("Project version : ")),1,0);
	layout->addWidget(new QLabel(QString::number(project->Version())),1,1);
	layout->addWidget(new QLabel(i18n(" LabPlot version : ")),1,2);
	layout->addWidget(new QLabel(project->LabPlot()),1,3);
	layout->addWidget(new QLabel(i18n("Title : ")),2,0);
	layout->addWidget(titlele = new KLineEdit(project->Title()),2,1,1,3);
	layout->addWidget(new QLabel(i18n("Author : ")),3,0);
	layout->addWidget(authorle = new KLineEdit(project->Author()),3,1,1,3);

	layout->addWidget(new QLabel(i18n("Created : ")),4,0);
	layout->addWidget(created = new KDateTimeWidget(project->Created()),4,1,1,3);
	layout->addWidget(new QLabel(i18n("Last modified : ")),5,0);
	layout->addWidget(modified = new KDateTimeWidget(project->Modified()),5,1,1,3);

	layout->addWidget(new QLabel(i18n("Notes : ")),6,0);
	layout->addWidget(noteste = new QTextEdit(),7,0,3,4);
//	noteste->setTextFormat( Qt::PlainText );
	noteste->setText(project->Notes());
}

void ProjectDialog::Apply() {
	kDebug()<<"ProjectDialog::Apply()"<<endl;
	project->setTitle(titlele->text());
	project->setAuthor(authorle->text());
	project->setNotes(noteste->toPlainText());
	// not changable
//	project->setCreated(created->dateTime());
//	project->setModified(created->dateTime());

	mw->setProject(project);
}

