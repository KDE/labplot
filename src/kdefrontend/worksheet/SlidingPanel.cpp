/***************************************************************************
File                 : SlidingPanel.cpp
Project              : LabPlot
Description          : Sliding panel shown in the presenter widget
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
	pal.setColor(QPalette::Background, Qt::gray);
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
