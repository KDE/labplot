/***************************************************************************
    File                 : WorksheetView.h
    Project              : LabPlot/SciDAVis
    Description          : Worksheet view
    --------------------------------------------------------------------
    Copyright            : (C) 2009 Tilman Benkert (thzs*gmx.net)
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

#include <QWidget>
#include <QGraphicsView>
class Worksheet;
class QGraphicsView;
class WorksheetModel;
class QToolButton;
class QHBoxLayout;
class QMenu;
class ActionManager;

class WorksheetGraphicsView;

class WorksheetView: public QWidget {
	Q_OBJECT

	public:
		WorksheetView(Worksheet *worksheet);
		virtual ~WorksheetView();

	protected:
		void init();

		Worksheet *m_worksheet;
		WorksheetModel *m_model;
		WorksheetGraphicsView *m_view_widget;
		QWidget *m_control_tabs;
// TODO		Ui::ControlTabs ui;
		QToolButton *m_hide_button;
		QHBoxLayout *m_main_layout;

		void createActions();
		void connectActions();

		void createContextMenu(QMenu *menu);
		void fillProjectMenu(QMenu *menu, bool *rc);

	public slots:
		void toggleControlTabBar();

	protected:
		void retranslateStrings();

	signals:
		void statusInfo(const QString &text);

	private slots:
		void handleScaleFactorChange(qreal factor);

	public:
		static ActionManager *actionManager();
		static void initActionManager();
	private:
		static ActionManager *action_manager;
		WorksheetView();
};

class WorksheetGraphicsView : public QGraphicsView
{
	Q_OBJECT

	public:
		WorksheetGraphicsView(QWidget * parent = 0);
		~WorksheetGraphicsView();

		void setScene(QGraphicsScene * scene);

		void setScaleFactor(qreal factor);
		qreal scaleFactor() const;

	protected:
		virtual void drawBackground(QPainter *painter, const QRectF &rect);
		virtual void wheelEvent(QWheelEvent *event);

	signals:
		void scaleFactorChanged(qreal factor);

	private:
		qreal m_scale_factor;
		qreal horizontal_screen_dpi;
		qreal vertical_screen_dpi;
};


#endif


