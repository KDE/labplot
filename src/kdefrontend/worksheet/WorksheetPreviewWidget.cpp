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
#include "backend/worksheet/Worksheet.h"

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
	ui.lwPreview->clear();
	if (!m_project)
		return;

	connect(m_project, &Project::childAspectAdded, this, &WorksheetPreviewWidget::aspectAdded);
	connect(m_project, &Project::childAspectAboutToBeRemoved, this, &WorksheetPreviewWidget::aspectAboutToBeRemoved);
	// TODO: handle moving of worksheets

	// add thumbnails for all available worksheets in the project
	const auto& worksheets = m_project->children<Worksheet>(AbstractAspect::ChildIndexFlag::Recursive);
	for (int i = 0; i < worksheets.size(); ++i)
		addPreview(worksheets.at(i), i);
}

void WorksheetPreviewWidget::aspectAdded(const AbstractAspect* aspect) {
	const auto* w = dynamic_cast<const Worksheet*>(aspect);
	if (!w)
		return;

	addPreview(w, indexOfWorksheet(w));
}

void WorksheetPreviewWidget::aspectAboutToBeRemoved(const AbstractAspect* aspect) {
	const auto* w = dynamic_cast<const Worksheet*>(aspect);
	if (!w)
		return;

	ui.lwPreview->takeItem(indexOfWorksheet(w));
}

void WorksheetPreviewWidget::addPreview(const Worksheet* w, int row) const {
	QPixmap pix;
	w->exportView(pix);
	ui.lwPreview->insertItem(row, new QListWidgetItem(QIcon(pix), w->name()));

	// TODO: connect(w, &Worksheet::changed, this, &WorksheetPreviewWidget::updatePreview);
}

void WorksheetPreviewWidget::updatePreview() {
	auto* w = dynamic_cast<Worksheet*>(QObject::sender());
	if (!w)
		return;

	QPixmap pix;
	w->exportView(pix);
	ui.lwPreview->item(indexOfWorksheet(w))->setIcon(QIcon(pix));
}

int WorksheetPreviewWidget::indexOfWorksheet(const Worksheet* w) const {
	const auto& worksheets = m_project->children<Worksheet>(AbstractAspect::ChildIndexFlag::Recursive);
	return worksheets.indexOf(const_cast<Worksheet*>(w));
}
