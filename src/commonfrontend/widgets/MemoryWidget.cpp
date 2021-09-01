/*
    File                 : MemoryWidget.cpp
    Project              : LabPlot
    Description          : widget showing the amount of memory consumed by the process
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2018 Alexander Semke (alexander.semke@web.de)
*/

/***************************************************************************
 *                                                                         *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/

#include "MemoryWidget.h"
#include "tools/getRSS.h"

#include <KLocalizedString>

MemoryWidget::MemoryWidget(QWidget* parent) : QLabel(parent) {
	connect(&m_timer, &QTimer::timeout, this, &MemoryWidget::refreshMemoryInfo);
	refreshMemoryInfo();
	m_timer.start(2000);
}

void MemoryWidget::refreshMemoryInfo() {
	size_t used = getCurrentRSS()/1024/1024;
	size_t peak = getPeakRSS()/1024/1024;
	setText(i18n("Memory used %1 MB, peak %2 MB", used, peak));
}
