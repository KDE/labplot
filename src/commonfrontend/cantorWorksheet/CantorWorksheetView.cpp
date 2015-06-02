/***************************************************************************
    File                 : CantorWorksheetView.cpp
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

#include "CantorWorksheetView.h"

#include <QIcon>
#include <QAction>
#include <QDebug>
#include <QTableView>
#include <QPushButton>
#include <QHBoxLayout>

#include <KLocalizedString>

#include "commonfrontend/cantorWorksheet/worksheetview.h"
#include "commonfrontend/cantorWorksheet/worksheet.h"
#include <cantor/backend.h>

CantorWorksheetView::CantorWorksheetView(CantorWorksheet* worksheet) : QWidget(),
    m_worksheet(worksheet) {
	
	QHBoxLayout* layout = new QHBoxLayout(this);
	layout->setContentsMargins(0,0,0,0);
	
	Worksheet* scene = new Worksheet(Cantor::Backend::createBackend("Maxima"), this);
	WorksheetView* m_view = new WorksheetView(scene, this);
	layout->addWidget(m_view);
	
	
	initActions();
	
	connect(m_worksheet, SIGNAL(requestProjectContextMenu(QMenu*)), this, SLOT(createContextMenu(QMenu*)));
}

void CantorWorksheetView::initActions() {
    m_restartBackendAction = new QAction(QIcon::fromTheme("system-reboot"), i18n("Restart Backend"), this);    
//     m_restartBackendMenu = new QMenu(i18n("Add new"));
//     m_restartBackendMenu->setIcon(QIcon::fromTheme("office-chart-line"));
//     m_restartBackendMenu->addAction(m_restartBackendAction);
//     connect(selectAllAction, SIGNAL(triggered()), SLOT(selectAllElements()));
}

void CantorWorksheetView::createContextMenu(QMenu* menu) const{
    Q_ASSERT(menu);
    
    #ifdef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
	QAction* firstAction = menu->actions().first();
    #else
	QAction* firstAction = 0;
	// if we're populating the context menu for the project explorer, then
	//there're already actions available there. Skip the first title-action
	//and insert the action at the beginning of the menu.
	if (menu->actions().size()>1)
	    firstAction = menu->actions().at(1);
    #endif
    
    menu->insertAction(firstAction, m_restartBackendAction);
}

CantorWorksheetView::~CantorWorksheetView() {

}
