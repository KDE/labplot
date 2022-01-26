/*
    File                 : MainWin.cc
    Project              : LabPlot
    Description          : Main window of the application
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2008-2018 Stefan Gerlach <stefan.gerlach@uni.kn>
    SPDX-FileCopyrightText: 2009-2020 Alexander Semke <alexander.semke@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#include "MainWin.h"

#include "backend/core/Project.h"
#include "backend/core/Folder.h"
#include "backend/core/AspectTreeModel.h"
#include "backend/core/Workbook.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/matrix/Matrix.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/datasources/LiveDataSource.h"
#include "backend/datasources/DatasetHandler.h"
#ifdef HAVE_LIBORIGIN
#include "backend/datasources/projects/OriginProjectParser.h"
#endif
#ifdef HAVE_CANTOR_LIBS
#include "backend/cantorWorksheet/CantorWorksheet.h"
#endif
#include "backend/datapicker/Datapicker.h"
#include "backend/note/Note.h"
#include "backend/lib/macros.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"

#ifdef HAVE_MQTT
#include "backend/datasources/MQTTClient.h"
#endif

#include "commonfrontend/core/PartMdiView.h"
#include "commonfrontend/ProjectExplorer.h"
#include "commonfrontend/matrix/MatrixView.h"
#include "commonfrontend/spreadsheet/SpreadsheetView.h"
#include "commonfrontend/worksheet/WorksheetView.h"
#ifdef HAVE_CANTOR_LIBS
#include "commonfrontend/cantorWorksheet/CantorWorksheetView.h"
#endif
#include "commonfrontend/datapicker/DatapickerView.h"
#include "commonfrontend/datapicker/DatapickerImageView.h"
#include "commonfrontend/note/NoteView.h"
#include "commonfrontend/widgets/MemoryWidget.h"

#include "kdefrontend/examples/ExamplesDialog.h"
#include "kdefrontend/colormaps/ColorMapsDialog.h"
#include "kdefrontend/datasources/ImportFileDialog.h"
#include "kdefrontend/datasources/ImportDatasetDialog.h"
#include "kdefrontend/datasources/ImportDatasetWidget.h"
#include "kdefrontend/datasources/ImportProjectDialog.h"
#include "kdefrontend/datasources/ImportSQLDatabaseDialog.h"
#include "kdefrontend/dockwidgets/CursorDock.h"
#include "kdefrontend/dockwidgets/ProjectDock.h"
#include "kdefrontend/HistoryDialog.h"
#include "kdefrontend/SettingsDialog.h"
#include "kdefrontend/GuiObserver.h"
#include "kdefrontend/widgets/LabelWidget.h"
#include "kdefrontend/widgets/FITSHeaderEditDialog.h"

#ifdef HAVE_KUSERFEEDBACK
#include <KUserFeedback/ApplicationVersionSource>
#include <KUserFeedback/PlatformInfoSource>
#include <KUserFeedback/QtVersionSource>
#include <KUserFeedback/ScreenInfoSource>
#include <KUserFeedback/StartCountSource>
#include <KUserFeedback/UsageTimeSource>
#endif

#ifdef HAVE_TOUCHBAR
#include "3rdparty/kdmactouchbar/src/kdmactouchbar.h"
#endif

#include <QMdiArea>
#include <QMenu>
#include <QMenuBar>
#include <QDockWidget>
#include <QStackedWidget>
#include <QUndoStack>
#include <QCloseEvent>
#include <QFileDialog>
#include <QMimeData>
#include <QElapsedTimer>
#include <QStatusBar>
#include <QTemporaryFile>
#include <QTimeLine>
// #include <QtWidgets>
// #include <QtQuickWidgets/QQuickWidget>
// #include <QQuickItem>
// #include <QQuickView>
// #include <QQmlApplicationEngine>
// #include <QQmlContext>

#include <KActionCollection>
#include <KConfigGroup>
#include <KStandardAction>
#include <kxmlguifactory.h>
#include <KMessageBox>
#include <KToolBar>
#include <KLocalizedString>
#include <KFilterDev>
#include <KRecentFilesAction>
#include <KActionMenu>
#include <KColorScheme>
#include <KColorSchemeManager>
#include <KToggleAction>
#include <KToggleFullScreenAction>
#include <kcoreaddons_version.h>

#ifdef HAVE_CANTOR_LIBS
#include <cantor/backend.h>
#include <KConfigDialog>
#include <KCoreConfigSkeleton>
#include <KConfigSkeleton>

//required to parse Cantor and Jupyter files
#include <QBuffer>
#include <KZip>
#include <QJsonDocument>
#include <QJsonParseError>
#endif

/*!
\class MainWin
\brief Main application window.

\ingroup kdefrontend
*/
MainWin::MainWin(QWidget *parent, const QString& filename)
	: KXmlGuiWindow(parent),
	m_schemeManager(new KColorSchemeManager(this)) {

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
	//save the current settings in MainWin
	m_recentProjectsAction->saveEntries( KSharedConfig::openConfig()->group("Recent Files") );

	KConfigGroup group = KSharedConfig::openConfig()->group("MainWin");
	group.writeEntry(QLatin1String("geometry"), saveGeometry());
	group.writeEntry(QLatin1String("WindowState"), saveState());
	group.writeEntry(QLatin1String("lastOpenFileFilter"), m_lastOpenFileFilter);
	group.writeEntry(QLatin1String("ShowMemoryInfo"), (m_memoryInfoWidget != nullptr));
	KSharedConfig::openConfig()->sync();

	//if welcome screen is shown, save its settings prior to deleting it
// 	if (dynamic_cast<QQuickWidget*>(centralWidget()))
// 		QMetaObject::invokeMethod(m_welcomeWidget->rootObject(), "saveWidgetDimensions");

	if (m_project) {
// 		if (dynamic_cast<QQuickWidget*>(centralWidget()) == nullptr)
// 			m_mdiArea->closeAllSubWindows();

		disconnect(m_project, nullptr, this, nullptr);
		delete m_project;
	}

	delete m_aspectTreeModel;
	delete m_guiObserver;
//	delete m_welcomeScreenHelper;
}

void MainWin::showPresenter() {
	const Worksheet* w = dynamic_cast<Worksheet*>(m_currentAspect);
	if (w) {
		auto* view = static_cast<WorksheetView*>(w->view());
		view->presenterMode();
	} else {
		//currently active object is not a worksheet but we're asked to start in the presenter mode
		//determine the first available worksheet and show it in the presenter mode
		auto worksheets = m_project->children<Worksheet>();
		if (worksheets.size() > 0) {
			auto* view = static_cast<WorksheetView*>(worksheets.constFirst()->view());
			view->presenterMode();
		} else {
			QMessageBox::information(this, i18n("Presenter Mode"),
				i18n("No worksheets are available in the project. The presenter mode will not be started."));
		}
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
	//m_touchBar->setTouchButtonStyle(KDMacTouchBar::IconOnly);
#endif
	setupGUI(Default, QLatin1String("/Applications/labplot2.app/Contents/Resources/labplot2ui.rc"));
	setUnifiedTitleAndToolBarOnMac(true);
#else
	setupGUI(Default, KXMLGUIClient::xmlFile());	// should be "labplot2ui.rc"
#endif

	DEBUG(Q_FUNC_INFO << ", Component name: " << STDSTRING(KXMLGUIClient::componentName()));
	DEBUG(Q_FUNC_INFO << ", XML file: " << STDSTRING(KXMLGUIClient::xmlFile()) << " (should be \"labplot2ui.rc\")");

	//all toolbars created via the KXMLGUI framework are locked on default:
	// * on the very first program start, unlock all toolbars
	// * on later program starts, set stored lock status
	//Furthermore, we want to show icons only after the first program start.
	KConfigGroup groupMain = KSharedConfig::openConfig()->group("MainWindow");
	if (groupMain.exists()) {
		//KXMLGUI framework automatically stores "Disabled" for the key "ToolBarsMovable"
		//in case the toolbars are locked -> load this value
		const QString& str = groupMain.readEntry(QLatin1String("ToolBarsMovable"), "");
		bool locked = (str == QLatin1String("Disabled"));
		KToolBar::setToolBarsLocked(locked);
	} else {
		//first start
		KToolBar::setToolBarsLocked(false);

		//show icons only
		const auto& toolbars = factory()->containers(QLatin1String("ToolBar"));
		for (auto* container : toolbars) {
			auto* toolbar = dynamic_cast<QToolBar*>(container);
			if (toolbar)
				toolbar->setToolButtonStyle(Qt::ToolButtonIconOnly);
		}
	}

	initMenus();

	auto* mainToolBar = qobject_cast<QToolBar*>(factory()->container("main_toolbar", this));
	if (!mainToolBar) {
		QMessageBox::critical(this, i18n("GUI configuration file not found"), i18n("%1 file was not found. Please check your installation.", KXMLGUIClient::xmlFile()));
		//TODO: the application is not really usable if the rc file was not found. We should quit the application. The following line crashes
		//the application because of the splash screen. We need to find another solution.
// 		QMetaObject::invokeMethod(this, "close", Qt::QueuedConnection); //call close as soon as we enter the eventloop
// 		return;
	} else {
		auto* tbImport = new QToolButton(mainToolBar);
		tbImport->setPopupMode(QToolButton::MenuButtonPopup);
		tbImport->setMenu(m_importMenu);
		tbImport->setDefaultAction(m_importFileAction);
		auto* lastAction = mainToolBar->actions().at(mainToolBar->actions().count() - 1);
		mainToolBar->insertWidget(lastAction, tbImport);

		qobject_cast<QMenu*>(factory()->container("import", this))->setIcon(QIcon::fromTheme("document-import"));
	}

	//hamburger menu
#if KCOREADDONS_VERSION >= QT_VERSION_CHECK(5, 81, 0)
	m_hamburgerMenu = KStandardAction::hamburgerMenu(nullptr, nullptr, actionCollection());
	toolBar()->addAction(m_hamburgerMenu);
	m_hamburgerMenu->hideActionsOf(toolBar());
	m_hamburgerMenu->setMenuBar(menuBar());

	QMenu* menu = new QMenu;
	menu->addAction(new QAction("test"));
	m_hamburgerMenu->setMenu(menu);
#endif

	setWindowIcon(QIcon::fromTheme("LabPlot2", QGuiApplication::windowIcon()));
	setAttribute( Qt::WA_DeleteOnClose );

	//make the status bar of a fixed size in order to avoid height changes when placing a ProgressBar there.
	QFont font;
	font.setFamily(font.defaultFamily());
	QFontMetrics fm(font);
	statusBar()->setFixedHeight(fm.height() + 5);

	//load recently used projects
	m_recentProjectsAction->loadEntries( KSharedConfig::openConfig()->group("Recent Files") );

	//General Settings
	const KConfigGroup& group = KSharedConfig::openConfig()->group("Settings_General");

	//title bar
	m_titleBarMode = static_cast<MainWin::TitleBarMode>(group.readEntry("TitleBar", 0));

	//auto-save
	m_autoSaveActive = group.readEntry<bool>("AutoSave", false);
	int interval = group.readEntry("AutoSaveInterval", 1);
	interval = interval*60*1000;
	m_autoSaveTimer.setInterval(interval);
	connect(&m_autoSaveTimer, &QTimer::timeout, this, &MainWin::autoSaveProject);

	if (!fileName.isEmpty()) {
		createMdiArea();
		setCentralWidget(m_mdiArea);
#ifdef HAVE_LIBORIGIN
		if (Project::isLabPlotProject(fileName) || OriginProjectParser::isOriginProject(fileName)) {
#else
		if (Project::isLabPlotProject(fileName)) {
#endif
			QTimer::singleShot(0, this, [=] () { openProject(fileName); });
		} else {
			newProject();
			QTimer::singleShot(0, this, [=] () { importFileDialog(fileName); });
		}
	} else {
		//There is no file to open. Depending on the settings do nothing,
		//create a new project or open the last used project.
		LoadOnStart load = (LoadOnStart)group.readEntry("LoadOnStart", static_cast<int>(LoadOnStart::NewProject));
		if (load != LoadOnStart::WelcomeScreen) {
			createMdiArea();
			setCentralWidget(m_mdiArea);

			switch(load) {
			case LoadOnStart::NewProject:
				newProject();
				break;
			case LoadOnStart::NewProjectWorksheet:
				newProject();
				newWorksheet();
				break;
			case LoadOnStart::NewProjectSpreadsheet:
				newProject();
				newSpreadsheet();
				break;
			case LoadOnStart::LastProject: {
				const QString& path = KSharedConfig::openConfig()->group("MainWin").readEntry("LastOpenProject", "");
				if (!path.isEmpty())
					openProject(path);
				break;
			}
			case LoadOnStart::Nothing:
			case LoadOnStart::WelcomeScreen:
				break;
			}

			updateGUIOnProjectChanges();
		} else { //welcome screen
// 			m_showWelcomeScreen = true;
// 			m_welcomeWidget = createWelcomeScreen();
// 			setCentralWidget(m_welcomeWidget);
		}
	}

	//read the settings of MainWin
	const KConfigGroup& groupMainWin = KSharedConfig::openConfig()->group(QLatin1String("MainWin"));

	//show memory info
	m_toggleMemoryInfoAction->setEnabled(statusBar()->isEnabled());	// disable/enable menu with statusbar
	bool memoryInfoShown = groupMainWin.readEntry(QLatin1String("ShowMemoryInfo"), true);
	DEBUG(Q_FUNC_INFO << ", memory info enabled in config: " << memoryInfoShown)
	m_toggleMemoryInfoAction->setChecked(memoryInfoShown);
	if (memoryInfoShown)
		toggleMemoryInfo();

	//restore the geometry
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

/**
 * @brief Creates a new MDI area, to replace the Welcome Screen as central widget
 */
void MainWin::createMdiArea() {
	KToolBar* toolbar = toolBar();
	if (toolbar)
		toolbar->setVisible(true);

	//Save welcome screen's dimensions.
// 	if (m_showWelcomeScreen)
// 		QMetaObject::invokeMethod(m_welcomeWidget->rootObject(), "saveWidgetDimensions");

	m_mdiArea = new QMdiArea;
	setCentralWidget(m_mdiArea);
	connect(m_mdiArea, &QMdiArea::subWindowActivated, this, &MainWin::handleCurrentSubWindowChanged);

	//set the view mode of the mdi area
	KConfigGroup group = KSharedConfig::openConfig()->group( "Settings_General" );
	int viewMode = group.readEntry("ViewMode", 0);
	if (viewMode == 1) {
		m_mdiArea->setViewMode(QMdiArea::TabbedView);
		int tabPosition = group.readEntry("TabPosition", 0);
		m_mdiArea->setTabPosition(QTabWidget::TabPosition(tabPosition));
		m_mdiArea->setTabsClosable(true);
		m_mdiArea->setTabsMovable(true);
		m_tileWindowsAction->setVisible(false);
		m_cascadeWindowsAction->setVisible(false);
	}

	connect(m_closeWindowAction, &QAction::triggered, m_mdiArea, &QMdiArea::closeActiveSubWindow);
	connect(m_closeAllWindowsAction, &QAction::triggered, m_mdiArea, &QMdiArea::closeAllSubWindows);
	connect(m_tileWindowsAction, &QAction::triggered, m_mdiArea, &QMdiArea::tileSubWindows);
	connect(m_cascadeWindowsAction, &QAction::triggered, m_mdiArea, &QMdiArea::cascadeSubWindows);
	connect(m_nextWindowAction, &QAction::triggered, m_mdiArea, &QMdiArea::activateNextSubWindow);
	connect(m_prevWindowAction, &QAction::triggered, m_mdiArea, &QMdiArea::activatePreviousSubWindow);
}

void MainWin::initActions() {
	// ******************** File-menu *******************************
	//add some standard actions
	m_newProjectAction = KStandardAction::openNew(this, &MainWin::newProject, actionCollection());
	m_openProjectAction = KStandardAction::open(this, static_cast<void (MainWin::*)()>(&MainWin::openProject), actionCollection());
	m_recentProjectsAction = KStandardAction::openRecent(this, &MainWin::openRecentProject, actionCollection());
	m_closeAction = KStandardAction::close(this, &MainWin::closeProject, actionCollection());
	actionCollection()->setDefaultShortcut(m_closeAction, QKeySequence()); //remove the shortcut, QKeySequence::Close will be used for closing sub-windows
	m_saveAction = KStandardAction::save(this, &MainWin::saveProject, actionCollection());
	m_saveAsAction = KStandardAction::saveAs(this, &MainWin::saveProjectAs, actionCollection());
	m_printAction = KStandardAction::print(this, &MainWin::print, actionCollection());
	m_printPreviewAction = KStandardAction::printPreview(this, &MainWin::printPreview, actionCollection());

	QAction* openExample = new QAction(i18n("&Open Example"), actionCollection());
	openExample->setIcon(QIcon::fromTheme(QLatin1String("folder-documents")));
	actionCollection()->addAction(QLatin1String("file_example_open"), openExample);
	connect(openExample, &QAction::triggered, this, [=](){
			auto* dlg = new ExamplesDialog(this);
			if (dlg->exec() == QDialog::Accepted)
				openProject(dlg->path());
			delete dlg;
	});

	m_toggleFullScreenAction = KStandardAction::fullScreen(this, &MainWin::toggleFullScreen, this, actionCollection());

	//QDEBUG(Q_FUNC_INFO << ", preferences action name:" << KStandardAction::name(KStandardAction::Preferences))
	KStandardAction::preferences(this, &MainWin::settingsDialog, actionCollection());
	// QAction* action = actionCollection()->action(KStandardAction::name(KStandardAction::Preferences)));
	KStandardAction::quit(this, &MainWin::close, actionCollection());

	//New Folder/Workbook/Spreadsheet/Matrix/Worksheet/Datasources
	m_newWorkbookAction = new QAction(QIcon::fromTheme("labplot-workbook-new"),i18n("Workbook"),this);
	actionCollection()->addAction("new_workbook", m_newWorkbookAction);
	m_newWorkbookAction->setWhatsThis(i18n("Creates a new workbook for collection spreadsheets, matrices and plots"));
	connect(m_newWorkbookAction, &QAction::triggered, this, &MainWin::newWorkbook);

	m_newDatapickerAction = new QAction(QIcon::fromTheme("color-picker-black"), i18n("Data Extractor"), this);
	m_newDatapickerAction->setWhatsThis(i18n("Creates a data extractor for getting data from a picture"));
	actionCollection()->addAction("new_datapicker", m_newDatapickerAction);
	connect(m_newDatapickerAction, &QAction::triggered, this, &MainWin::newDatapicker);

	m_newSpreadsheetAction = new QAction(QIcon::fromTheme("labplot-spreadsheet-new"),i18n("Spreadsheet"),this);
// 	m_newSpreadsheetAction->setShortcut(Qt::CTRL+Qt::Key_Equal);
	m_newSpreadsheetAction->setWhatsThis(i18n("Creates a new spreadsheet for data editing"));
	actionCollection()->addAction("new_spreadsheet", m_newSpreadsheetAction);
	connect(m_newSpreadsheetAction, &QAction::triggered, this, &MainWin::newSpreadsheet);

	m_newMatrixAction = new QAction(QIcon::fromTheme("labplot-matrix-new"),i18n("Matrix"),this);
// 	m_newMatrixAction->setShortcut(Qt::CTRL+Qt::Key_Equal);
	m_newMatrixAction->setWhatsThis(i18n("Creates a new matrix for data editing"));
	actionCollection()->addAction("new_matrix", m_newMatrixAction);
	connect(m_newMatrixAction, &QAction::triggered, this, &MainWin::newMatrix);

	m_newWorksheetAction = new QAction(QIcon::fromTheme("labplot-worksheet-new"),i18n("Worksheet"),this);
// 	m_newWorksheetAction->setShortcut(Qt::ALT+Qt::Key_X);
	m_newWorksheetAction->setWhatsThis(i18n("Creates a new worksheet for data plotting"));
	actionCollection()->addAction("new_worksheet", m_newWorksheetAction);
	connect(m_newWorksheetAction, &QAction::triggered, this, &MainWin::newWorksheet);

	m_newNotesAction = new QAction(QIcon::fromTheme("document-new"),i18n("Note"),this);
	m_newNotesAction->setWhatsThis(i18n("Creates a new note for arbitrary text"));
	actionCollection()->addAction("new_notes", m_newNotesAction);
	connect(m_newNotesAction, &QAction::triggered, this, &MainWin::newNotes);

// 	m_newScriptAction = new QAction(QIcon::fromTheme("insert-text"),i18n("Note/Script"),this);
// 	actionCollection()->addAction("new_script", m_newScriptAction);
// 	connect(m_newScriptAction, &QAction::triggered,SLOT(newScript()));

	m_newFolderAction = new QAction(QIcon::fromTheme("folder-new"),i18n("Folder"),this);
	m_newFolderAction->setWhatsThis(i18n("Creates a new folder to collect sheets and other elements"));
	actionCollection()->addAction("new_folder", m_newFolderAction);
	connect(m_newFolderAction, &QAction::triggered, this, &MainWin::newFolder);

	//"New file datasources"
	m_newLiveDataSourceAction = new QAction(QIcon::fromTheme("application-octet-stream"),i18n("Live Data Source..."),this);
	m_newLiveDataSourceAction->setWhatsThis(i18n("Creates a live data source to read data from a real time device"));
	actionCollection()->addAction("new_live_datasource", m_newLiveDataSourceAction);
	connect(m_newLiveDataSourceAction, &QAction::triggered, this, &MainWin::newLiveDataSourceActionTriggered);

	//Import/Export
	m_importFileAction = new QAction(QIcon::fromTheme("document-import"), i18n("From File..."), this);
	actionCollection()->setDefaultShortcut(m_importFileAction, Qt::CTRL+Qt::SHIFT+Qt::Key_I);
	m_importFileAction->setWhatsThis(i18n("Import data from a regular file"));
	connect(m_importFileAction, &QAction::triggered, this, [=]() {importFileDialog();});

	//second "import from file" action, with a shorter name, to be used in the sub-menu of the "Import"-menu.
	//the first action defined above will be used in the toolbar and touchbar where we need the more detailed name "Import From File".
	m_importFileAction_2 = new QAction(QIcon::fromTheme("document-import"), i18n("From File..."), this);
	actionCollection()->addAction("import_file", m_importFileAction_2);
	m_importFileAction_2->setWhatsThis(i18n("Import data from a regular file"));
	connect(m_importFileAction_2, &QAction::triggered, this, [=]() {importFileDialog();});

	m_importSqlAction = new QAction(QIcon::fromTheme("network-server-database"), i18n("From SQL Database..."), this);
	m_importSqlAction->setWhatsThis(i18n("Import data from a SQL database"));
	actionCollection()->addAction("import_sql", m_importSqlAction);
	connect(m_importSqlAction, &QAction::triggered, this, &MainWin::importSqlDialog);

	m_importDatasetAction = new QAction(QIcon::fromTheme(QLatin1String("database-index")), i18n("From Dataset Collection..."), this);
	m_importDatasetAction->setWhatsThis(i18n("Imports data from an online dataset"));
	actionCollection()->addAction("import_dataset_datasource", m_importDatasetAction);
	connect(m_importDatasetAction, &QAction::triggered, this, &MainWin::importDatasetDialog);

	m_importLabPlotAction = new QAction(QIcon::fromTheme("project-open"), i18n("LabPlot Project..."), this);
	m_importLabPlotAction->setWhatsThis(i18n("Import a project from a LabPlot project file (.lml)"));
	actionCollection()->addAction("import_labplot", m_importLabPlotAction);
	connect(m_importLabPlotAction, &QAction::triggered, this, &MainWin::importProjectDialog);

#ifdef HAVE_LIBORIGIN
	m_importOpjAction = new QAction(QIcon::fromTheme("project-open"), i18n("Origin Project (OPJ)..."), this);
	m_importOpjAction->setWhatsThis(i18n("Import a project from an OriginLab Origin project file (.opj)"));
	actionCollection()->addAction("import_opj", m_importOpjAction);
	connect(m_importOpjAction, &QAction::triggered, this, &MainWin::importProjectDialog);
#endif

	m_exportAction = new QAction(QIcon::fromTheme("document-export"), i18n("Export..."), this);
	m_exportAction->setWhatsThis(i18n("Export selected element"));
	actionCollection()->setDefaultShortcut(m_exportAction, Qt::CTRL+Qt::SHIFT+Qt::Key_E);
	actionCollection()->addAction("export", m_exportAction);
	connect(m_exportAction, &QAction::triggered, this, &MainWin::exportDialog);

	//Tools
	auto* action = new QAction(QIcon::fromTheme("color-management"), i18n("Color Maps Browser"), this);
	action->setWhatsThis(i18n("Open dialog to browse through the available color maps."));
	actionCollection()->addAction("color_maps", action);
	connect(action, &QAction::triggered, this, [=](){
			auto* dlg = new ColorMapsDialog(this);
			dlg->exec();
			delete dlg;
	});

#ifdef HAVE_FITS
	action = new QAction(QIcon::fromTheme("editor"), i18n("FITS Metadata Editor..."), this);
	action->setWhatsThis(i18n("Open editor to edit FITS meta data"));
	actionCollection()->addAction("edit_fits", action);
	connect(action, &QAction::triggered, this, &MainWin::editFitsFileDialog);
#endif

	// Edit
	//Undo/Redo-stuff
	m_undoAction = KStandardAction::undo(this, SLOT(undo()), actionCollection());
	m_redoAction = KStandardAction::redo(this, SLOT(redo()), actionCollection());
	m_historyAction = new QAction(QIcon::fromTheme("view-history"), i18n("Undo/Redo History..."),this);
	actionCollection()->addAction("history", m_historyAction);
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

	//Windows
	m_closeWindowAction  = new QAction(i18n("&Close"), this);
	actionCollection()->setDefaultShortcut(m_closeWindowAction, QKeySequence::Close);
	m_closeWindowAction->setStatusTip(i18n("Close the active window"));
	actionCollection()->addAction("close window", m_closeWindowAction);

	m_closeAllWindowsAction = new QAction(i18n("Close &All"), this);
	m_closeAllWindowsAction->setStatusTip(i18n("Close all the windows"));
	actionCollection()->addAction("close all windows", m_closeAllWindowsAction);

	m_tileWindowsAction = new QAction(i18n("&Tile"), this);
	m_tileWindowsAction->setStatusTip(i18n("Tile the windows"));
	actionCollection()->addAction("tile windows", m_tileWindowsAction);

	m_cascadeWindowsAction = new QAction(i18n("&Cascade"), this);
	m_cascadeWindowsAction->setStatusTip(i18n("Cascade the windows"));
	actionCollection()->addAction("cascade windows", m_cascadeWindowsAction);

	m_nextWindowAction = new QAction(QIcon::fromTheme("go-next-view"), i18n("Ne&xt"), this);
	actionCollection()->setDefaultShortcut(m_nextWindowAction, QKeySequence::NextChild);
	m_nextWindowAction->setStatusTip(i18n("Move the focus to the next window"));
	actionCollection()->addAction("next window", m_nextWindowAction);

	m_prevWindowAction = new QAction(QIcon::fromTheme("go-previous-view"), i18n("Pre&vious"), this);
	actionCollection()->setDefaultShortcut(m_prevWindowAction, QKeySequence::PreviousChild);
	m_prevWindowAction->setStatusTip(i18n("Move the focus to the previous window"));
	actionCollection()->addAction("previous window", m_prevWindowAction);

	//Actions for window visibility
	auto* windowVisibilityActions = new QActionGroup(this);
	windowVisibilityActions->setExclusive(true);

	m_visibilityFolderAction = new QAction(QIcon::fromTheme("folder"), i18n("Current &Folder Only"), windowVisibilityActions);
	m_visibilityFolderAction->setCheckable(true);
	m_visibilityFolderAction->setData(static_cast<int>(Project::MdiWindowVisibility::folderOnly));

	m_visibilitySubfolderAction = new QAction(QIcon::fromTheme("folder-documents"), i18n("Current Folder and &Subfolders"), windowVisibilityActions);
	m_visibilitySubfolderAction->setCheckable(true);
	m_visibilitySubfolderAction->setData(static_cast<int>(Project::MdiWindowVisibility::folderAndSubfolders));

	m_visibilityAllAction = new QAction(i18n("&All"), windowVisibilityActions);
	m_visibilityAllAction->setCheckable(true);
	m_visibilityAllAction->setData(static_cast<int>(Project::MdiWindowVisibility::allMdiWindows));

	connect(windowVisibilityActions, &QActionGroup::triggered, this, &MainWin::setMdiWindowVisibility);

	//show/hide the status and menu bars
	// KMainWindow should provide a menu that allows showing/hiding of the statusbar via showStatusbar()
	// see https://api.kde.org/frameworks/kxmlgui/html/classKXmlGuiWindow.html#a3d7371171cafabe30cb3bb7355fdfed1
	//KXMLGUI framework automatically stores "Disabled" for the key "StatusBar"
	KConfigGroup groupMain = KSharedConfig::openConfig()->group("MainWindow");
	const QString& str = groupMain.readEntry(QLatin1String("StatusBar"), "");
	bool statusBarDisabled = (str == QLatin1String("Disabled"));
	DEBUG(Q_FUNC_INFO << ", statusBar enabled in config: " << !statusBarDisabled)
	createStandardStatusBarAction();
	m_toggleStatusBarAction = KStandardAction::showStatusbar(this, &MainWin::toggleStatusBar, actionCollection());
	m_toggleStatusBarAction->setChecked(!statusBarDisabled);
	statusBar()->setEnabled(!statusBarDisabled);	// setVisible() does not work

	KStandardAction::showMenubar(this, &MainWin::toggleMenuBar, actionCollection());

	//show/hide the memory usage widget
	m_toggleMemoryInfoAction = new QAction(i18n("Show Memory Usage"));
	m_toggleMemoryInfoAction->setCheckable(true);
	connect(m_toggleMemoryInfoAction, &QAction::triggered, this, &MainWin::toggleMemoryInfo);

	//Actions for hiding/showing the dock widgets
	auto* docksActions = new QActionGroup(this);
	docksActions->setExclusive(false);

	m_toggleProjectExplorerDockAction = new QAction(QIcon::fromTheme("view-list-tree"), i18n("Project Explorer"), docksActions);
	m_toggleProjectExplorerDockAction->setCheckable(true);
	m_toggleProjectExplorerDockAction->setChecked(true);
	actionCollection()->addAction("toggle_project_explorer_dock", m_toggleProjectExplorerDockAction);

	m_togglePropertiesDockAction = new QAction(QIcon::fromTheme("view-list-details"), i18n("Properties Explorer"), docksActions);
	m_togglePropertiesDockAction->setCheckable(true);
	m_togglePropertiesDockAction->setChecked(true);
	actionCollection()->addAction("toggle_properties_explorer_dock", m_togglePropertiesDockAction);

	connect(docksActions, &QActionGroup::triggered, this, &MainWin::toggleDockWidget);

	//global search
	m_searchAction = new QAction(actionCollection());
	m_searchAction->setShortcut(QKeySequence::Find);
	connect(m_searchAction, &QAction::triggered, this, [=]() {
		if (m_project) {
			if (!m_projectExplorerDock->isVisible()) {
				m_toggleProjectExplorerDockAction->setChecked(true);
				toggleDockWidget(m_toggleProjectExplorerDockAction);
			}
			m_projectExplorer->search();
		}
	});
	this->addAction(m_searchAction);
}

void MainWin::initMenus() {
	//add the actions to toggle the status bar and the project and properties explorer widgets to the "View" menu.
	//this menu is created automatically when the default "full screen" action is created in initActions().
	auto* menu = dynamic_cast<QMenu*>(factory()->container("view", this));

	if (menu) {
		menu->addSeparator();
		menu->addAction(m_toggleProjectExplorerDockAction);
		menu->addAction(m_togglePropertiesDockAction);
	}

	//menu in the main toolbar for adding new aspects
	menu = dynamic_cast<QMenu*>(factory()->container("new", this));
	if (menu)
		menu->setIcon(QIcon::fromTheme("window-new"));

	//menu in the project explorer and in the toolbar for adding new aspects
	m_newMenu = new QMenu(i18n("Add New"), this);
	m_newMenu->setIcon(QIcon::fromTheme("window-new"));
	m_newMenu->addAction(m_newFolderAction);
	m_newMenu->addAction(m_newWorkbookAction);
	m_newMenu->addAction(m_newSpreadsheetAction);
	m_newMenu->addAction(m_newMatrixAction);
	m_newMenu->addAction(m_newWorksheetAction);
	m_newMenu->addAction(m_newNotesAction);
	m_newMenu->addAction(m_newDatapickerAction);
	m_newMenu->addSeparator();
	m_newMenu->addAction(m_newLiveDataSourceAction);

	//import menu
	m_importMenu = new QMenu(this);
	m_importMenu->setIcon(QIcon::fromTheme("document-import"));
	m_importMenu ->addAction(m_importFileAction_2);
	m_importMenu ->addAction(m_importSqlAction);
	m_importMenu->addAction(m_importDatasetAction);
	m_importMenu->addSeparator();
	m_importMenu->addAction(m_importLabPlotAction);
#ifdef HAVE_LIBORIGIN
	m_importMenu ->addAction(m_importOpjAction);
#endif

#ifdef HAVE_CANTOR_LIBS
	m_newMenu->addSeparator();
	m_newCantorWorksheetMenu = new QMenu(i18n("Notebook"), this);
	m_newCantorWorksheetMenu->setIcon(QIcon::fromTheme("archive-insert"));

	//"Adding Cantor backends to menu and context menu"
	QStringList backendNames = Cantor::Backend::listAvailableBackends();
#if !defined(NDEBUG) || defined(Q_OS_WIN) || defined(Q_OS_MACOS)
	WARN(Q_FUNC_INFO << ", " << backendNames.count() << " Cantor backends available:")
	for (const auto& b : backendNames)
		WARN("Backend: " << STDSTRING(b))
#endif

	if (backendNames.count() > 0) {
		unplugActionList(QLatin1String("backends_list"));
		QList<QAction*> newBackendActions;
		for (auto* backend : Cantor::Backend::availableBackends()) {
			if (!backend->isEnabled()) continue;
			QAction* action = new QAction(QIcon::fromTheme(backend->icon()), backend->name(), this);
			action->setData(backend->name());
			newBackendActions << action;
			m_newCantorWorksheetMenu->addAction(action);
		}

		connect(m_newCantorWorksheetMenu, &QMenu::triggered, this, &MainWin::newCantorWorksheet);
		plugActionList(QLatin1String("backends_list"), newBackendActions);
	}
	m_newMenu->addMenu(m_newCantorWorksheetMenu);
#else
	delete this->guiFactory()->container("notebook", this);
	delete this->guiFactory()->container("new_notebook", this);
	delete this->guiFactory()->container("notebook_toolbar", this);
#endif

	//menu subwindow visibility policy
	m_visibilityMenu = new QMenu(i18n("Window Visibility"), this);
	m_visibilityMenu->setIcon(QIcon::fromTheme("window-duplicate"));
	m_visibilityMenu ->addAction(m_visibilityFolderAction);
	m_visibilityMenu ->addAction(m_visibilitySubfolderAction);
	m_visibilityMenu ->addAction(m_visibilityAllAction);

// 	//menu for editing files
// 	m_editMenu = new QMenu(i18n("Edit"), this);
#ifdef HAVE_FITS
//	m_editMenu->addAction(m_editFitsFileAction);
#endif
	//set the action for the current color scheme checked
	KConfigGroup group = KSharedConfig::openConfig()->group(QLatin1String("Settings_General"));
#if KCOREADDONS_VERSION >= QT_VERSION_CHECK(5, 67, 0)	// KColorSchemeManager has a system default option
	QString schemeName = group.readEntry("ColorScheme");
#else
	KConfigGroup generalGlobalsGroup = KSharedConfig::openConfig(QLatin1String("kdeglobals"))->group("General");
	QString defaultSchemeName = generalGlobalsGroup.readEntry("ColorScheme", QStringLiteral("Breeze"));
	QString schemeName = group.readEntry("ColorScheme", defaultSchemeName);
#endif
	// default dark scheme on Windows is not optimal (Breeze dark is better)
	// we can't find out if light or dark mode is used, so we don't switch to Breeze/Breeze dark here
	DEBUG(Q_FUNC_INFO << ", Color scheme = " << STDSTRING(schemeName))
	KActionMenu* schemesMenu = m_schemeManager->createSchemeSelectionMenu(i18n("Color Scheme"), schemeName, this);
	schemesMenu->setIcon(QIcon::fromTheme(QStringLiteral("preferences-desktop-color")));
	connect(schemesMenu->menu(), &QMenu::triggered, this, &MainWin::colorSchemeChanged);

	QMenu* settingsMenu = dynamic_cast<QMenu*>(factory()->container("settings", this));
	if (settingsMenu) {
		auto* action = settingsMenu->insertSeparator(settingsMenu->actions().constFirst());
		settingsMenu->insertMenu(action, schemesMenu->menu());

		// add m_toggleMemoryInfoAction after the "Show status bar" action
		auto actions = settingsMenu->actions();
		const int index = actions.indexOf(m_toggleStatusBarAction);
		settingsMenu->insertAction(actions.at(index + 1), m_toggleMemoryInfoAction);
	}

#ifdef HAVE_CANTOR_LIBS
	QAction* action = new QAction(QIcon::fromTheme(QLatin1String("cantor")), i18n("Configure CAS..."), this);
	connect(action, &QAction::triggered, this, &MainWin::cantorSettingsDialog);
	action->setMenuRole(QAction::NoRole);	// prevent macOS Qt heuristics to select this action for preferences
	if (settingsMenu)
		settingsMenu->addAction(action);
#endif
}

void MainWin::colorSchemeChanged(QAction* action) {
	QString schemeName = KLocalizedString::removeAcceleratorMarker(action->text());

	//background of the mdi area is not updated on theme changes, do it here.
	QModelIndex index = m_schemeManager->indexForScheme(schemeName);
	const QPalette& palette = KColorScheme::createApplicationPalette( KSharedConfig::openConfig(index.data(Qt::UserRole).toString()) );
	const QBrush& brush = palette.brush(QPalette::Dark);
	m_mdiArea->setBackground(brush);

	//save the selected color scheme
	KConfigGroup group = KSharedConfig::openConfig()->group(QLatin1String("Settings_General"));
	group.writeEntry("ColorScheme", schemeName);
	group.sync();
}

/*!
	Asks to save the project if it was modified.
	\return \c true if the project still needs to be saved ("cancel" clicked), \c false otherwise.
 */
bool MainWin::warnModified() {
	if (m_project->hasChanged()) {
		int want_save = KMessageBox::warningYesNoCancel( this,
			i18n("The current project %1 has been modified. Do you want to save it?", m_project->name()),
			i18n("Save Project"));
		switch (want_save) {
		case KMessageBox::Yes:
			return !saveProject();
		case KMessageBox::No:
			break;
		case KMessageBox::Cancel:
			return true;
		}
	}

	return false;
}

/*!
 * updates the state of actions, menus and toolbars (enabled or disabled)
 * on project changes (project closes and opens)
 */
void MainWin::updateGUIOnProjectChanges() {
	if (m_closing)
		return;

	KXMLGUIFactory* factory = this->guiFactory();
	if (factory->container("worksheet", this) == nullptr) {
		//no worksheet menu found, most probably labplot2ui.rc
		//was not properly installed -> return here in order not to crash
		return;
	}

	//disable all menus if there is no project
	bool hasProject = (m_project != nullptr);
	m_saveAction->setEnabled(hasProject);
	m_saveAsAction->setEnabled(hasProject);
	m_importFileAction->setEnabled(hasProject);
	m_importFileAction_2->setEnabled(hasProject);
	m_importSqlAction->setEnabled(hasProject);
#ifdef HAVE_LIBORIGIN
	m_importOpjAction->setEnabled(hasProject);
#endif
	m_newWorkbookAction->setEnabled(hasProject);
	m_newSpreadsheetAction->setEnabled(hasProject);
	m_newMatrixAction->setEnabled(hasProject);
	m_newWorksheetAction->setEnabled(hasProject);
	m_newDatapickerAction->setEnabled(hasProject);
	m_closeAction->setEnabled(hasProject);
	m_toggleProjectExplorerDockAction->setEnabled(hasProject);
	m_togglePropertiesDockAction->setEnabled(hasProject);

	//disable print and export actions if there is no project
	//and don't activate them if a new project was created.
	//they will be activated later in updateGUI() once there is
	//a view to print or to export
	if (!hasProject) {
		m_printAction->setEnabled(false);
		m_printPreviewAction->setEnabled(false);
		m_exportAction->setEnabled(false);
	}

	if (!m_mdiArea || !m_mdiArea->currentSubWindow()) {
		factory->container("spreadsheet", this)->setEnabled(false);
		factory->container("matrix", this)->setEnabled(false);
		factory->container("worksheet", this)->setEnabled(false);
		factory->container("analysis", this)->setEnabled(false);
		factory->container("datapicker", this)->setEnabled(false);
		factory->container("spreadsheet_toolbar", this)->hide();
		factory->container("worksheet_toolbar", this)->hide();
		factory->container("cartesian_plot_toolbar", this)->hide();
// 		factory->container("histogram_toolbar",this)->hide();
// 		factory->container("barchart_toolbar",this)->hide();
		factory->container("datapicker_toolbar", this)->hide();
#ifdef HAVE_CANTOR_LIBS
		factory->container("notebook", this)->setEnabled(false);
		factory->container("notebook_toolbar", this)->hide();
#endif
	}

	factory->container("new", this)->setEnabled(hasProject);
	factory->container("edit", this)->setEnabled(hasProject);
	factory->container("import", this)->setEnabled(hasProject);

	updateTitleBar();

#ifdef HAVE_TOUCHBAR
	m_touchBar->clear();

	if (!hasProject) {
		m_touchBar->addAction(m_newProjectAction);
		m_touchBar->addAction(m_openProjectAction);
	} else {
		m_touchBar->addAction(m_importFileAction);
		m_touchBar->addAction(m_newWorksheetAction);
		m_touchBar->addAction(m_newSpreadsheetAction);
		m_touchBar->addAction(m_newMatrixAction);
	}
#endif
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

	KXMLGUIFactory* factory = this->guiFactory();
	if (factory->container("worksheet", this) == nullptr) {
		//no worksheet menu found, most probably labplot2ui.rc
		//was not properly installed -> return here in order not to crash
		return;
	}

	//reset the touchbar
#ifdef HAVE_TOUCHBAR
	m_touchBar->clear();

	m_touchBar->addAction(m_undoIconOnlyAction);
	m_touchBar->addAction(m_redoIconOnlyAction);
	m_touchBar->addSeparator();
#endif

	if (!m_mdiArea || !m_mdiArea->currentSubWindow()) {
		factory->container("spreadsheet", this)->setEnabled(false);
		factory->container("matrix", this)->setEnabled(false);
		factory->container("worksheet", this)->setEnabled(false);
		factory->container("analysis", this)->setEnabled(false);
		factory->container("datapicker", this)->setEnabled(false);
		factory->container("spreadsheet_toolbar", this)->hide();
		factory->container("worksheet_toolbar", this)->hide();
// 		factory->container("histogram_toolbar",this)->hide();
// 		factory->container("barchart_toolbar",this)->hide();
		factory->container("cartesian_plot_toolbar", this)->hide();
		factory->container("datapicker_toolbar", this)->hide();
#ifdef HAVE_CANTOR_LIBS
		factory->container("notebook", this)->setEnabled(false);
		factory->container("notebook_toolbar", this)->hide();
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
	//Handle the Worksheet-object
	const auto* w = dynamic_cast<Worksheet*>(m_currentAspect);
	if (!w)
		w = dynamic_cast<Worksheet*>(m_currentAspect->parent(AspectType::Worksheet));

	if (w) {
		bool update = (w != m_lastWorksheet);
		m_lastWorksheet = w;

		//populate worksheet menu
		auto* view = qobject_cast<WorksheetView*>(w->view());
		auto* menu = qobject_cast<QMenu*>(factory->container("worksheet", this));
		if (update) {
			menu->clear();
			view->createContextMenu(menu);
		}
		menu->setEnabled(true);

		//populate analysis menu
		menu = qobject_cast<QMenu*>(factory->container("analysis", this));
		if (update) {
			menu->clear();
			view->createAnalysisMenu(menu);
		}
		menu->setEnabled(true);

		//populate worksheet-toolbar
		auto* toolbar = qobject_cast<QToolBar*>(factory->container("worksheet_toolbar", this));
		if (update) {
			toolbar->clear();
			view->fillToolBar(toolbar);
		}
		toolbar->setVisible(true);
		toolbar->setEnabled(true);

		//populate the toolbar for cartesian plots
		toolbar = qobject_cast<QToolBar*>(factory->container("cartesian_plot_toolbar", this));
		if (update) {
			toolbar->clear();
			view->fillCartesianPlotToolBar(toolbar);
		}
		toolbar->setVisible(true);
		toolbar->setEnabled(true);

		//populate the touchbar on Mac
#ifdef HAVE_TOUCHBAR
		view->fillTouchBar(m_touchBar);
#endif
		//hide the spreadsheet toolbar
		factory->container("spreadsheet_toolbar", this)->setVisible(false);
	} else {
		factory->container("worksheet", this)->setEnabled(false);
		factory->container("worksheet_toolbar", this)->setVisible(false);
		factory->container("analysis", this)->setEnabled(false);
//		factory->container("drawing", this)->setEnabled(false);
		factory->container("worksheet_toolbar", this)->setEnabled(false);
		factory->container("cartesian_plot_toolbar", this)->setEnabled(false);
	}

	//Handle the Spreadsheet-object
	const auto* spreadsheet = this->activeSpreadsheet();
	if (!spreadsheet)
		spreadsheet = dynamic_cast<Spreadsheet*>(m_currentAspect->parent(AspectType::Spreadsheet));
	if (spreadsheet) {
		bool update = (spreadsheet != m_lastSpreadsheet);
		m_lastSpreadsheet = spreadsheet;

		//populate spreadsheet-menu
		auto* view = qobject_cast<SpreadsheetView*>(spreadsheet->view());
		auto* menu = qobject_cast<QMenu*>(factory->container("spreadsheet", this));
		if (update) {
			menu->clear();
			view->createContextMenu(menu);
		}
		menu->setEnabled(true);

		//populate spreadsheet-toolbar
		auto* toolbar = qobject_cast<QToolBar*>(factory->container("spreadsheet_toolbar", this));
		if (update) {
			toolbar->clear();
			view->fillToolBar(toolbar);
		}
		toolbar->setVisible(true);
		toolbar->setEnabled(true);

		//populate the touchbar on Mac
#ifdef HAVE_TOUCHBAR
		m_touchBar->addAction(m_importFileAction);
		view->fillTouchBar(m_touchBar);
#endif

		//spreadsheet has it's own search, unregister the shortcut for the global search here
		m_searchAction->setShortcut(QKeySequence());
	} else {
		factory->container("spreadsheet", this)->setEnabled(false);
		factory->container("spreadsheet_toolbar", this)->setVisible(false);
		m_searchAction->setShortcut(QKeySequence::Find);
	}

	//Handle the Matrix-object
	const auto* matrix = dynamic_cast<Matrix*>(m_currentAspect);
	if (!matrix)
		matrix = dynamic_cast<Matrix*>(m_currentAspect->parent(AspectType::Matrix));
	if (matrix) {
		//populate matrix-menu
		auto* view = qobject_cast<MatrixView*>(matrix->view());
		auto* menu = qobject_cast<QMenu*>(factory->container("matrix", this));
		menu->clear();
		view->createContextMenu(menu);
		menu->setEnabled(true);

		//populate the touchbar on Mac
#ifdef HAVE_TOUCHBAR
		m_touchBar->addAction(m_importFileAction);
		//view->fillTouchBar(m_touchBar);
#endif
	} else
		factory->container("matrix", this)->setEnabled(false);

#ifdef HAVE_CANTOR_LIBS
	const auto* cantorworksheet = dynamic_cast<CantorWorksheet*>(m_currentAspect);
	if (!cantorworksheet)
		cantorworksheet = dynamic_cast<CantorWorksheet*>(m_currentAspect->parent(AspectType::CantorWorksheet));
	if (cantorworksheet) {
		auto* view = qobject_cast<CantorWorksheetView*>(cantorworksheet->view());
		auto* menu = qobject_cast<QMenu*>(factory->container("notebook", this));
		menu->clear();
		view->createContextMenu(menu);
		menu->setEnabled(true);

		auto* toolbar = qobject_cast<QToolBar*>(factory->container("notebook_toolbar", this));
		toolbar->setVisible(true);
		toolbar->clear();
		view->fillToolBar(toolbar);
	} else {
		//no Cantor worksheet selected -> deactivate Cantor worksheet related menu and toolbar
		factory->container("notebook", this)->setEnabled(false);
		factory->container("notebook_toolbar", this)->setVisible(false);
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
		//populate datapicker-menu
		auto* view = qobject_cast<DatapickerView*>(datapicker->view());
		auto* menu = qobject_cast<QMenu*>(factory->container("datapicker", this));
		menu->clear();
		view->createContextMenu(menu);
		menu->setEnabled(true);

		//populate spreadsheet-toolbar
		auto* toolbar = qobject_cast<QToolBar*>(factory->container("datapicker_toolbar", this));
		toolbar->clear();
		view->fillToolBar(toolbar);
		toolbar->setVisible(true);
	} else {
		factory->container("datapicker", this)->setEnabled(false);
		factory->container("datapicker_toolbar", this)->setVisible(false);
	}
}

/*!
	creates a new empty project. Returns \c true, if a new project was created.
*/
bool MainWin::newProject() {
	//close the current project, if available
	if (!closeProject())
		return false;

// 	if (dynamic_cast<QQuickWidget*>(centralWidget())) {
// 		createMdiArea();
// 		setCentralWidget(m_mdiArea);
// 	}

	QApplication::processEvents(QEventLoop::AllEvents, 100);

	delete m_project; m_project = nullptr;
	delete m_aspectTreeModel; m_aspectTreeModel = nullptr;

	m_project = new Project();
	m_currentAspect = m_project;
	m_currentFolder = m_project;

	KConfigGroup group = KSharedConfig::openConfig()->group(QLatin1String("Settings_General"));
	auto vis = Project::MdiWindowVisibility(group.readEntry("MdiWindowVisibility", 0));
	m_project->setMdiWindowVisibility( vis );
	if (vis == Project::MdiWindowVisibility::folderOnly)
		m_visibilityFolderAction->setChecked(true);
	else if (vis == Project::MdiWindowVisibility::folderAndSubfolders)
		m_visibilitySubfolderAction->setChecked(true);
	else
		m_visibilityAllAction->setChecked(true);

	m_aspectTreeModel = new AspectTreeModel(m_project, this);
	connect(m_aspectTreeModel, &AspectTreeModel::statusInfo, [=](const QString& text){ statusBar()->showMessage(text); });

	//newProject is called for the first time, there is no project explorer yet
	//-> initialize the project explorer,  the GUI-observer and the dock widgets.
	if (!m_projectExplorer) {
		group = KSharedConfig::openConfig()->group(QLatin1String("MainWin"));

		m_projectExplorerDock = new QDockWidget(this);
		m_projectExplorerDock->setObjectName("projectexplorer");
		m_projectExplorerDock->setWindowTitle(i18nc("@title:window", "Project Explorer"));
		m_projectExplorerDock->setWindowTitle(m_projectExplorerDock->windowTitle().replace("&", QString()));
		m_projectExplorerDock->toggleViewAction()->setText("");

		m_projectExplorer = new ProjectExplorer(m_projectExplorerDock);
		m_projectExplorerDock->setWidget(m_projectExplorer);

		connect(m_projectExplorer, &ProjectExplorer::currentAspectChanged, this, &MainWin::handleCurrentAspectChanged);
		connect(m_projectExplorer, &ProjectExplorer::activateView, this, &MainWin::activateSubWindowForAspect);
		connect(m_projectExplorerDock, &QDockWidget::visibilityChanged, this, &MainWin::projectExplorerDockVisibilityChanged);

		//Properties dock
		m_propertiesDock = new QDockWidget(this);
		m_propertiesDock->setObjectName("aspect_properties_dock");
		m_propertiesDock->setWindowTitle(i18nc("@title:window", "Properties"));
		m_propertiesDock->setWindowTitle(m_propertiesDock->windowTitle().replace("&", QString()));

		//restore the position of the dock widgets:
		//"WindowState" doesn't always contain the positions of the dock widgets,
		//user opened the application and closed it without creating a new project
		//and with this the dock widgets - this creates a "WindowState" section in the settings without dock widgets positions.
		//So, we set our default positions first and then read from the saved "WindowState" section
		addDockWidget(Qt::LeftDockWidgetArea, m_projectExplorerDock);
		addDockWidget(Qt::RightDockWidgetArea, m_propertiesDock);
		if (group.keyList().indexOf("WindowState") != -1)
			restoreState(group.readEntry("WindowState", QByteArray()));

		auto* scrollArea = new QScrollArea(m_propertiesDock);
		scrollArea->setWidgetResizable(true);

		stackedWidget = new QStackedWidget(scrollArea);
		scrollArea->setWidget(stackedWidget);		// stacked widget inside scroll area
		m_propertiesDock->setWidget(scrollArea);	// scroll area inside dock

		connect(m_propertiesDock, &QDockWidget::visibilityChanged, this, &MainWin::propertiesDockVisibilityChanged);

		//GUI-observer;
		m_guiObserver = new GuiObserver(this);
	}

	m_projectExplorer->setModel(m_aspectTreeModel);
	m_projectExplorer->setProject(m_project);
	m_projectExplorer->setCurrentAspect(m_project);

	m_projectExplorerDock->show();
	m_propertiesDock->show();
	updateGUIOnProjectChanges();
	m_newProjectAction->setEnabled(false);

	connect(m_project, &Project::aspectAdded, this, &MainWin::handleAspectAdded);
	connect(m_project, &Project::aspectRemoved, this, &MainWin::handleAspectRemoved);
	connect(m_project, &Project::aspectAboutToBeRemoved, this, &MainWin::handleAspectAboutToBeRemoved);
	connect(m_project, SIGNAL(statusInfo(QString)), statusBar(), SLOT(showMessage(QString)));
	connect(m_project, &Project::changed, this, &MainWin::projectChanged);
	connect(m_project, &Project::requestProjectContextMenu, this, &MainWin::createContextMenu);
	connect(m_project, &Project::requestFolderContextMenu, this, &MainWin::createFolderContextMenu);
	connect(m_project, &Project::mdiWindowVisibilityChanged, this, &MainWin::updateMdiWindowVisibility);
	connect(m_project, &Project::closeRequested, this, &MainWin::closeProject);

	m_undoViewEmptyLabel = i18n("%1: created", m_project->name());
	updateTitleBar();

	return true;
}

void MainWin::openProject() {
	bool supportOthers = false;
	QString allExtensions = Project::supportedExtensions();
	QString extensions = i18n("LabPlot Projects (%1)", allExtensions);
	if (m_lastOpenFileFilter.isEmpty())
		m_lastOpenFileFilter = extensions;

#ifdef HAVE_LIBORIGIN
	extensions += QLatin1String(";;") + i18n("Origin Projects (%1)", OriginProjectParser::supportedExtensions());
	allExtensions += " " + OriginProjectParser::supportedExtensions();
	supportOthers = true;
#endif

#ifdef HAVE_CANTOR_LIBS
	extensions += QLatin1String(";;") + i18n("Cantor Projects (*.cws)");
	extensions += QLatin1String(";;") + i18n("Jupyter Notebooks (*.ipynb)");
	allExtensions += QLatin1String(" *.cws *.ipynb");
	supportOthers = true;
#endif

	//add an entry for "All supported files" if we support more than labplot
	if (supportOthers)
		extensions = i18n("All supported files (%1)", allExtensions) + QLatin1String(";;") + extensions;

	KConfigGroup group(KSharedConfig::openConfig(), "MainWin");
	const QString& dir = group.readEntry("LastOpenDir", "");
	const QString& path = QFileDialog::getOpenFileName(this,i18nc("@title:window", "Open Project"), dir, extensions, &m_lastOpenFileFilter);
	if (path.isEmpty())// "Cancel" was clicked
		return;

	this->openProject(path);

	//save new "last open directory"
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

// 	if (dynamic_cast<QQuickWidget*>(centralWidget())) {
// 		createMdiArea();
// 		setCentralWidget(m_mdiArea);
// 	}

	if (!newProject())
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
		parser.importTo(m_project, QStringList()); //TODO: add return code
		rc = true;
	}
#endif

#ifdef HAVE_CANTOR_LIBS
	else if (QFileInfo(filename).completeSuffix() == QLatin1String("cws")) {
		QFile file(filename);
		KZip archive(&file);
		rc = archive.open(QIODevice::ReadOnly);
		if (rc) {
			const auto* contentEntry = archive.directory()->entry(QLatin1String("content.xml"));
			if (contentEntry && contentEntry->isFile()) {
				const auto* contentFile = static_cast<const KArchiveFile*>(contentEntry);
				QByteArray data = contentFile->data();
				archive.close();

				//determine the name of the backend
				QDomDocument doc;
				doc.setContent(data);
				QString backendName = doc.documentElement().attribute(QLatin1String("backend"));

				if (!backendName.isEmpty()) {
					//create new Cantor worksheet and load the data
					auto* worksheet = new CantorWorksheet(backendName);
					worksheet->setName(QFileInfo(filename).fileName());
					worksheet->setComment(filename);

					rc = file.open(QIODevice::ReadOnly);
					if (rc) {
						QByteArray content = file.readAll();
						rc = worksheet->init(&content);
						if (rc)
							m_project->addChild(worksheet);
						else {
							delete worksheet;
							RESET_CURSOR;
							QMessageBox::critical(this, i18n("Failed to open project"), i18n("Failed to process the content of the file '%1'.", filename));
						}
					}else {
						RESET_CURSOR;
						QMessageBox::critical(this, i18n("Failed to open project"), i18n("Failed to open the file '%1'.", filename));
					}
				} else {
					RESET_CURSOR;
					rc = false;
					QMessageBox::critical(this, i18n("Failed to open project"), i18n("Failed to process the content of the file '%1'.", filename));
				}
			} else {
				RESET_CURSOR;
				rc = false;
				QMessageBox::critical(this, i18n("Failed to open project"), i18n("Failed to process the content of the file '%1'.", filename));
			}
		} else {
			RESET_CURSOR;
			QMessageBox::critical(this, i18n("Failed to open project"), i18n("Failed to open the file '%1'.", filename));
		}
	} else if (QFileInfo(filename).completeSuffix() == QLatin1String("ipynb")) {
		QFile file(filename);
		rc = file.open(QIODevice::ReadOnly);
		if (rc) {
			QByteArray content = file.readAll();
			QJsonParseError error;
			// TODO: use QJsonDocument& doc = QJsonDocument::fromJson(content, &error); if minimum Qt version is at least 5.10
			const QJsonDocument& jsonDoc = QJsonDocument::fromJson(content, &error);
			const QJsonObject& doc = jsonDoc.object();
			if (error.error == QJsonParseError::NoError) {
				//determine the backend name
				QString backendName;
				// TODO: use doc["metadata"]["kernelspec"], etc. if minimum Qt version is at least 5.10
				if ((doc["metadata"] != QJsonValue::Undefined && doc["metadata"].isObject())
					&& (doc["metadata"].toObject()["kernelspec"] != QJsonValue::Undefined && doc["metadata"].toObject()["kernelspec"].isObject()) ) {

					QString kernel;
					if (doc["metadata"].toObject()["kernelspec"].toObject()["name"] != QJsonValue::Undefined)
						kernel = doc["metadata"].toObject()["kernelspec"].toObject()["name"].toString();

					if (!kernel.isEmpty()) {
						if (kernel.startsWith(QLatin1String("julia")))
							backendName = QLatin1String("julia");
						else if (kernel == QLatin1String("sagemath"))
							backendName = QLatin1String("sage");
						else if (kernel == QLatin1String("ir"))
							backendName = QLatin1String("r");
						else if (kernel == QLatin1String("python3") || kernel == QLatin1String("python2"))
							backendName = QLatin1String("python");
						else
							backendName = kernel;
					} else
						backendName = doc["metadata"].toObject()["kernelspec"].toObject()["language"].toString();

					if (!backendName.isEmpty()) {
						//create new Cantor worksheet and load the data
						auto* worksheet = new CantorWorksheet(backendName);
						worksheet->setName(QFileInfo(filename).fileName());
						worksheet->setComment(filename);
						rc = worksheet->init(&content);
						if (rc)
							m_project->addChild(worksheet);
						else {
							delete worksheet;
							RESET_CURSOR;
							QMessageBox::critical(this, i18n("Failed to open project"), i18n("Failed to process the content of the file '%1'.", filename));
						}
					} else {
						RESET_CURSOR;
						rc = false;
						QMessageBox::critical(this, i18n("Failed to open project"), i18n("Failed to process the content of the file '%1'.", filename));
					}
				} else {
					RESET_CURSOR;
					rc = false;
					QMessageBox::critical(this, i18n("Failed to open project"), i18n("Failed to process the content of the file '%1'.", filename));
				}
			}
		} else {
			RESET_CURSOR;
			rc = false;
			QMessageBox::critical(this, i18n("Failed to open project"), i18n("Failed to open the file '%1'.", filename));
		}
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
	m_recentProjectsAction->addUrl( QUrl(filename) );
	updateTitleBar();
	updateGUIOnProjectChanges();
	updateGUI(); //there are most probably worksheets or spreadsheets in the open project -> update the GUI
	updateMdiWindowVisibility();
	m_saveAction->setEnabled(false);
	m_newProjectAction->setEnabled(true);

	statusBar()->showMessage( i18n("Project successfully opened (in %1 seconds).", (float)timer.elapsed()/1000) );

	KConfigGroup group = KSharedConfig::openConfig()->group(QLatin1String("MainWin"));
	group.writeEntry("LastOpenProject", filename);

	if (m_autoSaveActive)
		m_autoSaveTimer.start();

	RESET_CURSOR;
}

void MainWin::openRecentProject(const QUrl& url) {
// 	if (dynamic_cast<QQuickWidget*>(centralWidget())) {
// 		createMdiArea();
// 		setCentralWidget(m_mdiArea);
// 	}

	if (url.isLocalFile())	// fix for Windows
		this->openProject(url.toLocalFile());
	else
		this->openProject(url.path());
}

/*!
	Closes the current project, if available. Return \c true, if the project was closed.
*/
bool MainWin::closeProject() {
	if (m_project == nullptr)
		return true; //nothing to close

	if (warnModified())
		return false;

	if (!m_closing) {
// 		if (dynamic_cast<QQuickWidget*>(centralWidget()) && m_showWelcomeScreen) {
// 			m_welcomeWidget = createWelcomeScreen();
// 			setCentralWidget(m_welcomeWidget);
// 		}
	}

	//hide the sub-windows prior to deleting them in order to get rid of the shadows
	//drawn across the sub-windows by the style. The shadow is removed by closing/hiding
	//the sub-window explicitly but not if we just delete it.
	//TODO: the actual fix is in https://invent.kde.org/plasma/breeze/-/merge_requests/43,
	//we can remove this hack later.
	for (auto* window : m_mdiArea->subWindowList())
		window->hide();

	m_projectClosing = true;
	statusBar()->clearMessage();
	delete m_aspectTreeModel;
	m_aspectTreeModel = nullptr;
	delete m_project;
	m_project = nullptr;
	m_projectClosing = false;

	//update the UI if we're just closing a project
	//and not closing(quitting) the application
	if (!m_closing) {
		m_projectExplorerDock->hide();
		m_propertiesDock->hide();
		m_currentAspect = nullptr;
		m_currentFolder = nullptr;
		updateGUIOnProjectChanges();
		m_newProjectAction->setEnabled(true);

		if (m_autoSaveActive)
			m_autoSaveTimer.stop();
	}

	removeDockWidget(cursorDock);
	delete cursorDock;
	cursorDock = nullptr;
	cursorWidget = nullptr; // is deleted, because it's the child of cursorDock

	return true;
}

bool MainWin::saveProject() {
	QString fileName = m_project->fileName();
	if (fileName.isEmpty())
		return saveProjectAs();
	else {
		// don't overwrite OPJ files
		if (fileName.endsWith(".opj", Qt::CaseInsensitive))
			fileName.replace(".opj", ".lml");
		return save(fileName);
	}
}

bool MainWin::saveProjectAs() {
	KConfigGroup conf(KSharedConfig::openConfig(), "MainWin");
	const QString& dir = conf.readEntry("LastOpenDir", "");
	QString path = QFileDialog::getSaveFileName(this, i18nc("@title:window", "Save Project As"), dir + m_project->fileName(),
		i18n("LabPlot Projects (*.lml *.lml.gz *.lml.bz2 *.lml.xz *.LML *.LML.GZ *.LML.BZ2 *.LML.XZ)"));
	// The "Automatically select filename extension (.lml)" option does not change anything

	if (path.isEmpty())// "Cancel" was clicked
		return false;

	if (!path.endsWith(QLatin1String(".lml"), Qt::CaseInsensitive))
		path.append(QLatin1String(".lml"));

	//save new "last open directory"
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
	const KConfigGroup group = KSharedConfig::openConfig()->group("Settings_General");
	if (fileName.endsWith(QLatin1String(".lml"))) {
		if (group.readEntry("CompatibleSave", false))
			file = new KCompressionDevice(tempFileName, KCompressionDevice::GZip);
		else
			file = new KCompressionDevice(tempFileName, KCompressionDevice::Xz);
	} else	// use file ending to find out how to compress file
		file = new KFilterDev(tempFileName);
	if (!file)
		file = new QFile(tempFileName);

	bool ok;
	if (file->open(QIODevice::WriteOnly)) {
		m_project->setFileName(fileName);

		QPixmap thumbnail;
		const auto& windows = m_mdiArea->subWindowList();
		if (!windows.isEmpty()) {
			//determine the bounding rectangle surrounding all visible sub-windows
			QRect rect;
			for (auto* window: windows)
				rect = rect.united(window->frameGeometry());

			thumbnail = centralWidget()->grab(rect);
		}

		QXmlStreamWriter writer(file);
		m_project->setFileName(fileName);
		m_project->save(thumbnail, &writer);
		m_project->undoStack()->clear();
		m_project->setChanged(false);
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
			m_recentProjectsAction->addUrl( QUrl(fileName) );
			ok = true;

			//if the project dock is visible, refresh the shown content
			//(version and modification time might have been changed)
			if (stackedWidget->currentWidget() == projectDock)
				projectDock->setProject(m_project);

			//we have a file name now
			// -> auto save can be activated now if not happened yet
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

	RESET_CURSOR;
	return ok;
}

/*!
 * automatically saves the project in the specified time interval.
 */
void MainWin::autoSaveProject() {
	//don't auto save when there are no changes or the file name
	//was not provided yet (the project was never explicitly saved yet).
	if ( !m_project->hasChanged() || m_project->fileName().isEmpty())
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
	QMdiSubWindow* win = m_mdiArea->currentSubWindow();
	if (!win)
		return;

	AbstractPart* part = static_cast<PartMdiView*>(win)->part();
	statusBar()->showMessage(i18n("Preparing printing of %1", part->name()));
	if (part->printView())
		statusBar()->showMessage(i18n("%1 printed", part->name()));
	else
		statusBar()->clearMessage();
}

void MainWin::printPreview() {
	QMdiSubWindow* win = m_mdiArea->currentSubWindow();
	if (!win)
		return;

	AbstractPart* part = static_cast<PartMdiView*>(win)->part();
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
	Workbook* workbook = new Workbook(i18n("Workbook"));
	this->addAspectToProject(workbook);
}

/*!
	adds a new Datapicker to the project.
*/
void MainWin::newDatapicker() {
	Datapicker* datapicker = new Datapicker(i18n("Data Extractor"));
	this->addAspectToProject(datapicker);
}

/*!
	adds a new Spreadsheet to the project.
*/
void MainWin::newSpreadsheet() {
	Spreadsheet* spreadsheet = new Spreadsheet(i18n("Spreadsheet"));

	//if the current active window is a workbook or one of its children,
	//add the new matrix to the workbook
	Workbook* workbook = dynamic_cast<Workbook*>(m_currentAspect);
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

	//if the current active window is a workbook or one of its children,
	//add the new matrix to the workbook
	Workbook* workbook = dynamic_cast<Workbook*>(m_currentAspect);
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
	Worksheet* worksheet = new Worksheet(i18n("Worksheet"));
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
		//check whether one of spreadsheet columns is selected and determine the spreadsheet
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
void MainWin::newCantorWorksheet(QAction* action) {
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

void MainWin::handleCurrentSubWindowChanged(QMdiSubWindow* win) {
	if (!win) {
		updateGUI();
		return;
	}

	auto* view = static_cast<PartMdiView*>(win);
	if (view == m_currentSubWindow) {
		//do nothing, if the current sub-window gets selected again.
		//This event happens, when labplot loses the focus (modal window is opened or the user switches to another application)
		//and gets it back (modal window is closed or the user switches back to labplot).
		return;
	} else
		m_currentSubWindow = view;

	updateGUI();
	if (!m_suppressCurrentSubWindowChangedEvent)
		m_projectExplorer->setCurrentAspect(view->part());
}

void MainWin::handleAspectAdded(const AbstractAspect* aspect) {
	//register the signal-slot connections for aspects having a view.
	//if a folder or a workbook is being added, loop recursively through their children
	//and register the connections.
	const auto* part = dynamic_cast<const AbstractPart*>(aspect);
	if (part) {
// 		connect(part, &AbstractPart::importFromFileRequested, this, &MainWin::importFileDialog);
		connect(part, &AbstractPart::importFromFileRequested, this, [=]() {importFileDialog();});
		connect(part, &AbstractPart::importFromSQLDatabaseRequested, this, &MainWin::importSqlDialog);
		//TODO: export, print and print preview should be handled in the views and not in MainWin.
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
	//no need to react on removal of
	// - AbstractSimpleFilter
	// - columns in the data spreadsheet of a datapicker curve,
	//   this can only happen when changing the error type and is done on the level of DatapickerImage
	if (!aspect->inherits(AspectType::AbstractFilter)
		&& !(parent->parentAspect() && parent->parentAspect()->type() == AspectType::DatapickerCurve) )
		m_projectExplorer->setCurrentAspect(parent);
}

void MainWin::handleAspectAboutToBeRemoved(const AbstractAspect *aspect) {
	const auto* part = dynamic_cast<const AbstractPart*>(aspect);
	if (!part)
		return;

	const auto* workbook = dynamic_cast<const Workbook*>(aspect->parentAspect());
	auto* datapicker = dynamic_cast<const Datapicker*>(aspect->parentAspect());
	if (!datapicker)
		datapicker = dynamic_cast<const Datapicker*>(aspect->parentAspect()->parentAspect());

	if (!workbook && !datapicker) {
		PartMdiView* win = part->mdiSubWindow();
		if (win)
			m_mdiArea->removeSubWindow(win);
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
		updateMdiWindowVisibility();
	}

	m_currentAspect = aspect;

	//activate the corresponding MDI sub window for the current aspect
	activateSubWindowForAspect(aspect);
	m_suppressCurrentSubWindowChangedEvent = false;

	updateGUI();
}

void MainWin::activateSubWindowForAspect(const AbstractAspect* aspect) const {
	Q_ASSERT(aspect);
	const auto* part = dynamic_cast<const AbstractPart*>(aspect);
	if (part) {
		PartMdiView* win{nullptr};

		//for aspects being children of a Workbook, we show workbook's window, otherwise the window of the selected part
		const auto* workbook = dynamic_cast<const Workbook*>(aspect->parentAspect());
		auto* datapicker = dynamic_cast<const Datapicker*>(aspect->parentAspect());
		if (!datapicker)
			datapicker = dynamic_cast<const Datapicker*>(aspect->parentAspect()->parentAspect());

		if (workbook)
			win = workbook->mdiSubWindow();
		else if (datapicker)
			win = datapicker->mdiSubWindow();
		else
			win = part->mdiSubWindow();

		if (m_mdiArea && m_mdiArea->subWindowList().indexOf(win) == -1) {
			if (dynamic_cast<const Note*>(part))
				m_mdiArea->addSubWindow(win, Qt::Tool);
			else
				m_mdiArea->addSubWindow(win);
			win->show();

			//Qt provides its own "system menu" for every sub-window. The shortcut for the close-action
			//in this menu collides with our global m_closeAction.
			//remove the shortcuts in the system menu to avoid this collision.
			QMenu* menu = win->systemMenu();
			if (menu) {
				for (QAction* action : menu->actions())
					action->setShortcut(QKeySequence());
			}
		}
		if (m_mdiArea)
			m_mdiArea->setActiveSubWindow(win);
	} else {
		//activate the mdiView of the parent, if a child was selected
		const AbstractAspect* parent = aspect->parentAspect();
		if (parent) {
			activateSubWindowForAspect(parent);

			//if the parent's parent is a Workbook (a column of a spreadsheet in workbook was selected),
			//we need to select the corresponding tab in WorkbookView too
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

void MainWin::setMdiWindowVisibility(QAction* action) {
	m_project->setMdiWindowVisibility((Project::MdiWindowVisibility)(action->data().toInt()));
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
	//there're already actions available there. Skip the first title-action
	//and insert the action at the beginning of the menu.
	if (menu->actions().size()>1)
		firstAction = menu->actions().at(1);

	menu->insertMenu(firstAction, m_newMenu);

	//The tabbed view collides with the visibility policy for the subwindows.
	//Hide the menus for the visibility policy if the tabbed view is used.
	if (m_mdiArea->viewMode() != QMdiArea::TabbedView) {
		menu->insertSeparator(firstAction);
		menu->insertMenu(firstAction, m_visibilityMenu);
		menu->insertSeparator(firstAction);
	}
}

/*!
	this is called on a right click on a non-root folder in the project explorer
*/
void MainWin::createFolderContextMenu(const Folder*, QMenu* menu) const {
	//Folder provides it's own context menu. Add a separator before adding additional actions.
	menu->addSeparator();
	this->createContextMenu(menu);
}

void MainWin::undo() {
	WAIT_CURSOR;
	m_project->undoStack()->undo();
	if (m_project->undoStack()->index() == 0) {
		m_saveAction->setEnabled(false);
		m_undoAction->setEnabled(false);
		m_project->setChanged(false);
		updateTitleBar();
	}
	m_redoAction->setEnabled(true);
	RESET_CURSOR;
}

void MainWin::redo() {
	WAIT_CURSOR;
	m_project->undoStack()->redo();
	m_project->setChanged(true);
	projectChanged();
	if (m_project->undoStack()->index() == m_project->undoStack()->count())
		m_redoAction->setEnabled(false);
	RESET_CURSOR;
}

/*!
	Shows/hides mdi sub-windows depending on the current visibility policy.
*/
void MainWin::updateMdiWindowVisibility() const {
	auto windows = m_mdiArea->subWindowList();
	switch (m_project->mdiWindowVisibility()) {
	case Project::MdiWindowVisibility::allMdiWindows:
		for (auto* window : windows)
			window->show();

		break;
	case Project::MdiWindowVisibility::folderOnly:
		for (auto* window : windows) {
			auto* view = static_cast<PartMdiView*>(window);
			bool visible = view->part()->folder() == m_currentFolder;
			window->setVisible(visible);
		}
		break;
	case Project::MdiWindowVisibility::folderAndSubfolders:
		for (auto* window : windows) {
			auto* view = static_cast<PartMdiView*>(window);
			bool visible = view->part()->isDescendantOf(m_currentFolder);
			window->setVisible(visible);
		}
		break;
	}
}

void MainWin::toggleDockWidget(QAction* action)  {
	if (action->objectName() == "toggle_project_explorer_dock") {
		if (m_projectExplorerDock->isVisible())
			m_projectExplorerDock->hide();
// 			toggleHideWidget(m_projectExplorerDock, true);
		else
			m_projectExplorerDock->show();
// 			toggleShowWidget(m_projectExplorerDock, true);
	} else if (action->objectName() == "toggle_properties_explorer_dock") {
		if (m_propertiesDock->isVisible())
			m_propertiesDock->hide();
// 			toggleHideWidget(m_propertiesDock, false);
		else
			m_propertiesDock->show();
// 			toggleShowWidget(m_propertiesDock, false);
	}
}

void MainWin::toggleStatusBar(bool checked) {
	statusBar()->setVisible(checked);	// show/hide statusbar
	statusBar()->setEnabled(checked);
	// enabled/disable memory info menu with statusbar
	m_toggleMemoryInfoAction->setEnabled(checked);
}

void MainWin::toggleMemoryInfo() {
	DEBUG(Q_FUNC_INFO)
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
		m_propertiesDock->show();
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
	m_toggleProjectExplorerDockAction->setChecked(visible);
}

void MainWin::propertiesDockVisibilityChanged(bool visible) {
	m_togglePropertiesDockAction->setChecked(visible);
}

void MainWin::cursorDockVisibilityChanged(bool visible) {
	//if the cursor dock was closed, switch to the "Select and Edit" mouse mode
	if (!visible) {
// 		auto* worksheet = activeWorksheet();
		//TODO:
	}
}

void MainWin::cartesianPlotMouseModeChanged(CartesianPlot::MouseMode mode) {
	if (mode != CartesianPlot::MouseMode::Cursor) {
		if (cursorDock)
			cursorDock->hide();
	} else {
		if (!cursorDock) {
			cursorDock = new QDockWidget(i18n("Cursor"), this);
			cursorWidget = new CursorDock(cursorDock);
			cursorDock->setWidget(cursorWidget);
			connect(cursorDock, &QDockWidget::visibilityChanged, this, &MainWin::cursorDockVisibilityChanged);
	// 		cursorDock->setFloating(true);

			// does not work. Don't understand why
	//		if (m_propertiesDock)
	//			tabifyDockWidget(cursorDock, m_propertiesDock);
	//		else
				addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, cursorDock);
		}

		auto* worksheet = static_cast<Worksheet*>(QObject::sender());
		cursorWidget->setWorksheet(worksheet);
		cursorDock->show();
	}
}

void MainWin::toggleFullScreen(bool t) {
	m_toggleFullScreenAction->setFullScreen(this, t);
}

void MainWin::closeEvent(QCloseEvent* event) {
	m_closing = true;
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
		const QString& f = url.toLocalFile();

		bool open = Project::isLabPlotProject(f);
#ifdef HAVE_LIBORIGIN
		if (!open)
			open = OriginProjectParser::isOriginProject(f);
#endif

#ifdef HAVE_CANTOR_LIBS
		if (!open) {
			QFileInfo fi(f);
			open = (fi.completeSuffix() == QLatin1String("cws")) || (fi.completeSuffix() == QLatin1String("ipynb"));
		}
#endif

		if (open)
			openProject(f);
		else {
			if (!m_project)
				newProject();
			importFileDialog(f);
		}

		event->accept();
	} else
		event->ignore();
}

void MainWin::handleSettingsChanges() {
	const KConfigGroup group = KSharedConfig::openConfig()->group("Settings_General");

	//title bar
	MainWin::TitleBarMode titleBarMode = static_cast<MainWin::TitleBarMode>(group.readEntry("TitleBar", 0));
	if (titleBarMode != m_titleBarMode) {
		m_titleBarMode = titleBarMode;
		updateTitleBar();
	}

	//view mode
// 	if (dynamic_cast<QQuickWidget*>(centralWidget()) == nullptr) {
	QMdiArea::ViewMode viewMode = QMdiArea::ViewMode(group.readEntry("ViewMode", 0));
	if (m_mdiArea->viewMode() != viewMode) {
		m_mdiArea->setViewMode(viewMode);
		if (viewMode == QMdiArea::SubWindowView)
			this->updateMdiWindowVisibility();
	}

	if (m_mdiArea->viewMode() == QMdiArea::TabbedView) {
		m_tileWindowsAction->setVisible(false);
		m_cascadeWindowsAction->setVisible(false);
		QTabWidget::TabPosition tabPosition = QTabWidget::TabPosition(group.readEntry("TabPosition", 0));
		if (m_mdiArea->tabPosition() != tabPosition)
			m_mdiArea->setTabPosition(tabPosition);
	} else {
		m_tileWindowsAction->setVisible(true);
		m_cascadeWindowsAction->setVisible(true);
	}
// 	}

	//window visibility
	auto vis = Project::MdiWindowVisibility(group.readEntry("MdiWindowVisibility", 0));
	if (m_project && (vis != m_project->mdiWindowVisibility())) {
		if (vis == Project::MdiWindowVisibility::folderOnly)
			m_visibilityFolderAction->setChecked(true);
		else if (vis == Project::MdiWindowVisibility::folderAndSubfolders)
			m_visibilitySubfolderAction->setChecked(true);
		else
			m_visibilityAllAction->setChecked(true);
		m_project->setMdiWindowVisibility(vis);
	}

	//autosave
	bool autoSave = group.readEntry("AutoSave", 0);
	if (m_autoSaveActive != autoSave) {
		m_autoSaveActive = autoSave;
		if (autoSave)
			m_autoSaveTimer.start();
		else
			m_autoSaveTimer.stop();
	}

	int interval = group.readEntry("AutoSaveInterval", 1);
	interval *= 60*1000;
	if (interval != m_autoSaveTimer.interval())
		m_autoSaveTimer.setInterval(interval);

	//update the locale and the units in the dock widgets
	if (stackedWidget) {
		for (int i = 0; i < stackedWidget->count(); ++i) {
			auto* widget = stackedWidget->widget(i);
			BaseDock* dock = dynamic_cast<BaseDock*>(widget);
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

	//update spreadsheet header
	if (m_project) {
		const auto& spreadsheets = m_project->children<Spreadsheet>(AbstractAspect::ChildIndexFlag::Recursive);
		for (auto* spreadsheet : spreadsheets) {
			spreadsheet->updateHorizontalHeader();
			spreadsheet->updateLocale();
		}
	}

	bool showWelcomeScreen = group.readEntry<bool>(QLatin1String("ShowWelcomeScreen"), true);
	if (m_showWelcomeScreen != showWelcomeScreen)
		m_showWelcomeScreen = showWelcomeScreen;
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

	//disable undo/redo-actions if the history was cleared
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
	if (m_currentAspect->type() == AspectType::Spreadsheet ||
		m_currentAspect->type() == AspectType::Matrix ||
		m_currentAspect->type() == AspectType::Workbook)
		dlg->setCurrentIndex(m_projectExplorer->currentIndex());
	else if (m_currentAspect->type() == AspectType::Column &&
			m_currentAspect->parentAspect()->type() == AspectType::Spreadsheet)
		dlg->setCurrentIndex(m_aspectTreeModel->modelIndexOfAspect(m_currentAspect->parentAspect()));

	if (dlg->exec() == QDialog::Accepted) {
		dlg->importTo(statusBar());
		m_project->setChanged(true);
	}

	delete dlg;
	DEBUG("MainWin::importFileDialog() DONE");
}

void MainWin::importSqlDialog() {
	DEBUG("MainWin::importSqlDialog()");
	auto* dlg = new ImportSQLDatabaseDialog(this);

	// select existing container
	if (m_currentAspect->type() == AspectType::Spreadsheet ||
		m_currentAspect->type() == AspectType::Matrix ||
		m_currentAspect->type() == AspectType::Workbook)
		dlg->setCurrentIndex(m_projectExplorer->currentIndex());
	else if (m_currentAspect->type() == AspectType::Column &&
			m_currentAspect->parentAspect()->type() == AspectType::Spreadsheet)
		dlg->setCurrentIndex(m_aspectTreeModel->modelIndexOfAspect(m_currentAspect->parentAspect()));


	if (dlg->exec() == QDialog::Accepted) {
		dlg->importTo(statusBar());
		m_project->setChanged(true);
	}

	delete dlg;
	DEBUG("MainWin::importSqlDialog() DONE");
}

void MainWin::importProjectDialog() {
	DEBUG("MainWin::importProjectDialog()");

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
	auto* dlg = new ImportDatasetDialog(this);
	if (dlg->exec() == QDialog::Accepted) {
			Spreadsheet* spreadsheet = new Spreadsheet(i18n("Dataset%1", 1));
			auto* dataset = new DatasetHandler(spreadsheet);
			dlg->importToDataset(dataset, statusBar());

			QTimer timer;
			timer.setSingleShot(true);
			QEventLoop loop;
			connect(dataset,  &DatasetHandler::downloadCompleted, &loop, &QEventLoop::quit);
			connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
			timer.start(1500);
			loop.exec();

			if (timer.isActive()) {
				timer.stop();
				addAspectToProject(spreadsheet);
			}
			delete dataset;
	}
	delete dlg;
}

/*!
  opens the dialog for the export of the currently active worksheet, spreadsheet or matrix.
 */
void MainWin::exportDialog() {
	QMdiSubWindow* win = m_mdiArea->currentSubWindow();
	if (!win)
		return;

	AbstractPart* part = static_cast<PartMdiView*>(win)->part();
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
void MainWin::newLiveDataSourceActionTriggered() {
	auto* dlg = new ImportFileDialog(this, true);
	if (dlg->exec() == QDialog::Accepted) {
		if (dlg->sourceType() == LiveDataSource::SourceType::MQTT) {
#ifdef HAVE_MQTT
			auto* mqttClient = new MQTTClient(i18n("MQTT Client%1", 1));
			dlg->importToMQTT(mqttClient);

			//doesn't make sense to have more MQTTClients connected to the same broker
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
				QMessageBox::warning(this, "Warning", "There already is a MQTTClient with this host!");
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
		//doesn't make sense to add a new MQTTClient to an existing MQTTClient or to any of its successors
		QString className = parent->metaObject()->className();
		auto* clientAncestor = parent->ancestor<MQTTClient>();
		if (className == "MQTTClient")
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
	connect (dlg, &SettingsDialog::settingsChanged, this, &MainWin::handleSettingsChanges);
// 	connect (dlg, &SettingsDialog::resetWelcomeScreen, this, &MainWin::resetWelcomeScreen);
	dlg->exec();
}

#ifdef HAVE_CANTOR_LIBS
void MainWin::cantorSettingsDialog() {
	static auto* emptyConfig = new KCoreConfigSkeleton();
	auto* cantorDialog = new KConfigDialog(this,  QLatin1String("Cantor Settings"), emptyConfig);
	for (auto* backend : Cantor::Backend::availableBackends())
		if (backend->config()) //It has something to configure, so add it to the dialog
			cantorDialog->addPage(backend->settingsWidget(cantorDialog), backend->config(), backend->name(),  backend->icon());
	cantorDialog->show();

	DEBUG(Q_FUNC_INFO << ", found " << Cantor::Backend::availableBackends().size() << " backends")
	if (Cantor::Backend::availableBackends().size() == 0)
		KMessageBox::error(nullptr, i18n("No Cantor backends found. Please install the ones you want to use."));

}
#endif
