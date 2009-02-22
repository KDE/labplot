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
#include <KAction>
#include "core/PartMdiView.h"
// #include "elements/Set.h" //TODO remove
#include "plots/Plot.h"

class AbstractAspect;
class Folder;
class ProjectExplorer;
class Project;
class Worksheet;
class Table;

class MainWin : public KXmlGuiWindow
{
	Q_OBJECT
public:
	MainWin(QWidget *   parent = 0, const QString& filename=0);
	~MainWin();

	QMdiArea* getMdi() const { return m_mdi_area; }
	Table* activeTable() const;			//!< get active table
	Worksheet* activeWorksheet() const;			//!< get active worksheet
	Project* getProject() const { return m_project; }
	void setProject(Project *p) { m_project=p; }
	void updateGUI();		//!< update GUI of main window
	void updateSheetList();		//!< update dynamic sheet list menu
	void updateSetList();		//!< update dynamic set list menu
	void openXML(QIODevice *file);	//!< do the actual opening
	void saveXML(QIODevice *file);	//!< do the actual saving

private:
	QMdiArea *m_mdi_area;
	Project *m_project;
	KAction *m_newFolderAction;
	KAction *m_newSpreadsheetAction;
	KAction *m_newWorksheetAction;
	KAction *m_historyAction;
	KAction *m_undoAction;
	KAction *m_redoAction;
	void setupActions();
	void initProjectExplorer();
	bool warnModified();
	void ensureSheet();
	bool hasSheet(const QModelIndex & index) const;
// 	void addSet(Set s, const int sheet, const Plot::PlotType ptype);//TODO remove?
	AbstractAspect * m_current_aspect;
	Folder * m_current_folder;
	ProjectExplorer * m_project_explorer;
	QDockWidget * m_project_explorer_dock;
	void handleAspectAddedInternal(const AbstractAspect *aspect);
	QString m_fileName; //name of the file to be opened (command line argument)
	QString m_undoViewEmptyLabel;

public slots:
	Table* newSpreadsheet();
	Folder* newFolder();
	Worksheet* newWorksheet();
	void save(QString filename=0);	//!< save project (.lml format)
	void open(QString filename);	//!< open project (.lml format)
	void saveAs();	//!< save as different file name (.lml format)
	void showHistory();
	void createContextMenu(QMenu * menu) const;
	void createFolderContextMenu(const Folder * folder, QMenu * menu) const;
	void undo();
	void redo();
	//! Show/hide mdi windows depending on the currend folder
	void updateMdiWindowVisibility();

private slots:
	void initObject();
	void openNew();
	void print();
	void importDialog();
	void projectDialog();
	void settingsDialog();
	void newPlotActionTriggered(QAction*);
	void functionPlotActionTriggered(QAction*);
	void dataPlotActionTriggered(QAction*);

signals:
	void partActivated(AbstractPart*);

private slots:
	void handleAspectAdded(const AbstractAspect *aspect);
	void handleAspectAboutToBeRemoved(const AbstractAspect *aspect);
	void handleAspectRemoved(const AbstractAspect *parent);
	void handleAspectDescriptionChanged(const AbstractAspect *aspect);
	void handleCurrentAspectChanged(AbstractAspect *aspect);
	void handleCurrentSubWindowChanged(QMdiSubWindow*);
	void handleSubWindowStatusChange(PartMdiView * view, PartMdiView::SubWindowStatus from, PartMdiView::SubWindowStatus to);
	void setMdiWindowVisibility(QAction * action);
};

#endif
