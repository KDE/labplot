/*
	File                 : ActionsManager.h
	Project              : LabPlot
	Description          : Class managing all actions and their containers (menus and toolbars) in MainWin
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024-2026 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ACTIONSMANAGER_H
#define ACTIONSMANAGER_H

#include <QObject>

class DatapickerImageView;
class NotebookView;
class MainWin;
#ifdef HAVE_SCRIPTING
class ScriptEditor;
#endif
class SpreadsheetView;
class ToggleActionMenu;
class WorksheetView;

class QAction;
class QMenu;
class QToolButton;

class KActionMenu;
class KHamburgerMenu;
class KRecentFilesAction;
class KToggleAction;
class KToggleFullScreenAction;

#ifdef HAVE_PURPOSE
namespace Purpose {
class Menu;
}
#endif

#ifdef HAVE_TOUCHBAR
class KDMacTouchBar;
#endif

class ActionsManager : public QObject {
	Q_OBJECT

public:
	explicit ActionsManager(MainWin*);
	~ActionsManager() override;
	void init();

private:
	void initActions();
	void initMenus();

	void initWorksheetToolbarActions();
	void connectWorksheetToolbarActions(const WorksheetView*);

	void initPlotAreaToolbarActions();
	void connectPlotAreaToolbarActions(const WorksheetView*);

	void initSpreadsheetToolbarActions();
	void connectSpreadsheetToolbarActions(const SpreadsheetView*);

	void initDataExtractorToolbarActions();
	void connectDataExtractorToolbarActions(DatapickerImageView *);

#ifdef HAVE_CANTOR_LIBS
	void initNotebookToolbarActions();
	void connectNotebookToolbarActions(const NotebookView*);
#endif

#ifdef HAVE_SCRIPTING
	void initScriptToolbarActions();
	void connectScriptToolbarActions(const ScriptEditor*);
#endif

	MainWin* m_mainWindow{nullptr};
	friend class MainWin;

#ifdef Q_OS_MAC
#ifdef HAVE_TOUCHBAR
	KDMacTouchBar* m_touchBar;
#endif
	QAction* m_undoIconOnlyAction;
	QAction* m_redoIconOnlyAction;
#endif

	// main menu and toolbar
	KRecentFilesAction* m_recentProjectsAction{nullptr};
	QAction* m_searchAction{nullptr};
	QAction* m_saveAction{nullptr};
	QAction* m_saveAsAction{nullptr};
	QAction* m_printAction{nullptr};
	QAction* m_printPreviewAction{nullptr};
	QAction* m_importFileAction{nullptr};
	QAction* m_importDirAction{nullptr};
	QAction* m_importKaggleDatasetAction{nullptr};
	QAction* m_importSqlAction{nullptr};
	QAction* m_importDatasetAction{nullptr};
	QAction* m_importLabPlotAction{nullptr};
	QAction* m_importOpjAction{nullptr};
	QAction* m_exportAction{nullptr};
	QAction* m_newFolderAction{nullptr};
	QAction* m_newWorkbookAction{nullptr};
	QAction* m_newSpreadsheetAction{nullptr};
	QAction* m_newMatrixAction{nullptr};
	QAction* m_newWorksheetAction{nullptr};
#ifdef HAVE_SCRIPTING
	QAction* m_newPythonScriptAction{nullptr};
	// QList<QAction*> m_newScriptActions;
#endif
	QAction* m_newNotesAction{nullptr};
	QAction* m_newLiveDataSourceAction{nullptr};
	QAction* m_newProjectAction{nullptr};
	QAction* m_openProjectAction{nullptr};
	QAction* m_historyAction{nullptr};
	QAction* m_undoAction{nullptr};
	QAction* m_redoAction{nullptr};
	QAction* m_closeWindowAction{nullptr};
	QAction* m_closeAllWindowsAction{nullptr};
	QAction* m_nextWindowAction{nullptr};
	QAction* m_prevWindowAction{nullptr};
	QAction* m_newDatapickerAction{nullptr};
#ifdef HAVE_CANTOR_LIBS
	QAction* m_lastUsedNotebookAction{nullptr};
	ToggleActionMenu* m_tbNotebook{nullptr};
#endif
// #ifdef HAVE_SCRIPTING
// 	ToggleActionMenu* m_tbScript{nullptr};
// #endif
	ToggleActionMenu* m_tbImport{nullptr};

	// toggling dock widgets, status bar and full screen
	QAction* m_projectExplorerDockAction;
	QAction* m_propertiesDockAction;
	QAction* m_worksheetPreviewAction;
	KToggleAction* m_statusBarAction;
	QAction* m_memoryInfoAction;
	KToggleFullScreenAction* m_fullScreenAction;
	QAction* m_configureNotebookAction;

	// window visibility
	QAction* m_visibilityFolderAction;
	QAction* m_visibilitySubfolderAction;
	QAction* m_visibilityAllAction;

	// spreadsheet
	QAction* m_spreadsheetInsertRowAboveAction{nullptr};
	QAction* m_spreadsheetInsertRowBelowAction{nullptr};
	QAction* m_spreadsheetRemoveRowsAction{nullptr};
	QAction* m_spreadsheetInsertColumnLeftAction{nullptr};
	QAction* m_spreadsheetInsertColumnRightAction{nullptr};
	QAction* m_spreadsheetRemoveColumnsAction{nullptr};
	QAction* m_spreadsheetSortAction{nullptr};
	QAction* m_spreadsheetSortAscAction{nullptr};
	QAction* m_spreadsheetSortDescAction{nullptr};

	// worksheet
	ToggleActionMenu* m_worksheetAddNewPlotMenu{nullptr};
	ToggleActionMenu* m_worksheetZoomMenu{nullptr};
	ToggleActionMenu* m_worksheetMagnificationMenu{nullptr};
	KActionMenu* m_plotAddNewMenu{nullptr};

	QActionGroup* m_worksheetAddNewActionGroup{nullptr};
	QActionGroup* m_worksheetLayoutActionGroup{nullptr};
	QActionGroup* m_worksheeMouseModeActionGroup{nullptr};

	QActionGroup* m_plotMouseModeActionGroup{nullptr};
	QActionGroup* m_plotNavigationActionGroup{nullptr};

	// data extractor
	QActionGroup* m_dataExtractorMouseModeActionGroup{nullptr};
	QActionGroup* m_dataExtractorShiftActionGroup{nullptr};
	QAction* m_dataExtractorAddCurveAction{nullptr};
	ToggleActionMenu* m_dataExtractorZoomMenu{nullptr};
	ToggleActionMenu* m_dataExtractorMagnificationMenu{nullptr};

	// notebook
#ifdef HAVE_CANTOR_LIBS
	QAction* m_notebookRestartAction{nullptr};
	QAction* m_notebookEvaluateAction{nullptr};
	QAction* m_notebookZoomInAction{nullptr};
	QAction* m_notebookZoomOutAction{nullptr};
	QAction* m_notebookFindAction{nullptr};
#endif

	// script
#ifdef HAVE_SCRIPTING
	QAction* m_scriptRunAction{nullptr};
	QAction* m_scriptClearAction{nullptr};
#endif

	// Menus
	QMenu* m_visibilityMenu{nullptr};
	QMenu* m_newMenu{nullptr};
	QMenu* m_importMenu{nullptr};
	QMenu* m_newNotebookMenu{nullptr};
#ifdef HAVE_SCRIPTING
	QMenu* m_newScriptMenu{nullptr};
#endif
	KHamburgerMenu* m_hamburgerMenu{nullptr};

#ifdef HAVE_PURPOSE
	QAction* m_shareAction{nullptr};
	Purpose::Menu* m_shareMenu{nullptr};
	void fillShareMenu();
#endif

private Q_SLOTS:
	void toggleDockWidget(QAction*);
	void toggleStatusBar(bool);
	void toggleMenuBar(bool);
	void toggleMemoryInfo();
	void toggleFullScreen(bool);

	void updateGUI();
	void updateGUIOnProjectChanges();

#ifdef HAVE_CANTOR_LIBS
	void updateNotebookActions();
#endif

#ifdef HAVE_PURPOSE
	void shareActionFinished(const QJsonObject& output, int error, const QString& message);
#endif
};

#endif
