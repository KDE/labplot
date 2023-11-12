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

#include <QContextMenuEvent>
#include <QMenu>
#include <QScreen>

/*!
  \class WorksheetPreviewWidget
  \brief A widget showing the preview of all worksheets in the project.

  \ingroup kdefrontend
*/
WorksheetPreviewWidget::WorksheetPreviewWidget(QWidget* parent)
	: QWidget(parent) {
	auto* layout = new QVBoxLayout(this);
	layout->setSpacing(0);
	layout->setContentsMargins(0, 0, 0, 0);

	QWidget* widget = new QWidget(this);
	ui.setupUi(widget);
	layout->addWidget(widget);

	connect(ui.lwPreview, &QListWidget::currentRowChanged, this, &WorksheetPreviewWidget::currentChanged);

	// make the icon 5x5cm big
	const int iconSize = std::ceil(5.0 / 2.54 * QApplication::primaryScreen()->physicalDotsPerInchX());
	ui.lwPreview->setIconSize(QSize(iconSize, iconSize));
}

WorksheetPreviewWidget::~WorksheetPreviewWidget() = default;

void WorksheetPreviewWidget::setProject(Project* project) {
	m_project = project;

	// clear the content of the previous project
	m_suppressNavigate = true;
	ui.lwPreview->clear();
	m_suppressNavigate = false;

	if (!m_project)
		return;

	connect(m_project, &Project::loaded, this, &WorksheetPreviewWidget::initPreview);
	connect(m_project, &Project::childAspectAdded, this, &WorksheetPreviewWidget::aspectAdded);
	connect(m_project, &Project::childAspectAboutToBeRemoved, this, &WorksheetPreviewWidget::aspectAboutToBeRemoved);
	// TODO: handle moving of worksheets
}

/*!
 * called when the project was completely loaded,
 * creates thumbnails for all available worksheets in the project.
 */
void WorksheetPreviewWidget::initPreview() {
	const auto& worksheets = m_project->children<Worksheet>(AbstractAspect::ChildIndexFlag::Recursive);
	for (int i = 0; i < worksheets.size(); ++i)
		addPreview(worksheets.at(i), i);
}

void WorksheetPreviewWidget::aspectAdded(const AbstractAspect* aspect) {
	if (m_project->isLoading())
		return;

	const auto* w = dynamic_cast<const Worksheet*>(aspect);
	if (!w)
		return;

	addPreview(w, indexOfWorksheet(w));
}

void WorksheetPreviewWidget::aspectSelected(const AbstractAspect* aspect) {
	const auto* w = dynamic_cast<const Worksheet*>(aspect);
	if (!w)
		return;

	m_suppressNavigate = true;
	ui.lwPreview->setCurrentRow(indexOfWorksheet(w));
	m_suppressNavigate = false;
}

void WorksheetPreviewWidget::aspectDeselected(const AbstractAspect* aspect) {
	const auto* w = dynamic_cast<const Worksheet*>(aspect);
	if (!w)
		return;

	// when switching between the different worksheets in the project explorer, we don't need
	// to clear the selection in the preview since it's changed in aspectSelected().
	// only clear the selection if the deselected worksheet is still being selected in the preview,
	// this is needed when switching from a worksheet to a non-worksheet aspect in the project explorer.
	if (ui.lwPreview->currentRow() == indexOfWorksheet(w)) {
		m_suppressNavigate = true;
		ui.lwPreview->setCurrentRow(-1);
		m_suppressNavigate = false;
	}
}

void WorksheetPreviewWidget::currentChanged(int index) {
	if (m_suppressNavigate)
		return;

	const auto& worksheets = m_project->children<Worksheet>(AbstractAspect::ChildIndexFlag::Recursive);
	const auto* worksheet = worksheets.at(index);
	m_project->requestNavigateTo(worksheet->path());
}

void WorksheetPreviewWidget::aspectAboutToBeRemoved(const AbstractAspect* aspect) {
	const auto* w = dynamic_cast<const Worksheet*>(aspect);
	if (!w)
		return;

	disconnect(w, nullptr, this, nullptr);
	ui.lwPreview->takeItem(indexOfWorksheet(w));
}

void WorksheetPreviewWidget::addPreview(const Worksheet* w, int row) const {
	QPixmap pix(10, 10);
	const bool rc = w->exportView(pix);
	if (!rc) {
		// the view is not available yet, show the placeholder preview
		const auto icon = QIcon::fromTheme(QLatin1String("view-preview"));
		const int iconSize = std::ceil(5.0 / 2.54 * QApplication::primaryScreen()->physicalDotsPerInchX());
		pix = icon.pixmap(iconSize, iconSize);
	}
	ui.lwPreview->insertItem(row, new QListWidgetItem(QIcon(pix), w->name()));

	connect(w, &Worksheet::aspectDescriptionChanged, this, &WorksheetPreviewWidget::updateText);
	connect(w, &Worksheet::changed, this, &WorksheetPreviewWidget::updatePreview);
	connect(w, &Worksheet::selected, this, &WorksheetPreviewWidget::aspectSelected);
	connect(w, &Worksheet::deselected, this, &WorksheetPreviewWidget::aspectDeselected);
}

void WorksheetPreviewWidget::updatePreview() {
	auto* w = dynamic_cast<Worksheet*>(QObject::sender());
	if (!w)
		return;

	QPixmap pix(10, 10);
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

void WorksheetPreviewWidget::contextMenuEvent(QContextMenuEvent*) {
	const int index = ui.lwPreview->currentRow();
	if (index == -1)
		return;

	const auto& worksheets = m_project->children<Worksheet>(AbstractAspect::ChildIndexFlag::Recursive);
	auto* worksheet = worksheets.at(index);
	auto* menu = worksheet->createContextMenu();
	menu->exec(QCursor::pos());
}

void WorksheetPreviewWidget::resizeEvent(QResizeEvent*) {
	if (width() > height())
		ui.lwPreview->setFlow(QListView::Flow::TopToBottom);
	else
		ui.lwPreview->setFlow(QListView::Flow::LeftToRight);
}
