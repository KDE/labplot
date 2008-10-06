/***************************************************************************
    File                 : ProjectDialog.cc
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Stefan Gerlach
    Email (use @ for *)  : stefan.gerlach*uni-konstanz.de
    Description          : dialog for project settings
                           
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/
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

