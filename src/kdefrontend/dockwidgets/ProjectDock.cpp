/***************************************************************************
    File                 : ProjectDock.cpp
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2012 by Stefan Gerlach (stefan.gerlach*uni-konstanz.de)
    							(use @ for *)
    Description          : widget for project properties
                           
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

#include "ProjectDock.h"
#include "core/Project.h"
#include "../TemplateHandler.h"

/*!
  \class ProjectDock
  \brief  Provides a widget for editing the properties of a project

  \ingroup kdefrontend
*/

ProjectDock::ProjectDock(QWidget *parent): QWidget(parent){
	ui.setupUi(this);

	// SLOTS
	connect(ui.leTitle, SIGNAL( textChanged(const QString&) ), this, SLOT( titleChanged(const QString&) ) );
	connect(ui.leAuthor, SIGNAL( textChanged(const QString&) ), this, SLOT( authorChanged(const QString&) ) );
	connect(ui.tbComment, SIGNAL( textChanged() ), this, SLOT( commentChanged() ) );

	TemplateHandler* templateHandler = new TemplateHandler(this, TemplateHandler::Worksheet);
	ui.verticalLayout->addWidget(templateHandler, 0, 0);
	templateHandler->show();
	connect( templateHandler, SIGNAL(loadConfigRequested(KConfig&)), this, SLOT(loadConfig(KConfig&)));
	connect( templateHandler, SIGNAL(saveConfigRequested(KConfig&)), this, SLOT(saveConfig(KConfig&)));

	this->retranslateUi();
}

void ProjectDock::setProject(Project *project) {
	m_project = project;

	ui.lFileName->setText(project->fileName());
	ui.lProjectVersion->setText(QString::number(project->version()));
	ui.lLabPlotVersion->setText(project->labPlot());
	ui.leTitle->setText(project->name());
	ui.leAuthor->setText(project->author());
	ui.lCreated->setText(project->creationTime().toString());
	ui.lModified->setText(project->modificationTime().toString());
	ui.tbComment->setText(project->comment());

        //show default properties of a project
	KConfig config("", KConfig::SimpleConfig);
	loadConfig(config);

}

//************************************************************
//****************** SLOTS ********************************
//************************************************************
void ProjectDock::retranslateUi(){
}

void ProjectDock::titleChanged(const QString& title){
	if (m_initializing)
		return;

	m_project->setName(title);
}

void ProjectDock::authorChanged(const QString& author){
	if (m_initializing)
		return;

	m_project->setAuthor(author);
}

void ProjectDock::commentChanged(){
	if (m_initializing)
		return;

	m_project->setComment(ui.tbComment->toPlainText());
}

/*************************************************************/

void ProjectDock::loadConfig(KConfig& config){
	KConfigGroup group = config.group( "Project" );

	ui.leTitle->setText( group.readEntry("Title", m_project->name()) );
	ui.leAuthor->setText( group.readEntry("Author", m_project->author()) );
	ui.tbComment->setText( group.readEntry("Comment", m_project->comment()) );
}

void ProjectDock::saveConfig(KConfig& config){
	KConfigGroup group = config.group( "Project" );

	group.writeEntry("Title", ui.leTitle->text());
	group.writeEntry("Author", ui.leAuthor->text());
	group.writeEntry("Comment", ui.tbComment->toPlainText());
}
