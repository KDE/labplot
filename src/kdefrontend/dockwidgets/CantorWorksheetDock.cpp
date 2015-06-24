/***************************************************************************
    File                 : CantorWorksheetDock.cpp
    Project              : LabPlot
    Description          : Aspect providing a Cantor Worksheets for Multiple backends
    --------------------------------------------------------------------
    Copyright            : (C) 2015 Garvit Khatri (garvitdelhi@gmail.com)

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

#include "CantorWorksheetDock.h"
#include "backend/cantorWorksheet/CantorWorksheet.h"
#include <QDebug>
#include <cantor/session.h>
#include <KParts/ReadWritePart>

CantorWorksheetDock::CantorWorksheetDock(QWidget* parent): QWidget(parent) {
    ui.setupUi(this);   
    ui.tabWidget->setMovable(true);
}

void CantorWorksheetDock::setCantorWorksheets(QList< CantorWorksheet* > list) {
    m_cantorworksheetlist = list;
    int k = 0;
    int prev_index = ui.tabWidget->currentIndex();
    foreach(int i, index) {
	ui.tabWidget->removeTab(i-k);
	++k;
    }
    if (m_cantorworksheetlist.size()==1) {
	QList<Cantor::PanelPlugin*> plugins = m_cantorworksheetlist.first()->getPlugins();
	index.clear();
	foreach(Cantor::PanelPlugin* plugin, plugins) {
	    plugin->setParentWidget(this);
	    int i = ui.tabWidget->addTab(plugin->widget(), plugin->name());
	    index.append(i);
	}
    }
    ui.tabWidget->setCurrentIndex(prev_index);
}

