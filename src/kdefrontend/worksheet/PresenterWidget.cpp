/***************************************************************************
File                 : PresenterWidget.cpp
Project              : LabPlot
Description          : Widget for static presenting of worksheets
--------------------------------------------------------------------
Copyright            : (C) 2016 by Fabian Kristof (fkristofszabolcs@gmail.com)
***************************************************************************/

/***************************************************************************
*                                                                         *
*  This program is free software; you can redistribute it and/or modify   *
*  it under the terms of the GNU General Public License as published by   *
*  the Free Software Foundation; either version 2 of the License, or      *
*  (at your option) any later version.                                    *
*                                                                         *
*  This program is distributed in the hope that it will be useful,        *
*  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
*  GNU General Public License for more details.                           *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the Free Software           *
*   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
*   Boston, MA  02110-1301  USA                                           *
*                                                                         *
***************************************************************************/
#include "PresenterWidget.h"
#include "SlidingPanel.h"

#include <QKeyEvent>
#include <QLabel>
#include <QDesktopWidget>
#include <QApplication>
#include <QTimeLine>
#include <QPushButton>

#include <KLocalizedString>

PresenterWidget::PresenterWidget(const QPixmap &pixmap, const QString& worksheetName, QWidget *parent) : QWidget(parent),
	m_imageLabel(new QLabel(this)),
	m_timeLine(new QTimeLine(600)) {
	setAttribute(Qt::WA_DeleteOnClose);
	m_imageLabel->setPixmap(pixmap);
	m_imageLabel->adjustSize();

	QDesktopWidget* const dw = QApplication::desktop();
	const int primaryScreenIdx = dw->primaryScreen();
	const QRect& screenSize = dw->availableGeometry(primaryScreenIdx);

	const int moveRight = (screenSize.width() - m_imageLabel->width()) / 2.0;
	const int moveDown = (screenSize.height() - m_imageLabel->height()) / 2.0;
	m_imageLabel->move(moveRight, moveDown);

	m_panel = new SlidingPanel(this, worksheetName);
	qApp->installEventFilter(this);
	connect(m_timeLine, SIGNAL(valueChanged(qreal)), m_panel, SLOT(movePanel(qreal)));
	connect(m_panel->m_quitPresentingMode, SIGNAL(clicked(bool)), this, SLOT(close()));

	slideUp();
	setFocus();
}

PresenterWidget::~PresenterWidget() {
	delete m_imageLabel;
	delete m_timeLine;
}

bool PresenterWidget::eventFilter(QObject *watched, QEvent *event) {
	if (event->type() == QEvent::MouseMove) {
		foreach (const QObject* ob, m_panel->children()) {
			if (watched == ob)
				return false;
		}
		if (qobject_cast<SlidingPanel*>(watched) == m_panel)
			return false;
		if (!m_panel->shouldHide())
			slideDown();
		else if (m_panel->y() == 0) {
			if (m_panel->shouldHide())
				slideUp();
		}
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
