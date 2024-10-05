/*
	File                 : NotebookView.h
	Project              : LabPlot
	Description          : View class for Notebeook
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2015 Garvit Khatri <garvitdelhi@gmail.com>
	SPDX-FileCopyrightText: 2016-2024 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef NOTEBOOKVIEW_H
#define NOTEBOOKVIEW_H

#include <QWidget>
#ifdef HAVE_CANTOR_LIBS
#include <cantor/session.h>
#endif

class QActionGroup;
class QMenu;
class QToolBar;

class Notebook;
class Column;

namespace KParts {
class ReadWritePart;
}

class NotebookView : public QWidget {
	Q_OBJECT

public:
	explicit NotebookView(Notebook*);
	~NotebookView() override;

public Q_SLOTS:
	void createContextMenu(QMenu*);
	void fillColumnContextMenu(QMenu*, Column*);
	void fillToolBar(QToolBar*);

private Q_SLOTS:
	void triggerAction(QAction*);

private:
	Notebook* m_notebook;
	KParts::ReadWritePart* m_part{nullptr};

	QActionGroup* m_actionGroup{nullptr};
	QAction* m_evaluateEntryAction{nullptr};
	QAction* m_removeCurrentEntryAction{nullptr};
	QAction* m_restartBackendAction{nullptr};
	QAction* m_evaluateWorsheetAction{nullptr};
	QAction* m_zoomIn{nullptr};
	QAction* m_zoomOut{nullptr};
	QAction* m_find{nullptr};
	QAction* m_replace{nullptr};
	QAction* m_statisticsAction{nullptr};
	Column* m_contextMenuColumn{nullptr};

	QMenu* m_addNewMenu{nullptr};
	QMenu* m_plotDataMenu{nullptr};
	QMenu* m_linearAlgebraMenu{nullptr};
	QMenu* m_calculateMenu{nullptr};
	QMenu* m_settingsMenu{nullptr};

	void initActions();
	void initMenus();

private Q_SLOTS:
	void statusChanged(Cantor::Session::Status);
	void plotData(QAction*);
	void showStatistics();
};

#endif // NOTEBOOKVIEW_H
