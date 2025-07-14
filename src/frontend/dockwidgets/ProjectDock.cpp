/*
	File                 : ProjectDock.cpp
	Project              : LabPlot
	Description          : widget for project properties
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2012-2013 Stefan Gerlach <stefan.gerlach@uni-konstanz.de>
	SPDX-FileCopyrightText: 2013-2025 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ProjectDock.h"
#include "backend/core/Project.h"
#include "backend/worksheet/plots/cartesian/XYAnalysisCurve.h"

/*!
  \class ProjectDock
  \brief Provides a widget for editing the properties of a project

  \ingroup frontend
*/

ProjectDock::ProjectDock(QWidget* parent)
	: BaseDock(parent) {
	ui.setupUi(this);
	setBaseWidgets(ui.leName, ui.teComment);

	retranslateUi();

	// SLOTS
	connect(ui.leAuthor, &QLineEdit::textChanged, this, &ProjectDock::authorChanged);
	connect(ui.chkSaveDockStates, &QCheckBox::toggled, this, &ProjectDock::saveDockStatesChanged);
	connect(ui.chkSaveCalculations, &QCheckBox::toggled, this, &ProjectDock::saveCalculationsChanged);
	connect(ui.chkCompressFile, &QCheckBox::toggled, this, &ProjectDock::compressFileChanged);
}

void ProjectDock::setProject(Project* project) {
	CONDITIONAL_LOCK_RETURN;
	m_project = project;
	setAspects(QList<Project*>({project}));

	ui.leFileName->setText(project->fileName());
	ui.leAuthor->setText(m_project->author());

	ui.lVersion->setText(project->version());
	ui.lCreated->setText(project->creationTime().toString());
	ui.lModified->setText(project->modificationTime().toString());
	ui.chkSaveDockStates->setChecked(project->saveDefaultDockWidgetState());

	bool visible = !project->children<XYAnalysisCurve>(AbstractAspect::ChildIndexFlag::Recursive).isEmpty();
	ui.lSaveCalculations->setVisible(visible);
	ui.chkSaveCalculations->setVisible(visible);
	ui.chkSaveCalculations->setChecked(project->saveCalculations());

	ui.chkCompressFile->setChecked(project->fileCompression());

	// resize the height of the comment field to fit the content (word wrap is ignored)
	const double height = ui.teComment->document()->size().height() + ui.teComment->contentsMargins().top() * 2;
	// HACK: we set the fixed height first and then set the min and max values back to the default ones,
	// other methods don't seem to properly trigger the update of the layout and we don't get the proper
	// widgets sizes in the dock widget.
	ui.teComment->setFixedHeight(height);
	ui.teComment->setMinimumHeight(0);
	ui.teComment->setMaximumHeight(16777215);

	connect(m_project, &Project::authorChanged, this, &ProjectDock::projectAuthorChanged);
	connect(m_project, &Project::saveDefaultDockWidgetStateChanged, this, &ProjectDock::projectSaveDockStatesChanged);
	connect(m_project, &Project::saveCalculationsChanged, this, &ProjectDock::projectSaveCalculationsChanged);
}

void ProjectDock::retranslateUi() {
	QString msg = i18n("If checked, the state of the default application docks is saved in the project file and restored on project load.");
	ui.lSaveDockStates->setToolTip(msg);
	ui.chkSaveDockStates->setToolTip(msg);

	msg = i18n(
		"If checked, the results of the calculations in the analysis curves will be saved in the project file.\n"
		"Uncheck this option to reduce the size of the project file at costs of the longer project load times.");
	ui.lSaveCalculations->setToolTip(msg);
	ui.chkSaveCalculations->setToolTip(msg);
}

//************************************************************
//********************* SLOTS ********************************
//************************************************************
void ProjectDock::authorChanged() {
	CONDITIONAL_LOCK_RETURN;
	m_project->setAuthor(ui.leAuthor->text());
}

void ProjectDock::saveDockStatesChanged(bool state) {
	CONDITIONAL_LOCK_RETURN;
	m_project->setSaveDefaultDockWidgetState(state);
}

void ProjectDock::saveCalculationsChanged(bool state) {
	CONDITIONAL_LOCK_RETURN;
	m_project->setSaveCalculations(state);
}

void ProjectDock::compressFileChanged(bool compress) {
	CONDITIONAL_LOCK_RETURN;
	m_project->setFileCompression(compress);
}

//*************************************************************
//******** SLOTs for changes triggered in Project   ***********
//*************************************************************
void ProjectDock::projectAuthorChanged(const QString& author) {
	CONDITIONAL_LOCK_RETURN;
	ui.leAuthor->setText(author);
}

void ProjectDock::projectSaveDockStatesChanged(bool b) {
	CONDITIONAL_LOCK_RETURN;
	ui.chkSaveDockStates->setChecked(b);
}

void ProjectDock::projectSaveCalculationsChanged(bool b) {
	CONDITIONAL_LOCK_RETURN;
	ui.chkSaveCalculations->setChecked(b);
}
