/*
	File                 : ActionsManager.h
	Project              : LabPlot
	Description          : Class managing all actions and their containers (menus and toolbars) in MainWin
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ACTIONSMANAGER_H
#define ACTIONSMANAGER_H

#include <QObject>

class DatapickerView;
class NotebookView;
class MainWin;
class SpreadsheetView;
class ToggleActionMenu;
class WorksheetView;

class QAction;
class QMenu;
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

	void initSpreadsheetToolbarActions();
	void connectSpreadsheetToolbarActions(const SpreadsheetView*);

	void initDataExtractorToolbarActions();
	void connectDataExtractorToolbarActions(const DatapickerView*);

#ifdef HAVE_CANTOR_LIBS
	void initNotebookToolbarActions();
	void connectNotebookToolbarActions(const NotebookView*);
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

	ToggleActionMenu* m_worksheetZoomMenu{nullptr};
	ToggleActionMenu* m_worksheetMagnificationMenu{nullptr};

	QActionGroup* m_worksheetLayoutActionGroup{nullptr};
	QActionGroup* m_worksheeMouseModeActionGroup{nullptr};
	QActionGroup* m_worksheeZoomActionGroup{nullptr};
	QActionGroup* m_worksheeZoomFitActionGroup{nullptr};
	QActionGroup* m_worksheetMagnificationActionGroup{nullptr};

	KRecentFilesAction* m_recentProjectsAction;
	QAction* m_searchAction;
	QAction* m_saveAction;
	QAction* m_saveAsAction;
	QAction* m_printAction;
	QAction* m_printPreviewAction;
	QAction* m_importFileAction;
	QAction* m_importFileAction_2;
	QAction* m_importKaggleDatasetAction;
	QAction* m_importSqlAction;
	QAction* m_importDatasetAction;
	QAction* m_importLabPlotAction;
	QAction* m_importOpjAction;
	QAction* m_exportAction;
	QAction* m_newFolderAction;
	QAction* m_newWorkbookAction;
	QAction* m_newSpreadsheetAction;
	QAction* m_newMatrixAction;
	QAction* m_newWorksheetAction;
	QAction* m_newNotesAction;
	QAction* m_newLiveDataSourceAction;
	QAction* m_newProjectAction;
	QAction* m_openProjectAction;
	QAction* m_historyAction;
	QAction* m_undoAction;
	QAction* m_redoAction;
	QAction* m_closeWindowAction;
	QAction* m_closeAllWindowsAction;
	QAction* m_nextWindowAction;
	QAction* m_prevWindowAction;
	QAction* m_newDatapickerAction;

	// toggling dock widgets, status bar and full screen
	QAction* m_projectExplorerDockAction;
	QAction* m_propertiesDockAction;
	QAction* m_worksheetPreviewAction;
	KToggleAction* m_statusBarAction;
	QAction* m_memoryInfoAction;
	KToggleFullScreenAction* m_fullScreenAction;
	QAction* m_configureCASAction;

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

	// notebook
#ifdef HAVE_CANTOR_LIBS
	QAction* m_notebookRestartAction{nullptr};
	QAction* m_notebookEvaluateAction{nullptr};
	QAction* m_notebookZoomInAction{nullptr};
	QAction* m_notebookZoomOutAction{nullptr};
	QAction* m_notebookFindAction{nullptr};
#endif

	// Menus
	QMenu* m_visibilityMenu{nullptr};
	QMenu* m_newMenu{nullptr};
	QMenu* m_importMenu{nullptr};
	QMenu* m_newNotebookMenu{nullptr};
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
