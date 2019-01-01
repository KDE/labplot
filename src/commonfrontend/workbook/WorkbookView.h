/***************************************************************************
    File                 : WorkbookView.h
    Project              : LabPlot
    Description          : View class for Workbook
    --------------------------------------------------------------------
    Copyright            : (C) 2015 by Alexander Semke (alexander.semke@web.de)

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

#ifndef WORKBOOKVIEW_H
#define WORKBOOKVIEW_H

#include <QTabWidget>
#include <QTabBar>

class AbstractAspect;
class Workbook;
class QAction;
class QMenu;
class QPrinter;
class QToolBar;

class TabWidget : public QTabWidget {
	Q_OBJECT
public:
	explicit TabWidget(QWidget* parent) : QTabWidget(parent) {
		connect(tabBar(), SIGNAL(tabMoved(int, int)), this, SIGNAL(tabMoved(int, int)));
	}

signals:
	void tabMoved(int, int);
};

class WorkbookView : public QWidget {
	Q_OBJECT

public:
	explicit WorkbookView(Workbook*);
	~WorkbookView() override;

	int currentIndex() const;

private:
	TabWidget* m_tabWidget;
	Workbook* m_workbook;
	int lastSelectedIndex{0};
	bool m_initializing;

	//actions
	QAction* action_add_spreadsheet;
	QAction* action_add_matrix;

private  slots:
	void createContextMenu(QMenu*) const;
	void showTabContextMenu(QPoint);
	void addSpreadsheet();
	void addMatrix();
	void itemSelected(int);
	void tabChanged(int);
	void tabMoved(int, int);
	void handleDescriptionChanged(const AbstractAspect*);
	void handleAspectAdded(const AbstractAspect*);
	void handleAspectAboutToBeRemoved(const AbstractAspect*);
};

#endif
