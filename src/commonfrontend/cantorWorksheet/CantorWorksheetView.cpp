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
#include <QSizePolicy>
#include <QPainter>
#include <QPrintDialog>

#include <KLocalizedString>
#include <KMessageBox>
#include <cantor/backend.h>

CantorWorksheetView::CantorWorksheetView(CantorWorksheet* worksheet) : QWidget(),
    m_worksheet(worksheet) {
	
    layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    part = worksheet->getPart();
    layout->addWidget(this->part->widget());
    initActions();
    initMenus();
    connect(m_worksheet, SIGNAL(requestProjectContextMenu(QMenu*)), this, SLOT(createContextMenu(QMenu*)));
}

void CantorWorksheetView::initActions() {
    m_restartBackendAction = new QAction(QIcon::fromTheme("system-reboot"), i18n("Restart Backend"), this);
    connect(m_restartBackendAction, SIGNAL(triggered()), this, SLOT(restartActionTriggered()));
    m_evaluateWorsheetAction = new QAction(QIcon::fromTheme("system-run"), i18n("Evaluate Worksheet"), this);
    connect(m_evaluateWorsheetAction, SIGNAL(triggered()), this, SLOT(evaluateWorsheetActionTriggered()));
    m_evaluateEntryAction = new QAction(i18n("Evaluate Entry"), this);
    m_evaluateEntryAction->setShortcut(Qt::CTRL + Qt::Key_Return);
    connect(m_evaluateEntryAction, SIGNAL(triggered()), this, SLOT(evaluateEntryActionTriggered()));
    m_insertCommandEntryAction = new QAction(i18n("Insert Command Entry"), this);
    connect(m_insertCommandEntryAction, SIGNAL(triggered()), this, SLOT(insertCommandEntryActionTriggered()));
    m_insertTextEntryAction = new QAction(i18n("Insert Text Entry"), this);
    connect(m_insertTextEntryAction, SIGNAL(triggered()), this, SLOT(insertTextEntryActionTriggered()));
    m_insertLatexEntryAction = new QAction(i18n("Insert Latex Entry"), this);
    connect(m_insertLatexEntryAction, SIGNAL(triggered()), this, SLOT(insertLatexEntryActionTriggered()));
    m_insertPageBreakAction = new QAction(i18n("Insert Page Break"), this);
    connect(m_insertPageBreakAction, SIGNAL(triggered()), this, SLOT(insertPageBreakActionTriggered()));
    m_removeCurrentEntryAction = new QAction(i18n("Remove Current Entry"), this);
    connect(m_removeCurrentEntryAction, SIGNAL(triggered()), this, SLOT(removeCurrentEntryActionTriggered()));
    m_computeEigenvectorsAction = new QAction(i18n("Compute Eigenvectors"), this);
    connect(m_computeEigenvectorsAction, SIGNAL(triggered()), this, SLOT(computeEigenvectorsActionTriggered()));
    m_createMattrixAction = new QAction(i18n("Create Matrix"), this);
    connect(m_createMattrixAction, SIGNAL(triggered()), this, SLOT(createMattrixActionTriggered()));
    m_computeEigenvaluesAction = new QAction(i18n("Compute Eigenvalues"), this);
    connect(m_computeEigenvaluesAction, SIGNAL(triggered()), this, SLOT(computeEigenvaluesActionTriggered()));
    m_invertMattrixAction = new QAction(i18n("Invert Matrix"), this);
    connect(m_invertMattrixAction, SIGNAL(triggered()), this, SLOT(invertMattrixActionTriggered()));
    m_differentiationAction = new QAction(i18n("Differentiation"), this);
    connect(m_differentiationAction, SIGNAL(triggered()), this, SLOT(differentiationActionTriggered()));
    m_integrationAction = new QAction(i18n("Integration"), this);
    connect(m_integrationAction, SIGNAL(triggered()), this, SLOT(integrationActionTriggered()));
    m_solveEquationsAction = new QAction(i18n("Solve Equations"), this);
    connect(m_solveEquationsAction, SIGNAL(triggered()), this, SLOT(solveEquationsActionTriggered()));
}

void CantorWorksheetView::initMenus() {
    m_worksheetMenu = new QMenu("Worksheet", this);
    m_worksheetMenu->addAction(m_evaluateWorsheetAction);
    m_worksheetMenu->addAction(m_evaluateEntryAction);
    m_worksheetMenu->addAction(m_insertCommandEntryAction);
    m_worksheetMenu->addAction(m_insertTextEntryAction);
    m_worksheetMenu->addAction(m_insertLatexEntryAction);
    m_worksheetMenu->addAction(m_insertPageBreakAction);
    m_worksheetMenu->addAction(m_removeCurrentEntryAction);
    m_linearAlgebraMenu = new QMenu("Linear Algebra", this);
    m_linearAlgebraMenu->addAction(m_invertMattrixAction);
    m_linearAlgebraMenu->addAction(m_createMattrixAction);
    m_linearAlgebraMenu->addAction(m_computeEigenvectorsAction);
    m_linearAlgebraMenu->addAction(m_computeEigenvaluesAction);
    m_calculateMenu = new QMenu("Calculate", this);
    m_calculateMenu->addAction(m_solveEquationsAction);
    m_calculateMenu->addAction(m_integrationAction);
    m_calculateMenu->addAction(m_differentiationAction);
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
    menu->insertMenu(firstAction, m_worksheetMenu);
    menu->insertMenu(firstAction, m_linearAlgebraMenu);
    menu->insertMenu(firstAction, m_calculateMenu);
}

void CantorWorksheetView::fillToolBar(QToolBar* toolbar) {
    toolbar->addAction(m_restartBackendAction);
    toolbar->addAction(m_evaluateWorsheetAction);
}

void CantorWorksheetView::restartActionTriggered() {
    part->action("restart_backend")->trigger();
}

void CantorWorksheetView::evaluateWorsheetActionTriggered() {
    part->action("evaluate_worksheet")->trigger();
}

void CantorWorksheetView::computeEigenvaluesActionTriggered() {
    part->action("eigenvalues_assistant")->trigger();
}

void CantorWorksheetView::computeEigenvectorsActionTriggered() {
    part->action("eigenvectors_assistant")->trigger();
}

void CantorWorksheetView::createMattrixActionTriggered() {
    part->action("creatematrix_assistant")->trigger();
}

void CantorWorksheetView::differentiationActionTriggered() {
    part->action("differentiate_assistant")->trigger();
}

void CantorWorksheetView::evaluateEntryActionTriggered() {
    part->action("evaluate_current")->trigger();
}

void CantorWorksheetView::insertCommandEntryActionTriggered() {
    part->action("insert_command_entry")->trigger();
}

void CantorWorksheetView::insertLatexEntryActionTriggered() {
    part->action("insert_latex_entry")->trigger();
}

void CantorWorksheetView::insertPageBreakActionTriggered() {
    part->action("insert_page_break_entry")->trigger();
}

void CantorWorksheetView::insertTextEntryActionTriggered() {
    qDebug() << "here";
    part->action("insert_text_entry")->trigger();
}

void CantorWorksheetView::integrationActionTriggered() {
    part->action("integrate_assistant")->trigger();
}

void CantorWorksheetView::invertMattrixActionTriggered() {
    part->action("invertmatrix_assistant")->trigger();
}

void CantorWorksheetView::removeCurrentEntryActionTriggered() {
    part->action("remove_current")->trigger();
}

void CantorWorksheetView::solveEquationsActionTriggered() {
    part->action("solve_assistant")->trigger();
}

CantorWorksheetView::~CantorWorksheetView() {
    part->widget()->setParent(0);
}
