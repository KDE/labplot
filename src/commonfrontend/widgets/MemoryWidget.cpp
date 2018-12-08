/***************************************************************************
    File                 : MemoryWidget.cpp
    Project              : LabPlot
    Description          : widget showing the amount of memory consumed by the process
    --------------------------------------------------------------------
    Copyright            : (C) 2018 Alexander Semke (alexander.semke@web.de)
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

#include "MemoryWidget.h"
#include "tools/getRSS.h"

#include <KLocalizedString>

MemoryWidget::MemoryWidget(QWidget* parent) : QLabel(parent) {
	connect(&m_timer, &QTimer::timeout, this, &MemoryWidget::refreshMemoryInfo);
	m_timer.start(2000);
}

void MemoryWidget::refreshMemoryInfo() {
	size_t used = getCurrentRSS()/1024/1024;
	size_t peak = getPeakRSS()/1024/1024;
	setText(i18n("Memory used %1 MB, peak %2 MB", used, peak));
}
