/***************************************************************************
    File                 : CantorWorksheetView.cpp
    Project              : LabPlot
    Description          : View class for CantorWorksheet
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
#include "backend/cantorWorksheet/CantorWorksheet.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QToolBar>

#include <KToggleAction>
#include <KLocalizedString>
#include <KParts/ReadWritePart>

CantorWorksheetView::CantorWorksheetView(CantorWorksheet* worksheet) : QWidget(),
	m_worksheet(worksheet),
	m_part(nullptr),
	m_insertMarkdownEntryAction(nullptr),
	m_computeEigenvectorsAction(nullptr),
	m_createMattrixAction(nullptr),
	m_computeEigenvaluesAction(nullptr),
	m_invertMattrixAction(nullptr),
	m_differentiationAction(nullptr),
	m_integrationAction(nullptr),
	m_solveEquationsAction(nullptr),
	m_worksheetMenu(nullptr),
	m_linearAlgebraMenu(nullptr),
	m_calculateMenu(nullptr),
	m_settingsMenu(nullptr) {

	auto* layout = new QHBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);
	m_part = worksheet->part();
	if (m_part) {
		layout->addWidget(m_part->widget());
		initActions();
		connect(m_worksheet, SIGNAL(requestProjectContextMenu(QMenu*)), this, SLOT(createContextMenu(QMenu*)));
		connect(m_worksheet, SIGNAL(statusChanged(Cantor::Session::Status)), this, SLOT(statusChanged(Cantor::Session::Status)));
	} else {
		QLabel* label = new QLabel(i18n("Failed to initialize %1", m_worksheet->backendName()));
		label->setAlignment(Qt::AlignHCenter);
		layout->addWidget(label);
	}
}

void CantorWorksheetView::initActions() {
	auto* cantorActionGroup = new QActionGroup(this);
	cantorActionGroup->setExclusive(false);

	m_restartBackendAction = new QAction(QIcon::fromTheme("system-reboot"), i18n("Restart Backend"), cantorActionGroup);
	m_restartBackendAction->setData("restart_backend");

	m_evaluateWorsheetAction = new QAction(QIcon::fromTheme("system-run"), i18n("Evaluate Worksheet"), cantorActionGroup);
	m_evaluateWorsheetAction->setData("evaluate_worksheet");

	m_evaluateEntryAction = new QAction(QIcon::fromTheme(QLatin1String("media-playback-start")), i18n("Evaluate Entry"), cantorActionGroup);
	m_evaluateEntryAction->setShortcut(Qt::SHIFT + Qt::Key_Return);
	m_evaluateEntryAction->setData("evaluate_current");

	m_insertCommandEntryAction = new QAction(QIcon::fromTheme(QLatin1String("run-build")), i18n("Insert Command Entry"), cantorActionGroup);
	m_insertCommandEntryAction->setData("insert_command_entry");
	m_insertCommandEntryAction->setShortcut(Qt::CTRL + Qt::Key_Return);

	m_insertTextEntryAction = new QAction(QIcon::fromTheme(QLatin1String("draw-text")), i18n("Insert Text Entry"), cantorActionGroup);
	m_insertTextEntryAction->setData("insert_text_entry");

	//markdown entry is only available if cantor was compiled with libdiscovery (cantor 18.12 and later)
	if (m_part->action("insert_markdown_entry")) {
		m_insertTextEntryAction = new QAction(QIcon::fromTheme(QLatin1String("text-x-markdown")), i18n("Insert Markdown Entry"), cantorActionGroup);
		m_insertTextEntryAction->setData("insert_markdown_entry");
	}

	m_insertLatexEntryAction = new QAction(QIcon::fromTheme(QLatin1String("text-x-tex")), i18n("Insert LaTeX Entry"), cantorActionGroup);
	m_insertLatexEntryAction->setData("insert_latex_entry");

	m_insertPageBreakAction = new QAction(QIcon::fromTheme(QLatin1String("go-next-view-page")), i18n("Insert Page Break"), cantorActionGroup);
	m_insertPageBreakAction->setData("insert_page_break_entry");

	m_removeCurrentEntryAction = new QAction(QIcon::fromTheme(QLatin1String("edit-delete")), i18n("Remove Current Entry"), cantorActionGroup);
	m_removeCurrentEntryAction->setData("remove_current");

	m_zoomIn = new QAction(QIcon::fromTheme(QLatin1String("zoom-in")), i18n("Zoom In"), cantorActionGroup);
	m_zoomIn->setData("view_zoom_in");
	m_zoomIn->setShortcut(Qt::CTRL+Qt::Key_Plus);

	m_zoomOut = new QAction(QIcon::fromTheme(QLatin1String("zoom-out")), i18n("Zoom Out"), cantorActionGroup);
	m_zoomOut->setData("view_zoom_out");
	m_zoomOut->setShortcut(Qt::CTRL+Qt::Key_Minus);

	m_find = new QAction(QIcon::fromTheme(QLatin1String("edit-find")), i18n("Find"), cantorActionGroup);
	m_find->setData("edit_find");
	m_find->setShortcut(Qt::CTRL+Qt::Key_F);

	m_replace = new QAction(QIcon::fromTheme(QLatin1String("edit-find-replace")), i18n("Replace"), cantorActionGroup);
	m_replace->setData("edit_replace");
	m_replace->setShortcut(Qt::CTRL+Qt::Key_R);

	m_lineNumbers = new KToggleAction(i18n("Line Numbers"), cantorActionGroup);
	m_lineNumbers->setChecked(false);
	m_lineNumbers->setData("enable_expression_numbers");

	m_animateWorksheet = new KToggleAction(i18n("Animate Worksheet"), cantorActionGroup);
	m_animateWorksheet->setChecked(true);
	m_animateWorksheet->setData("enable_animations");

	m_latexTypesetting = new KToggleAction(i18n("LaTeX Typesetting"), cantorActionGroup);
	m_latexTypesetting->setChecked(true);
	m_latexTypesetting->setData("enable_typesetting");

	m_showCompletion = new QAction(i18n("Syntax Completion"), cantorActionGroup);
	m_showCompletion->setShortcut(Qt::CTRL + Qt::Key_Space);
	m_showCompletion->setData("show_completion");

	//actions, that are CAS-backend specific and not always available
	if (m_part->action("eigenvectors_assistant")) {
		m_computeEigenvectorsAction = new QAction(i18n("Compute Eigenvectors"), cantorActionGroup);
		m_computeEigenvectorsAction->setData("eigenvectors_assistant");
	}

	if (m_part->action("creatematrix_assistant")) {
		m_createMattrixAction = new QAction(i18n("Create Matrix"), cantorActionGroup);
		m_createMattrixAction->setData("creatematrix_assistant");
	}

	if (m_part->action("eigenvalues_assistant")) {
		m_computeEigenvaluesAction = new QAction(i18n("Compute Eigenvalues"), cantorActionGroup);
		m_computeEigenvaluesAction->setData("eigenvalues_assistant");
	}

	if (m_part->action("invertmatrix_assistant")) {
		m_invertMattrixAction = new QAction(i18n("Invert Matrix"), cantorActionGroup);
		m_invertMattrixAction->setData("invertmatrix_assistant");
	}

	if (m_part->action("differentiate_assistant")) {
		m_differentiationAction = new QAction(i18n("Differentiation"), cantorActionGroup);
		m_differentiationAction->setData("differentiate_assistant");
	}

	if (m_part->action("integrate_assistant")) {
		m_integrationAction = new QAction(i18n("Integration"), cantorActionGroup);
		m_integrationAction->setData("integrate_assistant");
	}

	if (m_part->action("solve_assistant")) {
		m_solveEquationsAction = new QAction(i18n("Solve Equations"), cantorActionGroup);
		m_solveEquationsAction->setData("solve_assistant");
	}

	connect(cantorActionGroup, SIGNAL(triggered(QAction*)), this, SLOT(triggerCantorAction(QAction*)));
}

void CantorWorksheetView::initMenus() {
	m_worksheetMenu = new QMenu(i18n("Worksheet"), m_part->widget());
	m_worksheetMenu->addAction(m_evaluateWorsheetAction);
	m_worksheetMenu->addSeparator();
	m_worksheetMenu->addAction(m_evaluateEntryAction);
	m_worksheetMenu->addAction(m_insertCommandEntryAction);
	m_worksheetMenu->addAction(m_insertTextEntryAction);
	if (m_insertMarkdownEntryAction)
		m_worksheetMenu->addAction(m_insertMarkdownEntryAction);
	m_worksheetMenu->addAction(m_insertLatexEntryAction);
	m_worksheetMenu->addAction(m_insertPageBreakAction);
	m_worksheetMenu->addSeparator();
	m_worksheetMenu->addAction(m_removeCurrentEntryAction);

	if (m_invertMattrixAction || m_createMattrixAction || m_computeEigenvectorsAction || m_computeEigenvaluesAction) {
		m_linearAlgebraMenu = new QMenu("Linear Algebra", m_part->widget());
		if (m_invertMattrixAction)
			m_linearAlgebraMenu->addAction(m_invertMattrixAction);
		if (m_createMattrixAction)
			m_linearAlgebraMenu->addAction(m_createMattrixAction);
		if (m_computeEigenvectorsAction)
			m_linearAlgebraMenu->addAction(m_computeEigenvectorsAction);
		if (m_computeEigenvaluesAction)
			m_linearAlgebraMenu->addAction(m_computeEigenvaluesAction);
	}

	if (m_solveEquationsAction || m_integrationAction || m_differentiationAction) {
		m_calculateMenu = new QMenu(i18n("Calculate"), m_part->widget());
		if (m_solveEquationsAction)
			m_calculateMenu->addAction(m_solveEquationsAction);
		if (m_integrationAction)
			m_calculateMenu->addAction(m_integrationAction);
		if (m_differentiationAction)
			m_calculateMenu->addAction(m_differentiationAction);
	}

	m_settingsMenu = new QMenu(i18n("Settings"), m_part->widget());
	m_settingsMenu->setIcon(QIcon::fromTheme(QLatin1String("settings-configure")));
	m_settingsMenu->addAction(m_lineNumbers);
	m_settingsMenu->addAction(m_animateWorksheet);
	m_settingsMenu->addAction(m_latexTypesetting);
	m_settingsMenu->addAction(m_showCompletion);
}

/*!
 * Populates the menu \c menu with the CantorWorksheet and CantorWorksheet view relevant actions.
 * The menu is used
 *   - as the context menu in CantorWorksheetView
 *   - as the "CantorWorksheet menu" in the main menu-bar (called form MainWin)
 *   - as a part of the CantorWorksheet context menu in project explorer
 */
void CantorWorksheetView::createContextMenu(QMenu* menu) {
	Q_ASSERT(menu);
	if (!m_part)
		return;

	QAction* firstAction = nullptr;
	// if we're populating the context menu for the project explorer, then
	//there're already actions available there. Skip the first title-action
	//and insert the action at the beginning of the menu.
	if (menu->actions().size()>1)
		firstAction = menu->actions().at(1);

	if (!m_worksheetMenu)
		initMenus();

	menu->insertMenu(firstAction, m_worksheetMenu);
	if (m_linearAlgebraMenu)
		menu->insertMenu(firstAction, m_linearAlgebraMenu);
	if (m_calculateMenu)
		menu->insertMenu(firstAction, m_calculateMenu);
	menu->insertSeparator(firstAction);
	menu->insertAction(firstAction, m_zoomIn);
	menu->insertAction(firstAction, m_zoomOut);
	menu->insertSeparator(firstAction);
	menu->insertAction(firstAction, m_find);
	menu->insertAction(firstAction, m_replace);
	menu->insertSeparator(firstAction);
	menu->insertMenu(firstAction, m_settingsMenu);
	menu->insertSeparator(firstAction);
	menu->insertAction(firstAction, m_restartBackendAction);
	menu->insertSeparator(firstAction);
}

void CantorWorksheetView::fillToolBar(QToolBar* toolbar) {
	if (!m_part)
		return;
	toolbar->addAction(m_restartBackendAction);
	toolbar->addAction(m_evaluateWorsheetAction);
}

/*!
 * Slot for actions triggered
 */
void CantorWorksheetView::triggerCantorAction(QAction* action) {
	QString actionName = action->data().toString();
	if(!actionName.isEmpty()) m_part->action(actionName.toStdString().c_str())->trigger();
}

CantorWorksheetView::~CantorWorksheetView() {
	if (m_part)
		m_part->widget()->setParent(nullptr);
}

void CantorWorksheetView::statusChanged(Cantor::Session::Status status) {
	if(status==Cantor::Session::Running) {
		m_evaluateWorsheetAction->setText(i18n("Interrupt"));
		m_evaluateWorsheetAction->setIcon(QIcon::fromTheme(QLatin1String("dialog-close")));
		emit m_worksheet->statusInfo(i18n("Calculating..."));
	} else {
		m_evaluateWorsheetAction->setText(i18n("Evaluate Worksheet"));
		m_evaluateWorsheetAction->setIcon(QIcon::fromTheme(QLatin1String("system-run")));
		emit m_worksheet->statusInfo(i18n("Ready"));
	}
}
