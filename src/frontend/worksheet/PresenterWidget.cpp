/*
	File                 : PresenterWidget.cpp
	Project              : LabPlot
	Description          : Widget for dynamic presenting of worksheets
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2016 Fabian Kristof <fkristofszabolcs@gmail.com>
	SPDX-FileCopyrightText: 2018-2023 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "PresenterWidget.h"
#include "SlidingPanel.h"
#include "backend/core/Settings.h"
#include "frontend/worksheet/WorksheetView.h"

#include <KConfigGroup>
#include <QKeyEvent>
#include <QPushButton>
#include <QScreen>

PresenterWidget::PresenterWidget(Worksheet* worksheet, QScreen* screen, bool interactive, QWidget* parent)
	: QWidget(parent)
	, m_worksheet(worksheet)
	, m_view(new WorksheetView(worksheet)) {
	setAttribute(Qt::WA_DeleteOnClose);
	setFocus();

	m_view->setParent(this);
	m_view->setInteractive(interactive);
	m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_view->setContextMenuPolicy(Qt::NoContextMenu);
	m_view->initPlotNavigationActions(); // init the relevant actions so we can also navigate in the plots in the presenter mode

	const QRect& screenSize = screen->geometry();
	m_view->setGeometry(screenSize); // use the full screen size for the view
	m_view->show();
	m_view->setFocus();

	m_panel = new SlidingPanelTop(screenSize, worksheet->name(), this);
	qApp->installEventFilter(this);
	connect(m_panel->quitButton(), &QPushButton::clicked, this, [=]() {
		close();
	});

	if (interactive) {
		const auto group = Settings::group(QStringLiteral("PresenterWidget"));
		const auto fixed = group.readEntry("PresenterWidgetNavigationPanelFixed", false);
		m_navigationPanel = new SlidingPanelBottom(screenSize, m_view, fixed, this);
	}
}

PresenterWidget::~PresenterWidget() {
	delete m_view;

	// since the temporary view created in the presenter widget is using the same scene underneath,
	// the original view was also resized in the full screen mode if "view size" is used.
	// resize the original view once more to make sure it has the proper scaling after the presenter was closed.
	if (m_worksheet->useViewSize())
		static_cast<WorksheetView*>(m_worksheet->view())->processResize();

	if (m_navigationPanel) {
		// save current settings for the navigation panel
		auto group = Settings::group(QStringLiteral("PresenterWidget"));
		group.writeEntry("PresenterWidgetNavigationPanelFixed", m_navigationPanel->isFixed());
	}
}

bool PresenterWidget::eventFilter(QObject* /*watched*/, QEvent* event) {
	if (event->type() == QEvent::MouseMove) {
		bool visible = m_panel->y() == 0;
		const auto pos = QCursor::pos();
		if (!visible && m_panel->insideRect(pos))
			m_panel->slideShow();
		else if (visible && !m_panel->insideRect(pos))
			m_panel->slideHide();

		if (m_navigationPanel && !m_navigationPanel->isFixed()) {
			visible = m_navigationPanel->y() < screen()->geometry().bottom();
			if (!visible && m_navigationPanel->insideRect(pos))
				m_navigationPanel->slideHide();
			else if (visible && !m_navigationPanel->insideRect(pos))
				m_navigationPanel->slideShow();
		}
	}

	return false;
}

void PresenterWidget::keyPressEvent(QKeyEvent* event) {
	if (event->key() == Qt::Key_Escape)
		close();
}

void PresenterWidget::focusOutEvent(QFocusEvent* e) {
	if (m_view->hasFocus())
		setFocus();

	if (e->reason() & Qt::BacktabFocusReason)
		close();
}
