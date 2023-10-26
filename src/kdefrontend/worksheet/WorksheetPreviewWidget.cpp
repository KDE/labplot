/*
	File                 : WorksheetPreviewWidget.cpp
	Project              : LabPlot
	Description          : A widget showing the preview of all worksheets in the project
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "WorksheetPreviewWidget.h"
#include "backend/core/Project.h"

/*!
  \class WorksheetPreviewWidget
  \brief A widget showing the preview of all worksheets in the project.

  \ingroup kdefrontend
*/
WorksheetPreviewWidget::WorksheetPreviewWidget(QWidget* parent) {
	ui.setupUi(parent);
}

WorksheetPreviewWidget::~WorksheetPreviewWidget() {

}

void WorksheetPreviewWidget::setProject(Project* project) {
	m_project = project;

	connect(m_project, &Project::childAspectAdded, this, &WorksheetPreviewWidget::aspectAdded);
	connect(m_project, &Project::childAspectRemoved, this, &WorksheetPreviewWidget::aspectRemoved);
}

void WorksheetPreviewWidget::aspectAdded(const AbstractAspect* aspect) {
	if (aspect->type() != AspectType::Worksheet)
		return;
}

void WorksheetPreviewWidget::aspectRemoved(const AbstractAspect* aspect) {
	if (aspect->type() != AspectType::Worksheet)
		return;
}

