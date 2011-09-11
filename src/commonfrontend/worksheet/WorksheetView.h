/***************************************************************************
    File                 : WorksheetView.h
    Project              : LabPlot/SciDAVis
    Description          : Worksheet view
    --------------------------------------------------------------------
    Copyright            : (C) 2009 Tilman Benkert (thzs*gmx.net)
    Copyright            : (C) 2009-2011 by Alexander Semke (alexander.semke*web.de)
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

#ifndef WORKSHEETVIEW_H
#define WORKSHEETVIEW_H

#include <QGraphicsView>

class QMenu;
class QWheelEvent;

class Worksheet;
class WorksheetModel;
class ActionManager;
#ifndef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
class KAction;
#endif

class WorksheetView : public QGraphicsView{
	Q_OBJECT

  public:
	WorksheetView(Worksheet *worksheet);
	virtual ~WorksheetView();

	enum MouseMode{NavigationMode, ZoomMode, SelectionMode};
	enum GridStyle{NoGrid, LineGrid, DotGrid};
	enum ExportFormat{Pdf, Eps, Svg, Png};
	enum ExportArea{ExportBoundingBox, ExportSelection, ExportWorksheet};

	struct GridSettings {
		GridStyle style;
		QColor color;
		int horizontalSpacing;
		int verticalSpacing;
		float opacity;
	};
	
	void createMenu(QMenu* menu=0) const;
	void setScene(QGraphicsScene * scene);
	void exportToFile(const QString&, const ExportFormat format, const ExportArea area) const;

	static ActionManager *actionManager();
	static void initActionManager();

  private:
	void initActions();
	void initMenus();
	
	void contextMenuEvent(QContextMenuEvent *);
	QMenu* createContextMenu();
	void fillProjectMenu(QMenu *menu, bool *rc);

	void wheelEvent(QWheelEvent *event);
	void mouseReleaseEvent (QMouseEvent * event);

	void drawBackground(QPainter *painter, const QRectF &rect);
	
	Worksheet *m_worksheet;
	WorksheetModel *m_model;
	MouseMode m_currentMouseMode;
	
	GridSettings m_gridSettings;
	
	static ActionManager *action_manager;
	WorksheetView();

	//Menus
	QMenu* m_zoomMenu;
	QMenu* m_layoutMenu;
	QMenu* m_gridMenu;
	
	//Actions
	#ifdef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
	QAction* zoomInAction;
	QAction* zoomOutAction;
	QAction* zoomOriginAction;
	QAction* zoomFitPageHeightAction;
	QAction* zoomFitPageWidthAction;
	QAction* zoomFitSelectionAction;
	
	QAction* navigationModeAction;
	QAction* zoomModeAction;
	QAction* selectionModeAction;
	
	QAction* verticalLayoutAction;
	QAction* horizontalLayoutAction;
	QAction* gridLayoutAction;
	QAction* breakLayoutAction;
	
	QAction* noGridAction;
	QAction* denseLineGridAction;
	QAction* sparseLineGridAction;
	QAction* denseDotGridAction;
	QAction* sparseDotGridAction;
	QAction* customGridAction;	
	QAction* snapToGridAction;	
	#else
	KAction* zoomInAction;
	KAction* zoomOutAction;
	KAction* zoomOriginAction;
	KAction* zoomFitPageHeightAction;
	KAction* zoomFitPageWidthAction;
	KAction* zoomFitSelectionAction;

	KAction* navigationModeAction;
	KAction* zoomModeAction;
	KAction* selectionModeAction;

	KAction* verticalLayoutAction;
	KAction* horizontalLayoutAction;
	KAction* gridLayoutAction;
	KAction* breakLayoutAction;
	
	KAction* noGridAction;
	KAction* denseLineGridAction;
	KAction* sparseLineGridAction;
	KAction* denseDotGridAction;
	KAction* sparseDotGridAction;
	KAction* customGridAction;
	KAction* snapToGridAction;
	#endif

  public slots:
	void zoomIn();
	void zoomOut();
	void zoomOrigin();
	void zoomFitPageWidth();
	void zoomFitPageHeight();
	void zoomFitSelection();

	void enableNavigationMode();
	void enableZoomMode();
	void enableSelectionMode();

	void changeLayout(QAction*);
	void changeGrid(QAction*);

	void print(QPrinter*) const;

  private slots:
	void selectItem(QGraphicsItem *);
	void deselectItem(QGraphicsItem*);
	void selectionChanged();
	
  signals:
	void statusInfo(const QString &text);
};

#endif
