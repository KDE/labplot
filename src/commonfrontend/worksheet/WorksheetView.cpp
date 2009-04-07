/***************************************************************************
    File                 : WorksheetView.cpp
    Project              : LabPlot/SciDAVis
    Description          : Worksheet view
    --------------------------------------------------------------------
    Copyright            : (C) 2009 Tilman Benkert (thzs*gmx.net)
                           (replace * with @ in the email addresses) 
                           
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

#include "worksheet/WorksheetView.h"
#include "worksheet/WorksheetModel.h"
#include "lib/ActionManager.h"
#include <QGraphicsView>

/**
 * \class WorksheetView
 * \brief Worksheet view
 *
 *
 */

WorksheetView::WorksheetView(Worksheet *worksheet)
 : m_worksheet(worksheet)
{
	m_model = new WorksheetModel(worksheet);
	init();
}

WorksheetView::~WorksheetView() 
{
	delete m_model;
}

//! Private ctor for initActionManager() only
WorksheetView::WorksheetView()
{
	m_model = NULL;
	createActions();
}

void WorksheetView::init() {
	createActions();
// TODO
}

void WorksheetView::createActions() {
// TODO
}
void WorksheetView::connectActions() {
// TODO
}

void WorksheetView::createContextMenu(QMenu *menu) {
// TODO
}

void WorksheetView::fillProjectMenu(QMenu *menu, bool *rc) {
// TODO
}

/* ========================= static methods ======================= */
ActionManager *WorksheetView::action_manager = 0;

ActionManager *WorksheetView::actionManager()
{
	if (!action_manager)
		initActionManager();
	
	return action_manager;
}

void WorksheetView::initActionManager()
{
	if (!action_manager)
		action_manager = new ActionManager();

	action_manager->setTitle(tr("Worksheet"));
	volatile WorksheetView *action_creator = new WorksheetView(); // initialize the action texts
	delete action_creator;
}

