/*
	File                 : MainWin.h
	Project              : LabPlot
	Description          : Main window of the application
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2011-2025 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2008-2018 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MAINWIN_H
#define MAINWIN_H

#include "SettingsDialog.h" // for Settings::Type enum
#include "backend/worksheet/plots/cartesian/CartesianPlot.h" // for CartesianPlot::MouseMode enum

#include <KXmlGuiWindow>
#include <QStringLiteral>
#include <QTimer>
#include <QPointer>

class AbstractAspect;
class ActionsManager;
class AspectTreeModel;
class Folder;
class ProjectExplorer;
class Project;
class Worksheet;
class Spreadsheet;
class GuiObserver;
class CursorDock;
class ContentDockWidget;
class MemoryWidget;
// class WelcomeScreenHelper;
class WorksheetPreviewWidget;

class QDockWidget;
class QDragEnterEvent;
class QDropEvent;
class QStackedWidget;
class QToolButton;
// class QQuickWidget;

class KColorSchemeManager;

namespace ads {
class CDockManager;
class CDockWidget;
class CDockAreaWidget;
}

#ifdef HAVE_KUSERFEEDBACK
#include <KUserFeedback/Provider>
#endif

class MainWin : public KXmlGuiWindow {
	Q_OBJECT

public:
	explicit MainWin(QWidget* parent = nullptr, const QString& fileName = QString());
	~MainWin() override;

	void showPresenter();
	AspectTreeModel* model() const;
	Project* project() const;
	void addAspectToProject(AbstractAspect*);

	enum class LoadOnStart { NewProject, LastProject, WelcomeScreen };
	enum class NewProject { WithSpreadsheet, WithWorksheet, WithSpreadsheetWorksheet, WithNotebook };
	enum class TitleBarMode { ShowFilePath, ShowFileName, ShowProjectName };

#ifdef HAVE_KUSERFEEDBACK
	KUserFeedback::Provider& userFeedbackProvider() {
		return m_userFeedbackProvider;
	}
#endif

private:
	ads::CDockManager* m_dockManagerContent{nullptr};
	ads::CDockManager* m_dockManagerMain{nullptr};
	KColorSchemeManager* m_schemeManager{nullptr};
	ContentDockWidget* m_currentDock{nullptr}; // Currently selected dock
	Project* m_project{nullptr};
	AspectTreeModel* m_aspectTreeModel{nullptr};
	ProjectExplorer* m_projectExplorer{nullptr};
	WorksheetPreviewWidget* m_worksheetPreviewWidget{nullptr};
	ads::CDockWidget* m_projectExplorerDock{nullptr};
	ads::CDockWidget* m_propertiesDock{nullptr};
	ads::CDockWidget* m_worksheetPreviewDock{nullptr};
	AbstractAspect* m_currentAspect{nullptr};
	ads::CDockWidget* m_currentAspectDock{nullptr};
	// introduce a QPointer for the current aspect dock area
	// we use QPointer because it internally sets itself to nullptr when the object it manages is deleted
	QPointer<ads::CDockAreaWidget> m_currentAspectDockArea{nullptr};
	Folder* m_currentFolder{nullptr};
	QString m_undoViewEmptyLabel;
	bool m_suppressCurrentSubWindowChangedEvent{false};
	bool m_closing{false};
	bool m_projectClosing{false};
	bool m_autoSaveActive{false};
	QTimer m_autoSaveTimer;
	// bool m_showWelcomeScreen{false};
	// bool m_saveWelcomeScreen{true};
	int undoStackIndexLastSave{0};
	MemoryWidget* m_memoryInfoWidget{nullptr};
	// QQuickWidget* m_welcomeWidget{nullptr};
	// WelcomeScreenHelper* m_welcomeScreenHelper{nullptr};
	QString m_lastOpenFileFilter;
	const Worksheet* m_lastWorksheet{nullptr};
	const Spreadsheet* m_lastSpreadsheet{nullptr};
	TitleBarMode m_titleBarMode{TitleBarMode::ShowFilePath};

#ifdef HAVE_KUSERFEEDBACK
	KUserFeedback::Provider m_userFeedbackProvider;
#endif

	// Docks
	ads::CDockWidget* cursorDock{nullptr};

	QStackedWidget* stackedWidget{nullptr};
	CursorDock* cursorWidget{nullptr};

	bool warnModified();
	void activateSubWindowForAspect(const AbstractAspect*);
	bool save(const QString&);
	// 	QQuickWidget* createWelcomeScreen();
	// 	void resetWelcomeScreen();
	void initDocks();
	void restoreDefaultDockState() const;
	void updateLocale();
	void migrateSettings();

	Spreadsheet* activeSpreadsheet() const;

	friend class ActionsManager;
	ActionsManager* m_actionsManager{nullptr};

	friend class GuiObserver;
	GuiObserver* m_guiObserver{nullptr};
	QLocale m_defaultSystemLocale; // default system locale, might be different from the default locale set at runtime

protected:
	void closeEvent(QCloseEvent*) override;
	void dragEnterEvent(QDragEnterEvent*) override;
	void dropEvent(QDropEvent*) override;

private Q_SLOTS:
	void initGUI(const QString&);
	void activateNextDock();
	void activatePreviousDock();
	void dockWidgetRemoved(ads::CDockWidget*);
	void dockFocusChanged(ads::CDockWidget* old, ads::CDockWidget* now);
	void undo();
	void redo();

	bool newProject(bool createInitialContent = true);
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

	void exampleProjectsDialog();
	void historyDialog();
	void importFileDialog(const QString& fileName = QString());
	void importKaggleDatasetDialog();
	void importSqlDialog();
	void importProjectDialog();
	void importDatasetDialog();
	void exportDialog();
	void editFitsFileDialog();
	void settingsDialog();
	void projectChanged();
	void colorSchemeChanged(QAction*);
	void openDatasetExample();

#ifdef HAVE_CANTOR_LIBS
	void newNotebook();
	void settingsNotebookDialog();
#endif

	void newFolder();
	void newWorkbook();
	void newSpreadsheet();
	void newMatrix();
	void newWorksheet();
	void newScript();
	void newNotes();
	void newDatapicker();
	void newLiveDataSource();

	void createContextMenu(QMenu*) const;
	void createFolderContextMenu(const Folder*, QMenu*) const;

	void handleAspectAdded(const AbstractAspect*);
	void handleAspectAboutToBeRemoved(const AbstractAspect*);
	void handleAspectRemoved(const AbstractAspect*, const AbstractAspect*, const AbstractAspect*);
	void handleCurrentAspectChanged(AbstractAspect*);
	void handleShowSubWindowRequested();

	void handleSettingsChanges(QList<Settings::Type>);

	void setDockVisibility(QAction*);
	void updateDockWindowVisibility() const;
	void projectExplorerDockVisibilityChanged(bool);
	void propertiesDockVisibilityChanged(bool);
	void worksheetPreviewDockVisibilityChanged(bool);
	void cursorDockVisibilityChanged(bool);
	void propertiesExplorerRequested();

	void focusCursorDock();

	void cartesianPlotMouseModeChanged(CartesianPlot::MouseMode);
};

#endif
