/*
	File                 : MainWin.h
	Project              : LabPlot
	Description          : Main window of the application
	--------------------------------------------------------------------
<<<<<<< HEAD
	SPDX-FileCopyrightText: 2011-2022 Alexander Semke <alexander.semke@web.de>
=======
	SPDX-FileCopyrightText: 2011-2023 Alexander Semke <alexander.semke@web.de>
>>>>>>> master
	SPDX-FileCopyrightText: 2008-2018 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef MAINWIN_H
#define MAINWIN_H

#include "backend/worksheet/plots/cartesian/CartesianPlot.h"

#include <KXmlGuiWindow>
#include <QTimer>

class AbstractAspect;
class AspectTreeModel;
class Folder;
class ProjectExplorer;
class Project;
class Worksheet;
class Note;
class Workbook;
class Datapicker;
class Spreadsheet;
class Matrix;
class GuiObserver;
class CursorDock;
class ContentDockWidget;
class MemoryWidget;
class CartesianPlot;
class InfoElementDialog;

#ifdef HAVE_CANTOR_LIBS
class CantorWorksheet;
#endif

class ImportDatasetWidget;
class TreeModel;
// class WelcomeScreenHelper;

class QDockWidget;
class QDragEnterEvent;
class QDropEvent;
class QMdiArea;
class QMdiSubWindow;
class QStackedWidget;
class QToolButton;
class QQuickWidget;

class KColorSchemeManager;
class KHamburgerMenu;
class KRecentFilesAction;
class KToggleAction;
class KToggleFullScreenAction;

namespace ads {
class CDockManager;
class CDockWidget;
}

#ifdef HAVE_KUSERFEEDBACK
#include <KUserFeedback/Provider>
#endif

#ifdef HAVE_TOUCHBAR
class KDMacTouchBar;
#endif

class MainWin : public KXmlGuiWindow {
	Q_OBJECT

public:
	explicit MainWin(QWidget* parent = nullptr, const QString& filename = QString());
	~MainWin() override;

	void showPresenter();
	AspectTreeModel* model() const;
	Project* project() const;
	void addAspectToProject(AbstractAspect*);
	static void updateLocale();

	enum class LoadOnStart { Nothing, NewProject, NewProjectWorksheet, NewProjectSpreadsheet, LastProject, WelcomeScreen };
	enum class TitleBarMode { ShowFilePath, ShowFileName, ShowProjectName };

#ifdef HAVE_KUSERFEEDBACK
	KUserFeedback::Provider& userFeedbackProvider() {
		return m_userFeedbackProvider;
	}
#endif

private:
	ads::CDockManager* m_DockManager{nullptr};
	KColorSchemeManager* m_schemeManager{nullptr};
	ContentDockWidget* m_currentDock{nullptr}; // Currently selected dock
	Project* m_project{nullptr};
	AspectTreeModel* m_aspectTreeModel{nullptr};
	ProjectExplorer* m_projectExplorer{nullptr};
	ads::CDockWidget* m_projectExplorerDock{nullptr};
	ads::CDockWidget* m_propertiesDock{nullptr};
	AbstractAspect* m_currentAspect{nullptr};
	ads::CDockWidget* m_currentAspectDock{nullptr};
	Folder* m_currentFolder{nullptr};
	QString m_undoViewEmptyLabel;
	bool m_suppressCurrentSubWindowChangedEvent{false};
	bool m_closing{false};
	bool m_projectClosing{false};
	bool m_autoSaveActive{false};
	QTimer m_autoSaveTimer;
	bool m_showWelcomeScreen{false};
	bool m_saveWelcomeScreen{true};
	int undoStackIndexLastSave{0};
	MemoryWidget* m_memoryInfoWidget{nullptr};
	QMdiSubWindow* m_welcomeWindow{nullptr};
	QQuickWidget* m_welcomeWidget{nullptr};
	// 	WelcomeScreenHelper* m_welcomeScreenHelper{nullptr};
	ImportDatasetWidget* m_importDatasetWidget{nullptr};
	QString m_lastOpenFileFilter;
	const Worksheet* m_lastWorksheet{nullptr};
	const Spreadsheet* m_lastSpreadsheet{nullptr};
	TitleBarMode m_titleBarMode{TitleBarMode::ShowFilePath};

#ifdef HAVE_KUSERFEEDBACK
	KUserFeedback::Provider m_userFeedbackProvider;
#endif

#ifdef Q_OS_MAC
#ifdef HAVE_TOUCHBAR
	KDMacTouchBar* m_touchBar;
#endif
	QAction* m_undoIconOnlyAction;
	QAction* m_redoIconOnlyAction;
#endif

	KRecentFilesAction* m_recentProjectsAction;
	QAction* m_searchAction;
	QAction* m_saveAction;
	QAction* m_saveAsAction;
	QAction* m_printAction;
	QAction* m_printPreviewAction;
	QAction* m_importFileAction;
	QAction* m_importFileAction_2;
	QAction* m_importSqlAction;
	QAction* m_importDatasetAction;
	QAction* m_importLabPlotAction;
	QAction* m_importOpjAction;
	QAction* m_exportAction;
	QAction* m_closeAction;
	QAction* m_newFolderAction;
	QAction* m_newWorkbookAction;
	QAction* m_newSpreadsheetAction;
	QAction* m_newMatrixAction;
	QAction* m_newWorksheetAction;
	QAction* m_newNotesAction;
	QAction* m_newLiveDataSourceAction;
	QAction* m_newSqlDataSourceAction;
	QAction* m_newScriptAction;
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
	QAction* m_toggleProjectExplorerDockAction;
	QAction* m_togglePropertiesDockAction;
	KToggleAction* m_toggleStatusBarAction;
	QAction* m_toggleMemoryInfoAction;
	KToggleFullScreenAction* m_toggleFullScreenAction;

	// window visibility
	QAction* m_visibilityFolderAction;
	QAction* m_visibilitySubfolderAction;
	QAction* m_visibilityAllAction;

	// Menus
	QMenu* m_visibilityMenu{nullptr};
	QMenu* m_newMenu{nullptr};
	QMenu* m_importMenu{nullptr};
	KHamburgerMenu* m_hamburgerMenu{nullptr};

	// Docks
	ads::CDockWidget* cursorDock{nullptr};

	QStackedWidget* stackedWidget{nullptr};
	CursorDock* cursorWidget{nullptr};

	void initActions();
	void initMenus();
	bool warnModified();
	void activateSubWindowForAspect(const AbstractAspect*);
	bool save(const QString&);
	// 	void toggleShowWidget(QWidget* widget, bool showToRight);
	// 	void toggleHideWidget(QWidget* widget, bool hideToLeft);

	Spreadsheet* activeSpreadsheet() const;

	friend class GuiObserver;
	GuiObserver* m_guiObserver{nullptr};

protected:
	void closeEvent(QCloseEvent*) override;
	void dragEnterEvent(QDragEnterEvent*) override;
	void dropEvent(QDropEvent*) override;

private Q_SLOTS:
	void initGUI(const QString&);
	// 	QQuickWidget* createWelcomeScreen();
	// 	void resetWelcomeScreen();
	// void createMdiArea();
	void createADS();
	void changeVisibleAllDocks(bool);
	void activateNextDock();
	void activatePreviousDock();
	void dockWidgetAboutToBeRemoved(ads::CDockWidget*);
	void dockWidgetRemoved(ads::CDockWidget*);
	void dockFocusChanged(ads::CDockWidget* old, ads::CDockWidget* now);
	void updateGUI();
	void updateGUIOnProjectChanges(const QByteArray& windowState = QByteArray());
	void undo();
	void redo();

	bool newProject();
	void openProject();
	void openProject(const QString&);
	void openRecentProject(const QUrl&);
	bool closeProject();
	bool saveProject();
	bool saveProjectAs();
	void autoSaveProject();
	void updateTitleBar();

	void print();
	void printPreview();

	void historyDialog();
	void importFileDialog(const QString& fileName = QString());
	void importSqlDialog();
	void importProjectDialog();
	void importDatasetDialog();
	void exportDialog();
	void editFitsFileDialog();
	void settingsDialog();
	void projectChanged();
	void colorSchemeChanged(QAction*);
	void openDatasetExample();

	// Cantor
#ifdef HAVE_CANTOR_LIBS
	void newCantorWorksheet(QAction* action);
	void cantorSettingsDialog();
#endif

	void newFolder();
	void newWorkbook();
	void newSpreadsheet();
	void newMatrix();
	void newWorksheet();
	void newNotes();
	void newDatapicker();
	// TODO: void newScript();
	void newLiveDataSource();

	void createContextMenu(QMenu*) const;
	void createFolderContextMenu(const Folder*, QMenu*) const;

	void handleAspectAdded(const AbstractAspect*);
	void handleAspectAboutToBeRemoved(const AbstractAspect*);
	void handleAspectRemoved(const AbstractAspect*, const AbstractAspect*, const AbstractAspect*);
	void handleCurrentAspectChanged(AbstractAspect*);
	//	void handleCurrentSubWindowChanged(QMdiSubWindow*);
	void handleShowSubWindowRequested();

	void handleSettingsChanges();

	void setDockVisibility(QAction*);
	void updateDockWindowVisibility() const;
	void toggleDockWidget(QAction*);
	void toggleStatusBar(bool);
	void toggleMenuBar(bool);
	void toggleMemoryInfo();
	void toggleFullScreen(bool);
	void projectExplorerDockVisibilityChanged(bool);
	void propertiesDockVisibilityChanged(bool);
	void cursorDockVisibilityChanged(bool);
	void propertiesExplorerRequested();

	void focusCursorDock();

	void cartesianPlotMouseModeChanged(CartesianPlot::MouseMode);
};

#endif
