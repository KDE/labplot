/*
	File                 : MainWin.cc
	Project              : LabPlot
	Description          : Main window of the application
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2008-2018 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-FileCopyrightText: 2009-2024 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "MainWin.h"

#include "backend/core/AspectTreeModel.h"
#include "backend/core/Folder.h"
#include "backend/core/Project.h"
#include "backend/core/Settings.h"
#include "backend/core/Workbook.h"
#include "backend/datasources/DatasetHandler.h"
#include "backend/datasources/LiveDataSource.h"
#include "backend/matrix/Matrix.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/worksheet/Worksheet.h"
#ifdef HAVE_LIBORIGIN
#include "backend/datasources/projects/OriginProjectParser.h"
#endif
#include "backend/datapicker/Datapicker.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/macros.h"
#include "backend/note/Note.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"

#ifdef HAVE_MQTT
#include "backend/datasources/MQTTClient.h"
#endif

#include "commonfrontend/ProjectExplorer.h"
#include "commonfrontend/core/ContentDockWidget.h"
#include "commonfrontend/datapicker/DatapickerImageView.h"
#include "commonfrontend/datapicker/DatapickerView.h"
#include "commonfrontend/matrix/MatrixView.h"
#include "commonfrontend/note/NoteView.h"
#include "commonfrontend/spreadsheet/SpreadsheetView.h"
#include "commonfrontend/widgets/MemoryWidget.h"
#include "commonfrontend/worksheet/WorksheetView.h"

#include "kdefrontend/CASSettingsDialog.h"
#include "kdefrontend/GuiObserver.h"
#include "kdefrontend/HistoryDialog.h"
#include "kdefrontend/SettingsDialog.h"
#include "kdefrontend/colormaps/ColorMapsDialog.h"
#include "kdefrontend/datasources/ImportDatasetDialog.h"
#include "kdefrontend/datasources/ImportDatasetWidget.h"
#include "kdefrontend/datasources/ImportFileDialog.h"
#include "kdefrontend/datasources/ImportKaggleDatasetDialog.h"
#include "kdefrontend/datasources/ImportProjectDialog.h"
#include "kdefrontend/datasources/ImportSQLDatabaseDialog.h"
#include "kdefrontend/dockwidgets/CursorDock.h"
#include "kdefrontend/dockwidgets/ProjectDock.h"
#include "kdefrontend/examples/ExamplesDialog.h"
#include "kdefrontend/widgets/FITSHeaderEditDialog.h"
#include "kdefrontend/widgets/LabelWidget.h"
#include "kdefrontend/worksheet/WorksheetPreviewWidget.h"

#ifdef HAVE_KUSERFEEDBACK
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <KUserFeedbackQt6/ApplicationVersionSource>
#include <KUserFeedbackQt6/PlatformInfoSource>
#include <KUserFeedbackQt6/QtVersionSource>
#include <KUserFeedbackQt6/ScreenInfoSource>
#include <KUserFeedbackQt6/StartCountSource>
#include <KUserFeedbackQt6/UsageTimeSource>
#else
#include <KUserFeedback/ApplicationVersionSource>
#include <KUserFeedback/PlatformInfoSource>
#include <KUserFeedback/QtVersionSource>
#include <KUserFeedback/ScreenInfoSource>
#include <KUserFeedback/StartCountSource>
#include <KUserFeedback/UsageTimeSource>
#endif
#endif

#ifdef HAVE_PURPOSE
#include <Purpose/AlternativesModel>
#include <purpose_version.h>
#if PURPOSE_VERSION >= QT_VERSION_CHECK(5, 104, 0)
#include <Purpose/Menu>
#else
#include <PurposeWidgets/Menu>
#endif
#include <QMimeType>
#endif

#include <DockAreaWidget.h>
#include <DockManager.h>

#ifdef HAVE_TOUCHBAR
#include "3rdparty/kdmactouchbar/src/kdmactouchbar.h"
#endif

#include <QActionGroup>
#include <QCloseEvent>
#include <QElapsedTimer>
#include <QFileDialog>
#include <QMenu>
#include <QMenuBar>
#include <QMimeData>
#include <QProcess>
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

#include <KActionCollection>
#include <KActionMenu>
#include <KColorScheme>
#include <KColorSchemeManager>
#include <kconfigwidgets_version.h>
#if KCONFIGWIDGETS_VERSION >= QT_VERSION_CHECK(5, 107, 0)
#include <KColorSchemeMenu>
#endif
#include <KCompressionDevice>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KMessageBox>
#include <KRecentFilesAction>
#include <KStandardAction>
#include <KToggleAction>
#include <KToggleFullScreenAction>
#include <KToolBar>
#include <kxmlguifactory.h>

#ifdef HAVE_CANTOR_LIBS
#include "backend/cantorWorksheet/CantorWorksheet.h"
#include "commonfrontend/cantorWorksheet/CantorWorksheetView.h"
#include <cantor/backend.h>
#endif

/*!
\class MainWin
\brief Main application window.

\ingroup kdefrontend
*/
MainWin::MainWin(QWidget* parent, const QString& filename)
	: KXmlGuiWindow(parent)
	, m_schemeManager(new KColorSchemeManager(this)) {
	initGUI(filename);
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
	m_recentProjectsAction->saveEntries(Settings::group(QStringLiteral("Recent Files")));

	auto group = Settings::group(QStringLiteral("MainWin"));
	group.writeEntry(QLatin1String("geometry"), saveGeometry()); // current geometry of the main window
	group.writeEntry(QLatin1String("WindowState"), saveState()); // current state of QMainWindow's toolbars
	group.writeEntry(QLatin1String("lastOpenFileFilter"), m_lastOpenFileFilter);
	group.writeEntry(QLatin1String("ShowMemoryInfo"), (m_memoryInfoWidget != nullptr));
	Settings::sync();

	if (m_project) {
		delete m_guiObserver;
		delete m_aspectTreeModel;
		disconnect(m_project, nullptr, this, nullptr);
		delete m_project;
	}

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

	initActions();

#ifdef Q_OS_MAC
#ifdef HAVE_TOUCHBAR
	// setup touchbar before GUI (otherwise actions in the toolbar are not selectable)
	m_touchBar = new KDMacTouchBar(this);
	// m_touchBar->setTouchButtonStyle(KDMacTouchBar::IconOnly);
#endif
	setUnifiedTitleAndToolBarOnMac(true);
#endif
	setupGUI();

	// all toolbars created via the KXMLGUI framework are locked on default:
	//  * on the very first program start, unlock all toolbars
	//  * on later program starts, set stored lock status
	// Furthermore, we want to show icons only after the first program start.
	KConfigGroup groupMain = Settings::group(QStringLiteral("MainWindow"));
	if (groupMain.exists()) {
		// KXMLGUI framework automatically stores "Disabled" for the key "ToolBarsMovable"
		// in case the toolbars are locked -> load this value
		const QString& str = groupMain.readEntry(QLatin1String("ToolBarsMovable"), "");
		bool locked = (str == QLatin1String("Disabled"));
		KToolBar::setToolBarsLocked(locked);
	}

	// in case we're starting for the first time, put all toolbars into the IconOnly mode
	// and maximize the main window. The occurence of LabPlot's own section "MainWin"
	// indicates whether this is the first start or not
	groupMain = Settings::group(QStringLiteral("MainWin"));
	if (!groupMain.exists()) {
		// first start
		KToolBar::setToolBarsLocked(false);

		// show icons only
		const auto& toolbars = factory()->containers(QLatin1String("ToolBar"));
		for (auto* container : toolbars) {
			auto* toolbar = dynamic_cast<QToolBar*>(container);
			if (toolbar)
				toolbar->setToolButtonStyle(Qt::ToolButtonIconOnly);
		}

		showMaximized();
	}

	initMenus();

	auto* mainToolBar = qobject_cast<QToolBar*>(factory()->container(QLatin1String("main_toolbar"), this));

#ifdef HAVE_CANTOR_LIBS
	auto* tbNotebook = new QToolButton(mainToolBar);
	tbNotebook->setPopupMode(QToolButton::MenuButtonPopup);
	tbNotebook->setMenu(m_newNotebookMenu);
	tbNotebook->setDefaultAction(m_configureCASAction);
	auto* lastAction = mainToolBar->actions().at(mainToolBar->actions().count() - 2);
	mainToolBar->insertWidget(lastAction, tbNotebook);
#endif

	auto* tbImport = new QToolButton(mainToolBar);
	tbImport->setPopupMode(QToolButton::MenuButtonPopup);
	tbImport->setMenu(m_importMenu);
	tbImport->setDefaultAction(m_importFileAction);
	auto* lastAction_ = mainToolBar->actions().at(mainToolBar->actions().count() - 1);
	mainToolBar->insertWidget(lastAction_, tbImport);

	// hamburger menu
#if KCONFIGWIDGETS_VERSION >= QT_VERSION_CHECK(5, 81, 0)
	m_hamburgerMenu = KStandardAction::hamburgerMenu(nullptr, nullptr, actionCollection());
	toolBar()->addAction(m_hamburgerMenu);
	m_hamburgerMenu->hideActionsOf(toolBar());
	m_hamburgerMenu->setMenuBar(menuBar());
#endif

	setWindowIcon(QIcon::fromTheme(QLatin1String("LabPlot2"), QGuiApplication::windowIcon()));
	setAttribute(Qt::WA_DeleteOnClose);

	// make the status bar of a fixed size in order to avoid height changes when placing a ProgressBar there.
	QFont font;
	font.setFamily(font.defaultFamily());
	QFontMetrics fm(font);
	statusBar()->setFixedHeight(fm.height() + 5);

	// load recently used projects
	m_recentProjectsAction->loadEntries(Settings::group(QStringLiteral("Recent Files")));

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
			if (!path.isEmpty())
				openProject(path);
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

	// read the settings of MainWin
	const KConfigGroup& groupMainWin = Settings::group(QStringLiteral("MainWin"));

	// show memory info
	m_memoryInfoAction->setEnabled(statusBar()->isEnabled()); // disable/enable menu with statusbar
	bool memoryInfoShown = groupMainWin.readEntry(QLatin1String("ShowMemoryInfo"), true);
	// DEBUG(Q_FUNC_INFO << ", memory info enabled in config: " << memoryInfoShown)
	m_memoryInfoAction->setChecked(memoryInfoShown);
	if (memoryInfoShown)
		toggleMemoryInfo();

	// restore the geometry
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

void MainWin::changeVisibleAllDocks(bool visible) {
	for (auto dock : m_dockManagerContent->dockWidgetsMap())
		dock->toggleView(visible);
}

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
	if (w == m_currentAspectDock)
		m_currentAspectDock = nullptr;
}

void MainWin::dockFocusChanged(ads::CDockWidget* old, ads::CDockWidget* now) {
	Q_UNUSED(old);
	if (!now) {
		updateGUI();
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

	updateGUI();
	if (!m_suppressCurrentSubWindowChangedEvent)
		m_projectExplorer->setCurrentAspect(view->part());
}

void MainWin::initActions() {
	// ******************** File-menu *******************************
	// add some standard actions
	m_newProjectAction = KStandardAction::openNew(
		this,
		[=]() {
			newProject(true);
		},
		actionCollection());
	m_openProjectAction = KStandardAction::open(this, static_cast<void (MainWin::*)()>(&MainWin::openProject), actionCollection());
	m_recentProjectsAction = KStandardAction::openRecent(this, &MainWin::openRecentProject, actionCollection());
	// m_closeAction = KStandardAction::close(this, &MainWin::closeProject, actionCollection());
	// actionCollection()->setDefaultShortcut(m_closeAction, QKeySequence()); // remove the shortcut, QKeySequence::Close will be used for closing sub-windows
	m_saveAction = KStandardAction::save(this, &MainWin::saveProject, actionCollection());
	m_saveAsAction = KStandardAction::saveAs(this, &MainWin::saveProjectAs, actionCollection());
	m_printAction = KStandardAction::print(this, &MainWin::print, actionCollection());
	m_printPreviewAction = KStandardAction::printPreview(this, &MainWin::printPreview, actionCollection());

	QAction* openExample = new QAction(i18n("&Open Example"), actionCollection());
	openExample->setIcon(QIcon::fromTheme(QLatin1String("folder-documents")));
	actionCollection()->addAction(QLatin1String("file_example_open"), openExample);
	connect(openExample, &QAction::triggered, this, [=]() {
		auto* dlg = new ExamplesDialog(this);
		if (dlg->exec() == QDialog::Accepted) {
			const auto& path = dlg->path();
			if (!path.isEmpty())
				openProject(path);
		}
		delete dlg;
	});

	m_fullScreenAction = KStandardAction::fullScreen(this, &MainWin::toggleFullScreen, this, actionCollection());

	// QDEBUG(Q_FUNC_INFO << ", preferences action name:" << KStandardAction::name(KStandardAction::Preferences))
	KStandardAction::preferences(this, &MainWin::settingsDialog, actionCollection());
	// QAction* action = actionCollection()->action(KStandardAction::name(KStandardAction::Preferences)));
	KStandardAction::quit(this, &MainWin::close, actionCollection());

	// New Folder/Workbook/Spreadsheet/Matrix/Worksheet/Datasources
	m_newWorkbookAction = new QAction(QIcon::fromTheme(QLatin1String("labplot-workbook-new")), i18n("Workbook"), this);
	actionCollection()->addAction(QLatin1String("new_workbook"), m_newWorkbookAction);
	m_newWorkbookAction->setWhatsThis(i18n("Creates a new workbook for collection spreadsheets, matrices and plots"));
	connect(m_newWorkbookAction, &QAction::triggered, this, &MainWin::newWorkbook);

	m_newDatapickerAction = new QAction(QIcon::fromTheme(QLatin1String("color-picker-black")), i18n("Data Extractor"), this);
	m_newDatapickerAction->setWhatsThis(i18n("Creates a data extractor for getting data from a picture"));
	actionCollection()->addAction(QLatin1String("new_datapicker"), m_newDatapickerAction);
	connect(m_newDatapickerAction, &QAction::triggered, this, &MainWin::newDatapicker);

	m_newSpreadsheetAction = new QAction(QIcon::fromTheme(QLatin1String("labplot-spreadsheet-new")), i18n("Spreadsheet"), this);
	// 	m_newSpreadsheetAction->setShortcut(Qt::CTRL+Qt::Key_Equal);
	m_newSpreadsheetAction->setWhatsThis(i18n("Creates a new spreadsheet for data editing"));
	actionCollection()->addAction(QLatin1String("new_spreadsheet"), m_newSpreadsheetAction);
	connect(m_newSpreadsheetAction, &QAction::triggered, this, &MainWin::newSpreadsheet);

	m_newMatrixAction = new QAction(QIcon::fromTheme(QLatin1String("labplot-matrix-new")), i18n("Matrix"), this);
	// 	m_newMatrixAction->setShortcut(Qt::CTRL+Qt::Key_Equal);
	m_newMatrixAction->setWhatsThis(i18n("Creates a new matrix for data editing"));
	actionCollection()->addAction(QLatin1String("new_matrix"), m_newMatrixAction);
	connect(m_newMatrixAction, &QAction::triggered, this, &MainWin::newMatrix);

	m_newWorksheetAction = new QAction(QIcon::fromTheme(QLatin1String("labplot-worksheet-new")), i18n("Worksheet"), this);
	// 	m_newWorksheetAction->setShortcut(Qt::ALT+Qt::Key_X);
	m_newWorksheetAction->setWhatsThis(i18n("Creates a new worksheet for data plotting"));
	actionCollection()->addAction(QLatin1String("new_worksheet"), m_newWorksheetAction);
	connect(m_newWorksheetAction, &QAction::triggered, this, &MainWin::newWorksheet);

	m_newNotesAction = new QAction(QIcon::fromTheme(QLatin1String("document-new")), i18n("Note"), this);
	m_newNotesAction->setWhatsThis(i18n("Creates a new note for arbitrary text"));
	actionCollection()->addAction(QLatin1String("new_notes"), m_newNotesAction);
	connect(m_newNotesAction, &QAction::triggered, this, &MainWin::newNotes);

	m_newFolderAction = new QAction(QIcon::fromTheme(QLatin1String("folder-new")), i18n("Folder"), this);
	m_newFolderAction->setWhatsThis(i18n("Creates a new folder to collect sheets and other elements"));
	actionCollection()->addAction(QLatin1String("new_folder"), m_newFolderAction);
	connect(m_newFolderAction, &QAction::triggered, this, &MainWin::newFolder);

	//"New file datasources"
	m_newLiveDataSourceAction = new QAction(QIcon::fromTheme(QLatin1String("edit-text-frame-update")), i18n("Live Data Source..."), this);
	m_newLiveDataSourceAction->setWhatsThis(i18n("Creates a live data source to read data from a real time device"));
	actionCollection()->addAction(QLatin1String("new_live_datasource"), m_newLiveDataSourceAction);
	connect(m_newLiveDataSourceAction, &QAction::triggered, this, &MainWin::newLiveDataSource);

	// Import/Export
	m_importFileAction = new QAction(QIcon::fromTheme(QLatin1String("document-import")), i18n("From File..."), this);
	actionCollection()->setDefaultShortcut(m_importFileAction, Qt::CTRL | Qt::SHIFT | Qt::Key_I);
	m_importFileAction->setWhatsThis(i18n("Import data from a regular file"));
	connect(m_importFileAction, &QAction::triggered, this, [=]() {
		importFileDialog();
	});

	// second "import from file" action, with a shorter name, to be used in the sub-menu of the "Import"-menu.
	// the first action defined above will be used in the toolbar and touchbar where we need the more detailed name "Import From File".
	m_importFileAction_2 = new QAction(QIcon::fromTheme(QLatin1String("document-import")), i18n("From File..."), this);
	actionCollection()->addAction(QLatin1String("import_file"), m_importFileAction_2);
	m_importFileAction_2->setWhatsThis(i18n("Import data from a regular file"));
	connect(m_importFileAction_2, &QAction::triggered, this, [=]() {
		importFileDialog();
	});

	m_importKaggleDatasetAction = new QAction(QIcon::fromTheme(QLatin1String("labplot-kaggle")), i18n("From kaggle.com..."), this);
	m_importKaggleDatasetAction->setWhatsThis(i18n("Import data from kaggle.com"));
	actionCollection()->addAction(QLatin1String("import_dataset_kaggle"), m_importKaggleDatasetAction);
	connect(m_importKaggleDatasetAction, &QAction::triggered, this, &MainWin::importKaggleDatasetDialog);

	m_importSqlAction = new QAction(QIcon::fromTheme(QLatin1String("network-server-database")), i18n("From SQL Database..."), this);
	m_importSqlAction->setWhatsThis(i18n("Import data from a SQL database"));
	actionCollection()->addAction(QLatin1String("import_sql"), m_importSqlAction);
	connect(m_importSqlAction, &QAction::triggered, this, &MainWin::importSqlDialog);

	m_importDatasetAction = new QAction(QIcon::fromTheme(QLatin1String("database-index")), i18n("From Dataset Collection..."), this);
	m_importDatasetAction->setWhatsThis(i18n("Import data from an online dataset"));
	actionCollection()->addAction(QLatin1String("import_dataset_datasource"), m_importDatasetAction);
	connect(m_importDatasetAction, &QAction::triggered, this, &MainWin::importDatasetDialog);

	m_importLabPlotAction = new QAction(QIcon::fromTheme(QLatin1String("project-open")), i18n("LabPlot Project..."), this);
	m_importLabPlotAction->setWhatsThis(i18n("Import a project from a LabPlot project file (.lml)"));
	actionCollection()->addAction(QLatin1String("import_labplot"), m_importLabPlotAction);
	connect(m_importLabPlotAction, &QAction::triggered, this, &MainWin::importProjectDialog);

#ifdef HAVE_LIBORIGIN
	m_importOpjAction = new QAction(QIcon::fromTheme(QLatin1String("project-open")), i18n("Origin Project (OPJ)..."), this);
	m_importOpjAction->setWhatsThis(i18n("Import a project from an OriginLab Origin project file (.opj)"));
	actionCollection()->addAction(QLatin1String("import_opj"), m_importOpjAction);
	connect(m_importOpjAction, &QAction::triggered, this, &MainWin::importProjectDialog);
#endif

	m_exportAction = new QAction(QIcon::fromTheme(QLatin1String("document-export")), i18n("Export..."), this);
	m_exportAction->setWhatsThis(i18n("Export selected element"));
	actionCollection()->setDefaultShortcut(m_exportAction, Qt::CTRL | Qt::SHIFT | Qt::Key_E);
	actionCollection()->addAction(QLatin1String("export"), m_exportAction);
	connect(m_exportAction, &QAction::triggered, this, &MainWin::exportDialog);

#ifdef HAVE_PURPOSE
	m_shareAction = new QAction(QIcon::fromTheme(QLatin1String("document-share")), i18n("Share"), this);
	actionCollection()->addAction(QLatin1String("share"), m_shareAction);
#endif

	// Tools
	auto* action = new QAction(QIcon::fromTheme(QLatin1String("color-management")), i18n("Color Maps Browser"), this);
	action->setWhatsThis(i18n("Open dialog to browse through the available color maps."));
	actionCollection()->addAction(QLatin1String("color_maps"), action);
	connect(action, &QAction::triggered, this, [=]() {
		auto* dlg = new ColorMapsDialog(this);
		dlg->exec();
		delete dlg;
	});

#ifdef HAVE_FITS
	action = new QAction(QIcon::fromTheme(QLatin1String("editor")), i18n("FITS Metadata Editor..."), this);
	action->setWhatsThis(i18n("Open editor to edit FITS meta data"));
	actionCollection()->addAction(QLatin1String("edit_fits"), action);
	connect(action, &QAction::triggered, this, &MainWin::editFitsFileDialog);
#endif

	// Edit
	// Undo/Redo-stuff
	m_undoAction = KStandardAction::undo(this, SLOT(undo()), actionCollection());
	m_redoAction = KStandardAction::redo(this, SLOT(redo()), actionCollection());
	m_historyAction = new QAction(QIcon::fromTheme(QLatin1String("view-history")), i18n("Undo/Redo History..."), this);
	actionCollection()->addAction(QLatin1String("history"), m_historyAction);
	connect(m_historyAction, &QAction::triggered, this, &MainWin::historyDialog);

#ifdef Q_OS_MAC
	m_undoIconOnlyAction = new QAction(m_undoAction->icon(), QString());
	connect(m_undoIconOnlyAction, &QAction::triggered, this, &MainWin::undo);

	m_redoIconOnlyAction = new QAction(m_redoAction->icon(), QString());
	connect(m_redoIconOnlyAction, &QAction::triggered, this, &MainWin::redo);
#endif
	// TODO: more menus
	//  Appearance
	// Analysis: see WorksheetView.cpp
	// Drawing
	// Script

	// Windows
	m_closeWindowAction = new QAction(i18n("&Close"), this);
	actionCollection()->setDefaultShortcut(m_closeWindowAction, QKeySequence::Close);
	m_closeWindowAction->setStatusTip(i18n("Close the active window"));
	actionCollection()->addAction(QLatin1String("close window"), m_closeWindowAction);

	m_closeAllWindowsAction = new QAction(i18n("Close &All"), this);
	m_closeAllWindowsAction->setStatusTip(i18n("Close all the windows"));
	actionCollection()->addAction(QLatin1String("close all windows"), m_closeAllWindowsAction);

	m_nextWindowAction = new QAction(QIcon::fromTheme(QLatin1String("go-next-view")), i18n("Ne&xt"), this);
	actionCollection()->setDefaultShortcut(m_nextWindowAction, QKeySequence::NextChild);
	m_nextWindowAction->setStatusTip(i18n("Move the focus to the next window"));
	actionCollection()->addAction(QLatin1String("next window"), m_nextWindowAction);

	m_prevWindowAction = new QAction(QIcon::fromTheme(QLatin1String("go-previous-view")), i18n("Pre&vious"), this);
	actionCollection()->setDefaultShortcut(m_prevWindowAction, QKeySequence::PreviousChild);
	m_prevWindowAction->setStatusTip(i18n("Move the focus to the previous window"));
	actionCollection()->addAction(QLatin1String("previous window"), m_prevWindowAction);

	// Actions for window visibility
	auto* windowVisibilityActions = new QActionGroup(this);
	windowVisibilityActions->setExclusive(true);

	m_visibilityFolderAction = new QAction(QIcon::fromTheme(QLatin1String("folder")), i18n("Current &Folder Only"), windowVisibilityActions);
	m_visibilityFolderAction->setCheckable(true);
	m_visibilityFolderAction->setData(static_cast<int>(Project::DockVisibility::folderOnly));

	m_visibilitySubfolderAction =
		new QAction(QIcon::fromTheme(QLatin1String("folder-documents")), i18n("Current Folder and &Subfolders"), windowVisibilityActions);
	m_visibilitySubfolderAction->setCheckable(true);
	m_visibilitySubfolderAction->setData(static_cast<int>(Project::DockVisibility::folderAndSubfolders));

	m_visibilityAllAction = new QAction(i18n("&All"), windowVisibilityActions);
	m_visibilityAllAction->setCheckable(true);
	m_visibilityAllAction->setData(static_cast<int>(Project::DockVisibility::allDocks));

	connect(windowVisibilityActions, &QActionGroup::triggered, this, &MainWin::setDockVisibility);

	// show/hide the status and menu bars
	//  KMainWindow should provide a menu that allows showing/hiding of the statusbar via showStatusbar()
	//  see https://api.kde.org/frameworks/kxmlgui/html/classKXmlGuiWindow.html#a3d7371171cafabe30cb3bb7355fdfed1
	// KXMLGUI framework automatically stores "Disabled" for the key "StatusBar"
	KConfigGroup groupMain = Settings::group(QStringLiteral("MainWindow"));
	const QString& str = groupMain.readEntry(QLatin1String("StatusBar"), "");
	bool statusBarDisabled = (str == QLatin1String("Disabled"));
	DEBUG(Q_FUNC_INFO << ", statusBar enabled in config: " << !statusBarDisabled)
	createStandardStatusBarAction();
	m_statusBarAction = KStandardAction::showStatusbar(this, &MainWin::toggleStatusBar, actionCollection());
	m_statusBarAction->setChecked(!statusBarDisabled);
	statusBar()->setEnabled(!statusBarDisabled); // setVisible() does not work

	KStandardAction::showMenubar(this, &MainWin::toggleMenuBar, actionCollection());

	// show/hide the memory usage widget
	m_memoryInfoAction = new QAction(i18n("Show Memory Usage"), this);
	m_memoryInfoAction->setCheckable(true);
	connect(m_memoryInfoAction, &QAction::triggered, this, &MainWin::toggleMemoryInfo);

	// Actions for hiding/showing the dock widgets
	auto* docksActions = new QActionGroup(this);
	docksActions->setExclusive(false);

	m_projectExplorerDockAction = new QAction(QIcon::fromTheme(QLatin1String("view-list-tree")), i18n("Project Explorer"), docksActions);
	m_projectExplorerDockAction->setCheckable(true);
	m_projectExplorerDockAction->setChecked(true);
	actionCollection()->addAction(QLatin1String("toggle_project_explorer_dock"), m_projectExplorerDockAction);

	m_propertiesDockAction = new QAction(QIcon::fromTheme(QLatin1String("view-list-details")), i18n("Properties Explorer"), docksActions);
	m_propertiesDockAction->setCheckable(true);
	m_propertiesDockAction->setChecked(true);
	actionCollection()->addAction(QLatin1String("toggle_properties_explorer_dock"), m_propertiesDockAction);

	m_worksheetPreviewAction = new QAction(QIcon::fromTheme(QLatin1String("view-preview")), i18n("Worksheet Preview"), docksActions);
	m_worksheetPreviewAction->setCheckable(true);
	m_worksheetPreviewAction->setChecked(true);
	actionCollection()->addAction(QLatin1String("toggle_worksheet_preview_dock"), m_worksheetPreviewAction);

	connect(docksActions, &QActionGroup::triggered, this, &MainWin::toggleDockWidget);

	// global search
	m_searchAction = new QAction(actionCollection());
	m_searchAction->setShortcut(QKeySequence::Find);
	connect(m_searchAction, &QAction::triggered, this, [=]() {
		if (m_project) {
			if (!m_projectExplorerDock->isVisible()) {
				m_projectExplorerDockAction->setChecked(true);
				toggleDockWidget(m_projectExplorerDockAction);
			}
			m_projectExplorer->search();
		}
	});
	this->addAction(m_searchAction);

#ifdef HAVE_CANTOR_LIBS
	// configure CAS backends
	m_configureCASAction = new QAction(QIcon::fromTheme(QLatin1String("cantor")), i18n("Configure CAS..."), this);
	m_configureCASAction->setWhatsThis(i18n("Opens the settings for Computer Algebra Systems to modify the available systems or to enable new ones"));
	m_configureCASAction->setMenuRole(QAction::NoRole); // prevent macOS Qt heuristics to select this action for preferences
	actionCollection()->addAction(QLatin1String("configure_cas"), m_configureCASAction);
	connect(m_configureCASAction, &QAction::triggered, this, &MainWin::casSettingsDialog);
#endif
}

void MainWin::initMenus() {
#ifdef HAVE_PURPOSE
	m_shareMenu = new Purpose::Menu(this);
	m_shareMenu->model()->setPluginType(QStringLiteral("Export"));
	connect(m_shareMenu, &Purpose::Menu::finished, this, &MainWin::shareActionFinished);
	m_shareAction->setMenu(m_shareMenu);
#endif

	// add the actions to toggle the status bar and the project and properties explorer widgets to the "View" menu.
	// this menu is created automatically when the default "full screen" action is created in initActions().
	auto* menu = dynamic_cast<QMenu*>(factory()->container(QLatin1String("view"), this));
	if (menu) {
		menu->addSeparator();
		menu->addAction(m_projectExplorerDockAction);
		menu->addAction(m_propertiesDockAction);
		menu->addAction(m_worksheetPreviewAction);
	}

	// menu in the main toolbar for adding new aspects
	menu = dynamic_cast<QMenu*>(factory()->container(QLatin1String("new"), this));
	if (menu)
		menu->setIcon(QIcon::fromTheme(QLatin1String("window-new")));

	// menu in the project explorer and in the toolbar for adding new aspects
	m_newMenu = new QMenu(i18n("Add New"), this);
	m_newMenu->setIcon(QIcon::fromTheme(QLatin1String("window-new")));
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
	m_importMenu = new QMenu(this);
	m_importMenu->setIcon(QIcon::fromTheme(QLatin1String("document-import")));
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
	menu = qobject_cast<QMenu*>(factory()->container(QLatin1String("import"), this));
	menu->setIcon(QIcon::fromTheme(QLatin1String("document-import")));

	// menu subwindow visibility policy
	m_visibilityMenu = new QMenu(i18n("Window Visibility"), this);
	m_visibilityMenu->setIcon(QIcon::fromTheme(QLatin1String("window-duplicate")));
	m_visibilityMenu->addAction(m_visibilityFolderAction);
	m_visibilityMenu->addAction(m_visibilitySubfolderAction);
	m_visibilityMenu->addAction(m_visibilityAllAction);

#ifdef HAVE_FITS
//	m_editMenu->addAction(m_editFitsFileAction);
#endif

	// set the action for the current color scheme checked
	auto group = Settings::group(QStringLiteral("Settings_General"));
#if KCONFIGWIDGETS_VERSION >= QT_VERSION_CHECK(5, 67, 0) // KColorSchemeManager has a system default option
	QString schemeName = group.readEntry("ColorScheme");
#else
	auto generalGlobalsGroup = KSharedConfig::openConfig(QLatin1String("kdeglobals"))->group("General");
	QString defaultSchemeName = generalGlobalsGroup.readEntry("ColorScheme", QStringLiteral("Breeze"));
	QString schemeName = group.readEntry("ColorScheme", defaultSchemeName);
#endif
	// default dark scheme on Windows is not optimal (Breeze dark is better)
	// we can't find out if light or dark mode is used, so we don't switch to Breeze/Breeze dark here
	DEBUG(Q_FUNC_INFO << ", Color scheme = " << STDSTRING(schemeName))
#if KCONFIGWIDGETS_VERSION >= QT_VERSION_CHECK(5, 107, 0) // use KColorSchemeMenu::createMenu and set the text and check an action manually
	auto* schemesMenu = KColorSchemeMenu::createMenu(m_schemeManager, this);
	schemesMenu->setText(i18n("Color Scheme"));
#else
	auto* schemesMenu = m_schemeManager->createSchemeSelectionMenu(i18n("Color Scheme"), schemeName, this);
#endif
	schemesMenu->setIcon(QIcon::fromTheme(QStringLiteral("preferences-desktop-color")));
	connect(schemesMenu->menu(), &QMenu::triggered, this, &MainWin::colorSchemeChanged);

	QMenu* settingsMenu = dynamic_cast<QMenu*>(factory()->container(QLatin1String("settings"), this));
	if (settingsMenu) {
		auto* action = settingsMenu->insertSeparator(settingsMenu->actions().constFirst());
		settingsMenu->insertMenu(action, schemesMenu->menu());

		// add m_memoryInfoAction after the "Show status bar" action
		auto actions = settingsMenu->actions();
		const int index = actions.indexOf(m_statusBarAction);
		settingsMenu->insertAction(actions.at(index + 1), m_memoryInfoAction);
	}

	// Cantor backends to menu and context menu
#ifdef HAVE_CANTOR_LIBS
	auto backendNames = Cantor::Backend::listAvailableBackends();
#if !defined(NDEBUG) || defined(Q_OS_WIN) || defined(Q_OS_MACOS)
	WARN(Q_FUNC_INFO << ", " << backendNames.count() << " Cantor backends available:")
	for (const auto& b : backendNames)
		WARN("Backend: " << STDSTRING(b))
#endif

	if (!backendNames.isEmpty()) {
		// sub-menu shown in the main toolbar
		m_newNotebookMenu = new QMenu(this);

		// sub-menu shown in the main menu bar
		auto* menu = dynamic_cast<QMenu*>(factory()->container(QLatin1String("new_notebook"), this));
		if (menu) {
			menu->setIcon(QIcon::fromTheme(QLatin1String("cantor")));
			m_newMenu->addSeparator();
			m_newMenu->addMenu(menu);
			updateNotebookActions();
		}

		if (settingsMenu)
			settingsMenu->addAction(m_configureCASAction);
	}
#else
	delete this->guiFactory()->container(QStringLiteral("notebook"), this);
	delete this->guiFactory()->container(QStringLiteral("new_notebook"), this);
	delete this->guiFactory()->container(QStringLiteral("notebook_toolbar"), this);
#endif
}

void MainWin::colorSchemeChanged(QAction* action) {
	// save the selected color scheme
	auto group = Settings::group(QStringLiteral("Settings_General"));
	const auto& schemeName = KLocalizedString::removeAcceleratorMarker(action->text());
	group.writeEntry(QStringLiteral("ColorScheme"), schemeName);
	group.sync();
}

/*!
	Asks to save the project if it was modified.
	\return \c true if the project still needs to be saved ("cancel" clicked), \c false otherwise.
 */
bool MainWin::warnModified() {
	if (m_project->hasChanged()) {
#if KCONFIGWIDGETS_VERSION >= QT_VERSION_CHECK(5, 100, 0)
		int option = KMessageBox::warningTwoActionsCancel(this,
														  i18n("The current project %1 has been modified. Do you want to save it?", m_project->name()),
														  i18n("Save Project"),
														  KStandardGuiItem::save(),
														  KStandardGuiItem::dontSave());
		switch (option) {
		case KMessageBox::PrimaryAction:
			return !saveProject();
		case KMessageBox::SecondaryAction:
			break;
		case KMessageBox::Cancel:
			return true;
		}
#else
		int option = KMessageBox::warningYesNoCancel(this,
													 i18n("The current project %1 has been modified. Do you want to save it?", m_project->name()),
													 i18n("Save Project"));
		switch (option) {
		case KMessageBox::Yes:
			return !saveProject();
		case KMessageBox::No:
			break;
		case KMessageBox::Cancel:
			return true;
		}
#endif
	}

	return false;
}

/*!
 * updates the state of actions, menus and toolbars (enabled or disabled)
 * on project changes (project closes and opens)
 */
void MainWin::updateGUIOnProjectChanges() {
	// return;
	if (m_closing)
		return;

	auto* factory = this->guiFactory();
	if (!m_dockManagerContent || !m_dockManagerContent->focusedDockWidget()) {
		factory->container(QLatin1String("spreadsheet"), this)->setEnabled(false);
		factory->container(QLatin1String("matrix"), this)->setEnabled(false);
		factory->container(QLatin1String("worksheet"), this)->setEnabled(false);
		factory->container(QLatin1String("datapicker"), this)->setEnabled(false);
		factory->container(QLatin1String("spreadsheet_toolbar"), this)->hide();
		factory->container(QLatin1String("worksheet_toolbar"), this)->hide();
		factory->container(QLatin1String("cartesian_plot_toolbar"), this)->hide();
		factory->container(QLatin1String("datapicker_toolbar"), this)->hide();
#ifdef HAVE_CANTOR_LIBS
		factory->container(QLatin1String("notebook"), this)->setEnabled(false);
		factory->container(QLatin1String("notebook_toolbar"), this)->hide();
#endif
	}

	updateTitleBar();

	// undo/redo actions are disabled in both cases - when the project is closed or opened
	m_undoAction->setEnabled(false);
	m_redoAction->setEnabled(false);
}

/*
 * updates the state of actions, menus and toolbars (enabled or disabled)
 * depending on the currently active window (worksheet or spreadsheet).
 */
void MainWin::updateGUI() {
	if (!m_project || m_project->isLoading())
		return;

	if (m_closing || m_projectClosing)
		return;

#ifdef HAVE_TOUCHBAR
	// reset the touchbar
	m_touchBar->clear();
	m_touchBar->addAction(m_undoIconOnlyAction);
	m_touchBar->addAction(m_redoIconOnlyAction);
	m_touchBar->addSeparator();
#endif

	auto* factory = this->guiFactory();
	if (!m_dockManagerContent || !m_dockManagerContent->focusedDockWidget()) {
		factory->container(QLatin1String("spreadsheet"), this)->setEnabled(false);
		factory->container(QLatin1String("matrix"), this)->setEnabled(false);
		factory->container(QLatin1String("worksheet"), this)->setEnabled(false);
		factory->container(QLatin1String("datapicker"), this)->setEnabled(false);
		factory->container(QLatin1String("spreadsheet_toolbar"), this)->hide();
		factory->container(QLatin1String("worksheet_toolbar"), this)->hide();
		factory->container(QLatin1String("cartesian_plot_toolbar"), this)->hide();
		factory->container(QLatin1String("datapicker_toolbar"), this)->hide();
#ifdef HAVE_CANTOR_LIBS
		factory->container(QLatin1String("notebook"), this)->setEnabled(false);
		factory->container(QLatin1String("notebook_toolbar"), this)->hide();
#endif
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

	Q_ASSERT(m_currentAspect);
	// Handle the Worksheet-object
	const auto* w = dynamic_cast<Worksheet*>(m_currentAspect);
	if (!w)
		w = dynamic_cast<Worksheet*>(m_currentAspect->parent(AspectType::Worksheet));

	if (w) {
		bool update = (w != m_lastWorksheet);
		m_lastWorksheet = w;

		// populate worksheet menu
		auto* view = qobject_cast<WorksheetView*>(w->view());
		auto* menu = qobject_cast<QMenu*>(factory->container(QLatin1String("worksheet"), this));
		if (update) {
			menu->clear();
			view->createContextMenu(menu);
		}
		menu->setEnabled(true);

		// populate worksheet-toolbar
		auto* toolbar = qobject_cast<QToolBar*>(factory->container(QLatin1String("worksheet_toolbar"), this));
		if (update) {
			toolbar->clear();
			view->fillToolBar(toolbar);
		}
		toolbar->setVisible(true);
		toolbar->setEnabled(true);

		// populate the toolbar for cartesian plots
		toolbar = qobject_cast<QToolBar*>(factory->container(QLatin1String("cartesian_plot_toolbar"), this));
		if (update) {
			toolbar->clear();
			view->fillCartesianPlotToolBar(toolbar);
		}
		toolbar->setVisible(true);
		toolbar->setEnabled(true);

		// populate the touchbar on Mac
#ifdef HAVE_TOUCHBAR
		view->fillTouchBar(m_touchBar);
#endif
		// hide the spreadsheet toolbar
		factory->container(QLatin1String("spreadsheet_toolbar"), this)->setVisible(false);
	} else {
		factory->container(QLatin1String("worksheet"), this)->setEnabled(false);
		factory->container(QLatin1String("worksheet_toolbar"), this)->setVisible(false);
		//		factory->container(QLatin1String("drawing"), this)->setEnabled(false);
		factory->container(QLatin1String("worksheet_toolbar"), this)->setEnabled(false);
		factory->container(QLatin1String("cartesian_plot_toolbar"), this)->setEnabled(false);
	}

	// Handle the Spreadsheet-object
	const auto* spreadsheet = this->activeSpreadsheet();
	if (!spreadsheet)
		spreadsheet = dynamic_cast<Spreadsheet*>(m_currentAspect->parent(AspectType::Spreadsheet));
	if (spreadsheet) {
		bool update = (spreadsheet != m_lastSpreadsheet);
		m_lastSpreadsheet = spreadsheet;

		// populate spreadsheet-menu
		auto* view = qobject_cast<SpreadsheetView*>(spreadsheet->view());
		auto* menu = qobject_cast<QMenu*>(factory->container(QLatin1String("spreadsheet"), this));
		if (update) {
			menu->clear();
			view->createContextMenu(menu);
		}
		menu->setEnabled(true);

		// populate spreadsheet-toolbar
		auto* toolbar = qobject_cast<QToolBar*>(factory->container(QLatin1String("spreadsheet_toolbar"), this));
		if (update) {
			toolbar->clear();
			view->fillToolBar(toolbar);
		}
		toolbar->setVisible(true);
		toolbar->setEnabled(true);

		// populate the touchbar on Mac
#ifdef HAVE_TOUCHBAR
		m_touchBar->addAction(m_importFileAction);
		view->fillTouchBar(m_touchBar);
#endif

		// spreadsheet has it's own search, unregister the shortcut for the global search here
		m_searchAction->setShortcut(QKeySequence());
	} else {
		factory->container(QLatin1String("spreadsheet"), this)->setEnabled(false);
		factory->container(QLatin1String("spreadsheet_toolbar"), this)->setVisible(false);
		m_searchAction->setShortcut(QKeySequence::Find);
	}

	// Handle the Matrix-object
	const auto* matrix = dynamic_cast<Matrix*>(m_currentAspect);
	if (!matrix)
		matrix = dynamic_cast<Matrix*>(m_currentAspect->parent(AspectType::Matrix));
	if (matrix) {
		// populate matrix-menu
		auto* view = qobject_cast<MatrixView*>(matrix->view());
		auto* menu = qobject_cast<QMenu*>(factory->container(QLatin1String("matrix"), this));
		menu->clear();
		view->createContextMenu(menu);
		menu->setEnabled(true);

		// populate the touchbar on Mac
#ifdef HAVE_TOUCHBAR
		m_touchBar->addAction(m_importFileAction);
		// view->fillTouchBar(m_touchBar);
#endif
	} else
		factory->container(QLatin1String("matrix"), this)->setEnabled(false);

#ifdef HAVE_CANTOR_LIBS
	const auto* cantorworksheet = dynamic_cast<CantorWorksheet*>(m_currentAspect);
	if (!cantorworksheet)
		cantorworksheet = dynamic_cast<CantorWorksheet*>(m_currentAspect->parent(AspectType::CantorWorksheet));
	if (cantorworksheet) {
		auto* view = qobject_cast<CantorWorksheetView*>(cantorworksheet->view());
		auto* menu = qobject_cast<QMenu*>(factory->container(QLatin1String("notebook"), this));
		menu->clear();
		view->createContextMenu(menu);
		menu->setEnabled(true);

		auto* toolbar = qobject_cast<QToolBar*>(factory->container(QLatin1String("notebook_toolbar"), this));
		toolbar->setVisible(true);
		toolbar->clear();
		view->fillToolBar(toolbar);
	} else {
		// no Cantor worksheet selected -> deactivate Cantor worksheet related menu and toolbar
		factory->container(QLatin1String("notebook"), this)->setEnabled(false);
		factory->container(QLatin1String("notebook_toolbar"), this)->setVisible(false);
	}
#endif

	const auto* datapicker = dynamic_cast<Datapicker*>(m_currentAspect);
	if (!datapicker)
		datapicker = dynamic_cast<Datapicker*>(m_currentAspect->parent(AspectType::Datapicker));
	if (!datapicker) {
		if (m_currentAspect && m_currentAspect->type() == AspectType::DatapickerCurve)
			datapicker = dynamic_cast<Datapicker*>(m_currentAspect->parentAspect());
	}

	if (datapicker) {
		// populate datapicker-menu
		auto* view = qobject_cast<DatapickerView*>(datapicker->view());
		auto* menu = qobject_cast<QMenu*>(factory->container(QLatin1String("datapicker"), this));
		menu->clear();
		view->createContextMenu(menu);
		menu->setEnabled(true);

		// populate spreadsheet-toolbar
		auto* toolbar = qobject_cast<QToolBar*>(factory->container(QLatin1String("datapicker_toolbar"), this));
		toolbar->clear();
		view->fillToolBar(toolbar);
		toolbar->setVisible(true);
	} else {
		factory->container(QLatin1String("datapicker"), this)->setEnabled(false);
		factory->container(QLatin1String("datapicker_toolbar"), this)->setVisible(false);
	}
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

	m_project = new Project();
	undoStackIndexLastSave = 0;
	m_currentAspect = m_project;
	m_currentFolder = m_project;

	KConfigGroup group = Settings::group(QStringLiteral("Settings_General"));
	auto vis = Project::DockVisibility(group.readEntry("DockVisibility", 0));
	m_project->setDockVisibility(vis);
	if (vis == Project::DockVisibility::folderOnly)
		m_visibilityFolderAction->setChecked(true);
	else if (vis == Project::DockVisibility::folderAndSubfolders)
		m_visibilitySubfolderAction->setChecked(true);
	else
		m_visibilityAllAction->setChecked(true);

	m_aspectTreeModel = new AspectTreeModel(m_project, this);
	connect(m_aspectTreeModel, &AspectTreeModel::statusInfo, [=](const QString& text) {
		statusBar()->showMessage(text);
	});

	m_newProjectAction->setEnabled(false);
#ifdef HAVE_PURPOSE
	m_shareAction->setEnabled(false); // sharing is only possible after the project was saved to a file
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
				addAspectToProject(new CantorWorksheet(backend));
#endif
			break;
		}
		}

		m_project->setChanged(false); // the project was initialized on startup, nothing has changed from user's perspective

		updateGUIOnProjectChanges();
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

	connect(m_closeWindowAction, &QAction::triggered, [this] {
		m_dockManagerContent->removeDockWidget(m_currentDock);
	});
	connect(m_closeAllWindowsAction, &QAction::triggered, [this]() {
		for (auto dock : m_dockManagerContent->dockWidgetsMap())
			m_dockManagerContent->removeDockWidget(dock);
	});

	connect(m_nextWindowAction, &QAction::triggered, this, &MainWin::activateNextDock);
	connect(m_prevWindowAction, &QAction::triggered, this, &MainWin::activatePreviousDock);

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
		m_projectExplorerDockAction->setChecked(true);
		m_propertiesDockAction->setChecked(true);
		m_worksheetPreviewAction->setChecked(false);
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

	this->openProject(path);

	// save new "last open directory"
	int pos = path.lastIndexOf(QLatin1String("/"));
	if (pos != -1) {
		const QString& newDir = path.left(pos);
		if (newDir != dir)
			group.writeEntry("LastOpenDir", newDir);
	}
}

void MainWin::openProject(const QString& filename) {
	if (m_project && filename == m_project->fileName()) {
		KMessageBox::information(this, i18n("The project file %1 is already opened.", filename), i18n("Open Project"));
		return;
	}

	// check whether the file can be opened for reading at all before closing the current project
	// and creating a new project and trying to load
	QFile file(filename);
	if (!file.exists()) {
		KMessageBox::error(this, i18n("The project file %1 doesn't exist.", filename), i18n("Open Project"));
		return;
	}

	if (!file.open(QIODevice::ReadOnly)) {
		KMessageBox::error(this, i18n("Couldn't read the project file %1.", filename), i18n("Open Project"));
		return;
	} else
		file.close();

	if (!newProject(false))
		return;

	WAIT_CURSOR;
	statusBar()->showMessage(i18n("Loading %1...", filename));
	QApplication::processEvents(QEventLoop::AllEvents, 0);
	m_project->setFileName(filename);
	QElapsedTimer timer;
	timer.start();
	bool rc = false;
	if (Project::isLabPlotProject(filename)) {
		rc = m_project->load(filename);
	}
#ifdef HAVE_LIBORIGIN
	else if (OriginProjectParser::isOriginProject(filename)) {
		OriginProjectParser parser;
		parser.setProjectFileName(filename);
		parser.importTo(m_project, QStringList()); // TODO: add return code
		rc = true;
	}
#endif

#ifdef HAVE_CANTOR_LIBS
	else if (filename.endsWith(QLatin1String(".cws"), Qt::CaseInsensitive) || filename.endsWith(QLatin1String(".ipynb"), Qt::CaseInsensitive)) {
		rc = m_project->loadNotebook(filename);
	}
#endif

	m_project->setChanged(false);

	if (!rc) {
		closeProject();
		RESET_CURSOR;
		return;
	}

	m_project->undoStack()->clear();
	m_undoViewEmptyLabel = i18n("%1: opened", m_project->name());
	m_recentProjectsAction->addUrl(QUrl(filename));

	updateGUIOnProjectChanges();
	updateGUI(); // there are most probably worksheets or spreadsheets in the open project -> update the GUI

	const auto& dockWidgetsState = m_project->dockWidgetState().toUtf8();
	if (!dockWidgetsState.isEmpty()) {
		for (auto dock : m_dockManagerContent->dockWidgetsMap()) {
			auto* d = dynamic_cast<ContentDockWidget*>(dock);
			if (d)
				d->part()->suppressDeletion(true);
		}
		changeVisibleAllDocks(false);
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

	m_saveAction->setEnabled(false);
	m_newProjectAction->setEnabled(true);
#ifdef HAVE_PURPOSE
	m_shareAction->setEnabled(true);
	fillShareMenu();
#endif
	statusBar()->showMessage(i18n("Project successfully opened (in %1 seconds).", (float)timer.elapsed() / 1000));

	KConfigGroup group = Settings::group(QStringLiteral("MainWin"));
	group.writeEntry("LastOpenProject", filename);

	if (m_autoSaveActive)
		m_autoSaveTimer.start();

	RESET_CURSOR;
}

void MainWin::openRecentProject(const QUrl& url) {
	if (url.isLocalFile()) // fix for Windows
		this->openProject(url.toLocalFile());
	else
		this->openProject(url.path());
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
	m_projectClosing = false;

	// update the UI if we're just closing a project
	// and not closing(quitting) the application
	if (!m_closing) {
		m_currentAspect = nullptr;
		m_currentFolder = nullptr;
		updateGUIOnProjectChanges();
		m_newProjectAction->setEnabled(true);

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
		// don't overwrite OPJ files
		if (fileName.endsWith(QLatin1String(".opj"), Qt::CaseInsensitive))
			fileName.replace(QLatin1String(".opj"), QLatin1String(".lml"));
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
	// The "Automatically select filename extension (.lml)" option does not change anything

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
	QTemporaryFile tempFile(QDir::tempPath() + QLatin1Char('/') + QLatin1String("labplot_save_XXXXXX"));
	if (!tempFile.open()) {
		KMessageBox::error(this, i18n("Couldn't open the temporary file for writing."));
		return false;
	}

	WAIT_CURSOR;
	const QString& tempFileName = tempFile.fileName();
	DEBUG("Using temporary file " << STDSTRING(tempFileName))
	tempFile.close();

	QIODevice* file;
	// if file ending is .lml, do xz compression or gzip compression in compatibility mode
	const KConfigGroup group = Settings::group(QStringLiteral("Settings_General"));
	if (fileName.endsWith(QLatin1String(".lml"))) {
		if (group.readEntry("CompatibleSave", false))
			file = new KCompressionDevice(tempFileName, KCompressionDevice::GZip);
		else
			file = new KCompressionDevice(tempFileName, KCompressionDevice::Xz);
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
			m_saveAction->setEnabled(false);
			m_recentProjectsAction->addUrl(QUrl(fileName));
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
			RESET_CURSOR;
			KMessageBox::error(this, i18n("Couldn't save the file '%1'.", fileName));
			ok = false;
		}
	} else {
		RESET_CURSOR;
		KMessageBox::error(this, i18n("Couldn't open the file '%1' for writing.", fileName));
		ok = false;
	}

	delete file;

#ifdef HAVE_PURPOSE
	m_shareAction->setEnabled(true); // sharing is possible after the project was saved to a file
	fillShareMenu();
#endif

	RESET_CURSOR;
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
	auto* workbook = dynamic_cast<Workbook*>(m_currentAspect);
	if (!workbook)
		workbook = static_cast<Workbook*>(m_currentAspect->parent(AspectType::Workbook));

	if (workbook)
		workbook->addChild(spreadsheet);
	else
		this->addAspectToProject(spreadsheet);
}

/*!
	adds a new Matrix to the project.
*/
void MainWin::newMatrix() {
	Matrix* matrix = new Matrix(i18n("Matrix"));

	// if the current active window is a workbook or one of its children,
	// add the new matrix to the workbook
	auto* workbook = dynamic_cast<Workbook*>(m_currentAspect);
	if (!workbook)
		workbook = static_cast<Workbook*>(m_currentAspect->parent(AspectType::Workbook));

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
	if (m_currentAspect->type() == AspectType::Spreadsheet)
		spreadsheet = dynamic_cast<Spreadsheet*>(m_currentAspect);
	else {
		// check whether one of spreadsheet columns is selected and determine the spreadsheet
		auto* parent = m_currentAspect->parentAspect();
		if (parent && parent->type() == AspectType::Spreadsheet)
			spreadsheet = dynamic_cast<Spreadsheet*>(parent);
	}

	return spreadsheet;
}

#ifdef HAVE_CANTOR_LIBS
/*
	adds a new Cantor Spreadsheet to the project.
*/
void MainWin::newNotebook() {
	auto* action = static_cast<QAction*>(QObject::sender());
	auto* cantorworksheet = new CantorWorksheet(action->data().toString());
	this->addAspectToProject(cantorworksheet);
}

/********************************************************************************/
#endif

/*!
	called if there were changes in the project.
	Adds "changed" to the window caption and activates the save-Action.
*/
void MainWin::projectChanged() {
	updateTitleBar();
	m_newProjectAction->setEnabled(true);
	m_saveAction->setEnabled(true);
	m_undoAction->setEnabled(true);
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
	if (!aspect->inherits(AspectType::AbstractFilter) && !(parent->parentAspect() && parent->parentAspect()->type() == AspectType::DatapickerCurve))
		m_projectExplorer->setCurrentAspect(parent);
}

void MainWin::handleAspectAboutToBeRemoved(const AbstractAspect* aspect) {
	const auto* part = dynamic_cast<const AbstractPart*>(aspect);
	if (!part)
		return;

	const auto* workbook = dynamic_cast<const Workbook*>(aspect->parentAspect());
	auto* datapicker = dynamic_cast<const Datapicker*>(aspect->parentAspect());
	if (!datapicker)
		datapicker = dynamic_cast<const Datapicker*>(aspect->parentAspect()->parentAspect());

	if (!workbook && !datapicker && part->dockWidgetExists()) {
		ContentDockWidget* win = part->dockWidget();
		if (win)
			m_dockManagerContent->removeDockWidget(win);
	}
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

	updateGUI();
}

void MainWin::activateSubWindowForAspect(const AbstractAspect* aspect) {
	Q_ASSERT(aspect);
	Q_ASSERT(m_dockManagerContent);
	const auto* part = dynamic_cast<const AbstractPart*>(aspect);
	if (part) {
		ContentDockWidget* win{nullptr};

		// for aspects being children of a Workbook, we show workbook's window, otherwise the window of the selected part
		const auto* workbook = dynamic_cast<const Workbook*>(aspect->parentAspect());
		auto* datapicker = dynamic_cast<const Datapicker*>(aspect->parentAspect());
		if (!datapicker)
			datapicker = dynamic_cast<const Datapicker*>(aspect->parentAspect()->parentAspect());

		if (workbook)
			win = workbook->dockWidget();
		else if (datapicker)
			win = datapicker->dockWidget();
		else
			win = part->dockWidget();

		auto* dock = m_dockManagerContent->findDockWidget(win->objectName());
		if (dock == nullptr) {
			// Add new dock if not found
			ads::CDockAreaWidget* areaWidget{nullptr};
			if (m_dockManagerContent->dockWidgetsMap().isEmpty() || !m_currentAspectDock) {
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
		m_dockManagerContent->setDockWidgetFocused(win);
	} else {
		// activate the mdiView of the parent, if a child was selected
		const AbstractAspect* parent = aspect->parentAspect();
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

	menu->insertMenu(firstAction, m_newMenu);

	menu->insertSeparator(firstAction);
	menu->insertMenu(firstAction, m_visibilityMenu);
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
	WAIT_CURSOR;
	m_project->undoStack()->undo();
	m_redoAction->setEnabled(true);

	const int index = m_project->undoStack()->index();
	if (index == 0)
		m_undoAction->setEnabled(false);

	const bool changed = (index != undoStackIndexLastSave);
	m_saveAction->setEnabled(changed);
	m_project->setChanged(changed);
	updateTitleBar();
	RESET_CURSOR;
}

void MainWin::redo() {
	WAIT_CURSOR;
	m_project->undoStack()->redo();
	m_undoAction->setEnabled(true);

	const int index = m_project->undoStack()->index();
	if (index == m_project->undoStack()->count())
		m_redoAction->setEnabled(false);

	const bool changed = (index != undoStackIndexLastSave);
	m_saveAction->setEnabled(changed);
	m_project->setChanged(changed);
	updateTitleBar();
	RESET_CURSOR;
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

void MainWin::toggleDockWidget(QAction* action) {
	if (action->objectName() == QLatin1String("toggle_project_explorer_dock")) {
		if (m_projectExplorerDock->isVisible())
			m_projectExplorerDock->toggleView(false);
		// 			toggleHideWidget(m_projectExplorerDock, true);
		else
			m_projectExplorerDock->toggleView(true);
		// 			toggleShowWidget(m_projectExplorerDock, true);
	} else if (action->objectName() == QLatin1String("toggle_properties_explorer_dock")) {
		if (m_propertiesDock->isVisible())
			m_propertiesDock->toggleView(false);
		// 			toggleHideWidget(m_propertiesDock, false);
		else
			m_propertiesDock->toggleView(true);
		// 			toggleShowWidget(m_propertiesDock, false);
	} else if (action->objectName() == QLatin1String("toggle_worksheet_preview_dock"))
		m_worksheetPreviewDock->toggleView(!m_worksheetPreviewDock->isVisible());
}

void MainWin::toggleStatusBar(bool checked) {
	statusBar()->setVisible(checked); // show/hide statusbar
	statusBar()->setEnabled(checked);
	// enabled/disable memory info menu with statusbar
	m_memoryInfoAction->setEnabled(checked);
}

void MainWin::toggleMemoryInfo() {
	if (m_memoryInfoWidget) {
		statusBar()->removeWidget(m_memoryInfoWidget);
		delete m_memoryInfoWidget;
		m_memoryInfoWidget = nullptr;
	} else {
		m_memoryInfoWidget = new MemoryWidget(statusBar());
		statusBar()->addPermanentWidget(m_memoryInfoWidget);
	}
}

void MainWin::toggleMenuBar(bool checked) {
	menuBar()->setVisible(checked);
}

void MainWin::propertiesExplorerRequested() {
	if (!m_propertiesDock->isVisible())
		m_propertiesDock->toggleView(true);
}

/*
void MainWin::toggleHideWidget(QWidget* widget, bool hideToLeft)
{
	auto* timeline = new QTimeLine(800, this);
	timeline->setEasingCurve(QEasingCurve::InOutQuad);

	connect(timeline, &QTimeLine::valueChanged, [=] {
		const qreal value = timeline->currentValue();
		const int widgetWidth = widget->width();
		const int widgetPosY = widget->pos().y();

		int moveX = 0;
		if (hideToLeft) {
			moveX = static_cast<int>(value * widgetWidth) - widgetWidth;
		}
		else {
			const int frameRight = this->frameGeometry().right();
			moveX = frameRight - static_cast<int>(value * widgetWidth);
		}
		widget->move(moveX, widgetPosY);
	});
	timeline->setDirection(QTimeLine::Backward);
	timeline->start();

	connect(timeline, &QTimeLine::finished, [widget] {widget->hide();});
	connect(timeline, &QTimeLine::finished, timeline, &QTimeLine::deleteLater);
}

void MainWin::toggleShowWidget(QWidget* widget, bool showToRight)
{
	auto* timeline = new QTimeLine(800, this);
	timeline->setEasingCurve(QEasingCurve::InOutQuad);

	connect(timeline, &QTimeLine::valueChanged, [=]() {
		if (widget->isHidden()) {
			widget->show();
		}
		const qreal value = timeline->currentValue();
		const int widgetWidth = widget->width();
		const int widgetPosY = widget->pos().y();
		int moveX = 0;

		if (showToRight) {
			moveX = static_cast<int>(value * widgetWidth) - widgetWidth;
		}
		else {
			const int frameRight = this->frameGeometry().right();
			moveX = frameRight - static_cast<int>(value * widgetWidth);
		}
		widget->move(moveX, widgetPosY);
	});

	timeline->setDirection(QTimeLine::Forward);
	timeline->start();

	connect(timeline, &QTimeLine::finished, timeline, &QTimeLine::deleteLater);
}
*/
void MainWin::projectExplorerDockVisibilityChanged(bool visible) {
	m_projectExplorerDockAction->setChecked(visible);
}

void MainWin::propertiesDockVisibilityChanged(bool visible) {
	m_propertiesDockAction->setChecked(visible);
}

void MainWin::worksheetPreviewDockVisibilityChanged(bool visible) {
	m_worksheetPreviewAction->setChecked(visible);
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

#ifdef HAVE_PURPOSE
void MainWin::fillShareMenu() {
	if (!m_shareMenu)
		return;

	m_shareMenu->clear(); // clear the menu, it will be refilled with the new file URL below
	QMimeType mime;
	m_shareMenu->model()->setInputData(
		QJsonObject{{QStringLiteral("mimeType"), mime.name()}, {QStringLiteral("urls"), QJsonArray{QUrl::fromLocalFile(m_project->fileName()).toString()}}});
	m_shareMenu->reload();
}

void MainWin::shareActionFinished(const QJsonObject& output, int error, const QString& message) {
	if (error)
		KMessageBox::error(this, i18n("There was a problem sharing the project: %1", message), i18n("Share"));
	else {
		const QString url = output[QStringLiteral("url")].toString();
		if (url.isEmpty())
			statusBar()->showMessage(i18n("Project shared successfully"));
		else
			KMessageBox::information(widget(),
									 i18n("You can find the shared project at: <a href=\"%1\">%1</a>", url),
									 i18n("Share"),
									 QString(),
									 KMessageBox::Notify | KMessageBox::AllowLink);
	}
}
#endif

void MainWin::toggleFullScreen(bool t) {
	m_fullScreenAction->setFullScreen(this, t);
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

void MainWin::updateLocale() {
	// Set default locale
	auto numberLocaleLanguage =
		static_cast<QLocale::Language>(Settings::group(QStringLiteral("Settings_General"))
										   .readEntry(QLatin1String("DecimalSeparatorLocale"), static_cast<int>(QLocale::Language::AnyLanguage)));

	auto numberOptions = static_cast<QLocale::NumberOptions>(
		Settings::group(QStringLiteral("Settings_General")).readEntry(QLatin1String("NumberOptions"), static_cast<int>(QLocale::DefaultNumberOptions)));

	QLocale l(numberLocaleLanguage == QLocale::AnyLanguage ? QLocale() : numberLocaleLanguage);
	l.setNumberOptions(numberOptions);
	QLocale::setDefault(l);
}

void MainWin::handleSettingsChanges() {
	const auto group = Settings::group(QStringLiteral("Settings_General"));

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
			m_visibilityFolderAction->setChecked(true);
		else if (vis == Project::DockVisibility::folderAndSubfolders)
			m_visibilitySubfolderAction->setChecked(true);
		else
			m_visibilityAllAction->setChecked(true);
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

	// update the locale and the units in the dock widgets
	updateLocale();
	if (stackedWidget) {
		for (int i = 0; i < stackedWidget->count(); ++i) {
			auto* widget = stackedWidget->widget(i);
			auto* dock = dynamic_cast<BaseDock*>(widget);
			if (dock) {
				dock->updateLocale();
				dock->updateUnits();
			} else {
				auto* labelWidget = dynamic_cast<LabelWidget*>(widget);
				if (labelWidget)
					labelWidget->updateUnits();
			}
		}
	}

	// update spreadsheet header
	if (m_project) {
		const auto& spreadsheets = m_project->children<Spreadsheet>(AbstractAspect::ChildIndexFlag::Recursive);
		for (auto* spreadsheet : spreadsheets) {
			spreadsheet->updateHorizontalHeader();
			spreadsheet->updateLocale();
		}
	}

	// bool showWelcomeScreen = group.readEntry<bool>(QLatin1String("ShowWelcomeScreen"), true);
	// if (m_showWelcomeScreen != showWelcomeScreen)
	// 	m_showWelcomeScreen = showWelcomeScreen;
}

void MainWin::openDatasetExample() {
	newProject();
	// 	addAspectToProject(m_welcomeScreenHelper->releaseConfiguredSpreadsheet());
}

/***************************************************************************************/
/************************************** dialogs ***************************************/
/***************************************************************************************/
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
		m_undoAction->setEnabled(false);
		m_redoAction->setEnabled(false);
	}
}

/*!
  Opens the dialog to import data to the selected workbook, spreadsheet or matrix
*/
void MainWin::importFileDialog(const QString& fileName) {
	DEBUG(Q_FUNC_INFO);
	auto* dlg = new ImportFileDialog(this, false, fileName);

	// select existing container
	if (m_currentAspect->type() == AspectType::Spreadsheet || m_currentAspect->type() == AspectType::Matrix || m_currentAspect->type() == AspectType::Workbook)
		dlg->setCurrentIndex(m_projectExplorer->currentIndex());
	else if (m_currentAspect->type() == AspectType::Column && m_currentAspect->parentAspect()->type() == AspectType::Spreadsheet)
		dlg->setCurrentIndex(m_aspectTreeModel->modelIndexOfAspect(m_currentAspect->parentAspect()));

	dlg->exec();
	DEBUG(Q_FUNC_INFO << " DONE");
}

void MainWin::importKaggleDatasetDialog() {
	DEBUG(Q_FUNC_INFO);

	if (ImportKaggleDatasetDialog::checkKaggle()) {
		auto* dlg = new ImportKaggleDatasetDialog(this);
		dlg->exec();
	} else
		QMessageBox::critical(
			this,
			i18n("Running Kaggle CLI tool failed"),
			i18n("Please follow the instructions on "
				 "<a href=\"https://www.kaggle.com/docs/api\">\"How to Use Kaggle\"</a> "
				 "to setup the Kaggle CLI tool."));

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
	if (QObject::sender() == m_importOpjAction)
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
	const QModelIndex& index = m_projectExplorer->currentIndex();
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
	auto* dlg = new SettingsDialog(this);
	connect(dlg, &SettingsDialog::settingsChanged, this, &MainWin::handleSettingsChanges);
	// 	connect (dlg, &SettingsDialog::resetWelcomeScreen, this, &MainWin::resetWelcomeScreen);
	dlg->exec();
}

#ifdef HAVE_CANTOR_LIBS
void MainWin::casSettingsDialog() {
	auto* dlg = new CASSettingsDialog(this);
	connect(dlg, &CASSettingsDialog::settingsChanged, this, &MainWin::updateNotebookActions);
	dlg->exec();

	DEBUG(Q_FUNC_INFO << ", found " << Cantor::Backend::availableBackends().size() << " backends")
	if (Cantor::Backend::availableBackends().size() == 0)
		KMessageBox::error(nullptr, i18n("No Cantor backends found. Please install the ones you want to use."));
}

void MainWin::updateNotebookActions() {
	auto* menu = static_cast<QMenu*>(factory()->container(QLatin1String("new_notebook"), this));
	unplugActionList(QLatin1String("backends_list"));
	QList<QAction*> newBackendActions;
	menu->clear();
	for (auto* backend : Cantor::Backend::availableBackends()) {
		if (!backend->isEnabled())
			continue;

		auto* action = new QAction(QIcon::fromTheme(backend->icon()), backend->name(), this);
		action->setData(backend->name());
		action->setWhatsThis(i18n("Creates a new %1 notebook", backend->name()));
		actionCollection()->addAction(QLatin1String("notebook_") + backend->name(), action);
		connect(action, &QAction::triggered, this, &MainWin::newNotebook);
		newBackendActions << action;
		menu->addAction(action);
		m_newNotebookMenu->addAction(action);
	}

	plugActionList(QLatin1String("backends_list"), newBackendActions);

	menu->addSeparator();
	menu->addAction(m_configureCASAction);

	m_newNotebookMenu->addSeparator();
	m_newNotebookMenu->addAction(m_configureCASAction);
}
#endif
