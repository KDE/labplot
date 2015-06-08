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
#include <QDebug>

CantorWorksheetDock::CantorWorksheetDock(QWidget* parent): QWidget(parent) {
    ui.setupUi(this);
    ui.general->setFocus();
}

void CantorWorksheetDock::setCantorWorksheets(QList< CantorWorksheet* > list) {
    m_cantorworksheetlist = list;
    m_cantorworksheet = list.first();
    if (list.size()==1){
	if(w) ui.tabWidget->addTab(w, "Variable Manager");
    } else {
	int index = ui.tabWidget->indexOf(ui.variable_manager);
	w = ui.tabWidget->widget(index);
	ui.tabWidget->removeTab(index);
    }
}

