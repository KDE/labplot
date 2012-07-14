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

#include <KDebug>
#include "ProjectDialog.h"
#include "core/Project.h"

/*!
	\class ProjectDialog
 	\brief Provides a dialog for editing the general information about the project (Name, author etc.) .
 	 \ingroup kdefrontend
 */

ProjectDialog::ProjectDialog(QWidget* parent, Project* p) : KDialog(parent) {

	QWidget *widget = new QWidget(this);
	ui.setupUi(widget);
	setMainWidget(widget);
	setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Apply );

	connect(ui.leTitle, SIGNAL( textChanged(const QString&) ), this, SLOT( dataChanged() ) );
	connect(ui.leAuthor, SIGNAL( textChanged(const QString&) ), this, SLOT( dataChanged() ) );
	connect(ui.tbNotes, SIGNAL( textChanged() ), this, SLOT( dataChanged() ) );
	connect(this,SIGNAL(applyClicked()),SLOT(apply()));
	connect(this,SIGNAL(okClicked()),SLOT(ok()));

 	setCaption( i18n("Project info") );
	setWindowIcon( KIcon("help-about") );
	resize( QSize(100,200) );

	project =p;
 	showProjectInfo();
	m_dataChanged=false;
	enableButtonApply( false );
}

void ProjectDialog::showProjectInfo() const{
	ui.lFileName->setText(project->fileName());
	ui.lProjectVersion->setText(QString::number(project->version()));
	ui.lLabPlotVersion->setText(project->labPlot());
	ui.leTitle->setText(project->name());
	ui.leAuthor->setText(project->author());
	ui.dtwCreated->setDateTime(project->creationTime());
	ui.dtwModified->setDateTime(project->modificationTime());
	ui.tbNotes->setText(project->comment());
}

//SLOTS
void ProjectDialog::apply() {
	project->setName(ui.leTitle->text());
	project->setAuthor(ui.leAuthor->text());
	project->setComment(ui.tbNotes->toPlainText());

	// not changable
//	project->setCreated(created->dateTime());
//	project->setModified(created->dateTime());

	m_dataChanged=false;
	enableButtonApply( false );
}

void ProjectDialog::ok(){
	if (m_dataChanged)
		apply();
}

void ProjectDialog::dataChanged(){
	m_dataChanged=true;
	enableButtonApply( true );
}
