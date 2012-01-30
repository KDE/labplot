/***************************************************************************
    File                 : MainWin.h
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2011 Alexander Semke (alexander.semke*web.de)
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
class AspectTreeModel;
class Folder;
class ProjectExplorer;
class Project;
class Worksheet;
class Spreadsheet;
class GuiObserver;
class QDockWidget;
class QStackedWidget;
class AxisDock;
class CartesianPlotDock;
class ColumnDock;
class XYCurveDock;
class SpreadsheetDock;
class WorksheetDock;

class MainWin : public KXmlGuiWindow{
	Q_OBJECT

public:
	MainWin(QWidget *   parent = 0, const QString& filename=0);
	~MainWin();

	Spreadsheet* activeSpreadsheet() const;
	Worksheet* activeWorksheet() const;

private:
	QMdiArea *m_mdiArea;
	Project *m_project;
	AspectTreeModel* m_aspectTreeModel;
	ProjectExplorer * m_projectExplorer;
	QDockWidget * m_projectExplorerDock;
	QDockWidget* m_propertiesDock;
	AbstractAspect * m_currentAspect;
	Folder * m_currentFolder;
	QString m_fileName; //name of the file to be opened (command line argument)
	QString m_undoViewEmptyLabel;

	KRecentFilesAction* m_recentProjectsAction;
	KAction* m_saveAction;
	KAction* m_saveAsAction;
	KAction* m_printAction;
	KAction* m_printPreviewAction;
	KAction* m_importAction;
	KAction* m_exportAction;
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

	//toggling doch widgets
	KAction* m_toggleProjectExplorerDockAction;
	KAction* m_togglePropertiesDockAction;
	
	//worksheet actions 
	KAction* worksheetZoomInAction;
	KAction* worksheetZoomOutAction;
	KAction* worksheetZoomOriginAction;
	KAction* worksheetZoomFitPageHeightAction;
	KAction* worksheetZoomFitPageWidthAction;
	KAction* worksheetZoomFitSelectionAction;

	KAction* worksheetNavigationModeAction;
	KAction* worksheetZoomModeAction;
	KAction* worksheetSelectionModeAction;

	KAction* worksheetVerticalLayoutAction;
	KAction* worksheetHorizontalLayoutAction;
	KAction* worksheetGridLayoutAction;
	KAction* worksheetBreakLayoutAction;
	
	KAction* m_visibilityFolderAction;
	KAction* m_visibilitySubfolderAction;
	KAction* m_visibilityAllAction;
	
	//Menus
	QMenu* m_visibilityMenu;
	QMenu* m_newMenu;
	
	//Docks
	QStackedWidget* stackedWidget;
	AxisDock* axisDock;
	CartesianPlotDock* cartesianPlotDock;
	SpreadsheetDock* spreadsheetDock;
	ColumnDock* columnDock;
	WorksheetDock* worksheetDock;
	XYCurveDock* lineSymbolCurveDock;
	
	void updateGUI();
	void openXML(QIODevice *file);

	void initActions();
	void initMenus();
	bool warnModified();
	bool hasSheet(const QModelIndex & index) const;
	void handleAspectAddedInternal(const AbstractAspect *aspect);
	void addAspectToProject(AbstractAspect* aspect);

	friend class GuiObserver;
	GuiObserver* m_guiObserver;
	
private slots:
	void initGUI();
	void undo();
	void redo();
	
	bool newProject();
	void openProject();
	void openProject(QString filename);
	void openRecentProject();
	bool closeProject();
	void saveProject();
	void saveProjectAs();
	
	void print();
	void printPreview();
	
	void historyDialog() const;
	void importFileDialog();
	void exportDialog();
	void projectDialog();
	void settingsDialog();
	void newPlotActionTriggered(QAction*);
	void functionPlotActionTriggered(QAction*);
	void dataPlotActionTriggered(QAction*);
	void projectChanged();

	void newFolder();
	void newSpreadsheet();
	void newWorksheet();
	void newScript();
	void newMatrix();
	void newFileDataSourceActionTriggered();
	void newSqlDataSourceActionTriggered();
	
	void createContextMenu(QMenu * menu) const;
	void createFolderContextMenu(const Folder * folder, QMenu * menu) const;
	
	void handleAspectAdded(const AbstractAspect *aspect);
	void handleAspectAboutToBeRemoved(const AbstractAspect *aspect);
	void handleAspectRemoved(const AbstractAspect *parent);
	void handleCurrentAspectChanged(AbstractAspect *aspect);
	void handleCurrentSubWindowChanged(QMdiSubWindow*);
	void handleSubWindowStatusChange(PartMdiView * view, PartMdiView::SubWindowStatus from, PartMdiView::SubWindowStatus to);
	
	void setMdiWindowVisibility(QAction*);
	void updateMdiWindowVisibility();
	void toggleDockWidget(QAction*) const;
	void expandAspect(const AbstractAspect*) const;
	
	void startTestCode();

signals:
	void partActivated(AbstractPart*);
};

#endif
