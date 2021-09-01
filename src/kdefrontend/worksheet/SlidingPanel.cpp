/*
File                 : SlidingPanel.cpp
Project              : LabPlot
Description          : Sliding panel shown in the presenter widget
--------------------------------------------------------------------
SPDX-FileCopyrightText: 2016 Fabian Kristof (fkristofszabolcs@gmail.com)
SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "SlidingPanel.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScreen>
#include <QSize>

#include <KLocalizedString>

SlidingPanel::SlidingPanel(QWidget *parent, const QString &worksheetName) : QFrame(parent) {
	setAttribute(Qt::WA_DeleteOnClose);

	m_worksheetName = new QLabel(worksheetName);
	QFont nameFont;
	nameFont.setPointSize(20);
	nameFont.setBold(true);
	m_worksheetName->setFont(nameFont);

	m_quitPresentingMode = new QPushButton(i18n("Quit Presentation"));
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

SlidingPanel::~SlidingPanel() {
	delete m_worksheetName;
	delete m_quitPresentingMode;
}

void SlidingPanel::movePanel(qreal value) {
	move(0, -height() + static_cast<int>(value * height()) );
	raise();
}

QPushButton* SlidingPanel::quitButton() const {
	return m_quitPresentingMode;
}

QSize SlidingPanel::sizeHint() const {
	QSize sh;
	const QRect& screenSize = QGuiApplication::primaryScreen()->availableGeometry();
	sh.setWidth(screenSize.width());
	sh.setHeight(m_worksheetName->sizeHint().height()
				 + layout()->contentsMargins().top() + layout()->contentsMargins().bottom());

	return sh;
}
