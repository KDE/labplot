/*
 *	File                 : ActionsManager.cpp
 *	Project              : LabPlot
 *	Description 	     : Class managing all actions and their containers (menus and toolbars) in MainWin
 *	--------------------------------------------------------------------
 *	SPDX-FileCopyrightText: 2024-2025 Alexander Semke <alexander.semke@web.de>
 *	SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "frontend/AboutDialog.h"
#include "backend/core/AbstractAspect.h"
#include "backend/core/Project.h"
#include "backend/core/Settings.h"
#include "backend/datapicker/Datapicker.h"
#include "backend/matrix/Matrix.h"
#include "backend/script/Script.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "frontend/ActionsManager.h"
#include "frontend/MainWin.h"
#include "frontend/datapicker/DatapickerView.h"
#include "frontend/datapicker/DatapickerImageView.h"
#include "frontend/matrix/MatrixView.h"
#include "frontend/script/ScriptEditor.h"
#include "frontend/spreadsheet/SpreadsheetView.h"
#include "frontend/widgets/toggleactionmenu.h"
#include "frontend/widgets/MemoryWidget.h"
#include "frontend/worksheet/WorksheetView.h"

#ifdef HAVE_CANTOR_LIBS
#include "backend/notebook/Notebook.h"
#include "frontend/notebook/NotebookView.h"
#include <cantor/backend.h>
#endif

#include <QActionGroup>
#include <QJsonArray>
#include <QMenuBar>
#include <QStatusBar>

#include <KActionCollection>
#include <KActionMenu>
#include <KColorScheme>
#include <KColorSchemeManager>
#include <KColorSchemeMenu>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KMessageBox>
#include <KRecentFilesAction>
#include <KStandardAction>
#include <KToggleFullScreenAction>
#include <KToolBar>
#include <kxmlguifactory.h>

#ifdef HAVE_PURPOSE
#include <Purpose/AlternativesModel>
#include <Purpose/Menu>
#include <QMimeType>
#include <purpose_version.h>
#endif

#ifdef HAVE_TOUCHBAR
#include "3rdparty/kdmactouchbar/src/kdmactouchbar.h"
#endif

#include <DockWidget.h>
#include <DockManager.h>

/*!
 * \class ActionsManager
 * \brief Class managing all actions and their containers (menus and toolbars) in MainWin.
 * This class is intended to simplify (or not to overload) the code in MainWin.
 *
 * Note, the actions shown in the toolbars are subject of modifications and customizations done by the user via the "Configure Toolbars"-dialog
 * and the modifications (custom icon, name and position of actions) are stored in $GOME/.local/share/kxmlgui5/labplot/labplotui.rc).
 * In this dialog it's only possible to manage the top-level actions . Because of this, it is sufficient to handle in this class the the top-level actions only.
 * For the actions used in the sub-menus it is enough to manage their lifecycle in the objects controlled via these toolbars like Worksheet, etc.
 *
 *  \ingroup frontend
 */

ActionsManager::ActionsManager(MainWin* mainWin) : m_mainWindow(mainWin) {
	// actions need to be created and added to the collection prior to calling setupGUI() in MainWin,
	// create them directly in constructor. everything else will be initialized in init() after setupGUI() was called.
	initActions();
}

void ActionsManager::init() {
	// all toolbars created via the KXMLGUI framework are locked on default:
	//  * on the very first program start, unlock all toolbars
	//  * on later program starts, set stored lock status
	// Furthermore, we want to show icons only after the first program start.
	KConfigGroup groupMain = Settings::group(QStringLiteral("MainWindow"));
	if (groupMain.exists()) {
		// KXMLGUI framework automatically stores "Disabled" for the key "ToolBarsMovable"
		// in case the toolbars are locked -> load this value
		const QString& str = groupMain.readEntry(QStringLiteral("ToolBarsMovable"), "");
		bool locked = (str == QStringLiteral("Disabled"));
		KToolBar::setToolBarsLocked(locked);
	}

	auto* factory = m_mainWindow->factory();

	// in case we're starting for the first time, put all toolbars into the IconOnly mode
	// and maximize the main window. The occurence of LabPlot's own section "MainWin"
	// indicates whether this is the first start or not
	groupMain = Settings::group(QStringLiteral("MainWin"));
	if (!groupMain.exists()) {
		// first start
		KToolBar::setToolBarsLocked(false);

		// show icons only
		const auto& toolbars = factory->containers(QStringLiteral("ToolBar"));
		for (auto* container : toolbars) {
			auto* toolbar = dynamic_cast<QToolBar*>(container);
			if (toolbar)
				toolbar->setToolButtonStyle(Qt::ToolButtonIconOnly);
		}

		m_mainWindow->showMaximized();
	}

	initMenus();

	auto* mainToolBar = qobject_cast<QToolBar*>(factory->container(QStringLiteral("main_toolbar"), m_mainWindow));

#ifdef HAVE_CANTOR_LIBS
	if (groupMain.exists()) {
		const QString& lastUsedNotebook = groupMain.readEntry(QLatin1String("lastUsedNotebook"), QString());
		if (!lastUsedNotebook.isEmpty()) {
			for (auto* action : m_newNotebookMenu->actions()) {
				if (lastUsedNotebook.compare(action->data().toString(), Qt::CaseInsensitive) == 0) {
					m_lastUsedNotebookAction = action;
					break;
				}
			}
		}
	}

	m_tbNotebook = new QToolButton(mainToolBar);
	m_tbNotebook->setPopupMode(QToolButton::MenuButtonPopup);
	m_tbNotebook->setMenu(m_newNotebookMenu); // m_newNotebookMenu is never nullptr and always contains at least the configure cas action
	m_tbNotebook->setDefaultAction(!m_lastUsedNotebookAction ? m_newNotebookMenu->actions().first() : m_lastUsedNotebookAction);
	auto* lastAction = mainToolBar->actions().at(mainToolBar->actions().count() - 2);
	mainToolBar->insertWidget(lastAction, m_tbNotebook);
#endif

	auto* tbImport = new QToolButton(mainToolBar);
	tbImport->setPopupMode(QToolButton::MenuButtonPopup);
	tbImport->setMenu(m_importMenu);
	tbImport->setDefaultAction(m_importFileAction);
	auto* lastAction_ = mainToolBar->actions().at(mainToolBar->actions().count() - 1);
	mainToolBar->insertWidget(lastAction_, tbImport);

	m_tbScript = new QToolButton(mainToolBar);
	m_tbScript->setPopupMode(QToolButton::MenuButtonPopup);
	m_tbScript->setMenu(m_newScriptMenu);
	if (m_newScriptActions.isEmpty()) {
		m_tbScript->setIcon(QIcon::fromTheme(QStringLiteral("quickopen"))); // a placeholder icon since we have no runtimes
		m_tbScript->setEnabled(false);
	} else {
		m_tbScript->setDefaultAction(m_newScriptActions.first());
	}
	auto* _lastAction = mainToolBar->actions().at(mainToolBar->actions().count() - 4);
	mainToolBar->insertWidget(_lastAction, m_tbScript);

	// hamburger menu
	m_hamburgerMenu = KStandardAction::hamburgerMenu(nullptr, nullptr, m_mainWindow->actionCollection());
	m_mainWindow->toolBar()->addAction(m_hamburgerMenu);
	m_hamburgerMenu->hideActionsOf(m_mainWindow->toolBar());
	m_hamburgerMenu->setMenuBar(m_mainWindow->menuBar());

	// load recently used projects
	m_recentProjectsAction->loadEntries(Settings::group(QStringLiteral("Recent Files")));

	// show memory info
	m_memoryInfoAction->setEnabled(m_mainWindow->statusBar()->isEnabled()); // disable/enable menu with statusbar
	const auto& groupMainWin = Settings::group(QStringLiteral("MainWin"));
	bool memoryInfoShown = groupMainWin.readEntry(QStringLiteral("ShowMemoryInfo"), true);
	// DEBUG(Q_FUNC_INFO << ", memory info enabled in config: " << memoryInfoShown)
	m_memoryInfoAction->setChecked(memoryInfoShown);
	if (memoryInfoShown)
		toggleMemoryInfo();

}

ActionsManager::~ActionsManager() {
	m_recentProjectsAction->saveEntries(Settings::group(QStringLiteral("Recent Files")));
}

void ActionsManager::initActions() {
	auto* collection = m_mainWindow->actionCollection();

	// ******************** File-menu *******************************
	// add some standard actions
	m_newProjectAction = KStandardAction::openNew(
		this,
		[=]() {
			m_mainWindow->newProject(true);
		},
		collection);
	m_openProjectAction = KStandardAction::open(m_mainWindow, static_cast<void (MainWin::*)()>(&MainWin::openProject), collection);
	m_recentProjectsAction = KStandardAction::openRecent(m_mainWindow, &MainWin::openRecentProject, collection);
	// m_closeAction = KStandardAction::close(m_mainWindow, &MainWin::closeProject, collection);
	// collection->setDefaultShortcut(m_closeAction, QKeySequence()); // remove the shortcut, QKeySequence::Close will be used for closing sub-windows
	m_saveAction = KStandardAction::save(m_mainWindow, &MainWin::saveProject, collection);
	m_saveAsAction = KStandardAction::saveAs(m_mainWindow, &MainWin::saveProjectAs, collection);
	m_printAction = KStandardAction::print(m_mainWindow, &MainWin::print, collection);
	m_printPreviewAction = KStandardAction::printPreview(m_mainWindow, &MainWin::printPreview, collection);

	QAction* openExample = new QAction(i18n("&Open Example"), collection);
	openExample->setIcon(QIcon::fromTheme(QStringLiteral("folder-documents")));
	collection->addAction(QStringLiteral("file_example_open"), openExample);
	connect(openExample, &QAction::triggered, m_mainWindow, &MainWin::exampleProjectsDialog);

	m_fullScreenAction = KStandardAction::fullScreen(m_mainWindow, &ActionsManager::toggleFullScreen, m_mainWindow, collection);

	// QDEBUG(Q_FUNC_INFO << ", preferences action name:" << KStandardAction::name(KStandardAction::Preferences))
	KStandardAction::preferences(m_mainWindow, &MainWin::settingsDialog, collection);
	// QAction* action = collection->action(KStandardAction::name(KStandardAction::Preferences)));
	KStandardAction::quit(m_mainWindow, &MainWin::close, collection);

	// New Folder/Workbook/Spreadsheet/Matrix/Worksheet/Datasources
	m_newWorkbookAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-workbook-new")), i18n("Workbook"), this);
	collection->addAction(QStringLiteral("new_workbook"), m_newWorkbookAction);
	m_newWorkbookAction->setWhatsThis(i18n("Creates a new workbook for collection spreadsheets, matrices and plots"));
	connect(m_newWorkbookAction, &QAction::triggered, m_mainWindow, &MainWin::newWorkbook);

	m_newDatapickerAction = new QAction(QIcon::fromTheme(QStringLiteral("color-picker-black")), i18n("Data Extractor"), this);
	m_newDatapickerAction->setWhatsThis(i18n("Creates a data extractor for getting data from a picture"));
	collection->addAction(QStringLiteral("new_datapicker"), m_newDatapickerAction);
	connect(m_newDatapickerAction, &QAction::triggered, m_mainWindow, &MainWin::newDatapicker);

	m_newSpreadsheetAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-spreadsheet-new")), i18n("Spreadsheet"), this);
	// 	m_newSpreadsheetAction->setShortcut(Qt::CTRL+Qt::Key_Equal);
	m_newSpreadsheetAction->setWhatsThis(i18n("Creates a new spreadsheet for data editing"));
	collection->addAction(QStringLiteral("new_spreadsheet"), m_newSpreadsheetAction);
	connect(m_newSpreadsheetAction, &QAction::triggered, m_mainWindow, &MainWin::newSpreadsheet);

	m_newMatrixAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-matrix-new")), i18n("Matrix"), this);
	// 	m_newMatrixAction->setShortcut(Qt::CTRL+Qt::Key_Equal);
	m_newMatrixAction->setWhatsThis(i18n("Creates a new matrix for data editing"));
	collection->addAction(QStringLiteral("new_matrix"), m_newMatrixAction);
	connect(m_newMatrixAction, &QAction::triggered, m_mainWindow, &MainWin::newMatrix);

	m_newWorksheetAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-worksheet-new")), i18n("Worksheet"), this);
	// 	m_newWorksheetAction->setShortcut(Qt::ALT+Qt::Key_X);
	m_newWorksheetAction->setWhatsThis(i18n("Creates a new worksheet for data plotting"));
	collection->addAction(QStringLiteral("new_worksheet"), m_newWorksheetAction);
	connect(m_newWorksheetAction, &QAction::triggered, m_mainWindow, &MainWin::newWorksheet);

	for (auto& language : Script::languages) {
		auto* action = new QAction(QIcon::fromTheme(QStringLiteral("quickopen")), language, this);
		action->setData(language);
		action->setWhatsThis(i18n("Creates a new %1 script", language));
		collection->addAction(QLatin1String("new_script_") + language, action);
		connect(action, &QAction::triggered, m_mainWindow, &MainWin::newScript);
		m_newScriptActions << action;
	}

	m_newNotesAction = new QAction(QIcon::fromTheme(QStringLiteral("document-new")), i18n("Note"), this);
	m_newNotesAction->setWhatsThis(i18n("Creates a new note for arbitrary text"));
	collection->addAction(QStringLiteral("new_notes"), m_newNotesAction);
	connect(m_newNotesAction, &QAction::triggered, m_mainWindow, &MainWin::newNotes);

	m_newFolderAction = new QAction(QIcon::fromTheme(QStringLiteral("folder-new")), i18n("Folder"), this);
	m_newFolderAction->setWhatsThis(i18n("Creates a new folder to collect sheets and other elements"));
	collection->addAction(QStringLiteral("new_folder"), m_newFolderAction);
	connect(m_newFolderAction, &QAction::triggered, m_mainWindow, &MainWin::newFolder);

	//"New file datasources"
	m_newLiveDataSourceAction = new QAction(QIcon::fromTheme(QStringLiteral("edit-text-frame-update")), i18n("Live Data Source..."), this);
	m_newLiveDataSourceAction->setWhatsThis(i18n("Creates a live data source to read data from a real time device"));
	collection->addAction(QStringLiteral("new_live_datasource"), m_newLiveDataSourceAction);
	connect(m_newLiveDataSourceAction, &QAction::triggered, m_mainWindow, &MainWin::newLiveDataSource);

	// Import/Export
	m_importFileAction = new QAction(QIcon::fromTheme(QStringLiteral("document-import")), i18n("From File..."), this);
	collection->setDefaultShortcut(m_importFileAction, Qt::CTRL | Qt::SHIFT | Qt::Key_I);
	m_importFileAction->setWhatsThis(i18n("Import data from a regular file"));
	connect(m_importFileAction, &QAction::triggered, this, [=]() {
		m_mainWindow->importFileDialog();
	});

	// second "import from file" action, with a shorter name, to be used in the sub-menu of the "Import"-menu.
	// the first action defined above will be used in the toolbar and touchbar where we need the more detailed name "Import From File".
	m_importFileAction_2 = new QAction(QIcon::fromTheme(QStringLiteral("document-import")), i18n("From File..."), this);
	collection->addAction(QStringLiteral("import_file"), m_importFileAction_2);
	m_importFileAction_2->setWhatsThis(i18n("Import data from a regular file"));
	connect(m_importFileAction_2, &QAction::triggered, this, [=]() {
		m_mainWindow->importFileDialog();
	});

	m_importKaggleDatasetAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-kaggle")), i18n("From kaggle.com..."), this);
	m_importKaggleDatasetAction->setWhatsThis(i18n("Import data from kaggle.com"));
	collection->addAction(QStringLiteral("import_dataset_kaggle"), m_importKaggleDatasetAction);
	connect(m_importKaggleDatasetAction, &QAction::triggered, m_mainWindow, &MainWin::importKaggleDatasetDialog);

	m_importSqlAction = new QAction(QIcon::fromTheme(QStringLiteral("network-server-database")), i18n("From SQL Database..."), this);
	m_importSqlAction->setWhatsThis(i18n("Import data from a SQL database"));
	collection->addAction(QStringLiteral("import_sql"), m_importSqlAction);
	connect(m_importSqlAction, &QAction::triggered, m_mainWindow, &MainWin::importSqlDialog);

	m_importDatasetAction = new QAction(QIcon::fromTheme(QStringLiteral("database-index")), i18n("From Dataset Collection..."), this);
	m_importDatasetAction->setWhatsThis(i18n("Import data from an online dataset"));
	collection->addAction(QStringLiteral("import_dataset_datasource"), m_importDatasetAction);
	connect(m_importDatasetAction, &QAction::triggered, m_mainWindow, &MainWin::importDatasetDialog);

	m_importLabPlotAction = new QAction(QIcon::fromTheme(QStringLiteral("project-open")), i18n("LabPlot Project..."), this);
	m_importLabPlotAction->setWhatsThis(i18n("Import a project from a LabPlot project file (.lml)"));
	collection->addAction(QStringLiteral("import_labplot"), m_importLabPlotAction);
	connect(m_importLabPlotAction, &QAction::triggered, m_mainWindow, &MainWin::importProjectDialog);

#ifdef HAVE_LIBORIGIN
	m_importOpjAction = new QAction(QIcon::fromTheme(QStringLiteral("project-open")), i18n("Origin Project (OPJ)..."), this);
	m_importOpjAction->setWhatsThis(i18n("Import a project from an OriginLab Origin project file (.opj)"));
	collection->addAction(QStringLiteral("import_opj"), m_importOpjAction);
	connect(m_importOpjAction, &QAction::triggered, m_mainWindow, &MainWin::importProjectDialog);
#endif

	m_exportAction = new QAction(QIcon::fromTheme(QStringLiteral("document-export")), i18n("Export..."), this);
	m_exportAction->setWhatsThis(i18n("Export selected element"));
	collection->setDefaultShortcut(m_exportAction, Qt::CTRL | Qt::SHIFT | Qt::Key_E);
	collection->addAction(QStringLiteral("export"), m_exportAction);
	connect(m_exportAction, &QAction::triggered, m_mainWindow, &MainWin::exportDialog);

#ifdef HAVE_PURPOSE
	m_shareAction = new QAction(QIcon::fromTheme(QStringLiteral("document-share")), i18n("Share"), this);
	collection->addAction(QStringLiteral("share"), m_shareAction);
#endif

	// Tools
	auto* action = new QAction(QIcon::fromTheme(QStringLiteral("color-management")), i18n("Color Maps Browser"), this);
	action->setWhatsThis(i18n("Open dialog to browse through the available color maps."));
	collection->addAction(QStringLiteral("color_maps"), action);
	// connect(action, &QAction::triggered, this, [=]() {
	// 	auto* dlg = new ColorMapsDialog(this);
	// 	dlg->exec();
	// 	delete dlg;
	// });

#ifdef HAVE_FITS
	action = new QAction(QIcon::fromTheme(QStringLiteral("editor")), i18n("FITS Metadata Editor..."), this);
	action->setWhatsThis(i18n("Open editor to edit FITS meta data"));
	collection->addAction(QStringLiteral("edit_fits"), action);
	connect(action, &QAction::triggered, m_mainWindow, &MainWin::editFitsFileDialog);
#endif

	// Edit
	// Undo/Redo-stuff
	m_undoAction = KStandardAction::undo(m_mainWindow, SLOT(undo()), collection);
	m_redoAction = KStandardAction::redo(m_mainWindow, SLOT(redo()), collection);
	m_historyAction = new QAction(QIcon::fromTheme(QStringLiteral("view-history")), i18n("Undo/Redo History..."), this);
	collection->addAction(QStringLiteral("history"), m_historyAction);
	connect(m_historyAction, &QAction::triggered, m_mainWindow, &MainWin::historyDialog);

#ifdef Q_OS_MAC
	m_undoIconOnlyAction = new QAction(m_undoAction->icon(), QString());
	connect(m_undoIconOnlyAction, &QAction::triggered, m_mainWindow, &MainWin::undo);

	m_redoIconOnlyAction = new QAction(m_redoAction->icon(), QString());
	connect(m_redoIconOnlyAction, &QAction::triggered, m_mainWindow, &MainWin::redo);
#endif
	// TODO: more menus
	//  Appearance
	// Analysis: see WorksheetView.cpp
	// Drawing
	// Script

	// Windows
	m_closeWindowAction = new QAction(i18n("&Close"), this);
	collection->setDefaultShortcut(m_closeWindowAction, QKeySequence::Close);
	m_closeWindowAction->setStatusTip(i18n("Close the active window"));
	collection->addAction(QStringLiteral("close window"), m_closeWindowAction);

	m_closeAllWindowsAction = new QAction(i18n("Close &All"), this);
	m_closeAllWindowsAction->setStatusTip(i18n("Close all the windows"));
	collection->addAction(QStringLiteral("close all windows"), m_closeAllWindowsAction);

	m_nextWindowAction = new QAction(QIcon::fromTheme(QStringLiteral("go-next-view")), i18n("Ne&xt"), this);
	collection->setDefaultShortcut(m_nextWindowAction, QKeySequence::NextChild);
	m_nextWindowAction->setStatusTip(i18n("Move the focus to the next window"));
	collection->addAction(QStringLiteral("next window"), m_nextWindowAction);

	m_prevWindowAction = new QAction(QIcon::fromTheme(QStringLiteral("go-previous-view")), i18n("Pre&vious"), this);
	collection->setDefaultShortcut(m_prevWindowAction, QKeySequence::PreviousChild);
	m_prevWindowAction->setStatusTip(i18n("Move the focus to the previous window"));
	collection->addAction(QStringLiteral("previous window"), m_prevWindowAction);

	// Actions for window visibility
	auto* windowVisibilityActions = new QActionGroup(this);
	windowVisibilityActions->setExclusive(true);

	m_visibilityFolderAction = new QAction(QIcon::fromTheme(QStringLiteral("folder")), i18n("Current &Folder Only"), windowVisibilityActions);
	m_visibilityFolderAction->setCheckable(true);
	m_visibilityFolderAction->setData(static_cast<int>(Project::DockVisibility::folderOnly));

	m_visibilitySubfolderAction =
		new QAction(QIcon::fromTheme(QStringLiteral("folder-documents")), i18n("Current Folder and &Subfolders"), windowVisibilityActions);
	m_visibilitySubfolderAction->setCheckable(true);
	m_visibilitySubfolderAction->setData(static_cast<int>(Project::DockVisibility::folderAndSubfolders));

	m_visibilityAllAction = new QAction(i18n("&All"), windowVisibilityActions);
	m_visibilityAllAction->setCheckable(true);
	m_visibilityAllAction->setData(static_cast<int>(Project::DockVisibility::allDocks));

	connect(windowVisibilityActions, &QActionGroup::triggered, m_mainWindow, &MainWin::setDockVisibility);

	// show/hide the status and menu bars
	//  KMainWindow should provide a menu that allows showing/hiding of the statusbar via showStatusbar()
	//  see https://api.kde.org/frameworks/kxmlgui/html/classKXmlGuiWindow.html#a3d7371171cafabe30cb3bb7355fdfed1
	// KXMLGUI framework automatically stores "Disabled" for the key "StatusBar"
	KConfigGroup groupMain = Settings::group(QStringLiteral("MainWindow"));
	const QString& str = groupMain.readEntry(QStringLiteral("StatusBar"), "");
	bool statusBarDisabled = (str == QStringLiteral("Disabled"));
	DEBUG(Q_FUNC_INFO << ", statusBar enabled in config: " << !statusBarDisabled)
	m_mainWindow->createStandardStatusBarAction();
	m_statusBarAction = KStandardAction::showStatusbar(this, &ActionsManager::toggleStatusBar, collection);
	m_statusBarAction->setChecked(!statusBarDisabled);
	m_mainWindow->statusBar()->setEnabled(!statusBarDisabled); // setVisible() does not work

	KStandardAction::showMenubar(this, &ActionsManager::toggleMenuBar, collection);

	// show/hide the memory usage widget
	m_memoryInfoAction = new QAction(i18n("Show Memory Usage"), this);
	m_memoryInfoAction->setCheckable(true);
	connect(m_memoryInfoAction, &QAction::triggered, this, &ActionsManager::toggleMemoryInfo);

	// Actions for hiding/showing the dock widgets
	auto* docksActions = new QActionGroup(this);
	docksActions->setExclusive(false);

	m_projectExplorerDockAction = new QAction(QIcon::fromTheme(QStringLiteral("view-list-tree")), i18n("Project Explorer"), docksActions);
	m_projectExplorerDockAction->setCheckable(true);
	m_projectExplorerDockAction->setChecked(true);
	collection->addAction(QStringLiteral("toggle_project_explorer_dock"), m_projectExplorerDockAction);

	m_propertiesDockAction = new QAction(QIcon::fromTheme(QStringLiteral("view-list-details")), i18n("Properties Explorer"), docksActions);
	m_propertiesDockAction->setCheckable(true);
	m_propertiesDockAction->setChecked(true);
	collection->addAction(QStringLiteral("toggle_properties_explorer_dock"), m_propertiesDockAction);

	m_worksheetPreviewAction = new QAction(QIcon::fromTheme(QStringLiteral("view-preview")), i18n("Worksheet Preview"), docksActions);
	m_worksheetPreviewAction->setCheckable(true);
	m_worksheetPreviewAction->setChecked(true);
	collection->addAction(QStringLiteral("toggle_worksheet_preview_dock"), m_worksheetPreviewAction);

	connect(docksActions, &QActionGroup::triggered, this, &ActionsManager::toggleDockWidget);

	// global search
	m_searchAction = new QAction(collection);
	m_searchAction->setShortcut(QKeySequence::Find);
	// connect(m_searchAction, &QAction::triggered, this, [=]() {
	// 	if (m_project) {
	// 		if (!m_projectExplorerDock->isVisible()) {
	// 			m_projectExplorerDockAction->setChecked(true);
	// 			toggleDockWidget(m_projectExplorerDockAction);
	// 		}
	// 		m_projectExplorer->search();
	// 	}
	// });
	m_mainWindow->addAction(m_searchAction);

#ifdef HAVE_CANTOR_LIBS
	// configure CAS backends
	m_configureNotebookAction = new QAction(QIcon::fromTheme(QStringLiteral("cantor")), i18n("Configure CAS..."), this);
	m_configureNotebookAction->setWhatsThis(i18n("Opens the settings for Computer Algebra Systems to modify the available systems or to enable new ones"));
	m_configureNotebookAction->setMenuRole(QAction::NoRole); // prevent macOS Qt heuristics to select this action for preferences
	collection->addAction(QStringLiteral("configure_cas"), m_configureNotebookAction);
	connect(m_configureNotebookAction, &QAction::triggered, m_mainWindow, &MainWin::settingsNotebookDialog);
#endif

	// hide "Donate" in the help menu
	auto* donateAction = collection->action(QStringLiteral("help_donate"));
	if (donateAction)
		collection->removeAction(donateAction);

	// custom about dialog
	auto* aboutAction = m_mainWindow->actionCollection()->action(QStringLiteral("help_about_app"));
	if (aboutAction) {
		// set menu icon
		aboutAction->setIcon(KAboutData::applicationData().programLogo().value<QIcon>());

		// disconnect default slot
		disconnect(aboutAction, nullptr, nullptr, nullptr);
		connect(aboutAction, &QAction::triggered, this,
		[=]() {
			AboutDialog aboutDialog(KAboutData::applicationData(), m_mainWindow);
			aboutDialog.exec();
		});
	}

	// actions used in the toolbars
	initSpreadsheetToolbarActions();
	initWorksheetToolbarActions();
	initPlotAreaToolbarActions();
	initDataExtractorToolbarActions();
#ifdef HAVE_CANTOR_LIBS
	initNotebookToolbarActions();
#endif
}

/*!
 * initializes worksheet related actions shown in the toolbar.
 */
void ActionsManager::initWorksheetToolbarActions() {
	auto* collection = m_mainWindow->actionCollection();

	// "add new" actions
	m_worksheetAddNewActionGroup = new QActionGroup(m_mainWindow);
	m_worksheetAddNewPlotMenu = new ToggleActionMenu(i18nc("@action", "New Plot Area"), this);
	m_worksheetAddNewPlotMenu->setPopupMode(QToolButton::MenuButtonPopup);
	connect(m_worksheetAddNewPlotMenu->menu(), &QMenu::triggered, m_worksheetAddNewPlotMenu, &ToggleActionMenu::setDefaultAction);
	collection->addAction(QStringLiteral("worksheet_new_plot_area"), m_worksheetAddNewPlotMenu);

	auto* action = new QAction(QIcon::fromTheme(QStringLiteral("draw-text")), i18n("Text"), m_worksheetAddNewActionGroup);
	collection->addAction(QStringLiteral("worksheet_new_text_label"), action);
	action->setData(static_cast<int>(WorksheetView::AddNewMode::TextLabel));

	action = new QAction(QIcon::fromTheme(QStringLiteral("viewimage")), i18n("Image"), m_worksheetAddNewActionGroup);
	collection->addAction(QStringLiteral("worksheet_new_image"), action);
	action->setData(static_cast<int>(WorksheetView::AddNewMode::Image));

	// layout actions
	m_worksheetLayoutActionGroup = new QActionGroup(m_mainWindow);

	action = new QAction(QIcon::fromTheme(QStringLiteral("labplot-edithlayout")), i18n("Vertical Layout"), m_worksheetLayoutActionGroup);
	action->setData(static_cast<int>(Worksheet::Layout::VerticalLayout));
	action->setCheckable(true);
	collection->addAction(QStringLiteral("worksheet_vertical_layout"), action);

	action = new QAction(QIcon::fromTheme(QStringLiteral("labplot-editvlayout")), i18n("Horizontal Layout"), m_worksheetLayoutActionGroup);
	action->setData(static_cast<int>(Worksheet::Layout::HorizontalLayout));
	action->setCheckable(true);
	collection->addAction(QStringLiteral("worksheet_horizontal_layout"), action);

	action = new QAction(QIcon::fromTheme(QStringLiteral("labplot-editgrid")), i18n("Grid Layout"), m_worksheetLayoutActionGroup);
	action->setData(static_cast<int>(Worksheet::Layout::GridLayout));
	action->setCheckable(true);
	collection->addAction(QStringLiteral("worksheet_grid_layout"), action);

	action = new QAction(QIcon::fromTheme(QStringLiteral("labplot-editbreaklayout")), i18n("No Layout"), m_worksheetLayoutActionGroup);
	action->setData(static_cast<int>(Worksheet::Layout::NoLayout));
	action->setEnabled(false);
	collection->addAction(QStringLiteral("worksheet_break_layout"), action);

	// mouse mode actions
	m_worksheeMouseModeActionGroup = new QActionGroup(m_mainWindow);

	action = new QAction(QIcon::fromTheme(QStringLiteral("labplot-cursor-arrow")), i18n("Select and Edit"), m_worksheeMouseModeActionGroup);
	action->setData(static_cast<int>(WorksheetView::MouseMode::Selection));
	action->setCheckable(true);
	collection->addAction(QStringLiteral("worksheet_select_mode"), action);

	action = new QAction(QIcon::fromTheme(QStringLiteral("input-mouse")), i18n("Navigate"), m_worksheeMouseModeActionGroup);
	action->setData(static_cast<int>(WorksheetView::MouseMode::Navigation));
	action->setCheckable(true);
	collection->addAction(QStringLiteral("worksheet_navigate_mode"), action);

	action = new QAction(QIcon::fromTheme(QStringLiteral("page-zoom")), i18n("Select and Zoom"), m_worksheeMouseModeActionGroup);
	action->setData(static_cast<int>(WorksheetView::MouseMode::ZoomSelection));
	action->setCheckable(true);
	collection->addAction(QStringLiteral("worksheet_zoom_select_mode"), action);

	// zoom actions
	m_worksheetZoomMenu = new ToggleActionMenu(i18nc("@action", "Zoom"), this);
	m_worksheetZoomMenu->setPopupMode(QToolButton::MenuButtonPopup);
	connect(m_worksheetZoomMenu->menu(), &QMenu::triggered, m_worksheetZoomMenu, &ToggleActionMenu::setDefaultAction);
	collection->addAction(QStringLiteral("worksheet_zoom"), m_worksheetZoomMenu);

	// magnification actions
	m_worksheetMagnificationMenu = new ToggleActionMenu(i18nc("@action", "Magnification"), this);
	m_worksheetMagnificationMenu->setPopupMode(QToolButton::MenuButtonPopup);
	connect(m_worksheetMagnificationMenu->menu(), &QMenu::triggered, m_worksheetMagnificationMenu, &ToggleActionMenu::setDefaultAction);
	collection->addAction(QStringLiteral("worksheet_magnification"), m_worksheetMagnificationMenu);
}

/*!
 * initializes plot area related actions shown in the toolbar.
 */
void ActionsManager::initPlotAreaToolbarActions() {
	auto* collection = m_mainWindow->actionCollection();

	// "add new" actions
	m_plotAddNewMenu = new KActionMenu(QIcon::fromTheme(QStringLiteral("list-add")), i18nc("@action", "Add New"), this);
	collection->addAction(QStringLiteral("plot_area_add_new"), m_plotAddNewMenu);

	// mouse mode
	m_plotMouseModeActionGroup = new QActionGroup(this);
	m_plotMouseModeActionGroup->setExclusive(true);
	auto* action = new QAction(QIcon::fromTheme(QStringLiteral("labplot-cursor-arrow")), i18n("Select and Edit"), m_plotMouseModeActionGroup);
	action->setData(static_cast<int>(CartesianPlot::MouseMode::Selection));
	action->setCheckable(true);
	action->setChecked(true);
	collection->addAction(QStringLiteral("plot_area_select_mode"), action);

	action = new QAction(QIcon::fromTheme(QStringLiteral("crosshairs")), i18n("Crosshair"), m_plotMouseModeActionGroup);
	action->setData(static_cast<int>(CartesianPlot::MouseMode::Crosshair));
	action->setCheckable(true);
	collection->addAction(QStringLiteral("plot_area_crosshair_mode"), action);

	action = new QAction(QIcon::fromTheme(QStringLiteral("labplot-zoom-select")), i18n("Select Region and Zoom In"), m_plotMouseModeActionGroup);
	action->setData(static_cast<int>(CartesianPlot::MouseMode::ZoomSelection));
	action->setCheckable(true);
	collection->addAction(QStringLiteral("plot_area_zoom_select_mode"), action);

	action = new QAction(QIcon::fromTheme(QStringLiteral("labplot-zoom-select-x")), i18n("Select X-Region and Zoom In"), m_plotMouseModeActionGroup);
	action->setData(static_cast<int>(CartesianPlot::MouseMode::ZoomXSelection));
	action->setCheckable(true);
	collection->addAction(QStringLiteral("plot_area_zoom_x_select_mode"), action);

	action = new QAction(QIcon::fromTheme(QStringLiteral("labplot-zoom-select-y")), i18n("Select Y-Region and Zoom In"), m_plotMouseModeActionGroup);
	action->setData(static_cast<int>(CartesianPlot::MouseMode::ZoomYSelection));
	action->setCheckable(true);
	collection->addAction(QStringLiteral("plot_area_zoom_y_select_mode"), action);

	action = new QAction(QIcon::fromTheme(QStringLiteral("debug-execute-from-cursor")), i18n("Cursor"), m_plotMouseModeActionGroup);
	action->setData(static_cast<int>(CartesianPlot::MouseMode::Cursor));
	action->setCheckable(true);
	collection->addAction(QStringLiteral("plot_area_cursor_mode"), action);

	// scale
	m_plotNavigationActionGroup = new QActionGroup(this);

	action = new QAction(QIcon::fromTheme(QStringLiteral("labplot-auto-scale-all")), i18n("Auto Scale"), m_plotNavigationActionGroup);
	action->setData(static_cast<int>(CartesianPlot::NavigationOperation::ScaleAuto));
	collection->addAction(QStringLiteral("plot_area_scale_auto"), action);

	action = new QAction(QIcon::fromTheme(QStringLiteral("labplot-auto-scale-x")), i18n("Auto Scale X"), m_plotNavigationActionGroup);
	action->setData(static_cast<int>(CartesianPlot::NavigationOperation::ScaleAutoX));
	collection->addAction(QStringLiteral("plot_area_scale_auto_x"), action);

	action = new QAction(QIcon::fromTheme(QStringLiteral("labplot-auto-scale-y")), i18n("Auto Scale Y"), m_plotNavigationActionGroup);
	action->setData(static_cast<int>(CartesianPlot::NavigationOperation::ScaleAutoY));
	collection->addAction(QStringLiteral("plot_area_scale_auto_y"), action);

	// zoom
	action = new QAction(QIcon::fromTheme(QStringLiteral("zoom-in")), i18n("Zoom In"), m_plotNavigationActionGroup);
	action->setData(static_cast<int>(CartesianPlot::NavigationOperation::ZoomIn));
	collection->addAction(QStringLiteral("plot_area_zoom_in"), action);

	action = new QAction(QIcon::fromTheme(QStringLiteral("zoom-out")), i18n("Zoom Out"), m_plotNavigationActionGroup);
	action->setData(static_cast<int>(CartesianPlot::NavigationOperation::ZoomOut));
	collection->addAction(QStringLiteral("plot_area_zoom_out"), action);

	action = new QAction(QIcon::fromTheme(QStringLiteral("labplot-zoom-in-x")), i18n("Zoom In X"), m_plotNavigationActionGroup);
	action->setData(static_cast<int>(CartesianPlot::NavigationOperation::ZoomInX));
	collection->addAction(QStringLiteral("plot_area_zoom_in_x"), action);

	action = new QAction(QIcon::fromTheme(QStringLiteral("labplot-zoom-out-x")), i18n("Zoom Out X"), m_plotNavigationActionGroup);
	action->setData(static_cast<int>(CartesianPlot::NavigationOperation::ZoomOutX));
	collection->addAction(QStringLiteral("plot_area_zoom_out_x"), action);

	action = new QAction(QIcon::fromTheme(QStringLiteral("labplot-zoom-in-y")), i18n("Zoom In Y"), m_plotNavigationActionGroup);
	action->setData(static_cast<int>(CartesianPlot::NavigationOperation::ZoomInY));
	collection->addAction(QStringLiteral("plot_area_zoom_in_y"), action);

	action = new QAction(QIcon::fromTheme(QStringLiteral("labplot-zoom-out-y")), i18n("Zoom Out Y"), m_plotNavigationActionGroup);
	action->setData(static_cast<int>(CartesianPlot::NavigationOperation::ZoomOutY));
	collection->addAction(QStringLiteral("plot_area_zoom_out_y"), action);

	// shift
	action = new QAction(QIcon::fromTheme(QStringLiteral("labplot-shift-left-x")), i18n("Shift Left X"), m_plotNavigationActionGroup);
	action->setData(static_cast<int>(CartesianPlot::NavigationOperation::ShiftLeftX));
	collection->addAction(QStringLiteral("plot_area_shift_left"), action);

	action = new QAction(QIcon::fromTheme(QStringLiteral("labplot-shift-right-x")), i18n("Shift Right X"), m_plotNavigationActionGroup);
	action->setData(static_cast<int>(CartesianPlot::NavigationOperation::ShiftRightX));
	collection->addAction(QStringLiteral("plot_area_shift_right"), action);

	action = new QAction(QIcon::fromTheme(QStringLiteral("labplot-shift-up-y")), i18n("Shift Up Y"), m_plotNavigationActionGroup);
	action->setData(static_cast<int>(CartesianPlot::NavigationOperation::ShiftUpY));
	collection->addAction(QStringLiteral("plot_area_shift_up"), action);

	action = new QAction(QIcon::fromTheme(QStringLiteral("labplot-shift-down-y")), i18n("Shift Down Y"), m_plotNavigationActionGroup);
	action->setData(static_cast<int>(CartesianPlot::NavigationOperation::ShiftDownY));
	collection->addAction(QStringLiteral("plot_area_shift_down"), action);
}

/*!
 * initializes spreadsheet related actions shown in the toolbar.
 */
void ActionsManager::initSpreadsheetToolbarActions() {
	auto* collection = m_mainWindow->actionCollection();

	// rows
	m_spreadsheetInsertRowAboveAction = new QAction(QIcon::fromTheme(QStringLiteral("edit-table-insert-row-above")), i18n("Insert Row Above"), m_mainWindow);
	collection->addAction(QStringLiteral("spreadsheet_insert_row_above"), m_spreadsheetInsertRowAboveAction);

	m_spreadsheetInsertRowBelowAction = new QAction(QIcon::fromTheme(QStringLiteral("edit-table-insert-row-below")), i18n("Insert Row Below"), m_mainWindow);
	collection->addAction(QStringLiteral("spreadsheet_insert_row_below"), m_spreadsheetInsertRowBelowAction);

	m_spreadsheetRemoveRowsAction = new QAction(QIcon::fromTheme(QStringLiteral("edit-table-delete-row")), i18n("Remo&ve Selected Row(s)"), m_mainWindow);
	collection->addAction(QStringLiteral("spreadsheet_remove_rows"), m_spreadsheetRemoveRowsAction);

	// columns
	m_spreadsheetInsertColumnLeftAction = new QAction(QIcon::fromTheme(QStringLiteral("edit-table-insert-column-left")), i18n("Insert Column Left"), m_mainWindow);
	collection->addAction(QStringLiteral("spreadsheet_insert_column_left"), m_spreadsheetInsertColumnLeftAction);

	m_spreadsheetInsertColumnRightAction = new QAction(QIcon::fromTheme(QStringLiteral("edit-table-insert-column-right")), i18n("Insert Column Right"), m_mainWindow);
	collection->addAction(QStringLiteral("spreadsheet_insert_column_right"), m_spreadsheetInsertColumnRightAction);

	m_spreadsheetRemoveColumnsAction = new QAction(QIcon::fromTheme(QStringLiteral("edit-table-delete-column")), i18n("Delete Selected Column(s)"), m_mainWindow);
	collection->addAction(QStringLiteral("spreadsheet_remove_columns"), m_spreadsheetRemoveColumnsAction);

	// sort
	m_spreadsheetSortAction = new QAction(QIcon::fromTheme(QStringLiteral("view-sort")), i18n("&Sort..."), m_mainWindow);
	m_spreadsheetSortAction->setToolTip(i18n("Sort multiple columns together"));
	collection->addAction(QStringLiteral("spreadsheet_sort"), m_spreadsheetSortAction);

	m_spreadsheetSortAscAction = new QAction(QIcon::fromTheme(QStringLiteral("view-sort-ascending")), i18n("Sort &Ascending"), m_mainWindow);
	m_spreadsheetSortAscAction->setToolTip(i18n("Sort the selected columns separately in ascending order"));
	collection->addAction(QStringLiteral("spreadsheet_sort_asc"), m_spreadsheetSortAscAction);

	m_spreadsheetSortDescAction = new QAction(QIcon::fromTheme(QStringLiteral("view-sort-descending")), i18n("Sort &Descending"), m_mainWindow);
	m_spreadsheetSortDescAction->setToolTip(i18n("Sort the selected columns separately in descending order"));
	collection->addAction(QStringLiteral("spreadsheet_sort_desc"), m_spreadsheetSortDescAction);
}

/*!
 * initializes notebook related actions shown in the toolbar.
 */
#ifdef HAVE_CANTOR_LIBS
void ActionsManager::initNotebookToolbarActions() {
	auto* collection = m_mainWindow->actionCollection();

	m_notebookZoomInAction = new QAction(QIcon::fromTheme(QStringLiteral("zoom-in")), i18n("Zoom In"), m_mainWindow);
	collection->addAction(QStringLiteral("notebook_zoom_in"), m_notebookZoomInAction);

	m_notebookZoomOutAction = new QAction(QIcon::fromTheme(QStringLiteral("zoom-out")), i18n("Zoom Out"), m_mainWindow);
	collection->addAction(QStringLiteral("notebook_zoom_out"), m_notebookZoomOutAction);

	m_notebookFindAction = new QAction(QIcon::fromTheme(QStringLiteral("edit-find")), i18n("Find"), m_mainWindow);
	collection->addAction(QStringLiteral("notebook_find"), m_notebookFindAction);

	m_notebookRestartAction = new QAction(QIcon::fromTheme(QStringLiteral("system-reboot")), i18n("Restart"), m_mainWindow);
	collection->addAction(QStringLiteral("notebook_restart"), m_notebookRestartAction);

	m_notebookEvaluateAction = new QAction(QIcon::fromTheme(QStringLiteral("system-run")), i18n("Evaluate Notebook"), m_mainWindow);
	collection->addAction(QStringLiteral("notebook_evaluate"), m_notebookEvaluateAction);
}
#endif

/*!
 * initializes data extractor related actions shown in the toolbar.
 */
void ActionsManager::initDataExtractorToolbarActions() {
	auto* collection = m_mainWindow->actionCollection();

	// mouse mode actions
	m_dataExtractorMouseModeActionGroup = new QActionGroup(m_mainWindow);

	auto* action = new QAction(QIcon::fromTheme(QStringLiteral("labplot-plot-axis-points")), i18n("Set Axis Points"), m_dataExtractorMouseModeActionGroup);
	action->setCheckable(true);
	action->setData(static_cast<int>(DatapickerImageView::MouseMode::ReferencePointsEntry));
	collection->addAction(QStringLiteral("data_extractor_set_reference_points"), action);

	action = new QAction(QIcon::fromTheme(QStringLiteral("labplot-xy-curve-points")), i18n("Set Curve Points"), m_dataExtractorMouseModeActionGroup);
	action->setCheckable(true);
	action->setData(static_cast<int>(DatapickerImageView::MouseMode::CurvePointsEntry));
	collection->addAction(QStringLiteral("data_extractor_set_curve_points"), action);

	action = new QAction(QIcon::fromTheme(QStringLiteral("labplot-xy-curve-segments")), i18n("Select Curve Segments"), m_dataExtractorMouseModeActionGroup);
	action->setCheckable(true);
	action->setData(static_cast<int>(DatapickerImageView::MouseMode::CurveSegmentsEntry));
	collection->addAction(QStringLiteral("data_extractor_set_segments"), action);

	action = new QAction(QIcon::fromTheme(QStringLiteral("input-mouse")), i18n("Navigate"), m_dataExtractorMouseModeActionGroup);
	action->setCheckable(true);
	action->setData(static_cast<int>(DatapickerImageView::MouseMode::Navigation));
	collection->addAction(QStringLiteral("data_extractor_navigate_mode"), action);

	action = new QAction(QIcon::fromTheme(QStringLiteral("page-zoom")), i18n("Select and Zoom"), m_dataExtractorMouseModeActionGroup);
	action->setCheckable(true);
	action->setData(static_cast<int>(DatapickerImageView::MouseMode::ZoomSelection));
	collection->addAction(QStringLiteral("data_extractor_zoom_select_mode"), action);

	// add curve action
	m_dataExtractorAddCurveAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-xy-curve")), i18n("New Curve"), m_mainWindow);
	collection->addAction(QStringLiteral("data_extractor_add_curve"), m_dataExtractorAddCurveAction);

	// shift actions
	m_dataExtractorShiftActionGroup = new QActionGroup(m_mainWindow);

	action = new QAction(QIcon::fromTheme(QStringLiteral("labplot-shift-left-x")), i18n("Shift Left"), m_dataExtractorShiftActionGroup);
	action->setData(static_cast<int>(DatapickerImageView::ShiftOperation::ShiftLeft));
	collection->addAction(QStringLiteral("data_extractor_shift_right"), action);

	action = new QAction(QIcon::fromTheme(QStringLiteral("labplot-shift-right-x")), i18n("Shift Right"), m_dataExtractorShiftActionGroup);
	action->setData(static_cast<int>(DatapickerImageView::ShiftOperation::ShiftRight));
	collection->addAction(QStringLiteral("data_extractor_shift_left"), action);

	action = new QAction(QIcon::fromTheme(QStringLiteral("labplot-shift-down-y")), i18n("Shift Up"), m_dataExtractorShiftActionGroup);
	action->setData(static_cast<int>(DatapickerImageView::ShiftOperation::ShiftUp));
	collection->addAction(QStringLiteral("data_extractor_shift_up"), action);

	action = new QAction(QIcon::fromTheme(QStringLiteral("labplot-shift-up-y")), i18n("Shift Down"), m_dataExtractorShiftActionGroup);
	action->setData(static_cast<int>(DatapickerImageView::ShiftOperation::ShiftDown));
	collection->addAction(QStringLiteral("data_extractor_shift_down"), action);

	// zoom actions
	m_dataExtractorZoomMenu = new ToggleActionMenu(i18nc("@action", "Zoom"), this);
	m_dataExtractorZoomMenu->setPopupMode(QToolButton::MenuButtonPopup);
	connect(m_dataExtractorZoomMenu->menu(), &QMenu::triggered, m_dataExtractorZoomMenu, &ToggleActionMenu::setDefaultAction);
	collection->addAction(QStringLiteral("data_extractor_zoom"), m_dataExtractorZoomMenu);

	// magnification actions
	m_dataExtractorMagnificationMenu = new ToggleActionMenu(i18nc("@action", "Magnification"), this);
	m_dataExtractorMagnificationMenu->setPopupMode(QToolButton::MenuButtonPopup);
	connect(m_dataExtractorMagnificationMenu->menu(), &QMenu::triggered, m_dataExtractorMagnificationMenu, &ToggleActionMenu::setDefaultAction);
	collection->addAction(QStringLiteral("data_extractor_magnification"), m_dataExtractorMagnificationMenu);
}

void ActionsManager::initMenus() {
#ifdef HAVE_PURPOSE
	m_shareMenu = new Purpose::Menu(m_mainWindow);
	m_shareMenu->model()->setPluginType(QStringLiteral("Export"));
	connect(m_shareMenu, &Purpose::Menu::finished, this, &ActionsManager::shareActionFinished);
	m_shareAction->setMenu(m_shareMenu);
#endif

	auto* factory = m_mainWindow->factory();

	// add the actions to toggle the status bar and the project and properties explorer widgets to the "View" menu.
	// this menu is created automatically when the default "full screen" action is created in initActions().
	auto* menu = dynamic_cast<QMenu*>(factory->container(QStringLiteral("view"), m_mainWindow));
	if (menu) {
		menu->addSeparator();
		menu->addAction(m_projectExplorerDockAction);
		menu->addAction(m_propertiesDockAction);
		menu->addAction(m_worksheetPreviewAction);
	}

	// menu in the main toolbar for adding new aspects
	menu = dynamic_cast<QMenu*>(factory->container(QStringLiteral("new"), m_mainWindow));
	if (menu)
		menu->setIcon(QIcon::fromTheme(QStringLiteral("window-new")));

	// menu in the project explorer and in the toolbar for adding new aspects
	m_newMenu = new QMenu(i18n("Add New"), m_mainWindow);
	m_newMenu->setIcon(QIcon::fromTheme(QStringLiteral("window-new")));
	m_newMenu->addAction(m_newFolderAction);
	m_newMenu->addAction(m_newWorkbookAction);
	m_newMenu->addAction(m_newSpreadsheetAction);
	m_newMenu->addAction(m_newMatrixAction);
	m_newMenu->addAction(m_newWorksheetAction);
	m_newMenu->addAction(m_newNotesAction);
	m_newMenu->addAction(m_newDatapickerAction);
	m_newMenu->addSeparator();
	m_newMenu->addAction(m_newLiveDataSourceAction);

	// import menu
	m_importMenu = new QMenu(m_mainWindow);
	m_importMenu->setIcon(QIcon::fromTheme(QStringLiteral("document-import")));
	m_importMenu->addAction(m_importFileAction_2);
	m_importMenu->addAction(m_importSqlAction);
	m_importMenu->addAction(m_importDatasetAction);
	m_importMenu->addAction(m_importKaggleDatasetAction);
	m_importMenu->addSeparator();
	m_importMenu->addAction(m_importLabPlotAction);
#ifdef HAVE_LIBORIGIN
	m_importMenu->addAction(m_importOpjAction);
#endif

	// icon for the menu "import" in the main menu created via the rc file
	menu = qobject_cast<QMenu*>(factory->container(QStringLiteral("import"), m_mainWindow));
	menu->setIcon(QIcon::fromTheme(QStringLiteral("document-import")));

	// menu subwindow visibility policy
	m_visibilityMenu = new QMenu(i18n("Window Visibility"), m_mainWindow);
	m_visibilityMenu->setIcon(QIcon::fromTheme(QStringLiteral("window-duplicate")));
	m_visibilityMenu->addAction(m_visibilityFolderAction);
	m_visibilityMenu->addAction(m_visibilitySubfolderAction);
	m_visibilityMenu->addAction(m_visibilityAllAction);

#ifdef HAVE_FITS
//	m_editMenu->addAction(m_editFitsFileAction);
#endif

	// set the action for the current color scheme checked
	auto group = Settings::group(QStringLiteral("Settings_General"));
	QString schemeName = group.readEntry("ColorScheme");
	// default dark scheme on Windows is not optimal (Breeze dark is better)
	// we can't find out if light or dark mode is used, so we don't switch to Breeze/Breeze dark here
	DEBUG(Q_FUNC_INFO << ", Color scheme = " << STDSTRING(schemeName))
	auto* schemesMenu = KColorSchemeMenu::createMenu(m_mainWindow->m_schemeManager, m_mainWindow);
	schemesMenu->setText(i18n("Color Scheme"));
	schemesMenu->setIcon(QIcon::fromTheme(QStringLiteral("preferences-desktop-color")));
	connect(schemesMenu->menu(), &QMenu::triggered, m_mainWindow, &MainWin::colorSchemeChanged);

	QMenu* settingsMenu = dynamic_cast<QMenu*>(factory->container(QStringLiteral("settings"), m_mainWindow));
	if (settingsMenu) {
		auto* action = settingsMenu->insertSeparator(settingsMenu->actions().constFirst());
		settingsMenu->insertMenu(action, schemesMenu->menu());

		// add m_memoryInfoAction after the "Show status bar" action
		auto actions = settingsMenu->actions();
		const int index = actions.indexOf(m_statusBarAction);
		settingsMenu->insertAction(actions.at(index + 1), m_memoryInfoAction);
	}

	// add Cantor backends to menu and context menu
#ifdef HAVE_CANTOR_LIBS
	auto backendNames = Cantor::Backend::listAvailableBackends();
#if !defined(NDEBUG) || defined(Q_OS_WIN) || defined(Q_OS_MACOS)
	WARN(Q_FUNC_INFO << ", " << backendNames.count() << " Cantor backends available:")
	for (const auto& b : backendNames)
		WARN("Backend: " << STDSTRING(b))
#endif

	// sub-menu shown in the main toolbar
	m_newNotebookMenu = new QMenu(m_mainWindow);

	if (!backendNames.isEmpty()) {
		// sub-menu shown in the main menu bar
		auto* menu = dynamic_cast<QMenu*>(factory->container(QStringLiteral("new_notebook"), m_mainWindow));
		if (menu) {
			menu->setIcon(QIcon::fromTheme(QStringLiteral("cantor")));
			m_newMenu->addSeparator();
			m_newMenu->addMenu(menu);
			updateNotebookActions();
		}
	}
#else
	delete factory->container(QStringLiteral("notebook"), m_mainWindow);
	delete factory->container(QStringLiteral("new_notebook"), m_mainWindow);
	delete factory->container(QStringLiteral("notebook_toolbar"), m_mainWindow);
#endif

	// This menu is at File > Add New > Script
	auto* newScriptMenu = dynamic_cast<QMenu*>(factory->container(QLatin1String("new_script"), m_mainWindow));
	if (newScriptMenu) {
		newScriptMenu->setIcon(QIcon::fromTheme(QLatin1String("quickopen")));
		m_newMenu->addSeparator();
		m_newMenu->addMenu(newScriptMenu);
		m_mainWindow->unplugActionList(QLatin1String("scripts_list"));
		m_mainWindow->plugActionList(QLatin1String("scripts_list"), m_newScriptActions);
	}

	// This menu is the menu of the script QToolButton in the toolbar
	m_newScriptMenu = new QMenu(m_mainWindow);
	m_newScriptMenu->setIcon(QIcon::fromTheme(QLatin1String("quickopen")));
	m_newScriptMenu->addActions(m_newScriptActions);

	if (m_newScriptActions.isEmpty()) {
		newScriptMenu->setEnabled(false);
		m_newScriptMenu->setEnabled(false);
	}
}

void MainWin::colorSchemeChanged(QAction* action) {
	// save the selected color scheme
	auto group = Settings::group(QStringLiteral("Settings_General"));
	const auto& schemeName = KLocalizedString::removeAcceleratorMarker(action->text());
	group.writeEntry(QStringLiteral("ColorScheme"), schemeName);
	group.sync();
}

/*!
 * updates the state of actions, menus and toolbars (enabled or disabled)
 * on project changes (project closes and opens)
 */
void ActionsManager::updateGUIOnProjectChanges() {
	if (m_mainWindow->m_closing)
		return;

	auto* factory = m_mainWindow->guiFactory();
	if (!m_mainWindow->m_dockManagerContent || !m_mainWindow->m_dockManagerContent->focusedDockWidget()) {
		factory->container(QStringLiteral("spreadsheet"), m_mainWindow)->setEnabled(false);
		factory->container(QStringLiteral("matrix"), m_mainWindow)->setEnabled(false);
		factory->container(QStringLiteral("worksheet"), m_mainWindow)->setEnabled(false);
		factory->container(QStringLiteral("data_extractor"), m_mainWindow)->setEnabled(false);
		factory->container(QStringLiteral("spreadsheet_toolbar"), m_mainWindow)->hide();
		factory->container(QStringLiteral("worksheet_toolbar"), m_mainWindow)->hide();
		factory->container(QStringLiteral("cartesian_plot_toolbar"), m_mainWindow)->hide();
		factory->container(QStringLiteral("data_extractor_toolbar"), m_mainWindow)->hide();
#ifdef HAVE_CANTOR_LIBS
		factory->container(QStringLiteral("notebook"), m_mainWindow)->setEnabled(false);
		factory->container(QStringLiteral("notebook_toolbar"), m_mainWindow)->hide();
#endif
		factory->container(QLatin1String("script"), m_mainWindow)->setEnabled(false);
		factory->container(QLatin1String("script_toolbar"), m_mainWindow)->hide();
	}

	m_mainWindow->updateTitleBar();

	// undo/redo actions are disabled in both cases - when the project is closed or opened
	m_undoAction->setEnabled(false);
	m_redoAction->setEnabled(false);
}

bool hasAction(const QList<QAction*>& actions) {
	for (const auto* action : actions) {
		if (!action->isSeparator())
			return true;
	}
	return false;
}

/*
 * updates the state of actions, menus and toolbars (enabled or disabled)
 * depending on the currently active window (worksheet or spreadsheet).
 */
void ActionsManager::updateGUI() {
	if (!m_mainWindow->m_project || m_mainWindow->m_project->isLoading())
		return;

	if (m_mainWindow->m_closing || m_mainWindow->m_projectClosing)
		return;

#ifdef HAVE_TOUCHBAR
	// reset the touchbar
	m_touchBar->clear();
	m_touchBar->addAction(m_undoIconOnlyAction);
	m_touchBar->addAction(m_redoIconOnlyAction);
	m_touchBar->addSeparator();
#endif

	auto* factory = m_mainWindow->guiFactory();
	if (!m_mainWindow->m_dockManagerContent || !m_mainWindow->m_dockManagerContent->focusedDockWidget()) {
		factory->container(QStringLiteral("spreadsheet"), m_mainWindow)->setEnabled(false);
		factory->container(QStringLiteral("matrix"), m_mainWindow)->setEnabled(false);
		factory->container(QStringLiteral("worksheet"), m_mainWindow)->setEnabled(false);
		factory->container(QStringLiteral("data_extractor"), m_mainWindow)->setEnabled(false);
		factory->container(QStringLiteral("spreadsheet_toolbar"), m_mainWindow)->hide();
		factory->container(QStringLiteral("worksheet_toolbar"), m_mainWindow)->hide();
		factory->container(QStringLiteral("cartesian_plot_toolbar"), m_mainWindow)->hide();
		factory->container(QStringLiteral("data_extractor_toolbar"), m_mainWindow)->hide();
#ifdef HAVE_CANTOR_LIBS
		factory->container(QStringLiteral("notebook"), m_mainWindow)->setEnabled(false);
		factory->container(QStringLiteral("notebook_toolbar"), m_mainWindow)->hide();
#endif
		factory->container(QLatin1String("script"), m_mainWindow)->setEnabled(false);
		factory->container(QLatin1String("script_toolbar"), m_mainWindow)->hide();
		m_printAction->setEnabled(false);
		m_printPreviewAction->setEnabled(false);
		m_exportAction->setEnabled(false);
		return;
	} else {
		m_printAction->setEnabled(true);
		m_printPreviewAction->setEnabled(true);
		m_exportAction->setEnabled(true);
	}

#ifdef HAVE_TOUCHBAR
	if (dynamic_cast<Folder*>(m_currentAspect)) {
		m_touchBar->addAction(m_newWorksheetAction);
		m_touchBar->addAction(m_newSpreadsheetAction);
		m_touchBar->addAction(m_newMatrixAction);
	}
#endif

	Q_ASSERT(m_mainWindow->m_currentAspect);

	// Handle the Worksheet-object
	const auto* w = dynamic_cast<Worksheet*>(m_mainWindow->m_currentAspect);
	if (!w)
		w = dynamic_cast<Worksheet*>(m_mainWindow->m_currentAspect->parent(AspectType::Worksheet));

	if (w) {
		bool update = false;
		if (w != m_mainWindow->m_lastWorksheet) {
			m_mainWindow->m_lastWorksheet = w;
			update = true;
		}

		// populate worksheet menu
		auto* view = qobject_cast<WorksheetView*>(w->view());
		auto* menu = qobject_cast<QMenu*>(factory->container(QStringLiteral("worksheet"), m_mainWindow));
		if (update) {
			menu->clear();
			view->createContextMenu(menu);
		} else if (!hasAction(menu->actions()))
			view->createContextMenu(menu);
		menu->setEnabled(true);

		// populate worksheet-toolbar
		if (update)
			connectWorksheetToolbarActions(view);
		auto* toolbar = qobject_cast<QToolBar*>(factory->container(QStringLiteral("worksheet_toolbar"), m_mainWindow));
		toolbar->setVisible(true);
		toolbar->setEnabled(true);

		// populate the toolbar for cartesian plots
		if (update)
			connectPlotAreaToolbarActions(view);
		toolbar = qobject_cast<QToolBar*>(factory->container(QStringLiteral("cartesian_plot_toolbar"), m_mainWindow));
		toolbar->setVisible(true);
		toolbar->setEnabled(true);

		// populate the touchbar on Mac
#ifdef HAVE_TOUCHBAR
		view->fillTouchBar(m_touchBar);
#endif
		// hide the spreadsheet toolbar
		factory->container(QStringLiteral("spreadsheet_toolbar"), m_mainWindow)->setVisible(false);
	} else {
		factory->container(QStringLiteral("worksheet"), m_mainWindow)->setEnabled(false);
		factory->container(QStringLiteral("worksheet_toolbar"), m_mainWindow)->setVisible(false);
		factory->container(QStringLiteral("worksheet_toolbar"), m_mainWindow)->setEnabled(false);
		factory->container(QStringLiteral("cartesian_plot_toolbar"), m_mainWindow)->setEnabled(false);
	}

	// Handle the Spreadsheet-object
	const auto* spreadsheet = m_mainWindow->activeSpreadsheet();
	if (spreadsheet) {
		bool update = false;
		if (spreadsheet != m_mainWindow->m_lastSpreadsheet) {
			update = true;
			m_mainWindow->m_lastSpreadsheet = spreadsheet;
		}

		// populate spreadsheet-menu
		auto* view = qobject_cast<SpreadsheetView*>(spreadsheet->view());
		auto* menu = qobject_cast<QMenu*>(factory->container(QStringLiteral("spreadsheet"), m_mainWindow));
		if (update) {
			menu->clear();
			view->createContextMenu(menu);
		} else if (!hasAction(menu->actions()))
			view->createContextMenu(menu);
		menu->setEnabled(true);

		// spreadsheet-toolbar
		auto* toolbar = qobject_cast<QToolBar*>(factory->container(QStringLiteral("spreadsheet_toolbar"), m_mainWindow));
		if (!view->isReadOnly()) {
			if (update)
				connectSpreadsheetToolbarActions(view);
			toolbar->setVisible(true);
			toolbar->setEnabled(true);
		} else
			toolbar->setVisible(false);

		// populate the touchbar on Mac
#ifdef HAVE_TOUCHBAR
		m_touchBar->addAction(m_importFileAction);
		view->fillTouchBar(m_touchBar);
#endif

		// spreadsheet has its own search, unregister the shortcut for the global search here
		m_searchAction->setShortcut(QKeySequence());
	} else {
		factory->container(QStringLiteral("spreadsheet"), m_mainWindow)->setEnabled(false);
		factory->container(QStringLiteral("spreadsheet_toolbar"), m_mainWindow)->setVisible(false);
		m_searchAction->setShortcut(QKeySequence::Find);
	}

	// Handle the Matrix-object
	const auto* matrix = dynamic_cast<Matrix*>(m_mainWindow->m_currentAspect);
	if (!matrix)
		matrix = dynamic_cast<Matrix*>(m_mainWindow->m_currentAspect->parent(AspectType::Matrix));
	if (matrix) {
		// populate matrix-menu
		auto* view = qobject_cast<MatrixView*>(matrix->view());
		auto* menu = qobject_cast<QMenu*>(factory->container(QStringLiteral("matrix"), m_mainWindow));
		menu->clear();
		view->createContextMenu(menu);
		menu->setEnabled(true);

		// populate the touchbar on Mac
#ifdef HAVE_TOUCHBAR
		m_touchBar->addAction(m_importFileAction);
		// view->fillTouchBar(m_touchBar);
#endif
	} else
		factory->container(QStringLiteral("matrix"), m_mainWindow)->setEnabled(false);

#ifdef HAVE_CANTOR_LIBS
	const auto* notebook = dynamic_cast<Notebook*>(m_mainWindow->m_currentAspect);
	if (!notebook)
		notebook = dynamic_cast<Notebook*>(m_mainWindow->m_currentAspect->parent(AspectType::Notebook));
	if (notebook) {
		auto* view = qobject_cast<NotebookView*>(notebook->view());
		auto* menu = qobject_cast<QMenu*>(factory->container(QStringLiteral("notebook"), m_mainWindow));
		menu->clear();
		view->createContextMenu(menu);
		menu->setEnabled(true);

		connectNotebookToolbarActions(view);
		factory->container(QStringLiteral("notebook"), m_mainWindow)->setEnabled(true);
		factory->container(QStringLiteral("notebook_toolbar"), m_mainWindow)->setVisible(true);
	} else {
		// no notebook selected -> deactivate notebook related menu and toolbar
		factory->container(QStringLiteral("notebook"), m_mainWindow)->setEnabled(false);
		factory->container(QStringLiteral("notebook_toolbar"), m_mainWindow)->setVisible(false);
	}
#endif

	const auto* script = dynamic_cast<Script*>(m_mainWindow->m_currentAspect);
	if (!script)
		script = dynamic_cast<Script*>(m_mainWindow->m_currentAspect->parent(AspectType::Script));
	if (script) {
		auto* view = qobject_cast<ScriptEditor*>(script->view());
		auto* menu = qobject_cast<QMenu*>(factory->container(QLatin1String("script"), m_mainWindow));
		menu->clear();
		view->createContextMenu(menu);
		menu->setEnabled(true);

		auto* toolbar = qobject_cast<QToolBar*>(factory->container(QLatin1String("script_toolbar"), m_mainWindow));
		toolbar->setVisible(true);
		toolbar->clear();
		view->fillToolBar(toolbar);
	} else {
		factory->container(QLatin1String("script"), m_mainWindow)->setEnabled(false);
		factory->container(QLatin1String("script_toolbar"), m_mainWindow)->setVisible(false);
	}

	const auto* datapicker = dynamic_cast<Datapicker*>(m_mainWindow->m_currentAspect);
	if (!datapicker)
		datapicker = dynamic_cast<Datapicker*>(m_mainWindow->m_currentAspect->parent(AspectType::Datapicker));
	if (!datapicker) {
		if (m_mainWindow->m_currentAspect && m_mainWindow->m_currentAspect->type() == AspectType::DatapickerCurve)
			datapicker = dynamic_cast<Datapicker*>(m_mainWindow->m_currentAspect->parentAspect());
	}

	if (datapicker) {
		// menu
		auto* view = qobject_cast<DatapickerView*>(datapicker->view());
		auto* menu = qobject_cast<QMenu*>(factory->container(QStringLiteral("data_extractor"), m_mainWindow));
		menu->clear();
		view->createContextMenu(menu);
		menu->setEnabled(true);

		// toolbar
		factory->container(QStringLiteral("data_extractor"), m_mainWindow)->setEnabled(true);
		factory->container(QStringLiteral("data_extractor_toolbar"), m_mainWindow)->setVisible(true);
	} else {
		factory->container(QStringLiteral("data_extractor"), m_mainWindow)->setEnabled(false);
		factory->container(QStringLiteral("data_extractor_toolbar"), m_mainWindow)->setVisible(false);
	}
}

#ifdef HAVE_CANTOR_LIBS
void ActionsManager::updateNotebookActions() {
	// auto* menu = static_cast<QMenu*>(factory()->container(QLatin1String("new_notebook"), this));
	m_mainWindow->unplugActionList(QLatin1String("backends_list"));
	QList<QAction*> newBackendActions;
	m_newNotebookMenu->clear();
	for (auto* backend : Cantor::Backend::availableBackends()) {
		if (!backend->isEnabled())
			continue;

		auto* action = new QAction(QIcon::fromTheme(backend->icon()), backend->name(), this);
		action->setData(backend->name());
		action->setWhatsThis(i18n("Creates a new %1 notebook", backend->name()));
		m_mainWindow->actionCollection()->addAction(QLatin1String("notebook_") + backend->name(), action);
		connect(action, &QAction::triggered, m_mainWindow, &MainWin::newNotebook);
		newBackendActions << action;
		m_newNotebookMenu->addAction(action);
	}

	m_mainWindow->plugActionList(QLatin1String("backends_list"), newBackendActions);

	m_newNotebookMenu->addSeparator();
	m_newNotebookMenu->addAction(m_configureNotebookAction);

	// we just updated the notebook action list. its possible that the defaultAction isn't in the list anymore
	if (m_tbNotebook && !m_newNotebookMenu->actions().contains(m_tbNotebook->defaultAction()))
		m_tbNotebook->setDefaultAction(m_newNotebookMenu->actions().first());
}
#endif

#ifdef HAVE_PURPOSE
void ActionsManager::fillShareMenu() {
	if (!m_shareMenu)
		return;

	m_shareMenu->clear(); // clear the menu, it will be refilled with the new file URL below
	QMimeType mime;
	m_shareMenu->model()->setInputData(
		QJsonObject{{QStringLiteral("mimeType"), mime.name()}, {QStringLiteral("urls"), QJsonArray{QUrl::fromLocalFile(m_mainWindow->m_project->fileName()).toString()}}});
	m_shareMenu->model()->setPluginType(QStringLiteral("Export"));
	m_shareMenu->reload();
}

void ActionsManager::shareActionFinished(const QJsonObject& output, int error, const QString& message) {
	if (error)
		KMessageBox::error(m_mainWindow, i18n("There was a problem sharing the project: %1", message), i18n("Share"));
	else {
		const QString url = output[QStringLiteral("url")].toString();
		if (url.isEmpty())
			m_mainWindow->statusBar()->showMessage(i18n("Project shared successfully"));
		else
			KMessageBox::information(m_mainWindow->widget(),
									 i18n("You can find the shared project at: <a href=\"%1\">%1</a>", url),
									 i18n("Share"),
									 QString(),
									 KMessageBox::Notify | KMessageBox::AllowLink);
	}
}
#endif

void ActionsManager::toggleStatusBar(bool checked) {
	m_mainWindow->statusBar()->setVisible(checked); // show/hide statusbar
	m_mainWindow->statusBar()->setEnabled(checked);
	// enabled/disable memory info menu with statusbar
	m_memoryInfoAction->setEnabled(checked);
}

void ActionsManager::toggleMemoryInfo() {
	if (m_mainWindow->m_memoryInfoWidget) {
		m_mainWindow->statusBar()->removeWidget(m_mainWindow->m_memoryInfoWidget);
		delete m_mainWindow->m_memoryInfoWidget;
		m_mainWindow->m_memoryInfoWidget = nullptr;
	} else {
		m_mainWindow->m_memoryInfoWidget = new MemoryWidget(m_mainWindow->statusBar());
		m_mainWindow->statusBar()->addPermanentWidget(m_mainWindow->m_memoryInfoWidget);
	}
}

void ActionsManager::toggleMenuBar(bool checked) {
	m_mainWindow->menuBar()->setVisible(checked);
}

void ActionsManager::toggleFullScreen(bool t) {
	m_fullScreenAction->setFullScreen(m_mainWindow, t);
}

void ActionsManager::toggleDockWidget(QAction* action) {
	if (action->objectName() == QStringLiteral("toggle_project_explorer_dock")) {
		if (m_mainWindow->m_projectExplorerDock->isVisible())
			m_mainWindow->m_projectExplorerDock->toggleView(false);
		else
			m_mainWindow->m_projectExplorerDock->toggleView(true);
	} else if (action->objectName() == QStringLiteral("toggle_properties_explorer_dock")) {
		if (m_mainWindow->m_propertiesDock->isVisible())
			m_mainWindow->m_propertiesDock->toggleView(false);
		else
			m_mainWindow->m_propertiesDock->toggleView(true);
	} else if (action->objectName() == QStringLiteral("toggle_worksheet_preview_dock"))
		m_mainWindow->m_worksheetPreviewDock->toggleView(!m_mainWindow->m_worksheetPreviewDock->isVisible());
}


// ##############################################################################
// ################  Connect actions to the actual views ########################
// ##############################################################################
void ActionsManager::connectWorksheetToolbarActions(const WorksheetView* view) {
	// add new actions
	disconnect(m_worksheetAddNewActionGroup, &QActionGroup::triggered, nullptr, nullptr);
	connect(m_worksheetAddNewActionGroup, &QActionGroup::triggered, view, &WorksheetView::addNew);

	// add new plot actions
	m_worksheetAddNewPlotMenu->menu()->clear();
	view->fillAddNewPlotMenu(m_worksheetAddNewPlotMenu);
	m_worksheetAddNewPlotMenu->setDefaultActionFromData(static_cast<int>(view->addNewMode()));

	// layout actions
	disconnect(m_worksheetLayoutActionGroup, &QActionGroup::triggered, nullptr, nullptr);
	connect(m_worksheetLayoutActionGroup, &QActionGroup::triggered, view, &WorksheetView::changeLayout);
	const auto layout = view->layout();
	for (auto* action : m_worksheetLayoutActionGroup->actions()) {
		if (static_cast<Worksheet::Layout>(action->data().toInt()) == layout) {
			action->setChecked(true);
			break;
		}
	}

	// mouse mode actions
	disconnect(m_worksheeMouseModeActionGroup, &QActionGroup::triggered, nullptr, nullptr);
	connect(m_worksheeMouseModeActionGroup, &QActionGroup::triggered, view, &WorksheetView::changeMouseMode);
	const auto mouseMode = view->mouseMode();
	for (auto* action : m_worksheeMouseModeActionGroup->actions()) {
		if (static_cast<WorksheetView::MouseMode>(action->data().toInt()) == mouseMode)
			action->setChecked(true);
	}

	// zoom actions
	m_worksheetZoomMenu->menu()->clear();
	view->fillZoomMenu(m_worksheetZoomMenu);
	m_worksheetZoomMenu->setDefaultActionFromData(static_cast<int>(view->zoomMode()));

	// magnification actions
	m_worksheetMagnificationMenu->menu()->clear();
	view->fillMagnificationMenu(m_worksheetMagnificationMenu);
	m_worksheetMagnificationMenu->setDefaultActionFromData(view->magnification());
}

void ActionsManager::connectPlotAreaToolbarActions(const WorksheetView* view) {
	// add new actions
	m_plotAddNewMenu->setMenu(view->plotAddNewMenu());

	// mouse mode actions
	disconnect(m_plotMouseModeActionGroup, &QActionGroup::triggered, nullptr, nullptr);
	connect(m_plotMouseModeActionGroup, &QActionGroup::triggered, view, &WorksheetView::changePlotMouseMode);

	// zoom/navigation actions
	disconnect(m_plotNavigationActionGroup, &QActionGroup::triggered, nullptr, nullptr);
	connect(m_plotNavigationActionGroup, &QActionGroup::triggered, view, &WorksheetView::changePlotNavigation);

	// register action groups in the view so their state is properly updated on selection changes in the view
	const_cast<WorksheetView*>(view)->registerCartesianPlotActions(m_plotMouseModeActionGroup, m_plotNavigationActionGroup);
}

void ActionsManager::connectSpreadsheetToolbarActions(const SpreadsheetView* view) {
	// disconnect from the old view
	disconnect(m_spreadsheetInsertRowAboveAction, &QAction::triggered, nullptr, nullptr);
	disconnect(m_spreadsheetInsertRowBelowAction, &QAction::triggered, nullptr, nullptr);
	disconnect(m_spreadsheetRemoveRowsAction, &QAction::triggered, nullptr, nullptr);
	disconnect(m_spreadsheetInsertColumnLeftAction, &QAction::triggered, nullptr, nullptr);
	disconnect(m_spreadsheetInsertColumnRightAction, &QAction::triggered, nullptr, nullptr);
	disconnect(m_spreadsheetRemoveColumnsAction, &QAction::triggered, nullptr, nullptr);
	disconnect(m_spreadsheetSortAction, &QAction::triggered, nullptr, nullptr);
	disconnect(m_spreadsheetSortAscAction, &QAction::triggered, nullptr, nullptr);
	disconnect(m_spreadsheetSortDescAction, &QAction::triggered, nullptr, nullptr);

	// connect to the new view
	connect(m_spreadsheetInsertRowAboveAction, &QAction::triggered, view, &SpreadsheetView::insertRowAbove);
	connect(m_spreadsheetInsertRowBelowAction, &QAction::triggered, view, &SpreadsheetView::insertRowBelow);
	connect(m_spreadsheetRemoveRowsAction, &QAction::triggered, view, &SpreadsheetView::removeSelectedRows);
	connect(m_spreadsheetInsertColumnLeftAction, &QAction::triggered, view, &SpreadsheetView::insertColumnLeft);
	connect(m_spreadsheetInsertColumnRightAction, &QAction::triggered, view, &SpreadsheetView::insertColumnRight);
	connect(m_spreadsheetRemoveColumnsAction, &QAction::triggered, view, &SpreadsheetView::removeSelectedColumns);
	connect(m_spreadsheetSortAction, &QAction::triggered, view, &SpreadsheetView::sortCustom);
	connect(m_spreadsheetSortAscAction, &QAction::triggered, view, &SpreadsheetView::sortAscending);
	connect(m_spreadsheetSortDescAction, &QAction::triggered, view, &SpreadsheetView::sortDescending);
}

#ifdef HAVE_CANTOR_LIBS
void ActionsManager::connectNotebookToolbarActions(const NotebookView* view) {
	// disconnect from the old view
	disconnect(m_notebookRestartAction, &QAction::triggered, nullptr, nullptr);
	disconnect(m_notebookEvaluateAction, &QAction::triggered, nullptr, nullptr);
	disconnect(m_notebookZoomInAction, &QAction::triggered, nullptr, nullptr);
	disconnect(m_notebookZoomOutAction, &QAction::triggered, nullptr, nullptr);
	disconnect(m_notebookFindAction, &QAction::triggered, nullptr, nullptr);

	// connect to the new view
	connect(m_notebookRestartAction, &QAction::triggered, view, &NotebookView::restart);
	connect(m_notebookEvaluateAction, &QAction::triggered, view, &NotebookView::evaluate);
	connect(m_notebookZoomInAction, &QAction::triggered, view, &NotebookView::zoomIn);
	connect(m_notebookZoomOutAction, &QAction::triggered, view, &NotebookView::zoomOut);
	connect(m_notebookFindAction, &QAction::triggered, view, &NotebookView::find);
}
#endif

void ActionsManager::connectDataExtractorToolbarActions(const DatapickerImageView* view) {
	// mouse mode actions
	disconnect(m_dataExtractorMouseModeActionGroup, &QActionGroup::triggered, nullptr, nullptr);
	connect(m_dataExtractorMouseModeActionGroup, &QActionGroup::triggered, view, &DatapickerImageView::changeMouseMode);
	const auto mouseMode = view->mouseMode();
	for (auto* action : m_worksheeMouseModeActionGroup->actions()) {
		if (static_cast<DatapickerImageView::MouseMode>(action->data().toInt()) == mouseMode)
			action->setChecked(true);
	}

	// add curve action
	disconnect(m_dataExtractorAddCurveAction, &QAction::triggered, nullptr, nullptr);
	connect(m_dataExtractorAddCurveAction, &QAction::triggered, view, &DatapickerImageView::addCurve);

	// shift actions
	disconnect(m_dataExtractorShiftActionGroup, &QActionGroup::triggered, nullptr, nullptr);
	connect(m_dataExtractorShiftActionGroup, &QActionGroup::triggered, view, &DatapickerImageView::changeSelectedItemsPosition);

	// zoom actions
	m_dataExtractorZoomMenu->menu()->clear();
	view->fillZoomMenu(m_dataExtractorZoomMenu);
	m_dataExtractorZoomMenu->setDefaultActionFromData(static_cast<int>(view->zoomMode()));

	// magnification actions
	m_dataExtractorMagnificationMenu->menu()->clear();
	view->fillMagnificationMenu(m_worksheetMagnificationMenu);
	m_dataExtractorMagnificationMenu->setDefaultActionFromData(view->magnification());
}
