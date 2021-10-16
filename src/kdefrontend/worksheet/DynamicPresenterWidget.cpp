/*
    File                 : DynamicPresenterWidget.cpp
    Project              : LabPlot
    Description          : Widget for dynamic presenting of worksheets
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2016 Fabian Kristof <fkristofszabolcs@gmail.com>
    SPDX-FileCopyrightText: 2018-2020 Alexander Semke <alexander.semke@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "DynamicPresenterWidget.h"
#include "commonfrontend/worksheet/WorksheetView.h"
#include "SlidingPanel.h"

#include <QKeyEvent>
#include <QPushButton>
#include <QScreen>
#include <QTimeLine>

DynamicPresenterWidget::DynamicPresenterWidget(Worksheet* worksheet, QWidget* parent) : QWidget(parent),
	m_view(new WorksheetView(worksheet)), m_timeLine(new QTimeLine(600)) {

	setAttribute(Qt::WA_DeleteOnClose);
	setFocus();

	m_view->setParent(this);
	m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	m_view->fitInView(m_view->sceneRect(), Qt::KeepAspectRatio);
	m_view->adjustSize();

	const QRect& screenSize = QGuiApplication::primaryScreen()->availableGeometry();

	const int moveRight = (screenSize.width() - m_view->width()) / 2.0;
	const int moveDown = (screenSize.height() - m_view->height()) / 2.0;
	m_view->move(moveRight, moveDown);
	m_view->show();

	m_panel = new SlidingPanel(this, worksheet->name());
	qApp->installEventFilter(this);
	connect(m_timeLine, &QTimeLine::valueChanged, m_panel, &SlidingPanel::movePanel);
	connect(m_panel->quitButton(), &QPushButton::clicked, this, [=]() {close();});
}

DynamicPresenterWidget::~DynamicPresenterWidget() {
	delete m_timeLine;
	delete m_view;
}

bool DynamicPresenterWidget::eventFilter(QObject* /*watched*/, QEvent* event) {
	if (event->type() == QEvent::MouseMove) {
		if (m_panel->y() != 0 && m_panel->rect().contains(QCursor::pos()))
			slideDown();
		else if (m_panel->y() == 0 && !m_panel->rect().contains(QCursor::pos()))
			slideUp();
	}

	return false;
}

void DynamicPresenterWidget::keyPressEvent(QKeyEvent *event) {
	if (event->key() == Qt::Key_Escape)
		close();
}

void DynamicPresenterWidget::slideDown() {
	m_timeLine->setDirection(QTimeLine::Forward);
	startTimeline();
}

void DynamicPresenterWidget::slideUp() {
	m_timeLine->setDirection(QTimeLine::Backward);
	startTimeline();
}

void DynamicPresenterWidget::startTimeline() {
	if (m_timeLine->state() != QTimeLine::Running)
		m_timeLine->start();
}

void DynamicPresenterWidget::focusOutEvent(QFocusEvent *e) {
	if (m_view->hasFocus())
		setFocus();

	if (e->reason() & Qt::BacktabFocusReason)
		close();
}
