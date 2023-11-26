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

ProjectDock::ProjectDock(QWidget* parent)
	: BaseDock(parent) {
	ui.setupUi(this);
	setBaseWidgets(ui.leName, ui.teComment, 1.2);

	QString msg = i18n(
		"If checked, the results of the calculations in the analysis curves will be saved in the project file.\n"
		"Uncheck this option to reduce the size of the project file at costs of the longer project load times.");

	ui.lSaveCalculations->setToolTip(msg);
	ui.chkSaveCalculations->setToolTip(msg);

	// SLOTS
	connect(ui.leAuthor, &QLineEdit::textChanged, this, &ProjectDock::authorChanged);
	connect(ui.chkSaveCalculations, &QCheckBox::toggled, this, &ProjectDock::saveCalculationsChanged);
}

void ProjectDock::setProject(Project* project) {
	m_project = project;
	setAspects(QList<Project*>({project}));

	CONDITIONAL_LOCK_RETURN;
	ui.leFileName->setText(project->fileName());
	ui.leName->setStyleSheet(QString());
	ui.leName->setToolTip(QString());
	ui.leName->setText(m_project->name());
	ui.leAuthor->setText(m_project->author());

	ui.teComment->setText(m_project->comment());

	// resize the height of the comment field to fit the content (word wrap is ignored)
	const QFont& font = ui.teComment->document()->defaultFont();
	QFontMetrics fontMetrics(font);
	const QSize& textSize = fontMetrics.size(0, m_project->comment());
	double height = textSize.height() + 50;
	ui.teComment->setMinimumSize(0, height);
	ui.teComment->resize(ui.teComment->width(), height);

	ui.lVersion->setText(project->version());
	ui.lCreated->setText(project->creationTime().toString());
	ui.lModified->setText(project->modificationTime().toString());

	bool visible = !project->children<XYAnalysisCurve>(AbstractAspect::ChildIndexFlag::Recursive).isEmpty();
	ui.lSettings->setVisible(visible);
	ui.lineSettings->setVisible(visible);
	ui.lSaveCalculations->setVisible(visible);
	ui.chkSaveCalculations->setVisible(visible);
	ui.chkSaveCalculations->setChecked(project->saveCalculations());

	connect(m_project, &Project::authorChanged, this, &ProjectDock::projectAuthorChanged);
	connect(m_project, &Project::saveCalculationsChanged, this, &ProjectDock::projectSaveCalculationsChanged);
}

//************************************************************
//****************** SLOTS ********************************
//************************************************************
void ProjectDock::authorChanged() {
	CONDITIONAL_LOCK_RETURN;

	m_project->setAuthor(ui.leAuthor->text());
}

void ProjectDock::saveCalculationsChanged(bool state) {
	CONDITIONAL_LOCK_RETURN;

	m_project->setSaveCalculations(state);
}

//*************************************************************
//******** SLOTs for changes triggered in Project   ***********
//*************************************************************
void ProjectDock::projectAuthorChanged(const QString& author) {
	CONDITIONAL_LOCK_RETURN;
	ui.leAuthor->setText(author);
}

void ProjectDock::projectSaveCalculationsChanged(bool b) {
	CONDITIONAL_LOCK_RETURN;
	ui.chkSaveCalculations->setChecked(b);
}
