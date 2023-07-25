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
#include "commonfrontend/worksheet/WorksheetView.h"

#include <QKeyEvent>
#include <QPushButton>
#include <QScreen>
#include <QTimeLine>

PresenterWidget::PresenterWidget(Worksheet* worksheet, bool interactive, QWidget* parent)
	: QWidget(parent)
	, m_worksheet(worksheet)
	, m_view(new WorksheetView(worksheet))
	, m_timeLine(new QTimeLine(600)) {
	setAttribute(Qt::WA_DeleteOnClose);
	setFocus();

	m_view->setParent(this);
	m_view->setInteractive(interactive);
	m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_view->setContextMenuPolicy(Qt::NoContextMenu);
	m_view->initActions(); // init the actions so we can also navigate in the plots

	const QRect& screenSize = screen()->geometry();
	m_view->setGeometry(screenSize); // use the full screen size for the view
	m_view->show();
	m_view->setFocus();

	m_panel = new SlidingPanel(this, worksheet->name());
	qApp->installEventFilter(this);
	connect(m_timeLine, &QTimeLine::valueChanged, m_panel, &SlidingPanel::movePanel);
	connect(m_panel->quitButton(), &QPushButton::clicked, this, [=]() {
		close();
	});
}

PresenterWidget::~PresenterWidget() {
	delete m_timeLine;
	delete m_view;

	// since the temporary view created in the presenter widget is using the same scene underneath,
	// the original view was also resized in the full screen mode if "view size" is used.
	// resize the original view once more to make sure it has the proper scaling after the presenter was closed.
	if (m_worksheet->useViewSize())
		static_cast<WorksheetView*>(m_worksheet->view())->processResize();
}

bool PresenterWidget::eventFilter(QObject* /*watched*/, QEvent* event) {
	if (event->type() == QEvent::MouseMove) {
		if (m_panel->y() != 0 && m_panel->rect().contains(QCursor::pos()))
			slideDown();
		else if (m_panel->y() == 0 && !m_panel->rect().contains(QCursor::pos()))
			slideUp();
	}

	return false;
}

void PresenterWidget::keyPressEvent(QKeyEvent* event) {
	if (event->key() == Qt::Key_Escape)
		close();
}

void PresenterWidget::slideDown() {
	m_timeLine->setDirection(QTimeLine::Forward);
	startTimeline();
}

void PresenterWidget::slideUp() {
	m_timeLine->setDirection(QTimeLine::Backward);
	startTimeline();
}

void PresenterWidget::startTimeline() {
	if (m_timeLine->state() != QTimeLine::Running)
		m_timeLine->start();
}

void PresenterWidget::focusOutEvent(QFocusEvent* e) {
	if (m_view->hasFocus())
		setFocus();

	if (e->reason() & Qt::BacktabFocusReason)
		close();
}
