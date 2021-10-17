/*
    File                 : ProjectDock.cpp
    Project              : LabPlot
    Description          : widget for project properties
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2012-2013 Stefan Gerlach <stefan.gerlach@uni-konstanz.de>
    SPDX-FileCopyrightText: 2013-2021 Alexander Semke <alexander.semke@web.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ProjectDock.h"
#include "backend/core/Project.h"
#include "backend/worksheet/plots/cartesian/XYAnalysisCurve.h"

/*!
  \class ProjectDock
  \brief Provides a widget for editing the properties of a project

  \ingroup kdefrontend
*/

ProjectDock::ProjectDock(QWidget* parent) : BaseDock(parent) {
	ui.setupUi(this);
	m_leName = ui.leName;
	m_teComment = ui.teComment;
	m_teComment->setFixedHeight(1.2 * m_leName->height());

	QString msg = i18n("If checked, the results of the calculations in the analysis curves will be saved in the project file.\n"
	"Uncheck this option to reduce the size of the project file at costs of the longer project load times.");

	ui.lSaveCalculations->setToolTip(msg);
	ui.chkSaveCalculations->setToolTip(msg);

	// SLOTS
	connect(ui.leName, &QLineEdit::textChanged, this, &ProjectDock::nameChanged);
	connect(ui.leAuthor, &QLineEdit::textChanged, this, &ProjectDock::authorChanged);
	connect(ui.teComment, &QTextEdit::textChanged, this, &ProjectDock::commentChanged);
	connect(ui.chkSaveCalculations, &QCheckBox::stateChanged, this, &ProjectDock::saveCalculationsChanged);
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

	bool visible = !project->children<XYAnalysisCurve>(AbstractAspect::ChildIndexFlag::Recursive).isEmpty();
	ui.lSettings->setVisible(visible);
	ui.lineSettings->setVisible(visible);
	ui.lSaveCalculations->setVisible(visible);
	ui.chkSaveCalculations->setVisible(visible);
	ui.chkSaveCalculations->setChecked(project->saveCalculations());

	connect(m_project, &Project::aspectDescriptionChanged, this, &ProjectDock::aspectDescriptionChanged);
	connect(m_project, &Project::authorChanged, this, &ProjectDock::projectAuthorChanged);
	connect(m_project, &Project::saveCalculationsChanged, this, &ProjectDock::projectSaveCalculationsChanged);
}

//************************************************************
//****************** SLOTS ********************************
//************************************************************
void ProjectDock::authorChanged() {
	if (m_initializing)
		return;

	m_project->setAuthor(ui.leAuthor->text());
}

void ProjectDock::saveCalculationsChanged() {
	if (m_initializing)
		return;

	m_project->setSaveCalculations(ui.chkSaveCalculations->isChecked());
}

//*************************************************************
//******** SLOTs for changes triggered in Project   ***********
//*************************************************************
void ProjectDock::projectAuthorChanged(const QString& author) {
	Lock lock(m_initializing);
	ui.leAuthor->setText(author);
}

void ProjectDock::projectSaveCalculationsChanged(bool b) {
	const Lock lock(m_initializing);
	ui.chkSaveCalculations->setChecked(b);
}
