/***************************************************************************
    File                 : WorksheetView.h
    Project              : LabPlot/SciDAVis
    Description          : Worksheet view
    --------------------------------------------------------------------
    Copyright            : (C) 2009 Tilman Benkert (thzs*gmx.net)
    Copyright            : (C) 2010 by Alexander Semke (alexander.semke*web.de)
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

#include <QtGui>
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

	void createMenu(QMenu* menu=0) const;
	void setScene(QGraphicsScene * scene);

	static ActionManager *actionManager();
	static void initActionManager();

  private:
	void createActions();

	void contextMenuEvent(QContextMenuEvent *);
	void createContextMenu(QMenu *menu);
	void fillProjectMenu(QMenu *menu, bool *rc);

	void wheelEvent(QWheelEvent *event);
	void mouseReleaseEvent (QMouseEvent * event);

	void drawBackground(QPainter *painter, const QRectF &rect);
	void setScaleFactor(qreal factor);
	
	Worksheet *m_worksheet;
	WorksheetModel *m_model;
	MouseMode m_currentMouseMode;

	static ActionManager *action_manager;
	WorksheetView();

	qreal m_scaleFactor;
	qreal m_scaleFactorUpperLimit;
	qreal m_scaleFactorLowerLimit;

	//Actions
	#ifdef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
	QAction* zoomInAction;
	QAction* zoomOutAction;
	QAction* zoomOriginAction;
	QAction* zoomFitPageHeightAction;
	QAction* zoomFitPageWidthAction;
	
	QAction* navigationModeAction;
	QAction* zoomModeAction;
	QAction* selectionModeAction;
	
	QAction* verticalLayoutAction;
	QAction* horizontalLayoutAction;
	QAction* gridLayoutAction;
	QAction* breakLayoutAction;
	#else
	KAction* zoomInAction;
	KAction* zoomOutAction;
	KAction* zoomOriginAction;
	KAction* zoomFitPageHeightAction;
	KAction* zoomFitPageWidthAction;

	KAction* navigationModeAction;
	KAction* zoomModeAction;
	KAction* selectionModeAction;

	KAction* verticalLayoutAction;
	KAction* horizontalLayoutAction;
	KAction* gridLayoutAction;
	KAction* breakLayoutAction;
	#endif

  public slots:
	void zoomIn();
	void zoomOut();
	void zoomOrigin();
	void zoomFitPageWidth();
	void zoomFitPageHeight();

	void enableNavigationMode();
	void enableZoomMode();
	void enableSelectionMode();

	void layout(QAction*);

  private slots:
	void handleScaleFactorChange(qreal factor);
	void startTestCode();
	
  signals:
	void statusInfo(const QString &text);
	void scaleFactorChanged(qreal factor);
};

#endif
