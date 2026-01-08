/*
	File                 : WorksheetPreviewWidget.cpp
	Project              : LabPlot
	Description          : A widget showing the preview of all worksheets in the project
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023-2026 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "WorksheetPreviewWidget.h"
#include "backend/core/Project.h"
#include "backend/core/Settings.h"
#include "backend/lib/trace.h"
#include "backend/worksheet/Worksheet.h"

#include <QContextMenuEvent>
#include <QMenu>
#include <QScreen>
#include <QTimer>

#include <gsl/gsl_const_cgs.h>

#include <frontend/GuiTools.h>

/*!
  \class WorksheetPreviewWidget
  \brief A widget showing the preview of all worksheets in the project.

  \ingroup frontend
*/
WorksheetPreviewWidget::WorksheetPreviewWidget(QWidget* parent)
	: QWidget(parent) {
	auto* layout = new QVBoxLayout(this);
	layout->setSpacing(0);
	layout->setContentsMargins(0, 0, 0, 0);

	QWidget* widget = new QWidget(this);
	ui.setupUi(widget);
	layout->addWidget(widget);

	setMinimumSize(150, 150);

	connect(ui.lwPreview, &QListWidget::currentRowChanged, this, &WorksheetPreviewWidget::currentChanged);

	// make the icon 5x5cm big
	const auto& group = Settings::group(QStringLiteral("Settings_Worksheet"));
	m_iconSize = group.readEntry(QLatin1String("PreviewThumbnailSize"), 3); // size in cm
	m_iconSize = std::ceil(m_iconSize/ GSL_CONST_CGS_INCH * GuiTools::dpi(this).first); // size in pixel
	ui.lwPreview->setIconSize(QSize(m_iconSize, m_iconSize));
}

WorksheetPreviewWidget::~WorksheetPreviewWidget() = default;

void WorksheetPreviewWidget::setProject(Project* project) {
	// TODO: disconnect to all slots here doesn't seem to work, we still need nullptr checks in the slots.
	// disconnect(m_project, nullptr, this, nullptr);

	m_project = project;

	// clear the content of the previous project
	m_suppressNavigate = true;
	ui.lwPreview->clear();
	m_suppressNavigate = false;

	if (!m_project)
		return;

	connect(m_project, &Project::loaded, this, &WorksheetPreviewWidget::initPreview);
	connect(m_project, &Project::childAspectAdded, this, &WorksheetPreviewWidget::aspectAdded);
	connect(m_project, &Project::aboutToClose, this, &WorksheetPreviewWidget::projectAboutToClose);
}

void WorksheetPreviewWidget::updatePreviewSize() {
	const auto& group = Settings::group(QStringLiteral("Settings_Worksheet"));
	int newSize = group.readEntry(QLatin1String("PreviewThumbnailSize"), 3); // size in cm
	newSize = std::ceil(newSize/ GSL_CONST_CGS_INCH * GuiTools::dpi(this).first); // size in pixel

	if (newSize != m_iconSize) {
		m_iconSize = newSize;
		ui.lwPreview->setIconSize(QSize(m_iconSize, m_iconSize));
	}
}

void WorksheetPreviewWidget::projectAboutToClose() {
	m_suppressNavigate = true;
	ui.lwPreview->clear();
	m_suppressNavigate = false;
	m_dirtyPreviews.clear();

	m_project = nullptr;
}

/*!
 * called when the project was completely loaded,
 * creates thumbnails for all available worksheets in the project.
 */
void WorksheetPreviewWidget::initPreview() {
	if (!m_project)
		return;
	const auto& worksheets = m_project->children<Worksheet>(AbstractAspect::ChildIndexFlag::Recursive);
	for (int i = 0; i < worksheets.size(); ++i)
		addPreview(worksheets.at(i), i);
}

void WorksheetPreviewWidget::addPreview(const Worksheet* w, int row) const {
	QPixmap pix(10, 10);
	const bool rc = w->exportView(pix);
	if (!rc) {
		// the view is not available yet, show the placeholder preview
		const auto icon = QIcon::fromTheme(QLatin1String("view-preview"));
		pix = icon.pixmap(m_iconSize, m_iconSize);
	}
	ui.lwPreview->insertItem(row, new QListWidgetItem(QIcon(pix), w->name()));

	connect(w, &Worksheet::aspectDescriptionChanged, this, &WorksheetPreviewWidget::updateText);
	connect(w, &Worksheet::changed, this, &WorksheetPreviewWidget::changed);
	connect(w, &Worksheet::selected, this, &WorksheetPreviewWidget::aspectSelected);
	connect(w, &Worksheet::deselected, this, &WorksheetPreviewWidget::aspectDeselected);
	connect(w, &Worksheet::aspectAboutToBeRemoved, this, &WorksheetPreviewWidget::aspectAboutToBeRemoved);
	// TODO: handle moving of worksheets
}

//*************************************************************
//**************************** SLOTs *************************
//*************************************************************

/*!
 * called when the current item in the list view was changed,
 * triggers the navigation to the corresponding worksheet.
 */
void WorksheetPreviewWidget::currentChanged(int index) {
	if (m_suppressNavigate)
		return;

	const auto& worksheets = m_project->children<Worksheet>(AbstractAspect::ChildIndexFlag::Recursive);
	const auto* worksheet = worksheets.at(index);
	m_project->requestNavigateTo(worksheet->path());
}

void WorksheetPreviewWidget::aspectAdded(const AbstractAspect* aspect) {
	if (m_project->isLoading())
		return;

	const auto* worksheet = dynamic_cast<const Worksheet*>(aspect);
	if (worksheet)
		addPreview(worksheet, indexOfWorksheet(worksheet));
	else {
		// in case a folder was added (copy&paste, duplicate, project import), or another aspect that can have worksheet as a children (e.g. SeasonalDecomposition),
		// check whether the added aspect has worksheet children add previews for them
		QTimer::singleShot(0, this, [=]() {
			const auto& worksheets = aspect->children<Worksheet>(AbstractAspect::ChildIndexFlag::Recursive);
			for (const auto* w : worksheets)
				addPreview(w, indexOfWorksheet(w));
		});
	}
}

void WorksheetPreviewWidget::aspectSelected(const AbstractAspect* aspect) {
	if (!m_project)
		return;

	const auto* w = dynamic_cast<const Worksheet*>(aspect);
	if (!w)
		return;

	m_suppressNavigate = true;
	ui.lwPreview->setCurrentRow(indexOfWorksheet(w));
	m_suppressNavigate = false;
}

void WorksheetPreviewWidget::aspectDeselected(const AbstractAspect* aspect) {
	if (!m_project)
		return;

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

void WorksheetPreviewWidget::aspectAboutToBeRemoved(const AbstractAspect* aspect) {
	if (!m_project)
		return;

	const auto* w = dynamic_cast<const Worksheet*>(aspect);
	if (!w)
		return;

	disconnect(w, nullptr, this, nullptr);
	ui.lwPreview->takeItem(indexOfWorksheet(w));
	m_dirtyPreviews.remove(w);
}

/*!
 * called if one of the worksheets was changed/modified
 */
void WorksheetPreviewWidget::changed() {
	const auto* w = dynamic_cast<Worksheet*>(QObject::sender());
	if (!w)
		return;

	// don't update the preview if the preview widget was hidden,
	// delay the update to the point when the widget becomes visible again
	if (!isVisible()) {
		if (!m_dirtyPreviews.contains(w))
			m_dirtyPreviews << w;
		return;
	}

	updatePreview(w);
}

void WorksheetPreviewWidget::updateText() {
	auto* w = dynamic_cast<Worksheet*>(QObject::sender());
	if (!w)
		return;

	ui.lwPreview->item(indexOfWorksheet(w))->setText(w->name());
}

//*************************************************************
//*************** helper functions and events *****************
//*************************************************************
void WorksheetPreviewWidget::updatePreview(const Worksheet* w) {
	if (!w)
		return;

	PERFTRACE(QStringLiteral("WorksheetPreviewWidget::updatePreview ") + w->name());
	QPixmap pix(10, 10);
	const bool rc = w->exportView(pix);
	if (!rc) {
		// the view is not available yet, show the placeholder preview
		const auto icon = QIcon::fromTheme(QLatin1String("view-preview"));
		pix = icon.pixmap(m_iconSize, m_iconSize);
	}

	ui.lwPreview->item(indexOfWorksheet(w))->setIcon(QIcon(pix));
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

void WorksheetPreviewWidget::showEvent(QShowEvent* event) {
	// in case there were worksheets modified while the preview widget was hidden,
	// update the previews for them prior to showing the widget
	for (auto* w : m_dirtyPreviews)
		updatePreview(w);

	m_dirtyPreviews.clear();
	QWidget::showEvent(event);
}
