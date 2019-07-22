/***************************************************************************
File                 : DynamicPresenterWidget.cpp
Project              : LabPlot
Description          : Widget for dynamic presenting of worksheets
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
#include "DynamicPresenterWidget.h"
#include "commonfrontend/worksheet/WorksheetView.h"
#include "SlidingPanel.h"

#include <QKeyEvent>
#include <QPushButton>
#include <QScreen>
#include <QTimeLine>

#include <KLocalizedString>

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
	connect(m_panel->quitButton(), &QPushButton::clicked, this, &DynamicPresenterWidget::close);
	grabMouse();

	slideUp();
}

DynamicPresenterWidget::~DynamicPresenterWidget() {
	delete m_timeLine;
	delete m_view;
}

bool DynamicPresenterWidget::eventFilter(QObject* watched, QEvent* event) {
	Q_UNUSED(watched);
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
