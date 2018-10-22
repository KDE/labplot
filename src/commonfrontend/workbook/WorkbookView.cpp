/***************************************************************************
    File                 : WorkbookView.cpp
    Project              : LabPlot
    Description          : View class for Workbook
    --------------------------------------------------------------------
    Copyright            : (C) 2015 by Alexander Semke (alexander.semke@web.de)

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

#include "WorkbookView.h"
#include "backend/core/AbstractAspect.h"
#include "backend/core/AbstractPart.h"
#include "backend/core/Workbook.h"
#include "backend/lib/macros.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/matrix/Matrix.h"

#include <QHBoxLayout>
#include <QMenu>

#include <KLocalizedString>

/*!
	\class WorkbookView
	\brief View class for Workbook

	\ingroup commonfrontend
 */
WorkbookView::WorkbookView(Workbook* workbook) : QWidget(),
	m_tabWidget(new TabWidget(this)),
	m_workbook(workbook),
	lastSelectedIndex(0) {

	m_tabWidget->setTabPosition(QTabWidget::South);
	m_tabWidget->setTabShape(QTabWidget::Rounded);
	m_tabWidget->setMovable(true);
	m_tabWidget->setContextMenuPolicy(Qt::CustomContextMenu);
	m_tabWidget->setMinimumSize(200, 200);

	auto* layout = new QHBoxLayout(this);
	layout->setContentsMargins(0,0,0,0);
	layout->addWidget(m_tabWidget);

	//add tab for each children view
	m_initializing = true;
	for (const auto* aspect : m_workbook->children<AbstractAspect>())
		handleAspectAdded(aspect);
	m_initializing = false;

	//Actions
	action_add_spreadsheet = new QAction(QIcon::fromTheme("labplot-spreadsheet"), i18n("Add new Spreadsheet"), this);
	action_add_matrix = new QAction(QIcon::fromTheme("labplot-matrix"), i18n("Add new Matrix"), this);
	connect(action_add_spreadsheet, SIGNAL(triggered()), this, SLOT(addSpreadsheet()));
	connect(action_add_matrix, SIGNAL(triggered()), this, SLOT(addMatrix()));

	//SIGNALs/SLOTs
	connect(m_workbook, SIGNAL(aspectDescriptionChanged(const AbstractAspect*)), this, SLOT(handleDescriptionChanged(const AbstractAspect*)));
	connect(m_workbook, SIGNAL(aspectAdded(const AbstractAspect*)), this, SLOT(handleAspectAdded(const AbstractAspect*)));
	connect(m_workbook, SIGNAL(aspectAboutToBeRemoved(const AbstractAspect*)), this, SLOT(handleAspectAboutToBeRemoved(const AbstractAspect*)));
	connect(m_workbook, SIGNAL(requestProjectContextMenu(QMenu*)), this, SLOT(createContextMenu(QMenu*)));
	connect(m_workbook, SIGNAL(workbookItemSelected(int)), this, SLOT(itemSelected(int)) );

	connect(m_tabWidget, SIGNAL(currentChanged(int)), SLOT(tabChanged(int)));
	connect(m_tabWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showTabContextMenu(QPoint)));
	connect(m_tabWidget, SIGNAL(tabMoved(int,int)), this, SLOT(tabMoved(int,int)));
}

WorkbookView::~WorkbookView() {
	//no need to react on currentChanged() in TabWidget when views are deleted
	disconnect(m_tabWidget, nullptr, nullptr, nullptr);

	//delete all children views here, its own view will be deleted in ~AbstractPart()
	for (const auto* part : m_workbook->children<AbstractPart>())
		part->deleteView();
}

int WorkbookView::currentIndex() const {
	return m_tabWidget->currentIndex();
}

//##############################################################################
//#########################  Private slots  ####################################
//##############################################################################
/*!
  called when the current tab was changed. Propagates the selection of \c Spreadsheet
  or of a \c Matrix object to \c Workbook.
*/
void WorkbookView::tabChanged(int index) {
	if (m_initializing)
		return;

	if (index==-1)
		return;

	m_workbook->setChildSelectedInView(lastSelectedIndex, false);
	m_workbook->setChildSelectedInView(index, true);
	lastSelectedIndex = index;
}

void WorkbookView::tabMoved(int from, int to) {
	Q_UNUSED(from);
	Q_UNUSED(to);
	//TODO:
// 	AbstractAspect* aspect = m_workbook->child<AbstractAspect>(to);
// 	if (aspect) {
// 		m_tabMoving = true;
// 		AbstractAspect* sibling = m_workbook->child<AbstractAspect>(from);
// 		qDebug()<<"insert: " << to << "  " <<  aspect->name() << ",  " << from << "  " << sibling->name();
// 		aspect->remove();
// 		m_workbook->insertChildBefore(aspect, sibling);
// 		qDebug()<<"inserted";
// 		m_tabMoving = false;
// 	}
}

void WorkbookView::itemSelected(int index) {
	m_tabWidget->setCurrentIndex(index);
}

/*!
 * Populates the menu \c menu with the spreadsheet and spreadsheet view relevant actions.
 * The menu is used
 *   - as the context menu in WorkbookView
 *   - as the "spreadsheet menu" in the main menu-bar (called form MainWin)
 *   - as a part of the spreadsheet context menu in project explorer
 */
void WorkbookView::createContextMenu(QMenu* menu) const {
	Q_ASSERT(menu);

	QAction* firstAction = nullptr;
	// if we're populating the context menu for the project explorer, then
	//there're already actions available there. Skip the first title-action
	//and insert the action at the beginning of the menu.
	if (menu->actions().size()>1)
		firstAction = menu->actions().at(1);

	menu->insertAction(firstAction, action_add_spreadsheet);
	menu->insertAction(firstAction, action_add_matrix);
	menu->insertSeparator(firstAction);
}

void WorkbookView::showTabContextMenu(QPoint point) {
	QMenu* menu = nullptr;
	auto* aspect = m_workbook->child<AbstractAspect>(m_tabWidget->currentIndex());
	auto* spreadsheet = dynamic_cast<Spreadsheet*>(aspect);
	if (spreadsheet) {
		menu = spreadsheet->createContextMenu();
	} else {
		Matrix* matrix = dynamic_cast<Matrix*>(aspect);
		if (matrix)
			menu = matrix->createContextMenu();
	}

	if (menu)
		menu->exec(m_tabWidget->mapToGlobal(point));
}

void WorkbookView::addMatrix() {
	Matrix* matrix = new Matrix(i18n("Matrix"));
	m_workbook->addChild(matrix);
}

void WorkbookView::addSpreadsheet() {
	Spreadsheet* spreadsheet = new Spreadsheet(i18n("Spreadsheet"));
	m_workbook->addChild(spreadsheet);
}

void WorkbookView::handleDescriptionChanged(const AbstractAspect* aspect) {
	int index = m_workbook->indexOfChild<AbstractAspect>(aspect);
	if (index != -1 && index<m_tabWidget->count())
		m_tabWidget->setTabText(index, aspect->name());
}

void WorkbookView::handleAspectAdded(const AbstractAspect* aspect) {
	const auto* part = dynamic_cast<const AbstractPart*>(aspect);
	if (!part)
		return;

	int index = m_workbook->indexOfChild<AbstractAspect>(aspect);
	m_tabWidget->insertTab(index, part->view(), aspect->name());
	m_tabWidget->setCurrentIndex(index);
	m_tabWidget->setTabIcon(m_tabWidget->count(), aspect->icon());
	this->tabChanged(index);
}

void WorkbookView::handleAspectAboutToBeRemoved(const AbstractAspect* aspect) {
	int index = m_workbook->indexOfChild<AbstractAspect>(aspect);
	m_tabWidget->removeTab(index);
}
