/***************************************************************************
    File                 : ProjectDock.cpp
    Project              : LabPlot
    Description          : widget for project properties
    --------------------------------------------------------------------
    Copyright            : (C) 2012-2013 by Stefan Gerlach (stefan.gerlach@uni-konstanz.de)
    Copyright            : (C) 2013 Alexander Semke (alexander.semke@web.de)

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
#include "backend/core/Project.h"
#include "kdefrontend/TemplateHandler.h"
#include <KConfigGroup>
#include <KConfig>

/*!
  \class ProjectDock
  \brief Provides a widget for editing the properties of a project

  \ingroup kdefrontend
*/

ProjectDock::ProjectDock(QWidget *parent): BaseDock(parent) {
	ui.setupUi(this);
	m_leName = ui.leName;
	// leComment = ui.tbComment; // not a qlineedit

	// SLOTS
	connect(ui.leName, &QLineEdit::textChanged, this, &ProjectDock::nameChanged);
	connect(ui.leAuthor, &QLineEdit::textChanged, this, &ProjectDock::authorChanged);
	connect(ui.tbComment, &QTextBrowser::textChanged, this, &ProjectDock::commentChanged);

	auto* templateHandler = new TemplateHandler(this, TemplateHandler::ClassName::Worksheet);
	ui.verticalLayout->addWidget(templateHandler);
	templateHandler->show();
	connect(templateHandler, &TemplateHandler::loadConfigRequested, this, &ProjectDock::loadConfig);
	connect(templateHandler, &TemplateHandler::saveConfigRequested, this, &ProjectDock::saveConfig);

	this->retranslateUi();
}

void ProjectDock::setProject(Project *project) {
	m_initializing = true;
	m_project = project;
	m_aspect = project;
	ui.leFileName->setText(project->fileName());
	ui.leName->setStyleSheet("");
	ui.leName->setToolTip("");
	ui.lVersion->setText(project->version());
	ui.lCreated->setText(project->creationTime().toString());
	ui.lModified->setText(project->modificationTime().toString());

	//show default properties of the project
	KConfig config(QString(), KConfig::SimpleConfig);
	loadConfig(config);

	connect(m_project, &Project::aspectDescriptionChanged, this, &ProjectDock::projectDescriptionChanged);

	m_initializing = false;
}

//************************************************************
//****************** SLOTS ********************************
//************************************************************
void ProjectDock::retranslateUi() {
}

void ProjectDock::authorChanged() {
	if (m_initializing)
		return;

	m_project->setAuthor(ui.leAuthor->text());
}

void ProjectDock::commentChanged() {
	if (m_initializing)
		return;

	m_project->setComment(ui.tbComment->toPlainText());
}

//*************************************************************
//******** SLOTs for changes triggered in Project   ***********
//*************************************************************
void ProjectDock::projectDescriptionChanged(const AbstractAspect* aspect) {
	if (m_project != aspect)
		return;

	m_initializing = true;
	if (aspect->name() != ui.leName->text())
			ui.leName->setText(aspect->name());
	else if (aspect->comment() != ui.tbComment->toPlainText())
			ui.tbComment->setText(aspect->comment());

	m_initializing = false;
}

//*************************************************************
//************************* Settings **************************
//*************************************************************
void ProjectDock::loadConfig(KConfig& config) {
	KConfigGroup group = config.group( "Project" );

	ui.leName->setText( group.readEntry("Name", m_project->name()) );
	ui.leAuthor->setText( group.readEntry("Author", m_project->author()) );
	ui.tbComment->setText( group.readEntry("Comment", m_project->comment()) );
}

void ProjectDock::saveConfig(KConfig& config) {
	KConfigGroup group = config.group( "Project" );

	group.writeEntry("Name", ui.leName->text());
	group.writeEntry("Author", ui.leAuthor->text());
	group.writeEntry("Comment", ui.tbComment->toPlainText());
}
