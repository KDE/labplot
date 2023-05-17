/*
	File                 : PartMdiView.cpp
	Project              : LabPlot
	Description          : QMdiSubWindow wrapper for aspect views.
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2013-2019 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2007, 2008 Tilman Benkert <thzs@gmx.net>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "commonfrontend/core/ContentDockWidget.h"
#include "backend/core/AbstractPart.h"
#include "backend/worksheet/Worksheet.h"

#include <DockWidget.h>

#include <QCloseEvent>
#include <QIcon>
#include <QUuid>

/*!
 * \class PartMdiView
 *
 * \brief QMdiSubWindow wrapper for aspect views.
 *
 * In addition to the functionality provided by QMdiSubWindow,
 * this class automatically updates the window title when AbstractAspect::caption() is changed
 * and holds the connection to the actual data visualized in this window via the pointer to \c AbstractPart.
 */
ContentDockWidget::ContentDockWidget(AbstractPart* part)
	: ads::CDockWidget(part->name())
	, m_part(part) {
	setWindowIcon(m_part->icon());
	setWidget(m_part->view());
	setAttribute(Qt::WA_DeleteOnClose);
	handleAspectDescriptionChanged(m_part);

	// resize the MDI sub window to fit the content of the view
	resize(m_part->view()->size());
	// Must be unique and must not be changed after the dock was added to the dockmanager, because
	// the objectname is used in the content manager map
	setObjectName(m_part->uuid().toString());

	connect(m_part, &AbstractPart::aspectDescriptionChanged, this, &ContentDockWidget::handleAspectDescriptionChanged);
	connect(m_part, &AbstractPart::aspectAboutToBeRemoved, this, &ContentDockWidget::handleAspectAboutToBeRemoved);
	// connect(this, &QMdiSubWindow::windowStateChanged, this, &ContentDockWidget::slotWindowStateChanged);
}

ContentDockWidget::~ContentDockWidget() {
	m_closing = true;
}

AbstractPart* ContentDockWidget::part() const {
	return m_part;
}

void ContentDockWidget::handleAspectDescriptionChanged(const AbstractAspect* aspect) {
	if (aspect != m_part)
		return;

	setWindowTitle(m_part->name());
}

void ContentDockWidget::handleAspectAboutToBeRemoved(const AbstractAspect* aspect) {
	if (aspect != m_part)
		return;

	close();
}

void ContentDockWidget::closeEvent(QCloseEvent* event) {
	m_part->deleteView();
	event->accept();
}

void ContentDockWidget::slotWindowStateChanged(Qt::WindowStates /*oldState*/, Qt::WindowStates newState) {
	if (m_closing)
		return;

	if (newState.testFlag(Qt::WindowActive) || newState.testFlag(Qt::WindowMaximized))
		m_part->registerShortcuts();
	else
		m_part->unregisterShortcuts();
}
