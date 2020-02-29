/***************************************************************************
    File                 : MainWin.cc
    Project              : LabPlot
    Description          : Main window of the application
    --------------------------------------------------------------------
    Copyright            : (C) 2008-2018 Stefan Gerlach (stefan.gerlach@uni.kn)
    Copyright            : (C) 2009-2020 Alexander Semke (alexander.semke@web.de)

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

#include "kdefrontend/datasources/ImportFileDialog.h"
#include "kdefrontend/datasources/ImportDatasetDialog.h"
#include "kdefrontend/datasources/ImportDatasetWidget.h"
#include "kdefrontend/datasources/ImportProjectDialog.h"
#include "kdefrontend/datasources/ImportSQLDatabaseDialog.h"
#include <kdefrontend/dockwidgets/CursorDock.h>
#include "kdefrontend/dockwidgets/ProjectDock.h"
#include "kdefrontend/HistoryDialog.h"
#include "kdefrontend/SettingsDialog.h"
#include "kdefrontend/GuiObserver.h"
#include "kdefrontend/widgets/FITSHeaderEditDialog.h"
#include "DatasetModel.h"
#include "welcomescreen/WelcomeScreenHelper.h"

#ifdef HAVE_KUSERFEEDBACK
#include <KUserFeedback/ApplicationVersionSource>
#include <KUserFeedback/PlatformInfoSource>
#include <KUserFeedback/QtVersionSource>
#include <KUserFeedback/ScreenInfoSource>
#include <KUserFeedback/StartCountSource>
#include <KUserFeedback/UsageTimeSource>
#endif

#ifdef Q_OS_MAC
#include "3rdparty/kdmactouchbar/src/kdmactouchbar.h"
#endif

#include <QMdiArea>
#include <QMenu>
#include <QDockWidget>
#include <QStackedWidget>
#include <QUndoStack>
#include <QCloseEvent>
#include <QFileDialog>
#include <QMimeData>
#include <QElapsedTimer>
#include <QHash>
#include <QStatusBar>
#include <QTemporaryFile>
#include <QTimeLine>
#include <QtWidgets>
#include <QtQuickWidgets/QQuickWidget>
#include <QQuickItem>
#include <QQuickView>
#include <QQmlApplicationEngine>
#include <QQmlContext>

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
#include <kconfigwidgets_version.h>

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

	//restore the geometry
	KConfigGroup group = KSharedConfig::openConfig()->group("MainWin");
	restoreGeometry(group.readEntry("geometry", QByteArray()));
}

MainWin::~MainWin() {
	//save the recent opened files
	m_recentProjectsAction->saveEntries( KSharedConfig::openConfig()->group("Recent Files") );
	KConfigGroup group = KSharedConfig::openConfig()->group("MainWin");
	group.writeEntry("geometry", saveGeometry());
	KSharedConfig::openConfig()->sync();

	//if welcome screen is shown, save its settings prior to deleting it
	if(dynamic_cast<QQuickWidget*>(centralWidget()))
		QMetaObject::invokeMethod(m_welcomeWidget->rootObject(), "saveWidgetDimensions");

	if (m_project) {

		if(dynamic_cast<QQuickWidget*>(centralWidget()) == nullptr)
			m_mdiArea->closeAllSubWindows();

		disconnect(m_project, nullptr, this, nullptr);
		delete m_project;
	}

	if (m_aspectTreeModel)
		delete m_aspectTreeModel;

	if (m_guiObserver)
		delete m_guiObserver;

	if(m_welcomeScreenHelper)
		delete m_welcomeScreenHelper;
}

void MainWin::showPresenter() {
	const Worksheet* w = dynamic_cast<Worksheet*>(m_currentAspect);
	if (w) {
		auto* view = dynamic_cast<WorksheetView*>(w->view());
		view->presenterMode();
	} else {
		//currently active object is not a worksheet but we're asked to start in the presenter mode
		//determine the first available worksheet and show it in the presenter mode
		QVector<Worksheet*> worksheets = m_project->children<Worksheet>();
		if (worksheets.size() > 0) {
			auto* view = qobject_cast<WorksheetView*>(worksheets.first()->view());
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
	statusBar()->showMessage(i18nc("%1 is the LabPlot version", "Welcome to LabPlot %1", QLatin1String(LVERSION)));

	initActions();

#ifdef Q_OS_MAC
	setupGUI(Default, QLatin1String("/Applications/labplot2.app/Contents/Resources/labplot2ui.rc"));
	m_touchBar = new KDMacTouchBar(this);
	//m_touchBar->setTouchButtonStyle(KDMacTouchBar::IconOnly);
#else
	setupGUI(Default, KXMLGUIClient::xmlFile());	// should be "labplot2ui.rc"
#endif

	DEBUG("Component name: " << KXMLGUIClient::componentName().toStdString());
	DEBUG("XML file: " << KXMLGUIClient::xmlFile().toStdString() << " (should be \"labplot2ui.rc\")");

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
		for (auto* container : factory()->containers(QLatin1String("ToolBar"))) {
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
		return;
	}

	auto* tbImport = new QToolButton(mainToolBar);
	tbImport->setPopupMode(QToolButton::MenuButtonPopup);
	tbImport->setMenu(m_importMenu);
	tbImport->setDefaultAction(m_importFileAction);
	mainToolBar->addWidget(tbImport);

	qobject_cast<QMenu*>(factory()->container("import", this))->setIcon(QIcon::fromTheme("document-import"));
	setWindowIcon(QIcon::fromTheme("LabPlot2", QGuiApplication::windowIcon()));
	setAttribute( Qt::WA_DeleteOnClose );

	//make the status bar of a fixed size in order to avoid height changes when placing a ProgressBar there.
	QFont font;
	font.setFamily(font.defaultFamily());
	QFontMetrics fm(font);
	statusBar()->setFixedHeight(fm.height() + 5);

	//load recently used projects
	m_recentProjectsAction->loadEntries( KSharedConfig::openConfig()->group("Recent Files") );


	//auto-save
	KConfigGroup group = KSharedConfig::openConfig()->group("Settings_General");
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
		LoadOnStart load = (LoadOnStart)group.readEntry("LoadOnStart", (int)Nothing);
		if (load != WelcomeScreen) {
			createMdiArea();
			setCentralWidget(m_mdiArea);

			if (load == NewProject)   //create new project
				newProject();
			else if (load == NewProjectWorksheet) { //create new project with a worksheet
				newProject();
				newWorksheet();
			} else if (load == LastProject) { //open last used project
				if (!m_recentProjectsAction->urls().isEmpty()) {
					QDEBUG("TO OPEN m_recentProjectsAction->urls() =" << m_recentProjectsAction->urls().constFirst());
					openRecentProject( m_recentProjectsAction->urls().constFirst() );
				}
			}

			updateGUIOnProjectChanges();
		} else{ //welcome screen
			m_showWelcomeScreen = true;
			m_welcomeWidget = createWelcomeScreen();
			setCentralWidget(m_welcomeWidget);
		}
	}

	//show memory info
	const bool showMemoryInfo = group.readEntry(QLatin1String("ShowMemoryInfo"), true);
	if (showMemoryInfo) {
		m_memoryInfoWidget = new MemoryWidget(statusBar());
		statusBar()->addPermanentWidget(m_memoryInfoWidget);
	}
}

/**
 * @brief Creates a new welcome screen to be set as central widget.
 */
QQuickWidget* MainWin::createWelcomeScreen() {
	QSize maxSize = qApp->primaryScreen()->availableSize();
	resize(maxSize);
	setMinimumSize(700, 400);
	showMaximized();

	KToolBar* toolbar = toolBar();
	if(toolbar)
		toolbar->setVisible(false);

	QList<QVariant> recentList;
	for (QUrl url : m_recentProjectsAction->urls())
		recentList.append(QVariant(url));

	//Set context property
	QQuickWidget* quickWidget = new QQuickWidget(this);
	QQmlContext *ctxt = quickWidget->rootContext();
	QVariant variant(recentList);
	ctxt->setContextProperty("recentProjects", variant);

	//Create helper object
	if(m_welcomeScreenHelper)
		delete m_welcomeScreenHelper;
	m_welcomeScreenHelper = new WelcomeScreenHelper();
	connect(m_welcomeScreenHelper, SIGNAL(openExampleProject(QString)), this, SLOT(openProject(const QString& )));

	ctxt->setContextProperty("datasetModel", m_welcomeScreenHelper->getDatasetModel());
	ctxt->setContextProperty("helper", m_welcomeScreenHelper);

	quickWidget->setSource(QUrl(QLatin1String("qrc:///main.qml")));
	quickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
	QObject *item = quickWidget->rootObject();

	//connect qml's signals
	QObject::connect(item, SIGNAL(recentProjectClicked(QUrl)), this, SLOT(openRecentProject(QUrl)));
	QObject::connect(item, SIGNAL(datasetClicked(QString, QString, QString)), m_welcomeScreenHelper, SLOT(datasetClicked(const QString&, const QString&, const QString&)));
	QObject::connect(item, SIGNAL(openDataset()), this, SLOT(openDatasetExample()));
	QObject::connect(item, SIGNAL(openExampleProject(QString)), m_welcomeScreenHelper, SLOT(exampleProjectClicked(const QString&)));
	m_welcomeScreenHelper->showFirstDataset();

	return quickWidget;
}

/**
 * @brief Initiates resetting the layout of the welcome screen
 */
void MainWin::resetWelcomeScreen() {
	if(dynamic_cast<QQuickWidget*>(centralWidget()))
		QMetaObject::invokeMethod(m_welcomeWidget->rootObject(), "restoreOriginalLayout");
}

/**
 * @brief Creates a new MDI area, to replace the Welcome Screen as central widget
 */
void MainWin::createMdiArea() {
	KToolBar* toolbar = toolBar();
	if(toolbar)
		toolbar->setVisible(true);

	//Save welcome screen's dimensions.
	if(m_showWelcomeScreen)
		QMetaObject::invokeMethod(m_welcomeWidget->rootObject(), "saveWidgetDimensions");

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
	m_newProjectAction = KStandardAction::openNew(this, SLOT(newProject()),actionCollection());
	m_openProjectAction = KStandardAction::open(this, SLOT(openProject()),actionCollection());
	m_recentProjectsAction = KStandardAction::openRecent(this, SLOT(openRecentProject(QUrl)),actionCollection());
	m_closeAction = KStandardAction::close(this, SLOT(closeProject()),actionCollection());
	actionCollection()->setDefaultShortcut(m_closeAction, QKeySequence()); //remove the shortcut, QKeySequence::Close will be used for closing sub-windows
	m_saveAction = KStandardAction::save(this, SLOT(saveProject()),actionCollection());
	m_saveAsAction = KStandardAction::saveAs(this, SLOT(saveProjectAs()),actionCollection());
	m_printAction = KStandardAction::print(this, SLOT(print()),actionCollection());
	m_printPreviewAction = KStandardAction::printPreview(this, SLOT(printPreview()),actionCollection());

    //TODO: on Mac OS when going full-screen we get a crash because of an stack-overflow
#ifndef Q_OS_MAC
    KStandardAction::fullScreen(this, SLOT(toggleFullScreen()), this, actionCollection());
#endif

	//New Folder/Workbook/Spreadsheet/Matrix/Worksheet/Datasources
	m_newWorkbookAction = new QAction(QIcon::fromTheme("labplot-workbook-new"),i18n("Workbook"),this);
	actionCollection()->addAction("new_workbook", m_newWorkbookAction);
	m_newWorkbookAction->setWhatsThis(i18n("Creates a new workbook for collection spreadsheets, matrices and plots"));
	connect(m_newWorkbookAction, &QAction::triggered, this, &MainWin::newWorkbook);

	m_newDatapickerAction = new QAction(QIcon::fromTheme("color-picker-black"), i18n("Datapicker"), this);
	m_newDatapickerAction->setWhatsThis(i18n("Creates a data picker for getting data from a picture"));
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
	m_newLiveDataSourceAction = new QAction(QIcon::fromTheme("application-octet-stream"),i18n("Live Data Source"),this);
	m_newLiveDataSourceAction->setWhatsThis(i18n("Creates a live data source to read data from a real time device"));
	actionCollection()->addAction("new_live_datasource", m_newLiveDataSourceAction);
	connect(m_newLiveDataSourceAction, &QAction::triggered, this, &MainWin::newLiveDataSourceActionTriggered);

	//Import/Export
	m_importFileAction = new QAction(QIcon::fromTheme("document-import"), i18n("Import from File"), this);
	actionCollection()->setDefaultShortcut(m_importFileAction, Qt::CTRL+Qt::SHIFT+Qt::Key_I);
	m_importFileAction->setWhatsThis(i18n("Import data from a regular file"));
	actionCollection()->addAction("import_file", m_importFileAction);
	connect(m_importFileAction, &QAction::triggered, this, [=]() {importFileDialog();});

	m_importSqlAction = new QAction(QIcon::fromTheme("network-server-database"), i18n("From SQL Database"), this);
	m_importSqlAction->setWhatsThis(i18n("Import data from a SQL database"));
	actionCollection()->addAction("import_sql", m_importSqlAction);
	connect(m_importSqlAction, &QAction::triggered, this, &MainWin::importSqlDialog);

	m_importDatasetAction = new QAction(QIcon::fromTheme(QLatin1String("database-index")), i18n("From Dataset Collection"), this);
	m_importDatasetAction->setWhatsThis(i18n("Imports data from an online dataset"));
	actionCollection()->addAction("import_dataset_datasource", m_importDatasetAction);
	connect(m_importDatasetAction, &QAction::triggered, this, &MainWin::importDatasetDialog);

	m_importLabPlotAction = new QAction(QIcon::fromTheme("project-open"), i18n("LabPlot Project"), this);
	m_importLabPlotAction->setWhatsThis(i18n("Import a project from a LabPlot project file (.lml)"));
	actionCollection()->addAction("import_labplot", m_importLabPlotAction);
	connect(m_importLabPlotAction, &QAction::triggered, this, &MainWin::importProjectDialog);

#ifdef HAVE_LIBORIGIN
	m_importOpjAction = new QAction(QIcon::fromTheme("project-open"), i18n("Origin Project (OPJ)"), this);
	m_importOpjAction->setWhatsThis(i18n("Import a project from an OriginLab Origin project file (.opj)"));
	actionCollection()->addAction("import_opj", m_importOpjAction);
	connect(m_importOpjAction, &QAction::triggered, this, &MainWin::importProjectDialog);
#endif

	m_exportAction = new QAction(QIcon::fromTheme("document-export"), i18n("Export"), this);
	m_exportAction->setWhatsThis(i18n("Export selected element"));
	actionCollection()->setDefaultShortcut(m_exportAction, Qt::CTRL+Qt::SHIFT+Qt::Key_E);
	actionCollection()->addAction("export", m_exportAction);
	connect(m_exportAction, &QAction::triggered, this, &MainWin::exportDialog);

	m_editFitsFileAction = new QAction(QIcon::fromTheme("editor"), i18n("FITS Metadata Editor"), this);
	m_editFitsFileAction->setWhatsThis(i18n("Open editor to edit FITS meta data"));
	actionCollection()->addAction("edit_fits", m_editFitsFileAction);
	connect(m_editFitsFileAction, &QAction::triggered, this, &MainWin::editFitsFileDialog);

	// Edit
	//Undo/Redo-stuff
	m_undoAction = KStandardAction::undo(this, SLOT(undo()), actionCollection());
	m_redoAction = KStandardAction::redo(this, SLOT(redo()), actionCollection());
	m_historyAction = new QAction(QIcon::fromTheme("view-history"), i18n("Undo/Redo History"),this);
	actionCollection()->addAction("history", m_historyAction);
	connect(m_historyAction, &QAction::triggered, this, &MainWin::historyDialog);

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

	//"Standard actions"
	KStandardAction::preferences(this, SLOT(settingsDialog()), actionCollection());
	KStandardAction::quit(this, SLOT(close()), actionCollection());

	//Actions for window visibility
	auto* windowVisibilityActions = new QActionGroup(this);
	windowVisibilityActions->setExclusive(true);

	m_visibilityFolderAction = new QAction(QIcon::fromTheme("folder"), i18n("Current &Folder Only"), windowVisibilityActions);
	m_visibilityFolderAction->setCheckable(true);
	m_visibilityFolderAction->setData(Project::folderOnly);

	m_visibilitySubfolderAction = new QAction(QIcon::fromTheme("folder-documents"), i18n("Current Folder and &Subfolders"), windowVisibilityActions);
	m_visibilitySubfolderAction->setCheckable(true);
	m_visibilitySubfolderAction->setData(Project::folderAndSubfolders);

	m_visibilityAllAction = new QAction(i18n("&All"), windowVisibilityActions);
	m_visibilityAllAction->setCheckable(true);
	m_visibilityAllAction->setData(Project::allMdiWindows);

	connect(windowVisibilityActions, &QActionGroup::triggered, this, &MainWin::setMdiWindowVisibility);

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
	QAction* searchAction = new QAction(actionCollection());
	searchAction->setShortcut(QKeySequence::Find);
	connect(searchAction, &QAction::triggered, this, [=]() {
		if (m_project) {
			if (!m_projectExplorerDock->isVisible()) {
				m_toggleProjectExplorerDockAction->setChecked(true);
				toggleDockWidget(m_toggleProjectExplorerDockAction);
			}
			m_projectExplorer->search();
		}
	});
	this->addAction(searchAction);
}

void MainWin::initMenus() {
	//menu in the main toolbar for adding new aspects
	auto* menu = dynamic_cast<QMenu*>(factory()->container("new", this));
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
	m_importMenu ->addAction(m_importFileAction);
	m_importMenu ->addAction(m_importSqlAction);
	m_importMenu->addAction(m_importDatasetAction);
	m_importMenu->addSeparator();
	m_importMenu->addAction(m_importLabPlotAction);
#ifdef HAVE_LIBORIGIN
	m_importMenu ->addAction(m_importOpjAction);
#endif

#ifdef HAVE_CANTOR_LIBS
	m_newMenu->addSeparator();
	m_newCantorWorksheetMenu = new QMenu(i18n("CAS Worksheet"), this);
	m_newCantorWorksheetMenu->setIcon(QIcon::fromTheme("archive-insert"));

	//"Adding Cantor backends to menu and context menu"
	QStringList m_availableBackend = Cantor::Backend::listAvailableBackends();
	if (m_availableBackend.count() > 0) {
		unplugActionList(QLatin1String("backends_list"));
		QList<QAction*> newBackendActions;
		for (Cantor::Backend* backend : Cantor::Backend::availableBackends()) {
			if (!backend->isEnabled()) continue;
			QAction* action = new QAction(QIcon::fromTheme(backend->icon()), backend->name(),this);
			action->setData(backend->name());
			newBackendActions << action;
			m_newCantorWorksheetMenu->addAction(action);
		}

		connect(m_newCantorWorksheetMenu, &QMenu::triggered, this, &MainWin::newCantorWorksheet);
		plugActionList(QLatin1String("backends_list"), newBackendActions);
	}
	m_newMenu->addMenu(m_newCantorWorksheetMenu);
#else
	delete this->guiFactory()->container("cas_worksheet", this);
	delete this->guiFactory()->container("new_cas_worksheet", this);
	delete this->guiFactory()->container("cas_worksheet_toolbar", this);
#endif

	//menu subwindow visibility policy
	m_visibilityMenu = new QMenu(i18n("Window Visibility Policy"), this);
	m_visibilityMenu->setIcon(QIcon::fromTheme("window-duplicate"));
	m_visibilityMenu ->addAction(m_visibilityFolderAction);
	m_visibilityMenu ->addAction(m_visibilitySubfolderAction);
	m_visibilityMenu ->addAction(m_visibilityAllAction);

	//menu for editing files
	m_editMenu = new QMenu(i18n("Edit"), this);
	m_editMenu->addAction(m_editFitsFileAction);

	//set the action for the current color scheme checked
	KConfigGroup group = KSharedConfig::openConfig()->group(QLatin1String("Settings_General"));
#if KCONFIGWIDGETS_VERSION > QT_VERSION_CHECK(5,67,0)
	// Since version 5.67 KColorSchemeManager has a system default option
	QString schemeName = group.readEntry("ColorScheme");
#else
	KConfigGroup generalGlobalsGroup = KSharedConfig::openConfig(QLatin1String("kdeglobals"))->group("General");
	QString defaultSchemeName = generalGlobalsGroup.readEntry("ColorScheme", QStringLiteral("Breeze"));
	QString schemeName = group.readEntry("ColorScheme", defaultSchemeName);
#endif
	KActionMenu* schemesMenu = m_schemeManager->createSchemeSelectionMenu(i18n("Color Scheme"), schemeName, this);
	schemesMenu->setIcon(QIcon::fromTheme(QStringLiteral("preferences-desktop-color")));

	QMenu* settingsMenu = dynamic_cast<QMenu*>(factory()->container("settings", this));
	if (settingsMenu)
		settingsMenu->insertMenu(settingsMenu->actions().constFirst(), schemesMenu->menu());

	connect(schemesMenu->menu(), &QMenu::triggered, this, &MainWin::colorSchemeChanged);

#ifdef HAVE_CANTOR_LIBS
	QAction* action = new QAction(QIcon::fromTheme(QLatin1String("cantor")), i18n("Configure CAS"), this);
	connect(action, &QAction::triggered, this, &MainWin::cantorSettingsDialog);
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
	bool b = (m_project == nullptr);
	m_saveAction->setEnabled(!b);
	m_saveAsAction->setEnabled(!b);
	m_printAction->setEnabled(!b);
	m_printPreviewAction->setEnabled(!b);
	m_importFileAction->setEnabled(!b);
	m_importSqlAction->setEnabled(!b);
#ifdef HAVE_LIBORIGIN
	m_importOpjAction->setEnabled(!b);
#endif
	m_exportAction->setEnabled(!b);
	m_newWorkbookAction->setEnabled(!b);
	m_newSpreadsheetAction->setEnabled(!b);
	m_newMatrixAction->setEnabled(!b);
	m_newWorksheetAction->setEnabled(!b);
	m_newDatapickerAction->setEnabled(!b);
	m_closeAction->setEnabled(!b);
	m_toggleProjectExplorerDockAction->setEnabled(!b);
	m_togglePropertiesDockAction->setEnabled(!b);

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
		factory->container("cas_worksheet", this)->setEnabled(false);
		factory->container("cas_worksheet_toolbar", this)->hide();
#endif
	}

	factory->container("new", this)->setEnabled(!b);
	factory->container("edit", this)->setEnabled(!b);
	factory->container("import", this)->setEnabled(!b);

	if (b)
		setCaption("LabPlot2");
	else
		setCaption(m_project->name());

#ifdef Q_OS_MAC
	m_touchBar->clear();

	if (b){
		m_touchBar->addAction(m_newProjectAction);
		m_touchBar->addAction(m_openProjectAction);
	} else {
		m_touchBar->addAction(m_importFileAction);
		m_touchBar->addAction(m_newWorksheetAction);
		m_touchBar->addAction(m_newSpreadsheetAction);
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
	if (m_project == nullptr || m_project->isLoading())
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
#ifdef Q_OS_MAC
	m_touchBar->clear();

	m_touchBar->addAction(m_undoAction);
	m_touchBar->addAction(m_redoAction);
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
		factory->container("cas_worksheet", this)->setEnabled(false);
		factory->container("cas_worksheet_toolbar", this)->hide();
#endif
		return;
	}


#ifdef Q_OS_MAC
	if (dynamic_cast<Folder*>(m_currentAspect)) {
	//if (m_currentAspect->type() == AspectType::Folder) {
		m_touchBar->addAction(m_newWorksheetAction);
		m_touchBar->addAction(m_newSpreadsheetAction);
		m_touchBar->addAction(m_newMatrixAction);
	}
#endif

	//Handle the Worksheet-object
	const Worksheet* w = dynamic_cast<Worksheet*>(m_currentAspect);
	if (!w)
		w = dynamic_cast<Worksheet*>(m_currentAspect->parent(AspectType::Worksheet));
	if (w) {
		//populate worksheet menu
		auto* view = qobject_cast<WorksheetView*>(w->view());
		auto* menu = qobject_cast<QMenu*>(factory->container("worksheet", this));
		menu->clear();
		view->createContextMenu(menu);
		menu->setEnabled(true);

		//populate analysis menu
		menu = qobject_cast<QMenu*>(factory->container("analysis", this));
		menu->clear();
		view->createAnalysisMenu(menu);
		menu->setEnabled(true);

		//populate worksheet-toolbar
		auto* toolbar = qobject_cast<QToolBar*>(factory->container("worksheet_toolbar", this));
		toolbar->clear();
		view->fillToolBar(toolbar);
		toolbar->setVisible(true);
		toolbar->setEnabled(true);

		//populate the toolbar for cartesian plots
		toolbar = qobject_cast<QToolBar*>(factory->container("cartesian_plot_toolbar", this));
		toolbar->clear();
		view->fillCartesianPlotToolBar(toolbar);
		toolbar->setVisible(true);
		toolbar->setEnabled(true);

		//populate the touchbar on Mac
#ifdef Q_OS_MAC
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
		//populate spreadsheet-menu
		auto* view = qobject_cast<SpreadsheetView*>(spreadsheet->view());
		auto* menu = qobject_cast<QMenu*>(factory->container("spreadsheet", this));
		menu->clear();
		view->createContextMenu(menu);
		menu->setEnabled(true);

		//populate spreadsheet-toolbar
		auto* toolbar = qobject_cast<QToolBar*>(factory->container("spreadsheet_toolbar", this));
		toolbar->clear();
		view->fillToolBar(toolbar);
		toolbar->setVisible(true);
		toolbar->setEnabled(true);

		//populate the touchbar on Mac
#ifdef Q_OS_MAC
		m_touchBar->addAction(m_importFileAction);
		view->fillTouchBar(m_touchBar);
#endif
	} else {
		factory->container("spreadsheet", this)->setEnabled(false);
		factory->container("spreadsheet_toolbar", this)->setVisible(false);
	}

	//Handle the Matrix-object
	const  Matrix* matrix = dynamic_cast<Matrix*>(m_currentAspect);
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
#ifdef Q_OS_MAC
		m_touchBar->addAction(m_importFileAction);
		//view->fillTouchBar(m_touchBar);
#endif
	} else
		factory->container("matrix", this)->setEnabled(false);

#ifdef HAVE_CANTOR_LIBS
	const CantorWorksheet* cantorworksheet = dynamic_cast<CantorWorksheet*>(m_currentAspect);
	if (!cantorworksheet)
		cantorworksheet = dynamic_cast<CantorWorksheet*>(m_currentAspect->parent(AspectType::CantorWorksheet));
	if (cantorworksheet) {
		auto* view = qobject_cast<CantorWorksheetView*>(cantorworksheet->view());
		auto* menu = qobject_cast<QMenu*>(factory->container("cas_worksheet", this));
		menu->clear();
		view->createContextMenu(menu);
		menu->setEnabled(true);

		auto* toolbar = qobject_cast<QToolBar*>(factory->container("cas_worksheet_toolbar", this));
		toolbar->setVisible(true);
		toolbar->clear();
		view->fillToolBar(toolbar);
	} else {
		//no Cantor worksheet selected -> deactivate Cantor worksheet related menu and toolbar
		factory->container("cas_worksheet", this)->setEnabled(false);
		factory->container("cas_worksheet_toolbar", this)->setVisible(false);
	}
#endif

	const Datapicker* datapicker = dynamic_cast<Datapicker*>(m_currentAspect);
	if (!datapicker)
		datapicker = dynamic_cast<Datapicker*>(m_currentAspect->parent(AspectType::Datapicker));
	if (!datapicker) {
		if (m_currentAspect->type() == AspectType::DatapickerCurve)
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

	if(dynamic_cast<QQuickWidget*>(centralWidget())) {
		createMdiArea();
		setCentralWidget(m_mdiArea);
	}

	QApplication::processEvents(QEventLoop::AllEvents, 100);

	if (m_project)
		delete m_project;

	if (m_aspectTreeModel)
		delete m_aspectTreeModel;

	m_project = new Project();
	m_currentAspect = m_project;
	m_currentFolder = m_project;

	KConfigGroup group = KSharedConfig::openConfig()->group( "Settings_General" );
	Project::MdiWindowVisibility vis = Project::MdiWindowVisibility(group.readEntry("MdiWindowVisibility", 0));
	m_project->setMdiWindowVisibility( vis );
	if (vis == Project::folderOnly)
		m_visibilityFolderAction->setChecked(true);
	else if (vis == Project::folderAndSubfolders)
		m_visibilitySubfolderAction->setChecked(true);
	else
		m_visibilityAllAction->setChecked(true);

	m_aspectTreeModel = new AspectTreeModel(m_project, this);
	connect(m_aspectTreeModel, &AspectTreeModel::statusInfo, [=](const QString& text){ statusBar()->showMessage(text); });

	//newProject is called for the first time, there is no project explorer yet
	//-> initialize the project explorer,  the GUI-observer and the dock widgets.
	if (m_projectExplorer == nullptr) {
		m_projectExplorerDock = new QDockWidget(this);
		m_projectExplorerDock->setObjectName("projectexplorer");
		m_projectExplorerDock->setWindowTitle(i18nc("@title:window", "Project Explorer"));
		addDockWidget(Qt::LeftDockWidgetArea, m_projectExplorerDock);

		m_projectExplorer = new ProjectExplorer(m_projectExplorerDock);
		m_projectExplorerDock->setWidget(m_projectExplorer);

		connect(m_projectExplorer, &ProjectExplorer::currentAspectChanged, this, &MainWin::handleCurrentAspectChanged);
		connect(m_projectExplorerDock, &QDockWidget::visibilityChanged, this, &MainWin::projectExplorerDockVisibilityChanged);

		//Properties dock
		m_propertiesDock = new QDockWidget(this);
		m_propertiesDock->setObjectName("aspect_properties_dock");
		m_propertiesDock->setWindowTitle(i18nc("@title:window", "Properties"));
		addDockWidget(Qt::RightDockWidgetArea, m_propertiesDock);

		auto* sa = new QScrollArea(m_propertiesDock);
		stackedWidget = new QStackedWidget(sa);
		sa->setWidget(stackedWidget);
		sa->setWidgetResizable(true);
		m_propertiesDock->setWidget(sa);

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
	setCaption(m_project->name());

	return true;
}

void MainWin::openProject() {
	KConfigGroup conf(KSharedConfig::openConfig(), "MainWin");
	const QString& dir = conf.readEntry("LastOpenDir", "");

	QString extensions = i18n("LabPlot Projects (%1)", Project::supportedExtensions());
#ifdef HAVE_LIBORIGIN
	extensions += i18n(";;Origin Projects (%1)", OriginProjectParser::supportedExtensions());
#endif

#ifdef HAVE_CANTOR_LIBS
	extensions += i18n(";;Cantor Projects (.cws)");
	extensions += i18n(";;Jupyter Notebooks (.ipynb)");
#endif

	const QString& path = QFileDialog::getOpenFileName(this,i18n("Open Project"), dir, extensions);
	if (path.isEmpty())// "Cancel" was clicked
		return;

	this->openProject(path);

	//save new "last open directory"
	int pos = path.lastIndexOf(QDir::separator());
	if (pos != -1) {
		const QString& newDir = path.left(pos);
		if (newDir != dir)
			conf.writeEntry("LastOpenDir", newDir);
	}
}

void MainWin::openProject(const QString& filename) {
	if (filename == m_currentFileName) {
		KMessageBox::information(this, i18n("The project file %1 is already opened.", filename), i18n("Open Project"));
		return;
	}

	if(dynamic_cast<QQuickWidget*>(centralWidget())) {
		createMdiArea();
		setCentralWidget(m_mdiArea);
	}

	if (!newProject())
		return;

	WAIT_CURSOR;
	QElapsedTimer timer;
	timer.start();
	bool rc = false;
	if (Project::isLabPlotProject(filename)) {
		m_project->setFileName(filename);
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

					file.open(QIODevice::ReadOnly);
					QByteArray content = file.readAll();
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

	m_currentFileName = filename;
	m_project->undoStack()->clear();
	m_undoViewEmptyLabel = i18n("%1: opened", m_project->name());
	m_recentProjectsAction->addUrl( QUrl(filename) );
	setCaption(m_project->name());
	updateGUIOnProjectChanges();
	updateGUI(); //there are most probably worksheets or spreadsheets in the open project -> update the GUI
	m_saveAction->setEnabled(false);

	statusBar()->showMessage( i18n("Project successfully opened (in %1 seconds).", (float)timer.elapsed()/1000) );

	if (m_autoSaveActive)
		m_autoSaveTimer.start();

	RESET_CURSOR;
}

void MainWin::openRecentProject(const QUrl& url) {
	if(dynamic_cast<QQuickWidget*>(centralWidget())) {
		createMdiArea();
		setCentralWidget(m_mdiArea);
	}

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

	if(!m_closing) {
		if(dynamic_cast<QQuickWidget*>(centralWidget()) && m_showWelcomeScreen) {
			m_welcomeWidget = createWelcomeScreen();
			setCentralWidget(m_welcomeWidget);
		}
	}

	m_projectClosing = true;
	statusBar()->clearMessage();
	delete m_aspectTreeModel;
	m_aspectTreeModel = nullptr;
	delete m_project;
	m_project = nullptr;
	m_currentFileName.clear();
	m_projectClosing = false;

	//update the UI if we're just closing a project
	//and not closing(quitting) the application
	if (!m_closing) {
		m_projectExplorerDock->hide();
		m_propertiesDock->hide();
		m_currentAspect = nullptr;
		m_currentFolder = nullptr;
		updateGUIOnProjectChanges();

		if (m_autoSaveActive)
			m_autoSaveTimer.stop();
	}

	removeDockWidget(cursorDock);
	delete cursorDock;
	cursorDock = nullptr;
	cursorWidget = nullptr; // is deleted, because it's the cild of cursorDock
	return true;
}

bool MainWin::saveProject() {
	const QString& fileName = m_project->fileName();
	if (fileName.isEmpty())
		return saveProjectAs();
	else
		return save(fileName);
}

bool MainWin::saveProjectAs() {
	KConfigGroup conf(KSharedConfig::openConfig(), "MainWin");
	const QString& dir = conf.readEntry("LastOpenDir", "");
	QString path  = QFileDialog::getSaveFileName(this, i18n("Save Project As"), dir,
	                i18n("LabPlot Projects (*.lml *.lml.gz *.lml.bz2 *.lml.xz *.LML *.LML.GZ *.LML.BZ2 *.LML.XZ)"));

	if (path.isEmpty())// "Cancel" was clicked
		return false;

	if (path.contains(QLatin1String(".lml"), Qt::CaseInsensitive) == false)
		path.append(QLatin1String(".lml"));

	//save new "last open directory"
	int pos = path.lastIndexOf(QDir::separator());
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
	QTemporaryFile tempFile(QDir::tempPath() + "/" + QLatin1String("labplot_save_XXXXXX"));
	if (!tempFile.open()) {
		KMessageBox::error(this, i18n("Couldn't open the temporary file for writing."));
		return false;
	}

	WAIT_CURSOR;
	const QString& tempFileName = tempFile.fileName();
	DEBUG("Using temporary file " << tempFileName.toStdString())
	tempFile.close();

	// use file ending to find out how to compress file
	QIODevice* file;
	// if ending is .lml, do gzip compression anyway
	if (fileName.endsWith(QLatin1String(".lml")))
		file = new KCompressionDevice(tempFileName, KCompressionDevice::GZip);
	else
		file = new KFilterDev(tempFileName);

	if (file == nullptr)
		file = new QFile(tempFileName);

	bool ok;
	if (file->open(QIODevice::WriteOnly)) {
		m_project->setFileName(fileName);

		QPixmap thumbnail = centralWidget()->grab();

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
			setCaption(m_project->name());
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

/*!
	prints the current sheet (worksheet, spreadsheet or matrix)
*/
void MainWin::print() {
	QMdiSubWindow* win = m_mdiArea->currentSubWindow();
	if (!win)
		return;

	AbstractPart* part = dynamic_cast<PartMdiView*>(win)->part();
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

	AbstractPart* part = dynamic_cast<PartMdiView*>(win)->part();
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
	Datapicker* datapicker = new Datapicker(i18n("Datapicker"));
	this->addAspectToProject(datapicker);
}
/*!
	adds a new Spreadsheet to the project.
*/
void MainWin::newSpreadsheet() {
	Spreadsheet* spreadsheet = new Spreadsheet(i18n("Spreadsheet"));

	//if the current active window is a workbook and no folder/project is selected in the project explorer,
	//add the new spreadsheet to the workbook
	Workbook* workbook = dynamic_cast<Workbook*>(m_currentAspect);
	if (workbook) {
		QModelIndex index = m_projectExplorer->currentIndex();
		const auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
		if (!aspect->inherits(AspectType::Folder)) {
			workbook->addChild(spreadsheet);
			return;
		}
	}

	this->addAspectToProject(spreadsheet);
}

/*!
	adds a new Matrix to the project.
*/
void MainWin::newMatrix() {
	Matrix* matrix = new Matrix(i18n("Matrix"));

	//if the current active window is a workbook and no folder/project is selected in the project explorer,
	//add the new matrix to the workbook
	Workbook* workbook = dynamic_cast<Workbook*>(m_currentAspect);
	if (workbook) {
		QModelIndex index = m_projectExplorer->currentIndex();
		const auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
		if (!aspect->inherits(AspectType::Folder)) {
			workbook->addChild(matrix);
			return;
		}
	}

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
	if(dynamic_cast<QQuickWidget*>(centralWidget()) != nullptr) {
		return nullptr;
	}

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
	CantorWorksheet* cantorworksheet = new CantorWorksheet(action->data().toString());
	this->addAspectToProject(cantorworksheet);
}

/********************************************************************************/
#endif

/*!
	called if there were changes in the project.
	Adds "changed" to the window caption and activates the save-Action.
*/
void MainWin::projectChanged() {
	setCaption(i18n("%1    [Changed]", m_project->name()));
	m_saveAction->setEnabled(true);
	m_undoAction->setEnabled(true);
	return;
}

void MainWin::handleCurrentSubWindowChanged(QMdiSubWindow* win) {
	if (!win)
		return;

	auto* view = qobject_cast<PartMdiView*>(win);
	if (!view) {
		updateGUI();
		return;
	}

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
		if (worksheet)
			connect(worksheet, &Worksheet::cartesianPlotMouseModeChanged, this, &MainWin::cartesianPlotMouseModeChanged);
	}
}

void MainWin::handleAspectRemoved(const AbstractAspect* parent,const AbstractAspect* before,const AbstractAspect* aspect) {
	Q_UNUSED(before);
	Q_UNUSED(aspect);

	//no need to react on removal of
	// - AbstractSimpleFilter
	// - columns in the data spreadsheet of a datapicker curve,
	//   this can only happen when changing the error type and is done on the level of DatapickerImage
	if (!dynamic_cast<const AbstractSimpleFilter*>(aspect)
		&& !(parent->parentAspect() && parent->parentAspect()->type() == AspectType::DatapickerCurve) )
		m_projectExplorer->setCurrentAspect(parent);
}

void MainWin::handleAspectAboutToBeRemoved(const AbstractAspect *aspect) {
	const auto* part = qobject_cast<const AbstractPart*>(aspect);
	if (!part) return;

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
void MainWin::handleCurrentAspectChanged(AbstractAspect *aspect) {
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
	const auto* part = dynamic_cast<const AbstractPart*>(aspect);
	if (part) {
		//for LiveDataSource we currently don't show any view
		/*if (dynamic_cast<const LiveDataSource*>(part))
		    return;*/

		PartMdiView* win;

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
void MainWin::createFolderContextMenu(const Folder* folder, QMenu* menu) const {
	Q_UNUSED(folder);

	//Folder provides it's own context menu. Add a separator before adding additional actions.
	menu->addSeparator();
	this->createContextMenu(menu);
}

void MainWin::undo() {
	WAIT_CURSOR;
	m_project->undoStack()->undo();
	if (m_project->undoStack()->index() == 0) {
		setCaption(m_project->name());
		m_saveAction->setEnabled(false);
		m_undoAction->setEnabled(false);
		m_project->setChanged(false);
	}
	m_redoAction->setEnabled(true);
	RESET_CURSOR;
}

void MainWin::redo() {
	WAIT_CURSOR;
	m_project->undoStack()->redo();
	projectChanged();
	if (m_project->undoStack()->index() == m_project->undoStack()->count())
		m_redoAction->setEnabled(false);
	RESET_CURSOR;
}

/*!
	Shows/hides mdi sub-windows depending on the current visibility policy.
*/
void MainWin::updateMdiWindowVisibility() const {
	QList<QMdiSubWindow *> windows = m_mdiArea->subWindowList();
	PartMdiView* part_view;
	switch (m_project->mdiWindowVisibility()) {
	case Project::allMdiWindows:
		for (auto* window : windows)
			window->show();

		break;
	case Project::folderOnly:
		for (auto* window : windows) {
			part_view = qobject_cast<PartMdiView *>(window);
			Q_ASSERT(part_view);
			if (part_view->part()->folder() == m_currentFolder)
				part_view->show();
			else
				part_view->hide();
		}
		break;
	case Project::folderAndSubfolders:
		for (auto* window : windows) {
			part_view = qobject_cast<PartMdiView *>(window);
			if (part_view->part()->isDescendantOf(m_currentFolder))
				part_view->show();
			else
				part_view->hide();
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
	if (mode != CartesianPlot::Cursor) {
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

void MainWin::toggleFullScreen() {
	if (this->windowState() == Qt::WindowFullScreen)
		this->setWindowState(m_lastWindowState);
	else {
		m_lastWindowState = this->windowState();
		this->showFullScreen();
	}
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
	const KConfigGroup group = KSharedConfig::openConfig()->group( "Settings_General" );

	if(dynamic_cast<QQuickWidget*>(centralWidget()) == nullptr) {
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

	//show memory info
	bool showMemoryInfo = group.readEntry(QLatin1String("ShowMemoryInfo"), true);
	if (m_showMemoryInfo != showMemoryInfo) {
		m_showMemoryInfo = showMemoryInfo;
		if (showMemoryInfo) {
			m_memoryInfoWidget = new MemoryWidget(statusBar());
			statusBar()->addPermanentWidget(m_memoryInfoWidget);
		} else {
			if (m_memoryInfoWidget) {
				statusBar()->removeWidget(m_memoryInfoWidget);
				delete m_memoryInfoWidget;
				m_memoryInfoWidget = nullptr;
			}
		}
	}

	//update the units
	if (stackedWidget) {
		BaseDock* dock = dynamic_cast<BaseDock*>(stackedWidget->currentWidget());
		if (dock)
			dock->updateUnits();
	}

	bool showWelcomeScreen = group.readEntry<bool>(QLatin1String("ShowWelcomeScreen"), true);
	if(m_showWelcomeScreen != showWelcomeScreen)
		m_showWelcomeScreen = showWelcomeScreen;
}

void MainWin::openDatasetExample() {
	newProject();
	addAspectToProject(m_welcomeScreenHelper->releaseConfiguredSpreadsheet());
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
	DEBUG("MainWin::importFileDialog()");
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
		type = ImportProjectDialog::ProjectOrigin;
	else
		type = ImportProjectDialog::ProjectLabPlot;

	auto* dlg = new ImportProjectDialog(this, type);

	// set current folder
	dlg->setCurrentFolder(m_currentFolder);

	if (dlg->exec() == QDialog::Accepted) {
		dlg->importTo(statusBar());
		m_project->setChanged(true);
	}

	delete dlg;

	DEBUG("MainWin::importProjectDialog() DONE");
}

/*!
 * \brief opens a dialog to import datasets
 */
void MainWin::importDatasetDialog() {
	ImportDatasetDialog* dlg = new ImportDatasetDialog(this);
	if (dlg->exec() == QDialog::Accepted) {
			Spreadsheet* spreadsheet = new Spreadsheet(i18n("Dataset%1", 1));
			DatasetHandler* dataset = new DatasetHandler(spreadsheet);
			dlg->importToDataset(dataset, statusBar());

			QTimer timer;
			timer.setSingleShot(true);
			QEventLoop loop;
			connect(dataset,  &DatasetHandler::downloadCompleted, &loop, &QEventLoop::quit);
			connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
			timer.start(1500);
			loop.exec();

			if(timer.isActive()){
				timer.stop();
				addAspectToProject(spreadsheet);
				delete dataset;
			}
			else
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

	AbstractPart* part = dynamic_cast<PartMdiView*>(win)->part();
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
	ImportFileDialog* dlg = new ImportFileDialog(this, true);
	if (dlg->exec() == QDialog::Accepted) {
		if (static_cast<LiveDataSource::SourceType>(dlg->sourceType()) == LiveDataSource::MQTT) {
#ifdef HAVE_MQTT
			MQTTClient* mqttClient = new MQTTClient(i18n("MQTT Client%1", 1));
			dlg->importToMQTT(mqttClient);

			mqttClient->setName(mqttClient->clientHostName());

			QVector<const MQTTClient*> existingClients = m_project->children<const MQTTClient>(AbstractAspect::Recursive);

			//doesn't make sense to have more MQTTClients connected to the same broker
			bool found = false;
			for (const auto* client : existingClients) {
				if (client->clientHostName() == mqttClient->clientHostName() && client->clientPort() == mqttClient->clientPort()) {
					found = true;
					break;
				}
			}

			if (!found)
				addAspectToProject(mqttClient);
			else {
				delete mqttClient;
				QMessageBox::warning(this, "Warning", "There already is a MQTTClient with this host!");
			}
#endif
		} else {
			LiveDataSource* dataSource = new LiveDataSource(i18n("Live data source%1", 1), false);
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
		MQTTClient* clientAncestor = parent->ancestor<MQTTClient>();
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
	connect (dlg, &SettingsDialog::resetWelcomeScreen, this, &MainWin::resetWelcomeScreen);
	dlg->exec();
}

#ifdef HAVE_CANTOR_LIBS
void MainWin::cantorSettingsDialog()
{
	static KCoreConfigSkeleton* emptyConfig = new KCoreConfigSkeleton();
	KConfigDialog *cantorDialog = new KConfigDialog(this,  QLatin1String("Cantor Settings"), emptyConfig);
	for (auto* backend : Cantor::Backend::availableBackends())
		if (backend->config()) //It has something to configure, so add it to the dialog
			cantorDialog->addPage(backend->settingsWidget(cantorDialog), backend->config(), backend->name(),  backend->icon());

	cantorDialog->show();
}
#endif
