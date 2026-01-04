/*
	File                 : MainWin.cc
	Project              : LabPlot
	Description          : Main window of the application
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2008-2025 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-FileCopyrightText: 2009-2025 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "MainWin.h"
#include "backend/core/AbstractFilter.h"
#include "backend/core/AspectTreeModel.h"
#include "backend/core/Project.h"
#include "backend/core/Settings.h"
#include "backend/core/Workbook.h"
#include "backend/datasources/DatasetHandler.h"
#include "backend/datasources/LiveDataSource.h"
#include "backend/matrix/Matrix.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/worksheet/Worksheet.h"
#ifdef HAVE_LIBORIGIN
#include "frontend/datasources/ImportOriginLayersDialog.h"
#include "backend/datasources/projects/OriginProjectParser.h"
#endif
#include "backend/datapicker/Datapicker.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/note/Note.h"
#ifdef HAVE_SCRIPTING
#include "backend/script/Script.h"
#endif
#ifdef HAVE_MQTT
#include "backend/datasources/MQTTClient.h"
#endif

#include "frontend/ActionsManager.h"
#include "frontend/GuiObserver.h"
#include "frontend/HistoryDialog.h"
#include "frontend/ProjectExplorer.h"
#include "frontend/SettingsDialog.h"
#include "frontend/colormaps/ColorMapsDialog.h"
#include "frontend/core/ContentDockWidget.h"
#include "frontend/datasources/ImportDatasetDialog.h"
#include "frontend/datasources/ImportDatasetWidget.h"
#include "frontend/datasources/ImportFileDialog.h"
#include "frontend/datasources/ImportKaggleDatasetDialog.h"
#include "frontend/datasources/ImportProjectDialog.h"
#include "frontend/datasources/ImportSQLDatabaseDialog.h"
#include "frontend/dockwidgets/CursorDock.h"
#include "frontend/dockwidgets/ProjectDock.h"
#include "frontend/examples/ExamplesDialog.h"
#include "frontend/widgets/FITSHeaderEditDialog.h"
#include "frontend/widgets/LabelWidget.h"
#include "frontend/worksheet/WorksheetPreviewWidget.h"
#include "frontend/worksheet/WorksheetView.h"

#ifdef HAVE_KUSERFEEDBACK
#include <KUserFeedback/ApplicationVersionSource>
#include <KUserFeedback/PlatformInfoSource>
#include <KUserFeedback/QtVersionSource>
#include <KUserFeedback/ScreenInfoSource>
#include <KUserFeedback/StartCountSource>
#include <KUserFeedback/UsageTimeSource>
#endif

#include <DockAreaWidget.h>
#include <DockManager.h>
#include <AutoHideSideBar.h>

#include <QCloseEvent>
#include <QElapsedTimer>
#include <QFileDialog>
#include <QMenu>
#include <QMimeData>
#include <QStackedWidget>
#include <QStatusBar>
#include <QTemporaryFile>
#include <QUndoStack>
// #include <QtWidgets>
// #include <QtQuickWidgets/QQuickWidget>
// #include <QQuickItem>
// #include <QQuickView>
// #include <QQmlApplicationEngine>
// #include <QQmlContext>

#include <KColorSchemeManager>
#include <KCompressionDevice>
#include <KMessageBox>
#include <KRecentFilesAction>
#include <KToolBar>
#ifdef HAVE_SCRIPTING
#include <KTextEditor/Editor>
#include <KTextEditor/Document>
#include <KTextEditor/View>
#endif

#ifdef HAVE_CANTOR_LIBS
#include "backend/notebook/Notebook.h"
#include "frontend/notebook/NotebookView.h"
#include <cantor/backend.h>
#endif


/*!
\class MainWin
\brief Main application window.

\ingroup frontend
*/
MainWin::MainWin(QWidget* parent, const QString& fileName)
	: KXmlGuiWindow(parent)
	, m_schemeManager(new KColorSchemeManager(this)) {
	migrateSettings(); // call this at the very beginning to migrate the application settings first
	updateLocale();

	if (!fileName.isEmpty())
		DEBUG(Q_FUNC_INFO << ", file name = " << fileName.toStdString())
	initGUI(fileName);
	setAcceptDrops(true);

#ifdef HAVE_KUSERFEEDBACK
	m_userFeedbackProvider.setProductIdentifier(QStringLiteral("org.kde.labplot"));
	m_userFeedbackProvider.setFeedbackServer(QUrl(QStringLiteral("https://telemetry.kde.org/")));
	m_userFeedbackProvider.setSubmissionInterval(7);
	m_userFeedbackProvider.setApplicationStartsUntilEncouragement(5);
	m_userFeedbackProvider.setEncouragementDelay(30);

	// software version info
	m_userFeedbackProvider.addDataSource(new KUserFeedback::ApplicationVersionSource);
	m_userFeedbackProvider.addDataSource(new KUserFeedback::QtVersionSource);

	// info about the machine
	m_userFeedbackProvider.addDataSource(new KUserFeedback::PlatformInfoSource);
	m_userFeedbackProvider.addDataSource(new KUserFeedback::ScreenInfoSource);

	// usage info
	m_userFeedbackProvider.addDataSource(new KUserFeedback::StartCountSource);
	m_userFeedbackProvider.addDataSource(new KUserFeedback::UsageTimeSource);
#endif
}

MainWin::~MainWin() {
	// save the current settings in MainWin
	auto group = Settings::group(QStringLiteral("MainWin"));
	group.writeEntry(QLatin1String("geometry"), saveGeometry()); // current geometry of the main window
	group.writeEntry(QLatin1String("WindowState"), saveState()); // current state of QMainWindow's toolbars
	group.writeEntry(QLatin1String("lastOpenFileFilter"), m_lastOpenFileFilter);
	group.writeEntry(QLatin1String("ShowMemoryInfo"), (m_memoryInfoWidget != nullptr));
#ifdef HAVE_CANTOR_LIBS
	if (m_actionsManager->m_lastUsedNotebookAction)
		group.writeEntry(QLatin1String("lastUsedNotebook"), m_actionsManager->m_lastUsedNotebookAction->data().toString());
#endif

	Settings::sync();

	delete m_guiObserver;
	delete m_actionsManager;
	delete m_aspectTreeModel;
	disconnect(m_project, nullptr, this, nullptr);
	delete m_project;

	// if welcome screen is shown, save its settings prior to deleting it
	// 	if (dynamic_cast<QQuickWidget*>(centralWidget()))
	// 		QMetaObject::invokeMethod(m_welcomeWidget->rootObject(), "saveWidgetDimensions");
	//	delete m_welcomeScreenHelper;
}

void MainWin::showPresenter() {
	const auto* w = dynamic_cast<Worksheet*>(m_currentAspect);
	if (w) {
		auto* view = static_cast<WorksheetView*>(w->view());
		view->presenterMode();
	} else {
		// currently active object is not a worksheet but we're asked to start in the presenter mode
		// determine the first available worksheet and show it in the presenter mode
		auto worksheets = m_project->children<Worksheet>();
		if (worksheets.size() > 0) {
			auto* view = static_cast<WorksheetView*>(worksheets.constFirst()->view());
			view->presenterMode();
		} else
			QMessageBox::information(this, i18n("Presenter Mode"), i18n("No worksheets are available in the project. The presenter mode will not be started."));
	}
}

AspectTreeModel* MainWin::model() const {
	return m_aspectTreeModel;
}

Project* MainWin::project() const {
	return m_project;
}

void MainWin::initGUI(const QString& fileName) {
	if (statusBar()->isEnabled())
		statusBar()->showMessage(i18nc("%1 is the LabPlot version", "Welcome to LabPlot %1", QLatin1String(LVERSION)));

	m_actionsManager = new ActionsManager(this);
	setupGUI();
	m_actionsManager->init();

#ifdef Q_OS_MAC
#ifdef HAVE_TOUCHBAR
	// setup touchbar before GUI (otherwise actions in the toolbar are not selectable)
	m_touchBar = new KDMacTouchBar(this);
	// m_touchBar->setTouchButtonStyle(KDMacTouchBar::IconOnly);
#endif
	setUnifiedTitleAndToolBarOnMac(true);
#endif

	setWindowIcon(QIcon::fromTheme(QLatin1String("LabPlot"), QGuiApplication::windowIcon()));
	setAttribute(Qt::WA_DeleteOnClose);

	// make the status bar of a fixed size in order to avoid height changes when placing a ProgressBar there.
	QFont font;
	font.setFamily(font.defaultFamily());
	QFontMetrics fm(font);
	statusBar()->setFixedHeight(fm.height() + 5);

	// General Settings
	auto group = Settings::group(QStringLiteral("Settings_General"));

	// title bar
	m_titleBarMode = static_cast<MainWin::TitleBarMode>(group.readEntry("TitleBar", 0));

	// auto-save
	m_autoSaveActive = group.readEntry<bool>("AutoSave", false);
	int interval = group.readEntry("AutoSaveInterval", 1);
	interval = interval * 60 * 1000;
	m_autoSaveTimer.setInterval(interval);
	connect(&m_autoSaveTimer, &QTimer::timeout, this, &MainWin::autoSaveProject);

	if (!fileName.isEmpty()) {
		initDocks();
		if (Project::isSupportedProject(fileName)) {
			QTimer::singleShot(0, this, [=]() {
				openProject(fileName);
			});
		} else {
			newProject();
			QTimer::singleShot(0, this, [=]() {
				importFileDialog(fileName);
			});
		}
	} else {
		// There is no file to open. Depending on the settings do nothing,
		// create a new project or open the last used project.
		auto load = (LoadOnStart)group.readEntry("LoadOnStart", static_cast<int>(LoadOnStart::NewProject));

		// in case we're starting with the settings created with an older version where the LoadOnStart enum had more values
		// or in case the config file was manipulated, we need to ensure we start with proper values and properly initialize the docks
		// by mapping the old/manipulated values to the new/correct values:
		// * old value 0 - "do nothing" -> map to new values "new project" and "with spreadsheet" which are default
		// * old value 1 - "new project" -> map to new values "new project" and "with spreadsheet" which are default
		// * old value 2 - "new project with worksheet" -> map to new values "new project" and "with worksheet"
		// * old value 3 - "new project with spreadsheet" -> map to new values "new project" and "with spreadsheet"
		// * old value 4 - "last project" -> map to the new "last project"
		// * any higher values or <0, manipulated file -> map to the new default values
		if (load > LoadOnStart::LastProject) {
			int oldLoad = static_cast<int>(load);
			if (oldLoad == 2) { // old "new project with worksheet"
				load = LoadOnStart::NewProject;
				group.writeEntry(QStringLiteral("LoadOnStart"), static_cast<int>(load));
				group.writeEntry(QStringLiteral("NewProject"), static_cast<int>(NewProject::WithWorksheet));
			} else if (oldLoad == 3) { // old "new project with spreadsheet"
				load = LoadOnStart::NewProject;
				group.writeEntry(QStringLiteral("LoadOnStart"), static_cast<int>(load));
				group.writeEntry(QStringLiteral("NewProject"), static_cast<int>(NewProject::WithSpreadsheet));
			} else if (oldLoad == 4) { // old "last project"
				load = LoadOnStart::LastProject;
				group.writeEntry(QStringLiteral("LoadOnStart"), static_cast<int>(load));
			} else if (oldLoad > 4 || oldLoad < 0) {
				load = LoadOnStart::NewProject;
				group.writeEntry(QStringLiteral("LoadOnStart"), static_cast<int>(load));
			}
		}

		switch (load) {
		case LoadOnStart::NewProject:
			initDocks();
			newProject();
			break;
		case LoadOnStart::LastProject: {
			initDocks();
			const QString& path = Settings::group(QStringLiteral("MainWin")).readEntry("LastOpenProject", "");
			if (!path.isEmpty()) {
				if (!openProject(path))
					newProject();
			}
			else
				newProject();
			break;
		}
		case LoadOnStart::WelcomeScreen:
			// TODO:
			// m_showWelcomeScreen = true;
			// m_welcomeWidget = createWelcomeScreen();
			// setCentralWidget(m_welcomeWidget);
			break;
		}
	}

	// restore the geometry
	const KConfigGroup& groupMainWin = Settings::group(QStringLiteral("MainWin"));
	if (groupMainWin.hasKey(QStringLiteral("geometry")))
		restoreGeometry(groupMainWin.readEntry("geometry", QByteArray()));

	m_lastOpenFileFilter = groupMainWin.readEntry(QLatin1String("lastOpenFileFilter"), QString());
}

/**
 * @brief Creates a new welcome screen to be set as central widget.
 */
/*
QQuickWidget* MainWin::createWelcomeScreen() {
	QSize maxSize = qApp->primaryScreen()->availableSize();
	resize(maxSize);
	setMinimumSize(700, 400);
	showMaximized();

	KToolBar* toolbar = toolBar();
	if (toolbar)
		toolbar->setVisible(false);

	QList<QVariant> recentList;
	for (QUrl& url : m_recentProjectsAction->urls())
		recentList.append(QVariant(url));

	//Set context property
	QQuickWidget* quickWidget = new QQuickWidget(this);
	QQmlContext* ctxt = quickWidget->rootContext();
	QVariant variant(recentList);
	ctxt->setContextProperty("recentProjects", variant);

	//Create helper object
	if (m_welcomeScreenHelper)
		delete m_welcomeScreenHelper;
	m_welcomeScreenHelper = new WelcomeScreenHelper();
	connect(m_welcomeScreenHelper, &WelcomeScreenHelper::openExampleProject,
			this, QOverload<const QString&>::of(&MainWin::openProject));

	ctxt->setContextProperty("datasetModel", m_welcomeScreenHelper->getDatasetModel());
	ctxt->setContextProperty("helper", m_welcomeScreenHelper);

	quickWidget->setSource(QUrl(QLatin1String("qrc:///main.qml")));
	quickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
	QObject *item = quickWidget->rootObject();

	//connect qml's signals
	QObject::connect(item, SIGNAL(recentProjectClicked(QUrl)), this, SLOT(openRecentProject(QUrl)));
	QObject::connect(item, SIGNAL(datasetClicked(QString,QString,QString)), m_welcomeScreenHelper, SLOT(datasetClicked(QString,QString,QString)));
	QObject::connect(item, SIGNAL(openDataset()), this, SLOT(openDatasetExample()));
	QObject::connect(item, SIGNAL(openExampleProject(QString)), m_welcomeScreenHelper, SLOT(exampleProjectClicked(QString)));
	Q_EMIT m_welcomeScreenHelper->showFirstDataset();

	return quickWidget;
}
*/
/**
 * @brief Initiates resetting the layout of the welcome screen
 */
/*
void MainWin::resetWelcomeScreen() {
	if (dynamic_cast<QQuickWidget*>(centralWidget()))
		QMetaObject::invokeMethod(m_welcomeWidget->rootObject(), "restoreOriginalLayout");
}
*/

void MainWin::activateNextDock() {
	const auto* focusedDock = m_dockManagerContent->focusedDockWidget();

	auto itrForward = m_dockManagerContent->dockWidgetsMap().constBegin();

	bool focusedFound = false;
	while (itrForward != m_dockManagerContent->dockWidgetsMap().constEnd()) {
		auto* dock = itrForward.value();
		if (focusedFound) {
			dock->toggleView(true);
			m_dockManagerContent->setDockWidgetFocused(dock);
			return;
		}

		if (dock == focusedDock)
			focusedFound = true;
		itrForward++;
	}

	if (!focusedFound) {
		if (!m_dockManagerContent->dockWidgetsMap().count())
			return;
		auto* dock = m_dockManagerContent->dockWidgetsMap().first();
		dock->toggleView(true);
		m_dockManagerContent->setDockWidgetFocused(dock);
		return;
	}

	// select the first dock otherwise
	auto* dock = m_dockManagerContent->dockWidgetsMap().first();
	if (dock) {
		dock->toggleView(true);
		m_dockManagerContent->setDockWidgetFocused(dock);
	}
}

void MainWin::activatePreviousDock() {
	const auto* focusedDock = m_dockManagerContent->focusedDockWidget();

	auto itrForward = QMapIterator<QString, ads::CDockWidget*>(m_dockManagerContent->dockWidgetsMap());
	itrForward.toBack();

	bool focusedFound = false;
	while (itrForward.hasPrevious()) {
		itrForward.previous();
		auto* dock = itrForward.value();
		if (focusedFound) {
			dock->toggleView(true);
			m_dockManagerContent->setDockWidgetFocused(dock);
			return;
		}

		if (dock == focusedDock) {
			focusedFound = true;
		}
	}

	if (!focusedFound) {
		if (!m_dockManagerContent->dockWidgetsMap().count())
			return;
		auto* dock = m_dockManagerContent->dockWidgetsMap().first();
		dock->toggleView(true);
		m_dockManagerContent->setDockWidgetFocused(dock);
		return;
	}

	// select the last dock otherwise
	auto* dock = m_dockManagerContent->dockWidgetsMap().last();
	if (dock) {
		dock->toggleView(true);
		m_dockManagerContent->setDockWidgetFocused(dock);
	}
}

void MainWin::dockWidgetRemoved(ads::CDockWidget* w) {
	if (w == m_currentAspectDock) {
		m_currentAspectDock = nullptr;
		m_currentAspectDockArea = nullptr;
	}
}

void MainWin::dockFocusChanged(ads::CDockWidget* old, ads::CDockWidget* now) {
	Q_UNUSED(old);
	if (!now) {
		m_actionsManager->updateGUI();
		return;
	}

	auto* view = dynamic_cast<ContentDockWidget*>(now);
	if (!view)
		return; // project explorer or propertiesexplorer can be ignored
	if (view == m_currentDock) {
		// do nothing, if the current sub-window gets selected again.
		// This event happens, when labplot loses the focus (modal window is opened or the user switches to another application)
		// and gets it back (modal window is closed or the user switches back to labplot).
		return;
	} else
		m_currentDock = view;

	if (!m_suppressCurrentSubWindowChangedEvent)
		m_projectExplorer->setCurrentAspect(view->part());
}

/*!
	Asks to save the project if it was modified.
	\return \c true if the project still needs to be saved ("cancel" clicked), \c false otherwise.
 */
bool MainWin::warnModified() {
	if (m_project->hasChanged()) {
		int option = KMessageBox::warningTwoActionsCancel(this, i18n("The current project \"%1\" has been modified. Do you want to save it?", m_project->name()),
				i18n("Save Project"), KStandardGuiItem::save(), KStandardGuiItem::dontSave());
		switch (option) {
		case KMessageBox::PrimaryAction:
			return !saveProject();
		case KMessageBox::SecondaryAction:
			break;
		case KMessageBox::Cancel:
			return true;
		}
	}

	return false;
}

/*!
	creates a new empty project. Returns \c true, if a new project was created.
	the parameter \c createInitialContent whether a default content (new worksheet, etc. )
	has to be created automatically (it's \c false if a project is opened, \c true if a new project is created).
*/
bool MainWin::newProject(bool createInitialContent) {
	// close the current project, if available
	if (!closeProject())
		return false;

	QApplication::processEvents(QEventLoop::AllEvents, 100);

	KConfigGroup group = Settings::group(QStringLiteral("Settings_General"));

	m_project = new Project();
	Project::currentProject = m_project;
	undoStackIndexLastSave = 0;
	m_currentAspect = m_project;
	m_currentFolder = m_project;

	auto vis = Project::DockVisibility(group.readEntry("DockVisibility", 0));
	m_project->setDockVisibility(vis);
	if (vis == Project::DockVisibility::folderOnly)
		m_actionsManager->m_visibilityFolderAction->setChecked(true);
	else if (vis == Project::DockVisibility::folderAndSubfolders)
		m_actionsManager->m_visibilitySubfolderAction->setChecked(true);
	else
		m_actionsManager->m_visibilityAllAction->setChecked(true);

	m_aspectTreeModel = new AspectTreeModel(m_project, this);
	connect(m_aspectTreeModel, &AspectTreeModel::statusInfo, [=](const QString& text) {
		statusBar()->showMessage(text);
	});

	m_actionsManager->m_newProjectAction->setEnabled(false);
#ifdef HAVE_PURPOSE
	m_actionsManager->m_shareAction->setEnabled(false); // sharing is only possible after the project was saved to a file
#endif

	m_projectExplorer->setModel(m_aspectTreeModel);
	m_projectExplorer->setProject(m_project);
	m_projectExplorer->setCurrentAspect(m_project);
	m_worksheetPreviewWidget->setProject(m_project);
	m_guiObserver = new GuiObserver(this); // initialize after all docks were createad

	connect(m_project, &Project::childAspectAdded, this, &MainWin::handleAspectAdded);
	connect(m_project, &Project::childAspectRemoved, this, &MainWin::handleAspectRemoved);
	connect(m_project, &Project::childAspectAboutToBeRemoved, this, &MainWin::handleAspectAboutToBeRemoved);
	connect(m_project, SIGNAL(statusInfo(QString)), statusBar(), SLOT(showMessage(QString)));
	connect(m_project, &Project::changed, this, &MainWin::projectChanged);
	connect(m_project, &Project::requestProjectContextMenu, this, &MainWin::createContextMenu);
	connect(m_project, &Project::requestFolderContextMenu, this, &MainWin::createFolderContextMenu);
	connect(m_project, &Project::mdiWindowVisibilityChanged, this, &MainWin::updateDockWindowVisibility);
	connect(m_project, &Project::closeRequested, this, &MainWin::closeProject);

	// depending on the settings, create the default project content (add a worksheet, etc.)
	if (createInitialContent) {
		const auto newProject = (NewProject)group.readEntry(QStringLiteral("NewProject"), static_cast<int>(NewProject::WithSpreadsheet));
		switch (newProject) {
		case NewProject::WithWorksheet:
			newWorksheet();
			break;
		case NewProject::WithSpreadsheet:
			newSpreadsheet();
			break;
		case NewProject::WithSpreadsheetWorksheet:
			newSpreadsheet();
			newWorksheet();
			break;
		case NewProject::WithNotebook: {
#ifdef HAVE_CANTOR_LIBS
			const auto& backend = group.readEntry(QLatin1String("LoadOnStartNotebook"), QString());
			if (Cantor::Backend::listAvailableBackends().indexOf(backend) != -1)
				addAspectToProject(new Notebook(backend));
#endif
			break;
		}
		}

		m_project->setChanged(false); // the project was initialized on startup, nothing has changed from user's perspective

		m_actionsManager->updateGUIOnProjectChanges();
		m_undoViewEmptyLabel = i18n("%1: created", m_project->name());
	}

	return true;
}

void MainWin::initDocks() {
	auto* toolbar = toolBar();
	if (toolbar)
		toolbar->setVisible(true);

	// As per documentation the configuration Flags must be set prior a DockManager will be created!
	// https://github.com/githubuser0xFFFF/Qt-Advanced-Docking-System/blob/master/doc/user-guide.md#configuration-flags
	ads::CDockManager::setConfigFlag(ads::CDockManager::XmlCompressionEnabled, false);
	ads::CDockManager::setConfigFlag(ads::CDockManager::FocusHighlighting, true);
	ads::CDockManager::setConfigFlag(ads::CDockManager::MiddleMouseButtonClosesTab, true);
	ads::CDockManager::setConfigFlag(ads::CDockManager::AllTabsHaveCloseButton, true);
	ads::CDockManager::setConfigFlag(ads::CDockManager::RetainTabSizeWhenCloseButtonHidden, true);
	// must be after the config flags!
	ads::CDockManager::setAutoHideConfigFlags(ads::CDockManager::DefaultAutoHideConfig);
	ads::CDockManager::setAutoHideConfigFlag(ads::CDockManager::AutoHideShowOnMouseOver, true);

	// main dock manager for the default docks of the application (project explorer, etc)
	m_dockManagerMain = new ads::CDockManager(this);

	// project explorer
	m_projectExplorerDock = new ads::CDockWidget(i18nc("@title:window", "Project Explorer"));
	m_projectExplorerDock->setObjectName(QLatin1String("project-explorer"));
	m_projectExplorerDock->setWindowTitle(m_projectExplorerDock->windowTitle().replace(QLatin1String("&"), QString()));
	m_projectExplorerDock->toggleViewAction()->setText(QLatin1String(""));

	m_projectExplorer = new ProjectExplorer(m_projectExplorerDock);
	m_projectExplorerDock->setWidget(m_projectExplorer);

	connect(m_projectExplorer, &ProjectExplorer::currentAspectChanged, this, &MainWin::handleCurrentAspectChanged);
	connect(m_projectExplorer, &ProjectExplorer::activateView, this, &MainWin::activateSubWindowForAspect);
	connect(m_projectExplorerDock, &ads::CDockWidget::viewToggled, this, &MainWin::projectExplorerDockVisibilityChanged);

	// properties explorer
	m_propertiesDock = new ads::CDockWidget(i18nc("@title:window", "Properties"));
	m_propertiesDock->setObjectName(QLatin1String("properties-explorer"));
	m_propertiesDock->setWindowTitle(m_propertiesDock->windowTitle().replace(QLatin1String("&"), QString()));

	auto* scrollArea = new QScrollArea(m_propertiesDock);
	scrollArea->setWidgetResizable(true);
	stackedWidget = new QStackedWidget(scrollArea);
	scrollArea->setWidget(stackedWidget); // stacked widget inside scroll area
	m_propertiesDock->setWidget(scrollArea); // scroll area inside dock
	connect(m_propertiesDock, &ads::CDockWidget::viewToggled, this, &MainWin::propertiesDockVisibilityChanged);

	// worksheet preview
	m_worksheetPreviewDock = new ads::CDockWidget(i18nc("@title:window", "Worksheet Preview"));
	m_worksheetPreviewDock->setObjectName(QLatin1String("worksheet-preview"));
	m_worksheetPreviewDock->setWindowTitle(m_worksheetPreviewDock->windowTitle().replace(QLatin1String("&"), QString()));
	m_worksheetPreviewDock->toggleViewAction()->setText(QLatin1String(""));

	m_worksheetPreviewWidget = new WorksheetPreviewWidget(m_worksheetPreviewDock);
	m_worksheetPreviewDock->setWidget(m_worksheetPreviewWidget);
	connect(m_worksheetPreviewDock, &ads::CDockWidget::viewToggled, this, &MainWin::worksheetPreviewDockVisibilityChanged);

	auto contentDock = new ads::CDockWidget(i18nc("@title:window", "Content"));
	contentDock->setObjectName(QLatin1String("content-dock"));
	m_dockManagerContent = new ads::CDockManager(contentDock);
	contentDock->setWidget(m_dockManagerContent);

	// resize to the minimal sizes
	// TODO: doesn't work, the default docks are smaller than they should be
	m_projectExplorerDock->resize(m_projectExplorerDock->minimumSize());
	m_propertiesDock->resize(m_propertiesDock->minimumSize());
	m_worksheetPreviewDock->resize(m_worksheetPreviewDock->minimumSize());

	auto* area = m_dockManagerMain->setCentralWidget(contentDock);
	Q_ASSERT(area); // Check if success

	// add the default docks to the main dock manager and don't allow to stretch them horizontally,
	// the available space should go to the content dock widgets
	auto* areaWidget = m_dockManagerMain->addDockWidget(ads::LeftDockWidgetArea, m_projectExplorerDock);
	auto policy = areaWidget->sizePolicy();
	policy.setHorizontalStretch(0);
	areaWidget->setSizePolicy(policy);

	areaWidget = m_dockManagerMain->addDockWidget(ads::RightDockWidgetArea, m_worksheetPreviewDock);
	policy = areaWidget->sizePolicy();
	policy.setHorizontalStretch(0);
	areaWidget->setSizePolicy(policy);

	areaWidget = m_dockManagerMain->addDockWidget(ads::RightDockWidgetArea, m_propertiesDock, m_worksheetPreviewDock->dockAreaWidget());
	policy = areaWidget->sizePolicy();
	policy.setHorizontalStretch(0);
	areaWidget->setSizePolicy(policy);

	// signal-slot connections for the window handling for the content docks
	connect(m_dockManagerContent, &ads::CDockManager::focusedDockWidgetChanged, this, &MainWin::dockFocusChanged); // TODO: seems not to work
	connect(m_dockManagerContent, &ads::CDockManager::dockWidgetRemoved, this, &MainWin::dockWidgetRemoved);

	connect(m_actionsManager->m_closeWindowAction, &QAction::triggered, [this] {
		m_dockManagerContent->removeDockWidget(m_currentDock);
	});
	connect(m_actionsManager->m_closeAllWindowsAction, &QAction::triggered, [this]() {
		for (auto dock : m_dockManagerContent->dockWidgetsMap())
			m_dockManagerContent->removeDockWidget(dock);
	});

	connect(m_actionsManager->m_nextWindowAction, &QAction::triggered, this, &MainWin::activateNextDock);
	connect(m_actionsManager->m_prevWindowAction, &QAction::triggered, this, &MainWin::activatePreviousDock);

	// restore the last used dock state
	restoreDefaultDockState();
}

/*!
 * restores the default state of the default application dock widgets (Project Explorer, etc.)
 */
void MainWin::restoreDefaultDockState() const {
	auto group = Settings::group(QStringLiteral("MainWin"));
	if (group.hasKey(QStringLiteral("DockWidgetState"))) {
		auto state = group.readEntry(QStringLiteral("DockWidgetState"), QByteArray());
		m_dockManagerMain->restoreState(state); // restore the state of of the default docks
	} else {
		// the state of the dock widgets not available yet, starting for the first time:
		// show the project and properties explorers, hide the worksheet preview.
		// for this, check the actions for the project and properties explorer, uncheck for worksheet preview
		m_actionsManager->m_projectExplorerDockAction->setChecked(true);
		m_actionsManager->m_propertiesDockAction->setChecked(true);
		m_actionsManager->m_worksheetPreviewAction->setChecked(false);
		m_worksheetPreviewDock->toggleView(false);
	}
}

void MainWin::openProject() {
	bool supportOthers = false;
	QString allExtensions = Project::supportedExtensions();
	QString extensions = i18n("LabPlot Projects (%1)", allExtensions);
	if (m_lastOpenFileFilter.isEmpty())
		m_lastOpenFileFilter = extensions;

#ifdef HAVE_LIBORIGIN
	extensions += QLatin1String(";;") + i18n("Origin Projects (%1)", OriginProjectParser::supportedExtensions());
	allExtensions += QLatin1String(" ") + OriginProjectParser::supportedExtensions();
	supportOthers = true;
#endif

#ifdef HAVE_CANTOR_LIBS
	extensions += QLatin1String(";;") + i18n("Cantor Projects (*.cws)");
	extensions += QLatin1String(";;") + i18n("Jupyter Notebooks (*.ipynb)");
	allExtensions += QLatin1String(" *.cws *.ipynb");
	supportOthers = true;
#endif

	// add an entry for "All supported files" if we support more than labplot
	if (supportOthers)
		extensions = i18n("All supported files (%1)", allExtensions) + QLatin1String(";;") + extensions;

	KConfigGroup group = Settings::group(QStringLiteral("MainWin"));
	const QString& dir = group.readEntry("LastOpenDir", "");
	const QString& path = QFileDialog::getOpenFileName(this, i18nc("@title:window", "Open Project"), dir, extensions, &m_lastOpenFileFilter);
	if (path.isEmpty()) // "Cancel" was clicked
		return;

	if (!openProject(path)) {
		newProject();
		return;
	}

	// save new "last open directory"
	int pos = path.lastIndexOf(QLatin1String("/"));
	if (pos != -1) {
		const QString& newDir = path.left(pos);
		if (newDir != dir)
			group.writeEntry("LastOpenDir", newDir);
	}
}

bool MainWin::openProject(const QString& fileName) {
	if (m_project && fileName == m_project->fileName()) {
		KMessageBox::information(this, i18n("The project file %1 is already opened.", fileName), i18n("Open Project"));
		return false;
	}

	// check whether the file can be opened for reading at all before closing the current project
	// and creating a new project and trying to load
	QFile file(fileName);
	if (!file.exists()) {
		KMessageBox::error(this, i18n("The project file %1 doesn't exist.", fileName), i18n("Open Project"));
		return false;
	}

	if (!file.open(QIODevice::ReadOnly)) {
		KMessageBox::error(this, i18n("Couldn't read the project file %1.", fileName), i18n("Open Project"));
		return false;
	} else
		file.close();

	if (!newProject(false))
		return false;

	statusBar()->showMessage(i18n("Loading %1...", fileName));
	QApplication::processEvents(QEventLoop::AllEvents, 0);
	m_project->setFileName(fileName);
	QElapsedTimer timer;
	timer.start();
	bool rc = false;
	if (Project::isLabPlotProject(fileName)) {
		WAIT_CURSOR_AUTO_RESET;
		rc = m_project->load(fileName);
	}
#ifdef HAVE_LIBORIGIN
	else if (OriginProjectParser::isOriginProject(fileName)) {
		OriginProjectParser parser;
		parser.setProjectFileName(fileName);

		// check if graphs have multiple layer
		bool hasUnusedObjects, hasMultiLayerGraphs;
		parser.checkContent(hasUnusedObjects, hasMultiLayerGraphs);
		DEBUG(Q_FUNC_INFO << ", project has multilayer graphs: " << hasMultiLayerGraphs)
		// ask how to import OPJ graph layers if graphs have multiple layer
		if (hasMultiLayerGraphs) {
			auto* dlg = new ImportOriginLayersDialog(this);
			bool graphLayersAsPlotArea = true;
			if (dlg->exec() == QDialog::Accepted)
				graphLayersAsPlotArea = dlg->graphLayersAsPlotArea();
			delete dlg;

			parser.setGraphLayerAsPlotArea(graphLayersAsPlotArea);
		}

		WAIT_CURSOR_AUTO_RESET;
		parser.importTo(m_project, QStringList()); // TODO: add return code
		rc = true;
	}
#endif

#ifdef HAVE_CANTOR_LIBS
	else if (fileName.endsWith(QLatin1String(".cws"), Qt::CaseInsensitive) || fileName.endsWith(QLatin1String(".ipynb"), Qt::CaseInsensitive)) {
		WAIT_CURSOR_AUTO_RESET;
		rc = m_project->loadNotebook(fileName);
	}
#endif

	m_project->setChanged(false);

	if (!rc) {
		closeProject();
		return false;
	}

	m_project->undoStack()->clear();
	m_undoViewEmptyLabel = i18n("%1: opened", m_project->name());
	m_actionsManager->m_recentProjectsAction->addUrl(QUrl(fileName));

	m_actionsManager->updateGUIOnProjectChanges();
	m_actionsManager->updateGUI(); // there are most probably worksheets or spreadsheets in the open project -> update the GUI

	const auto& dockWidgetsState = m_project->dockWidgetState().toUtf8();
	if (!dockWidgetsState.isEmpty()) {
		for (auto dock : m_dockManagerContent->dockWidgetsMap()) {
			auto* d = dynamic_cast<ContentDockWidget*>(dock);
			if (d)
				d->part()->suppressDeletion(true);
		}

		for (auto dock : m_dockManagerContent->dockWidgetsMap()) {
			auto* d = dynamic_cast<ContentDockWidget*>(dock);
			if (d)
				d->part()->suppressDeletion(false);
		}

		// restore the state of the content docks
		m_dockManagerContent->restoreState(dockWidgetsState);

		// restore the state of the default docks if it was saved in the project file
		if (m_project->saveDefaultDockWidgetState())
			m_dockManagerMain->restoreState(m_project->defaultDockWidgetState().toUtf8());
	} else
		updateDockWindowVisibility();

	m_actionsManager->m_saveAction->setEnabled(false);
	m_actionsManager->m_newProjectAction->setEnabled(true);
#ifdef HAVE_PURPOSE
	m_actionsManager->m_shareAction->setEnabled(true);
	m_actionsManager->fillShareMenu();
#endif
	statusBar()->showMessage(i18n("Project successfully opened (in %1 seconds).", (float)timer.elapsed() / 1000));

	KConfigGroup group = Settings::group(QStringLiteral("MainWin"));
	group.writeEntry("LastOpenProject", fileName);

	if (m_autoSaveActive)
		m_autoSaveTimer.start();

	return true;
}

void MainWin::openRecentProject(const QUrl& url) {
	QString path = url.isLocalFile() ?  url.toLocalFile() : url.path(); // fix for Windows
	if (!openProject(path)) {
		newProject();
		return;
	}
}

/*!
	Closes the current project, if available. Return \c true, if the project was closed.
*/
bool MainWin::closeProject() {
	if (m_project == nullptr)
		return true; // nothing to close

	if (warnModified())
		return false;

	// clear the worksheet preview before deleting the project and before deleting the dock widgets
	// so we don't need to react on current aspect changes
	if (m_worksheetPreviewWidget)
		m_worksheetPreviewWidget->setProject(nullptr);

	if (!m_closing) {
		// 		if (dynamic_cast<QQuickWidget*>(centralWidget()) && m_showWelcomeScreen) {
		// 			m_welcomeWidget = createWelcomeScreen();
		// 			setCentralWidget(m_welcomeWidget);
		// 		}
	}

	for (auto dock : m_dockManagerContent->dockWidgetsMap())
		m_dockManagerContent->removeDockWidget(dock);

	m_projectClosing = true;
	statusBar()->clearMessage();
	delete m_guiObserver;
	m_guiObserver = nullptr;
	delete m_aspectTreeModel;
	m_aspectTreeModel = nullptr;
	delete m_project;
	m_project = nullptr;
	Project::currentProject = m_project;
	m_projectClosing = false;

	// update the UI if we're just closing a project
	// and not closing(quitting) the application
	if (!m_closing) {
		m_currentAspect = nullptr;
		m_currentFolder = nullptr;
		m_actionsManager->updateGUIOnProjectChanges();
		m_actionsManager->m_newProjectAction->setEnabled(true);

		if (m_autoSaveActive)
			m_autoSaveTimer.stop();
	}

	if (cursorDock) {
		delete cursorDock;
		cursorDock = nullptr;
		cursorWidget = nullptr; // is deleted, because it's the child of cursorDock
	}

	return true;
}

bool MainWin::saveProject() {
	QString fileName = m_project->fileName();
	if (fileName.isEmpty())
		return saveProjectAs();
	else {
		// don't overwrite OPJ files, replace ending
		if (fileName.endsWith(QLatin1String(".opj"), Qt::CaseInsensitive)) {
			//fileName.replace(QLatin1String(".opj"), QLatin1String(".lml"), Qt::CaseInsensitive);
			// better append ending (don't overwrite existing project.lml file)
			fileName.append(QLatin1String(".lml"));
			DEBUG(Q_FUNC_INFO << ", renamed file name to " << fileName.toStdString())
		}
		return save(fileName);
	}
}

bool MainWin::saveProjectAs() {
	KConfigGroup conf = Settings::group(QStringLiteral("MainWin"));
	const QString& dir = conf.readEntry("LastOpenDir", "");
	QString path = QFileDialog::getSaveFileName(this,
												i18nc("@title:window", "Save Project As"),
												dir + m_project->fileName(),
												i18n("LabPlot Projects (*.lml *.lml.gz *.lml.bz2 *.lml.xz *.LML *.LML.GZ *.LML.BZ2 *.LML.XZ)"));
	// The "Automatically select file name extension (.lml)" option does not change anything

	if (path.isEmpty()) // "Cancel" was clicked
		return false;

	if (!path.endsWith(QLatin1String(".lml"), Qt::CaseInsensitive))
		path.append(QLatin1String(".lml"));

	// save new "last open directory"
	int pos = path.lastIndexOf(QLatin1String("/"));
	if (pos != -1) {
		const QString& newDir = path.left(pos);
		if (newDir != dir)
			conf.writeEntry("LastOpenDir", newDir);
	}

	return save(path);
}

/*!
 * auxiliary function that does the actual saving of the project
 */
bool MainWin::save(const QString& fileName) {
	DEBUG(Q_FUNC_INFO << ", file name = " << fileName.toStdString())
	QTemporaryFile tempFile(QDir::tempPath() + QLatin1Char('/') + QLatin1String("labplot_save_XXXXXX"));
	if (!tempFile.open()) {
		KMessageBox::error(this, i18n("Couldn't open the temporary file for writing."));
		return false;
	}

	WAIT_CURSOR_AUTO_RESET;
	const QString& tempFileName = tempFile.fileName();
	DEBUG("Using temporary file " << STDSTRING(tempFileName))
	tempFile.close();

	QIODevice* file;
	// if file ending is .lml, do xz compression or gzip compression in compatibility mode
	if (fileName.endsWith(QLatin1String(".lml"))) {
		if (m_project->fileCompression())
			file = new KCompressionDevice(tempFileName, KCompressionDevice::Xz);
		else
			file = new KCompressionDevice(tempFileName, KCompressionDevice::None);
	} else // use file ending to find out how to compress file
		file = new KCompressionDevice(tempFileName);
	if (!file)
		file = new QFile(tempFileName);

	bool ok;
	if (file->open(QIODevice::WriteOnly)) {
		m_project->setFileName(fileName);

		// thumbnail, used for the project preview
		QPixmap thumbnail = centralWidget()->grab(centralWidget()->childrenRect());

		// instead of doing a screenshot of the whole application window including also the special docks (project explorer, etc.) above,
		// we can also implement the following two cases:
		// 1. if the states of the dock widgets are saved in the project file, do the screenshot of the whole window
		// 2. if the states are not saved, determine the area of dock widgets without the special docks.
		// The logic below is an attempt to achieve this but it doesn't produce the desired results in all cases,
		// for example not if some of dock widgets are placed above the special docks.
		// TODO: decide what to do with this logic.
		/*
		if (m_project->saveDockStates())
			thumbnail = centralWidget()->grab(centralWidget()->childrenRect());
		else {
			// determine the bounding rectangle of the content (area without the special docks like project explorer, etc.)
			const auto& docks = m_dockManagerContent->dockWidgetsMap();
			QRect rect;
			for (auto* dock : docks) {
				auto dockRect = QRect(dock->mapToGlobal(dock->geometry().topLeft()), dock->geometry().size());
				rect = rect.united(dockRect);
			}

			if (!rect.isEmpty())
				thumbnail = centralWidget()->grab(QRect(centralWidget()->mapFromGlobal(rect.topLeft()), rect.size()));
		}
		*/

		// set the state of the content dock widgets
		auto state = m_dockManagerContent->saveState();
		// This conversion is fine, because in the dockmanager xml compression is turned off
		m_project->setDockWidgetState(QString::fromStdString(state.data()));

		// set the state of the default dock widgets, if needed
		if (m_project->saveDefaultDockWidgetState()) {
			state = m_dockManagerMain->saveState();
			m_project->setDefaultDockWidgetState(QString::fromStdString(state.data()));
		}

		m_project->setFileName(fileName);

		QXmlStreamWriter writer(file);
		m_project->save(thumbnail, &writer);
		m_project->setChanged(false);
		undoStackIndexLastSave = m_project->undoStack()->index();
		file->close();

		// target file must not exist
		if (QFile::exists(fileName))
			QFile::remove(fileName);

		// do not rename temp file. Qt still holds a handle (which fails renaming on Windows) and deletes it
		bool rc = QFile::copy(tempFileName, fileName);
		if (rc) {
			updateTitleBar();
			statusBar()->showMessage(i18n("Project saved"));
			m_actionsManager->m_saveAction->setEnabled(false);
			m_actionsManager->m_recentProjectsAction->addUrl(QUrl(fileName));
			ok = true;

			// if the project dock is visible, refresh the shown content
			//(version and modification time might have been changed)
			if (stackedWidget->currentWidget() == m_guiObserver->m_projectDock)
				m_guiObserver->m_projectDock->setProject(m_project);

			// we have a file name now
			//  -> auto save can be activated now if not happened yet
			if (m_autoSaveActive && !m_autoSaveTimer.isActive())
				m_autoSaveTimer.start();
		} else {
			KMessageBox::error(this, i18n("Couldn't save the file '%1'.", fileName));
			ok = false;
		}
	} else {
		KMessageBox::error(this, i18n("Couldn't open the file '%1' for writing.", fileName));
		ok = false;
	}

	delete file;

#ifdef HAVE_PURPOSE
	m_actionsManager->m_shareAction->setEnabled(true); // sharing is possible after the project was saved to a file
	m_actionsManager->fillShareMenu();
#endif

	return ok;
}

/*!
 * automatically saves the project in the specified time interval.
 */
void MainWin::autoSaveProject() {
	// don't auto save when there are no changes or the file name
	// was not provided yet (the project was never explicitly saved yet).
	if (!m_project->hasChanged() || m_project->fileName().isEmpty())
		return;

	this->saveProject();
}

void MainWin::updateTitleBar() {
	QString title;
	if (m_project) {
		switch (m_titleBarMode) {
		case TitleBarMode::ShowProjectName:
			title = m_project->name();
			break;
		case TitleBarMode::ShowFileName:
			if (m_project->fileName().isEmpty())
				title = m_project->name();
			else {
				QFileInfo fi(m_project->fileName());
				title = fi.baseName();
			}
			break;
		case TitleBarMode::ShowFilePath:
			if (m_project->fileName().isEmpty())
				title = m_project->name();
			else
				title = m_project->fileName();
		}

		if (m_project->hasChanged())
			title += QLatin1String("    [") + i18n("Changed") + QLatin1Char(']');
	} else
		title = QLatin1String("LabPlot");

	setCaption(title);
}

/*!
	prints the current sheet (worksheet, spreadsheet or matrix)
*/
void MainWin::print() {
	if (!m_currentAspectDock)
		return;

	AbstractPart* part = static_cast<ContentDockWidget*>(m_currentAspectDock)->part();
	statusBar()->showMessage(i18n("Preparing printing of %1", part->name()));
	if (part->printView())
		statusBar()->showMessage(i18n("%1 printed", part->name()));
	else
		statusBar()->clearMessage();
}

void MainWin::printPreview() {
	if (!m_currentAspectDock)
		return;

	AbstractPart* part = static_cast<ContentDockWidget*>(m_currentAspectDock)->part();
	statusBar()->showMessage(i18n("Preparing printing of %1", part->name()));
	if (part->printPreview())
		statusBar()->showMessage(i18n("%1 printed", part->name()));
	else
		statusBar()->clearMessage();
}

/**************************************************************************************/

/*!
	adds a new Folder to the project.
*/
void MainWin::newFolder() {
	Folder* folder = new Folder(i18n("Folder"));
	this->addAspectToProject(folder);
}

/*!
	adds a new Workbook to the project.
*/
void MainWin::newWorkbook() {
	auto* workbook = new Workbook(i18n("Workbook"));
	this->addAspectToProject(workbook);
}

/*!
	adds a new Datapicker to the project.
*/
void MainWin::newDatapicker() {
	auto* datapicker = new Datapicker(i18n("Data Extractor"));
	this->addAspectToProject(datapicker);
}

/*!
	adds a new Spreadsheet to the project.
*/
void MainWin::newSpreadsheet() {
	auto* spreadsheet = new Spreadsheet(i18n("Spreadsheet"));

	// if the current active window is a workbook or one of its children,
	// add the new matrix to the workbook
	auto* workbook = m_currentAspect->castTo<Workbook>();
	if (!workbook)
		workbook = m_currentAspect->parent<Workbook>();

	if (workbook)
		workbook->addChild(spreadsheet);
	else
		this->addAspectToProject(spreadsheet);
}

/*!
	adds a new Script to the project.
*/
#ifdef HAVE_SCRIPTING
void MainWin::newScript() {
	auto* action = static_cast<QAction*>(QObject::sender());
	auto* script = new Script(i18n("%1", action->data().toString()), action->data().toString());
	this->addAspectToProject(script);
}
#endif
/*!
	adds a new Matrix to the project.
*/
void MainWin::newMatrix() {
	Matrix* matrix = new Matrix(i18n("Matrix"));

	// if the current active window is a workbook or one of its children,
	// add the new matrix to the workbook
	auto* workbook = m_currentAspect->castTo<Workbook>();
	if (!workbook)
		workbook = m_currentAspect->parent<Workbook>();

	if (workbook)
		workbook->addChild(matrix);
	else
		this->addAspectToProject(matrix);
}

/*!
	adds a new Worksheet to the project.
*/
void MainWin::newWorksheet() {
	auto* worksheet = new Worksheet(i18n("Worksheet"));
	this->addAspectToProject(worksheet);
}

/*!
	adds a new Note to the project.
*/
void MainWin::newNotes() {
	Note* notes = new Note(i18n("Note"));
	this->addAspectToProject(notes);
}

/*!
	returns a pointer to a \c Spreadsheet object, if the currently active Mdi-Subwindow
	or if the currently selected tab in a \c WorkbookView is a \c SpreadsheetView
	Otherwise returns \c 0.
*/
Spreadsheet* MainWin::activeSpreadsheet() const {
	// 	if (dynamic_cast<QQuickWidget*>(centralWidget()))
	// 		return nullptr;

	if (!m_currentAspect)
		return nullptr;

	Spreadsheet* spreadsheet = nullptr;
	if (auto* s = m_currentAspect->castTo<Spreadsheet>())
		spreadsheet = s;
	else {
		// check whether one of spreadsheet columns is selected and determine the spreadsheet
		if (auto* parent = m_currentAspect->parentAspect()) {
			if (auto* s = parent->castTo<Spreadsheet>())
				spreadsheet = s;
		}
	}

	return spreadsheet;
}

#ifdef HAVE_CANTOR_LIBS
/*
	adds a new Cantor Spreadsheet to the project.
*/
void MainWin::newNotebook() {
	auto* action = static_cast<QAction*>(QObject::sender());
	m_actionsManager->m_lastUsedNotebookAction = action;
	auto* notebook = new Notebook(action->data().toString());
	this->addAspectToProject(notebook);
}

/********************************************************************************/
#endif

/*!
	called if there were changes in the project.
	Adds "changed" to the window caption and activates the save-Action.
*/
void MainWin::projectChanged() {
	updateTitleBar();
	m_actionsManager->m_newProjectAction->setEnabled(true);
	m_actionsManager->m_saveAction->setEnabled(true);
	m_actionsManager->m_undoAction->setEnabled(true);
}

void MainWin::handleAspectAdded(const AbstractAspect* aspect) {
	// register the signal-slot connections for aspects having a view.
	// if a folder or a workbook is being added, loop recursively through their children
	// and register the connections.
	const auto* part = dynamic_cast<const AbstractPart*>(aspect);
	if (part) {
		// 		connect(part, &AbstractPart::importFromFileRequested, this, &MainWin::importFileDialog);
		connect(part, &AbstractPart::importFromFileRequested, this, [=]() {
			importFileDialog();
		});
		connect(part, &AbstractPart::importFromSQLDatabaseRequested, this, &MainWin::importSqlDialog);
		// TODO: export, print and print preview should be handled in the views and not in MainWin.
		connect(part, &AbstractPart::exportRequested, this, &MainWin::exportDialog);
		connect(part, &AbstractPart::printRequested, this, &MainWin::print);
		connect(part, &AbstractPart::printPreviewRequested, this, &MainWin::printPreview);
		connect(part, &AbstractPart::showRequested, this, &MainWin::handleShowSubWindowRequested);

		const auto* worksheet = dynamic_cast<const Worksheet*>(aspect);
		if (worksheet) {
			connect(worksheet, &Worksheet::cartesianPlotMouseModeChanged, this, &MainWin::cartesianPlotMouseModeChanged);
			connect(worksheet, &Worksheet::propertiesExplorerRequested, this, &MainWin::propertiesExplorerRequested);
		} else if (aspect->type() == AspectType::Workbook) {
			for (auto* child : aspect->children<AbstractAspect>())
				handleAspectAdded(child);
		}
	} else if (aspect->type() == AspectType::Folder)
		for (auto* child : aspect->children<AbstractAspect>())
			handleAspectAdded(child);
}

void MainWin::handleAspectRemoved(const AbstractAspect* parent, const AbstractAspect* /*before*/, const AbstractAspect* aspect) {
	// no need to react on removal of
	//  - AbstractSimpleFilter
	//  - columns in the data spreadsheet of a datapicker curve,
	//    this can only happen when changing the error type and is done on the level of DatapickerImage

	if (m_projectExplorer->currentAspect() != aspect)
		return; // If the current aspect is not selected, there is no need to select the parent

	if (!aspect->inherits<AbstractFilter>() && !(parent->parentAspect() && parent->parentAspect()->type() == AspectType::DatapickerCurve))
		m_projectExplorer->setCurrentAspect(parent);
}

void MainWin::handleAspectAboutToBeRemoved(const AbstractAspect* aspect) {
	const auto* part = dynamic_cast<const AbstractPart*>(aspect);
	if (!part || !part->dockWidget())
		return;

	// don't do anything for children of Workbook and Datapicker, the dock widget is assigned to the parent
	// TODO: everything seems to work correctly also without this check and return. do we really need it?
	if (aspect->ancestor<Workbook>() || aspect->ancestor<Datapicker>())
		return;

	if (auto* win = part->dockWidget())
		m_dockManagerContent->removeDockWidget(win);
}

/*!
	called when the current aspect in the tree of the project explorer was changed.
	Selects the new aspect.
*/
void MainWin::handleCurrentAspectChanged(AbstractAspect* aspect) {
	DEBUG(Q_FUNC_INFO)
	if (!aspect)
		aspect = m_project; // should never happen, just in case

	m_suppressCurrentSubWindowChangedEvent = true;
	if (aspect->folder() != m_currentFolder) {
		m_currentFolder = aspect->folder();
		updateDockWindowVisibility();
	}

	m_currentAspect = aspect;

	// activate the corresponding MDI sub window for the current aspect
	activateSubWindowForAspect(aspect);
	m_suppressCurrentSubWindowChangedEvent = false;

	m_actionsManager->updateGUI();
}

void MainWin::activateSubWindowForAspect(const AbstractAspect* aspect) {
	Q_ASSERT(aspect);
	Q_ASSERT(m_dockManagerContent);
	const auto* part = dynamic_cast<const AbstractPart*>(aspect);
	if (part) {
		ContentDockWidget* win = part->dockWidget();

		auto* dock = m_dockManagerContent->findDockWidget(win->objectName());
		if (dock == nullptr) {
			// Add new dock if not found
			ads::CDockAreaWidget* areaWidget{nullptr};
			// when a dock area is empty, it is deleted and becomes invalid (for example after all its dock widgets are hidden/unpinned)
			// so we also need to check that the dock area of the current aspect is valid
			if (m_dockManagerContent->dockWidgetsMap().isEmpty() || !m_currentAspectDock || !m_currentAspectDockArea) {
				// If only project explorer and properties dock exist place it right to the project explorer
				areaWidget = m_dockManagerContent->addDockWidget(ads::CenterDockWidgetArea, win);
			} else {
				// Add dock on top of the current aspect, so it is directly visible
				areaWidget = m_dockManagerContent->addDockWidget(ads::CenterDockWidgetArea, win, m_currentAspectDock->dockAreaWidget());
			}

			// the dock area for the content widgets should be stretched to take the whole free space
			auto policy = areaWidget->sizePolicy();
			policy.setHorizontalStretch(1);
			areaWidget->setSizePolicy(policy);

			win->show();

			// Qt provides its own "system menu" for every sub-window. The shortcut for the close-action
			// in this menu collides with our global m_closeAction.
			// remove the shortcuts in the system menu to avoid this collision.
			for (QAction* action : win->titleBarActions())
				action->setShortcut(QKeySequence());
		} else
			dock->toggleView(true);

		m_currentAspectDock = win;
		m_currentAspectDockArea = m_currentAspectDock->dockAreaWidget();
		m_dockManagerContent->setDockWidgetFocused(win);
	} else {
		// activate the mdiView of the parent, if a child was selected
		const auto* parent = aspect->parentAspect();
		if (parent) {
			activateSubWindowForAspect(parent);

			// if the parent's parent is a Workbook (a column of a spreadsheet in workbook was selected),
			// we need to select the corresponding tab in WorkbookView too
			if (parent->parentAspect()) {
				auto* workbook = dynamic_cast<Workbook*>(parent->parentAspect());
				auto* datapicker = dynamic_cast<Datapicker*>(parent->parentAspect());
				if (!datapicker)
					datapicker = dynamic_cast<Datapicker*>(parent->parentAspect()->parentAspect());

				if (workbook)
					workbook->childSelected(parent);
				else if (datapicker)
					datapicker->childSelected(parent);
			}
		}
	}
	return;
}

void MainWin::setDockVisibility(QAction* action) {
	m_project->setDockVisibility((Project::DockVisibility)(action->data().toInt()));
}

/*!
	shows the sub window of a worksheet, matrix or a spreadsheet.
	Used if the window was closed before and the user asks to show
	the window again via the context menu in the project explorer.
*/
void MainWin::handleShowSubWindowRequested() {
	if (m_currentAspect)
		activateSubWindowForAspect(m_currentAspect);
}

/*!
	this is called on a right click on the root folder in the project explorer
*/
void MainWin::createContextMenu(QMenu* menu) const {
	QAction* firstAction = nullptr;
	// if we're populating the context menu for the project explorer, then
	// there're already actions available there. Skip the first title-action
	// and insert the action at the beginning of the menu.
	if (menu->actions().size() > 1)
		firstAction = menu->actions().at(1);

	menu->insertMenu(firstAction, m_actionsManager->m_newMenu);

	menu->insertSeparator(firstAction);
	menu->insertMenu(firstAction, m_actionsManager->m_visibilityMenu);
	menu->insertSeparator(firstAction);
}

/*!
	this is called on a right click on a non-root folder in the project explorer
*/
void MainWin::createFolderContextMenu(const Folder*, QMenu* menu) const {
	// Folder provides it's own context menu. Add a separator before adding additional actions.
	menu->addSeparator();
	this->createContextMenu(menu);
}

void MainWin::undo() {
	WAIT_CURSOR_AUTO_RESET;
	m_project->undoStack()->undo();
	m_actionsManager->m_redoAction->setEnabled(true);

	const int index = m_project->undoStack()->index();
	if (index == 0)
		m_actionsManager->m_undoAction->setEnabled(false);

	const bool changed = (index != undoStackIndexLastSave);
	m_actionsManager->m_saveAction->setEnabled(changed);
	m_project->setChanged(changed);
	updateTitleBar();
}

void MainWin::redo() {
	WAIT_CURSOR_AUTO_RESET;
	m_project->undoStack()->redo();
	m_actionsManager->m_undoAction->setEnabled(true);

	const int index = m_project->undoStack()->index();
	if (index == m_project->undoStack()->count())
		m_actionsManager->m_redoAction->setEnabled(false);

	const bool changed = (index != undoStackIndexLastSave);
	m_actionsManager->m_saveAction->setEnabled(changed);
	m_project->setChanged(changed);
	updateTitleBar();
}

/*!
	Shows/hides docks depending on the current visibility policy.
*/
void MainWin::updateDockWindowVisibility() const {
	auto windows = m_dockManagerContent->dockWidgetsMap();
	switch (m_project->dockVisibility()) {
	case Project::DockVisibility::allDocks:
		for (auto* window : windows)
			window->toggleView(true);

		break;
	case Project::DockVisibility::folderOnly:
		for (auto* window : windows) {
			auto* view = dynamic_cast<ContentDockWidget*>(window);
			if (view) {
				bool visible = view->part()->folder() == m_currentFolder;
				window->toggleView(visible);
			}
		}
		break;
	case Project::DockVisibility::folderAndSubfolders:
		for (auto* window : windows) {
			auto* view = dynamic_cast<ContentDockWidget*>(window);
			if (view) {
				bool visible = view->part()->isDescendantOf(m_currentFolder);
				window->toggleView(visible);
			}
		}
		break;
	}
}

void MainWin::propertiesExplorerRequested() {
	if (!m_propertiesDock->isVisible())
		m_propertiesDock->toggleView(true);
}

void MainWin::projectExplorerDockVisibilityChanged(bool visible) {
	m_actionsManager->m_projectExplorerDockAction->setChecked(visible);
}

void MainWin::propertiesDockVisibilityChanged(bool visible) {
	m_actionsManager->m_propertiesDockAction->setChecked(visible);
}

void MainWin::worksheetPreviewDockVisibilityChanged(bool visible) {
	m_actionsManager->m_worksheetPreviewAction->setChecked(visible);
}

void MainWin::cursorDockVisibilityChanged(bool visible) {
	// if the cursor dock was closed, switch to the "Select and Edit" mouse mode
	if (!visible) {
		// 		auto* worksheet = activeWorksheet();
		// TODO:
	}
}

void MainWin::cartesianPlotMouseModeChanged(CartesianPlot::MouseMode mode) {
	if (mode != CartesianPlot::MouseMode::Cursor) {
		if (cursorDock)
			cursorDock->toggleView(false);
	} else {
		if (!cursorDock) {
			cursorDock = new ads::CDockWidget(i18n("Cursor"), this);
			cursorWidget = new CursorDock(cursorDock);
			cursorDock->setWidget(cursorWidget);
			connect(cursorDock, &ads::CDockWidget::viewToggled, this, &MainWin::cursorDockVisibilityChanged);
			if (m_propertiesDock->sideTabWidget())
				m_dockManagerMain->addAutoHideDockWidget(m_propertiesDock->sideTabWidget()->sideBarLocation(), cursorDock);
			else
				m_dockManagerMain->addDockWidget(ads::CenterDockWidgetArea, cursorDock, m_propertiesDock->dockAreaWidget());
		} else
			focusCursorDock();

		auto* worksheet = static_cast<Worksheet*>(QObject::sender());
		connect(cursorWidget, &CursorDock::cursorUsed, this, &MainWin::focusCursorDock);
		cursorWidget->setWorksheet(worksheet);
		cursorDock->toggleView(true);
	}
}

void MainWin::focusCursorDock() {
	if (cursorDock) {
		cursorDock->toggleView(true);
		m_dockManagerMain->setDockWidgetFocused(cursorDock);
	}
}

void MainWin::closeEvent(QCloseEvent* event) {
	m_closing = true;

	// save the current state of the default dock widgets (project explorer, etc) _before_ all other content docks are closed
	auto group = Settings::group(QStringLiteral("MainWin"));
	group.writeEntry(QLatin1String("DockWidgetState"), m_dockManagerMain->saveState());

	if (!this->closeProject()) {
		m_closing = false;
		event->ignore();
	}
}

void MainWin::dragEnterEvent(QDragEnterEvent* event) {
	event->accept();
}

void MainWin::dropEvent(QDropEvent* event) {
	if (event->mimeData() && !event->mimeData()->urls().isEmpty()) {
		QUrl url = event->mimeData()->urls().at(0);
		const QString& fileName = url.toLocalFile();
		if (Project::isSupportedProject(fileName))
			openProject(fileName);
		else {
			if (!m_project)
				newProject();
			importFileDialog(fileName);
		}

		event->accept();
	} else
		event->ignore();
}

/*!
 * set the default locale of the application to the locale specified in the application settings for the number format and options.
 */
void MainWin::updateLocale() {
	// language used for the number format
	const auto group = Settings::group(QStringLiteral("Settings_General"));
	auto language = static_cast<QLocale::Language>(group.readEntry(QLatin1String("NumberFormat"), static_cast<int>(QLocale::Language::AnyLanguage)));
	QLocale newLocale(language == QLocale::AnyLanguage ? m_defaultSystemLocale : language);

	// number options
	auto numberOptions = static_cast<QLocale::NumberOptions>(group.readEntry(QLatin1String("NumberOptions"), static_cast<int>(QLocale::DefaultNumberOptions)));
	newLocale.setNumberOptions(numberOptions);
	QLocale::setDefault(newLocale);
}

/*!
 * used to migrate application settings if there were changes between the releases.
 */
void MainWin::migrateSettings() {
	// migrate the settings for the number format for versions older than 2.12 that had the decimal separator only:
	auto group = Settings::group(QStringLiteral("Settings_General"));
	if (group.hasKey(QLatin1String("DecimalSeparatorLocale"))) {
		// map from the old enum values for the decimal separator to new values of the used languages for the number format,
		// use languages that don't use any group separator for this.
		// old enum class DecimalSeparator { Dot, Comma, Arabic, Automatic };
		QLocale::Language language(QLocale::AnyLanguage); // AnyLanguage was used for 'Automatic'
		int decimalSeparator = group.readEntry(QLatin1String("DecimalSeparatorLocale"), 0);
		if (decimalSeparator == 0) // Dot
			language = QLocale::English;
		else if (decimalSeparator == 1) // Comma
			language = QLocale::German;
		else if (decimalSeparator == 2) // Arabic
			language = QLocale::Arabic;

		// delete the old entry and write the new one
		group.deleteEntry(QLatin1String("DecimalSeparatorLocale"));
		group.writeEntry(QLatin1String("NumberFormat"), static_cast<int>(language));
	}
}

void MainWin::handleSettingsChanges(QList<Settings::Type> changes) {
	WAIT_CURSOR_AUTO_RESET;
	const auto group = Settings::group(QStringLiteral("Settings_General"));

	// handle general settings
	// TODO: handle only those settings that were really changed, similar to how it's done for the nubmer format, etc. further below
	if (changes.contains(Settings::Type::General)) {
		// title bar
		MainWin::TitleBarMode titleBarMode = static_cast<MainWin::TitleBarMode>(group.readEntry("TitleBar", 0));
		if (titleBarMode != m_titleBarMode) {
			m_titleBarMode = titleBarMode;
			updateTitleBar();
		}

		// window visibility
		auto vis = Project::DockVisibility(group.readEntry("DockVisibility", 0));
		if (m_project && (vis != m_project->dockVisibility())) {
			if (vis == Project::DockVisibility::folderOnly)
				m_actionsManager->m_visibilityFolderAction->setChecked(true);
			else if (vis == Project::DockVisibility::folderAndSubfolders)
				m_actionsManager->m_visibilitySubfolderAction->setChecked(true);
			else
				m_actionsManager->m_visibilityAllAction->setChecked(true);
			m_project->setDockVisibility(vis);
		}

		// autosave
		bool autoSave = group.readEntry("AutoSave", 0);
		if (m_autoSaveActive != autoSave) {
			m_autoSaveActive = autoSave;
			if (autoSave)
				m_autoSaveTimer.start();
			else
				m_autoSaveTimer.stop();
		}

		int interval = group.readEntry("AutoSaveInterval", 1);
		interval *= 60 * 1000;
		if (interval != m_autoSaveTimer.interval())
			m_autoSaveTimer.setInterval(interval);
	}

	// update the number format in all visible dock widgets, worksheet elements and spreadsheets, if changed
	if (changes.contains(Settings::Type::General_Number_Format)) {
		updateLocale(); // set the new default runtime locale

		// dock widgets
		if (stackedWidget) {
			for (int i = 0; i < stackedWidget->count(); ++i) {
				auto* widget = stackedWidget->widget(i);
				auto* dock = dynamic_cast<BaseDock*>(widget);
				if (dock)
					dock->updateLocale();
				else {
					auto* labelWidget = dynamic_cast<LabelWidget*>(widget);
					if (labelWidget)
						labelWidget->updateLocale();
				}
			}
		}

		if (m_project) {
			// worksheet elements
			const auto& worksheets = m_project->children<Worksheet>(AbstractAspect::ChildIndexFlag::Recursive);
			for (const auto* worksheet : worksheets) {
				if (worksheet->viewCreated()) {
					const auto& elements = worksheet->children<WorksheetElement>(AbstractAspect::ChildIndexFlag::Recursive);
					for (auto* element : elements)
						element->updateLocale();
				}
			}

			// spreadsheets
			const auto& spreadsheets = m_project->children<Spreadsheet>(AbstractAspect::ChildIndexFlag::Recursive);
			for (auto* spreadsheet : spreadsheets) {
				if (spreadsheet->viewCreated())
					spreadsheet->updateLocale();
			}

			// matrices
			const auto& matrices = m_project->children<Matrix>(AbstractAspect::ChildIndexFlag::Recursive);
			for (auto* matrix : matrices) {
				if (matrix->viewCreated())
					matrix->updateLocale();
			}
		}
	}

	// update units in all dock widgets, if changed
	if (changes.contains(Settings::Type::General_Units)) {
		if (stackedWidget) {
			for (int i = 0; i < stackedWidget->count(); ++i) {
				auto* widget = stackedWidget->widget(i);
				auto* dock = dynamic_cast<BaseDock*>(widget);
				if (dock)
					dock->updateUnits();
				else {
					auto* labelWidget = dynamic_cast<LabelWidget*>(widget);
					if (labelWidget)
						labelWidget->updateUnits();
				}
			}
		}
	}

	// update the size of the preview thumbnails
	if (changes.contains(Settings::Type::Worksheet))
		m_worksheetPreviewWidget->updatePreviewSize();

	if (changes.contains(Settings::Type::Spreadsheet)) {
		// update spreadsheet header
		if (m_project) {
			const auto& spreadsheets = m_project->children<Spreadsheet>(AbstractAspect::ChildIndexFlag::Recursive);
			for (auto* spreadsheet : spreadsheets)
				spreadsheet->updateHorizontalHeader();
		}
	}

	// bool showWelcomeScreen = group.readEntry<bool>(QLatin1String("ShowWelcomeScreen"), true);
	// if (m_showWelcomeScreen != showWelcomeScreen)
	// 	m_showWelcomeScreen = showWelcomeScreen;

#ifdef HAVE_CANTOR_LIBS
	if (changes.contains(Settings::Type::Notebook))
		m_actionsManager->updateNotebookActions();
#else
	Q_UNUSED(changes)
#endif
}

void MainWin::openDatasetExample() {
	newProject();
	// 	addAspectToProject(m_welcomeScreenHelper->releaseConfiguredSpreadsheet());
}

/***************************************************************************************/
/************************************** dialogs ****************************************/
/***************************************************************************************/
/*!
  shows the dialog with the example projects.
*/
void MainWin::exampleProjectsDialog() {
	auto* dlg = new ExamplesDialog(this);
	if (dlg->exec() == QDialog::Accepted) {
		const auto& path = dlg->path();
		if (!path.isEmpty())
			openProject(path);
	}
	delete dlg;
}

/*!
  shows the dialog with the Undo-history.
*/
void MainWin::historyDialog() {
	if (!m_project->undoStack())
		return;

	auto* dialog = new HistoryDialog(this, m_project->undoStack(), m_undoViewEmptyLabel);
	int index = m_project->undoStack()->index();
	if (dialog->exec() != QDialog::Accepted) {
		if (m_project->undoStack()->count() != 0)
			m_project->undoStack()->setIndex(index);
	}

	// disable undo/redo-actions if the history was cleared
	//(in both cases, when accepted or rejected in the dialog)
	if (m_project->undoStack()->count() == 0) {
		m_actionsManager->m_undoAction->setEnabled(false);
		m_actionsManager->m_redoAction->setEnabled(false);
	}
}

/*!
  Opens the dialog to import data into the selected container.
*/
void MainWin::importFileDialog(const QString& fileName) {
	DEBUG(Q_FUNC_INFO << ", file name = " << fileName.toStdString());
	auto* dlg = new ImportFileDialog(this, false, fileName);

	// select existing container
	const auto type = m_currentAspect->type();
	if (type == AspectType::Spreadsheet || type == AspectType::Matrix || type == AspectType::Workbook)
		dlg->setCurrentIndex(m_projectExplorer->currentIndex());
	else if (type == AspectType::Column && m_currentAspect->parentAspect()->type() == AspectType::Spreadsheet)
		dlg->setCurrentIndex(m_aspectTreeModel->modelIndexOfAspect(m_currentAspect->parentAspect()));

	dlg->exec();
	DEBUG(Q_FUNC_INFO << " DONE");
}

/*!
  Opens the dialog to import multiple files in a directory into the selected container.
*/
void MainWin::importDirDialog(const QString& dir) {
	auto* dlg = new ImportFileDialog(this, false /* live source */, dir, true /* import directory */);

	// when importing a directory, the data is imported into multiple spreadsheet or matrix objects that are
	// created as children of the current parent folder/project or workbook.
	// select the current folder or workbook or use the parent folder of the current aspec if none of them is selected
	AbstractAspect* targetAspect = nullptr;
	const auto type = m_currentAspect->type();
	if (type == AspectType::Folder || type == AspectType::Workbook)
		targetAspect = m_currentAspect;
	else {
		targetAspect = m_currentAspect->parent<Folder>();
		if (!targetAspect)
			targetAspect = m_project;
	}

	dlg->setCurrentIndex(m_aspectTreeModel->modelIndexOfAspect(targetAspect));
	dlg->exec();
}

void MainWin::importKaggleDatasetDialog() {
	DEBUG(Q_FUNC_INFO);

	if (ImportKaggleDatasetDialog::checkKaggle()) {
		auto* dlg = new ImportKaggleDatasetDialog(this);
		dlg->exec();
	} else {
		QMessageBox::critical(this,
							  i18n("Kaggle CLI tool not found"),
							  i18n("Provide the path to the Kaggle CLI tool in the application settings and try again."));
		auto* dlg = new SettingsDialog(this, m_defaultSystemLocale);
		connect(dlg, &SettingsDialog::settingsChanged, this, &MainWin::handleSettingsChanges);
		dlg->navigateTo(Settings::Type::Datasets);
		dlg->exec();
	}

	DEBUG(Q_FUNC_INFO << " DONE");
}

void MainWin::importSqlDialog() {
	DEBUG(Q_FUNC_INFO);
	auto* dlg = new ImportSQLDatabaseDialog(this);

	// select existing container
	if (m_currentAspect->type() == AspectType::Spreadsheet || m_currentAspect->type() == AspectType::Matrix || m_currentAspect->type() == AspectType::Workbook)
		dlg->setCurrentIndex(m_projectExplorer->currentIndex());
	else if (m_currentAspect->type() == AspectType::Column && m_currentAspect->parentAspect()->type() == AspectType::Spreadsheet)
		dlg->setCurrentIndex(m_aspectTreeModel->modelIndexOfAspect(m_currentAspect->parentAspect()));

	dlg->exec();
	DEBUG(Q_FUNC_INFO << " DONE");
}

void MainWin::importProjectDialog() {
	DEBUG(Q_FUNC_INFO);

	ImportProjectDialog::ProjectType type;
	if (QObject::sender() == m_actionsManager->m_importOpjAction)
		type = ImportProjectDialog::ProjectType::Origin;
	else
		type = ImportProjectDialog::ProjectType::LabPlot;

	auto* dlg = new ImportProjectDialog(this, type);

	// set current folder
	dlg->setCurrentFolder(m_currentFolder);

	if (dlg->exec() == QDialog::Accepted) {
		dlg->importTo(statusBar());
		m_project->setChanged(true);
	}

	delete dlg;
	DEBUG(Q_FUNC_INFO << ", DONE");
}

/*!
 * \brief opens a dialog to import datasets
 */
void MainWin::importDatasetDialog() {
	DEBUG(Q_FUNC_INFO);
	auto* dlg = new ImportDatasetDialog(this);
	dlg->exec();
	DEBUG(Q_FUNC_INFO << ", DONE");
}

/*!
  opens the dialog for the export of the currently active worksheet, spreadsheet or matrix.
 */
void MainWin::exportDialog() {
	if (!m_currentAspectDock)
		return;

	AbstractPart* part = static_cast<ContentDockWidget*>(m_currentAspectDock)->part();
	if (part->exportView())
		statusBar()->showMessage(i18n("%1 exported", part->name()));
}

void MainWin::editFitsFileDialog() {
	auto* editDialog = new FITSHeaderEditDialog(this);
	if (editDialog->exec() == QDialog::Accepted) {
		if (editDialog->saved())
			statusBar()->showMessage(i18n("FITS files saved"));
	}
}

/*!
  adds a new file data source to the current project.
*/
void MainWin::newLiveDataSource() {
	auto* dlg = new ImportFileDialog(this, true);
	if (dlg->exec() == QDialog::Accepted) {
		if (dlg->sourceType() == LiveDataSource::SourceType::MQTT) {
#ifdef HAVE_MQTT
			auto* mqttClient = new MQTTClient(i18n("MQTT Client%1", 1));
			dlg->importToMQTT(mqttClient);

			// doesn't make sense to have more MQTTClients connected to the same broker
			auto clients = m_project->children<const MQTTClient>(AbstractAspect::ChildIndexFlag::Recursive);
			bool found = false;
			for (const auto* client : clients) {
				if (client->clientHostName() == mqttClient->clientHostName() && client->clientPort() == mqttClient->clientPort()) {
					found = true;
					break;
				}
			}

			if (!found) {
				mqttClient->setName(mqttClient->clientHostName());
				addAspectToProject(mqttClient);
			} else {
				delete mqttClient;
				QMessageBox::warning(this, i18n("Warning"), i18n("There already is a MQTTClient with this host!"));
			}
#endif
		} else {
			auto* dataSource = new LiveDataSource(i18n("Live data source%1", 1), false);
			dlg->importToLiveDataSource(dataSource, statusBar());
			addAspectToProject(dataSource);
		}
	}
	delete dlg;
}

void MainWin::addAspectToProject(AbstractAspect* aspect) {
	const auto& index = m_projectExplorer->currentIndex();
	if (index.isValid()) {
		auto* parent = static_cast<AbstractAspect*>(index.internalPointer());
#ifdef HAVE_MQTT
		// doesn't make sense to add a new MQTTClient to an existing MQTTClient or to any of its successors
		QString className = QLatin1String(parent->metaObject()->className());
		auto* clientAncestor = parent->ancestor<MQTTClient>();
		if (className == QLatin1String("MQTTClient"))
			parent = parent->parentAspect();
		else if (clientAncestor != nullptr)
			parent = clientAncestor->parentAspect();
#endif
		parent->folder()->addChild(aspect);
	} else
		m_project->addChild(aspect);
}

void MainWin::settingsDialog() {
	auto* dlg = new SettingsDialog(this, m_defaultSystemLocale);
	connect(dlg, &SettingsDialog::settingsChanged, this, &MainWin::handleSettingsChanges);
	dlg->exec();
}

#ifdef HAVE_CANTOR_LIBS
void MainWin::settingsNotebookDialog() {
	auto* dlg = new SettingsDialog(this, m_defaultSystemLocale);
	connect(dlg, &SettingsDialog::settingsChanged, this, &MainWin::handleSettingsChanges);
	dlg->navigateTo(Settings::Type::Notebook);
	dlg->exec();
}
#endif
