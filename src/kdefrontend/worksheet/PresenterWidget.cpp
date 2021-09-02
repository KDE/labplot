/*
    File                 : PresenterWidget.cpp
    Project              : LabPlot
    Description          : Widget for static presenting of worksheets
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2016 Fabian Kristof <fkristofszabolcs@gmail.com>
    SPDX-FileCopyrightText: 2018-2020 Alexander Semke <alexander.semke@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "PresenterWidget.h"
#include "SlidingPanel.h"

#include <QApplication>
#include <QKeyEvent>
#include <QLabel>
#include <QPushButton>
#include <QScreen>
#include <QTimeLine>

PresenterWidget::PresenterWidget(const QPixmap &pixmap, const QString& worksheetName, QWidget *parent) : QWidget(parent),
	m_imageLabel(new QLabel(this)), m_timeLine(new QTimeLine(600)) {

	setAttribute(Qt::WA_DeleteOnClose);
	m_imageLabel->setPixmap(pixmap);
	m_imageLabel->adjustSize();

	const QRect& screenSize = QGuiApplication::primaryScreen()->availableGeometry();

	const int moveRight = (screenSize.width() - m_imageLabel->width()) / 2.0;
	const int moveDown = (screenSize.height() - m_imageLabel->height()) / 2.0;
	m_imageLabel->move(moveRight, moveDown);

	m_panel = new SlidingPanel(this, worksheetName);
	qApp->installEventFilter(this);
	connect(m_timeLine, &QTimeLine::valueChanged, m_panel, &SlidingPanel::movePanel);
	connect(m_panel->quitButton(), &QPushButton::clicked, this, [=]() {close();});
}

PresenterWidget::~PresenterWidget() {
	delete m_imageLabel;
	delete m_timeLine;
}

bool PresenterWidget::eventFilter(QObject* watched, QEvent* event) {
	Q_UNUSED(watched);
	if (event->type() == QEvent::MouseMove) {
		if (m_panel->y() != 0 && m_panel->rect().contains(QCursor::pos()))
			slideDown();
		else if (m_panel->y() == 0 && !m_panel->rect().contains(QCursor::pos()))
			slideUp();
	}

	return false;
}

void PresenterWidget::keyPressEvent(QKeyEvent *event) {
	if (event->key() == Qt::Key_Escape)
		close();
}

void PresenterWidget::focusOutEvent(QFocusEvent*) {
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
