/***************************************************************************
    File                 : CantorWorksheetView.h
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

#ifndef CANTORWORKSHEETVIEW_H
#define CANTORWORKSHEETVIEW_H

#include <QWidget>
#include <QAction>
#include <QMenu>
#include <QToolBar>
#include <QHBoxLayout>
#include <QTableView>
#include "backend/cantorWorksheet/CantorWorksheet.h"
#include <cantor/session.h>
#include <KParts/ReadWritePart>

class CantorWorksheetView : public QWidget {
    Q_OBJECT
    
    public:
	CantorWorksheetView(CantorWorksheet* cantorWorksheet, KParts::ReadWritePart*);
	
	~CantorWorksheetView();
	
    public slots:
	void createContextMenu(QMenu*) const;
	void fillToolBar(QToolBar*);

    private slots:
	void restartActionTriggered();
	void evaluateWorsheetActionTriggered();
	void evaluateEntryActionTriggered();
	void insertCommandEntryActionTriggered();
	void insertTextEntryActionTriggered();
	void insertLatexEntryActionTriggered();
	void insertPageBreakActionTriggered();
	void removeCurrentEntryActionTriggered();
	void computeEigenvectorsActionTriggered();
	void createMattrixActionTriggered();
	void computeEigenvaluesActionTriggered();
	void invertMattrixActionTriggered();
	void differentiationActionTriggered();
	void integrationActionTriggered();
	void solveEquationsActionTriggered();

    private:
	CantorWorksheet* m_worksheet;  
	QAction* m_restartBackendAction;
	QAction* m_evaluateWorsheetAction;
	QAction* m_evaluateEntryAction;
	QAction* m_insertCommandEntryAction;
	QAction* m_insertTextEntryAction;
	QAction* m_insertLatexEntryAction;
	QAction* m_insertPageBreakAction;
	QAction* m_removeCurrentEntryAction;
	QAction* m_computeEigenvectorsAction;
	QAction* m_createMattrixAction;
	QAction* m_computeEigenvaluesAction;
	QAction* m_invertMattrixAction;
	QAction* m_differentiationAction;
	QAction* m_integrationAction;
	QAction* m_solveEquationsAction;
	QMenu* m_worksheetMenu;
	QMenu* m_linearAlgebraMenu;
	QMenu* m_calculateMenu;
	QHBoxLayout* layout;
	KParts::ReadWritePart* part;
	
	void initActions();
	void initMenus();
};

#endif // CANTORWORKSHEETVIEW_H
