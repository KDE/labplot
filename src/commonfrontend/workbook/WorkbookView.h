/*
    File                 : WorkbookView.h
    Project              : LabPlot
    Description          : View class for Workbook
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2015-2020 Alexander Semke <alexander.semke@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef WORKBOOKVIEW_H
#define WORKBOOKVIEW_H

#include <QWidget>

class AbstractAspect;
class Workbook;
class QAction;
class QMenu;
class QTabWidget;
class QToolBar;

class WorkbookView : public QWidget {
	Q_OBJECT

public:
	explicit WorkbookView(Workbook*);
	~WorkbookView() override;

	int currentIndex() const;

private:
	QTabWidget* m_tabWidget;
	Workbook* m_workbook;
	int lastSelectedIndex{0};
	bool m_initializing;

	//actions
	QAction* action_add_spreadsheet;
	QAction* action_add_matrix;

private Q_SLOTS:
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
