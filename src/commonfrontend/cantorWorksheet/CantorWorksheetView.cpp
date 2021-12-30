/*
    File                 : CantorWorksheetView.cpp
    Project              : LabPlot
    Description          : View class for CantorWorksheet
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2015 Garvit Khatri <garvitdelhi@gmail.com>
    SPDX-FileCopyrightText: 2016-2021 Alexander Semke <alexander.semke@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


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
	m_worksheet(worksheet) {

	auto* layout = new QHBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);
	m_part = worksheet->part();
	if (m_part) {
		layout->addWidget(m_part->widget());
		initActions();
		connect(m_worksheet, &CantorWorksheet::requestProjectContextMenu, this, &CantorWorksheetView::createContextMenu);
		connect(m_worksheet, &CantorWorksheet::statusChanged, this, &CantorWorksheetView::statusChanged);
	} else {
		QString msg = "<b>" + i18n("Failed to initialize %1.", m_worksheet->backendName()) + "</b><br>";
		msg += worksheet->error();
		QLabel* label = new QLabel(msg);
		label->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
		layout->addWidget(label);
	}
}

void CantorWorksheetView::initActions() {
	m_actionGroup = new QActionGroup(this);
	m_actionGroup->setExclusive(false);

	//general notebook specific actions
	m_zoomIn = new QAction(QIcon::fromTheme(QLatin1String("zoom-in")), i18n("Zoom In"), m_actionGroup);
	m_zoomIn->setData("view_zoom_in");
	m_zoomIn->setShortcut(Qt::CTRL+Qt::Key_Plus);

	m_zoomOut = new QAction(QIcon::fromTheme(QLatin1String("zoom-out")), i18n("Zoom Out"), m_actionGroup);
	m_zoomOut->setData("view_zoom_out");
	m_zoomOut->setShortcut(Qt::CTRL+Qt::Key_Minus);

	m_find = new QAction(QIcon::fromTheme(QLatin1String("edit-find")), i18n("Find"), m_actionGroup);
	m_find->setData("edit_find");
	m_find->setShortcut(Qt::CTRL+Qt::Key_F);

	m_replace = new QAction(QIcon::fromTheme(QLatin1String("edit-find-replace")), i18n("Replace"), m_actionGroup);
	m_replace->setData("edit_replace");
	m_replace->setShortcut(Qt::CTRL+Qt::Key_R);

	m_restartBackendAction = new QAction(QIcon::fromTheme("system-reboot"), i18n("Restart Backend"), m_actionGroup);
	m_restartBackendAction->setData("restart_backend");

	m_evaluateWorsheetAction = new QAction(QIcon::fromTheme("system-run"), i18n("Evaluate Notebook"), m_actionGroup);
	m_evaluateWorsheetAction->setData("evaluate_worksheet");

	//all other actions are initialized in initMenus() since they are only required for the main and context menu

	connect(m_actionGroup, &QActionGroup::triggered, this, &CantorWorksheetView::triggerAction);
}

void CantorWorksheetView::initMenus() {
	//initialize the remaining actions

	//entry specific actions
	m_evaluateEntryAction = new QAction(QIcon::fromTheme(QLatin1String("media-playback-start")), i18n("Evaluate Entry"), m_actionGroup);
	m_evaluateEntryAction->setShortcut(Qt::SHIFT + Qt::Key_Return);
	m_evaluateEntryAction->setData("evaluate_current");

	m_removeCurrentEntryAction = new QAction(QIcon::fromTheme(QLatin1String("edit-delete")), i18n("Remove Current Entry"), m_actionGroup);
	m_removeCurrentEntryAction->setData("remove_current");

	//actions for the "Add New" menu
	auto* insertCommandEntryAction = new QAction(QIcon::fromTheme(QLatin1String("run-build")), i18n("Command"), m_actionGroup);
	insertCommandEntryAction->setData("insert_command_entry");
	insertCommandEntryAction->setShortcut(Qt::CTRL + Qt::Key_Return);

	auto* insertTextEntryAction = new QAction(QIcon::fromTheme(QLatin1String("draw-text")), i18n("Text"), m_actionGroup);
	insertTextEntryAction->setData("insert_text_entry");

	//markdown entry is only available if cantor was compiled with libdiscovery (cantor 18.12 and later)
	QAction* insertMarkdownEntryAction = nullptr;
	if (m_part->action("insert_markdown_entry")) {
		insertMarkdownEntryAction = new QAction(QIcon::fromTheme(QLatin1String("text-x-markdown")), i18n("Markdown"), m_actionGroup);
		insertMarkdownEntryAction->setData("insert_markdown_entry");
	}

	auto* insertLatexEntryAction = new QAction(QIcon::fromTheme(QLatin1String("text-x-tex")), i18n("LaTeX"), m_actionGroup);
	insertLatexEntryAction->setData("insert_latex_entry");

	auto* insertImageEntryAction = new QAction(QIcon::fromTheme(QLatin1String("image-x-generic")), i18n("Image"), m_actionGroup);
	insertImageEntryAction->setData("insert_image_entry");

	auto* insertPageBreakAction = new QAction(QIcon::fromTheme(QLatin1String("go-next-view-page")), i18n("Page Break"), m_actionGroup);
	insertPageBreakAction->setData("insert_page_break_entry");

// 	auto* insertHorizLineAction = new QAction(QIcon::fromTheme(QLatin1String("newline")), i18n("Horizontal Line"), m_actionGroup);
// 	insertHorizLineAction->setData("insert_horizontal_line_entry");

// 	auto* insertHierarchyEntryAction = new QAction(QIcon::fromTheme(QLatin1String("view-list-tree")), i18n("Hierarchy"), m_actionGroup);
// 	insertHierarchyEntryAction->setData("insert_hierarchy_entry");

	//actions for "assistants", that are backend specific and not always available
	QAction* computeEigenvectorsAction = nullptr;
	if (m_part->action("eigenvectors_assistant")) {
		computeEigenvectorsAction = new QAction(i18n("Compute Eigenvectors"), m_actionGroup);
		computeEigenvectorsAction->setData("eigenvectors_assistant");
	}

	QAction* createMatrixAction = nullptr;
	if (m_part->action("creatematrix_assistant")) {
		createMatrixAction = new QAction(i18n("Create Matrix"), m_actionGroup);
		createMatrixAction->setData("creatematrix_assistant");
	}

	QAction* computeEigenvaluesAction = nullptr;
	if (m_part->action("eigenvalues_assistant")) {
		computeEigenvaluesAction = new QAction(i18n("Compute Eigenvalues"), m_actionGroup);
		computeEigenvaluesAction->setData("eigenvalues_assistant");
	}

	QAction* invertMatrixAction = nullptr;
	if (m_part->action("invertmatrix_assistant")) {
		invertMatrixAction = new QAction(i18n("Invert Matrix"), m_actionGroup);
		invertMatrixAction->setData("invertmatrix_assistant");
	}

	QAction* differentiationAction = nullptr;
	if (m_part->action("differentiate_assistant")) {
		differentiationAction = new QAction(i18n("Differentiation"), m_actionGroup);
		differentiationAction->setData("differentiate_assistant");
	}

	QAction* integrationAction = nullptr;
	if (m_part->action("integrate_assistant")) {
		integrationAction = new QAction(i18n("Integration"), m_actionGroup);
		integrationAction->setData("integrate_assistant");
	}

	QAction* solveEquationsAction = nullptr;
	if (m_part->action("solve_assistant")) {
		solveEquationsAction = new QAction(i18n("Solve Equations"), m_actionGroup);
		solveEquationsAction->setData("solve_assistant");
	}

	//menus

	//"Add New"
	m_addNewMenu = new QMenu(i18n("Add New"), m_part->widget());
	m_addNewMenu->setIcon(QIcon::fromTheme(QLatin1String("list-add")));

	m_addNewMenu->addAction(insertCommandEntryAction);
	m_addNewMenu->addAction(insertTextEntryAction);
	if (insertMarkdownEntryAction)
		m_addNewMenu->addAction(insertMarkdownEntryAction);
	m_addNewMenu->addAction(insertLatexEntryAction);
	m_addNewMenu->addAction(insertImageEntryAction);
	m_addNewMenu->addSeparator();
	m_addNewMenu->addAction(insertPageBreakAction);
// 	m_addNewMenu->addAction(insertHorizLineAction);
// 	m_addNewMenu->addAction(insertHierarchyEntryAction);

	//"Assistants"
	if (invertMatrixAction || createMatrixAction || computeEigenvectorsAction || computeEigenvaluesAction) {
		m_linearAlgebraMenu = new QMenu(i18n("Linear Algebra"), m_part->widget());
		if (invertMatrixAction)
			m_linearAlgebraMenu->addAction(invertMatrixAction);
		if (createMatrixAction)
			m_linearAlgebraMenu->addAction(createMatrixAction);
		if (computeEigenvectorsAction)
			m_linearAlgebraMenu->addAction(computeEigenvectorsAction);
		if (computeEigenvaluesAction)
			m_linearAlgebraMenu->addAction(computeEigenvaluesAction);
	}

	if (solveEquationsAction || integrationAction || differentiationAction) {
		m_calculateMenu = new QMenu(i18n("Calculate"), m_part->widget());
		if (solveEquationsAction)
			m_calculateMenu->addAction(solveEquationsAction);
		if (integrationAction)
			m_calculateMenu->addAction(integrationAction);
		if (differentiationAction)
			m_calculateMenu->addAction(differentiationAction);
	}

	//"Notebook Settings"
	m_settingsMenu = new QMenu(i18n("Settings"), m_part->widget());
	m_settingsMenu->setIcon(QIcon::fromTheme(QLatin1String("settings-configure")));
	m_settingsMenu->addAction(m_part->action("enable_expression_numbers"));
	m_settingsMenu->addAction(m_part->action("enable_highlighting"));
	m_settingsMenu->addAction(m_part->action("enable_completion"));
	m_settingsMenu->addAction(m_part->action("enable_animations"));
	m_settingsMenu->addSeparator();
	m_settingsMenu->addAction(m_part->action("enable_typesetting"));
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

	if (!m_addNewMenu)
		initMenus();

	menu->insertAction(firstAction, m_evaluateWorsheetAction);
	menu->insertSeparator(firstAction);
	menu->insertAction(firstAction, m_evaluateEntryAction);
	menu->addSeparator();
	menu->insertMenu(firstAction, m_addNewMenu);
	menu->insertSeparator(firstAction);
	menu->insertAction(firstAction, m_removeCurrentEntryAction);

	//results related actions
	menu->insertSeparator(firstAction);
	menu->addAction(m_part->action("all_entries_collapse_results"));
	menu->addAction(m_part->action("all_entries_uncollapse_results"));
	menu->addAction(m_part->action("all_entries_remove_all_results"));

	//assistants, if available
	if (m_linearAlgebraMenu || m_calculateMenu) {
		menu->insertSeparator(firstAction);
		auto* menuAssistants = new QMenu(i18n("Assistants"));
		menuAssistants->setIcon(QIcon::fromTheme(QLatin1String("quickwizard")));
		if (m_linearAlgebraMenu)
			menuAssistants->addMenu(m_linearAlgebraMenu);
		if (m_calculateMenu)
			menuAssistants->addMenu(m_calculateMenu);

		menu->insertMenu(firstAction, menuAssistants);
	}


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
	toolbar->addAction(m_evaluateWorsheetAction);
	toolbar->addAction(m_find);
	toolbar->addAction(m_zoomIn);
	toolbar->addAction(m_zoomOut);
	toolbar->addSeparator();
	toolbar->addAction(m_restartBackendAction);
}

/*!
 * Slot for actions triggered
 */
void CantorWorksheetView::triggerAction(QAction* action) {
	const auto& name = action->data().toString();
	if (!name.isEmpty()) {
		auto* action = m_part->action(name.toStdString().c_str());
		if (action)
			action->trigger();
	}
}

CantorWorksheetView::~CantorWorksheetView() {
	if (m_part)
		m_part->widget()->setParent(nullptr);
}

void CantorWorksheetView::statusChanged(Cantor::Session::Status status) {
	if (status == Cantor::Session::Running) {
		m_evaluateWorsheetAction->setText(i18n("Interrupt"));
		m_evaluateWorsheetAction->setIcon(QIcon::fromTheme(QLatin1String("dialog-close")));
		Q_EMIT m_worksheet->statusInfo(i18n("Calculating..."));
	} else {
		m_evaluateWorsheetAction->setText(i18n("Evaluate Notebook"));
		m_evaluateWorsheetAction->setIcon(QIcon::fromTheme(QLatin1String("system-run")));
		Q_EMIT m_worksheet->statusInfo(i18n("Ready"));
	}
}
