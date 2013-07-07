/***************************************************************************
    File                 : WorksheetView.h
    Project              : LabPlot/SciDAVis
    Description          : Worksheet view
    --------------------------------------------------------------------
    Copyright            : (C) 2009 Tilman Benkert (thzs*gmx.net)
    Copyright            : (C) 2009-2013 by Alexander Semke (alexander.semke*web.de)
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
#include "backend/worksheet/Worksheet.h"

class QMenu;
class QToolBar;
class QToolButton;
class QWheelEvent;
class QTimeLine;

class WorksheetModel;
class AbstractAspect;
class AbstractWorksheetElement;

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
	
	void setScene(QGraphicsScene*);
	void exportToFile(const QString&, const ExportFormat format, const ExportArea area);

  private:
	void initActions();
	void initMenus();
	
	void contextMenuEvent(QContextMenuEvent*);
	void wheelEvent(QWheelEvent*);
	void mousePressEvent(QMouseEvent*);
	void mouseReleaseEvent(QMouseEvent*);

	void drawBackground(QPainter*, const QRectF&);
	void exportPaint(QPainter* painter, const QRectF& targetRect, const QRectF& sourceRect);

	Worksheet *m_worksheet;
	WorksheetModel *m_model;
	MouseMode m_currentMouseMode;
	GridSettings m_gridSettings;
	QList<QGraphicsItem*> m_selectedItems;
	bool m_suppressSelectionChangedEvent;
	AbstractWorksheetElement* lastAddedWorksheetElement;
	QTimeLine* m_fadeInTimeLine;
	QTimeLine* m_fadeOutTimeLine;

	//Menus
	QMenu* m_addNewMenu;
	QMenu* m_zoomMenu;
	QMenu* m_layoutMenu;
	QMenu* m_gridMenu;

	QToolButton* tbZoom;
	QAction* currentZoomAction;

	//Actions
	QAction* zoomInAction;
	QAction* zoomOutAction;
	QAction* zoomOriginAction;
	QAction* zoomFitPageHeightAction;
	QAction* zoomFitPageWidthAction;
	QAction* zoomFitSelectionAction;
	
	QAction* navigationModeAction;
	QAction* zoomModeAction;
	QAction* selectionModeAction;
	
	QAction* addPlotAction;
	QAction* addTextLabelAction;
	
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

  public slots:
	void createContextMenu(QMenu*);
	void fillToolBar(QToolBar*);
	void print(QPrinter*) const;
	void selectItem(QGraphicsItem*);

  private slots:
	void enableNavigationMode();
	void enableZoomMode();
	void enableSelectionMode();

	void addNew(QAction*);
	void aspectAboutToBeRemoved(const AbstractAspect*);

	void changeZoom(QAction*);
	void changeLayout(QAction*);
	void changeGrid(QAction*);
	void changeSnapToGrid();

	void deselectItem(QGraphicsItem*);
	void selectionChanged();
	void updateBackground();
	void layoutChanged(Worksheet::Layout);

	void fadeIn(qreal);
	void fadeOut(qreal);

  signals:
	void statusInfo(const QString&);
};

#endif
