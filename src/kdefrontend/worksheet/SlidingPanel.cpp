/*
	File                 : SlidingPanel.cpp
	Project              : LabPlot
	Description          : Sliding panel shown in the presenter widget
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2016 Fabian Kristof <fkristofszabolcs@gmail.com>
	SPDX-FileCopyrightText: 2016-2023 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "SlidingPanel.h"

#include "commonfrontend/worksheet/WorksheetView.h"

#include <QAction>
#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScreen>
#include <QSize>
#include <QTimeLine>
#include <QToolBar>

#include <KLocalizedString>

SlidingPanel::SlidingPanel(const QRect& screenRect, Position position, QWidget* parent)
	: QFrame(parent)
	, m_screenRect(screenRect)
	, m_pos(position)
	, m_timeLine(new QTimeLine(400, this)) {
	setAttribute(Qt::WA_DeleteOnClose);
	connect(m_timeLine, &QTimeLine::valueChanged, this, &SlidingPanel::movePanel);
}

SlidingPanel::~SlidingPanel() {
}

void SlidingPanel::slideShow() {
	// timeline: 0 --> 1
	m_timeLine->setDirection(QTimeLine::Forward);
	startTimeline();
}

void SlidingPanel::slideHide() {
	// timeline: 1 --> 0
	m_timeLine->setDirection(QTimeLine::Backward);
	startTimeline();
}

void SlidingPanel::startTimeline() {
	if (m_timeLine->state() != QTimeLine::Running)
		m_timeLine->start();
}

void SlidingPanel::movePanel(qreal value) {
	switch (m_pos) {
	case Position::Top:
		move(pos().x(), -height() + static_cast<int>(value * height()));
		break;
	case Position::Bottom:
		move(pos().x(), m_screenRect.height() - static_cast<int>((1 - value) * height()));
		break;
	}
	raise();
}

bool SlidingPanel::insideRect(QPoint screenPos) {
	const auto leftTop = mapToGlobal(QPoint(rect().left(), rect().top()));
	const auto rightBottom = mapToGlobal(QPoint(rect().right(), rect().bottom()));
	const auto globalRect = QRect(leftTop.x(), leftTop.y(), rightBottom.x(), rightBottom.y());
	return globalRect.contains(mapToGlobal(screenPos));
}

// ####################################################################################################

SlidingPanelTop::SlidingPanelTop(const QRect& screenRect, const QString& worksheetName, QWidget* parent)
	: SlidingPanel(screenRect, SlidingPanel::Position::Top, parent) {
	setAttribute(Qt::WA_DeleteOnClose);

	m_worksheetName = new QLabel(worksheetName, this);
	QFont nameFont;
	nameFont.setPointSize(20);
	nameFont.setBold(true);
	m_worksheetName->setFont(nameFont);

	m_quitPresentingMode = new QPushButton(i18n("Quit Presentation"), this);
	m_quitPresentingMode->setIcon(QIcon::fromTheme(QLatin1String("window-close")));

	auto* hlayout = new QHBoxLayout;
	hlayout->addWidget(m_worksheetName);
	auto* spacer = new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Minimum);
	hlayout->addItem(spacer);
	hlayout->addWidget(m_quitPresentingMode);
	setLayout(hlayout);

	QPalette pal(palette());
	pal.setColor(QPalette::Window, Qt::lightGray);
	setAutoFillBackground(true);
	setPalette(pal);

	move(0, 0);
	raise();
	show();
}

QPushButton* SlidingPanelTop::quitButton() const {
	return m_quitPresentingMode;
}

QSize SlidingPanelTop::sizeHint() const {
	QSize sh;
	sh.setWidth(m_screenRect.width());
	sh.setHeight(m_worksheetName->sizeHint().height() + layout()->contentsMargins().top() + layout()->contentsMargins().bottom());

	return sh;
}

// ####################################################################################################
SlidingPanelBottom::SlidingPanelBottom(const QRect& screenRect, WorksheetView* view, bool fixed, QWidget* parent)
	: SlidingPanel(screenRect, SlidingPanel::Position::Bottom, parent)
	, m_toolBar(new QToolBar(this))
	, m_fixed(fixed) {
	auto* layout = new QHBoxLayout();
	layout->setContentsMargins(0, 0, 0, 0);
	setLayout(layout);

	view->fillCartesianPlotNavigationToolBar(m_toolBar, false /* disable cursor action */);

	// add an action to pin/unpin the panel (to make it fixed or floating)
	m_toolBar->addSeparator();
	auto* pinAction = new QAction(QIcon::fromTheme(QStringLiteral("pin")), i18n("Pin the navigation panel"));
	pinAction->setCheckable(true);
	pinAction->setChecked(m_fixed);
	connect(pinAction, &QAction::toggled, this, [=](bool toggled) {
		m_fixed = toggled;
	});
	m_toolBar->addAction(pinAction);

	layout->addWidget(m_toolBar);

	QPalette pal(palette());
	pal.setColor(QPalette::Window, Qt::lightGray);
	setAutoFillBackground(true);
	setPalette(pal);

	move(screenRect.width() / 2 - m_toolBar->sizeHint().width() / 2, screenRect.bottom() - m_toolBar->sizeHint().height());
	raise();
	show();
}

bool SlidingPanelBottom::isFixed() const {
	return m_fixed;
}

// TODO: why is SlidingPanel::insideRect() not enough?
bool SlidingPanelBottom::insideRect(QPoint screenPos) {
	QRect rect = this->rect();
	rect.moveTo(screen()->geometry().width() / 2 - m_toolBar->sizeHint().width() / 2, screen()->geometry().bottom() - rect.height());
	// Last part is a hack, because contains seems to be failing here
	const bool inside = rect.contains(screenPos) || screenPos.y() >= screen()->geometry().bottom();
	return inside;
}

QSize SlidingPanelBottom::sizeHint() const {
	return m_toolBar->sizeHint();
}
