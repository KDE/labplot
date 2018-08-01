/***************************************************************************
    File                 : MainWin.h
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2011-2018 Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2008-2018 by Stefan Gerlach (stefan.gerlach@uni.kn)
    Description          : Main window of the application
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
class Image;
class Spreadsheet;
class Matrix;
class GuiObserver;
class AxisDock;
class NoteDock;
class CartesianPlotDock;
class HistogramDock;
class BarChartPlotDock;
class CartesianPlotLegendDock;
class CustomPointDock;
class ColumnDock;
class LiveDataDock;
class MatrixDock;
class ProjectDock;
class SpreadsheetDock;
class XYCurveDock;
class XYEquationCurveDock;
class XYDataReductionCurveDock;
class XYDifferentiationCurveDock;
class XYIntegrationCurveDock;
class XYInterpolationCurveDock;
class XYSmoothCurveDock;
class XYFitCurveDock;
class XYFourierFilterCurveDock;
class XYFourierTransformCurveDock;
class XYConvolutionCurveDock;
class WorksheetDock;
class LabelWidget;
class DatapickerImageWidget;
class DatapickerCurveWidget;

#ifdef HAVE_CANTOR_LIBS
class CantorWorksheet;
class CantorWorksheetDock;
#endif

class QDockWidget;
class QStackedWidget;
class QDragEnterEvent;
class QDropEvent;
class QMdiArea;
class QMdiSubWindow;
class QToolButton;
class KRecentFilesAction;

class MainWin : public KXmlGuiWindow {
	Q_OBJECT

public:
	explicit MainWin(QWidget* parent = 0, const QString& filename=0);
	~MainWin();

	void showPresenter();
	AspectTreeModel* model() const;
	Project* project() const;
	void addAspectToProject(AbstractAspect*);

private:
	QMdiArea* m_mdiArea;
	QMdiSubWindow* m_currentSubWindow;
	Project* m_project;
	AspectTreeModel* m_aspectTreeModel;
	ProjectExplorer* m_projectExplorer;
	QDockWidget* m_projectExplorerDock;
	QDockWidget* m_propertiesDock;
	AbstractAspect* m_currentAspect;
	Folder* m_currentFolder;
	QString m_currentFileName;
	QString m_undoViewEmptyLabel;
	bool m_suppressCurrentSubWindowChangedEvent;
	bool m_closing;
	bool m_autoSaveActive;
	QTimer m_autoSaveTimer;
	Qt::WindowStates m_lastWindowState; //< last window state before switching to full screen mode

	KRecentFilesAction* m_recentProjectsAction;
	QAction* m_saveAction;
	QAction* m_saveAsAction;
	QAction* m_printAction;
	QAction* m_printPreviewAction;
	QAction* m_importFileAction;
	QAction* m_importSqlAction;
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
	QAction* m_historyAction;
	QAction* m_undoAction;
	QAction* m_redoAction;
	QAction* m_tileWindows;
	QAction* m_cascadeWindows;
	QAction* m_newDatapickerAction;
	QAction* m_editFitsFileAction;

	//toggling doch widgets
	QAction* m_toggleProjectExplorerDocQAction;
	QAction* m_togglePropertiesDocQAction;

	//worksheet actions
	QAction* worksheetZoomInAction;
	QAction* worksheetZoomOutAction;
	QAction* worksheetZoomOriginAction;
	QAction* worksheetZoomFitPageHeightAction;
	QAction* worksheetZoomFitPageWidthAction;
	QAction* worksheetZoomFitSelectionAction;

	QAction* worksheetNavigationModeAction;
	QAction* worksheetZoomModeAction;
	QAction* worksheetSelectionModeAction;

	QAction* worksheetVerticalLayoutAction;
	QAction* worksheetHorizontalLayoutAction;
	QAction* worksheetGridLayoutAction;
	QAction* worksheetBreakLayoutAction;

	QAction* m_visibilityFolderAction;
	QAction* m_visibilitySubfolderAction;
	QAction* m_visibilityAllAction;
	QAction* m_toggleProjectExplorerDockAction;
	QAction* m_togglePropertiesDockAction;

	//Menus
	QMenu* m_visibilityMenu;
	QMenu* m_newMenu;
	QMenu* m_importMenu;
	QMenu* m_editMenu;

	//Docks
	QStackedWidget* stackedWidget;
	AxisDock* axisDock;
	NoteDock* notesDock;
	CartesianPlotDock* cartesianPlotDock;
	CartesianPlotLegendDock* cartesianPlotLegendDock;
	ColumnDock* columnDock;
	LiveDataDock* m_liveDataDock;
	MatrixDock* matrixDock;
	SpreadsheetDock* spreadsheetDock;
	ProjectDock* projectDock;
	XYCurveDock* xyCurveDock;
	XYEquationCurveDock* xyEquationCurveDock;
	XYDataReductionCurveDock* xyDataReductionCurveDock;
	XYDifferentiationCurveDock* xyDifferentiationCurveDock;
	XYIntegrationCurveDock* xyIntegrationCurveDock;
	XYInterpolationCurveDock* xyInterpolationCurveDock;
	XYSmoothCurveDock* xySmoothCurveDock;
	XYFitCurveDock* xyFitCurveDock;
	XYFourierFilterCurveDock* xyFourierFilterCurveDock;
	XYFourierTransformCurveDock* xyFourierTransformCurveDock;
	XYConvolutionCurveDock* xyConvolutionCurveDock;
	HistogramDock* histogramDock;
	WorksheetDock* worksheetDock;
	LabelWidget* textLabelDock;
	CustomPointDock* customPointDock;
	DatapickerImageWidget* datapickerImageDock;
	DatapickerCurveWidget* datapickerCurveDock;

	void initActions();
	void initMenus();
	bool warnModified();
	void activateSubWindowForAspect(const AbstractAspect*) const;
	bool save(const QString&);

	Workbook* activeWorkbook() const;
	Spreadsheet* activeSpreadsheet() const;
	Matrix* activeMatrix() const;
	Worksheet* activeWorksheet() const;
	Datapicker* activeDatapicker() const;

	//Cantor
#ifdef HAVE_CANTOR_LIBS
	QMenu* m_newCantorWorksheetMenu;
	CantorWorksheetDock* cantorWorksheetDock;
	CantorWorksheet* activeCantorWorksheet() const;
#endif

	friend class GuiObserver;
	GuiObserver* m_guiObserver;

protected:
	void closeEvent(QCloseEvent*);
	void dragEnterEvent(QDragEnterEvent*);
	void dropEvent(QDropEvent*);

private slots:
	void initGUI(const QString&);
	void updateGUI();
	void updateGUIOnProjectChanges();
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

	void print();
	void printPreview();

	void historyDialog();
	void importFileDialog(const QString& fileName = QString());
	void importSqlDialog();
	void importProjectDialog();
	void exportDialog();
	void editFitsFileDialog();
	void settingsDialog();
	void projectChanged();
	void colorSchemeChanged(QAction*);

	//Cantor
#ifdef HAVE_CANTOR_LIBS
	void newCantorWorksheet(QAction* action);
#endif

	void newFolder();
	void newWorkbook();
	void newSpreadsheet();
	void newMatrix();
	void newWorksheet();
	void newNotes();
	void newDatapicker();
	//TODO: void newScript();
	void newLiveDataSourceActionTriggered();

	void createContextMenu(QMenu*) const;
	void createFolderContextMenu(const Folder*, QMenu*) const;

	void handleAspectAdded(const AbstractAspect*);
	void handleAspectAboutToBeRemoved(const AbstractAspect*);
	void handleAspectRemoved(const AbstractAspect*,const AbstractAspect*,const AbstractAspect*);
	void handleCurrentAspectChanged(AbstractAspect* );
	void handleCurrentSubWindowChanged(QMdiSubWindow*);
	void handleShowSubWindowRequested();

	void handleSettingsChanges();

	void setMdiWindowVisibility(QAction*);
	void updateMdiWindowVisibility() const;
	void toggleDockWidget(QAction*) const;
	void toggleFullScreen();
	void projectExplorerDockVisibilityChanged(bool);
	void propertiesDockVisibilityChanged(bool);
};

#endif
