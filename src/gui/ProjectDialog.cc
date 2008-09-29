//LabPlot : ProjectDialog.cc

#include <KDebug>
#include "ProjectDialog.h"
#include "../MainWin.h"

ProjectDialog::ProjectDialog(MainWin *mw) : KDialog(mw) {
	kDebug()<<endl;
	setCaption(i18n("Project settings"));
	project = mw->getProject();

	setupGUI();
}

void ProjectDialog::setupGUI() {
	kDebug()<<endl;
	QWidget *widget = new QWidget(this);
	ui.setupUi(widget);
	setMainWidget(widget);

	setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Apply );
	connect(this,SIGNAL(applyClicked()),SLOT(apply()));
	connect(this,SIGNAL(okClicked()),SLOT(apply()));

	ui.lFileName->setText(project->filename());
	ui.lProjectVersion->setText(QString::number(project->version()));
	ui.lLabPlotVersion->setText(project->labPlot());
	ui.leTitle->setText(project->title());
	ui.leAuthor->setText(project->author());
	ui.dtwCreated->setDateTime(project->created());
	ui.dtwModified->setDateTime(project->modified());
	ui.tbNotes->setText(project->notes());
	ui.tbNotes->setReadOnly(false);
//	noteste->setTextFormat( Qt::PlainText );
}

void ProjectDialog::apply() {
	kDebug()<<endl;
	project->setTitle(ui.leTitle->text());
	project->setAuthor(ui.leAuthor->text());
	project->setNotes(ui.tbNotes->toPlainText());
	// not changable
//	project->setCreated(created->dateTime());
//	project->setModified(created->dateTime());
}

