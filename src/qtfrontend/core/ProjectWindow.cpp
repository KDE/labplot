/***************************************************************************
    File                 : ProjectWindow.cpp
    Project              : SciDAVis
    Description          : Standard view on a Project; main window.
    --------------------------------------------------------------------
    Copyright            : (C) 2007-2008 Knut Franke (knut.franke*gmx.de)
    Copyright            : (C) 2007-2008 Tilman Benkert (thzs*gmx.net)
    Copyright            : (C) 2007 by Knut Franke, Tilman Benkert
                           (some parts taken from former ApplicationWindow class
						    (C) 2004-2007 by Ion Vasilief (ion_vasilief*yahoo.fr))
                           (replace * with @ in the email addresses) 

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
#include "core/ProjectWindow.h"

#include "core/Project.h"
#include "core/AspectTreeModel.h"
#include "core/AbstractPart.h"
#include "core/PartMdiView.h"
#include "core/ProjectExplorer.h"
#include "core/interfaces.h"
#include "core/ImportDialog.h"
#include "core/AbstractImportFilter.h"
#include "lib/ActionManager.h"
#include "lib/ShortcutsDialog.h"
#include "core/globals.h"

#include <QMenuBar>
#include <QMenu>
#include <QMdiArea>
#include <QDockWidget>
#include <QToolBar>
#include <QApplication>
#include <QUndoStack>
#include <QUndoView>
#include <QToolButton>
#include <QPluginLoader>
#include <QSignalMapper>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QTabWidget>
#include <QMessageBox>
#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QHttp>
#include <QBuffer>
#include <QDesktopServices>
#include <QUrl>
#include <QTextStream>

ActionManager * ProjectWindow::action_manager = 0;

#define WAIT_CURSOR QApplication::setOverrideCursor(QCursor(Qt::WaitCursor))
#define RESET_CURSOR QApplication::restoreOverrideCursor()

class ProjectWindow::Private
{
	public:

		//! Used when checking for new versions
		QHttp http;
		//! Used when checking for new versions
		QBuffer version_buffer;
}
;
ProjectWindow::ProjectWindow(Project* project)
	: m_project(project), d(new Private())
{
	init();
}

void ProjectWindow::init()
{
	setAttribute(Qt::WA_DeleteOnClose);
	setWindowIcon(QIcon(":/appicon"));

	if (!action_manager)
		action_manager = new ActionManager();

	action_manager->setTitle(tr("global", "global/ProjectWindow keyboard shortcuts"));

	m_mdi_area = new QMdiArea();
	setCentralWidget(m_mdi_area);
	connect(m_mdi_area, SIGNAL(subWindowActivated(QMdiSubWindow*)),
			this, SLOT(handleCurrentSubWindowChanged(QMdiSubWindow*)));
	m_current_aspect = m_project;
	m_current_folder = m_project;

	initDockWidgets();
	initActions();
	initToolBars();
	initMenus();
	m_buttons.new_aspect->setMenu(m_menus.new_aspect);
	// TODO: move all strings to one method to be called on a language change
	
	connect(m_project, SIGNAL(aspectDescriptionChanged(const AbstractAspect *)), 
		this, SLOT(handleAspectDescriptionChanged(const AbstractAspect *)));
	connect(m_project, SIGNAL(aspectAdded(const AbstractAspect *)),
		this, SLOT(handleAspectAdded(const AbstractAspect *)));
	connect(m_project, SIGNAL(aspectRemoved(const AbstractAspect *, const AbstractAspect *, const AbstractAspect *)),
		this, SLOT(handleAspectRemoved(const AbstractAspect *)));
	connect(m_project, SIGNAL(aspectAboutToBeRemoved(const AbstractAspect *)),
		this, SLOT(handleAspectAboutToBeRemoved(const AbstractAspect *)));
	connect(m_project, SIGNAL(statusInfo(const QString&)),
			statusBar(), SLOT(showMessage(const QString&)));

	handleAspectDescriptionChanged(m_project);

	// init action managers
	foreach(QObject *plugin, QPluginLoader::staticInstances()) 
	{
		ActionManagerOwner * manager_owner = qobject_cast<ActionManagerOwner *>(plugin);
		if (manager_owner) 
			manager_owner->initActionManager();
	}
	connect(&d->http, SIGNAL(done(bool)), this, SLOT(receivedVersionFile(bool)));

	connect(m_project, SIGNAL(requestProjectContextMenu(QMenu*)), this, SLOT(createContextMenu(QMenu*)));
	connect(m_project, SIGNAL(requestFolderContextMenu(const Folder*,QMenu*)), this, SLOT(createFolderContextMenu(const Folder*,QMenu*)));
	connect(m_project, SIGNAL(mdiWindowVisibilityChanged()), this, SLOT(updateMdiWindowVisibility()));
}

ProjectWindow::~ProjectWindow()
{
	disconnect(m_project, 0, this, 0);
}

void ProjectWindow::handleAspectDescriptionChanged(const AbstractAspect *aspect)
{
	if (aspect != static_cast<AbstractAspect *>(m_project)) return;
	setWindowTitle(m_project->caption() + " - SciDAVis");
}

void ProjectWindow::handleAspectAdded(const AbstractAspect *aspect)
{
	handleAspectAddedInternal(aspect);
	updateMdiWindowVisibility();
	handleCurrentSubWindowChanged(m_mdi_area->currentSubWindow());
}

void ProjectWindow::handleAspectAddedInternal(const AbstractAspect * aspect)
{
	foreach(const AbstractAspect * child, aspect->children<AbstractAspect>())
		handleAspectAddedInternal(child);

	const AbstractPart *part = qobject_cast<const AbstractPart*>(aspect);
	if (part)
	{
		PartMdiView *win = part->mdiSubWindow();
		Q_ASSERT(win);
		m_mdi_area->addSubWindow(win);
		connect(win, SIGNAL(statusChanged(PartMdiView *, PartMdiView::SubWindowStatus, PartMdiView::SubWindowStatus)), 
				this, SLOT(handleSubWindowStatusChange(PartMdiView *, PartMdiView::SubWindowStatus, PartMdiView::SubWindowStatus)));
	}
}

void ProjectWindow::handleAspectAboutToBeRemoved(const AbstractAspect *aspect)
{
	const AbstractPart *part = qobject_cast<const AbstractPart*>(aspect);
	if (!part) return;
	PartMdiView *win = part->mdiSubWindow();
	Q_ASSERT(win);
	disconnect(win, SIGNAL(statusChanged(PartMdiView *, PartMdiView::SubWindowStatus, PartMdiView::SubWindowStatus)), 
		this, SLOT(handleSubWindowStatusChange(PartMdiView *, PartMdiView::SubWindowStatus, PartMdiView::SubWindowStatus)));
	m_mdi_area->removeSubWindow(win);
	if (m_mdi_area->currentSubWindow() == 0)
	{
		m_menus.part->clear();
		m_menus.part->setEnabled(false);
	}
}

void ProjectWindow::handleAspectRemoved(const AbstractAspect *parent)
{
	m_project_explorer->setCurrentAspect(parent);
}

void ProjectWindow::initDockWidgets()
{
	// project explorer
	m_project_explorer_dock = new QDockWidget(this);
	m_project_explorer_dock->setWindowTitle(tr("Project Explorer"));
	m_project_explorer = new ProjectExplorer(m_project_explorer_dock);
	m_project_explorer->setModel(new AspectTreeModel(m_project, this));
	m_project_explorer_dock->setWidget(m_project_explorer);
	addDockWidget(Qt::BottomDockWidgetArea, m_project_explorer_dock);
	connect(m_project_explorer, SIGNAL(currentAspectChanged(AbstractAspect *)),
		this, SLOT(handleCurrentAspectChanged(AbstractAspect *)));
	m_project_explorer->setCurrentAspect(m_project);
}

void ProjectWindow::initActions()
{
	m_actions.quit = new QAction(tr("&Quit"), this);
	m_actions.quit->setIcon(QIcon(QPixmap(":/quit.xpm")));
	m_actions.quit->setShortcut(tr("Ctrl+Q"));
	action_manager->addAction(m_actions.quit, "quit");
	connect(m_actions.quit, SIGNAL(triggered(bool)), qApp, SLOT(closeAllWindows()));
	
	m_actions.open_project = new QAction(tr("&Open Project"), this);
	m_actions.open_project->setIcon(QIcon(QPixmap(":/fileopen.xpm")));
	m_actions.open_project->setShortcut(tr("Ctrl+O"));
	action_manager->addAction(m_actions.open_project, "open_project");
	connect(m_actions.open_project, SIGNAL(triggered(bool)), this, SLOT(openProject()));

	m_actions.save_project = new QAction(tr("&Save Project"), this);
	m_actions.save_project->setIcon(QIcon(QPixmap(":/filesave.xpm")));
	m_actions.save_project->setShortcut(tr("Ctrl+S"));
	action_manager->addAction(m_actions.save_project, "save_project");
	connect(m_actions.save_project, SIGNAL(triggered(bool)), this, SLOT(saveProject()));

	m_actions.save_project_as = new QAction(tr("Save Project &As"), this);
	action_manager->addAction(m_actions.save_project_as, "save_project_as");
	connect(m_actions.save_project_as, SIGNAL(triggered(bool)), this, SLOT(saveProjectAs()));

	m_actions.new_folder = new QAction(tr("New F&older"), this);
	action_manager->addAction(m_actions.new_folder, "new_folder");
	m_actions.new_folder->setIcon(QIcon(QPixmap(":/folder_closed.xpm")));
	connect(m_actions.new_folder, SIGNAL(triggered(bool)), this, SLOT(addNewFolder()));

	m_actions.new_project = new QAction(tr("New &Project"), this);
	m_actions.new_project->setShortcut(tr("Ctrl+N"));
	action_manager->addAction(m_actions.new_project, "new_project");
	m_actions.new_project->setIcon(QIcon(QPixmap(":/new.xpm")));
	connect(m_actions.new_project, SIGNAL(triggered(bool)), this, SLOT(newProject()));

	m_actions.close_project = new QAction(tr("&Close Project"), this);
	action_manager->addAction(m_actions.close_project, "close_project");
	m_actions.close_project->setIcon(QIcon(QPixmap(":/close.xpm")));
	connect(m_actions.close_project, SIGNAL(triggered(bool)), this, SLOT(close())); // TODO: capture closeEvent for saving

	m_actions.keyboard_shortcuts_dialog = new QAction(tr("&Keyboard Shortcuts"), this);
	action_manager->addAction(m_actions.keyboard_shortcuts_dialog, "keyboard_shortcuts_dialog");
	connect(m_actions.keyboard_shortcuts_dialog, SIGNAL(triggered(bool)), this, SLOT(showKeyboardShortcutsDialog()));

	m_actions.preferences_dialog = new QAction(tr("&Preferences"), this);
	action_manager->addAction(m_actions.preferences_dialog, "preferences_dialog");
	connect(m_actions.preferences_dialog, SIGNAL(triggered(bool)), this, SLOT(showPreferencesDialog()));

	m_actions.show_history = new QAction(tr("Undo/Redo &History"), this);
	action_manager->addAction(m_actions.show_history, "show_history");
	connect(m_actions.show_history, SIGNAL(triggered(bool)), this, SLOT(showHistory()));

	m_part_maker_map = new QSignalMapper(this);
	connect(m_part_maker_map, SIGNAL(mapped(QObject*)), this, SLOT(addNewAspect(QObject*)));
	foreach(QObject *plugin, QPluginLoader::staticInstances()) {
		PartMaker *maker = qobject_cast<PartMaker*>(plugin);
		if (maker) {
			QAction *make = maker->makeAction(this);
			connect(make, SIGNAL(triggered()), m_part_maker_map, SLOT(map()));
			m_part_maker_map->setMapping(make, plugin);
			m_part_makers << make;
		}
	}

	m_actions.import_aspect = new QAction(tr("&Import"), this);
	action_manager->addAction(m_actions.import_aspect, "import_aspect");
	// TODO: we need a new icon for generic imports
	m_actions.import_aspect->setIcon(QIcon(QPixmap(":/fileopen.xpm")));
	connect(m_actions.import_aspect, SIGNAL(triggered()), this, SLOT(importAspect()));

	m_actions.cascade_windows = new QAction(tr("&Cascade"), this);
	action_manager->addAction(m_actions.cascade_windows, "cascade_windows");
	connect(m_actions.cascade_windows, SIGNAL(triggered()), m_mdi_area, SLOT(cascadeSubWindows()));
	
	m_actions.tile_windows = new QAction(tr("&Tile"), this);
	action_manager->addAction(m_actions.tile_windows, "tile_windows");
	connect(m_actions.tile_windows, SIGNAL(triggered()), m_mdi_area, SLOT(tileSubWindows()));

	m_actions.choose_folder = new QAction(tr("Select &Folder"), this);
	m_actions.choose_folder->setIcon(QIcon(QPixmap(":/folder_closed.xpm")));
	action_manager->addAction(m_actions.choose_folder, "choose_folder");
	connect(m_actions.choose_folder, SIGNAL(triggered()), this, SLOT(chooseFolder()));

	m_actions.next_subwindow = new QAction(tr("&Next","next window"), this);
	m_actions.next_subwindow->setIcon(QIcon(QPixmap(":/next.xpm")));
	m_actions.next_subwindow->setShortcut(tr("F5","next window shortcut"));
	action_manager->addAction(m_actions.next_subwindow, "next_subwindow");
	connect(m_actions.next_subwindow, SIGNAL(triggered()), m_mdi_area, SLOT(activateNextSubWindow()));

	m_actions.previous_subwindow = new QAction(tr("&Previous","previous window"), this);
	m_actions.previous_subwindow->setIcon(QIcon(QPixmap(":/prev.xpm")));
	m_actions.previous_subwindow->setShortcut(tr("F6", "previous window shortcut"));
	action_manager->addAction(m_actions.previous_subwindow, "previous_subwindow");
	connect(m_actions.previous_subwindow, SIGNAL(triggered()), m_mdi_area, SLOT(activatePreviousSubWindow()));

	m_actions.close_current_window = new QAction(tr("Close &Window"), this);
	m_actions.close_current_window->setIcon(QIcon(QPixmap(":/close.xpm")));
	m_actions.close_current_window->setShortcut(tr("Ctrl+W", "close window shortcut"));
	action_manager->addAction(m_actions.close_current_window, "close_current_window");
	connect(m_actions.close_current_window, SIGNAL(triggered()), m_mdi_area, SLOT(closeActiveSubWindow()));

	m_actions.close_all_windows = new QAction(tr("Close &All Windows"), this);
	action_manager->addAction(m_actions.close_all_windows, "close_all_windows");
	connect(m_actions.close_all_windows, SIGNAL(triggered()), m_mdi_area, SLOT(closeAllSubWindows()));

	// TODO: duplicate action (or maybe in the part menu?)
	
	m_actions.about = new QAction(tr("&About SciDAVis"), this);
	m_actions.about->setShortcut( tr("Shift+F1") );
	action_manager->addAction(m_actions.about, "about");
	connect(m_actions.about, SIGNAL(triggered()), this, SLOT(showAboutDialog()));

	m_actions.show_manual = new QAction(tr("&Help"), this);
	m_actions.show_manual->setShortcut( tr("F1") );
	action_manager->addAction(m_actions.show_manual, "show_manual");
	connect(m_actions.show_manual, SIGNAL(triggered()), this, SLOT(showHelp()));

	m_actions.select_manual_folder = new QAction(tr("&Choose Help Folder..."), this);
	action_manager->addAction(m_actions.select_manual_folder, "select_manual_folder");
	connect(m_actions.select_manual_folder, SIGNAL(triggered()), this, SLOT(chooseHelpFolder()));

	m_actions.show_homepage = new QAction(tr("&SciDAVis Homepage"), this);
	action_manager->addAction(m_actions.show_homepage, "show_homepage");
	connect(m_actions.show_homepage, SIGNAL(triggered()), this, SLOT(showHomePage()));

	m_actions.show_forums = new QAction(tr("SciDAVis &Forums"), this);
	action_manager->addAction(m_actions.show_forums, "show_forums");
	connect(m_actions.show_forums, SIGNAL(triggered()), this, SLOT(showForums()));

	m_actions.show_bugtracker = new QAction(tr("Report a &Bug"), this);
	action_manager->addAction(m_actions.show_bugtracker, "show_bugtracker");
	connect(m_actions.show_bugtracker, SIGNAL(triggered()), this, SLOT(showBugTracker()));

	m_actions.download_manual = new QAction(tr("Download &Manual"), this);
	action_manager->addAction(m_actions.download_manual, "download_manual");
	connect(m_actions.download_manual, SIGNAL(triggered()), this, SLOT(downloadManual()));

	m_actions.download_translations = new QAction(tr("Download &Translations"), this);
	action_manager->addAction(m_actions.download_translations, "download_translations");
	connect(m_actions.download_translations, SIGNAL(triggered()), this, SLOT(downloadTranslation()));

	m_actions.check_updates = new QAction(tr("Search for &Updates"), this);
	action_manager->addAction(m_actions.check_updates, "check_updates");
	connect(m_actions.check_updates, SIGNAL(triggered()), this, SLOT(searchForUpdates()));

	Q_ASSERT(m_project->undoStack());
	m_actions.undo = new QAction(tr("&Undo"), this);
	action_manager->addAction(m_actions.undo, "undo");
	m_actions.undo->setIcon(QIcon(QPixmap(":/undo.xpm")));
	m_actions.undo->setShortcut(tr("Ctrl+Z"));
	m_actions.undo->setEnabled(m_project->undoStack()->canUndo());
	connect(m_actions.undo, SIGNAL(triggered()), this, SLOT(undo()));
	connect(m_project->undoStack(), SIGNAL(canUndoChanged(bool)), m_actions.undo, SLOT(setEnabled(bool)));

	m_actions.redo = new QAction(tr("&Redo"), this);
	action_manager->addAction(m_actions.redo, "redo");
	m_actions.redo->setIcon(QIcon(QPixmap(":/redo.xpm")));
	m_actions.redo->setShortcut(tr("Ctrl+Y"));
	m_actions.redo->setEnabled(m_project->undoStack()->canRedo());
	connect(m_actions.redo, SIGNAL(triggered()), this, SLOT(redo()));
	connect(m_project->undoStack(), SIGNAL(canRedoChanged(bool)), m_actions.redo, SLOT(setEnabled(bool)));
}

void ProjectWindow::initMenus()
{
	m_menus.file = menuBar()->addMenu(tr("&File"));
	m_menus.new_aspect = m_menus.file->addMenu(tr("&New"));
	m_menus.new_aspect->addAction(m_actions.new_project);
	m_menus.new_aspect->addAction(m_actions.new_folder);
	foreach(QAction *a, m_part_makers)
		m_menus.new_aspect->addAction(a);

	m_menus.file->addAction(m_actions.open_project);
	m_menus.file->addAction(m_actions.save_project);
	m_menus.file->addAction(m_actions.save_project_as);
	m_menus.file->addAction(m_actions.close_project);
	m_menus.file->addSeparator();
	m_menus.file->addAction(m_actions.quit);

	m_menus.edit = menuBar()->addMenu(tr("&Edit"));
	m_menus.edit->addAction(m_actions.undo);
	m_menus.edit->addAction(m_actions.redo);
	connect(m_menus.edit, SIGNAL(aboutToShow()), this, SLOT(nameUndoRedo()));
	connect(m_menus.edit, SIGNAL(aboutToHide()), this, SLOT(renameUndoRedo()));
	m_menus.edit->addAction(m_actions.show_history);
	m_menus.edit->addSeparator();
	m_menus.edit->addAction(m_actions.keyboard_shortcuts_dialog);
	m_menus.edit->addAction(m_actions.preferences_dialog);

	m_menus.view = menuBar()->addMenu(tr("&View"));

	m_menus.toolbars = createToolbarsMenu();
	if(!m_menus.toolbars) m_menus.toolbars = new QMenu(this);
	m_menus.toolbars->setTitle(tr("Toolbars"));
	
	m_menus.dockwidgets = createDockWidgetsMenu();
	if(!m_menus.dockwidgets) m_menus.dockwidgets = new QMenu(this);
	m_menus.dockwidgets->setTitle(tr("Dock Widgets"));

	m_menus.view->addMenu(m_menus.toolbars);
	m_menus.view->addMenu(m_menus.dockwidgets);
	m_menus.view->addSeparator();

	m_menus.part = menuBar()->addMenu(QString());
	m_menus.part->hide();

	m_menus.windows = menuBar()->addMenu(tr("&Windows"));
	connect( m_menus.windows, SIGNAL(aboutToShow()), this, SLOT(handleWindowsMenuAboutToShow()) );
	
	m_menus.win_policy_menu = new QMenu(tr("Show &Windows"));
	QActionGroup * policy_action_group = new QActionGroup(m_menus.win_policy_menu);
	policy_action_group->setExclusive(true);

	m_actions.visibility_folder = new QAction(tr("Current &Folder Only"), policy_action_group);
	action_manager->addAction(m_actions.visibility_folder, "visibility_folder");
	m_actions.visibility_folder->setCheckable(true);
	m_actions.visibility_folder->setData(Project::folderOnly);
	m_actions.visibility_subfolders = new QAction(tr("Current Folder and &Subfolders"), policy_action_group);
	action_manager->addAction(m_actions.visibility_subfolders, "visibility_subfolders");
	m_actions.visibility_subfolders->setCheckable(true);
	m_actions.visibility_subfolders->setData(Project::folderAndSubfolders);
	m_actions.visibility_all = new QAction(tr("&All"), policy_action_group);
	action_manager->addAction(m_actions.visibility_all, "visibility_all");
	m_actions.visibility_all->setCheckable(true);
	m_actions.visibility_all->setData(Project::allMdiWindows);
	connect(policy_action_group, SIGNAL(triggered(QAction*)), this, SLOT(setMdiWindowVisibility(QAction*)));
	m_menus.win_policy_menu->addActions(policy_action_group->actions());
	connect( m_menus.win_policy_menu, SIGNAL(aboutToShow()), this, SLOT(handleWindowsPolicyMenuAboutToShow()) );
	
	m_menus.view->addMenu(m_menus.win_policy_menu);

	m_menus.help = menuBar()->addMenu(tr("&Help"));
	m_menus.help->addAction(m_actions.show_manual);
	m_menus.help->addAction(m_actions.select_manual_folder);
	m_menus.help->addSeparator();
	m_menus.help->addAction(m_actions.show_homepage);
	m_menus.help->addAction(m_actions.check_updates);
	m_menus.help->addAction(m_actions.download_manual);
	m_menus.help->addAction(m_actions.download_translations);
	m_menus.help->addSeparator();
	m_menus.help->addAction(m_actions.show_forums);
	m_menus.help->addAction(m_actions.show_bugtracker);
	m_menus.help->addSeparator();
	m_menus.help->addAction(m_actions.about);
}

void ProjectWindow::initToolBars()
{
	m_toolbars.file = new QToolBar( tr( "File" ), this );
	m_toolbars.file->setObjectName("file_toolbar"); // this is needed for QMainWindow::restoreState()
	addToolBar( Qt::TopToolBarArea, m_toolbars.file );
	
	m_buttons.new_aspect = new QToolButton(this);
	m_buttons.new_aspect->setPopupMode(QToolButton::InstantPopup);
	m_buttons.new_aspect->setIcon(QPixmap(":/new_aspect.xpm"));
	m_buttons.new_aspect->setToolTip(tr("New Aspect"));
	m_toolbars.file->addWidget(m_buttons.new_aspect);

	m_toolbars.file->addAction(m_actions.import_aspect);

	m_toolbars.edit = new QToolBar( tr("Edit"), this);
	m_toolbars.edit->setObjectName("edit_toolbar");
	addToolBar(Qt::TopToolBarArea, m_toolbars.edit);

	m_toolbars.edit->addAction(m_actions.undo);
	m_toolbars.edit->addAction(m_actions.redo);
}

void ProjectWindow::addNewFolder()
{
	addNewAspect(new Folder(tr("Folder %1").arg(1)));
}

void ProjectWindow::newProject()
{
	Project * prj = new Project();
	prj->view()->showMaximized();
}

void ProjectWindow::openProject()
{
	QString filter = tr("SciDAVis project")+" (*.sciprj);;";
	filter += tr("Compressed SciDAVis project")+" (*.sciprj.gz)";

	QString working_dir = qApp->applicationDirPath();
	QString selected_filter;
	QString file_name = QFileDialog::getOpenFileName(this, tr("Open project"), working_dir, filter, &selected_filter);
	// TODO: determine whether it is a 0.1.x project and run an import filter if it is
	//
	//

	if (file_name.isEmpty())
		return;

	QFile file(file_name);
	if (!file.open(QIODevice::ReadOnly)) 
	{
		QString msg_text = tr("Could not open file \"%1\".").arg(file_name);
		QMessageBox::critical(this, tr("Error opening project"), msg_text);
		statusBar()->showMessage(msg_text);
		return;
	}
	XmlStreamReader reader(&file);
	Project * prj = new Project();
	prj->view(); // ensure the view (ProjectWindow) is created
	if (prj->load(&reader) == false)
	{
		QString msg_text = reader.errorString();
		QMessageBox::critical(this, tr("Error opening project"), msg_text);
		statusBar()->showMessage(msg_text);
		delete prj;
		return;
	}
	if (reader.hasWarnings())
	{
		QString msg_text = tr("The following problems occured when loading the project:\n");
		QStringList warnings = reader.warningStrings();
		foreach(QString str, warnings)
			msg_text += str + "\n";
		QMessageBox::warning(this, tr("Project loading partly failed"), msg_text);
		statusBar()->showMessage(msg_text);
	}
	file.close();
	prj->undoStack()->clear();
	prj->view()->showMaximized();
	prj->setFileName(file_name);
}

void ProjectWindow::saveProject()
{
	if (m_project->fileName().isEmpty())
		saveProjectAs();
	else
	{
		QFile file(m_project->fileName());
		if (!file.open(QIODevice::WriteOnly)) 
		{
			QString msg_text = tr("Could not open file \"%1\".").arg(m_project->fileName());
			QMessageBox::critical(this, tr("Error saving project"), msg_text);
			statusBar()->showMessage(msg_text);
			return;
		}
		QXmlStreamWriter writer(&file);
		m_project->save(&writer);
		m_project->undoStack()->clear();
		file.close();
	}
}

void ProjectWindow::saveProjectAs()
{
	QString filter = tr("SciDAVis project")+" (*.sciprj);;";
	filter += tr("Compressed SciDAVis project")+" (*.sciprj.gz)";

	QString working_dir = qApp->applicationDirPath();
	QString selected_filter;
	QString path;
	if (!m_project->fileName().isEmpty())
		path = m_project->fileName();
	else
		path = working_dir + QString("/") + m_project->name() + ".sciprj";
	QString fn = QFileDialog::getSaveFileName(this, tr("Save project as"), path, filter, &selected_filter);
	if ( !fn.isEmpty() )
	{
			QFileInfo fi(fn);
		// TODO: remember path
		//		working_dir = fi.dirPath(true);
		QString base_name = fi.fileName();
		if (!base_name.endsWith(".sciprj") && !base_name.endsWith(".sciprj.gz"))
		{
			fn.append(".sciprj");
		}
		bool compress = false;
		if (fn.endsWith(".gz"))
		{
			fn = fn.left(fn.length() -3);
			compress = true;
		}
		
		m_project->setFileName(fn);
		saveProject();
		// TODO: activate this code later
//		if (selected_filter.contains(".gz") || compress)
//			file_compress((char *)fn.toAscii().constData(), "wb9");
	}
}

QMenu * ProjectWindow::createToolbarsMenu()
{
    QMenu *menu = 0;
    QList<QToolBar *> toolbars = qFindChildren<QToolBar *>(this);
    if (toolbars.size())
	{
        menu = new QMenu(this);
		foreach(QToolBar *toolbar, toolbars)
		{
            if (toolbar->parentWidget() == this)
                menu->addAction(toolbar->toggleViewAction());
        }
    }
    return menu;
}

QMenu * ProjectWindow::createDockWidgetsMenu()
{
    QMenu *menu = 0;
    QList<QDockWidget *> dockwidgets = qFindChildren<QDockWidget *>(this);
    if (dockwidgets.size())
	{
        menu = new QMenu(this);
		foreach(QDockWidget *widget, dockwidgets)
		{
            if (widget->parentWidget() == this)
                menu->addAction(widget->toggleViewAction());
        }
    }
    return menu;
}

void ProjectWindow::addNewAspect(QObject* obj)
{
	AbstractAspect *aspect = qobject_cast<AbstractAspect*>(obj);
	if (!aspect) {
		PartMaker *maker = qobject_cast<PartMaker*>(obj);
		if (!maker) return;
		aspect = maker->makePart();
	}
	QModelIndex index = m_project_explorer->currentIndex();

	if(!index.isValid()) 
		m_project->addChild(aspect);
	else
	{
		AbstractAspect * parent_aspect = static_cast<AbstractAspect *>(index.internalPointer());
		Q_ASSERT(parent_aspect->folder()); // every aspect contained in the project should have a folder
		parent_aspect->folder()->addChild(aspect);
	}
}

void ProjectWindow::handleCurrentAspectChanged(AbstractAspect *aspect)
{
	if(!aspect) aspect = m_project; // should never happen, just in case
	if(aspect->folder() != m_current_folder)
	{
		m_current_folder = aspect->folder();
		updateMdiWindowVisibility();
	}
	if(aspect != m_current_aspect)
	{
		AbstractPart * part = qobject_cast<AbstractPart*>(aspect);
		if (part)
			m_mdi_area->setActiveSubWindow(part->mdiSubWindow());
	}
	m_current_aspect = aspect;
}

void ProjectWindow::handleCurrentSubWindowChanged(QMdiSubWindow* win) 
{
	PartMdiView *view = qobject_cast<PartMdiView*>(win);
	m_menus.part->clear();
	m_menus.part->setEnabled(false);
	if (!view) return;
	emit partActivated(view->part());
	if (view->status() == PartMdiView::Visible)
		m_menus.part->setEnabled(view->part()->fillProjectMenu(m_menus.part)); 
	m_project_explorer->setCurrentAspect(view->part());
}

void ProjectWindow::handleSubWindowStatusChange(PartMdiView * view, PartMdiView::SubWindowStatus from, PartMdiView::SubWindowStatus to) 
{
	if (view == m_mdi_area->currentSubWindow())
	{
		m_menus.part->clear();
		if (from == PartMdiView::Hidden && to == PartMdiView::Visible)
			m_menus.part->setEnabled(view->part()->fillProjectMenu(m_menus.part)); 
		else if (to == PartMdiView::Hidden && from == PartMdiView::Visible)
			m_menus.part->setEnabled(false);
	}
}

void ProjectWindow::updateMdiWindowVisibility()
{
	QList<QMdiSubWindow *> windows = m_mdi_area->subWindowList();
	PartMdiView * part_view;
	switch(m_project->mdiWindowVisibility()) 
	{
		case Project::allMdiWindows:
			foreach(QMdiSubWindow *window, windows) 
			{
				part_view = qobject_cast<PartMdiView *>(window);
				Q_ASSERT(part_view);
				part_view->show();
			}
			break;
		case Project::folderOnly:
			foreach(QMdiSubWindow *window, windows) 
			{
				part_view = qobject_cast<PartMdiView *>(window);
				Q_ASSERT(part_view);
				if(part_view->part()->folder() == m_current_folder)
					part_view->show();
				else
					part_view->hide();
			}
			break;
		case Project::folderAndSubfolders:
			foreach(QMdiSubWindow *window, windows) 
			{
				part_view = qobject_cast<PartMdiView *>(window);
				if(part_view->part()->isDescendantOf(m_current_folder))
					part_view->show();
				else
					part_view->hide();
			}
			break;
	}
}

void ProjectWindow::importAspect()
{
	QMap<QString, AbstractImportFilter*> filter_map;

	foreach(QObject * plugin, QPluginLoader::staticInstances()) {
		FileFormat * ff = qobject_cast<FileFormat*>(plugin);
		if (!ff) continue;
		AbstractImportFilter *filter = ff->makeImportFilter();
		filter_map[filter->nameAndPatterns()] = filter;
	}

	ImportDialog *id = new ImportDialog(filter_map, this);
	if (id->exec() != QDialog::Accepted)
		return;

	AbstractImportFilter * filter = filter_map[id->selectedFilter()];
	QFile file;
	switch (id->destination()) {
		case ImportDialog::CurrentProject:
			foreach(QString file_name, id->selectedFiles()) {
				file.setFileName(file_name);
				if (!file.open(QIODevice::ReadOnly)) {
					statusBar()->showMessage(tr("Could not open file \"%1\".").arg(file_name));
					return;
				}
				AbstractAspect * aspect = filter->importAspect(&file);
				file.close();
				if (aspect->inherits("Project")) {
					Folder * folder = new Folder(aspect->name());
					folder->setComment(aspect->comment());
					folder->setCaptionSpec(aspect->captionSpec());
					foreach (AbstractAspect * child, aspect->children<AbstractAspect>(AbstractAspect::IncludeHidden))
						child->reparent(folder);
					addNewAspect(folder);
					delete aspect;
				} else
					addNewAspect(aspect);
			}
			break;
		case ImportDialog::NewProjects:
			foreach(QString file_name, id->selectedFiles()) {
				file.setFileName(file_name);
				if (!file.open(QIODevice::ReadOnly)) {
					statusBar()->showMessage(tr("Could not open file \"%1\".").arg(file_name));
					return;
				}
				AbstractAspect * aspect = filter->importAspect(&file);
				file.close();
				if (aspect->inherits("Project")) {
					static_cast<Project*>(aspect)->view()->showMaximized();
				} else {
					Project * project = new Project();
					project->addChild(aspect);
					project->view()->showMaximized();
				}
			}
			break;
	}
	qDeleteAll(filter_map);
	delete id;
}

void ProjectWindow::showKeyboardShortcutsDialog()
{
	QList<ActionManager *> managers;
	managers.append(action_manager);
	// TODO: add action manager for project window first
	foreach(QObject *plugin, QPluginLoader::staticInstances()) 
	{
		ActionManagerOwner * manager_owner = qobject_cast<ActionManagerOwner *>(plugin);
		if (manager_owner) 
			managers.append(manager_owner->actionManager());
	}
	ShortcutsDialog dialog(managers, this);
	dialog.setWindowTitle(tr("Customize Keyboard Shortcuts"));
	dialog.resize(width()-width()/5, height()-height()/5);
	dialog.exec();
}

void ProjectWindow::showPreferencesDialog()
{
	QDialog dialog;
	QVBoxLayout layout(&dialog);

    QDialogButtonBox button_box;
	button_box.setOrientation(Qt::Horizontal);
    button_box.setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::NoButton|QDialogButtonBox::Ok);
    QObject::connect(&button_box, SIGNAL(accepted()), &dialog, SLOT(accept()));
    QObject::connect(&button_box, SIGNAL(rejected()), &dialog, SLOT(reject()));

	QTabWidget tab_widget;
	QList<ConfigPageWidget *> widgets;
	ConfigPageWidget * current;

	current = Project::makeConfigPage();
	tab_widget.addTab(current, Project::configPageLabel());
	widgets.append(current);

	foreach(QObject * plugin, QPluginLoader::staticInstances()) 
	{
		ConfigPageMaker * ctm = qobject_cast<ConfigPageMaker*>(plugin);
		if (!ctm) continue;
		current = ctm->makeConfigPage();
		tab_widget.addTab(current, ctm->configPageLabel());
		widgets.append(current);
	}

	layout.addWidget(&tab_widget);
	layout.addWidget(&button_box);

	dialog.setWindowTitle(tr("Preferences"));
	if (dialog.exec() != QDialog::Accepted)
		return;

	foreach(current, widgets)
		current->apply();
}

void ProjectWindow::showHistory()
{
	if (!m_project->undoStack()) return;
	QDialog dialog;
	QVBoxLayout layout(&dialog);

    QDialogButtonBox button_box;
	button_box.setOrientation(Qt::Horizontal);
    button_box.setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::NoButton|QDialogButtonBox::Ok);
    QObject::connect(&button_box, SIGNAL(accepted()), &dialog, SLOT(accept()));
    QObject::connect(&button_box, SIGNAL(rejected()), &dialog, SLOT(reject()));

	int index = m_project->undoStack()->index();
	QUndoView undo_view(m_project->undoStack());

	layout.addWidget(&undo_view);
	layout.addWidget(&button_box);

	dialog.setWindowTitle(tr("Undo/Redo History"));
	if (dialog.exec() == QDialog::Accepted)
		return;

	m_project->undoStack()->setIndex(index);
}


void ProjectWindow::handleWindowsMenuAboutToShow()
{
	m_menus.windows->clear();
	m_menus.windows->addAction(m_actions.cascade_windows);
	m_menus.windows->addAction(m_actions.tile_windows);
	m_menus.windows->addSeparator();
	m_menus.windows->addAction(m_actions.next_subwindow);
	m_menus.windows->addAction(m_actions.previous_subwindow);
	m_menus.windows->addSeparator();
	m_menus.windows->addAction(m_actions.close_current_window);
	m_menus.windows->addAction(m_actions.close_all_windows);
	m_menus.windows->addSeparator();
	m_menus.windows->addAction(m_actions.choose_folder);
	m_menus.windows->addSeparator();

	m_actions.close_current_window->setEnabled(m_mdi_area->currentSubWindow() != 0 && m_mdi_area->currentSubWindow()->isVisible());

	QList<QMdiSubWindow *> windows = m_mdi_area->subWindowList();
	foreach(QMdiSubWindow *win, windows)
		if (!win->isVisible()) windows.removeAll(win);

	foreach(QMdiSubWindow *win, windows)
	{
		QAction * action = m_menus.windows->addAction(win->windowTitle());
		connect(action, SIGNAL(triggered()), win, SLOT(setFocus()));
		action->setCheckable(true);
		action->setChecked( m_mdi_area->activeSubWindow() == win );
	}
}

void ProjectWindow::handleWindowsPolicyMenuAboutToShow()
{
	if (m_project->mdiWindowVisibility() == Project::folderOnly) 
		m_actions.visibility_folder->setChecked(true);
	else if (m_project->mdiWindowVisibility() == Project::folderAndSubfolders) 
		m_actions.visibility_subfolders->setChecked(true);
	else
		m_actions.visibility_all->setChecked(true);
}

void ProjectWindow::createContextMenu(QMenu * menu) const
{
	menu->addMenu(m_menus.new_aspect);
	menu->addMenu(m_menus.win_policy_menu);
	menu->addSeparator();

	// TODO:
	// Find
	// ----
	// Append Project
	// Save Project As
	// ----
}

void ProjectWindow::createFolderContextMenu(const Folder * folder, QMenu * menu) const
{
	Q_UNUSED(folder)
	menu->addSeparator();

	menu->addMenu(m_menus.new_aspect);

	if (m_project->mdiWindowVisibility() == Project::folderOnly) 
		m_actions.visibility_folder->setChecked(true);
	else if (m_project->mdiWindowVisibility() == Project::folderAndSubfolders) 
		m_actions.visibility_subfolders->setChecked(true);
	else
		m_actions.visibility_all->setChecked(true);
	menu->addMenu(m_menus.win_policy_menu);
	menu->addSeparator();
}

void ProjectWindow::setMdiWindowVisibility(QAction * action) 
{
	m_project->setMdiWindowVisibility((Project::MdiWindowVisibility)(action->data().toInt()));
}
		
void ProjectWindow::chooseFolder()
{
	QList<Folder *> list = m_project->children<Folder>(AbstractAspect::Recursive);
	list.prepend(m_project);
	
	// TODO: make a nicer dialog
	QDialog dialog;
	QVBoxLayout layout(&dialog);
	QLabel label(tr("Select Folder"));
	QComboBox selection;
	for (int i=0; i<list.size(); i++)
		selection.addItem(list.at(i)->path(), i);

    QDialogButtonBox button_box(&dialog);
    button_box.setOrientation(Qt::Horizontal);
    button_box.setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::NoButton|QDialogButtonBox::Ok);
    QObject::connect(&button_box, SIGNAL(accepted()), &dialog, SLOT(accept()));
    QObject::connect(&button_box, SIGNAL(rejected()), &dialog, SLOT(reject()));

	layout.addWidget(&label);
	layout.addWidget(&selection);
	layout.addWidget(&button_box);

	dialog.setWindowTitle(label.text());
	if (dialog.exec() != QDialog::Accepted)
		return;
	int index = selection.currentIndex();
	if (index >= 0 && index < list.size())
		m_project_explorer->setCurrentAspect(list.at(index));
}

void ProjectWindow::showAboutDialog()
{
	SciDAVis::about();
}

void ProjectWindow::showHelp()
{
// TODO
/*
	QFile helpFile(helpFilePath);
	if (!helpFile.exists())
	{
		QMessageBox::critical(this,tr("Help Files Not Found!"),
				tr("Please indicate the location of the help file!")+"<br>"+
				tr("The manual can be downloaded from the following internet address:")+
				"<p><a href = http://sourceforge.net/project/showfiles.php?group_id=199120>http://sourceforge.net/project/showfiles.php?group_id=199120</a></p>");
		QString fn = QFileDialog::getOpenFileName(QDir::currentDirPath(), "*.html", this );
		if (!fn.isEmpty())
		{
			QFileInfo fi(fn);
			helpFilePath=fi.absFilePath();
			saveSettings();
		}
	}

	QFileInfo fi(helpFilePath);
	QString profilePath = QString(fi.dirPath(true)+"/scidavis.adp");
	if (!QFile(profilePath).exists())
	{
		QMessageBox::critical(this,tr("Help Profile Not Found!"),
				tr("The assistant could not start because the file <b>%1</b> was not found in the help file directory!").arg("scidavis.adp")+"<br>"+
				tr("This file is provided with the SciDAVis manual which can be downloaded from the following internet address:")+
				"<p><a href = http://sourceforge.net/project/showfiles.php?group_id=199120>http://sourceforge.net/project/showfiles.php?group_id=199120</a></p>");
		return;
	}

	QStringList cmdLst = QStringList() << "-profile" << profilePath;
	assistant->setArguments( cmdLst );
	assistant->showPage(helpFilePath);
*/
}

void ProjectWindow::chooseHelpFolder()
{
// TODO
/*
	QString dir = QFileDialog::getExistingDirectory(this, tr("Choose the location of the SciDAVis help folder!"),
			qApp->applicationDirPath());

	if (!dir.isEmpty())
	{
		helpFilePath = dir + "/index.html";

		QFile helpFile(helpFilePath);
		if (!helpFile.exists())
		{
			QMessageBox::critical(this, tr("index.html File Not Found!"),
					tr("There is no file called <b>index.html</b> in this folder.<br>Please choose another folder!"));
		}
	}
	*/
}

void ProjectWindow::downloadManual()
{
	QDesktopServices::openUrl(QUrl("http://sourceforge.net/project/showfiles.php?group_id=199120"));
}

void ProjectWindow::downloadTranslation()
{
	QDesktopServices::openUrl(QUrl("http://sourceforge.net/project/showfiles.php?group_id=199120"));
}

void ProjectWindow::showHomePage()
{
	QDesktopServices::openUrl(QUrl("http://scidavis.sf.net"));
}

void ProjectWindow::showForums()
{
	QDesktopServices::openUrl(QUrl("http://sourceforge.net/forum/?group_id=199120"));
}

void ProjectWindow::showBugTracker()
{
	QDesktopServices::openUrl(QUrl("http://sourceforge.net/tracker/?group_id=199120&atid=968214"));
}

void ProjectWindow::searchForUpdates()
{
    int choice = QMessageBox::question(this, SciDAVis::versionString() + SciDAVis::extraVersion(),
					tr("SciDAVis will now try to determine whether a new version of SciDAVis is available. Please modify your firewall settings in order to allow SciDAVis to connect to the internet.") + "\n" +
					tr("Do you wish to continue?"),
					QMessageBox::Yes|QMessageBox::Default, QMessageBox::No|QMessageBox::Escape);

    if (choice == QMessageBox::Yes)
    {
        d->version_buffer.open(QIODevice::WriteOnly);
        d->http.setHost("scidavis.sourceforge.net");
        d->http.get("/current_version.txt", &d->version_buffer);
    }
}

void ProjectWindow::receivedVersionFile(bool error)
{
	if (error)
	{
		QMessageBox::warning(this, tr("HTTP get version file"),
				tr("Error while fetching version file with HTTP: %1.").arg(d->http.errorString()));
		return;
	}

	d->version_buffer.close();

	if (d->version_buffer.open(QIODevice::ReadOnly))
	{
		QTextStream t( &d->version_buffer );
		t.setCodec("UTF-8");
		QString version_line = t.readLine();
		d->version_buffer.close();

		QStringList list = version_line.split(".");
		if(list.count() > 2)
		{
			int available_version = (list.at(0).toInt() << 16) + (list.at(1).toInt() << 8) + (list.at(2).toInt());

			if (available_version > SciDAVis::version())
			{
				if(QMessageBox::question(this, tr("Updates Available"),
							tr("There is a newer version of SciDAVis (%1) available for download. Would you like to download it now?").arg(version_line),
							QMessageBox::Yes|QMessageBox::Default, QMessageBox::No|QMessageBox::Escape) == QMessageBox::Yes)
					QDesktopServices::openUrl(QUrl("http://sourceforge.net/project/showfiles.php?group_id=199120"));
			}
			else
			{
				QMessageBox::information(this, SciDAVis::versionString() + SciDAVis::extraVersion(),
						tr("No updates available. Your are already running the latest version."));
			}
		}
		else QMessageBox::information(this, tr("Invalid version file"),
						tr("The version file (contents: \"%1\") could not be decoded into a valid version number.").arg(version_line));
	}
}

void ProjectWindow::nameUndoRedo()
{
	m_actions.undo->setText(tr("&Undo") + " " + m_project->undoStack()->undoText());
	m_actions.redo->setText(tr("&Redo") + " " + m_project->undoStack()->redoText());
}

void ProjectWindow::renameUndoRedo()
{
	m_actions.undo->setText(tr("&Undo"));
	m_actions.redo->setText(tr("&Redo"));
}

void ProjectWindow::undo()
{
	WAIT_CURSOR;
	m_project->undoStack()->undo();
	RESET_CURSOR;
}

void ProjectWindow::redo()
{
	WAIT_CURSOR;
	m_project->undoStack()->redo();
	RESET_CURSOR;
}


