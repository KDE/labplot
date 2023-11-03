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

#include <QScreen>

/*!
  \class WorksheetPreviewWidget
  \brief A widget showing the preview of all worksheets in the project.

  \ingroup kdefrontend
*/
WorksheetPreviewWidget::WorksheetPreviewWidget(QWidget* parent) : QWidget(parent) {
	auto* layout = new QVBoxLayout(this);
	layout->setSpacing(0);
	layout->setContentsMargins(0, 0, 0, 0);

	QWidget* widget = new QWidget(this);
	ui.setupUi(widget);
	layout->addWidget(widget);

	connect(ui.lwPreview, &QListWidget::currentRowChanged, this, &WorksheetPreviewWidget::currentChanged);

	// make the icon 6x6cm big
	static const int themeIconSize = std::ceil(6.0 / 2.54 * QApplication::primaryScreen()->physicalDotsPerInchX());
	ui.lwPreview->setIconSize(QSize(themeIconSize, themeIconSize));
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

void WorksheetPreviewWidget::currentChanged(int index) {
	const auto& worksheets = m_project->children<Worksheet>(AbstractAspect::ChildIndexFlag::Recursive);
	const auto* worksheet = worksheets.at(index);
	m_project->requestNavigateTo(worksheet->path());
}

void WorksheetPreviewWidget::aspectAboutToBeRemoved(const AbstractAspect* aspect) {
	const auto* w = dynamic_cast<const Worksheet*>(aspect);
	if (!w)
		return;

	ui.lwPreview->takeItem(indexOfWorksheet(w));
}

void WorksheetPreviewWidget::addPreview(const Worksheet* w, int row) const {
	QPixmap pix(10, 10);
	w->exportView(pix);
	ui.lwPreview->insertItem(row, new QListWidgetItem(QIcon(pix), w->name()));

	connect(w, &Worksheet::aspectDescriptionChanged, this, &WorksheetPreviewWidget::updateText);
	connect(w, &Worksheet::changed, this, &WorksheetPreviewWidget::updatePreview);
}

void WorksheetPreviewWidget::updatePreview() {
	auto* w = dynamic_cast<Worksheet*>(QObject::sender());
	if (!w)
		return;

	QPixmap pix(10,10);
	w->exportView(pix);
	ui.lwPreview->item(indexOfWorksheet(w))->setIcon(QIcon(pix));
}

void WorksheetPreviewWidget::updateText() {
	auto* w = dynamic_cast<Worksheet*>(QObject::sender());
	if (!w)
		return;

	ui.lwPreview->item(indexOfWorksheet(w))->setText(w->name());
}

int WorksheetPreviewWidget::indexOfWorksheet(const Worksheet* w) const {
	const auto& worksheets = m_project->children<Worksheet>(AbstractAspect::ChildIndexFlag::Recursive);
	return worksheets.indexOf(const_cast<Worksheet*>(w));
}
