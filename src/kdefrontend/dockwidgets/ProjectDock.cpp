/***************************************************************************
    File                 : ProjectDock.cpp
    Project              : LabPlot
    Description          : widget for project properties
    --------------------------------------------------------------------
    Copyright            : (C) 2012-2013 by Stefan Gerlach (stefan.gerlach@uni-konstanz.de)
    Copyright            : (C) 2013-2021 Alexander Semke (alexander.semke@web.de)

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

/*!
  \class ProjectDock
  \brief Provides a widget for editing the properties of a project

  \ingroup kdefrontend
*/

ProjectDock::ProjectDock(QWidget* parent) : BaseDock(parent) {
	ui.setupUi(this);
	m_leName = ui.leName;
	m_teComment = ui.teComment;
	m_teComment->setFixedHeight(m_leName->height());

	// SLOTS
	connect(ui.leName, &QLineEdit::textChanged, this, &ProjectDock::nameChanged);
	connect(ui.leAuthor, &QLineEdit::textChanged, this, &ProjectDock::authorChanged);
	connect(ui.teComment, &QTextEdit::textChanged, this, &ProjectDock::commentChanged);
}

void ProjectDock::setProject(Project* project) {
	m_project = project;
	m_aspect = project;

	Lock lock(m_initializing);
	ui.leFileName->setText(project->fileName());
	ui.leName->setStyleSheet("");
	ui.leName->setToolTip("");
	ui.leName->setText(m_project->name());
	ui.leAuthor->setText(m_project->author());
	ui.teComment->setText(m_project->comment());
	ui.lVersion->setText(project->version());
	ui.lCreated->setText(project->creationTime().toString());
	ui.lModified->setText(project->modificationTime().toString());

	connect(m_project, &Project::aspectDescriptionChanged, this, &ProjectDock::aspectDescriptionChanged);
	connect(m_project, &Project::authorChanged, this, &ProjectDock::projectAuthorChanged);
}

//************************************************************
//****************** SLOTS ********************************
//************************************************************
void ProjectDock::authorChanged() {
	if (m_initializing)
		return;

	m_project->setAuthor(ui.leAuthor->text());
}

//*************************************************************
//******** SLOTs for changes triggered in Project   ***********
//*************************************************************
void ProjectDock::projectAuthorChanged(const QString& author) {
	Lock lock(m_initializing);
	ui.leAuthor->setText(author);
}
