/***************************************************************************
    File                 : MainWin.h
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Stefan Gerlach (stefan.gerlach*uni-konstanz.de)
    Copyright            : (C) 2007-2008 Knut Franke (knut.franke*gmx.de)
    Copyright            : (C) 2007-2008 Tilman Benkert (thzs*gmx.net)
                           (replace * with @ in the email addresses)
    Description          : main class

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
#ifndef MAINWIN_H
#define MAINWIN_H
#include <KXmlGuiWindow>
#include <KRecentFilesAction>
#include "core/PartMdiView.h"
#include <QModelIndex>

class AbstractAspect;
class Folder;
class ProjectExplorer;
class Project;
class Worksheet;
class Spreadsheet;

class MainWin : public KXmlGuiWindow{
	Q_OBJECT

public:
	MainWin(QWidget *   parent = 0, const QString& filename=0);
	~MainWin();

	Spreadsheet* activeSpreadsheet() const;
	Worksheet* activeWorksheet() const;

private:
	QMdiArea *m_mdi_area;
	Project *m_project;
	ProjectExplorer * m_project_explorer;
	QDockWidget * m_project_explorer_dock;
	AbstractAspect * m_current_aspect;
	Folder * m_current_folder;
	QString m_fileName; //name of the file to be opened (command line argument)
	QString m_undoViewEmptyLabel;

	KRecentFilesAction* m_recentProjectsAction;
	KAction* m_saveAction;
	KAction* m_saveAsAction;
	KAction* m_printAction;
	KAction* m_printPreviewAction;
	KAction* m_importAction;
	KAction* m_projectInfoAction;
	KAction* m_closeAction;
	KAction *m_newFolderAction;
	KAction *m_newSpreadsheetAction;
	KAction *m_newMatrixAction;
	KAction *m_newWorksheetAction;
	KAction *m_newFileDataSourceAction;
	KAction *m_newSqlDataSourceAction;
	KAction *m_newScriptAction;
	KAction *m_newProjectAction;
	KAction *m_historyAction;
	KAction *m_undoAction;
	KAction *m_redoAction;

	void updateGUI();
	void openXML(QIODevice *file);
	void saveXML(QIODevice *file);

	void setupActions();
	void initProjectExplorer();
	bool warnModified();
	void ensureSheet();
	bool hasSheet(const QModelIndex & index) const;
	void handleAspectAddedInternal(const AbstractAspect *aspect);
	void addAspectToProject(AbstractAspect* aspect);

public slots:
	Spreadsheet* newSpreadsheet();
private slots:
	void initObject();
	void undo();
	void redo();
	
	void newProject();
	void openProject();
	void openProject(QString filename);
	void openRecentProject();
	void closeProject();
	void saveProject(QString filename=0);
	void saveProjectAs();
	
	void print();
	void printPreview();
	
	void showHistory() const;
	void importFileDialog();
	void projectDialog();
	void settingsDialog();
	void axesDialog();
	void newPlotActionTriggered(QAction*);
	void functionPlotActionTriggered(QAction*);
	void dataPlotActionTriggered(QAction*);
	void projectChanged();

	Folder* newFolder();
	Worksheet* newWorksheet();
	void newScript();
	void newMatrix();
	void newFileDataSourceActionTriggered();
	void newSqlDataSourceActionTriggered();
	
	void createContextMenu(QMenu * menu) const;
	void createFolderContextMenu(const Folder * folder, QMenu * menu) const;
	
	void handleAspectAdded(const AbstractAspect *aspect);
	void handleAspectAboutToBeRemoved(const AbstractAspect *aspect);
	void handleAspectRemoved(const AbstractAspect *parent);
	void handleAspectDescriptionChanged(const AbstractAspect *aspect);
	void handleCurrentAspectChanged(AbstractAspect *aspect);
	void handleCurrentSubWindowChanged(QMdiSubWindow*);
	void handleSubWindowStatusChange(PartMdiView * view, PartMdiView::SubWindowStatus from, PartMdiView::SubWindowStatus to);
	
	void setMdiWindowVisibility(QAction * action);
	void updateMdiWindowVisibility();
	
signals:
	void partActivated(AbstractPart*);
};

#endif
