/*
	File                 : SlidingPanel.cpp
	Project              : LabPlot
	Description          : Sliding panel shown in the presenter widget
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2016 Fabian Kristof <fkristofszabolcs@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "SlidingPanel.h"

#include "commonfrontend/worksheet/WorksheetView.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScreen>
#include <QSize>
#include <QTimeLine>
#include <QToolButton>
#include <QAction>

#include <KLocalizedString>

SlidingPanel::SlidingPanel(const QRect& screenRect, Position position, QWidget* parent)
	: QFrame(parent)
	, m_pos(position)
	, m_screenRect(screenRect)
	, m_timeLine(new QTimeLine(600, this)) {
	setAttribute(Qt::WA_DeleteOnClose);
	connect(m_timeLine, &QTimeLine::valueChanged, this, &SlidingPanel::movePanel);
}

SlidingPanel::~SlidingPanel() {
}

void SlidingPanel::slideDown() {
	// timeline: 0 --> 1
	m_timeLine->setDirection(QTimeLine::Forward);
	startTimeline();
}

void SlidingPanel::slideUp() {
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
	pal.setColor(QPalette::Window, Qt::gray);
	setAutoFillBackground(true);
	setPalette(pal);

	move(0, 0);
	raise();
	show();
}

bool SlidingPanelTop::insideRect(QPoint screenPos) {
	return rect().contains(screenPos);
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
SlidingPanelBottom::SlidingPanelBottom(const QRect& screenRect, WorksheetView* view, QWidget* parent)
	: SlidingPanel(screenRect, SlidingPanel::Position::Bottom, parent) {
	m_hlayout = new QHBoxLayout;

	addButtonToLayout(CartesianPlot::NavigationOperation::ScaleAuto, m_hlayout, view);
	addButtonToLayout(CartesianPlot::NavigationOperation::ScaleAutoX, m_hlayout, view);
	addButtonToLayout(CartesianPlot::NavigationOperation::ScaleAutoY, m_hlayout, view);
	addButtonToLayout(CartesianPlot::NavigationOperation::ZoomIn, m_hlayout, view);
	addButtonToLayout(CartesianPlot::NavigationOperation::ZoomOut, m_hlayout, view);
	addButtonToLayout(CartesianPlot::NavigationOperation::ZoomInX, m_hlayout, view);
	addButtonToLayout(CartesianPlot::NavigationOperation::ZoomOutX, m_hlayout, view);
	addButtonToLayout(CartesianPlot::NavigationOperation::ZoomInY, m_hlayout, view);
	addButtonToLayout(CartesianPlot::NavigationOperation::ZoomOutY, m_hlayout, view);
	addButtonToLayout(CartesianPlot::NavigationOperation::ShiftLeftX, m_hlayout, view);
	addButtonToLayout(CartesianPlot::NavigationOperation::ShiftRightX, m_hlayout, view);
	addButtonToLayout(CartesianPlot::NavigationOperation::ShiftUpY, m_hlayout, view);
	addButtonToLayout(CartesianPlot::NavigationOperation::ShiftDownY, m_hlayout, view);

	setLayout(m_hlayout);

	QPalette pal(palette());
	pal.setColor(QPalette::Window, Qt::gray);
	setAutoFillBackground(true);
	setPalette(pal);

	move(screenRect.width() / 2 - m_hlayout->sizeHint().width() / 2, screenRect.bottom());
	raise();
	show();
}

class ToolButton: public QToolButton {
public:
	ToolButton(QWidget *parent = nullptr): QToolButton(parent) {

	}
	void setDefaultAction(QAction *action) {

// Works fine, triggers actionChanged
//		action->setVisible(false);
//		action->setVisible(true);
//		action->setChecked(false);
//		action->setChecked(true);
//		action->setEnabled(false);
//		action->setEnabled(true);

		if (action) {
			connect(action, &QAction::changed, this, &ToolButton::actionChanged);
			//connect(action, &QAction::triggered, this, &ToolButton::actionChanged);
		}
		QToolButton::setDefaultAction(action);
	}

	void actionChanged() {
		// TODO: for some reason it will not be called when action changes properties!
		// To trigger that
		// setCheckable(action->isCheckable());
		// setChecked(action->isChecked());
		// setEnabled(action->isEnabled());
		// will be executed
		QToolButton::setDefaultAction(defaultAction());
	}

};

void SlidingPanelBottom::addButtonToLayout(CartesianPlot::NavigationOperation op, QBoxLayout* layout, WorksheetView* view) {
	auto* button = new ToolButton(this);
	button->setDefaultAction(view->action(op));
	m_sizeHintHeight = qMax(m_sizeHintHeight, button->sizeHint().height());
	layout->addWidget(button);
}

bool SlidingPanelBottom::insideRect(QPoint screenPos) {
	QRect rect = this->rect();
	rect.moveTo(screen()->geometry().width() / 2 - m_hlayout->sizeHint().width() / 2, screen()->geometry().bottom() - rect.height());
	// Last part is a hack, because contains seems to be failing here
	const bool inside = rect.contains(screenPos) || screenPos.y() >= screen()->geometry().bottom();
	return inside;
}

QSize SlidingPanelBottom::sizeHint() const {
	QSize sh;
	sh.setWidth(m_hlayout->sizeHint().width());
	sh.setHeight(m_sizeHintHeight + layout()->contentsMargins().top() + layout()->contentsMargins().bottom());

	return sh;
}
